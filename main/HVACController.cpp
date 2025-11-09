#include "HVACController.h"

#include <time.h>
#include <math.h>

#include "PowerLog.h"
#include "TemperatureLog.h"
#include "ScheduleManager.h"

namespace controller {

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

void HVACController::enableScheduling(bool enabled) {
  schedulingEnabled_ = enabled;
  scheduleTargetValid_ = false;
  lastScheduledModeSpecified_ = false;
  lastScheduleCheckMs_ = 0;
  lastScheduleCheckMinute_ = -1;
}

void HVACController::setFanMode(FanMode mode) { fanMode_ = mode; }

void HVACController::setSystemMode(SystemMode mode) {
  systemMode_ = mode;
  if (systemMode_ == SystemMode::kIdle || systemMode_ == SystemMode::kFanOnly) {
    compressor_.requestOff();
  }
}

void HVACController::update() {
  sensors_.update();
  pollSchedule();
  applyControlLogic();
  compressor_.update();
  updateFanState();
  logState();
  lastControlUpdate_ = millis();
}

void HVACController::pollSchedule() { updateTargetsFromSchedule(); }

void HVACController::updateTargetsFromSchedule() {
  if (!schedulingEnabled_) {
    return;
  }
  time_t now = time(nullptr);
  int minutesOfDay = -1;
  if (now != (time_t)-1) {
    tm *timeinfoPtr = localtime(&now);
    if (timeinfoPtr != nullptr) {
      minutesOfDay = static_cast<int>(timeinfoPtr->tm_hour) * 60 +
                     static_cast<int>(timeinfoPtr->tm_min);
    }
  }

  unsigned long nowMs = millis();
  if (scheduleTargetValid_) {
    bool minuteChanged = minutesOfDay != lastScheduleCheckMinute_ &&
                         minutesOfDay >= 0;
    constexpr unsigned long kScheduleCheckIntervalMs = 15000;
    if (!minuteChanged && (nowMs - lastScheduleCheckMs_) < kScheduleCheckIntervalMs) {
      return;
    }
  }
  lastScheduleCheckMs_ = nowMs;

  scheduler::ScheduleTarget scheduled = schedule_.targetFor(now);
  constexpr float kTemperatureTolerance = 0.05f;
  bool scheduleTemperatureChanged =
      !scheduleTargetValid_ ||
      fabsf(lastScheduledTemperature_ - scheduled.temperature) > kTemperatureTolerance;
  bool temperatureDifferent = isnan(targetTemperature_) ||
                              fabsf(targetTemperature_ - scheduled.temperature) >
                                  kTemperatureTolerance;

  bool modeSpecified = scheduled.mode != scheduler::ScheduledMode::kUnspecified;
  SystemMode desiredMode = systemMode_;
  if (modeSpecified) {
    switch (scheduled.mode) {
      case scheduler::ScheduledMode::kCooling:
        desiredMode = SystemMode::kCooling;
        break;
      case scheduler::ScheduledMode::kHeating:
        desiredMode = SystemMode::kHeating;
        break;
      case scheduler::ScheduledMode::kFanOnly:
        desiredMode = SystemMode::kFanOnly;
        break;
      case scheduler::ScheduledMode::kIdle:
        desiredMode = SystemMode::kIdle;
        break;
      case scheduler::ScheduledMode::kUnspecified:
        break;
    }
  }

  bool modeChangedFromSchedule = !scheduleTargetValid_ ||
                                 (modeSpecified != lastScheduledModeSpecified_) ||
                                 (modeSpecified && lastScheduledSystemMode_ != desiredMode);

  bool shouldUpdateTarget = temperatureDifferent || scheduleTemperatureChanged ||
                            (modeSpecified ? modeChangedFromSchedule :
                                             lastScheduledModeSpecified_);

  if (shouldUpdateTarget) {
    setTargetTemperature(scheduled.temperature);
    lastScheduledTemperature_ = scheduled.temperature;
  }

  if (modeSpecified) {
    if (modeChangedFromSchedule || systemMode_ != desiredMode) {
      setSystemMode(desiredMode);
    }
    lastScheduledSystemMode_ = desiredMode;
    lastScheduledModeSpecified_ = true;
  } else {
    lastScheduledModeSpecified_ = false;
  }

  scheduleTargetValid_ = true;
  if (minutesOfDay >= 0) {
    lastScheduleCheckMinute_ = minutesOfDay;
  }
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
