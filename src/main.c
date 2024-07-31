#include "bsp/board.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "keymap.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico_pca9555.h"
#include "quadrature_encoder.pio.h"
#include "ssd1306.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "ws2812.pio.h"
#include <math.h>
#include <squirrel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// I2C mutex
mutex_t i2c_mutex;

uint64_t last_interaction =
    0; // the board_millis() value of the last
       // interaction with the keyboard. Used to trigger screensaver.

// The time in ms that the keyboard will wait before being detected as idle. Set
// to UINT64_MAX to (effictivly) disable. (585 million years).
uint64_t idle_timeout = 30000;

uint16_t cps = 0; // cps = characters per second
uint16_t wpm = 0; // wpm = words per minute ( assuming 5 characters per word )

// neopixel helpers
#define NUM_PIXELS 90
#define WS2812_PIN 26
uint8_t leds[NUM_PIXELS * 3] = {0}; // The state of each LED in the LED strip.

static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// USB HID

// Send a HID report with the given keycodes to the host.
static void send_hid_kbd_codes(uint8_t keycode_assembly[6]) {
  // skip if hid is not ready yet
  if (!tud_hid_ready()) {
    return;
  };
  tud_hid_keyboard_report(
      REPORT_ID_KEYBOARD, modifiers,
      keycode_assembly); // Send the report. A report can be for a keyboard,
                         // mouse, joystick etc. In a keyboard report, the first
                         // argument is the report ID, the second is the
                         // modifiers (ctrl, shift, alt etc.). These are stored
                         // as a bitfield. The third is the keycodes to send.
                         // All keycodes that are sent are considered currently
                         // pressed. Detecting key presses and releases is done
                         // by the host. The only requirement from the firmware
                         // is that it sends a report with all currently pressed
                         // keys every 10ms.
}

// Send a HID report with no keycodes to the host.
static void send_hid_kbd_null() {
  // skip if hid is not ready yet
  if (!tud_hid_ready()) {
    return;
  };
  tud_hid_keyboard_report(
      REPORT_ID_KEYBOARD, modifiers,
      NULL); // Send a report with no keycodes. (no keys pressed)
}

// Every 10ms, we will sent 1 report for each HID device. In this case, we only
// have 1 HID device, the keyboard.
void hid_task(void) {
  // Poll every 10ms
  const uint32_t interval_ms = 10;    // Time between each report
  static uint32_t next_report_ms = 0; // Time of next report

  if (board_millis() - next_report_ms < interval_ms) { // If we are running too
                                                       // fast, return.
    return;
  };
  next_report_ms += interval_ms; // Schedule next report

  // Reports are sent in a chain, with one report for each HID device.

  // First, send the keyboard report. In a keyboard report, 6 keycodes can be
  // registered as pressed at once. A keycode is a number that represents a key
  // (such as 'a', 'b', '1', '2', etc).

  uint8_t keycode_assembly[6] = {
      0}; // The keycodes to send in the report. A max
          // of 6 keycodes can be regisered as currently pressed at once.
  uint_fast8_t index = 0; // The index of the keycode_assembly array.

  for (int i = 0; i <= 0xFF; i++) { // Loop through all keycodes.
    if (active_keycodes[i]) { // If the keycode is registered as active (pressed
                              // down),
      keycode_assembly[index] = i; // Add the keycode to the assembly array.
      index++;                     // Increment the index of the assembly array.
      if (index >= 6) { // If the report is full, stop adding keycodes. (this
                        // ignores any keycodes after the 6th active keycode)
        break;
      }
    }
  }
  // If there are any keycodes to send, send them.
  if (index > 0) {
    send_hid_kbd_codes(keycode_assembly);
  } else {
    send_hid_kbd_null();
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report,
                                uint16_t len) {
  if (report[0] == REPORT_ID_KEYBOARD) {
    // Keyboard report is done. Now, send the media key report.
    tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &active_media_code,
                   2); // Send the report.
    return;
  }

  (void)instance;
  (void)report;
  (void)len;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  // This callback is not used, but is required by tinyusb.
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;
  return 0;
}
uint8_t temp_leds[NUM_PIXELS * 3] = {0};

uint16_t data_index = 0;

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  // report_id was the first byte of the buffer, report_id was removed from the
  // buffer before this function was called.

  if (report_id == 0b00000000) {
    memcpy(leds, temp_leds, NUM_PIXELS * 3);
    data_index = 0;
    return;
  }
  if (bufsize == 60) {
    // Run length decode.
    for (uint8_t i = 0; i < 60; i += 4) {
      uint8_t length = buffer[i];
      if (length == 0) {
        break;
      }
      uint8_t r = buffer[i + 1];
      uint8_t g = buffer[i + 2];
      uint8_t b = buffer[i + 3];
      for (uint8_t j = 0; j < length; j++) {
        temp_leds[data_index] = r;
        temp_leds[data_index + 1] = g;
        temp_leds[data_index + 2] = b;
        data_index += 3;
      }
    }
  }

  // Recived data from the host.
  if (report_type == HID_REPORT_TYPE_OUTPUT) {
    // Keyboard type report
    if (report_id == REPORT_ID_KEYBOARD) {
      // Set keyboard LED e.g Capslock, Numlock etc...
      // bufsize should be (at least) 1
      if (bufsize < 1) {
        return;
      }

      uint8_t const kbd_leds = buffer[0];
      if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
        /*put_pixel(urgb_u32(255, 0, 0));*/
      } else if (kbd_leds & KEYBOARD_LED_NUMLOCK) {
        /*put_pixel(urgb_u32(0, 255, 0));*/
      } else if (kbd_leds & KEYBOARD_LED_SCROLLLOCK) {
        /*put_pixel(urgb_u32(0, 0, 255));*/
      } else {
        /*put_pixel(0);*/
      }
    }
  }
}

