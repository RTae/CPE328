#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>

#define BUTTON PD2
#define LEB PB1

void init_timer1(uint16_t count){
    TCCR1A |= (1 << COM1A0);               // Open toggle mode on OC1A/OC1B
    TCCR1B |= (1 << WGM12) | (1 << CS12);  // Set CTC mode and clk/256
    OCR1A = count;
}

void start_timer1(){
    TCCR1B |= (1 << CS12);     // Set clk/256
}

void stop_timer1(){
    TCCR1B &= ~(1 << CS12);    // No clock source
}

int main(void) {

    /* Init LED*/
    DDRB |= (1 << LEB);
    PORTB &= ~(1 << LEB);
    
    int flag = 0;

    /* Init Timer  */
    init_timer1(15624); // 15624  = ((8000000)/(256*2)) - 1
    
    while (1) {
        /* Button push for toggle  */
        if((PIND & (1 << BUTTON)) == 0) {
            if(flag)
                flag = 0;
            else
                flag = 1;
            
            _delay_ms(10);
            
            while((PIND & (1 << BUTTON)) == 0);
            _delay_ms(10);
        }
        
        /* Toggle start or stop timer  */
        if(flag){
            start_timer1();
        } else{
            stop_timer1();
        }
    }
}
