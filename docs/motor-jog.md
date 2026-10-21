# Supervised motor jog bring-up

`firmware/motor_jog` is a separate sketch from the encoder-only monitor. It uses
Z STEP D4, Z DIR D7, active-low ENABLE D8 and the existing continuity-confirmed
encoder D2/D3 takeoffs. It assumes a 200-full-step motor and a 600-cycle encoder
mechanically coupled 1:1. It emits step commands and supervises the encoder;
it does not apply automatic position corrections, home the axis or acquire RF data.

The October 21 assembly was subsequently shown to use a rubber band around bare
motor/encoder shafts, rather than a rigid 1:1 coupling. Normal PROBE/JOG tracking
has not been qualified for that transmission. Use the separately bounded
BELTTEST diagnostic to observe it; its completion does not establish pointing.

## Hardware setup

Disconnect USB and motor supply before installing a module or moving motor leads.
Verify carrier orientation from its pin labels, not pot position. Use only the
Z driver socket, with X/Y/A drivers and A-axis clone jumpers removed. The EN/GND
shunt must be removed because D8 is actively driven high to disable the driver.
Confirm motor coil pairs and continuity to the intended STEP/DIR/ENABLE pins.
Keep the shaft clear, mechanically coupled to the encoder, with no antenna load
for the initial test.

The user's initial mode setting has no Z mode jumpers. A DRV8825 then uses full
steps; use `CONFIG 1`. The first proposed supply is 12 V with a 1 A bench limit.
For the photographed R250 resistors, proposed VREF is 1.00 V, corresponding to a
0.8 A full-scale phase-current limit. Bench current limit and driver phase-current
limit are different quantities. The actual measured supply/VREF must be recorded;
the proposed settings are not proof of what was applied.

Boot USB first: the firmware sets ENABLE high and does not move automatically.
With the firmware reporting disabled, apply the checked motor supply and measure
VREF at a verified reference point relative to driver GND. Keep the driver disabled
while adjusting current. Arming energizes the coils and can cause an alignment
movement even without step pulses. Keep the physical bench output-off control
accessible. Software limits/STOP are not a hardware emergency stop or a substitute
for physical travel constraints. Disabling releases holding torque.

## Build and protocol

Build `tools/readback/Dockerfile`, `Dockerfile.build`, then `Dockerfile.motion` in
that directory, tagged respectively `antenna-rig-readback:20261021`,
`antenna-rig-build:20261021`, and `antenna-rig-motion-build:20261021`.
The motion layer installs Arduino-registry AccelStepper 1.64.0. Compile for
`arduino:avr:uno`, and upload only `motor_jog.ino.hex`, with verification enabled.
The original rotor firmware and encoder-monitor build are retained separately.

Commands are uppercase, one per newline at 115200 baud:

| Command | Effect |
| --- | --- |
| `STATUS` | Report state; no motion |
| `CONFIG 1` | Set software subdivisions (1,2,4,8,16,32); disarmed only; does not change hardware jumpers |
| `PING` | Refresh the 2-second heartbeat timeout |
| `ARM` | Energize coils; does not emit step pulses; requires CONFIG and no fault |
| `ZERO` | Set a relative reference after >=500 ms armed and >=250 ms with no encoder change |
| `PROBE` | One +9-degree nominal move to determine encoder polarity; requires a fresh ZERO |
| `TICK` | Diagnostic: CONFIG 1 only, fresh ZERO required; command one +1.8-degree full step, observe for at least 1 second after the last pulse, then disable automatically |
| `BELTTEST` | Diagnostic: CONFIG 1 and fresh ZERO only; 20 full steps (36 nominal motor degrees), record encoder without enforcing 1:1 tracking, observe for 1 second, disable automatically |
| `JOG -9` | Relative jog, up to 9 degrees magnitude; requires successful PROBE |
| `STOP` or `!` | Immediately cancel step generation and disable coils; invalidate reference |
| `CLEAR` | Clear a latched fault while disabled; does not arm |

Positive means the firmware's STEP/DIR convention, not a claimed physical CW
direction. The probe learns raw encoder polarity; mechanical viewing direction
still needs a documented convention. Jogs are rounded to integer driver steps.
With full stepping, motor command resolution is 1.8 degrees, not encoder resolution.

Telemetry is newline JSON about four times per second. `state`: 0 disabled,
1 holding, 2 moving, 3 settling, 4 fault. Fields include `armed`, `ref`, `sign`,
`microsteps`, `step`, `target`, `enc`, `bad`, `fault`, and completed move counter
`done`. `enc` is relative raw encoder counts; multiply by 0.15 for raw degrees,
and by learned `sign` for the command-direction convention after a successful
probe. Before ZERO or PROBE those conventions are not established. Status is live
only when its timestamp/age is current.

