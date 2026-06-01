#ifndef LIB_STRATEGY_RUNNER_HPP_
#define LIB_STRATEGY_RUNNER_HPP

#include "GlobalDefs.hpp"
#include "Itamotorino.h"
#include "StateMachine.hpp"
#include <unordered_map>
#include <utility>

const std::unordered_map<MotorSpeeds, std::pair<int, int>> speeds = {
  {MotorSpeeds::kForwardFast, {255, 255}},
  {MotorSpeeds::kForwardSlow, {128, 128}},
  {MotorSpeeds::kStopped,     {0, 0}},
  {MotorSpeeds::kReverseSlow, {-128, -128}},
  {MotorSpeeds::kReverseFast, {-255, -255}},
  {MotorSpeeds::kLeftSlow,    {-128, 128}},
  {MotorSpeeds::kLeftFast,    {-255, 255}},
  {MotorSpeeds::kRightSlow,   {128, -128}},
  {MotorSpeeds::kRightFast,   {255, -255}},
};

class StrategyRunner {
  private:
    Itamotorino motor_handle = Itamotorino(AIN1, AIN2, BIN1, BIN2, PWMA, PWMB);
    void FollowEnemy(StateMachine &state_machine);
    void SearchLeft(StateMachine &state_machine);
    void SearchRight(StateMachine &state_machine);

  public:
    StrategyRunner();
    void RunStrategy(StateMachine &state_machine);
    void SetMotors(MotorSpeeds m);
};

#endif