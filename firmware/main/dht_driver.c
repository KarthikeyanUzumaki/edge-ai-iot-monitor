#include "dht_driver.h"
#include "esp_log.h"
#include "rom/ets_sys.h"


static const char *TAG = "DHT_DRIVER";

void dht_init(void) {
  gpio_reset_pin(DHT_PIN);
  gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);
}

dht_reading_t dht_read_data(void) {
  dht_reading_t result = {0, 0, -1};
  uint8_t bits[5] = {0};
  uint8_t cnt = 7;
  uint8_t idx = 0;

  gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT_PIN, 0);
  ets_delay_us(20000);
  gpio_set_level(DHT_PIN, 1);
  ets_delay_us(40);
  gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);

  int timeout = 0;
  while (gpio_get_level(DHT_PIN) == 0)
    if (timeout++ > 1000)
      return result;
  timeout = 0;
  while (gpio_get_level(DHT_PIN) == 1)
    if (timeout++ > 1000)
      return result;

  for (int i = 0; i < 40; i++) {
    timeout = 0;
    while (gpio_get_level(DHT_PIN) == 0)
      if (timeout++ > 1000)
        return result;

    ets_delay_us(28);

    if (gpio_get_level(DHT_PIN)) {
      bits[idx] |= (1 << cnt);
      timeout = 0;
      while (gpio_get_level(DHT_PIN) == 1)
        if (timeout++ > 1000)
          return result;
    }

    if (cnt == 0) {
      cnt = 7;
      idx++;
    } else {
      cnt--;
    }
  }

  if ((bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
    result.hum = (float)bits[0];
    result.temp = (float)bits[2];
    result.status = 0;
  } else {
    result.status = -2;
  }

  return result;
}