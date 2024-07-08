#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "hardware/pio.h"
#include "pico_pca9555.h"
#include "squirrel_constructors.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "ws2812.pio.h"
#include <squirrel.h>

#define NUM_PIXELS 90
#define WS2812_PIN 26

static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

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
  // This callback is not used, but is required by tinyusb.
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

uint8_t leds[270] = {0}; // The state of each LED in the LED strip.

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  // report_id was the first byte of the buffer, report_id was removed from the
  // buffer before this function was called.

  // Check if the report is for the LED strip.
  if (bufsize == 63) {
    if (report_id == 0b00000000) {
      // Keyboard part 1 (63 bytes) - 21 LEDs
      memcpy(leds, buffer, 63);
      return;
    }
    if (report_id == 0b00000001) {
      // Keybaord part 2 (63 bytes) - 21 LEDs
      memcpy(leds + 63, buffer, 63);
      return;
    }
    if (report_id == 0b00000010) {
      // Keybaord part 3 (63 bytes) - 21 LEDs
      memcpy(leds + 126, buffer, 63);
      return;
    }
    if (report_id == 0b00000011) {
      // Keybaord part 4 (63 bytes) - 21 LEDs
      memcpy(leds + 189, buffer, 63);
      return;
    }
    if (report_id == 0b00000100) {
      // Keybaord part 5 (18 bytes) - 6 LEDs + LED bar (45 bytes) - 15 LEDs
      memcpy(leds + 207, buffer, 63);
      return;
    }
  }
  if (bufsize == 45) {
    if (report_id == 0b00000101) {
      // LED bar only (18 bytes) - 6 LEDs
      memcpy(leds + 225, buffer, 45);
      return;
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

// Width and height of the keyboard matrix.
const int KEYBOARD_X = 15;
const int KEYBOARD_Y = 5;

struct key *keys[5][15]; // Pointer array to the key array.

struct key key_array[5][15]; // The base key array.

void make_keys(void) {
  // Row 1
  key_array[0][0] = make_key(HID_KEY_ESCAPE);
  key_array[0][1] = make_key(HID_KEY_1);
  key_array[0][2] = make_key(HID_KEY_2);
  key_array[0][3] = make_key(HID_KEY_3);
  key_array[0][4] = make_key(HID_KEY_4);
  key_array[0][5] = make_key(HID_KEY_5);
  key_array[0][6] = make_key(HID_KEY_6);
  key_array[0][7] = make_key(HID_KEY_7);
  key_array[0][8] = make_key(HID_KEY_8);
  key_array[0][9] = make_key(HID_KEY_9);
  key_array[0][10] = make_key(HID_KEY_0);
  key_array[0][11] = make_key(HID_KEY_MINUS);
  key_array[0][12] = make_key(HID_KEY_EQUAL);
  key_array[0][13] = make_key(HID_KEY_MENU);
  key_array[0][14] = make_key(HID_KEY_CAPS_LOCK);

  // Row 2
  key_array[1][0] = make_key(HID_KEY_TAB);
  key_array[1][1] = make_key(HID_KEY_Q);
  key_array[1][2] = make_key(HID_KEY_D);
  key_array[1][3] = make_key(HID_KEY_R);
  key_array[1][4] = make_key(HID_KEY_W);
  key_array[1][5] = make_key(HID_KEY_B);
  key_array[1][6] = make_key(HID_KEY_J);
  key_array[1][7] = make_key(HID_KEY_F);
  key_array[1][8] = make_key(HID_KEY_U);
  key_array[1][9] = make_key(HID_KEY_P);
  key_array[1][10] = make_key(HID_KEY_SEMICOLON);
  key_array[1][11] = make_key(HID_KEY_BRACKET_LEFT);
  key_array[1][12] = make_key(HID_KEY_BRACKET_RIGHT);
  key_array[1][13] = make_key(HID_KEY_BACKSLASH);
  key_array[1][14] = make_key(HID_KEY_GRAVE);

  // Row 3
  key_array[2][0] = make_key(HID_KEY_BACKSPACE);
  key_array[2][1] = make_key(HID_KEY_A);
  key_array[2][2] = make_key(HID_KEY_S);
  key_array[2][3] = make_key(HID_KEY_H);
  key_array[2][4] = make_key(HID_KEY_T);
  key_array[2][5] = make_key(HID_KEY_G);
  key_array[2][6] = make_key(HID_KEY_Y);
  key_array[2][7] = make_key(HID_KEY_N);
  key_array[2][8] = make_key(HID_KEY_E);
  key_array[2][9] = make_key(HID_KEY_O);
  key_array[2][10] = make_key(HID_KEY_I);
  key_array[2][11] = make_key(HID_KEY_APOSTROPHE);
  key_array[2][12] = make_key(HID_KEY_ENTER);
  key_array[2][13] = make_key(HID_KEY_DELETE);
  key_array[2][14] = make_key(HID_KEY_PRINT_SCREEN);

  // Row 4
  key_array[3][0] = make_modifier(KEYBOARD_MODIFIER_LEFTSHIFT);
  key_array[3][1] = make_key(HID_KEY_Z);
  key_array[3][2] = make_key(HID_KEY_X);
  key_array[3][3] = make_key(HID_KEY_M);
  key_array[3][4] = make_key(HID_KEY_C);
  key_array[3][5] = make_key(HID_KEY_V);
  key_array[3][6] = make_key(HID_KEY_K);
  key_array[3][7] = make_key(HID_KEY_L);
  key_array[3][8] = make_key(HID_KEY_COMMA);
  key_array[3][9] = make_key(HID_KEY_PERIOD);
  key_array[3][10] = make_key(HID_KEY_SLASH);
  key_array[3][11] = make_key(0x65); // Compose?
  key_array[3][12] = make_key(HID_KEY_PAGE_UP);
  key_array[3][13] = make_key(HID_KEY_ARROW_UP);
  key_array[3][14] = make_key(HID_KEY_PAGE_DOWN);

  // Row 5
  key_array[4][0] = make_modifier(KEYBOARD_MODIFIER_LEFTCTRL);
  key_array[4][1] = make_key(HID_KEY_GUI_LEFT);
  key_array[4][2] = make_modifier(KEYBOARD_MODIFIER_LEFTALT);
  key_array[4][3] = make_key(HID_KEY_VOLUME_DOWN);
  key_array[4][4] = make_key(HID_KEY_MUTE);        // TODO: layers
  key_array[4][5] = make_key(HID_KEY_VOLUME_UP);   // TODO: layers
  key_array[4][6] = make_key(HID_KEY_SPACE);       // TODO: layers
  key_array[4][7] = make_key(HID_KEY_AGAIN);       // TODO: Media Controls
  key_array[4][8] = make_key(HID_KEY_FIND);        // TODO: Media Controls
  key_array[4][9] = make_key(HID_KEY_APPLICATION); // TODO: Media Controls
  key_array[4][10] = make_key(HID_KEY_HOME);
  key_array[4][11] = make_key(HID_KEY_END);
  key_array[4][12] = make_key(HID_KEY_ARROW_LEFT);
  key_array[4][13] = make_key(HID_KEY_ARROW_DOWN);
  key_array[4][14] = make_key(HID_KEY_ARROW_RIGHT);

  // Fill the keys array with pointers to the key_array.
  for (uint8_t y = 0; y < KEYBOARD_Y; y++) {
    for (uint8_t x = 0; x < KEYBOARD_X; x++) {
      keys[y][x] = &key_array[y][x];
    }
  }
};

const uint8_t PCA9555_ADDR = 0b0100000; // 3 address pins are grounded.

const uint16_t outputs_lookup[16] = {
    0b0000000010000000, // 15
    0b0000000001000000, // 14
    0b0000000000100000, // 13
    0b0000000000010000, // 12
    0b0000000000001000, // 11
    0b0000000000000100, // 10
    0b0000000000000010, // 9
    0b0001000000000000, // 8
    0b0010000000000000, // 7
    0b0100000000000000, // 6
    0b1000000000000000, // 5
    0b0000100000000000, // 4
    0b0000010000000000, // 3
    0b0000001000000000, // 2
    0b0000000100000000, // 1
};

void check_keys() {
  // PCA9555 uses two sets of 8-bit outputs
  // Loop through all columns
  for (uint8_t x = 0; x < KEYBOARD_X; x++) {
    uint16_t column_outputs = outputs_lookup[x];
    pca9555_output(&i2c1_inst, PCA9555_ADDR, column_outputs);

    // Get the state of all keys in the column
    bool r1 = gpio_get(1);
    /*bool r2 = gpio_get(0);*/
    bool r3 = gpio_get(29);
    bool r4 = gpio_get(28);
    bool r5 = gpio_get(27);

    // Check the state of each key in the column for changes.
    check_key(keys[0][x], r1, &layers, &default_layer);
    /*check_key(keys[1][x], r2, &layers, &default_layer);*/
    check_key(keys[2][x], r3, &layers, &default_layer);
    check_key(keys[3][x], r4, &layers, &default_layer);
    check_key(keys[4][x], r5, &layers, &default_layer);
  }
}

void pca9555_init(void) {
  // Initialize the I2C bus.
  i2c_init(&i2c1_inst, 100000);

  // Configure the I2C pins.
  gpio_set_function(6, GPIO_FUNC_I2C);
  gpio_set_function(7, GPIO_FUNC_I2C);
  gpio_pull_up(6);
  gpio_pull_up(7);

  // Configure the PCA9555's pins as outputs.
  pca9555_configure(&i2c1_inst, PCA9555_ADDR, 0x0000);
};

void debugging_init(void) {
  stdio_init_all();
  uart_init(uart0, 115200); // UART debugging

  // Configure the board's LED pins as an output for debugging.
  gpio_init(25);
  gpio_set_dir(25, GPIO_OUT);
  gpio_put(25, 0);
}

void row_setup(void) {
  // Configure the row pins as inputs with pull-down resistors.
  gpio_init(1);
  gpio_set_dir(1, GPIO_IN);
  gpio_pull_down(1);

  /*  gpio_init(0);*/
  /*gpio_set_dir(0, GPIO_IN);*/
  /*gpio_pull_down(0);*/

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

void led_init(void) {
  // Initialize the PIO state machine.
  PIO pio = pio0;
  uint offset = pio_add_program(pio, &ws2812_program);
  uint sm = pio_claim_unused_sm(pio, true);
  ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
}

void led_task(void) {
  // Update the LED strip with the new data.
  for (int i = 0; i < 90; i++) {
    put_pixel(urgb_u32(leds[i * 3], leds[i * 3 + 1], leds[i * 3 + 2]));
  }
  sleep_ms(1);
}

void core1_main() {
  while (true) {
    led_task();
  }
}

void core0_main() {
  while (true) {
    check_keys(); // Check the keys on the keyboard for their states.
    tud_task();   // tinyusb device task.
    hid_task();   // Send HID reports to the host.
  }
}

// The main function, runs tinyusb and the key scanning loop.
int main(void) {
  debugging_init(); // Initialize debugging utilities

  board_init();               // Initialize the pico board
  tud_init(BOARD_TUD_RHPORT); // Initialize the tinyusb device stack
  tusb_init();                // Initialize tinyusb

  make_keys();    // Initialize the keys on the keyboard
  row_setup();    // Initialize the rows of the keyboard
  pca9555_init(); // Initialize the PCA9555 I2C GPIO expander
  led_init();     // Initialize the WS2812 LED strip

  // Core 1 loop
  multicore_launch_core1(core1_main);
  // Core 0 loop
  core0_main();
}
