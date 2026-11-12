"""plan-33-C / -D: build the capacitor footprints from the datasheet numbers below.

usage: python3 -I fp-build.py C      footprints-C.json, and the C rows of fpgroups.tsv
       python3 -I fp-build.py D      footprints-D.json, and the D rows of fpgroups.tsv

The numbers are transcribed from each datasheet (the `note` of every object
names the drawing, file and page, and each value with its range); the rules
are plan-33-C section 4: R1 (the datasheet's land pattern, IPC-7351 level B
when offered), R4 (terminals from the outline: pad length = terminal length
+ 0.3, pad width = terminal width + 0.05, span = terminal-centre span + 0.3),
R5/R8 (radial leads), R7 (axial leads), R9 (SMD cans).  Mid-values per
docs/parts-format.md.  Pin 1 is on the left: `+` for polarized parts.
"""
import csv
import json
import math
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
TAG = {"KEMET": "KEMET", "Cal-Chip": "CALCHIP", "AVX": "AVX"}


def r2(x):
    return round(x + 1e-9, 3)


def geometry(o):
    return json.dumps({k: v for k, v in o.items() if k not in ("id", "description", "note")}, sort_keys=True)


def chip(fid, desc, body_w, body_h, pad_w, pad_h, span, note):
    """A two-pad chip: `pad_w` along x (the pad length), `pad_h` across."""
    return {"id": fid, "description": desc, "note": note, "style": "dual", "pins": 2, "perSide": 1,
            "body": {"width": r2(body_w), "height": r2(body_h)}, "pitch": 1, "span": r2(span),
            "pad": {"width": r2(pad_w), "height": r2(pad_h), "shape": "rect"}}


# ---- C: MLCC ----------------------------------------------------------------

# KEMET Table 3 "Chip Capacitor Land Pattern Design Recommendations per IPC-7351", density level B
# (C = half the pad-centre distance, Y = pad length, X = pad width), identical in KEM_C1002, C1003,
# C1007, C1010 and C1091 (p.12 of KEM_C1002_X7R_SMD); body L x W nominal from Table 1 p.3.
KEMET_MLCC = {"0402": (0.45, 0.62, 0.62, 1.00, 0.50), "0603": (0.80, 0.95, 1.00, 1.60, 0.80),
              "0805": (0.90, 1.15, 1.45, 2.00, 1.25), "1206": (1.50, 1.15, 1.80, 3.20, 1.60)}
# Cal-Chip GMC dimension tables p.3-10 (L, W, BW range), R4 on the band.
CALCHIP_MLCC = {"0402": (1.0, 0.5, 0.1, 0.35, "GMC04"), "0603": (1.6, 0.8, 0.1, 0.4, "GMC10"),
                "0805": (2.0, 1.25, 0.25, 0.75, "GMC21"), "1206": (3.2, 1.6, 0.25, 0.75, "GMC31")}
# KYOCERA AVX dimension tables (L, W, t), R4 on the band t.
AVX_MLCC = {"0402": (1.00, 0.50, 0.25, "cx5r.pdf p.3: L 1.00 +-0.20, W 0.50 +-0.20, t 0.25 +-0.10"),
            "0805": (2.01, 1.25, 0.50, "X7RDielectric.pdf p.3 and KGM_X7R.pdf p.3: L 2.01 +-0.20, W 1.25 +-0.20, "
                                        "t 0.50 +-0.25")}


