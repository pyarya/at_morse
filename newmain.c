/*
 * File:   newavr-main.c
 * Author: main
 *
 * Created on November 30, 2024, 2:35 PM
 */

#define F_CPU 14745600UL

#include "defines.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "lcd.h"


// Timing and Timeout
#define DOT 200
#define DASH 1500
#define TIMEOUT 3000

volatile uint16_t overflow =0;

volatile uint16_t capture = 0;
volatile uint16_t c_start = 0;
volatile uint16_t c_end = 0;

// Morse Code Lookup Table
const char *morse_table[36] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", // A-J
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", // K-T
    "..-", "...-", ".--", "-..-", "-.--", "--..",                       // U-Z
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", // 0-7
    "---..", "----."                                                // 8-9
};
const char ascii_table[36] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

int DOTMIN = 200;
int DOTMAX = 1500;
int DASHMIN = 1501;
int DASHMAX = 2500;
int LETTERMIN = 1501;
int LETTERMAX = 2500;
int SPACEMIN = 3400;
int SPACEMAX = 3600;

int morseCounter = 0;
char morse[8] = {0};

FILE lcd_str = FDEV_SETUP_STREAM ( lcd_putchar , NULL , _FDEV_SETUP_WRITE ) ; 

ISR(TIMER1_CAPT_vect) {
    if (capture == 0) {
        c_start = ICR1;
        overflow = 0;
        TCCR1B ^= (1<<ICES1);
        capture =1;
    } else {
        c_end = ICR1;
        TCCR1B ^= (1<<ICES1);
        capture = 2;
    }
}

void init() {
    
    
    // Button
    DDRB &= ~( 1 << PB2 ); //Enable as input INPUT BUTTON
    PORTB |= (1 << PB2 ); //Enable pull up resistor

// Input switch to change mode !!!!!!!!!!!!
    DDRC &= ~( 1 << PC1 ); //Enable as input (Mode select button)
    PORTC |= (1 << PC1 ); //Enable pull up resistor

    TCCR0B |=(1<<CS00);
    TCNT0 = 0;
    TCCR0A = 0; // Normal mode
    TIMSK1 = (1 << ICIE1) | (1 << TOIE1); // Enable Timer Overflow Interrupt

    // IR Reciever as input
    DDRB &= ~(1 << PB0);
    PORTB |= (1 << PB0);
    

    // IR PB3 Output Emitter
    DDRB |= (1 << PB3);
    PORTB |= (1 << PB3);

    lcd_init();
    _delay_ms(10);

    sei();

}

// REVIEW LATER
void recieve(uint16_t *on_time, uint16_t *off_time) {
    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
    fprintf (&lcd_str, "AWAITING"); //Prints input
    uint32_t ticks;
    capture = 0;

   // Wait for first rising edge
    while (capture != 1);
    cli();
    uint16_t start1 = c_start; // Capture start time of the first pulse
    sei();

    // Wait for second rising edge
    capture = 0;
    while (capture != 1);
    cli();
    uint16_t end1 = c_start; // Capture start time of the second pulse
    sei();

    // Calculate ON time
    if (end1 > start1) {
        ticks = end1 - start1;
    } else {
        ticks = (65536 - start1) + end1 + (overflow * 65536);
    }
    *on_time = (ticks * 8UL) / (F_CPU / 1000UL); // Convert ticks to milliseconds

    // Wait for third capture (signal rising edge)
    while (capture != 1);

    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
    fprintf (&lcd_str, "past cap3!!!"); //Prints input
    _delay_ms(100);
    //
    cli();
    uint16_t start2 = c_start; // Capture the start of the next signal (rising edge)
    sei();

    // Calculate OFF time
    if (start2 > end1) {
        ticks = start2 - end1;
    } else {
        ticks = (65536 - end1) + start2 + (overflow * 65536);
    }
    *off_time = (ticks * 8UL) / (F_CPU / 1000UL); // Convert ticks to milliseconds

    // Reset for the next measurement
    capture = 0;
    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
    fprintf (&lcd_str, "NOW PRINTING..."); //Prints input
    _delay_ms(100);
    bintoascii(on_time, off_time);
}

uint16_t get_timeBUTTON() {
    int time =0;
    while (!( PINB & ( 1 << PB2 ) ) ) {
        
        _delay_ms(1);  // Small delay
        time += 1;  // Increment the hold time by the delay amount
    }
    time = time*10;
    return (time); 
}

char transmit () {
    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
    fprintf (&lcd_str, "GETTING INPUT"); //Prints input
    uint16_t time = get_timeBUTTON();
    if (time > DOT && time < DASH) {
        PORTB &= ~(1 << PB3); // IR Output
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "TRANSMITTING ."); //Prints input
        _delay_ms(100);
        PORTB |= (1 << PB3);
        _delay_ms(100);
        PORTB &= ~(1 << PB3); // IR Output
        _delay_ms(100);
        PORTB |= (1 << PB3);
    } else if (time >= DASH && time < TIMEOUT) {
        PORTB &= ~(1 << PB3); // IR Output
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "TRANSMITTING -"); //Prints input
        _delay_ms(200);
        PORTB |= (1 << PB3);
        _delay_ms(200);
        PORTB |= (1 << PB3);
        
    } else {
        // Handle Timeout
    }
    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
    fprintf (&lcd_str, "DATA TRANSMITTED"); //Prints input
    _delay_ms(50);
    return 0;
}



void bintoascii(int on, int off)// this needs to change depending on how the timer variables are implemented
{
    // this condition is to check if the signal changes from on to off. If that is true, then proceed with translation
        //realistically this can be changed if this translate is only set to trigger every time the signal changes
        if (DOTMIN <= on && on <= DOTMAX) // window for placing a dot into the morse buffer
        {
            morse[morseCounter] = '.';
            morseCounter++;
        }
        if (DASHMIN <= on && on <= DASHMAX)// window for placing a dash into the morse buffer
        {
            morse[morseCounter] = '-';
            morseCounter++;
        }
        
        if (LETTERMIN <= off && off <= LETTERMAX) {
            morse[morseCounter] = '\0'; // Null-terminate Morse buffer
            int found = 0;  // Flag to indicate if the string was found
            for (int i = 0; i < 36; i++) {
                if (strcmp(morse, morse_table[i]) == 0) {
                    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
                    fprintf(&lcd_str, "%c", ascii_table[i]);
                     _delay_ms(500);
                    found = 1;
                    break;
                }   
            }
            if (!found) {
                    fprintf(&lcd_str, "?");
                    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
                     _delay_ms(500);
                }
            
            morseCounter = 0; // Reset buffer
            morse[0] = '\0';
            }   
        
                    
        if (SPACEMIN <= off && off <= SPACEMAX) //  signal is off long enough to place a space character, clearing morse buffer
        {
            morse[0] = '\0';
            morseCounter = 0;
            fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
            fprintf(&lcd_str, " ");
            _delay_ms(100);
        }
}

        
int main ( void ) {
    init();
    uint8_t state = 0; //TX mode = 0x00, RX mode = 0xFF, TX default state
    
    while ( 1 ) {
        
        // check device state
        if (PINC & (1 << PC1)) { 
            fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
            fprintf (&lcd_str, "RECEIVE MODE"); //Prints input
        
            uint16_t on_time = 0, off_time = 0;
            recieve(&on_time, &off_time); 
        } 
        else{
            fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
            fprintf (&lcd_str, "TRANSMIT MODE"); //Prints input
        
            while (( PINB & ( 1 << PB2 ) ) );
            transmit(); 
        }
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "past receive!!!"); //Prints input
        

    }
    
}
