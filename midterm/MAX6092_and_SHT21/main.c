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

#define SHT21Address 0x80      /*SHT21 Address*/

#define CS PB2
#define CS_DDR DDB2
#define MOSI DDB3
#define CLK DDB5

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

float readTempSHT21() {
    uint8_t high_byte;
    uint8_t low_byte;
   
    // Send S
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();
    
    // SLA+W
    I2C_Write((uint8_t)(SHT21Address));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    // Select for temperature
    I2C_Write((uint8_t) 0xE3);
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();
    
    // SR
    I2C_Start();
    if(I2C_GetStatusCode() != 0x10) I2C_Stop();
    
    // SLA+R
    I2C_Write((uint8_t)(SHT21Address + 1));
    if (I2C_GetStatusCode() != 0x40) I2C_Stop();
    
    // Wait
    // while((PINC & (1 << PORTC5)));
    
    // Read MSB
    high_byte = I2C_ReadAck();
    if (I2C_GetStatusCode() != 0x50) I2C_Stop();
    
    // Read LSB
    low_byte = I2C_ReadAck();
    if (I2C_GetStatusCode() != 0x50) I2C_Stop();
    
    // Read Checksum
    I2C_ReadNAck();
    if (I2C_GetStatusCode() != 0x58) I2C_Stop();
    
    // Send P
    I2C_Stop();
    
    // Convert
    char buffer[10];
    uint16_t result = (high_byte << 8) | low_byte; 
    
    result &= 0xFFFC;
    
	float temperatureC = (float)result / 65536 ;	//T= -46.85 + 175.72 * ST/2^16
	temperatureC = temperatureC * 175.72;
	temperatureC = temperatureC - 46.85;
    
    return temperatureC;
}

void SPI_Init() {
    DDRB |= (1 << DDB2) | (1 << DDB3) | (1 << DDB5);
    
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR1);
}

void SPI_Read(uint8_t *data) {
    //chip select
    PORTB &= ~(1 << PORTB2);
    
    //send burst read command
    SPDR = 0xBF;
    
    //wait
    while((SPSR & (1 << SPIF)));
    
    //clock out 8 bytes
    for(int i = 0; i < 8; i++) {
        SPDR = 0x00;
        while(!(SPSR & (1 << SPIF)));
        data[i] = SPDR;
        data[i] = ((data[i] >> 4) * 10) + (data[i] & 0x0F);
    }
    
    //chip select high
    PORTB |= (1 << PORTB2);
}

int main(void) {
    
    LCD_Init();
    SPI_Init();
    I2C_Init();
    LCD_Clear();
       
    while (1) {
        uint8_t rtc_data[8] = {};
        unsigned char buffer[10];
        SPI_Read(rtc_data);
        float temp = readTempSHT21();
        
        dtostrf(temp, 3, 2, buffer);
        strcat(buffer, " C\n");
        
        char line1[16] = {};
        
        sprintf(line1, "%.2u/%.2u/%.2u", rtc_data[3], rtc_data[4], rtc_data[6]);
        
        LCD_Clear();
        LCD_String(line1);
        
        LCD_Command(0xC0);
        LCD_String(buffer);
        
        _delay_ms(1000);
    }
}
