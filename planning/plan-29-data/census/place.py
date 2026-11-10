"""plan-29 census, step 8 (plan-29-A Phase 3): copy the needed datasheet PDFs into the tree.

usage: python3 -I place.py <candidates.tsv> <pdf-dir>    copy and write datasheets.tsv
       python3 -I place.py --verify <candidates.tsv>     every needed URL is placed; no unused PDF
       python3 -I place.py --prune <candidates.tsv>      drop the lines and PDFs no kept row needs

A PDF is needed when a row with `pdf` = 1 and a link on its maker's own site
(plan-29-A section 3 rule 3: vishay.com; onsemi.com; taiwansemi.com, ts.com.tw)
has `decision` add, pending, candidate or check.  (Sibling rows, decide.py,
point at one of those same URLs.)  Each distinct file (by SHA-256: two URL
spellings of one document share one file) lands at
datasheets/diode/<maker-slug>/<name>.pdf (plan-29-A section 4): <name> is the
last path segment of the first URL (sorted) of that file -- for a URL with a
query (taiwansemi's pdf.php?pn=SS14) the last segment of the URL it redirects
to (cached in <pdf-dir>/effective.tsv) -- `.pdf` appended when missing,
characters outside A-Za-z0-9._- replaced by `-`; a clash with another file of
the same name gets `-<first 6 hex of the md5 of its URL>`.
datasheets/diode/datasheets.tsv: path, URL, bytes, SHA-256 (one line per URL).
"""
import csv
import hashlib
import os
import re
import shutil
import subprocess
import sys
import urllib.parse

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
BASE = os.path.join("datasheets", "diode")
TSV = os.path.join(ROOT, BASE, "datasheets.tsv")
KEEP = ("add", "pending", "candidate", "check")

SLUG = {"Vishay General Semiconductor - Diodes Division": "vishay", "onsemi": "onsemi",
        "Taiwan Semiconductor Corporation": "tsc"}
OWN = {"vishay": ("vishay.com",), "onsemi": ("onsemi.com",), "tsc": ("taiwansemi.com", "ts.com.tw")}


def full(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def host_of(url):
    return urllib.parse.urlparse(full(url)).netloc.lower()


def own_site(maker, url):
    h = host_of(url)
    return any(h == x or h.endswith("." + x) for x in OWN[SLUG[maker]])


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def effective(url, pdf_dir, cache):
    if url in cache:
        return cache[url]
    try:
        out = subprocess.run(["curl", "-sIL", "--max-time", "40", "-o", "/dev/null", "-w",
                              "%{url_effective}", full(url)], capture_output=True, text=True,
                             timeout=60).stdout.strip()
    except subprocess.TimeoutExpired:
        out = ""
    cache[url] = out or url
    with open(os.path.join(pdf_dir, "effective.tsv"), "a") as f:
        f.write(f"{url}\t{cache[url]}\n")
    return cache[url]


def base_name(url, pdf_dir=None, cache=None):
    u = full(url)
    parsed = urllib.parse.urlparse(u)
    if parsed.query and pdf_dir is not None:
        parsed = urllib.parse.urlparse(effective(url, pdf_dir, cache))
    last = urllib.parse.unquote(parsed.path.rstrip("/").split("/")[-1]) or "datasheet"
    name = re.sub(r"[^A-Za-z0-9._-]", "-", last)
    if not name.lower().endswith(".pdf"):
        name += ".pdf"
    return name


def needed(tsv):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    urls = {}
    for r in rows:
        if r["pdf"] == "1" and r["decision"] in KEEP and own_site(r["maker"], r["datasheet_url"]):
            urls.setdefault(r["datasheet_url"], SLUG[r["maker"]])
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
    cache = {}
    eff = os.path.join(pdf_dir, "effective.tsv")
    if os.path.exists(eff):
        for line in open(eff):
            a, _, b = line.rstrip("\n").partition("\t")
            cache[a] = b
    files = {}  # sha -> [urls]
    for u in urls:
        files.setdefault(sha256(os.path.join(pdf_dir, md5(u) + ".pdf")), []).append(u)
    by_name = {}
    for sha, group in files.items():
        first = sorted(group)[0]
        by_name.setdefault((urls[first], base_name(first, pdf_dir, cache).lower()), []).append(sha)
    path_of = {}
    for (s, _), shas in by_name.items():
        for sha in shas:
            first = sorted(files[sha])[0]
            name = base_name(first, pdf_dir, cache)
            if len(shas) > 1:
                name = f"{name[:-4]}-{md5(first)[:6]}.pdf"
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


def on_disk():
    found = set()
    for dirpath, _, names in os.walk(os.path.join(ROOT, BASE)):
        for f in names:
            if f.endswith(".pdf"):
                found.add(os.path.relpath(os.path.join(dirpath, f), ROOT))
    return found


def verify(tsv):
    _, urls = needed(tsv)
    p = placed()
    missing = [u for u in urls if u not in p or not os.path.exists(os.path.join(ROOT, p[u]["path"]))]
    used = {p[u]["path"] for u in urls if u in p}
    unused = sorted(on_disk() - used)
    for u in missing:
        print("missing", u)
    for f in unused:
        print("unused", f)
    print(f"{len(urls)} URLs, {len(used)} PDFs, {len(missing)} missing, {len(unused)} unused")
    return 1 if missing or unused else 0


def prune(tsv):
    """Keeps only the datasheets.tsv lines whose URL a kept row needs, and
    deletes every PDF under BASE that no remaining line names."""
    _, urls = needed(tsv)
    p = placed()
    rows = sorted((r for u, r in p.items() if u in urls), key=lambda r: (r["path"], r["url"]))
    with open(TSV, "w") as out:
        out.write("path\turl\tbytes\tsha256\n")
        for r in rows:
            out.write("\t".join([r["path"], r["url"], r["bytes"], r["sha256"]]) + "\n")
    keep = {r["path"] for r in rows}
    gone = sorted(on_disk() - keep)
    for g in gone:
        os.remove(os.path.join(ROOT, g))
        print("deleted", g)
    print(f"{len(gone)} PDFs deleted, {len(keep)} kept")
    return 0


def main(argv):
    if argv[0] == "--prune":
        return prune(argv[1])
    if argv[0] == "--verify":
        return verify(argv[1])
    return place(*argv[:2])


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
