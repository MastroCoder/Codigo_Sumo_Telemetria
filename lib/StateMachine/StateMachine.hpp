#ifndef LIB_STATE_MACHINE_HPP_
#define LIB_STATE_MACHINE_HPP_

#include "types.hpp"
#include <Arduino.h>
#include "pins.h"
#include "internal_defs.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <map>
#include <array>

class StateMachine {
  private:
    EventBits_t detection_bits;
    EventBits_t line_bits;
    EventGroupHandle_t *detections;
    EventGroupHandle_t *line;
    void UpdateSensorState();
    void UpdateQTRState();
  
  public:
    States states;
    StateMachine(EventGroupHandle_t *detections_handle, EventGroupHandle_t *line_handle);
    void ResolveIRReceiver(uint16_t command);
    void UpdateState();
};

#endif