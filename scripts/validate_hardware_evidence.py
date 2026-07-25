#!/usr/bin/env python3
"""Validate release artifacts, PNG evidence and physical Old/New 3DS results."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any

SHA_RE = re.compile(r"^[0-9a-f]{64}$")
COMMIT_RE = re.compile(r"^[0-9a-f]{40}$")
PNG_KINDS = {
    "campaign", "briefing", "benchmark_config", "loaded_mission",
    "pause", "outcome", "benchmark_result", "stereo_comparison",
}
VERDICTS = {"PASS", "WARN", "FAIL"}
REVIEWS = {"PASS", "WARN", "N/A"}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def obj(value: Any, label: str, errors: list[str]) -> dict[str, Any]:
    if not isinstance(value, dict):
        errors.append(f"{label} must be an object")
        return {}
    return value


def text(value: Any, label: str, errors: list[str]) -> str:
    if not isinstance(value, str) or not value.strip():
        errors.append(f"{label} must be a non-empty string")
        return ""
    return value.strip()


def number(value: Any, label: str, errors: list[str]) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        errors.append(f"{label} must be a number")
        return 0.0
    result = float(value)
    if result < 0:
        errors.append(f"{label} must be non-negative")
    return result


def validate_artifacts(data: dict[str, Any], artifact_dir: Path, errors: list[str]) -> None:
    entries = obj(data.get("artifacts"), "artifacts", errors)
    sums_path = artifact_dir / "SHA256SUMS.txt"
    sums: dict[str, str] = {}
    if not sums_path.is_file():
        errors.append(f"Missing checksum file: {sums_path}")
    else:
        for raw in sums_path.read_text(encoding="utf-8").splitlines():
            parts = raw.split(maxsplit=1)
            if len(parts) == 2:
                sums[parts[1].lstrip("*")] = parts[0]
    for key, filename in (("cia", "CitadelDefense3D.cia"), ("3dsx", "CitadelDefense3D.3dsx")):
        entry = obj(entries.get(key), f"artifacts.{key}", errors)
        declared_name = text(entry.get("filename"), f"artifacts.{key}.filename", errors)
        declared_sha = text(entry.get("sha256"), f"artifacts.{key}.sha256", errors)
        if declared_name and declared_name != filename:
            errors.append(f"artifacts.{key}.filename must be {filename}")
        if declared_sha and not SHA_RE.fullmatch(declared_sha):
            errors.append(f"artifacts.{key}.sha256 must be lowercase SHA-256")
            continue
        path = artifact_dir / filename
        if not path.is_file() or path.stat().st_size == 0:
            errors.append(f"Missing or empty artifact: {path}")
            continue
        actual = sha256(path)
        if declared_sha and declared_sha != actual:
            errors.append(f"Manifest SHA mismatch for {filename}")
        if sums.get(filename) != actual:
            errors.append(f"SHA256SUMS.txt mismatch for {filename}")


def validate_png(data: dict[str, Any], root: Path, errors: list[str]) -> None:
    entries = data.get("png_evidence")
    if not isinstance(entries, list):
        errors.append("png_evidence must be an array")
        return
    seen: set[str] = set()
    for index, raw in enumerate(entries):
        entry = obj(raw, f"png_evidence[{index}]", errors)
        kind = text(entry.get("kind"), f"png_evidence[{index}].kind", errors)
        rel = text(entry.get("path"), f"png_evidence[{index}].path", errors)
        if kind in seen:
            errors.append(f"Duplicate PNG kind: {kind}")
        seen.add(kind)
        if rel:
            path = (root / rel).resolve()
            try:
                path.relative_to(root.resolve())
            except ValueError:
                errors.append(f"PNG path escapes repository: {rel}")
            else:
                if path.suffix.lower() != ".png" or not path.is_file() or path.stat().st_size == 0:
                    errors.append(f"Missing, empty or non-PNG evidence: {rel}")
        review = obj(entry.get("review"), f"png_evidence[{index}].review", errors)
        for field in ("readability", "framing", "hud_overlap", "touch_alignment"):
            value = text(review.get(field), f"png_evidence[{index}].review.{field}", errors)
            if value and value not in REVIEWS:
                errors.append(f"Invalid PNG review value: {value}")
        if "FAIL" in review.values():
            errors.append(f"png_evidence[{index}] contains FAIL")
        if kind == "stereo_comparison":
            text(entry.get("emulator_limitation_note"), "stereo emulator limitation note", errors)
    missing = PNG_KINDS - seen
    extra = seen - PNG_KINDS
    if missing:
        errors.append("Missing PNG kinds: " + ", ".join(sorted(missing)))
    if extra:
        errors.append("Unknown PNG kinds: " + ", ".join(sorted(extra)))


def benchmark(case: Any, label: str, render_limit: float, errors: list[str]) -> None:
    data = obj(case, label, errors)
    average = number(data.get("average_ms"), f"{label}.average_ms", errors)
    worst = number(data.get("worst_ms"), f"{label}.worst_ms", errors)
    render = number(data.get("render_ms"), f"{label}.render_ms", errors)
    memory = number(data.get("minimum_free_linear_memory_bytes"), f"{label}.minimum_free_linear_memory_bytes", errors)
    if average > 33.333:
        errors.append(f"{label} average exceeds 33.333 ms")
    if worst > 36.0:
        errors.append(f"{label} worst exceeds 36 ms")
    if render > render_limit:
        errors.append(f"{label} render exceeds {render_limit} ms")
    if memory < 512 * 1024:
        errors.append(f"{label} memory is below 512 KiB")


def device(raw: Any, label: str, required: bool, errors: list[str]) -> str | None:
    if raw is None and not required:
        return None
    data = obj(raw, label, errors)
    for field in ("model", "system", "tester", "tested_at", "benchmark_profile", "notes"):
        text(data.get(field), f"{label}.{field}", errors)
    launch = text(data.get("launch_format"), f"{label}.launch_format", errors)
    speedup = text(data.get("cpu_speedup"), f"{label}.cpu_speedup", errors)
    if launch and launch not in {"CIA", "3DSX", "BOTH"}:
        errors.append(f"{label}.launch_format must be CIA, 3DSX or BOTH")
    if speedup and speedup not in {"ENABLED", "DISABLED", "N/A"}:
        errors.append(f"{label}.cpu_speedup must be ENABLED, DISABLED or N/A")
    benchmark(data.get("mono"), f"{label}.mono", 18.0, errors)
    benchmark(data.get("stereo"), f"{label}.stereo", 28.0, errors)
    for field in ("ten_entry_stability", "touch", "screens", "stereo_comfort"):
        value = text(data.get(field), f"{label}.{field}", errors)
        if value and value not in VERDICTS:
            errors.append(f"{label}.{field} must be PASS, WARN or FAIL")
        if value == "FAIL":
            errors.append(f"{label}.{field} is FAIL")
    verdict = text(data.get("verdict"), f"{label}.verdict", errors)
    if verdict and verdict not in VERDICTS:
        errors.append(f"{label}.verdict must be PASS, WARN or FAIL")
    return verdict or None


def validate_manifest(manifest: Path, version: str, commit: str, artifact_dir: Path, root: Path, release: bool) -> list[str]:
    errors: list[str] = []
    try:
        data = json.loads(manifest.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        return [f"Cannot read manifest: {exc}"]
    if not isinstance(data, dict):
        return ["Manifest root must be an object"]
    if data.get("schema_version") != 1:
        errors.append("schema_version must equal 1")
    declared_version = text(data.get("release"), "release", errors)
    declared_commit = text(data.get("commit"), "commit", errors)
    if declared_version and declared_version != version:
        errors.append(f"Release mismatch: {declared_version} != {version}")
    if declared_commit and not COMMIT_RE.fullmatch(declared_commit):
        errors.append("commit must be a lowercase 40-character Git SHA")
    if declared_commit and declared_commit != commit:
        errors.append(f"Commit mismatch: {declared_commit} != {commit}")
    validate_artifacts(data, artifact_dir, errors)
    validate_png(data, root, errors)
    physical = obj(data.get("physical_tests"), "physical_tests", errors)
    old = device(physical.get("old3ds"), "physical_tests.old3ds", True, errors)
    new = device(physical.get("new3ds"), "physical_tests.new3ds", False, errors)
    decision = text(data.get("decision"), "decision", errors)
    deviations = data.get("deviations")
    if decision and decision not in VERDICTS:
        errors.append("decision must be PASS, WARN or FAIL")
    if not isinstance(deviations, list) or any(not isinstance(item, str) for item in deviations):
        errors.append("deviations must be an array of strings")
        deviations = []
    if release:
        if old == "FAIL": errors.append("Old 3DS verdict FAIL blocks release")
        if old not in {"PASS", "WARN"}: errors.append("Old 3DS PASS or WARN is required")
        if decision == "FAIL": errors.append("Top-level FAIL blocks release")
        if decision == "PASS" and old != "PASS": errors.append("PASS requires Old 3DS PASS")
        if new == "FAIL" and decision != "WARN": errors.append("New 3DS FAIL requires top-level WARN")
        if decision == "WARN" and not deviations: errors.append("WARN requires deviations")
        if decision == "PASS" and deviations: errors.append("PASS cannot contain deviations")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--expected-version", required=True)
    parser.add_argument("--expected-commit", required=True)
    parser.add_argument("--artifact-dir", required=True, type=Path)
    parser.add_argument("--repository-root", default=Path.cwd(), type=Path)
    parser.add_argument("--release", action="store_true")
    args = parser.parse_args()
    errors = validate_manifest(args.manifest, args.expected_version, args.expected_commit, args.artifact_dir, args.repository_root, args.release)
    if errors:
        print("hardware evidence: FAIL", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1
    print("hardware evidence: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
