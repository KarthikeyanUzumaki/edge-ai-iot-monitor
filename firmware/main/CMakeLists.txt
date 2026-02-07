idf_component_register(
    SRCS "main.c" "dht_driver.c" "wifi_manager.c" "edge_ai.cc"
    INCLUDE_DIRS "."
    REQUIRES esp-tflite-micro nvs_flash esp_event driver esp_wifi esp_http_client json
)