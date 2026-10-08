#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Raaj Draw: draw the branding artwork and write it over Inkscape's.

Run from the repository root:  python raaj/make-assets.py
Needs Pillow. The results are committed, so builds do not need to run this.

Writes: app icons (PNG sizes, scalable and symbolic SVG, .ico), the About-menu logo icons,
the start/welcome images, the About screen (about00.svgz) and the NSIS installer bitmaps.
File names stay Inkscape's so nothing else has to change.
"""

import gzip
import os
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
INK = (22, 24, 29)
PAPER = (251, 250, 247)
STONE = (243, 241, 236)
MUTED = (100, 106, 116)
TEAL_LIGHT = (95, 179, 170)
TEAL = (31, 107, 100)

FONT_BOLD = [r"C:\Windows\Fonts\segoeuib.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"]
FONT_REGULAR = [r"C:\Windows\Fonts\segoeui.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"]

# The mark, in a 64×64 design space (same as the website favicon):
# dark rounded square, a teal pen stroke from bottom-left to top-right, a light nib dot, a light cap line.
PEN_FROM, PEN_TO, PEN_WIDTH = (19.5, 45.0), (41.5, 19.5), 11.0
DOT, DOT_R = (18.0, 46.0), 3.4
CAP_FROM, CAP_TO, CAP_WIDTH = (38.0, 18.0), (45.0, 22.0), 3.2

LOGO_SVG = """<svg xmlns="http://www.w3.org/2000/svg" width="{size}" height="{size}" viewBox="0 0 64 64">
  <defs><linearGradient id="raaj-pen" x1="0" y1="0" x2="1" y2="1">
    <stop offset="0" stop-color="#5fb3aa"/><stop offset="1" stop-color="#1f6b64"/></linearGradient></defs>
  <rect width="64" height="64" rx="14" fill="#16181d"/>
  <path d="M19.5 45 L41.5 19.5" stroke="url(#raaj-pen)" stroke-width="11" stroke-linecap="round"/>
  <circle cx="18" cy="46" r="3.4" fill="#fbfaf7"/>
  <path d="M38 18 L45 22" stroke="#fbfaf7" stroke-width="3.2" stroke-linecap="round"/>
</svg>
"""

SYMBOLIC_SVG = """<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 16 16">
  <path fill="#bebebe" d="M3.5 0h9A3.5 3.5 0 0 1 16 3.5v9a3.5 3.5 0 0 1-3.5 3.5h-9A3.5 3.5 0 0 1 0 12.5v-9A3.5 3.5 0 0 1 3.5 0zm6.9 4.1a1.4 1.4 0 0 0-1.9.4l-4.3 5.9a1.4 1.4 0 0 0 2.2 1.6l4.3-5.9a1.4 1.4 0 0 0-.3-2zM4 11.1a.9.9 0 1 0 0 1.8.9.9 0 0 0 0-1.8z"/>
