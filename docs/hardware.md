# Hardware and proposed wiring

## Recorded inventory

| Part | Known | Still to establish |
| --- | --- | --- |
| Arduino Uno | Existing satellite-rotor firmware installed | Original source, protocol, calibration, backup |
| CNC shield | V3.0 | Clone pinout, populated axis, A-axis clone jumpers |
| Driver | Purple carrier board | Chip identity, sense resistors, current limit, microstep jumpers |
| Motor | Longruner 17HD48002H-22B | Confirm its own label/ratings and coil pairing |
| Encoder | LPD3806-600BM-5G-24C | Own wiring sheet, actual cycles/revolution, electrical output |
| Coupling | Printed motor/encoder mount under construction | Shaft alignment, stiffness, slip, backlash |

A Zyltech sheet for the same motor part number lists 1.8 degrees, 1.7 A/phase,
0.59 N m holding torque and 5% step accuracy. Treat this as supporting evidence,
not certification of the Longruner unit. Holding torque is not allowable bearing
load. Confirm coil pairs with an ohmmeter rather than relying on wire colors.

## Preserve the current rig

Do not upload the diagnostic sketch until the working firmware has been backed
up and the rewiring is deliberate. Existing firmware might already support useful
serial rotor commands, but its protocol must be identified rather than guessed.
Opening a serial port commonly resets an Uno and can trigger startup motion;
remove motor power before probing it. See firmware-backup.md.

## Proposed diagnostic wiring (NOT compatible with arbitrary existing firmware)

The diagnostic sketch uses Uno D2 and D3 for external interrupts. On the common
V3 shield mapping, these are ALSO the X and Y STEP signals. Consequently:

- Disconnect motor power and remove X/Y/A driver modules before diagnostic use.
- Reserve the Z socket for later motor control; do not drive X/Y with this map.
- Remove A-axis clone jumpers and check the actual shield's connectivity.
- The diagnostic holds shared driver ENABLE D8 high and emits no STEP pulses.
- Before a future powered-motion test, confirm Z STEP D4 and Z DIR D7 by checking
  the actual shield. The diagnostic does not implement motion.

| Signal | Proposed Uno pin | Notes |
| --- | --- | --- |
| Encoder A | D2 | Also X STEP on common V3 boards |
| Encoder B | D3 | Also Y STEP on common V3 boards |
| Driver disable | D8 HIGH | Common active-low enable on V3; verify board |
| Encoder supply | Regulated 5 V | Verify unit supports 5 V |
| Encoder return | GND | Common logic ground |

Similar encoders are specified as 5-24 V supply, NPN open collector. Verify this
on the supplied unit. Pull each output up to **5 V**, e.g. with 4.7k resistors;
do not pull it to a 12/24 V motor rail or feed high voltage to an Uno input.
The sketch enables weak internal pull-ups as a fallback. Scope A and B to verify
clean 0-5 V levels and quadrature; external pull-ups are preferable for long leads.
Use the unit's wiring sheet; published cable colors are not reliable across clones.

Keep encoder signals separated from motor wiring; use signal/ground twisted pairs
where practical. The example uses short ISRs, x4 decoding, atomic snapshots,
invalid-transition counting, and raw signed counts. Invalid transitions reveal
some faults, but cannot detect every missed full quadrature cycle.

## Driver configuration

Purple is consistent with Pololu-style DRV8825 carriers but does not prove the
chip identity. DRV8825 supports up to 1/32; A4988 up to 1/16. The same three
HIGH mode inputs mean different maximum subdivision on those two drivers.
Read the IC marking and inspect jumpers before setting software steps/revolution.
Confirm by comparing commanded pulses against an independently observed revolution.

Set current limits using the actual driver's sense resistance and documentation;
a VREF formula for a different clone may be wrong. Supply current is not coil
current. Do not unplug a motor from an energized driver. Maintain adequate driver
cooling. Holding current can radiate interference; compare holding/disabled states
only if disabling does not let the antenna move.

## Mechanics

- Let a table bearing carry the load. The encoder is a sensor, not the thrust
  bearing; avoid forcing it to carry table weight or bending loads.
- Use a suitable coupling for shaft misalignment. A printed rigid coupler can
  impose side load; a compliant coupling can introduce torsional deflection.
- A motor-mounted encoder cannot observe slip/flex/backlash downstream. Prefer an
  encoder sensing the table axis when the final mechanism permits it.
- Keep antenna boresight, table reference and axis squareness measurable. Center
  the antenna's useful radiating region close to the rotation axis.
- Balance the load and strain-relieve coax. Test with its actual service loop.
- Keep metal motors/encoder bodies below and away from the radiating aperture,
  using a nonconductive mast where practical. A PLA support is not RF-invisible;
  test mount effects. Account for printed-part creep and temperature drift.
- Use bounded sweeps and unwind; an ordinary slip ring is not a characterized
  2.4 GHz rotary joint. Do not continuously twist the RF coax.
