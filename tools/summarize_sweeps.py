"""Summarize completed v4 SWEEP180 cycles from timestamped motion_session logs.

Uses nominal 0.15 degrees per encoder count. Commanded motor travel is not an
independent motor-angle measurement, so reported shortfall is not pure slip.
Only the first completed endpoint for each boot/done counter is summarized;
aborted cycles are not represented as completed measurements.
"""
import argparse
import json
from pathlib import Path


def summarize(lines):
    boot = 0
    found = {}
    for line in lines:
        try:
            timestamp, payload = line.split(" ", 1)
            packet = json.loads(payload)
        except (ValueError, TypeError):
            continue
        if not isinstance(packet, dict):
            continue
        if "boot" in packet:
            boot += 1
        if not (packet.get("diag") == 3 and packet.get("leg") == 3
                and packet.get("armed") == 0 and packet.get("fault") == 0
                and packet.get("step") == 0 and packet.get("microsteps") == 1):
            continue
        key = (boot, packet["done"])
        if key in found:
            continue
        outward = packet["out_enc"]
        home = packet["home_enc"]
        returning = home - outward
        found[key] = {
            "boot": boot, "done": packet["done"], "completed_utc": timestamp,
            "outward_counts": outward, "return_counts": returning,
            "home_residual_counts": home,
            "outward_encoder_deg": round(outward * 0.15, 4),
            "return_encoder_deg": round(returning * 0.15, 4),
            "home_residual_deg": round(home * 0.15, 4),
            "outward_shortfall_deg": round(180 - abs(outward) * 0.15, 4),
            "return_shortfall_deg": round(180 - abs(returning) * 0.15, 4),
            "outward_transfer_ratio": abs(outward) / 1200,
            "return_transfer_ratio": abs(returning) / 1200,
            "invalid_transition_counter": packet.get("bad"),
        }
    cycles = list(found.values())
    ranges = {}
    for name in ("outward_encoder_deg", "return_encoder_deg", "home_residual_deg"):
        values = [r[name] for r in cycles]
        ranges[name] = round(max(values) - min(values), 4) if len(values) > 1 else None
    return {
        "cycles": cycles,
        "ranges_deg": ranges,
        "limitations": "Nominal encoder angles versus commanded motor angles; "
        "not pure slip or certified pointing accuracy. Per-cycle zeros are separate. "
        "Endpoint residual is captured before disable and excludes later relaxation. "
        "Repeated samples are deduplicated; aborted/faulted cycles are not counted "
        "as completed. A transfer ratio is descriptive, not a qualified correction.",
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    with args.log.open() as source:
        result = summarize(source)
    rendered = json.dumps(result, indent=2, allow_nan=False) + "\n"
    if args.output:
        args.output.write_text(rendered)
    print(rendered, end="")
