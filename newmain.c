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
#define DOT 300
#define DASH 900
#define TIMEOUT 1500

volatile uint16_t overflow =0;

volatile uint16_t capture = 0;
volatile uint16_t c_start = 0;
volatile uint16_t c_end = 0;

// Definition of putCHAR function
void putCHAR(int detectedCHAR);

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
int DOTMAX = 3000;
int DASHMIN = 3001;
int DASHMAX = 5000;
int LETTERMIN = 3001;
int LETTERMAX = 5000;
int SPACEMIN = 5000;
int SPACEMAX = 7000;

int morseCounter = 0;
char morse[8] = {0};

FILE lcd_str = FDEV_SETUP_STREAM ( lcd_putchar , NULL , _FDEV_SETUP_WRITE ) ; 

ISR(TIMER1_CAPT_vect) {
    if (capture == 0) {
        capture = 1;
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
void recieve() {
    int time = 0;
    int detectedCHAR = 0; // 0 "space", 1 ".", 2 "-" 
    
    
    capture = 0;
    while (capture == 0) {}  // wait for input to be capture
    _delay_ms(100); // wait for initial pulse to finish, the intiial pulse will always BE 10ms, so this will stop it from triggering twice
    // reset capture flag
    
    
    capture =0;
    // then count time until flag is triggered again
    while (capture ==0) {
        time++;
        _delay_ms(1);
    }
    
    if (time < 102) { // "." case
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "DOT RECIEVED"); //Prints input
        _delay_ms(200);
        detectedCHAR = 1; // Indicate . is detected
         _delay_ms(10);
    }

    else if (102 < time && time < 250) {
         fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "DASH RECIEVED"); //Prints input
        _delay_ms(200);
        detectedCHAR = 2; // Indicate - is detected
         _delay_ms(10);
    } else {
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "SPACE RECIEVED"); //Prints input
        _delay_ms(200);
        detectedCHAR = 0; // Indicate - is detected
         _delay_ms(10);
    }

    putCHAR(detectedCHAR);
   
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
// TESTING
  
    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
    fprintf (&lcd_str, "GETTING INPUT"); //Prints input
    uint16_t time = get_timeBUTTON();
    
    if (time < DASH) {
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "DOT SENDING."); //Prints input
        PORTB &= ~(1 << PB3);   // HIGH (first pulse - LED ON)
        _delay_ms(100);         // ON for 200 ms
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "DOT SENDING.."); //Prints input
        PORTB |= (1 << PB3);    // LOW (LED OFF)
        _delay_ms(100);         // OFF for 100 ms
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "DOT SENDING..."); //Prints input
        PORTB &= ~(1 << PB3);   // HIGH (second pulse - LED ON)
        _delay_ms(100);         // ON for 200 ms
        PORTB |= (1 << PB3);    // LOW (LED OFF)
    } else if (time >= DASH && time < TIMEOUT) {
       fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "DASH SENDING."); //Prints input
        PORTB &= ~(1 << PB3);   // HIGH (first pulse - LED ON)
        _delay_ms(100);         
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "DASH SENDING.."); //Prints input
        PORTB |= (1 << PB3);    // LOW (LED OFF)
        _delay_ms(200);        
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "DASH SENDING..."); //Prints input
        PORTB &= ~(1 << PB3);   // HIGH (second pulse - LED ON)
        _delay_ms(100);         // ON for 200 ms
        PORTB |= (1 << PB3);    // LOW (LED OFF)
    } else {
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "SPACE SENDING."); //Prints input
        PORTB &= ~(1 << PB3);   // HIGH (first pulse - LED ON)
        _delay_ms(100);         
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "SPACE SENDING.."); //Prints input
        PORTB |= (1 << PB3);    // LOW (LED OFF)
        _delay_ms(300);        
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "SPACE SENDING..."); //Prints input
        PORTB &= ~(1 << PB3);   // HIGH (second pulse - LED ON)
        _delay_ms(100);         // ON for 200 ms
        PORTB |= (1 << PB3);    // LOW (LED OFF)
    }
    fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
    fprintf (&lcd_str, "DATA TRANSMITTED"); //Prints input
    _delay_ms(50);
    return 0;
}



void putCHAR(int detectedCHAR)// this needs to change depending on how the timer variables are implemented
{
    if (detectedCHAR == 1) { // Dot
        morse[morseCounter] = '.'; // Add dot to buffer
        morseCounter++;
    } 
    else if (detectedCHAR == 2) { // Dash
        morse[morseCounter] = '-'; // Add dash to buffer
        morseCounter++;
    } 
    else if (detectedCHAR == 0) { // Space received
        morse[morseCounter] = '\0'; // Null-terminate the buffer
        int found = 0; // Flag to indicate if a match is found

        for (int i = 0; i < 36; i++) {
            if (strcmp(morse, morse_table[i]) == 0) {
                fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
                fprintf(&lcd_str, "MESSAGE: %c", ascii_table[i]); // Print the translated character
                _delay_ms(500);
                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(&lcd_str, "?"); // Print '?' if no match is found
        }

        // Reset buffer for next character
        morseCounter = 0;
        morse[0] = '\0';
    }
}

        
int main ( void ) {
    init();
  
    
    while ( 1 ) {
        
        // check device state
        if (PINC & (1 << PC1)) { 
            
            fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
            fprintf (&lcd_str, "RECEIVE MODE"); //Prints input
        
            uint16_t on_time = 0, off_time = 0;
            recieve(&on_time, &off_time); 
        } 
        else{
            PORTB |= (1 << PB3);  // LOW (ensure starting state)
            fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
            fprintf (&lcd_str, "TRANSMIT MODE"); //Prints input
        
            while (( PINB & ( 1 << PB2 ) ) );
            transmit(); 
        }
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "past receive!!!"); //Prints input
    }
}