#!/usr/bin/env python3
"""plan-42-C: turn the two 8-SOIC groups whose parts variant-overrides.tsv marks
`adj` (MIC2954-08, MIC29204: output 1.24 V to 29 V by two resistors, pin-strapped
for 5 V) into adjustable descriptions: the `adjust` pin of the one element is
the datasheet's feedback / ADJUST pin, Vref 1.235 V typ, Vout = Vref (1 + R1/R2)
with the divider tap at the pin (ground-referenced).

usage: python3 -I variant-fix.py
"""
import csv
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

FIX = {
    ("mic2954.pdf", "8-SOIC (0.154\", 3.90mm Width)"): "FB",
    ("MIC2920.pdf", "8-SOIC (0.154\", 3.90mm Width)"): "ADJUST",
}


def main():
    rows = list(csv.DictReader(open(specs.SPECS), delimiter="\t"))
    n = 0
    for s in rows:
        pin = FIX.get((os.path.basename(s["pdf"]), s["package"]))
        if not pin or s["maker"] != "Microchip Technology":
            continue
        regs = json.loads(s["regulators"])
        regs[0]["adjust"] = pin
        s["regulators"] = json.dumps(regs, separators=(",", ":"))
        s["vref"] = "1.235"
        s["adjust_to"] = "ground"
        n += 1
    specs.write(rows)
    print(n, "spec rows made adjustable")


if __name__ == "__main__":
    main()