</svg>
"""


def font(candidates, size):
    for path in candidates:
        if os.path.exists(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def draw_mark(size, rounded=True, background=INK):
    """The mark at `size` pixels, drawn at 8× and scaled down for smooth edges."""
    s = 8
    big = size * s
    k = big / 64.0
    img = Image.new("RGBA", (big, big), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    if rounded:
        d.rounded_rectangle([0, 0, big - 1, big - 1], radius=int(14 * k), fill=background)
    else:
        d.rectangle([0, 0, big - 1, big - 1], fill=background)

    # teal pen: a gradient band masked by a round-capped thick line
    band = Image.new("RGBA", (big, big))
    bd = ImageDraw.Draw(band)
    for i in range(big):
        t = i / max(1, big - 1)
        c = tuple(int(TEAL_LIGHT[j] + (TEAL[j] - TEAL_LIGHT[j]) * t) for j in range(3))
        bd.line([(0, i), (big, i)], fill=c + (255,))
    mask = Image.new("L", (big, big), 0)
    md = ImageDraw.Draw(mask)
    (x0, y0), (x1, y1) = [(p[0] * k, p[1] * k) for p in (PEN_FROM, PEN_TO)]
    w = PEN_WIDTH * k
    md.line([(x0, y0), (x1, y1)], fill=255, width=int(w))
    for (x, y) in ((x0, y0), (x1, y1)):
        md.ellipse([x - w / 2, y - w / 2, x + w / 2, y + w / 2], fill=255)
    img.paste(band, (0, 0), mask)

    cx, cy, r = DOT[0] * k, DOT[1] * k, DOT_R * k
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=PAPER)
    (a0, b0), (a1, b1) = [(p[0] * k, p[1] * k) for p in (CAP_FROM, CAP_TO)]
    cw = CAP_WIDTH * k
    d.line([(a0, b0), (a1, b1)], fill=PAPER, width=int(cw))
    for (x, y) in ((a0, b0), (a1, b1)):
        d.ellipse([x - cw / 2, y - cw / 2, x + cw / 2, y + cw / 2], fill=PAPER)
    return img.resize((size, size), Image.LANCZOS)


def write(path, data):
    p = ROOT / path
    p.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(data, Image.Image):
        data.save(p)
    elif isinstance(data, bytes):
        p.write_bytes(data)
    else:
        p.write_text(data, encoding="utf-8", newline="\n")
    print("wrote", path)


def banner(width, height, title, subtitle, dark=True):
    """Start-screen style banner: mark on the left, name and a line of text."""
    bg = INK if dark else STONE
    fg = PAPER if dark else INK
    sub = (170, 175, 182) if dark else MUTED
    img = Image.new("RGB", (width, height), bg)
    d = ImageDraw.Draw(img)
    # soft teal arc in the corner
    d.ellipse([width - 170, -150, width + 170, 190], fill=(29, 45, 44) if dark else (227, 239, 237))
    mark = draw_mark(int(height * 0.5))
    img.paste(mark, (int(height * 0.25), int(height * 0.25)), mark)
    tx = int(height * 0.25) + mark.width + 28
    d.text((tx, int(height * 0.30)), title, font=font(FONT_BOLD, int(height * 0.2)), fill=fg)
    d.text((tx, int(height * 0.58)), subtitle, font=font(FONT_REGULAR, int(height * 0.085)), fill=sub)
    return img


def tile(size, glyph):
    """Small square illustration for the welcome screen's account page."""
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle([0, 0, size - 1, size - 1], radius=size // 6, fill=(227, 239, 237, 255))
    f = font(FONT_BOLD, int(size * 0.42))
    bbox = d.textbbox((0, 0), glyph, font=f)
    d.text(((size - (bbox[2] - bbox[0])) / 2 - bbox[0], (size - (bbox[3] - bbox[1])) / 2 - bbox[1]),
           glyph, font=f, fill=TEAL)
    return img


ABOUT_SVG = """<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="750" height="625" viewBox="0 0 750 625">
  <defs>
    <linearGradient id="bg" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#16181d"/><stop offset="1" stop-color="#1d2d2c"/></linearGradient>
    <linearGradient id="pen" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#5fb3aa"/><stop offset="1" stop-color="#1f6b64"/></linearGradient>
  </defs>
  <rect width="750" height="625" fill="url(#bg)"/>
  <circle cx="660" cy="80" r="220" fill="#5fb3aa" opacity="0.08"/>
  <circle cx="90" cy="600" r="160" fill="#5fb3aa" opacity="0.06"/>
  <g transform="translate(305 120) scale(2.2)">
    <rect width="64" height="64" rx="14" fill="#0f1114"/>
    <path d="M19.5 45 L41.5 19.5" stroke="url(#pen)" stroke-width="11" stroke-linecap="round"/>
    <circle cx="18" cy="46" r="3.4" fill="#fbfaf7"/>
    <path d="M38 18 L45 22" stroke="#fbfaf7" stroke-width="3.2" stroke-linecap="round"/>
  </g>
  <text x="375" y="350" text-anchor="middle" fill="#f1f0ec" font-family="Inter, 'Segoe UI', sans-serif" font-size="56" font-weight="700">Raaj Draw</text>
  <text x="375" y="392" text-anchor="middle" fill="#9a9ea6" font-family="Inter, 'Segoe UI', sans-serif" font-size="20">Vector drawing by Raaj Software</text>
  <text x="375" y="560" text-anchor="middle" fill="#9a9ea6" font-family="Inter, 'Segoe UI', sans-serif" font-size="15">Based on Inkscape, free software by the Inkscape developers (GNU GPL)</text>
  <text x="375" y="584" text-anchor="middle" fill="#5fb3aa" font-family="Inter, 'Segoe UI', sans-serif" font-size="15">draw.raajsoftware.com</text>
</svg>
"""


def main():
    # application icons (Inkscape's file names, so the build and desktop files stay as they are)
    for n in (16, 22, 24, 32, 48, 256):
        write(f"share/icons/application/{n}x{n}/org.inkscape.Inkscape.png", draw_mark(n))
    write("share/icons/application/scalable/org.inkscape.Inkscape.svg", LOGO_SVG.format(size=256))
    write("share/icons/application/symbolic/org.inkscape.Inkscape-symbolic.svg", SYMBOLIC_SVG)
    write("share/branding/inkscape.svg", LOGO_SVG.format(size=256))
    write("raaj/branding/raajdraw.svg", LOGO_SVG.format(size=256))

    ico = draw_mark(256)
    ico_path = ROOT / "share/branding/inkscape.ico"
    ico.save(ico_path, sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    print("wrote share/branding/inkscape.ico")

    # "About Raaj Draw" menu icon in each icon theme
    for theme in ("hicolor", "Tango", "Dash"):
        if (ROOT / f"share/icons/{theme}/scalable/actions/inkscape-logo.svg").exists():
            write(f"share/icons/{theme}/scalable/actions/inkscape-logo.svg", LOGO_SVG.format(size=16))
    for theme in ("hicolor", "Dash", "multicolor"):
        if (ROOT / f"share/icons/{theme}/symbolic/actions/inkscape-logo-symbolic.svg").exists():
            write(f"share/icons/{theme}/symbolic/actions/inkscape-logo-symbolic.svg", SYMBOLIC_SVG)

    # start / welcome screens (700×220) and the account-page tiles
    write("share/screens/start-splash.png", banner(700, 220, "Raaj Draw", "Starting…"))
    write("share/screens/start-welcome.png", banner(700, 220, "Raaj Draw", "Welcome! Set up how Raaj Draw looks and works."))
    write("share/screens/start-support.png", banner(700, 220, "Raaj Draw", "Your account and plan"))
    write("share/screens/start-support-time.png", tile(150, "₹"))
    write("share/screens/start-support-money.png", tile(140, "✓"))

    # About screen
    write("share/screens/about/about00.svgz", gzip.compress(ABOUT_SVG.encode("utf-8"), mtime=0))

    # NSIS installer bitmaps (24-bit BMP)
    write("packaging/nsis/header.bmp", banner(150, 57, "Raaj Draw", "", dark=False).convert("RGB"))
    side = Image.new("RGB", (164, 314), INK)
    mark = draw_mark(96)
    side.paste(mark, (34, 60), mark)
    sd = ImageDraw.Draw(side)
    sd.text((82, 190), "Raaj Draw", font=font(FONT_BOLD, 22), fill=PAPER, anchor="mm")
    sd.text((82, 218), "by Raaj Software", font=font(FONT_REGULAR, 12), fill=(170, 175, 182), anchor="mm")
    write("packaging/nsis/welcomefinish.bmp", side)


if __name__ == "__main__":
    main()
