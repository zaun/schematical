"""plan-36-A Phase 3: draft specs.tsv rows for the newer Microchip datasheets.

usage: python3 -I specs-draft.py <cand.tsv> <txt-dir> > specs-draft.tsv

For every PDF that an in-scope AVR Dx / Ex / Lx / Sx, tinyAVR 0/1/2 or
megaAVR 0 row uses, reads the last "Product Identification System" figure
of the datasheet: the `CODE = description` entries under "Package Type" /
"Package Style" and under "Temperature Range". It writes one specs.tsv row
with the regex of that scheme, the codes being exactly the ones the figure
lists:

  AVR Dx/Ex/Lx/Sx      (?P<dev>AVR\\d+[A-Z]{2}\\d+)T?-(?P<temp>I|E)/(?P<pkg>SS|SO|...)(?:VAO)?
  tinyAVR, megaAVR 0   (?P<dev>ATTINY\\d+|ATMEGA\\d+)-(?P<pkg>SS|M|...)(?P<temp>N|F|U)R?(?:-VAO)?

The note gives the figure's line in the pdftotext -layout text and the codes'
descriptions. The draft is read against each figure before it is copied to
specs.tsv (Phase 3); verdicts.py also requires the `dev` group to name a
device the datasheet's text prints.
"""
import csv
import re
import sys
from collections import OrderedDict

MODERN = ("AVR Dx", "AVR Ex", "AVR Lx/Sx")
TINYMEGA = ("AVR tinyAVR 0/1/2", "AVR megaAVR 0")
ENTRY = re.compile(r"(?<![0-9A-Za-z])([0-9A-Z]{1,4})\s*=\s*(\S.*?)(?=\s{2,}|$)")
PKG_WORDS = re.compile(r"QFN|SOIC|SSOP|TSSOP|TQFP|PDIP|SPDIP|DIP|QFP|BGA|SOT|DFN", re.I)
TEMP_WORDS = re.compile(r"°C|\dC\b|C to", re.I)


def figure(lines):
    """(text line, package codes, temperature codes) of the last Product
    Identification System figure. Its entries are `CODE = description`,
    in one or two columns; a package entry names a package (QFN, SOIC, ...),
    a temperature entry a range in °C."""
    starts = [i for i, l in enumerate(lines) if "Product Identification System" in l and "...." not in l]
    if not starts:
        return None, {}, {}
    i = starts[-1]
    pkg, temp = OrderedDict(), OrderedDict()
    for line in lines[i:i + 45]:
        for m in ENTRY.finditer(line):
            code, desc = m.group(1), m.group(2)
            if code.isdigit() or re.search(r"Tape|Reel|Automotive|Industrial$|Standard|pins", desc, re.I):
                continue
            if TEMP_WORDS.search(desc):
                temp.setdefault(code, desc)
            elif PKG_WORDS.search(desc):
                pkg.setdefault(code, desc)
    return i + 1, pkg, temp


def table_codes(text, fam):
    """Fallback for a figure that is an image: the codes the ordering table's
    part numbers use (AVR...-I/SO -> SO)."""
    if fam in MODERN:
        found = re.findall(r"\bAVR\d+[A-Z]{2}\d+T?-([IE])/([0-9A-Z]+?)(?:VAO)?\b", text)
        return OrderedDict((p, "ordering table") for _, p in found), OrderedDict((t, "ordering table") for t, _ in found)
    return OrderedDict(), OrderedDict()


def alt(codes):
    return "|".join(sorted(codes, key=lambda c: (-len(c), c)))


def main(tsv, txt_dir):
    pdfs = OrderedDict()
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["scope"] == "in" and r["pdf"] and r["family"] in MODERN + TINYMEGA:
            pdfs.setdefault(r["pdf"], r["family"])
    print("pdf\tmpn_regex\tpackage_field\tnote")
    for h, fam in pdfs.items():
        lines = open(f"{txt_dir}/{h}.txt", errors="replace").read().split("\n")
        at, pkg, temp = figure(lines)
        src = "Product Identification System"
        tpkg, ttemp = table_codes("\n".join(lines).upper(), fam)
        if not pkg or not temp:
            pkg, temp = tpkg, ttemp
            src = "ordering table (the figure is an image)"
        else:
            more = [c for c in tpkg if c not in pkg]
            for c in more:
                pkg[c] = "ordering table"
            for c in ttemp:
                temp.setdefault(c, "ordering table")
            if more:
                src += " plus the ordering table's " + "/".join(more) + " (the figure omits them)"
        if not pkg or not temp:
            print(f"{h}\t\t\tDRAFT: no figure read ({fam})")
            continue
        if fam in MODERN:
            rx = rf"(?P<dev>AVR\d+[A-Z]{{2}}\d+)T?-(?P<temp>{alt(temp)})/(?P<pkg>{alt(pkg)})(?:VAO)?"
        else:
            rx = rf"(?P<dev>ATTINY\d+|ATMEGA\d+)-(?P<pkg>{alt(pkg)})(?P<temp>{alt(temp)})R?(?:-VAO)?"
        desc = "; ".join(f"{k}={v}" for k, v in pkg.items())
        print(f"{h}\t{rx}\tpkg\t{src}, text line {at}: {desc}")


if __name__ == "__main__":
    main(*sys.argv[1:3])
