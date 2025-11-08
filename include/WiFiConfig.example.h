#pragma once

/**
 * Example Wi-Fi credentials configuration.
 *
 * Copy this file to `include/WiFiConfig.h` and update the SSID and password
 * values before compiling. The actual `WiFiConfig.h` file is ignored by
 * version control to prevent accidental credential leaks.
 */
namespace WiFiConfig {
static constexpr const char *kSsid = "YOUR_WIFI_SSID";
static constexpr const char *kPassword = "YOUR_WIFI_PASSWORD";
}
