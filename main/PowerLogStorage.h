#pragma once

#include <Arduino.h>

#include "PowerLog.h"

namespace storage {

class PowerLogStorage {
 public:
  explicit PowerLogStorage(logging::PowerLog &log, const char *path = "/powerlog.bin");

  bool begin();
  bool load();
  bool save();
  void loop();
  void markDirty();

 private:
  struct PersistedEntry {
    unsigned long timestamp;
    float energyWhAccumulated;
    float instantaneousWatts;
    uint8_t fanSpeed;
    uint8_t compressorActive;
  };

  logging::PowerLog &log_;
  const char *path_;
  bool available_ = false;
  bool dirty_ = false;
  unsigned long lastSnapshotMillis_ = 0;
  size_t lastKnownCount_ = 0;
  bool hasLastKnownLatest_ = false;
  unsigned long lastKnownLatestTimestamp_ = 0;
  float lastKnownTotalEnergyWh_ = 0.0f;

  static constexpr uint32_t kMagic = 0x504C4F47;  // 'PLOG'
  static constexpr uint16_t kVersion = 1;
  static constexpr unsigned long kSnapshotIntervalMs = 6UL * 60UL * 60UL * 1000UL;

  void updateDirtyFlags();
};

}  // namespace storage

