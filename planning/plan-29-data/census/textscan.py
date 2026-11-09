"""plan-29 census, step 5: which candidate part numbers appear in their datasheet.

usage: python3 -I textscan.py <candidates.tsv> <pdf-dir> <out.tsv>

For each candidate / check row: runs pdftotext on the row's downloaded
datasheet (cached as <md5>.txt in <pdf-dir>), turns every run of
non-alphanumeric characters in the text and in the part number into one
space, and writes `hit` = 1 when the part number is found verbatim between
token boundaries ("SS14" does not hit "SS140"; "1N4001-E3/53" hits
"1N4001-E3/53" and "1N4001 E3/53", not "1N4001-E3/54").  `pdf` = 1 when the download is a PDF.
A miss is not a rejection (many datasheets give an ordering-code scheme, not a
list of part numbers); plan-29-A Phase 3 reads those datasheets.
Adds columns: pdf, hit, dead (1 when an obsolete / discontinued / NRND / end-of-life
word stands within 300 characters of the part number in the datasheet text).
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
        subprocess.run(["pdftotext", "-q", pdf, txt], check=False, timeout=120)
    if not os.path.exists(txt):
        return None
    return open(txt, errors="replace").read()


DEAD = re.compile("OBSOLETE|DISCONTINU|NOT RECOMMENDED FOR NEW|END OF LIFE|NRND")


def norm(s):
    return " " + re.sub(r"[^0-9A-Z]+", " ", s.upper()).strip() + " "


def main(tsv, directory, out_path):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    cache = {}
    cols = list(rows[0].keys()) + ["pdf", "hit", "dead"]
    with open(out_path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for row in rows:
            pdf, hit, dead = "0", "0", "0"
            if row["decision"] in ("candidate", "check") and row["datasheet_url"]:
                url = row["datasheet_url"]
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
            out.write("\t".join(row[c] for c in cols) + "\n")


if __name__ == "__main__":
    main(*sys.argv[1:4])
