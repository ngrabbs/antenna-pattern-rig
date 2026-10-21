# RF measurement procedure and future automation

No instrument-control integration is implemented in this repository yet. Identify
the RF source/receiver models and their actual protocols before adding adapters.
For the NanoVNA-F V2, verify commands against SYSJOINT documentation/firmware;
do not assume every NanoVNA clone uses an identical serial protocol.

## State sequence for a future scan

Reference -> move to target -> confirm position within tolerance -> require a
quiet encoder settling window -> trigger RF measurement -> wait for actual
instrument completion -> record -> next target. Record counts before and after RF
capture and reject/flag movement. Have timeouts and abort handling in both motion
and measurement. Test stopped motors for driver RF emissions. A finite deadband
avoids hunting between adjacent encoder bins.

Start with stop-and-measure scans. With continuous motion, angular blur is roughly
angular speed times RF acquisition duration: 2 degrees/s and a 1 s acquisition
smear about 2 degrees. Frequency-swept instruments sample different frequencies at
different times; assign a static position only when the table remains static.

## Range and polarization

- S21 between a stationary source antenna and rotating AUT provides a relative
  received pattern. Fix AGC/gain on SDRs; verify linearity with known attenuators.
- Calibrate/reference cables and preserve connector/cable arrangements. Check
  background with the receiving antenna replaced by a termination.
- A fixed linear source measures one polarization component. Orthogonal H/V power
  measurements, summed in LINEAR power, give total power; a known same-hand CP
  source measures a corresponding CP component with reference imperfections.
- Rotating the entire antenna in azimuth maps a pattern cut. Axial ratio requires
  polarization roll around the propagation/boresight axis at a fixed pointing
  direction; the azimuth turntable alone does not provide that second motion.
- One turntable gives one cut. Remounting the antenna provides other cuts but is
  not automatically a calibrated full-sphere scan. Absolute gain requires a gain
  reference (or another calibrated method); relative patterns do not establish
  radiation efficiency.
- Validate height/distance sensitivity, keep fixtures/people away, keep polarization
  consistent, and repeat sweeps. Revisit a reference angle to identify drift.
- Measure the intended chassis/camera configuration as well as the bare antenna.
- For radiated tests, use frequencies and power levels authorized for the setup.

## Recording

Keep raw linear complex S21 or raw receiver power when available, in addition to
derived dB. Record frequencies individually. Retain raw runs rather than overwriting
them with a normalized graph. Absolute RF phase on a rotating off-center antenna
includes path-length changes and cable flex; do not interpret it as a pure
polarization measurement without a calibrated coherent setup.

Recommended future CSV fields:

```
timestamp_utc,run_id,point_id,requested_deg,commanded_steps,encoder_counts_before,encoder_counts_after,measured_deg,frequency_hz,s21_real,s21_imag,power_dbm,polarization,approach_direction,settle_ms,status
```

Use only fields appropriate to the instrument; blank is not zero. Define 0 degrees
as antenna boresight toward the source; record rotation sign and which viewing
direction defines it. Keep unwrapped positions during acquisition, including cable
travel limits. A full-circle 1-degree plot has 360 unique bins; 0 and 360 are the
same physical pointing, but a repeated endpoint is useful as a closure/drift check.

Start with 1-degree spacing and a trial 0.5-1 s settling interval PLUS the actual
RF acquisition time; measure the required settling rather than treating these
starting values as specifications. A -90 to +90 degree scan has 181 points at
1-degree spacing. A full-circle endpoint-inclusive scan has 361. Timestamp,
temperature, antenna/CAD revision, axis setup and calibration belong with each run.
