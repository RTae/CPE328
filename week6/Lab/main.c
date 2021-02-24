#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LCD_Dir  DDRB			/* Define LCD data port direction */
#define LCD_Port PORTB			/* Define LCD data port */
#define RS PB0				    /* Define Register Select pin */
#define EN PB1 				    /* Define Enable signal pin */

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

void ADC_Init(){
    ADMUX  |= (1 << REFS0) | (1 << MUX2) | (1 << MUX0);           // Select the ADC channel and AVCC as Ref voltage
    ADCSRA |= (1 << ADEN) |(1<<ADPS2) | (1<<ADPS1);               // Select ADC Prescalar to 64 
    DIDR0 |= (1 << ADC5D);
}

uint16_t ADC_Read(){
    
    ADCSRA |= (1<<ADSC);                    // Enable ADC and Start the conversion
    
    while( !(ADCSRA & (1<<ADIF)) );         // Wait for interrupt

    uint16_t d_out = ADC;                   // Copt ADC out already concatenate ADCL ADCH
    ADCSRA |= (1<<ADIF);                    // Clear Flag
    return d_out;                           // Return value
}

int main(void) {

    USART_Init(53);         /* Initialization of USART*/
    ADC_Init();             /* Initialization of ADC*/
	LCD_Init();             /* Initialization of LCD*/
    LCD_Clear();
    _delay_ms(1000);
    
    uint16_t sensor;
    float temp;
    unsigned char text[] = "Temp = ";
    unsigned char buffer[10];

    while (1) {
        sensor = ADC_Read();          /* Read data from sensor */
        temp = (((sensor/1024.0) * 5) - 0.5) * 100.0 ;
        
        dtostrf(temp, 4, 2, buffer);
        strcat(buffer, " C\n");
        
        print(text);
        print(buffer);
        
        LCD_Clear();          // Clear LCD
        _delay_ms(100); 
        LCD_String(text);     // Sent Message
        LCD_String(buffer);

        _delay_ms(1000);
    }
}
