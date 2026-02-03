#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor_task.h"
#include "inference_task.h"
#include "comms_task.h"

void app_main(void)
{
    printf("Edge AI IoT Monitor Starting...\n");

    // Initialize tasks
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    xTaskCreate(inference_task, "inference_task", 4096, NULL, 5, NULL);
    xTaskCreate(comms_task, "comms_task", 4096, NULL, 5, NULL);
}
