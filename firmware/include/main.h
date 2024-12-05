/* 
 * File:   main.h
 * 
 * Created on December 5, 2024, 12:20 AM
 */

#ifndef MAIN_H //blessed multiple include prevent
#define	MAIN_H

#define F_CPU 14745600UL

#include <string.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/delay.h>

// Input Timing
#define DOT 300 // Time for DOT
#define DASH 900 // Time for DASH
#define ENDCHAR 1500 // Time for end char
#define SPACE 2500 // Time for Space
#define TIMEOUT 3500 

// prototypes
void putCHAR(int detectedCHAR);
void IRtransmit(int delay, char *msg);
void var_delay_ms(uint16_t delay);
void lcd_init();
void recieve();
uint16_t get_timeBUTTON();
char transmit();

#endif	/* MAIN_H */

