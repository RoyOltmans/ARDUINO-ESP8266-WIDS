// Not yet finished

#include "PubSubClient.h"
WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "servername";
int mqtt_port = 1883;
int retrymqtt = 5;

void mqtt_send_msg(char *topic, char *message) {
  int count = 1;
  while (!client.connected() && count <= retrymqtt) {  // Loop till connection is established
    Serial.print("Attempting MQTT connection...");     
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX); // Create a random client ID
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("home/nodemcu/", "hello world");
      // ... and resubscribe
      client.subscribe("home/nodemcu/");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
	count++;
  }
}