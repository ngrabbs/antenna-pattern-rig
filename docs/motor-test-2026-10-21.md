# Motor bring-up, October 21, 2026

Powered motion remains unqualified. Do not interpret successful compilation,
holding torque or software step counts as verified antenna rotation.

## Reported setup and checks

- Original purple driver in Z; provisionally identified as DRV8825, two R250
  sense resistors photographed. Adjustment screw measured 0.993 V after motor
  supply was restored: approximately 0.794 A full-scale if that identification
  and VREF point are correct. An initial low reading occurred without VMOT.
- User reports approximately 12 V at VMOT and approximately 0 V at M0/M1/M2,
  matching the empty mode jumper positions and software CONFIG 1.
- Motor winding resistance: red/blue approximately 3 ohms; green/black
  approximately 3 ohms. Initially, bottom-to-top connector colors were
  black/green/red/blue. Continuity mapped these to 2A/1A/1B/2B respectively.
  The carrier underside photo matches the usual A2/A1/B1/B2 output layout.
  The entire connector was later reversed as a user-requested diagnostic;
  this did not resolve the fault. Preserve the actual orientation in future logs.
- Uno D4 to carrier STP and D7 to DIR each measured 0.4 ohm.
- User confirmed holding torque, firm 1:1 encoder coupling, and motor twitching
  during attempts. Bench supply reportedly remained in constant-voltage mode.

**Correction from the later assembly photo:** the coupling is a rubber band
around two bare shafts. The earlier report of firm 1:1 coupling was not proof of
rigid angular transfer. Shaft diameter ratio, band stretch and slip are unqualified.
The user also reports seeing a motor step during the zero-count TICK test. Thus
the powered encoder discrepancy cannot, on its own, diagnose a motor/driver fault.
The assistant's earlier rigid-coupling assumption overstated what was established.

## Encoder and motion observations

An approximate manual quarter turn of the coupled motor shaft changed the
encoder from -2 to -576: -574 counts, nominally -86.1 degrees, with no invalid
transitions. This supports the gross encoder scale; it is not a calibrated angle
or proof of behavior under energized motor conditions.

Repeated 9-degree probes stopped after three software-counted full steps
(5.4 nominal degrees commanded), with only a few encoder counts observed.
Fault 2 (following disagreement) disabled the driver before the requested five
steps completed. Reversing the whole motor connector produced the same fault,
with a final reported +3 counts relative to ZERO. These are failed tests, not
completed sweeps. In the later session one invalid transition was already present
at boot; it did not increase during the failed powered attempts.

## Scope observations and limitations

Rigol DS1202Z-E at 192.168.1.50, remote SCPI on TCP 5555. A first single-pulse
capture at the reported STP contact measured approximately 5.04 V maximum and
17.2 microseconds positive pulse width. Original scope settings and captures
are retained in ignored local data.

A later two-channel capture showed approximately 17-microsecond pulses spaced
0.361 seconds apart on CH2 and slower transitions on CH1. The user reported a
high probability that the hand-held probes had been reversed. Channel identities
were therefore not established, and that capture cannot assign encoder behavior
to a particular contact. Probe contact stability is another limitation. The user
removed the probes; further tests must not require holding two probes while typing.

A remote memory-depth change while stopped stalled SCPI replies; the user
power-cycled the scope and communication recovered. Subsequent setup left memory
depth unchanged. Do not repeat that command sequence.

## Next diagnostic

Firmware v2 adds explicit `TICK`: one full step, a one-second observation interval,
then automatic disable with the existing guards retained. Native tests cover
positive/negative encoder response, delayed response, no-response failure, full-step
interlock, one-shot reference invalidation, heartbeat and invalid-transition stops.
The first physical TICK test (19:05 UTC / 14:05 CDT) commanded one software-counted
full step. Encoder displacement remained zero through the one-second energized
observation interval; invalid transitions remained zero. The controller then
disabled with fault 2 and `done=0`. Thus the discrepancy also occurs with an
isolated step given time to settle, rather than only during the multi-step probe.
This does not distinguish a missing driver output, wiring fault, mechanical
problem, or a motor/encoder response problem; the software counter alone does
not verify the physical pulse or angular displacement in this particular run.
No cause for the powered-motion discrepancy has yet been established.
A spare driver that reportedly remained at 9 mV VREF
was not qualified and is not evidence of a confirmed replacement test.

Firmware v3 adds a separate BELTTEST: fixed 20 full steps (36 nominal motor
degrees), recording encoder response without the rigid 1:1 following test. It
retains independent stop/heartbeat/invalid-transition/travel/time guards and
automatically disables after observation. Native tests cover zero motion and
both encoder polarities, a fixed target, one-second observation, automatic disable,
configuration/reference interlocks, STOP, heartbeat, encoder invalidity, travel
and move timeout.

