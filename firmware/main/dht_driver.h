#ifndef DHT_DRIVER_H
#define DHT_DRIVER_H

#include "driver/gpio.h"

#define DHT_PIN GPIO_NUM_15

typedef struct {
  float temp;
  float hum;
  int status; // 0 = Success, -1 = Timeout, -2 = Checksum Error
} dht_reading_t;

// Function Prototypes
void dht_init(void);
dht_reading_t dht_read_data(void);

#endif