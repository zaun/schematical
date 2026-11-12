"""plan-33-B section 3.3 (plan-29-A section 3 rule 5): which datasheets carry a status mark.

usage: python3 -I dead-scan.py <candidates.tsv> <pdf-dir>

textscan.py's `dead` looks only near a verbatim hit, and nearly every
capacitor part passes by its part-number scheme instead (plan-33-A section 2),
so this reads every placed PDF of a row with check = ok, page by page
(pdftotext -layout), for OBSOLETE / DISCONTINUED / NOT RECOMMENDED FOR NEW /
END OF LIFE / NRND.  It writes census/dead-pdfs.txt (one PDF path per line
with any such word) and prints each matching line with its page, for the
manual reading whose verdicts go in census/dead-verdicts.tsv (decide.py
refuses to pick an `ok` row from a listed PDF without a verdict line).
"""
import csv
import hashlib
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
PLACED = os.path.join(ROOT, "datasheets", "capacitor", "datasheets.tsv")
OUT = os.path.join(HERE, "dead-pdfs.txt")
DEAD = re.compile(r"OBSOLETE|DISCONTINU(?!OUS)|NOT RECOMMENDED FOR NEW|END[- ]OF[- ]LIFE|NRND", re.I)


def main(tsv, pdf_dir):
    paths = {p["url"]: p["path"] for p in csv.DictReader(open(PLACED), delimiter="\t")}
    urls = {}
    for r in csv.DictReader(open(tsv, newline="", encoding="utf-8"), delimiter="\t"):
        if r.get("check") == "ok" and r["datasheet_url"] in paths:
            urls.setdefault(paths[r["datasheet_url"]], r["datasheet_url"])
    found = []
    for path, url in sorted(urls.items()):
        pdf = os.path.join(pdf_dir, hashlib.md5(url.encode()).hexdigest() + ".pdf")
        text = subprocess.run(["pdftotext", "-layout", pdf, "-"], capture_output=True,
                              text=True, errors="replace").stdout
        hit = False
        for i, page in enumerate(text.split("\f"), 1):
            for line in page.splitlines():
                if DEAD.search(line):
                    hit = True
                    print(f"{path}\tp.{i}\t{line.strip()[:170]}")
        if hit:
            found.append(path)
    with open(OUT, "w") as f:
        f.write("".join(p + "\n" for p in found))
    print(len(urls), "PDFs read,", len(found), "with a status word")
    return 0


if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:3]))
