"""plan-34-A Phase 3: what fetch.py, textscan.py, sibling.py, place.py and
specs-skeleton.py share: which rows are eligible, each maker's own hosts,
the § 4 YAGEO URL rewrite, and the PDF cache paths."""
import hashlib
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
TREE = os.path.join(ROOT, "datasheets", "resistor")
PLACED = os.path.join(TREE, "datasheets.tsv")

# Rows out for scope (plan-34-A § 3 rule 2), for a number DigiKey does not
# give, or for the part-by-part stock check (§ 5 rule 2) are not eligible.
NOT_ELIGIBLE = {"automotive", "current-sense", "resistance not a number", "stock check"}

OWN = {
    "YAGEO": ("yageogroup.com", "www.yageogroup.com", "yageo.com", "www.yageo.com"),
    "Vishay Dale": ("www.vishay.com", "vishay.com"),
    "Viking Tech": ("www.viking.com.tw", "viking.com.tw"),
}
SLUG = {"YAGEO": "yageo", "Vishay Dale": "vishay-dale", "Viking Tech": "viking"}

YAGEO_LR = re.compile(r"^https://www\.yageo\.com/upload/media/product/app/datasheet/lr/yageo-([a-z0-9_-]+)_datasheet\.pdf$",
                      re.I)


def eligible(row):
    return row["reason"] not in NOT_ELIGIBLE


def full_url(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def host(url):
    u = full_url(url)
    return u.split("/")[2].lower() if "//" in u else ""


def own_site(row):
    return host(row["datasheet_url"]) in OWN[row["maker"]]


def rewrite(url):
    """§ 4 rule 3: a yageo.com /lr/ URL that answers HTML is fetched from
    yageogroup.com's asset path instead; None when the rule does not apply."""
    m = YAGEO_LR.match(full_url(url))
    if not m:
        return None
    return "https://yageogroup.com/content/datasheet/asset/file/YAGEO-%s_DATASHEET" % m.group(1).upper()


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
    return " " + re.sub(r"[^0-9A-Z]+", " ", s.upper()).strip() + " "
