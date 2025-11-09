#pragma once

#include <Arduino.h>

#include "Compressor.h"
#include "FanController.h"
#include "SensorManager.h"

namespace scheduler {
class ScheduleManager;
}

namespace logging {
class TemperatureLog;
class PowerLog;
}

namespace controller {

enum class FanMode : uint8_t { kAuto, kOff, kLow, kMedium, kHigh };

enum class SystemMode : uint8_t { kCooling, kHeating, kFanOnly, kIdle };

class HVACController {
 public:
  HVACController(Compressor &compressor,
                 FanController &fan,
                 SensorManager &sensors,
                 scheduler::ScheduleManager &schedule,
                 logging::TemperatureLog &temperatureLog,
                 logging::PowerLog &powerLog);

  void begin();

  void setTargetTemperature(float target);
  float targetTemperature() const { return targetTemperature_; }

  void setHysteresis(float hysteresis);
  float hysteresis() const { return hysteresis_; }

  void enableScheduling(bool enabled);
  bool schedulingEnabled() const { return schedulingEnabled_; }

  void setFanMode(FanMode mode);
  FanMode fanMode() const { return fanMode_; }

  void setSystemMode(SystemMode mode);
  SystemMode systemMode() const { return systemMode_; }

  void setCompressorTemperatureLimit(float limit);
  float compressorTemperatureLimit() const { return compressorTemperatureLimit_; }

  SensorManager &sensors() { return sensors_; }
  const SensorManager &sensors() const { return sensors_; }

  Compressor &compressor() { return compressor_; }
  const Compressor &compressor() const { return compressor_; }

  FanController &fan() { return fan_; }
  const FanController &fan() const { return fan_; }

  void update();

  bool compressorRunning() const { return compressor_.isRunning(); }

 private:
  void updateTargetsFromSchedule();
  void applyControlLogic();
  void updateFanState();
  void logState();

  Compressor &compressor_;
  FanController &fan_;
  SensorManager &sensors_;
  scheduler::ScheduleManager &schedule_;
  logging::TemperatureLog &temperatureLog_;
  logging::PowerLog &powerLog_;

  float targetTemperature_ = 23.0f;  // Celsius default
  float hysteresis_ = 1.0f;
  float compressorTemperatureLimit_ = 60.0f;
  FanMode fanMode_ = FanMode::kAuto;
  SystemMode systemMode_ = SystemMode::kCooling;
  bool schedulingEnabled_ = false;

  unsigned long lastControlUpdate_ = 0;
};

}  // namespace controller
