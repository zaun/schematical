#!/usr/bin/env python3
"""plan-42-D/E/F Phase 2: the hand-check items, one object per package family.

usage: python3 -I fp-verify-packets.py <out dir> [<objects per family>]

Per letter, the new objects (footprints-<L>.json) are grouped into families by
their id (SOT / TSOT, SC70, SOIC, MSOP / VSSOP, TSSOP, SSOP, TO-220, TO-92,
TO-263, DPAK / TO-252, SPAK, SOT-223, PDIP, ball arrays, QFN, DFN / SON /
WSON / MLF / LFCSP, X2SON). One object per family (two when asked, from
different makers) goes into <out dir>/<L>.txt with its JSON, its note and its
evidence rows (footprints-<L>-<packet>.evidence.tsv).
"""
import collections
import csv
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)

FAMILIES = [
    ("SOT-223", r"^SOT223"), ("SOT-89", r"^SOT89"), ("SOT-563/583", r"^SOT5[68]3"),
    ("SOT-23 / TSOT", r"^(TSOT|SOT23|SOT-23)"), ("SC-70", r"^SC70"),
    ("S-PAK", r"^SPAK"), ("DPAK", r"^(DPAK|TO-252)"), ("D2PAK / DDPAK", r"^TO-263"),
    ("TO-220", r"^TO-220"), ("TO-92", r"^TO-92"), ("DIP", r"^(PDIP|DIP)"),
    ("SOIC / SOP", r"^(SOIC|SOP)"), ("MSOP / VSSOP", r"^(MSOP|VSSOP|HVSSOP)"),
    ("TSSOP / SSOP", r"^(TSSOP|HTSSOP|SSOP)"),
    ("ball array", r"^(DSBGA|WLCSP|WLP|WCSP|UFBGA)"),
    ("QFN", r"^(QFN|LQFN|TQFN|VQFN|FQFN|FTQFN)"),
    ("X2SON", r"^X2SON"),
    ("DFN / SON / MLF", r"^(DFN|TDFN|UDFN|WSON|VSON|SON|MLF|LFCSP|UQFN|FC2QFN)"),
]


def family(i):
    for name, rx in FAMILIES:
        if re.match(rx, i):
            return name
    return "other"


def main(out, per):
    os.makedirs(out, exist_ok=True)
    for L in "DEF":
        objs = json.load(open(os.path.join(DATA, f"footprints-{L}.json")))["footprints"]
        ev = collections.defaultdict(list)
        for path in glob.glob(os.path.join(DATA, f"footprints-{L}-*.evidence.tsv")):
            for r in csv.DictReader(open(path), delimiter="\t"):
                ev[r["footprint"]].append(f"{r['pdf']} p.{r['page']}: {r['note']}")
        fam = collections.defaultdict(list)
        for o in objs:
            fam[family(o["id"])].append(o)
        with open(os.path.join(out, L + ".txt"), "w") as f:
            n = 0
            for name in sorted(fam):
                lst = fam[name]
                step = max(1, len(lst) // per)
                chosen = [("", o) for o in lst[::step][:per]]
                for _, o in chosen:
                    n += 1
                    f.write(f"### ITEM {L}-{n:02d} family={name} id={o['id']}\n")
                    f.write("  object: " + json.dumps(o, ensure_ascii=False, separators=(",", ":")) + "\n")
                    for e in ev.get(o["id"], [])[:3]:
                        f.write("  evidence: " + e[:600] + "\n")
                    f.write("\n")
            print(L, n, "items from", len(fam), "families,", len(objs), "objects")


if __name__ == "__main__":
    main(sys.argv[1], int(sys.argv[2]) if len(sys.argv) > 2 else 1)
