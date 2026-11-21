#!/usr/bin/env python3
"""plan-42-C: the one group the second refill left (MIC29152 in TO-252-5), read by hand.

Writes specs-pins-r3-01.tsv / .evidence.tsv and pinout-count-overrides-r3-01.tsv.
Table 3-2 of MIC2915x-30x-50x-75x-...-DS20005685B.pdf (p.22-23): adjustable
TO-252 pins 1 ENABLE, 2 INPUT, 3 and TAB GND, 4 OUTPUT, 5 ADJUST; reference
voltage 1.240 V typical (electrical characteristics); 2.3 V <= VIN <= 26 V.
"""
import csv
import json
import os

DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PDF = ("datasheets/regulator/microchip/MIC2915x-30x-50x-75x-High-Current-Low-Dropout-"
       "Regulators-DS20005685B.pdf")
PKG = "TO-252-6, DPAK (5 Leads + Tab)"
cols = ["pdf", "package", "series", "pinout", "pinout_adj", "regulators", "regulators_adj", "analog",
        "analog_adj", "vref", "vin_min", "dropout", "adjust_to"]
pinout = [[1, "ENABLE", "input"], [2, "INPUT", "power"], [3, "GND", "ground"], [4, "OUTPUT", "output"],
          [5, "ADJUST", "input"]]
regs = [{"type": "linear", "in": "INPUT", "out": "OUTPUT", "enable": "ENABLE", "adjust": "ADJUST"}]
row = [PDF, PKG, "-", json.dumps(pinout, separators=(",", ":")), "", json.dumps(regs, separators=(",", ":")), "",
       json.dumps(["ADJUST"]), "", "1.24", "2.3", "", "ground"]


def w(name, header, lines):
    with open(os.path.join(DATA, name), "w") as f:
        wr = csv.writer(f, delimiter="\t", lineterminator="\n")
        wr.writerow(header)
        for line in lines:
            wr.writerow(line)


w("specs-pins-r3-01.tsv", cols, [row])
w("specs-pins-r3-01.evidence.tsv", ["pdf", "package", "series", "page", "note"],
  [[PDF, PKG, "-", "22-23", "Table 3-2 Pin Function Table: adjustable TO-252: 1 ENABLE (high = enable), 2 INPUT, "
    "3 GND (tab also GND), 4 OUTPUT, 5 ADJUST (divider from OUTPUT to GND); VREF 1.240 V typ; 2.3 V <= VIN <= 26 V (note 8)"]])
w("pinout-count-overrides-r3-01.tsv", ["pdf", "package", "count", "evidence"],
  [[PDF, PKG, "5", "DS20005685B Table 3-2: TO-252 pins 1, 2, 3 (+ TAB), 4, 5; the tab shares pin 3's name and is not numbered"]])
