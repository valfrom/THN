#include "SettingsStorage.h"

#include <FS.h>
#include <LittleFS.h>

namespace storage {

namespace {
constexpr const char *kKeyTarget = "target";
constexpr const char *kKeyHysteresis = "hysteresis";
constexpr const char *kKeyCompressorTempLimit = "compressorTempLimit";
constexpr const char *kKeyCompressorMinAmbient = "compressorMinAmbient";
constexpr const char *kKeyFanMode = "fanMode";
constexpr const char *kKeySystemMode = "systemMode";
constexpr const char *kKeyScheduling = "scheduling";
constexpr const char *kKeyTimezoneMinutes = "timezoneOffsetMinutes";
constexpr const char *kKeyWeekday = "weekday";
constexpr const char *kKeyWeekend = "weekend";

String toLowerCopy(const String &value) {
  String copy = value;
  copy.toLowerCase();
  return copy;
}

}  // namespace

SettingsStorage::SettingsStorage(const char *path) : path_(path) {}

bool SettingsStorage::begin() const { return LittleFS.begin(); }

bool SettingsStorage::load(controller::HVACController &hvac,
                           scheduler::ScheduleManager &schedule) const {
  File file = LittleFS.open(path_, "r");
  if (!file) {
    return false;
  }

  bool applied = false;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.isEmpty() || line.startsWith('#')) {
      continue;
    }
    int separator = line.indexOf('=');
    if (separator <= 0) {
      continue;
    }
    String key = line.substring(0, separator);
    String value = line.substring(separator + 1);
    key.trim();
    value.trim();
    if (key.equalsIgnoreCase(kKeyTarget)) {
      hvac.setTargetTemperature(value.toFloat());
      applied = true;
    } else if (key.equalsIgnoreCase(kKeyHysteresis)) {
      hvac.setHysteresis(value.toFloat());
      applied = true;
    } else if (key.equalsIgnoreCase(kKeyCompressorTempLimit)) {
      hvac.setCompressorTemperatureLimit(value.toFloat());
      applied = true;
    } else if (key.equalsIgnoreCase(kKeyCompressorMinAmbient)) {
      hvac.setCompressorMinimumAmbient(value.toFloat());
      applied = true;
    } else if (key.equalsIgnoreCase(kKeyFanMode)) {
      hvac.setFanMode(fanModeFromString(value));
      applied = true;
    } else if (key.equalsIgnoreCase(kKeySystemMode)) {
      hvac.setSystemMode(systemModeFromString(value));
      applied = true;
    } else if (key.equalsIgnoreCase(kKeyScheduling)) {
      hvac.enableScheduling(value.equalsIgnoreCase("true") || value == "1");
      applied = true;
    } else if (key.equalsIgnoreCase(kKeyTimezoneMinutes)) {
      schedule.setTimezoneOffsetMinutes(value.toInt());
      applied = true;
    } else if (key.equalsIgnoreCase(kKeyWeekday)) {
      scheduler::ScheduleEntry entries[scheduler::ScheduleManager::kMaxEntries];
      size_t count = 0;
      if (parseSchedule(value, entries, count)) {
        schedule.setWeekdaySchedule(entries, count);
        applied = true;
      }
    } else if (key.equalsIgnoreCase(kKeyWeekend)) {
      scheduler::ScheduleEntry entries[scheduler::ScheduleManager::kMaxEntries];
      size_t count = 0;
      if (parseSchedule(value, entries, count)) {
        schedule.setWeekendSchedule(entries, count);
        applied = true;
      }
    }
  }
  file.close();
  return applied;
}

bool SettingsStorage::save(const controller::HVACController &hvac,
                           const scheduler::ScheduleManager &schedule) const {
  File file = LittleFS.open(path_, "w");
  if (!file) {
    return false;
  }

  file.printf("%s=%.2f\n", kKeyTarget, hvac.targetTemperature());
  file.printf("%s=%.2f\n", kKeyHysteresis, hvac.hysteresis());
  file.printf("%s=%.1f\n", kKeyCompressorTempLimit, hvac.compressorTemperatureLimit());
  file.printf("%s=%.1f\n", kKeyCompressorMinAmbient, hvac.compressorMinimumAmbient());
  file.printf("%s=%s\n", kKeyFanMode, fanModeToString(hvac.fanMode()).c_str());
  file.printf("%s=%s\n", kKeySystemMode, systemModeToString(hvac.systemMode()).c_str());
  file.printf("%s=%s\n", kKeyScheduling, hvac.schedulingEnabled() ? "true" : "false");
  file.printf("%s=%d\n", kKeyTimezoneMinutes, schedule.timezoneOffsetMinutes());

  size_t weekdayCount = 0;
  const scheduler::ScheduleEntry *weekday = schedule.weekdayEntries(weekdayCount);
  String weekdayString = scheduleToString(weekday, weekdayCount);
  file.printf("%s=%s\n", kKeyWeekday, weekdayString.c_str());

  size_t weekendCount = 0;
  const scheduler::ScheduleEntry *weekend = schedule.weekendEntries(weekendCount);
  String weekendString = scheduleToString(weekend, weekendCount);
  file.printf("%s=%s\n", kKeyWeekend, weekendString.c_str());

  file.close();
  return true;
}

