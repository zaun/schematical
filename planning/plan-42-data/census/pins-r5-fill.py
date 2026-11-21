#!/usr/bin/env python3
"""plan-42-C: the group the verdict override of TL2575-ADJIKV left (TPS62020, adjustable, 10-PowerTFSOP).

The DGQ package of slvs076c.pdf already had its fixed variant (TPS62026); the
adjustable TPS62020DGQ has the same pin table (slvs076c.pdf "Pin Assignments",
10-pin MSOP PowerPAD: 1 EN, 2 / 3 VIN, 4 GND, 5 FB, 6 MODE, 7 / 8 SW, 9 / 10 PGND)
with FB as the feedback pin (the adjustable VFDFN row of the same PDF has Vref
0.5 V, ground-referenced). Writes specs-pins-r5-01.tsv / .evidence.tsv.
"""
import csv
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

DATA = specs.os.path.dirname(specs.SPECS)
PDF = "datasheets/regulator/ti/slvs076c.pdf"
PKG = "10-PowerTFSOP, 10-MSOP (0.118\", 3.00mm Width)"
table = specs.load()
row = next(s for k, s in table.items() if k[2] == PDF and k[3] == PKG)
regs = json.loads(row["regulators"])
regs_adj = json.loads(json.dumps(regs))
regs_adj[0]["adjust"] = "FB"
cols = ["pdf", "package", "series", "pinout", "pinout_adj", "regulators", "regulators_adj", "analog",
        "analog_adj", "vref", "vin_min", "dropout", "adjust_to"]
out = [PDF, PKG, "-", row["pinout"], row["pinout"], row["regulators"],
       json.dumps(regs_adj, separators=(",", ":")), row["analog"], row["analog"], "0.5", row["vin_min"],
       row["dropout"], "ground"]
with open(os.path.join(DATA, "specs-pins-r5-01.tsv"), "w") as f:
    w = csv.writer(f, delimiter="\t", lineterminator="\n")
    w.writerow(cols)
    w.writerow(out)
with open(os.path.join(DATA, "specs-pins-r5-01.evidence.tsv"), "w") as f:
    w = csv.writer(f, delimiter="\t", lineterminator="\n")
    w.writerow(["pdf", "package", "series", "page", "note"])
    w.writerow([PDF, PKG, "-", "3", "hand-filled (refill): the DGQ package pin table is shared by the fixed TPS62026 and the "
                "adjustable TPS62020 (FB = feedback; the DRC variant of the same PDF is Vref 0.5 V, ground-referenced)"])
