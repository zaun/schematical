"""plan-36-D: the quad, dual-row QFN, PLCC and grid footprints, plus the RP2040.

usage: python3 -I fp-specs-D.py     writes footprints-D.json, fills package-codes.tsv
                                     (drawing, footprint, package) for D's rows and
                                     footprint-device.tsv (per-device footprints of split codes)

As fp-specs-C.py: each (code, pins) reuses an existing footprint of its
family (QFP: TQFP- / LQFP- / QFP-; QFN: QFN- / VQFN- / WQFN- / UQFN- /
TQFN- / HVQFN-; PLCC-; grid) when the pin count, pitch and spans agree and
every pad side (the centre pad included) is within 0.10 mm, else makes a
new one (`<family>-<pins>-MC-<code>`). Pad rules (plan-03-B § 3):
  R1  land pattern: spanX = C1, spanY = C2, pad Y1 x X1, centre X2 x Y2
  R2  gull wing: pad L(max)+0.8 x b(max)+0.05, span D(max)+1.0-pad length
  R3  J-lead: pad 2.0 x b(max)+0.1, span = E(nom) - 1.0
  R4  QFN: pad L+0.3 x b+0.05, span = D - L + 0.3, centre = exposed pad (nominal)
  R6  BGA: round pad 0.8 x the nominal ball, to 0.05
Values with no nominal take the mid of min and max (noted).
"""
import csv
import json
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.join(HERE, "..")
ROOT = os.path.abspath(os.path.join(DATA, "..", ".."))
M = "datasheets/mcu/microchip/"


def r1(c1, c2, y1, x1, x2=None, y2=None):
    return {"spanX": c1, "spanY": c2, "len": y1, "wid": x1, "centre": (x2, y2) if x2 else None}


def r2(lmax, bmax, dmax):
    pl = round(lmax + 0.8, 2)
    s = round(dmax + 1.0 - pl, 2)
    return {"spanX": s, "spanY": s, "len": pl, "wid": round(bmax + 0.05, 2), "centre": None}


def r3(bmax, enom):
    s = round(enom - 1.0, 3)
    return {"spanX": s, "spanY": s, "len": 2.0, "wid": round(bmax + 0.1, 2), "centre": None}


def r4(l, b, d, ep):
    s = round(d - l + 0.3, 2)
    return {"spanX": s, "spanY": s, "len": round(l + 0.3, 2), "wid": round(b + 0.05, 2),
            "centre": (ep, ep) if ep else None}


def r6(ball):
    return round(round(0.8 * ball / 0.05) * 0.05, 2)


