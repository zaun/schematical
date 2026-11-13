"""plan-36-E: the Espressif module footprints, read from the datasheets' vector drawings.

usage: /path/to/venv/python -I fp-specs-E.py [--diag ID]
       (needs PyMuPDF: python3 -m venv V && V/bin/pip install pymupdf)

Writes footprints-E.json, fills package-codes.tsv (drawing, footprint,
package `Module`) for the ESP32 rows (a module code's default: the
antenna-on-board module) and writes footprint-mpn.tsv (`mpn code
footprint drawing`) for the U variants (external antenna, a shorter body),
whose land pattern is a figure of its own. A figure drawn once for both
(WROVER-E / -IE) is one footprint.

Per module figure ("Recommended PCB Land Pattern"):
- pads: every closed quad / rect path and filled polygon, plus rectangles
  drawn as four loose blue lines; overlapping shapes (outline + fill
  strips) merge into one pad. Only pads inside the body (+1.5 mm) count,
  so the legend's sample pad drops out.
- scale: the edge pads' long side in points over its printed length (mm).
- body: W x L, the module size of the datasheet's ordering table. Its top
  is the outline's top edge (a black line W long with pads below it; the
  outline's sides, where drawn to the bottom, must be L long, else the L
  dimension's extension line must sit at top + L); its centre x is the
  outline's centre, which the pad ring's centre must match.
- designators: the numbers of the datasheet's "Pin Layout (Top View)"
  figure ("Pin N" or bare). Ring pads and ring labels (those on the
  outline) are matched in order around the centre, at the cyclic offset
  closest in normalised position (no pad off by more than 8 % of the
  outline); each inner pad takes the nearest inner label (a thermal grid
  is one pin, as the figures number it). Every label gets a pad.
- runs of pads are fitted to their pitch (no pad moves over 0.03 mm).
Coordinates: mm from the body centre, x right, y down, antenna end up
(the figures' orientation; pin 1 is at the top of the left side).
"""
import csv
import json
import math
import os
import re
import subprocess
import sys

import pymupdf

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.join(HERE, "..")
ROOT = os.path.abspath(os.path.join(DATA, "..", ".."))
E = "datasheets/mcu/espressif/"

