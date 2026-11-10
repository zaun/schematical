"""plan-29-I § 3.2: check DigiKey's ratings against each spec row's datasheet.

usage: python3 -I vrio-check.py <pdf-dir> [--write]

Per spec row, the first add row (by part number) of the group: its
DigiKey vr and io (zeners: vz and power) parsed to numbers
(`values()`), with any line of work/values.tsv (mpn datasheet vr io vz pd source; an empty mpn = every add row of
that datasheet)
replacing them.  The PDF text (pdftotext -layout) must print each number
(token boundaries; a whole number also matches `N.0`).  A row passes when
all are printed; else it is listed as MISS.  With --write the
`vr_io_checked` column becomes `text` (all found), `values.tsv` (from a
read value) or `MISS <what>`.  gen-parts.py takes the same values.
"""
import collections
import csv
import hashlib
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))


def number(text, unit):
    """'50 V' -> 50.0, '1.2 kV' -> 1200, '500mA' -> 0.5, '1A (DC)' -> 1, '500 mW' -> 0.5; None when absent."""
    m = re.match(r"\s*([0-9.]+)\s*(k|m|µ|u)?\s*" + unit, text or "")
    if not m:
        return None
    v = float(m.group(1))
    return v * {"k": 1000, "m": 0.001, "µ": 1e-6, "u": 1e-6, None: 1}[m.group(2)]


def overrides():
    """work/values.tsv lines by part number; a line with an empty mpn and a
    datasheet path applies to every add row of that datasheet (key
    "@<path>")."""
    p = os.path.join(DATA, "work", "values.tsv")
    if not os.path.exists(p):
        return {}
    out = {}
    for r in csv.DictReader(open(p), delimiter="\t"):
        out[r["mpn"] or "@" + r["datasheet"]] = r
    return out


def values(r, over):
    zener = r["category"].startswith("zener")
    out = ({"vz": number(r["vz"], "V"), "pd": number(r["power"], "W")} if zener
           else {"vr": number(r["vr"], "V"), "io": number(r["io"], "A")})
    src = "digikey"
    for o in (over.get("@" + r.get("_path", "")), over.get(r["mpn"])):
        if o:
            for k in out:
                if o.get(k):
                    out[k] = float(o[k])
                    src = "values.tsv"
    return out, src


def fmt(v):
    s = ("%.4f" % v).rstrip("0").rstrip(".")
    return s


def printed(text, v):
    s = fmt(v)
    alts = {s}
    if "." not in s:
        alts.add(s + ".0")
    alts.add(fmt(v * 1000))  # mA / mW in the table
    # A dot leader ("GP10M....1000 V") may stand before the number; a decimal point may not.
    return any(re.search(r"(?<![0-9])(?<![0-9]\.)" + re.escape(a) + r"(?![0-9])", text) for a in alts)


def main(argv):
    pdf_dir = argv[0]
    write = "--write" in argv
    paths = {r["url"]: r["path"] for r in csv.DictReader(open(os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")), delimiter="\t")}
    groups = collections.defaultdict(list)
    for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv")), delimiter="\t"):
        if r["decision"] == "add":
            r["_path"] = paths[r["datasheet_url"]]
            groups[(r["maker"], r["category"], paths[r["datasheet_url"]], r["supplier_package"])].append(r)
    over = overrides()
    spec = os.path.join(DATA, "specs.tsv")
    specs = list(csv.DictReader(open(spec), delimiter="\t"))
    cols = list(specs[0].keys())
    texts = {}
    miss = 0
    for s in specs:
        g = sorted(groups[(s["maker"], s["category"], s["datasheet"], s["supplier_package"])], key=lambda r: r["mpn"])
        bad = [f"{r['mpn']}: no {k}" for r in g for k, v in values(r, over)[0].items() if v is None or v <= 0]
        first = g[0]
        vals, src = values(first, over)
        if s["datasheet"] not in texts:
            texts[s["datasheet"]] = subprocess.run(["pdftotext", "-layout", os.path.join(ROOT, s["datasheet"]), "-"],
                                                   capture_output=True, text=True, errors="replace").stdout
        t = texts[s["datasheet"]]
        notfound = [k for k, v in vals.items() if v and not printed(t, v)]
        if bad or notfound:
            miss += 1
            s["vr_io_checked"] = "MISS " + "; ".join(bad[:3] + [f"{first['mpn']} {k}={fmt(vals[k])} not printed" for k in notfound])
            print(s["datasheet"], s["supplier_package"], s["vr_io_checked"])
        else:
            s["vr_io_checked"] = "text" if src == "digikey" else "values.tsv"
    print(f"{len(specs) - miss} spec rows checked, {miss} MISS")
    if write:
        with open(spec, "w") as out:
            out.write("\t".join(cols) + "\n")
            for s in specs:
                out.write("\t".join(s[c] for c in cols) + "\n")
    return 1 if miss else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
