"""plan-36-A Phase 2: which in-scope part numbers print verbatim in their datasheet.

usage: python3 -I textscan.py <cand.tsv> <pdf-dir> <txt-dir>

Adapted from planning/plan-29-data/census/textscan.py. Runs `pdftotext
-layout` once per PDF (<txt-dir>/<md5>.txt) and rewrites <cand.tsv> in
place, filling for each `scope` = in row:

  pdf           the md5 of the row's datasheet URL when that PDF is on disk
                (place.py turns it into the tree path in Phase 4)
  hit           `verbatim` when the part number prints between token
                boundaries, case aside, with an optional footnote mark
                `(n)` after it; else empty (verdicts.py tries the regex)
  package_code  for a verbatim hit on a line of the ordering table: the
                first later token on that line that the datasheet's
                "Package Type" legend defines (`32A`, `28M1`, `32M1-A`)
  dead          1 when OBSOLETE / DISCONTINUED / NOT RECOMMENDED FOR NEW /
                END OF LIFE / NRND stands within 300 characters of the hit,
                or on the first page; Phase 4 reads each such row by hand

Only a URL on the maker's own site counts (§ 3 rule 3); a distributor copy
leaves pdf empty for sibling.py. A row whose PDF prints neither its part number nor its device name has the
wrong document (DigiKey links a PIC18F datasheet for some AVR128DA28 rows);
its pdf is left empty for sibling.py. A row whose pdf is already set to something other than its own URL's md5
(sibling.py's choice) is scanned against that PDF.
"""
import csv
import hashlib
import os
import re
import subprocess
import sys
import urllib.parse

DEAD = re.compile(r"OBSOLETE|DISCONTINU|NOT RECOMMENDED FOR NEW|END[- ]OF[- ]LIFE|\bNRND\b", re.I)
# A "Package Type" legend line: a code, two or more spaces, then "<n>-lead" / "-pad" / "-ball" / "-pin" ...
# The code holds a digit (`32A`, `S8S1`) or is 2-3 capitals (`PW`, `ML`); Atmel's `A2 64` keeps its first token.
LEGEND = re.compile(r"^[ \t]*((?-i:(?=[A-Z-]*\d)[0-9A-Z][0-9A-Z-]{1,9}|[A-Z]{2,3}))(?: \d{2,3})?[ \t]+"
                    r"(\d+[ \t]?[- ][ \t]?(?:lead|pad|ball|pin|bump)|Plastic|Thin|Very|Ultra)", re.I | re.M)


OWN = ("microchip.com", "espressif.com", "raspberrypi.com")


def own_site(url):
    """plan-36-A § 3 rule 3: a link on the maker's own site (documentation.espressif.com is under espressif.com)."""
    u = "https:" + url if url.startswith("//") else url
    host = urllib.parse.urlparse(u).netloc.lower()
    return any(host == x or host.endswith("." + x) for x in OWN)


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def text(pdf_dir, txt_dir, h):
    pdf = os.path.join(pdf_dir, h + ".pdf")
    txt = os.path.join(txt_dir, h + ".txt")
    if not os.path.exists(pdf):
        return None
    if not os.path.exists(txt):
        subprocess.run(["pdftotext", "-q", "-layout", pdf, txt], check=False, timeout=600)
    if not os.path.exists(txt):
        return None
    return dashes(open(txt, errors="replace").read())


def dashes(body):
    """Atmel tables print some part numbers with an en dash (`ATmega128–16AN`)."""
    return body.replace("\u2013", "-").replace("\u2011", "-").replace("\u2010", "-")


def legend(body):
    """code -> description, from the datasheet's "Package Type" legends:
    `32A  32-lead, ... (TQFP)` lines, and (Atmel-era layouts that put the
    code on a line of its own, `44ML` under `ML, 44 - Lead, ...`) a line that
    is only a pin-count-led code within 40 lines after a "Package Type"
    heading."""
    out = {}
    for m in LEGEND.finditer(body):
        line_end = body.find("\n", m.start(1))
        out.setdefault(m.group(1).upper(), body[m.start(2): line_end if line_end >= 0 else len(body)].strip())
    lines = body.split("\n")
    for i, line in enumerate(lines):
        if re.search(r"Package\s+Type", line, re.I):
            for later in lines[i + 1: i + 41]:
                m = re.match(r"^\s*(\d{1,3}[A-Z][A-Z0-9-]{0,6})\s*$", later.upper())
                if m:
                    out.setdefault(m.group(1), "")
    return out


def legend_pins(desc, code=""):
    m = re.match(r"(\d+)[ \t]?[- ][ \t]?(?:lead|pad|ball|pin|bump)", desc, re.I)
    if m:
        return int(m.group(1))
    m = re.match(r"(\d+)", code)
    return int(m.group(1)) if m else None


def dk_pins(row):
    m = re.match(r"(\d+)", row.get("dk_package") or "")
    return int(m.group(1)) if m else None


WINDOW = 12
# Families whose ordering tables name an Atmel-style package code (`32A`, `28M1`).
# The newer ones (AVR Dx/Ex/Lx/Sx, tinyAVR 0/1/2, megaAVR 0) spell the package
# in the part number itself, and verdicts.py reads it with specs.tsv's regex.
CLASSIC = ("AVR AT90", "AVR ATmega", "AVR ATtiny", "AVR XMEGA")


