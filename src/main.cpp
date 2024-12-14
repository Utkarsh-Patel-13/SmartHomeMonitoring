#include "DHT20.h"
#include <Servo.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <Adafruit_CAP1188.h>

// Pins
#define LED_PIN 13
#define LDR_PIN 32
#define SERVO_PIN 26
#define BUTTON_1 0
#define BUTTON_2 35
#define CAP_RESET 27

// Thresholds
float TEMP_THRESHOLD = 23.0;
int MOISTURE_THRESHOLD = 30;
int LDR_THRESHOLD = 1200;

// Operation Modes
enum OperationMode
{
  MODE_OFF,
  MODE_MANUAL,
  MODE_AUTO
};

// Sensors
DHT20 DHT;
Servo servo;
OperationMode currentMode = MODE_AUTO;
TFT_eSPI tft = TFT_eSPI();
Adafruit_CAP1188 capSensor = Adafruit_CAP1188(CAP_RESET);

// System States
int servoPosition = 0;
bool isRotating = false;
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_READ_INTERVAL = 5000;
unsigned long lastSettingsCheck = 0;
const unsigned long SETTINGS_CHECK_INTERVAL = 8000;
bool showThresholds = false;
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_TIME = 200;
int touchedPins = 0;

// WiFi credentials
const char *ssid = "SETUP-3CDC";
const char *password = "effect5824being";

// API endpoints
const char *serverUrl = "";
const char *settingsUrl = "";
const char *modeUrl = "";

void setup()
{
  Serial.begin(9600);

  // Display
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);

  // Initialize pins and buttons
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);

  // I2C devices
  Wire.begin();
  DHT.begin();

  // Servo
  servo.attach(SERVO_PIN);
  servo.write(0);

  delay(1000);
  Serial.println("Smart Home Monitor initialized");

  // WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize capacitive sensor
  if (!capSensor.begin())
  {
    Serial.println("Capacitive sensor not found");
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 220);
    tft.print("CAP Sensor Error!");
  }
}

/**
 * Handles automatic lighting control based on ambient light levels
 * Turns LED on when light level falls below threshold, off otherwise
 */
void handleLighting()
{
  int currentLight = analogRead(LDR_PIN);
  digitalWrite(LED_PIN, currentLight < LDR_THRESHOLD ? HIGH : LOW);
}

/**
 * Controls servo motor for temperature regulation
 * Activates fan (servo) when temperature exceeds threshold
 * @param temperature Current temperature reading
 */
void handleTemperatureControl(float temperature)
{
  if (temperature > TEMP_THRESHOLD)
  {
    if (!isRotating)
    {
      isRotating = true;
    }
  }
  else if (temperature <= TEMP_THRESHOLD)
  {
    isRotating = false;
    servo.writeMicroseconds(1500);
    return;
  }

  if (isRotating)
  {
    servo.writeMicroseconds(2000);
  }
}

/**
 * Sends sensor data to remote server via HTTP POST
 * @param temperature Current temperature reading
 * @param moisture Current humidity reading
 * @param lightLevel Current ambient light level
 */
void sendSensorData(float temperature, float moisture, int lightLevel)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    // Simplified JSON payload without redundant soil_moisture field
    String payload = "{\"temperature\":" + String(temperature) +
                     ",\"humidity\":" + String(moisture) +
                     ",\"light_level\":" + String(lightLevel) + "}";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

/**
 * Updates the TFT display with current sensor readings or threshold values
 * Toggles between two display modes: current readings and threshold settings
 * @param temperature Current temperature reading
 * @param humidity Current humidity reading
 * @param lightLevel Current ambient light level
 */
