"""plan-29-B–G: rows whose DigiKey link is another product's datasheet.

usage: python3 -I wrong-pdf.py <pdf-dir> <file name>...

The named PDFs (under datasheets/diode/<maker>/) are not diode datasheets
(found by B, F, G: an IRL620 MOSFET, a PE60 potentiometer, IHLP inductors).
A row on one has no datasheet of its own, so § 3 rule 3's sibling check
applies: a downloaded maker-site PDF of the same maker that prints the part
number verbatim becomes its datasheet (decision add, a spec row added with
mpn_regex .* when its group is new); else skip, "DigiKey's link is another
product's datasheet (<file>); no other datasheet on the maker's site prints
it".  Rewrites candidates.tsv and specs.tsv.
"""
import csv
import hashlib
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PLACED = os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")


def norm(s):
    return " " + re.sub(r"[^0-9A-Z]+", " ", s.upper()).strip() + " "


def main(pdf_dir, names):
    cand = os.path.join(DATA, "candidates.tsv")
    spec = os.path.join(DATA, "specs.tsv")
    rows = list(csv.DictReader(open(cand), delimiter="\t"))
    cols = list(rows[0].keys())
    specs = list(csv.DictReader(open(spec), delimiter="\t"))
    scols = list(specs[0].keys())
    placed = list(csv.DictReader(open(PLACED), delimiter="\t"))
    path_of = {p["url"]: p["path"] for p in placed}
    wrong = {u for u, p in path_of.items() if os.path.basename(p) in names}
    texts = {}
    for r in rows:
        u = r["datasheet_url"]
        if r["decision"] in ("add", "pending") and u not in wrong and u in path_of and u not in texts:
            t = os.path.join(pdf_dir, hashlib.md5(u.encode()).hexdigest() + ".txt")
            texts[u] = (r["maker"], norm(open(t, errors="replace").read()) if os.path.exists(t) else "")
    keys = {(s["maker"], s["category"], s["datasheet"], s["supplier_package"]) for s in specs}
    moved = skipped = 0
    for r in rows:
        if r["datasheet_url"] not in wrong or r["decision"] not in ("add", "skip", "pending"):
            continue
        name = os.path.basename(path_of[r["datasheet_url"]])
        k = norm(r["mpn"])
        sib = sorted(u for u, (m, t) in texts.items() if m == r["maker"] and k in t)
        if sib:
            r["datasheet_url"] = sib[0]
            r["decision"] = "add"
            r["reason"] = f"datasheet from a same-maker PDF (DigiKey link: {name}, another product's)"
            key = (r["maker"], r["category"], path_of[sib[0]], r["supplier_package"])
            if key not in keys:
                keys.add(key)
                specs.append(dict(zip(scols, list(key) + [".*"] + [""] * (len(scols) - 5))))
            moved += 1
        else:
            r["decision"] = "skip"
            r["reason"] = (f"DigiKey's link is another product's datasheet ({name}); "
                           "no other datasheet on the maker's site prints it")
            skipped += 1
    with open(cand, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    specs.sort(key=lambda s: (s["maker"], s["category"], s["datasheet"], s["supplier_package"]))
    with open(spec, "w") as out:
        out.write("\t".join(scols) + "\n")
        for s in specs:
            out.write("\t".join(s[c] for c in scols) + "\n")
    print(f"{moved} rows to a sibling PDF, {skipped} skip")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], set(sys.argv[2:])))
