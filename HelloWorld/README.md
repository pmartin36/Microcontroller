# HelloWorld

Reads a push button and responds on each press:

- The onboard LED (GPIO 2) is lit while the button is held.
- Each press prints `pushed` to serial and advances a NeoPixel pattern: three evenly spaced pixels step one position along the strip, each lit with a random color.

The button input is debounced in software. A reading must hold steady for 30 ms before it counts as a change.

## Wiring

| Part | ESP32 pin |
|---|---|
| Push button, one leg | 3V3 |
| Push button, diagonally opposite leg | GPIO 27 (internal pull-down enabled) |
| NeoPixel strip, 15 LEDs, data in | GPIO 25 |

## Dependencies

- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) (installed by PlatformIO from `lib_deps`)

## Serial

115200 baud. Prints `On` at boot and `pushed` on each button press.