# id, device code, pdf, land page, layout page, W, L, edge pad long side (mm),
# MPN prefixes using it (U variants; empty = the code's default), keep-out note
SPECS = [dict(zip(("id", "code", "pdf", "land", "layout", "W", "L", "long", "mpns"), row), **(row[9] if len(row) > 9 else {}))
         for row in [
    ("ESP32-C3-MINI-1", "ESP32-C3-MINI-1", "esp32-c3-mini-1_datasheet_en.pdf", 38, 10, 13.2, 16.6, 0.8, ""),
    ("ESP32-C3-MINI-1U", "ESP32-C3-MINI-1", "esp32-c3-mini-1_datasheet_en.pdf", 39, 10, 13.2, 12.5, 0.8, "ESP32-C3-MINI-1U"),
    ("ESP32-C3-WROOM-02", "ESP32-C3-WROOM-02", "esp32-c3-wroom-02_datasheet_en.pdf", 37, 10, 18.0, 20.0, 1.5, ""),
    ("ESP32-C3-WROOM-02U", "ESP32-C3-WROOM-02", "esp32-c3-wroom-02_datasheet_en.pdf", 38, 10, 18.0, 14.3, 1.5, "ESP32-C3-WROOM-02U"),
    ("ESP32-C6-MINI-1", "ESP32-C6-MINI-1", "esp32-c6-mini-1_mini-1u_datasheet_en.pdf", 44, 10, 13.2, 16.6, 0.8, ""),
    ("ESP32-C6-MINI-1U", "ESP32-C6-MINI-1", "esp32-c6-mini-1_mini-1u_datasheet_en.pdf", 45, 10, 13.2, 12.5, 0.8, "ESP32-C6-MINI-1U"),
    ("ESP32-C6-WROOM-1", "ESP32-C6-WROOM-1", "esp32-c6-wroom-1-wroom-1u_datasheet_en.pdf", 44, 10, 18.0, 25.5, 1.5, ""),
    ("ESP32-C6-WROOM-1U", "ESP32-C6-WROOM-1", "esp32-c6-wroom-1-wroom-1u_datasheet_en.pdf", 45, 10, 18.0, 19.2, 1.5, "ESP32-C6-WROOM-1U"),
    ("ESP32-MINI-1", "ESP32-MINI-1", "esp32-mini-1_datasheet_en.pdf", 44, 10, 13.2, 19.0, 0.8, ""),
    ("ESP32-MINI-1U", "ESP32-MINI-1", "esp32-mini-1_datasheet_en.pdf", 45, 10, 13.2, 13.5, 0.8, "ESP32-MINI-1U", {"fill_grid": True}),
    ("ESP32-S2-MINI-2", "ESP32-S2-MINI-2", "esp32-s2-mini-2_esp32-s2-mini-2u_datasheet_en.pdf", 31, 10, 15.4, 20.5, 0.8, ""),
    ("ESP32-S2-MINI-2U", "ESP32-S2-MINI-2", "esp32-s2-mini-2_esp32-s2-mini-2u_datasheet_en.pdf", 32, 10, 15.4, 15.4, 0.8, "ESP32-S2-MINI-2U"),
    ("ESP32-S2-SOLO-2", "ESP32-S2-SOLO-2", "esp32-s2-solo-2_esp32-s2-solo-2u_datasheet_en.pdf", 31, 11, 18.0, 25.5, 1.5, ""),
    ("ESP32-S2-SOLO-2U", "ESP32-S2-SOLO-2", "esp32-s2-solo-2_esp32-s2-solo-2u_datasheet_en.pdf", 32, 11, 18.0, 19.2, 1.5, "ESP32-S2-SOLO-2U"),
    ("ESP32-S3-MINI-1", "ESP32-S3-MINI-1", "esp32-s3-mini-1_mini-1u_datasheet_en.pdf", 45, 10, 15.4, 20.5, 0.8, ""),
    ("ESP32-S3-MINI-1U", "ESP32-S3-MINI-1", "esp32-s3-mini-1_mini-1u_datasheet_en.pdf", 46, 10, 15.4, 15.4, 0.8, "ESP32-S3-MINI-1U"),
    ("ESP32-S3-WROOM-1", "ESP32-S3-WROOM-1", "esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf", 45, 10, 18.0, 25.5, 1.5, ""),
    ("ESP32-S3-WROOM-1U", "ESP32-S3-WROOM-1", "esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf", 46, 10, 18.0, 19.2, 1.5, "ESP32-S3-WROOM-1U"),
    ("ESP32-WROOM-32E", "ESP32-WROOM-32", "esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf", 43, 10, 18.0, 25.5, 1.5, ""),
    ("ESP32-WROOM-32UE", "ESP32-WROOM-32", "esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf", 44, 10, 18.0, 19.2, 1.5, "ESP32-WROOM-32UE"),
    ("ESP32-WROVER-E", "ESP32-WROVER", "esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf", 45, 11, 18.0, 31.4, 1.5, ""),
]]
for _s in SPECS:
    _s["pdf"] = E + _s["pdf"]


