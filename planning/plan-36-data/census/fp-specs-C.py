"""plan-36-C Phase 2: the dual-row and small footprints, code by code.

usage: python3 -I fp-specs-C.py     writes footprints-C.json and fills package-codes.tsv
                                     (drawing, footprint) for the dual rows; prints the table

Each (code, pins) either reuses a footprint (an id already in
Parts/footprints.json, or one made earlier in this list) when the pin count,
pitch and span agree and every pad side is within 0.10 mm of what the pad
rule gives for the Microchip drawing (C § 3), or makes a new one. The
numbers are the drawing's (drawings.json for text pages, read by eye for the
image pages; the source is in each row). Pad rules (plan-03-B § 3):
  R1  the drawing's recommended land pattern: span C, pad Y1 x X1
  R2  gull wing, outline only: pad length L(max) + 0.8, width b(max) + 0.05,
      span = E(max, tip to tip) + 1.0 - pad length
  R4  leadless (DFN), outline only: pad length L + 0.3, width b + 0.05,
      span = terminal-centre span (E - L) + 0.3; exposed pad = `centre`
  R5  through hole: drill = b(max) + 0.25 rounded up to 0.05, round pad
      drill + 0.7 (at least 1.5), pin 1 rect, span = row spacing
Body = mid of min and max (0.01 mm), width across the rows.
"""
import csv
import json
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.join(HERE, "..")
ROOT = os.path.abspath(os.path.join(DATA, "..", ".."))
M = "datasheets/mcu/microchip/"


def r2(lmax, bmax, emax):
    pl = round(lmax + 0.8, 2)
    return {"len": pl, "wid": round(bmax + 0.05, 2), "span": round(emax + 1.0 - pl, 2)}


def r4(l, b, e):
    return {"len": round(l + 0.3, 2), "wid": round(b + 0.05, 2), "span": round(e - l + 0.3, 2)}


def r5(bmax):
    drill = math.ceil(round((bmax + 0.25) / 0.05, 6)) * 0.05
    return {"drill": round(drill, 2), "pad": round(max(1.5, drill + 0.7), 2)}


def mid(a, b):
    return round((a + b) / 2, 2)


