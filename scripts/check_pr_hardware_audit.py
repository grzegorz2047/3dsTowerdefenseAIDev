#!/usr/bin/env python3
"""Enforce the hardware-audit section for PRs touching sensitive paths."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SENSITIVE_PREFIXES = (
    "source/Renderer.cpp",
    "include/Renderer.hpp",
    "source/UiRenderer.cpp",
    "include/UiRenderer.hpp",
    "include/UiState.hpp",
    "source/HardwareTelemetryRuntime.cpp",
    "include/HardwareTelemetry",
    "assets/",
    "data/",
    "romfs/",
    "shaders/",
    ".github/workflows/build.yml",
    "Makefile",
    "scripts/validate_hardware_evidence.py",
    "hardware-audit/",
)
SENSITIVE_SUFFIXES = (".v.pica", ".pica", ".png", ".t3x")
CHECKLIST_PHRASES = (
    "podstawowa rozgrywka nie zależy od funkcji New 3DS",
    "zachowane są budżety wierzchołków, obiektów i pamięci liniowej",
    "brak nowych alokacji liniowych lub transferów tekstur wykonywanych stale co klatkę",
    "oba ekrany zostały sprawdzone",
    "każdy widoczny przycisk dolnego ekranu ma zgodny obszar dotykowy",
    "mono i stereo używają fizycznego suwaka 3D zgodnie z kontraktem",
    "dołączono lub wskazano pakiet PNG wymagany przez audyt",
    "zapisano wynik `PASS/WARN/FAIL` dla Old 3DS albo jawnie wskazano, że fizyczny retest pozostaje wymagany",
    "wynik New 3DS, jeśli dostępny, jest raportowany osobno",
)
EVIDENCE_LABELS = (
    "Commit artefaktów",
    "SHA-256 CIA",
    "SHA-256 3DSX",
    "Link do artefaktów/logów",
    "Link do PNG",
    "Fizyczny model konsoli i wynik",
)


def is_sensitive(path: str) -> bool:
    return path.startswith(SENSITIVE_PREFIXES) or path.endswith(SENSITIVE_SUFFIXES)


def _field_value(body: str, label: str) -> str:
    match = re.search(rf"(?mi)^-\s*{re.escape(label)}:\s*(.+?)\s*$", body)
    return match.group(1).strip() if match else ""


def validate_pr_body(body: str, changed_files: list[str]) -> list[str]:
    if not any(is_sensitive(path) for path in changed_files):
        return []
    errors: list[str] = []
    for heading in (
        "## Audyt sprzętowy Nintendo 3DS",
        "### Wpływ",
        "### Obowiązkowa checklista dla zmian wizualnych lub sprzętowych",
        "## Dowody",
    ):
        if heading not in body:
            errors.append(f"Missing PR section: {heading}")

    for profile in ("Old", "New"):
        value = _field_value(body, f"Profil {profile} 3DS")
        if value not in {"BRAK", "NISKI", "ŚREDNI", "WYSOKI"}:
            errors.append(f"Select one impact value for Profil {profile} 3DS")
    changed_areas = _field_value(body, "Zmienione obszary")
    placeholder = "CPU / PICA200 / pamięć liniowa / geometria / tekstury / górny ekran / dolny ekran / dotyk / stereo / release / N/D"
    if not changed_areas or changed_areas == placeholder:
        errors.append("Replace the Zmienione obszary placeholder with the actual scope")

    for phrase in CHECKLIST_PHRASES:
        pattern = rf"(?mi)^-\s*\[[xX]\]\s*{re.escape(phrase)}\s*$"
        if not re.search(pattern, body):
            errors.append(f"Unchecked or missing hardware checklist item: {phrase}")

    for label in EVIDENCE_LABELS:
        value = _field_value(body, label)
        if not value:
            errors.append(f"Fill evidence field: {label}")
        elif value in {"TODO", "TBD", "-"}:
            errors.append(f"Evidence field {label} still contains a placeholder")

    old_result = _field_value(body, "Fizyczny model konsoli i wynik")
    if old_result and not re.search(r"(?i)(PASS|WARN|FAIL|RETEST WYMAGANY)", old_result):
        errors.append(
            "Fizyczny model konsoli i wynik must contain PASS/WARN/FAIL or RETEST WYMAGANY"
        )
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--body-file", required=True, type=Path)
    parser.add_argument("--changed-files-file", required=True, type=Path)
    args = parser.parse_args(argv)
    body = args.body_file.read_text(encoding="utf-8")
    changed_files = [
        line.strip()
        for line in args.changed_files_file.read_text(encoding="utf-8").splitlines()
        if line.strip()
    ]
    sensitive = [path for path in changed_files if is_sensitive(path)]
    errors = validate_pr_body(body, changed_files)
    if errors:
        print("hardware PR audit: FAIL", file=sys.stderr)
        print("Sensitive files:", file=sys.stderr)
        for path in sensitive:
            print(f"- {path}", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1
    if sensitive:
        print("hardware PR audit: PASS")
    else:
        print("hardware PR audit: N/A (no sensitive files)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
