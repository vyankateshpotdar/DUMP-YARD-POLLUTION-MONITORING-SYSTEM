#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ============== PIN CONFIGURATION ==============
#define DHTPIN 4          // GPIO pin connected to DHT11 data pin
#define DHTTYPE DHT11     // DHT 11 sensor type
#define LED_PIN 5         // GPIO pin for LED alert
#define BUZZER_PIN 6      // GPIO pin for buzzer alert

// ============== WIFI CONFIGURATION ==============
const char* ssid = "YOUR_SSID";           // Replace with your WiFi SSID
const char* password = "YOUR_PASSWORD";   // Replace with your WiFi password

// ============== TELEGRAM CONFIGURATION ==============
const char* telegramToken = "YOUR_BOT_TOKEN";    // Replace with your Telegram bot token
const char* telegramChatID = "YOUR_CHAT_ID";    // Replace with your Telegram chat ID

// ============== BACKEND SERVER CONFIGURATION ==============
const char* backendServer = "http://YOUR_SERVER_IP:3000";  // Replace with your server IP

// ============== SENSOR SETUP ==============
DHT dht(DHTPIN, DHTTYPE);

// ============== DETECTION THRESHOLDS ==============
float TEMP_SPIKE_THRESHOLD = 5.0;         // Temperature spike in °C
float HUMIDITY_SPIKE_THRESHOLD = 15.0;    // Humidity spike in %
float TEMP_ALERT_THRESHOLD = 35.0;        // High temperature threshold
float HUMIDITY_ALERT_THRESHOLD = 75.0;    // High humidity threshold

// ============== VARIABLES ==============
float previousTemp = 0;
float previousHumidity = 0;
bool alertActive = false;
unsigned long lastReadTime = 0;
unsigned long alertStartTime = 0;
const unsigned long READ_INTERVAL = 30000;  // Read every 30 seconds
const unsigned long ALERT_DURATION = 300000; // Alert duration 5 minutes

// ============== SETUP FUNCTION ==============
void setup() {
  Serial.begin(9600);
  delay(1000);
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Initial sensor reading
  delay(2000);
  readSensor();
  
  Serial.println("========================================");
  Serial.println("DUMP YARD POLLUTION MONITOR STARTED!");
  Serial.println("========================================");
  Serial.println();
}

// ============== MAIN LOOP ==============
void loop() {
  // Read sensor every READ_INTERVAL milliseconds
  if (millis() - lastReadTime >= READ_INTERVAL) {
    readSensor();
    detectPollution();
    lastReadTime = millis();
  }
  
  // Deactivate alert after ALERT_DURATION
  if (alertActive && (millis() - alertStartTime > ALERT_DURATION)) {
    deactivateAlert();
  }
  
  delay(100);
}

// ============== SENSOR READING FUNCTION ==============
void readSensor() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Check for sensor read failure
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("❌ ERROR: Failed to read from DHT sensor!");
    return;
  }
  
  // Display readings
  Serial.print("📊 Sensor Reading: ");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C | Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  
  // Store for comparison
  previousTemp = temperature;
  previousHumidity = humidity;
}

// ============== POLLUTION DETECTION FUNCTION ==============
void detectPollution() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Validate sensor reading
  if (isnan(humidity) || isnan(temperature)) {
    return;
  }
  
  // Calculate differences from previous reading
  float tempDifference = temperature - previousTemp;
  float humidityDifference = humidity - previousHumidity;
  
  Serial.print("📈 Changes: ");
  Serial.print("Temp Δ: ");
  Serial.print(tempDifference);
  Serial.print("°C | Humidity Δ: ");
  Serial.print(humidityDifference);
  Serial.println("%");
  
  // Detection Logic: BOTH temperature AND humidity spikes required
  // This prevents false alarms from single sensor fluctuations
  if (tempDifference >= TEMP_SPIKE_THRESHOLD && 
      humidityDifference >= HUMIDITY_SPIKE_THRESHOLD) {
    
    Serial.println("🔴 🔴 🔴 POLLUTION DETECTED! 🔴 🔴 🔴");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("°C | Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    
    if (!alertActive) {
      activateAlert(temperature, humidity);
    }
  }
  else {
    Serial.println("✅ Air Quality: NORMAL");
  }
  
  Serial.println();
}

// ============== ALERT ACTIVATION FUNCTION ==============
void activateAlert(float temp, float humidity) {
  alertActive = true;
  alertStartTime = millis();
  
  // Activate hardware alerts
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  
  Serial.println("⚠️  ⚠️  ⚠️  ALERT ACTIVATED! ⚠️  ⚠️  ⚠️");
  Serial.println("LED: ON");
  Serial.println("BUZZER: ON");
  
  // Send notifications
  sendTelegramAlert(temp, humidity);
  sendToBackend(temp, humidity);
}

// ============== ALERT DEACTIVATION FUNCTION ==============
void deactivateAlert() {
  alertActive = false;
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("✅ Alert cleared - Hardware reset");
  Serial.println();
}

// ============== TELEGRAM NOTIFICATION FUNCTION ==============
void sendTelegramAlert(float temp, float humidity) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected - Cannot send Telegram alert");
    return;
  }
  
  HTTPClient http;
  
  // Create alert message
  String message = "🔴 DUMP YARD POLLUTION DETECTED!%0A";
  message += "Temperature: " + String(temp, 1) + "°C%0A";
  message += "Humidity: " + String(humidity, 1) + "%25%0A";
  message += "Time: " + String(millis() / 1000) + "s%0A";
  message += "Action: Check hostel area immediately!";
  
  // Construct Telegram API URL
  String url = "https://api.telegram.org/bot" + String(telegramToken) + 
               "/sendMessage?chat_id=" + String(telegramChatID) + 
               "&text=" + message;
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.println("✅ Telegram notification sent successfully!");
  } else {
    Serial.println("❌ Failed to send Telegram notification");
    Serial.print("HTTP Error Code: ");
    Serial.println(httpCode);
  }
  
  http.end();
}

// ============== BACKEND LOGGING FUNCTION ==============
void sendToBackend(float temp, float humidity) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected - Cannot send to backend");
    return;
  }
  
  HTTPClient http;
  String url = String(backendServer) + "/api/pollution/log";
  
  // Create JSON payload
  String jsonData = "{";
  jsonData += "\"temperature\":" + String(temp, 2) + ",";
  jsonData += "\"humidity\":" + String(humidity, 2);
  jsonData += "}";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(jsonData);
  
  if (httpCode > 0) {
    Serial.println("✅ Data sent to backend successfully!");
    String response = http.getString();
    Serial.print("Response: ");
    Serial.println(response);
  } else {
    Serial.println("❌ Failed to send data to backend");
    Serial.print("HTTP Error Code: ");
    Serial.println(httpCode);
  }
  
  http.end();
}

// ============== WIFI CONNECTION FUNCTION ==============
void connectToWiFi() {
  Serial.println("\n🔌 Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi Connected Successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("❌ Failed to connect to WiFi");
    Serial.println("⚠️  System will continue with local monitoring only");
  }
  
  Serial.println();
}

// ============== END OF CODE ==============
// Author: College Project - Pollution Monitoring
// Date: 2026
// Status: Ready for Deployment
