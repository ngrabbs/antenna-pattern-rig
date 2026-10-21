"""Summarize v5/v6 FEEDBACK180 serial captures, including incomplete/faulted cycles.

Angles use nominal 0.15 degrees per encoder count and per-cycle ZERO. Endpoint
errors are encoder-referenced, not independently calibrated antenna angles.
Elapsed/settling times come from 4 Hz telemetry and are approximate. Post-disable
readings are kept separate from the energized endpoint measurements.
Grouping assumes no aborted attempt is cleared and retried with the same done
counter within one boot; use separate captures for that case.
"""
import argparse
import json
from pathlib import Path


def elapsed(start, end):
    return ((end - start) & 0xffffffff) / 1000


def summarize(lines):
    boot = 0
    groups = {}
    for line in lines:
        try:
            timestamp, payload = line.split(" ", 1)
            p = json.loads(payload)
        except (ValueError, TypeError):
            continue
        if not isinstance(p, dict):
            continue
        if "boot" in p:
            boot += 1
        if p.get("diag") != 4 or p.get("leg") not in (1, 2, 3):
            continue
        key = (boot, p["done"] + (p["leg"] != 3))
        groups.setdefault(key, []).append((timestamp, p))

    cycles = []
    for (boot, cycle), rows in groups.items():
        done = next((r for r in rows if r[1]["leg"] == 3 and not r[1]["armed"]
                     and r[1]["fault"] == 0), None)
        outward = [r for r in rows if r[1]["leg"] == 1]
        returning = [r for r in rows if r[1]["leg"] == 2]
        fault = next((r for r in rows if r[1]["fault"]), None)
        result = {"boot": boot, "cycle": cycle, "completed": done is not None,
                  "fault": fault[1]["fault"] if fault else 0}
        if done:
            timestamp, p = done
            sign = p["enc_sign"]
            out = p["out_enc"] * sign
            home = p["home_enc"] * sign
            result.update({
                "completed_utc": timestamp, "encoder_sign": sign,
                "outward_raw_counts": p["out_enc"], "home_raw_counts": p["home_enc"],
                "outward_deg": round(out * 0.15, 4),
                "outward_target_error_deg": round((out + 1200) * 0.15, 4),
                "home_residual_deg": round(home * 0.15, 4),
                "tolerance_counts": p.get("enc_tol", 6),
                "microsteps": p.get("microsteps"),
                "within_initial_tolerance": sign in (-1, 1) and abs(out + 1200) <= p.get("enc_tol", 6) and abs(home) <= p.get("enc_tol", 6),
                "outward_motor_step_position": p["out_steps"],
                "home_motor_step_position": p["step"],
                "return_pulses": p["leg_steps"],
                "return_corrections": p["corrections"],
                "outward_corrections_observed": max((r[1]["corrections"] for r in outward), default=None),
                "invalid_transition_counter": p["bad"],
            })
            if outward and returning:
                result["outward_elapsed_s"] = elapsed(outward[0][1]["ms"], returning[0][1]["ms"])
                result["return_elapsed_s"] = elapsed(returning[0][1]["ms"], p["ms"])
                result["cycle_elapsed_s"] = elapsed(outward[0][1]["ms"], p["ms"])
            for name, leg, endpoint in (("outward", outward, returning[0] if returning else None),
                                        ("return", returning, done)):
                if leg and endpoint and leg[-1][1]["state"] == 3:
                    tail = []
                    for row in reversed(leg):
                        if row[1]["state"] != 3:
                            break
                        tail.append(row)
                    result[name + "_final_settling_telemetry_s"] = elapsed(tail[-1][1]["ms"], endpoint[1]["ms"])
                    counts = [row[1]["enc"] for row in tail]
                    result[name + "_final_settling_encoder_range_deg"] = round((max(counts) - min(counts)) * 0.15, 4)
            post = next((r for r in rows if r[1]["leg"] == 3
                         and elapsed(p["ms"], r[1]["ms"]) >= 2), None)
            if post:
                result["home_two_seconds_after_disable_deg"] = round(post[1]["enc"] * sign * 0.15, 4)
        cycles.append(result)
    return {"cycles": cycles, "limitations": __doc__.strip()}


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
