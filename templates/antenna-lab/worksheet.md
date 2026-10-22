---
title: "Antenna characterization"
subtitle: "Lab worksheet | Template v1.0"
author: "Operator: TBD | Antenna ID: TBD"
date: "Session date: TBD"
lang: en-US
---

# 01 / Define the session

**Session ID:** TBD  
**Antenna ID / revision:** TBD  
**Operator / reviewer:** TBD / TBD  
**Local date, time and zone / UTC start:** TBD / TBD  
**Record status:** NOT RUN  
**Last completed stage / next action:** TBD / TBD

Work through one stage at a time. Replace `TBD` with a value, evidence link, or
`N/A - reason`. Check a box only after doing the action. An unchecked box is not
a failed test. Use **NOT RUN, PASS, FAIL, INCONCLUSIVE, or N/A** for test verdicts;
use **EXPLORATORY** for observations without predeclared acceptance limits.

## What are we trying to establish?

**Use case / installation:** TBD.  
**Question this session should answer:** TBD.  
**Design claim or simulation to compare against, including source and revision:** TBD.

| Measurement / claim | In scope? | Requirement and source | Planned evidence |
| --- | --- | --- | --- |
| Match / impedance bandwidth | TBD | TBD | VNA trace |
| Relative pattern / beamwidth | TBD | TBD | Angle-power data |
| Position / RF repeatability | TBD | TBD | Repeated runs |
| Polarization / axial ratio | TBD | TBD | Optional stage 10 |
| Absolute gain | TBD | TBD | Optional stage 11 |

- [ ] Identify the physical antenna and exact band; a "2.2/2.4 GHz design" does not establish that this unit covers both bands.
- [ ] Declare acceptance thresholds before testing, or label the run exploratory.
- [ ] Choose the receiving/transmitting configuration and accessible angular cut.
- [ ] Assign a unique run directory; retain failed attempts and raw data.

**Session folder / repository commit:** TBD / TBD.  
**What this setup will not establish today:** TBD.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 02 / Identify and inspect the hardware

**DUT model, serial or build ID:** TBD.  
**Design files / board revision / assembly date:** TBD / TBD / TBD.  
**Nominal frequency range / polarization / feed impedance:** TBD / TBD / TBD.

| Physical item | Measured value / observation | Evidence |
| --- | --- | --- |
| Patch / radiator dimensions, mm | TBD | TBD |
| Ground plane and overall dimensions, mm | TBD | TBD |
| Gap, substrate and critical spacings | TBD | TBD |
| Feed connector, hybrid / load, unused ports | TBD | TBD |
| Camera, chassis, radome and mounting state | TBD | TBD |
| Damage, joints, shorts or assembly deviations | TBD | TBD |

- [ ] Photograph front, back, feed, installed mount, and the complete RF path.
- [ ] Check dimensions and assembly against this design's drawings.
- [ ] Confirm intended electrical bonds; do not infer a fault just because a design has a DC short to ground.
- [ ] Terminate unused RF ports as the design requires.
- [ ] Identify any feed hybrid or isolation load that could absorb reflected power.
- [ ] Record the configuration being tested: bare antenna or representative installed assembly.

## Instruments and actual connections

| Role | Model / ID / firmware | Port / cable / calibration record |
| --- | --- | --- |
| VNA | NanoVNA-F V2; confirm: TBD | TBD |
| RF source | LibreSDR B210-compatible; confirm: TBD | TXRA / pad / log: TBD |
| Power receiver | tinySA Ultra; confirm: TBD | RF input / coax: TBD |
| Positioner | Uno + CNC shield + Z driver; confirm: TBD | Encoder / microsteps: TBD |
| References / pads / adapters | TBD | Value, band and uncertainty: TBD |

**Photo index and connection sketch:** TBD.  
**Changes since the previous run:** TBD.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 03 / Measure impedance and matching

**VNA model / firmware / calibration kit ID:** TBD / TBD / TBD.  
**Calibration time / verification artifact:** TBD / TBD.  
**Reference plane (where the calibration standards attach):** TBD.

