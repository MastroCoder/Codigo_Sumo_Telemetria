#ifndef LIB_HTTP_HANDLER_HPP_
#define LIB_HTTP_HANDLER_HPP_

#include "internal_defs.h"
#include "esp_http_client.h"
#include <ctype.h>
#include <sys/param.h>
#include "esp_event.h"
#include "esp_system.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

class HTTPHandler {
  private:
    esp_http_client_config_t *config;
    static char *output_buffer;
    static char int output_len;
    static esp_err_t EventHandler(esp_http_client_event *e);
  
  public:
    HTTPHandler(esp_http_client_config_t *c);
};

#endif