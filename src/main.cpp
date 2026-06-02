#define DECODE_SONY

#include <Arduino.h>
#include <QTRSensors.h>
#include "GlobalDefs.hpp"
#include "Detectors.hpp"
#include "StateMachine.hpp"
#include "Itamotorino.h"
#include "FileHandler.hpp"
#include "NVSHandler.hpp"
#include "WiFiHandler.hpp"
#include "HTTPHandler.hpp"
#include "StrategyRunner.hpp"
#include "HallSensorHandler.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_littlefs.h>
#include <IRremote.hpp>

SemaphoreHandle_t motor_telemetry_semaphore = xSemaphoreCreateMutex();

EnemyDetector enemy_detector;
LineDetector line_detector;
StateMachine state_machine;
StrategyRunner strategy_runner;
HallSensorHandler hall_handler = HallSensorHandler();

wifi_sta_config_t sta_config = {
  WIFI_SSID,
  WIFI_PASS,
};

wifi_config_t wifi_config = {
  .sta = sta_config,
};

esp_http_client_config_t http_config = {
  .host = API_URL,
  .path = "/",
  .query= "esp"
};

WiFiHandler wifi_handle;
HTTPHandler http_handle;

const esp_vfs_littlefs_conf_t vfs_config = {
  .base_path = "/littlefs",
  .partition_label = "telemetry",
  .format_if_mount_failed = false,
  .dont_mount = false
};

NVSHandler qtr_info = NVSHandler("QTR");
FileHandler file_handle = FileHandler(&vfs_config);

TaskHandle_t motor_task_handle, sensing_task_handle, telemetry_task_handle, file_send_handle;

void SensorsTask(void *pvParameters);
void MotorsTask(void *pvParameters);
void TelemetryTask(void *pvParameters);
void FileSendTask(void *pvParameters);

char *write_buffer;
size_t buffer_cap = 32;

esp_err_t ReallocIfNotEnough(char **buf, size_t size_to_put);
esp_err_t PutTimestampInBuffer(char **buf);
esp_err_t PutRPMsInBuffer(char **buf);
esp_err_t PutEventBitsInBuffer(char **buf, EventBits_t bits, unsigned int size, bool is_last);
esp_err_t PutAllDataInBuffer(char **buf);

void setup() {
  Serial.begin(115200);
  while (!Serial) {;}
  disableCore0WDT();
  disableCore1WDT();
  Serial.println("Disabled core watchdogs");
  state_machine = StateMachine(&enemy_detector.event_group, &line_detector.event_group);
  Serial.println("Started detectors");
  Serial.println("Started strategy runner and hall sensor handler");
  
  if (qtr_info.StartStorage(NVS_READWRITE) == ESP_OK){
    line_detector.Calibrate(QTRCalibrate::kCalibrate, qtr_info);
  }

  Serial.println("Calibrated sensors");
  IrReceiver.begin(IR_PIN, true, LED_BUILTIN);
  IrReceiver.enableIRIn();
  
  xTaskCreatePinnedToCore(SensorsTask, "sensor_task", MAX_STACK_DEPTH, NULL, 1, &sensing_task_handle, 0);
  xTaskCreatePinnedToCore(MotorsTask, "motors_task", MAX_STACK_DEPTH, NULL, 1, &motor_task_handle, 1);
  xTaskCreatePinnedToCore(TelemetryTask, "telemetry_task", MAX_STACK_DEPTH, NULL, 1, &telemetry_task_handle, 1);
}

void loop() {
}

void SensorsTask(void *pvParameters){
  for (;;){
    if (IrReceiver.decode()){
      IrReceiver.resume();
      state_machine.ResolveIRReceiver(IrReceiver.decodedIRData.command);
    }
    if (state_machine.states.fight_state == FightState::kFighting){
      enemy_detector.Detect();
      line_detector.Detect();
      state_machine.UpdateState();
    }
  }
}