- [ ] Disable and disconnect external RF sources from the DUT before connecting the VNA.
- [ ] Inspect connectors and calibrate open/short/load at the chosen reference plane.
- [ ] Verify the calibration with a suitable known load or verification device; record its result.
- [ ] Sweep each intended band, including margins; choose enough frequency points to resolve features.
- [ ] Save raw complex S11 or Touchstone data and a clearly labeled plot.
- [ ] Repeat after reconnecting the DUT; investigate a material change.

**Start / stop frequency, MHz:** TBD / TBD.  
**Points / averaging / power setting if adjustable:** TBD / TBD / TBD.  
**Requirement, e.g. maximum VSWR across a stated band:** TBD, source: TBD.

| Quantity | Result | Evidence / qualification |
| --- | --- | --- |
| Minimum S11 magnitude, dB, and frequency, MHz | TBD | TBD |
| VSWR at selected test frequencies | TBD | TBD |
| Impedance at selected frequencies, ohms | TBD | TBD |
| Band edges meeting declared match limit, MHz | TBD | TBD |
| Reconnect change | TBD | TBD |

Record S11 log magnitude as a signed value: e.g. -15 dB. Positive return loss for
that example is 15 dB. Keep the convention explicit. A narrow S11 dip is an
impedance feature; it does not alone locate the frequency of best radiation.

**Interpretation:** A good input match does not prove gain or efficiency. With a
hybrid-fed antenna, isolation-load dissipation can conceal poor radiator matching.
Record what was measured at the input and leave radiation claims for later stages.

**Selected RF test frequencies and reasons:** TBD.  
**Plots / raw files:** TBD.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 04 / Qualify positioning with this load

**Firmware commit / boot tag:** TBD / TBD.  
**Microstep jumpers / software setting:** TBD / TBD.  
**Motor supply voltage / current limit / measured VREF:** TBD / TBD / TBD.  
**Encoder location and transmission to the actual antenna axis:** TBD.

Current rig history: nominal 2400 x4 counts per encoder revolution, or 0.15 degrees
per count. Equal grooved pulleys and a friction belt are nominally 1:1. Confirm
this session's mechanism. Counts at the encoder are not independent proof of
antenna angle or absence of downstream slip.

- [ ] Make wiring/jumper changes with motor power and USB disconnected; verify driver orientation.
- [ ] Establish the persistent serial session with motor power off; verify disabled startup and matching microstep configuration.
- [ ] Secure the antenna and provide coax clearance over the entire bounded travel.
- [ ] Confirm power at the driver, then enable motor power and command a small supervised movement.
- [ ] Check encoder sign, valid transitions and observed movement; stop on missing feedback.
- [ ] Establish a physical reference and zero; document the view defining positive rotation.
- [ ] Exercise outbound/return travel with the final antenna and coax attached.

**Physical zero / positive direction / permitted travel:** TBD / TBD / TBD.  
**Declared positioning and closure limits:** TBD.  
**Independent angle reference, uncertainty and coverage, if available:** TBD.

| Trial / direction | Requested angle, deg | Encoder angle, deg | Independent angle, deg | Closure / invalids / faults |
| --- | --- | --- | --- | --- |
| TBD | TBD | TBD | TBD | TBD |
| TBD | TBD | TBD | TBD | TBD |
| TBD | TBD | TBD | TBD | TBD |

**Loaded repeatability / reversal effect / post-disable relaxation:** TBD.  
**Chosen motor holding state during RF capture:** TBD; position stability: TBD.  
**Accuracy claim supported by the reference, or "not calibrated":** TBD.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 05 / Establish the RF range and angle convention

**Test frequency list, MHz / wavelength at each, m:** TBD / TBD.  
**Source antenna model, band and polarization:** TBD / TBD / TBD.  
**DUT cut / installed orientation:** TBD / TBD.

| Geometry / environment | Value / evidence |
| --- | --- |
| Antenna separation and measurement endpoints, m | TBD |
| Source and DUT heights, m | TBD / TBD |
| Largest source and DUT assembly dimensions, m | TBD / TBD |
| Rotation axis, radiator center and boresight alignment | TBD |
| Floor, walls, nearby metal and people | TBD |
| Mount material, coax route, bend loop and strain relief | TBD |
| Temperature / weather if applicable | TBD |

