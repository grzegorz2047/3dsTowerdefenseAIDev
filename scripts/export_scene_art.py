#!/usr/bin/env python3

"""Compile Blender-style scene JSON into the runtime .art format."""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from pathlib import Path
from typing import Any

ALLOWED_THEMES = {"Default", "VhalPass"}
ALLOWED_PROP_TYPES = {
    "Pine",
    "Rock",
    "Ruin",
    "Banner",
    "Barricade",
    "Wagon",
    "Cliff",
    "Watchtower",
}
COORDINATE_SYSTEM = "blender_z_up_minus_y_forward"
MAXIMUM_PROPS = 48
LEVEL_ID_PATTERN = re.compile(r"^[a-z0-9_]+$")
OBJECT_NAME_PATTERN = re.compile(r"^[A-Za-z][A-Za-z0-9_.-]*$")
EPSILON = 1.0e-4


class SceneExportError(ValueError):
    pass


def fail(source: Path, message: str) -> None:
    raise SceneExportError(f"{source}: {message}")


def finite_number(source: Path, value: object, field: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        fail(source, f"{field} must be a number")
    result = float(value)
    if not math.isfinite(result):
        fail(source, f"{field} must be finite")
    return result


def vector3(source: Path, value: object, field: str) -> tuple[float, float, float]:
    if not isinstance(value, list) or len(value) != 3:
        fail(source, f"{field} must contain exactly three numbers")
    return tuple(
        finite_number(source, component, f"{field}[{index}]")
        for index, component in enumerate(value)
    )  # type: ignore[return-value]


def compact_number(value: float, force_decimal: bool) -> str:
    if abs(value) < EPSILON:
        value = 0.0
    text = f"{value:.4f}".rstrip("0").rstrip(".")
    if force_decimal and "." not in text:
        text += ".0"
    return text


def normalize_rotation(value: float) -> float:
    result = (value + 180.0) % 360.0 - 180.0
    if abs(result) < EPSILON:
        return 0.0
    return result


def load_scene(source: Path) -> dict[str, Any]:
    try:
        data = json.loads(source.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        fail(source, f"cannot parse JSON: {error}")

    if not isinstance(data, dict):
        fail(source, "root must be a JSON object")

    expected_keys = {"schema", "level", "theme", "coordinate_system", "objects"}
    missing = sorted(expected_keys - data.keys())
    unknown = sorted(data.keys() - expected_keys)
    if missing:
        fail(source, f"missing fields: {', '.join(missing)}")
    if unknown:
        fail(source, f"unknown fields: {', '.join(unknown)}")
    if data["schema"] != 1:
        fail(source, "unsupported schema; expected 1")
    if not isinstance(data["level"], str) or not LEVEL_ID_PATTERN.fullmatch(data["level"]):
        fail(source, "level must use lowercase letters, digits and underscores")
    if data["theme"] not in ALLOWED_THEMES:
        fail(source, f"unsupported theme: {data['theme']}")
    if data["coordinate_system"] != COORDINATE_SYSTEM:
        fail(source, f"coordinate_system must be {COORDINATE_SYSTEM}")
    if not isinstance(data["objects"], list) or not data["objects"]:
        fail(source, "objects must be a non-empty array")
    if len(data["objects"]) > MAXIMUM_PROPS:
        fail(source, f"scene exceeds the {MAXIMUM_PROPS}-prop runtime budget")
    return data


def compile_object(source: Path, raw: object) -> dict[str, Any]:
    if not isinstance(raw, dict):
        fail(source, "every object must be a JSON object")

    expected_keys = {
        "name",
        "type",
        "group",
        "order",
        "location",
        "scale",
        "rotation_euler_deg",
    }
    missing = sorted(expected_keys - raw.keys())
    unknown = sorted(raw.keys() - expected_keys)
    if missing:
        fail(source, f"object is missing fields: {', '.join(missing)}")
    if unknown:
        fail(source, f"object has unknown fields: {', '.join(unknown)}")

    name = raw["name"]
    if not isinstance(name, str) or not OBJECT_NAME_PATTERN.fullmatch(name):
        fail(source, "object.name must be a stable Blender-safe identifier")

    prop_type = raw["type"]
    if prop_type not in ALLOWED_PROP_TYPES:
        fail(source, f"{name}: unsupported prop type: {prop_type}")

    group = raw["group"]
    if not isinstance(group, str) or not group.strip() or "\n" in group or "\r" in group:
        fail(source, f"{name}: group must be a single non-empty line")
    if group.startswith(";") or len(group) > 80:
        fail(source, f"{name}: group must be plain text up to 80 characters")

    order = raw["order"]
    if isinstance(order, bool) or not isinstance(order, int) or not 0 <= order <= 9999:
        fail(source, f"{name}: order must be an integer from 0 to 9999")

    location = vector3(source, raw["location"], f"{name}.location")
    scale = vector3(source, raw["scale"], f"{name}.scale")
    rotation = vector3(source, raw["rotation_euler_deg"], f"{name}.rotation_euler_deg")

    if abs(location[2]) > EPSILON:
        fail(source, f"{name}: runtime scene props must sit on Blender Z=0")
    if max(scale) - min(scale) > EPSILON:
        fail(source, f"{name}: scale must be uniform on all axes")
    if abs(rotation[0]) > EPSILON or abs(rotation[1]) > EPSILON:
        fail(source, f"{name}: only Blender Z yaw is supported")
    if rotation[2] < -360.0 or rotation[2] > 360.0:
        fail(source, f"{name}: Blender Z rotation must be within -360..360 degrees")

    game_x = location[0]
    game_z = -location[1]
    game_scale = scale[0]
    game_rotation = normalize_rotation(-rotation[2])

    if not -4.0 <= game_x <= 20.0 or not -4.0 <= game_z <= 20.0:
        fail(source, f"{name}: converted game position is outside -4..20")
    if not 0.25 <= game_scale <= 3.0:
        fail(source, f"{name}: uniform scale is outside 0.25..3.0")

    return {
        "name": name,
        "type": prop_type,
        "group": group,
        "order": order,
        "x": game_x,
        "z": game_z,
        "scale": game_scale,
        "rotation": game_rotation,
    }


def render_scene(source: Path) -> str:
    data = load_scene(source)
    compiled = [compile_object(source, raw) for raw in data["objects"]]

    names = [item["name"] for item in compiled]
    if len(names) != len(set(names)):
        fail(source, "object names must be unique")
    orders = [item["order"] for item in compiled]
    if len(orders) != len(set(orders)):
        fail(source, "object order values must be unique")

    compiled.sort(key=lambda item: (item["order"], item["name"]))
    lines = [
        "; Citadel Defense 3D scene art v1",
        f"level={data['level']}",
        f"theme={data['theme']}",
    ]

    previous_group: str | None = None
    for item in compiled:
        if item["group"] != previous_group:
            lines.append(f"; {item['group']}")
            previous_group = item["group"]
        lines.append(
            "prop="
            + ",".join(
                (
                    item["type"],
                    compact_number(item["x"], True),
                    compact_number(item["z"], True),
                    compact_number(item["scale"], True),
                    compact_number(item["rotation"], False),
                )
            )
        )
    return "\n".join(lines) + "\n"


def write_or_check(source: Path, output: Path, check: bool) -> None:
    generated = render_scene(source)
    if check:
        try:
            current = output.read_text(encoding="utf-8")
        except OSError as error:
            fail(output, f"cannot read generated scene: {error}")
        if current != generated:
            fail(
                output,
                "generated scene is stale; run scripts/export_scene_art.py without --check",
            )
        return

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(generated, encoding="utf-8", newline="\n")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Compile Blender-style scene JSON into a Citadel Defense .art file."
    )
    parser.add_argument("source", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)

    try:
        write_or_check(args.source, args.output, args.check)
    except SceneExportError as error:
        print(f"Scene export failed: {error}", file=sys.stderr)
        return 1

    action = "Verified" if args.check else "Generated"
    print(f"{action} {args.output} from {args.source}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
