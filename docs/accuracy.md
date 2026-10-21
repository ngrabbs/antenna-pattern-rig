# Angular resolution, accuracy, and calibration

Let M = full motor steps/revolution (nominally 200), u = driver subdivisions,
P = encoder cycles/channel/revolution (nominally 600), q = quadrature multiplier,
Gm = motor revolutions/table revolution, Ge = encoder revolutions/table revolution.

```
motor increment (degrees)   = 360 / (M * u * Gm)
encoder increment (degrees) = 360 / (P * q * Ge)
alignment angle (degrees)  = atan(lateral displacement / separation) * 180 / pi
```

Direct drive at 1/16 gives 0.1125 degrees/commanded microstep. Encoder x4 gives
0.15 degrees/count. Microsteps are commanded current-vector subdivisions, not
guaranteed equal mechanical angles. The encoder's resolution does not establish
its line-spacing accuracy or the table's absolute pointing accuracy.

An ideal calibrated bin-center convention has +/-0.075-degree quantization for
0.15-degree bins. This excludes zero-reference uncertainty, encoder nonuniformity,
missed counts, shaft/coupler movement, axis tilt and RF boresight alignment. A
manual zero can introduce additional quantization approaching a full count.
Do not add these errors as an RSS budget without justified statistical models.

## Sensible first build targets

- Use 1-degree graph samples initially; a 45-degree beam has ~45 intervals across
  its half-power width. Use 0.5 degrees after validation if useful.
- Treat +/-0.5 to 1 degree as an initial overall pointing-accuracy goal to test,
  not a predicted property of an unbuilt mount.
- Targets separated by 1 degree are 6.667 encoder counts apart. Exact 1-degree
  positions generally do not land on both encoder and motor grids. Store requested
  angle, commanded steps, and measured counts; plot measured angles.
- A 0.1-degree graph grid is finer than this direct-drive encoder can independently
  resolve. More plotted samples or interpolation do not create angle information.
- A 5:1 motor reduction gives 0.0225 degrees/1/16 motor microstep. If the encoder
  stays on the motor, nominal table readout is 0.03 degrees/count, but downstream
  backlash remains invisible. If encoder is on the table, it stays 0.15 degrees/count.

## Distance is a separate error term

Encoder resolution does not improve with distance. Range affects alignment,
far-field validity, signal level, and multipath. At 3 m, a 10 mm lateral alignment
error is about 0.191 degrees. At a 2 m laser target, 0.15 degrees is about 5.24 mm;
0.5 degrees is about 17.45 mm.

For far-field setup start with R >= 2D^2/lambda for BOTH antennas and assemblies,
then use a practical multi-wavelength range and validate it by changing height and
separation. At 2.425 GHz, 10 cm D gives ~0.162 m, whereas 30 cm D gives ~1.46 m.
This formula alone does not guarantee an accurate indoor or ground-reflecting range.

## Acceptance experiment before trusting fine samples

1. Mark a physical zero. Verify 2400 x4 counts over one independently indexed
   encoder revolution, in both directions, at intended speed. Do not infer a
   revolution solely from the uncalibrated motor step count.
2. At standstill, exercise coax/service loop and inspect for count noise and table
   deflection. A steady motor count does not prove a steady antenna.
3. Attach a laser to the TABLE or antenna fixture, not just the motor. At local
   targets near -10, -5, 0, +5, +10 degrees, measure the beam position on a flat
   target normal to the zero ray. If pivot is on that ray, theta=atan(x/L).
   Account for a laser offset from the pivot or calibrate it out; never apply one
   flat-wall scale to a full revolution. Keep the beam below eye level.
4. Approach each target ten times from the same direction, with the final antenna
   and coax attached. Repeat from the opposite direction. Record raw counts and
   observed angle. Separate mean error, spread, and reversal hysteresis.
5. Repeat representative points around the complete intended travel using a
   suitable physical angle reference; the local wall check does not calibrate 360
   degrees. Check axis wobble separately.
6. Repeat with motor holding current on/off if mechanically stable. Check RF noise
   and boresight power as well as optical position.
7. Only claim an accuracy after documenting reference uncertainty, position error,
   hysteresis, repeatability, zero alignment and calibration coverage.

For future encoder feedback use a finite tolerance and settling window. Do not
chase fractional encoder counts or claim a stable value proves true accuracy.
Test reverse sweeps separately; use one consistent approach direction for production
scans if the mechanism shows backlash. Incremental A/B encoders need manual zero
or an added repeatable home/index sensor after power loss/reset.
