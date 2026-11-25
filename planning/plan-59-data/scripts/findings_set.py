#!/usr/bin/env python3
"""findings_set.py <G|H> <rows.txt>: fills verdict cells of findings.md.
rows.txt holds blocks:  ID<TAB>verdict<TAB>evidence (one line each; no '|' allowed)."""
import sys, re
col, rows = sys.argv[1], sys.argv[2]
path = "/Users/justinzaun/Development/logic/.claude/worktrees/P-59/planning/plan-59-data/findings.md"
lines = open(path).read().split("\n")
done = 0
for raw in open(rows).read().split("\n"):
    if not raw.strip() or raw.startswith("#"):
        continue
    pid, verdict, evidence = raw.split("\t", 2)
    assert "|" not in evidence, f"{pid}: evidence holds a pipe"
    assert verdict in ("works", "workaround", "blocked"), verdict
    for i, line in enumerate(lines):
        cells = line.split("|")
        if len(cells) == 8 and cells[1].strip() == pid:
            k = 3 if col == "G" else 5
            cells[k] = f" {verdict} "
            cells[k + 1] = f" {evidence.strip()} "
            lines[i] = "|".join(cells)
            done += 1
            break
    else:
        sys.exit(f"no row {pid}")
open(path, "w").write("\n".join(lines))
print(f"{done} rows set for {col}")