# code, pins, kind, pitch, result, body, drawing, source, package, new id
QUAD = [
    # --- TQFP / LQFP
    ("32A", 32, "QFP", 0.8, r2(0.75, 0.45, 9.25), 7.0, "32A",
     M + "40001910A.pdf p.20 (L 0.45-0.75, B 0.30-0.45, D 8.75-9.25; body 7.00)", "TQFP", "TQFP-32-MC-32A"),
    ("PT", 32, "QFP", 0.8, r1(8.4, 8.4, 1.55, 0.55), 7.0, "C04-074 + C04-2074",
     M + "AVR16-32DD28-32-DataSheet-DS40002441.pdf p.670-671 (R1: C1 = C2 8.40, X 0.55, Y 1.55)", "TQFP", "TQFP-32-MC-PT"),
    ("A", 32, "QFP", 0.8, r1(8.4, 8.4, 1.55, 0.55), 7.0, "C04-074 + C04-2074",
     M + "ATmega3208-3209-Data-Sheet-DS40002174A.pdf p.526 (the same drawing; R1 as PT-32)", "TQFP", None),
    ("TQFP32", 32, "QFP", 0.8, r2(0.75, 0.45, 9.00), 7.0, "Atmel TQFP32 (JEDEC MS-026)",
     M + "7707S.pdf p.16 (image; L 0.45-0.75, b 0.30-0.45, D 9.00 BSC)", "TQFP", "TQFP-32-MC-TQFP32"),
    ("44A", 44, "QFP", 0.8, r2(0.75, 0.45, 12.25), 10.0, "44A",
     M + "2466S.pdf p.13 (L 0.45-0.75, B 0.30-0.45, D 11.75-12.25; body 10.00)", "TQFP", "TQFP-44-MC-44A"),
    ("44ML", 44, "QFP", 0.8, r2(0.75, 0.45, 12.25), 10.0, "ML",
     M + "Atmel-7766-8-bit-AVR-ATmega16U4-32U4_Summary.pdf p.18 (image; ML: the 44A numbers)", "TQFP", None),
    ("48AA", 48, "QFP", 0.5, r2(0.75, 0.27, 9.25), 7.0, "48AA",
     M + "Atmel-2548-8-bit-AVR-Microcontroller-Battery-Management-ATmega406_Datasheet-Summary.pdf p.18 (LQFP; L 0.45-0.75, B 0.17-0.27, D 8.75-9.25)",
     "TQFP", "LQFP-48-MC-48AA"),
    ("PT", 48, "QFP", 0.5, r1(8.4, 8.4, 1.5, 0.3), 7.0, "C04-300 + C04-2300",
     M + "AVR128DA28-32-48-64-Data-Sheet-DS40002183.pdf p.680-681 (R1: C 8.40, X1 0.30, Y1 1.50)", "TQFP", "TQFP-48-MC-PT"),
    ("A", 48, "QFP", 0.5, r1(8.4, 8.4, 1.5, 0.3), 7.0, "C04-300 + C04-2300",
     M + "ATmega3208-3209-Data-Sheet-DS40002174A.pdf p.534-535 (R1)", "TQFP", None),
    ("64A", 64, "QFP", 0.8, r2(0.75, 0.45, 16.25), 14.0, "64A",
     M + "2467S.pdf p.13 (L 0.45-0.75, B 0.30-0.45, D 15.75-16.25; body 14.00)", "TQFP", "TQFP-64-MC-64A"),
    ("A2", 64, "QFP", 0.8, r2(0.75, 0.45, 16.00), 14.0, "A2 (Atmel Nantes TQFP64)",
     M + "7679S.pdf p.14 (image; L 0.45-0.75, f 0.30-0.45, D 16.00 BSC)", "TQFP", "TQFP-64-MC-A2"),
    ("MD", 64, "QFP", 0.8, r2(0.75, 0.45, 16.00), 14.0, "MD",
     M + "doc7593.pdf p.430 (image; MD: the A2 numbers)", "TQFP", None),
    ("PT", 64, "QFP", 0.5, r1(11.4, 11.4, 1.5, 0.3), 10.0, "C04-085C + C04-2085B",
     M + "AVR128DA28-32-48-64-DataSheet-DS40002183B.pdf p.600-601 (R1: C 11.40, X1 0.30, Y1 1.50)", "TQFP", None),
    ("100A", 100, "QFP", 0.5, r2(0.75, 0.27, 16.25), 14.0, "100A",
     M + "2552S.pdf p.23 (L 0.45-0.75, B 0.17-0.27, D 15.75-16.25; body 14.00)", "TQFP", "TQFP-100-MC-100A"),
    # --- QFN, Microchip land patterns (R1)
    ("M", 20, "QFN", 0.4, r1(3.0, 3.0, 0.8, 0.2, 1.8, 1.8), 3.0, "C04-21380 (REB)",
     M + "ATtiny1614-16-17-DataSheet-DS40002204A.pdf p.577-578 (R1: C 3.00, X1 0.20, Y1 0.80, centre 1.80)", "QFN", "VQFN-20-MC-REB"),
    ("REB", 20, "QFN", 0.4, r1(3.0, 3.0, 0.8, 0.2, 1.8, 1.8), 3.0, "C04-21380",
     M + "AVR16EB14_20_28_32-Prelim-DataSheet-DS40002522.pdf p.596-597 (R1)", "QFN", None),
    ("2LX", 20, "QFN", 0.4, r1(3.0, 3.0, 0.8, 0.2, 1.8, 1.8), 3.0, "C04-476",
     M + "AVR32LA14-20-28-32-DataSheet-DS40002684.pdf p.549-550 (R1)", "QFN", None),
    ("M", 24, "QFN", 0.5, r1(4.0, 4.0, 0.85, 0.3, 2.75, 2.75), 4.0, "C04-21386 (RLB)",
     M + "ATtiny1614-16-17-DataSheet-DS40002204A.pdf p.581-582 (R1: C 4.00, X1 0.30, Y1 0.85, centre 2.75)", "QFN", "VQFN-24-MC-RLB"),
    ("STX", 28, "QFN", 0.4, r1(4.0, 4.0, 0.8, 0.2, 2.75, 2.75), 4.0, "C04-456",
     M + "AVR16-32DD28-32-DataSheet-DS40002441.pdf p.664-665 (R1: C 4.00, X1 0.20, Y1 0.80, centre 2.75)", "QFN", "VQFN-28-MC-STX"),
    ("3LW", 28, "QFN", 0.4, r1(4.0, 4.0, 0.8, 0.2, 2.75, 2.75), 4.0, "C04-565",
     M + "AVR32SD20-28-32-Data-Sheet-DS40002629.pdf p.739-740 (R1)", "QFN", None),
    ("RXB", 32, "QFN", 0.5, r1(5.0, 5.0, 0.8, 0.3, 3.2, 3.2), 5.0, "C04-21395",
     M + "AVR128DA28-32-48-64-DataSheet-DS40002183B.pdf p.585-586 (R1: C 5.00, X1 0.30, Y1 0.80, centre 3.20)", "QFN", "VQFN-32-MC-RXB"),
    ("M", 32, "QFN", 0.5, r1(5.0, 5.0, 0.8, 0.3, 3.2, 3.2), 5.0, "C04-21395",
     M + "ATmega3208-3209-Data-Sheet-DS40002174A.pdf p.530-531 (R1)", "QFN", None),
    ("QZB", 32, "QFN", 0.5, r1(5.0, 5.0, 0.8, 0.3, 3.2, 3.2), 5.0, "C04-21511",
     M + "AVR32LA14-20-28-32-DataSheet-DS40002684.pdf p.563-564 (R1)", "QFN", None),
    ("32MS1", 32, "QFN", 0.5, r1(5.0, 5.0, 0.8, 0.3, 3.2, 3.2), 5.0, "C04-21395",
     M + "ATmega48PB-88PB-168PB-DataSheet-DS40001889.pdf p.386-387 (R1)", "QFN", None),
    ("32PN", 32, "QFN", 0.5, r1(5.0, 5.0, 0.8, 0.3, 3.2, 3.2), 5.0, "C04-21395",
     M + "ATtiny87-167-DataSheet-DS40002167A.pdf p.287-288 (R1)", "QFN", None),
    ("PW", 44, "QFN", 0.5, r1(6.8, 6.8, 0.95, 0.3, 5.4, 5.4), 7.0, "C04-21421",
     M + "ATxmega128-64-32-16A4U-DataSheet-DS40002166A.pdf p.77-78 (R1: C 6.80, X1 0.30, Y1 0.95, centre 5.40)", "QFN", "VQFN-44-MC-PW"),
    ("6LX", 48, "QFN", 0.4, r1(5.9, 5.9, 0.85, 0.2, 4.2, 4.2), 6.0, "C04-494 + C04-2494",
     M + "AVR128DA28-32-48-64-DataSheet-DS40002183B.pdf p.591-592 (R1: C 5.90, X1 0.20, Y1 0.85, centre 4.20)", "QFN", "VQFN-48-MC-6LX"),
    ("M", 48, "QFN", 0.4, r1(5.9, 5.9, 0.85, 0.2, 4.2, 4.2), 6.0, "C04-494 / C04-504 (same land pattern)",
     M + "ATmega4808-09-DataSheet-DS40002173C.pdf p.543-547 (6LX and 6MX: one land pattern, R1)", "QFN", None),
    ("MR", 64, "QFN", 0.5, r1(8.9, 8.9, 0.85, 0.3, 5.5, 5.5), 9.0, "C04-154 + C04-2154A",
     M + "AVR64DA28-32-48-64-DataSheet-DS40002233.pdf p.682 (image; current MR drawing: C 8.90, X1 0.30, Y1 0.85, centre 5.50; the older C04-149 of DS40002183B has a 7.15 pad)",
     "QFN", "VQFN-64-MC-MR"),
    # --- QFN, Atmel outlines (R4)
    ("20M1", 20, "QFN", 0.5, r4(0.40, 0.23, 4.0, 2.6), 4.0, "20M1",
     M + "2535S.pdf p.15 (b 0.18-0.30 nom 0.23, L nom 0.40, D 4.00, exposed 2.60)", "QFN", "QFN-20-MC-20M1"),
    ("20M2", 20, "QFN", 0.45, r4(0.40, 0.22, 3.0, 1.55), 3.0, "20M2",
     M + "8183s.pdf p.18 (pitch 0.45, b nom 0.22, L nom 0.40, D 3.00, exposed 1.55)", "QFN", "VQFN-20-MC-20M2"),
    ("28M1", 28, "QFN", 0.45, r4(0.40, 0.22, 4.0, 2.4), 4.0, "28M1",
     M + "8008S.pdf p.16 (pitch 0.45, b nom 0.22, L nom 0.40, D 4.00, exposed 2.40)", "QFN", "VQFN-28-MC-28M1"),
    ("32M1-A", 32, "QFN", 0.5, r4(0.40, 0.23, 5.0, 3.1), 5.0, "32M1-A",
     M + "1062s.pdf p.10 (b nom 0.23, L nom 0.40, D 5.00, exposed 3.10)", "QFN", "QFN-32-MC-32M1-A"),
    ("32Z", 32, "QFN", 0.5, r4(0.40, 0.25, 5.0, 3.1), 5.0, "PN (32Z)",
     M + "ATxmega32E5-16E5-8E5-Data-Sheet-DS40002059A.pdf p.76 (image; b nom 0.25, L nom 0.40, D 5.00, exposed 3.10)", "QFN", None),
    ("QFN32", 32, "QFN", 0.5, r4(0.40, 0.25, 5.0, 3.6), 5.0, "PG",
     M + "7707S.pdf p.15 (image; AT90USB82/162: b nom 0.25, L nom 0.40, D 5.00, exposed 3.60)", "QFN", "QFN-32-MC-PG"),
    ("32MA", 32, "QFN", 0.4, r4(0.35, 0.20, 4.0, 2.8), 4.0, "PM (32MA)",
     M + "ATxmega32E5-16E5-8E5-Data-Sheet-DS40002059A.pdf p.77 (image; pitch 0.40, b nom 0.20, L nom 0.35, D 4.00, exposed 2.80)", "QFN", "UQFN-32-MC-32MA"),
    ("PV", 32, "QFN", 0.65, r4(0.60, 0.30, 7.0, 4.5), 7.0, "PV",
     M + "Atmel-8209-8-bit-AVR-ATmega16M1-32M1-64M1_Summary.pdf p.17 (image; pitch 0.65, b nom 0.30, L nom 0.60, D 7.00, exposed 4.50)", "QFN", "QFN-32-MC-PV"),
    ("44M1", 44, "QFN", 0.5, r4(0.64, 0.23, 7.0, 5.2), 7.0, "44M1",
     M + "2512S.pdf p.17 (b nom 0.23, L nom 0.64, D 7.00, exposed 5.20)", "QFN", "VQFN-44-MC-44M1"),
    ("44M1-A", 44, "QFN", 0.5, r4(0.64, 0.23, 7.0, 5.2), 7.0, "44M1-A",
     M + "doc2502.pdf p.309 (the 44M1 numbers)", "QFN", None),
    ("44PW", 44, "QFN", 0.5, r4(0.55, 0.23, 7.0, 5.2), 7.0, "PW (Atmel)",
     M + "Atmel-7766-8-bit-AVR-ATmega16U4-32U4_Summary.pdf p.19 (image; b nom 0.23, L nom 0.55, D 7.00, exposed 5.20)", "QFN", "VQFN-44-MC-44PW"),
    ("64M1", 64, "QFN", 0.5, r4(0.40, 0.25, 9.0, 5.4), 9.0, "64M1",
     M + "2467S.pdf p.14 (b nom 0.25, L nom 0.40, D 9.00, exposed 5.40)", "QFN", "QFN-64-MC-64M1"),
    ("Z64-1", 64, "QFN", 0.5, r4(0.40, 0.25, 9.0, 5.4), 9.0, "Z64-1",
     M + "7679S.pdf p.15 (image; b nom 0.25, L nom 0.40, D 9.00; exposed 5.4 per the Package Type legend)", "QFN", None),
    ("Z64-2", 64, "QFN", 0.5, r4(0.40, 0.25, 9.0, 6.0), 9.0, "Z64-2",
     M + "7679S.pdf p.15 (image; exposed 6.0 per the legend)", "QFN", "QFN-64-MC-Z64-2"),
    ("64M2", 64, "QFN", 0.5, r4(0.40, 0.25, 9.0, 7.65), 9.0, "64M2",
     M + "ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf p.415 (b nom 0.25, L nom 0.40, D 9.00, exposed 7.65)", "QFN", "QFN-64-MC-64M2"),
    ("64M", 64, "QFN", 0.5, r4(0.40, 0.24, 9.0, 7.65), 9.0, "PP (64M)",
     M + "Atmel-8134-8-16-bit-Atmel-XMEGA-D3-Microcontroller_Datasheet.pdf p.62 (image; b 0.18-0.30 mid 0.24, L nom 0.40, exposed 7.65)", "QFN", None),
    ("64Z3", 64, "QFN", 0.5, r4(0.40, 0.24, 9.0, 7.65), 9.0, "PP (64Z3)",
     M + "Atmel-8361-8-and-16-bit-AVR-XMEGA-Microcontrollers-ATxmega384C3_Datasheet.pdf p.64 (image; drawing PP as 64M)", "QFN", None),
    ("PS", 64, "QFN", 0.5, r4(0.40, 0.25, 9.0, 7.5), 9.0, "PS",
     M + "doc7593.pdf p.432 (image; b nom 0.25, L nom 0.40, D 9.00, exposed 7.50)", "QFN", "QFN-64-MC-PS"),
    # --- PLCC (R3)
    ("44J", 44, "PLCC", 1.27, r3(0.813, (17.399 + 17.653) / 2), 16.59, "44J",
     M + "doc2512.pdf p.247 (B 0.660-0.813, E 17.399-17.653 tip to tip; body D1 16.51-16.662)", "PLCC", "PLCC-44-MC-44J"),
]

