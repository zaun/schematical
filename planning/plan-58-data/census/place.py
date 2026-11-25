"""plan-58-A Phase 3: copy the needed datasheet PDFs into the tree.

usage: python3 -I place.py <candidates.tsv> <pdf-dir>   copy and write datasheets.tsv
       python3 -I place.py --verify                      every needed URL is placed, hashes match, no unused PDF
       python3 -I place.py --path                        print "maker<TAB>mpn<TAB>file" per needed row
       python3 -I place.py --selftest

A PDF is needed when a row of planning/plan-58-data/candidates.tsv with
`pdf` = 1 has `decision` add or pending.  Each distinct file (by SHA-256: two
URL spellings of one document share one file) lands at
datasheets/switch/slide/<maker-slug>/<name>.pdf (maker-slug ck, nkk, cit,
eswitch): <name> is the URL path's last segment of the first URL (sorted) of
that file, query dropped, `.pdf` appended when missing, characters outside
A-Za-z0-9._- replaced by `-`; a clash with another file of the same maker gets
`-<first 6 hex of the SHA-256>`.  datasheets/switch/slide/datasheets.tsv:
file, url, bytes, sha256 (one line per URL).
"""
import csv
import hashlib
import os
import re
import shutil
import sys
import urllib.parse

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
CAND = os.path.join(ROOT, "planning", "plan-58-data", "candidates.tsv")
BASE = os.path.join("datasheets", "switch", "slide")
TSV = os.path.join(ROOT, BASE, "datasheets.tsv")
KEEP = ("add", "pending")
SLUGS = [("C&K", "ck"), ("NKK", "nkk"), ("CIT", "cit"), ("E-Switch", "eswitch")]


def slug(maker):
    for prefix, s in SLUGS:
        if maker.startswith(prefix):
            return s
    raise SystemExit("no slug for maker " + maker)


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def base_name(url):
    full = "https:" + url if url.startswith("//") else url
    last = urllib.parse.unquote(urllib.parse.urlparse(full).path.rstrip("/").split("/")[-1]) or "datasheet"
    name = re.sub(r"[^A-Za-z0-9._-]", "-", last)
    if not name.lower().endswith(".pdf"):
        name += ".pdf"
    return name


def needed(tsv):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    urls = {}
    for r in rows:
        if r["pdf"] == "1" and r["decision"] in KEEP:
            urls.setdefault(r["datasheet"], slug(r["maker"]))
    return rows, urls


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def placed():
    if not os.path.exists(TSV):
        return {}
    return {r["url"]: r for r in csv.DictReader(open(TSV), delimiter="\t")}


def place(tsv, pdf_dir):
    _, urls = needed(tsv)
    files = {}  # sha -> [urls]
    for u in urls:
        files.setdefault(sha256(os.path.join(pdf_dir, md5(u) + ".pdf")), []).append(u)
    by_name = {}
    for sha, group in files.items():
        first = sorted(group)[0]
        by_name.setdefault((urls[first], base_name(first).lower()), []).append(sha)
    path_of = {}
    for (s, _), shas in by_name.items():
        for sha in shas:
            name = base_name(sorted(files[sha])[0])
            if len(shas) > 1:
                name = f"{name[:-4]}-{sha[:6]}.pdf"
            path_of[sha] = os.path.join(BASE, s, name)
    lines = []
    for sha, group in files.items():
        dst = os.path.join(ROOT, path_of[sha])
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        if not os.path.exists(dst) or sha256(dst) != sha:
            shutil.copyfile(os.path.join(pdf_dir, md5(sorted(group)[0]) + ".pdf"), dst)
        for u in group:
            lines.append((path_of[sha], u, str(os.path.getsize(dst)), sha))
    lines.sort()
    with open(TSV, "w") as out:
        out.write("file\turl\tbytes\tsha256\n")
        for line in lines:
            out.write("\t".join(line) + "\n")
    print(f"{len(files)} PDFs placed for {len(urls)} URLs")
    return 0


def verify():
    _, urls = needed(CAND)
    p = placed()
    bad = []
    for u in urls:
        if u not in p:
            bad.append("missing " + u)
            continue
        f = os.path.join(ROOT, p[u]["file"])
        if not os.path.exists(f):
            bad.append("no file " + p[u]["file"])
        elif sha256(f) != p[u]["sha256"] or str(os.path.getsize(f)) != p[u]["bytes"]:
            bad.append("hash or size differs " + p[u]["file"])
    used = {p[u]["file"] for u in urls if u in p}
    on_disk = set()
    for dirpath, _, names in os.walk(os.path.join(ROOT, BASE)):
        for f in names:
            if f.endswith(".pdf"):
                on_disk.add(os.path.relpath(os.path.join(dirpath, f), ROOT))
    unused = sorted(on_disk - used)
    bad += ["unused " + f for f in unused]
    for b in bad:
        print(b)
    print(f"{len(urls)} URLs, {len(used)} PDFs, {len(bad)} problems")
    return 1 if bad else 0


def main(argv):
    if argv == ["--verify"]:
        return verify()
    if argv == ["--path"]:
        rows, urls = needed(CAND)
        p = placed()
        for r in rows:
            if r["datasheet"] in urls and r["datasheet"] in p:
                print(f"{r['maker']}\t{r['mpn']}\t{p[r['datasheet']]['file']}")
        return 0
    if argv == ["--selftest"]:
        assert slug("C&K") == "ck" and slug("E-Switch") == "eswitch" and slug("CIT") == "cit" and slug("NKK") == "nkk"
        assert base_name("//mm.digikey.com/Volume0/x/ABC%20D.pdf?x=1") == "ABC-D.pdf"
        assert base_name("https://www.littelfuse.com/assetdocs/littelfuse-c-k-slide-s-series-datasheet?assetguid=ea") \
            == "littelfuse-c-k-slide-s-series-datasheet.pdf"
        print("ok")
        return 0
    if len(argv) == 2:
        return place(*argv)
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