// 3 address pins are grounded.
// See https://www.nxp.com/docs/en/data-sheet/PCA9555.pdf for more details.
const uint8_t PCA9555_ADDR = 0b0100000;

// outputs_lookup is a lookup table that provides the correct pin outputs from
// the PCA9555 chip for each column of the keyboard.
const uint16_t outputs_lookup[16] = {
    0b0000000010000000, // physical column 1
    0b0000000001000000, // 2
    0b0000000000100000, // 3
    0b0000000000010000, // 4
    0b0000000000000001, // 5
    0b0000000000000010, // 6
    0b0000000000000100, // 7
    0b0000000000001000, // 8
    0b0100000000000000, // 9
    0b0010000000000000, // 10
    0b0001000000000000, // 11
    0b0000100000000000, // 12
    0b0000010000000000, // 13
    0b0000001000000000, // 14
    0b0000000100000000, // 15
    0b1000000000000000, // unused - not connected
};

void any_keypress(void) {
  last_interaction = board_millis();
  tud_remote_wakeup();
}

// debounce checks keys twice over 200us and if the key is still in the same
// position, it counts is as confirmed and runs check_key. This prevents
// chatter. (false key activations)
void debounce(uint8_t column) {
  // Get the state of all keys in the column
  bool r1 = gpio_get(1);
  bool r2 = gpio_get(0);
  bool r3 = gpio_get(29);
  bool r4 = gpio_get(28);
  bool r5 = gpio_get(27);

  bool r1_prev = r1;
  bool r2_prev = r2;
  bool r3_prev = r3;
  bool r4_prev = r4;
  bool r5_prev = r5;

  // Wait for 10us
  sleep_us(10);

  // Get the state of all keys in the column again.
  r1 = gpio_get(1);
  r2 = gpio_get(0);
  r3 = gpio_get(29);
  r4 = gpio_get(28);
  r5 = gpio_get(27);

  // If the key is still in the same state after 20ms, run check_key.
  // Also, if any key is pressed, update the last_interaction time.
  if (r1 == r1_prev) {
    check_key(keys[0][column], r1);
    if (r1) {
      any_keypress();
    }
  }
  if (r2 == r2_prev) {
    check_key(keys[1][column], r2);
    if (r2) {
      any_keypress();
    }
  }
  if (r3 == r3_prev) {
    check_key(keys[2][column], r3);
    if (r3) {
      any_keypress();
    }
  }
  if (r4 == r4_prev) {
    check_key(keys[3][column], r4);
    if (r4) {
      any_keypress();
    }
  }
  if (r5 == r5_prev) {
    check_key(keys[4][column], r5);
    if (r5) {
      any_keypress();
    }
  }
}

// check_keys loops through all columns and runs a check on each key.
void check_keys() {
  // PCA9555 uses two sets of 8-bit outputs
  // Loop through all columns
  if (mutex_try_enter(&i2c_mutex, NULL)) {
    for (uint8_t x = 0; x < 15; x++) {
      uint16_t column_outputs = outputs_lookup[x];
      pca9555_output(&i2c1_inst, PCA9555_ADDR, column_outputs);
      debounce(x);
    }
    mutex_exit(&i2c_mutex);
  };
}

void row_setup(void) {
  // Configure the row pins as inputs with pull-down resistors.
  gpio_init(1);
  gpio_set_dir(1, GPIO_IN);
  gpio_pull_down(1);

  gpio_init(0);
  gpio_set_dir(0, GPIO_IN);
  gpio_pull_down(0);

  gpio_init(29);
  gpio_set_dir(29, GPIO_IN);
  gpio_pull_down(29);

  gpio_init(28);
  gpio_set_dir(28, GPIO_IN);
  gpio_pull_down(28);

  gpio_init(27);
  gpio_set_dir(27, GPIO_IN);
  gpio_pull_down(27);
}

// LED strip

PIO led_pio = pio0;

void led_init(void) {
  uint led_pio_offset = pio_add_program(led_pio, &ws2812_program);
  uint led_sm = pio_claim_unused_sm(led_pio, true);
  ws2812_program_init(led_pio, led_sm, led_pio_offset, WS2812_PIN, 800000,
                      false);
}