Use wavelength = speed of light / frequency. For both antennas, check the usual
far-field starting criterion **R >= 2D squared / wavelength** and choose a
practical multi-wavelength separation. The formula alone does not establish a
reflection-free range, particularly for small antennas or an indoor bench.

**Calculated source / DUT far-field distances, m:** TBD / TBD.  
**Chosen separation and justification:** TBD.  
**Proposed height or distance change for checking reflections:** TBD.

- [ ] Verify both antennas operate at each selected frequency.
- [ ] Define mechanical zero separately from RF peak direction.
- [ ] Record the viewing direction and sign convention with a photo or sketch.
- [ ] Keep source aim and source polarization fixed within a scan.
- [ ] Identify the measured component: linear projection, same-hand CP, or other.
- [ ] Keep fixtures and cables repeatable; plan one environment sensitivity check.

**Angle mapping:** encoder zero = TBD; boresight is at TBD or NOT ESTABLISHED.  
**Coverage:** TBD to TBD degrees, in plane TBD. A 180-degree excursion and return
covers the same cut twice; it does not establish a full-circle rear pattern.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 06 / Validate the source and power measurement

**RF chain:** source port TBD -> pad/filter TBD -> source antenna TBD -> air path
-> DUT -> cable TBD -> receiver port TBD.

**Source settings:** frequency TBD MHz; gain TBD; waveform / IQ amplitude TBD;
finite duration / stop method TBD. Software gain is not calibrated output dBm.  
**Receiver settings:** mode TBD; span TBD; RBW TBD; attenuation TBD; LNA TBD;
detector / averaging TBD; AGC OFF or N/A: TBD.  
**Power estimator and exact frequency bins / bandwidth:** TBD.

- [ ] Confirm selected frequency/power are authorized and the source port is terminated by the intended RF chain.
- [ ] Verify the radio image and channel mapping; keep transmitter off until the receive path is ready.
- [ ] Warm up instruments and freeze gain, attenuation and measurement settings.
- [ ] Record transmitter-off background using the same estimator as the scan.
- [ ] Start a finite low-level tone; verify it appears and disappears with TX on/off.
- [ ] Distinguish the wanted tone from LO leakage, spurs and other transmitters.
- [ ] Insert a known extra pad with TX off for connection changes; verify the expected level change within a declared tolerance.
- [ ] Compare driver-holding and disabled RF readings only if the DUT stays fixed.

| Check | Declared limit | Observation / evidence | Verdict |
| --- | --- | --- | --- |
| TX-off background statistic, dBm | TBD | TBD | NOT RUN |
| TX-on signal and margin, dB | TBD | TBD | NOT RUN |
| Extra pad: expected / observed change, dB | TBD | TBD | NOT RUN |
| Fixed-angle drift over stated time | TBD | TBD | NOT RUN |
| Driver interference / position change | TBD | TBD | NOT RUN |

**Usable level range / saturation and floor flags:** TBD.  
**Calibration or relative-only status:** TBD.  
**Averaging / noise statistic and number of samples:** TBD.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 07 / Plan and acquire the pattern

**Run ID / timestamp / operator:** TBD / TBD / TBD.  
**Frequency / source polarization / antenna cut:** TBD / TBD / TBD.  
**Scan mode:** TBD - stopped points or exploratory continuous motion.  
**Start / stop / spacing / speed:** TBD / TBD / TBD / TBD.  
**Settling criterion / acquisition time / repeats per point:** TBD / TBD / TBD.

Stopped acquisition is preferred for qualification: move -> verify position ->
require a quiet interval -> acquire RF -> verify position again -> save -> advance.
For moving acquisition, record timing and angular span. **Angular blur is roughly
speed multiplied by acquisition time**; interpolation does not remove that blur.

**Motion tolerance / heartbeat / maximum travel / abort behavior:** TBD.  
**RF completion / timeout handling and finite TX duration:** TBD.  
**Raw data folder / configuration snapshot / code commit:** TBD / TBD / TBD.

