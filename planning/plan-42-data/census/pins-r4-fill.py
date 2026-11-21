#!/usr/bin/env python3
"""plan-42-C: the group the third refill left (TPS71918, dual LDO, 6-WSON DRV), read by hand.

Writes specs-pins-r4-01.tsv / .evidence.tsv. sbvs088c.pdf "Pin Descriptions"
(DRV column): 1 OUT1, 2 IN (both regulators), 3 OUT2, 4 EN2, 5 GND (the
thermal pad is also ground, not a pin), 6 EN1; ENx high turns regulator x on;
VIN 2.7 V to 6.5 V; dropout 230 mV typ / 400 mV max at 200 mA.
"""
import csv
import json
import os

DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PDF = "datasheets/regulator/ti/sbvs088c.pdf"
PKG = "6-WDFN Exposed Pad"
cols = ["pdf", "package", "series", "pinout", "pinout_adj", "regulators", "regulators_adj", "analog",
        "analog_adj", "vref", "vin_min", "dropout", "adjust_to"]
pinout = [[1, "OUT1", "output"], [2, "IN", "power"], [3, "OUT2", "output"], [4, "EN2", "input"],
          [5, "GND", "ground"], [6, "EN1", "input"]]
regs = [{"type": "linear", "in": "IN", "out": "OUT1", "enable": "EN1"},
        {"type": "linear", "in": "IN", "out": "OUT2", "enable": "EN2"}]
row = [PDF, PKG, "-", json.dumps(pinout, separators=(",", ":")), "", json.dumps(regs, separators=(",", ":")), "",
       "[]", "", "", "2.7", "0.4", ""]
with open(os.path.join(DATA, "specs-pins-r4-01.tsv"), "w") as f:
    w = csv.writer(f, delimiter="\t", lineterminator="\n")
    w.writerow(cols)
    w.writerow(row)
with open(os.path.join(DATA, "specs-pins-r4-01.evidence.tsv"), "w") as f:
    w = csv.writer(f, delimiter="\t", lineterminator="\n")
    w.writerow(["pdf", "package", "series", "page", "note"])
    w.writerow([PDF, PKG, "-", "3-4", "Pin Descriptions (DRV column): 1 OUT1, 2 IN, 3 OUT2, 4 EN2, 5 GND, 6 EN1; EN high = on; "
                "VIN 2.7 to 6.5 V; dropout 400 mV max at 200 mA; hand-read, not by a packet agent"])
