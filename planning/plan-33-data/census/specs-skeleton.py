"""plan-33-A Phase 4: the specs.tsv skeleton.

usage: python3 -I specs-skeleton.py <specs.tsv>

One row per placed PDF (datasheets/capacitor/datasheets.tsv): `maker` (the
folder slug: kemet, calchip, avx), `datasheet` (the PDF's path), `url` (the
first of its URLs, sorted), and the scheme columns `mpn_regex cap_group
volt_group volt_map case_group note` empty (plan-33-B Phase 1 fills them).
Refuses to overwrite a specs.tsv that already has a filled row.
"""
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
PLACED = os.path.join(ROOT, "datasheets", "capacitor", "datasheets.tsv")
COLS = ["maker", "datasheet", "url", "mpn_regex", "cap_group", "volt_group", "volt_map",
        "case_group", "note"]


def main(dst):
    if os.path.exists(dst):
        for r in csv.DictReader(open(dst), delimiter="\t"):
            if r["mpn_regex"] or r["cap_group"] or r["volt_map"]:
                print("refused: specs.tsv already has filled rows")
                return 1
    urls = {}
    for r in csv.DictReader(open(PLACED), delimiter="\t"):
        urls.setdefault(r["path"], []).append(r["url"])
    with open(dst, "w") as out:
        out.write("\t".join(COLS) + "\n")
        for path in sorted(urls):
            maker = path.split("/")[2]
            out.write("\t".join([maker, path, sorted(urls[path])[0]] + [""] * 6) + "\n")
    print(len(urls), "rows")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
