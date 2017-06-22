#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include "./ArduinoOTA.h"

const char* ssid = "ACCESSPOINT";
const char* password = "PASSWORD_AP";
const byte rxPin = 2; // Wire this to Tx Pin of ESP8266
const byte txPin = 3; // Wire this to Rx Pin of ESP8266
int led_pin = 13;
int dimmer_pin[] = {14, 5, 15};


#define D0 16
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)
#define D3 0
#define D4 2 // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO 
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3 // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)
#define N_DIMMERS 3


// We'll use a software serial interface to connect to ESP8266
SoftwareSerial ESPserial (D2, D3);

void setup() {
  //Serial.begin(115200);
  
  Serial.begin(9600);     // communication with the host computer
  //while (!Serial)   { ; }
  // Start the software serial for communication with the ESP8266-client
  ESPserial.begin(9600);
  Serial.println("Remember to to set Both NL & CR in the serial monitor.");
  
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  
  /* switch off led */
  digitalWrite(led_pin, HIGH);

  /* configure dimmers, and OTA server events */
  analogWriteRange(1000);
  analogWrite(led_pin,990);

  for (int i=0; i<N_DIMMERS; i++)
  {
    pinMode(dimmer_pin[i], OUTPUT);
    analogWrite(dimmer_pin[i],50);
  }
  ArduinoOTA.onStart([]() {
                          String type;
                          if (ArduinoOTA.getCommand() == U_FLASH)
                            type = "sketch";
                          else // U_SPIFFS
                            type = "filesystem";

                          // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                          Serial.println("Start updating " + type);
                          for(int i=0; i<N_DIMMERS;i++)
                          {
                            analogWrite(dimmer_pin[i], 0);
                            analogWrite(led_pin,0);
                          }
  });
  ArduinoOTA.onEnd([]() {
                          for (int i=0;i<30;i++)
                          {
                            analogWrite(led_pin,(i*100) % 1001);
                            delay(50);
                          }
                          Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  delay(30);
  while (ESPserial.available()){
        String inData = ESPserial.readStringUntil('\n');
        Serial.println("Got reponse from ESP8266: " + inData);
  } 
}
