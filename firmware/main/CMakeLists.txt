idf_component_register(SRCS "main.c" "dht_driver.c" "wifi_manager.c" "edge_ai.cc"
                    INCLUDE_DIRS "."
                    REQUIRES esp-tflite-micro)