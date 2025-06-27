#include <WiFi.h>
#include <esp_wifi.h>
#include "types.h"
#include "web_interface.h"
#include "deauth.h"
#include "definitions.h"
#include "boot_button_pressed.h"

int curr_channel = 1;
static uint8_t macAddr[6];
void setup() {
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif
#ifdef LED
  pinMode(LED, OUTPUT);
  pinMode(MODE_LED, OUTPUT);
#endif

  initBootButtonAsInterrupt();
  
#ifdef GENERATE_RANDOM_MAC
  // Generate random MAC
  do {
    for (int i = 0; i < 6; i++) {
      macAddr[i] = random(256); // Random byte
    }
    macAddr[0] &= 0xFC; // Clear the lowest two bits: use unicast & universally administered address
  } while (macAddr[0] == 0x00); // Prevent all-zero MAC address
  esp_wifi_set_mac(WIFI_IF_AP, macAddr);
  uint8_t last_mac5 = macAddr[5];
  do {
    macAddr[5] = random(256);
  } while (macAddr[5] == last_mac5);
  esp_wifi_set_mac(WIFI_IF_STA, macAddr);
#endif

  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP(AP_SSID, AP_PASS);
  esp_wifi_set_max_tx_power(WIFI_TX_MAX_POWER);

  start_web_interface();
  digitalWrite(MODE_LED, LOW);
}

void loop() {
  if (deauth_type == DEAUTH_TYPE_ALL) {
    if (curr_channel > CHANNEL_MAX) curr_channel = 1;
    esp_wifi_set_channel(curr_channel, WIFI_SECOND_CHAN_NONE);
    curr_channel++;
    delay(10);
  } else {
    web_interface_handle_client();
  }
}
