#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TC74_ADDR 0x9A

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

/* Terms
 * S = Start
 * SR = Repeated Start
 * P = Stop
 * SLA+W = Slave Address Write mode
 * SLA+R = Slave Address Read mode
 * ACK = Acknowledge
 * NACK = Not ACK
 */

void I2C_Init()
{
  //SCL, SDA as output
  DDRC |= (1 << DDC4) | (1 << DDC5);
  
  //init I2C
  //62.5kHz @ prescaler /4
  TWBR = 14;
  TWCR |= (1 << TWPS0);

  //enable I2C
  TWCR |= (1 << TWEN);
}

void I2C_Start(){
  //send S
  TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
  //wait complete then check status
  while(!(TWCR & (1 << TWINT)));
}

void I2C_Stop(){
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
}

void I2C_Write(uint8_t data){
  TWDR = data;
  TWCR = (1 << TWEN) | (1 << TWINT);
  //wait complete then check status
  while(!(TWCR & (1 << TWINT)));
  if((TWSR & 0xF8) != 0x18)
    return 2;
}

uint8_t I2C_ReadAck() {
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

uint8_t I2C_ReadNAck() {
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

uint8_t I2C_GetStatusCode() {
    uint8_t status;
    status = TWSR & 0xF8;
    return status;
}

int readTempTC74() {
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();

    I2C_Write((uint8_t)(TC74_ADDR));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    I2C_Write((uint8_t) 0x00);
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();

    I2C_Start();
    if(I2C_GetStatusCode() != 0x10) I2C_Stop();

    I2C_Write((uint8_t)(TC74_ADDR + 1));
    if (I2C_GetStatusCode() != 0x40) I2C_Stop();
    
    int temp = I2C_ReadNAck();
    if (I2C_GetStatusCode() != 0x58) I2C_Stop();

    I2C_Stop();
    
    return temp;
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
    SPCR |= (1 << SPE) | (1 << MSTR);
}

void writeAD5611(uint8_t data){
    uint8_t dacSPI0 = 0;
    uint8_t dacSPI1 = 0;
    
    PORTB &= ~(1 << CS);                                            // Chip select low
    
    dacSPI0 = ((data & 0x03ff ) >> 4);
    dacSPI1 = data << 4;

    SPDR = dacSPI0;                                                    // put dummy byte in SPDR
    while(!(SPSR & (1<<SPIF)));                                     // wait for SPIF high 
    
    
    SPDR = dacSPI1;                                                    // put dummy byte in SPDR
    while(!(SPSR & (1<<SPIF)));                                     // wait for SPIF high 
    
    PORTB |= (1 << CS);                                             // Chip select high
    _delay_ms(20);
}

int main(void) {
    
    USART_Init(53);
    I2C_Init();
    SPI_Init();
    
    int temp;
    unsigned char buffer[10];
    
    while (1) {
        temp = readTempTC74();
        float voltage = 0.01 * temp + 0.5;
        int voltage_DAC = (int)((voltage / 5) * 1024);
        sprintf(buffer, "%d C\r\n", temp);
        print(buffer);
        writeAD5611(voltage_DAC);
        _delay_ms(1000);
    }
}
