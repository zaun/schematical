#!/usr/bin/env python3
"""plan-42-B Phase 4: one add row per distinct (maker, scheme) with the page
and line of its PDF that prints it.

usage: python3 -I scheme-handcheck.py > scheme-handcheck.txt

For each distinct (maker, scheme) of the `add` rows that are not printed
verbatim: the most-stocked such row, and the first line of its PDF, on a page
after an ordering-information heading, that prints the row's name once the
maker's packing suffix is removed (auto-scheme.py's cores). A pattern document
(voltage codes, `xy` wildcards) prints NO CORE FOUND: those, and the older
Linear Technology "PACKAGE/ORDER INFORMATION" tables whose heading the
extraction garbles, are read by hand and recorded in plan-42-B Corrections.
"""
import collections
import importlib.util
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.append(HERE)
import common  # noqa: E402
import specs  # noqa: E402

root = common.ROOT
rows = common.load(os.path.join(common.DATA, "candidates.tsv"))
common.init(rows)
table = specs.load(os.path.join(common.DATA, "specs.tsv"))
by = collections.OrderedDict()
for r in rows:
    if r["reason"] == "add" and r["hit"] != "1":
        s = table[specs.key(r)]
        by.setdefault((r["maker"], s["scheme"]), []).append(r)
print(len(by), "distinct (maker, scheme);", sum(len(v) for v in by.values()), "add rows by scheme")
spec = importlib.util.spec_from_file_location("autos", os.path.join(HERE, "auto-scheme.py"))
autos = importlib.util.module_from_spec(spec)
spec.loader.exec_module(autos)
HEAD = re.compile(r"ORDER(ING)? (INFORMATION|GUIDE)|PRODUCT IDENTIFICATION|PACKAGE OPTION|ORDERABLE|PART NUMBER", re.I)
for i, (k, rs) in enumerate(by.items()):
    r = max(rs, key=lambda r: int(r["stock"]))
    t = subprocess.run(["pdftotext", "-q", "-layout", os.path.join(root, r["pdf"]), "-"],
                       capture_output=True, text=True).stdout
    ev = None
    for c in autos.cores(r["mpn"], r["maker"]):
        key = common.norm(c)
        for pn, pg in enumerate(t.split("\f"), 1):
            if key not in common.norm(pg):
                continue
            seen = False
            for ln in pg.splitlines():
                if HEAD.search(ln):
                    seen = True
                if seen and key in common.norm(ln):
                    ev = (pn, c, "ORD", ln.strip()[:80])
                    break
            if ev:
                break
        if ev:
            break
    print(i, r["maker"][:3], r["mpn"], "|", r["pdf"].split("/")[-1], "| rows", len(rs), "|",
          ev if ev else "NO CORE FOUND (pattern doc)")
