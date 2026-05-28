#include "esp_littlefs.h"
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include "mbedtls/base64.h"

class FileHandler {
  private:
    const esp_vfs_littlefs_conf_t *conf;
    static FILE *file;
    
  public:
    FileHandler(const esp_vfs_littlefs_conf_t *conf);
    void Mount();
    esp_err_t OpenFile(const char* file_name, const char *type);
    esp_err_t CloseFile();
    esp_err_t ReadFile(char* buf, int len, long byte_to_read);
    esp_err_t Write(const char* fmt);
    char* EncodeToBase64(char* src, int read_len);
    // DecodeFromBase64 vai ser desnecessário (espero)
    void Unmount();
};