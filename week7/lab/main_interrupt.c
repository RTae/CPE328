#define F_CPU 8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BUTTON PD2
#define LEB PB1

volatile int flag = 0;

/* For interrupt case INT0 */
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
    EICRA |= (1 << ISC01); // Falling edge of INT0 generate an interrupt reuest
    EIMSK |= (1 << INT0); // Enable External Interrupt request 0
}

void init_timer1(uint16_t count){
    TCCR1A |= (1 << COM1A0);   // Open toggle mode on OC1A/OC1B
    TCCR1B |= (1 << WGM12) | (1 << CS12);   // Set CTC mode and clk/256
    OCR1A = count;
}

void start_timer1(){
    TCCR1B |= (1 << CS12);   // Set clk/256
}

void stop_timer1(){
    TCCR1B &= ~(1 << CS12);  // No clock source
}

int main(void) {

    /* Init LED*/
    DDRB |= (1 << LEB);
    PORTB &= ~(1 << LEB);

    /* Init Timer */
    init_timer1(15624); // 15624  = ((8000000)/(256*2)) - 1
    
    /* Init Interrupt */
    init_int0();
    sei();
    
    while (1) {
    }
}
