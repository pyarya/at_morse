// tried implementing for avr-c from www.technoblogy.com/show?1GYR

#include "../include/keeb.h"
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

static volatile char kbd_buffer[KEYBOARD_BUFFER_SIZE];
static volatile uint8_t buf_head = 0;
static volatile uint8_t buf_tail = 0;

void buffer_write(unsigned char input){
// adding characters to buffer 
}

ISR(INT0_vect){
    static uint16_t ScanCode = 0, ScanCodeBit = 1; // Break is for identifying key release
    static uint8_t Break = 0, Mod = 0;
    uint8_t sc_stripped; // scancode stripped of start, stop and parity bits
    // ScanCodeBit tracks which bit position of the scan code we are on
    // Modifier checks for shift/ctrl/ whatever/ might not need this one?
    // static bc need to keep their state every time the ISR is called
    //
    if (PIND & (1<<DATA_PIN)) { // PB1 High, read data bit
        ScanCode |= ScanCodeBit; // reads ScanCode at bit position speicifed by ScanCodeBit
    }
    ScanCodeBit<<=1;

    // Full Scancode Interpreting Goes Here
    // set break to 1 if Break code is given
    // also ignore if any mod keys are pressed

    if (ScanCodeBit == 0x800) { //end of scancode
        if ((ScanCode & 0x401) != 0x400) { //start and stop bit validation TODO:double check this
            return;
        }
        sc_stripped = (ScanCode & 0x1FE) >> 1; // scancode (8-bit)
        ScanCode = 0;
        ScanCodeBit = 1;
        if (sc_stripped == 0xAA) { 
        // code sent on poweron/reset, ignored but there are other ones we might also need to ignore
            return;
        }
        if (sc_stripped == 0xF0) {
            Break = 1;
            return;
        }
        if (Break) {
            Break = 0;Mod = 0;
            return
        }
        
        unsigned char input = pgm_read_byte(&Keymap[stripped_sc]);
        if (input == 32 && stripped_sc != 0x29) return; // 32 is ASCII code for spacebar
        // this ignores the invalid codes replaced with whitespace, also checks if scancode was not for space key
        
        buffer_write(input); //write to buffer
    }
    // Sending thru serial w UART 
    // NOTE: could also store in buffer but this helps check if keypresses are working for now
    while (!(UCSR0A & (1 << UDRE0))); // Wait for the UART buffer to be ready
    UDR0 = key;
}

void kbd_init() {
    ScanCode = 0;
    ScanCodeBit = 1;
    Break = 0;
    Mod = 0;

    DDRD &= ~((1<<CLK_PIN) | (1<<DATA_PIN));
    EICRA |= (1<< ISC01); // Table 12-2 ISC01:1 ISC00:0, falling edge interrupt
    EIMSK |= (1<<INT0); // interrupt 0 enable
    PORTD |= (1<<CLK_PIN) | (1<<DATA_PIN);
    sei();
}