bool SettingsStorage::parseSchedule(const String &value,
                                    scheduler::ScheduleEntry *entries,
                                    size_t &count) const {
  count = 0;
  if (value.length() == 0) {
    return true;
  }

  int start = 0;
  while (start < value.length() && count < scheduler::ScheduleManager::kMaxEntries) {
    int end = value.indexOf(';', start);
    if (end == -1) {
      end = value.length();
    }
    String token = value.substring(start, end);
    token.trim();
    if (!token.isEmpty()) {
      int colon = token.indexOf(':');
      int equals = token.indexOf('=');
      if (colon > 0 && equals > colon) {
        uint8_t hour = static_cast<uint8_t>(token.substring(0, colon).toInt());
        uint8_t minute = static_cast<uint8_t>(token.substring(colon + 1, equals).toInt());
        int modeSeparator = token.indexOf('|', equals + 1);
        String tempPart = modeSeparator == -1 ? token.substring(equals + 1)
                                              : token.substring(equals + 1, modeSeparator);
        float temperature = tempPart.toFloat();
        scheduler::ScheduledMode mode = scheduler::ScheduledMode::kUnspecified;
        if (modeSeparator != -1) {
          String modePart = token.substring(modeSeparator + 1);
          modePart.trim();
          modePart = toLowerCopy(modePart);
          mode = scheduleModeFromString(modePart);
        }
        entries[count++] = scheduler::ScheduleEntry(hour, minute, temperature, mode);
      }
    }
    start = end + 1;
  }
  return true;
}

String SettingsStorage::scheduleToString(const scheduler::ScheduleEntry *entries, size_t count) {
  String result;
  for (size_t i = 0; i < count; ++i) {
    if (i > 0) {
      result += ';';
    }
    if (entries[i].hour < 10) {
      result += '0';
    }
    result += String(entries[i].hour);
    result += ':';
    if (entries[i].minute < 10) {
      result += '0';
    }
    result += String(entries[i].minute);
    result += '=';
    result += String(entries[i].temperature, 1);
    if (entries[i].mode != scheduler::ScheduledMode::kUnspecified) {
      result += '|';
      result += scheduleModeToString(entries[i].mode);
    }
  }
  return result;
}

String SettingsStorage::fanModeToString(controller::FanMode mode) {
  switch (mode) {
    case controller::FanMode::kAuto:
      return "auto";
    case controller::FanMode::kOff:
      return "off";
    case controller::FanMode::kLow:
      return "low";
    case controller::FanMode::kMedium:
      return "medium";
    case controller::FanMode::kHigh:
      return "high";
  }
  return "auto";
}

controller::FanMode SettingsStorage::fanModeFromString(const String &value) {
  String normalized = toLowerCopy(value);
  if (normalized == "off") {
    return controller::FanMode::kOff;
  }
  if (normalized == "low") {
    return controller::FanMode::kLow;
  }
  if (normalized == "medium") {
    return controller::FanMode::kMedium;
  }
  if (normalized == "high") {
    return controller::FanMode::kHigh;
  }
  return controller::FanMode::kAuto;
}

String SettingsStorage::systemModeToString(controller::SystemMode mode) {
  switch (mode) {
    case controller::SystemMode::kCooling:
      return "cooling";
    case controller::SystemMode::kHeating:
      return "heating";
    case controller::SystemMode::kFanOnly:
      return "fan";
    case controller::SystemMode::kIdle:
      return "idle";
  }
  return "cooling";
}

controller::SystemMode SettingsStorage::systemModeFromString(const String &value) {
  String normalized = toLowerCopy(value);
  if (normalized == "heating") {
    return controller::SystemMode::kHeating;
  }
  if (normalized == "fan") {
    return controller::SystemMode::kFanOnly;
  }
  if (normalized == "idle") {
    return controller::SystemMode::kIdle;
  }
  return controller::SystemMode::kCooling;
}

String SettingsStorage::scheduleModeToString(scheduler::ScheduledMode mode) {
  switch (mode) {
    case scheduler::ScheduledMode::kCooling:
      return "cooling";
    case scheduler::ScheduledMode::kHeating:
      return "heating";
    case scheduler::ScheduledMode::kFanOnly:
      return "fan";
    case scheduler::ScheduledMode::kIdle:
      return "idle";
    case scheduler::ScheduledMode::kUnspecified:
      break;
  }
  return "";
}

scheduler::ScheduledMode SettingsStorage::scheduleModeFromString(const String &value) {
  if (value == "cooling") {
    return scheduler::ScheduledMode::kCooling;
  }
  if (value == "heating") {
    return scheduler::ScheduledMode::kHeating;
  }
  if (value == "fan") {
    return scheduler::ScheduledMode::kFanOnly;
  }
  if (value == "idle") {
    return scheduler::ScheduledMode::kIdle;
  }
  return scheduler::ScheduledMode::kUnspecified;
}

}  // namespace storage