def pattern(mpn):
    return re.compile(r"(?<![A-Z0-9/-])" + re.escape(mpn.upper()) + r"(?:\(\d+\))*(?![A-Z0-9])")


def codes_on(line, codes):
    return [t for t in re.findall(r"[0-9A-Z][0-9A-Z-]*", line) if t in codes]


def near(up, m, codes, want):
    """The package code for the hit m: a legend code on its own line, else the
    nearest within WINDOW lines (legend lines aside), preferring DigiKey's pin
    count. (code, own line?) or None."""
    start = up.rfind("\n", 0, m.start()) + 1
    end = up.find("\n", m.end())
    end = len(up) if end < 0 else end
    own = codes_on(up[m.end():end], codes) or codes_on(up[start:m.start()], codes)
    if own:
        return own[0], True
    lines_before = up[max(0, start - 4000):start].split("\n")[-WINDOW - 1:-1]
    lines_after = up[end + 1:end + 4000].split("\n")[:WINDOW]
    found = []
    for d in range(1, WINDOW + 1):
        for line in ([lines_before[-d]] if d <= len(lines_before) else []) + \
                    ([lines_after[d - 1]] if d <= len(lines_after) else []):
            if LEGEND.match(line):
                continue
            for c in codes_on(line, codes):
                found.append((d, c))
    good = [x for x in found if legend_pins(codes[x[1]], x[1]) == want]
    pick = (good or found)[:1]
    return (pick[0][1], False) if pick else None


def scan(row, body, codes):
    """hit, package_code, dead for one row. Every occurrence of the part
    number is tried: the first whose own line holds a legend code wins, else
    the first that has one nearby (an ordering table whose package cell spans
    several rows). pkgcheck.py checks every code against DigiKey."""
    up = body.upper()
    hit, pkg, dead = "", "", "0"
    first = up.split("\f", 1)[0]
    fallback = None
    want = dk_pins(row)
    for k, m in enumerate(pattern(row["mpn"]).finditer(up)):
        if k >= 60:
            break
        hit = "verbatim"
        if DEAD.search(up[max(0, m.start() - 300): m.end() + 300]):
            dead = "1"
        if not codes:
            continue
        got = near(up, m, codes, want)
        if got and got[1]:
            pkg = got[0]
            break
        if got and fallback is None:
            fallback = got[0]
    if hit and not pkg and fallback:
        pkg = fallback
    if DEAD.search(first):
        dead = "1"
    return hit, pkg, dead


def names(up):
    """Every name-like token of an upper-cased text, with list titles
    expanded: `ATMEGA8U2/16U2/32U2` also names ATMEGA16U2 and ATMEGA32U2,
    `AVR16/32DD28/32` names AVR16DD28 ... AVR32DD32."""
    out = set(re.findall(r"[0-9A-Z]+(?:-[0-9A-Z]+)*", up))
    out |= set(re.findall(r"[0-9A-Z]+", up))
    for m in re.finditer(r"\b(ATXMEGA|ATMEGA|ATTINY|AT90[A-Z]*)(\d[0-9A-Z]*(?:/\d[0-9A-Z]*)+)", up):
        out |= {m.group(1) + x for x in m.group(2).split("/")}
    for m in re.finditer(r"AVR((?:\d+/)*\d+)([A-Z]{2})((?:\d+/)*\d+)", up):
        out |= {f"AVR{f}{m.group(2)}{q}" for f in m.group(1).split("/") for q in m.group(3).split("/")}
    return out


def main(tsv, pdf_dir, txt_dir):
    os.makedirs(txt_dir, exist_ok=True)
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    cols = list(rows[0].keys())
    cache = {}
    n = {"pdf": 0, "verbatim": 0, "pkg": 0, "dead": 0, "wrong": 0}
    for r in rows:
        if r["scope"] != "in":
            continue
        url = r["datasheet_url"]
        h = r["pdf"] if r["pdf"] else (md5(url) if url and own_site(url) else "")
        if h and h not in cache:
            body = text(pdf_dir, txt_dir, h)
            cache[h] = None if body is None else (body, legend(body), names(body.upper()))
        if not h or cache[h] is None:
            r["pdf"], r["hit"], r["package_code"], r["dead"] = "", "", "", ""
            continue
        body, codes, named = cache[h]
        r["pdf"] = h
        r["hit"], r["package_code"], r["dead"] = scan(r, body, codes if r["family"] in CLASSIC else {})
        if not r["hit"] and r["device"].upper() not in named:
            # The wrong document (DigiKey links a PIC18F datasheet for some
            # AVR128DA28 rows): neither the part number nor the device prints
            # in it, so sibling.py looks for the right one.
            r["pdf"], r["hit"], r["package_code"], r["dead"] = "", "", "", ""
            n["wrong"] += 1
            continue
        n["pdf"] += 1
        n["verbatim"] += r["hit"] == "verbatim"
        n["pkg"] += bool(r["package_code"])
        n["dead"] += r["dead"] == "1"
    with open(tsv, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    print(f"in-scope rows with a PDF {n['pdf']}, verbatim {n['verbatim']} (with package code {n['pkg']}), dead {n['dead']}, wrong PDF (neither the part number nor the device prints) {n['wrong']}")


if __name__ == "__main__":
    main(*sys.argv[1:4])