void MotorsTask(void *pvParameters){
  for (;;){
    if (xSemaphoreTake(motor_telemetry_semaphore, pdMS_TO_TICKS(30))){
      if (state_machine.states.robot_task == RobotTask::kRunMatches &&
          state_machine.states.fight_state == FightState::kFighting){
        strategy_runner.RunStrategy(state_machine);
      }
      if (state_machine.states.fight_state == FightState::kStop){
        strategy_runner.SetMotors(MotorSpeeds::kStopped);
        vTaskDelete(motor_task_handle);
      }
      xSemaphoreGive(motor_telemetry_semaphore);
    }
    else{
      Serial.println("Still blocked by write.");
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void TelemetryTask(void *pvParameters){
  for (;;){
    if (xSemaphoreTake(motor_telemetry_semaphore, pdMS_TO_TICKS(30))){
      if (state_machine.states.robot_task == RobotTask::kRunMatches && 
          state_machine.states.fight_state == FightState::kFighting){
        if (file_handle.file == NULL){
          if (file_handle.OpenFile("data", "w+") == ESP_OK){
            file_handle.Write("Timestamp,RPM1,RPM2,S1,S2,S3,S4,QTR1,QTR2\n");
          }
        }
        // funções para escrita
        write_buffer = (char*) calloc(buffer_cap, sizeof(char));
        if (write_buffer != NULL) {
          if (PutAllDataInBuffer(&write_buffer) == ESP_OK){
            file_handle.Write(write_buffer);
            free(write_buffer);
          }
        }
      }
      else if (state_machine.states.fight_state == FightState::kStop){
        file_handle.CloseFile();
        // passar o controle para a tarefa de wifi
        xTaskCreatePinnedToCore(FileSendTask, "file_send_task", MAX_STACK_DEPTH, NULL, 1, &file_send_handle, 1);
        vTaskDelete(telemetry_task_handle);
      }
      xSemaphoreGive(motor_telemetry_semaphore);
    }
    else{
      Serial.println("Blocked by Motors Task");
    }
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

void FileSendTask(void *pvParameters){
  if (state_machine.states.robot_task == RobotTask::kSendData && 
      state_machine.states.fight_state == FightState::kStop){
    wifi_handle = WiFiHandler(&wifi_config);
    if (wifi_handle.Connect() == ESP_OK){
      http_handle = HTTPHandler(&http_config);
      char *buf = file_handle.EncodeFileToBase64();
      if (http_handle.SendTelemetryData(buf) == ESP_OK) http_handle.EndSession();
    }
  }
}

esp_err_t ReallocIfNotEnough(char **buf, size_t size_to_put){
  size_t len = strlen(*buf);
  if (len + size_to_put + 1 < buffer_cap) return ESP_OK;
  
  while(buffer_cap < len + size_to_put + 1) buffer_cap *= 2;

  char *new_buf = (char*) realloc(*buf, buffer_cap);
  if (new_buf == NULL) return ESP_FAIL;
  *buf = new_buf;
  return ESP_OK;
}

esp_err_t PutTimestampInBuffer(char **buf){
  char *tmp = (char*) malloc(sizeof(long)*8 + 2);
  if (tmp == NULL) return ESP_FAIL;
  int n = sprintf(tmp, "%ld,", millis() - state_machine.start_time);
  if (ReallocIfNotEnough(buf, (size_t)n) != ESP_OK) return ESP_FAIL;
  strcat(*buf, tmp);
  free(tmp);
  return ESP_OK;
}

esp_err_t PutRPMsInBuffer(char **buf){
  std::pair<unsigned int, unsigned int> measurements = hall_handler.CalculateRPM(state_machine.start_time);
  char *tmp = (char*) malloc(sizeof(unsigned int) * 4 + 2);
  if (tmp == NULL) return ESP_FAIL;
  int n = sprintf(tmp, "%u,", measurements.first);
  if (ReallocIfNotEnough(buf, (size_t)n) != ESP_OK) return ESP_FAIL;
  strcat(*buf, tmp);
  
  n = sprintf(tmp, "%u,", measurements.second);
  if (ReallocIfNotEnough(buf, (size_t)n) != ESP_OK) return ESP_FAIL;
  strcat(*buf, tmp);
  free(tmp);
  return ESP_OK;
}

esp_err_t PutEventBitsInBuffer(char **buf, EventBits_t bits, unsigned int size, bool is_last){
  if (ReallocIfNotEnough(buf, (size_t)size*2) != ESP_OK) return ESP_FAIL;
  int test_bit = 0;
  while (test_bit < size){
    if (bits & 0x01){
      strcat(*buf, "1");
    }
    else{
      strcat(*buf, "0");
    }
    ++test_bit;
    bits = bits >> 1;
    if (!is_last || test_bit < size){
      strcat(*buf, ",");
    }
    else{
      strcat(*buf, "\n");
    }
  }
  return ESP_OK;
}

esp_err_t PutAllDataInBuffer(char **buf){
  if (PutTimestampInBuffer(buf) != ESP_OK) return ESP_FAIL;
  if (PutRPMsInBuffer(buf) != ESP_OK) return ESP_FAIL;
  if (PutEventBitsInBuffer(buf, xEventGroupGetBits(enemy_detector.event_group), 5, false) != ESP_OK) return ESP_FAIL;
  if (PutEventBitsInBuffer(buf, xEventGroupGetBits(line_detector.event_group), 2, true) != ESP_OK) return ESP_FAIL;
  return ESP_OK;
}