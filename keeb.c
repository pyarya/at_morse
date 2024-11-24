// tried implementing for avr-c from www.technoblogy.com/show?1GYR

#include "keeb.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define KeymapSize 132

const char Keymap[] PROGMEM = // stores keymap in flash memory for space

//  whitespace for unused keys, array starting at scancode 0x00
//  could be different depending on the keyboard we get, will need to test.
//  usually scancodes are one of sets 1,2 or 3.
    "             \011`      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09"
    "  ./l;p-   \' [=    \015] \\        \010  1 47   0.2568\033  +3-*9      ";
