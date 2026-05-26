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

esp_err_t FileHandler::OpenFile(const char* file_name, const char* type){
  char *full_name = (char *) calloc((strlen(file_name) + strlen(conf->base_path) + 2), sizeof(char));
  if (full_name == NULL) return ESP_FAIL;
  strcat(full_name, conf->base_path);
  strcat(full_name, "/");
  strcat(full_name, file_name);
  file = fopen(full_name, type);
  free(full_name);
  if (file == NULL) return ESP_FAIL;
  return ESP_OK;
}

esp_err_t FileHandler::Write(const char *fmt){
  if (file != NULL){
    if (fprintf(file, "%s", fmt) >= 0) return ESP_OK;
    else return ESP_ERR_INVALID_STATE;
  }
  return ESP_ERR_NOT_FOUND;
}

esp_err_t FileHandler::ReadFile(char *buf, int len, long byte_to_read){
  if (file != NULL){
    fseek(file, byte_to_read, SEEK_SET);
    if (fgets(buf, len, file) != NULL) return ESP_OK;
    else return ESP_ERR_INVALID_STATE; 
  }
  return ESP_FAIL;
}

esp_err_t FileHandler::CloseFile(){
  if (file != NULL) {
    fclose(file);
    file = NULL;
    return ESP_OK;
  }
  return ESP_ERR_INVALID_STATE;
}

void FileHandler::Unmount(){
  esp_vfs_littlefs_unregister(conf->partition_label);
}