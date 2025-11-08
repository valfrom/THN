#pragma once

#include <Arduino.h>

namespace controller {

using TemperatureReader = float (*)();

struct TemperatureSample {
  float value = NAN;
  unsigned long timestamp = 0;
};

/**
 * Manages temperature sensors for ambient air and compressor coil readings.
 */
class SensorManager {
 public:
  SensorManager();

  void setAmbientReader(TemperatureReader reader);
  void setCoilReader(TemperatureReader reader);

  void update();

  bool hasAmbient() const { return !isnan(ambient_.value); }
  bool hasCoil() const { return !isnan(coil_.value); }

  TemperatureSample ambient() const { return ambient_; }
  TemperatureSample coil() const { return coil_; }

 private:
  TemperatureReader ambientReader_ = nullptr;
  TemperatureReader coilReader_ = nullptr;

  TemperatureSample ambient_;
  TemperatureSample coil_;
};

}  // namespace controller
