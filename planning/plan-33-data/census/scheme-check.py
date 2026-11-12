"""plan-33-B section 3.2: the inclusion-rule check of every scanned candidate row.

usage: python3 -I scheme-check.py [--tsv candidates.tsv] [--specs specs.tsv] [--report]

For every ranked row whose `pdf` column is filled (textscan.py
scanned it), writes `check`:

  no maker pdf  the row's datasheet is not a downloaded PDF on its maker's own
                site (plan-33-A section 3 rule 2), or that PDF is not placed
  ok            hit = 1 (printed verbatim), or the PDF's spec row (specs.tsv)
                has an mpn_regex that re.fullmatch-es the part number, whose
                decoded `cap` equals the bucket's capacitance (3 significant
                digits), whose decoded `volt` equals DigiKey's rated voltage,
                and (when the spec row has a case_group) whose `case` maps to a
                token found in DigiKey's package
  regex / cap / volt / case / table   the first test that failed

A regex with a named group `base` (the part number less a packaging code the
ordering table leaves out, e.g. KEMET's `ESK107M016AE3(1)` rows) also needs
that base printed in the PDF between token boundaries (textscan.py's rule;
the PDF's text from <pdf-dir>/<md5>.txt): else `table`.  Groups `b1`, `b2`,
`b3` name the pieces a ratings table prints around a placeholder
(KYOCERA AVX's `TAJA475*006#NJ`: b1 `TAJA475`, the tolerance `*`, b2 `006`,
the packaging `#`, b3 `NJ`; KEMET's `T491A105(1)010A(2)`): the table must
print `b1 b2 b3` or `b1<tol>b2 b3` (`TCJA686M002#0250E`), b3 also with the
`E` AVX lets an order leave off; any one probe (base or b-pieces) passing is
enough.

cap_group: `eia` (two significant digits, then the number of zeros, in pF; an
`R` is the decimal point: `104` = 100000 pF, `1R0` = 1 pF, `338` = 3300 µF),
`eiauf` (the same code in µF:
KYOCERA AVX's aluminum `471` = 470 µF) or `map:<code>=<farads>;...`.
volt_map: `<code>=<volts>;...`.  case_group: `<code>=<package token>;...`.
A column `check` is added after `dead` when missing.  --report prints the
counts per check and per spec row.
"""
import argparse
import collections
import csv
import hashlib
import os
import re
import sys
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PLACED = os.path.join(ROOT, "datasheets", "capacitor", "datasheets.tsv")
OWN = {"KEMET": ("yageogroup.com", "kemet.com"), "Cal-Chip Electronics": ("calchip.com",),
       "KYOCERA AVX": ("kyocera-avx.com", "avx.com", "kyocera.com")}


