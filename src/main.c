#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include <squirrel.h>

// Send a HID report with the given keycodes to the host.
static void send_hid_kbd_codes(uint8_t keycode_assembly[6]) {
  // skip if hid is not ready yet
  if (!tud_hid_ready()) {
    printf("HID not ready\n");
    return;
  };
  printf("sending HID.\n");
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
  printf("done sending HID.\n");
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
  // (such as 'a', 'b' or capslock).

  uint8_t keycode_assembly[6] = {
      0}; // The keycodes to send in the report. A max
          // of 6 keycodes can be regisered as currently pressed at once.
  uint_fast8_t index = 0; // The index of the keycode_assembly array.

  for (int i = 0; i <= 0xFF; i++) { // Loop through all keycodes.
    if (active_keycodes[i]) { // If the keycode is registered as active (pressed
                              // down),
      keycode_assembly[index] = i; // Add the keycode to the assembly array.
      index++;                     // Increment the index of the assembly array.
      printf("Keycode: %d\n", i);
      if (index >= 6) { // If the report is full, stop adding keycodes. (this
                        // ignores any keycodes after the 6th active keycode)
        break;
      }
    }
  }
  if (index > 0) { // If there are any keycodes to send, send them.
    printf("starting sending HID.\n");
    send_hid_kbd_codes(keycode_assembly);
  } else {
    printf("Sending nothing\n");
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

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  // This callback is not used, but is required by tinyusb.
  // Here you can receive data sent from the host to the device (such as
  // capslock LEDs, etc.)
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;
}

const int KEYBOARD_X = 1;
const int KEYBOARD_Y = 1;

void test_down(struct key *key, uint_fast8_t keycode, uint_fast8_t layer,
               bool (*layers)[16], uint_fast8_t *default_layer) {
  (void)key;
  (void)layer;
  active_keycodes[keycode] = true; // Mark the keycode as active.
}
void test_up(struct key *key, uint_fast8_t keycode, uint_fast8_t layer,
             bool (*layers)[16], uint_fast8_t *default_layer) {
  (void)key;
  (void)layer;
  active_keycodes[keycode] = false; // Mark the keycode as active.
}
struct key key1 = {
    .rising = {test_down},
    .risingargs = {HID_KEY_C},
    .falling = {test_up},
    .fallingargs = {HID_KEY_C},
};

struct key *keys[1] = {&key1};
const int PCA9555_ADDR = 0b0100000;
const int PCA9555_CMD_SET_INPUTS_0 = _u(0x00);
const int PCA9555_CMD_SET_INPUTS_1 = _u(0x01);
const int PCA9555_CMD_SET_OUTPUTS_0 = _u(0x02);
const int PCA9555_CMD_SET_OUTPUTS_1 = _u(0x03);
const int PCA9555_CMD_POLARITY_INVERT_0 = _u(0x04);
const int PCA9555_CMD_POLARITY_INVERT_1 = _u(0x05);
const int PCA9555_CMD_CONFIGURE_0 = _u(0x06);
const int PCA9555_CMD_CONFIGURE_1 = _u(0x07);

void check_keys() {
  uint8_t buf[2] = {PCA9555_CMD_SET_OUTPUTS_0, 0b11111111};
  i2c_write_blocking(&i2c1_inst, PCA9555_ADDR, buf, 2, false);

  uint8_t buf2[2] = {PCA9555_CMD_SET_OUTPUTS_1, 0b11111111};
  i2c_write_blocking(&i2c1_inst, PCA9555_ADDR, buf2, 2, false);
  //  buf[1] = buf[1] << 1;
  //  for (int y = 0; y < KEYBOARD_Y; y++) {
  bool r1 = gpio_get(1);
  check_key(keys[0], r1, &layers, &default_layer);
  //  }
  //}
}

void pca9555_init(void) {
  stdio_init_all();
  uart_init(uart0, 115200);

  i2c_init(&i2c1_inst, 100000);

  gpio_set_function(6, GPIO_FUNC_I2C);
  gpio_set_function(7, GPIO_FUNC_I2C);
  gpio_pull_up(6);
  gpio_pull_up(7);

  uint8_t buf[2] = {PCA9555_CMD_CONFIGURE_0, 0b00000000};
  i2c_write_blocking(&i2c1_inst, PCA9555_ADDR, buf, 2, false);
  buf[0] = PCA9555_CMD_CONFIGURE_1;
  i2c_write_blocking(&i2c1_inst, PCA9555_ADDR, buf, 2, false);

  gpio_init(14);
  gpio_set_dir(14, GPIO_IN);
  gpio_pull_down(14);

  gpio_init(25);
  gpio_set_dir(25, GPIO_OUT);
  gpio_put(25, 0);
};

// The main function, runs tinyusb and the key scanning loop.
int main(void) {
  board_init();               // Initialize the pico board
  tud_init(BOARD_TUD_RHPORT); // Initialize tinyusb device stack
  tusb_init();                // Initialize tinyusb
  pca9555_init();             // Initialize the PCA9555 I2C GPIO expander

  while (true) {
    check_keys(); // Check the keys on the keyboard for their state.
    tud_task();   // tinyusb device task.
    hid_task();   // Send HID reports to the host.
  }
}
