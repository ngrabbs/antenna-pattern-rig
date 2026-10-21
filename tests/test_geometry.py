import importlib.util
import math
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location(
    "geometry", Path(__file__).resolve().parents[1] / "tools" / "geometry.py")
geometry = importlib.util.module_from_spec(spec)
spec.loader.exec_module(geometry)


class GeometryTests(unittest.TestCase):
    def test_direct_drive_resolution_and_alignment(self):
        result = geometry.calculate()
        self.assertAlmostEqual(result["command_increment_deg"], 0.1125)
        self.assertEqual(result["encoder_counts_per_table_rev"], 2400)
        self.assertAlmostEqual(result["encoder_increment_deg"], 0.15)
        self.assertAlmostEqual(result["alignment_angle_deg"], 0.19098522436)

    def test_motor_reduction_does_not_change_output_encoder_resolution(self):
        result = geometry.calculate(motor_ratio=5, encoder_ratio=1)
        self.assertAlmostEqual(result["command_increment_deg"], 0.0225)
        self.assertAlmostEqual(result["encoder_increment_deg"], 0.15)

    def test_motor_mounted_encoder_reduction(self):
        result = geometry.calculate(motor_ratio=5, encoder_ratio=5)
        self.assertAlmostEqual(result["encoder_increment_deg"], 0.03)

    def test_invalid_geometry_is_rejected(self):
        for kwargs in ({"distance_m": 0}, {"encoder_ratio": -1},
                       {"motor_ratio": math.nan}, {"lateral_mm": math.inf},
                       {"lateral_mm": -1}, {"quadrature": 3}):
            with self.subTest(kwargs=kwargs), self.assertRaises(ValueError):
                geometry.calculate(**kwargs)


if __name__ == "__main__":
    unittest.main()
