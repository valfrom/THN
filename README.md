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

| Component | Default GPIO (NodeMCU silkscreen) | Notes |
| --- | --- | --- |
| Compressor relay | `GPIO5` (`D1`) | Active HIGH output. Update `kCompressorRelayPin` in `main/main.ino` as needed. |
| Fan low/medium/high relays | `GPIO14/12/13` (`D5/D6/D7`) | Only one speed is energized at a time. |
| Ambient temperature sensor | `GPIO4` (`D2`, OneWire bus) | First detected DS18B20 on the shared bus. |
| Coil temperature sensor | `GPIO4` (`D2`, OneWire bus) | Second detected DS18B20 on the shared bus. |

All pin assignments live near the top of [`main/main.ino`](main/main.ino). Adjust them to match your
wiring. If you need additional GPIOs (e.g., blower enable), extend the controller classes in
`main/` (see `Compressor.h`, `FanController.h`, and `HVACController.h`).

## Configuration

1. Copy `main/WiFiConfig.example.h` to `main/WiFiConfig.h` and update the SSID and password
   constants. The real credentials file is ignored by Git.
2. (Optional) Edit the default schedules in `main/main.ino` to match your preferences.
3. Two DS18B20 sensors on the `GPIO4` (`D2`) OneWire bus are used by default. The first discovered device
   becomes the ambient probe and the second (if present) is assigned to the coil. If you only have
   one sensor it will be treated as the ambient probe and coil temperatures will read `NAN`. Update
   `initializeSensors()` in `main/main.ino` if you need to bind sensors by address or support a
   different probe type.
4. If your compressor or fan draws different power, update `kConsumptionTable` in `main/main.ino`
   so energy logging is accurate.

## Building with the Arduino IDE

1. Install the **ESP8266** board support package via the Arduino Boards Manager.
2. Install the following libraries if they are not already present:
   - ESP8266WiFi (bundled with the board package)
   - OneWire (by Paul Stoffregen)
   - DallasTemperature
3. Open `main/main.ino` in the Arduino IDE. The IDE will treat the `main` directory as the sketch folder.
4. Select the correct board (e.g., *NodeMCU 1.0 (ESP-12E Module)*) and serial port.
5. Build and upload the sketch.

## Runtime behavior

- On boot, the device connects to Wi-Fi using the credentials in `WiFiConfig.h`, synchronizes time
  via NTP, and loads the default schedules and power table.
- The control loop runs every second. Compressor state changes always respect the minimum runtime
  and restart delay defined in `main/Compressor.h`.
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
across reboots, update the defaults in `main/main.ino` or extend the project with your preferred
storage mechanism.

## Project structure

```
main/
  main.ino              # Arduino sketch entry point
  Compressor.[h|cpp]    # Compressor relay protections and scheduling
  FanController.[h|cpp] # Fan relay coordination and safety logic
  SensorManager.[h|cpp] # DS18B20 integration and caching helpers
  HVACController.[h|cpp]# Core thermostat logic tying everything together
  ScheduleManager.[h|cpp]
  TemperatureLog.[h|cpp]
  PowerLog.[h|cpp]
  WebInterface.[h|cpp]  # HTTP API and inline HTML dashboard (WebInterfaceHtml.h)
  WiFiConfig.example.h  # Template Wi-Fi credentials (copy to WiFiConfig.h)
```

Feel free to expand the system with additional sensors, a heating mode, or persistent settings by
building on the provided abstractions.
