#ifndef EDGE_AI_H
#define EDGE_AI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_NORMAL = 0,
    STATE_WARNING = 1,
    STATE_CRITICAL = 2
} system_state_t;

typedef struct {
    float temp;
    float hum;
    float probability;    // <--- This replaced risk_score
    system_state_t state; 
    char message[32];
} inference_result_t;

// Init function to load the model
void ai_init(void);

// Run inference
inference_result_t analyze_environment(float temp, float hum);

#ifdef __cplusplus
}
#endif

#endif