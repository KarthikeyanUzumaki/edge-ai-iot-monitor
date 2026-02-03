#include "comms_task.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void comms_task(void *pvParameters)
{
    while (1) {
        // Handle communications
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
