# Current rig profile - starting facts, not a completed test

This is a copy aid for stage 02. Confirm physical settings each session. Historical
observations are dated 2026-10-21; they are not fresh readings or a calibration.

| Item | Recorded setup | Confirm or measure for each session |
| --- | --- | --- |
| Position controller | Uno R3; CNC shield V3 clone; Z STEP D4, DIR D7, active-low enable D8 | Serial identity, boot disabled, firmware commit, actual continuity if wiring changed |
| Driver | Purple presumed DRV8825; R250 sense resistors; measured VREF 0.993 V | Chip identity, orientation, VREF, supply, cooling |
| Mode / motor | Only M2 jumpered; CONFIG 16; Longruner 17HD48002H-22B | Jumper/software agreement; label and coil pairs |
| Power | 12 V bench supply; 1 A supply-current limit in earlier tests | Present voltage/current limit; supply current is not phase current |
| Encoder | LPD3806 family; A/B x4; nominal 2400 counts/revolution; 0.15 deg/count | Indexed scale, location relative to antenna axis, coupling, invalids |
| Mechanics | Equal grooved pulleys, friction belt; nominal 1:1 | Slip, creep, axis tilt and actual antenna movement with coax/load |
| VNA | NanoVNA-F V2 | Firmware, kit, calibration plane, load verification |
| RF source | LibreSDR B210mini, serial D5I16MA; custom FPGA; UHD subdevice A:A/TXRA used | Port identity, image hash, frequency, gain and tone duration |
| Source path | 10 dB pad and log-periodic antenna | Pad response, source antenna band/model, polarization and aim |
| RF receiver | tinySA Ultra; RF input | Firmware, Ultra mode, RBW, attenuation, LNA, detector, estimator and calibration |
| Other available radios | Pluto, HackRF, RTL-SDR | Exact model and frequency support before assigning an alternate role |

## Existing software capability

- `firmware/motor_jog` v6 boots disabled and requires explicit commands. A persistent
  host session avoids a reset between movements. Opening serial/uploading resets
  the Uno; establish or change the connection with motor power off.
- FEEDBACK180 has been exercised with antenna and coax attached. It controls the
  encoder endpoints of one 180-degree outward-and-return excursion.
- General arbitrary-angle closed-loop stop-and-measure automation is **not yet
  implemented**. Do not infer that stage 07 can request an arbitrary encoder target
  through the existing FEEDBACK180 command. Select a supported exploratory scan
  or implement and validate the missing controller mode first.
- `tools/rf` generates a bounded tone, captures tinySA traces and plots a moving
  scan. Capture/analysis frequency is currently fixed at 2400 MHz; retuning the
  source alone does not change these scripts' frequency bins.
- Source gain and IQ amplitude are uncalibrated output controls. The 10 dB pad does
  not by itself establish radiated power or legal operating conditions.

The previous exploratory scan used 30 kHz RBW, 10 dB receiver attenuation, LNA off,
41 points over 2399.5-2400.5 MHz, and approximately 0.30 s captures. It excluded
LO leakage at 2399.9 MHz from the intended 2400 MHz tone estimator. Those are a
historical reproducible configuration, not required settings for every antenna.

## Evidence and references

- [First exploratory RF result](../../results/2026-10-21-rf-quick/README.md): the
  measurement chain worked, but distance/height were not recorded and the result
  was not calibrated gain or free-space pointing accuracy.
- [Motor observations](../../docs/motor-test-2026-10-21.md) and
  [angular accuracy](../../docs/accuracy.md).
- [RF script configuration](../../tools/rf/README.md) and
  [tinySA USB interface](https://tinysa.org/wiki/pmwiki.php?n=Main.USBInterface).
- [Polarization measurement methods](https://www.antenna-theory.com/measurements/polarization.php)
  and [radiation pattern/gain methods](https://www.antenna-theory.com/measurements/radpattern.php).

Use the DUT's actual design revision and primary instrument documentation. The
linked CubeSat cross-patch design has alternative band-specific variants and a
hybrid/isolation load; do not apply its claims to an unidentified patch.
