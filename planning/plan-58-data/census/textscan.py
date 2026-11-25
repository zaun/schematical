"""plan-58-A Phase 2: which candidate part numbers appear in their datasheet.

usage: python3 -I textscan.py <candidates.tsv> <pdf-dir> <out.tsv>
       python3 -I textscan.py --selftest

For each row whose `decision` is `add`: runs pdftotext -layout on the row's
downloaded datasheet (cached as <md5>.txt in <pdf-dir>), and writes `hit` = 1
when the part number is in the text verbatim (case and spacing ignored) and
stands on its own: its letters and digits in order, at most two punctuation or
space characters between them, and no letter or digit right before or after
(so `AS12AP` does not count where only `AS12APC` is printed; plan-27-A
Correction 6).  The text is NFKC-normalised, so full-width characters count.
A miss is not a rejection: a slide switch's number is built from an ordering
key (`OS 1 0 2 0 1 1 M S 2 Q N 1`), and C-F decide those rows (`pending`).
Adds columns (plan-58-A section 4.1): pdf (1 when the download is a PDF), hit,
dead (1 when "obsolete", "discontinued", "NRND" or "end of life" stands within
300 characters of the number).  Rows that are not `add` get 0 0 0.
"""
import csv
import hashlib
import os
import re
import subprocess
import sys
import unicodedata

DEAD = re.compile(r"OBSOLETE|DISCONTINU|NOTRECOMMENDEDFORNEW|NRND|ENDOFLIFE")


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
        subprocess.run(["pdftotext", "-layout", "-q", pdf, txt], check=False, timeout=180)
    if not os.path.exists(txt):
        return None
    return open(txt, errors="replace").read()


def norm(s):
    return re.sub(r"[^0-9A-Z]", "", s.upper())


def bounded(mpn, raw):
    chars = [c for c in mpn if c.isalnum()]
    if not chars:
        return False
    pat = r"(?<![A-Za-z0-9])" + r"[^A-Za-z0-9\n]{0,2}".join(re.escape(c) for c in chars) + r"(?![A-Za-z0-9])"
    return re.search(pat, raw, re.I) is not None


def scan(mpn, raw):
    """(hit, dead) of one part number in one document's text."""
    nfkc = unicodedata.normalize("NFKC", raw)
    body = norm(nfkc)
    key = norm(mpn)
    at = body.find(key) if key else -1
    hit = at >= 0 and bounded(mpn, nfkc)
    dead = at >= 0 and DEAD.search(body[max(0, at - 300):at + 300]) is not None
    return ("1" if hit else "0"), ("1" if dead else "0")


def main(tsv, directory, out_path):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    cache = {}
    cols = list(rows[0].keys()) + ["pdf", "hit", "dead"]
    with open(out_path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for row in rows:
            pdf, hit, dead = "0", "0", "0"
            if row["decision"] == "add" and row["datasheet"]:
                url = row["datasheet"]
                if url not in cache:
                    cache[url] = text_of(directory, url)
                if cache[url] is not None:
                    pdf = "1"
                    hit, dead = scan(row["mpn"], cache[url])
            row["pdf"], row["hit"], row["dead"] = pdf, hit, dead
            out.write("\t".join(row[c] for c in cols) + "\n")
    return 0


def selftest():
    # verbatim
    assert scan("MSN13ASG40", "Ordering: MSN13ASG40 is a slide switch") == ("1", "0")
    # spaced / lower case / punctuated (the ordering key prints digits apart)
    assert scan("OS102011MS2QN1", "Part OS 102011 MS 2Q N1 stocked")[0] == "1"
    assert scan("os102011ms2qn1", "OS102011MS2QN1")[0] == "1"
    # a prefix of a longer printed code is not a hit
    assert scan("AS12AP", "Only AS12APC is listed")[0] == "0"
    assert scan("AS12AP", "no such code")[0] == "0"
    # obsolete next to the number
    assert scan("AS12AP", "AS12AP  -  OBSOLETE, use AS12AP-B")[1] == "1"
    assert scan("AS12AP", "AS12AP active. " + "x" * 400 + " obsolete series")[1] == "0"
    # full-width characters count
    assert scan("AS12AP", "ＡＳ１２ＡＰ")[0] == "1"
    print("ok")
    return 0


if __name__ == "__main__":
    if sys.argv[1:] == ["--selftest"]:
        sys.exit(selftest())
    if len(sys.argv) != 4:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(*sys.argv[1:4]))
