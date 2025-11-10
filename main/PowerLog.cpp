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
    lastFanSpeedState_ = fanSpeed;
    lastCompressorState_ = compressorActive;
    initialized_ = true;
    return;
  }

  unsigned long start = lastTimestamp_;
  unsigned long end = timestamp;
  if (end < start) {
    start = end;
  }

  while (start < end) {
    unsigned long minute = start / 60000UL;
    ensureMinute(minute);
    unsigned long minuteEnd = (minute + 1) * 60000UL;
    unsigned long segmentEnd = end < minuteEnd ? end : minuteEnd;
    unsigned long segmentDuration = segmentEnd - start;
    if (segmentDuration > 0) {
      accumulateSegment(segmentDuration, lastWatts_, lastFanSpeedState_, lastCompressorState_);
    }
    start = segmentEnd;
  }

  lastTimestamp_ = timestamp;
  lastWatts_ = watts;
  lastFanSpeedState_ = fanSpeed;
  lastCompressorState_ = compressorActive;
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

void PowerLog::ensureMinute(unsigned long minute) {
  if (hasCurrentMinute_ && minute == currentMinute_) {
    return;
  }

  currentMinute_ = minute;
  hasCurrentMinute_ = true;
  currentIndex_ = head_;
  entries_[currentIndex_] = {minute * 60000UL,
                             totalEnergyWh_,
                             0.0f,
                             lastFanSpeedState_,
                             lastCompressorState_};
  head_ = (head_ + 1) % kMaxEntries;
  if (count_ < kMaxEntries) {
    ++count_;
  }
  resetMinuteAggregates();
}

void PowerLog::resetMinuteAggregates() {
  wattMillisAccumulated_ = 0.0f;
  durationMsAccumulated_ = 0;
  compressorOnDurationMs_ = 0;
  for (unsigned long &duration : fanDurationMs_) {
    duration = 0;
  }
}

void PowerLog::accumulateSegment(unsigned long durationMs,
                                 float watts,
                                 controller::FanSpeed fanSpeed,
                                 bool compressorActive) {
  wattMillisAccumulated_ += watts * static_cast<float>(durationMs);
  durationMsAccumulated_ += durationMs;
  if (compressorActive) {
    compressorOnDurationMs_ += durationMs;
  }

  size_t fanIndex = static_cast<size_t>(fanSpeed);
  if (fanIndex < (sizeof(fanDurationMs_) / sizeof(fanDurationMs_[0]))) {
    fanDurationMs_[fanIndex] += durationMs;
  }

  float hours = static_cast<float>(durationMs) / (1000.0f * 60.0f * 60.0f);
  totalEnergyWh_ += watts * hours;

  Entry &entry = entries_[currentIndex_];
  entry.timestamp = currentMinute_ * 60000UL;
  entry.energyWhAccumulated = totalEnergyWh_;
  entry.instantaneousWatts = durationMsAccumulated_ > 0
                                  ? wattMillisAccumulated_ / static_cast<float>(durationMsAccumulated_)
                                  : 0.0f;
  entry.fanSpeed = durationMsAccumulated_ > 0 ? dominantFanSpeed() : fanSpeed;
  if (durationMsAccumulated_ > 0) {
    entry.compressorActive = (compressorOnDurationMs_ * 2 >= durationMsAccumulated_);
  } else {
    entry.compressorActive = compressorActive;
  }
}

controller::FanSpeed PowerLog::dominantFanSpeed() const {
  unsigned long bestDuration = 0;
  size_t bestIndex = static_cast<size_t>(lastFanSpeedState_);
  const size_t count = sizeof(fanDurationMs_) / sizeof(fanDurationMs_[0]);
  for (size_t i = 0; i < count; ++i) {
    if (fanDurationMs_[i] > bestDuration) {
      bestDuration = fanDurationMs_[i];
      bestIndex = i;
    }
  }
  return static_cast<controller::FanSpeed>(bestIndex);
}

}  // namespace logging
