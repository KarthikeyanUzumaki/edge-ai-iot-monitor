#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_event.h"

#define WIFI_SSID "ACT_0E41_5G"
#define WIFI_PASS "EMACxt8B"

void wifi_init_sta(void);
void wifi_wait_for_connection(void);

#endif