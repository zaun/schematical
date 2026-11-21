#!/usr/bin/env python3
"""plan-42-C: verdict-overrides.tsv for the spec rows whose output cannot be modelled.

usage: python3 -I pin-overrides.py

Writes planning/plan-42-data/verdict-overrides.tsv (mpn, verdict, reason; read
by verdicts.py) with one row per download-set row of a spec row listed in
EXCLUDED below, and blanks the pinout columns of those spec rows in specs.tsv
(they hold no data the model could use). Reasons:

  no pin numbers      the datasheet's 8-pin drawing is a photograph without pin
                      numbers and Microchip's later revisions number it in
                      opposite orders (LR12, plan-42-C Corrections)
  digital-interface   the output is set or the part is controlled over a serial
                      interface (I2C); plan-42-A section 3 rule 2 leaves such
                      parts out
  output not set by a feedback divider
                      the output is set by a resistor or a strap on a SET / SEL
                      / RSEL / TRIM / VC pin (a current source or a lookup
                      table), so there is no feedback pin and no Vref to model;
                      the regulator kind models Vref x (1 + Rtop / Rbottom)
                      only (plan-42-G section 3.3)
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

DIGITAL = "digital-interface"
SETPIN = "output not set by a feedback divider"
NOPINS = "no pin numbers in the datasheet (its pin drawing is a photo; later revisions number it both ways)"
# (maker, pdf file name, package or None for every package) -> (verdict, reason)
EXCLUDED = [
    ("Analog Devices", "max77675.pdf", None, DIGITAL),
    ("Analog Devices", "MAX77827.pdf", None, SETPIN),
    ("Analog Devices", "MAX77839.pdf", None, SETPIN),
    ("Analog Devices", "lt3080.pdf", None, SETPIN),
    ("Analog Devices", "datasheet-e9e355.pdf", None, SETPIN),
    ("Analog Devices", "MAX38902A-MAX38902D.pdf", "8-WFDFN Exposed Pad", SETPIN),
    ("Analog Devices", "MAX17220-MAX17225.pdf", None, SETPIN),
    ("Analog Devices", "MAX17227A.pdf", None, SETPIN),
    ("Analog Devices", "MAX17227J.pdf", None, SETPIN),
    ("Texas Instruments", "lm3370.pdf", None, DIGITAL),
    ("Microchip Technology", "Microsemi_LX7219_Datasheet_V2.pdf", None, DIGITAL),
    ("Microchip Technology", "MIC2141.pdf", None, SETPIN),
    ("Microchip Technology", "20005384A.pdf", None, SETPIN),
    ("Microchip Technology", "LR12-C080113.pdf", "8-SOIC (0.154\", 3.90mm Width)", NOPINS),
]
BLANK = specs.COLS[8:]


def main():
    rows = common.load(specs.CANDIDATES)
    common.init(rows)
    out = []
    keys = set()
    for r in rows:
        if not common.eligible(r) or not r["pdf"]:
            continue
        for maker, name, package, reason in EXCLUDED:
            if r["maker"] == maker and os.path.basename(r["pdf"]) == name and package in (None, r["package"]):
                out.append((r["mpn"], "out" if reason == DIGITAL else "skip", reason))
                keys.add(specs.key(r))
    with open(os.path.join(common.DATA, "verdict-overrides.tsv"), "w") as f:
        f.write("mpn\tverdict\treason\n")
        for line in sorted(out):
            f.write("\t".join(line) + "\n")
    table = list(csv.DictReader(open(specs.SPECS), delimiter="\t"))
    for s in table:
        if specs.key(s) in keys:
            for c in BLANK:
                s[c] = ""
    specs.write(table)
    print(len(out), "download-set rows overridden in", len(keys), "spec rows")


if __name__ == "__main__":
    main()
