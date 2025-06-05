#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();

// Configuration - CHANGE THESE VALUES!
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASSWORD";
const char* weatherAPI = "OPENWEATHER_API_KEY";
const char* city = "Tacoma";
const float latitude = 47.25288;  // Change to your city's latitude
const float longitude = -122.44429; // Change to your city's longitude

// Weather data structure
struct WeatherData {
  float currentTemp;
  float highTemp;
  float lowTemp;
  int humidity;
  float windSpeed;
  float uvIndex;
  int aqi;
  String description;
  String sunrise;
  String sunset;
};

WeatherData weather;

void setup() {
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  connectToWiFi();
  updateWeatherData();
  displayWeather();
}

void loop() {
  delay(600000); // Update every 10 minutes
  updateWeatherData();
  displayWeather();
}

void connectToWiFi() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Connecting to WiFi...", 10, 10, 2);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    Serial.print(".");
    attempts++;
    
    tft.fillRect(10, 40, 300, 20, TFT_BLACK);
    for (int i = 0; i < attempts; i++) {
      tft.fillCircle(20 + (i * 10), 50, 3, TFT_BLUE);
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    tft.fillScreen(TFT_BLACK);
    tft.drawString("WiFi Connected!", 10, 10, 2);
    delay(2000);
  } else {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("WiFi Failed!", 10, 10, 2);
    while(1);
  }
}

void updateWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    getWeatherData();
    getUVData();
    getAQIData();
  }
}

void getWeatherData() {
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + 
               String(city) + "&appid=" + String(weatherAPI) + "&units=metric";
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);
    
    weather.currentTemp = doc["main"]["temp"];
    weather.highTemp = doc["main"]["temp_max"];
    weather.lowTemp = doc["main"]["temp_min"];
    weather.humidity = doc["main"]["humidity"];
    weather.windSpeed = doc["wind"]["speed"];
    weather.description = doc["weather"][0]["description"].as<String>();
    
    long sunrise = doc["sys"]["sunrise"];
    long sunset = doc["sys"]["sunset"];
    weather.sunrise = formatTime(sunrise);
    weather.sunset = formatTime(sunset);
    
    Serial.println("Weather data updated");
  }
  
  http.end();
}

void getUVData() {
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/uvi?lat=" + 
               String(latitude) + "&lon=" + String(longitude) + "&appid=" + String(weatherAPI);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(512);
    deserializeJson(doc, payload);
    
    weather.uvIndex = doc["value"];
    Serial.println("UV data updated");
  }
  
  http.end();
}

void getAQIData() {
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/air_pollution?lat=" + 
               String(latitude) + "&lon=" + String(longitude) + "&appid=" + String(weatherAPI);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    
    int aqiLevel = doc["list"][0]["main"]["aqi"];
    weather.aqi = convertToUSAQI(aqiLevel);
    
    Serial.println("AQI data updated");
  } else {
    weather.aqi = -1;
  }
  
  http.end();
}

int convertToUSAQI(int owmAqi) {
  switch(owmAqi) {
    case 1: return 25;   // Good
    case 2: return 75;   // Fair
    case 3: return 125;  // Moderate
    case 4: return 175;  // Poor
    case 5: return 275;  // Very Poor
    default: return -1;
  }
}

String formatTime(long timestamp) {
  timestamp += 3600; // Add 1 hour for timezone (adjust as needed)
  int hours = (timestamp % 86400L) / 3600;
  int minutes = (timestamp % 3600) / 60;
  
  return String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);
}

void displayWeather() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Title
  tft.drawString(String(city) + " Weather", 10, 5, 2);
  
  // Current temperature
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(String(weather.currentTemp, 1) + "°C", 10, 30, 4);
  
  // High/Low
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("H:" + String(weather.highTemp, 0) + "° L:" + String(weather.lowTemp, 0) + "°", 120, 45, 2);
  
  // Description
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(weather.description, 10, 70, 2);
  
  // Details
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Humidity: " + String(weather.humidity) + "%", 10, 95, 1);
  tft.drawString("Wind: " + String(weather.windSpeed, 1) + " m/s", 150, 95, 1);
  
  tft.drawString("UV Index: " + String(weather.uvIndex, 1), 10, 110, 1);
  if (weather.aqi > 0) {
    tft.drawString("AQI: " + String(weather.aqi) + " " + getAQIStatus(weather.aqi), 10, 125, 1);
  }
  
  tft.drawString("Sunrise: " + weather.sunrise, 10, 140, 1);
  tft.drawString("Sunset: " + weather.sunset, 150, 140, 1);
  
  // Status
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Last update: " + String(millis()/60000) + "m ago", 10, 155, 1);
}

String getAQIStatus(int aqi) {
  if (aqi <= 50) return "Good";
  else if (aqi <= 100) return "Fair";
  else if (aqi <= 150) return "Moderate";
  else if (aqi <= 200) return "Poor";
  else return "V.Poor";
}