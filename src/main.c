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

// Width and height of the keyboard matrix.
const int KEYBOARD_X = 15;
const int KEYBOARD_Y = 5;

struct key key1 = {
    .rising = {key_down},
    .risingargs = {HID_KEY_A},
    .falling = {key_up},
    .fallingargs = {HID_KEY_A},
};
struct key key2 = {
    .rising = {key_down},
    .risingargs = {HID_KEY_B},
    .falling = {key_up},
    .fallingargs = {HID_KEY_B},
};
struct key key3 = {
    .rising = {key_down},
    .risingargs = {HID_KEY_C},
    .falling = {key_up},
    .fallingargs = {HID_KEY_C},
};

// The default layer of the keyboard.
struct key* keys[15][5] = {
  {&key1, &key1,&key1,&key1,&key1},
  {&key2, &key2,&key2,&key2,&key2},
  {&key3, &key3,&key3,&key3,&key3},
  {&key1, &key1,&key1,&key1,&key1},
  {&key2, &key2,&key2,&key2,&key2},
  {&key3, &key3,&key3,&key3,&key3},
  {&key1, &key1,&key1,&key1,&key1},
  {&key2, &key2,&key2,&key2,&key2},
  {&key3, &key3,&key3,&key3,&key3},
  {&key1, &key1,&key1,&key1,&key1},
  {&key2, &key2,&key2,&key2,&key2},
  {&key3, &key3,&key3,&key3,&key3},
  {&key1, &key1,&key1,&key1,&key1},
  {&key2, &key2,&key2,&key2,&key2},
  {&key3, &key3,&key3,&key3,&key3},
};

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
  uint16_t column_outputs = 0b0000000000000001; // Start at the leftmost column
  // PCA9555 uses two sets of 8-bit outputs
  uint8_t outputs_0[2] = {PCA9555_CMD_SET_OUTPUTS_0, 0b00000000}; 
  uint8_t outputs_1[2] = {PCA9555_CMD_SET_OUTPUTS_1, 0b00000000};
  // Loop through all columns
  for (int x = 0; x < KEYBOARD_X; x++) {
    // Split the column_outputs into 2 sets of 8-bit numbers.
    outputs_0[1] = column_outputs & 0xFF;
    outputs_1[1] = (column_outputs >> 8) & 0xFF;

    // Write the two 8-bit numbers
    i2c_write_blocking(&i2c1_inst, PCA9555_ADDR, outputs_0, 2, false);
    i2c_write_blocking(&i2c1_inst, PCA9555_ADDR, outputs_1, 2, false);

    // Get the state of all keys in the column
    bool r1 = gpio_get(1);
    bool r2 = gpio_get(0);
    bool r3 = gpio_get(29);
    bool r4 = gpio_get(28);
    bool r5 = gpio_get(27);
    // Check the state of each key in the column for changes.
    check_key(keys[x][0], r1, &layers, &default_layer);
    check_key(keys[x][1], r2, &layers, &default_layer);
    check_key(keys[x][2], r3, &layers, &default_layer);
    check_key(keys[x][3], r4, &layers, &default_layer);
    check_key(keys[x][4], r5, &layers, &default_layer);

    // If all possible columns have been checked, return.
    if (column_outputs == 0b1000000000000000) {
      return;
    }
    // Check the next column
    column_outputs = column_outputs << 1;
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
  uint8_t buf[2] = {PCA9555_CMD_CONFIGURE_0, 0b00000000};
  i2c_write_blocking(&i2c1_inst, PCA9555_ADDR, buf, 2, false);
  buf[0] = PCA9555_CMD_CONFIGURE_1;
  i2c_write_blocking(&i2c1_inst, PCA9555_ADDR, buf, 2, false);
};

void debugging_init(void) {
  // Configure the board's LED pin as an output for debugging.
  gpio_init(25);
  gpio_set_dir(25, GPIO_OUT);

  // stdio_init_all();
  uart_init(uart0, 115200); // UART debugging
}

// The main function, runs tinyusb and the key scanning loop.
int main(void) {
  debugging_init();           // Initialize debugging utilities
  board_init();               // Initialize the pico board
  tud_init(BOARD_TUD_RHPORT); // Initialize the tinyusb device stack
  tusb_init();                // Initialize tinyusb
  pca9555_init();             // Initialize the PCA9555 I2C GPIO expander

  while (true) {
    check_keys(); // Check the keys on the keyboard for their state.
    tud_task();   // tinyusb device task.
    hid_task();   // Send HID reports to the host.
  }
}