"""plan-36-A Phase 4 (§ 4): copy the datasheet PDFs the `add` rows use into the tree.

usage: python3 -I place.py <cand.tsv> <pdf-dir>          copy, write datasheets.tsv, set `pdf` to the tree path
       python3 -I place.py --verify <candidates.tsv>      every add row's PDF is placed; nothing extra

Adapted from planning/plan-29-data/census/place.py. A PDF is needed when an
in-scope row with `verdict` = add names it (`pdf`: the md5 textscan.py /
sibling.py / verdicts.py wrote). Each distinct file (by SHA-256) lands at
datasheets/mcu/<maker>/<name>.pdf, <maker> = microchip / espressif /
raspberry-pi. <name> is the last path segment of its first URL (sorted;
<pdf-dir>/index.tsv maps md5 -> URL), query dropped, %20 -> -, `.pdf`
appended when missing, characters outside A-Za-z0-9._- -> -; a clash gets
-<first 6 hex of the md5>. A file whose SHA-256 equals one of the six spike
PDFs already in datasheets/mcu/ (fae501f) is not copied; its rows point at
that file. datasheets/mcu/datasheets.tsv: path, url, bytes, sha256 (one line
per URL). After placing, each add row's `pdf` is the tree path.

--verify: every add row's `pdf` is a path listed in datasheets.tsv that
exists, and every PDF under datasheets/mcu/ is used by an add row, except the
six spike PDFs, which predate plan-36 and are not its to remove.
"""
import csv
import hashlib
import os
import re
import shutil
import sys
import urllib.parse

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
BASE = os.path.join("datasheets", "mcu")
TSV = os.path.join(ROOT, BASE, "datasheets.tsv")
SLUG = {"Microchip Technology": "microchip", "Espressif Systems": "espressif", "Raspberry Pi": "raspberry-pi"}
SPIKE = ("atmega2560.pdf", "atmega328p.pdf", "atmega32u4.pdf", "atmega4809.pdf", "attiny13a.pdf", "attiny85.pdf")


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def base_name(url):
    u = ("https:" + url if url.startswith("//") else url)
    last = urllib.parse.urlparse(u).path.rstrip("/").split("/")[-1] or "datasheet"
    last = last.replace("%20", "-")
    last = urllib.parse.unquote(last)
    name = re.sub(r"[^A-Za-z0-9._-]", "-", last)
    if not name.lower().endswith(".pdf"):
        name += ".pdf"
    return name


def place(tsv, pdf_dir):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    index = {r["md5"]: r["url"] for r in csv.DictReader(open(os.path.join(pdf_dir, "index.tsv")), delimiter="\t")}
    need = {}
    for r in rows:
        if r["scope"] == "in" and r["verdict"] == "add":
            need.setdefault(r["pdf"], SLUG[r["maker"]])
    spike = {sha256(os.path.join(ROOT, BASE, f)): os.path.join(BASE, f) for f in SPIKE
             if os.path.exists(os.path.join(ROOT, BASE, f))}
    files = {}  # sha -> [md5]
    for h in need:
        files.setdefault(sha256(os.path.join(pdf_dir, h + ".pdf")), []).append(h)
    names = {}
    for sha, hs in files.items():
        first = sorted(hs, key=lambda h: index[h])[0]
        names.setdefault((need[first], base_name(index[first]).lower()), []).append(sha)
    path_of = {}
    for (slug, _), shas in names.items():
        for sha in shas:
            first = sorted(files[sha], key=lambda h: index[h])[0]
            name = base_name(index[first])
            if len(shas) > 1:
                name = f"{name[:-4]}-{first[:6]}.pdf"
            path_of[sha] = spike.get(sha, os.path.join(BASE, slug, name))
    lines = []
    tree = {}
    for sha, hs in files.items():
        dst = os.path.join(ROOT, path_of[sha])
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        if not os.path.exists(dst) or sha256(dst) != sha:
            shutil.copyfile(os.path.join(pdf_dir, hs[0] + ".pdf"), dst)
        for h in hs:
            lines.append((path_of[sha], index[h], str(os.path.getsize(dst)), sha))
            tree[h] = path_of[sha]
    lines.sort()
    with open(TSV, "w") as out:
        out.write("path\turl\tbytes\tsha256\n")
        for line in lines:
            out.write("\t".join(line) + "\n")
    for r in rows:
        if r["scope"] == "in" and r["verdict"] == "add":
            r["pdf"] = tree[r["pdf"]]
    cols = list(rows[0].keys())
    with open(tsv, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    print(f"{len(files)} PDFs placed for {len(need)} URLs ({sum(1 for s in files if s in spike)} are spike PDFs already in the tree)")
    return 0


def verify(tsv):
    rows = [r for r in csv.DictReader(open(tsv), delimiter="\t") if r["scope"] == "in" and r["verdict"] == "add"]
    listed = {r["path"] for r in csv.DictReader(open(TSV), delimiter="\t")} if os.path.exists(TSV) else set()
    used = {r["pdf"] for r in rows}
    missing = sorted(p for p in used if p not in listed or not os.path.exists(os.path.join(ROOT, p)))
    disk = set()
    for dirpath, _, fs in os.walk(os.path.join(ROOT, BASE)):
        for f in fs:
            if f.lower().endswith(".pdf"):
                disk.add(os.path.relpath(os.path.join(dirpath, f), ROOT))
    unused = sorted(disk - used - {os.path.join(BASE, f) for f in SPIKE})
    for p in missing:
        print("missing", p)
    for p in unused:
        print("unused", p)
    print(f"{len(rows)} add rows, {len(used)} PDFs, {len(missing)} missing, {len(unused)} unused")
    return 1 if missing or unused else 0


if __name__ == "__main__":
    a = sys.argv[1:]
    sys.exit(verify(a[1]) if a[0] == "--verify" else place(*a[:2]))