# per-device footprints of codes that name two packages (device, code, pins, footprint spec)
DEVICE_SPLIT = [
    # AT90PWM3B / 316 print the code QFN32 for a 7 x 7 mm, 0.65 mm pitch body
    (("AT90PWM3B", "AT90PWM316"), "QFN32", 32,
     ("QFN32", 32, "QFN", 0.65, r4(0.55, 0.29, 7.0, 3.75), 7.0, "Atmel QFN32 7x7 (MLF)",
      M + "Atmel-4317-8-bit-AVR-Flash_Microcontroller-AT90PWM2-3-2B-3B_summary.pdf p.15 (image; pitch 0.65, L 0.35-0.75 mid 0.55, b 0.23-0.35 mid 0.29, D 7.00, exposed 2.25-5.25 mid 3.75)",
      "QFN", "QFN-32-MC-PWM7X7")),
    # AT90PWM81 / 161: QFN32 5 x 5 drawing PN, exposed pad 1.25-3.60
    (("AT90PWM81", "AT90PWM161"), "QFN32", 32,
     ("QFN32", 32, "QFN", 0.5, r4(0.40, 0.25, 5.0, 2.43), 5.0, "PN (AT90PWM81)",
      M + "doc7734.pdf p.306 (image; b nom 0.25, L nom 0.40, D 5.00, exposed 1.25-3.60 mid 2.43)", "QFN", "QFN-32-MC-PN")),
]

