"""plan-42-A Phase 3: what fetch.js's URL list, textscan.py, sibling.py,
place.py and specs-skeleton.py share: which rows are eligible (the download
set), each maker's own hosts, the section 4 rule 3 / 4 rewrites, and the PDF
cache paths."""
import csv
import hashlib
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
TREE = os.path.join(ROOT, "datasheets", "regulator")
PLACED = os.path.join(TREE, "datasheets.tsv")

# download-set.py: the picked rows, then the next `below quota` rows by
# stock, up to 1.5 x each (family, maker) share of the family's quota.
QUOTA = {"linear-fixed": 240, "linear-adjustable": 90, "linear-multi": 30, "switching": 240}

OWN = {
    "Texas Instruments": ("www.ti.com", "ti.com"),
    "Analog Devices": ("www.analog.com", "analog.com", "datasheets.maximintegrated.com", "maximintegrated.com"),
    "Microchip Technology": ("ww1.microchip.com", "www.microchip.com", "microchip.com"),
}
SLUG = {"Texas Instruments": "ti", "Analog Devices": "analog-devices", "Microchip Technology": "microchip"}

_SET = set()


def load(path):
    return list(csv.DictReader(open(path), delimiter="\t"))


def init(rows):
    """The download set: the part numbers of `download-set.tsv` (plan-42-A's
    final set; it stays fixed after A, so a later `reason` such as `add` does
    not change it), else computed from <rows> by download-set.py's rule."""
    import collections
    path = os.path.join(DATA, "download-set.tsv")
    if os.path.exists(path):
        _SET.clear()
        _SET.update(r["mpn"] for r in load(path))
        return len(_SET)
    pool = [r for r in rows if r["reason"] in ("candidate", "below quota")]
    pool.sort(key=lambda r: (r["reason"] != "candidate", -int(r["stock"]), r["mpn"]))
    taken = collections.Counter()
    _SET.clear()
    for r in pool:
        key = (r["family"], r["maker"])
        if taken[key] < QUOTA[r["family"]] // 3 * 3 // 2:
            taken[key] += 1
            _SET.add(r["mpn"])
    return len(_SET)


def eligible(row):
    return row["mpn"] in _SET


def full_url(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def host(url):
    u = full_url(url)
    return u.split("/")[2].lower() if "//" in u else ""


def own_site(row):
    return host(row["datasheet_url"]) in OWN[row["maker"]]


def device_runs(base):
    """The names a base is tried under: itself without anything from its first
    `/` on, lowercased; then its leading letters-and-digits run."""
    name = base.split("/")[0]
    out = [name]

    def add(x):
        if x and x not in out:
            out.append(x)
    add(name.split("-")[0])
    m = re.match(r"[A-Za-z0-9]+", name)
    if m:
        add(m.group(0))
    m = re.match(r"([A-Za-z]+)([0-9]+)", name)   # the family: LP8340, TPS793 (digits dropped to 3)
    if m:
        add(m.group(0))
        digits = m.group(2)
        while len(digits) > 3:
            digits = digits[:-1]
            add(m.group(1) + digits)
    return out


def ti_goto(url):
    """The device of a TI `suppproductinfo.tsp?...gotoUrl=<ti.com/lit/gpn/<device>>`
    link (that page is an HTML redirect shell; the PDF is the device's
    `lit/ds/symlink/<device>.pdf`), lowercase, or None (plan-42-A Corrections)."""
    import urllib.parse
    if "suppproductinfo.tsp" not in url or "gotoUrl=" not in url:
        return None
    goto = url.split("gotoUrl=", 1)[1].split("&", 1)[0]
    for _ in range(3):
        goto = urllib.parse.unquote(goto)
    m = re.search(r"/lit/gpn/([A-Za-z0-9._-]+)", goto)
    return m.group(1).lower() if m else None


def sources(row):
    """The URLs a row's datasheet is fetched from, in order (section 4 rules
    1-4): its maker-site link as given; else for a TI row
    https://www.ti.com/lit/ds/symlink/<device>.pdf (the full device, then the
    leading run), for a Microchip row
    https://ww1.microchip.com/downloads/en/DeviceDoc/<base>.pdf (both) and then
    `page:https://www.microchip.com/en-us/product/<name>`, a product page
    fetch.js reads the first ww1 PDF link of. Other off-site links give none."""
    url = row["datasheet_url"].strip()
    if url and own_site(row):
        gpn = ti_goto(url)
        if gpn:
            return ["https://www.ti.com/lit/ds/symlink/%s.pdf" % gpn, full_url(url)]
        return [full_url(url)]
    base = row["base"]
    if not base:
        return []
    names = device_runs(base)
    if row["maker"] == "Texas Instruments":
        return ["https://www.ti.com/lit/ds/symlink/%s.pdf" % n.lower() for n in names]
    if row["maker"] == "Microchip Technology":
        return (["https://ww1.microchip.com/downloads/en/DeviceDoc/%s.pdf" % n for n in names]
                + ["page:https://www.microchip.com/en-us/product/%s" % n for n in names])
    return []


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def pdf_path(directory, url):
    return os.path.join(directory, md5(url) + ".pdf")


def is_pdf(path):
    if not os.path.exists(path):
        return False
    with open(path, "rb") as f:
        return f.read(5) == b"%PDF-"


def actual_urls(directory):
    """key URL -> the URL whose body was kept (a `page:` key's PDF link)."""
    out = {}
    path = os.path.join(directory, "index.tsv")
    if os.path.exists(path):
        for line in open(path):
            f = line.rstrip("\n").split("\t")
            if len(f) >= 6 and f[5]:
                out[f[4]] = f[5]
    return out


def first_pdf(row, directory):
    """The first of a row's source URLs that downloaded as a PDF, or None."""
    for url in sources(row):
        if is_pdf(pdf_path(directory, url)):
            return url
    return None


def norm(s):
    """Every run of non-alphanumerics becomes one space (textscan's token rule)."""
    return " " + re.sub(r"[^0-9A-Z]+", " ", (s or "").upper()).strip() + " "
