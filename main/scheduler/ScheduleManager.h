#pragma once

#include <Arduino.h>
#include <time.h>

namespace scheduler {

struct ScheduleEntry {
  uint8_t hour;
  uint8_t minute;
  float temperature;
};

class ScheduleManager {
 public:
  static constexpr size_t kMaxEntries = 12;

  ScheduleManager();

  void setDefaultTemperature(float temperature);

  void setWeekdaySchedule(const ScheduleEntry *entries, size_t count);
  void setWeekendSchedule(const ScheduleEntry *entries, size_t count);

  float targetFor(time_t now) const;

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

  static float resolveTarget(const ScheduleData &schedule, int minutesOfDay, float fallback);

  float defaultTemperature_ = 23.0f;
  ScheduleData weekday_;
  ScheduleData weekend_;
};

}  // namespace scheduler
