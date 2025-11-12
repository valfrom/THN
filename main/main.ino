#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>

#include "HVACController.h"
#include "SensorManager.h"
#include "WebInterface.h"
#include "PowerLog.h"
#include "PowerLogStorage.h"
#include "TemperatureLog.h"
#include "ScheduleManager.h"
#include "SettingsStorage.h"

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
using scheduler::ScheduledMode;
using scheduler::ScheduleManager;
using storage::SettingsStorage;

namespace {
// Pin configuration -- adjust for your hardware layout.
constexpr uint8_t kCompressorRelayPin = 16;   // GPIO5 (D0)
constexpr FanController::Pins kFanPins = {5, 14, 12};  // GPIO5/14/12 (D1/D5/D6)
constexpr uint8_t kOneWireBusPin = 4;        // GPIO4 (D2)

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
storage::PowerLogStorage powerLogStorage(powerLog);
SettingsStorage settingsStorage;
HVACController hvac(compressor, fan, sensors, scheduleManager, temperatureLog, powerLog);
WebInterface webInterface(hvac, scheduleManager, temperatureLog, powerLog, &settingsStorage, 80);

const PowerLog::ConsumptionRate kConsumptionTable[] = {
    {FanSpeed::kOff, false, 5.0f},   {FanSpeed::kLow, false, 110.0f},
    {FanSpeed::kMedium, false, 125.0f}, {FanSpeed::kHigh, false, 140.0f},
    {FanSpeed::kLow, true, 600.0f}, {FanSpeed::kMedium, true, 650.0f},
    {FanSpeed::kHigh, true, 700.0f},
};

const ScheduleEntry kDefaultWeekday[] = {
    ScheduleEntry(6, 0, 23.0f, ScheduledMode::kCooling),
    ScheduleEntry(9, 0, 26.0f, ScheduledMode::kCooling),
    ScheduleEntry(17, 30, 23.5f, ScheduledMode::kCooling),
    ScheduleEntry(22, 0, 25.0f, ScheduledMode::kIdle),
};

const ScheduleEntry kDefaultWeekend[] = {
    ScheduleEntry(8, 0, 23.5f, ScheduledMode::kCooling),
    ScheduleEntry(12, 0, 25.0f, ScheduledMode::kCooling),
    ScheduleEntry(18, 0, 23.0f, ScheduledMode::kCooling),
    ScheduleEntry(23, 0, 25.5f, ScheduledMode::kIdle),
};

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

void logInitialTemperatureReadings() {
  if (!hasAmbientSensor && !hasCoilSensor) {
    Serial.println(F("Initial temperature readings unavailable (no sensors detected)."));
    return;
  }

  refreshDallasTemperatures();

  if (hasAmbientSensor) {
    Serial.print(F("Initial ambient temperature: "));
    if (!isnan(cachedAmbientC)) {
      Serial.print(cachedAmbientC, 2);
      Serial.println(F(" °C"));
    } else {
      Serial.println(F("unavailable"));
    }
  }

  if (hasCoilSensor) {
    Serial.print(F("Initial coil temperature: "));
    if (!isnan(cachedCoilC)) {
      Serial.print(cachedCoilC, 2);
      Serial.println(F(" °C"));
    } else {
      Serial.println(F("unavailable"));
    }
  }

  dallasCacheValid = false;
}

void configureSchedule() {
  scheduleManager.setDefaultTemperature(23.5f);
  scheduleManager.setWeekdaySchedule(kDefaultWeekday, sizeof(kDefaultWeekday) / sizeof(ScheduleEntry));
  scheduleManager.setWeekendSchedule(kDefaultWeekend, sizeof(kDefaultWeekend) / sizeof(ScheduleEntry));
}

void configureOta() {
  ArduinoOTA.setHostname("thn-hvac");

  ArduinoOTA.onStart([]() { Serial.println(F("OTA update starting")); });

  ArduinoOTA.onEnd([]() {
    Serial.println();
    Serial.println(F("OTA update complete"));
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA progress: %u%%\r", (progress * 100) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error[%u]: ", error);
    switch (error) {
      case OTA_AUTH_ERROR:
        Serial.println(F("Auth failed"));
        break;
      case OTA_BEGIN_ERROR:
        Serial.println(F("Begin failed"));
        break;
      case OTA_CONNECT_ERROR:
        Serial.println(F("Connect failed"));
        break;
      case OTA_RECEIVE_ERROR:
        Serial.println(F("Receive failed"));
        break;
      case OTA_END_ERROR:
        Serial.println(F("End failed"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }
  });

  ArduinoOTA.begin();
  Serial.println(F("OTA ready"));
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println(F("HVAC controller booting"));

  connectWiFi();
  configureOta();
  configureTime();

  initializeSensors();
  logInitialTemperatureReadings();
  configureSchedule();

  powerLog.setConsumptionTable(kConsumptionTable,
                               sizeof(kConsumptionTable) / sizeof(PowerLog::ConsumptionRate));

  bool storageReady = powerLogStorage.begin();
  if (!storageReady) {
    Serial.println(F("Failed to mount LittleFS; persistence disabled."));
  } else {
    if (powerLogStorage.load()) {
      Serial.println(F("Power log restored from storage."));
    } else {
      Serial.println(F("No saved power log found; starting fresh power history."));
    }
  }

  hvac.setSystemMode(SystemMode::kCooling);
  hvac.setFanMode(FanMode::kAuto);
  hvac.enableScheduling(true);
  scheduler::ScheduleTarget initialTarget = scheduleManager.targetFor(time(nullptr));
  hvac.setTargetTemperature(initialTarget.temperature);
  switch (initialTarget.mode) {
    case scheduler::ScheduledMode::kCooling:
      hvac.setSystemMode(SystemMode::kCooling);
      break;
    case scheduler::ScheduledMode::kHeating:
      hvac.setSystemMode(SystemMode::kHeating);
      break;
    case scheduler::ScheduledMode::kFanOnly:
      hvac.setSystemMode(SystemMode::kFanOnly);
      break;
    case scheduler::ScheduledMode::kIdle:
      hvac.setSystemMode(SystemMode::kIdle);
      break;
    case scheduler::ScheduledMode::kUnspecified:
      break;
  }
  hvac.setHysteresis(1.0f);

  if (storageReady) {
    if (settingsStorage.load(hvac, scheduleManager)) {
      Serial.println(F("Settings restored from storage."));
    } else {
      Serial.println(F("No saved settings found; using defaults."));
    }
  }

  scheduleManager.update(hvac);
  hvac.begin();

  webInterface.begin();
}

void loop() {
  ArduinoOTA.handle();
  scheduleManager.update(hvac);
  hvac.update();
  powerLogStorage.update();
  webInterface.handleClient();
}
