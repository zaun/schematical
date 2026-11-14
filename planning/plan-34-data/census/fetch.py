#!/usr/bin/env python3
"""plan-34-A Phase 3: download every distinct datasheet URL of the eligible rows.

usage: python3 -I fetch.py <candidates.tsv> <pdf-dir>

Fetches each URL into <pdf-dir>/<md5 of the URL as DigiKey gives it>.pdf (8
at a time) and writes <pdf-dir>/index.tsv: md5, the attempt that worked (or
the last HTTP status), bytes, `%PDF` when it is one, the URL, and the URL
actually fetched. A file already there that starts with %PDF- is kept, so
the script can be run again; a file the rewrite gave keeps that URL in
<md5>.via, so a re-run names and records it the same way (§ Corrections 8). Attempts: a bare `curl -sL` (vishay.com,
yageogroup.com and viking.com.tw answer it with a PDF, plan-34-A § 2), then
plan-29's Safari-header attempt; a yageo.com /lr/ URL that gives no PDF is
fetched from yageogroup.com (§ 4 rule 3). The files are only read
afterwards, by pdftotext; nothing runs them.
"""
import csv
import os
import subprocess
import sys
import urllib.parse
from concurrent.futures import ThreadPoolExecutor

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

SAFARI = ("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 "
          "(KHTML, like Gecko) Version/18.6 Safari/605.1.15")
ATTEMPTS = [
    ("plain", []),
    ("safari", ["--http2", "-H", "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
                "-H", "Accept-Language: en-US,en;q=0.9", "-A", SAFARI]),
]


def attempt(full, flags, path):
    origin = "{0.scheme}://{0.netloc}/".format(urllib.parse.urlparse(full))
    cmd = ["curl", "-sL", "--max-time", "60", "--compressed", "-e", origin, "-o", path,
           "-w", "%{http_code}", *flags, full]
    try:
        return subprocess.run(cmd, capture_output=True, text=True, timeout=90).stdout.strip()
    except subprocess.TimeoutExpired:
        return "timeout"


def fetch(args):
    url, directory = args
    path = common.pdf_path(directory, url)
    via_file = path[:-4] + ".via"   # the rewrite URL that gave a cached file, if one did
    if common.is_pdf(path):
        via = open(via_file).read().strip() if os.path.exists(via_file) else ""
        return (common.md5(url), "cached", str(os.path.getsize(path)), "%PDF", url, via)
    targets = [common.full_url(url)]
    if common.rewrite(url):
        targets.append(common.rewrite(url))
    code = "none"
    for target in targets:
        for name, flags in ATTEMPTS:
            code = attempt(target, flags, path)
            if common.is_pdf(path):
                via = "" if target == targets[0] else target
                if via:
                    with open(via_file, "w") as f:
                        f.write(via + "\n")
                return (common.md5(url), name, str(os.path.getsize(path)), "%PDF", url, via)
    size = os.path.getsize(path) if os.path.exists(path) else 0
    if os.path.exists(path):
        os.remove(path)
    return (common.md5(url), code, str(size), "", url, "")


def main(tsv, directory):
    os.makedirs(directory, exist_ok=True)
    urls = sorted({r["datasheet_url"] for r in csv.DictReader(open(tsv), delimiter="\t")
                   if common.eligible(r) and r["datasheet_url"]})
    with ThreadPoolExecutor(8) as pool:
        results = list(pool.map(fetch, [(u, directory) for u in urls]))
    with open(os.path.join(directory, "index.tsv"), "w") as out:
        for r in sorted(results):
            out.write("\t".join(r) + "\n")
    print(len(urls), "urls;", sum(1 for r in results if r[3] == "%PDF"), "are PDFs;",
          sum(1 for r in results if r[5]), "fetched by the yageogroup.com rewrite")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
