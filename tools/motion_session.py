"""Single serial connection for supervised motor_jog firmware (pyserial in Docker).

Uses a unique run directory. Only new atomic control.json requests are executed;
an existing request is skipped at startup. No ARM, ZERO or movement is automatic.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import re
import signal
import time

import serial


ALLOWED = re.compile(r"(?:STATUS|CONFIG (?:1|2|4|8|16|32)|ARM|ZERO|PROBE|TICK|JOG [+-]?(?:\d+(?:\.\d*)?|\.\d+)|STOP|CLEAR|!)\Z")


def utc_now():
    return datetime.now(timezone.utc).isoformat()


def read_request(path):
    try:
        value = json.loads(path.read_text())
        return value if isinstance(value, dict) else {}
    except (FileNotFoundError, ValueError):
        return {}


def atomic_json(path, value):
    temp = path.with_suffix(path.suffix + ".tmp")
    temp.write_text(json.dumps(value, indent=2, allow_nan=False) + "\n")
    temp.replace(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--duration", type=float, default=1800)
    args = parser.parse_args()
    if not 0 < args.duration <= 86400:
        parser.error("duration must be positive and at most 86400 seconds")
    root = args.output
    root.mkdir(parents=True, exist_ok=True)
    control = root / "control.json"
    seen_id = read_request(control).get("id")
    state = {"started_utc": utc_now(), "running": True, "status": None,
             "received_utc": None, "last_reply": None, "samples": 0, "error": None,
             "heartbeat_enabled": True}
    port = None
    last_status = last_ping = 0.0

    def stop(signum, frame):
        raise SystemExit(0)

    signal.signal(signal.SIGTERM, stop)
    try:
        with (root / "serial.log").open("x") as log:
            port = serial.Serial(args.port, 115200, timeout=0.02, exclusive=True)
            deadline = time.monotonic() + args.duration
            while time.monotonic() < deadline:
                now = time.monotonic()
                if state["heartbeat_enabled"] and now - last_status < 0.9 and now - last_ping >= 0.25:
                    port.write(b"PING\n")
                    last_ping = now
                request = read_request(control)
                if request.get("id") and request["id"] != seen_id:
                    seen_id = request["id"]
                    command = request.get("command", "")
                    accepted, reason = False, None
                    if command in ("HEARTBEAT_OFF", "HEARTBEAT_ON"):
                        state["heartbeat_enabled"] = command == "HEARTBEAT_ON"
                        accepted = True
                    elif not isinstance(command, str) or not ALLOWED.fullmatch(command):
                        reason = "command_not_allowed"
                    elif command not in ("!", "STOP", "STATUS") and now - last_status > 0.9:
                        reason = "stale_status"
                    else:
                        port.write((command + "\n").encode("ascii"))
                        port.flush()
                        accepted = True
                    atomic_json(root / "control_result.json", {
                        "id": seen_id, "command": command, "accepted": accepted,
                        "reason": reason, "timestamp_utc": utc_now()})
                raw = port.readline()
                if raw:
                    line = raw.decode("ascii", errors="replace").strip()
                    log.write(utc_now() + " " + line + "\n")
                    log.flush()
                    try:
                        packet = json.loads(line)
                    except ValueError:
                        continue
                    if isinstance(packet, dict):
                        if "ms" in packet and "armed" in packet:
                            state["status"] = packet
                            state["samples"] += 1
                            state["received_utc"] = utc_now()
                            last_status = time.monotonic()
                        else:
                            state["last_reply"] = packet
                state["status_age_s"] = round(time.monotonic() - last_status, 3) if last_status else None
                atomic_json(root / "latest.json", state)
    except Exception as exc:
        state["error"] = str(exc)
        raise
    finally:
        if port is not None:
            try:
                port.write(b"!\nSTOP\n")
                port.flush()
            except serial.SerialException:
                pass
            port.close()
        state["running"] = False
        state["ended_utc"] = utc_now()
        atomic_json(root / "latest.json", state)


if __name__ == "__main__":
    main()
