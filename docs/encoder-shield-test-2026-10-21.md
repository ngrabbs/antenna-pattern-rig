# Encoder through CNC Shield V3.0: 2026-10-21

## Setup and wiring correction

The user confirmed all driver sockets empty, the EN/GND shunt removed, and no
12-36 V motor supply connected. The Uno/shield/encoder were USB powered. Voltage
between the encoder's red and black wires was reported as +5 V. The same encoder
diagnostic firmware used in the successful bare-Uno test remained installed;
no code changes or reflash were needed for this test.

The initial photo-based identification of signal takeoffs did not yield encoder
counts: all 2035 valid samples stayed at zero, including during the user-reported
revolution. Invalid transitions also stayed zero. A zero error counter did not
prove that signals were reaching the Uno.

With USB disconnected, the user found by continuity that D2 reaches a pin in the
blue X-labeled block below the STEP/DIR header, and D3 reaches the corresponding
Y-labeled block. The green wire was moved to the exact D2-confirmed pin and white
to the D3-confirmed pin. Adjacent pins were left unbridged. Red/black supply wiring
was retained. Firmware continued to use D2/D3; the nominal scale remained 2400
counts/revolution.

This measured pin identification supersedes the earlier photo-based position/color
advice. The record does not assign a signal to every pin in either block or prove
that another shield clone has the same physical layout.

## Retest results

One serial connection remained open during the turn and return. The user indexed
the shaft by hand, then reported overshooting the return position and correcting
back to the original mark.

| Observation | Counts | Nominal angle | Invalid transitions |
| --- | ---: | ---: | ---: |
| Starting reference | 0 | 0 degrees | 0 |
| User-reported one revolution | +2399 | +359.85 degrees | 0 |
| Return to original mark, after overshoot/correction | +3 | +0.45 degrees | 0 |
| Final 10-second stationary window, min/max | +3 / +3 | +0.45 / +0.45 degrees | 0 |

There were 1124 valid numeric samples, with firmware millis strictly increasing
from 100 to 112400. The invalid-transition counter stayed zero throughout. The
capture was stopped gracefully after the final stationary check. Both runs' raw
logs and summaries are retained under ignored `data/local/` and on the Linux host.

## Interpretation

The corrected signal connections restored encoder response through the shield.
Both directions are being counted, and no stationary drift was observed in the
final window. The one-turn endpoint is one count (0.15 nominal degrees) from the
expected 2400; return closure is three counts (0.45 nominal degrees).

Those residuals include manual reference-position uncertainty. This is a successful
basic functional test, not proof of 0.15-degree absolute accuracy, exact line-spacing
accuracy, or immunity to motor electrical noise. Keep the 2400-count conversion;
verify with a repeatable physical reference and repeat under the final motor,
coupling and coax load before claiming table pointing accuracy. Motor drive has
not been tested and motion firmware remains unimplemented.
