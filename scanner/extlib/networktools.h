#include "ESP8266WiFi.h"
#include "ESP8266Ping.h"

const char* ssid = "ACCESSPOINT";
const char* password = "PASSWORD123";
const char* remote_host = "www.google.com";

void funcping() {
  Serial.print("Pinging host ");
  Serial.println(remote_host);
  delay(200);
  if(Ping.ping(remote_host)) {
    Serial.println("Success!!");
  } else {
    Serial.println("Error :(");
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
