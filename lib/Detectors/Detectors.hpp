#ifndef LIB_DETECTORS_HPP_
#define LIB_DETECTORS_HPP_

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "types.hpp"
#include <QTRSensors.h>
#include <map>
#include "pins.h"
#include "internal_defs.h"
#include <Arduino.h>
#include "NVSHandler.hpp"

class Detector {
  public:
    EventGroupHandle_t event_group;
    Detector();
    ~Detector() = default;
    virtual void Detect() = 0;
};

class EnemyDetector : public Detector {
  public:
    EnemyDetector();
    void Detect() override; 
};

class LineDetector : public Detector {
  private:
    QTRSensors qtr;
    uint16_t qtr_values[QTR_COUNT];
    // Isso deveria ser visível ao usuário?
    const char* kMinOnKeys[QTR_COUNT] = {"min_on_1", "min_on_2"};
    const char* kMaxOnKeys[QTR_COUNT] = {"max_on_1", "max_on_2"};
    const char* kMinOffKeys[QTR_COUNT] = {"min_off_1", "min_off_2"};
    const char* kMaxOffKeys[QTR_COUNT] = {"max_off_1", "max_off_2"};
    NVSHandler nvs = NVSHandler("QTR"); // meio feio, não? Melhor definir um default e um pra usar bonitinho?

  public:
    LineDetector();
    void Calibrate(QTRCalibrate option);
    void Detect() override;
};

#endif