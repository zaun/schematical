"""plan-36-B Phase 1 (§ 3.1): package-codes.tsv from the add rows' package codes.

usage: python3 -I package-codes.py <candidates.tsv> <txt-dir> <out.tsv>

One row per (code, pins): the newer Microchip codes are per pin count (`PT`
is the 32-, 48- and 64-pin TQFP; `SS` is SSOP in AVR Dx and SOIC150 in
tinyAVR, at different pin counts), so the pair is the key. Columns:

  code      the package code (A's `package_code`)
  geometry  dual / quad / drqfn / plcc / grid / module (B § 3.1)
  pins      the package's lead count (the module: its device; RP2040: 56)
  name      the datasheet's description of the code (its Package Type legend
            or Product Identification figure), else DigiKey's package
  drawing   (C-E) PDF:page of the drawing
  footprint (C-E) footprint id
  rows      how many add rows use it; families

Pins: the classic codes lead with their pin count (`32A`), checked against
DigiKey; the newer AVRs carry it in the device name (`AVR128DA28` = 28,
DigiKey lists AVR16EB28T-E/STX as 14-TSSOP), tinyAVR / megaAVR 0 take
DigiKey's count, which agrees with the device digit for every row.
Geometry comes from the description (pkgcheck.ptype) and DigiKey's package.
"""
import csv
import os
import re
import sys
from collections import OrderedDict, Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pkgcheck as P  # noqa: E402
import textscan as T  # noqa: E402

FIG = {}


def figure_names(spec_path):
    """code -> description from specs.tsv notes (`MR=VQFN64; 6LX=VQFN48; ...`)."""
    out = {}
    for r in csv.DictReader(open(spec_path), delimiter="\t"):
        for m in re.finditer(r"(?:^|; |: )([0-9A-Z]{1,4})=([^;]+)", r["note"]):
            out.setdefault((r["pdf"], m.group(1)), m.group(2).strip())
    return out


def pins_of(r):
    m = re.match(r"AVR\d+[A-Z]{2}(\d+)$", r["device"])
    if m:
        return int(m.group(1))
    m = re.match(r"(\d+)", r["dk_package"])
    if m:
        return int(m.group(1))
    if "SOT-23-6" in r["dk_package"]:
        return 6
    return None


def geometry(desc, dk):
    text = f"{desc} {dk}"
    if re.search(r"Dual Rows|2-row|DRQFN", text, re.I):
        return "drqfn"
    if re.search(r"DFN|Dual Flat", text, re.I):
        return "dual"  # leadless, two sides (DFN / UDFN / VDFN, MLF10)
    t = P.ptype(desc) or P.ptype(dk)
    return {"BGA": "grid", "LCC": "plcc", "QFP": "quad", "QFN": "quad", "TSSOP": "dual", "SSOP": "dual",
            "DIP": "dual", "SOT": "dual", "SO": "dual"}.get(t, "?")


def main(tsv, txt_dir, out_path):
    here = os.path.dirname(os.path.abspath(tsv))
    fig = figure_names(os.path.join(here, "specs.tsv"))
    index = {}  # tree path -> md5s of its URLs (the txt files are named by md5)
    for r in csv.DictReader(open(os.path.join(here, "..", "..", "datasheets", "mcu", "datasheets.tsv")), delimiter="\t"):
        index.setdefault(r["path"], set()).add(T.md5(r["url"]))
    legends = {}
    table = OrderedDict()
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["verdict"] != "add":
            continue
        code = r["package_code"]
        if r["family"] == "ESP32":
            key = (code, "module")
        elif r["family"] == "RP2040":
            key = (code, 56)
        else:
            key = (code, pins_of(r))
        desc = ""
        for h in sorted(index.get(r["pdf"], ())):
            if h not in legends:
                p = os.path.join(txt_dir, h + ".txt")
                legends[h] = T.legend(T.dashes(open(p, errors="replace").read())) if os.path.exists(p) else {}
            got = legends[h].get(code, "") or fig.get((h, code), "")
            desc = got if got and got != "ordering table" else desc
        e = table.setdefault(key, {"names": Counter(), "dk": Counter(), "fam": Counter(), "n": 0})
        if desc:
            e["names"][desc] += 1
        e["dk"][r["dk_package"]] += 1
        e["fam"][r["family"]] += 1
        e["n"] += 1
    with open(out_path, "w") as out:
        out.write("code\tgeometry\tpins\tname\tdrawing\tfootprint\trows\n")
        for (code, pins), e in sorted(table.items(), key=lambda kv: (str(kv[0][0]), str(kv[0][1]))):
            name = e["names"].most_common(1)[0][0] if e["names"] else e["dk"].most_common(1)[0][0]
            dk = e["dk"].most_common(1)[0][0]
            if pins == "module":
                g, pins = "module", ""
            else:
                g = geometry(name, dk)
            fams = ",".join(sorted(e["fam"]))
            out.write(f"{code}\t{g}\t{pins}\t{name}\t\t\t{e['n']} {fams}\n")
    print(len(table), "package codes")


if __name__ == "__main__":
    main(*sys.argv[1:4])
