  
#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define CS PB2
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

void print(unsigned char *buffer) {
    for(int i=0; buffer[i] != 0; i++){
        USART_Transmit(buffer[i]);
    }
}

void SPI_Init()
{   
    /* set MOSI CLK CS as Output*/
    DDRB |= (1 << CS_DDR) | (1 << CLK) | (1 << MOSI);
    /* Enable SPI, Master mode, clk/16 */
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

uint16_t SPI_READ()
{
    uint8_t rx_byte;
    uint16_t rx_12bits;
    
    PORTB &= ~(1 << CS);                        // Chip select low

    SPDR = 0xFF;                                // put dummy byte in SPDR

    while(!(SPSR & (1<<SPIF)));                 // wait for SPIF high 

    rx_byte = SPDR & 0b00111111;                  // copy SPDR out
    
    rx_12bits = rx_byte << 7;
    
    SPDR = 0xFF;                                // put dummy byte in SPDR

    while(!(SPSR & (1<<SPIF)));                 // wait for SPIF high     

    rx_byte = SPDR >>= 1;                         // copy SPDR out

    rx_12bits |= rx_byte;                             // Concat bit
    
    PORTB |= (1 << CS);                         // Chip select high

    return rx_12bits;
}

int main(void) {

    USART_Init(53);
    SPI_Init();

    uint16_t sensor;
    uint16_t temp;
    unsigned char text[] = "Temp = ";
    unsigned char buffer[10];

    while (1) {
        sensor = SPI_READ();             // Read data from sensor
        
        temp = ((sensor/4095.0) * 5.0 - 0.5) * 100.0;
        
        sprintf(buffer,"%u",sensor);     // convert to string
        strcat(buffer, "\n");
        
        print(text);
        print(buffer);

        _delay_ms(1000);
    }
}
