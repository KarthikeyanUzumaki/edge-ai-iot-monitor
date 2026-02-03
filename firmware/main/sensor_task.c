#include "sensor_task.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void sensor_task(void *pvParameters)
{
    while (1) {
        // Read sensor data
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
