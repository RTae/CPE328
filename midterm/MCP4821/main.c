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

void SPI_Init()
{   
    /* set MOSI CLK CS as Output*/
    DDRB |= (1 << CS_DDR) | (1 << CLK) | (1 << MOSI);
    // Chip select high
    PORTB |= (1 << CS);
    // Chip select low
    PORTB &= ~(1 << CS);
    /* Enable SPI, Master mode, clk/16 */
    SPCR |= (1 << SPE) | (1 << MSTR);
}

void writeMCP4821(int16_t data){
    uint8_t highByte = ((data>>8)&0xff) | 0x10;
    uint8_t lowByte = data & 0xff;
    
    PORTB &= ~(1 << CS);                                            // Chip select low

    SPDR = highByte;                                                    // put dummy byte in SPDR
    while(!(SPSR & (1<<SPIF)));                                     // wait for SPIF high 
    
    SPDR = lowByte;                                                    // put dummy byte in SPDR
    while(!(SPSR & (1<<SPIF)));                                     // wait for SPIF high 
    
    PORTB |= (1 << CS);                                             // Chip select high
    _delay_ms(20);
}

int main(void) {
    SPI_Init();
    while (1) {
        uint16_t x = 3500; // mV
        writeMCP4821(x);
        _delay_ms(1000);
    }
}
