"""plan-36-C/D: the package drawings of the add rows' datasheets, as numbers.

usage: python3 -I drawings.py <candidates.tsv> <out drawings.json>

Reads each datasheet's "Packaging Information" / "Package Drawings" pages
(pdftotext -layout) and records one entry per drawing page:
  pdf, page, title (the drawing's title line), codes (package codes the
  page names: Atmel `8S2` / `28P3` headings and title-block codes, Microchip
  `(SS)` title codes), drawing (`C04-073`, `C04-2073`, Atmel `8S2`),
  dims  {symbol: [min, nom, max]} from a `Dimension Limits` (Microchip) or
        `COMMON DIMENSIONS` (Atmel) table, millimetres
  land  {symbol: value} from a `RECOMMENDED LAND PATTERN` table (C, C1, C2,
        E, X1, Y1, X2, Y2, G1, ...)
  pins  the table's `N` / `Number of Pins` (when printed)
fp-codes.py and fp-make.py pick the drawing of each package code from these.
"""
import csv
import json
import os
import re
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
NUM = r"-?\d+(?:\.\d+)?"


def nums(s):
    return [float(x) for x in re.findall(r"(?<![\w.])" + NUM + r"(?![\w.])", s)]


def parse_page(text):
    d = {"title": "", "codes": [], "drawing": "", "dims": {}, "land": {}, "pins": None}
    m = re.search(r"^\s*(\d+)[- ](?:Lead|Pad|Ball|Pin|Terminal)[^\n]*", text, re.M | re.I)
    if m:
        d["title"] = re.sub(r"\s+", " ", m.group(0).strip())
    d["codes"] = sorted(set(re.findall(r"\((\w{1,4})\)\s*[-–]", d["title"])))
    for m in re.finditer(r"^\s*(?:\d+(?:\.\d+)*\s+)?([0-9]{1,3}[A-Z][A-Z0-9-]{0,6})\s*$", text, re.M):
        d["codes"].append(m.group(1))
    for m in re.finditer(r"DRAWING NO\.[^\n]*\n[^\n]*?\b([0-9]{1,3}[A-Z][A-Z0-9-]{0,6})\b", text):
        d["codes"].append(m.group(1))
    m = re.search(r"Microchip Technology Drawing\s+(C04-\w+)", text)
    if m:
        d["drawing"] = m.group(1)
    land = "RECOMMENDED LAND PATTERN" in text.upper()
    cols = None  # x centres of the MIN / NOM / MAX header
    land_on = False
    for line in text.split("\n"):
        h = re.search(r"\bMIN\b\s+\bNOM\b\s+\bMAX\b", line)
        if h:
            cols = [line.index("MIN") + 1.5, line.index("NOM") + 1.5, line.index("MAX") + 1.5]
            nt = line.find("NOTE")
            cols.append(nt + 2 if nt > 0 else 10 ** 6)
            continue
        if "RECOMMENDED LAND PATTERN" in line.upper():
            land_on = True
        # `Name ... SYM  values` (Microchip) or `   SYM  values` (Atmel)
        # anything left of the symbol (a row name, or a note / drawing label that
        # shares the line: `rotrusion. Allowable    B   0.30  –  0.45`)
        m = re.match(r"^(?:(.*?)\s{2,}|\s*)([A-Za-z]\d?)\s+(-?\d.*|[-–]\s.*)$", line)
        if not m or cols is None:
            continue
        name, sym, rest = (m.group(1) or ""), m.group(2), m.group(3)
        if re.search(r"Units|Dimension Limits", line):
            continue
        start = line.index(rest)
        vals = [None, None, None]
        found = False
        for v in re.finditer(r"(?<![\w.])" + NUM + r"(?![\w.])", rest):
            x = start + (v.start() + v.end()) / 2
            k = min(range(4), key=lambda i: abs(cols[i] - x))
            if k < 3:
                vals[k] = float(v.group(0))
                found = True
        if not found:
            continue
        if "Number of" in name or (sym == "N" and not name):
            d["pins"] = int([v for v in vals if v is not None][0])
            continue
        is_land = land and land_on and re.search(r"Contact|Pad|Pitch|Spac|Width|Length|Grid", name, re.I) \
            and not re.search(r"Overall|Molded|Lead|Standoff|Thickness|Foot|Terminal", name)
        if is_land:
            d["land"][sym] = [v for v in vals if v is not None][-1]
        else:
            d["dims"].setdefault(sym, vals)
    d["codes"] = sorted(set(d["codes"]))
    return d


def main(tsv, out):
    pdfs = sorted({r["pdf"] for r in csv.DictReader(open(tsv), delimiter="\t") if r["verdict"] == "add"})
    result = []
    for pdf in pdfs:
        txt = subprocess.run(["pdftotext", "-q", "-layout", os.path.join(ROOT, pdf), "-"],
                             capture_output=True, text=True, timeout=600).stdout
        pages = txt.split("\f")
        start = None
        for i, p in enumerate(pages):
            if re.search(r"Packaging Information|Package Drawings|Packaging Diagrams|Package Information|Mechanical Data|Package Outline",
                         p, re.I) and p.count("....") < 5 and i > 3:
                start = i if start is None else start
        if start is None:
            continue
        for i in range(start, len(pages)):
            p = pages[i]
            if not re.search(r"DIMENSION|Dimension Limits|LAND PATTERN|COMMON DIMENSIONS", p, re.I):
                continue
            d = parse_page(p)
            d.update(pdf=pdf, page=i + 1)
            if d["dims"] or d["land"]:
                result.append(d)
    json.dump(result, open(out, "w"), indent=0)
    print(len(pdfs), "PDFs,", len(result), "drawing pages")


if __name__ == "__main__":
    main(*sys.argv[1:3])
