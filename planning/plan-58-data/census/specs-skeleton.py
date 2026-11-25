"""plan-58-A Phase 3: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <candidates.tsv> <specs.tsv>
       python3 -I specs-skeleton.py --selftest

One row per distinct (maker, series, mounting, circuit) of the `add` and
`pending` rows: `terminals` is the sorted DigiKey `termination` values of the
rows (for the reader); `mpn_regex`, `pins`, `footprint`, `note` are empty
(plan-58-C..F fill them; formats in plan-58-A section 4.2).  Refuses to
overwrite a specs.tsv that already has a filled row.
"""
import csv
import os
import sys

COLS = ["maker", "series", "mounting", "circuit", "mpn_regex", "terminals", "pins", "footprint", "note"]


def skeleton(rows):
    groups = {}
    for r in rows:
        if r["decision"] in ("add", "pending"):
            groups.setdefault((r["maker"], r["series"], r["mounting"], r["circuit"]), set()).add(r["termination"])
    return [list(k) + ["", "; ".join(sorted(groups[k])), "", "", ""] for k in sorted(groups)]


def main(src, dst):
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter="\t"):
            if r["mpn_regex"] or r["pins"] or r["footprint"] or r["note"]:
                print("refused: specs.tsv already has filled rows")
                return 1
    out_rows = skeleton(list(csv.DictReader(open(src), delimiter="\t")))
    with open(dst, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for row in out_rows:
            out.write("\t".join(row) + "\n")
    print(len(out_rows), "rows")
    return 0


def selftest():
    rows = [{"maker": "M", "series": "S", "mounting": "TH", "circuit": "SPDT", "termination": "PC Pin", "decision": "add"},
            {"maker": "M", "series": "S", "mounting": "TH", "circuit": "SPDT", "termination": "Solder Lug", "decision": "pending"},
            {"maker": "M", "series": "S", "mounting": "TH", "circuit": "DPDT", "termination": "PC Pin", "decision": "add"},
            {"maker": "M", "series": "S", "mounting": "Panel Mount", "circuit": "DPDT", "termination": "PC Pin", "decision": "out"}]
    got = skeleton(rows)
    assert len(got) == 2 and got[0][3] == "DPDT" and got[0][5] == "PC Pin" and got[1][3] == "SPDT", got
    spdt = [g for g in got if g[3] == "SPDT"][0]
    assert spdt[5] == "PC Pin; Solder Lug" and spdt[4] == "" and len(spdt) == len(COLS)
    print("ok")
    return 0


if __name__ == "__main__":
    if sys.argv[1:] == ["--selftest"]:
        sys.exit(selftest())
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(*sys.argv[1:3]))
