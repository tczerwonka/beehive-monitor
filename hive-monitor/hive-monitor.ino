///////////////////////////////////////////////////////////////////////////////
// hive-monitor.ino 
//  ESP8266 on NodeMCU board to report temperature from a thermocouple
//  and 1wire temp sensor via MQTT to a server
//  Voltage monitor on input pin.
//  Ultimate use is to monitor bee hive temperature from an embedded K-type
//  thermocouple probe along with external temperature.  Load cells can be
//  connected to the unit at a future date.
///////////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#include "myconfig.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <PubSubClient.h>
//use esp8266 2.7.3 version or suffer the errors
//board mgr url: http://arduino.esp8266.com/stable/package_esp8266com_index.json


/******************************************************************************/
//HX711 loadcell
//using https://github.com/RobTillaart/HX711
//use scale.set_offset(4294465165); and scale.set_scale(-21.719906);
//using an xlr connector because I have them
//shield - ground
//2 - vcc
//1 SCK / green
//3 DT / yellow
#include <HX711.h>
HX711 scale;
uint8_t dataPin  = 9; //SD2 / yellow / SCK
uint8_t clockPin = 10; //SD3 / green / DT



#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include "Adafruit_MCP9600.h"

/******************************************************************************/
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/******************************************************************************/
#define I2C_ADDRESS (0x67)
Adafruit_MCP9600 mcp;

/******************************************************************************/
volatile unsigned long i = 0;
char charBuffer[32];

WiFiClient espClient;
PubSubClient client(espClient);

#define MOSFET_PIN 12

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(MOSFET_PIN, OUTPUT);



  // We start by connecting to a WiFi network
  if (debugOutput) {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int maxWait = 500;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (debugOutput) Serial.print(".");
    if (maxWait <= 0)
      ESP.restart();
    maxWait--;
  }
  if (debugOutput) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  delay(500);
  do_update();
  client.setServer(mqtt_host, mqtt_port);
  reconnect();

  //enable power for MCP9600
  digitalWrite(MOSFET_PIN, HIGH);

  //1wire
  sensors.begin();

  //load cell setup
  scale.begin(dataPin, clockPin);
  //these values from the calibration code for this library in github
  scale.set_offset(4294465165); 
  scale.set_scale(-21.719906);
  //reset the scale to zero = 0
  //scale.tare(0);


  Serial.println("MCP9600 HW test");
  Wire.begin(D1, D2);

  /* Initialise the driver with I2C_ADDRESS and the default I2C bus. */
  if (!mcp.begin(I2C_ADDRESS)) {
    Serial.println("Sensor not found. Check wiring!");
    while (1)
      ;
  }

  Serial.println("Found MCP9600!");

  mcp.setADCresolution(MCP9600_ADCRESOLUTION_18);
  Serial.print("ADC resolution set to ");
  switch (mcp.getADCresolution()) {
    case MCP9600_ADCRESOLUTION_18: Serial.print("18"); break;
    case MCP9600_ADCRESOLUTION_16: Serial.print("16"); break;
    case MCP9600_ADCRESOLUTION_14: Serial.print("14"); break;
    case MCP9600_ADCRESOLUTION_12: Serial.print("12"); break;
  }
  Serial.println(" bits");

  mcp.setThermocoupleType(MCP9600_TYPE_K);
  Serial.print("Thermocouple type set to ");
  switch (mcp.getThermocoupleType()) {
    case MCP9600_TYPE_K: Serial.print("K"); break;
    case MCP9600_TYPE_J: Serial.print("J"); break;
    case MCP9600_TYPE_T: Serial.print("T"); break;
    case MCP9600_TYPE_N: Serial.print("N"); break;
    case MCP9600_TYPE_S: Serial.print("S"); break;
    case MCP9600_TYPE_E: Serial.print("E"); break;
    case MCP9600_TYPE_B: Serial.print("B"); break;
    case MCP9600_TYPE_R: Serial.print("R"); break;
  }
  Serial.println(" type");

  mcp.setFilterCoefficient(3);
  Serial.print("Filter coefficient value set to: ");
  Serial.println(mcp.getFilterCoefficient());

  mcp.setAlertTemperature(1, 30);
  Serial.print("Alert #1 temperature set to ");
  Serial.println(mcp.getAlertTemperature(1));
  mcp.configureAlert(1, true, true);  // alert 1 enabled, rising temp

  mcp.enable(true);

  Serial.println(F("------------------------------"));
}


