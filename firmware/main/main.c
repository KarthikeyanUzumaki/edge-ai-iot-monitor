#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "driver/gpio.h" // Required for LEDs

// MODULES
#include "dht_driver.h"
#include "wifi_manager.h"
#include "edge_ai.h"

// ====================================================
// CONFIGURATION
// ====================================================
#define SERVER_URL "http://192.168.0.105:5000/data" // <--- CHECK IP

// PIN DEFINITIONS
#define LED_PIN_G      GPIO_NUM_18  // GREEN (Normal)
#define LED_PIN_Y      GPIO_NUM_19  // YELLOW (Warning)
#define LED_PIN_R      GPIO_NUM_21  // RED (Critical)
#define LED_ONBOARD    GPIO_NUM_2   // Blue Onboard LED

static const char *TAG = "MAIN_APP";
QueueHandle_t ai_queue;

TaskHandle_t h_sensor_task = NULL;
TaskHandle_t h_net_task = NULL;

// --- LED HELPER FUNCTION ---
void update_traffic_lights(system_state_t state) {
    // Reset all first
    gpio_set_level(LED_PIN_G, 0);
    gpio_set_level(LED_PIN_Y, 0);
    gpio_set_level(LED_PIN_R, 0);

    // Set based on state
    switch(state) {
        case STATE_NORMAL:
            gpio_set_level(LED_PIN_G, 1);
            break;
        case STATE_WARNING:
            gpio_set_level(LED_PIN_Y, 1);
            break;
        case STATE_CRITICAL:
            gpio_set_level(LED_PIN_R, 1);
            // Blink Red? (Optional advanced feature)
            break;
    }
}

// 1. SENSOR & AI TASK
void sensor_task(void *pvParameters) {
    dht_init();
    
    // Configure LED Pins as Output
    gpio_reset_pin(LED_PIN_G); gpio_set_direction(LED_PIN_G, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_PIN_Y); gpio_set_direction(LED_PIN_Y, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_PIN_R); gpio_set_direction(LED_PIN_R, GPIO_MODE_OUTPUT);

    dht_reading_t raw_data;
    inference_result_t analysis;

    while (1) {
        raw_data = dht_read_data();
        
        if (raw_data.status == 0) {
            // A. RUN EDGE AI
            analysis = analyze_environment(raw_data.temp, raw_data.hum);
            
            // B. UPDATE PHYSICAL LEDS
            update_traffic_lights(analysis.state);

            // C. LOGGING
            if (analysis.state == STATE_CRITICAL) {
                ESP_LOGE(TAG, "🚨 ALERT: %s", analysis.message);
            } else {
                ESP_LOGI(TAG, "Env: %s (State: %d)", analysis.message, analysis.state);
            }

            // D. SEND TO QUEUE
            xQueueSend(ai_queue, &analysis, 0); 
        } else {
            ESP_LOGW(TAG, "Sensor Read Error: %d", raw_data.status);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

// 2. NETWORK TASK
void network_task(void *pvParameters) {
    inference_result_t package;
    char *post_buffer = NULL;

    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    while (1) {
        if (xQueueReceive(ai_queue, &package, portMAX_DELAY) == pdTRUE) {
            
            cJSON *root = cJSON_CreateObject();
            cJSON_AddNumberToObject(root, "temp", package.temp);
            cJSON_AddNumberToObject(root, "hum", package.hum);
            cJSON_AddNumberToObject(root, "risk_score", package.risk_score);
            cJSON_AddStringToObject(root, "status_msg", package.message);
            cJSON_AddNumberToObject(root, "alert_level", (int)package.state); 
            cJSON_AddNumberToObject(root, "free_heap", esp_get_free_heap_size());

            post_buffer = cJSON_PrintUnformatted(root);

            esp_http_client_set_header(client, "Content-Type", "application/json");
            esp_http_client_set_post_field(client, post_buffer, strlen(post_buffer));
            
            esp_err_t err = esp_http_client_perform(client);
            if (err == ESP_OK) {
                // Blink Onboard Blue LED on successful upload
                gpio_set_level(LED_ONBOARD, 1);
                vTaskDelay(pdMS_TO_TICKS(50));
                gpio_set_level(LED_ONBOARD, 0);
            } 

            cJSON_Delete(root);
            free(post_buffer);
        }
    }
}

// 3. MONITOR TASK
void monitor_task(void *pvParameters) {
    while (1) {
        ESP_LOGI("HEALTH", "Heap: %lu", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void) {
    nvs_flash_init();
    
    // Init Onboard LED
    gpio_reset_pin(LED_ONBOARD);
    gpio_set_direction(LED_ONBOARD, GPIO_MODE_OUTPUT);

    wifi_init_sta();
    wifi_wait_for_connection();

    ai_queue = xQueueCreate(5, sizeof(inference_result_t));
    ai_init();

    xTaskCreate(sensor_task, "AI_Sensor", 4096, NULL, 5, &h_sensor_task);
    xTaskCreate(network_task, "Network", 4096, NULL, 4, &h_net_task);
    xTaskCreate(monitor_task, "Monitor", 2048, NULL, 1, NULL);
}