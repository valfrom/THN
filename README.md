# THN HVAC Controller

An ESP8266-based control stack for a portable HVAC unit featuring compressor and fan
safety logic, scheduling, logging, and a web dashboard.

## Features

- Compressor control with enforced 5-minute minimum runtime and 120-second restart delay.
- Fan controller that guarantees at least low speed when cooling is active.
- Dual temperature sensing (ambient air + coil/compressor) with hysteresis control.
- Weekday/weekend scheduling to automatically adjust target temperatures.
- Rolling temperature and power logs with configurable consumption estimates.
- Wi-Fi enabled single-page web interface for monitoring and configuration.

## Hardware overview

| Component | Default pin | Notes |
| --- | --- | --- |
| Compressor relay | `D6` | Active HIGH output. Update `kCompressorRelayPin` in `src/main/main.ino` as needed. |
| Fan low/medium/high relays | `D0` / `D1` / `D5` | Only one speed is energized at a time. |
| Ambient temperature sensor | `D2` (OneWire) | First detected DS18B20 on the shared bus. |
| Coil temperature sensor | `D2` (OneWire) | Second detected DS18B20 on the shared bus. |

All pin assignments live near the top of [`src/main/main.ino`](src/main/main.ino). Adjust them to
match your wiring. If you need additional GPIOs (e.g., blower enable), extend the controller
classes in `src/controller/`.

## Configuration

1. Copy `include/WiFiConfig.example.h` to `include/WiFiConfig.h` and update the SSID and password
   constants. The real credentials file is ignored by Git.
2. (Optional) Edit the default schedules in `src/main/main.ino` to match your preferences.
3. Two DS18B20 sensors on the `D2` OneWire bus are used by default. The first discovered device
   becomes the ambient probe and the second (if present) is assigned to the coil. If you only have
   one sensor it will be treated as the ambient probe and coil temperatures will read `NAN`. Update
   `initializeSensors()` in `src/main/main.ino` if you need to bind sensors by address or support a
   different probe type.
4. If your compressor or fan draws different power, update `kConsumptionTable` in
   `src/main/main.ino` so energy logging is accurate.

## Building with the Arduino IDE

1. Install the **ESP8266** board support package via the Arduino Boards Manager.
2. Install the following libraries if they are not already present:
   - ESP8266WiFi (bundled with the board package)
   - LittleFS (bundled with the board package)
   - OneWire (by Paul Stoffregen)
   - DallasTemperature
3. Open `src/main/main.ino` in the Arduino IDE. The IDE will treat `src/main` as the sketch folder.
4. Select the correct board (e.g., *NodeMCU 1.0 (ESP-12E Module)*) and serial port.
5. Build and upload the sketch.
6. To upload the web UI assets, choose **Tools → ESP8266 LittleFS Data Upload**. Ensure the
   [`data/index.html`](data/index.html) file is present before running the uploader.

## Runtime behavior

- On boot, the device connects to Wi-Fi using the credentials in `WiFiConfig.h`, synchronizes time
  via NTP, and loads the default schedules and power table.
- The control loop runs every second. Compressor state changes always respect the minimum runtime
  and restart delay defined in `src/controller/Compressor.h`.
- Hysteresis is centered around the target temperature. The compressor engages when the ambient
  temperature exceeds the target + hysteresis/2 and disengages below target - hysteresis/2.
- In automatic mode the fan idles when idle but is forced to at least low speed whenever cooling is
  active. Manual fan modes override the requested speed but still respect the low-speed safety
  requirement.

## Web interface

Once the module is on your network, open a browser to `http://<device-ip>/`. The single-page app
provides:

- Live status, including temperatures, compressor/fan state, Wi-Fi details, and cumulative energy.
- Inputs to adjust target temperature, hysteresis, system/fan modes, and scheduling toggle.
- Editors for weekday and weekend schedules using a simple `HH:MM=value` syntax.
- Tables showing the most recent temperature and power readings.

All settings are persisted in RAM and reapplied immediately. To make schedule changes permanent
across reboots, update the defaults in `src/main/main.ino` or extend the project with your preferred
storage mechanism.

## Project structure

```
include/              # Local configuration headers (Wi-Fi credentials)
src/controller/       # Low-level compressor, fan, sensor, and HVAC logic
src/interface/        # ESP8266 web server and REST endpoints
src/logging/          # Temperature and power rolling logs
src/scheduler/        # Weekday/weekend schedule calculations
src/main/             # Arduino sketch entry point
data/                 # Web UI assets served from LittleFS
```

Feel free to expand the system with additional sensors, a heating mode, or persistent settings by
building on the provided abstractions.
