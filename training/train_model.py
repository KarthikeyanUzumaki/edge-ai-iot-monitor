import numpy as np
import tensorflow as tf
import pandas as pd
import math

# 1. GENERATE SYNTHETIC DATASET
# We simulate 2000 random readings mimicking your DHT11
samples = 2000
temp = np.random.uniform(15, 45, samples)  # Temp between 15C and 45C
hum = np.random.uniform(20, 95, samples)   # Hum between 20% and 95%
labels = []

for t, h in zip(temp, hum):
    # The Logic (Ground Truth)
    # 0 = Normal, 1 = Warning, 2 = Critical
    if t > 36.0 or h > 90.0:
        labels.append(2) # Critical
    elif t > 34.0 or h > 75.0:
        labels.append(1) # Warning
    else:
        labels.append(0) # Normal

# Convert to Numpy Arrays
inputs = np.column_stack((temp, hum))
outputs = np.array(labels)

print(f"✅ Generated {samples} samples.")

# 2. BUILD THE MODEL
# A simple Neural Network: 2 Inputs -> 8 Neurons -> 3 Outputs (Softmax)
model = tf.keras.Sequential([
    tf.keras.layers.Dense(8, activation='relu', input_shape=(2,)),
    tf.keras.layers.Dense(8, activation='relu'),
    tf.keras.layers.Dense(3, activation='softmax') # 3 Classes: Norm, Warn, Crit
])

model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])

# 3. TRAIN
print("🧠 Training Model...")
model.fit(inputs, outputs, epochs=50, batch_size=32, verbose=0)
print("✅ Training Complete.")

# 4. CONVERT TO TENSORFLOW LITE (TinyML)
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Save TFLite file
with open("model.tflite", "wb") as f:
    f.write(tflite_model)

# 5. CONVERT TO C HEADER FILE (Hex Dump)
# This creates the byte array we need for ESP32
def hex_to_c_array(data, var_name):
    c_str = ''
    c_str += '#include <cstdint>\n\n'
    c_str += f'alignas(16) const unsigned char {var_name}[] = {{\n'
    for i, val in enumerate(data):
        c_str += f'0x{val:02x}, '
        if (i + 1) % 12 == 0:
            c_str += '\n'
    c_str += '\n};\n'
    c_str += f'const unsigned int {var_name}_len = {len(data)};\n'
    return c_str

with open("model_data.h", "w") as f:
    f.write(hex_to_c_array(tflite_model, "g_model"))

print("🎉 SUCCESS! 'model_data.h' generated. Copy this to your firmware/main folder.")