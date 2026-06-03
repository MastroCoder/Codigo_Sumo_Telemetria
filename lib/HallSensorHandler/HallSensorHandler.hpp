#ifndef LIB_HALL_SENSOR_HANDLER_HPP_
#define LIB_HALL_SENSOR_HANDLER_HPP_

#include <Arduino.h>
#include "GlobalDefs.hpp"
#include <utility>

class HallSensorHandler {
  private:
    unsigned long start_time;
    static volatile unsigned char count_left;
    static volatile unsigned char count_right;
    static void IRAM_ATTR IncrementLeft();
    static void IRAM_ATTR IncrementRight();
  public:
    HallSensorHandler();
    void Init();
    std::pair<unsigned int, unsigned int> CalculateRPM(long state_machine_start_time);
};

#endif