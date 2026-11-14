#!/usr/bin/env python3
"""plan-34-A § Corrections 7: confirm picked rows from a fresh relisting.

usage: python3 -I stock-merge.py <raw dir> <candidates.tsv> <relist dir> <stock-check.tsv>

Each unchecked picked row from a cut piece (cut-picked.py) whose exact part
number is in the relisting (top.js JSON lines, listed today) with stock
above 0 and status Active or Last Time Buy gets a stock-check.tsv row
`found` 1 with that stock, status and today's date. Rows not in the
relisting stay unchecked for stock-check.sh (one call each).
"""
import datetime
import glob
import json
import os
import subprocess
import sys

raw, cand, relist, out = sys.argv[1:5]
here = os.path.dirname(os.path.abspath(__file__))
fresh = {}
for path in glob.glob(os.path.join(relist, "*.jsonl")):
    for line in open(path):
        if line.strip():
            d = json.loads(line)
            fresh[d["mpn"]] = d
todo = subprocess.run(["python3", "-I", os.path.join(here, "cut-picked.py"), raw, cand, out],
                      capture_output=True, text=True, check=True).stdout.split("\n")
today = datetime.date.today().isoformat()
done = 0
with open(out, "a") as f:
    for line in todo:
        if not line:
            continue
        mpn = line.split("\t")[1]
        d = fresh.get(mpn)
        if d and d["stock"] > 0 and d["status"] in ("Active", "Last Time Buy"):
            f.write("\t".join([mpn, "1", str(d["stock"]), d["status"], today]) + "\n")
            done += 1
left = sum(1 for l in todo if l) - done
print(f"stock-merge: {done} confirmed from the relisting ({len(fresh)} parts listed); {left} still unchecked")
