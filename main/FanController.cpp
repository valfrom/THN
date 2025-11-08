#include "FanController.h"

namespace controller {

namespace {
void setPin(uint8_t pin, bool on) {
  if (pin == UINT8_MAX) {
    return;
  }
  digitalWrite(pin, on ? LOW : HIGH);
}
}  // namespace

FanController::FanController(const Pins &pins) : pins_(pins) {}

void FanController::begin() {
  if (pins_.low != UINT8_MAX) {
    pinMode(pins_.low, OUTPUT);
    digitalWrite(pins_.low, HIGH);
  }
  if (pins_.medium != UINT8_MAX) {
    pinMode(pins_.medium, OUTPUT);
    digitalWrite(pins_.medium, HIGH);
  }
  if (pins_.high != UINT8_MAX) {
    pinMode(pins_.high, OUTPUT);
    digitalWrite(pins_.high, HIGH);
  }
  currentSpeed_ = FanSpeed::kOff;
}

void FanController::setRequestedSpeed(FanSpeed speed) {
  requestedSpeed_ = speed;
}

void FanController::enforceMinimumSpeed(FanSpeed minimumSpeed) {
  if (static_cast<uint8_t>(minimumSpeed) > static_cast<uint8_t>(minimumSpeed_)) {
    minimumSpeed_ = minimumSpeed;
  }
}

void FanController::update() {
  FanSpeed target = requestedSpeed_;
  if (static_cast<uint8_t>(minimumSpeed_) > static_cast<uint8_t>(target)) {
    target = minimumSpeed_;
  }
  if (target != currentSpeed_) {
    applySpeed(target);
    currentSpeed_ = target;
  }
  // Reset minimum enforcement for next cycle.
  minimumSpeed_ = FanSpeed::kOff;
}

void FanController::applySpeed(FanSpeed speed) {
  // Ensure only one relay is active at a time.
  setPin(pins_.low, speed == FanSpeed::kLow);
  setPin(pins_.medium, speed == FanSpeed::kMedium);
  setPin(pins_.high, speed == FanSpeed::kHigh);
}

}  // namespace controller