GRID = [
    # code, pins, rows, columns, pitch, ball, body, drawing, source
    ("15CC1", 15, 4, 4, 0.65, 0.30, 3.0, "15CC1", M + "8183s.pdf p.16 (b 0.25-0.35 nom 0.30, e 0.65, D 3.00)", "BGA", "UFBGA-15-MC-15CC1"),
    ("49C2", 49, 7, 7, 0.65, 0.35, 5.0, "49C2", M + "Atmel-42712-ATmega164A_Datasheet_Summary.pdf p.21 (b 0.30-0.40 nom 0.35, e 0.65, D 5.00)", "BGA", "VFBGA-49-MC-49C2"),
    ("7P", 49, 7, 7, 0.65, 0.35, 5.0, "7P", M + "Atmel-8493 XMEGA C4 p.64 (image; ball 0.35, pitch 0.65, 5 x 5)", "BGA", None),
    ("100C2", 100, 10, 10, 0.65, 0.35, 7.0, "100C2", M + "ATxmega128A1U-64A1U-Data-Sheet-DS40002058A.pdf p.79 (b 0.30-0.40 nom 0.35, e 0.65, D 7.00)", "BGA", "VFBGA-100-MC-100C2"),
]


def drqfn44():
    """44MC, C04-21399 S3B land pattern (R1): outer A1-A24, 6 a side at 0.70,
    0.60 x 0.30 at C 4.90; inner B1-B20, 5 a side between them, 0.42 x 0.30
    at C 3.85; centre 2.65. Counter-clockwise from the top of the left side."""
    pads = []
    po, pi_, e = 4.90 / 2, 3.85 / 2, 0.70
    outer = [(-2.5 + k) * e for k in range(6)]          # -1.75 .. 1.75
    inner = [(-2 + k) * e for k in range(5)]             # -1.40 .. 1.40
    sides = [(-1, 0), (0, 1), (1, 0), (0, -1)]           # left, bottom, right, top
    n = 1
    for sx, sy in sides:
        for k, t in enumerate(outer):
            pos = t if (sx, sy) in ((-1, 0), (0, 1)) else -t
            x, y = (sx * po, pos) if sx else (pos, sy * po)
            w, h = (0.60, 0.30) if sx else (0.30, 0.60)
            pads.append({"ball": f"A{n}", "x": round(x, 3), "y": round(y, 3), "width": w, "height": h, "shape": "rect"})
            n += 1
    n = 1
    for sx, sy in sides:
        for k, t in enumerate(inner):
            pos = t if (sx, sy) in ((-1, 0), (0, 1)) else -t
            x, y = (sx * pi_, pos) if sx else (pos, sy * pi_)
            w, h = (0.42, 0.30) if sx else (0.30, 0.42)
            pads.append({"ball": f"B{n}", "x": round(x, 3), "y": round(y, 3), "width": w, "height": h, "shape": "rect"})
            n += 1
    pads.append({"pin": 0, "x": 0, "y": 0, "width": 2.65, "height": 2.65, "shape": "rect"})
    return {"id": "DRQFN-44-MC-44MC", "description": "Microchip C04-21399 (S3B) 44-lead dual-row VQFN 5 x 5",
            "note": "Microchip C04-21399 S3B + C04-23399 land pattern (R1: outer 24X 0.60 x 0.30 at C1/C2 4.90, inner 20X 0.42 x 0.30 at C3/C4 3.85, pitch 0.70, centre 2.65), "
                    + M + "ATmega164A_PA-324A_PA-644A_PA-1284_P_Data-Sheet-40002070B.pdf p.658-660; pads named A1-A24 / B1-B20 as the drawing (plan-36-J)",
            "style": "explicit", "pins": 44, "body": {"width": 5.0, "height": 5.0}, "pads": pads}


