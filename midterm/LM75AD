#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LCD_Dir  DDRD			/* Define LCD data port direction */
#define LCD_Port PORTD			/* Define LCD data port */
#define RS PD0				    /* Define Register Select pin */
#define EN PD1 				    /* Define Enable signal pin */

#define LM75_ADDRESS 0x92       /*SHT21 Address*/

void LCD_Command( unsigned char cmnd )
{
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0);  /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);		                   /* RS=0, command reg. */
	LCD_Port |= (1<<EN);		                   /* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);    /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0);    /* sending upper */
	LCD_Port |= (1<<RS);		                     /* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4);      /* sending lower */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Dir = 0xFF;			    /* Make LCD port direction as o/p */
	_delay_ms(20);			    /* LCD Power ON delay always >15ms */
	
	LCD_Command(0x02);		    /* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);          /* 2 line, 5*7 matrix in 4-bit mode */
	LCD_Command(0x0c);          /* Display on cursor off*/
	LCD_Command(0x06);          /* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);          /* Clear display screen*/
	_delay_ms(2);
}


void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0; str[i]!=0 && str[i]!=0x000a; i++)		/* Send each char of string till the NULL And String should not be newline*/
	{
		LCD_Char(str[i]);
	}
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
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

float readTempLM75AD() {
    uint8_t high_byte;
    uint8_t low_byte;
   
    // Send S
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();
    
    // SLA+W
    I2C_Write((uint8_t)(LM75_ADDRESS));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    // Select for temperature
    I2C_Write((uint8_t) 0x00);
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();
    
    // SR
    I2C_Start();
    if(I2C_GetStatusCode() != 0x10) I2C_Stop();
    
    // SLA+R
    I2C_Write((uint8_t)(LM75_ADDRESS + 1));
    if (I2C_GetStatusCode() != 0x40) I2C_Stop();

    // Read MSB
    high_byte = I2C_ReadAck();
    if (I2C_GetStatusCode() != 0x50) I2C_Stop();
     
    // Read LSB
    low_byte = I2C_ReadNAck();
    if (I2C_GetStatusCode() != 0x58) I2C_Stop();
    
    // Send P
    I2C_Stop();
    
    // Convert
    uint16_t result = (high_byte << 8) | low_byte; 
    
	float temperatureC = (float)result / 256.0 ;
    
    return temperatureC;
}

int main(void) {
    
    LCD_Init();
    I2C_Init();
    LCD_Clear();
                   
    while (1) {
        unsigned char buffer[10];
        float temp = readTempLM75AD();
        
        dtostrf(temp, 3, 2, buffer);
        strcat(buffer, " C\n");
        
        LCD_Clear();
        LCD_String(buffer);
        _delay_ms(1000);
    }
}
