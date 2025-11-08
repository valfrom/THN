#pragma once

#include <Arduino.h>

namespace controller {

enum class FanSpeed : uint8_t { kOff = 0, kLow = 1, kMedium = 2, kHigh = 3 };

/**
 * Controls the indoor fan via individual relay pins.
 *
 * The controller assumes latching relays where HIGH energizes the relay.
 */
class FanController {
 public:
  struct Pins {
    uint8_t low;
    uint8_t medium;
    uint8_t high;
  };

  explicit FanController(const Pins &pins);

  void begin();

  void setRequestedSpeed(FanSpeed speed);
  FanSpeed requestedSpeed() const { return requestedSpeed_; }

  void enforceMinimumSpeed(FanSpeed minimumSpeed);

  FanSpeed currentSpeed() const { return currentSpeed_; }

  void update();

 private:
  void applySpeed(FanSpeed speed);

  Pins pins_;
  FanSpeed requestedSpeed_ = FanSpeed::kOff;
  FanSpeed minimumSpeed_ = FanSpeed::kOff;
  FanSpeed currentSpeed_ = FanSpeed::kOff;
};

}  // namespace controller