def mlcc_objects():
    out = []
    for eia, (c, y, x, bl, bw) in KEMET_MLCC.items():
        out.append(chip(f"CAP-{eia}-KEMET", f"{eia} chip capacitor (KEMET land pattern)", bl, bw, y, x, 2 * c,
                        f"R1: KEMET IPC-7351 level B C {c} Y {y} X {x} (span 2C), KEM_C1002_X7R_SMD.pdf p.12 "
                        f"(same in KEM_C1003, C1007, C1010, C1091); body {bl} x {bw} p.3"))
    for eia, (bl, bw, b0, b1, code) in CALCHIP_MLCC.items():
        band = (b0 + b1) / 2
        out.append(chip(f"CAP-{eia}-CALCHIP", f"{eia} chip capacitor (Cal-Chip GMC outline)", bl, bw, band + 0.3,
                        bw + 0.05, bl - band + 0.3,
                        f"R4: {code} L {bl}, W {bw}, BW {r2(band)} mid ({b0}-{b1}); pad {r2(band + 0.3)} x "
                        f"{r2(bw + 0.05)}, span {r2(bl - band + 0.3)}; GMC-Series-3.pdf p.3"))
    for eia, (bl, bw, t, src) in AVX_MLCC.items():
        out.append(chip(f"CAP-{eia}-AVX", f"{eia} chip capacitor (KYOCERA AVX outline)", bl, bw, t + 0.3, bw + 0.05,
                        bl - t + 0.3, f"R4: {src}; pad {r2(t + 0.3)} x {r2(bw + 0.05)}, span {r2(bl - t + 0.3)}"))
    # KGM 0603: the KGM catalog's recommended land (a gap, b pad length, c pad width), the 1.6 +-0.2 row.
    a, b, c = 0.9, 0.9, 0.95
    out.append(chip("CAP-0603-AVX", "0603 chip capacitor (KYOCERA KGM land pattern)", 1.6, 0.8, b, c, a + b,
                    "R1: KGM_Series_e.pdf p.22 recommended land, 0603 (15) L 1.6 +-0.2 row: a 0.9 mid (0.8-1.0), "
                    "b 0.9 mid (0.8-1.0), c 0.95 mid (0.8-1.1); span a + b"))
    return out


# ---- C: molded chips --------------------------------------------------------

# KEMET Table 2 "Land Dimensions/Courtyard", density level B (W pad width, L pad length, S gap):
# T491 p.15, T495 p.20, T510 p.7, T52X p.37, T59X p.22, A700 p.16 print the same numbers per case.
KEMET_LAND = {"3216": (1.23, 1.80, 0.82), "3528": (2.23, 1.80, 1.12), "6032": (2.23, 2.37, 2.57),
              "7343": (2.43, 2.37, 3.87)}
KEMET_BODY = {"3216": (3.2, 1.6), "3528": (3.5, 2.8), "6032": (6.0, 3.2), "7343": (7.3, 4.3), "2012": (2.0, 1.25)}
# JEITA RC-2371 lands (G gap, Z outer, X width, Y length; min-max) for the cases with no IPC row:
# KEM_T2076_T52X-530.pdf p.38 (P, S) and KEM_T2073_T59X.pdf p.22 (S).
KEMET_JEITA = {"P": ((0.40, 1.05), (2.05, 2.60), (0.80, 1.00), "2012"),
               "S": ((1.00, 1.65), (3.25, 3.80), (1.1, 1.30), "3216")}

# KYOCERA AVX / Cal-Chip case tables: L (+-0.20), W (+0.20 -0.10), W1 (+-0.20), A (+0.30 -0.20).
AVX_CASE = {"3216": (3.20, 1.60, 1.20, 0.80), "3528": (3.50, 2.80, 2.20, 0.80), "6032": (6.00, 3.20, 2.20, 1.30),
            "7343": (7.30, 4.30, 2.40, 1.30), "2012": (2.05, 1.30, 1.00, 0.50)}


def molded_kemet(case, metric, pdfs):
    if case in KEMET_JEITA:
        g, z, x, _ = KEMET_JEITA[case]
        gm, zm, xm = sum(g) / 2, sum(z) / 2, sum(x) / 2
        bl, bw = KEMET_BODY[metric]
        return chip(f"CAPM-{metric}-{case}-KEMET", f"{metric} metric molded chip capacitor, KEMET case {case}",
                    bl, bw, (zm - gm) / 2, xm, (zm + gm) / 2,
                    f"R1: KEMET JEITA land, case {case}: G {r2(gm)} mid ({g[0]}-{g[1]}), Z {r2(zm)} mid "
                    f"({z[0]}-{z[1]}), X {r2(xm)} mid ({x[0]}-{x[1]}); pad (Z-G)/2, span (Z+G)/2; {pdfs}")
    w, l, s = KEMET_LAND[metric]
    bl, bw = KEMET_BODY[metric]
    return chip(f"CAPM-{metric}-{case}-KEMET", f"{metric} metric molded chip capacitor, KEMET case {case}",
                bl, bw, l, w, s + l,
                f"R1: KEMET Table 2 Land Dimensions, case {case} ({metric}) level B W {w} L {l} S {s} "
                f"(span S + L); {pdfs}; body {bl} x {bw}")


