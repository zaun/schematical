"""plan-29-A Phase 4, rule 5: datasheet status verdicts -> census/dead-verdicts.tsv.

usage: python3 -I dead-scan.py <candidates.tsv> <pdf-dir>        write dead-verdicts.tsv
       python3 -I dead-scan.py --other <candidates.tsv> <pdf-dir> print the other dead-word lines

textscan.py's `dead` looks only near the first place a part number is
printed, so a part named first in a ratings table and again under an
ordering table's "DISCONTINUED" heading slipped through (onsemi s2m-d.pdf:
S2A).  This reads every PDF of a row that has a datasheet and a verbatim
hit (decision add, candidate or check) and contains a dead word anywhere:

  onsemi ordering tables (pdftotext in reading order, not -layout: on
  two-column pages -layout puts the heading on a line of the other
  column): the lines after a "DISCONTINUED (Note n)" heading,
  up to the note itself ("n. DISCONTINUED: ...") or the page end, are the
  discontinued section.  A part printed there (token boundaries) and in no
  other ordering-table line of the PDF is `out`, word DISCONTINUED, that page.
  Every other row of such a PDF gets `add` (the word belongs to other parts).

A part named on a revision-history line that marks OPNs discontinued or
removed (and in no live ordering-table line) is `out`, word "discontinued
(revision history)".

Lines with OBSOLETE / NRND / NOT RECOMMENDED FOR NEW / END OF LIFE outside
onsemi's sections are printed by --other for a manual reading; their
verdicts (all reviewed) are in MANUAL below.
"""
import csv
import hashlib
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "dead-verdicts.tsv")
DEAD = re.compile(r"OBSOLETE|DISCONTINU(?!OUS)|NOT RECOMMENDED FOR NEW|END OF LIFE|NRND", re.I)
BROAD = re.compile(r"OBSOLETE|DISCONTINU|NOT RECOMMENDED FOR NEW|END OF LIFE|NRND", re.I)
HEADING = re.compile(r"^\s*DISCONTINUED\s*\(Note\s*\d+\)", re.I)
NOTE = re.compile(r"^\s*\d+\.\s*DISCONTINUED:", re.I)
ORDER = re.compile(r"ORDERING INFORMATION|ORDERING AND SHIPPING|Part Number|^\s*Device\b", re.I)


def norm(s):
    return " " + re.sub(r"[^0-9A-Z]+", " ", s.upper()).strip() + " "


def pages(pdf):
    out = subprocess.run(["pdftotext", pdf, "-"], capture_output=True, text=True,
                         errors="replace").stdout
    return out.split("\f")


TABLE_HEAD = re.compile(r"^\s*(Device|Part Number|Orderable Part|Order(ing)? (Part )?Number|Part No|Package|Marking|Shipping)", re.I)
REVISION = re.compile(r"discontinued|EOL|removal|removed", re.I)


def sections(pg):
    """(discontinued lines, live ordering-table lines, revision-history dead lines) of one page.

    A live table starts at "ORDERING INFORMATION" only when a table header
    (Device / Part Number / ...) follows within three non-blank lines (a
    "see page 2" pointer is not a table) and runs to a DISCONTINUED heading,
    a footnote, or the page end."""
    lines = pg.splitlines()
    dead, live, revision = [], [], []
    state = None
    for i, line in enumerate(lines):
        if HEADING.match(line):
            state = "dead"
            continue
        if NOTE.match(line) or line.lstrip().startswith("\u2020"):
            state = None
            continue
        if state != "dead" and re.search(r"ORDERING INFORMATION", line, re.I):
            ahead = [x for x in lines[i + 1:i + 8] if x.strip()][:3]
            state = "live" if any(TABLE_HEAD.match(x) for x in ahead) else None
            continue
        if state == "dead":
            dead.append(line)
        elif state == "live":
            live.append(line)
        if DEAD.search(line) or (REVISION.search(line) and re.search(r"discontinu|EOL", line, re.I)):
            revision.append(line + " " + (lines[i + 1] if i + 1 < len(lines) else ""))
    return dead, live, revision


def main(argv):
    other = argv[0] == "--other"
    if other:
        argv = argv[1:]
    tsv, pdf_dir = argv
    rows = [r for r in csv.DictReader(open(tsv), delimiter="\t")
            if (r["decision"] in ("add", "candidate", "check") or "datasheet status" in r["reason"])
            and r["pdf"] == "1" and r["hit"] == "1"]
    by_pdf = {}
    for r in rows:
        by_pdf.setdefault(r["datasheet_url"], []).append(r)
    verdicts = []
    for url, group in sorted(by_pdf.items()):
        pdf = os.path.join(pdf_dir, hashlib.md5(url.encode()).hexdigest() + ".pdf")
        pgs = pages(pdf)
        if not any(DEAD.search(p) for p in pgs):
            # Only textscan.py's broader word list matches (e.g. Vishay's
            # "Discontinuous Mode PFC"): the word names no part.
            if not other and any(BROAD.search(p) for p in pgs):
                verdicts.extend((r["mpn"], url, "", "", "add") for r in group)
            continue
        dead_txt = {}
        rev_txt = {}
        live_txt = ""
        for i, p in enumerate(pgs, 1):
            d, l, rv = sections(p)
            if d:
                dead_txt[i] = norm(" \n ".join(d))
            if rv:
                rev_txt[i] = [norm(x) for x in rv if REVISION.search(x) and re.search(r"OPN|device|product", x, re.I)]
            live_txt += norm(" \n ".join(l))
        if other:
            if not dead_txt:
                for i, p in enumerate(pgs, 1):
                    for line in p.splitlines():
                        if DEAD.search(line):
                            print(f"{os.path.basename(url)}\tp.{i}\t{line.strip()[:150]}")
            continue
        for r in group:
            key = norm(r["mpn"])
            page = next((i for i, t in dead_txt.items() if key in t), None)
            rev = next((i for i, ts in rev_txt.items() if any(key in t for t in ts)), None)
            if page is not None and key not in live_txt:
                verdicts.append((r["mpn"], url, str(page), "DISCONTINUED", "out"))
            elif rev is not None and key not in live_txt:
                verdicts.append((r["mpn"], url, str(rev), "discontinued (revision history)", "out"))
            else:
                verdicts.append((r["mpn"], url, "", "", "add"))
    if other:
        return 0
    with open(OUT, "w") as f:
        f.write("mpn\tpdf\tpage\tword\tverdict\n")
        for v in verdicts:
            f.write("\t".join(v) + "\n")
    print(len(verdicts), "verdicts,", sum(v[4] == "out" for v in verdicts), "out")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
