"""plan-27 census, step 7: copy the needed datasheet PDFs into the tree.

usage: python3 -I place.py <candidates.tsv> <pdf-dir>    copy and write datasheets.tsv
       python3 -I place.py --verify <candidates.tsv>     every needed URL is placed; no unused PDF
       python3 -I place.py --path <candidates.tsv>       print "maker<TAB>mpn<TAB>path" per row

A PDF is needed when a row with `pdf` = 1 has `decision` add, pending,
candidate or check.  Each distinct file (by SHA-256: two URL spellings of one
document share one file) lands at datasheets/switch/tactile/<maker-slug>/<name>.pdf
(plan-27-A section 4): <name> is the URL path's last segment of the first URL
(sorted) of that file, query dropped, `.pdf` appended when missing, characters
outside A-Za-z0-9._- replaced by `-`; a clash with another file gets
`-<first 6 hex of the SHA-256>`.
datasheets/switch/tactile/datasheets.tsv: path, URL, bytes, SHA-256 (one line per URL).
"""
import csv
import hashlib
import os
import re
import shutil
import sys
import urllib.parse

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
BASE = os.path.join("datasheets", "switch", "tactile")
TSV = os.path.join(ROOT, BASE, "datasheets.tsv")
KEEP = ("add", "pending", "candidate", "check")

SLUGS = [("C&K", "ck"), ("E-Switch", "eswitch"), ("Aratas", "omron"), ("Panasonic", "panasonic"),
         ("Alps", "alps"), ("Same Sky", "samesky"), ("TE ", "te"), ("APEM", "apem"),
         ("NKK", "nkk"), ("Würth", "wurth")]


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
            urls.setdefault(r["datasheet_url"], slug(r["maker"]))
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
            first = sorted(files[sha])[0]
            name = base_name(first)
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
        out.write("path\turl\tbytes\tsha256\n")
        for line in lines:
            out.write("\t".join(line) + "\n")
    print(f"{len(files)} PDFs placed for {len(urls)} URLs")
    return 0


def verify(tsv):
    _, urls = needed(tsv)
    p = placed()
    missing = [u for u in urls if u not in p or not os.path.exists(os.path.join(ROOT, p[u]["path"]))]
    used = {p[u]["path"] for u in urls if u in p}
    on_disk = set()
    for dirpath, _, names in os.walk(os.path.join(ROOT, BASE)):
        for f in names:
            if f.endswith(".pdf"):
                on_disk.add(os.path.relpath(os.path.join(dirpath, f), ROOT))
    unused = sorted(on_disk - used)
    for u in missing:
        print("missing", u)
    for f in unused:
        print("unused", f)
    print(f"{len(urls)} URLs, {len(used)} PDFs, {len(missing)} missing, {len(unused)} unused")
    return 1 if missing or unused else 0


def main(argv):
    if argv[0] == "--verify":
        return verify(argv[1])
    if argv[0] == "--path":
        rows, urls = needed(argv[1])
        p = placed()
        for r in rows:
            if r["datasheet_url"] in urls and r["datasheet_url"] in p:
                print(f"{r['maker']}\t{r['mpn']}\t{p[r['datasheet_url']]['path']}")
        return 0
    return place(*argv[:2])


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
