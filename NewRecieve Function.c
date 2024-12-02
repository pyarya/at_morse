// new receive and ISR interrupt


ISR(TIMER1_CAPT_vect) {
    if (capture == 0) {
        capture = 1;
    }
}

void recieve() {
    int time = 0;
    capture =0;
    while (capture == 0) {}  // wait for input to be capture
    _delay_ms(10); // wait for initial pulse to finish, the intiial pulse will always BE 10ms, so this will stop it from triggering twice
    // reset capture flag
    capture =0;
    // then count time until flag is triggered again
    while (capture ==0) {
        time++;
        _delay_ms(1);
    }

    // then we can use if statements to determine whether it is a ., -, or space
}