def page_pads(pdf, pno):
    doc = pymupdf.open(os.path.join(ROOT, pdf))
    page = doc[pno - 1]
    pads, hlines, vlines = [], [], []
    bh, bv = [], []  # blue single segments: pads drawn as four loose lines
    for d in page.get_drawings():
        kinds = "".join(i[0] for i in d["items"])
        r = d["rect"]
        if not page.rect.contains(r):
            continue
        col = d.get("color")
        fill = d.get("fill")
        if kinds == "l" and col and col[2] > 0.5 and col[0] < 0.3:
            a, b = d["items"][0][1], d["items"][0][2]
            if abs(a.y - b.y) < 0.05 and abs(a.x - b.x) > 1:
                bh.append((min(a.x, b.x), max(a.x, b.x), a.y))
            elif abs(a.x - b.x) < 0.05 and abs(a.y - b.y) > 1:
                bv.append((min(a.y, b.y), max(a.y, b.y), a.x))
        closed = kinds in ("qu", "re") or (fill is not None and set(kinds) == {"l"} and len(kinds) >= 4)
        if closed and r.width < 40 and r.height < 40 and max(r.width, r.height) > 1 \
                and not (fill and fill[0] > 0.5 and fill[2] < 0.5):
            pads.append([r.x0, r.y0, r.x1, r.y1])
        elif col and max(col) < 0.1:
            segs = []
            for it in d["items"]:
                if it[0] == "l":
                    segs.append((it[1], it[2]))
                elif it[0] == "re":
                    q = it[1]
                    segs += [(q.tl, q.tr), (q.tr, q.br), (q.br, q.bl), (q.bl, q.tl)]
                elif it[0] == "qu":
                    q = it[1]
                    segs += [(q.ul, q.ur), (q.ur, q.lr), (q.lr, q.ll), (q.ll, q.ul)]
            for a, b in segs:
                if abs(a.y - b.y) < 0.2 and abs(a.x - b.x) > 5:
                    hlines.append((min(a.x, b.x), max(a.x, b.x), a.y))
                elif abs(a.x - b.x) < 0.2 and abs(a.y - b.y) > 5:
                    vlines.append((min(a.y, b.y), max(a.y, b.y), a.x))
    for x0, x1, y0 in bh:
        for u0, u1, y1 in bh:
            if y1 > y0 + 1 and abs(u0 - x0) < 0.1 and abs(u1 - x1) < 0.1:
                sides = [x for a, b, x in bv if a < y0 + 0.1 and b > y1 - 0.1 and (abs(x - x0) < 0.1 or abs(x - x1) < 0.1)]
                if len({round(x) for x in sides}) == 2:
                    pads.append([x0, y0, x1, y1])
                    break
    # merge overlapping shapes (a pad drawn as outline + fill strips) into one box
    out = []
    for p in pads:
        p = list(p)
        merged = True
        while merged:
            merged = False
            for q in out:
                if p[0] <= q[2] + 0.2 and q[0] <= p[2] + 0.2 and p[1] <= q[3] + 0.2 and q[1] <= p[3] + 0.2:
                    out.remove(q)
                    p = [min(p[0], q[0]), min(p[1], q[1]), max(p[2], q[2]), max(p[3], q[3])]
                    merged = True
                    break
        out.append(p)
    out = [tuple(p) for p in out if p[2] - p[0] > 1 and p[3] - p[1] > 1]
    return out, hlines, vlines


