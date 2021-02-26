#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LCD_Dir  DDRB			      /* Define LCD data port direction */
#define LCD_Port PORTB			    /* Define LCD data port */
#define RS PB0				          /* Define Register Select pin */
#define EN PB1 				          /* Define Enable signal pin */
#define DS1307_ADDR 0xD0        /* Define DS1307 Address */

void LCD_Command( unsigned char cmnd )
{
	  LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0);  /* sending upper nibble */
	  LCD_Port &= ~ (1<<RS);		                     /* RS=0, command reg. */
	  LCD_Port |= (1<<EN);		                       /* Enable pulse */
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
	  LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0);  /* sending upper */
	  LCD_Port |= (1<<RS);		                       /* RS=1, data reg. */
	  LCD_Port|= (1<<EN);
	  _delay_us(1);
	  LCD_Port &= ~ (1<<EN);

	  _delay_us(200);

	  LCD_Port = (LCD_Port & 0x0F) | (data << 4);    /* sending lower */
	  LCD_Port |= (1<<EN);
	  _delay_us(1);
	  LCD_Port &= ~ (1<<EN);
	  _delay_ms(2);
}

void LCD_Init (void)			      /* LCD Initialize function */
{
	  LCD_Dir = 0xFF;			        /* Make LCD port direction as o/p */
	  _delay_ms(20);			        /* LCD Power ON delay always >15ms */
	
	  LCD_Command(0x02);		      /* send for 4 bit initialization of LCD  */
	  LCD_Command(0x28);          /* 2 line, 5*7 matrix in 4-bit mode */
	  LCD_Command(0x0c);          /* Display on cursor off*/
	  LCD_Command(0x06);          /* Increment cursor (shift cursor to right)*/
	  LCD_Command(0x01);          /* Clear display screen*/
	  _delay_ms(2);
}


void LCD_String (char *str)		                    /* Send string to LCD function */
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
    // SCL, SDA as output
    DDRC |= (1 << DDC4) | (1 << DDC5);
  
    // Init I2C
    // 100kHz @ prescaler /4
    TWBR = 8;
    TWCR |= (1 << TWPS0);

    // Enable I2C
    TWCR |= (1 << TWEN);
}

void I2C_Start(){
    // Send S
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
    // Wait complete
    while(!(TWCR & (1 << TWINT)));
}

void I2C_Stop(){
    // Send P
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
}

void I2C_Write(uint8_t data){
    // Write data
    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT);
    // Wait complete
    while(!(TWCR & (1 << TWINT)));
}

uint8_t I2C_ReadAck() {
    // Read and Send Ack
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    // Wait complete
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

uint8_t I2C_ReadNAck() {
    // Read and not send Ack
    TWCR = (1<<TWINT)|(1<<TWEN);
    // Wait complete
    while (!(TWCR & (1<<TWINT)));
    return TWDR;
}

uint8_t I2C_GetStatusCode() {
    uint8_t status;
    // Get Status
    status = TWSR & 0xF8;
    return status;
}

void dateFormat(char *dateBuffer, uint8_t day, uint8_t date, uint8_t month, uint8_t year){
    char dict_day[7][5] = {"Mon", "Tue", "Wed", "Thr", "Fri", "Sat", "Sun"};
    char dict_month[12][5] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",};
    
    char temp[10];
    
    // Format date from DS1307
    strcat(dateBuffer, dict_day[day]);
    strcat(dateBuffer, " ");
    sprintf(temp, "%u", date);
    strcat(dateBuffer, temp);
    strcat(dateBuffer, " ");
    strcat(dateBuffer, dict_month[month]);
    strcat(dateBuffer, " ");
    sprintf(temp, "20%u\n", year);
    strcat(dateBuffer, temp);
}

void readTimeDS1307() {
    // Send S
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();
    
    // Send SLA + W
    I2C_Write((uint8_t)(DS1307_ADDR));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    // Send address that want to read 
    I2C_Write((uint8_t) 0x00);
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();
    
    // SR
    I2C_Start();
    if(I2C_GetStatusCode() != 0x10) I2C_Stop();
    
    // Send SLA + R
    I2C_Write((uint8_t)(DS1307_ADDR + 1));
    if (I2C_GetStatusCode() != 0x40) I2C_Stop();
    
    uint8_t temp[7];
    for(int i=0; i < 8; i++){
        // Get data and send Ack
        temp[i] = I2C_ReadAck();
        if (I2C_GetStatusCode() != 0x50) I2C_Stop();
        
        // Get data and not send Ack
        if(i == 7){
            temp[i] = I2C_ReadNAck();
            if (I2C_GetStatusCode() != 0x58) I2C_Stop();
        }
    }
    
    // Send P
    I2C_Stop();

    // Format to time system
    uint8_t second = ((temp[0] & 0x70) >> 4) * 10 + (temp[0] & 0x0F);
    uint8_t minute = ((temp[1] & 0x70) >> 4) * 10 + (temp[1] & 0x0F);
    uint8_t hour = ((temp[2] & 0x70) >> 4) * 10 + (temp[2] & 0x0F);
    uint8_t day = (temp[3] & 0x0F);
    uint8_t date = ((temp[4] & 0x70) >> 4) * 10 + (temp[4] & 0x0F);
    uint8_t month = ((temp[5] & 0x70) >> 4) * 10 + (temp[5] & 0x0F);
    uint8_t year = ((temp[6] & 0x70) >> 4) * 10 + (temp[6] & 0x0F);
    
    // Format the term to show
    char timeBuffer[10];
    char dateBuffer[20] = "";
    sprintf(timeBuffer, "%.2u:%.2u:%.2u\n", hour, minute, second);
    dateFormat(dateBuffer, day, date, month, year);
    
    // Show to lcd
    LCD_Clear();
    LCD_String(dateBuffer);
    LCD_Command (0xC0);
    LCD_String(timeBuffer);
    _delay_ms(1000);
}


uint8_t setTimeDS1307(uint8_t *data){

    // Send S
    I2C_Start();
    if(I2C_GetStatusCode() != 0x08) I2C_Stop();
    
    // Send SLA + W
    I2C_Write((uint8_t)(DS1307_ADDR));
    if (I2C_GetStatusCode() != 0x18) I2C_Stop();
    
    // Send address to write
    I2C_Write((uint8_t)(0x00));
    if (I2C_GetStatusCode() != 0x28) I2C_Stop();

    for(int i=0; i < 8; i++){
      // Write each data to slave
      I2C_Write((uint8_t)(data[i]));
      if (I2C_GetStatusCode() != 0x28) I2C_Stop();
    }

    // Send P
    I2C_Stop();
}

int main(void) {
    I2C_Init();             /* Initialization of I2C*/
    LCD_Init();             /* Initialization of LCD*/
    LCD_Clear();
    _delay_ms(1000);   
    
    /* Second, Minute, Hour, Day, Date, Month, Year*/
    uint8_t timeInit[] = {0x00, 0x36, 0x19, 0x02, 0x24, 0x01, 0x21};
    
    // Set time for DS1307 
    setTimeDS1307(timeInit);
    
    while (1) {
      // Read the current time from DS1307 
      readTimeDS1307();
    }
}
