#include "keymap.h"
#include "squirrel.h"
#include "squirrel_constructors.h"
#include "tusb.h"

#define LAYER_WORKMAN 0
#define LAYER_QWERTY 1
#define LAYER_FN1 2
#define LAYER_FN2 3

struct key *keys[5][15];
struct key key_array[5][15];

void make_keys(void) {
  // Fill the key_array with empty keys.
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 15; x++) {
      key_array[y][x] = make_key();
    }
  }

  // Fill the keys array with pointers to the key_array.
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 15; x++) {
      keys[y][x] = &key_array[y][x];
    }
  }

  make_workman_layer(LAYER_WORKMAN);
  make_qwerty_layer(LAYER_QWERTY);
  make_fn1_layer(LAYER_FN1);
  make_fn2_layer(LAYER_FN2);
};

void make_workman_layer(uint8_t layer) {
  // Row 1
  key_add_keycode(keys[0][0], layer, HID_KEY_ESCAPE);
  key_add_keycode(keys[0][1], layer, HID_KEY_1);
  key_add_keycode(keys[0][2], layer, HID_KEY_2);
  key_add_keycode(keys[0][3], layer, HID_KEY_3);
  key_add_keycode(keys[0][4], layer, HID_KEY_4);
  key_add_keycode(keys[0][5], layer, HID_KEY_5);
  key_add_keycode(keys[0][6], layer, HID_KEY_6);
  key_add_keycode(keys[0][7], layer, HID_KEY_7);
  key_add_keycode(keys[0][8], layer, HID_KEY_8);
  key_add_keycode(keys[0][9], layer, HID_KEY_9);
  key_add_keycode(keys[0][10], layer, HID_KEY_0);
  key_add_keycode(keys[0][11], layer, HID_KEY_MINUS);
  key_add_keycode(keys[0][12], layer, HID_KEY_EQUAL);
  key_add_custom_code(
      keys[0][13],
      layer); // when held, enter a code in binary then press enter.
  key_add_keycode(keys[0][14], layer, HID_KEY_CAPS_LOCK);

  // Row 2
  key_add_keycode(keys[1][0], layer, HID_KEY_TAB);
  key_add_keycode(keys[1][1], layer, HID_KEY_Q);
  key_add_keycode(keys[1][2], layer, HID_KEY_D);
  key_add_keycode(keys[1][3], layer, HID_KEY_R);
  key_add_keycode(keys[1][4], layer, HID_KEY_W);
  key_add_keycode(keys[1][5], layer, HID_KEY_B);
  key_add_keycode(keys[1][6], layer, HID_KEY_J);
  key_add_keycode(keys[1][7], layer, HID_KEY_F);
  key_add_keycode(keys[1][8], layer, HID_KEY_U);
  key_add_keycode(keys[1][9], layer, HID_KEY_P);
  key_add_keycode(keys[1][10], layer, HID_KEY_SEMICOLON);
  key_add_keycode(keys[1][11], layer, HID_KEY_BRACKET_LEFT);
  key_add_keycode(keys[1][12], layer, HID_KEY_BRACKET_RIGHT);
  key_add_keycode(keys[1][13], layer, HID_KEY_BACKSLASH);
  key_add_keycode(keys[1][14], layer, HID_KEY_GRAVE);

  // Row 3
  key_add_keycode(keys[2][0], layer, HID_KEY_BACKSPACE);
  key_add_keycode(keys[2][1], layer, HID_KEY_A);
  key_add_keycode(keys[2][2], layer, HID_KEY_S);
  key_add_keycode(keys[2][3], layer, HID_KEY_H);
  key_add_keycode(keys[2][4], layer, HID_KEY_T);
  key_add_keycode(keys[2][5], layer, HID_KEY_G);
  key_add_keycode(keys[2][6], layer, HID_KEY_Y);
  key_add_keycode(keys[2][7], layer, HID_KEY_N);
  key_add_keycode(keys[2][8], layer, HID_KEY_E);
  key_add_keycode(keys[2][9], layer, HID_KEY_O);
  key_add_keycode(keys[2][10], layer, HID_KEY_I);
  key_add_keycode(keys[2][11], layer, HID_KEY_APOSTROPHE);
  key_add_keycode(keys[2][12], layer, HID_KEY_ENTER);
  key_add_keycode(keys[2][13], layer, HID_KEY_DELETE);
  key_add_keycode(keys[2][14], layer, HID_KEY_PRINT_SCREEN);

  // Row 4
  key_add_mod(keys[3][0], layer, KEYBOARD_MODIFIER_LEFTSHIFT);
  key_add_keycode(keys[3][1], layer, HID_KEY_Z);
  key_add_keycode(keys[3][2], layer, HID_KEY_X);
  key_add_keycode(keys[3][3], layer, HID_KEY_M);
  key_add_keycode(keys[3][4], layer, HID_KEY_C);
  key_add_keycode(keys[3][5], layer, HID_KEY_V);
  key_add_keycode(keys[3][6], layer, HID_KEY_K);
  key_add_keycode(keys[3][7], layer, HID_KEY_L);
  key_add_keycode(keys[3][8], layer, HID_KEY_COMMA);
  key_add_keycode(keys[3][9], layer, HID_KEY_PERIOD);
  key_add_keycode(keys[3][10], layer, HID_KEY_SLASH);
  key_add_mod(keys[3][11], layer, KEYBOARD_MODIFIER_RIGHTALT);
  key_add_keycode(keys[3][12], layer, HID_KEY_PAGE_UP);
  key_add_keycode(keys[3][13], layer, HID_KEY_ARROW_UP);
  key_add_keycode(keys[3][14], layer, HID_KEY_PAGE_DOWN);

  // Row 5
  key_add_mod(keys[4][0], layer, KEYBOARD_MODIFIER_LEFTCTRL);
  key_add_keycode(keys[4][1], layer, HID_KEY_GUI_LEFT);
  key_add_mod(keys[4][2], layer, KEYBOARD_MODIFIER_LEFTALT);
  key_add_momentary(keys[4][3], layer, LAYER_FN1);  // FN_1
  key_add_momentary(keys[4][4], layer, LAYER_FN2);  // FN_2
  key_add_default(keys[4][5], layer, LAYER_QWERTY); // QWERTY
  key_add_keycode(keys[4][6], layer, HID_KEY_SPACE);
  key_add_media(keys[4][7], layer, HID_USAGE_CONSUMER_VOLUME_DECREMENT);
  key_add_media(keys[4][8], layer, HID_USAGE_CONSUMER_PLAY_PAUSE);
  key_add_media(keys[4][9], layer, HID_USAGE_CONSUMER_VOLUME_INCREMENT);
  key_add_keycode(keys[4][10], layer, HID_KEY_HOME);
  key_add_keycode(keys[4][11], layer, HID_KEY_END);
  key_add_keycode(keys[4][12], layer, HID_KEY_ARROW_LEFT);
  key_add_keycode(keys[4][13], layer, HID_KEY_ARROW_DOWN);
  key_add_keycode(keys[4][14], layer, HID_KEY_ARROW_RIGHT);
}

