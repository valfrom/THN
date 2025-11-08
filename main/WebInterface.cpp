#include "WebInterface.h"
#include "WebInterfaceHtml.h"

#include <ESP8266WiFi.h>

namespace interface {

WebInterface::WebInterface(controller::HVACController &controller,
                           scheduler::ScheduleManager &schedule,
                           logging::TemperatureLog &temperatureLog,
                           logging::PowerLog &powerLog,
                           uint16_t port)
    : controller_(controller),
      schedule_(schedule),
      temperatureLog_(temperatureLog),
      powerLog_(powerLog),
      server_(port) {}

void WebInterface::begin() {
  registerRoutes();
  server_.begin();
  Serial.println(F("Web interface started."));
}

void WebInterface::handleClient() { server_.handleClient(); }

void WebInterface::registerRoutes() {
  server_.on("/", HTTP_GET, [this]() { serveIndex(); });
  server_.on("/api/state", HTTP_GET, [this]() { handleState(); });
  server_.on("/api/config", HTTP_POST, [this]() { handleConfig(); });
  server_.onNotFound([this]() { handleNotFound(); });
}

void WebInterface::serveIndex() {
  server_.send_P(200, "text/html", kWebInterfaceHtml);
}

void WebInterface::handleState() {
  String json = "{";
  json += "\"ssid\":\"" + WiFi.SSID() + "\"";
  json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += ",\"target\":" + String(controller_.targetTemperature(), 2);
  json += ",\"hysteresis\":" + String(controller_.hysteresis(), 2);
  json += ",\"fanMode\":\"" + fanModeToString(controller_.fanMode()) + "\"";
  json += ",\"systemMode\":\"" + systemModeToString(controller_.systemMode()) + "\"";
  json += controller_.schedulingEnabled() ? ",\"scheduling\":true" : ",\"scheduling\":false";
  json += controller_.compressorRunning() ? ",\"compressor\":true" : ",\"compressor\":false";
  json += ",\"fanSpeed\":\"" + fanSpeedToString(controller_.fan().currentSpeed()) + "\"";

  const controller::SensorManager &sensors = controller_.sensors();
  if (sensors.hasAmbient()) {
    json += ",\"ambient\":" + String(sensors.ambient().value, 2);
  }
  if (sensors.hasCoil()) {
    json += ",\"coil\":" + String(sensors.coil().value, 2);
  }

  appendTemperatureLog(json, 30);
  appendPowerLog(json, 30);

  size_t weekdayCount = 0;
  const scheduler::ScheduleEntry *weekday = schedule_.weekdayEntries(weekdayCount);
  json += ",\"weekday\":[";
  for (size_t i = 0; i < weekdayCount; ++i) {
    if (i > 0) {
      json += ",";
    }
    String hourStr = weekday[i].hour < 10 ? "0" + String(weekday[i].hour)
                                          : String(weekday[i].hour);
    String minuteStr = weekday[i].minute < 10 ? "0" + String(weekday[i].minute)
                                              : String(weekday[i].minute);
    json += "{\"time\":\"" + hourStr + ":" + minuteStr +
            "\",\"temp\":" + String(weekday[i].temperature, 1) + "}";
  }
  json += "]";

  size_t weekendCount = 0;
  const scheduler::ScheduleEntry *weekend = schedule_.weekendEntries(weekendCount);
  json += ",\"weekend\":[";
  for (size_t i = 0; i < weekendCount; ++i) {
    if (i > 0) {
      json += ",";
    }
    String hourStr = weekend[i].hour < 10 ? "0" + String(weekend[i].hour)
                                          : String(weekend[i].hour);
    String minuteStr = weekend[i].minute < 10 ? "0" + String(weekend[i].minute)
                                              : String(weekend[i].minute);
    json += "{\"time\":\"" + hourStr + ":" + minuteStr +
            "\",\"temp\":" + String(weekend[i].temperature, 1) + "}";
  }
  json += "]";

  json += "}";
  server_.send(200, "application/json", json);
}

void WebInterface::handleConfig() {
  if (server_.hasArg("target")) {
    controller_.setTargetTemperature(server_.arg("target").toFloat());
  }
  if (server_.hasArg("hysteresis")) {
    controller_.setHysteresis(server_.arg("hysteresis").toFloat());
  }
  if (server_.hasArg("fanMode")) {
    controller_.setFanMode(fanModeFromString(server_.arg("fanMode")));
  }
  if (server_.hasArg("systemMode")) {
    controller_.setSystemMode(systemModeFromString(server_.arg("systemMode")));
  }
  if (server_.hasArg("scheduling")) {
    controller_.enableScheduling(server_.arg("scheduling") == "true");
  }
  if (server_.hasArg("weekday")) {
    updateScheduleFromArg(server_.arg("weekday"), &scheduler::ScheduleManager::setWeekdaySchedule);
  }
  if (server_.hasArg("weekend")) {
    updateScheduleFromArg(server_.arg("weekend"), &scheduler::ScheduleManager::setWeekendSchedule);
  }

  server_.send(200, "application/json", "{\"status\":\"ok\"}");
}

void WebInterface::handleNotFound() { server_.send(404, "application/json", "{\"error\":\"not found\"}"); }

String WebInterface::fanModeToString(controller::FanMode mode) {
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

controller::FanMode WebInterface::fanModeFromString(const String &value) {
  if (value == "off") {
    return controller::FanMode::kOff;
  }
  if (value == "low") {
    return controller::FanMode::kLow;
  }
  if (value == "medium") {
    return controller::FanMode::kMedium;
  }
  if (value == "high") {
    return controller::FanMode::kHigh;
  }
  return controller::FanMode::kAuto;
}

String WebInterface::fanSpeedToString(controller::FanSpeed speed) {
  switch (speed) {
    case controller::FanSpeed::kOff:
      return "off";
    case controller::FanSpeed::kLow:
      return "low";
    case controller::FanSpeed::kMedium:
      return "medium";
    case controller::FanSpeed::kHigh:
      return "high";
  }
  return "off";
}

String WebInterface::systemModeToString(controller::SystemMode mode) {
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

controller::SystemMode WebInterface::systemModeFromString(const String &value) {
  if (value == "heating") {
    return controller::SystemMode::kHeating;
  }
  if (value == "fan") {
    return controller::SystemMode::kFanOnly;
  }
  if (value == "idle") {
    return controller::SystemMode::kIdle;
  }
  return controller::SystemMode::kCooling;
}

void WebInterface::appendTemperatureLog(String &json, size_t maxEntries) const {
  json += ",\"temperatureLog\":[";
  size_t count = 0;
  temperatureLog_.forEach([&](const logging::TemperatureLog::Entry &entry) {
    if (count >= maxEntries) {
      return;
    }
    if (count > 0) {
      json += ",";
    }
    String ambient = isnan(entry.ambient) ? String("null") : String(entry.ambient, 2);
    String coil = isnan(entry.coil) ? String("null") : String(entry.coil, 2);
    json += "{\"t\":" + String(entry.timestamp) + ",\"ambient\":" +
            ambient + ",\"coil\":" + coil + "}";
    ++count;
  });
  json += "]";
}

void WebInterface::appendPowerLog(String &json, size_t maxEntries) const {
  json += ",\"powerLog\":[";
  size_t count = 0;
  powerLog_.forEach([&](const logging::PowerLog::Entry &entry) {
    if (count >= maxEntries) {
      return;
    }
    if (count > 0) {
      json += ",";
    }
    json += "{\"t\":" + String(entry.timestamp) +
            ",\"wh\":" + String(entry.energyWhAccumulated, 2) +
            ",\"watts\":" + String(entry.instantaneousWatts, 1) +
            ",\"fan\":\"" + fanSpeedToString(entry.fanSpeed) +
            "\",\"compressor\":" + String(entry.compressorActive ? "true" : "false") + "}";
    ++count;
  });
  json += "]";
  json += ",\"energyWh\":" + String(powerLog_.totalEnergyWh(), 2);
}

void WebInterface::updateScheduleFromArg(
    const String &arg,
    void (scheduler::ScheduleManager::*setter)(const scheduler::ScheduleEntry *, size_t)) {
  if (arg.length() == 0) {
    return;
  }
  scheduler::ScheduleEntry entries[scheduler::ScheduleManager::kMaxEntries];
  size_t count = 0;
  int start = 0;
  while (start < arg.length() && count < scheduler::ScheduleManager::kMaxEntries) {
    int end = arg.indexOf(';', start);
    if (end == -1) {
      end = arg.length();
    }
    String token = arg.substring(start, end);
    token.trim();
    if (token.length() > 0) {
      int equals = token.indexOf('=');
      int colon = token.indexOf(':');
      if (equals > colon && colon > 0) {
        uint8_t hour = static_cast<uint8_t>(token.substring(0, colon).toInt());
        uint8_t minute = static_cast<uint8_t>(token.substring(colon + 1, equals).toInt());
        float temperature = token.substring(equals + 1).toFloat();
        entries[count++] = {hour, minute, temperature};
      }
    }
    start = end + 1;
  }
  if (count > 0) {
    (schedule_.*setter)(entries, count);
  }
}

}  // namespace interface
