# ESP32 Weather Display 🌤️

A professional-grade weather monitoring system using LILYGO T-Display-S3 ESP32-S3 with OLED display integration.

## Features
- 🌡️ Current, high, and low temperatures
- 💧 Humidity and wind speed
- ☀️ UV Index and Air Quality Index
- 🌅 Sunrise and sunset times
- 🔄 Auto-refresh every 10 minutes

## Hardware
- LILYGO T-Display-S3 ESP32-S3
- 1.91" RM67162 AMOLED Display

## Setup
1. Install Arduino IDE and ESP32 board support
2. Install libraries: TFT_eSPI, ArduinoJson
3. Get free API key from OpenWeatherMap
4. Update WiFi credentials and API key in code
5. Upload to ESP32

## Configuration
Update these lines in the code:
```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* weatherAPI = "YOUR_OPENWEATHER_API_KEY";
const char* city = "YOUR_CITY";