#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>

#include "dht_driver.h"
#include "edge_ai.h"
#include "wifi_manager.h"

#define SERVER_URL "http://192.168.0.105:5000/data"

static const char *TAG = "MAIN_APP";
QueueHandle_t ai_queue;

TaskHandle_t h_sensor_task = NULL;
TaskHandle_t h_net_task = NULL;

void sensor_task(void *pvParameters) {
  dht_init();
  dht_reading_t raw_data;
  inference_result_t analysis;

  while (1) {
    raw_data = dht_read_data();

    if (raw_data.status == 0) {

      analysis = analyze_environment(raw_data.temp, raw_data.hum);

      if (analysis.state == STATE_CRITICAL) {
        ESP_LOGE(TAG, "ALERT: %s (Score: %d)", analysis.message,
                 analysis.risk_score);
      } else {
        ESP_LOGI(TAG, "Environment: %s (Score: %d)", analysis.message,
                 analysis.risk_score);
      }

      xQueueSend(ai_queue, &analysis, 0);
    } else {
      ESP_LOGW(TAG, "Sensor Read Error: %d", raw_data.status);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

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
        ESP_LOGI("NET", "Sent Payload (Size: %d)", strlen(post_buffer));
      } else {
        ESP_LOGW("NET", "Upload Failed: %s", esp_err_to_name(err));
      }

      cJSON_Delete(root);
      free(post_buffer);
    }
  }
}

void monitor_task(void *pvParameters) {
  while (1) {
    ESP_LOGI("HEALTH", "Heap: %lu | Stack Sensor: %d | Stack Net: %d",
             esp_get_free_heap_size(),
             (int)uxTaskGetStackHighWaterMark(h_sensor_task),
             (int)uxTaskGetStackHighWaterMark(h_net_task));
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void app_main(void) {
  nvs_flash_init();

  wifi_init_sta();
  wifi_wait_for_connection();

  ai_queue = xQueueCreate(5, sizeof(inference_result_t));

  xTaskCreate(sensor_task, "AI_Sensor", 4096, NULL, 5, &h_sensor_task);
  xTaskCreate(network_task, "Network", 4096, NULL, 4, &h_net_task);
  xTaskCreate(monitor_task, "Monitor", 2048, NULL, 1, NULL);
}