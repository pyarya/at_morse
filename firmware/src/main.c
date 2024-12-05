/*
 * ECE 312 FINAL PROJECT
 * Sean Theriault, Daniel Lee, Evan Bak, Aryan Aryal
 */

#include "../includes/main.h"
#include "../includes/lcd.h"

// Interrupt Variable, for finding time between received pulse
volatile uint16_t capture = 0;

// Definition of putCHAR function
void putCHAR(int detectedCHAR);

void IRtransmit(int delay, char *msg);

void var_delay_ms(uint16_t delay);

// Morse Code Lookup Table
const char *morse_table[36] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", // A-J
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", // K-T
    "..-", "...-", ".--", "-..-", "-.--", "--..",                       // U-Z
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", // 0-7
    "---..", "----."                                                // 8-9
};
const char ascii_table[36] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static char messageBuffer[100]; // Stores message for receiver
int morseCounter = 0;
char morse[8] = {0};

FILE lcd_str = FDEV_SETUP_STREAM ( lcd_putchar , NULL , _FDEV_SETUP_WRITE ) ; 

// Capture interrupt, sets capture variable to 1 when a trigger is detected on PBO (Interrupt Capture Pin)
ISR(TIMER1_CAPT_vect) {
    if (capture == 0) {
        capture = 1;
    }
}

void init() {
    
    // Buzzer Setup
    DDRB |= (1 << PB1); 
    PORTB &= ~(1 << PB1);
    
    // Input Button
    DDRB &= ~( 1 << PB2 ); 
    PORTB |= (1 << PB2 ); 

    // Switch to Change Operation Mode
    DDRC &= ~( 1 << PC1 );
    PORTC |= (1 << PC1 ); 

    
    // Do we still need this?
    TCCR0B |=(1<<CS00);
    TCNT0 = 0;
    TCCR0A = 0; // Normal mode
    TIMSK1 = (1 << ICIE1) | (1 << TOIE1); // Enable Timer Overflow Interrupt
    // ****
    
    // IR Receiver 
    DDRB &= ~(1 << PB0);
    PORTB |= (1 << PB0);
    
    // IR Emitter
    DDRB |= (1 << PB3);
    PORTB |= (1 << PB3);
    
    // Initialization
    lcd_init();
    _delay_ms(10);
    
    sei(); // Enable interrupts so data transfer can begin

}

// This function is used as both a delay, and to sound the buzzer when data is received
void buzz(int buzzTIME ){
    int buzzCLK = 0;
    while (buzzCLK < buzzTIME) {
        PORTB |= (1 << PB1); 
        _delay_ms(0.5);
        PORTB &=~ (1 << PB1); 
        _delay_ms(0.5);
        buzzCLK++;
    }
}


// This function is responsible for detecting the capture flags updates and taking the time between them, 
// in order to determine what data has been sent
void recieve() {
    int time = 0;
    int detectedCHAR = 0; // 0 "space", 1 ".", 2 "-" 
    
    capture = 0;
    while (capture == 0) {}  // wait for input to be capture
    _delay_ms(200); // wait for initial pulse to finish, the initial pulse will always BE 10ms, so this will stop it from triggering twice
    // reset capture flag
    capture =0;
    // then count time until flag is triggered again
    while (capture ==0) { // Check against time to add a 'timeout'
        time++;
        _delay_ms(1);
    }
    if (time < 200) { // "." case
        fprintf (&lcd_str, "\x1b\x01" ); 
        fprintf (&lcd_str, "DOT RECEIVED");    
        buzz(20);
        detectedCHAR = 1; // Indicate . is detected
        _delay_ms(300);
    } else if (201 <= time && time < 301) { // "-" Case
        fprintf (&lcd_str, "\x1b\x01" ); 
        fprintf (&lcd_str, "DASH RECEIVED"); 
        buzz(40);
        detectedCHAR = 2; // Indicate - is detected
         _delay_ms(300);
    } else if (401 <= time && time < 405){
        // If SPACE IS RECEIVED
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "SPACE RECEIVED"); //Prints input
        detectedCHAR = 3;
        buzz(80);
        _delay_ms(300);
      
    } else if (301 <= time && time < 401) {
         // IF END CHAR IS RECEIVED 
        fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
        fprintf (&lcd_str, "PRINTING..."); //Prints input
        detectedCHAR = 0; // Indicate - is detected
        buzz(50);
        _delay_ms(100);
    } else {
        // JUST DONT RUN DETECTED CHAR
        fprintf (&lcd_str, "\x1b\x01" ); 
        fprintf (&lcd_str, "INPUT CANCELLED");
        detectedCHAR = 4;
        buzz(20);
        _delay_ms(50);
        buzz(20);
        _delay_ms(250);
        return;
    }
    putCHAR(detectedCHAR);
}

/* This function is responsible for finding how long the user is holding the button 
 * and display the current character that hold would transmit.
 */
