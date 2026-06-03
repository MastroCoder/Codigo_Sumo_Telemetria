#ifndef LIB_WIFIHANDLER_HPP_
#define LIB_WIFIHANDLER_HPP_

#include <string.h>
#include <Arduino.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define WIFI_SSID "sSID"
#define WIFI_PASS "PASS"
#define MAX_RETRY 5

#define WIFI_CONNECTED_BIT (1<<0)
#define WIFI_FAIL_BIT (1<<1)

class WiFiHandler{
  private:
    wifi_config_t *conf;
    static uint8_t retry_count;
    static EventGroupHandle_t wifi_events_handle;
    static void EventHandler(void *arg, esp_event_base_t e_base, 
                                int32_t e_id, void *e_data);
  public:
    WiFiHandler();
    WiFiHandler(wifi_config_t *conf);
    esp_err_t Connect();
};

#endif