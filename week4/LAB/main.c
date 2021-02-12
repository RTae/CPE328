#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define CS PORTB2
#define CS_DDR DDB2
#define MOSI DDB3
#define CLK DDB5


void USART_Init(unsigned int ubrr) {
    UBRR0 = ubrr;
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_Transmit( unsigned char data ) {
    while ( !( UCSR0A & (1 << UDRE0)) );
    UDR0 = data;
}

void SPI_Init()
{   
    /* set MOSI CLK CS as Output*/
    DDRB |= (1 << CS_DDR) | (1 << CLK) | (1 << MOSI);
    /* Chip select high*/
    PORTB |= (1 << CS);
    /* Enable SPI, Master mode, clk/16 */
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

uint16_t SPI_READ()
{
    uint8_t data1;
    uint16_t d_out;
    
    PORTB &= ~(1 << CS);                        // Chip select low

    SPDR = 0xFF;                                // put dummy byte in SPDR

    while(!(SPSR & (1<<SPIF)));                 // wait for SPIF high 

    data1 = SPDR & 0b00111111;                  // copy SPDR out
    
    d_out = data1 << 7;
    
    SPDR = 0xFF;                                // put dummy byte in SPDR

    while(!(SPSR & (1<<SPIF)));                 // wait for SPIF high     

    data1 = SPDR >>= 1;                         // copy SPDR out

    d_out |= data1;                             // Concat bit
    
    PORTB |= (1 << CS);                         // Chip select high

    return d_out;
}

int main(void) {

    USART_Init(53);
    SPI_Init();

    uint16_t sensor;
    int temp;
    unsigned char buffer[10];

    while (1) {
        sensor = SPI_READ();             // Read data from sensor
        temp = ((sensor/4095.0) * 5 - 0.5) * 100;
        sprintf(buffer,"%d",temp);     // convert to string

        for(int i=0; buffer[i] != 0; i++){
            USART_Transmit(buffer[i]);
        }

        _delay_ms(1000);
    }
}
