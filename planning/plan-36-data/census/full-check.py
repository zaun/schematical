"""plan-36-B Phase 3 (§ 3.4): the ten full checks, pin by pin against the figure.

usage: python3 -I full-check.py <candidates.tsv> > full-checks-auto.tsv

For each (device, pinout) of FULL, every figure of that pin count in the
device's datasheets (the add rows' PDFs) is read pin by pin with
transcribe.figure_pins (number -> printed pad name, from the PDF's word
boxes; supply names compared as pinouts.supply prints them, aliases as
pinout-check.ALIAS). The pinout passes when one figure of its geometry
agrees at every pin: `full ok (figure p<page>)`, else the differing pins.
A figure the word boxes cannot read is checked by eye and written to
full-checks-eye.tsv by hand.
"""
import csv
import importlib.util
import os
import sys
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import pinouts as P  # noqa: E402
import transcribe as T  # noqa: E402

spec = importlib.util.spec_from_file_location("pinout_check", os.path.join(HERE, "pinout-check.py"))
PC = importlib.util.module_from_spec(spec)
spec.loader.exec_module(PC)

FULL = [("ATmega328P", "p28"), ("ATmega328P", "quad32"), ("ATmega2560", "quad100"), ("ATmega32U4", "quad44"),
        ("ATmega1284P", "p40"), ("ATmega1284P", "quad44"), ("ATmega4809", "quad48"), ("ATtiny85", "p8"),
        ("ATtiny13A", "p8"), ("ATtiny84A", "p14"), ("ATtiny1614", "p14"), ("AVR128DA48", "quad48")]
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))


def same(atdf, printed):
    if printed is None:
        return False
    if atdf == printed or P.supply(atdf) == printed:
        return True
    return printed in PC.ALIAS.get(atdf, ())


def main(tsv):
    here = os.path.dirname(os.path.abspath(tsv))
    pins = defaultdict(list)
    for r in csv.DictReader(open(os.path.join(here, "pinouts.tsv")), delimiter="\t"):
        pins[(r["device"], r["pinout"])].append(r["name"])
    pdfs = defaultdict(set)
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["verdict"] == "add":
            dev = r["atdf_device"][:-5] if r["atdf_device"].endswith(".atdf") else P.proper(r["device"])
            pdfs[dev].add(r["pdf"])
    print("device\tpinout\tpdf\tpages\tresult")
    for dev, po in FULL:
        want = pins[(dev, po)]
        n = len(want)
        geometry = "dual" if po.startswith("p") else "quad"
        best = None
        for pdf in sorted(pdfs[dev]):
            pages, _ = T.pin_pages(os.path.join(ROOT, pdf))
            for p in pages[:12]:
                for g, got, _c in T.figure_pins(T.page_words(os.path.join(ROOT, pdf), p), n):
                    if not T.layout_ok(g, geometry, n):
                        continue
                    seq = [T.pad_name(got[k][4]) for k in range(1, n + 1)]
                    diff = [(k, want[k - 1], seq[k - 1]) for k in range(1, n + 1) if not same(want[k - 1], seq[k - 1])]
                    res = (pdf, str(p), "full ok (pin by pin against the figure)" if not diff else
                           "differs: " + " ".join(f"{k}:{a}/{b}" for k, a, b in diff))
                    if best is None or not diff:
                        best = res
                    if not diff:
                        break
                if best and best[2].startswith("full ok"):
                    break
            if best and best[2].startswith("full ok"):
                break
        if best is None:
            best = ("", "", "no figure read (check by eye)")
        print(f"{dev}\t{po}\t{best[0]}\t{best[1]}\t{best[2]}")


if __name__ == "__main__":
    main(sys.argv[1])