def molded_r4(maker, case, metric, src, w1=None, a=None, bw=None):
    bl, cbw, cw1, ca = AVX_CASE[metric]
    bw = cbw if bw is None else bw
    w1 = cw1 if w1 is None else w1
    a = ca if a is None else a
    wm, am = bw + 0.05, a + 0.05
    tag = TAG[maker]
    return chip(f"CAPM-{metric}-{case}-{tag}", f"{metric} metric molded chip capacitor, {maker} case {case}",
                bl, wm, am + 0.3, w1 + 0.05, bl - am + 0.3,
                f"R4: {src} case {case}: L {bl}, W {r2(wm)} mid (+0.20 -0.10), W1 {w1}, A {r2(am)} mid "
                f"(+0.30 -0.20); pad {r2(am + 0.3)} x {r2(w1 + 0.05)}, span {r2(bl - am + 0.3)}")


def tac_l():
    lm, wm, s_min = 1.70, 0.925, 0.55
    term = (lm - s_min) / 2
    return chip("CAPM-1608-L-AVX", "1608 metric molded chip capacitor, KYOCERA AVX TAC case L", lm, wm,
                term + 0.3, wm + 0.05, lm - term + 0.3,
                f"R4: TAC.pdf p.1 case L: L 1.70 mid (1.60 +0.20 -0), W 0.925 mid (0.85 +0.15 -0), terminal "
                f"spacing S 0.55 min, so terminal length (L - S)/2 = {r2(term)}; pad {r2(term + 0.3)} x "
                f"{r2(wm + 0.05)}, span {r2(lm - term + 0.3)}")


METRIC_OF = {"0603": "1608", "0805": "2012", "1206": "3216", "1411": "3528", "2312": "6032", "2917": "7343"}


def case_of(group, mpns):
    """The maker's case letter of a molded-chip group: DigiKey's size code, else from the part number."""
    if group["size_code"] not in ("", "-"):
        return group["size_code"]
    m = mpns[0]
    if m.startswith("TC") and not m.startswith("TCJ"):
        return m[-2]
    if m.startswith("T5") or m.startswith("T4"):
        return m[4]
    if m.startswith("A70"):
        return m[4]
    raise SystemExit(f"no case for {group} {mpns}")


def molded_id(group, mpns):
    maker, pdf = group["maker"], os.path.basename(group["pdf"])
    metric = METRIC_OF[group["package"][:4]]
    case = case_of(group, mpns)
    if maker == "KEMET":
        if case in KEMET_JEITA:
            return f"CAPM-{metric}-{case}-KEMET"
        # Cases whose IPC row is the same numbers share one object per metric size.
        return f"CAPM-{metric}-{'ABCD'[['3216', '3528', '6032', '7343'].index(metric)]}-KEMET"
    if maker == "AVX" and case == "L":
        return "CAPM-1608-L-AVX"
    return f"CAPM-{metric}-{case}-{TAG[maker]}"


