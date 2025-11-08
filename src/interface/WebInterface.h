#pragma once

#include <ESP8266WebServer.h>
#include <LittleFS.h>

#include "../controller/HVACController.h"
#include "../logging/PowerLog.h"
#include "../logging/TemperatureLog.h"
#include "../scheduler/ScheduleManager.h"

namespace interface {

class WebInterface {
 public:
  WebInterface(controller::HVACController &controller,
               scheduler::ScheduleManager &schedule,
               logging::TemperatureLog &temperatureLog,
               logging::PowerLog &powerLog,
               uint16_t port = 80);

  void begin();
  void handleClient();

 private:
  void registerRoutes();
  void handleState();
  void handleConfig();
  void handleNotFound();
  void serveIndex();

  static String fanModeToString(controller::FanMode mode);
  static controller::FanMode fanModeFromString(const String &value);
  static String fanSpeedToString(controller::FanSpeed speed);
  static String systemModeToString(controller::SystemMode mode);
  static controller::SystemMode systemModeFromString(const String &value);

  void appendTemperatureLog(String &json, size_t maxEntries) const;
  void appendPowerLog(String &json, size_t maxEntries) const;

  void updateScheduleFromArg(const String &arg,
                             void (scheduler::ScheduleManager::*setter)(const scheduler::ScheduleEntry *,
                                                                          size_t));

  controller::HVACController &controller_;
  scheduler::ScheduleManager &schedule_;
  logging::TemperatureLog &temperatureLog_;
  logging::PowerLog &powerLog_;

  ESP8266WebServer server_;
};

}  // namespace interface
