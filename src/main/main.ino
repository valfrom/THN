#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>

#include "../controller/HVACController.h"
#include "../controller/SensorManager.h"
#include "../interface/WebInterface.h"
#include "../logging/PowerLog.h"
#include "../logging/TemperatureLog.h"
#include "../scheduler/ScheduleManager.h"

#include "WiFiConfig.h"

using controller::Compressor;
using controller::FanController;
using controller::FanMode;
using controller::FanSpeed;
using controller::HVACController;
using controller::SensorManager;
using controller::SystemMode;
using interface::WebInterface;
using logging::PowerLog;
using logging::TemperatureLog;
using scheduler::ScheduleEntry;
using scheduler::ScheduleManager;

namespace {
// Pin configuration -- adjust for your hardware layout.
constexpr uint8_t kCompressorRelayPin = D1;
constexpr FanController::Pins kFanPins = {D5, D6, D7};
constexpr uint8_t kOneWireBusPin = D0;

constexpr unsigned long kControlIntervalMs = 1000;

OneWire oneWire(kOneWireBusPin);
DallasTemperature dallasSensors(&oneWire);

DeviceAddress ambientAddress;
DeviceAddress coilAddress;
bool hasAmbientSensor = false;
bool hasCoilSensor = false;

float cachedAmbientC = NAN;
float cachedCoilC = NAN;
bool dallasCacheValid = false;

void refreshDallasTemperatures() {
  dallasSensors.requestTemperatures();
  if (hasAmbientSensor) {
    float reading = dallasSensors.getTempC(ambientAddress);
    cachedAmbientC = (reading == DEVICE_DISCONNECTED_C) ? NAN : reading;
  } else {
    cachedAmbientC = NAN;
  }
  if (hasCoilSensor) {
    float reading = dallasSensors.getTempC(coilAddress);
    cachedCoilC = (reading == DEVICE_DISCONNECTED_C) ? NAN : reading;
  } else {
    cachedCoilC = NAN;
  }
  dallasCacheValid = true;
}

float readAmbientTemperature() {
  if (!hasAmbientSensor) {
    if (!hasCoilSensor) {
      dallasCacheValid = false;
    }
    return NAN;
  }
  if (!dallasCacheValid) {
    refreshDallasTemperatures();
  }
  float value = cachedAmbientC;
  if (!hasCoilSensor) {
    dallasCacheValid = false;
  }
  return value;
}

float readCoilTemperature() {
  if (!hasCoilSensor) {
    dallasCacheValid = false;
    return NAN;
  }
  if (!dallasCacheValid) {
    refreshDallasTemperatures();
  }
  float value = cachedCoilC;
  dallasCacheValid = false;
  return value;
}

Compressor compressor(kCompressorRelayPin);
FanController fan(kFanPins);
SensorManager sensors;
ScheduleManager scheduleManager;
TemperatureLog temperatureLog;
PowerLog powerLog;
HVACController hvac(compressor, fan, sensors, scheduleManager, temperatureLog, powerLog);
WebInterface webInterface(hvac, scheduleManager, temperatureLog, powerLog, 80);

const PowerLog::ConsumptionRate kConsumptionTable[] = {
    {FanSpeed::kOff, false, 5.0f},   {FanSpeed::kLow, false, 45.0f},
    {FanSpeed::kMedium, false, 65.0f}, {FanSpeed::kHigh, false, 90.0f},
    {FanSpeed::kLow, true, 1550.0f}, {FanSpeed::kMedium, true, 1575.0f},
    {FanSpeed::kHigh, true, 1600.0f},
};

const ScheduleEntry kDefaultWeekday[] = {
    {6, 0, 23.0f}, {9, 0, 26.0f}, {17, 30, 23.5f}, {22, 0, 25.0f},
};

const ScheduleEntry kDefaultWeekend[] = {
    {8, 0, 23.5f}, {12, 0, 25.0f}, {18, 0, 23.0f}, {23, 0, 25.5f},
};

unsigned long lastControlUpdate = 0;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFiConfig::kSsid, WiFiConfig::kPassword);
  Serial.print(F("Connecting to Wi-Fi"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.print(F("Connected: "));
  Serial.println(WiFi.localIP());
}

void configureTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
  Serial.print(F("Synchronizing time"));
  time_t now = time(nullptr);
  while (now < 100000) {
    delay(500);
    Serial.print('.');
    now = time(nullptr);
  }
  Serial.println();
}

void initializeSensors() {
  dallasSensors.begin();
  dallasSensors.setResolution(12);
  hasAmbientSensor = dallasSensors.getAddress(ambientAddress, 0);
  hasCoilSensor = dallasSensors.getAddress(coilAddress, hasAmbientSensor ? 1 : 0);

  if (!hasAmbientSensor && !hasCoilSensor) {
    Serial.println(F("No DS18B20 sensors detected on the OneWire bus."));
  } else {
    Serial.print(F("DS18B20 sensors detected: "));
    Serial.println(dallasSensors.getDeviceCount());
    if (hasAmbientSensor) {
      Serial.print(F("  Ambient sensor assigned to index 0"));
      Serial.println();
    }
    if (hasCoilSensor) {
      Serial.print(F("  Coil sensor assigned to index "));
      Serial.println(hasAmbientSensor ? 1 : 0);
    }
  }

  sensors.setAmbientReader(readAmbientTemperature);
  sensors.setCoilReader(readCoilTemperature);
}

void configureSchedule() {
  scheduleManager.setDefaultTemperature(23.5f);
  scheduleManager.setWeekdaySchedule(kDefaultWeekday, sizeof(kDefaultWeekday) / sizeof(ScheduleEntry));
  scheduleManager.setWeekendSchedule(kDefaultWeekend, sizeof(kDefaultWeekend) / sizeof(ScheduleEntry));
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println(F("HVAC controller booting"));

  connectWiFi();
  configureTime();

  initializeSensors();
  configureSchedule();

  powerLog.setConsumptionTable(kConsumptionTable,
                               sizeof(kConsumptionTable) / sizeof(PowerLog::ConsumptionRate));

  hvac.setSystemMode(SystemMode::kCooling);
  hvac.setFanMode(FanMode::kAuto);
  hvac.enableScheduling(true);
  hvac.setTargetTemperature(scheduleManager.targetFor(time(nullptr)));
  hvac.setHysteresis(1.0f);
  hvac.begin();

  webInterface.begin();
}

void loop() {
  unsigned long now = millis();
  if (now - lastControlUpdate >= kControlIntervalMs) {
    hvac.update();
    lastControlUpdate = now;
  }
  webInterface.handleClient();
  delay(10);
}
