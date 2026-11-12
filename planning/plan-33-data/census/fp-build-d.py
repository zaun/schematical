"""plan-33-D: the leaded and can footprints (run through `fp-build.py D`).

Every group of fpgroups.tsv with letter D gets one footprint from its
datasheet's own numbers: tables are parsed from the placed PDF (pdftotext
-layout) where the PDF prints them as text (KEMET and KYOCERA AVX can
tables, KEMET GOLDMAX styles), and transcribed below, with their page,
where the PDF prints a picture (Cal-Chip SMD cans, KYOCERA AVX SMD lands and
plates, discs, dipped tantalums).  Rules, plan-33-C section 4.1: R8 radial
leads (R5 drill and pad), R7 axial leads, R9 SMD cans (R1 land table, else
R4 on the terminal drawing).  Mid-values per docs/parts-format.md; a value
printed as a maximum only is used as printed.  Pin 1 (`+` on a polarized
part, marked by `pin1Shape` `rect` on a through-hole one) is on the left.
"""
import collections
import csv
import importlib.util
import math
import os
import re
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))

_spec = importlib.util.spec_from_file_location("fpl", os.path.join(HERE, "footprint-lines.py"))
fpl = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(fpl)


def r2(x):
    return round(x + 1e-9, 3)


def fmt(x):
    s = f"{r2(x):.2f}".rstrip("0").rstrip(".")
    return s


def up05(x):
    return math.ceil(round(x / 0.05, 6)) * 0.05


def text_of(path):
    return subprocess.run(["pdftotext", "-layout", os.path.join(ROOT, path), "-"], capture_output=True, text=True,
                          errors="replace").stdout


# ---- object makers ------------------------------------------------------------

def radial(fid, desc, span, lead, body, note, polar, lead_text):
    """R8: dual, 2 pins, span = lead spacing; R5 drill = lead + 0.25 (up to 0.05), round pad = drill + 0.7
    (at least 1.5); `body` = (w, h, round)."""
    drill = r2(up05(lead + 0.25))
    pad = r2(max(1.5, drill + 0.7))
    o = {"id": fid, "description": desc,
         "note": f"R8: {note}; lead {lead_text} -> drill {fmt(drill)}, pad {fmt(pad)}",
         "style": "dual", "pins": 2, "perSide": 1,
         "body": {"width": r2(body[0]), "height": r2(body[1])}, "pitch": 1, "span": r2(span),
         "pad": {"width": pad, "height": pad, "shape": "round"}, "drill": drill}
    if body[2]:
        o["body"]["round"] = True
    if polar:
        o["pin1Shape"] = "rect"
    return o


def smd_can(fid, desc, plate_w, plate_h, pad_len, pad_w, span, note):
    return {"id": fid, "description": desc, "note": note, "style": "dual", "pins": 2, "perSide": 1,
            "body": {"width": r2(plate_w), "height": r2(plate_h)}, "pitch": 1, "span": r2(span),
            "pad": {"width": r2(pad_len), "height": r2(pad_w), "shape": "rect"}}


def r4_can(fid, desc, plate_w, plate_h, term, width, gap, src):
    """R9 by R4 on the terminals: pad length = terminal length + 0.3, width = terminal width + 0.05; the
    inner pad edges stay at the terminal gap, so span = gap + terminal + 0.3."""
    return smd_can(fid, desc, plate_w, plate_h, term + 0.3, width + 0.05, gap + term + 0.3,
                   f"R9 (R4 on the terminals): {src}; terminal {fmt(term)} x {fmt(width)}, gap {fmt(gap)} -> pad "
                   f"{fmt(term + 0.3)} x {fmt(width + 0.05)}, span {fmt(gap + term + 0.3)}")


def r1_can(fid, desc, plate_w, plate_h, a, b, c, src):
    """R9 by R1: the datasheet's land (A gap, B pad length, C pad width): span = A + B."""
    return smd_can(fid, desc, plate_w, plate_h, b, c, a + b,
                   f"R9 (R1, the datasheet's land): {src}; A {fmt(a)}, B {fmt(b)}, C {fmt(c)} -> pad {fmt(b)} x "
                   f"{fmt(c)}, span A + B = {fmt(a + b)}; body {fmt(plate_w)} x {fmt(plate_h)}")


