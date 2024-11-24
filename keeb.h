#ifndef PS2_KEYBOARD_H // if not defined! prevents multiple includes
#define PS2_KEYBOARD_H

#include <stdint.h>

// pin definitions
#define DATA_PIN PD4 // data pin 
#define CLOCK_PIN PD2 // clock pin INT0


void keeb_init(void);
char keeb_read(void);
uint8_t keeb_available(void);

#endif