def drqfn64():
    """64MC (Atmel outline only, R4 adapted to two rows): outer A1-A34 (8, 9,
    8, 9 a side), inner B1-B30 (7, 8, 7, 8) staggered, pitch eT 0.65 along a
    row, rows eR 0.65 apart; L 0.40, b 0.23; outer pads 0.70 x 0.28 reaching
    0.3 beyond the edge, inner pads 0.50 x 0.28 centred on their terminal;
    centre 4.00 (unnumbered)."""
    e, half = 0.65, 3.5
    outer_c = half - 0.20 + 0.15       # terminal centre 0.20 in from the edge, pad 0.3 longer outward
    inner_c = half - 0.20 - 0.65
    counts_o, counts_i = [8, 9, 8, 9], [7, 8, 7, 8]
    sides = [(-1, 0), (0, 1), (1, 0), (0, -1)]
    pads = []
    for prefix, counts, c, ln in (("A", counts_o, outer_c, 0.70), ("B", counts_i, inner_c, 0.50)):
        n = 1
        for (sx, sy), cnt in zip(sides, counts):
            offs = [(-(cnt - 1) / 2 + k) * e for k in range(cnt)]
            for t in offs:
                pos = t if (sx, sy) in ((-1, 0), (0, 1)) else -t
                x, y = (sx * c, pos) if sx else (pos, sy * c)
                w, h = (ln, 0.28) if sx else (0.28, ln)
                pads.append({"ball": f"{prefix}{n}", "x": round(x, 3), "y": round(y, 3), "width": w, "height": h, "shape": "rect"})
                n += 1
    pads.append({"pin": 0, "x": 0, "y": 0, "width": 4.0, "height": 4.0, "shape": "rect"})
    return {"id": "DRQFN-64-MC-64MC", "description": "Atmel 64MC 64-lead 2-row staggered QFN 7 x 7",
            "note": "Atmel 64MC outline (no land pattern; R4 per row: outer pads L 0.40 + 0.3 = 0.70 reaching 0.3 beyond the edge, inner pads 0.50 centred, width b 0.23 + 0.05; eT 0.65, eR 0.65; exposed 4.00), "
                    + M + "8018s.pdf p.20; outer A1-A34 8/9/8/9 a side and inner B1-B30 7/8/7/8 from the bottom view, mirrored; pads named as the drawing (plan-36-J)",
            "style": "explicit", "pins": 64, "body": {"width": 7.0, "height": 7.0}, "pads": pads}


