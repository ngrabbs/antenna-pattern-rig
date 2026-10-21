# First encoder-only hardware test: 2026-10-21

## Setup

- Bare Arduino Uno R3; CNC Shield separate on bench, unpowered.
- LPD3806-600BM-5G-24C wired by the user: red 5V, black GND, green D2, white D3.
- Cable braid insulated; firmware internal pull-ups enabled. No external pull-ups
  were specified for this initial test.
- No motor supply or motor motion.
- Diagnostic compiled inside Docker using Arduino AVR core 1.8.8. Sketch uses
  4604 flash bytes and 237 bytes static RAM. AVRDUDE verified the application upload.
- Uploaded application HEX SHA-256:
  `c38fc4869f2d296553f6faa82ee744a0ac6ec3e0438a7543a7ff08be0845c992`
- Original satellite-rotor flash was backed up before replacement; EEPROM was not
  read, written or verified by this session's tooling.

## Observations

One serial connection stayed open throughout the hand-turn tests. All 1449 parsed
sample timestamps were strictly increasing, from firmware millis 100 to 144900.
The raw stream includes a short startup fragment before the diagnostic banner;
only valid five-field numeric samples were used in the results.

| Observation | Encoder counts | Nominal angle | Invalid transitions |
| --- | ---: | ---: | ---: |
| Initial reference; unchanged until first motion at millis 55000 | 0 | 0 degrees | 0 |
| User reported one mechanical revolution and stopped | -2381 | -357.15 degrees | 0 |
| User reversed to original starting mark | -10 | -1.5 degrees | 0 |
| Final 10-second stationary window, min/max | -10 / -10 | -1.5 / -1.5 degrees | 0 |

The invalid-transition counter remained zero across all parsed samples. The capture
was stopped gracefully after the final stationary check. Raw serial data and
machine-readable status/summary are retained in ignored `data/local/` and on the
Linux host; they are not part of the public repository.

## Interpretation and next work

This establishes basic live encoder operation, count changes in both directions,
and no observed stationary drift in the final window under these bench conditions.
The first turn differs from the nominal -2400 by 19 counts (2.85 degrees), and
return closure differs from zero by 10 counts (1.5 degrees). Those are combined
manual-reference/counting observations. They do not isolate hand-index error,
missed transitions, or encoder scale error, so exact counts per physical revolution
and angular accuracy are not yet verified. Zero invalid transitions alone cannot
exclude complete quadrature cycles being missed.

Keep the nominal 2400-count conversion. Next use a repeatable physical angular
reference and several complete revolutions, then repeat with external pull-ups and
the final motor/coax environment if needed. Motor drive and table repeatability
are still separate bring-up steps.