void make_fn1_layer(uint8_t layer) {
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 15; x++) {
      // Fill the array with pass-through keys.
      key_add_pass_through(keys[y][x], layer);
    }
  }

  // Row 1
  key_add_keycode(keys[0][1], layer, HID_KEY_F1);
  key_add_keycode(keys[0][2], layer, HID_KEY_F2);
  key_add_keycode(keys[0][3], layer, HID_KEY_F3);
  key_add_keycode(keys[0][4], layer, HID_KEY_F4);
  key_add_keycode(keys[0][5], layer, HID_KEY_F5);
  key_add_keycode(keys[0][6], layer, HID_KEY_F6);
  key_add_keycode(keys[0][7], layer, HID_KEY_F7);
  key_add_keycode(keys[0][8], layer, HID_KEY_F8);
  key_add_keycode(keys[0][9], layer, HID_KEY_F9);
  key_add_keycode(keys[0][10], layer, HID_KEY_F10);
  key_add_keycode(keys[0][11], layer, HID_KEY_F11);
  key_add_keycode(keys[0][12], layer, HID_KEY_F12);

  key_add_media(keys[1][1], layer, HID_USAGE_CONSUMER_SLEEP);
  key_add_media(keys[1][2], layer, HID_USAGE_CONSUMER_POWER);
  key_add_media(keys[1][3], layer, HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT);
  key_add_media(keys[1][4], layer, HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT);
  key_add_media(keys[1][5], layer, HID_USAGE_CONSUMER_AL_LOCAL_BROWSER);
  key_add_media(keys[1][6], layer, HID_USAGE_CONSUMER_AL_CALCULATOR);
  key_add_media(keys[1][7], layer, HID_USAGE_CONSUMER_AL_EMAIL_READER);
  key_add_media(keys[1][8], layer, HID_USAGE_CONSUMER_BASS_DECREMENT);
  key_add_media(keys[1][9], layer, HID_USAGE_CONSUMER_BASS_INCREMENT);
  key_add_media(keys[1][10], layer, HID_USAGE_CONSUMER_TREBLE_DECREMENT);
  key_add_media(keys[1][11], layer, HID_USAGE_CONSUMER_TREBLE_INCREMENT);
  key_add_media(keys[1][12], layer, HID_USAGE_CONSUMER_BASS_BOOST);
  key_add_media(keys[1][13], layer, HID_USAGE_CONSUMER_AC_BACK);
  key_add_media(keys[1][14], layer, HID_USAGE_CONSUMER_AC_FORWARD);

  // Row 4
  // TODO: Mouse keys

  // Row 5
  key_add_media(keys[4][7], layer, HID_USAGE_CONSUMER_SCAN_PREVIOUS);
  key_add_media(keys[4][8], layer, HID_USAGE_CONSUMER_MUTE);
  key_add_media(keys[4][9], layer, HID_USAGE_CONSUMER_SCAN_NEXT);
  // TODO: Mouse keys
}

