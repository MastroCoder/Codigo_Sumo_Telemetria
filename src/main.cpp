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

const char* round_number_key = "round_number";

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

WiFiHandler wifi_handler;
HTTPHandler http_handler;

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
    if (state_machine.states.robot_task == RobotTask::kRunMatches &&
        state_machine.states.fight_state == FightState::kFighting){
      strategy_runner.RunStrategy(state_machine);
    }
    if (state_machine.states.fight_state == FightState::kStop){
      strategy_runner.SetMotors(MotorSpeeds::kStopped);
      vTaskDelete(motor_task_handle);
    }
  }
}

void TelemetryTask(void *pvParameters){
  if (state_machine.states.robot_task == RobotTask::kRunMatches){
    if (file_handle.file == NULL){
      if (file_handle.OpenFile("data", "w+") == ESP_OK){
        file_handle.Write("Timestamp,RPM,S1,S2,S3,S4,S5,QTR1,QTR2\n");
      }
    }
    unsigned long timestamp = millis() - state_machine.start_time;
    int rpm = 0;
    EventBits_t sensor_bits = xEventGroupGetBits(enemy_detector.event_group);
    EventBits_t qtr_bits = xEventGroupGetBits(line_detector.event_group);
    char *buf = (char*)malloc(sizeof(char) * MAX_HTTP_BUFFER);
    if (buf == NULL) return;
    snprintf(buf, sizeof(timestamp), "%lu", timestamp); 
  }
}

void FileSendTask(void *pvParameters){
  if (state_machine.states.robot_task == RobotTask::kSendData){
    wifi_handler = WiFiHandler(&wifi_config);
    if (wifi_handler.Connect() == ESP_OK){
      http_handler = HTTPHandler(&http_config);
      char *buf = file_handle.EncodeFileToBase64();
      if (http_handler.SendTelemetryData(buf) == ESP_OK) http_handler.EndSession();
    }
  }
}