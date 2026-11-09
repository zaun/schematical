"""plan-27 census, step 7: copy the needed datasheet PDFs into the tree.

usage: python3 -I place.py <candidates.tsv> <pdf-dir>    copy and write datasheets.tsv
       python3 -I place.py --verify <candidates.tsv>     check every needed PDF is in the tree
       python3 -I place.py --path <candidates.tsv>       print "maker<TAB>mpn<TAB>path" per row

A PDF is needed when a row with `pdf` = 1 has `decision` add, pending,
candidate or check (an `out` / `skip` row's PDF is not copied unless another
row needs it).  Each lands at datasheets/switch/tactile/<maker-slug>/<name>.pdf
(plan-27-A section 4): <name> is the URL path's last segment, query dropped,
`.pdf` appended when missing, characters outside A-Za-z0-9._- replaced by `-`;
a clash between two URLs gets `-<first 6 hex of the md5>`.  The mapping is a
pure function of the set of needed URLs, so --verify recomputes it.
datasheets/switch/tactile/datasheets.tsv: path, URL, bytes, SHA-256.
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


def mapping(urls):
    """url -> repo-relative path; deterministic for a given url set."""
    by_name = {}
    for url, s in urls.items():
        by_name.setdefault((s, base_name(url).lower()), []).append(url)
    out = {}
    for (s, _), group in by_name.items():
        for url in group:
            name = base_name(url)
            if len(group) > 1:
                stem = name[:-4]
                name = f"{stem}-{md5(url)[:6]}.pdf"
            out[url] = os.path.join(BASE, s, name)
    return out


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main(argv):
    if argv[0] == "--verify":
        _, urls = needed(argv[1])
        paths = mapping(urls)
        missing = [p for p in paths.values() if not os.path.exists(os.path.join(ROOT, p))]
        on_disk = set()
        for dirpath, _, files in os.walk(os.path.join(ROOT, BASE)):
            for f in files:
                if f.endswith(".pdf"):
                    on_disk.add(os.path.relpath(os.path.join(dirpath, f), ROOT))
        extra = sorted(on_disk - set(paths.values()))
        for p in missing:
            print("missing", p)
        for p in extra:
            print("unused", p)
        print(f"{len(paths)} needed, {len(missing)} missing, {len(extra)} unused")
        return 1 if missing or extra else 0
    if argv[0] == "--path":
        rows, urls = needed(argv[1])
        paths = mapping(urls)
        for r in rows:
            if r["datasheet_url"] in paths and r["pdf"] == "1":
                print(f"{r['maker']}\t{r['mpn']}\t{paths[r['datasheet_url']]}")
        return 0
    tsv, pdf_dir = argv
    _, urls = needed(tsv)
    paths = mapping(urls)
    lines = []
    for url in sorted(paths, key=lambda u: paths[u]):
        src = os.path.join(pdf_dir, md5(url) + ".pdf")
        dst = os.path.join(ROOT, paths[url])
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        if not os.path.exists(dst) or sha256(dst) != sha256(src):
            shutil.copyfile(src, dst)
        lines.append("\t".join([paths[url], url, str(os.path.getsize(dst)), sha256(dst)]))
    with open(os.path.join(ROOT, BASE, "datasheets.tsv"), "w") as out:
        out.write("path\turl\tbytes\tsha256\n")
        out.write("\n".join(lines) + "\n")
    print(len(lines), "PDFs placed")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
