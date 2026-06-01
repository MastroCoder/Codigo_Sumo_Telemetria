#include "WiFiHandler.hpp"

uint8_t WiFiHandler::retry_count;
EventGroupHandle_t WiFiHandler::wifi_events_handle;

WiFiHandler::WiFiHandler(wifi_config_t *conf){
  this->conf = conf;
  retry_count = 0;
  wifi_events_handle = xEventGroupCreate();
}

WiFiHandler::WiFiHandler() = default;

void WiFiHandler::EventHandler(void *arg, esp_event_base_t e_base,
                                  int32_t e_id, void *e_data){
  if (e_base == WIFI_EVENT){
    if (e_id == WIFI_EVENT_STA_START){
      esp_wifi_connect();
    }
    else if (WIFI_EVENT_STA_DISCONNECTED){
      if(retry_count < MAX_RETRY){
        ++retry_count;
        esp_wifi_connect();
      }
      else{
        xEventGroupSetBits(wifi_events_handle, WIFI_FAIL_BIT);   
      }     
    }
  }
  else if (e_base == IP_EVENT && e_id == IP_EVENT_STA_GOT_IP){
    retry_count = 0;
    xEventGroupSetBits(wifi_events_handle, WIFI_CONNECTED_BIT);
  } 
  return;
}

esp_err_t WiFiHandler::Connect(){
  wifi_events_handle = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &EventHandler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &EventHandler, NULL, &instance_got_ip));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, conf));
  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t bits = xEventGroupWaitBits(
    wifi_events_handle,
    WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
    pdFALSE,
    pdFALSE,
    portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
    * happened. */
  if (bits & WIFI_CONNECTED_BIT) {
    Serial.println("Connected!");
    return ESP_OK;
  } else if (bits & WIFI_FAIL_BIT) {
    Serial.println("Not connected!");
    return ESP_ERR_WIFI_NOT_CONNECT;
  } else {
    Serial.println("An error has occurred.");
    return ESP_FAIL;
  }
}