#include "HTTPHandler.hpp"

char* HTTPHandler::output_buffer;
int HTTPHandler::output_len;

HTTPHandler::HTTPHandler() = default;

HTTPHandler::HTTPHandler(esp_http_client_config_t *conf){
  this->config = conf;
  this->config->event_handler = EventHandler;
}

esp_err_t HTTPHandler::EventHandler(esp_http_client_event_t *e){
  switch (e->event_id){
    case HTTP_EVENT_ON_HEADER:
      Serial.printf("Triggered HTTP_EVENT_ON_HEADER, key=%s, value=%s\n", e->header_key, e->header_value);
      break;
      
    case HTTP_EVENT_ON_DATA:
      if (output_len == 0 && e->user_data) memset(e->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);

      if (!esp_http_client_is_chunked_response(e->client)){
        int copy_len = 0;
        if (e->user_data) {
          copy_len = MIN(e->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
          if (copy_len) {
            memcpy(e->user_data + output_len, e->data, copy_len);
          }
        } 
        else {
          int content_len = esp_http_client_get_content_length(e->client);
          if (output_buffer == NULL) {
            output_buffer = (char *) calloc(content_len + 1, sizeof(char));
            output_len = 0;
            if (output_buffer == NULL) {
              Serial.println("Failed to allocate memory for output buffer");
              return ESP_FAIL;
            }
          }
          copy_len = MIN(e->data_len, (content_len - output_len));
          if (copy_len) {
            memcpy(output_buffer + output_len, e->data, copy_len);
          }
        }
        output_len += copy_len;
      }
      break;
    case HTTP_EVENT_ON_FINISH:
      if (output_buffer != NULL){
        free(output_buffer);
        output_buffer = NULL;
      }
      output_len = 0;
      break;
    case HTTP_EVENT_DISCONNECTED:
      {  
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)e->data, &mbedtls_err, NULL);
        if (err != 0){
          Serial.printf("Last ESP error code: 0x%x\n", err);
          Serial.printf("Last MBEDTLS failure:0x%x\n", mbedtls_err);
        }
        if (output_buffer != NULL){
          free(output_buffer);
          output_buffer = NULL;
        }
        output_len = 0;
        break;
      }
    default:
      break;
  }
  return ESP_OK;
}

esp_err_t HTTPHandler::SendTelemetryData(char* base64_buf){
  char res_buf[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
  config->user_data = res_buf;
  client = esp_http_client_init(config);
  esp_http_client_set_url(client, "http://" API_URL "/post-round");
  esp_http_client_set_method(client, HTTP_METHOD_POST);
  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_post_field(client, base64_buf, strlen(base64_buf));
  return esp_http_client_perform(client);
}

esp_err_t HTTPHandler::CloseMatch(){
  char res_buf[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
  config->user_data = res_buf;
  client = esp_http_client_init(config);
  esp_http_client_set_url(client, "http://" API_URL "/close-current-match");
  esp_http_client_set_method(client, HTTP_METHOD_POST);
  esp_http_client_set_header(client, "Content-Type", "application/json");
  return esp_http_client_perform(client);
}

void HTTPHandler::EndSession(){
  esp_http_client_cleanup(client);
}