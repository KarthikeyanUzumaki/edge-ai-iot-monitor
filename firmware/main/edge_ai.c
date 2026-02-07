#include "edge_ai.h"
#include <stdio.h>
#include <string.h>

#define TEMP_WARN_THRESH 28.0
#define TEMP_CRIT_THRESH 32.0
#define HUM_WARN_THRESH 70.0
#define HUM_CRIT_THRESH 85.0

inference_result_t analyze_environment(float temp, float hum) {
  inference_result_t result;
  result.temp = temp;
  result.hum = hum;
  result.risk_score = 0;
  result.state = STATE_NORMAL;
  strcpy(result.message, "Nominal");

  if (temp > 25.0) {
    result.risk_score += (int)((temp - 25.0) * 10);
  }

  if (hum > HUM_WARN_THRESH) {
    result.risk_score += 10;
  }

  if (result.risk_score > 100)
    result.risk_score = 100;
  if (result.risk_score < 0)
    result.risk_score = 0;

  if (temp >= TEMP_CRIT_THRESH || hum >= HUM_CRIT_THRESH) {
    result.state = STATE_CRITICAL;
    if (temp >= TEMP_CRIT_THRESH)
      strcpy(result.message, "CRITICAL: Overheat");
    else
      strcpy(result.message, "CRITICAL: Moisture");
  } else if (temp >= TEMP_WARN_THRESH || hum >= HUM_WARN_THRESH) {
    result.state = STATE_WARNING;
    strcpy(result.message, "WARNING: Limits near");
  }

  return result;
}