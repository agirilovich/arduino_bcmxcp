#include "MQTT_task.h"

void initializeMQTTTopic(MQTTPubSubClient mqtt, const char *mqtt_user, const char *mqtt_pass, const char *Topic, const String ConfigMessage)
{

  Serial.print("Testing connection to mqtt broker...");

  while (!mqtt.connect(DEVICE_BOARD_NAME, mqtt_user, mqtt_pass))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println(" connected!");

  //Publish message to AutoDiscovery topic
  mqtt.publish(String(Topic) + "/config", ConfigMessage, false, 0);
  
  //Gracefully close connection to MQTT broker
  mqtt.disconnect();

}


void publishMQTTPayload(MQTTPubSubClient mqtt, const char *mqtt_user, const char *mqtt_pass, const char *Topic, unsigned int PayloadMessage)
{
  mqtt.connect(DEVICE_BOARD_NAME, mqtt_user, mqtt_pass);
  //mqtt.update();
  mqtt.publish(String(Topic) + "/state", String(PayloadMessage), false, 0);
  mqtt.disconnect();
}