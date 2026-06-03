#ifndef LIB_GLOBAL_DEFS_HPP_
#define LIB_GLOBAL_DEFS_HPP_

#define QTR1 36u
#define QTR2 39u

#define PWMA 4u
#define PWMB 21u
#define AIN1 16u
#define AIN2 22u
#define BIN1 23u
#define BIN2 5u

#define S1 32u
#define S2 33u
#define S3 25u
#define S4 27u

#define IR_PIN 17u

#define H1 34u
#define H2 19u

#define PWM_FREQ 1000
#define PWM_RES 8
#define PWM_CH1 1
#define PWM_CH2 2

#define MAX_STACK_DEPTH 10000u

#define S1_BIT (1<<0)
#define S2_BIT (1<<1)
#define S3_BIT (1<<2)
#define S4_BIT (1<<3)

#define QTR1_BIT (1<<0)
#define QTR2_BIT (1<<1)

#define QTR_COUNT 2

#define API_URL "CORRECT_IP:5000"
#define MAX_HTTP_BUFFER 2048 

enum class RobotTask : unsigned char {
  kRunMatches = 0x42, 
  kSendData
};

enum class FightState : unsigned char {
  kStandby = 0x0,
  kFighting,
  kStop
};

enum class Strategy : unsigned char {
  kSearchLeft = 0x3,
  kSearchRight,
  kFollowEnemy,
};

enum class SensorState : unsigned char {
  kLeft,
  kFrontLeft,
  kFront,
  kFrontRight,
  kRight,
  kNone
};

enum class QTRState : unsigned char {
  kLeft,
  kRight,
  kBoth,
  kNone
};

enum class MotorSpeeds : unsigned char {
  kReverseFast,
  kReverseSlow,
  kStopped,
  kForwardSlow,
  kForwardFast,
  kLeftSlow,
  kLeftFast,
  kRightSlow,
  kRightFast
};

struct States {
  RobotTask robot_task {RobotTask::kRunMatches};
  FightState fight_state {FightState::kStandby};
  Strategy strategy {Strategy::kSearchLeft};
  SensorState sensor_state {SensorState::kNone};
  QTRState qtr_state {QTRState::kNone};
};
#endif