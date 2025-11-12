#include "PowerLogStorage.h"

#include <FS.h>
#include <LittleFS.h>
#include <math.h>

namespace storage {

namespace {
constexpr float kEnergyEpsilonWh = 0.05f;
}  // namespace

PowerLogStorage::PowerLogStorage(logging::PowerLog &log, const char *path)
    : log_(log), path_(path) {}

bool PowerLogStorage::begin() {
  available_ = LittleFS.begin();
  dirty_ = false;
  lastKnownCount_ = log_.size();
  logging::PowerLog::Entry latest;
  hasLastKnownLatest_ = log_.latestEntry(latest);
  lastKnownLatestTimestamp_ = hasLastKnownLatest_ ? latest.timestamp : 0;
  lastKnownTotalEnergyWh_ = log_.totalEnergyWh();
  lastSnapshotMillis_ = LittleFS.exists(path_)
                           ? millis()
                           : millis() - kSnapshotIntervalMs;
  return available_;
}

bool PowerLogStorage::load() {
  if (!available_) {
    return false;
  }

  File file = LittleFS.open(path_, "r");
  if (!file) {
    return false;
  }

  struct Header {
    uint32_t magic;
    uint16_t version;
    uint16_t count;
    float totalEnergyWh;
  } header{};

  if (file.read(reinterpret_cast<uint8_t *>(&header), sizeof(header)) != sizeof(header)) {
    file.close();
    return false;
  }

  if (header.magic != kMagic || header.version != kVersion) {
    file.close();
    return false;
  }

  uint16_t count = header.count;
  if (count > logging::PowerLog::kMaxEntries) {
    count = logging::PowerLog::kMaxEntries;
  }

  logging::PowerLog::Entry buffer[logging::PowerLog::kMaxEntries];
  size_t restored = 0;
  for (; restored < count; ++restored) {
    PersistedEntry entry{};
    if (file.read(reinterpret_cast<uint8_t *>(&entry), sizeof(entry)) != sizeof(entry)) {
      break;
    }
    buffer[restored] = {entry.timestamp,
                        entry.energyWhAccumulated,
                        entry.instantaneousWatts,
                        static_cast<controller::FanSpeed>(entry.fanSpeed),
                        entry.compressorActive != 0};
  }
  file.close();

  if (restored == 0) {
    return false;
  }

  log_.restoreEntries(buffer, restored, header.totalEnergyWh);
  lastKnownCount_ = log_.size();
  logging::PowerLog::Entry latest;
  hasLastKnownLatest_ = log_.latestEntry(latest);
  lastKnownLatestTimestamp_ = hasLastKnownLatest_ ? latest.timestamp : 0;
  lastKnownTotalEnergyWh_ = log_.totalEnergyWh();
  dirty_ = false;
  lastSnapshotMillis_ = millis();
  return true;
}

bool PowerLogStorage::save() {
  if (!available_) {
    return false;
  }

  logging::PowerLog::Entry entries[logging::PowerLog::kMaxEntries];
  size_t count = log_.copyEntries(entries, logging::PowerLog::kMaxEntries);

  File file = LittleFS.open(path_, "w");
  if (!file) {
    return false;
  }

  struct Header {
    uint32_t magic;
    uint16_t version;
    uint16_t count;
    float totalEnergyWh;
  } header{kMagic, kVersion, static_cast<uint16_t>(count), log_.totalEnergyWh()};

  if (file.write(reinterpret_cast<const uint8_t *>(&header), sizeof(header)) != sizeof(header)) {
    file.close();
    return false;
  }

  for (size_t i = 0; i < count; ++i) {
    PersistedEntry entry{entries[i].timestamp,
                         entries[i].energyWhAccumulated,
                         entries[i].instantaneousWatts,
                         static_cast<uint8_t>(entries[i].fanSpeed),
                         static_cast<uint8_t>(entries[i].compressorActive ? 1 : 0)};
    if (file.write(reinterpret_cast<const uint8_t *>(&entry), sizeof(entry)) != sizeof(entry)) {
      file.close();
      return false;
    }
  }

  file.close();
  dirty_ = false;
  lastSnapshotMillis_ = millis();
  lastKnownCount_ = count;
  logging::PowerLog::Entry latest;
  hasLastKnownLatest_ = log_.latestEntry(latest);
  lastKnownLatestTimestamp_ = hasLastKnownLatest_ ? latest.timestamp : 0;
  lastKnownTotalEnergyWh_ = log_.totalEnergyWh();
  return true;
}

void PowerLogStorage::loop() {
  if (!available_) {
    return;
  }

  updateDirtyFlags();

  unsigned long now = millis();
  unsigned long elapsed = now - lastSnapshotMillis_;
  if (elapsed >= kSnapshotIntervalMs) {
    save();
  }
}

void PowerLogStorage::markDirty() { dirty_ = true; }

void PowerLogStorage::updateDirtyFlags() {
  size_t currentCount = log_.size();
  if (currentCount != lastKnownCount_) {
    lastKnownCount_ = currentCount;
    dirty_ = true;
  }

  logging::PowerLog::Entry latest;
  bool hasLatest = log_.latestEntry(latest);
  if (hasLatest) {
    if (!hasLastKnownLatest_ || latest.timestamp != lastKnownLatestTimestamp_) {
      lastKnownLatestTimestamp_ = latest.timestamp;
      hasLastKnownLatest_ = true;
      dirty_ = true;
    }
  } else if (hasLastKnownLatest_) {
    hasLastKnownLatest_ = false;
    lastKnownLatestTimestamp_ = 0;
    dirty_ = true;
  }

  float totalEnergy = log_.totalEnergyWh();
  if (fabsf(totalEnergy - lastKnownTotalEnergyWh_) > kEnergyEpsilonWh) {
    lastKnownTotalEnergyWh_ = totalEnergy;
    dirty_ = true;
  }
}

}  // namespace storage

