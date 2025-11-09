#pragma once

#include <Arduino.h>
#include "HVACController.h"
#include "ScheduleManager.h"

namespace storage {

class SettingsStorage {
 public:
  explicit SettingsStorage(const char *path = "/settings.cfg");

  bool begin() const;

  bool load(controller::HVACController &hvac, scheduler::ScheduleManager &schedule) const;
  bool save(const controller::HVACController &hvac,
            const scheduler::ScheduleManager &schedule) const;

 private:
  bool parseSchedule(const String &value,
                     scheduler::ScheduleEntry *entries,
                     size_t &count) const;

  static String scheduleToString(const scheduler::ScheduleEntry *entries, size_t count);
  static String fanModeToString(controller::FanMode mode);
  static controller::FanMode fanModeFromString(const String &value);
  static String systemModeToString(controller::SystemMode mode);
  static controller::SystemMode systemModeFromString(const String &value);
  static String scheduleModeToString(scheduler::ScheduledMode mode);
  static scheduler::ScheduledMode scheduleModeFromString(const String &value);

  const char *path_;
};

}  // namespace storage
