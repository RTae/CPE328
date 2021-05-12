#define F_CPU 8000000UL
 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <string.h>
#include <stdlib.h>
 
/*Constant variable*/
#define PIN_DATA PB0
#define PIN_CLOCK PB1
#define PIXEL_COUNT  8
#define PIXEL_BRIGHTNESS 0xff
 
/*Interrupt variable*/
volatile unsigned char data_in[20];
volatile unsigned char data_count = 0;
volatile unsigned char command_ready;
 
/* Led usage variable */
volatile uint8_t loop;
uint8_t LED[PIXEL_COUNT][3];
uint8_t LEB_TEMP[PIXEL_COUNT][3];
unsigned char text_ok[] = "OK!!\r\n";
unsigned char text_error[] = "WRONG COMMAND!!\r\n";
 
/*Define Color*/
uint8_t RED[3] = {255, 0, 0};
uint8_t GREEN[3] = {0, 255, 0};
uint8_t BLUE[3] = {0, 0, 255};
uint8_t ORANGE[3] = {255, 102, 0};
uint8_t YELLOW[3] = {255, 255, 0};
uint8_t PURPLE[3] = {128, 0, 128};
uint8_t WHITE[3] = {255, 255, 255};
uint8_t BLACK[3] = {0, 0, 0};
 
/*Interrupt of receive USART*/
ISR (USART_RX_vect){
    loop = 0;
    unsigned char text = UDR0;
    /*When see enter stop*/
    if (text == 0x000D) {
        data_count = 0;
        command_ready = 1;
    } else {
        data_in[data_count] = text;
        data_count++;
    }
}
 
/*Clear command array to set new command*/
void clearMem(){
    memset(data_in, 0, 20);
}
 
/*Get number after ':' */
int parse_assignment (volatile unsigned char *text)
{
    char cmdValue[16];
    const char *pch;
    pch = strchr(text, ':');
    strcpy(cmdValue, pch+1);
    return atoi(cmdValue);
}
 
/*USART Transmit*/
void USART_Transmit( unsigned char data ) {
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1 << UDRE0)) ) ;
    /* Put data into buffer, sends the data */
    UDR0 = data;
}
 
/*Print Text*/
void print(unsigned char *buffer) {
    for(int i=0; buffer[i] != 0; i++){
        USART_Transmit(buffer[i]);
    }
}
 
/*USART Init*/
void USART_Init(unsigned int ubrr) {
    /* Set baud rate */
    UBRR0 = ubrr;
    /* Double Transmission Speed*/
    UCSR0A |= (1 << U2X0);
    /* Enable receiver and transmitter */
    UCSR0B |= (1 << RXEN0)|(1 << TXEN0);
    /* Set frame format: 8data */
    UCSR0C |= (1 << UCSZ01)|(1 << UCSZ00);
    // Enable the USART Receive interrupt
    UCSR0B |= (1 << RXCIE0 );
    sei();
}
 
/*SPI Init*/
void SPI_Init(){
    /*Set Data and Clock pin to output*/
    DDRB |= (1 << PIN_DATA) | (1 << PIN_CLOCK);
    /*Set Data and Clock as low*/
    PORTB &= ~((1 << PIN_DATA) | (1 << PIN_CLOCK));
}
 
/*SPI write*/
void spi_write(uint8_t spi_data) {
	for (uint8_t i = 0; i < PIXEL_COUNT; i++) {
		if (spi_data & 0x80) {
                /*Set data high*/
                PORTB |= (1 << PIN_DATA);
			} else {
                /*Set data low*/
                PORTB &= ~(1 << PIN_DATA);
		}
        /*Set clock high*/
		PORTB |= (1 << PIN_CLOCK);
        /*Shit to next bit*/
		spi_data <<= 1;
        /*Set clock low*/
		PORTB &= ~(1 << PIN_CLOCK);
	}
}
 
void start_frame() {
    /*Send 32bit of 0 for start frame*/
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x00);
}
 
void end_frame() {
    /*Send 00001111 for end frame (8 leds)*/
	spi_write(0xff);
    spi_write(0xff);
    spi_write(0xff);
    spi_write(0xff);
}
 
void write_led(uint8_t *color){
    /*1110000 | BRIDTHESS (32 level)*/
    spi_write(PIXEL_BRIGHTNESS);
    spi_write(color[2]); // B
    spi_write(color[1]); // G
    spi_write(color[0]); // R
}
 
void setLEDsColor(uint8_t *L1, uint8_t *L2, uint8_t *L3, uint8_t *L4, 
                  uint8_t *L5, uint8_t *L6, uint8_t *L7, uint8_t *L8){
    /*Send start frame*/
	start_frame();
 
    /*Write 8 leds*/
    write_led(L1);
    write_led(L2);
    write_led(L3);
    write_led(L4);
    write_led(L5);
    write_led(L6);
    write_led(L7);
    write_led(L8);
 
    /*Send end frame*/
	end_frame();
}
 
