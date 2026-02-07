# 🌟 AURA PRO: Industrial IoT Environmental Monitor

> **A robust, full-stack IoT solution featuring Edge AI, Real-time Telemetry, and Self-Healing Firmware.**

![Project Status](https://img.shields.io/badge/Status-Operational-success)
![Tech Stack](https://img.shields.io/badge/Stack-ESP32%20%7C%20Node.js%20%7C%20React-blue)

## 📖 Executive Summary
Aura Pro is an end-to-end IoT system designed to monitor critical environmental conditions in real-time. Unlike standard Arduino prototypes, this system utilizes **FreeRTOS** for multitasking and implements **Edge AI** logic on the ESP32 to classify risk levels locally before transmission, reducing cloud latency and bandwidth.

---

## 🏗️ System Architecture

**1. Firmware Layer (ESP32 / C)**
* **Core:** ESP-IDF (Espressif IoT Development Framework).
* **OS:** FreeRTOS with independent tasks for Sensing, Networking, and System Health.
* **Intelligence:** Local decision tree algorithm classifies state (NORMAL/WARNING/CRITICAL).
* **Resilience:** Self-healing HTTP client with automatic WiFi reconnection logic.

**2. Backend Layer (Node.js)**
* **Security:** Input validation via `Joi` schemas.
* **Stability:** Rate limiting to prevent DDoS/flooding.
* **Architecture:** RESTful API with in-memory buffering for time-series data.

**3. Frontend Layer (React)**
* **UI/UX:** Dark-mode "Command Center" aesthetic.
* **Visualization:** Live, auto-updating SVG charts using `Recharts`.
* **Feedback:** Dynamic status badges driven by Edge AI scores.

---

## 🚀 Key Engineering Features

### 🧠 Edge Computing (The "Brain")
Instead of sending raw data blindly, the firmware analyzes temperature and humidity trends locally.
* **Risk Scoring:** Calculates a 0-100 risk score based on thermal thresholds.
* **Latency Reduction:** Critical alerts are flagged immediately at the source.

### 🛡️ Defensive Programming
* **Watchdog Monitors:** Dedicated tasks monitor stack depth and heap memory to prevent overflows.
* **Fail-Safe:** Sensor read errors trigger automatic retry mechanisms without stalling the network stack.

---

## 🛠️ Tech Stack & Hardware

| Component | Technology | Role |
| :--- | :--- | :--- |
| **MCU** | ESP32-WROOM-32 | Main Controller |
| **OS** | FreeRTOS | Task Scheduling |
| **Sensor** | DHT11 | Temperature/Humidity |
| **Protocol** | HTTP/JSON | Telemetry Transport |
| **Backend** | Node.js + Express | API Gateway |
| **Frontend** | React + Lucide | Data Visualization |

---

## 📸 Usage
1.  **Boot System:** ESP32 connects to WiFi and begins sampling (2s interval).
2.  **Monitor:** Dashboard displays live graph and "Device Health" stats (Free Heap).
3.  **Stress Test:** Breathing on the sensor triggers "WARNING" state on Dashboard immediately.

---

## 👨‍💻 Author
*Karthikeyan B*
*Embedded Systems & Full-Stack Developer*
*EDGE AI DEVELOPER*
