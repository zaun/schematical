"""plan-33-A Phase 2: which ranked part numbers appear in their datasheet.

usage: python3 -I textscan.py <candidates.tsv> <pdf-dir> [--max-rank N]

Rewrites <candidates.tsv> in place, filling `pdf`, `hit` and `dead` for every
`candidate` / `add` row with `rank` at most N (default 10) whose datasheet
was downloaded by fetch.py; other rows keep what they have.  Runs pdftotext
-layout on each PDF (cached as <md5>.txt in <pdf-dir>), turns every run of
non-alphanumeric characters in the text and in the part number into one
space, and writes `hit` = 1 when the part number is found verbatim between
token boundaries (plan-29's rule).  `pdf` = 1 when the download is a PDF.
`dead` = 1 when an obsolete / discontinued / NRND / end-of-life word stands
within 300 characters of a hit.  A miss is not a rejection: the datasheets
print part-number schemes (plan-33-B section 3).
"""
import csv
import hashlib
import os
import re
import subprocess
import sys


def text_of(directory, url):
    md5 = hashlib.md5(url.encode()).hexdigest()
    pdf = os.path.join(directory, md5 + ".pdf")
    txt = os.path.join(directory, md5 + ".txt")
    if not os.path.exists(pdf):
        return None
    with open(pdf, "rb") as f:
        if f.read(4) != b"%PDF":
            return None
    if not os.path.exists(txt):
        subprocess.run(["pdftotext", "-q", "-layout", pdf, txt], check=False, timeout=300)
    if not os.path.exists(txt):
        return None
    return open(txt, errors="replace").read()


DEAD = re.compile("OBSOLETE|DISCONTINU|NOT RECOMMENDED FOR NEW|END OF LIFE|NRND")


def norm(s):
    return " " + re.sub(r"[^0-9A-Z]+", " ", s.upper()).strip() + " "


def main(argv):
    max_rank = 10
    if "--max-rank" in argv:
        i = argv.index("--max-rank")
        max_rank = int(argv[i + 1])
        argv = argv[:i] + argv[i + 2:]
    tsv, directory = argv[0], argv[1]
    with open(tsv, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        cols = reader.fieldnames
        rows = list(reader)
    cache = {}
    scanned = hits = pdfs = 0
    for row in rows:
        if row["decision"] not in ("candidate", "add") or not row["rank"] \
                or int(row["rank"]) > max_rank:
            continue
        pdf, hit, dead = "0", "0", "0"
        url = row["datasheet_url"]
        if url:
            if url not in cache:
                text = text_of(directory, url)
                cache[url] = None if text is None else norm(text)
            body = cache[url]
            if body is not None:
                pdf = "1"
                key = norm(row["mpn"])
                at = body.find(key) if key.strip() else -1
                hit = "1" if at >= 0 else "0"
                if at >= 0 and DEAD.search(body[max(0, at - 300):at + 300]):
                    dead = "1"
        row["pdf"], row["hit"], row["dead"] = pdf, hit, dead
        scanned += 1
        pdfs += pdf == "1"
        hits += hit == "1"
    with open(tsv, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, cols, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    print(f"{scanned} rows scanned, {pdfs} with a PDF, {hits} hit")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
