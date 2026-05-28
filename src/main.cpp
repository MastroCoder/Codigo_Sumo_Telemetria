#define DECODE_SONY

#include <Arduino.h>
#include <QTRSensors.h>
#include "GlobalDefs.hpp"
#include "Detectors.hpp"
#include "StateMachine.hpp"
#include "Itamotorino.h"
#include "FileHandler.hpp"
#include "NVSHandler.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_littlefs.h>
#include <IRremote.hpp>

const char* round_number_key = "round_number";

Itamotorino motor_handle = Itamotorino(AIN1, AIN2, BIN1, BIN2, PWMA, PWMB);

EnemyDetector enemy_detector = EnemyDetector();
LineDetector line_detector = LineDetector();
StateMachine state_machine = StateMachine(&enemy_detector.event_group, &line_detector.event_group);

const esp_vfs_littlefs_conf_t vfs_config = {
  .base_path = "/littlefs",
  .partition_label = "telemetry",
  .format_if_mount_failed = false,
  .dont_mount = false
};

NVSHandler match_and_qtr_info = NVSHandler("MatchesAndQTR");
FileHandler file_handle = FileHandler(&vfs_config);

TaskHandle_t motor_task_handle, sensing_task_handle, telemetry_task_handle;

uint16_t round_number;

void SensorsTask(void *pvParameters);
void MotorsTask(void *pvParameters);
void TelemetryTask(void *pvParameters);

void setup() {
  disableCore0WDT();
  disableCore1WDT();
  if (match_and_qtr_info.StartStorage(NVS_READWRITE) == ESP_OK){
    line_detector.Calibrate(QTRCalibrate::kCalibrate, match_and_qtr_info);
    if (match_and_qtr_info.ReadUInt16(round_number_key, &round_number) != ESP_OK){
      round_number = 1;
      match_and_qtr_info.WriteUInt16(round_number_key, &round_number);
    }
    else{
      ++round_number;
      match_and_qtr_info.WriteUInt16(round_number_key, &round_number);
    }
  }
  IrReceiver.begin(IR_PIN, true, LED_BUILTIN);
  IrReceiver.enableIRIn();  
}

void loop() {
}

void SensorsTask(void *pvParameters){
  if (IrReceiver.decode()){
    state_machine.ResolveIRReceiver(IrReceiver.decodedIRData.command);
  }
  enemy_detector.Detect();
  line_detector.Detect();
  state_machine.UpdateState();
}

void MotorsTask(void *pvParameters){
  
}