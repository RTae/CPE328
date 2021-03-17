#define F_CPU 8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BUTTON PD2
#define LEB PB1

volatile int flag = 0;

ISR(INT0_vect){
    if (flag == 1) {
        flag = 0;
        start_timer1();
    } else {
        flag = 1;
        stop_timer1();
    }
}

void init_int0() {
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
}

void init_timer1(uint16_t count){
    TCCR1A |= (1 << COM1A0);
    TCCR1B |= (1 << WGM12) | (1 << CS12);
    OCR1A = count;
}

void start_timer1(){
    TCCR1B |= (1 << CS12);
}

void stop_timer1(){
    TCCR1B &= ~(1 << CS12);
}

int main(void) {

    /* Init LED*/
    DDRB |= (1 << LEB);
    PORTB &= ~(1 << LEB);
    init_timer1(31250);
    
    init_int0();
    sei();
    
    while (1) {
    }
}
