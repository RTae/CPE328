#define F_CPU 8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>

#define PMW_PIN PD5

/* Handle case when interrupt INI0*/
volatile int flag = 1;

ISR(INT0_vect){
    if(flag == 0){
        flag = 1;
    }else {
        flag = 0;
    }
}

/* Init Interrupt INT0*/
void init_INT0() {
    EICRA |= (1 << ISC01) | (1 << ISC00); // Raising edge of INT0 generate an interrupt reuest
    EIMSK |= (1 << INT0); // Enable External Interrupt request 0
}

/* Init Timer TIMER0 For PWM signal */
void init_TIMER0() {
    TCCR0A |= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B |= (1 << CS01) | (1 << CS00);
}

/* Value for duty circle*/
void PWM_LED(uint8_t dim){
    OCR0B = dim;
}

int main(void) {
    /* Init PD5 */
    DDRD |= (1 << PMW_PIN);
    PORTD &= ~(1 << PMW_PIN);
    
    init_INT0(); // Init Interrupt
    init_TIMER0(); // Init Timer
    sei(); // Enable Interrupt
    
    int PWM_Vector[16] = {0,1,2,3,4,6,8,12,16,23,32,45,64,90,128,180,255};
    
    while (1) {
        
        /* Rising LED  */
        for(int i = 0; i < 16; i++){
            PWM_LED(PWM_Vector[i]);
            _delay_ms(30);
        }

        /* Dimming LED  */
        for(int i = 15; i >= 0; i--){
            PWM_LED(PWM_Vector[i]);
            _delay_ms(30);
        }
        
        /* When interrupt go to sleep mode */
        if(flag == 0){
            set_sleep_mode(SLEEP_MODE_STANDBY);
            sleep_enable();
            sleep_cpu();
        }
    }
} 
