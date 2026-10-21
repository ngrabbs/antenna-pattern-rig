# Antenna pattern rig

Single-axis antenna-pattern turntable using an Arduino Uno, CNC Shield V3.0,
Longruner 17HD48002H-22B stepper, and LPD3806-600BM-5G-24C incremental encoder.
RF instrument: NanoVNA-F V2; other instrument models are not yet recorded.

**Status:** supervised motor firmware has completed powered 20-step and 180-degree
outward/return diagnostics. Equal grooved pulleys and a rubber band transfer motion
to the encoder; commanded motor angle and encoder angle disagree. See the
[motor test results](docs/motor-test-2026-10-21.md). Encoder-targeted FEEDBACK180
completed three unloaded and two antenna-mounted bench cycles within the initial
+/-0.9-degree endpoint tolerance. A third loaded cycle was intentionally stopped
to address harsh full steps. Firmware v6 completed its first mounted 1/16-step cycle with encoder endpoints
of -180.00 and 0.00 degrees; the user reported much smoother motion. Antenna pointing
accuracy is not calibrated. Firmware boots
disabled and requires explicit setup and commands. See [motor guide](docs/motor-jog.md).
RF instrument capture and automatic antenna-pattern acquisition are not implemented. Earlier on
2026-10-21 the encoder diagnostic was
built in Docker, uploaded and verified, then tested by hand on the bare Uno:
both directions counted, no invalid transitions were observed, and the final
10-second stationary window showed no count drift. The manually indexed turn
registered -2381 counts and the return ended at -10; exact scale and angle
accuracy remain unverified. See [first encoder test](docs/encoder-test-2026-10-21.md).
Before that upload, the existing Uno rotor firmware was
read successfully in Docker: two complete flash images matched, and a HEX copy
was validated. That readback used no write/erase commands. The original program
is now preserved as private backups; the Uno currently runs `motor_jog`.
EEPROM and restoration remain unverified; see [readback record](docs/readback-2026-10-21.md).
The driver photo shows R250 sense resistors; DRV8825 identity is provisional.
The user measured 0.993 V at the adjustment screw, approximately 12 V at VMOT,
and low mode inputs with no jumpers (full steps). No license has been selected yet.

The driver is in Z with only M2 jumpered (1/16 stepping); equal-size grooved pulleys
give a reported nominal 1:1 ratio, with elastic transfer and slip unqualified.
The earlier rigid-coupling assumption is superseded by the assembly photo and
test results. Software configuration is
`CONFIG 16`. Earlier tests used full steps with no jumpers. The motion build uses the GPLv3/commercial AccelStepper 1.64.0
dependency noted in the motor guide.

The subsequent [encoder-through-shield test](docs/encoder-shield-test-2026-10-21.md)
passed a basic functional check after continuity-confirmed D2/D3 takeoffs were
used: 2399 counts for a manually indexed revolution, +3 counts on return, zero
invalid transitions, and no final stationary drift. The initial photo-based
takeoff instructions are superseded by the measured connections documented in
[hardware notes](docs/hardware.md#actual-shield-takeoffs-continuity-check-2026-10-21).

## Starting resolution

Assuming 200 full motor steps/revolution, direct drive, and 600 encoder cycles per
channel/revolution:

| Mode | Nominal increment |
| --- | ---: |
| Motor full step | 1.8 degrees |
| Motor 1/8 microstep | 0.225 degrees |
| Motor 1/16 microstep | 0.1125 degrees |
| Motor 1/32 microstep (if supported) | 0.05625 degrees |
| Encoder x1 | 0.6 degrees |
| Encoder x2 | 0.3 degrees |
| Encoder x4 | 0.15 degrees |

These are resolutions, not measured pointing accuracies. Start with 1-degree
sampling, validate repeatability under the real antenna and coax load, then
consider 0.5-degree sampling. A first-build accuracy goal of +/-0.5 to 1 degree
is an engineering target, not a specification or prediction guaranteed by these
parts. Encoder line-spacing accuracy has not been established.

## Files

- [Hardware and wiring](docs/hardware.md): existing firmware, proposed pins,
  driver identification, and mechanical constraints.
- [Encoder bench procedure](docs/encoder-test.md) and [first test results](docs/encoder-test-2026-10-21.md).
- [Accuracy and calibration](docs/accuracy.md): error budget, distance geometry,
  laser calibration, and reduction ratios.
- [Measurement procedure](docs/measurement.md): stop/settle/measure, range
  validation, polarization, and data conventions.
- [Preserve existing firmware](docs/firmware-backup.md): read-only backup guidance.
- [Sources](docs/sources.md): component and measurement references.
- [Validation](docs/validation.md): tests performed and remaining hardware checks.
- `firmware/encoder_monitor/`: x4 encoder diagnostic; no step pulses.
- `firmware/motor_jog/`: bounded step commands with encoder supervision; see [protocol](docs/motor-jog.md).
- `tools/geometry.py`: offline resolution and alignment calculations; no hardware I/O.
- `tools/capture_encoder.py`: serial capture and live JSON status; pyserial in Docker.
- `tools/motion_session.py`, `tools/motion_request.py`: persistent serial control,
  fresh-status heartbeats and explicit requests, with no automatic arming/motion.
- `data/run-template.json`: metadata template, with unknowns left null.

## Offline calculations

Python 3.9+; standard library only:

```sh
python3 tools/geometry.py --microsteps 16 --distance-m 3 --lateral-mm 10
python3 tools/geometry.py --microsteps 16 --motor-ratio 5 --encoder-ratio 1
python3 -m unittest discover -s tests -v
c++ -std=c++11 -Wall -Wextra -pedantic tests/quadrature_test.cpp -o /tmp/antenna-quadrature-test
/tmp/antenna-quadrature-test
c++ -std=c++11 -Wall -Wextra -pedantic tests/motion_test.cpp -o /tmp/antenna-motion-test
/tmp/antenna-motion-test
```

Both ratios mean **shaft revolutions per table revolution**, separately for the
motor and encoder. A motor reduction does not improve encoder resolution when
the encoder is mounted directly on the output table.

## Next steps

1. Preserve the verified original rotor flash backups; EEPROM calibration and
   restoration remain unverified.
2. Record driver chip markings, jumper settings, supply, encoder label/wiring,
   coupling arrangement, and CAD revision.
3. Confirm 2400 signed encoder counts per mechanical revolution in x4 mode.
4. Validate angle repeatability with the final load and coax attached.
5. Validate the implemented bounded motor jogs, zero/probe, stop and fault paths
   against the actual driver/encoder and motor supply before automated sweeps.
6. Add instrument capture after motion/encoder acceptance checks pass.

Do not interpret a software zero as an absolute encoder reference. Power loss,
reset, and an unpowered manual movement invalidate the position reference.
