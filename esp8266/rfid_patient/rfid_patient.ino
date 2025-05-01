#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Chirag";
const char* password = "chirag88";

// OpenWeatherMap and Telegram details
const String API_KEY = "04ab113cbe484dd30cd4ed17241e86b7";
const String CITY = "thane,IN";
const String WEATHER_API_URL = "http://api.openweathermap.org/data/2.5/forecast?q=" + CITY + "&appid=" + API_KEY + "&units=metric";

const String TELEGRAM_BOT_TOKEN = "7860231589:AAGhQM74WZkmCHKPvj56hfOtjBuNY3twu3U";
const String CHAT_ID = "5186811689";
const String TELEGRAM_URL = "https://api.telegram.org/bot" + TELEGRAM_BOT_TOKEN + "/sendMessage";

// Sensor and LED pins
const int IR_SENSOR_PIN_1 = D5;
const int IR_SENSOR_PIN_2 = D6;
const int LED_PIN = D7;

unsigned long lastCheck = 0;
const unsigned long checkInterval = 15000; // 15 seconds

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(IR_SENSOR_PIN_1, INPUT);
  pinMode(IR_SENSOR_PIN_2, INPUT);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void loop() {
  bool insideMotion = digitalRead(IR_SENSOR_PIN_1) == LOW;
  bool outsideMotion = digitalRead(IR_SENSOR_PIN_2) == LOW;

  if (insideMotion || outsideMotion) {
    digitalWrite(LED_PIN, HIGH);
    checkWeatherAndNotify();
    digitalWrite(LED_PIN, LOW);
  }

  if (millis() - lastCheck > checkInterval) {
    lastCheck = millis();
  }
}

void checkWeatherAndNotify() {
  Serial.println("Checking weather...");

  if (WiFi.status() == WL_CONNECTED) {
    // HTTP for weather API
    HTTPClient weatherHttp;
    WiFiClient weatherClient;

    weatherHttp.begin(weatherClient, WEATHER_API_URL);
    int httpCode = weatherHttp.GET();

    String telegramMessage = "🚶 Motion detected!\n";

    if (httpCode == HTTP_CODE_OK) {
      Serial.println("Weather data received.");
      DynamicJsonDocument doc(16 * 1024);
      DeserializationError error = deserializeJson(doc, weatherHttp.getStream());

      if (error) {
        Serial.print("JSON Parsing Error: ");
        Serial.println(error.c_str());
        telegramMessage += "Weather: Failed to get weather info.";
      } else {
        bool rainExpected = false;
        int count = 0;

        for (JsonObject forecast : doc["list"].as<JsonArray>()) {
          const char* dt_txt = forecast["dt_txt"];
          float rainVolume = forecast["rain"]["3h"] | 0.0;

          if (rainVolume > 0.0) {
            telegramMessage += "🌧️ Rain expected at ";
            telegramMessage += dt_txt;
            telegramMessage += ". Don't forget your umbrella!";
            rainExpected = true;
            break;
          }

          count++;
          if (count >= 5) break;
        }

        if (!rainExpected) {
          telegramMessage += "☀️ No rain expected in the next few hours.";
        }
      }

    } else {
      Serial.println("Failed to fetch weather data. Code: " + String(httpCode));
      telegramMessage += "Weather: Could not retrieve data.";
    }

    weatherHttp.end(); // Important to release socket
    delay(500); // Allow time between connections

    // Send Telegram message via HTTPS
    WiFiClientSecure telegramClient;
    telegramClient.setInsecure();

    HTTPClient telegramHttp;
    String url = TELEGRAM_URL + "?chat_id=" + CHAT_ID + "&text=" + urlencode(telegramMessage);

    telegramHttp.begin(telegramClient, url);
    int telegramCode = telegramHttp.GET();

    if (telegramCode > 0) {
      Serial.println("Telegram message sent.");
      String response = telegramHttp.getString();
      Serial.println("Telegram Response: " + response);
    } else {
      Serial.println("Telegram send failed. Error code: " + String(telegramCode));
    }

    telegramHttp.end();

  } else {
    Serial.println("WiFi not connected.");
  }
}

// Simple URL encoder (Telegram requires properly encoded URLs)
String urlencode(String str) {
  String encoded = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      code0 = ((c >> 4) & 0xf) + '0';
      if (((c >> 4) & 0xf) > 9) code0 = ((c >> 4) & 0xf) - 10 + 'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}
