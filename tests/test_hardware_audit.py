from __future__ import annotations

import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from check_pr_hardware_audit import validate_pr_body
from validate_hardware_evidence import validate_manifest

GOOD_BODY = """## Audyt sprzętowy Nintendo 3DS
### Wpływ
- Profil Old 3DS: NISKI
- Profil New 3DS: NISKI
- Zmienione obszary: release / PNG
- Uzasadnienie `N/D`, jeśli pełny audyt nie jest potrzebny: nie dotyczy
### Obowiązkowa checklista dla zmian wizualnych lub sprzętowych
- [x] podstawowa rozgrywka nie zależy od funkcji New 3DS
- [x] zachowane są budżety wierzchołków, obiektów i pamięci liniowej
- [x] brak nowych alokacji liniowych lub transferów tekstur wykonywanych stale co klatkę
- [x] oba ekrany zostały sprawdzone
- [x] każdy widoczny przycisk dolnego ekranu ma zgodny obszar dotykowy
- [x] mono i stereo używają fizycznego suwaka 3D zgodnie z kontraktem
- [x] dołączono lub wskazano pakiet PNG wymagany przez audyt
- [x] zapisano wynik `PASS/WARN/FAIL` dla Old 3DS albo jawnie wskazano, że fizyczny retest pozostaje wymagany
- [x] wynik New 3DS, jeśli dostępny, jest raportowany osobno
## Dowody
- Commit artefaktów: abc
- SHA-256 CIA: abc
- SHA-256 3DSX: def
- Link do artefaktów/logów: Actions
- Link do PNG: hardware-audit/releases/example
- Fizyczny model konsoli i wynik: RETEST WYMAGANY: #192
"""


class PullRequestAuditTests(unittest.TestCase):
    def test_sensitive_change_requires_completed_body(self) -> None:
        self.assertEqual(validate_pr_body(GOOD_BODY, ["source/Renderer.cpp"]), [])
        self.assertTrue(validate_pr_body("", ["source/Renderer.cpp"]))

    def test_non_sensitive_change_is_not_blocked(self) -> None:
        self.assertEqual(validate_pr_body("", ["source/Enemy.cpp"]), [])


class EvidenceManifestTests(unittest.TestCase):
    def make_package(self, root: Path) -> tuple[Path, Path, dict]:
        artifact_dir = root / "dist"
        evidence_dir = root / "hardware-audit" / "releases" / "v1"
        artifact_dir.mkdir(parents=True)
        evidence_dir.mkdir(parents=True)
        artifacts = {}
        sums = []
        for key, filename in (("cia", "CitadelDefense3D.cia"), ("3dsx", "CitadelDefense3D.3dsx")):
            path = artifact_dir / filename
            path.write_bytes(filename.encode())
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            artifacts[key] = {"filename": filename, "sha256": digest}
            sums.append(f"{digest}  {filename}")
        (artifact_dir / "SHA256SUMS.txt").write_text("\n".join(sums) + "\n")
        png = []
        for kind in ("campaign", "briefing", "benchmark_config", "loaded_mission", "pause", "outcome", "benchmark_result", "stereo_comparison"):
            path = evidence_dir / f"{kind}.png"
            path.write_bytes(b"png")
            entry = {
                "kind": kind,
                "path": str(path.relative_to(root)),
                "review": {"readability": "PASS", "framing": "PASS", "hud_overlap": "PASS", "touch_alignment": "N/A"},
            }
            if kind == "stereo_comparison":
                entry["emulator_limitation_note"] = "Emulator does not reproduce physical depth."
            png.append(entry)
        device = {
            "model": "Old Nintendo 3DS XL", "system": "11.17 + Luma", "tester": "tester",
            "tested_at": "2026-07-25", "launch_format": "BOTH", "benchmark_profile": "stress",
            "cpu_speedup": "N/A",
            "mono": {"average_ms": 30.0, "worst_ms": 35.0, "render_ms": 17.0, "minimum_free_linear_memory_bytes": 600000},
            "stereo": {"average_ms": 31.0, "worst_ms": 35.5, "render_ms": 27.0, "minimum_free_linear_memory_bytes": 590000},
            "ten_entry_stability": "PASS", "touch": "PASS", "screens": "PASS", "stereo_comfort": "PASS",
            "verdict": "PASS", "notes": "No regressions.",
        }
        manifest = {
            "schema_version": 1, "release": "v1", "commit": "a" * 40,
            "artifacts": artifacts, "png_evidence": png,
            "physical_tests": {"old3ds": device, "new3ds": None},
            "deviations": [], "decision": "PASS",
        }
        manifest_path = evidence_dir / "manifest.json"
        manifest_path.write_text(json.dumps(manifest))
        return manifest_path, artifact_dir, manifest

    def test_valid_release_package(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            manifest, artifacts, _ = self.make_package(root)
            self.assertEqual(validate_manifest(manifest, "v1", "a" * 40, artifacts, root, True), [])

    def test_old_3ds_fail_blocks_release(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            manifest_path, artifacts, data = self.make_package(root)
            data["physical_tests"]["old3ds"]["verdict"] = "FAIL"
            data["physical_tests"]["old3ds"]["touch"] = "FAIL"
            data["decision"] = "FAIL"
            data["deviations"] = ["Old 3DS failed"]
            manifest_path.write_text(json.dumps(data))
            errors = validate_manifest(manifest_path, "v1", "a" * 40, artifacts, root, True)
            self.assertIn("Old 3DS verdict FAIL blocks release", errors)


if __name__ == "__main__":
    unittest.main()