def wlcsp12():
    """12U-3: 4 x 6 staggered WLCSP; rows A-D 0.3465 apart (D1 1.039 / 3),
    columns 1-6 0.20 apart (E1 1.000 / 5); balls on alternate columns (A1 A3
    A5, B2 B4 B6, ...); pad 0.8 x b 0.269 -> 0.20 (R6)."""
    balls = ["A1", "A3", "A5", "B2", "B4", "B6", "C1", "C3", "C5", "D2", "D4", "D6"]
    pads = []
    for b in balls:
        r = "ABCD".index(b[0])
        c = int(b[1:])
        pads.append({"ball": b, "x": round((c - 3.5) * 0.2, 3), "y": round((r - 1.5) * (1.039 / 3), 3),
                     "width": 0.2, "height": 0.2, "shape": "round"})
    return {"id": "WLCSP-12-MC-12U-3", "description": "Atmel 12U-3 12-ball staggered WLCSP",
            "note": "Atmel 12U-3 (R6: ball b 0.269 nom x 0.8 -> 0.20; rows D1 1.039 / 3, columns E1 1.000 / 5; body 1.403 x 1.555 max), "
                    + M + "Atmel-8235-8-bit-AVR-Microcontroller-ATtiny20_Datasheet_Summary.pdf p.13 (image; the ordering table's 12U-1 is this drawing, revision history: \"Changed text ... from 12U-1 to 12U-3\")",
            "style": "explicit", "pins": 12, "body": {"width": 1.403, "height": 1.555}, "pads": pads}


