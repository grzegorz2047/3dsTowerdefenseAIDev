#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "scripts" / "export_scene_art.py"
SPEC = importlib.util.spec_from_file_location("export_scene_art", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
EXPORTER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(EXPORTER)


class SceneExportTests(unittest.TestCase):
    def write_scene(self, directory: Path, objects: list[dict[str, object]]) -> Path:
        source = directory / "scene.json"
        source.write_text(
            json.dumps(
                {
                    "schema": 1,
                    "level": "test_scene",
                    "theme": "Default",
                    "coordinate_system": "blender_z_up_minus_y_forward",
                    "objects": objects,
                }
            ),
            encoding="utf-8",
        )
        return source

    def object(self, **overrides: object) -> dict[str, object]:
        result: dict[str, object] = {
            "name": "rock_a",
            "type": "Rock",
            "group": "test group",
            "order": 10,
            "location": [2.5, -4.0, 0.0],
            "scale": [1.25, 1.25, 1.25],
            "rotation_euler_deg": [0.0, 0.0, -30.0],
        }
        result.update(overrides)
        return result

    def test_converts_blender_axes_and_yaw(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            source = self.write_scene(Path(temporary), [self.object()])
            generated = EXPORTER.render_scene(source)

        self.assertIn("level=test_scene\n", generated)
        self.assertIn("theme=Default\n", generated)
        self.assertIn("prop=Rock,2.5,4.0,1.25,30\n", generated)

    def test_orders_objects_deterministically(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            source = self.write_scene(
                Path(temporary),
                [
                    self.object(name="later", order=20),
                    self.object(name="earlier", order=10, type="Pine"),
                ],
            )
            generated = EXPORTER.render_scene(source)

        self.assertLess(generated.index("prop=Pine"), generated.index("prop=Rock"))

    def test_rejects_non_uniform_scale(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            source = self.write_scene(
                Path(temporary),
                [self.object(scale=[1.0, 1.1, 1.0])],
            )
            with self.assertRaisesRegex(EXPORTER.SceneExportError, "scale must be uniform"):
                EXPORTER.render_scene(source)

    def test_rejects_pitch_or_roll(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            source = self.write_scene(
                Path(temporary),
                [self.object(rotation_euler_deg=[10.0, 0.0, 0.0])],
            )
            with self.assertRaisesRegex(EXPORTER.SceneExportError, "only Blender Z yaw"):
                EXPORTER.render_scene(source)

    def test_rejects_duplicate_names_and_orders(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            duplicate_names = self.write_scene(
                directory,
                [self.object(), self.object(order=20)],
            )
            with self.assertRaisesRegex(EXPORTER.SceneExportError, "names must be unique"):
                EXPORTER.render_scene(duplicate_names)

            duplicate_orders = self.write_scene(
                directory,
                [self.object(), self.object(name="rock_b")],
            )
            with self.assertRaisesRegex(EXPORTER.SceneExportError, "order values must be unique"):
                EXPORTER.render_scene(duplicate_orders)

    def test_check_detects_stale_runtime_file(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            source = self.write_scene(directory, [self.object()])
            output = directory / "scene.art"
            output.write_text("stale\n", encoding="utf-8")
            with self.assertRaisesRegex(EXPORTER.SceneExportError, "generated scene is stale"):
                EXPORTER.write_or_check(source, output, check=True)


if __name__ == "__main__":
    unittest.main()