def layout_labels(pdf, pno):
    """{pin number: (x, y)} from the Pin Layout figure: each number word next to a "Pin" word."""
    xml = subprocess.run(["pdftotext", "-bbox", "-f", str(pno), "-l", str(pno), os.path.join(ROOT, pdf), "-"],
                         capture_output=True, text=True).stdout
    words = [(float(a), float(b), float(c), float(d), w) for a, b, c, d, w in
             re.findall(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>', xml)]
    # the figure: between the "Pin Layout" heading and the "Figure N. Pin Layout (Top View)" caption
    ys = [w[1] for i, w in enumerate(words) if w[4] == "Layout" and i + 1 < len(words) and words[i + 1][4] == "(Top"]
    hs = [w[1] for i, w in enumerate(words) if w[4] == "Layout" and (i + 1 == len(words) or words[i + 1][4] != "(Top")]
    if not ys:  # no caption: the figure ends at its note or the next section
        ys = [w[1] for w in words if w[4] in ("Note", "Note:", "3.2") and w[1] > min(hs)]
    y1 = min(ys)
    y0 = max([h for h in hs if h < y1], default=0)
    words = [w for w in words if y0 < w[1] < y1]
    pins = [w for w in words if w[4] == "Pin"]
    # number words of the label size (drops rotated figure text)
    cw = sorted((w[2] - w[0]) / len(w[4]) for w in words if w[4].isdigit())
    med = cw[len(cw) // 2] if cw else 0
    words = [w for w in words if not w[4].isdigit() or (w[2] - w[0]) / len(w[4]) < 2.5 * med]
    out = {}
    for k, w in enumerate(words):
        if not w[4].isdigit():
            continue
        cx, cy = (w[0] + w[2]) / 2, (w[1] + w[3]) / 2
        best = None
        for p in pins:
            px, py = (p[0] + p[2]) / 2, (p[1] + p[3]) / 2
            d = math.hypot(px - cx, py - cy)
            if d < 12 and (best is None or d < best[0]):
                best = (d, px, py)
        n = int(w[4])
        if n in out:
            raise SystemExit(f"{pdf} p.{pno}: label {n} twice")
        out[n] = ((cx + best[1]) / 2, (cy + best[2]) / 2) if best else (cx, cy)
    return out


def ring_split(pts, tol):
    """(ring, inner) index lists: ring = within tol of the bbox boundary (tol in the pts' units)."""
    x0 = min(p[0] for p in pts); x1 = max(p[0] for p in pts)
    y0 = min(p[1] for p in pts); y1 = max(p[1] for p in pts)
    ring = [i for i, p in enumerate(pts) if min(p[0] - x0, x1 - p[0], p[1] - y0, y1 - p[1]) < tol]
    return ring, [i for i in range(len(pts)) if i not in ring], (x0, y0, x1, y1)


def assign(pads_mm, labels):
    """pads_mm: [(x, y, w, h)]; labels: {n: (x, y)}. Returns [n per pad].

    Ring pads and ring labels (on the outline) are matched in order around
    the centre, at the cyclic offset closest in normalised position; each
    inner pad takes the nearest inner label (a thermal grid all one pin)."""
    ppts = [(p[0], p[1]) for p in pads_mm]
    lnums = sorted(labels)
    lpts = [labels[n] for n in lnums]
    pring, pinner, pb = ring_split(ppts, 1.0)
    lw = max(p[0] for p in lpts) - min(p[0] for p in lpts)
    lh = max(p[1] for p in lpts) - min(p[1] for p in lpts)
    lring, linner, lb = ring_split(lpts, 0.12 * min(lw, lh))
    if len(pring) != len(lring):
        raise SystemExit(f"{len(pring)} ring pads, {len(lring)} ring labels")

    def nf(b):
        return lambda p: ((p[0] - b[0]) / (b[2] - b[0]), (p[1] - b[1]) / (b[3] - b[1]))
    npd, nlb = nf(pb), nf(lb)
    ang = lambda q: math.atan2(q[1] - 0.5, q[0] - 0.5)
    po = sorted(pring, key=lambda i: ang(npd(ppts[i])))
    lo = sorted(lring, key=lambda j: ang(nlb(lpts[j])))
    n = len(po)
    best = min(range(n), key=lambda k: sum(math.dist(npd(ppts[po[i]]), nlb(lpts[lo[(i + k) % n]])) for i in range(n)))
    res = [None] * len(pads_mm)
    worst = 0
    for i in range(n):
        j = lo[(i + best) % n]
        res[po[i]] = lnums[j]
        worst = max(worst, math.dist(npd(ppts[po[i]]), nlb(lpts[j])))
    if worst > 0.08:
        raise SystemExit(f"ring match off by {worst:.2f} of the outline")
    for i in pinner:
        if not linner:
            raise SystemExit(f"inner pad at {ppts[i]} but no inner label")
        res[i] = lnums[min(linner, key=lambda j: math.dist(npd(ppts[i]), nlb(lpts[j])))]
    if set(res) != set(lnums):
        raise SystemExit(f"labels without pads: {sorted(set(lnums) - set(res))}")
    return res


def snap(v):
    """to 0.01 mm, halves away from zero"""
    return math.copysign(math.floor(abs(v) * 100 + 0.5 + 1e-9) / 100, v)


def regularise(mm, nums):
    """Fit each straight run of pads (a side's edge pads, a thermal grid's rows and
    columns) to an even pitch: the drawing's 0.01 mm jitter goes, nothing moves
    more than 0.03 mm (checked). Mirror-image columns / rows get equal |x| / |y|."""
    mm = [list(p) for p in mm]

    def runs(axis, key):
        groups = {}
        for i, p in enumerate(mm):
            if not key(p):
                continue
            k = next((g for g in groups if abs(g - p[1 - axis]) < 0.05), p[1 - axis])
            groups.setdefault(k, []).append(i)
        return [g for g in groups.values() if len(g) >= 3]

    def fit(idx, axis):
        idx = sorted(idx, key=lambda i: mm[i][axis])
        d = sorted(mm[b][axis] - mm[a][axis] for a, b in zip(idx, idx[1:]))
        pitch = round(d[len(d) // 2], 2)
        n = len(idx)
        c = snap(sum(mm[i][axis] for i in idx) / n)
        cross = snap(sum(mm[i][1 - axis] for i in idx) / n)
        for k, i in enumerate(idx):
            v = round(c + (k - (n - 1) / 2) * pitch, 3)
            if abs(v - mm[i][axis]) > 0.03 or abs(cross - mm[i][1 - axis]) > 0.03:
                raise SystemExit(f"pad {nums[i]} at {mm[i][:2]} is off the run ({v}, pitch {pitch})")
            mm[i][axis] = v
            mm[i][1 - axis] = cross
        return cross

    # side columns (pads wider than tall) run along y; top / bottom rows along x
    for g in runs(1, lambda p: p[2] > p[3]):
        fit(g, 1)
    for g in runs(0, lambda p: p[3] > p[2]):
        fit(g, 0)
    # thermal grids: square pads off the ring sharing a pin with others
    many = {n for n in nums if nums.count(n) > 1}
    sq = lambda p, i: p[2] == p[3] and nums[i] in many
    for axis in (0, 1):
        groups = {}
        for i, p in enumerate(mm):
            if sq(p, i):
                k = next((g for g in groups if abs(g - p[1 - axis]) < 0.05), p[1 - axis])
                groups.setdefault(k, []).append(i)
        for g in groups.values():
            if len(g) >= 2:
                fit(g, axis)
    # mirror pairs: a side column at -x and one at +x (rows at -y / +y) share |x| (|y|)
    for axis in (0, 1):
        vals = sorted({p[axis] for p in mm if (p[2] > p[3]) == (axis == 0) and p[2] != p[3]})
        for v in vals:
            if v < 0 and any(abs(w + v) < 0.03 for w in vals):
                w = next(w for w in vals if abs(w + v) < 0.03)
                m = snap((w - v) / 2)
                for p in mm:
                    if p[axis] == v:
                        p[axis] = -m
                    elif p[axis] == w:
                        p[axis] = m
    return [tuple(p) for p in mm]


def build(spec, diag=False):
    pdf, land, W, L, plong = spec["pdf"], spec["land"], spec["W"], spec["L"], spec["long"]
    raw, hl, vl = page_pads(pdf, land)
    # edge pads: the most common size class; scale from their long side
    sizes = {}
    for p in raw:
        k = (round(min(p[2] - p[0], p[3] - p[1]), 0), round(max(p[2] - p[0], p[3] - p[1]), 0))
        sizes.setdefault(k, []).append(max(p[2] - p[0], p[3] - p[1]))
    edge = max(sizes.values(), key=len)
    s = sorted(edge)[len(edge) // 2] / plong
    # body top: a horizontal line W long above the pad ring (the topmost such line
    # with pads below it); its ends give the body's x extent
    wl = sorted((y, x0, x1) for x0, x1, y in hl if abs((x1 - x0) / s - W) < 0.06)
    if diag:
        print("scale", s, "W lines", [(round(y, 1), round(x0, 1), round((x1 - x0) / s, 2)) for y, x0, x1 in wl])
        print("L lines", [(round(x, 1), round((y1 - y0) / s, 2), round(y0, 1)) for y0, y1, x in vl if abs((y1 - y0) / s - L) < 0.5])
    body = None
    for y, x0, x1 in wl:
        inside = [p for p in raw if x0 - 1.5 * s < (p[0] + p[2]) / 2 < x1 + 1.5 * s and y < p[1] < y + (L + 1.5) * s]
        if len(inside) >= 12:
            body = (y, x0, x1)
            break
    if body is None:
        raise SystemExit(f"{spec['id']}: no body top line {W} mm long")
    top, bx0, bx1 = body
    if bx0 is not None:
        # the outline's sides: vertical segments at the line's ends, run down from the top
        for xe in (bx0, bx1):
            reach = top
            for y0, y1, x in sorted(vl):
                if abs(x - xe) < 0.6 and y0 <= reach + 0.6:
                    reach = max(reach, y1)
            got = (reach - top) / s
            if got > 0.9 * L and abs(got - L) > 0.08:
                raise SystemExit(f"{spec['id']}: outline side at x {xe:.1f} is {got:.2f} mm, not L {L}")
            if got <= 0.9 * L and not any(abs(y - (top + L * s)) < 0.8 for x0, x1, y in hl):
                # the outline stops at the antenna area: the L dimension's extension line instead
                raise SystemExit(f"{spec['id']}: outline side {got:.2f} mm and no line at L {L}")
    pads = [p for p in raw if top < p[1] < top + (L + 1.5) * s
            and (bx0 is None or bx0 - 1.5 * s < (p[0] + p[2]) / 2 < bx1 + 1.5 * s)]
    ring_x = [((p[0] + p[2]) / 2) for p in pads]
    cx = (min(ring_x) + max(ring_x)) / 2
    if bx0 is not None and abs(cx - (bx0 + bx1) / 2) > 0.1 * s:
        raise SystemExit(f"{spec['id']}: pad ring centre {cx:.1f} off the outline centre {(bx0 + bx1) / 2:.1f}")
    cy = top + L * s / 2
    mm = [(snap(((p[0] + p[2]) / 2 - cx) / s) + 0.0, snap(((p[1] + p[3]) / 2 - cy) / s) + 0.0,
           snap((p[2] - p[0]) / s), snap((p[3] - p[1]) / s)) for p in pads]
    if spec.get("fill_grid"):
        # a thermal grid drawn with a cell missing: complete it (same size as the others)
        inner = [p for p in mm if abs(p[0]) < W / 2 - 2 and abs(p[1] - (max(q[1] for q in mm) - 4)) < 6 and p[2] == p[3]]
        xs = sorted({p[0] for p in inner}); ys = sorted({p[1] for p in inner})
        have = {(p[0], p[1]) for p in inner}
        miss = [(x, y) for x in xs for y in ys if (x, y) not in have]
        if len(xs) != 3 or len(ys) != 3 or len(miss) != 1:
            raise SystemExit(f"{spec['id']}: grid {xs} {ys} missing {miss}")
        mm.append((miss[0][0], miss[0][1], inner[0][2], inner[0][3]))
        spec["filled"] = miss[0]
    labels = layout_labels(pdf, spec["layout"])
    nums = assign(mm, labels)
    mm = regularise(mm, nums)
    # the antenna area: a second W-long line below the top, above the pads
    ant = [y for y, x0, x1 in wl if top + 1 < y < top + L * s / 2 and min(p[1] for p in pads) > y]
    spec["antenna"] = round((ant[0] - top) / s, 2) if ant else None
    return mm, nums, s


def describe(spec, mm, nums):
    many = sorted({k for k in nums if nums.count(k) > 1})
    grid = ", ".join(f"pin {k} a {nums.count(k)}-pad GND grid" for k in many)
    return (f"Espressif {spec['id']} module, {spec['W']} x {spec['L']} mm, {max(nums)} pins on {len(mm)} pads"
            + (f" ({grid})" if grid else ""))


def note(spec, s):
    pdf = spec["pdf"]
    t = subprocess.run(["pdftotext", "-f", str(spec["land"]), "-l", str(spec["land"]), os.path.join(ROOT, pdf), "-"],
                       capture_output=True, text=True).stdout
    fig = next((l.strip() for l in t.splitlines() if l.strip().startswith("Figure") and "Land Pattern" in l),
               "Recommended PCB Land Pattern")
    out = (f"{pdf} p.{spec['land']} {fig}: pads from the drawing's vector outlines "
           f"(scale {s:.3f} pt/mm from the {spec['long']} mm edge pads), runs fitted to their pitch; "
           f"pin numbers from the Pin Layout (Top View), p.{spec['layout']}; origin the body centre, antenna end up")
    if spec.get("antenna"):
        out += f"; antenna area (keep copper and parts clear): the top {spec['antenna']} mm of the body, full width"
    if spec.get("filled"):
        out += (f"; the figure leaves the thermal grid cell at {spec['filled']} undrawn (where its dimension lines "
                f"cross); drawn like the other eight, as in the pin layout and the non-U figure")
    return out


def main(argv):
    diag = argv[argv.index("--diag") + 1] if "--diag" in argv else None
    objs, codes, mpn_rows = [], {}, []
    cand = [r for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv")), delimiter="\t")
            if r["family"] == "ESP32" and r["verdict"] == "add"]
    for spec in SPECS:
        if diag and spec["id"] != diag:
            continue
        mm, nums, s = build(spec, diag=bool(diag))
        print(spec["id"], len(mm), "pads", len(set(nums)), "pins", f"scale {s:.3f}")
        if diag:
            for p, n in sorted(zip(mm, nums), key=lambda t: t[1]):
                print(n, p)
            continue
        order = sorted(range(len(mm)), key=lambda i: (nums[i], mm[i][1], mm[i][0]))
        pads = [{"pin": nums[i], "x": mm[i][0] + 0.0, "y": mm[i][1] + 0.0, "width": mm[i][2],
                 "height": mm[i][3], "shape": "rect"} for i in order]
        nt = note(spec, s)
        objs.append({"id": spec["id"], "description": describe(spec, mm, nums), "note": nt,
                     "style": "explicit", "pins": max(nums), "body": {"width": spec["W"], "height": spec["L"]},
                     "pads": pads})
        fig = nt.split(": pads")[0]
        if spec["mpns"]:
            for r in cand:
                if r["mpn"].startswith(spec["mpns"]):
                    mpn_rows.append((r["mpn"], spec["code"], spec["id"], fig))
        else:
            codes[spec["code"]] = (spec["id"], fig)
    if diag:
        return
    with open(os.path.join(DATA, "footprints-E.json"), "w") as out:
        json.dump({"footprints": objs}, out, indent=1)
    path = os.path.join(DATA, "package-codes.tsv")
    pc = list(csv.DictReader(open(path), delimiter="\t"))
    cols = list(pc[0].keys())
    for r in pc:
        if r["geometry"] == "module" and r["code"] in codes:
            r["footprint"], r["drawing"] = codes[r["code"]]
            r["package"] = "Module"
    with open(path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in pc:
            out.write("\t".join(r.get(c, "") for c in cols) + "\n")
    with open(os.path.join(DATA, "footprint-mpn.tsv"), "w") as out:
        out.write("mpn\tcode\tfootprint\tdrawing\n")
        for row in sorted(mpn_rows):
            out.write("\t".join(row) + "\n")
    print(len(objs), "footprints,", len(codes), "codes,", len(mpn_rows), "MPN rows")


if __name__ == "__main__":
    main(sys.argv[1:])
