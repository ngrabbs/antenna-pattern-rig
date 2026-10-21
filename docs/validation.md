# Validation record

2026-10-21, software-only checks:

- Python unit tests: four tests passed, including separate motor/encoder reduction
  ratios, direct-drive resolution/alignment, and invalid input rejection.
- C++11 quadrature decoder: simulated 600-cycle revolution produced +2400 counts;
  reverse revolution returned to zero; repeated states, one-edge bounce and
  two-bit invalid jumps passed their checks.
- Arduino compile for `arduino:avr:uno`, AVR core 1.8.8: passed; 4604 bytes flash,
  237 bytes static RAM. Compiler warnings were unused parameters inside the
  Arduino core's new.cpp, not the project's source.
- Run metadata JSON parsed successfully.

To compile using an installed official Arduino CLI/core (no upload):

```sh
arduino-cli compile --fqbn arduino:avr:uno --warnings all firmware/encoder_monitor
```

After backing up installed firmware and making the documented diagnostic wiring,
the sketch can be uploaded deliberately using Arduino IDE/CLI. Serial output is
115200 baud, with comment lines prefixed `#` and CSV samples at 10 Hz. Send a
lowercase `z` to establish a new relative zero; the reference ID increments.
Counts remain signed and unwrapped; the invalid-transition counter is cumulative
until reset. Reset restarts the session and loses the position reference.

**Not verified:** physical encoder counts/revolution, output voltages, counting
under motor electrical noise, driver identity, load-dependent motion, shield
wiring, absolute angle, repeatability, calibration, backup/restore of the existing
  firmware, motor control, or RF capture. No board was connected, flashed or reset.

## Subsequent hardware readback, 2026-10-21

After the software-only checks above, the Uno was connected on a Linux host.
Docker AVRDUDE identified ATmega328P and read the full 32768-byte flash twice;
the images matched. Intel HEX was independently decoded and matched every byte.
See [readback record](readback-2026-10-21.md). This does not validate the diagnostic
sketch, motion, encoder wiring, EEPROM contents, or restoration.
