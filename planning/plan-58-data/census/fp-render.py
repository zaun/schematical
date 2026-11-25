"""plan-58-C Phase 2: draw footprints to PNG, to read beside the datasheet drawing.

usage: python3 -I fp-render.py <out-dir> <footprints.json> [id ...]
       python3 -I fp-render.py --selftest

Draws each footprint (every id of the file, or the ones named) as
<out-dir>/<id>.png: a 1 mm grid (darker every 5 mm), the body outline (blue), every
pad (copper; a round pad as an ellipse, a hole white), its pin number (`0` pads are
labelled `.`), the origin cross (red) and the id; x grows right, y grows down, as the
library's footprints do.  60 pixels per millimetre.  Only `explicit` footprints (the
slide-switch style) are drawn.  A render from the JSON, not the app's renderer: a
check that the pads sit where the drawing's dimensions put them.
"""
import json
import os
import sys

from PIL import Image, ImageDraw

PX = 60


def extent(fp):
    xs, ys = [fp["body"]["width"] / 2], [fp["body"]["height"] / 2]
    for p in fp["pads"]:
        xs.append(abs(p["x"]) + p["width"] / 2)
        ys.append(abs(p["y"]) + p["height"] / 2)
    return max(xs) + 1.5, max(ys) + 1.5


def render(fp, path):
    ex, ey = extent(fp)
    w, h = int(2 * ex * PX), int(2 * ey * PX)
    im = Image.new("RGB", (w, h), "white")
    d = ImageDraw.Draw(im)

    def P(x, y):
        return (w / 2 + x * PX, h / 2 + y * PX)

    for i in range(-int(ex) - 1, int(ex) + 2):
        d.line([P(i, -ey), P(i, ey)], fill=(200, 200, 200) if i % 5 == 0 else (235, 235, 235))
    for j in range(-int(ey) - 1, int(ey) + 2):
        d.line([P(-ex, j), P(ex, j)], fill=(200, 200, 200) if j % 5 == 0 else (235, 235, 235))
    b = fp["body"]
    d.rectangle([P(-b["width"] / 2, -b["height"] / 2), P(b["width"] / 2, b["height"] / 2)], outline=(0, 0, 220), width=3)
    for p in fp["pads"]:
        x0, y0 = P(p["x"] - p["width"] / 2, p["y"] - p["height"] / 2)
        x1, y1 = P(p["x"] + p["width"] / 2, p["y"] + p["height"] / 2)
        copper = (217, 140, 26)
        if p.get("shape") == "round":
            d.rounded_rectangle([x0, y0, x1, y1], radius=min(x1 - x0, y1 - y0) / 2, fill=copper)
        else:
            d.rectangle([x0, y0, x1, y1], fill=copper)
        if p.get("drill"):
            cx, cy = P(p["x"], p["y"])
            r = p["drill"] / 2 * PX
            if p.get("slotLength"):
                half = (p["slotLength"] - p["drill"]) / 2 * PX
                horizontal = p["width"] >= p["height"]
                box = [cx - r - (half if horizontal else 0), cy - r - (0 if horizontal else half),
                       cx + r + (half if horizontal else 0), cy + r + (0 if horizontal else half)]
                d.rounded_rectangle(box, radius=r, fill="white")
            else:
                d.ellipse([cx - r, cy - r, cx + r, cy + r], fill="white")
        d.text(P(p["x"], p["y"]), str(p.get("pin", ".")) if p.get("pin", 0) else ".", fill="black", anchor="mm")
    ox, oy = P(0, 0)
    d.line([ox - 12, oy, ox + 12, oy], fill=(255, 0, 0), width=2)
    d.line([ox, oy - 12, ox, oy + 12], fill=(255, 0, 0), width=2)
    d.text((6, 4), fp["id"], fill="black")
    im.save(path)


def main(out_dir, path, ids):
    os.makedirs(out_dir, exist_ok=True)
    n = 0
    for fp in json.load(open(path))["footprints"]:
        if (not ids or fp["id"] in ids) and fp["style"] == "explicit":
            render(fp, os.path.join(out_dir, fp["id"] + ".png"))
            n += 1
    print(n, "images in", out_dir)
    return 0


def selftest():
    import tempfile
    fp = {"id": "T", "style": "explicit", "pins": 2, "body": {"width": 6, "height": 4},
          "pads": [{"pin": 1, "x": -1.5, "y": 0, "width": 1.6, "height": 1.6, "shape": "round", "drill": 0.8},
                   {"pin": 0, "x": 2, "y": 0, "width": 1.2, "height": 2.6, "shape": "round", "drill": 1.0, "slotLength": 2.2}]}
    with tempfile.TemporaryDirectory() as d:
        render(fp, os.path.join(d, "T.png"))
        im = Image.open(os.path.join(d, "T.png"))
        assert im.size[0] > 200 and im.size[1] > 150
    print("ok")
    return 0


if __name__ == "__main__":
    if sys.argv[1:] == ["--selftest"]:
        sys.exit(selftest())
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1], sys.argv[2], set(sys.argv[3:])))
