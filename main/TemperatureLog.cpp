#include "TemperatureLog.h"

#include <math.h>

namespace logging {

TemperatureLog::TemperatureLog() = default;

void TemperatureLog::ensureMinute(unsigned long minute) {
  if (hasCurrentMinute_ && minute == currentMinute_) {
    return;
  }

  currentMinute_ = minute;
  hasCurrentMinute_ = true;
  ambientSum_ = 0.0f;
  ambientCount_ = 0;
  coilSum_ = 0.0f;
  coilCount_ = 0;

  currentIndex_ = head_;
  entries_[currentIndex_] = {minute * 60000UL, NAN, NAN};
  head_ = (head_ + 1) % kMaxEntries;
  if (count_ < kMaxEntries) {
    ++count_;
  }
}

void TemperatureLog::addReading(unsigned long timestamp, float ambient, float coil) {
  unsigned long minute = timestamp / 60000UL;
  ensureMinute(minute);

  if (!isnan(ambient)) {
    ambientSum_ += ambient;
    ++ambientCount_;
  }
  if (!isnan(coil)) {
    coilSum_ += coil;
    ++coilCount_;
  }

  float ambientAverage = ambientCount_ > 0 ? ambientSum_ / static_cast<float>(ambientCount_) : NAN;
  float coilAverage = coilCount_ > 0 ? coilSum_ / static_cast<float>(coilCount_) : NAN;
  entries_[currentIndex_] = {minute * 60000UL, ambientAverage, coilAverage};
}

}  // namespace logging