/*Custom delay*/
void my_delay_ms(int n) {
 while(n--) {
  _delay_ms(1);
 }
}
 
/*Running led*/
void ledRunning(uint8_t *color, int delay){
    /*C 0 0 0 0 0 0 0*/
    setLEDsColor(color, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK);
    my_delay_ms(delay);
    /*0 C 0 0 0 0 0 0*/
    setLEDsColor(BLACK, color, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK);
    my_delay_ms(delay);
    /*0 0 C 0 0 0 0 0*/
    setLEDsColor(BLACK, BLACK, color, BLACK, BLACK, BLACK, BLACK, BLACK);
    my_delay_ms(delay);
    /*0 0 0 C 0 0 0 0*/
    setLEDsColor(BLACK, BLACK, BLACK, color, BLACK, BLACK, BLACK, BLACK);
    my_delay_ms(delay);
    /*0 0 0 0 C 0 0 0*/
    setLEDsColor(BLACK, BLACK, BLACK, BLACK, color, BLACK, BLACK, BLACK);
    my_delay_ms(delay);
    /*0 0 0 0 0 C 0 0*/
    setLEDsColor(BLACK, BLACK, BLACK, BLACK, BLACK, color, BLACK, BLACK);
    my_delay_ms(delay);
    /*0 0 0 0 0 0 C 0*/
    setLEDsColor(BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, color, BLACK);
    my_delay_ms(delay);
    /*0 0 0 0 0 0 0 C*/
    setLEDsColor(BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, color);
    my_delay_ms(delay);
}
 
/*Blink LED*/
void blinkLED(uint8_t *color, int delay){
    setLEDsColor(color, color, color, color, color, color, color, color);
    my_delay_ms(delay);
    setLEDsColor(BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK);
    my_delay_ms(delay);
}
 
/*Rainbow mode*/
void ledRainbow(int delay){
    /*C 0 0 0 0 0 0 0*/
    setLEDsColor(ORANGE, RED, PURPLE, BLUE, GREEN, YELLOW, ORANGE, RED);
    my_delay_ms(delay);
    /*0 C 0 0 0 0 0 0*/
    setLEDsColor(YELLOW, ORANGE, RED, PURPLE, BLUE, GREEN, YELLOW, ORANGE);
    my_delay_ms(delay);
    /*0 0 C 0 0 0 0 0*/
    setLEDsColor(GREEN, YELLOW, ORANGE, RED, PURPLE, BLUE, GREEN, YELLOW);
    my_delay_ms(delay);
    /*0 0 0 C 0 0 0 0*/
    setLEDsColor(BLUE, GREEN, YELLOW, ORANGE, RED, PURPLE, BLUE, GREEN);
    my_delay_ms(delay);
    /*0 0 0 0 C 0 0 0*/
    setLEDsColor(PURPLE, BLUE, GREEN, YELLOW, ORANGE, RED, PURPLE, BLUE);
    my_delay_ms(delay);
    /*0 0 0 0 0 C 0 0*/
    setLEDsColor(RED, PURPLE, BLUE, GREEN, YELLOW, ORANGE, RED, PURPLE);
    my_delay_ms(delay);
    /*0 0 0 0 0 0 C 0*/
    setLEDsColor(ORANGE, RED, PURPLE, BLUE, GREEN, YELLOW, ORANGE, RED);
    my_delay_ms(delay);
    /*0 0 0 0 0 0 0 C*/
    setLEDsColor(YELLOW, ORANGE, RED, PURPLE, BLUE, GREEN, YELLOW, ORANGE);
    my_delay_ms(delay);
}
 
/*Set all led to black*/
void clearLED(){
    setLEDsColor(BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK);
}
 
/*Running command led*/
void runningLEDCommnd(){
    /* Set loop */
    loop = 1;
    /*Get delay time*/
    int delay = parse_assignment(data_in);
    /*If text index 2 is R running led with red color*/
    if (data_in[1] == 'R'){
        while(loop){
            ledRunning(RED, delay);
        }
    /*If text index 2 is G running led with green color*/
    } else if(data_in[1] == 'G'){
        while(loop){
            ledRunning(GREEN, delay);
        }
    /*If text index 2 is O running led with orange color*/
    } else if(data_in[1] == 'O'){
        while(loop){
            ledRunning(ORANGE, delay);
        }
    /*If text index 2 is Y running led with yellow color*/
    }else if(data_in[1] == 'Y'){
        while(loop){
            ledRunning(YELLOW, delay);
        }
    /*If text index 2 is P running led with purple color*/
    }else if(data_in[1] == 'P'){
        while(loop){
            ledRunning(PURPLE, delay);
        }
    /*If text index 2 is B running led with blue color*/
    }else{
        while(loop){
            ledRunning(BLUE, delay);
        }
    }
    /*Clear memory*/
    clearMem();
    /*Set command to not ready*/
    command_ready = 0;
}
 