IN = 25.4
# code, pins, family, pitch, rule result, body (w, h), centre (w, h) or None, drawing, source, package, new id
SPECS = [
    # --- SOIC 150 mil
    ("SS", 8, "SOIC", 1.27, {"len": 1.55, "wid": 0.60, "span": 5.40}, None, None, "C04-057 SN + land",
     M + "ATtiny202-204-402-404-406-DataSheet-DS40002318A.pdf p.497-498 (R1: C 5.40, X1 0.60, Y1 1.55)", "SOIC", None),
    ("8S1", 8, "SOIC", 1.27, {"len": 1.55, "wid": 0.60, "span": 5.40}, None, None, "C04-057 SN + land",
     "datasheets/mcu/attiny13a.pdf p.175-176 (R1: C 5.40, X1 0.60, Y1 1.55)", "SOIC", None),
    ("8-PIN-SOIC150", 8, "SOIC", 1.27, r2(1.27, 0.51, 6.20), (mid(3.81, 3.99), mid(4.80, 5.05)), None, "8S1 rev G",
     M + "Atmel-42505-8-bit-AVR-Microcontrollers-ATtiny102-ATtiny104_Summary.pdf p.13 (image; R2: L 0.40-1.27, b 0.31-0.51, E 5.79-6.20; body E1 3.81-3.99 x D 4.80-5.05)",
     "SOIC", "SOIC-8-MC-8S1"),
    ("S8S1", 8, "SOIC", 1.27, r2(1.27, 0.51, 6.20), (mid(3.81, 3.99), mid(4.80, 5.05)), None, "8S1 rev A",
     M + "2535S.pdf p.14 (image; maxima only: L 1.27, B 0.51, H 6.20 -> R2 as 8S1 rev G)", "SOIC", None),
    ("SS", 14, "SOIC", 1.27, {"len": 1.55, "wid": 0.60, "span": 5.40}, None, None, "C04-065 SL + land",
     M + "ATtiny1614-16-17-DataSheet-DS40002204A.pdf p.569-570 (R1: C 5.40, X 0.60, Y 1.55)", "SOIC", None),
    ("SL", 14, "SOIC", 1.27, {"len": 1.55, "wid": 0.60, "span": 5.40}, None, None, "C04-065 SL + land",
     M + "AVR16EB14_20_28_32-Prelim-DataSheet-DS40002522.pdf p.587-588 (R1)", "SOIC", None),
    ("14S1", 14, "SOIC", 1.27, r2(0.050 * IN, 0.020 * IN, 0.244 * IN), (mid(0.1497 * IN, 0.1574 * IN), mid(0.3367 * IN, 0.3444 * IN)),
     None, "14S1", M + "8006S.pdf p.16 (inches: L .016-.050, b .013-.020, H .2284-.244; body E .1497-.1574 x D .3367-.3444)",
     "SOIC", "SOIC-14-MC-14S1"),
    ("14-PIN-SOIC150", 14, "SOIC", 1.27, r2(1.27, 0.51, 6.20), (mid(3.80, 4.00), mid(8.55, 8.75)), None, "TU",
     M + "Atmel-42505-8-bit-AVR-Microcontrollers-ATtiny102-ATtiny104_Summary.pdf p.14 (image; L 0.40-1.27, b 0.33-0.51, E 5.80-6.20)",
     "SOIC", None),
    # --- SOIC 208 mil (EIAJ)
    ("8S2", 8, "SOIC", 1.27, r2(0.85, 0.48, 8.26), (mid(5.18, 5.40), mid(5.13, 5.35)), None, "8S2",
     M + "2535S.pdf p.13 (L 0.51-0.85, b 0.35-0.48, E 7.70-8.26; body E1 5.18-5.40 x D 5.13-5.35)", "SOIC", "SOIC-8W-MC-8S2"),
    # --- SOIC 300 mil
    ("S", 20, "SOIC", 1.27, {"len": 1.95, "wid": 0.60, "span": 9.40}, None, None, "C04-094 SO + land",
     M + "ATtiny1624-26-27-DataSheet-DS40002234A.pdf p.508-509 (R1: C 9.40, X 0.60, Y 1.95)", "SOIC", None),
    ("SO", 20, "SOIC", 1.27, {"len": 1.95, "wid": 0.60, "span": 9.40}, None, None, "C04-094 SO + land",
     M + "AVR32-16DD20-14-Complete-DataSheet-DS40002413.pdf p.655-656 (R1)", "SOIC", None),
    ("20S2", 20, "SOIC", 1.27, r2(1.27, 0.51, 10.65), (mid(7.40, 7.60), mid(12.60, 13.00)), None, "20S2",
     M + "8197S.pdf p.16 (image; L 0.40-1.27, b 0.33-0.51, H 10.00-10.65; body E 7.40-7.60 x D 12.60-13.00)", "SOIC", "SOIC-20W-MC-20S2"),
    ("20S", 20, "SOIC", 1.27, r2(1.27, 0.51, 10.65), (mid(7.40, 7.60), mid(12.60, 13.00)), None, "20S2",
     M + "1477S.pdf p.12 (image; the 20S section prints drawing 20S2)", "SOIC", None),
    ("SO24", 24, "SOIC", 1.27, r2(1.27, 0.49, 10.65), (mid(7.40, 7.60), mid(15.20, 15.60)), None, "TD",
     M + "Atmel-7710-8-bit-Microcontroller-AT90PWM216-316_Datasheet-Summary.pdf p.19 (image; L 0.40-1.27, B 0.35-0.49, H 10.00-10.65; body E 7.40-7.60 x D 15.20-15.60)",
     "SOIC", "SOIC-24W-MC-TD"),
    ("SO", 28, "SOIC", 1.27, {"len": 2.0, "wid": 0.60, "span": 9.40}, None, None, "C04-052 (drawing-number table)",
     M + "AVR64DD32-28-Complete-DataSheet-DS40002315.pdf drawing-number table: 28 SOIC C04-00052 SO (the drawing pages are images); SOIC-28W is drawn from C04-052C + C04-2052A",
     "SOIC", None),
    # --- SSOP
    ("SS", 20, "SSOP", 0.65, {"len": 1.85, "wid": 0.45, "span": 7.00}, None, None, "C04-072 SS + land",
     M + "AVR16EB14_20_28_32-Prelim-DataSheet-DS40002522.pdf p.593-594 (R1: C 7.00, X1 0.45, Y1 1.85)", "SSOP", None),
    ("X", 20, "SSOP", 0.65, {"len": 1.85, "wid": 0.45, "span": 7.00}, None, None, "C04-072 (drawing-number table)",
     M + "ATtiny424-426-427-824-826-827-DataSheet-DS40002311A.pdf drawing-number table: 20 SSOP C04-00072; land C04-2072 as for SS-20", "SSOP", None),
    ("SS", 28, "SSOP", 0.65, {"len": 1.85, "wid": 0.45, "span": 7.00}, None, None, "C04-073 SS + C04-2073",
     M + "AVR128DA28-32-48-64-DataSheet-DS40002183B.pdf p.582-583 (R1)", "SSOP", None),
    ("X", 28, "SSOP", 0.65, {"len": 1.85, "wid": 0.45, "span": 7.00}, None, None, "C04-073 SS + C04-2073",
     M + "ATmega3208-3209-Data-Sheet-DS40002174A.pdf p.522-523 (R1)", "SSOP", None),
    # --- TSSOP
    ("ST", 14, "TSSOP", 0.65, {"len": 1.45, "wid": 0.45, "span": 5.90}, None, None, "C04-087 + land",
     M + "ATtiny1624-26-27-DataSheet-DS40002234A.pdf p.505-506 (R1: C 5.90, X 0.45, Y 1.45; same drawing as AVR16EB14 p.590)", "TSSOP", None),
    ("X", 14, "TSSOP", 0.65, {"len": 1.45, "wid": 0.45, "span": 5.90}, None, None, "C04-087 + land",
     M + "ATtiny1624-26-27-DataSheet-DS40002234A.pdf p.505-506 (R1)", "TSSOP", None),
    ("8X", 8, "TSSOP", 0.65, r2(0.70, 0.30, 6.60), (mid(4.30, 4.50), mid(2.90, 3.10)), None, "8X",
     M + "Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet-Summary.pdf p.17 (L 0.50-0.70, b 0.25-0.30, E 6.20-6.60; body E1 4.30-4.50 x D 2.90-3.10)",
     "TSSOP", "TSSOP-8-MC-8X"),
    ("14X", 14, "TSSOP", 0.65, r2(0.75, 0.30, 6.50), (mid(4.30, 4.50), mid(4.90, 5.10)), None, "14X",
     M + "Atmel-8235-8-bit-AVR-Microcontroller-ATtiny20_Datasheet.pdf p.210 (L 0.45-0.75, b 0.19-0.30, E 6.25-6.50; body 4.30-4.50 x 4.90-5.10)",
     "TSSOP", "TSSOP-14-MC-14X"),
    ("20X", 20, "TSSOP", 0.65, r2(0.75, 0.30, 6.50), (mid(4.30, 4.50), mid(6.40, 6.60)), None, "20X",
     M + "8197S.pdf p.17 (image; L 0.45-0.75, b 0.19-0.30, E 6.25-6.50; body 4.30-4.50 x 6.40-6.60)", "TSSOP", "TSSOP-20-MC-20X"),
    # --- PDIP
    ("8P3", 8, "PDIP", 2.54, r5(0.022 * IN), None, None, "8P3", M + "2535S.pdf p.12 (inches: b .014-.022, row .300)", "DIP", None),
    ("14P3", 14, "PDIP", 2.54, r5(0.559), None, None, "14P3", M + "8183s.pdf p.15 (B 0.356-0.559, row 7.62)", "DIP", None),
    ("20P3", 20, "PDIP", 2.54, r5(0.559), None, None, "20P3", M + "Atmel-2543-AVR-ATtiny2313_Datasheet.pdf p.217 (B 0.356-0.559)", "DIP", None),
    ("28P3", 28, "PDIP", 2.54, r5(0.533), (mid(7.112, 7.493), mid(34.544, 34.798)), None, "28P3",
     M + "Atmel-2486-8-bit-AVR-microcontroller-ATmega8_L_summary.pdf p.8 (B 0.381-0.533, row 7.62; body E1 7.112-7.493 x D 34.544-34.798)",
     "DIP", "PDIP-28-MC-28P3"),
    ("SP", 28, "PDIP", 2.54, r5(0.022 * IN), None, None, "C04-070",
     M + "AVR128DA28-32-48-64-Data-Sheet-DS40002183.pdf p.657 (inches: b .014-.022, row .300 / E .290-.335)", "DIP", None),
    ("40P6", 40, "PDIP", 2.54, r5(0.559), None, None, "40P6", M + "2466S.pdf p.14 (B 0.356-0.559, row 15.24)", "DIP", None),
    ("P", 40, "PDIP", 2.54, r5(0.023 * IN), None, None, "C04-016",
     M + "ATmega4808-09-DataSheet-DS40002173C.pdf p.538 (inches: b .014-.023, row .600)", "DIP", None),
    # --- SOT-23-6
    ("6ST1", 6, "SOT23", 0.95, r2(0.55, 0.50, 3.00), (mid(1.50, 1.75), mid(2.80, 3.00)), None, "6ST1",
     M + "ATtiny4-5-9-10-Data-Sheet-DS40002060A.pdf p.161 (L 0.30-0.55, b 0.30-0.50, E 2.60-3.00; body E1 1.50-1.75 x D 2.80-3.00)",
     "SOT", "SOT23-6-MC-6ST1"),
    # --- DFN (R4; exposed pad = unnumbered centre, B § Decisions)
    ("8MA4", 8, "UDFN", 0.50, r4(0.30, 0.25, 2.00), (2.00, 2.00), (0.90, 1.50), "8MA4",
     M + "Atmel-8127-AVR-8-bit-Microcontroller-ATtiny4-ATtiny5-ATtiny9-ATtiny10_Datasheet-Summary.pdf p.13 (image; L 0.20-0.40 nom 0.30, b 0.20-0.30, E 2.00; exposed E2 0.90 x D2 1.50)",
     "DFN", "UDFN-8-MC-8MA4"),
    ("8-PIN-UDFN", 8, "UDFN", 0.50, r4(0.40, 0.25, 3.00), (3.00, 2.00), (1.30, 1.50), "8MA2",
     M + "Atmel-42505-8-bit-AVR-Microcontrollers-ATtiny102-ATtiny104_Summary.pdf p.12 (image; L 0.35-0.45 nom 0.40, b nom 0.25, E 3.00, D 2.00; exposed E2 1.30 x D2 1.50)",
     "DFN", "UDFN-8-MC-8MA2"),
    ("10M1", 10, "VDFN", 0.50, r4(0.40, 0.25, 3.00), (3.00, 3.00), (1.64, 2.60), "10M1",
     M + "2535S.pdf p.16 (L 0.30-0.50, b nom 0.25, D = E 3.00; exposed pad 1.64 x 2.60 per the title)", "DFN", "VDFN-10-MC-10M1"),
]


