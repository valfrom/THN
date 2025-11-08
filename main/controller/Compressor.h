#pragma once

#include <Arduino.h>

namespace controller {

/**
 * Controls the compressor relay while enforcing runtime and restart delays.
 */
class Compressor {
 public:
  Compressor(uint8_t relayPin,
             unsigned long minRuntimeMs = 5UL * 60UL * 1000UL,
             unsigned long restartDelayMs = 120UL * 1000UL);

  void begin();

  /** Request that the compressor be on. */
  void requestOn();

  /** Request that the compressor be off. */
  void requestOff();

  /** Immediately turns the compressor off and clears requests. */
  void forceOff();

  /** Updates the relay pin based on the requested state and timing limits. */
  void update();

  bool isRunning() const { return running_; }

  bool isRequested() const { return requestedOn_; }

  bool canTurnOn() const;

  unsigned long timeSinceLastOn() const;

  unsigned long timeSinceLastOff() const;

 private:
  void turnOn();
  void turnOff();

  uint8_t relayPin_;
  unsigned long minRuntimeMs_;
  unsigned long restartDelayMs_;

  bool running_ = false;
  bool requestedOn_ = false;

  unsigned long lastOnTimestamp_ = 0;
  unsigned long lastOffTimestamp_ = 0;
};

}  // namespace controller
