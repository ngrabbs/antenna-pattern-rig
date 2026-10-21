# Hardware and proposed wiring

## Recorded inventory

| Part | Known | Still to establish |
| --- | --- | --- |
| Arduino Uno | Original rotor flash backed up; motor-jog firmware now installed and upload-verified | Original source, EEPROM calibration, restoration |
| CNC shield | V3.0; user reports DRV8825 installed in Z for motor bring-up | Z pinout/orientation check, measured powered behavior |
| Driver | Purple carrier; user reads V8825; photo shows two R250 sense resistors | Confirm full chip identity, measured VREF/current limit, microstep jumpers |
| Motor | Longruner 17HD48002H-22B | Confirm its own label/ratings and coil pairing |
| Encoder | LPD3806-600BM-5G-24C; basic 5 V hand-turn test completed on bare Uno | Precisely indexed counts/revolution, noise immunity, mounted repeatability |
| Coupling | User confirms mechanical 1:1 motor/encoder coupling | Shaft alignment, stiffness, slip, backlash |

A Zyltech sheet for the same motor part number lists 1.8 degrees, 1.7 A/phase,
0.59 N m holding torque and 5% step accuracy. Treat this as supporting evidence,
not certification of the Longruner unit. Holding torque is not allowable bearing
load. Confirm coil pairs with an ohmmeter rather than relying on wire colors.

## Preserve the current rig

The original rotor flash was backed up and verified before the encoder diagnostic
was uploaded. Preserve those backups. The original rotor firmware might support useful
serial rotor commands, but its protocol must be identified rather than guessed.
Opening a serial port commonly resets an Uno and can trigger startup motion;
remove motor power before probing it. See firmware-backup.md.

## First encoder test: bare Uno

For the first manual encoder test, the CNC shield is separate from the Uno and
unpowered. Disconnect USB before wiring; leave the motor supply disconnected.
For the stated encoder model, the published connection table is:

| Encoder conductor | Bare Uno connection | Function |
| --- | --- | --- |
| Red | 5V | Encoder supply, not VIN or a motor supply |
| Black | GND | Supply and signal return |
| Green | D2 | A phase |
| White | D3 | B phase |
| Braided shield/drain | Insulated for initial bench test | Not another signal or supply wire |

These match the user's reported conductor colors and the connection table in
[the original encoder application paper](https://eprints.covenantuniversity.edu.ng/18650/1/Oguntosin_2019_J._Phys.__Conf._Ser._1299_012108.pdf).
The actual unit's own labeling takes priority. A/B color variants would reverse
the reported sign; power polarity must be correct. The sketch's internal pull-ups
support an initial slow hand-turn test. Use external ~4.7k pull-ups to 5 V for
stronger edges if needed; never pull inputs to 12/24 V. For later noise control,
terminate the cable braid appropriately at the controller end.

Reconnect USB only after checking connections. Monitor at 115200 baud. Mark a
shaft reference, send `z`, turn one measured mechanical revolution slowly, and
expect approximately +2400 or -2400 counts (360 degrees in magnitude). Reverse
to the mark and check closure. Count drift at rest and invalid transitions should
ideally be zero. A count alone does not prove the reference mark was a full turn.
See [encoder bench procedure](encoder-test.md).

## Proposed shield diagnostic wiring (NOT compatible with arbitrary existing firmware)

The diagnostic sketch uses Uno D2 and D3 for external interrupts. On the common
V3 shield mapping, these are ALSO the X and Y STEP signals. Consequently:

- Disconnect motor power and remove X/Y/A driver modules before diagnostic use.
- Reserve the Z socket for later motor control; do not drive X/Y with this map.
- Remove A-axis clone jumpers and check the actual shield's connectivity.
- The diagnostic holds shared driver ENABLE D8 high and emits no STEP pulses.
- Before a future powered-motion test, confirm Z STEP D4 and Z DIR D7 by checking
  the actual shield. The diagnostic does not implement motion.

### Actual shield takeoffs: continuity check, 2026-10-21

The initial attempt using photo-identified pins in the upper-left STEP/DIR header
produced no encoder counts, although the encoder supply measured 5 V. The user
then checked continuity with power disconnected and reported:

- Uno D2 reaches a pin in the blue **X-labeled block below the STEP/DIR header**.
- Uno D3 reaches a pin in the corresponding **Y-labeled block**.
- All driver sockets are empty.

Use the **exact pin independently confirmed to D2** for encoder green/A and the
**exact pin confirmed to D3** for white/B. This description does not establish that
every pin in an X/Y block is that signal; adjacent pins can carry other nets.
Do not bridge pins or install clone jumpers for the encoder. The earlier
photo-based header position/color advice is superseded by the continuity check.
The electrical requirements remain D2/D3; firmware pin definitions are unchanged.

After moving the wires to those continuity-confirmed pins, the encoder registered
2399 counts on a manually indexed revolution and +3 counts after returning to
the starting mark, with zero invalid transitions and no final stationary drift.
See [shield retest](encoder-shield-test-2026-10-21.md). This confirms functional
operation through these takeoffs; it does not locate every pin on other clones
or establish calibrated angular accuracy.

Remove any shunt bridging the **EN/GND** breakout before mounting/powering this
shield with the diagnostic: the sketch drives D8 HIGH, so a shunt grounding that
net would conflict with the output. The user confirmed this shunt removed and
the 12-36 V motor supply disconnected for the encoder-only shield test.

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

### Observations from 2026-10-21 driver photograph

- The two large resistors next to the adjustment pot are marked **R250**:
  0.25 ohm each, normally one sense resistor per phase, not a parallel pair.
- The user reports the chip marking looks like **V8825**. This is consistent with
  DRV8825, but the full marking is not legible in the photograph. Identification
  remains provisional.
- Assuming DRV8825 and the usual direct VREF circuit, TI specifies
  `I_limit = VREF / (5 * R_sense)`. For 0.25 ohm this becomes
  `I_limit[A] = VREF[V] / 1.25`, or `VREF[V] = 1.25 * I_limit[A]`.

| Full-scale phase current limit | VREF at the IC reference input |
| --- | --- |
| 0.8 A | 1.00 V |
| 1.0 A | 1.25 V |
| 1.2 A | 1.50 V |

An initial 0.8 A / 1.00 V setting was proposed for low-load motor tests; it has
NOT been measured or applied. This is the full-scale chopping current setting,
not DC supply current or a guaranteed current at every microstep. The common
`I = 2 * VREF` shortcut applies to 0.10-ohm boards and does not apply here.
Measure at a verified VREF point relative to driver ground; do not assume an
unidentified pot's metal adjuster is electrically the VREF node. Wiring and module
insertion/removal require power off. See [TI DRV8825 datasheet, section 8.3.2](https://www.ti.com/lit/ds/symlink/drv8825.pdf#page=12).

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
