"""Submit one request to an existing motion_session; does not open serial.

Wait for control_result.json AND firmware status/reply before another request.
"""
import argparse
import json
from pathlib import Path
import uuid

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--output", type=Path, default=Path("/data"))
parser.add_argument("command", nargs="+")
args = parser.parse_args()
request = {"id": str(uuid.uuid4()), "command": " ".join(args.command)}
temporary = args.output / "control.json.tmp"
temporary.write_text(json.dumps(request) + "\n")
temporary.replace(args.output / "control.json")
print(json.dumps(request))
