#!/usr/bin/env python3
"""plan-34-A § 4: copy the needed datasheet PDFs into the tree.

usage: python3 -I place.py <candidates.tsv> <pdf-dir>   copy, write datasheets.tsv, set `pdf` to the path
       python3 -I place.py --verify <candidates.tsv>    every needed PDF is placed; no unused PDF
       python3 -I place.py --prune <candidates.tsv>     (plan-34-B) drop the PDFs no `add` row uses

A PDF is needed when an eligible row's `pdf` names it (textscan.py: the
row's own maker-site URL; sibling.py: a same-maker one). Each distinct file
(by SHA-256: two URLs of one document share one file) lands at
datasheets/resistor/<maker-slug>/<name>.pdf: <name> is the last path segment
of the first (sorted) URL of the file, query dropped, `%20` -> `-`, `.pdf`
appended when missing, characters outside A-Za-z0-9._- -> `-`; a clash with
another file of the same name gets `-<first 6 hex of the md5 of its URL>`.
The URL recorded is the one that gave the PDF (the yageogroup.com rewrite of
§ 4 rule 3 when it was used; <pdf-dir>/index.tsv). datasheets.tsv: path,
url (the one that gave the PDF), bytes, sha256, digikey_url (the row's
link), one line per DigiKey URL. The copy rewrites `pdf` in
<candidates.tsv> from the URL to the path; --verify and --prune read paths.
"""
import collections
import csv
import hashlib
import os
import re
import shutil
import sys
import urllib.parse

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

COLS = ["path", "url", "bytes", "sha256", "digikey_url"]


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def name_of(url):
    segment = urllib.parse.urlparse(url).path.rstrip("/").split("/")[-1].replace("%20", "-")
    if not segment.lower().endswith(".pdf"):
        segment += ".pdf"
    return re.sub(r"[^A-Za-z0-9._-]", "-", segment)


def read(tsv):
    return list(csv.DictReader(open(tsv), delimiter="\t"))


def write(tsv, rows):
    cols = list(rows[0].keys())
    with open(tsv, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")


def place(tsv, pdf_dir):
    rows = read(tsv)
    via = {}
    for line in open(os.path.join(pdf_dir, "index.tsv")):
        f = line.rstrip("\n").split("\t")
        via[f[4]] = f[5] if len(f) > 5 and f[5] else common.full_url(f[4])
    makers = {}
    for r in rows:
        if common.eligible(r) and r["pdf"] and not r["pdf"].startswith("datasheets/"):
            makers.setdefault(r["pdf"], r["maker"])
    by_sha = collections.defaultdict(list)
    for url in sorted(makers):
        by_sha[(makers[url], sha256(common.pdf_path(pdf_dir, url)))].append(url)
    taken = {}
    lines = []
    path_of = {}
    for (maker, digest), urls in sorted(by_sha.items(), key=lambda kv: kv[1][0]):
        first = urls[0]
        name = name_of(via[first])
        rel = os.path.join("datasheets", "resistor", common.SLUG[maker], name)
        if rel in taken and taken[rel] != digest:
            stem, ext = os.path.splitext(name)
            rel = os.path.join("datasheets", "resistor", common.SLUG[maker],
                               f"{stem}-{common.md5(first)[:6]}{ext}")
        taken[rel] = digest
        dst = os.path.join(common.ROOT, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copyfile(common.pdf_path(pdf_dir, first), dst)
        for url in urls:
            path_of[url] = rel
            lines.append([rel, via[url], str(os.path.getsize(dst)), digest, url])
    with open(common.PLACED, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for line in sorted(lines):
            out.write("\t".join(line) + "\n")
    for r in rows:
        if r["pdf"] in path_of:
            r["pdf"] = path_of[r["pdf"]]
    write(tsv, rows)
    print(len(set(path_of.values())), "PDFs placed for", len(path_of), "URLs")


def tree_files():
    out = set()
    for slug in common.SLUG.values():
        d = os.path.join(common.TREE, slug)
        if os.path.isdir(d):
            out |= {os.path.join("datasheets", "resistor", slug, f) for f in os.listdir(d)}
    return out


def verify(tsv, keep):
    rows = read(tsv)
    need = {r["pdf"] for r in rows if keep(r) and r["pdf"]}
    have = tree_files()
    listed = {r["path"] for r in csv.DictReader(open(common.PLACED), delimiter="\t")}
    missing = sorted(p for p in need if p not in have or p not in listed)
    unused = sorted(have - need)
    for p in missing:
        print("missing", p)
    for p in unused:
        print("unused", p)
    print(f"{len(missing)} missing, {len(unused)} unused")
    return 1 if missing or unused else 0


def prune(tsv):
    rows = read(tsv)
    need = {r["pdf"] for r in rows if r["decision"] == "add" and r["pdf"]}
    for p in sorted(tree_files() - need):
        os.remove(os.path.join(common.ROOT, p))
        print("deleted", p)
    kept = [line for line in open(common.PLACED) if line.startswith("path\t") or line.split("\t")[0] in need]
    with open(common.PLACED, "w") as out:
        out.writelines(kept)


def main():
    if sys.argv[1] == "--verify":
        sys.exit(verify(sys.argv[2], lambda r: common.eligible(r)))
    if sys.argv[1] == "--verify-add":
        sys.exit(verify(sys.argv[2], lambda r: r["decision"] == "add"))
    if sys.argv[1] == "--prune":
        prune(sys.argv[2])
        return
    place(sys.argv[1], sys.argv[2])


if __name__ == "__main__":
    main()