- [ ] Require stages 02-06 to be complete, or record why proceeding is exploratory.
- [ ] Confirm clearance and the intended motor-power state immediately before motion.
- [ ] Record a fixed-angle reference reading before scanning.
- [ ] Save actual encoder counts and RF acquisition timestamps, not just target angles.
- [ ] Save raw power or complex S21, instrument settings and validity flags.
- [ ] Monitor feedback, receiver saturation, signal margin, communications and travel limits.
- [ ] Stop on a fault; retain the failed attempt and use a new run ID for the retry.
- [ ] Repeat the same angular cut on return and revisit the reference angle.

| Acquisition record | Value / file |
| --- | --- |
| Outward / return raw records | TBD / TBD |
| Accepted / rejected / missing samples | TBD / TBD / TBD |
| Reference power before / after, dBm | TBD / TBD |
| End angle / closure / post-disable position | TBD / TBD / TBD |
| Faults, changes and interrupted attempts | TBD |

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 08 / Check repeatability and reduce the data

**Comparison limits declared before these repeats:** TBD.  
**Runs being compared and what changed between them:** TBD.

- [ ] Repeat at least one complete cut; retain outward and return separately.
- [ ] Repeat representative angles after changing height or separation to test range sensitivity.
- [ ] Compare the same configuration before/after reconnecting or remounting when relevant.
- [ ] Verify the intended tone/component is used consistently; exclude identified spurs.
- [ ] Plot actual measured angle and flag floor-limited or saturated samples.
- [ ] Preserve raw levels alongside normalized curves; report bin size and rejected points.
- [ ] Extract beamwidth only when both -3 dB crossings are resolved in the measured cut.

Normalize a power trace by subtracting its chosen peak in dB. For comparisons,
record whether traces share one normalization reference or have separate peaks;
separate normalization hides amplitude drift. Average powers in linear units
before converting to dB. If using medians in dB, label them as medians.

| Result | Value / convention | Evidence / limit |
| --- | --- | --- |
| Peak direction relative to defined zero | TBD | TBD |
| -3 dB crossings / beamwidth, degrees | TBD / TBD | TBD |
| Relative sidelobes / null bounds in measured cut | TBD | TBD |
| Outward-return difference and statistic | TBD | TBD |
| Run-to-run / reference-power drift | TBD | TBD |
| Height/distance sensitivity | TBD | TBD |
| Angle timing, bin width and blur | TBD | TBD |

**Comparison to design claim / simulation and configuration differences:** TBD.  
**Dominant uncertainties:** angle reference TBD; positioning TBD; RF response TBD;
polarization TBD; reflections TBD; timing TBD; repeatability TBD.

Do not combine these as an RSS uncertainty budget without justified models.
This rig's encoder endpoint tolerance is not independently calibrated antenna
pointing accuracy. A weak reading at the background floor is a null-depth bound.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 09 / Close the session and state the result

## Findings suitable for a reader who was not in the lab

**Antenna and tested configuration:** TBD.  
**Question answered / frequency and cut:** TBD.  
**Method, range, polarization and angle reference:** TBD.  
**Key measured result:** TBD.  
**Repeatability / uncertainty or explicit limitation:** TBD.

| Original requirement / claim | Measured result | Verdict | Evidence |
| --- | --- | --- | --- |
| TBD | TBD | NOT RUN | TBD |
| TBD | TBD | NOT RUN | TBD |
| TBD | TBD | NOT RUN | TBD |

**Supported claims:** TBD.  
**Not established / inconclusive findings:** TBD.  
**Next improvement or test:** TBD.

## Preserve evidence and leave the rig in a known state

- [ ] Stop RF generation and verify the transmitter process/output is off.
- [ ] Disable the driver; record any post-disable movement; turn off motor supply.
- [ ] Close serial sessions before unplugging; retain fault and failed-run logs.
- [ ] Save raw data, calibration records, photos, settings and analysis version.
- [ ] Link plots to their run IDs, frequency, cut, polarization and normalization.
- [ ] Update the current stage, unresolved items and next action on page 1.
- [ ] Review the Markdown in GitHub; export only the reviewed revision for sharing.
- [ ] Check the exported PDF for readable figures, complete tables and correct labels.

**Raw archive / public evidence paths:** TBD / TBD.  
**Data checksums / analysis commit / report revision:** TBD / TBD / TBD.  
**Stop time local + zone / UTC:** TBD / TBD.  
**TX off / driver disabled / bench supply off confirmed by:** TBD.  
**Operator review / independent review, if applicable:** TBD / TBD.

