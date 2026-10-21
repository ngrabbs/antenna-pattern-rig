"""Offline nominal resolution/alignment calculator; no hardware I/O."""
import argparse
import json
import math


def positive(value):
    number = float(value)
    if not math.isfinite(number) or number <= 0:
        raise argparse.ArgumentTypeError("must be finite and greater than zero")
    return number


def calculate(*, microsteps=16, motor_ratio=1.0, encoder_ratio=1.0,
              encoder_ppr=600.0, motor_steps=200.0, quadrature=4,
              distance_m=3.0, lateral_mm=10.0):
    positive_values = (microsteps, motor_ratio, encoder_ratio, encoder_ppr,
                       motor_steps, distance_m)
    if any(not math.isfinite(x) or x <= 0 for x in positive_values):
        raise ValueError("counts, ratios and distance must be finite and positive")
    if quadrature not in (1, 2, 4):
        raise ValueError("quadrature must be 1, 2 or 4")
    if not math.isfinite(lateral_mm) or lateral_mm < 0:
        raise ValueError("lateral_mm must be finite and nonnegative")
    return {
        "command_increment_deg": 360 / (motor_steps * microsteps * motor_ratio),
        "encoder_counts_per_table_rev": encoder_ppr * quadrature * encoder_ratio,
        "encoder_increment_deg": 360 / (encoder_ppr * quadrature * encoder_ratio),
        "alignment_angle_deg": math.degrees(math.atan2(lateral_mm / 1000, distance_m)),
        "notice": "Nominal resolution only; no assembled accuracy has been measured.",
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--microsteps", type=int, choices=(1, 2, 4, 8, 16, 32), default=16)
    parser.add_argument("--motor-ratio", type=positive, default=1.0,
                        help="motor revolutions per table revolution")
    parser.add_argument("--encoder-ratio", type=positive, default=1.0,
                        help="encoder revolutions per table revolution")
    parser.add_argument("--encoder-ppr", type=positive, default=600.0,
                        help="cycles per channel per encoder revolution")
    parser.add_argument("--motor-steps", type=positive, default=200.0)
    parser.add_argument("--quadrature", type=int, choices=(1, 2, 4), default=4)
    parser.add_argument("--distance-m", type=positive, default=3.0)
    parser.add_argument("--lateral-mm", type=float, default=10.0)
    args = parser.parse_args()
    try:
        result = calculate(**vars(args))
    except ValueError as exc:
        parser.error(str(exc))
    print(json.dumps(result, indent=2, allow_nan=False))


if __name__ == "__main__":
    main()