## First BELTTEST result and observed band slip

At approximately 19:12 UTC / 14:12 CDT, the diagnostic reached 20 software-counted
full steps (36 nominal commanded motor degrees), then disabled automatically with
`done=1`, `fault=0`, `bad=0`. The first disabled sample recorded +97 encoder
counts (14.55 nominal encoder degrees); later samples relaxed to approximately
+100 counts (15 degrees) with the driver disabled. The user confirmed that the
motor ran 20 steps and the rubber band was slipping. The command-completion
counter is not a tracking pass and does not independently measure motor angle.

Observed band slip establishes an unreliable mechanical transmission and explains
why rigid 1:1 tracking cannot qualify this assembly. This result does not certify
motor step accuracy, loaded torque, driver current calibration, or absolute antenna
pointing. Do not fit a calibration ratio to this slipping-band run. Replace the
friction band with a positive mechanical connection before positioning tests.
For final antenna angle, sensing the turntable output also captures transmission
backlash or slip that a motor-shaft encoder cannot see.

## Repeat runs and equal grooved pulleys

The second BELTTEST at approximately 19:16 UTC completed the same 20 commanded
steps, then disabled with `done=2`, `fault=0`, `bad=0`. A post-disable reading was
+224 counts (33.6 degrees). The user subsequently reported installing pulleys
locked to the shafts, with equal belt-running sizes, grooves rather than teeth,
and no observed rubber-band slip. This establishes a reported nominal 1:1 ratio,
not independently measured pitch diameters or a positive belt engagement.

The third BELTTEST, after that clarification, at approximately 19:18 UTC completed
20 commanded steps and disabled with `done=3`, `fault=0`, `bad=0`. The stable
post-disable reading was +230 counts (34.5 nominal encoder degrees), versus 240
counts (36 degrees) expected for ideal 1:1 transfer. The 10-count/1.5-degree
difference does not by itself distinguish elastic lag, effective pulley ratio,
residual slip, motor position error or another source. Neither run is a calibrated
pointing-accuracy result. Raw logs for all three runs are retained privately.

## Additional commanded-increment consistency checks

At the user's request, three more positive-direction BELTTEST sequences were run
with the reported equal grooved pulleys. Each used ARM, settled ZERO, the fixed
20-step command, automatic disable and a reading two seconds after observing
disable. Each sequence started from its current location, so this checks repeated
commanded increments at different positions, not return-to-the-same-target
positioning repeatability or reversal hysteresis.

| Additional trial | First disabled count | Count two seconds later | Encoder angle two seconds later |
| --- | ---: | ---: | ---: |
| 1 (`done=4`) | 211 | 211 | 31.65 degrees |
| 2 (`done=5`) | 210 | 211 | 31.65 degrees |
| 3 (`done=6`) | 257 | 258 | 38.70 degrees |

All reached 20 software-counted steps and disabled with fault=0 and bad=0.
Completion is not a tracking pass. The settled spread is 47 encoder counts,
nominally 7.05 degrees, compared with 240 counts / 36 degrees expected under
ideal 1:1 transfer. The first two matching readings do not establish overall
consistency; the third disproves that provisional impression. Further identical
repeats were stopped rather than selecting only matching runs.

These results are not satisfactory for fine antenna positioning. They do not
isolate the cause: motor angle was not independently measured, and an equal-size
grooved friction transmission is not proof of a constant effective ratio or
absence of creep. Independent reference marks/angle measurements on both shafts,
or a positive mechanical transmission, are needed before calibration or feedback
qualification. Raw telemetry and per-trial timestamps/states are retained in
ignored local data; the controller was left disabled after trial 3.

## Requested 180-degree outward-and-return diagnostic

Firmware v4 adds the explicit SWEEP180 cycle: fixed -100 motor full steps,
two-second endpoint observation, return to the original software step zero without
resetting encoder zero, two-second observation, automatic disable. It records
turnaround and return encoder counts, so outbound transfer and reversal residual
can be evaluated separately. Physical direction is relative to the GPIO convention,
not an independently established left/right reference. The user confirmed motor
power off and USB connected before the firmware upload.

Native tests cover both encoder polarities, zero response, a nonzero return residual,
retained zero across reversal, endpoint observation periods, automatic disable,
full-step/reference interlocks, travel bounds, watchdog, invalid transitions,
movement/settling timeouts and STOP preventing an automatic return. This diagnostic
has separate explicit encoder/command travel and time bounds documented in
motor-jog.md; it does not relax normal jog guards.

## First 180-degree outward-and-return result

