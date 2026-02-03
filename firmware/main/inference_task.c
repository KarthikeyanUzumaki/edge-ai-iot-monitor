#include "inference_task.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void inference_task(void *pvParameters)
{
    while (1) {
        // Run inference
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