# ---- KEMET radial cans (text tables) ---------------------------------------------

def kemet_radial_table(path, order):
    """{code: (D, P, d)}; order 'Pd' (ES*: D L P d) or 'dP' (A7xx: D L d P)."""
    out = {}
    for line in text_of(path).splitlines():
        m = re.match(r"\s+([A-Z][A-Z0-9])\s+(\d+(?:\.\d+)?)\s+±[\d.]+\s+(\d+(?:\.\d+)?)\s+[±+][\d./+−-]*\s+"
                     r"(\d+(?:\.\d+)?)\s+(?:±[\d.]+|Nominal)\s+(\d+(?:\.\d+)?)\s+(±[\d.]+|Nominal)", line)
        if not m:
            continue
        code, d, _, x, y, _ = m.groups()
        if order == "Pd":
            out.setdefault(code, (float(d), float(x), float(y)))
        else:
            out.setdefault(code, (float(d), float(y), float(x)))
    return out


# ---- KEMET SMD cans (text tables) -------------------------------------------------

def kemet_vchip_table(path):
    """EDK/EEV/EXV/EDH: {code: (D, A, C max, P, W)}."""
    first, second = {}, {}
    for line in text_of(path).splitlines():
        t = line.split()
        if len(t) >= 9 and re.fullmatch(r"[0-9A-Z]{2}", t[0]) and t[2].startswith("±") and "Maximum" in t:
            first.setdefault(t[0], (float(t[1]), float(t[5]), float(t[7])))
        elif len(t) >= 9 and re.fullmatch(r"[0-9A-Z]{2}", t[0]) and t[2] == "Maximum":
            second.setdefault(t[0], (float(t[5]), float(t[7])))
    return {c: first[c] + second[c] for c in first if c in second}


def kemet_polysmd_table(path):
    """A76x/A78x/A740: {code: (D, W, H, C, Rmin, Rmax, P)}."""
    out = {}
    for line in text_of(path).splitlines():
        m = re.match(r"\s+([A-Z]{2})\s+(\d+(?:\.\d+)?)\s+±[\d.]+\s+[\d.]+\s+±[\d.]+\s+([\d.]+)\s+±[\d.]+\s+([\d.]+)\s+"
                     r"±[\d.]+\s+([\d.]+)\s+±[\d.]+\s+([\d.]+)\s*[–-]\s*([\d.]+)\s+([\d.]+)", line)
        if m:
            c, d, w, h, cc, r1, r2_, p = m.groups()
            out.setdefault(c, tuple(float(x) for x in (d, w, h, cc, r1, r2_, p)))
    return out


# ---- KYOCERA AVX radial cans (text tables) ----------------------------------------

def avx_radial_table(path):
    """{code: (D, d, P)} from rows `code D(in) L(in) d(in) P(in) ...`, two tables per line."""
    out = {}
    for line in text_of(path).splitlines():
        for m in re.finditer(r"(?:^|\s)(\d{4})\s+([\d.]+)\s+\([\d.]+\)\s+([\d.]+)\s+\([\d.]+\)\s+([\d.]+)\s+\([\d.]+\)"
                             r"\s+([\d.]+)\s+\([\d.]+\)", line):
            code, d, _, lead, p = m.groups()
            out.setdefault(code, (float(d), float(lead), float(p)))
    return out


# ---- transcribed (picture) tables -------------------------------------------------

# KYOCERA AVX SMD cans: plate W = H (case dimensions p.1) and RECOMMENDED LAND PATTERN (A, B, C).
AVX_SMD = {
    "kyocera-avx-apa-series.pdf": ({"0606": 6.6, "0608": 6.6, "0609": 6.6, "0809": 8.3, "0812": 8.3, "1010": 10.3},
                                   {"0606": (1.9, 3.5, 1.6), "0608": (1.9, 3.5, 1.6), "0609": (1.9, 3.5, 1.6),
                                    "0809": (3.1, 4.2, 2.2), "0812": (3.1, 4.2, 2.2), "1010": (4.5, 4.4, 2.2)},
                                   "APA case dimensions p.1 (W, H +-0.20), land p.5"),
    "kyocera-avx-aha-series.pdf": ({"0608": 6.6, "1010": 10.3},
                                   {"0608": (1.9, 3.5, 1.6), "1010": (4.0, 4.0, 2.5)},
                                   "AHA case dimensions p.1, land p.5"),
    "kyocera-avx-aea-series.pdf": ({"0810": 8.3, "1010": 10.3, "1213": 13.0, "1616": 17.0},
                                   {"0810": (3.0, 3.5, 2.5), "1010": (4.0, 4.0, 2.5), "1213": (4.3, 5.8, 2.5),
                                    "1616": (6.6, 6.5, 5.0)},
                                   "AEA case dimensions p.1, land p.6"),
}

