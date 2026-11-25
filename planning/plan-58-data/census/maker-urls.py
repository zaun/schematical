"""plan-58-A Phase 3: maker-hosted URLs for datasheets DigiKey links on a distributor's server.

usage: python3 -I maker-urls.py --list      the distinct non-maker datasheet URLs of add / pending rows
       python3 -I maker-urls.py --verify    every one has a row in maker-urls.tsv, and every non-empty
                                            maker_url downloads as a PDF (curl, Safari's header set)
       python3 -I maker-urls.py --url URL   the URL a part's datasheets.json entry uses
       python3 -I maker-urls.py --selftest

maker-urls.tsv: header `url maker_url how`.  `maker_url` is the same document
(or its current revision) on the maker's own site, or empty when the maker's
site has none; then the DigiKey (or distributor) URL stays (owner, 2026-11-08,
plan-27-A).  `how` says where it was found or what was tried.  A URL is
"not the maker's" when its host is one of NOT_MAKER.
"""
import csv
import importlib.util
import os
import subprocess
import sys
import tempfile
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
CAND = os.path.join(DATA, "candidates.tsv")
TSV = os.path.join(DATA, "maker-urls.tsv")
# E-Switch's own product pages (e-switch.com/product/500-series-..., 500a-..., 500r-..., eg-...) link their
# datasheets on configured-product-images.s3.amazonaws.com, so that bucket is the maker's own host (as in
# plan-27: its decide.py HOSTS and maker-urls.py NOT_MAKER did the same); only DigiKey's mm.digikey.com is not.
NOT_MAKER = ("mm.digikey.com",)


def is_non_maker(url):
    host = urllib.parse.urlparse("https:" + url if url.startswith("//") else url).netloc
    return any(host == h or host.endswith("." + h) for h in NOT_MAKER)


def non_maker():
    urls = {}
    for r in csv.DictReader(open(CAND), delimiter="\t"):
        if r["decision"] in ("add", "pending") and r["datasheet"] and is_non_maker(r["datasheet"]):
            urls.setdefault(r["datasheet"], set()).add((r["maker"], r["series"]))
    return urls


def table():
    if not os.path.exists(TSV):
        return {}
    return {r["url"]: r for r in csv.DictReader(open(TSV), delimiter="\t")}


def fetcher():
    spec = importlib.util.spec_from_file_location("fetch58", os.path.join(HERE, "fetch.py"))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def verify():
    t = table()
    nm = non_maker()
    missing = [u for u in nm if u not in t]
    for u in missing:
        print("missing", u)
    f = fetcher()
    bad = 0
    found = 0
    with tempfile.TemporaryDirectory() as d:
        for u in nm:
            m = t.get(u, {}).get("maker_url", "")
            if not m:
                continue
            found += 1
            path = os.path.join(d, "x.pdf")
            ok = False
            for _, flags in f.ATTEMPTS:
                f.attempt(f.full_url(m), flags, path)
                if f.whole_pdf(path):
                    ok = True
                    break
            if not ok:
                bad += 1
                print("not a PDF", m)
            if os.path.exists(path):
                os.remove(path)
    print(f"{len(nm)} non-maker URLs, {found} with a maker URL, {len(missing)} missing, {bad} not PDFs")
    return 1 if missing or bad else 0


def selftest():
    assert is_non_maker("//mm.digikey.com/Volume0/x.pdf")
    assert not is_non_maker("https://configured-product-images.s3.amazonaws.com/Datasheets/x.pdf")
    assert not is_non_maker("https://www.littelfuse.com/assetdocs/x")
    assert not is_non_maker("https://www.nkkswitches.com/pdf/Aslides.pdf")
    print("ok")
    return 0


def main(a):
    if a == ["--selftest"]:
        return selftest()
    if a == ["--list"]:
        for u, groups in sorted(non_maker().items()):
            print(u, "; ".join(f"{m} {s}" for m, s in sorted(groups)), sep="\t")
        return 0
    if a == ["--verify"]:
        return verify()
    if len(a) == 2 and a[0] == "--url":
        row = table().get(a[1])
        print(row["maker_url"] if row and row["maker_url"] else a[1])
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
