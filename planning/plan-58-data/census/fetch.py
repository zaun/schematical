"""plan-58-A Phase 2: download every distinct datasheet URL of the `add` rows.

usage: python3 -I fetch.py <candidates.tsv> <pdf-dir>
       python3 -I fetch.py --selftest

Fetches the distinct `datasheet` URL of every row whose `decision` is `add`
(a protocol-relative `//host/...` URL gets `https:`) into
<pdf-dir>/<md5 of the URL>.pdf and writes <pdf-dir>/index.tsv: md5, which
attempt worked (or the last http status), bytes, first four bytes (the %PDF
check), URL.  A file already in <pdf-dir> that is a whole PDF is kept, so the
script can be run again.  A body that does not start with %PDF is not kept,
and pdfinfo must open the file.  Exit 1 lists the URLs that are missing.

Headers (plan-58-A section 2, verified 2026-11-24 at planning):
  www.littelfuse.com   Safari 18.6's full request header set over HTTP/2, the
                       site's own origin as Referer (a bare curl gets 403).
  mm.digikey.com       Cloudflare refuses curl: scripts/safari-digikey-pdf.sh
                       fetches it inside a Safari tab on its own origin, one
                       at a time.
  every other host     the same Safari header set first, a plain curl second.
"""
import csv
import hashlib
import os
import subprocess
import sys
import threading
import urllib.parse
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
SAFARI_PDF = os.path.join(ROOT, "scripts", "safari-digikey-pdf.sh")

SAFARI = ("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 "
          "(KHTML, like Gecko) Version/18.6 Safari/605.1.15")

SAFARI_FLAGS = ["--http2", "-H", "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
                "-H", "Sec-Fetch-Site: none", "-H", "Sec-Fetch-Mode: navigate", "-H", "Sec-Fetch-Dest: document",
                "-H", "Priority: u=0, i", "-H", "Accept-Language: en-US,en;q=0.9",
                "-H", "Accept-Encoding: gzip, deflate, br", "-A", SAFARI]

ATTEMPTS = [("safari", SAFARI_FLAGS), ("plain", [])]

# Safari fetches one document at a time (one tab of the DigiKey window).
SAFARI_LOCK = threading.Lock()


def full_url(url):
    """The URL curl gets: protocol-relative gets https:, a space becomes %20."""
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def whole_pdf(path):
    """A whole PDF: starts with %PDF and pdfinfo can open it (a download cut
    off by the network starts with %PDF too)."""
    if not os.path.exists(path):
        return False
    with open(path, "rb") as f:
        if f.read(4) != b"%PDF":
            return False
    return subprocess.run(["pdfinfo", path], capture_output=True).returncode == 0


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


def via_safari(full, path):
    with SAFARI_LOCK:
        try:
            subprocess.run([SAFARI_PDF, full, path], capture_output=True, timeout=120)
        except subprocess.TimeoutExpired:
            return "timeout", b""
    head = b""
    if os.path.exists(path):
        with open(path, "rb") as f:
            head = f.read(4)
    return "safari-tab", head


def fetch(args):
    url, directory = args
    full = full_url(url)
    md5 = hashlib.md5(url.encode()).hexdigest()
    path = os.path.join(directory, md5 + ".pdf")
    if whole_pdf(path):
        return (md5, "cached", str(os.path.getsize(path)), "%PDF", url)
    code, head = "none", b""
    if urllib.parse.urlparse(full).netloc == "mm.digikey.com":
        tries = [("safari-tab", None)] * 2
    else:
        tries = ATTEMPTS + ATTEMPTS  # each twice: the network drops some transfers
    for name, flags in tries:
        if flags is None:
            code, head = via_safari(full, path)
        else:
            code, head = attempt(full, flags, path)
        if head == b"%PDF" and whole_pdf(path):
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
    urls = sorted({r["datasheet"] for r in rows if r["decision"] == "add" and r["datasheet"]})
    with open(os.path.join(directory, "urls.txt"), "w") as out:
        out.write("\n".join(urls) + "\n")
    with ThreadPoolExecutor(8) as pool:
        results = list(pool.map(fetch, [(u, directory) for u in urls]))
    with open(os.path.join(directory, "index.tsv"), "w") as out:
        for r in sorted(results):
            out.write("\t".join(r) + "\n")
    missing = [r for r in results if r[3] != "%PDF"]
    for r in missing:
        print("missing", r[1], r[4])
    print(len(urls), "urls;", len(urls) - len(missing), "are PDFs;", len(missing), "missing")
    return 1 if missing else 0


def selftest():
    # protocol-relative URL, space
    assert full_url("//mm.digikey.com/Volume0/x.pdf") == "https://mm.digikey.com/Volume0/x.pdf"
    assert full_url("https://a.com/b c.pdf") == "https://a.com/b%20c.pdf"
    # the header set: Safari's, as § 2 lists it
    flat = " ".join(SAFARI_FLAGS)
    for need in ("--http2", "Sec-Fetch-Site: none", "Sec-Fetch-Mode: navigate", "Sec-Fetch-Dest: document",
                 "Priority: u=0, i", "Accept-Language", "Accept-Encoding: gzip, deflate, br", "Version/18.6"):
        assert need in flat, need
    # the %PDF check: an HTML body and a cut-off PDF are not kept
    import tempfile
    with tempfile.TemporaryDirectory() as d:
        html = os.path.join(d, "a.pdf")
        open(html, "wb").write(b"<html>Just a moment...</html>")
        assert not whole_pdf(html)
        cut = os.path.join(d, "b.pdf")
        open(cut, "wb").write(b"%PDF-1.4\n1 0 obj\n")
        assert not whole_pdf(cut)
        assert not whole_pdf(os.path.join(d, "missing.pdf"))
    print("ok")
    return 0


if __name__ == "__main__":
    if sys.argv[1:] == ["--selftest"]:
        sys.exit(selftest())
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1], sys.argv[2]))
