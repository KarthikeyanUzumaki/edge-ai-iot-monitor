import tensorflow as tf
import numpy as np
import os

# 1. Define a simple model
model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu', input_shape=(2,)),
    tf.keras.layers.Dense(3, activation='softmax')
])

# 2. Convert to TFLite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# 3. Generate C Header
hex_array = ', '.join([f'0x{b:02x}' for b in tflite_model])
header_content = f"""
#include <cstdint>
// Compatible TFLite Model
alignas(16) const unsigned char g_model[] = {{ {hex_array} }};
const unsigned int g_model_len = {len(tflite_model)};
"""

# 4. Save to "main/model_data.h" (Correct Path)
file_path = os.path.join("main", "model_data.h")
with open(file_path, "w") as f:
    f.write(header_content)

print(f"✅ SUCCESS: Brain saved to {file_path}")