void led_task(void) {
  // Update the LED strip with the new data.
  for (int i = 0; i < 90; i++) {
    put_pixel(urgb_u32(leds[i * 3], leds[i * 3 + 1], leds[i * 3 + 2]));
  }
  sleep_ms(1);
}

// Rotary Encoder

#define ROTARY_A_PIN 3 // The B pin must be the A pin + 1.
#define ROTARY_SW_PIN 2

PIO rot_pio = pio1;
int rotary_value, rotary_delta, rotary_last_value = 0;
const uint rot_sm = 0; // must be loaded at 0

void rotary_init(void) {
  pio_add_program(rot_pio, &quadrature_encoder_program);
  quadrature_encoder_program_init(rot_pio, rot_sm, ROTARY_A_PIN, 0);
}

void rotary_task(void) {
  rotary_value = -(quadrature_encoder_get_count(rot_pio, rot_sm) / 2);
  rotary_delta = rotary_value - rotary_last_value;
  rotary_last_value = rotary_value;
  if (rotary_delta != 0) {
    last_interaction = board_millis();
  }
}

// I2C Display
ssd1306_t display;

char screensaver_text[14] = {'S', 'L', 'A', 'B', ' ', 'K', 'E',
                             'Y', 'B', 'O', 'A', 'R', 'D', ' '};

void draw_screensaver(int frame) {
  // write 'SLAB KEYBOARD' waving across the screen
  for (uint8_t x = 0; x < 14; x++) {
    int y = 2 * sin(-frame / 10.0 + x * 2) + 8;
    if (y < 0) {
      y = 0;
    }
    if (y > 32) {
      y = 32;
    }
    int letter_x = x * 22 + (-frame % 384) + (22 * 3);
    if (letter_x < 0) {
      letter_x += 384;
    }
    if (letter_x > 384) {
      letter_x -= 384;
    }
    ssd1306_draw_char(&display, letter_x, y, 3, screensaver_text[x]);
    ssd1306_draw_char(&display, letter_x - 384, y, 3, screensaver_text[x]);
  }
}

void draw_homescreen(int frame) {
  // Layer number display
  ssd1306_draw_empty_square(&display, 0, 20, 15, 10);
  char layer_number[2];
  for (int i = 15; i >= default_layer; i--) { // 15-0
    if (!layers[i]) {
      continue;
    }
    sprintf(layer_number, "%d", i);
    if (i < 10) { // If the number is less than 10, add a 0 to the start
      layer_number[1] = layer_number[0];
      layer_number[0] = '0';
    }
    break;
  }
  ssd1306_draw_string(&display, 2, 22, 1, layer_number);
  // WPM display
  wpm = cps * 60 / 5;
  char wpm_str[4];
  sprintf(wpm_str, "%d", wpm);
  ssd1306_draw_string(&display, 0, 0, 1, wpm_str);
  // CPS display
  char cps_str[4];
  sprintf(cps_str, "%d", cps);
  ssd1306_draw_string(&display, 0, 10, 1, cps_str);
}

void display_task(void) {
  ssd1306_clear(&display);

  if (board_millis() - last_interaction > idle_timeout) {
    draw_screensaver(board_millis() / 10);
  } else {
    draw_homescreen(board_millis() / 10);
  }

  // Update the display.
  mutex_enter_blocking(&i2c_mutex);
  ssd1306_show(&display);
  mutex_exit(&i2c_mutex);
}

void i2c_devices_init(void) {
  // Initialize the I2C bus.
  i2c_init(&i2c1_inst, 400000); // 400kHz

  // Configure the I2C pins.
  gpio_set_function(6, GPIO_FUNC_I2C);
  gpio_set_function(7, GPIO_FUNC_I2C);

  // Initialize the I2C mutex.
  mutex_init(&i2c_mutex);

  // We don't need to lock the I2C mutex here because this function is run
  // before the multicore loop.
  pca9555_configure(&i2c1_inst, PCA9555_ADDR, 0x0000);
  ssd1306_init(&display, 128, 32, 0x3C, i2c1);
  ssd1306_set_rotation(&display, ROT_180);
}

// Core 1 deals with the LED strip, rotary encoder and OLED display.
void core1_main() {
  while (true) {
    led_task();
    rotary_task();
    display_task();
  }
}

// Core 0 deals with keyboard and USB HID.
void core0_main() {
  while (true) {
    check_keys(); // Check the keys on the keyboard for their states.
    tud_task();   // tinyusb device task.
    hid_task();   // Send HID reports to the host.
  }
}

// The main function, runs tinyusb and the key scanning loop.
int main(void) {
  board_init();               // Initialize the pico board
  tud_init(BOARD_TUD_RHPORT); // Initialize the tinyusb device stack
  tusb_init();                // Initialize tinyusb

  make_keys();        // Initialize the keys on the keyboard
  row_setup();        // Initialize the rows of the keyboard
  led_init();         // Initialize the WS2812 LED strip
  rotary_init();      // Initialize the rotary encoder
  i2c_devices_init(); // Initialize the I2C devices

  // Core 1 loop
  multicore_launch_core1(core1_main);
  // Core 0 loop
  core0_main();
}
