#include "HVACController.h"

#include "PowerLog.h"
#include "TemperatureLog.h"
#include "ScheduleManager.h"

namespace controller {

namespace {
constexpr unsigned long kControlUpdateIntervalMs = 1000;
}

HVACController::HVACController(Compressor &compressor,
                               FanController &fan,
                               SensorManager &sensors,
                               scheduler::ScheduleManager &schedule,
                               logging::TemperatureLog &temperatureLog,
                               logging::PowerLog &powerLog)
    : compressor_(compressor),
      fan_(fan),
      sensors_(sensors),
      schedule_(schedule),
      temperatureLog_(temperatureLog),
      powerLog_(powerLog) {}

void HVACController::begin() {
  compressor_.begin();
  fan_.begin();
}

void HVACController::setTargetTemperature(float target) { targetTemperature_ = target; }

void HVACController::setHysteresis(float hysteresis) {
  hysteresis_ = max(0.1f, hysteresis);
}

void HVACController::setCompressorTemperatureLimit(float limit) {
  if (isnan(limit) || limit < 0.0f) {
    return;
  }
  compressorTemperatureLimit_ = limit;
}

void HVACController::setCompressorMinimumAmbient(float minimumAmbient) {
  if (isnan(minimumAmbient)) {
    return;
  }
  compressorMinAmbientC_ = max(7.0f, minimumAmbient);
}

void HVACController::enableScheduling(bool enabled) { schedulingEnabled_ = enabled; }

void HVACController::setFanMode(FanMode mode) { fanMode_ = mode; }

void HVACController::setSystemMode(SystemMode mode) {
  systemMode_ = mode;
  if (systemMode_ == SystemMode::kIdle || systemMode_ == SystemMode::kFanOnly) {
    compressor_.requestOff();
  }
}

void HVACController::update() {
  unsigned long now = millis();
  if ((now - lastControlUpdate_) < kControlUpdateIntervalMs) {
    compressor_.update();
    updateFanState();
    return;
  }
  lastControlUpdate_ = now;

  sensors_.update();
  applyControlLogic();
  compressor_.update();
  updateFanState();
  logState();
}

void HVACController::applyControlLogic() {
  if (systemMode_ != SystemMode::kCooling && systemMode_ != SystemMode::kHeating) {
    compressor_.requestOff();
    return;
  }

  if (sensors_.hasCoil()) {
    float coilTemperature = sensors_.coil().value;
    if (!isnan(coilTemperature) && coilTemperature >= compressorTemperatureLimit_) {
      compressor_.forceOff();
      return;
    }
  }

  if (!sensors_.hasAmbient()) {
    compressor_.requestOff();
    return;
  }

  float ambient = sensors_.ambient().value;
  if (isnan(ambient)) {
    compressor_.requestOff();
    return;
  }
  if (ambient < compressorMinAmbientC_) {
    compressor_.requestOff();
    return;
  }
  float upper = targetTemperature_ + (hysteresis_ / 2.0f);
  float lower = targetTemperature_ - (hysteresis_ / 2.0f);

  if (systemMode_ == SystemMode::kCooling) {
    if (compressor_.isRunning()) {
      if (ambient <= lower) {
        compressor_.requestOff();
      } else {
        compressor_.requestOn();
      }
    } else {
      if (ambient >= upper) {
        compressor_.requestOn();
      } else {
        compressor_.requestOff();
      }
    }
  } else {  // Heating
    if (compressor_.isRunning()) {
      if (ambient >= upper) {
        compressor_.requestOff();
      } else {
        compressor_.requestOn();
      }
    } else {
      if (ambient <= lower) {
        compressor_.requestOn();
      } else {
        compressor_.requestOff();
      }
    }
  }
}

void HVACController::updateFanState() {
  FanSpeed desired = FanSpeed::kOff;
  if (systemMode_ == SystemMode::kIdle) {
    desired = FanSpeed::kOff;
  } else {
    switch (fanMode_) {
      case FanMode::kAuto:
        desired = compressor_.isRunning() ? FanSpeed::kMedium : FanSpeed::kLow;
        break;
      case FanMode::kOff:
        desired = FanSpeed::kOff;
        break;
      case FanMode::kLow:
        desired = FanSpeed::kLow;
        break;
      case FanMode::kMedium:
        desired = FanSpeed::kMedium;
        break;
      case FanMode::kHigh:
        desired = FanSpeed::kHigh;
        break;
    }
  }

  fan_.setRequestedSpeed(desired);

  if ((systemMode_ == SystemMode::kCooling || systemMode_ == SystemMode::kHeating) &&
      (compressor_.isRunning() || compressor_.isRequested())) {
    fan_.enforceMinimumSpeed(FanSpeed::kLow);
  }

  fan_.update();
}

void HVACController::logState() {
  unsigned long timestamp = millis();
  if (sensors_.hasAmbient() || sensors_.hasCoil()) {
    float ambient = sensors_.hasAmbient() ? sensors_.ambient().value : NAN;
    float coil = sensors_.hasCoil() ? sensors_.coil().value : NAN;
    temperatureLog_.addReading(timestamp, ambient, coil);
  }
  powerLog_.logState(timestamp, fan_.currentSpeed(), compressor_.isRunning());
}

}  // namespace controller
