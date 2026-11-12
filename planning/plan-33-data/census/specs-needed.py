"""plan-33-B Phase 1 helper: which spec rows the pick walk reaches next.

usage: python3 -I specs-needed.py [--all]

Walks every bucket in rank order over the rows scheme-check.py checked
(column `check`), as decide.py does, and stops at the first row whose PDF's
specs.tsv row has no mpn_regex yet (the scheme decides that row, so it must
be read) or at the first `ok` row.  Prints, per such PDF, the buckets
waiting on it and three sample part numbers, most-waited first.  --all also
prints every spec row no walk has reached.
"""
import collections
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))


def main(argv):
    paths = {p["url"]: p["path"] for p in csv.DictReader(
        open(os.path.join(ROOT, "datasheets", "capacitor", "datasheets.tsv")), delimiter="\t")}
    specs = {s["datasheet"]: s for s in csv.DictReader(open(os.path.join(DATA, "specs.tsv")), delimiter="\t")}
    rows = list(csv.DictReader(open(os.path.join(DATA, "candidates.tsv"), newline="", encoding="utf-8"),
                               delimiter="\t"))
    by = collections.defaultdict(list)
    for r in rows:
        if r["rank"] and r["decision"] not in ("out",):
            by[r["bucket"]].append(r)
    waiting = collections.defaultdict(list)
    reached = set()
    unscanned = 0
    for b, g in by.items():
        g.sort(key=lambda r: int(r["rank"]))
        for r in g:
            if r.get("check", "") == "":
                unscanned += 1
                break
            path = paths.get(r["datasheet_url"])
            if r["check"] == "no maker pdf" or path is None:
                continue
            reached.add(path)
            if r["hit"] != "1" and not specs[path]["mpn_regex"]:
                waiting[path].append(r)
                break
            if r["check"] == "ok":
                break
    for path, g in sorted(waiting.items(), key=lambda kv: -len(kv[1])):
        print(f"{len(g):4}  {path}  {', '.join(r['mpn'] for r in g[:3])}")
    print(len(waiting), "PDFs block", sum(len(g) for g in waiting.values()), "buckets;",
          unscanned, "buckets reach an unscanned rank")
    if "--all" in argv:
        for p in sorted(set(specs) - reached):
            print("not reached:", p)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
