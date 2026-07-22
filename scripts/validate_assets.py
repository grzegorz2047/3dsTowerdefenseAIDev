#!/usr/bin/env python3

import json
import math
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST_DIR = ROOT / "assets" / "manifests"
LICENSE_REGISTRY = ROOT / "assets" / "LICENSES.md"

BUDGETS = {
    "enemy": (350, 1),
    "elite_enemy": (600, 1),
    "boss": (1200, 2),
    "tower": (600, 2),
    "projectile": (80, 1),
    "base": (500, 2),
    "spawn": (500, 2),
    "tile": (120, 1),
    "decoration": (150, 1),
}

ID_PATTERN = re.compile(r"^[a-z0-9_]+$")
REQUIRED_FIELDS = {
    "schema",
    "id",
    "kind",
    "display_name",
    "author",
    "license",
    "source",
    "generated",
    "forward_axis",
    "up_axis",
    "pivot",
    "triangle_count",
    "material_count",
    "bounds",
}


def fail(path: Path, message: str) -> None:
    raise ValueError(f"{path.relative_to(ROOT)}: {message}")


def finite_vector(path: Path, value: object, field: str) -> list[float]:
    if not isinstance(value, list) or len(value) != 3:
        fail(path, f"{field} must contain exactly three numbers")
    result: list[float] = []
    for component in value:
        if not isinstance(component, (int, float)) or not math.isfinite(component):
            fail(path, f"{field} contains a non-finite value")
        result.append(float(component))
    return result


def power_of_two(value: object) -> bool:
    return isinstance(value, int) and value > 0 and (value & (value - 1)) == 0


def validate_manifest(path: Path, registered_licenses: str, seen_ids: set[str]) -> None:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        fail(path, f"cannot parse JSON: {error}")

    if not isinstance(data, dict):
        fail(path, "root must be a JSON object")

    missing = sorted(REQUIRED_FIELDS - data.keys())
    if missing:
        fail(path, f"missing required fields: {', '.join(missing)}")

    if data["schema"] != 1:
        fail(path, "unsupported schema; expected 1")

    asset_id = data["id"]
    if not isinstance(asset_id, str) or not ID_PATTERN.fullmatch(asset_id):
        fail(path, "id must use lowercase letters, digits and underscores")
    if asset_id in seen_ids:
        fail(path, f"duplicate asset id: {asset_id}")
    seen_ids.add(asset_id)

    kind = data["kind"]
    if kind not in BUDGETS:
        fail(path, f"unsupported kind: {kind}")

    for field in ("display_name", "author", "license", "source", "generated"):
        if not isinstance(data[field], str) or not data[field].strip():
            fail(path, f"{field} must be a non-empty string")

    if f"`{asset_id}`" not in registered_licenses:
        fail(path, "asset id is missing from assets/LICENSES.md")

    if data["forward_axis"] != "+Z" or data["up_axis"] != "+Y":
        fail(path, "axes must be forward +Z and up +Y")

    allowed_pivots = {"base_center"}
    if kind == "projectile":
        allowed_pivots.add("center")
    if data["pivot"] not in allowed_pivots:
        fail(path, f"invalid pivot for {kind}: {data['pivot']}")

    triangle_count = data["triangle_count"]
    material_count = data["material_count"]
    if not isinstance(triangle_count, int) or triangle_count <= 0:
        fail(path, "triangle_count must be a positive integer")
    if not isinstance(material_count, int) or material_count <= 0:
        fail(path, "material_count must be a positive integer")

    max_triangles, max_materials = BUDGETS[kind]
    if triangle_count > max_triangles:
        fail(path, f"triangle budget exceeded: {triangle_count} > {max_triangles}")
    if material_count > max_materials:
        fail(path, f"material budget exceeded: {material_count} > {max_materials}")

    bounds = data["bounds"]
    if not isinstance(bounds, dict) or "min" not in bounds or "max" not in bounds:
        fail(path, "bounds must contain min and max")
    minimum = finite_vector(path, bounds["min"], "bounds.min")
    maximum = finite_vector(path, bounds["max"], "bounds.max")
    if any(low >= high for low, high in zip(minimum, maximum)):
        fail(path, "each bounds.min component must be lower than bounds.max")
    if data["pivot"] == "base_center" and abs(minimum[1]) > 0.1001:
        fail(path, "base_center assets must start near y=0")

    texture = data.get("texture")
    if texture is not None:
        if not isinstance(texture, dict):
            fail(path, "texture must be an object")
        for field in ("path", "width", "height"):
            if field not in texture:
                fail(path, f"texture is missing {field}")
        if not isinstance(texture["path"], str) or not texture["path"].strip():
            fail(path, "texture.path must be non-empty")
        if not power_of_two(texture["width"]) or not power_of_two(texture["height"]):
            fail(path, "texture dimensions must be powers of two")
        if texture["width"] > 256 or texture["height"] > 256:
            fail(path, "texture dimensions exceed 256x256")


def main() -> int:
    if not LICENSE_REGISTRY.is_file():
        print("assets/LICENSES.md is missing", file=sys.stderr)
        return 1

    manifests = sorted(MANIFEST_DIR.glob("*.asset.json"))
    if not manifests:
        print("No asset manifests found", file=sys.stderr)
        return 1

    registry = LICENSE_REGISTRY.read_text(encoding="utf-8")
    seen_ids: set[str] = set()
    try:
        for manifest in manifests:
            validate_manifest(manifest, registry, seen_ids)
    except ValueError as error:
        print(f"Asset validation failed: {error}", file=sys.stderr)
        return 1

    print(f"Validated {len(manifests)} asset manifest(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
