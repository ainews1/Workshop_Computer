# Rolling Dice

A minimal program card for the Music Thing Modular Workshop System Computer.

Press the **Z switch down** to roll a die. For about two seconds the six LEDs blink in a random tumbling pattern, then the result (1–6) settles as that many lit LEDs.

## Panel

| Control | Function |
|---|---|
| **Z switch Down** | Momentary roll button |
| **LEDs 0–5** | Random blinking while rolling; then number of lit LEDs equals the rolled value 1–6 |

## Build

Requires the Raspberry Pi Pico SDK.

```bash
cd releases/102_rolling_dice
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DPICO_SDK_PATH=/path/to/pico-sdk -DPICO_NO_PICOTOOL=1
mingw32-make -j4
```

This produces `rolling_dice.elf` and `rolling_dice.uf2`.

## Notes

- The die uses a fast xorshift random generator seeded from the card's unique flash ID mixed with hardware entropy, so every power-on gives a different sequence.
- `ProcessSample` is kept very light so the card easily fits inside the 20 µs sample budget.
- No audio, CV, pulse, or normalisation-probe features are used.
