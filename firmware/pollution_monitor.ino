# Pollution Monitor

This Arduino code interface the DHT11 sensor to monitor temperature and humidity levels, activates LED alerts, and sends notifications through Telegram.

## Includes
- DHT11 Sensor for temperature/humidity measurements
- LED for visual alerts
- Telegram bot for notifications

## Connections
- DHT11 Sensor connected to GPIO pin 4
- LED connected to GPIO pin 5

## Required Libraries
```cpp
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
```

## WiFi and Telegram Bot Credentials
```cpp
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";
const char* telegramToken = "YOUR_BOT_TOKEN";
const char* chatID = "YOUR_CHAT_ID";
```

## Main Setup
```cpp
DHT dht(4, DHT11);

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(5, OUTPUT);
  WiFi.begin(ssid, password);

  // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}
```

## Loop Execution
```cpp
void loop() {
  delay(2000);

  // Read temperature and humidity
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Spike Detection Logic
  if (t > 30.0 || h > 70.0) {
    digitalWrite(5, HIGH); // Turn on LED alert
    String message = "Alert: High temperature of " + String(t) + "°C and humidity " + String(h) + "% detected!";
    bot.sendMessage(chatID, message, "");
  } else {
    digitalWrite(5, LOW); // Turn off LED alert
  }
}
```

## Backend Logging
// Add your logging to backend functionality here.
