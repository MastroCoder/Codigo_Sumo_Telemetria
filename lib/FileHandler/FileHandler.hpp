#ifndef LIB_FILEHANDLER_HPP_
#define LIB_FILEHANDLER_HPP_

#include "esp_littlefs.h"
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <string.h>
#include "type_defs.hpp"
#include "esp_log.h"
#include <Arduino.h>

class FileHandler {
  private:
    const esp_vfs_littlefs_conf_t *conf;
    static FILE *file;
    
  public:
    FileHandler(const esp_vfs_littlefs_conf_t *conf);
    void Mount();
    esp_err_t CreateFile(const char* file_name);
    void CloseFile();
    esp_err_t ReadFile(char* buf, int len, long byte_to_read);
    esp_err_t Write(const char* fmt);
    void Unmount();
};

#endif