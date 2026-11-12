"""plan-33-A Phase 2: download every distinct datasheet URL of the ranked rows.

usage: python3 -I fetch.py <candidates.tsv> <pdf-dir> [--max-rank N]
       python3 -I fetch.py --verify <candidates.tsv> <pdf-dir> [--max-rank N]

Fetches the distinct datasheet_url of every `candidate` / `add` row whose
`rank` is at most N (default 10) into <pdf-dir>/<md5 of the URL>.pdf (8 at a
time) and appends to <pdf-dir>/index.tsv: md5, which attempt worked (or the
last http status), bytes, first four bytes (the %PDF check), URL.  A file
already in <pdf-dir> that starts with %PDF is kept, and a URL already in
index.tsv is not tried again, so the script can be run again (B runs it with
a larger --max-rank for buckets whose first ten ranks all fail).  The files
are only read afterwards by pdftotext (textscan.py); nothing runs them.

Attempts, in order (plan-33-A section 2): a bare curl (yageogroup.com and
calchip.com answer it with 200 application/pdf), then a `Mozilla/5.0` user
agent (datasheets.kyocera-avx.com answers a bare curl with 406 text/html),
then plan-29's Safari-header attempt for the other hosts DigiKey links.

--verify prints the URLs of those rows that index.tsv does not list and
`N untried` (0 when every URL was tried).
"""
import csv
import hashlib
import os
import subprocess
import sys
import urllib.parse
from concurrent.futures import ThreadPoolExecutor

SAFARI = ("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 "
          "(KHTML, like Gecko) Version/18.6 Safari/605.1.15")

ATTEMPTS = [
    ("plain", []),
    ("mozilla", ["-A", "Mozilla/5.0 (Macintosh)"]),
    ("safari", ["--http2", "-H", "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
                "-H", "Sec-Fetch-Site: none", "-H", "Sec-Fetch-Mode: navigate", "-H", "Sec-Fetch-Dest: document",
                "-H", "Accept-Language: en-US,en;q=0.9", "-A", SAFARI]),
]


def full(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def attempt(url, flags, path):
    origin = "{0.scheme}://{0.netloc}/".format(urllib.parse.urlparse(url))
    cmd = ["curl", "-sL", "--max-time", "60", "--compressed", "-e", origin, "-o", path,
           "-w", "%{http_code}", *flags, url]
    try:
        code = subprocess.run(cmd, capture_output=True, text=True, timeout=90).stdout.strip()
    except subprocess.TimeoutExpired:
        code = "timeout"
    head = b""
    if os.path.exists(path):
        with open(path, "rb") as f:
            head = f.read(4)
    return code, head


def fetch(args):
    url, directory = args
    md5 = hashlib.md5(url.encode()).hexdigest()
    path = os.path.join(directory, md5 + ".pdf")
    if os.path.exists(path):
        with open(path, "rb") as f:
            if f.read(4) == b"%PDF":
                return (md5, "cached", str(os.path.getsize(path)), "%PDF", url)
    code, head = "none", b""
    for name, flags in ATTEMPTS:
        code, head = attempt(full(url), flags, path)
        if head == b"%PDF":
            return (md5, name, str(os.path.getsize(path)), "%PDF", url)
    size = os.path.getsize(path) if os.path.exists(path) else 0
    if os.path.exists(path):
        os.remove(path)
    return (md5, code, str(size), "", url)


def wanted(tsv, max_rank):
    out = set()
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["decision"] in ("candidate", "add") and r["datasheet_url"] and r["rank"] \
                and int(r["rank"]) <= max_rank:
            out.add(r["datasheet_url"])
    return sorted(out)


def indexed(directory):
    path = os.path.join(directory, "index.tsv")
    if not os.path.exists(path):
        return {}
    return {line.rstrip("\n").split("\t")[4]: line for line in open(path) if line.strip()}


def main(argv):
    verify = argv[0] == "--verify"
    if verify:
        argv = argv[1:]
    max_rank = 10
    if "--max-rank" in argv:
        i = argv.index("--max-rank")
        max_rank = int(argv[i + 1])
        argv = argv[:i] + argv[i + 2:]
    tsv, directory = argv[0], argv[1]
    os.makedirs(directory, exist_ok=True)
    urls = wanted(tsv, max_rank)
    done = indexed(directory)
    if verify:
        untried = [u for u in urls if u not in done]
        for u in untried:
            print("untried", u)
        print(len(urls), "urls,", sum(1 for u in urls if u in done and "%PDF" in done[u]),
              "PDFs,", len(untried), "untried")
        return 1 if untried else 0
    todo = [u for u in urls if u not in done]
    with ThreadPoolExecutor(8) as pool:
        results = list(pool.map(fetch, [(u, directory) for u in todo]))
    with open(os.path.join(directory, "index.tsv"), "a") as out:
        for r in sorted(results):
            out.write("\t".join(r) + "\n")
    print(len(urls), "urls;", len(todo), "tried now;",
          sum(1 for r in results if r[3] == "%PDF"), "of those are PDFs")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