def molded_objects(ids):
    out = []
    for fid in sorted(ids):
        _, metric, case, tag = fid.split("-")
        if tag == "KEMET" and case in KEMET_JEITA:
            out.append(molded_kemet(case, metric, "KEM_T2076_T52X-530.pdf p.38" +
                                    (", KEM_T2073_T59X.pdf p.22" if case == "S" else "")))
        elif tag == "KEMET":
            out.append(molded_kemet(case, metric, "KEM_T2005_T491.pdf p.15, KEM_T2009_T495.pdf p.20, "
                                                  "KEM_T2014_T510.pdf p.7, KEM_T2076_T52X-530.pdf p.37-38, "
                                                  "KEM_T2073_T59X.pdf p.22, KEM_T2091_A700-A720.pdf p.16"))
        elif fid == "CAPM-1608-L-AVX":
            out.append(tac_l())
        elif tag == "AVX":
            out.append(molded_r4("AVX", case, metric,
                                 "KYOCERA AVX case dimensions (TAJ.pdf p.1; the same rows in TCJ, TPS, TPM, TLJ, "
                                 "NOJ, NOS, F92, F93 p.1 and TAZ.pdf p.11)"))
        else:
            # PT case P is 2012-15: W 1.35 (+0.20 -0.10), not the 2012-12 cases' 1.30.
            out.append(molded_r4("Cal-Chip", case, metric,
                                 "Cal-Chip case dimensions (TC-Series.pdf p.1; PT-SERIES.pdf p.1)",
                                 bw=1.35 if case == "P" else None))
    return out


# ---- groups -----------------------------------------------------------------

def read_groups():
    with open(os.path.join(DATA, "fpgroups.tsv"), newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        return reader.fieldnames, list(reader)


def add_rows():
    paths = {p["url"]: p["path"] for p in csv.DictReader(
        open(os.path.join(os.path.dirname(os.path.dirname(DATA)), "datasheets", "capacitor", "datasheets.tsv")),
        delimiter="\t")}
    out = {}
    for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv"), newline="", encoding="utf-8"),
                            delimiter="\t"):
        if r["decision"] == "add":
            short = {"KEMET": "KEMET", "Cal-Chip Electronics": "Cal-Chip", "KYOCERA AVX": "AVX"}[r["maker"]]
            key = (short, r["type"], r["package"], r["size"], r["spacing"], r["size_code"], paths[r["datasheet_url"]])
            out.setdefault(key, []).append(r["mpn"])
    return out


def build_c():
    cols, groups = read_groups()
    mpns = add_rows()
    objs = mlcc_objects()
    molded = set()
    for g in groups:
        if g["letter"] != "C":
            continue
        key = (g["maker"], g["type"], g["package"], g["size"], g["spacing"], g["size_code"], g["pdf"])
        if g["type"] == "ceramic":
            eia = g["package"][:4]
            g["footprint"] = f"CAP-{eia}-{TAG[g['maker']]}"
            g["part_package"], g["pinout"] = eia, "two-pin"
        else:
            fid = molded_id(g, mpns[key])
            molded.add(fid)
            g["footprint"] = fid
            g["part_package"], g["pinout"] = g["package"][:4], "polar"
        obj_note = None
        g["note"] = os.path.basename(g["pdf"])
    objs += molded_objects(molded)
    by_id = {o["id"]: o for o in objs}
    used = {g["footprint"] for g in groups if g["letter"] == "C"}
    objs = [o for o in objs if o["id"] in used]
    for g in groups:
        if g["letter"] == "C":
            g["note"] = by_id[g["footprint"]]["note"].split(";")[0] + "; " + os.path.basename(g["pdf"])
    missing = used - set(by_id)
    if missing:
        raise SystemExit(f"no object for {sorted(missing)}")
    write(cols, groups, "footprints-C.json", objs)


def write(cols, groups, name, objs):
    with open(os.path.join(DATA, "fpgroups.tsv"), "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, cols, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(groups)
    with open(os.path.join(DATA, name), "w") as f:
        f.write('{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n')
        f.write(",\n".join("    " + json.dumps(o, ensure_ascii=False) for o in sorted(objs, key=lambda o: o["id"])))
        f.write("\n  ]\n}\n")
    print(f"{len(objs)} footprints in {name}")


if __name__ == "__main__":
    if sys.argv[1:] == ["C"]:
        build_c()
    elif sys.argv[1:] == ["D"]:
        import importlib.util
        spec = importlib.util.spec_from_file_location("fpd", os.path.join(HERE, "fp-build-d.py"))
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        mod.build_d(sys.modules[__name__])
    else:
        sys.exit(__doc__)
