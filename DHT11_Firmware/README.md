# DHT11 Sensor Firmware

This directory contains the Arduino sketches for using the DHT11 sensor to detect temperature and report it via LED alerts and Telegram notifications.

## Directory Structure
- **/DHT11_Firmware/**  
  - `DHT11_Temperature_Monitor.ino`  
  - `Telegram_Notification.ino`
  - `LED_Alert.ino`

## Requirements
- Arduino IDE
- DHT sensor library
- TelegramBot library for Arduino
- Necessary hardware (DHT11 sensor, LED, etc.)  

## Usage
1. Install the necessary libraries in your Arduino IDE.
2. Upload `DHT11_Temperature_Monitor.ino` to your Arduino board.
3. Set up your LED and Telegram bot accordingly.