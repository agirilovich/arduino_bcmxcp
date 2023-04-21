// Load Wi-Fi library
#include <WiFi.h>

WiFiClient client;

//Import credentials from external file out of git repo
#include <Credentials.h>
const char *ssid = ssid_name;
const char *password = ssid_password;

const char *mqtt_host = mqtt_server;
const int mqtt_port = 1883;
const char *mqtt_user = mqtt_username;
const char *mqtt_pass = mqtt_password;

#include "MQTT_task.h"
MQTTPubSubClient mqtt;

const char *Topic    = "/homeassistant/sensor/well/config";   // Topic in MQTT to publish
const String ConfigMessage  = String("{\"name\":") + DEVICE_BOARD_NAME + String(", \"device_class\": \"distance\", \"state_class\": \"measurement\",\"unit_of_measurement\": \"mm\", \"state_topic\": StateTopic}");       // Message for Autodiscovery

#include "main.h"

void setup() {
  Serial.begin(9600);
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.print("Connecting to MQTT broker host: ");
  Serial.println(mqtt_host);

  while (!client.connect(mqtt_host, mqtt_port))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected!");

  mqtt.begin(client);

  //Initialise MQTT autodiscovery topic and sensor
  initializeMQTTTopic(mqtt, mqtt_user, mqtt_pass, Topic, ConfigMessage);

  //Open connection to UPS
  upsdrv_initups();

  // print your version information
  //upsdrv_banner();	
  // prep data, settings for UPS monitoring
  //upsdrv_initinfo();

}
void loop() {
  // update state data of UPS if possible
  //upsdrv_updateinfo();
}