def existing():
    fps = json.load(open(os.path.join(ROOT, "schematical/schematical/Parts/footprints.json")))["footprints"]
    return {f["id"]: f for f in fps}


def pad_of(f):
    if "pad" not in f:
        return None
    return f["pad"]


def matches(f, pins, pitch, res):
    """C § 3's reuse test against an existing dual footprint."""
    if f.get("style") != "dual" or f.get("pins") != pins or abs(f.get("pitch", 0) - pitch) > 1e-6:
        return False
    p = pad_of(f)
    if "drill" in res:
        return f.get("drill") is not None and abs(f.get("span") - (15.24 if pins >= 40 else 7.62)) < 1e-6 \
            and abs(p["width"] - res["pad"]) <= 0.10 + 1e-9
    return abs(f["span"] - res["span"]) <= 0.10 + 1e-9 and abs(p["width"] - res["len"]) <= 0.10 + 1e-9 \
        and abs(p["height"] - res["wid"]) <= 0.10 + 1e-9 and "centre" not in f


def build(spec):
    code, pins, fam, pitch, res, body, centre, drawing, src, package, new_id = spec
    obj = {"id": new_id, "description": "", "note": "", "style": "dual", "pins": pins}
    if "drill" in res:
        obj["body"] = {"width": body[0], "height": body[1]}
        obj.update(pitch=pitch, span=15.24 if pins >= 40 else 7.62,
                   pad={"width": res["pad"], "height": res["pad"], "shape": "round"}, drill=res["drill"], pin1Shape="rect")
    else:
        obj["body"] = {"width": body[0], "height": body[1]}
        obj.update(pitch=pitch, span=res["span"], pad={"width": res["len"], "height": res["wid"], "shape": "rect"})
    if centre:
        obj["centre"] = {"width": centre[0], "height": centre[1], "shape": "rect"}
    rule = "R5" if "drill" in res else ("R4" if centre else ("R1" if "land" in drawing else "R2"))
    obj["description"] = f"Microchip {drawing} {pins}-lead {fam}"
    obj["note"] = f"Microchip/Atmel {drawing} ({rule}), {src}"
    return obj


