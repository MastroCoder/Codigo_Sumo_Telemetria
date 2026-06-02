#include "Detectors.hpp"

const std::map<unsigned int, unsigned int> sensor_to_bit = {
  {S1, S1_BIT}, {S2, S2_BIT}, {S3, S3_BIT}, {S4, S4_BIT}
};

const unsigned int qtr_bits[QTR_COUNT] = {QTR1_BIT, QTR2_BIT};

Detector::Detector(){
  event_group = xEventGroupCreate();
}

EnemyDetector::EnemyDetector(){
  for (const auto &[sensor, _] : sensor_to_bit){
    pinMode(sensor, INPUT);
  }
}

void EnemyDetector::Detect(){
  for (const auto &[sensor, bit] : sensor_to_bit){
    if (digitalRead(sensor)) xEventGroupSetBits(event_group, bit);
    else xEventGroupClearBits(event_group, bit);
  }
};

LineDetector::LineDetector(){
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){QTR1, QTR2}, QTR_COUNT);
  for (auto &i : (const uint8_t[]){QTR1, QTR2}){
    Serial.println(i);
  }
}

// provavelmente deveria ter salvaguardas se chamarem a funcao mais de uma vez
void LineDetector::Calibrate(QTRCalibrate option, NVSHandler &nvs){
  if (option == QTRCalibrate::kUseNVS){
    for (uint8_t i = 0; i < QTR_COUNT; i++){
      nvs.ReadUInt16(kMinOnKeys[i], &qtr.calibrationOn.minimum[i]);
      nvs.ReadUInt16(kMaxOnKeys[i], &qtr.calibrationOn.maximum[i]);
    }
  }
  else if (option == QTRCalibrate::kCalibrate){
    for(uint16_t i = 0; i <= 800; i++){
      qtr.calibrate();
    }
    Serial.println("Calibrated sensors, values: ");
    for (uint8_t i = 0; i < QTR_COUNT; i++){
      Serial.println(qtr.calibrationOn.minimum[i]);
      nvs.WriteUInt16(kMinOnKeys[i], &qtr.calibrationOn.minimum[i]);
      Serial.println(qtr.calibrationOn.maximum[i]);
      nvs.WriteUInt16(kMaxOnKeys[i], &qtr.calibrationOn.maximum[i]);
    }
  }
}

void LineDetector::Detect(){
  qtr.readCalibrated(qtr_values);
  for (uint8_t i = 0; i < QTR_COUNT; i++){
    if (qtr_values[i] >= 750) xEventGroupSetBits(event_group, qtr_bits[i]);
    else xEventGroupClearBits(event_group, qtr_bits[i]);
  }  
}