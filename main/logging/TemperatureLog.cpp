#include "TemperatureLog.h"

namespace logging {

TemperatureLog::TemperatureLog() = default;

void TemperatureLog::addReading(unsigned long timestamp, float ambient, float coil) {
  entries_[head_] = {timestamp, ambient, coil};
  head_ = (head_ + 1) % kMaxEntries;
  if (count_ < kMaxEntries) {
    ++count_;
  }
}

}  // namespace logging