def main():
    have = existing()
    made = {}
    new_objs = []
    rows = []
    for spec in SPECS:
        code, pins, fam, pitch, res, body, centre, drawing, src, package, new_id = spec
        fp = None
        if code == "SO" and pins == 28:
            fp = "SOIC-28W"  # same Microchip drawing (C04-052 / land C04-2052A)
        if fp is None:
            prefix = {"PDIP": "PDIP-", "SOIC": "SOIC-", "SSOP": "SSOP-", "TSSOP": "TSSOP-", "SOT23": "SOT23-"}.get(fam, "-")
            for fid, f in list(have.items()) + list(made.items()):
                # only the same package family (an oscillator's DIP-8 or a CDIP is not a PDIP)
                if fid.startswith(prefix) and matches(f, pins, pitch, res) and not centre:
                    fp = fid
                    break
        if fp is None and centre is None and new_id is None:
            raise SystemExit(f"{code} {pins}: nothing to reuse and no new id")
        if fp is None:
            obj = build(spec)
            made[obj["id"]] = obj
            new_objs.append(obj)
            fp = obj["id"]
        rows.append((code, str(pins), fp, f"{drawing}: {src}", package))
    with open(os.path.join(DATA, "footprints-C.json"), "w") as out:
        json.dump({"footprints": new_objs}, out, indent=1)
    # fill package-codes.tsv
    path = os.path.join(DATA, "package-codes.tsv")
    pc = list(csv.DictReader(open(path), delimiter="\t"))
    cols = list(pc[0].keys())
    if "package" not in cols:
        cols.insert(cols.index("footprint") + 1, "package")
    look = {(c, p): (fp, d, pk) for c, p, fp, d, pk in rows}
    for r in pc:
        k = (r["code"], r["pins"])
        if k in look and r["geometry"] == "dual":
            r["footprint"], r["drawing"], r["package"] = look[k][0], look[k][1], look[k][2]
        r.setdefault("package", "")
    with open(path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in pc:
            out.write("\t".join(r.get(c, "") for c in cols) + "\n")
    for code, pins, fp, d, pk in rows:
        print(f"{code}\t{pins}\t{fp}\t{pk}")
    print(len(new_objs), "new footprints:", ", ".join(o["id"] for o in new_objs))


if __name__ == "__main__":
    main()
