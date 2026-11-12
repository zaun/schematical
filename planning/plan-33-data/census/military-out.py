"""plan-33-D correction: rows military by series name (decide.py's MILITARY) become `out`.

usage: python3 -I military-out.py

select.py tested only Ratings / Grade for MIL; KEMET files its MIL-spec parts
under series such as `Military, MIL-C-11015, CK12` and KYOCERA AVX under
`CWR11`, with no MIL in Ratings.  This sets every such row to `out`, reason
`military`, in candidates.tsv as B left it (no re-walk: the other buckets'
picks do not move, since a military row was `alt` everywhere but one bucket),
and prints the buckets that lost their pick.
"""
import csv
import importlib.util
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
spec = importlib.util.spec_from_file_location("decide", os.path.join(HERE, "decide.py"))
decide = importlib.util.module_from_spec(spec)
spec.loader.exec_module(decide)

path = os.path.join(DATA, "candidates.tsv")
rows, cols = decide.read(path)
lost, n = [], 0
for r in rows:
    if r["rank"] and decide.MILITARY.search(r["series"] + " " + r["ratings"]) and r["decision"] != "out":
        if r["decision"] == "add":
            lost.append(r["bucket"])
        r["decision"], r["reason"] = "out", "military"
        n += 1
decide.write(path, rows, cols)
print(n, "rows out (military);", len(lost), "buckets lost their pick:", ", ".join(lost))
