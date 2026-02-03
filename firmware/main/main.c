#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define My_led GPIO_NUM_2

void app_main(void){
    gpio_reset_pin(My_led);
    gpio_set_direction(My_led, GPIO_MODE_OUTPUT);
    float i=0;
    while(i<10){
        gpio_set_level(My_led, 1);
        printf("LED is ON\n");
        vTaskDelay(500/ portTICK_PERIOD_MS);
        gpio_set_level(My_led, 0);
        printf("LED is OFF\n");
        vTaskDelay(1000/ portTICK_PERIOD_MS);
        i=i+ 1;
    }
    printf("Blinking Ended\n");
}