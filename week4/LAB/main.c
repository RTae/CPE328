  
#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    // Chip select high
    PORTB |= (1 << CS);
    // Chip select low
    PORTB &= ~(1 << CS);
    /* Enable SPI, Master mode, clk/16 */
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

uint16_t SPI_READ()
{
    uint16_t high_byte;
    uint16_t low_byte;
    uint16_t out_12bits;
    
    PORTB &= ~(1 << CS);                                            // Chip select low

    SPDR = 0xFF;                                                    // put dummy byte in SPDR

    while(!(SPSR & (1<<SPIF)));                                     // wait for SPIF high 
    
    /*xx0[B11][B10][B9][B8][B7]*/
    high_byte = SPDR;                                               // copy SPDR out
        
    SPDR = 0xFF;                                                    // put dummy byte in SPDR

    while(!(SPSR & (1<<SPIF)));                                     // wait for SPIF high     
    
    /*[B6][B5][B4][B3][B2][B1][B0][B1]*/
    low_byte = SPDR;                                                // copy SPDR out
    
    /*xx0[B11][B10][B9][B8][B7] 0   0   0   0   0   0   0    0 */
    /*                                                      OR */
    /*000 0     0    0   0  0  [B6][B5][B4][B3][B2][B1][B0][B1]*/
    /*---------------------------------------------------------*/
    /*xx0[B11][B10][B9][B8][B7][B6][B5][B4][B3][B2][B1][B0][B1]*/
    out_12bits = (high_byte << 8) | low_byte;                       // Concatenate bit
    
    /*[B11][B10][B9][B8][B7][B6][B5][B4][B3][B2][B1][B0][B1]000*/
    out_12bits <<= 3;                                               // Shift left 3
    
    /*0000[B11][B10][B9][B8][B7][B6][B5][B4][B3][B2][B1][B0]*/
    out_12bits >>= 4;                                               // Shift right 4
    
    PORTB |= (1 << CS);                                             // Chip select high

    return out_12bits;
}

int main(void) {

    USART_Init(53);
    SPI_Init();

    uint16_t sensor;
    float temp;
    unsigned char text[] = "Temperature = ";
    unsigned char buffer[10];

    while (1) {
        sensor = SPI_READ();             // Read data from sensor
        
        // temp = (((sensor/4096.0) * 5.0) - 0.5) * 100.0 ;
        
        dtostrf(temp, 3, 2, buffer);
        strcat(buffer, " °C\n");
        
        print(text);
        print(buffer);

        _delay_ms(1000);
    }
}
