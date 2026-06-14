# ESP32 Relay Board Demo

A minimal PlatformIO/Arduino demo that drives a relay board from an ESP32 dev board. Six relays are sequenced round-robin using non-blocking timing so the main loop never stalls.

## Hardware

- ESP32 dev board (e.g. DOIT ESP32 DevKit V1)
- 6+ channel relay board (active-low)
- Jumper wires connecting the first six relay inputs to the GPIO pins below

### Pin Mapping

| Relay | GPIO |
|-------|------|
| 1     | 13   |
| 2     | 32   |
| 3     | 14   |
| 4     | 27   |
| 5     | 26   |
| 6     | 25   |

The on-board LED (GPIO 2) is used as a program status indicator.

## How It Works

The `loop()` function runs continuously without any blocking `delay()` calls. Two independent timers are driven by `millis()` comparisons:

| Timer | Rate | Purpose |
|-------|------|---------|
| Status LED | 500 ms | Blinks the on-board LED to confirm the program is running |
| Relay sequencer | 100 ms | Steps through each relay one at a time, deactivating the previous and activating the next |

Because both timers are independent, the LED blinks at its own 500 ms cadence while the relay sequence advances five times faster at 100 ms — they are not coupled or synchronised to each other.

Relays are active-low: `LOW` energises the coil, `HIGH` releases it. All relays are initialised to `HIGH` (off) in `setup()`.

## Building & Flashing

This project uses [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Upload
pio run --target upload

# Open serial monitor (115200 baud)
pio device monitor
```

## Photos / Video

See [PhotosVideos/](PhotosVideos/) for reference images of the wiring and a short clip of the relay sequence in action.
