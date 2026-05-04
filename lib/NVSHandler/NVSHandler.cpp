#include "NVSHandler.hpp"

NVSHandler::NVSHandler(const char *name){
  namespace_name = name;
}

esp_err_t NVSHandler::StartStorage(nvs_open_mode_t open_mode = NVS_READWRITE){
  nvs_err = nvs_flash_init();
  if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    nvs_err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(nvs_err);
  nvs_err = nvs_open(namespace_name, open_mode, &nvs_namespace_handle);
  return nvs_err;
}

inline void NVSHandler::CloseStorage(){ nvs_close(nvs_namespace_handle); }

esp_err_t NVSHandler::ReadUInt16(const char *key, uint16_t *const value){
  nvs_err = nvs_get_u16(nvs_namespace_handle, key, value);
  return nvs_err;
}

esp_err_t NVSHandler::WriteUInt16(const char *key, const uint16_t *value){
  nvs_err = nvs_set_u16(nvs_namespace_handle, key, *value);
  return nvs_err;
}