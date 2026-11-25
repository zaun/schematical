"""plan-58-C: merge filled spec fragments into specs.tsv.

usage: python3 -I merge-specs.py <fragment.tsv>...
       python3 -I merge-specs.py --prune <maker>    drop the maker's still-empty skeleton rows whose
                                                     line (series, mounting, circuit) has no `add` row
                                                     in candidates.tsv any more (its pending rows were skipped)
       python3 -I merge-specs.py --selftest

A fragment has specs.tsv's header and its filled rows.  For every
(maker, series, mounting, circuit) a fragment names, the skeleton rows (or earlier
filled rows) of that key in specs.tsv are replaced by the fragment's rows (a key
may have several rows with different mpn_regex).  A fragment key that specs.tsv
does not have is an error (exit 1): the skeleton is the list of lines to fill.
The file is rewritten sorted by (maker, series, mounting, circuit, mpn_regex).
"""
import csv
import os
import sys

DATA = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SPECS = os.path.join(DATA, "specs.tsv")
COLS = ["maker", "series", "mounting", "circuit", "mpn_regex", "terminals", "pins", "footprint", "note"]
KEY = ("maker", "series", "mounting", "circuit")


def key(r):
    return tuple(r[k] for k in KEY)


def merge(base, fragments):
    have = {key(r) for r in base}
    new = {}
    for r in fragments:
        if key(r) not in have:
            raise SystemExit("not a line of specs.tsv: " + " / ".join(key(r)))
        new.setdefault(key(r), []).append(r)
    out = [r for r in base if key(r) not in new] + [r for rows in new.values() for r in rows]
    return sorted(out, key=lambda r: key(r) + (r["mpn_regex"],))


def prune(maker):
    cand = os.path.join(DATA, "candidates.tsv")
    live = {(r["maker"], r["series"], r["mounting"], r["circuit"])
            for r in csv.DictReader(open(cand), delimiter="\t") if r["decision"] == "add"}
    base = list(csv.DictReader(open(SPECS), delimiter="\t"))
    keep = [r for r in base if r["maker"] != maker or r["mpn_regex"] or key(r) in live]
    with open(SPECS, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for r in keep:
            out.write("\t".join(r[c] for c in COLS) + "\n")
    print(len(base) - len(keep), "empty rows of", maker, "dropped;", len(keep), "rows in specs.tsv")
    return 0


def main(paths):
    base = list(csv.DictReader(open(SPECS), delimiter="\t"))
    frags = []
    for p in paths:
        frags += list(csv.DictReader(open(p), delimiter="\t"))
    rows = merge(base, frags)
    with open(SPECS, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in COLS) + "\n")
    print(len(frags), "fragment rows merged;", len(rows), "rows in specs.tsv")
    return 0


def selftest():
    def r(series, circ, rx=""):
        return dict(zip(COLS, ["M", series, "TH", circ, rx, "", "", "", ""]))
    base = [r("A", "SPDT"), r("A", "DPDT"), r("B", "SPDT")]
    got = merge(base, [r("A", "SPDT", "x1"), r("A", "SPDT", "x2")])
    assert [(g["series"], g["circuit"], g["mpn_regex"]) for g in got] == \
        [("A", "DPDT", ""), ("A", "SPDT", "x1"), ("A", "SPDT", "x2"), ("B", "SPDT", "")]
    try:
        merge(base, [r("C", "SPDT", "x")])
    except SystemExit:
        pass
    else:
        raise AssertionError("an unknown line must be refused")
    print("ok")
    return 0


if __name__ == "__main__":
    if sys.argv[1:] == ["--selftest"]:
        sys.exit(selftest())
    if len(sys.argv) == 3 and sys.argv[1] == "--prune":
        sys.exit(prune(sys.argv[2]))
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1:]))
