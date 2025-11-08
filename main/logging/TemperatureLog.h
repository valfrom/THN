#pragma once

#include <Arduino.h>

namespace logging {

class TemperatureLog {
 public:
  struct Entry {
    unsigned long timestamp;
    float ambient;
    float coil;
  };

  static constexpr size_t kMaxEntries = 120;

  TemperatureLog();

  void addReading(unsigned long timestamp, float ambient, float coil);

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

 private:
  Entry entries_[kMaxEntries];
  size_t head_ = 0;
  size_t count_ = 0;
};

}  // namespace logging
