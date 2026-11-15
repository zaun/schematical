"""plan-40-B: what scheme-check.py, pinout-check.py, specs-fill.py and
verdicts.py share: reading specs.tsv, a row's spec, the scheme match, and
the pin count of a DigiKey Package / Case."""
import csv
import os
import re

import common

SPECS = os.path.join(common.DATA, "specs.tsv")
CANDIDATES = os.path.join(common.DATA, "candidates.tsv")
COLS = ["maker", "series", "pdf", "package", "rows", "example", "scheme", "label", "pinout", "elements"]
# SD: a half bridge's switch node, one element's source and the other's drain.
NAMES = re.compile(r"^(B|C|E|G|D|S|SD)([1-9]?)$|^NC$")
# scheme values that are no regex: `=` every row of the group is printed
# verbatim (no scheme needed); `-` the PDF has no ordering table (only a
# verbatim hit counts); `n/r` not read (the group is below the pick,
# plan-40-B Corrections 1).
NOT_REGEX = ("=", "-", "n/r")


def key(r):
    return (r["maker"], r["series"], r["pdf"], r["package"])


def load(path=SPECS):
    return {key(r): r for r in csv.DictReader(open(path), delimiter="\t")}


def matches(spec, mpn):
    """The scheme full-matches the part number ('-': the datasheet has no
    ordering table: only a verbatim hit counts)."""
    s = (spec or {}).get("scheme", "")
    if not s or s in NOT_REGEX:
        return False
    return re.fullmatch(s, mpn) is not None


def in_table(row, spec):
    return row.get("hit") == "1" or matches(spec, row["mpn"])


def pin_count(package):
    """The pins a DigiKey Package / Case string names, or None: `8-SOIC`,
    `TO-236-3, SC-59, SOT-23-3`, `6-TSSOP, SC-88, SOT-363`, `TO-252-3, DPak (2
    Leads + Tab)`. A count that several names give differently: the first."""
    for part in [p.strip() for p in package.split(",")]:
        m = re.match(r"^(\d+)-", part)
        if m:
            return int(m.group(1))
        m = re.match(r"^[A-Za-z]+-\d+[A-Z]*-(\d+)\b", part)   # TO-236-3, SOT-23-3, TO-220-3 Full Pack
        if m:
            return int(m.group(1))
        m = re.search(r"\bSO-(\d+)\b", part)                   # PowerPAK SO-8
        if m:
            return int(m.group(1))
    return None


def pinout_names(pinout):
    """The pin names of a pinout string in pin order ('A1=G' entries give their names)."""
    out = []
    for item in pinout.split(","):
        item = item.strip()
        if "=" in item:
            item = item.split("=", 1)[1]
        out.append(item)
    return out


def elements(text):
    """[(type, control, a, b)] of an `elements` string (`npn:B,C,E;pnp:B2,C2,E2`)."""
    out = []
    for e in text.split(";"):
        e = e.strip()
        if not e:
            continue
        t, rest = e.split(":", 1)
        names = [n.strip() for n in rest.split(",")]
        out.append((t.strip(), *names))
    return out