void blinkLEDCommad(){
    /* Set loop */
    loop = 1;
    /*Get delay time*/
    int delay = parse_assignment(data_in);
    /*If text index 2 is R blink led with red color*/
    if (data_in[1] == 'R'){
        while(loop){
            blinkLED(RED, delay);
        }
    /*If text index 2 is R blink led with green color*/
    } else if(data_in[1] == 'G'){
        while(loop){
            blinkLED(GREEN, delay);
        }
    /*If text index 2 is O blink led with orange color*/
    }else if(data_in[1] == 'O'){
        while(loop){
            blinkLED(ORANGE, delay);
        }
    /*If text index 2 is R blink led with yellow color*/
    }else if(data_in[1] == 'Y'){
        while(loop){
            blinkLED(YELLOW, delay);
        }
    /*If text index 2 is R blink led with purple color*/
    }else if(data_in[1] == 'P'){
        while(loop){
            blinkLED(PURPLE, delay);
        }
    /*If text index 2 is R blink led with blue color*/
    } else{
        while(loop){
            blinkLED(BLUE, delay);
        }
    }
    /*Clear memory*/
    clearMem();
    /*Set command to not ready*/
    command_ready = 0;
}
 
void rainbowLEDCommand() {
    loop = 1;
    /*Get delay time*/
    int delay = parse_assignment(data_in);
    while(loop){
        ledRainbow(delay);
    }
}
 
/* Set all color by loop through led*/
void setAllColorLoop(uint8_t *color){
    for(uint8_t i=0;i<PIXEL_COUNT;i++){
        for(uint8_t j=0;j<3;j++){
            LED[i][j] = color[j];
        }
    }
}
 
/* Set all color command*/
void setAllColorCommand(){
    if (data_in[1] == 'R'){
        setAllColorLoop(RED);
    } else if(data_in[1] == 'G'){
        setAllColorLoop(GREEN);
    } else if(data_in[1] == 'O'){
        setAllColorLoop(ORANGE);
    } else if(data_in[1] == 'Y'){
        setAllColorLoop(YELLOW);
    } else if(data_in[1] == 'P'){
        setAllColorLoop(PURPLE);
    }
    else{
        setAllColorLoop(BLUE);
    }
}
 
/* Set specific led color*/
void setColorLEDLoop(int index, uint8_t *color){
    for(uint8_t j=0;j<3;j++){
        LED[index][j] = color[j];
    }
}
 
/*Set specific led color command*/
void setColorCommand(){
    int index = parse_assignment(data_in)-1;
    if (data_in[1] == 'R'){
        setColorLEDLoop(index, RED);
    } else if(data_in[1] == 'G'){
        setColorLEDLoop(index, GREEN);
    }else if(data_in[1] == 'O'){
        setColorLEDLoop(index, ORANGE);
    }else if(data_in[1] == 'Y'){
        setColorLEDLoop(index, YELLOW);
    }else if(data_in[1] == 'P'){
        setColorLEDLoop(index, PURPLE);
    } else{
        setColorLEDLoop(index, BLUE);
    }
}
 
/*ON - OFF Led*/
void onOffLedCommad(){
    int index = parse_assignment(data_in)-1;
    if(data_in[1] == 'F'){
        for(uint8_t j=0;j<3;j++){
            LEB_TEMP[index][j] = LED[index][j];
            LED[index][j] = BLACK[j];
 
        }
    } else if(data_in[0] == 'O'){
        for(uint8_t j=0;j<3;j++){
            LED[index][j] = LEB_TEMP[index][j];
        }
    }
}
 
int main(void)
{   
    SPI_Init();
    USART_Init(103);
    while(1) {
        /* Wait for USART Interrupt*/
        if (command_ready) {
            /*LED RUNNING*/
            if (data_in[0] == 'R'){
                print(text_ok);
                runningLEDCommnd();
            /*LED BLINK*/
            } else if (data_in[0] == 'B') {
                print(text_ok);
                blinkLEDCommad();
            }else if (data_in[0] == 'I') {
                print(text_ok);
                rainbowLEDCommand();
            } else if ( data_in[0] == 'S' || 
                        data_in[0] == 'L' || 
                        data_in[0] == 'O') {
                print(text_ok);
                /*SET ALL LED COLOR*/
                if(data_in[0] == 'S'){
                    setAllColorCommand();
                /*SET SPECIFIC LED COLOR*/
                } else if(data_in[0] == 'L') {
                    setColorCommand();
                /*TOGGLE LED COLOR*/
                } else if(data_in[0] == 'O') {
                    onOffLedCommad();
                }
                setLEDsColor(LED[0], LED[1], LED[2], LED[3], 
                             LED[4], LED[5], LED[6], LED[7]);
                clearMem();
                command_ready = 0;
            /*OFF ALL LED COLOR*/
            } else if(data_in[0] == 'F'){
                print(text_ok);
                clearLED();
                clearMem();
                command_ready = 0;
            /*HANDLE WRONG COMMAND*/
            } else {
                print(text_error);
                clearMem();
                command_ready = 0;
            }
        }
    }
}  
