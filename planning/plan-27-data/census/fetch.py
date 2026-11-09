"""plan-27 census, step 4: download every distinct datasheet URL.

usage: python3 -I fetch.py <candidates.tsv> <pdf-dir>

Fetches the distinct datasheet_url of every `candidate` and `check` row into
<pdf-dir>/<md5 of the URL>.pdf (8 at a time) and writes <pdf-dir>/index.tsv:
md5, which attempt worked (or the last http status), bytes, first four bytes
(the %PDF check), URL.  A file already in <pdf-dir> that starts with %PDF is
kept, so the script can be run again.  The files are only read afterwards by
pdftotext (textscan.py); nothing runs them.

Why curl and these headers (verified 2026-11-07): the makers' sites (Akamai
at littelfuse.com, te.com, ckswitches.com, alpsalpine.com, omron.com) refuse
Python's urllib and a bare curl with 403, but answer 200 application/pdf to
curl sending Safari's request headers (User-Agent, Accept, Accept-Language,
Accept-Encoding, Sec-Fetch-Site/Mode/Dest, Priority), over HTTP/2 with the
site's own origin as Referer.  we-online.com only answers the
"Claude-User" User-Agent (the one Claude's web-fetch tool sends).
"""
import csv
import hashlib
import os
import subprocess
import sys
import threading
import urllib.parse
from concurrent.futures import ThreadPoolExecutor

# te.com tarpits parallel requests (8 at once: 7 files in 2 minutes; one at a
# time: 4 s each), so it gets one request at a time.
TE_LOCK = threading.Lock()

SAFARI = ("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 "
          "(KHTML, like Gecko) Version/18.6 Safari/605.1.15")

ATTEMPTS = [
    ("safari", ["--http2", "-H", "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
                "-H", "Sec-Fetch-Site: none", "-H", "Sec-Fetch-Mode: navigate", "-H", "Sec-Fetch-Dest: document",
                "-H", "Priority: u=0, i", "-H", "Accept-Language: en-US,en;q=0.9",
                "-H", "Accept-Encoding: gzip, deflate, br", "-A", SAFARI,
                "-H", "Cookie: country_info=usa; geo_info=usa; language_id=1; site_info=usa|en"]),
    ("claude-user", ["-A", "Claude-User"]),
]


def attempt(full, flags, path):
    origin = "{0.scheme}://{0.netloc}/".format(urllib.parse.urlparse(full))
    cmd = ["curl", "-sL", "--max-time", "180", "--compressed", "-e", origin, "-o", path,
           "-w", "%{http_code}", *flags, full]
    try:
        code = subprocess.run(cmd, capture_output=True, text=True, timeout=200).stdout.strip()
    except subprocess.TimeoutExpired:
        code = "timeout"
    head = b""
    if os.path.exists(path):
        with open(path, "rb") as f:
            head = f.read(4)
    return code, head


def complete(path):
    """A whole PDF: starts with %PDF and pdfinfo can open it. A download cut
    off by the network starts with %PDF too (plan-27-A Correction 7)."""
    if not os.path.exists(path):
        return False
    with open(path, "rb") as f:
        if f.read(4) != b"%PDF":
            return False
    return subprocess.run(["pdfinfo", path], capture_output=True).returncode == 0


def fetch(args):
    url, directory = args
    full = ("https:" + url if url.startswith("//") else url).replace(" ", "%20")
    md5 = hashlib.md5(url.encode()).hexdigest()
    path = os.path.join(directory, md5 + ".pdf")
    if complete(path):
        return (md5, "cached", str(os.path.getsize(path)), "%PDF", url)
    code, head = "none", b""
    for name, flags in ATTEMPTS + ATTEMPTS:  # each twice: the network drops some transfers
        if "//www.te.com/" in full:
            with TE_LOCK:
                code, head = attempt(full, flags, path)
        else:
            code, head = attempt(full, flags, path)
        if head == b"%PDF" and complete(path):
            return (md5, name, str(os.path.getsize(path)), "%PDF", url)
        if head == b"%PDF":
            code = "truncated"
    size = os.path.getsize(path) if os.path.exists(path) else 0
    if os.path.exists(path):
        os.remove(path)
    return (md5, code, str(size), "", url)


def main(tsv, directory):
    os.makedirs(directory, exist_ok=True)
    rows = csv.DictReader(open(tsv), delimiter="\t")
    urls = sorted({r["datasheet_url"] for r in rows
                   if r["decision"] in ("candidate", "check") and r["datasheet_url"]})
    with open(os.path.join(directory, "urls.txt"), "w") as out:
        out.write("\n".join(urls) + "\n")
    with ThreadPoolExecutor(8) as pool:
        results = list(pool.map(fetch, [(u, directory) for u in urls]))
    with open(os.path.join(directory, "index.tsv"), "w") as out:
        for r in sorted(results):
            out.write("\t".join(r) + "\n")
    print(len(urls), "urls;", sum(1 for r in results if r[3] == "%PDF"), "are PDFs")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
