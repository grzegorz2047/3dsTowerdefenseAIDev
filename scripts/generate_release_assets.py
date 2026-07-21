#!/usr/bin/env python3
"""Generate deterministic, original PNG and WAV assets for CIA packaging."""

from __future__ import annotations

import binascii
import pathlib
import struct
import wave
import zlib

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build" / "release-assets"


def png_chunk(kind: bytes, payload: bytes) -> bytes:
    return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", binascii.crc32(kind + payload) & 0xFFFFFFFF)


def write_png(path: pathlib.Path, width: int, height: int, pixel) -> None:
    rows = bytearray()
    for y in range(height):
        rows.append(0)
        for x in range(width):
            rows.extend(pixel(x, y, width, height))
    data = b"\x89PNG\r\n\x1a\n"
    data += png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
    data += png_chunk(b"IDAT", zlib.compress(bytes(rows), 9))
    data += png_chunk(b"IEND", b"")
    path.write_bytes(data)


def branded_pixel(x: int, y: int, width: int, height: int) -> bytes:
    nx = x / max(width - 1, 1)
    ny = y / max(height - 1, 1)
    r = int(18 + 22 * ny)
    g = int(28 + 35 * ny)
    b = int(48 + 58 * nx)

    center_x = width * 0.5
    ground_y = height * 0.74
    tower_width = width * 0.30
    tower_height = height * 0.46
    in_tower = abs(x - center_x) <= tower_width * 0.5 and ground_y - tower_height <= y <= ground_y
    in_roof = abs(x - center_x) + abs(y - (ground_y - tower_height)) * 1.25 <= tower_width * 0.62
    in_path = abs(y - (height * 0.83 - (x - center_x) * 0.12)) <= max(height * 0.045, 2)

    if in_path:
        r, g, b = 170, 118, 58
    if in_tower:
        r, g, b = 48, 142, 218
    if in_roof:
        r, g, b = 82, 202, 126

    border = min(x, y, width - 1 - x, height - 1 - y)
    if border < max(1, width // 48):
        r, g, b = 230, 238, 248
    return bytes((r, g, b, 255))


def write_silence(path: pathlib.Path) -> None:
    sample_rate = 8000
    frame_count = sample_rate // 4
    with wave.open(str(path), "wb") as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(sample_rate)
        output.writeframes(b"\x00\x00" * frame_count)


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    write_png(OUTPUT / "icon.png", 48, 48, branded_pixel)
    write_png(OUTPUT / "banner.png", 256, 128, branded_pixel)
    write_silence(OUTPUT / "banner.wav")


if __name__ == "__main__":
    main()