void make_fn2_layer(uint8_t layer) {
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 15; x++) {
      // Fill the array with pass-through keys.
      key_add_pass_through(keys[y][x], layer);
    }
  }
  // Row 1
  key_add_media(keys[0][1], layer, HID_USAGE_CONSUMER_SLEEP);
  key_add_media(keys[0][2], layer, HID_USAGE_CONSUMER_POWER);
  key_add_media(keys[0][3], layer, HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT);
  key_add_media(keys[0][4], layer, HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT);
  key_add_media(keys[0][5], layer, HID_USAGE_CONSUMER_AL_LOCAL_BROWSER);
  key_add_media(keys[0][6], layer, HID_USAGE_CONSUMER_AL_CALCULATOR);
  key_add_media(keys[0][7], layer, HID_USAGE_CONSUMER_AL_EMAIL_READER);
  key_add_media(keys[0][8], layer, HID_USAGE_CONSUMER_BASS_DECREMENT);
  key_add_media(keys[0][9], layer, HID_USAGE_CONSUMER_BASS_INCREMENT);
  key_add_media(keys[0][10], layer, HID_USAGE_CONSUMER_TREBLE_DECREMENT);
  key_add_media(keys[0][11], layer, HID_USAGE_CONSUMER_TREBLE_INCREMENT);
  key_add_media(keys[0][12], layer, HID_USAGE_CONSUMER_BASS_BOOST);
  key_add_media(keys[0][13], layer, HID_USAGE_CONSUMER_AC_BACK);
  key_add_media(keys[0][14], layer, HID_USAGE_CONSUMER_AC_FORWARD);
}

