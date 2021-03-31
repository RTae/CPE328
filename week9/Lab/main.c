#define F_CPU 8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>

#define PMW_PIN PD5

volatile int flag = 1;

ISR(INT0_vect){
    if(flag == 0){
        flag = 1;
    }else {
        flag = 0;
    }
}

void init_INT0() {
    EICRA |= (1 << ISC01) | (1 << ISC00); // Raising edge of INT0 generate an interrupt reuest
    EIMSK |= (1 << INT0); // Enable External Interrupt request 0
}

void init_TIMER0() {
    TCCR0A |= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B |= (1 << CS01) | (1 << CS00);
}

void PWM_LED(uint8_t dim){
    OCR0B = dim;
}

int main(void) {
    DDRD |= (1 << PMW_PIN);
    PORTD &= ~(1 << PMW_PIN);
    
    init_INT0();
    init_TIMER0();
    sei();
    
    int PWM_Vector[16] = {0,1,2,3,4,6,8,12,16,23,32,45,64,90,128,180,255};
    
    while (1) {
        
        for(int i = 0; i < 16; i++){
            PWM_LED(PWM_Vector[i]);
            _delay_ms(30);
        }
        
        for(int i = 15; i >= 0; i--){
            PWM_LED(PWM_Vector[i]);
            _delay_ms(30);
        }
        
        if(flag == 0){
            set_sleep_mode(SLEEP_MODE_STANDBY);
            sleep_enable();
            sleep_cpu();
        }
    }
} 