The user enabled motor power and the complete cycle ran from approximately
19:29:19 to 19:30:35 UTC (14:29 to 14:30 CDT), about 75.5 seconds between the
first sweep telemetry and the first completed sample. The motor software counter
went from 0 to -100 full steps and back to 0. The same encoder zero was retained
throughout. Both endpoints were observed while energized before recording.

| Measurement | Commanded motor travel | Encoder travel | Apparent shortfall |
| --- | ---: | ---: | ---: |
| Outward | -180 degrees | -1161 counts = -174.15 degrees | 5.85 degrees / 3.25% |
| Return leg | +180 degrees | +1178 counts = +176.70 degrees | 3.30 degrees / 1.83% |

At commanded motor step zero, the captured pre-disable residual was +17 encoder
counts (+2.55 degrees). The driver then disabled automatically with `done=1`,
`leg=3`, `fault=0`, `bad=0`. Subsequent disabled readings reached +19 counts
(+2.85 degrees) by 19:31:09 UTC. This additional 0.30 degree relaxation is distinct
from the recorded endpoint residual. Raw logs, live snapshots, endpoint data and
numeric summary are retained privately in the run-06 data directory.

The percentages compare nominal encoder angle with commanded motor angle, assuming
the reported equal pulleys. They are not isolated friction-slip measurements:
actual motor angle was not independently referenced. Effective pulley ratio,
elastic storage, mechanical slip, motor error and encoder uncertainty remain
possible contributors. The different outward/return discrepancies, nonzero
closure residual and prior variable increments do not justify applying one fixed
slip correction. This cycle measures disagreement and return residual; it is not
a demonstration of an accurately homed antenna or a qualified feedback controller.

## Two further full sweep cycles using the encoder as the working reference

The user chose to proceed treating encoder angle as the working measurement.
Two further complete outward-and-return cycles were run without a firmware change
or automatic encoder correction. ARM and ZERO were repeated before each cycle,
so residuals are relative to that cycle's starting encoder reference, not an
independent physical home. All three full cycles are compared below; endpoint
angles were captured while holding, immediately before automatic disable.

| Cycle | Outward encoder travel | Return encoder travel | Home residual |
| --- | ---: | ---: | ---: |
| 1 | -1161 counts / -174.15 degrees | +1178 / +176.70 degrees | +17 / +2.55 degrees |
| 2 | -1172 counts / -175.80 degrees | +1174 / +176.10 degrees | +2 / +0.30 degrees |
| 3 | -1174 counts / -176.10 degrees | +1173 / +175.95 degrees | -1 / -0.15 degrees |

Cycles 2 and 3 completed at approximately 19:39 and 19:40 UTC (14:39 and 14:40
CDT), with fault=0, bad=0 and automatic disable. Their two-second post-disable
readings remained +2 and -1 counts respectively. Across all three cycles the
outward travel spread was 1.95 degrees, return travel spread 0.75 degree, and
home residual spread 2.70 degrees. The latest pair's outward spread was 0.30
degree and home residual spread 0.45 degree; these smaller ranges do not erase
the first cycle or qualify global repeatability.

The tests use encoder readings as a working reference; they do not establish
whether the motor, transmission or another component caused the mismatch.
No correction factor or closed-loop position command was applied. Encoder-based
target stopping is a sensible next supervised experiment, with bounded motor
travel/time and an appropriate tolerance for the current full-step hardware.
It must not be described as absolute antenna pointing calibration.

`tools/summarize_sweeps.py` reproduces completed-cycle counts, encoder angles,
descriptive transfer ratios and ranges from a timestamped serial log. It
deduplicates repeated endpoint packets and separates boots. Raw logs and endpoint
records stay in ignored local data. The summary was checked against all three
captured endpoint tuples and the independently calculated first-cycle values.

## Encoder-targeted firmware prepared for the next test

At the user's request to proceed, FEEDBACK180 was added as a separate explicit
mode. It learns encoder polarity from five outward full steps, seeks logical
-1200 encoder counts, then seeks the preserved encoder zero, with a +/-6-count
(nominal +/-0.9-degree) endpoint tolerance appropriate to this initial full-step
test. It uses finite motor bursts, endpoint observation, pulse/correction budgets,
motor/encoder travel bounds, timeouts and automatic disable; see motor-jog.md.
This applies bounded encoder feedback rather than a constant slip multiplier.

Native feedback and previous motion tests passed; the Docker build used 16662
flash bytes and 1018 static RAM bytes. The user confirmed motor power off and USB
connected before the v5 upload, which AVRDUDE verified. Physical encoder-targeted
results remain pending. The controller boots disabled; uploading does not arm it.
