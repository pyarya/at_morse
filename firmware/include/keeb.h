#ifndef PS2_KEYBOARD_H // if not defined! prevents multiple includes
#define PS2_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

// pin definitions
#define CLK_PIN PD2 // clock pin INT0
#define DATA_PIN PD3 // data pin 

// ring buffer for handling the stream of input data
#define KBD_BUFFER_SIZE 16 

void kbd_init(void); // initialize keyboard ports and interrupts for system
char kbd_read(void); // reading characters from buffer
uint8_t kbd_available(void); // checking character availability

#endif