void loop() {
  int nVoltageRaw;
  float fVoltage;
  String strBuffer;
  int sleepval;



  Serial.println("==========");
  //5s delay after power on voltage
  delay(5000);
  Serial.print("Hot Junction: ");
  Serial.println(mcp.readThermocouple());
  Serial.print("Cold Junction: ");
  Serial.println(mcp.readAmbient());
  //Serial.print("ADC: ");
  //Serial.print(mcp.readADC() * 2);
  //Serial.println(" uV");

  strBuffer = String(mcp.readThermocouple());
  strBuffer.toCharArray(charBuffer, 10);
  if (!client.publish(mqtt_topic_prefix_tcouple_hot, charBuffer, false)) {
    ESP.restart();
    delay(100);
  }

  strBuffer = String(mcp.readAmbient());
  strBuffer.toCharArray(charBuffer, 10);
  if (!client.publish(mqtt_topic_prefix_tcouple_cold, charBuffer, false)) {
    ESP.restart();
    delay(100);
  }

  Serial.println("==========");


  sensors.requestTemperatures();
  Serial.print("1wire temperature: ");
  Serial.println(sensors.getTempCByIndex(0));
  strBuffer = String(sensors.getTempCByIndex(0));
  strBuffer.toCharArray(charBuffer, 10);
  if (!client.publish(mqtt_topic_prefix_1wire, charBuffer, false)) {
    ESP.restart();
    delay(100);
  }

  Serial.println("==========");
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);

  strBuffer = String(rssi);
  strBuffer.toCharArray(charBuffer, 10);
  if (!client.publish(mqtt_topic_prefix_rssi, charBuffer, false)) {
    ESP.restart();
    delay(100);
  }

  Serial.println("==========");
  if (scale.is_ready())
  {
    Serial.print("Weight: ");
    float weight = scale.get_units(1);
    Serial.println(weight);
    strBuffer = String(weight);
    if (!client.publish(mqtt_topic_prefix_weight, charBuffer, false)) {
      ESP.restart();
      delay(100);
    }
  } else {
    Serial.println("HX711 not ready");
  }


  Serial.println("==========");

  //determine how long to sleep
  //get ADC
  //voltage divider --
  //  22k from battery to A0
  //  47k from A0 to ground
  //  my 4-wire setup measured 46.958k and 21.547k
  nVoltageRaw = analogRead(A0);
  fVoltage = (float)nVoltageRaw * 0.00460474;
  //4.71v is the maximum to read
  //will operate down to 2.5v but needs reset after
  //fVoltage = (float)nVoltageRaw;
  Serial.print("Battery voltage: ");
  Serial.println(fVoltage);

  //report voltage
  strBuffer = String(fVoltage);
  strBuffer.toCharArray(charBuffer, 10);
  if (!client.publish(mqtt_topic_prefix_voltage, charBuffer, false)) {
    ESP.restart();
    delay(100);
  }

  if (fVoltage < 3.7) {
    //600 seconds
    Serial.println("low volt: 600s sleep");
    sleepval = 6e8;
  } else {
    //50 seconds
    //Serial.println("50s sleep");
    //sleepval = 5e7;
    //120sec
    Serial.println("120s sleep");
    sleepval = 12e7;
  }

  //testing 5s sleep
  //sleepval = 5e6;

  //turn off power
  digitalWrite(MOSFET_PIN, LOW);
  delay(1000);

  ESP.deepSleep(sleepval); /* Sleep for 50 seconds */

  if (WiFi.status() != WL_CONNECTED) {
    ESP.restart();
    delay(100);
  }
}

void reconnect() {
  int maxWait = 0;
  while (!client.connected()) {
    if (debugOutput) Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_id)) {
      if (debugOutput) Serial.println("connected");
    } else {
      if (debugOutput) {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      }
      delay(5000);
      if (maxWait > 10)
        ESP.restart();
      maxWait++;
    }
  }
}





/******************************************************************************/
/******************************************************************************/
void do_update() {
  if (debugOutput) Serial.println("do update");
  t_httpUpdate_return ret = ESPhttpUpdate.update(update_server, 80, update_uri, firmware_version);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      if (debugOutput) Serial.println("[update] Update failed.");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      if (debugOutput) Serial.println("[update] no Update needed");
      break;
    case HTTP_UPDATE_OK:
      if (debugOutput) Serial.println("[update] Update ok.");  // may not called we reboot the ESP
      break;
  }
}
