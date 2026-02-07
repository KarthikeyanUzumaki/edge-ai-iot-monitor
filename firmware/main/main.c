#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "driver/gpio.h"
#include "cJSON.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"

// ====================================================
// CONFIGURATION (EDIT THESE 3 LINES)
// ====================================================
#define WIFI_SSID      "ACT_0E41_5G"
#define WIFI_PASS      "EMACxt8B"
#define SERVER_URL     "http://192.168.0.105:5000/data"  // <--- CHANGED .106 to .105

// HARDWARE PIN DEFINITIONS
#define DHT_PIN        GPIO_NUM_15            // <--- UPDATED TO PIN 15
#define LED_PIN        GPIO_NUM_2             // Onboard Blue LED

static const char *TAG = "IOT_FIRMWARE";
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

typedef struct {
    float temp;
    float hum;
} dht_data_t;

// ====================================================
// DHT11 SENSOR DRIVER (MANUAL IMPLEMENTATION)
// ====================================================
int dht_read(dht_data_t *data) {
    uint8_t bits[5] = {0};
    uint8_t cnt = 7;
    uint8_t idx = 0;

    // 1. SEND START SIGNAL
    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_PIN, 0);
    ets_delay_us(20000); // Pull low for 20ms
    gpio_set_level(DHT_PIN, 1);
    ets_delay_us(40);
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);

    // 2. WAIT FOR SENSOR RESPONSE
    int timeout = 0;
    while (gpio_get_level(DHT_PIN) == 0) {
        if (timeout++ > 1000) return -1; // Timeout waiting for response
        ets_delay_us(1);
    }
    timeout = 0;
    while (gpio_get_level(DHT_PIN) == 1) {
        if (timeout++ > 1000) return -1; // Timeout waiting for data
        ets_delay_us(1);
    }

    // 3. READ 40 BITS OF DATA
    for (int i = 0; i < 40; i++) {
        timeout = 0;
        while (gpio_get_level(DHT_PIN) == 0) {
            if (timeout++ > 1000) return -1;
            ets_delay_us(1);
        }

        ets_delay_us(28); // Wait to see if bit is 0 or 1

        if (gpio_get_level(DHT_PIN)) {
            bits[idx] |= (1 << cnt);
            timeout = 0;
            while (gpio_get_level(DHT_PIN) == 1) {
                if (timeout++ > 1000) return -1;
                ets_delay_us(1);
            }
        }
        
        if (cnt == 0) {
            cnt = 7;
            idx++;
        } else {
            cnt--;
        }
    }

    // 4. VERIFY CHECKSUM
    if ((bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
        data->hum = bits[0];
        data->temp = bits[2];
        return 0; // Success
    }
    return -2; // Checksum Error
}

// ====================================================
// WIFI & HTTP FUNCTIONS
// ====================================================
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGW(TAG, "WiFi Disconnected. Retrying...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "WiFi Connected! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void send_data_to_backend(dht_data_t *data) {
    char *post_data = NULL;
    cJSON *root = cJSON_CreateObject();
    
    // Create JSON: {"temp": 25.0, "hum": 60.0, "accel_x": 0, "accel_y": 0}
    cJSON_AddNumberToObject(root, "temp", data->temp);
    cJSON_AddNumberToObject(root, "hum", data->hum);
    cJSON_AddNumberToObject(root, "accel_x", 0); // Placeholder for now
    cJSON_AddNumberToObject(root, "accel_y", 0); // Placeholder for now

    post_data = cJSON_PrintUnformatted(root);
    
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Data Sent! Temp: %.1f C | Hum: %.1f %%", data->temp, data->hum);
    } else {
        ESP_LOGW(TAG, "HTTP Send Failed (Check Server IP)");
    }

    esp_http_client_cleanup(client);
    cJSON_Delete(root);
    free(post_data);
}

// ====================================================
// MAIN APPLICATION LOOP
// ====================================================
void app_main(void) {
    // 1. Initialize Storage & GPIO
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // 2. Start WiFi
    ESP_LOGI(TAG, "Starting WiFi...");
    wifi_init_sta();

    // 3. Wait for Connection
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    dht_data_t sensor_data = {0};

    // 4. Main Loop
    while (1) {
        // Read Sensor (Try reading on GPIO 15)
        if (dht_read(&sensor_data) == 0) {
            gpio_set_level(LED_PIN, 1); // Blink LED on success
            send_data_to_backend(&sensor_data);
            gpio_set_level(LED_PIN, 0);
        } else {
            ESP_LOGE(TAG, "DHT Read Failed! Check wire on GPIO 15 (P15)");
        }

        // Wait 2 seconds
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}