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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_littlefs.h>
#include <IRremote.hpp>

SemaphoreHandle_t motor_telemetry_semaphore = xSemaphoreCreateMutex();

EnemyDetector enemy_detector = EnemyDetector();
LineDetector line_detector = LineDetector();
StateMachine state_machine = StateMachine(&enemy_detector.event_group, &line_detector.event_group);
StrategyRunner strategy_runner = StrategyRunner();

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
  
void setup() {
  disableCore0WDT();
  disableCore1WDT();
  if (qtr_info.StartStorage(NVS_READWRITE) == ESP_OK){
    line_detector.Calibrate(QTRCalibrate::kCalibrate, qtr_info);
  }
  IrReceiver.begin(IR_PIN, true, LED_BUILTIN);
  IrReceiver.enableIRIn();
  
  xTaskCreatePinnedToCore(SensorsTask, "sensor_task", 4096, NULL, 1, &sensing_task_handle, 0);
  xTaskCreatePinnedToCore(MotorsTask, "motors_task", 2048, NULL, 1, &motor_task_handle, 1);
  xTaskCreatePinnedToCore(TelemetryTask, "telemetry_task", MAX_STACK_DEPTH, NULL, 1, &telemetry_task_handle, 1);
}

void loop() {
}

void SensorsTask(void *pvParameters){
  for (;;){
    if (IrReceiver.decode()){
      state_machine.ResolveIRReceiver(IrReceiver.decodedIRData.command);
    }
    enemy_detector.Detect();
    line_detector.Detect();
    state_machine.UpdateState();
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
      if (state_machine.states.robot_task == RobotTask::kRunMatches){
        if (file_handle.file == NULL){
          if (file_handle.OpenFile("data", "w+") == ESP_OK){
            file_handle.Write("Timestamp,RPM,S1,S2,S3,S4,S5,QTR1,QTR2\n");
          }
        }
        // funções para escrita
      }
    }
    else{
      Serial.println("Blocked by Motors Task");
    }
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

void FileSendTask(void *pvParameters){
  if (state_machine.states.robot_task == RobotTask::kSendData){
    wifi_handle = WiFiHandler(&wifi_config);
    if (wifi_handle.Connect() == ESP_OK){
      http_handle = HTTPHandler(&http_config);
      char *buf = file_handle.EncodeFileToBase64();
      if (http_handle.SendTelemetryData(buf) == ESP_OK) http_handle.EndSession();
    }
  }
}