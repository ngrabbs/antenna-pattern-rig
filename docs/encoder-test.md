# Encoder-only bench test

Hardware: bare ATmega328P Uno with CNC shield removed and motor power off.
Firmware: `firmware/encoder_monitor`; no motor motion commands are implemented.
The first test is recorded in [encoder-test-2026-10-21.md](encoder-test-2026-10-21.md).
The subsequent shield wiring correction and successful retest are recorded in
[encoder-shield-test-2026-10-21.md](encoder-shield-test-2026-10-21.md). Use
continuity-confirmed takeoffs on the actual shield before applying power.

## Wiring, with USB disconnected

- Red -> Uno 5V.
- Black -> Uno GND.
- Green (published A) -> Uno D2.
- White (published B) -> Uno D3.
- Braid/drain -> insulated and left disconnected for this initial bench test.

This color map follows the [LPD3806 application paper](https://eprints.covenantuniversity.edu.ng/18650/1/Oguntosin_2019_J._Phys.__Conf._Ser._1299_012108.pdf);
check any contrary labeling on the actual unit before applying power. Do not use
VIN or a motor rail for this test. Internal pull-ups are enabled in firmware;
external 4.7k pull-ups from A and B to 5 V can improve long-cable/noisy performance.

## Test sequence

1. Reconnect USB after inspection. Wait for serial monitoring to be attached at
   115200 baud. Opening the port can reset the Uno and start a new count session.
2. Leave the shaft still for 10 seconds; record count drift and invalid transitions.
3. Put a reference mark on the shaft and housing. Send lowercase `z` while still.
4. Slowly turn one physical revolution to the reference mark; stop and record.
   Expect magnitude 2400 counts and 360 degrees with the assumed 600-cycle unit.
5. Reverse one revolution back to the original pose; record the closure error.
6. Repeat several times and test modestly faster hand rotation. The decoder counts
   every edge in interrupts; serial reports are only 10 Hz and do not limit edge
   acquisition to 10 Hz.

Signed direction is a software convention. Swap A/B or invert sign later to match
the chosen fixture convention. Do not declare physical CW without specifying
the viewing direction.

`z` increments reference_id and establishes a manual relative zero. It does not
home the fixture. Invalid-transition counts are cumulative until reset, so compare
their increase over each test. A quiet count and zero invalid transitions cannot
prove no complete quadrature cycles were missed; the physical closure/revolution
tests are necessary.

For a first pass, report observed start/end counts rather than assigning an angle
accuracy specification. If counts are erratic, check grounds, open-collector pull-ups,
connections and signal levels with a scope before changing the mechanical scale.

## Docker build and capture

Build `tools/readback/Dockerfile` as `antenna-rig-readback:20261021` first, then
build the compiler layer:

```sh
docker build -f tools/readback/Dockerfile.build -t antenna-rig-build:20261021 .
```

Compile in the build image using a source directory mounted read-only and a
separate writable build directory. Upload only the application `.ino.hex`, not
the `.with_bootloader.hex`, with the actual Uno positively identified and shield
separate. The tested upload used AVRDUDE `-p atmega328p -c arduino -b 115200 -D`
and an application-only `flash:w` operation; default verification remained enabled.

`tools/capture_encoder.py --port /dev/arduino --output /data --duration 1800`
runs inside the readback image (which includes pyserial), with only the Uno device,
the capture script and a unique writable data directory mounted. It sends no
serial commands, opens the port exclusively at 115200, saves the raw stream to
`serial.csv` and writes `latest.json` atomically. Opening the connection may reset
the Uno. Stop the capture container gracefully before opening another serial tool.
Create a fresh data directory for each connection; raw-file overwrite is refused.
Check sample timestamps and `running`/`error` before treating status as live.

## Recording

Retain serial CSV in ignored `data/local/` or `backups/` until reviewed. Record
firmware SHA, decoder sign, reference method, pull-ups, supply voltage, cable
arrangement, count change and invalid-transition change for each test. No completed
physical encoder test is claimed by this procedure.
