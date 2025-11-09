#include "ScheduleManager.h"

namespace scheduler {

namespace {
int toMinutes(uint8_t hour, uint8_t minute) { return hour * 60 + minute; }
}

ScheduleManager::ScheduleManager() = default;

void ScheduleManager::setDefaultTemperature(float temperature) {
  defaultTemperature_ = temperature;
}

void ScheduleManager::setWeekdaySchedule(const ScheduleEntry *entries, size_t count) {
  copyAndSort(entries, count, weekday_);
}

void ScheduleManager::setWeekendSchedule(const ScheduleEntry *entries, size_t count) {
  copyAndSort(entries, count, weekend_);
}

ScheduleTarget ScheduleManager::targetFor(time_t now) const {
  if (now == 0) {
    return {defaultTemperature_, ScheduledMode::kUnspecified};
  }

  tm timeinfo;
#if defined(ESP8266)
  localtime_r(&now, &timeinfo);
#else
  timeinfo = *localtime(&now);
#endif
  int minutes = toMinutes(static_cast<uint8_t>(timeinfo.tm_hour),
                          static_cast<uint8_t>(timeinfo.tm_min));
  bool weekend = (timeinfo.tm_wday == 0 || timeinfo.tm_wday == 6);
  const ScheduleData &schedule = weekend ? weekend_ : weekday_;
  return resolveTarget(schedule, minutes,
                       {defaultTemperature_, ScheduledMode::kUnspecified});
}

const ScheduleEntry *ScheduleManager::weekdayEntries(size_t &count) const {
  count = weekday_.count;
  return weekday_.entries;
}

const ScheduleEntry *ScheduleManager::weekendEntries(size_t &count) const {
  count = weekend_.count;
  return weekend_.entries;
}

void ScheduleManager::copyAndSort(const ScheduleEntry *entries,
                                  size_t count,
                                  ScheduleData &destination) {
  destination.count = min(count, kMaxEntries);
  for (size_t i = 0; i < destination.count; ++i) {
    destination.entries[i] = entries[i];
  }
  // Simple insertion sort by minutes of day.
  for (size_t i = 1; i < destination.count; ++i) {
    ScheduleEntry key = destination.entries[i];
    int keyMinutes = toMinutes(key.hour, key.minute);
    size_t j = i;
    while (j > 0) {
      int prevMinutes = toMinutes(destination.entries[j - 1].hour,
                                  destination.entries[j - 1].minute);
      if (prevMinutes <= keyMinutes) {
        break;
      }
      destination.entries[j] = destination.entries[j - 1];
      --j;
    }
    destination.entries[j] = key;
  }
}

ScheduleTarget ScheduleManager::resolveTarget(const ScheduleData &schedule,
                                              int minutesOfDay,
                                              const ScheduleTarget &fallback) {
  if (schedule.count == 0) {
    return fallback;
  }

  ScheduleTarget target = {schedule.entries[schedule.count - 1].temperature,
                           schedule.entries[schedule.count - 1].mode};
  if (target.mode == ScheduledMode::kUnspecified) {
    target.mode = fallback.mode;
  }
  for (size_t i = 0; i < schedule.count; ++i) {
    int entryMinutes = toMinutes(schedule.entries[i].hour, schedule.entries[i].minute);
    if (entryMinutes <= minutesOfDay) {
      target.temperature = schedule.entries[i].temperature;
      if (schedule.entries[i].mode != ScheduledMode::kUnspecified) {
        target.mode = schedule.entries[i].mode;
      }
    } else {
      break;
    }
  }
  return target;
}

}  // namespace scheduler
