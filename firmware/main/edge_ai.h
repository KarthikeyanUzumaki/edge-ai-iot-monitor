#ifndef EDGE_AI_H
#define EDGE_AI_H

typedef enum {
  STATE_NORMAL = 0,
  STATE_WARNING = 1,
  STATE_CRITICAL = 2
} system_state_t;

typedef struct {
  float temp;
  float hum;
  int risk_score;
  system_state_t state;
  char message[32];
} inference_result_t;

inference_result_t analyze_environment(float temp, float hum);

#endif