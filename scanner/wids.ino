// Original by Ray Burnette 20161013 compiled on Linux 16.3 using Arduino 1.6.12
//
// Author: Ray Burnette
// Refactoring: Roy Oltmans
// Changelog: Make Modulair 20170517
//            Add Message delivery via Serial
//            Refactor/Cleanup code
//            Original is Sniffing 6 renamed to wids

#include "ESP8266WiFi.h"
#include "extlib/promiscuous.h"

#define disable 0
#define enable  1

void setup() {
  Serial.begin(9600);     // communication with the host computer
  delay(20000); // Wait to delay scan before serial output has been initilized
  monitor_mode();
  Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
  Serial.printf("Wifi Intrusion Detection System WIDS v0.1\n\r");
  Serial.println(F("Type:   /-------MAC------/-----WiFi Access Point SSID-----/  /----MAC---/  Chnl  RSSI"));
}

void loop() {
  channel = 1;
  wifi_set_channel(channel);
  while (true) {
    //wifi_promiscuous_enable(enable);
    //mqtt_send_msg();
    nothing_new++;                          // Array is not finite, check bounds and adjust if required
    if (nothing_new > 200) {
      nothing_new = 0;
      channel++;
      if (channel == 15) break;             // Only scan channels 1 to 14
      wifi_set_channel(channel);
    }
    delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()
  }
}


