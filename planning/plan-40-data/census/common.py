"""plan-40-A Phase 3: what fetch.js's URL list, textscan.py, sibling.py,
place.py and specs-skeleton.py share: which rows are eligible, each maker's
own hosts, the section 4 rule 3 onsemi URL rewrite, and the PDF cache paths."""
import hashlib
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
TREE = os.path.join(ROOT, "datasheets", "transistor")
PLACED = os.path.join(TREE, "datasheets.tsv")

# plan-40-A section 3: rules 5-7 apply to every eligible row.
ELIGIBLE = {"candidate", "duplicate", "below quota"}

OWN = {
    "onsemi": ("www.onsemi.com", "onsemi.com"),
    "Nexperia": ("assets.nexperia.com", "www.nexperia.com", "nexperia.com"),
    "Diodes Incorporated": ("www.diodes.com", "diodes.com"),
    "Infineon Technologies": ("www.infineon.com", "infineon.com"),
    "Vishay Siliconix": ("www.vishay.com", "vishay.com"),
}
SLUG = {
    "onsemi": "onsemi", "Nexperia": "nexperia", "Diodes Incorporated": "diodes",
    "Infineon Technologies": "infineon", "Vishay Siliconix": "vishay-siliconix",
}


def eligible(row):
    return row["reason"] in ELIGIBLE


def full_url(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def host(url):
    u = full_url(url)
    return u.split("/")[2].lower() if "//" in u else ""


def own_site(row):
    return host(row["datasheet_url"]) in OWN[row["maker"]]


def source(row):
    """The URL a row's datasheet is fetched from (section 4 rules 1-3), or
    None: its maker-site link as given; for an onsemi row whose link is not
    on onsemi's site (a Rochester or DigiKey copy, a Fairchild link, none)
    https://www.onsemi.com/pdf/datasheet/<base lowercase>-d.pdf; other
    makers' off-site links are not fetched."""
    url = row["datasheet_url"].strip()
    if url and own_site(row):
        return full_url(url)
    if row["maker"] == "onsemi" and row["base"]:
        return "https://www.onsemi.com/pdf/datasheet/%s-d.pdf" % row["base"].lower()
    return None


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def pdf_path(directory, url):
    return os.path.join(directory, md5(url) + ".pdf")


def is_pdf(path):
    if not os.path.exists(path):
        return False
    with open(path, "rb") as f:
        return f.read(5) == b"%PDF-"


def norm(s):
    """Every run of non-alphanumerics becomes one space (textscan's token rule)."""
    return " " + re.sub(r"[^0-9A-Z]+", " ", (s or "").upper()).strip() + " "
