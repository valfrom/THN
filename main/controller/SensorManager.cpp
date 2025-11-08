#include "SensorManager.h"

namespace controller {

SensorManager::SensorManager() = default;

void SensorManager::setAmbientReader(TemperatureReader reader) {
  ambientReader_ = reader;
}

void SensorManager::setCoilReader(TemperatureReader reader) {
  coilReader_ = reader;
}

void SensorManager::update() {
  if (ambientReader_) {
    float value = ambientReader_();
    if (!isnan(value)) {
      ambient_.value = value;
      ambient_.timestamp = millis();
    }
  }
  if (coilReader_) {
    float value = coilReader_();
    if (!isnan(value)) {
      coil_.value = value;
      coil_.timestamp = millis();
    }
  }
}

}  // namespace controller
