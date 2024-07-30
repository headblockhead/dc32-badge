#ifndef SLAB_KEYMAP_H
#define SLAB_KEYMAP_H

#include "squirrel.h"

extern struct key
    *keys[5][15]; // Array of pointers to the array of keys on the keyboard.

extern struct key key_array[5][15]; // An array of keys on the keyboard.

// make_keys creates a 15x5 array of key structs (key_array) + makes an array
// of pointers to those structs (keys).
void make_keys(void);

#endif
