#include "FileHandler.hpp"

FILE * FileHandler::file;

FileHandler::FileHandler(const esp_vfs_littlefs_conf_t *conf){
  this->conf = conf;
}

void FileHandler::Mount(){
  esp_err_t err = esp_vfs_littlefs_register(conf);
  if (err != ESP_OK){
    switch (err){
      case ESP_FAIL:
        Serial.println("Failed to mount LittleFS");
        break;
      case ESP_ERR_NOT_FOUND:
        Serial.println("Failed to find partition");
        break;
      default:
        Serial.println(esp_err_to_name(err));
        break;
    }
  }
}

esp_err_t FileHandler::CreateFile(const char* file_name){
  file = fopen(file_name, "w+");
  if (file == NULL) return ESP_FAIL;
  return ESP_OK;
}

esp_err_t FileHandler::Write(const char *fmt){
  if (file != NULL){
    fprintf(file, "%s", fmt);
    return ESP_OK;
  }
  return ESP_ERR_NOT_FOUND;
}

esp_err_t FileHandler::ReadFile(char *buf, int len, long byte_to_read){
  if (file == NULL) return ESP_FAIL;
  fseek(file, byte_to_read, SEEK_SET);
  fgets(buf, len, file);
  Serial.println(buf);
  return ESP_OK;
}

void FileHandler::CloseFile(){
  fclose(file);
  file = NULL;
}

void FileHandler::Unmount(){
  esp_vfs_littlefs_unregister(conf->partition_label);
}