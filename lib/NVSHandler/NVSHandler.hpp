#ifndef LIB_NVS_HANDLER_HPP_
#define LIB_NVS_HANDLER_HPP_

#include "nvs_flash.h"
#include "nvs.h"
#include <Arduino.h>

class NVSHandler {
  private:
    const char *namespace_name;
    esp_err_t nvs_err;
    nvs_handle_t nvs_namespace_handle;
  
  public:
    NVSHandler(const char *name);
    esp_err_t StartStorage(nvs_open_mode_t open_mode);
    void CloseStorage();
    esp_err_t ReadUInt16(const char *key, uint16_t *const value);   // takes const ptr to value
    esp_err_t WriteUInt16(const char *key, const uint16_t *value);  // takes pointer to const value
};

#endif