`TICK` is a bounded diagnostic for the October 21 bring-up discrepancy. It does
not learn polarity or qualify the rig for jogs. It keeps the ordinary 24-count
moving error bound, heartbeat, invalid-transition and travel checks. After the
single pulse it observes for at least 1000 ms and requires 250 ms encoder quiet;
the existing 1500 ms settling timeout still applies. A displacement magnitude
within 6 counts of the expected 12 counts increments `done` and disables with
no fault. Otherwise it disables with a following or settling fault. Inspect the
recorded encoder displacement; this tolerance is a diagnostic acceptance bound,
not a claim of pointing accuracy. Early STOP or any other fault can shorten the
observation. Every attempt requires ARM and a fresh ZERO; no repeat is automatic.
Firmware boot identification is now `motor_jog_v3_disabled`.

`BELTTEST` explicitly bypasses only expected motor-versus-encoder displacement
checks for this unqualified elastic transmission. It never learns sign or enables
normal jogs. Its fixed target is 20 pulses at CONFIG 1, using the existing nominal
5-degree/s speed and 10-degree/s-squared acceleration (about 8 seconds of motion).
Heartbeat, STOP, invalid transitions, 50-degree nominal encoder excursion, 15-second
movement timeout and 1.5-second settling timeout remain active. After the target,
it requires at least 1 second observation and 250 ms quiet, then increments `done`
and disables regardless of encoder displacement. That `done` indicates completion
of the diagnostic command only, including when the band slips or encoder stays
still. Telemetry `diag`: 0 ordinary, 1 TICK, 2 BELTTEST. A new ARM resets diagnostic
mode; each diagnostic requires a fresh ZERO. Inspect visible shaft marks and
recorded counts to assess whether motion was transferred. No antenna load should
be attached during this bench test.

## Bounds and faults

- Nominal maximum speed 5 degrees/s, acceleration 10 degrees/s squared.
- Maximum requested jog 9 degrees; commanded position limited to +/-45 degrees.
- Encoder excursion fault beyond +/-50 nominal degrees from ZERO. Limits are
  software checks with margins, not guarantees about mechanical stopping distance.
- During motion, following disagreement above 24 counts (3.6 nominal degrees)
  trips a fault. A completed move must settle within 6 counts (0.9 nominal degrees).
- A probe checks displacement magnitude before learning polarity. Later moves
  check signed displacement. There is no correction motion to chase an encoder bin.
- Any new invalid encoder transition after ZERO faults the controller.
- Move timeout 15 s; settling timeout 1.5 s, with 250 ms of quiet required.
- Missing heartbeat for >2 s disables the driver and latches a fault.
- Oversized serial frames and malformed commands while armed disable/fault.
- STOP disables immediately; it is not an accelerated deceleration. Faults remain
  latched until CLEAR. STOP/FAULT/serial reset require re-arming, ZERO and PROBE.

Fault codes: 0 none, 1 communication timeout, 2 encoder following mismatch,
3 invalid encoder transition, 4 encoder travel limit, 5 settling timeout,
6 move timeout, 7 bad command/frame.

The Arduino loop and serial telemetry are cooperative rather than hard real time.
Very fine subdivision, high speed, heavy load, driver fault sensing and physical
limit-switch operation have not been qualified. The current firmware has no
hard-limit input or independent driver-power readback.

## Persistent host connection

`tools/motion_session.py` runs in the readback Docker image (pyserial included).
Use a fresh data directory, mount only the identified Uno and required script/data
paths, and keep one serial connection open. The session sends heartbeats while
telemetry is fresh. It **never automatically arms, zeros or moves** the motor.
Existing control requests are skipped on startup so an old ARM/JOG is not replayed.

`tools/motion_request.py --output /data CONFIG 1` writes an atomic request for that
running session. Wait for `control_result.json`, firmware acknowledgement and state
before sending another request; the single request file is not a multi-item queue.
`latest.json` reports status, age, error and whether capture is running. Raw timestamped
JSON serial data goes to `serial.log`; stale status must not be treated as live.

Host-only `HEARTBEAT_OFF` / `HEARTBEAT_ON` permit testing the watchdog while status
continues to be collected. On graceful session shutdown, STOP is sent. If the host
dies, the firmware's heartbeat timeout provides the software disable path. Opening
the Uno serial connection normally resets it, losing references and configuration.

## Validation boundary

Native tests exercise parser/framing, initial disabled state, setup interlocks,
both encoder polarities, multiple subdivisions, round-trip motion, travel bounds,
stall/following errors, invalid transitions, movement/settling timeouts, heartbeat
timeout/wraparound, and fault latching. These model tests do not establish that
physical driver wiring, current, motion or stopping behavior is correct. Record
actual hardware results separately; do not infer motor performance from build success.

Dependency: [AccelStepper 1.64](https://www.airspayce.com/mikem/arduino/AccelStepper/),
GPLv3/commercial dual-license library by Mike McCauley. Library source is obtained
by Arduino CLI in the build container, not vendored here. Firmware binaries are
retained privately in ignored build/test output. No project-wide license decision
is made by this note.
