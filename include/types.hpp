#ifndef INCLUDE_TYPES_HPP_
#define INCLUDE_TYPES_HPP_

enum class FightState : unsigned char {
  kStandby = 0x0,
  kFighting,
  kStop
};

enum class Strategy : unsigned char {
  kSearchLeft = 0x3,
  kSearchRight,
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

enum class QTRCalibrate : unsigned char {
  kUseNVS,
  kCalibrate
};

struct States {
  FightState fight_state {FightState::kStandby};
  Strategy strategy {Strategy::kSearchLeft};
  SensorState sensor_state {SensorState::kNone};
  QTRState qtr_state {QTRState::kNone};
};

#endif