**Session disposition:** NOT RUN - replace after reviewing evidence.  
**Optional stage 10 / 11 disposition and reasons:** TBD / TBD.

<!-- pagebreak -->

# 10 / Optional: polarization characterization

**Module disposition:** NOT RUN.  
**Claim sought:** TBD - linear component pattern, axial ratio, or CP handedness.  
**Additional reference / roll fixture available:** TBD.

The azimuth turntable alone does not provide polarization roll about the
propagation axis. Keep the DUT pointing direction fixed while rotating a linear
probe/source about that axis, or use qualified circular-polarization references.

## Orthogonal linear components

- [ ] Record source orientation 1 and an orthogonal orientation 2, with the same geometry and source level.
- [ ] Check antenna phase-center/aim and level stability after changing orientation.
- [ ] Acquire both components on the same angle grid; retain each separately.
- [ ] Sum powers in linear units only when component calibrations and conditions are consistent.

**Orientation 1 / 2 definitions:** TBD / TBD.  
**Run IDs / level correction between orientations:** TBD / TBD.  
**Combined-power method and evidence:** TBD.

## Axial ratio or handedness

- [ ] For axial ratio, sweep linear-probe roll at a fixed pointing angle; measure enough roll angles to find maximum and minimum.
- [ ] Keep frequency, distance and power fixed; check both extrema are above background.
- [ ] State the plane-wave and probe assumptions. Under those assumptions, axial ratio in dB is the maximum-minus-minimum received power in dB.
- [ ] Use a known polarization reference and explicit viewing convention before claiming RHCP/LHCP.

| Frequency / pointing angle | Roll range / increments | Max / min, dBm | Axial ratio, dB / uncertainty |
| --- | --- | --- | --- |
| TBD | TBD | TBD | TBD |
| TBD | TBD | TBD | TBD |

Two H/V readings alone do not establish axial ratio or handedness. Matching
source/DUT CP conventions and reference purity matter in a CP component test.

**Supported polarization claim / unresolved assumptions:** TBD / TBD.  
**Evidence files:** TBD.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.

<!-- pagebreak -->

# 11 / Optional: absolute gain by substitution

**Module disposition:** NOT RUN.  
**Known-gain reference antenna / calibration source:** TBD / TBD.  
**Reference frequency, gain convention, polarization and uncertainty:** TBD.

A normalized pattern alone cannot supply gain in dBi or dBic. This module requires
a credible gain reference and compatible polarization, geometry and mismatch
conventions. Without these, record a relative substitution comparison only.

- [ ] Confirm the reference calibration covers the test frequency and states gain versus realized gain.
- [ ] Check polarization compatibility/corrections and far-field/range conditions for both antennas.
- [ ] Place the reference and DUT at the same measurement position and pointing direction in turn.
- [ ] Preserve source level, geometry and receiver settings; check drift before and after substitution.
- [ ] Record differences in receive cable loss and connector/adaptor configuration.
- [ ] Account for feed mismatch if required by the stated gain convention; do not silently mix conventions.
- [ ] Repeat substitutions and state the combined uncertainty with defensible assumptions.

| Substitution record | Reference | DUT |
| --- | --- | --- |
| Frequency / orientation | TBD | TBD |
| Received level, dBm | TBD | TBD |
| Receive-path loss, dB | TBD | TBD |
| Polarization / mismatch treatment | TBD | TBD |
| Run ID / evidence | TBD | TBD |

For compatible conditions and conventions, the received-level difference gives
the gain difference after path-loss and other documented corrections. Write the
actual calculation below with units and correction signs; do not apply a blanket
3 dB correction to arbitrary polarization states.

**Calculation / uncertainty budget:** TBD.  
**Result and convention (dBi, dBic, realized gain, etc.):** TBD.  
**Repeat substitutions / drift:** TBD.  
**Unresolved corrections or reasons for a relative-only result:** TBD.

Radiation efficiency remains unestablished by this one-cut substitution test.

**Stage verdict / evidence / next action:** NOT RUN / TBD / TBD.
