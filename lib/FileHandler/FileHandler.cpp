#include "FileHandler.hpp"

FileHandler::FileHandler(const esp_vfs_littlefs_conf_t *conf){
  this->conf = conf;
}

// Incluir logging de possíveis erros aqui?
esp_err_t FileHandler::Mount(){
  return esp_vfs_littlefs_register(conf);
}

esp_err_t FileHandler::CreateFile(const char* file_name, const char* op){
  file = fopen(file_name, op);
  if (file == NULL) return ESP_FAIL;
  return ESP_OK;
}

esp_err_t FileHandler::Write(const char* msg){
  if (file == NULL) return ESP_ERR_INVALID_STATE;
  int err = fprintf(file, msg);
  if (err < 0) return ESP_FAIL;
  return ESP_OK;
}

void FileHandler::Unmount(){
  esp_vfs_littlefs_unregister(conf->partition_label);
}