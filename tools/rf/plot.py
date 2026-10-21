"""Plot an exploratory moving scan; never infer absolute gain or boresight."""
import argparse
from datetime import datetime
import json
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--capture', type=Path, required=True)
p.add_argument('--baseline', type=Path, required=True)
p.add_argument('--motion', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
def power(row):
    # Exclude the observed LO leakage 100 kHz below the intended tone.
    values = [v for f, v in row['points'] if abs(f - 2400000000) <= 25000]
    if len(values) != 3:
        raise ValueError('Expected three bins around the intended tone')
    return max(values)
motion = []
for line in a.motion.read_text().splitlines():
    try:
        ts, payload = line.split(' ', 1)
        s = json.loads(payload)
        if 'enc' in s:
            motion.append((datetime.fromisoformat(ts).timestamp(), s['enc']))
    except (ValueError, KeyError):
        continue
if len(motion) < 2:
    raise ValueError('Missing motion timestamps')
mt, me = np.array(motion).T
baseline = [power(json.loads(line)) for line in a.baseline.read_text().splitlines()]
noise = float(np.percentile(baseline, 95))
groups = {1: [], 2: []}
scan_times = []
for line in a.capture.read_text().splitlines():
    r = json.loads(line)
    b, c = r['before']['status'], r['after']['status']
    if any(s['fault'] or not s['armed'] or not s['ref'] or s['diag'] != 4 for s in (b, c)):
        continue
    if b['leg'] != c['leg'] or b['leg'] not in groups:
        continue
    if not mt[0] <= r['start_unix'] <= r['end_unix'] <= mt[-1]:
        continue
    mid = (r['start_unix'] + r['end_unix']) / 2
    # Receive timestamps approximate encoder sample times; this is not hardware sync.
    angle = -float(np.interp(mid, mt, me)) * .15
    span = abs(float(np.interp(r['end_unix'], mt, me) - np.interp(r['start_unix'], mt, me))) * .15
    groups[b['leg']].append((angle, power(r), span))
    scan_times.append(r['end_unix'] - r['start_unix'])
if any(len(g) < 10 for g in groups.values()):
    raise ValueError('Insufficient two-direction scan data')
binned = {}
for leg, rows in groups.items():
    bins = {}
    for angle, dbm, span in rows:
        bins.setdefault(int(round(angle / 2)), []).append(dbm)
    binned[leg] = {k: float(np.median(v)) for k, v in bins.items()}
peak = max(v for bins in binned.values() for v in bins.values())
common = sorted(set(binned[1]) & set(binned[2]))
usable = [k for k in common if min(binned[1][k], binned[2][k]) > noise + 6]
diff = [abs(binned[1][k] - binned[2][k]) for k in usable]
summary = dict(frequency_hz=2400000000, angle_reference='relative start; not antenna boresight',
               estimator='maximum of 2399975000/2400000000/2400025000 Hz bins; 30 kHz RBW',
               baseline_p95_dbm=noise, normalization_peak_dbm=peak,
               scan_duration_median_s=float(np.median(scan_times)),
               observed_angle_span_max_deg=max(r[2] for rows in groups.values() for r in rows),
               samples={str(k): len(v) for k, v in groups.items()},
               angular_coverage_deg={str(k): [min(r[0] for r in v), max(r[0] for r in v)] for k, v in groups.items()},
               common_2deg_bins=len(common), bins_both_above_background_plus_6db=len(usable),
               direction_difference_median_db=float(np.median(diff)) if diff else None)
fig, axes = plt.subplots(2, 1, figsize=(10, 8), sharex=True, constrained_layout=True)
for leg, label, color in ((1, 'Outward', '#1769aa'), (2, 'Return', '#dc6b22')):
    rows = np.array(groups[leg])
    bins = sorted(binned[leg])
    angles = [k * 2 for k in bins]
    levels = [binned[leg][k] for k in bins]
    axes[0].scatter(rows[:, 0], rows[:, 1], s=5, alpha=.18, color=color)
    axes[0].plot(angles, levels, label=label + ' — 2° bin median', color=color)
    axes[1].plot(angles, np.array(levels) - peak, label=label, color=color)
axes[0].axhline(noise, color='gray', linestyle=':', label='TX-off background: 95th percentile')
axes[1].axhline(noise - peak, color='gray', linestyle=':')
axes[0].set_ylabel('Tone-bin level (tinySA dBm)')
axes[1].set_ylabel('Relative level (dB)')
axes[1].set_xlabel('Encoder angle from starting position (degrees)')
axes[1].set_xlim(0, 180)
for ax in axes:
    ax.grid(alpha=.25)
    ax.legend(fontsize=9)
fig.suptitle('First exploratory patch scan · 2400 MHz\nMoving acquisition; relative zero; uncalibrated range', fontsize=14)
a.output.mkdir(parents=True, exist_ok=True)
fig.savefig(a.output / 'quick-pattern.png', dpi=170)
fig.savefig(a.output / 'quick-pattern.svg')
(a.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
print(json.dumps(summary, indent=2))
