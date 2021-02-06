#include <avr/io.h>
#include <util/delay.h>

#define LCD_Dir  DDRB			/* Define LCD data port direction */
#define LCD_Port PORTB			/* Define LCD data port */
#define RS PB0				    /* Define Register Select pin */
#define EN PB1 				    /* Define Enable signal pin */


void USART_Init(unsigned int ubrr) {
    /* Set baud rate */
    UBRR0 = ubrr;
    /* Double Transmission Speed*/
    UCSR0A |= (1 << U2X0);
    /* Enable receiver and transmitter */
    UCSR0B |= (1 << RXEN0)|(1 << TXEN0);
    /* Set frame format: 8data */
    UCSR0C |= (1 << UCSZ01)|(1 << UCSZ00);
}

void USART_Transmit( unsigned char data ) {
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1 << UDRE0)) ) ;
    /* Put data into buffer, sends the data */
    UDR0 = data;
}
unsigned char USART_Receive() {
    /* Wait for data to be received */
    while ( !(UCSR0A & (1 << RXC0)) ) ;
    /* Get and return received data from buffer */
    return UDR0;
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
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0);    /* sending upper nibble */
	LCD_Port |= (1<<RS);		                     /* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4);      /* sending lower nibble */
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
	for(i=0; str[i]!=0 && str[i]!=0x000D; i++)		/* Send each char of string till the NULL And String should not be newline*/
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

int main(void) {
    
    USART_Init(103); // USART init
    char text_hello[] = "Hello "; // Hello[space]
    unsigned char text_name[20]; // Buffer to keep char from UART
    int k = 0; // Loop check lenght of text
    int index_con = 0; // index for concat
    
	LCD_Init();			/* Initialization of LCD*/
    LCD_Clear();
    _delay_ms(1000);

    while (1) {
        char receive_char = USART_Receive(); // receive char from USART
        
        if (receive_char == 0x000a){ // check for new line
            int MAX_BUFFER = sizeof(text_hello) + k;
            
            char buffer[MAX_BUFFER];
            
            for(int j=0;j<sizeof(text_hello);j++){ // Concat Hello
                if (text_hello[j] == 0x00){
                   break;
                } else{
                    buffer[index_con] = text_hello[j];
                    index_con++;
                }
            }
            
            for(int j=0;j<k;j++){ // Concat String
               buffer[index_con] = text_name[j];
               index_con++;
            }
            
            for(int i=0; i < sizeof(buffer) - 1; i++){ // Sent to computer with USART
                USART_Transmit(buffer[i]);
            }            
            
            LCD_Clear(); // Clear LCD
            _delay_ms(3000);
            LCD_String(buffer);	 // Sent Message
            
            k=0;
            index_con=0;
        } else {
            text_name[k] = receive_char;
            k++;
        }
     }
}
