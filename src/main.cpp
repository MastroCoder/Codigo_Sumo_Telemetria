#include <Arduino.h>
#include <QTRSensors.h>
#include "GlobalDefs.hpp"
#include "Detectors.hpp"
#include "StateMachine.hpp"
#include "Itamotorino.h"
#include "FileHandler.hpp"
#include <freertos/FreeRTOS.h>
#include <esp_littlefs.h>
#include <IRremote.hpp>

Itamotorino motor_handle = Itamotorino(AIN1, AIN2, BIN1, BIN2, PWMA, PWMB);

EnemyDetector enemy_detector;
LineDetector line_detector;
StateMachine state_machine = StateMachine(&enemy_detector.event_group, &line_detector.event_group);

const esp_vfs_littlefs_conf_t vfs_config = {
  .base_path = "/",
  .partition_label = "telemetry",
  .format_if_mount_failed = true,
  .dont_mount = false
};

NVSHandler match_info = NVSHandler("Matches");
FileHandler file_handle = FileHandler(&vfs_config);

TaskHandle_t motor_task_handle, sensing_task_handle, telemetry_task_handle;

void setup() {
  disableCore0WDT();
  disableCore1WDT();
  
  // need to initialize all sensors as input

  IrReceiver.begin(IR_PIN, true, LED_BUILTIN);
  IrReceiver.enableIRIn();  
}

void loop() {
}

