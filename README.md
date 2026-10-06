# Microcontroller

ESP32 projects built with [PlatformIO](https://platformio.org/) and the Arduino framework.

| Project | Description |
|---|---|
| [HelloWorld](HelloWorld/) | Button input with debouncing, driving the onboard LED and a NeoPixel strip |
| [SleepButton](SleepButton/) | 15x5 NeoPixel matrix display with a 3x5 digit font |

## Hardware

- ESP32 devboard (ESP32-D0WD-V3, 4MB flash) with a CP2102 USB-to-UART bridge. PlatformIO board ID: `esp32dev`.

## Building and flashing

Each project is a standalone PlatformIO project. Open the project folder (the one containing `platformio.ini`) in VS Code with the PlatformIO extension, or use the CLI from that folder:

```sh
pio run                  # build
pio run -t upload        # flash
pio device monitor       # serial monitor at the project's monitor_speed
```

On Linux, your user must be in the `dialout` group to access `/dev/ttyUSB0`.
