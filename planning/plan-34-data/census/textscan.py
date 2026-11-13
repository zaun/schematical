#!/usr/bin/env python3
"""plan-34-A Phase 3: each eligible row's own maker-site datasheet, and
whether it prints the row's part number.

usage: python3 -I textscan.py <candidates.tsv> <pdf-dir> <out.tsv>

For each eligible row whose DigiKey datasheet_url is on its maker's own site
(common.OWN) and downloaded as a PDF: `pdf` = that URL (place.py turns it
into the file's path in the tree), `hit` = 1 when the part number stands
verbatim between token boundaries in the `pdftotext -layout` text (cached
as <md5>.txt in <pdf-dir>), `dead` = 1 when an obsolete / discontinued /
NRND / end-of-life word stands within 300 characters of it. Other rows get
empty `pdf` and 0, 0 (sibling.py may give them a PDF). The columns `hit`
and `dead` are appended; `pdf` is select.py's own column.
"""
import csv
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

DEAD = re.compile("OBSOLETE|DISCONTINU|NOT RECOMMENDED FOR NEW|END OF LIFE|NRND")


def text_of(directory, url):
    pdf = common.pdf_path(directory, url)
    if not common.is_pdf(pdf):
        return None
    txt = pdf[:-4] + ".txt"
    if not os.path.exists(txt):
        subprocess.run(["pdftotext", "-q", "-layout", pdf, txt], check=False, timeout=180)
    if not os.path.exists(txt):
        return None
    return open(txt, errors="replace").read()


def scan(body, mpn):
    """(hit, dead) of a part number in a normalized text."""
    key = common.norm(mpn)
    at = body.find(key) if key.strip() else -1
    if at < 0:
        return "0", "0"
    return "1", "1" if DEAD.search(body[max(0, at - 300):at + 300]) else "0"


def main(tsv, directory, out_path):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    cols = list(rows[0].keys()) + [c for c in ("hit", "dead") if c not in rows[0]]
    cache = {}
    with open(out_path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for row in rows:
            row["pdf"], row["hit"], row["dead"] = "", "0", "0"
            url = row["datasheet_url"]
            if common.eligible(row) and url and common.own_site(row):
                if url not in cache:
                    text = text_of(directory, url)
                    cache[url] = None if text is None else common.norm(text)
                if cache[url] is not None:
                    row["pdf"] = url
                    row["hit"], row["dead"] = scan(cache[url], row["mpn"])
            out.write("\t".join(row[c] for c in cols) + "\n")
    print(sum(1 for r in rows if r["pdf"]), "rows with an own maker-site PDF;",
          sum(1 for r in rows if r["hit"] == "1"), "printed verbatim;",
          sum(1 for r in rows if r["dead"] == "1"), "dead")


if __name__ == "__main__":
    main(*sys.argv[1:4])
