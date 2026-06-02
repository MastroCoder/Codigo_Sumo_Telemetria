#include "HallSensorHandler.hpp"

volatile unsigned char HallSensorHandler::count_left;
volatile unsigned char HallSensorHandler::count_right;

HallSensorHandler::HallSensorHandler(){
  // H1 e H2, se nos GPIOs 34 e 35, necessitam de pullup externo.
  pinMode(H1, INPUT);
  pinMode(H2, INPUT);

  attachInterrupt(H1, IncrementLeft, RISING);
  attachInterrupt(H2, IncrementRight, RISING);
  start_time = 0;
}

void IRAM_ATTR HallSensorHandler::IncrementLeft(){
  count_left++;
}

void IRAM_ATTR HallSensorHandler::IncrementRight(){
  count_right++;
}

std::pair<unsigned int, unsigned int> HallSensorHandler::CalculateRPM(long state_machine_start_time){
  std::pair<unsigned int, unsigned int> rpm_measurements;
  if (start_time == 0) start_time = state_machine_start_time;
  unsigned long time_diff = millis() - start_time;
  start_time = millis();
  if (time_diff > 0){
    rpm_measurements = {(60  * 1000 * count_left) / time_diff, (60  * 1000 * count_right) / time_diff};
  }
  count_left = count_right = 0;
  return rpm_measurements;
}