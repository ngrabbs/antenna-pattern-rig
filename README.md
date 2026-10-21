# Antenna pattern rig

Single-axis antenna-pattern turntable using an Arduino Uno, CNC Shield V3.0,
Longruner 17HD48002H-22B stepper, and LPD3806-600BM-5G-24C incremental encoder.
RF instrument: NanoVNA-F V2; other instrument models are not yet recorded.

**Status:** planning and encoder diagnostic firmware. No motor-control or RF
instrument automation is implemented yet. No hardware has been flashed or tested.
The Uno already contains satellite-rotor firmware; preserve it before uploading
anything. Purple driver modules are present but their IC model and microstep
configuration are unconfirmed. No license has been selected yet.

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
- [Accuracy and calibration](docs/accuracy.md): error budget, distance geometry,
  laser calibration, and reduction ratios.
- [Measurement procedure](docs/measurement.md): stop/settle/measure, range
  validation, polarization, and data conventions.
- [Preserve existing firmware](docs/firmware-backup.md): read-only backup guidance.
- [Sources](docs/sources.md): component and measurement references.
- [Validation](docs/validation.md): tests performed and remaining hardware checks.
- `firmware/encoder_monitor/`: x4 encoder diagnostic; no step pulses.
- `tools/geometry.py`: offline resolution and alignment calculations; no hardware I/O.
- `data/run-template.json`: metadata template, with unknowns left null.

## Offline calculations

Python 3.9+; standard library only:

```sh
python3 tools/geometry.py --microsteps 16 --distance-m 3 --lateral-mm 10
python3 tools/geometry.py --microsteps 16 --motor-ratio 5 --encoder-ratio 1
python3 -m unittest discover -s tests -v
c++ -std=c++11 -Wall -Wextra -pedantic tests/quadrature_test.cpp -o /tmp/antenna-quadrature-test
/tmp/antenna-quadrature-test
```

Both ratios mean **shaft revolutions per table revolution**, separately for the
motor and encoder. A motor reduction does not improve encoder resolution when
the encoder is mounted directly on the output table.

## Next steps

1. Back up the existing rotor firmware and any calibration/configuration.
2. Record driver chip markings, jumper settings, supply, encoder label/wiring,
   coupling arrangement, and CAD revision.
3. Confirm 2400 signed encoder counts per mechanical revolution in x4 mode.
4. Validate angle repeatability with the final load and coax attached.
5. Implement bounded motor motion, reference/zero handling, stop and fault paths.
6. Add instrument capture after motion/encoder acceptance checks pass.

Do not interpret a software zero as an absolute encoder reference. Power loss,
reset, and an unpowered manual movement invalidate the position reference.