void updateDisplay(float temperature, float humidity, int lightLevel)
{
  tft.fillScreen(TFT_BLACK);

  if (!showThresholds)
  {
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(5, 5);
    tft.print("Current Values");

    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5, 30);
    tft.print("Mode:");
    tft.setCursor(5, 50);
    switch (currentMode)
    {
    case MODE_AUTO:
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.print("AUTO");
      break;
    case MODE_MANUAL:
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.print("MANUAL");
      break;
    case MODE_OFF:
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.print("OFF");
      break;
    }

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5, 80);
    tft.print("Temp:");
    tft.setCursor(5, 100);
    tft.print(temperature, 1);
    tft.print("C");

    tft.setCursor(5, 130);
    tft.print("Moist:");
    tft.setCursor(5, 150);
    tft.print(humidity);
    tft.print("%");

    tft.setCursor(5, 180);
    tft.print("Light:");
    tft.setCursor(5, 200);
    tft.print(lightLevel);

    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 220);
    tft.print("Touch: ");
    tft.print(touchedPins);
    tft.print(" pins");

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 220);
    tft.print("Touch: 1=Auto 2=Manual 3=Off");
  }
  else
  {
    tft.setTextColor(TFT_BLACK, TFT_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(5, 5);
    tft.print("Thresholds");

    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.setTextSize(2);

    tft.setCursor(5, 40);
    tft.print("Temp:");
    tft.setCursor(5, 60);
    tft.print(TEMP_THRESHOLD, 1);
    tft.print("C");

    tft.setCursor(5, 100);
    tft.print("Moist:");
    tft.setCursor(5, 120);
    tft.print(MOISTURE_THRESHOLD);
    tft.print("%");

    tft.setCursor(5, 160);
    tft.print("Light:");
    tft.setCursor(5, 180);
    tft.print(LDR_THRESHOLD);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(5, 230);
  tft.print("Press BTN to toggle view");
}

/**
 * Sends current operation mode to server
 * Converts enum mode to string representation for API
 * @param mode Current operation mode (AUTO/MANUAL/OFF)
 */
void updateModeOnServer(OperationMode mode)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(modeUrl);
    http.addHeader("Content-Type", "application/json");

    String modeStr;
    switch (mode)
    {
    case MODE_AUTO:
      modeStr = "AUTO";
      break;
    case MODE_MANUAL:
      modeStr = "MANUAL";
      break;
    case MODE_OFF:
      modeStr = "OFF";
      break;
    }

    String payload = "{\"mode\":\"" + modeStr + "\"}";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0)
    {
      Serial.print("Mode update HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Mode update Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}

/**
 * Reads capacitive touch sensor input
 * Changes system operation mode based on number of pins touched:
 * 1 pin = AUTO, 2 pins = MANUAL, 3 pins = OFF
 */
void readCapacitiveSensor()
{
  touchedPins = 0;
  uint8_t touched = capSensor.touched();

  if (touched)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      if (touched & (1 << i))
      {
        touchedPins++;
      }
    }

    if (touchedPins >= 1 && touchedPins <= 3)
    {
      OperationMode newMode;
      switch (touchedPins)
      {
      case 1:
        newMode = MODE_AUTO;
        break;
      case 2:
        newMode = MODE_MANUAL;
        break;
      case 3:
        newMode = MODE_OFF;
        break;
      default:
        return;
      }

      if (newMode != currentMode)
      {
        currentMode = newMode;
        updateModeOnServer(currentMode);
      }

      delay(500);
    }
  }
}

/**
 * Main sensor reading function
 * Handles button input for display toggle
 * Reads all sensors at specified intervals
 * Updates display and sends data to server
 */
void readAndDisplaySensors()
{
  if ((millis() - lastButtonPress > DEBOUNCE_TIME) &&
      (digitalRead(BUTTON_1) == LOW || digitalRead(BUTTON_2) == LOW))
  {
    showThresholds = !showThresholds;
    lastButtonPress = millis();
  }

  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL)
  {
    int status = DHT.read();
    float temperature = DHT.getTemperature();
    float humidity = DHT.getHumidity();
    int lightLevel = analogRead(LDR_PIN);

    readCapacitiveSensor();

    updateDisplay(temperature, humidity, lightLevel);

    Serial.println("=== Sensor Readings ===");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Soil Moisture: ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.print("Light Level: ");
    Serial.println(lightLevel);
    Serial.println("====================");

    sendSensorData(temperature, humidity, lightLevel);

    lastSensorRead = millis();
  }
}

/**
 * Fetches and updates system settings from remote server
 * Updates thresholds and operation mode based on server response
 * Runs at specified intervals defined by SETTINGS_CHECK_INTERVAL
 */
void updateSystemSettings()
{
  if (millis() - lastSettingsCheck >= SETTINGS_CHECK_INTERVAL)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;
      http.begin(settingsUrl);

      int httpResponseCode = http.GET();

      if (httpResponseCode > 0)
      {
        String payload = http.getString();

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
          TEMP_THRESHOLD = doc["temp_threshold"];
          MOISTURE_THRESHOLD = doc["moisture_threshold"];
          LDR_THRESHOLD = doc["light_threshold"];

          String modeStr = doc["operation_mode"].as<String>();
          if (modeStr == "AUTO")
          {
            currentMode = MODE_AUTO;
          }
          else if (modeStr == "MANUAL")
          {
            currentMode = MODE_MANUAL;
          }
          else if (modeStr == "OFF")
          {
            currentMode = MODE_OFF;
          }

          Serial.println("Settings updated successfully");
        }
      }
      http.end();
    }
    lastSettingsCheck = millis();
  }
}

void loop()
{
  updateSystemSettings();

  readAndDisplaySensors();

  switch (currentMode)
  {
  case MODE_AUTO:
    handleLighting();
    if (millis() - DHT.lastRead() >= SENSOR_READ_INTERVAL)
    {
      DHT.read();
      handleTemperatureControl(DHT.getTemperature());
    }
    break;

  case MODE_MANUAL:
    break;

  case MODE_OFF:
    digitalWrite(LED_PIN, LOW);
    servo.writeMicroseconds(1500);
    isRotating = false;
    break;
  }

  delay(20);
}