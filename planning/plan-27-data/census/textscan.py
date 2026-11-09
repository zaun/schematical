"""plan-27 census, step 5: which candidate part numbers appear in their datasheet.

usage: python3 -I textscan.py <candidates.tsv> <pdf-dir> <out.tsv>

For each candidate / check row: runs pdftotext on the row's downloaded
datasheet (cached as <md5>.txt in <pdf-dir>), strips every non-alphanumeric
character from the text and from the part number, and writes `hit` = 1 when
the part number is found verbatim and stands on its own (`bounded`: not the
prefix or middle of a longer printed code; the text NFKC-normalised, so
full-width characters count).  `pdf` = 1 when the download is a PDF.
A miss is not a rejection (many datasheets give an ordering-code scheme, not a
list of part numbers); phase A reads those datasheets.
Adds columns: pdf, hit, dead (1 when an obsolete / discontinued / NRND / end-of-life
word stands within 300 characters of the part number in the datasheet text).
"""
import csv
import hashlib
import os
import re
import subprocess
import sys
import unicodedata


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


DEAD = re.compile("OBSOLETE|DISCONTINU|NOTRECOMMENDEDFORNEW|ENDOFLIFE|NRND")


def bounded(mpn, raw):
    """The part number stands on its own in the text: its letters and digits
    in order, at most two punctuation / space characters between them, and
    no letter or digit right before or after (so B3SE-1002 does not count
    where only B3SE-1002P is printed). plan-27-A Correction 6."""
    chars = [c for c in mpn if c.isalnum()]
    if not chars:
        return False
    pat = r"(?<![A-Za-z0-9])" + r"[^A-Za-z0-9\n]{0,2}".join(re.escape(c) for c in chars) + r"(?![A-Za-z0-9])"
    return re.search(pat, raw, re.I) is not None


def norm(s):
    return re.sub(r"[^0-9A-Z]", "", s.upper())


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
                    if text is None:
                        cache[url] = None
                    else:
                        nfkc = unicodedata.normalize("NFKC", text)
                        cache[url] = (norm(nfkc), nfkc)
                body, raw = cache[url] if cache[url] is not None else (None, None)
                if body is not None:
                    pdf = "1"
                    key = norm(row["mpn"])
                    at = body.find(key) if key else -1
                    hit = "1" if at >= 0 and bounded(row["mpn"], raw) else "0"
                    if at >= 0 and DEAD.search(body[max(0, at - 300):at + 300]):
                        dead = "1"
            row["pdf"], row["hit"], row["dead"] = pdf, hit, dead
            out.write("\t".join(row[c] for c in cols) + "\n")


if __name__ == "__main__":
    main(*sys.argv[1:4])
