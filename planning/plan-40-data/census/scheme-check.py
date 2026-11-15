#!/usr/bin/env python3
"""plan-40-B section 3.1: the ordering-code schemes against the eligible rows.

usage: python3 -I scheme-check.py [--maker <maker>] [--list] [<candidates.tsv> [<specs.tsv>]]

Per maker: the eligible rows with a PDF, how many are printed verbatim
(`hit`), matched by their spec row's `scheme` only, and in neither (B skips
them: not in the datasheet's ordering table); then `<n> eligible rows
without a scheme` (a spec row whose `scheme` is empty; `-` means the PDF has
no ordering table and only a verbatim hit counts). --list prints each row
in neither, and each row without a scheme.
"""
import collections
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402


def main(argv):
    maker = None
    listing = "--list" in argv
    argv = [a for a in argv if a != "--list"]
    if "--maker" in argv:
        i = argv.index("--maker")
        maker = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    cand = argv[0] if argv else specs.CANDIDATES
    spec_path = argv[1] if len(argv) > 1 else specs.SPECS
    table = specs.load(spec_path)
    counts = collections.defaultdict(collections.Counter)
    missing = 0
    for r in csv.DictReader(open(cand), delimiter="\t"):
        if not common.eligible(r) or not r["pdf"] or (maker and r["maker"] != maker):
            continue
        s = table.get(specs.key(r))
        c = counts[r["maker"]]
        c["rows"] += 1
        if s is not None and s["scheme"] == "n/r":
            c["not read"] += 1
            continue
        if s is None or not s["scheme"]:
            missing += 1
            if listing:
                print("no scheme", r["mpn"], "|", r["series"], "|", r["pdf"], "|", r["package"])
            continue
        if r["hit"] == "1":
            c["verbatim"] += 1
        elif specs.matches(s, r["mpn"]):
            c["scheme"] += 1
        else:
            c["neither"] += 1
            if listing:
                print("neither", r["mpn"], "|", r["pdf"], "|", s["scheme"])
    for m in sorted(counts):
        c = counts[m]
        print(f"{m}: {c['rows']} rows with a PDF; {c['verbatim']} verbatim, {c['scheme']} by scheme, "
              f"{c['neither']} in neither, {c['not read']} not read (n/r)")
    print(f"{missing} eligible rows without a scheme")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
