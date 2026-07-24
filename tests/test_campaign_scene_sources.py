#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXPORTER_PATH = ROOT / "scripts" / "export_scene_art.py"
SPEC = importlib.util.spec_from_file_location("export_scene_art", EXPORTER_PATH)
assert SPEC is not None and SPEC.loader is not None
EXPORTER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(EXPORTER)

CAMPAIGN_SCENES = (
    "tutorial",
    "ash_gate",
    "ruined_village",
    "stone_bridge",
    "echo_valley",
    "last_citadel",
    "portal_nexus",
)


class CampaignSceneSourceTests(unittest.TestCase):
    def test_campaign_scenes_are_source_backed_and_distinct(self) -> None:
        compositions: set[str] = set()
        for scene_id in CAMPAIGN_SCENES:
            source = ROOT / "assets" / "scenes" / f"{scene_id}.scene.json"
            runtime = ROOT / "romfs" / "scenes" / f"{scene_id}.art"
            self.assertTrue(source.is_file(), scene_id)
            self.assertTrue(runtime.is_file(), scene_id)

            payload = json.loads(source.read_text(encoding="utf-8"))
            objects = payload["objects"]
            self.assertEqual(payload["level"], scene_id)
            self.assertEqual(payload["theme"], "VhalPass")
            self.assertGreaterEqual(len(objects), 18)
            self.assertLessEqual(len(objects), 48)
            self.assertGreaterEqual(len({item["type"] for item in objects}), 5)
            self.assertGreaterEqual(len({item["group"] for item in objects}), 3)

            generated = EXPORTER.render_scene(source)
            self.assertEqual(runtime.read_text(encoding="utf-8"), generated)
            signature = "|".join(
                f"{item['type']}:{item['location']}:{item['scale']}:{item['rotation_euler_deg']}"
                for item in sorted(objects, key=lambda item: item["order"])
            )
            self.assertNotIn(signature, compositions)
            compositions.add(signature)


if __name__ == "__main__":
    unittest.main()
