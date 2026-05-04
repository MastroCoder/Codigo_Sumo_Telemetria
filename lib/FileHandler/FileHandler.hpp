#include "esp_littlefs.h"
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <string.h>
#include "types.hpp"

class FileHandler {
  private:
    const esp_vfs_littlefs_conf_t *conf;
    FILE *file;
    
  public:
    FileHandler(const esp_vfs_littlefs_conf_t *conf);
    esp_err_t Mount();
    esp_err_t CreateFile(const char* file_name, const char* op);
    esp_err_t Write(const char* msg);
    void Unmount();
};