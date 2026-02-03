#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void led_task(void *pvParameters)
{
    while (1) {
        // Blink LED
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
