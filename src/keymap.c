#include "keymap.h"
#include "squirrel.h"
#include "squirrel_constructors.h"
#include "tusb.h"

struct key *keys[5][15];
struct key key_array[5][15];

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
  key_array[4][3] = make_key(HID_KEY_0); // TODO: layer shifters
  key_array[4][4] = make_key(HID_KEY_1);
  key_array[4][5] = make_key(HID_KEY_2);
  key_array[4][6] = make_key(HID_KEY_SPACE);
  key_array[4][7] = make_media(HID_USAGE_CONSUMER_VOLUME_DECREMENT);
  key_array[4][8] = make_media(HID_USAGE_CONSUMER_PLAY_PAUSE);
  key_array[4][9] = make_media(HID_USAGE_CONSUMER_VOLUME_INCREMENT);
  key_array[4][10] = make_key(HID_KEY_HOME);
  key_array[4][11] = make_key(HID_KEY_END);
  key_array[4][12] = make_key(HID_KEY_ARROW_LEFT);
  key_array[4][13] = make_key(HID_KEY_ARROW_DOWN);
  key_array[4][14] = make_key(HID_KEY_ARROW_RIGHT);

  // Fill the keys array with pointers to the key_array.
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 15; x++) {
      keys[y][x] = &key_array[y][x];
    }
  }
};
