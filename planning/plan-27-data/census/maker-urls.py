"""plan-27-A Phase 5: maker-hosted URLs for datasheets DigiKey links on a distributor's server.

usage: python3 -I maker-urls.py --list     the distinct non-maker datasheet URLs of add / pending rows
       python3 -I maker-urls.py --verify   every one of them has a row in maker-urls.tsv
       python3 -I maker-urls.py --url URL  the URL a part's datasheets.json entry uses

maker-urls.tsv: header `dk_url maker_url how`.  `maker_url` is the same
document (or its current revision) on the maker's own site, or empty when
none was found; then the DigiKey URL stays (owner, 2026-11-08).  `how` says
where it was found or what was tried.
"""
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
CAND = os.path.join(DATA, "candidates.tsv")
TSV = os.path.join(DATA, "maker-urls.tsv")
NOT_MAKER = ("mm.digikey.com", "waldomemea.com")


def non_maker():
    urls = {}
    for r in csv.DictReader(open(CAND), delimiter="\t"):
        u = r["datasheet_url"]
        if r["decision"] in ("add", "pending") and any(h in u for h in NOT_MAKER):
            urls.setdefault(u, set()).add((r["maker"].split(" (")[0], r["series"]))
    return urls


def table():
    if not os.path.exists(TSV):
        return {}
    return {r["dk_url"]: r for r in csv.DictReader(open(TSV), delimiter="\t")}


def main(a):
    if a == ["--list"]:
        for u, groups in sorted(non_maker().items()):
            print(u, "; ".join(f"{m} {s}" for m, s in sorted(groups)), sep="\t")
        return 0
    if a == ["--verify"]:
        t = table()
        missing = [u for u in non_maker() if u not in t]
        for u in missing:
            print("missing", u)
        found = sum(1 for u in non_maker() if t.get(u, {}).get("maker_url"))
        print(f"{len(non_maker())} non-maker URLs, {found} with a maker URL, {len(missing)} missing")
        return 1 if missing else 0
    if len(a) == 2 and a[0] == "--url":
        row = table().get(a[1])
        print(row["maker_url"] if row and row["maker_url"] else a[1])
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
