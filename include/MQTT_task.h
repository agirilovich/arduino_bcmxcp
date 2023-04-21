#include <MQTTPubSubClient.h>

void initializeMQTTTopic(MQTTPubSubClient mqtt, const char *mqtt_user, const char *mqtt_pass, const char *Topic, const String ConfigMessage);

void publishMQTTPayload(MQTTPubSubClient mqtt, const char *mqtt_user, const char *mqtt_pass, const char *Topic, unsigned int PayloadMessage);


