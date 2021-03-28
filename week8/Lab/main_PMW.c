#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>

#define PMW_PIN PD5

void TIMER0_Init() {
    TCCR0A |= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B |= (1 << CS01) | (1 << CS00);
}

void PWM_LED(uint8_t dim){
    OCR0B = dim;
}

int main(void) {
    
    DDRD |= (1 << PMW_PIN);
    PORTD &= ~(1 << PMW_PIN);
    
    TIMER0_Init();
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
    }
}
