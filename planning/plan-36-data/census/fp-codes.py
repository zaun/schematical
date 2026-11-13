"""plan-36-C/D Phase 1: the drawing page(s) of each package code.

usage: python3 -I fp-codes.py <candidates.tsv> <drawings.json> [--geometry dual|quad,drqfn,plcc,grid] > fp-pages.tsv

For each package-codes.tsv row of the asked geometries (dual by default),
looks through the drawings (drawings.py) of the PDFs its add rows use for
the code's drawing:
  - a classic Atmel code (`8S2`, `28P3`) whose page names the code;
  - a newer Microchip code (`SS`, `RXB`, `PT`) by its title `(SS)` and the
    pin count, or (tinyAVR / megaAVR 0 letters, which are not drawing codes:
    `SS` = SOIC150, `S` = SOIC300, `X` = TSSOP/SSOP, `M` = VQFN) by the
    title's pin count, package type and body width;
and prefers a PDF that has the recommended land pattern too. Columns:
`code geometry pins pdf page land_page drawing title`. A code with no
drawing found has an empty page (Phase 1's check).
"""
import csv
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pinouts as P  # noqa: E402
import pkgcheck as K  # noqa: E402

WIDTH = {"SOIC150": (r"3\.90|Narrow|150", r"7\.50|Wide|300|5\.30|208"),
         "SOIC300": (r"7\.50|Wide|300", r"3\.90|Narrow|150")}


def norm(drawing):
    """C04-00087 / C04-087 / C04-087B -> C04-87."""
    m = re.match(r"C04-0*(\d+)", drawing or "")
    return f"C04-{m.group(1)}" if m else None


def tpins(title):
    m = re.match(r"(\d+)", title or "")
    return int(m.group(1)) if m else None


def main(argv):
    tsv, dpath = argv[0], argv[1]
    geoms = argv[argv.index("--geometry") + 1].split(",") if "--geometry" in argv else ["dual"]
    here = os.path.dirname(os.path.abspath(tsv))
    drawings = json.load(open(dpath))
    by_pdf = {}
    for d in drawings:
        by_pdf.setdefault(d["pdf"], []).append(d)
    codes = [r for r in csv.DictReader(open(os.path.join(here, "package-codes.tsv")), delimiter="\t") if r["geometry"] in geoms]
    pdfs = {}
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["verdict"] == "add" and r["family"] not in ("ESP32",):
            key = (r["package_code"], P.pins_of(r) if r["family"] != "RP2040" else "56")
            pdfs.setdefault(key, set()).add(r["pdf"])
    numbers = {}
    np = os.path.join(here, "drawing-numbers.tsv")
    if os.path.exists(np):
        for t in csv.DictReader(open(np), delimiter="\t"):
            numbers.setdefault(t["pdf"], []).append(t)
    hand = {}
    hp = os.path.join(here, "fp-pages-hand.tsv")
    if os.path.exists(hp):
        # drawings read by eye (image pages) or found only through a drawing-number table
        for t in csv.DictReader(open(hp), delimiter="\t"):
            hand[(t["code"], t["pins"])] = t
    print("code\tgeometry\tpins\tpdf\tpage\tland_page\tdrawing\ttitle")
    for c in codes:
        n = int(c["pins"])
        want_type = K.ptype(c["name"])
        best = None
        for pdf in sorted(pdfs.get((c["code"], c["pins"]), ())):
            ds = by_pdf.get(pdf, [])
            for i, d in enumerate(ds):
                score = 0
                if c["code"] in d["codes"] and re.match(r"\d", c["code"]):
                    score = 3
                elif f"({c['code']})" in d["title"] and tpins(d["title"]) == n:
                    score = 3
                elif tpins(d["title"]) == n and want_type and K.ptype(d["title"]) == want_type:
                    score = 1
                    for key, (yes, no) in WIDTH.items():
                        if key in c["name"]:
                            if re.search(yes, d["title"]) and not re.search(no, d["title"]):
                                score = 2
                            elif re.search(no, d["title"]):
                                score = 0
                if not score or not d["dims"]:
                    continue
                # the land-pattern page: the next pages with the same title and a land table
                land = next((e for e in ds[i + 1:i + 3] if e["land"] and e["title"] == d["title"]), None)
                if d["land"]:
                    land = d
                key = (score, land is not None)
                if best is None or key > best[0]:
                    best = (key, pdf, d, land)
        if best is None:
            # the datasheet's drawing-number table names the Microchip drawing
            # (C04-00087); take that drawing where another datasheet prints it as text
            for pdf in sorted(pdfs.get((c["code"], c["pins"]), ())):
                for t in numbers.get(pdf, []):
                    if int(t["pins"]) != n or K.ptype(t["type"]) not in {K.ptype(x) for x in c["name"].split(",")}:
                        continue
                    num = norm(t["drawing"])
                    for i, d in enumerate(drawings):
                        if norm(d["drawing"]) == num and d["dims"]:
                            same = [e for e in drawings if e["pdf"] == d["pdf"] and e["title"] == d["title"] and e["land"]]
                            best = ((0, bool(same)), d["pdf"], d, same[0] if same else None)
                            break
                    if best:
                        break
                if best:
                    break
        if best is None and (c["code"], c["pins"]) in hand:
            t = hand[(c["code"], c["pins"])]
            print("\t".join(t[k] for k in ("code", "geometry", "pins", "pdf", "page", "land_page", "drawing", "title")))
            continue
        if best is None:
            print(f"{c['code']}\t{c['geometry']}\t{n}\t\t\t\t\t")
            continue
        _, pdf, d, land = best
        print(f"{c['code']}\t{c['geometry']}\t{n}\t{pdf}\t{d['page']}\t{land['page'] if land else ''}\t"
              f"{d['drawing'] or ','.join(d['codes'])}\t{d['title']}")


if __name__ == "__main__":
    main(sys.argv[1:])
