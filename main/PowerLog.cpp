#include "PowerLog.h"

namespace logging {

PowerLog::PowerLog() = default;

void PowerLog::setConsumptionTable(const ConsumptionRate *rates, size_t count) {
  rates_ = rates;
  rateCount_ = count;
}

void PowerLog::logState(unsigned long timestamp,
                        controller::FanSpeed fanSpeed,
                        bool compressorActive) {
  float watts = lookupWatts(fanSpeed, compressorActive);

  if (!initialized_) {
    lastTimestamp_ = timestamp;
    lastWatts_ = watts;
    initialized_ = true;
  }

  unsigned long deltaMs = timestamp - lastTimestamp_;
  float hours = static_cast<float>(deltaMs) / (1000.0f * 60.0f * 60.0f);
  totalEnergyWh_ += lastWatts_ * hours;

  entries_[head_] = {timestamp, totalEnergyWh_, watts, fanSpeed, compressorActive};
  head_ = (head_ + 1) % kMaxEntries;
  if (count_ < kMaxEntries) {
    ++count_;
  }

  lastTimestamp_ = timestamp;
  lastWatts_ = watts;
}

float PowerLog::lookupWatts(controller::FanSpeed fanSpeed, bool compressorActive) const {
  if (!rates_ || rateCount_ == 0) {
    // Fallback generic estimates.
    if (compressorActive) {
      return 1500.0f;
    }
    switch (fanSpeed) {
      case controller::FanSpeed::kHigh:
        return 100.0f;
      case controller::FanSpeed::kMedium:
        return 75.0f;
      case controller::FanSpeed::kLow:
        return 50.0f;
      case controller::FanSpeed::kOff:
      default:
        return 5.0f;
    }
  }

  for (size_t i = 0; i < rateCount_; ++i) {
    const ConsumptionRate &rate = rates_[i];
    if (rate.fanSpeed == fanSpeed && rate.compressorActive == compressorActive) {
      return rate.watts;
    }
  }

  // If no exact match, return best-effort fallback.
  return compressorActive ? 1500.0f : 25.0f;
}

}  // namespace logging
