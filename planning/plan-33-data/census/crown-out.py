"""plan-33-D correction: KEMET PHH22x / PHV22x hybrid "crown" capacitors are `out`.

usage: python3 -I crown-out.py

PHH227 / PHH223 (radial crown: a ring of tabs round the + pin, KEM_A4115_PHH227.pdf
p.3) and PHV227 / PHV223 (vertical SMD on a four-pad land, KEM_A4136_PHV227.pdf
p.8) are not two-lead parts, which the capacitor kind's pinouts are (plan-33-E
section 3.2); their datasheets do not say which tabs / pads are which pole.
Sets every such row to `out`, reason `crown terminals (not a two-lead part)`,
without a re-walk (their buckets' other rows are the same families), and
prints the buckets that lost their pick.
"""
import importlib.util
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
spec = importlib.util.spec_from_file_location("decide", os.path.join(HERE, "decide.py"))
decide = importlib.util.module_from_spec(spec)
spec.loader.exec_module(decide)

path = os.path.join(DATA, "candidates.tsv")
rows, cols = decide.read(path)
lost, n = [], 0
for r in rows:
    if r["rank"] and re.match(r"PH[HV]22\d", r["mpn"]) and r["decision"] != "out":
        if r["decision"] == "add":
            lost.append(r["bucket"])
        r["decision"], r["reason"] = "out", "crown terminals (not a two-lead part)"
        n += 1
decide.write(path, rows, cols)
print(n, "rows out (crown);", len(lost), "buckets lost their pick:", ", ".join(lost))
