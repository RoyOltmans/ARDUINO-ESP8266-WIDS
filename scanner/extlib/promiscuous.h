// This-->tab == "functions.h"

// Expose Espressif SDK functionality
extern "C" {
#include "user_interface.h"
  typedef void (*freedom_outside_cb_t)(uint8 status);
  int  wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
  void wifi_unregister_send_pkt_freedom_cb(void);
  int  wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
}

#include "ESP8266WiFi.h"
#include "./structures.h"
#include "SoftwareSerial.h"
#include "./serialat.h"

#define MAX_APS_TRACKED 100
#define MAX_CLIENTS_TRACKED 200

#define disable 0
#define enable 1


unsigned int channel = 1;

beaconinfo aps_known[MAX_APS_TRACKED];                    // Array to save MACs of known APs
int aps_known_count = 0;                                  // Number of known APs
int nothing_new = 0;
clientinfo clients_known[MAX_CLIENTS_TRACKED];            // Array to save MACs of known CLIENTs
int clients_known_count = 0;                              // Number of known CLIENTs

int register_beacon(beaconinfo beacon)
{
  int known = 0;   // Clear known flag
  for (int u = 0; u < aps_known_count; u++)
  {
    if (! memcmp(aps_known[u].bssid, beacon.bssid, ETH_MAC_LEN)) {
      known = 1;
      break;
    }   // AP known => Set known flag
  }
  if (! known)  // AP is NEW, copy MAC to array and return it
  {
    memcpy(&aps_known[aps_known_count], &beacon, sizeof(beacon));
    aps_known_count++;

    if ((unsigned int) aps_known_count >=
        sizeof (aps_known) / sizeof (aps_known[0]) ) {
      Serial.printf("exceeded max aps_known\n");
      aps_known_count = 0;
    }
  }
  return known;
}

int register_client(clientinfo ci)
{
  int known = 0;   // Clear known flag
  for (int u = 0; u < clients_known_count; u++)
  {
    if (! memcmp(clients_known[u].station, ci.station, ETH_MAC_LEN)) {
      known = 1;
      break;
    }
  }
  if (! known)
  {
    memcpy(&clients_known[clients_known_count], &ci, sizeof(ci));
    clients_known_count++;
    if ((unsigned int) clients_known_count >=
        sizeof (clients_known) / sizeof (clients_known[0]) ) {
      Serial.printf("exceeded max clients_known\n");
      clients_known_count = 0;
    }
  }
  return known;
}

void print_beacon(beaconinfo beacon)
{
  String beacon_ssid = "";
  char beacon_mac[19] = "";
  String beacon_channel = "";
  String beacon_rssi = "";
  String beacon_string = "";
  if (beacon.err != 0) {
    //Serial.printf("BEACON ERR: (%d)  ", beacon.err);
  } else {
	snprintf(beacon_mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x", beacon.bssid[0], beacon.bssid[1], beacon.bssid[2], beacon.bssid[3], beacon.bssid[4], beacon.bssid[5]);
	beacon_ssid = (char *)beacon.ssid;
    beacon_channel = String(beacon.channel);
	beacon_rssi = String(beacon.rssi);
	beacon_string = "BEACON: <=============== [" + beacon_ssid + "] " + String(beacon_mac) + " - " + beacon_channel + " - " + beacon_rssi;
  }
  send_serial_msg(beacon_string);
}

void print_client(clientinfo ci)
{
  int u = 0;
  int known = 0;   // Clear known flag
  String device_ap_ssid = "";
  char device_mac[19] = "";
  char device_ap_mac[19] = "";
  String device_channel = "";
  String device_rssi = "";
  String device_string = "";
  if (ci.err != 0) {
    // nothing
  } else {
	snprintf(device_mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x", ci.station[0], ci.station[1], ci.station[2], ci.station[3], ci.station[4], ci.station[5]);
    for (u = 0; u < aps_known_count; u++)
    {
      if (! memcmp(aps_known[u].bssid, ci.bssid, ETH_MAC_LEN)) {
        device_ap_ssid = (char *)aps_known[u].ssid;
        known = 1;     // AP known => Set known flag
        break;
      }
    }
	device_channel = String(aps_known[u].channel);
    if (! known)  {
	  snprintf(device_ap_mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x", ci.bssid[0], ci.bssid[1], ci.bssid[2], ci.bssid[3], ci.bssid[4], ci.bssid[5]);
	  device_string = "DEVICE: " + String(device_mac) + " ==> Unknown/Malformed packet";
    } else {
		snprintf(device_ap_mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x", ci.ap[0], ci.ap[1], ci.ap[2], ci.ap[3], ci.ap[4], ci.ap[5]);
		device_string = "DEVICE: " + String(device_mac) + " ==> [" + device_ap_ssid + "] " + String(device_ap_mac) + " - " + device_channel + " - " + String(ci.rssi);
    }
  }
  send_serial_msg(device_string);
}

void promisc_cb(uint8_t *buf, uint16_t len)
{
  int i = 0;
  uint16_t seq_n_new = 0;
  if (len == 12) {
    struct RxControl *sniffer = (struct RxControl*) buf;
  } else if (len == 128) {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    struct beaconinfo beacon = parse_beacon(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
    if (register_beacon(beacon) == 0) {
      print_beacon(beacon);
      nothing_new = 0;
    }
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    //Is data or QOS?
    if ((sniffer->buf[0] == 0x08) || (sniffer->buf[0] == 0x88)) {
      struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
      if (memcmp(ci.bssid, ci.station, ETH_MAC_LEN)) {
        if (register_client(ci) == 0) {
          print_client(ci);
          nothing_new = 0;
        }
      }
    }
  }
}

void monitor_mode() {
  wifi_set_opmode(STATION_MODE);            // Promiscuous works only with station mode
  wifi_set_channel(channel);
  wifi_promiscuous_enable(disable);
  wifi_set_promiscuous_rx_cb(promisc_cb);   // Set up promiscuous callback
  wifi_promiscuous_enable(enable);
}