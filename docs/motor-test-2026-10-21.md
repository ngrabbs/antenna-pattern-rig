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
