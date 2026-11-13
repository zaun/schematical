"""plan-36-A Phase 2: download every distinct datasheet URL of the in-scope rows.

usage: python3 -I fetch.py <cand.tsv> <pdf-dir>

Adapted from planning/plan-29-data/census/fetch.py. Each distinct
datasheet_url of a `scope` = in row goes to <pdf-dir>/<md5 of the URL>.pdf
with a bare `curl -sL` (plan-36-A § 3 rule 3; microchip.com, espressif.com
and raspberrypi.com answer it). A file is kept only when it starts with
%PDF- and `pdfinfo` reads it. A Microchip URL that does not give a PDF is
retried once on
ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/<file>
(same file name); a success is cached under the original URL's md5 and the
retry URL is recorded in index.tsv. Files already present and valid are kept,
so the script can be run again. The PDFs are only read by pdftotext later.

index.tsv: md5, how (plain / retry / cached / the last http status), bytes,
url, retry url.
"""
import csv
import hashlib
import os
import subprocess
import sys
import urllib.parse
from concurrent.futures import ThreadPoolExecutor

RETRY = "https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/"


def full(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def valid(path):
    if not os.path.exists(path):
        return False
    with open(path, "rb") as f:
        if f.read(5) != b"%PDF-":
            return False
    r = subprocess.run(["pdfinfo", path], capture_output=True, timeout=60)
    return r.returncode == 0


def curl(url, path):
    try:
        return subprocess.run(["curl", "-sL", "--max-time", "300", "-o", path, "-w", "%{http_code}", full(url)],
                              capture_output=True, text=True, timeout=330).stdout.strip()
    except subprocess.TimeoutExpired:
        return "timeout"


def fetch(args):
    url, directory = args
    h = md5(url)
    path = os.path.join(directory, h + ".pdf")
    if valid(path):
        return (h, "cached", str(os.path.getsize(path)), url, "")
    code = curl(url, path)
    if valid(path):
        return (h, "plain", str(os.path.getsize(path)), url, "")
    host = urllib.parse.urlparse(full(url)).netloc.lower()
    if host.endswith("microchip.com"):
        name = urllib.parse.urlparse(full(url)).path.rstrip("/").split("/")[-1]
        if name.lower().endswith(".pdf"):
            alt = RETRY + name
            code = curl(alt, path)
            if valid(path):
                return (h, "retry", str(os.path.getsize(path)), url, alt)
    if os.path.exists(path):
        os.remove(path)
    return (h, code, "0", url, "")


def main(tsv, directory):
    os.makedirs(directory, exist_ok=True)
    rows = csv.DictReader(open(tsv), delimiter="\t")
    urls = sorted({r["datasheet_url"] for r in rows if r["scope"] == "in" and r["datasheet_url"]})
    with ThreadPoolExecutor(6) as pool:
        results = list(pool.map(fetch, [(u, directory) for u in urls]))
    with open(os.path.join(directory, "index.tsv"), "w") as out:
        out.write("md5\thow\tbytes\turl\tretry\n")
        for r in sorted(results, key=lambda r: r[3]):
            out.write("\t".join(r) + "\n")
    ok = [r for r in results if r[2] != "0"]
    print(len(urls), "urls;", len(ok), "are PDFs;", sum(int(r[2]) for r in ok) / 1e6, "MB")
    for r in results:
        if r[2] == "0":
            print(" no pdf", r[1], r[3])


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
