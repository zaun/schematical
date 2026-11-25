"""plan-58-C Phase 1: decide a maker's `pending` rows with the datasheets' part-number keys.

usage: python3 -I scheme-check.py --maker 'C&K'    decide that maker's pending rows
       python3 -I scheme-check.py --selftest

planning/plan-58-data/schemes.tsv has the columns `maker series regex source`:
an anchored Python regular expression over the part number, built from the
positions and allowed letters of the datasheet's part-number key, and where the
key is (PDF file and page).  For every row of the maker whose `decision` is
`pending` the script finds the scheme of its series (error, exit 1, when the
series has none) and sets `decision` to `add` when the part number
fullmatches the regex, else to `skip` with the reason
`number does not fit the datasheet's part-number key`.  It rewrites
candidates.tsv and the "Slide switches (plan-58)" section of
datasheets/skipped-parts.md (decide.py's writer).  Rows that are not `pending`
are not touched, so a second run changes nothing.
"""
import csv
import importlib.util
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
CAND = os.path.join(DATA, "candidates.tsv")
SCHEMES = os.path.join(DATA, "schemes.tsv")
REASON = "number does not fit the datasheet's part-number key"


def read(path):
    with open(path) as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    return rows, list(rows[0].keys())


def write(path, rows, cols):
    with open(path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")


def load_schemes(path, maker):
    out = {}
    for r in csv.DictReader(open(path), delimiter="\t"):
        if r["maker"] == maker:
            out.setdefault(r["series"], []).append(re.compile(r["regex"]))
    return out


def decide(rows, schemes, maker):
    """Rewrites the maker's pending rows in place; returns the series without a scheme."""
    missing = set()
    for r in rows:
        if r["maker"] != maker or r["decision"] != "pending":
            continue
        regexes = schemes.get(r["series"])
        if regexes is None:
            missing.add(r["series"])
        elif any(rx.fullmatch(r["mpn"]) for rx in regexes):
            r["decision"], r["reason"] = "add", ""
        else:
            r["decision"], r["reason"] = "skip", REASON
    return missing


def decide_module():
    spec = importlib.util.spec_from_file_location("decide58", os.path.join(HERE, "decide.py"))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def main(maker):
    rows, cols = read(CAND)
    missing = decide(rows, load_schemes(SCHEMES, maker), maker)
    if missing:
        for s in sorted(missing):
            print(f"no scheme for {maker} series {s!r}")
        return 1
    write(CAND, rows, cols)
    mine = [r for r in rows if r["maker"] == maker]
    print(maker, sum(1 for r in mine if r["decision"] == "add"), "add,",
          sum(1 for r in mine if r["decision"] == "skip"), "skip,",
          sum(1 for r in mine if r["decision"] == "pending"), "pending")
    return decide_module().write_skipped()


def selftest():
    rows = [{"maker": "M", "series": "S", "mpn": mpn, "decision": d, "reason": ""}
            for mpn, d in (("S101031MS02Q", "pending"), ("S101031MS09Q", "pending"), ("S101031MS02QE", "add"))]
    schemes = {"S": [re.compile(r"S10[12]03[12]MS02Q")]}
    assert decide(rows, schemes, "M") == set()
    assert [r["decision"] for r in rows] == ["add", "skip", "add"], rows
    assert rows[1]["reason"] == REASON
    assert decide(rows, schemes, "M") == set()          # a second run changes nothing
    assert [r["decision"] for r in rows] == ["add", "skip", "add"]
    other = [{"maker": "M", "series": "X", "mpn": "A", "decision": "pending", "reason": ""}]
    assert decide(other, schemes, "M") == {"X"}          # a series without a scheme
    assert decide(other, schemes, "N") == set() and other[0]["decision"] == "pending"   # another maker
    print("ok")
    return 0


if __name__ == "__main__":
    a = sys.argv[1:]
    if a == ["--selftest"]:
        sys.exit(selftest())
    if len(a) == 2 and a[0] == "--maker":
        sys.exit(main(a[1]))
    print(__doc__)
    sys.exit(2)
