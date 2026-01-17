#!/usr/bin/env python3
"""
Convert a wallpaper image into a raw 320x200 (Mode 13h) byte stream
compatible with HuTaOS. Output is 64000 bytes of palette indices that
match the VGA palette configured in graphics_initialize().

Usage:
    python3 wallpaper-to-bin.py <input_image> <output_bin>
"""

import sys
import os
from PIL import Image


def build_vga_palette():
    """Build the same 256-color VGA palette used by the kernel."""
    palette = []

    # First 16 colors - EGA compatibility
    palette.extend([
        (0, 0, 0), (0, 0, 170), (0, 170, 0), (0, 170, 170),
        (170, 0, 0), (170, 0, 170), (170, 85, 0), (170, 170, 170),
        (85, 85, 85), (85, 85, 255), (85, 255, 85), (85, 255, 255),
        (255, 85, 85), (255, 85, 255), (255, 255, 85), (255, 255, 255),
    ])

    # 6x6x6 color cube (216 colors)
    for r in range(6):
        for g in range(6):
            for b in range(6):
                palette.append(
                    (int(r * 255 / 5), int(g * 255 / 5), int(b * 255 / 5))
                )

    # Grayscale ramp (24 shades)
    for i in range(1, 24):
        gray = int(i * 255 / 23)
        palette.append((gray, gray, gray))

    # Pad to 256 entries
    while len(palette) < 256:
        palette.append((0, 0, 0))

    return palette


def find_closest_color(r, g, b, palette):
    """Return palette index with smallest weighted distance."""
    best_idx = 0
    best_dist = float("inf")
    for idx, (pr, pg, pb) in enumerate(palette):
        dist = ((r - pr) * 0.299) ** 2 + ((g - pg) * 0.587) ** 2 + ((b - pb) * 0.114) ** 2
        if dist < best_dist:
            best_dist = dist
            best_idx = idx
    return best_idx


def convert(input_path, output_path):
    palette = build_vga_palette()

    img = Image.open(input_path).convert("RGB")
    img = img.resize((320, 200), Image.LANCZOS)

    pixels = list(img.getdata())
    vga_bytes = bytearray()

    for (r, g, b) in pixels:
        vga_bytes.append(find_closest_color(r, g, b, palette))

    with open(output_path, "wb") as f:
        f.write(vga_bytes)

    print(f"Wrote {len(vga_bytes)} bytes to {output_path}")


def main():
    if len(sys.argv) != 3:
        print("Usage: python3 wallpaper-to-bin.py <input_image> <output_bin>")
        sys.exit(1)

    input_path, output_path = sys.argv[1], sys.argv[2]
    if not os.path.exists(input_path):
        print(f"Input file not found: {input_path}")
        sys.exit(1)

    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    convert(input_path, output_path)


if __name__ == "__main__":
    main()