def rp2040():
    """QFN-56-RP2040 from RP2040 datasheet § 5.1.2 Figure 167: pads from 6.00
    to 7.75 across (0.875 long, centred 3.4375 from the middle), 0.20 wide,
    pitch 0.40, 14 a side; centre 3.20 = pin 57 (GND)."""
    return {"id": "QFN-56-RP2040", "description": "RP2040 QFN-56 7 x 7, the datasheet's recommended footprint",
            "note": "RP2040 datasheet § 5.1.2 Figure 167 Recommended PCB Footprint (R1: 7.75 / 6.00 outer / inner extent -> pad 0.875 x 0.20 at span 6.875, pitch 0.40, 5.40 row length, centre 3.20; pin 57 is the centre GND pad), datasheets/mcu/raspberry-pi/rp2040-datasheet.pdf p.607",
            "style": "quad", "pins": 57, "body": {"width": 7.0, "height": 7.0}, "sides": [14, 14, 14, 14],
            "pitch": 0.4, "spanX": 6.875, "spanY": 6.875, "pad": {"width": 0.875, "height": 0.2, "shape": "rect"},
            "centre": {"width": 3.2, "height": 3.2, "shape": "rect"}, "slots": list(range(1, 57)) + [0]}


PREFIX = {"QFP": ("TQFP-", "LQFP-", "QFP-"), "QFN": ("QFN-", "VQFN-", "WQFN-", "UQFN-", "TQFN-", "HVQFN-"),
          "PLCC": ("PLCC-",)}


