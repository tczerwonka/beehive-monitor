/*************************************************/
/* Debugging                                     */
/*************************************************/
const bool debugOutput = true;  // set to true for serial OUTPUT

/*************************************************/
/* Settings for WLAN                             */
/*************************************************/
const char* ssid = "SSID";
const char* password = "ergerg123erg";

/*************************************************/
/* Update settings                               */
/*************************************************/ 
const char* firmware_version = "beeeeeez_0.0.1";
const char* update_server = "myhost";
const char* update_uri = "/path/update.php";

/*************************************************/
/* MQTTCloud data                               */
/*************************************************/
const char* mqtt_host = "mqtthost";
const char* mqtt_id = "ESP8266-Hive";
const char* mqtt_topic_prefix_voltage = "/hive1/voltage";
const char* mqtt_topic_prefix_tcouple_hot = "/hive1/thermocouple_hot";
const char* mqtt_topic_prefix_tcouple_cold = "/hive1/thermocouple_cold";
const char* mqtt_topic_prefix_1wire = "/hive1/1wire";
const char* mqtt_topic_prefix_weight = "/hive1/weight";
const int mqtt_port = 1883;

