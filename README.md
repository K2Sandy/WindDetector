# Smart Wind Detector: IoT Early Warning System 🌪️📡

> **🥇 Awarded the Gold Medal at the International Indonesia Inventors Day 2024.**

The Smart Wind Detector is an IoT-based environmental monitoring and early warning system. Designed to mitigate risks associated with extreme weather, the system accurately measures real-time wind speed and direction, providing immediate local feedback and remote notifications when dangerous thresholds are met.

This project demonstrates comprehensive hardware and software integration, transforming standard sensor components into a fully functional, automated disaster-mitigation tool.

## ✨ Key Features
* **Real-Time Monitoring:** Continuously tracks and calculates wind speed and direction metrics using physical anemometer sensors.
* **Automated SMS Alerts:** Integrates a GSM SIM800L module to instantly dispatch emergency text messages to users when wind speeds exceed predefined safety limits.
* **Local Visual & Audio Feedback:** Displays live weather data on an LCD screen and triggers high-intensity LEDs alongside a siren during critical wind events.
* **Complex Systems Integration:** Seamlessly bridges modified base-sensor logic with multiple output peripherals (GSM, Displays, Audio) to create a standalone warning unit.

## 🛠️ Tech Stack & Hardware
* **Language:** C/C++
* **Microcontroller:** Arduino (Compatible Boards)
* **Communication:** GSM SIM800L Module
* **Sensors & Outputs:** Wind Speed/Direction Sensor, LCD Display, LEDs, Siren
* **IDE:** Arduino IDE

## 🚀 Getting Started

To flash this code and run the system on your hardware:

1. Clone this repository:
   `git clone https://github.com/K2Sandy/Smart-Wind-Detector.git`
2. Open the `.ino` project file using the **Arduino IDE**.
3. Ensure you have the necessary libraries installed via the Library Manager (e.g., `LiquidCrystal` for the display, and `SoftwareSerial` for the GSM module).
4. Wire your hardware components according to the defined pinouts in the code. *(Make sure the SIM800L module has a stable power supply and an active SIM card).*
5. Connect your Arduino board via USB, select the correct Port and Board in the IDE.
6. Click **Upload** to flash the logic into the microcontroller.

## 🤝 Acknowledgments
* The core sensor-reading logic was adapted from the original wind sensor manufacturer's code. Our team significantly modified and expanded this foundation to integrate the GSM module, LCD, and warning peripherals, transforming it into a complete early warning system.
* Proudly developed to represent our team at the International Indonesia Inventors Day.