uint16_t get_timeBUTTON() {
    int time =0;
    while (!( PINB & ( 1 << PB2 ) ) ) {
        _delay_ms(1);  // wait
        time += 1;  // update time
        if (time < DASH/10) {
            fprintf(&lcd_str, "\x1b\xc0"); 
            fprintf(&lcd_str, "INPUT: DOT");
        }
        else if (time >= DASH/10 && time < ENDCHAR/10) {
            fprintf(&lcd_str, "\x1b\xc0"); 
            fprintf(&lcd_str, "INPUT: DASH");
        } 
        else if (time >= ENDCHAR/10 && time < SPACE/10) {
            fprintf(&lcd_str, "\x1b\xc0"); 
            fprintf(&lcd_str, "INPUT:  END");
        } 
        else if (time >= SPACE/10 && time < TIMEOUT/10) {
            fprintf(&lcd_str, "\x1b\xc0"); 
            fprintf(&lcd_str, "INPUT: SPACE");
        } else if (time >= TIMEOUT/10 && time < 500) {
            fprintf(&lcd_str, "\x1b\xc0"); 
            fprintf(&lcd_str, "INPUT: CANCEL");
        } else {
            fprintf (&lcd_str, "\x1b\x01" ); 
            fprintf (&lcd_str, "GETTING INPUT");
            time = 0; //CYCLE INPUT
        }
    }
    time = time*10;
    return (time); 
}

char transmit () {
    // Define Variables to be passed to IRtransmit
    int delay;
    char *msg;
    
    _delay_ms(5); // debounce   
    fprintf (&lcd_str, "\x1b\x01" ); 
    fprintf (&lcd_str, "GETTING INPUT"); 
    uint16_t time = get_timeBUTTON();
    
    if (time < DASH) {
        // SEND DOT
        delay = 100;
        msg = "DOT SENDING";        
    } else if (time >= DASH && time < ENDCHAR) {
        // SEND DASH 
        delay = 300;
        msg = "DASH SENDING";
    } else if (time >= ENDCHAR && time < SPACE) {
        // SEND ENDCHAR
        delay = 400;
        msg = "ENDING CHARACTER";
    } else if (time >= SPACE && time < TIMEOUT) {
        // SEND SPACE
        delay = 500;
        msg = "SPACE SENDING";
    } else {
        delay = 600;
        msg = "CANCELLING";
    }
    
    IRtransmit(delay, msg);
    fprintf (&lcd_str, "\x1b\x01" ); 
    fprintf (&lcd_str, "DATA TRANSMITTED"); 
    _delay_ms(100);
    return 0;
}

void IRtransmit(int delay, char *msg) {
        fprintf (&lcd_str, "\x1b\x01" ); 
        fprintf (&lcd_str, "%s.",msg); 
        PORTB &= ~(1 << PB3); // HIGH PULSE
        _delay_ms(100);         
        fprintf (&lcd_str, "\x1b\x01" ); 
        fprintf (&lcd_str, "%s..",msg); 
        PORTB |= (1 << PB3); // Hold Low
        var_delay_ms(delay); // For time matched to transmitting character
        fprintf (&lcd_str, "\x1b\x01" ); 
        fprintf (&lcd_str, "%s...",msg); 
        PORTB &= ~(1 << PB3);   // 2nd HIGH PULSE, signals end of data transmission
        _delay_ms(100);         
        PORTB |= (1 << PB3);    
}

// Used to operate variable delays 
void var_delay_ms(uint16_t delay) {
    while (delay--) {
        _delay_ms(1);
    }
}
void putCHAR(int detectedCHAR) {
    static int messageIndex = 0;   // Index for the message buffer
    if (detectedCHAR == 1) { // Dot
        morse[morseCounter] = '.'; // Add dot to buffer
        morseCounter++;
    } 
    else if (detectedCHAR == 2) { // Dash
        morse[morseCounter] = '-'; // Add dash to buffer
        morseCounter++;
    } 
    else if (detectedCHAR == 4) {
        morseCounter = 0;
        morse[0] = '\0';
    }
    else if (detectedCHAR == 3) { // SPACE
        messageBuffer[messageIndex] = ' ';
        messageIndex++;
        messageBuffer[messageIndex] = '\0'; // Null-terminate the message
    } 
    else if (detectedCHAR == 0) { // Space received
        morse[morseCounter] = '\0'; // Null-terminate the Morse buffer
        int found = 0; // Flag to indicate if a match is found

        // Check Morse code against the lookup table
        for (int i = 0; i < 36; i++) {
            if (strcmp(morse, morse_table[i]) == 0) {
                messageBuffer[messageIndex] = ascii_table[i];
                messageIndex++;
                messageBuffer[messageIndex] = '\0'; // Null-terminate the message
                found = 1;
                
                break;
            }
        }
        if (!found) {
            fprintf(&lcd_str, "?"); // Print '?' if no match is found
        }
        // Reset Morse buffer for the next character
        morseCounter = 0;
        morse[0] = '\0';
    }
}
        
int main ( void ) {
    init();
    while ( 1 ) {
        
        // check device state
        if (PINC & (1 << PC1)) { 
            // Receive Mode
            fprintf(&lcd_str, "\x1b\x01");
            fprintf(&lcd_str, "RECEIVE MODE"); 
            if (messageBuffer[0] != '\0') {  // If there is a message, we will print it
                fprintf(&lcd_str, "\x1b\x01"); 
                fprintf(&lcd_str, "MESSAGE:"); 
                fprintf(&lcd_str, "\x1b\xc0"); 
                fprintf(&lcd_str, "%s", messageBuffer); // Print the message
            }
            // Await data
            recieve(); 
        } 
        else{
            // Transmit Mode
            PORTB |= (1 << PB3);  // LOW (ensure starting state)
            fprintf (&lcd_str, "\x1b\x01" ); //Clears the display
            fprintf (&lcd_str, "TRANSMIT MODE"); //Prints input
            
            // Wait for input button press
            while (( PINB & ( 1 << PB2 ) ) );
            transmit(); 
        }
    }
}
