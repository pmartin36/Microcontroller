# SleepButton

A 15x5 NeoPixel matrix display driven by an ESP32, with a push button for input. Work in progress.

## Current state

- `main.cpp` runs the button example from [HelloWorld](../../HelloWorld/): each debounced press prints `pushed` and steps a three-pixel pattern along the first row.
- `char_setters.hpp` draws digits 0 to 9 from a 3x5 pixel font (`DIGITS` in `constants.hpp`), and `displayAlignedNumber` writes multi-digit numbers left, right, or center aligned.
- `display_setters.hpp` has `setDisplayAsClockTime`, which lays out `H:MM` across the matrix. `getTime` in `utils.hpp` and `setDisplayAsNumber` are empty stubs.

## Display layout

75 LEDs arranged as 5 rows of 15, wired in a serpentine: even rows run left to right, odd rows run right to left. `setPixelXY(strip, x, y, color)` maps (x, y) coordinates to the strip index, with (0, 0) at the first LED.

## Wiring

| Part | ESP32 pin |
|---|---|
| Push button, one leg | 3V3 |
| Push button, diagonally opposite leg | GPIO 27 (internal pull-down enabled) |
| NeoPixel matrix data in | GPIO 25 |
| Onboard LED | GPIO 2 (lit while the button is held) |

## Dependencies

- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) (installed by PlatformIO from `lib_deps`)

## Serial

115200 baud. `platformio.ini` sets `targets = upload, monitor`, so uploading opens the serial monitor automatically.