def full(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def own_site(maker, url):
    h = urllib.parse.urlparse(full(url)).netloc.lower()
    return any(h == x or h.endswith("." + x) for x in OWN.get(maker, ()))


def parse_map(text):
    out = {}
    for item in filter(None, (text or "").split(";")):
        k, _, v = item.partition("=")
        out[k] = v
    return out


def eia_pf(code):
    if "R" in code:
        try:
            return float(code.replace("R", "."))
        except ValueError:
            return None
    if not re.fullmatch(r"\d{3}", code):
        return None
    # Every bucket is at least 10 pF, so KEMET's small-value codes (9 = x0.1,
    # 8 = x0.01, below 10 pF) never decide a row; 8 and 9 are powers of ten
    # here, as in the electrolytic codes (`338` = 3300 uF).
    digits, mult = int(code[:2]), int(code[2])
    return digits * 10 ** mult


def cap_farads(group, code):
    if group == "eia":
        pf = eia_pf(code)
        return None if pf is None else pf * 1e-12
    if group == "eiauf":
        pf = eia_pf(code)
        return None if pf is None else pf * 1e-6
    if group == "phx":
        # KEMET PHH/PHV: the first digit is the number of digits, the last
        # two the significant figures, in µF (`425` = 2500 µF, `311` = 110 µF).
        if not re.fullmatch(r"\d{3}", code) or int(code[0]) < 2:
            return None
        return int(code[1:]) * 10 ** (int(code[0]) - 2) * 1e-6
    if group.startswith("map:"):
        v = parse_map(group[4:]).get(code)
        return None if v is None else float(v)
    return None


def same3(a, b):
    return a is not None and b is not None and f"{a:.3g}" == f"{b:.3g}"


def dk_volts(text):
    m = re.match(r"\s*([\d.]+)\s*V", text or "")
    return float(m.group(1)) if m else None


TEXTS = {}


def norm(s):
    return " " + re.sub(r"[^0-9A-Z]+", " ", s.upper()).strip() + " "


def pdf_text(pdf_dir, url):
    if url not in TEXTS:
        p = os.path.join(pdf_dir, hashlib.md5(url.encode()).hexdigest() + ".txt")
        # A table's placeholders, KEMET's `(1)` / `(2)` (`T491A105(1)010A(2)`,
        # `ESK107M016AE3(1)`), are gaps, as AVX's `*` and `#` are.
        raw = re.sub(r"\(\d\)", " ", open(p, errors="replace").read()) if os.path.exists(p) else ""
        TEXTS[url] = norm(raw) if raw else ""
    return TEXTS[url]


def check_row(r, spec, pdf_dir):
    if spec is None:
        return "no maker pdf"
    if r["hit"] == "1":
        return "ok"
    rx = spec["mpn_regex"]
    if not rx or rx == "NONE":
        return "regex"
    m = re.fullmatch(rx, r["mpn"])
    if not m:
        return "regex"
    g = m.groupdict()
    if not same3(cap_farads(spec["cap_group"], g.get("cap") or ""), float(r["farads"])):
        return "cap"
    v = parse_map(spec["volt_map"]).get(g.get("volt") or "")
    if v is None or not same3(float(v), dk_volts(r["voltage"])):
        return "volt"
    if spec["case_group"]:
        token = parse_map(spec["case_group"]).get(g.get("case") or "")
        if token is None or token not in r["package"]:
            return "case"
    probes = table_probes(g)
    if probes and not any(norm(p) in pdf_text(pdf_dir, r["datasheet_url"]) for p in probes):
        return "table"
    return "ok"


def table_probes(g):
    """The ways a ratings table may print the part (any one must be there)."""
    probes = []
    if g.get("base"):
        probes.append(g["base"])
    if g.get("b1"):
        b1, b2, b3, tol = g["b1"], g.get("b2") or "", g.get("b3") or "", g.get("tol") or ""
        for tail in ([b3, b3 + "E"] if b3 else [""]):
            probes.append(" ".join([b1, b2, tail]))
            probes.append(" ".join([b1 + tol + b2, tail]))
    return probes


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--tsv", default=os.path.join(DATA, "candidates.tsv"))
    ap.add_argument("--specs", default=os.path.join(DATA, "specs.tsv"))
    ap.add_argument("--report", action="store_true")
    ap.add_argument("--pdf-dir", default="/tmp/p33/pdf")
    a = ap.parse_args()
    with open(a.tsv, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        cols = list(reader.fieldnames)
        rows = list(reader)
    if "check" not in cols:
        cols.insert(cols.index("dead") + 1, "check")
    paths = {p["url"]: p["path"] for p in csv.DictReader(open(PLACED), delimiter="\t")}
    specs = {s["datasheet"]: s for s in csv.DictReader(open(a.specs), delimiter="\t")}
    counts = collections.Counter()
    per_spec = collections.Counter()
    for r in rows:
        r.setdefault("check", "")
        # Every scanned, ranked row: decide.py's earlier verdicts are redone.
        if not r["rank"] or r["pdf"] == "":
            continue
        spec = None
        if r["pdf"] == "1" and own_site(r["maker"], r["datasheet_url"]):
            spec = specs.get(paths.get(r["datasheet_url"], ""))
        r["check"] = check_row(r, spec, a.pdf_dir)
        counts[r["check"]] += 1
        if spec is not None:
            per_spec[(spec["datasheet"], r["check"])] += 1
    with open(a.tsv, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, cols, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    print(", ".join(f"{v} {k}" for k, v in sorted(counts.items())))
    if a.report:
        for (path, c), n in sorted(per_spec.items()):
            print(f"{n:6}  {c:6}  {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
