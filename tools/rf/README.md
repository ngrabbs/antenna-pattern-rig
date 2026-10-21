# Exploratory moving RF capture

These scripts were exercised on 2026-10-21 with LibreSDR B210mini serial D5I16MA,
custom FPGA SHA256 7f0e329b4724e0d919a06b9924aaa3c77305b6f25038deebd6171657ac7c8a99,
and tinySA4_v1.4-156-g4eb315d (Ultra). Install dependencies in Docker, not on the
m75q host. `Dockerfile` builds antenna-rig-rf:20261021; `Dockerfile.plot` adds plots.

- `tone.py` explicitly starts a finite 2400 MHz test tone on subdevice A:A/TXRA.
  UHD gain 50 and IQ amplitude 0.1 were used through a 10 dB pad and log antenna.
  Gain is not calibrated output dBm. The LO is 100 kHz below the intended tone;
  measurable LO leakage is excluded from the analysis. The FPGA is supplied as a
  read-only external file; do not replace it with a standard B210 image.
- `capture.py` never moves the motor or enables the transmitter. It reads the
  existing motion session's latest.json and records 41 tinySA frequency bins from
  2399.5 to 2400.5 MHz, 30 kHz RBW, 10 dB attenuation, LNA off and averaging off.
  Each capture takes about 0.30 s. Preserve its full JSONL including before/after
  telemetry and the raw receiver response. Output files must not already exist.
- `plot.py` reads a transmitter-off baseline, RF JSONL and timestamped motor
  serial.log. It estimates angle by interpolating encoder receive timestamps at
  each acquisition midpoint. This is approximate software timing, not hardware
  synchronization. It uses the largest of three bins within 25 kHz of 2400 MHz,
  excludes LO leakage at 2399.9 MHz, and plots medians in 2-degree bins. Frequency-
  swept bin timing and encoder telemetry latency limit angular precision.

Start capture and a bounded tone, verify a received signal, then explicitly run
one supervised FEEDBACK180 through the existing persistent motion session. Check
that RF processes and motion telemetry remain live; abort/STOP on errors. Stop
transmission and capture after the motor disables. No scan starts automatically
from these files. The initial mounting direction is relative zero, not established
boresight. A 180-degree excursion and return does not measure a full 360-degree
pattern. Weak bins near the TX-off floor are upper limits on a null, not gain.

Example offline analysis inside the plotting container (mount the private data):

```
python3 plot.py --capture sweep-retry.jsonl --baseline baseline.jsonl \
  --motion serial-rf-completed.log --output figures
```

Use a frequency and radiated power authorized for the setup. Current capture
frequency is fixed at 2400 MHz; changing the tone frequency also requires changing
capture and analysis ranges. Raw logs and FPGA images remain private.
