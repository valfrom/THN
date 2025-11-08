#include "Compressor.h"

namespace controller {

Compressor::Compressor(uint8_t relayPin,
                       unsigned long minRuntimeMs,
                       unsigned long restartDelayMs)
    : relayPin_(relayPin),
      minRuntimeMs_(minRuntimeMs),
      restartDelayMs_(restartDelayMs) {}

void Compressor::begin() {
  pinMode(relayPin_, OUTPUT);
  digitalWrite(relayPin_, LOW);
  running_ = false;
  requestedOn_ = false;
  lastOffTimestamp_ = millis();
}

void Compressor::requestOn() { requestedOn_ = true; }

void Compressor::requestOff() { requestedOn_ = false; }

void Compressor::forceOff() {
  requestedOn_ = false;
  if (running_) {
    turnOff();
  }
}

void Compressor::update() {
  if (running_) {
    // Already running; ensure minimum runtime before honoring an off request.
    if (!requestedOn_) {
      unsigned long runtime = millis() - lastOnTimestamp_;
      if (runtime >= minRuntimeMs_) {
        turnOff();
      }
    }
  } else {
    // Currently off; honor on request if restart delay has been satisfied.
    if (requestedOn_ && canTurnOn()) {
      turnOn();
    }
  }
}

bool Compressor::canTurnOn() const {
  if (running_) {
    return true;
  }
  unsigned long elapsed = millis() - lastOffTimestamp_;
  return elapsed >= restartDelayMs_;
}

unsigned long Compressor::timeSinceLastOn() const {
  return running_ ? (millis() - lastOnTimestamp_)
                  : (lastOnTimestamp_ == 0 ? 0 : millis() - lastOnTimestamp_);
}

unsigned long Compressor::timeSinceLastOff() const {
  return running_ ? (lastOffTimestamp_ == 0 ? 0 : millis() - lastOffTimestamp_)
                  : (millis() - lastOffTimestamp_);
}

unsigned long Compressor::restartDelayRemaining() const {
  if (running_) {
    return 0;
  }
  unsigned long elapsed = millis() - lastOffTimestamp_;
  if (elapsed >= restartDelayMs_) {
    return 0;
  }
  return restartDelayMs_ - elapsed;
}

void Compressor::turnOn() {
  running_ = true;
  digitalWrite(relayPin_, HIGH);
  lastOnTimestamp_ = millis();
}

void Compressor::turnOff() {
  running_ = false;
  digitalWrite(relayPin_, LOW);
  lastOffTimestamp_ = millis();
}

}  // namespace controller
