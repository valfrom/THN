#pragma once

#include <Arduino.h>

#include "FanController.h"

namespace logging {

class PowerLog {
 public:
  struct ConsumptionRate {
    controller::FanSpeed fanSpeed;
    bool compressorActive;
    float watts;
  };

  struct Entry {
    unsigned long timestamp;
    float energyWhAccumulated;
    float instantaneousWatts;
    controller::FanSpeed fanSpeed;
    bool compressorActive;
  };

  static constexpr size_t kMaxEntries = 120;

  PowerLog();

  void setConsumptionTable(const ConsumptionRate *rates, size_t count);

  void logState(unsigned long timestamp,
                controller::FanSpeed fanSpeed,
                bool compressorActive);

  size_t size() const { return count_; }

  template <typename Callback>
  void forEach(Callback callback) const {
    size_t processed = 0;
    while (processed < count_) {
      size_t index = (head_ + kMaxEntries - count_ + processed) % kMaxEntries;
      callback(entries_[index]);
      ++processed;
    }
  }

  float totalEnergyWh() const { return totalEnergyWh_; }

 private:
  float lookupWatts(controller::FanSpeed fanSpeed, bool compressorActive) const;

  void ensureMinute(unsigned long minute);
  void resetMinuteAggregates();
  void accumulateSegment(unsigned long durationMs,
                         float watts,
                         controller::FanSpeed fanSpeed,
                         bool compressorActive);
  controller::FanSpeed dominantFanSpeed() const;

  Entry entries_[kMaxEntries];
  size_t head_ = 0;
  size_t count_ = 0;

  const ConsumptionRate *rates_ = nullptr;
  size_t rateCount_ = 0;

  unsigned long lastTimestamp_ = 0;
  float lastWatts_ = 0.0f;
  float totalEnergyWh_ = 0.0f;
  bool initialized_ = false;

  unsigned long currentMinute_ = 0;
  bool hasCurrentMinute_ = false;
  size_t currentIndex_ = kMaxEntries;
  float wattMillisAccumulated_ = 0.0f;
  unsigned long durationMsAccumulated_ = 0;
  unsigned long fanDurationMs_[4] = {0, 0, 0, 0};
  unsigned long compressorOnDurationMs_ = 0;
  controller::FanSpeed lastFanSpeedState_ = controller::FanSpeed::kOff;
  bool lastCompressorState_ = false;
};

}  // namespace logging
