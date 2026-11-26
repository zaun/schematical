#!/usr/bin/env python3
"""plan-73-B: downloads the eight USB-UART bridge datasheets into datasheets/controller-uart/.

usage (from the repo root): python3 -I planning/plan-73-data/fetch-datasheets.py

Silicon Labs answers Python urllib with a Safari User-Agent plus Accept and
Accept-Language headers (tested 2026-11-25: every URL below returned 200 and a
PDF); WCH needs only a User-Agent. Each file is checked: it starts with %PDF-
and has the byte size measured at planning (a new revision changes the size:
the script then says so and keeps the new file, and the implementer re-reads
the pages the plan cites before using it).
"""
import os
import sys
import urllib.request

HEADERS = {
    "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 "
                  "(KHTML, like Gecko) Version/17.0 Safari/605.1.15",
    "Accept": "application/pdf,*/*",
    "Accept-Language": "en-US,en;q=0.9",
}
SL = "https://www.silabs.com/documents/public/data-sheets/"
FILES = [  # (local id, URL, bytes at planning 2026-11-25)
    ("cp2102-9", SL + "CP2102-9.pdf", 2172800),
    ("cp2102n", SL + "cp2102n-datasheet.pdf", 2252034),
    ("cp2102c", SL + "cp2102c-datasheet.pdf", 664111),
    ("cp2104", SL + "CP2104.pdf", 2067999),
    ("cp2105", SL + "cp2105.pdf", 1601911),
    ("cp2108", SL + "cp2108-datasheet.pdf", 813001),
    ("cp2110", SL + "CP2110.pdf", 1185044),
    ("ch340", "https://www.wch-ic.com/download/file?id=79", 325640),
]


def main():
    out_dir = os.path.join("datasheets", "controller-uart")
    if not os.path.isdir(out_dir):
        sys.exit("run from the repo root (no datasheets/controller-uart here)")
    bad = 0
    for ident, url, size in FILES:
        req = urllib.request.Request(url, headers=HEADERS)
        with urllib.request.urlopen(req, timeout=60) as r:
            body = r.read()
        if not body.startswith(b"%PDF-"):
            print(f"FAIL {ident}: {url} did not return a PDF")
            bad += 1
            continue
        with open(os.path.join(out_dir, ident + ".pdf"), "wb") as f:
            f.write(body)
        note = "" if len(body) == size else f"  (planning size {size}: a new revision)"
        print(f"ok   {ident}.pdf {len(body)} bytes{note}")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
