#!/usr/bin/env python3
"""plan-44-A Phase 3 (section 4 rule 4): a same-maker PDF for download-set rows with none.

usage: python3 -I sibling.py <candidates.tsv> <pdf-dir> <out.tsv>

A download-set row whose own link (or a § 4 rewrite) gave no PDF
(textscan.py left `pdf` empty) gets the first (by URL) downloaded PDF of the
same maker that prints its part number verbatim; `pdf`, `hit` (1) and `dead`
are set from that PDF. The rest stay without a PDF (B skips them: "no
datasheet on the maker's site", with their link's host).
"""
import csv
import importlib.util
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

_spec = importlib.util.spec_from_file_location("textscan", os.path.join(common.HERE, "textscan.py"))
textscan = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(textscan)


def main(tsv, directory, out_path):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    cols = list(rows[0].keys())
    texts = {}
    for r in rows:
        if r["pdf"] and r["pdf"] not in texts.get(r["maker"], {}):
            texts.setdefault(r["maker"], {})[r["pdf"]] = common.norm(textscan.text_of(directory, r["pdf"]))
    need = found = 0
    for r in rows:
        if not common.eligible(r) or r["pdf"]:
            continue
        need += 1
        for url in sorted(texts.get(r["maker"], {})):
            hit, dead = textscan.scan(texts[r["maker"]][url], r["mpn"])
            if hit == "1":
                r["pdf"], r["hit"], r["dead"] = url, hit, dead
                found += 1
                break
    with open(out_path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    print(need, "eligible rows without their own PDF;", found, "printed in a same-maker PDF")


if __name__ == "__main__":
    main(*sys.argv[1:4])
