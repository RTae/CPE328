#define F_CPU 8000000L
#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t xSemaphore;

ISR (INT0_vect){
    xSemaphoreGiveFromISR(xSemaphore, NULL);
    
}

ISR (INT1_vect){
    xSemaphoreGiveFromISR(xSemaphore, NULL);
    
}

void buttonHandlerSlow(void* params){
    DDRB |= 0x0f;
    unsigned char led_state[4] = {0x01, 0x02, 0x04, 0x08};
    while(1){
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE){
            for(int i=0; i < sizeof(led_state); i++){
                PORTB |= led_state[i];
                vTaskDelay(500 / portTICK_PERIOD_MS);
                PORTB &= ~led_state[i];
                vTaskDelay(500 / portTICK_PERIOD_MS);   
            }
        }
    }
}

void buttonHandlerFast(void* params){
    DDRB |= 0xf0;
    unsigned char led_state[4] = {0x10, 0x20, 0x40, 0x80};
    while(1){
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE){
            for(int i=0; i < sizeof(led_state); i++){
                PORTB |= led_state[i];
                vTaskDelay(250 / portTICK_PERIOD_MS);
                PORTB &= ~led_state[i];
                vTaskDelay(250 / portTICK_PERIOD_MS);   
            }
        }
    }
}

int main(void) {
    PORTD |= (1 << PORTD2) | (1 << PORTD3);
    EICRA |= (1 << ISC01) | (1 << ISC11);
    EIMSK |= (1 << INT0) | (1 << INT1);
    
    xSemaphore = xSemaphoreCreateBinary();
    
    xTaskCreate(buttonHandlerSlow, "slow", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(buttonHandlerFast, "fast", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();
    while(1);
}
