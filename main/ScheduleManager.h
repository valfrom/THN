#pragma once

#include <Arduino.h>
#include <time.h>

namespace scheduler {

enum class ScheduledMode : uint8_t {
  kUnspecified = 0,
  kCooling,
  kHeating,
  kFanOnly,
  kIdle,
};

struct ScheduleEntry {
  uint8_t hour;
  uint8_t minute;
  float temperature;
  ScheduledMode mode;

  constexpr ScheduleEntry(uint8_t h = 0,
                          uint8_t m = 0,
                          float t = 0.0f,
                          ScheduledMode mo = ScheduledMode::kUnspecified)
      : hour(h), minute(m), temperature(t), mode(mo) {}
};

struct ScheduleTarget {
  float temperature;
  ScheduledMode mode;
};

class ScheduleManager {
 public:
  static constexpr size_t kMaxEntries = 12;

  ScheduleManager();

  void setDefaultTemperature(float temperature);

  void setWeekdaySchedule(const ScheduleEntry *entries, size_t count);
  void setWeekendSchedule(const ScheduleEntry *entries, size_t count);

  ScheduleTarget targetFor(time_t now) const;

  const ScheduleEntry *weekdayEntries(size_t &count) const;
  const ScheduleEntry *weekendEntries(size_t &count) const;

 private:
  struct ScheduleData {
    ScheduleEntry entries[kMaxEntries];
    size_t count = 0;
  };

  static void copyAndSort(const ScheduleEntry *entries,
                          size_t count,
                          ScheduleData &destination);

  static ScheduleTarget resolveTarget(const ScheduleData &schedule,
                                      int minutesOfDay,
                                      const ScheduleTarget &fallback);

  float defaultTemperature_ = 23.0f;
  ScheduleData weekday_;
  ScheduleData weekend_;
};

}  // namespace scheduler
