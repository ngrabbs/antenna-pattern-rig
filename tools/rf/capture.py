"""Quick moving-sweep exploration: retain RF scan and bracketed encoder telemetry.

Angle is relative to user-selected zero, not established antenna boresight.
This process never commands motor movement or RF transmission.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import time
import serial

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--port', required=True)
p.add_argument('--motion', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
p.add_argument('--seconds', type=float, default=10)
a = p.parse_args()
if not 0 < a.seconds <= 240:
    p.error('Duration must be 0..240 s')
def state():
    s = json.loads(a.motion.read_text())
    age = (datetime.now(timezone.utc) - datetime.fromisoformat(s['received_utc'])).total_seconds()
    if not s['running'] or s['error'] or age > 1.2:
        raise RuntimeError('Stale motion telemetry')
    return s
with serial.Serial(a.port, 115200, timeout=.1, exclusive=True) as port, a.output.open('x') as log:
    port.reset_input_buffer()
    def command(cmd):
        start = time.monotonic()
        port.write((cmd + '\r').encode())
        port.flush()
        data = b''
        while time.monotonic() - start < 5:
            data += port.read(4096)
            if data.endswith(b'ch> '):
                return data.decode(errors='replace')
        raise TimeoutError('tinySA command timeout: ' + cmd)
    for cmd in ['mode low input', 'ultra on', 'rbw 30', 'attenuate 10', 'lna off', 'calc off']:
        response = command(cmd)
        if 'usage:' in response or 'error' in response.lower():
            raise RuntimeError(response)
    deadline = time.monotonic() + a.seconds
    while time.monotonic() < deadline:
        before = state()
        t0 = time.time()
        raw = command('scan 2399500000 2400500000 41 3')
        t1 = time.time()
        after = state()
        points = []
        for line in raw.splitlines():
            cols = line.split()
            if len(cols) >= 2:
                try:
                    freq, dbm = float(cols[0]), float(cols[1])
                    if 2399500000 <= freq <= 2400500000:
                        points.append([freq, dbm])
                except ValueError:
                    pass
        if len(points) != 41:
            raise RuntimeError('Unexpected RF point count')
        row = dict(start_unix=t0, end_unix=t1, before=before, after=after,
                   points=points, raw=raw)
        log.write(json.dumps(row) + '\n')
        log.flush()
        peak = max(points, key=lambda v: v[1])
        print(json.dumps(dict(time=t1, scan_s=round(t1-t0, 3),
                              peak_hz=peak[0], peak_dbm=peak[1],
                              enc=after['status']['enc'], leg=after['status']['leg'],
                              fault=after['status']['fault'])), flush=True)
