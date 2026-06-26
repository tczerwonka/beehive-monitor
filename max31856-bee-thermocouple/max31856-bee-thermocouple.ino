//------------------------------------------------------------------------------
// max31856-bee-thermocouple.ino
//  T Czerwonka 2026
// https://github.com/tczerwonka/beehive-monitor
// Arduino/ESP32 code to poll data every minute from four MAX31856 thermocouple
// devices placed in different bee hives and then report temps back via mqtt
//------------------------------------------------------------------------------
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Adafruit_MAX31856.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ==== MQTT CONFIG ====
const char* mqtt_server = "192.168.1.200";
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32c3/bees/sensors/2";

// ==== SPI PINS ====
#define SCK_PIN 4
#define MISO_PIN 5
#define MOSI_PIN 6
#define LED_PIN 8
#define VOLTAGE_PIN 1
#define ONE_WIRE_PIN 8

uint8_t csPins[4] = {7, 9, 10, 2};
const int NUM_SENSORS = 4;
const char* sensorNames[4] = {"A", "B", "C", "D"};

// Calibration
const float VOLTAGE_GAIN = 0.826;
const float VOLTAGE_OFFSET = 0.106;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_MAX31856* sensors[4];

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature oneWireSensors(&oneWire);

float readInputVoltage() {
  int samples = 12;
  long total = 0;

  for (int i = 0; i < samples; i++) {
    total += analogRead(VOLTAGE_PIN);
    delayMicroseconds(200);
  }

  float avg = total / (float)samples;
  float vout = (avg / 4095.0) * 3.3;
  float vin = vout * 6.0;
  return (vin * VOLTAGE_GAIN) + VOLTAGE_OFFSET;
}

float readExternalTempF() {
  oneWireSensors.requestTemperatures();
  float tempC = oneWireSensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) return NAN;
  return (tempC * 9.0 / 5.0) + 32.0;
}

void setupSensors() {
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);
  SPI.setFrequency(1000000);

  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(csPins[i], OUTPUT);
    digitalWrite(csPins[i], HIGH);
  }

  for (int i = 0; i < NUM_SENSORS; i++) {
    sensors[i] = new Adafruit_MAX31856(csPins[i]);

    if (!sensors[i]->begin()) {
      Serial.printf("Sensor %s not found!\n", sensorNames[i]);
      continue;
    }

    sensors[i]->setThermocoupleType(MAX31856_TCTYPE_K);
    sensors[i]->setConversionMode(MAX31856_CONTINUOUS);
  }

  delay(100);
}

void reconnectMQTT() {
  int attempts = 0;
  while (!client.connected() && attempts < 3) {
    Serial.print("Connecting MQTT...");
    if (client.connect("ESP32C3_MAX31856")) {
      Serial.println("connected");
      return;
    } else {
      Serial.print(" failed rc=");
      Serial.println(client.state());
      delay(1000);
    }
    attempts++;
  }
}

float cToF(float c) { return (c * 9.0 / 5.0) + 32.0; }

void publishTemperatures() {
  digitalWrite(LED_PIN, LOW);

  StaticJsonDocument<512> doc;

  Serial.println("==== Temperature Readings (F) ====");

  for (int i = 0; i < NUM_SENSORS; i++) {
    float tempC = sensors[i]->readThermocoupleTemperature();

    if (isnan(tempC)) {
      Serial.printf("Sensor %s: ERROR\n", sensorNames[i]);
      doc[sensorNames[i]] = nullptr;
      continue;
    }

    float tempF = cToF(tempC);
    doc[sensorNames[i]] = tempF;
    Serial.printf("Sensor %s: %.2f F\n", sensorNames[i], tempF);
  }

  float extTemp = readExternalTempF();
  if (isnan(extTemp)) {
    Serial.println("External sensor ERROR");
    doc["external"] = nullptr;
  } else {
    Serial.printf("External Temp: %.2f F\n", extTemp);
    doc["external"] = extTemp;
  }

  float vin = readInputVoltage();
  Serial.printf("Input Voltage: %.4f V\n", vin);
  doc["Vin"] = vin;

  // ==== RSSI ====
  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    Serial.printf("WiFi RSSI: %d dBm\n", rssi);
    doc["RSSI"] = rssi;
  } else {
    Serial.println("WiFi not connected");
    doc["RSSI"] = nullptr;
  }

  char buffer[512];
  serializeJson(doc, buffer);

  if (client.connected()) {
    client.publish(mqtt_topic, buffer);
  } else {
    Serial.println("MQTT not connected, skipping publish");
  }

  Serial.print("Published JSON: ");
  Serial.println(buffer);
  Serial.println("=================================");

  digitalWrite(LED_PIN, HIGH);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  Serial.printf("Wake reason: %d\n", reason);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  WiFiManager wm;
  wm.autoConnect("ESP32C3-Setup");

  client.setServer(mqtt_server, mqtt_port);

  oneWireSensors.begin();
  setupSensors();
}

void loop() {
  if (!client.connected()) reconnectMQTT();

  client.loop();

  publishTemperatures();

  delay(250);

  Serial.println("Entering deep sleep...\n");

  esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL);
  delay(100);
  esp_deep_sleep_start();
}
