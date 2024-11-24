#ifndef PS2_KEYBOARD_H // if not defined! prevents multiple includes
#define PS2_KEYBOARD_H

#include <stdint.h>

// pin definitions
#define DATA_PIN PD4 // data pin 
#define CLOCK_PIN PD2 // clock pin INT0

// ring buffer for handling the stream of input data
#define KEYBOARD_BUFFER_SIZE 16 

void keeb_init(void); // initialize keyboard ports and interrupts for system
char keeb_read(void); // reading characters from buffer
uint8_t keeb_available(void); // checking character availability

#endif
