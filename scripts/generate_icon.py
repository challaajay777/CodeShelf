#!/usr/bin/env python3
"""
Generate high-resolution application icon for CodeShelf.
Creates a professional multi-resolution .ico file and PNG icon.
"""

from PIL import Image, ImageDraw, ImageFont
import os
from pathlib import Path

def create_icon_image(size: int) -> Image.Image:
    # 256x256 base canvas
    scale = size / 256.0
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Base dimensions
    pad = int(16 * scale)
    radius = int(48 * scale)
    box = [pad, pad, size - pad, size - pad]

    # Background rounded rect (deep slate)
    bg_color = (24, 32, 47, 255) # Slate 900
    border_color = (51, 65, 85, 255) # Slate 700
    border_w = max(1, int(4 * scale))

    draw.rounded_rectangle(box, radius=radius, fill=bg_color, outline=border_color, width=border_w)

    # Draw a stylized code shelf inside
    # Shelf baseline
    shelf_y = int(185 * scale)
    shelf_h = max(2, int(8 * scale))
    shelf_pad = int(36 * scale)
    draw.rounded_rectangle(
        [shelf_pad, shelf_y, size - shelf_pad, shelf_y + shelf_h],
        radius=max(1, int(4 * scale)),
        fill=(71, 85, 105, 255) # Slate 600
    )

    # Left Book / Binder (Blue)
    b1_x0 = int(50 * scale)
    b1_w = int(24 * scale)
    b1_y0 = int(80 * scale)
    draw.rounded_rectangle([b1_x0, b1_y0, b1_x0 + b1_w, shelf_y], radius=max(1, int(4 * scale)), fill=(59, 130, 246, 255))

    # Second Book / Binder (Cyan)
    b2_x0 = int(80 * scale)
    b2_w = int(22 * scale)
    b2_y0 = int(100 * scale)
    draw.rounded_rectangle([b2_x0, b2_y0, b2_x0 + b2_w, shelf_y], radius=max(1, int(4 * scale)), fill=(14, 165, 233, 255))

    # Center Code Brackets: < / >
    # Draw '<'
    bracket_color = (241, 245, 249, 255) # Slate 100
    line_w = max(2, int(9 * scale))

    # Left angle '<'
    p1 = (int(125 * scale), int(105 * scale))
    p2 = (int(110 * scale), int(128 * scale))
    p3 = (int(125 * scale), int(151 * scale))
    draw.line([p1, p2], fill=bracket_color, width=line_w)
    draw.line([p2, p3], fill=bracket_color, width=line_w)

    # Slash '/'
    s1 = (int(155 * scale), int(95 * scale))
    s2 = (int(140 * scale), int(160 * scale))
    draw.line([s1, s2], fill=(96, 165, 250, 255), width=line_w) # Accent blue

    # Right angle '>'
    p4 = (int(170 * scale), int(105 * scale))
    p5 = (int(185 * scale), int(128 * scale))
    p6 = (int(170 * scale), int(151 * scale))
    draw.line([p4, p5], fill=bracket_color, width=line_w)
    draw.line([p5, p6], fill=bracket_color, width=line_w)

    # Right book / binder (Emerald)
    b3_x0 = int(198 * scale)
    b3_w = int(22 * scale)
    b3_y0 = int(90 * scale)
    draw.rounded_rectangle([b3_x0, b3_y0, b3_x0 + b3_w, shelf_y], radius=max(1, int(4 * scale)), fill=(16, 185, 129, 255))

    return img

def main():
    root = Path(__file__).resolve().parent.parent
    res_icons = root / "resources" / "icons"
    res_icons.mkdir(parents=True, exist_ok=True)

    sizes = [16, 32, 48, 64, 128, 256]
    images = [create_icon_image(s) for s in sizes]

    ico_path = res_icons / "codeshelf.ico"
    png_path = res_icons / "codeshelf.png"

    # Save PNG (256x256)
    images[-1].save(png_path, format="PNG")

    # Save multi-size ICO
    images[-1].save(
        ico_path,
        format="ICO",
        sizes=[(s, s) for s in sizes],
        append_images=images[:-1]
    )

    print(f"Generated icon: {ico_path} ({os.path.getsize(ico_path)} bytes)")
    print(f"Generated PNG:  {png_path} ({os.path.getsize(png_path)} bytes)")

if __name__ == "__main__":
    main()
