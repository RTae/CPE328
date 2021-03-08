#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DS1621 0x90

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
  //100kHz @ prescaler /4
  TWBR = 8;
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

void configDS1621() {
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();

    I2C_Write((uint8_t)(DS1621));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    I2C_Write((uint8_t) 0x22);
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();

    I2C_Start();
    if(I2C_GetStatusCode() != 0x10) I2C_Stop();

    I2C_Write((uint8_t)(DS1621));
    if (I2C_GetStatusCode() != 0x40) I2C_Stop();
    
    I2C_Write((uint8_t) (0 << 6 | 0 << 5 | 0 << 4 | 0 << 1 | 1));
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();
    
    I2C_Stop();
    
}

void startConvertDS1621(){
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();

    I2C_Write((uint8_t)(DS1621));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    I2C_Write((uint8_t) 0xEE);
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();

    I2C_Stop();
}

int8_t stopConvertDS1621(){
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();

    I2C_Write((uint8_t)(DS1621));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    I2C_Write((uint8_t) 0x22);
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();
    
    I2C_Start();
    if(I2C_GetStatusCode() != 0x10) I2C_Stop();

    I2C_Write((uint8_t)(DS1621 + 1));
    if (I2C_GetStatusCode() != 0x40) I2C_Stop();
    
    int8_t flag = I2C_ReadNAck() & 0x80;
    
    return flag;
}
float readTempDS1621() {
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();

    I2C_Write((uint8_t)(DS1621));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    I2C_Write((uint8_t) 0xAA);
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();

    I2C_Start();
    if(I2C_GetStatusCode() != 0x10) I2C_Stop();

    I2C_Write((uint8_t)(DS1621 + 1));
    if (I2C_GetStatusCode() != 0x40) I2C_Stop();
    
    int8_t MSB = I2C_ReadAck();
    if (I2C_GetStatusCode() != 0x50)I2C_Stop();
    
    int8_t LSB = I2C_ReadNAck();
    if (I2C_GetStatusCode() != 0x58) I2C_Stop();

    I2C_Stop();
    
    float t = (float) MSB;
    if (LSB & 0x80){
      t += 0.5;
    }
    if (MSB & 0x80){
      t -= 256;
    }

    return t; 
}


int main(void) {
    
    USART_Init(53);
    I2C_Init();
    
    float temp;
    unsigned char buffer[10];
    
    configDS1621();
        
    while (1) {
        startConvertDS1621();
        while(stopConvertDS1621());
        temp = readTempDS1621();
        
        dtostrf(temp, 4, 2, buffer);
        strcat(buffer, " C\r\n");
        print(buffer);
        _delay_ms(1000);
    }
}