# Cal-Chip SMD cans (picture tables p.1-2): case code -> (plate, terminal length, gap, W min, W max).
CALCHIP_SMD = {
    "CV2-SERIES.pdf": ({"E55": (5.3, 2.3, 1.5, 0.5, 0.8), "F55": (6.6, 2.7, 2.0, 0.5, 0.8),
                        "F80": (6.6, 2.7, 2.0, 0.5, 0.8), "G10": (8.4, 3.0, 3.1, 0.7, 1.1),
                        "H10": (10.4, 3.3, 4.7, 0.7, 1.1)},
                       "CV2 CONSTRUCTION AND DIMENSIONS p.1 (A = B plate, C terminal, P gap, W)"),
    "CVH-SERIES-1.pdf": ({"F60": (6.6, 2.4, 2.2, 0.5, 0.8), "F80": (6.6, 2.4, 2.2, 0.5, 0.8),
                          "G10": (8.3, 3.3, 3.1, 0.8, 1.2), "H10": (10.3, 3.2, 4.4, 0.8, 1.2),
                          "K14": (13.0, 4.7, 4.4, 0.8, 1.2)},
                         "CVH DIMENSIONS p.2 (B = C plate, A terminal, E gap; W 0.5-0.8 to 8 x 6.5, 0.8-1.2 above)"),
    "CVL-SERIES-1.pdf": ({"F80": (6.6, 2.6, 1.9, 0.5, 0.8)},
                         "CVL DIMENSIONS p.2 (B = C plate, A terminal, E gap, W)"),
}
# CCJ (p.1 picture): plate W = H, C tip to tip, R terminal width range, P gap.
CCJ = {"8X12": (8.3, 9.0, 0.7, 1.1, 3.1), "10X13": (10.3, 11.0, 0.7, 1.1, 4.5)}

# KYOCERA AVX TAP (tap.pdf p.1 maximum case dimensions D; lead d 0.50 +-0.05 from KYOCERA AVX TapTep-2.pdf p.2,
# https://www.kyocera-avx.com/docs/techinfo/Tantalum-NiobiumCapacitors/TapTep-2.pdf, which tap.pdf points to).
TAP_D = {"A": 4.5, "B": 4.5, "C": 5.0, "D": 5.0, "E": 5.5, "F": 6.0, "G": 6.5, "H": 7.0, "J": 8.0, "K": 8.5,
         "L": 9.0, "M": 9.0, "N": 9.0, "P": 10.0, "R": 10.0}
# KEMET T35X (T354K476K020AT.pdf p.3): D maximum per case; all leads 0.5 +-0.05.
T35X_D = {"A": 4.5, "B": 4.5, "C": 5.0, "D": 5.0, "E": 5.5, "F": 6.0, "G": 6.3, "H": 7.6, "J": 8.4, "K": 8.9,
          "L": 8.9, "M": 10.2}

# Ceramic discs: thickness maximum and lead diameter (Table 1 p.5-7 and Dimensions p.3).
DISC = {"KEM_C1103_KJY.pdf": (5.0, 0.60, 0.1, "KJY Table 1A/1B p.5-6: thickness 5.0 max, lead 0.60 +-0.1"),
        "KEM_C1104_KJN.pdf": (6.0, 0.60, 0.1, "KJN Table 1A-1C p.5-7: thickness 6.0 max, lead 0.60 +-0.1"),
        "KEM_C1127_CGP.pdf": (5.0, 0.60, 0.1, "CGP Table 1A p.5: thickness 5 max (2-3 kV rows); Dimensions "
                                              "p.3: lead 0.60 +-0.1 (1-6 kV)")}

