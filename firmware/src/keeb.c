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

ISR(INT0_vect){
    static int ScanCode = 0, ScanCodeBit = 1, Break = 0, Modifier = 0; // Break is for identifying key release
    // ScanCodeBit tracks which bit position of the scan code we are on
    // Modifier checks for shift/ctrl/ whatever/ might not need this one?
    // static bc need to keep their state every time the ISR is called
    //
    // NOTE: i think need to change interrupt pins to PD2 and PD3 for Keyboard and move LCD pins
    if (PIND & (1<<DATA_PIN)) { // PB1 High, read data bit
        ScanCode |= ScanCodeBit; // reads ScanCode at bit position speicifed by ScanCodeBit
    }
    ScanCodeBit<<=1;

    // Full Scancode Interpreting Goes Here
    // set break to 1 if Break code is given
    // also ignore if any mod keys are pressed


    // once ScanCode is Interpreted, search keymap array.
    // check pgm_read_byte() function 

    // Sending thru serial w UART
    while (!(UCSR0A & (1 << UDRE0))); // Wait for the UART buffer to be ready
    UDR0 = key;
}

void keeb_init()
    EICRA = ; // should be falling edge interrupt
    PORTD |= (1<<CLK_PIN) | (1<<DATA_PIN);
    EIMSK |= (1<<INT0);
    sei();
}