def matches(f, pins, pitch, res):
    if f.get("style") != "quad" or f.get("pins") != pins or abs(f.get("pitch", 0) - pitch) > 1e-6 or "slots" in f:
        return False
    if f.get("sides") != [pins // 4] * 4:
        return False
    p = f["pad"]
    ok = abs(f["spanX"] - res["spanX"]) <= 0.10 + 1e-9 and abs(f["spanY"] - res["spanY"]) <= 0.10 + 1e-9 \
        and abs(p["width"] - res["len"]) <= 0.10 + 1e-9 and abs(p["height"] - res["wid"]) <= 0.10 + 1e-9
    c = f.get("centre")
    if res["centre"] is None:
        return ok and c is None
    return ok and c is not None and abs(c["width"] - res["centre"][0]) <= 0.10 + 1e-9 \
        and abs(c["height"] - res["centre"][1]) <= 0.10 + 1e-9


def build_quad(spec, slots=None):
    code, pins, kind, pitch, res, body, drawing, src, package, new_id = spec
    obj = {"id": new_id, "description": f"Microchip {drawing} {pins}-lead {package}",
           "note": f"{drawing}, {src}", "style": "quad", "pins": pins,
           "body": {"width": body, "height": body}, "sides": [pins // 4] * 4, "pitch": pitch,
           "spanX": res["spanX"], "spanY": res["spanY"], "pad": {"width": res["len"], "height": res["wid"], "shape": "rect"}}
    if res["centre"]:
        obj["centre"] = {"width": res["centre"][0], "height": res["centre"][1], "shape": "rect"}
    return obj


def main():
    fps = json.load(open(os.path.join(ROOT, "schematical/schematical/Parts/footprints.json")))["footprints"]
    have = {f["id"]: f for f in fps}
    for p in ("footprints-C.json",):
        for f in json.load(open(os.path.join(DATA, p)))["footprints"]:
            have.setdefault(f["id"], f)
    made, new_objs, rows, dev_rows = {}, [], [], []

    def place(spec):
        code, pins, kind, pitch, res, body, drawing, src, package, new_id = spec
        if kind == "PLCC":
            # the PLCC-44 of plan-03-B numbers from the top middle; reuse only if R3 agrees
            f = have["PLCC-44"]
            if abs(f["pad"]["height"] - res["wid"]) <= 0.10 and abs(f["spanX"] - res["spanX"]) <= 0.10:
                return "PLCC-44"
            obj = dict(have["PLCC-44"])
            obj.update(id=new_id, description=f"Atmel {drawing} 44-lead PLCC", note=f"Atmel {drawing} (R3), {src}",
                       spanX=res["spanX"], spanY=res["spanY"], pad={"width": 2.0, "height": res["wid"], "shape": "rect"},
                       body={"width": body, "height": body})
            made[new_id] = obj
            new_objs.append(obj)
            return new_id
        for fid, f in list(have.items()) + list(made.items()):
            if fid.startswith(PREFIX[kind]) and matches(f, pins, pitch, res):
                return fid
        if new_id is None:
            raise SystemExit(f"{code} {pins}: nothing to reuse and no new id")
        obj = build_quad(spec)
        made[new_id] = obj
        new_objs.append(obj)
        return new_id

    for spec in QUAD:
        rows.append((spec[0], str(spec[1]), place(spec), f"{spec[6]}: {spec[7]}", spec[8]))
    for devices, code, pins, spec in DEVICE_SPLIT:
        fid = place(spec)
        for d in devices:
            dev_rows.append((d, code, str(pins), fid, f"{spec[6]}: {spec[7]}"))
    # grid
    pin_balls = {}
    for r in csv.DictReader(open(os.path.join(DATA, "pinouts.tsv")), delimiter="\t"):
        if r["pinout"].startswith("bga"):
            pin_balls.setdefault((r["device"], r["pinout"]), []).append(r["designator"])
    for code, pins, rows_n, cols, pitch, ball, body, drawing, src, package, new_id in GRID:
        sets = {tuple(sorted(v)) for (d, po), v in pin_balls.items() if po == f"bga{pins}"}
        balls = sorted(max(sets, key=len)) if sets else []
        fid = None
        for f in list(made.values()):
            if f.get("style") == "grid" and f["pins"] == pins and abs(f["pitch"] - pitch) < 1e-9 \
                    and abs(f["pad"]["width"] - r6(ball)) <= 0.10:
                fid = f["id"]
        if fid is None:
            obj = {"id": new_id, "description": f"Atmel {drawing} {pins}-ball BGA", "note": f"Atmel {drawing} (R6: ball {ball} x 0.8 -> {r6(ball)}), {src}",
                   "style": "grid", "pins": pins, "rows": rows_n, "columns": cols, "body": {"width": body, "height": body},
                   "pitch": pitch, "pad": {"width": r6(ball), "height": r6(ball), "shape": "round"}, "balls": balls}
            made[new_id] = obj
            new_objs.append(obj)
            fid = new_id
        rows.append((code, str(pins), fid, f"{drawing}: {src}", package))
    for obj, code, pins, pk, dr in ((drqfn44(), "44MC", 44, "QFN", "C04-21399 S3B"), (drqfn64(), "64MC", 64, "QFN", "64MC"),
                                    (wlcsp12(), "12U-1", 12, "WLCSP", "12U-3"), (rp2040(), "QFN-56", 56, "QFN", "RP2040 § 5.1.2")):
        made[obj["id"]] = obj
        new_objs.append(obj)
        rows.append((code, str(pins), obj["id"], f"{dr}: {obj['note'][:160]}", pk))
    with open(os.path.join(DATA, "footprints-D.json"), "w") as out:
        json.dump({"footprints": new_objs}, out, indent=1)
    path = os.path.join(DATA, "package-codes.tsv")
    pc = list(csv.DictReader(open(path), delimiter="\t"))
    cols = list(pc[0].keys())
    look = {(c, p): (fp, d, pk) for c, p, fp, d, pk in rows}
    for r in pc:
        k = (r["code"], r["pins"])
        if k in look and r["geometry"] in ("quad", "drqfn", "plcc", "grid"):
            r["footprint"], r["drawing"], r["package"] = look[k]
    with open(path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in pc:
            out.write("\t".join(r.get(c, "") for c in cols) + "\n")
    with open(os.path.join(DATA, "footprint-device.tsv"), "w") as out:
        out.write("device\tcode\tpins\tfootprint\tdrawing\n")
        for row in dev_rows:
            out.write("\t".join(row) + "\n")
    for c, p, fp, d, pk in rows:
        print(f"{c}\t{p}\t{fp}\t{pk}")
    for row in dev_rows:
        print("device", row[0], row[1], row[3])
    print(len(new_objs), "new footprints")


if __name__ == "__main__":
    main()
