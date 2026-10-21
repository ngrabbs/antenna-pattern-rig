"""Capture the encoder diagnostic's serial CSV; requires pyserial (Docker image).

This tool sends no serial commands. Opening the port may reset the Uno.
Use a separate output directory for every connection/reset session.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import signal
import time

import serial


def utc_now():
    return datetime.now(timezone.utc).isoformat()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--duration", type=float, default=1800)
    args = parser.parse_args()
    if not 0 < args.duration <= 86400:
        parser.error("duration must be positive and no more than 86400 seconds")
    args.output.mkdir(parents=True, exist_ok=True)
    state = {"started_utc": utc_now(), "running": True, "samples": 0,
             "last_sample": None, "error": None}

    def save():
        temporary = args.output / "latest.json.tmp"
        temporary.write_text(json.dumps(state, indent=2) + "\n")
        temporary.replace(args.output / "latest.json")

    def stop(signum, frame):
        raise SystemExit(0)

    signal.signal(signal.SIGTERM, stop)
    save()
    try:
        # Exclusive mode avoids another process silently sharing the serial stream.
        with (args.output / "serial.csv").open("x") as log:
            with serial.Serial(args.port, 115200, timeout=0.25, exclusive=True) as port:
                deadline = time.monotonic() + args.duration
                while time.monotonic() < deadline:
                    raw = port.readline()
                    if not raw:
                        continue
                    line = raw.decode("ascii", errors="replace").strip()
                    log.write(line + "\n")
                    log.flush()
                    fields = line.split(",")
                    if len(fields) != 5:
                        continue
                    try:
                        sample = dict(zip(
                            ("millis", "reference_id", "counts", "relative_table_deg",
                             "invalid_transitions"),
                            (int(fields[0]), int(fields[1]), int(fields[2]),
                             float(fields[3]), int(fields[4]))))
                    except ValueError:
                        continue
                    sample["received_utc"] = utc_now()
                    state["last_sample"] = sample
                    state["samples"] += 1
                    save()
    except Exception as exc:
        state["error"] = str(exc)
        raise
    finally:
        state["running"] = False
        state["ended_utc"] = utc_now()
        save()


if __name__ == "__main__":
    main()
