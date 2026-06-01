#include "StrategyRunner.hpp"

StrategyRunner::StrategyRunner(){
  motor_handle.setupADC(PWM_CH1, PWM_FREQ, PWM_RES, PWM_CH2, PWM_FREQ, PWM_RES);
}

void StrategyRunner::RunStrategy(StateMachine &state_machine){
  switch (state_machine.states.strategy){
    case Strategy::kSearchLeft:
      SearchLeft(state_machine);
      break;
    case Strategy::kSearchRight:
      SearchRight(state_machine);
      break;
    case Strategy::kFollowEnemy:
      FollowEnemy(state_machine);
      break;
  }
}

void StrategyRunner::SearchLeft(StateMachine &state_machine){
  state_machine.UpdateState();
  if (state_machine.states.sensor_state == SensorState::kNone && state_machine.states.fight_state == FightState::kFighting){
    motor_handle.setSpeeds(speeds.at(MotorSpeeds::kLeftSlow).first, speeds.at(MotorSpeeds::kLeftSlow).second);
  }
  else{
    while (state_machine.states.fight_state == FightState::kFighting){
      FollowEnemy(state_machine);
    }
  }
}

void StrategyRunner::SearchRight(StateMachine &state_machine){
  state_machine.UpdateState();
  if (state_machine.states.sensor_state == SensorState::kNone && state_machine.states.fight_state == FightState::kFighting){
    motor_handle.setSpeeds(speeds.at(MotorSpeeds::kRightSlow).first, speeds.at(MotorSpeeds::kRightSlow).second);
  }
  else{
    while (state_machine.states.fight_state == FightState::kFighting){
      FollowEnemy(state_machine);
    }
  }
}

// essa bomba vai ter que ser trocada de contexto pra fazer a telemetria. Puta merda...
void StrategyRunner::FollowEnemy(StateMachine &state_machine){
  std::pair<int, int> speed = speeds.at(MotorSpeeds::kForwardSlow);
  if (state_machine.states.fight_state == FightState::kFighting){
    state_machine.UpdateState();
    if (state_machine.states.qtr_state != QTRState::kNone){
      switch (state_machine.states.qtr_state){
        case QTRState::kLeft:
          motor_handle.setSpeeds(speeds.at(MotorSpeeds::kRightFast).first, speeds.at(MotorSpeeds::kRightFast).second);
          break;
        case QTRState::kRight:
          motor_handle.setSpeeds(speeds.at(MotorSpeeds::kLeftFast).first, speeds.at(MotorSpeeds::kLeftFast).second);
          break;
        case QTRState::kBoth:
          motor_handle.setSpeeds(speeds.at(MotorSpeeds::kReverseFast).first, speeds.at(MotorSpeeds::kReverseFast).second);
          break;
      }
    }
    else{
      switch (state_machine.states.sensor_state){
        case SensorState::kFront:
          speed = speeds.at(MotorSpeeds::kForwardFast);
          break;
        case SensorState::kFrontLeft:
          speed = speeds.at(MotorSpeeds::kLeftSlow);
          break;
        case SensorState::kFrontRight:
          speed = speeds.at(MotorSpeeds::kRightSlow);
          break;
        case SensorState::kLeft:
          speed = speeds.at(MotorSpeeds::kLeftFast);
          break;
        case SensorState::kRight:
          speed = speeds.at(MotorSpeeds::kRightFast);
          break;
        case SensorState::kNone:
          break;
      }
      motor_handle.setSpeeds(speed.first, speed.second);
    }
  }
}

void StrategyRunner::SetMotors(MotorSpeeds m){
  motor_handle.setSpeeds(speeds.at(m).first, speeds.at(m).second); 
}