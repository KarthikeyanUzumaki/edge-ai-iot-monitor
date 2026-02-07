#include "edge_ai.h"
#include "model_data.h" // The file you generated
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "esp_log.h"
#include <string.h>

static const char *TAG = "EDGE_AI";

// Globals for TFLite
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Memory Arena (Tensor Workspace)
// 4KB is usually enough for simple models
constexpr int kTensorArenaSize = 4 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

void ai_init(void) {
    model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        ESP_LOGE(TAG, "Model schema mismatch!");
        return;
    }

    // Pull in only the operations we need (Fully Connected / Softmax)
    static tflite::MicroMutableOpResolver<3> resolver;
    resolver.AddFullyConnected();
    resolver.AddRelu();
    resolver.AddSoftmax();

    // Build the Interpreter
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    // Allocate memory
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        ESP_LOGE(TAG, "AllocateTensors() failed");
        return;
    }

    // Get pointers to input/output
    input = interpreter->input(0);
    output = interpreter->output(0);
    
    ESP_LOGI(TAG, "TinyML Model Loaded! Arena Used: %d bytes", interpreter->arena_used_bytes());
}

inference_result_t analyze_environment(float temp, float hum) {
    inference_result_t result;
    result.temp = temp;
    result.hum = hum;

    if (!interpreter) {
        strcpy(result.message, "AI Error");
        return result;
    }

    // 1. Load Input Data (Normalize if you did in Python)
    // Our Python script didn't normalize, so we pass raw values.
    input->data.f[0] = temp;
    input->data.f[1] = hum;

    // 2. Run Inference
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        ESP_LOGE(TAG, "Invoke failed");
        return result;
    }

    // 3. Read Outputs (Probabilities)
    float prob_norm = output->data.f[0];
    float prob_warn = output->data.f[1];
    float prob_crit = output->data.f[2];

    // 4. Determine State (Argmax)
    if (prob_crit > prob_warn && prob_crit > prob_norm) {
        result.state = STATE_CRITICAL;
        result.probability = prob_crit;
        strcpy(result.message, "AI: Critical");
    } else if (prob_warn > prob_norm) {
        result.state = STATE_WARNING;
        result.probability = prob_warn;
        strcpy(result.message, "AI: Warning");
    } else {
        result.state = STATE_NORMAL;
        result.probability = prob_norm;
        strcpy(result.message, "AI: Normal");
    }

    return result;
}