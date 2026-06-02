#include "StateMachine.hpp"

constexpr EventBits_t sensor_bits = S1_BIT | S2_BIT | S3_BIT | S4_BIT;

const std::map<uint16_t, FightState> command_to_fight_state = {
  {static_cast<uint16_t>(FightState::kStandby), FightState::kStandby}, 
  {static_cast<uint16_t>(FightState::kFighting), FightState::kFighting},
  {static_cast<uint16_t>(FightState::kStop), FightState::kStop},
};

const std::map<uint16_t, Strategy> command_to_strategy = {
  {static_cast<uint16_t>(Strategy::kSearchLeft), Strategy::kSearchLeft},
  {static_cast<uint16_t>(Strategy::kSearchRight), Strategy::kSearchRight},
  {static_cast<uint16_t>(Strategy::kFollowEnemy), Strategy::kFollowEnemy} 
};

const std::map<uint16_t, RobotTask> command_to_robot_task = {
  {static_cast<uint16_t>(RobotTask::kRunMatches), RobotTask::kRunMatches},
  {static_cast<uint16_t>(RobotTask::kSendData), RobotTask::kSendData}
};

StateMachine::StateMachine(EventGroupHandle_t detections_handle, EventGroupHandle_t line_handle){
  detections = detections_handle;
  line=line_handle;
  start_time = 0; 
}

void StateMachine::ResolveIRReceiver(uint16_t command){
  if (states.robot_task == RobotTask::kRunMatches){
    Serial.print("Command: ");
    Serial.println(command);
    if (command_to_fight_state.find(command) != command_to_fight_state.end()){
      switch (command_to_fight_state.find(command)->second){
        case FightState::kStop:
          states.fight_state = FightState::kStop;
          states.robot_task = RobotTask::kSendData;
          break;
        default:
          if (states.fight_state == FightState::kStandby){
            states.fight_state = command_to_fight_state.find(command)->second;
            if (states.fight_state == FightState::kFighting && start_time == 0){
              start_time = millis();
            }
          }
          break;
      }
      Serial.println(static_cast<uint16_t>(states.fight_state)); 
    }
    else if (command_to_strategy.find(command) != command_to_strategy.end()){
      if (states.fight_state == FightState::kStandby) states.strategy = command_to_strategy.find(command)->second;
    }
    else if (command_to_robot_task.find(command) != command_to_robot_task.end()){
      if (states.fight_state == FightState::kStandby) states.robot_task = command_to_robot_task.find(command)->second;
    }
  }
}

// Talvez isso deevesse ser feito com interrupts? Interrompe, faz, depois volta à escrita de logs.
void StateMachine::UpdateSensorState(){
  if (detections == nullptr) return;
  
  detection_bits = xEventGroupWaitBits(detections, sensor_bits, pdFALSE, pdTRUE, pdMS_TO_TICKS(5));

  switch (detection_bits) {
    case 0b0010:
    case 0b0110:
    case 0b0100:
      states.sensor_state = SensorState::kFront;
      break;
    case 0b0011:
      states.sensor_state = SensorState::kFrontLeft;
      break;
    case 0b1100:
      states.sensor_state = SensorState::kFrontRight;
      break;
    case 0b0001:
      states.sensor_state = SensorState::kLeft;
      break;
    case 0b1000:
      states.sensor_state = SensorState::kRight;
    default:
      break;
  }
}

void StateMachine::UpdateQTRState(){
  if (line == nullptr) return;

  line_bits = xEventGroupWaitBits(line, QTR1_BIT | QTR2_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(5));

  switch (line_bits){
    case 0b01:
      states.qtr_state = QTRState::kLeft;
      break;
    case 0b10:
      states.qtr_state = QTRState::kRight;
      break;
    case 0b11:
      states.qtr_state = QTRState::kBoth;
      break;
    case 0b00:
    default:
      states.qtr_state = QTRState::kNone;
      break;
  }
}

void StateMachine::UpdateState(){
  if (states.robot_task == RobotTask::kRunMatches){
    UpdateQTRState();
    UpdateSensorState();
  }
  else return; // Programar depois
}