"""plan-36-A Phase 2/3: check each row's package_code against DigiKey's package.

usage: python3 -I pkgcheck.py <cand.tsv> <txt-dir> [--all]

For every in-scope row with a package_code, the code's legend line in the
row's datasheet (textscan.legend) gives a pin count and a package type. It
must agree with DigiKey's "Package / Case": the same pin count, the same
geometry class (atdfmatch.geom) and the same package type (ptype: DIP, SO,
SSOP, TSSOP, QFP, QFN, BGA, ...). Prints one line per disagreement and a
count. DigiKey is wrong for some rows (B § 3.1), so a disagreement is read on
the datasheet page, not fixed blindly.
"""
import csv
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import atdfmatch as A  # noqa: E402
import textscan as T  # noqa: E402


TYPES = [("BGA", r"BGA|WLCSP|Ball Grid"), ("LCC", r"PLCC|J-Lead|Chip Carrier|\bLCC"),
         ("QFN", r"QFN|MLF|No[- ]Lead|DFN|Micro Lead"), ("QFP", r"QFP|Quad Flat Pack(?!age \(QFN)|Gull Wing Quad"),
         ("TSSOP", r"TSSOP"), ("SSOP", r"(?<!T)SSOP|Shrink Small"), ("DIP", r"DIP|Dual Inline|Dual In-line"),
         ("SOT", r"SOT"), ("SO", r"SOIC|\bSO\b|Small Outline")]


def ptype(text):
    """The package type a description names: BGA, LCC, QFN, QFP, TSSOP, SSOP, DIP, SOT or SO."""
    for name, rx in TYPES:
        if re.search(rx, text or "", re.I):
            return name
    return None


def main(tsv, txt_dir, show_all=False):
    legends = {}
    bad = 0
    n = 0
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["scope"] != "in" or not r["package_code"] or r["family"] in ("ESP32", "RP2040"):
            continue
        h = r["pdf"]
        if h not in legends:
            p = os.path.join(txt_dir, h + ".txt")
            legends[h] = T.legend(open(p, errors="replace").read()) if os.path.exists(p) else {}
        code = r["package_code"]
        desc = legends[h].get(code, "")
        pins = T.legend_pins(desc, code)
        dk = T.dk_pins(r)
        g1, g2 = A.geom(desc) if desc else None, A.geom(r["dk_package"])
        n += 1
        why = []
        if pins != dk:
            why.append(f"pins {pins} vs {dk}")
        if desc and g1 and g1 != g2:
            why.append(f"geometry {g1} vs {g2}")
        t1, t2 = ptype(desc), ptype(r["dk_package"])
        if desc and t1 and t2 and t1 != t2:
            why.append(f"type {t1} vs {t2}")
        if why or show_all:
            bad += bool(why)
            print(f"{r['mpn']}\t{code}\t{'; '.join(why)}\t{desc[:70]}\t{r['dk_package']}")
    print(f"{n} rows with a package code, {bad} disagree with DigiKey")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], "--all" in sys.argv)
