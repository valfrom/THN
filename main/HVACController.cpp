#include "HVACController.h"

#include <cmath>
#include <limits>

#include "PowerLog.h"
#include "TemperatureLog.h"
#include "ScheduleManager.h"

namespace controller {

namespace {
constexpr unsigned long kControlUpdateIntervalMs = 1000;
constexpr unsigned long kCooldownMinimumRuntimeMs = 5UL * 60UL * 1000UL;
constexpr float kCooldownCoilTemperatureThresholdC = 20.0f;
constexpr float kCooldownTemperatureDeltaThresholdC = 2.0f;
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
  compressorMinAmbientC_ = max(0.0f, minimumAmbient);
}

void HVACController::setCompressorCooldownTemperature(float temperature) {
  if (isnan(temperature) || temperature < 0.0f) {
    return;
  }
  compressorCooldownTemperature_ = temperature;
}

void HVACController::setCompressorCooldownDurationMinutes(float minutes) {
  if (isnan(minutes) || minutes < 0.0f) {
    return;
  }
  compressorCooldownDurationMinutes_ = minutes;
  double durationMs = static_cast<double>(minutes) * 60.0 * 1000.0;
  if (durationMs < 0.0) {
    durationMs = 0.0;
  }
  double maxValue = static_cast<double>(std::numeric_limits<unsigned long>::max());
  if (durationMs > maxValue) {
    durationMs = maxValue;
  }
  compressorCooldownDurationMs_ = static_cast<unsigned long>(durationMs);
}

bool HVACController::compressorCooldownActive() const { return cooldownActive(); }

unsigned long HVACController::compressorCooldownRemainingMs() const {
  if (!cooldownActive()) {
    return 0;
  }
  unsigned long now = millis();
  return compressorCooldownUntil_ > now ? (compressorCooldownUntil_ - now) : 0;
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
  updateCooldownState();
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
  if (cooldownActive()) {
    compressor_.forceOff();
    return;
  }

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

  if (systemMode_ == SystemMode::kHeating && compressor_.isRunning() && sensors_.hasCoil()) {
    float coilTemperature = sensors_.coil().value;
    if (!isnan(coilTemperature) && coilTemperature < kCooldownCoilTemperatureThresholdC &&
        std::fabs(ambient - coilTemperature) < kCooldownTemperatureDeltaThresholdC &&
        compressor_.minimumRuntimeRemaining() == 0 &&
        compressor_.timeSinceLastOn() >= kCooldownMinimumRuntimeMs) {
      compressorCooldownUntil_ = millis() + compressorCooldownDurationMs_;
      compressor_.forceOff();
      return;
    }
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
  if (cooldownActive()) {
    fan_.setRequestedSpeed(FanSpeed::kOff);
    fan_.enforceMinimumSpeed(FanSpeed::kOff);
    fan_.update();
    return;
  }

  FanSpeed desired = FanSpeed::kOff;
  if (systemMode_ == SystemMode::kIdle) {
    desired = FanSpeed::kOff;
  } else {
    switch (fanMode_) {
      case FanMode::kAuto: {
        bool forceLowFan = false;
        if (systemMode_ == SystemMode::kHeating && sensors_.hasCoil()) {
          float coilTemperature = sensors_.coil().value;
          forceLowFan =
              !isnan(coilTemperature) && coilTemperature < compressorCooldownTemperature_;
        }
        FanSpeed autoSpeed = compressor_.isRunning() ? FanSpeed::kMedium : FanSpeed::kLow;
        if (systemMode_ == SystemMode::kHeating && sensors_.hasAmbient() &&
            sensors_.hasCoil()) {
          float ambientTemperature = sensors_.ambient().value;
          float coilTemperature = sensors_.coil().value;
          if (!isnan(ambientTemperature) && !isnan(coilTemperature)) {
            float temperatureDifference = std::fabs(ambientTemperature - coilTemperature);
            if (temperatureDifference < 5.0f) {
              autoSpeed = FanSpeed::kLow;
            } else if (temperatureDifference < 8.0f) {
              autoSpeed = FanSpeed::kMedium;
            } else {
              autoSpeed = FanSpeed::kHigh;
            }
          }
        }
        desired = forceLowFan ? FanSpeed::kLow : autoSpeed;
        break;
      }
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

void HVACController::updateCooldownState() {
  if (compressorCooldownUntil_ == 0) {
    return;
  }
  unsigned long now = millis();
  long remaining = static_cast<long>(compressorCooldownUntil_ - now);
  if (remaining <= 0) {
    compressorCooldownUntil_ = 0;
  }
}

bool HVACController::cooldownActive() const {
  if (compressorCooldownUntil_ == 0) {
    return false;
  }
  unsigned long now = millis();
  long remaining = static_cast<long>(compressorCooldownUntil_ - now);
  return remaining > 0;
}

}  // namespace controller