# Leaded MLCC with one table each (body W x T, lead d nominal and range).
LEADED = {
    "KEM_C1059_RADIAL_MOLDED_X7R.pdf": {"052": (4.83, 2.29, 5.08, 0.635, 0.102, -0.051)},
    "KEM_C1073_HT_RADIAL_MOLDED_C0G.pdf": {"052": (4.83, 2.29, 5.08, 0.635, 0.102, -0.051)},
    "KEM_C1118_A-SERIES.pdf": {"05": (5.08, 2.54, 5.08, 0.508, 0.051, -0.051)},
    "KEM_C1119_S-SERIES.pdf": {"08": (12.70, 2.54, 10.16, 0.635, 0.051, -0.051)},
    "SR-Series.pdf": {"15": (3.81, 2.54, 2.54, 0.508, 0, 0), "21": (5.08, 3.175, 5.08, 0.508, 0, 0),
                      "30": (7.62, 3.81, 5.08, 0.508, 0, 0), "40": (10.16, 3.81, 5.08, 0.508, 0, 0)},
    "SE-Style.pdf": {"04": (10.2, 5.08, 5.08, 0.508, 0, 0)},
}
LEADED_NOTE = {"KEM_C1059_RADIAL_MOLDED_X7R.pdf": "C1059 Dimensions p.2 (L, T +-0.25, S 5.08 +-0.38)",
               "KEM_C1073_HT_RADIAL_MOLDED_C0G.pdf": "C1073 Dimensions p.1 (L, T +-0.25, S 5.08 +-0.38)",
               "KEM_C1118_A-SERIES.pdf": "A-Series Dimensions p.2 (radial 05: W x T, S +-0.76)",
               "KEM_C1119_S-SERIES.pdf": "S-Series Dimensions p.2 (radial 08: W x T, S +-0.76)",
               "SR-Series.pdf": "SR SIZE AND CAPACITANCE SPECIFICATIONS p.2 (W x T max, L.S. nominal +-0.787, "
                                "L.D. 0.508 nominal)",
               "SE-Style.pdf": "SE DIMENSIONS p.2 (L x T max, LS nominal +-0.787, LD 0.508 nominal)"}


def goldmax_table(path):
    """{style: (S or None, L, T, T footnote mark)} from the style rows of Dimensions p.3."""
    out = {}
    for line in text_of(path).splitlines():
        m = re.match(r"\s+(?:C3\dX\s+)?(3\d\d)\s+(.*)", line)
        if not m:
            continue
        mm = re.findall(r"\(([\d.]+)\)(\d?)", m.group(2))
        if len(mm) == 5:
            s, l, h, t, _ = mm
        elif len(mm) == 4:
            s = None
            l, h, t, _ = mm
        else:
            continue
        out.setdefault(m.group(1), (float(s[0]) if s else None, float(l[0]), float(t[0]), t[1]))
    return out


# KEMET AXIMAX (Dimensions p.2 picture): style -> (L max, D max); lead 0.51 +0.025 -0.076 (R7: max 0.535).
AXIMAX = {"410": (4.32, 2.41), "412": (4.32, 3.05), "420": (5.08, 2.54), "430": (6.10, 3.81), "440": (6.60, 3.81)}
AXIMAX_X8R = {"410": (4.32, 2.41), "430": (6.10, 3.81)}


def axial(fid, length, dia, lead_max, src):
    span = 2.54 * math.ceil((length + 3.0) / 2.54 - 1e-9)
    drill = r2(up05(lead_max + 0.25))
    pad = r2(max(1.5, drill + 0.7))
    return {"id": fid, "description": f"Axial ceramic capacitor, {fmt(span)} mm lead spacing",
            "note": f"R7: {src}; body {fmt(length)} max + 3.0 = {fmt(length + 3)} -> span {fmt(span)}; lead "
                    f"{fmt(lead_max)} max -> drill {fmt(drill)}, pad {fmt(pad)}",
            "style": "dual", "pins": 2, "perSide": 1, "body": {"width": r2(length), "height": r2(dia)},
            "pitch": 1, "span": r2(span), "pad": {"width": pad, "height": pad, "shape": "round"}, "drill": drill}


# ---- the build -------------------------------------------------------------------------

def mm_values(s):
    return [float(x) for x in re.findall(r"([\d.]+)mm", s)]