void make_qwerty_layer(uint8_t layer) {
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 15; x++) {
      // Fill the array with pass-through keys.
      key_add_pass_through(keys[y][x], layer);
    }
  }

  // Row 1
  key_add_keycode(keys[0][0], layer, HID_KEY_ESCAPE);
  key_add_keycode(keys[0][1], layer, HID_KEY_1);
  key_add_keycode(keys[0][2], layer, HID_KEY_2);
  key_add_keycode(keys[0][3], layer, HID_KEY_3);
  key_add_keycode(keys[0][4], layer, HID_KEY_4);
  key_add_keycode(keys[0][5], layer, HID_KEY_5);
  key_add_keycode(keys[0][6], layer, HID_KEY_6);
  key_add_keycode(keys[0][7], layer, HID_KEY_7);
  key_add_keycode(keys[0][8], layer, HID_KEY_8);
  key_add_keycode(keys[0][9], layer, HID_KEY_9);
  key_add_keycode(keys[0][10], layer, HID_KEY_0);
  key_add_keycode(keys[0][11], layer, HID_KEY_MINUS);
  key_add_keycode(keys[0][12], layer, HID_KEY_EQUAL);
  key_add_custom_code(
      keys[0][13],
      layer); // when held, enter a code in binary then press enter.
  key_add_keycode(keys[0][14], layer, HID_KEY_CAPS_LOCK);

  // Row 2
  key_add_keycode(keys[1][0], layer, HID_KEY_TAB);
  key_add_keycode(keys[1][1], layer, HID_KEY_Q);
  key_add_keycode(keys[1][2], layer, HID_KEY_W);
  key_add_keycode(keys[1][3], layer, HID_KEY_E);
  key_add_keycode(keys[1][4], layer, HID_KEY_R);
  key_add_keycode(keys[1][5], layer, HID_KEY_T);
  key_add_keycode(keys[1][6], layer, HID_KEY_Y);
  key_add_keycode(keys[1][7], layer, HID_KEY_U);
  key_add_keycode(keys[1][8], layer, HID_KEY_I);
  key_add_keycode(keys[1][9], layer, HID_KEY_O);
  key_add_keycode(keys[1][10], layer, HID_KEY_P);
  key_add_keycode(keys[1][11], layer, HID_KEY_BRACKET_LEFT);
  key_add_keycode(keys[1][12], layer, HID_KEY_BRACKET_RIGHT);
  key_add_keycode(keys[1][13], layer, HID_KEY_BACKSLASH);
  key_add_keycode(keys[1][14], layer, HID_KEY_GRAVE);

  // Row 3
  key_add_keycode(keys[2][0], layer, HID_KEY_BACKSPACE);
  key_add_keycode(keys[2][1], layer, HID_KEY_A);
  key_add_keycode(keys[2][2], layer, HID_KEY_S);
  key_add_keycode(keys[2][3], layer, HID_KEY_D);
  key_add_keycode(keys[2][4], layer, HID_KEY_F);
  key_add_keycode(keys[2][5], layer, HID_KEY_G);
  key_add_keycode(keys[2][6], layer, HID_KEY_H);
  key_add_keycode(keys[2][7], layer, HID_KEY_J);
  key_add_keycode(keys[2][8], layer, HID_KEY_K);
  key_add_keycode(keys[2][9], layer, HID_KEY_L);
  key_add_keycode(keys[2][10], layer, HID_KEY_SEMICOLON);
  key_add_keycode(keys[2][11], layer, HID_KEY_APOSTROPHE);
  key_add_keycode(keys[2][12], layer, HID_KEY_ENTER);
  key_add_keycode(keys[2][13], layer, HID_KEY_DELETE);
  key_add_keycode(keys[2][14], layer, HID_KEY_PRINT_SCREEN);

  // Row 4
  key_add_mod(keys[3][0], layer, KEYBOARD_MODIFIER_LEFTSHIFT);
  key_add_keycode(keys[3][1], layer, HID_KEY_Z);
  key_add_keycode(keys[3][2], layer, HID_KEY_X);
  key_add_keycode(keys[3][3], layer, HID_KEY_C);
  key_add_keycode(keys[3][4], layer, HID_KEY_V);
  key_add_keycode(keys[3][5], layer, HID_KEY_B);
  key_add_keycode(keys[3][6], layer, HID_KEY_N);
  key_add_keycode(keys[3][7], layer, HID_KEY_M);
  key_add_keycode(keys[3][8], layer, HID_KEY_COMMA);
  key_add_keycode(keys[3][9], layer, HID_KEY_PERIOD);
  key_add_keycode(keys[3][10], layer, HID_KEY_SLASH);
  key_add_mod(keys[3][11], layer, KEYBOARD_MODIFIER_RIGHTALT);
  key_add_keycode(keys[3][12], layer, HID_KEY_PAGE_UP);
  key_add_keycode(keys[3][13], layer, HID_KEY_ARROW_UP);
  key_add_keycode(keys[3][14], layer, HID_KEY_PAGE_DOWN);

  // Row 5
  key_add_mod(keys[4][0], layer, KEYBOARD_MODIFIER_LEFTCTRL);
  key_add_keycode(keys[4][1], layer, HID_KEY_GUI_LEFT);
  key_add_mod(keys[4][2], layer, KEYBOARD_MODIFIER_LEFTALT);
  key_add_momentary(keys[4][3], layer, LAYER_FN1);   // FN_1
  key_add_momentary(keys[4][4], layer, LAYER_FN2);   // FN_2
  key_add_default(keys[4][5], layer, LAYER_WORKMAN); // WORKMAN
  key_add_keycode(keys[4][6], layer, HID_KEY_SPACE);
  key_add_media(keys[4][7], layer, HID_USAGE_CONSUMER_VOLUME_DECREMENT);
  key_add_media(keys[4][8], layer, HID_USAGE_CONSUMER_PLAY_PAUSE);
  key_add_media(keys[4][9], layer, HID_USAGE_CONSUMER_VOLUME_INCREMENT);
  key_add_keycode(keys[4][10], layer, HID_KEY_HOME);
  key_add_keycode(keys[4][11], layer, HID_KEY_END);
  key_add_keycode(keys[4][12], layer, HID_KEY_ARROW_LEFT);
  key_add_keycode(keys[4][13], layer, HID_KEY_ARROW_DOWN);
  key_add_keycode(keys[4][14], layer, HID_KEY_ARROW_RIGHT);
}
