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

## Subsequent encoder bring-up, 2026-10-21

The unchanged encoder diagnostic source was compiled in Docker on the Linux host
with Arduino AVR core 1.8.8, uploaded to the bare Uno, and verified by AVRDUDE:
4604 flash bytes. The original rotor image remains in private backups. A hand-turn
test captured 1449 valid samples with no invalid transitions; the final 10 seconds
remained at -10 counts. Manual turn/return endpoints were -2381 and -10 counts,
so this is a functional bring-up result, not an exact scale or accuracy calibration.
See [test record](encoder-test-2026-10-21.md). No motor was powered or driven.

## Encoder through CNC shield, 2026-10-21

Initial shield takeoffs gave 2035 samples with no count changes despite a reported
physical revolution and a measured 5 V encoder supply. After the user identified
D2/D3 by continuity in the blue X/Y-labeled block below the STEP/DIR header and
moved the signal wires, the unchanged firmware reported 2399 counts for one
manually indexed turn and +3 counts on return (user overshot and corrected).
Across 1124 parsed samples, invalid transitions stayed zero and timestamps were
strictly increasing. The final 10 seconds stayed at +3 counts. All driver sockets
were empty, the EN/GND shunt was removed, and motor power was disconnected.
See [shield test record](encoder-shield-test-2026-10-21.md).

## Motor-jog firmware baseline, 2026-10-21

Native C++11 checks passed for explicit configuration/arming/reference interlocks,
both encoder polarities, subdivisions 1/8/32, round-trip moves, planned travel,
stall/following faults, invalid transitions, moving/settling timeouts, heartbeat
timeout and wraparound, fault latching and serial framing/emergency stop handling.
The test source is `tests/motion_test.cpp`.

Docker build using Arduino AVR core 1.8.8 and AccelStepper 1.64.0 passed: 13124
flash bytes and 730 bytes static RAM. AVRDUDE verified the application upload.
Live telemetry showed boot disabled, explicit `CONFIG 1` accepted, and a `JOG 1`
request rejected while disarmed, with zero commanded steps and no fault.
Actual supply/VREF measurements and powered movements remain pending. Unit-model
tests and a verified upload do not establish the physical motion/stop behavior.

## Subsequent powered diagnostics and encoder-targeted mode

The baseline paragraph above records the earlier unpowered state. Subsequent
measurements and powered movements are documented in
[motor-test-2026-10-21.md](motor-test-2026-10-21.md), including three complete
fixed-step 180-degree outward/return cycles. Those did not establish agreement
between motor commands and encoder angle or absolute pointing accuracy.

FEEDBACK180 native tests (`tests/feedback_test.cpp`) pass both encoder polarities,
several transmission gains, asymmetric direction gains, endpoint relaxation,
full-step tolerance convergence and millis rollover. They also exercise missing
feedback at startup and after learning, wrong-direction response, unreachable
targets/correction oscillation, motor/encoder travel and pulse-budget limits,
continuous jitter/settling timeout, leg timeout, configuration/reference interlocks,
heartbeat loss, invalid transitions and STOP. Existing motion tests still pass.
Physical encoder-targeted positioning results remain pending; model success is
not a demonstration of hardware convergence or actual antenna pointing.

The v5 Docker build passed with 16662 flash bytes and 1018 static RAM bytes.