def group_rows():
    paths = {p["url"]: p["path"] for p in csv.DictReader(
        open(os.path.join(ROOT, "datasheets", "capacitor", "datasheets.tsv")), delimiter="\t")}
    out = collections.defaultdict(list)
    for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv"), newline="", encoding="utf-8"),
                            delimiter="\t"):
        if r["decision"] == "add":
            key = (fpl.SHORT[r["maker"]], r["type"], r["package"], r["size"], r["spacing"], fpl.maker_case(r),
                   paths[r["datasheet_url"]])
            out[key].append(r)
    return out


def build_one(g, rows, cache):
    maker, pdf = g["maker"], os.path.basename(g["pdf"])
    tag = {"KEMET": "KEMET", "Cal-Chip": "CALCHIP", "AVX": "AVX"}[maker]
    polar = g["type"] != "ceramic"
    code = g["size_code"]
    pkg = g["package"]

    def table(fn, *a):
        key = (fn.__name__, g["pdf"]) + a
        if key not in cache:
            cache[key] = fn(g["pdf"], *a)
        return cache[key]

    # KEMET radial cans
    if maker == "KEMET" and pkg == "Radial, Can":
        t = table(kemet_radial_table, "dP" if pdf.startswith("KEM_A41") or pdf[:9] in (
            "KEM_A4067", "KEM_A4068", "KEM_A4069", "KEM_A4072") else "Pd")
        d, p, lead = t[code]
        fid = f"CAPE-RAD-{fmt(d)}-{fmt(p)}-L{fmt(lead)}-KEMET"
        return radial(fid, f"Radial can capacitor, {fmt(d)} mm, {fmt(p)} mm lead spacing", p, lead,
                      (d, d, True), f"{pdf} Dimensions, size code {code}: D {fmt(d)} +-0.5, P {fmt(p)} +-0.5",
                      polar, f"{fmt(lead)} nominal"), "Radial"
    # KYOCERA AVX radial cans
    if maker == "AVX" and pkg == "Radial, Can":
        d, lead, p = table(avx_radial_table)[code]
        dm = d + 0.25 if pdf == "rea-series.pdf" else d
        fid = f"CAPE-RAD-{fmt(dm)}-{fmt(p)}-L{fmt(lead)}-AVX"
        tol = "D +0.50 -0 -> " + fmt(dm) if pdf == "rea-series.pdf" else "D +-0.50"
        return radial(fid, f"Radial can capacitor, {fmt(d)} mm, {fmt(p)} mm lead spacing", p, lead, (dm, dm, True),
                      f"{pdf} CASE DIMENSIONS p.1, code {code}: {tol}, P {fmt(p)} +-0.50", polar,
                      f"{fmt(lead)} +-0.05"), "Radial"
    # KEMET V-chip SMD cans
    if maker == "KEMET" and pkg.startswith("Radial, Can - SMD") and pdf[:9] in ("KEM_A4001", "KEM_A4002",
                                                                               "KEM_A4003", "KEM_A4049"):
        d, a, c, p, w = table(kemet_vchip_table)[code]
        term = (c - p) / 2
        fid = f"CAPE-SMD-{fmt(d)}-{code}-{pdf[10:13]}-KEMET"
        return r4_can(fid, f"SMD can capacitor, {fmt(d)} mm (KEMET size {code})", a, a, term, w, p,
                      f"{pdf} Dimensions p.2, size code {code}: A/B {fmt(a)}, C {fmt(c)} max, P {fmt(p)}, W {fmt(w)}; "
                      f"terminal (C - P)/2"), "SMD can"
    if maker == "KEMET" and pkg.startswith("Radial, Can - SMD"):
        d, w, h, c, rmin, rmax, p = table(kemet_polysmd_table)[code]
        term, width = (c - p) / 2, (rmin + rmax) / 2
        fid = f"CAPE-SMD-{fmt(d)}-{code}-{pdf[10:14]}-KEMET"
        return r4_can(fid, f"SMD can capacitor, {fmt(d)} mm (KEMET size {code})", w, h, term, width, p,
                      f"{pdf} Dimensions p.2, size code {code}: W {fmt(w)}, H {fmt(h)}, C {fmt(c)}, R {fmt(rmin)}-"
                      f"{fmt(rmax)} mid, P {fmt(p)}; terminal (C - P)/2"), "SMD can"
    if maker == "AVX" and pkg.startswith("Radial, Can - SMD"):
        plates, lands, src = AVX_SMD[pdf]
        a, b, c = lands[code]
        fid = f"CAPE-SMD-{code}-{pdf[12:15].upper()}-AVX"
        return r1_can(fid, f"SMD can capacitor, KYOCERA AVX case {code}", plates[code], plates[code], a, b, c,
                      f"{pdf} {src}, case {code}"), "SMD can"
    if maker == "Cal-Chip" and pdf in CALCHIP_SMD:
        sizes, src = CALCHIP_SMD[pdf]
        plate, term, gap, wmin, wmax = sizes[code]
        fid = f"CAPE-SMD-{code}-{pdf[:3]}-CALCHIP"
        return r4_can(fid, f"SMD can capacitor, Cal-Chip case {code}", plate, plate, term, (wmin + wmax) / 2, gap,
                      f"{pdf} {src}, case {code}: plate {fmt(plate)}, terminal {fmt(term)}, gap {fmt(gap)}, "
                      f"W {fmt(wmin)}-{fmt(wmax)} mid"), "SMD can"
    if maker == "Cal-Chip" and pdf.startswith("CCJ"):
        plate, c, rmin, rmax, p = CCJ[code]
        term = (c - p) / 2
        fid = f"CAPE-SMD-{code}-CCJ-CALCHIP"
        return r4_can(fid, f"SMD can capacitor, Cal-Chip case {code}", plate, plate, term, (rmin + rmax) / 2, p,
                      f"CCJ-SERIES-2.pdf CONSTRUCTION AND DIMENSIONS p.1, {code}: W = H {fmt(plate)}, C {fmt(c)}, "
                      f"R {fmt(rmin)}-{fmt(rmax)} mid, P {fmt(p)}; terminal (C - P)/2"), "SMD can"
    # dipped tantalums
    if pdf == "tap.pdf":
        sp = mm_values(g["spacing"])
        s = sp[0] if sp else 5.0
        d = TAP_D[code]
        fid = f"CAPT-RAD-{fmt(s)}-D{fmt(d)}-AVX"
        why = "" if sp else " (C wire: DigiKey lists 159 of the 161 C-wire TAP parts at 5.00 mm)"
        return radial(fid, f"Dipped tantalum capacitor, {fmt(s)} mm lead spacing", s, 0.5, (d, d, True),
                      f"tap.pdf p.1 case {code}: D {fmt(d)} max; lead spacing {fmt(s)} (DigiKey){why}", True,
                      "0.50 +-0.05 (TapTep-2.pdf p.2)"), "Radial"
    if pdf == "T354K476K020AT.pdf":
        s = mm_values(g["spacing"])[0]
        d = T35X_D[code]
        fid = f"CAPT-RAD-{fmt(s)}-D{fmt(d)}-KEMET"
        return radial(fid, f"Dipped tantalum capacitor, {fmt(s)} mm lead spacing", s, 0.5, (d, d, True),
                      f"T35X Dimensions p.3 case {code}: D {fmt(d)} max, lead spacing {fmt(s)}", True,
                      "0.5 +-0.05"), "Radial"
    # ceramic discs
    if pdf in DISC:
        t, lead, _, src = DISC[pdf]
        d = mm_values(g["size"])[0]
        s = mm_values(g["spacing"])[0]
        fid = f"CAP-DISC-{fmt(s)}-D{fmt(d)}-{pdf[10:13]}-{tag}"
        return radial(fid, f"Ceramic disc capacitor, {fmt(s)} mm lead spacing", s, lead, (d, t, False),
                      f"{src}; body D {fmt(d)} max (Table 1) x T {fmt(t)}, spacing {fmt(s)} +-1.0", False,
                      f"{fmt(lead)} +-0.1"), "Disc"
    # leaded MLCC
    if pdf in ("KEM_C1049_GOLDMAX_C0G.pdf", "KEM_C1050_GOLDMAX_X7R.pdf", "KEM_C1055_HIVOLT_GOLDMAX_300_X7R.pdf"):
        tab = table(goldmax_table)
        styles = {r["mpn"][1:4] for r in rows}
        dims = []
        for st in sorted(styles):
            s, l, t, mark = tab[st]
            big = mark == "1" and pdf != "KEM_C1055_HIVOLT_GOLDMAX_300_X7R.pdf" and any(
                float(r["farads"]) >= 4.7e-6 for r in rows if r["mpn"][1:4] == st)
            dims.append((st, s, l, 4.07 if big else t))
        st, s, l, t = max(dims, key=lambda x: (x[2], x[3]))
        s = mm_values(g["spacing"])[0] if s is None else s
        lead, lt = (0.6775, "0.64 +0.10 -0.025 -> 0.678 mid") if st.startswith("35") else (
            (0.5475, "0.51 +0.10 -0.025 -> 0.548 mid") if pdf != "KEM_C1055_HIVOLT_GOLDMAX_300_X7R.pdf"
            else (0.51, "0.51"))
        fid = f"CAP-RAD-{fmt(s)}-{fmt(l)}X{fmt(t)}-KEMET"
        return radial(fid, f"Radial leaded MLC capacitor, {fmt(s)} mm lead spacing", s, lead, (l, t, False),
                      f"{pdf} Dimensions p.3, style {','.join(sorted(styles))}: S {fmt(s)} +-0.78, L {fmt(l)} max, "
                      f"T {fmt(t)} max" + (" (4.07 for >= 4.7 uF, footnote 1)" if t == 4.07 else ""),
                      False, lt), "Radial"
    if pdf in LEADED:
        tab = LEADED[pdf]
        m = rows[0]["mpn"]
        key = {"KEM_C1059_RADIAL_MOLDED_X7R.pdf": m[1:4], "KEM_C1073_HT_RADIAL_MOLDED_C0G.pdf": m[1:4],
               "KEM_C1118_A-SERIES.pdf": m[3:5], "KEM_C1119_S-SERIES.pdf": m[3:5], "SR-Series.pdf": m[2:4],
               "SE-Style.pdf": m[2:4]}[pdf]
        w, t, s, ld, plus, minus = tab[key]
        lead = ld + (plus + minus) / 2
        lt = f"{fmt(ld)}" + (f" {plus:+.3f} {minus:+.3f} -> {fmt(lead)} mid" if plus or minus else " nominal")
        fid = f"CAP-RAD-{fmt(s)}-{fmt(w)}X{fmt(t)}-{tag}"
        return radial(fid, f"Radial leaded MLC capacitor, {fmt(s)} mm lead spacing", s, lead, (w, t, False),
                      f"{LEADED_NOTE[pdf]}, style {key}: body {fmt(w)} x {fmt(t)}, spacing {fmt(s)}", False,
                      lt), "Radial"
    if pdf in ("KEM_C1040_AXIMAX_C0G.pdf", "KEM_C1041_AXIMAX_X7R.pdf", "KEM_C1071_AXIMAX_X8R.pdf"):
        styles = {r["mpn"][1:4] for r in rows}
        tab = AXIMAX_X8R if "X8R" in pdf else AXIMAX
        st = max(styles, key=lambda s: tab[s])
        length, dia = tab[st]
        fid = f"CAP-AX-{fmt(2.54 * math.ceil((length + 3.0) / 2.54 - 1e-9))}-L{fmt(length)}-D{fmt(dia)}-KEMET"
        return axial(fid, length, dia, 0.535,
                     f"{pdf} Dimensions p.2, style {st}: L {fmt(length)} max, D {fmt(dia)} max, LD 0.51 +0.025 "
                     f"-0.076"), "Axial"
    raise SystemExit(f"no rule for D group {maker} {pdf} {pkg} {code}")


def build_d(fpb):
    cols, groups = fpb.read_groups()
    rows_by = group_rows()
    cache = {}
    objs = {}
    for g in groups:
        if g["letter"] != "D":
            continue
        key = (g["maker"], g["type"], g["package"], g["size"], g["spacing"], g["size_code"], g["pdf"])
        obj, pkg = build_one(g, rows_by[key], cache)
        prev = objs.get(obj["id"])
        if prev is not None and fpb.geometry(prev) != fpb.geometry(obj):
            raise SystemExit(f"two geometries for {obj['id']}")
        objs.setdefault(obj["id"], obj)
        g["footprint"], g["part_package"] = obj["id"], pkg
        g["pinout"] = "two-pin" if g["type"] == "ceramic" else "polar"
        g["note"] = obj["note"].split(";")[0] + "; " + os.path.basename(g["pdf"])
    fpb.write(cols, groups, "footprints-D.json", list(objs.values()))
