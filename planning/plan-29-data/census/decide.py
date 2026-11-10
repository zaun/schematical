"""plan-29 census, step 9 (plan-29-A Phase 4): first decisions, section 3 rules 1-5.

usage: python3 -I decide.py <candidates.tsv> <pdf-dir>   rewrite `decision`/`reason` in place
       python3 -I decide.py --dead <candidates.tsv> <pdf-dir>
                                                         print the text around each dead row's
                                                         part number (for dead-verdicts.tsv)
       python3 -I decide.py --skipped                    rewrite the "Diodes (plan-29)" section
                                                         of datasheets/skipped-parts.md
       python3 -I decide.py --verify-skipped             every skip row is in that section

A `candidate` / `check` row (rows already add/pending/skip/out are kept):
  maker-site PDF of its own (rule 3)       -> its datasheet
  else printed verbatim in a downloaded maker-site PDF of the same maker
  (the first such URL, sorted)             -> that PDF is its datasheet: datasheet_url
                                              becomes the sibling's URL, pdf = hit = 1,
                                              dead from the sibling's text
  else                                     -> skip, "no datasheet on the maker's site (<why>)"
  with a datasheet: dead = 1 -> the verdict in dead-verdicts.tsv (`out` "datasheet
  status: <word>", or `add`); hit = 1 -> add; hit = 0 -> pending.
A `check` row keeps "check; mounting from the datasheet" in its reason.
Every datasheet_url is rewritten to the first URL (sorted) of its file
(SHA-256), so one PDF has one URL.  PDFs under datasheets/diode/ that no add or
pending row uses are printed (place.py --prune removes them).
"""
import collections
import csv
import hashlib
import os
import re
import sys
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
DATA = os.path.dirname(HERE)
CAND = os.path.join(DATA, "candidates.tsv")
VERDICTS = os.path.join(HERE, "dead-verdicts.tsv")
SKIPPED = os.path.join(ROOT, "datasheets", "skipped-parts.md")
HEAD = "## Diodes (plan-29)"

SHORT = {"Vishay General Semiconductor - Diodes Division": "Vishay", "onsemi": "onsemi",
         "Taiwan Semiconductor Corporation": "TSC"}
OWN = {"Vishay": ("vishay.com",), "onsemi": ("onsemi.com",), "TSC": ("taiwansemi.com", "ts.com.tw")}
DEAD = re.compile("OBSOLETE|DISCONTINU|NOT RECOMMENDED FOR NEW|END OF LIFE|NRND")
LEAF_GROUP = {"single": "Diodes", "array": "Diodes", "bridge": "Diodes", "zener": "Diodes",
              "zener-array": "Diodes"}


def full(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def host_of(url):
    return urllib.parse.urlparse(full(url)).netloc.lower()


def own_site(maker, url):
    h = host_of(url)
    return any(h == x or h.endswith("." + x) for x in OWN[SHORT[maker]])


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def norm(s):
    return " " + re.sub(r"[^0-9A-Z]+", " ", s.upper()).strip() + " "


def read(path):
    with open(path) as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    return rows, list(rows[0].keys())


def write(path, rows, cols):
    with open(path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")


def text(pdf_dir, url):
    p = os.path.join(pdf_dir, md5(url) + ".txt")
    return open(p, errors="replace").read() if os.path.exists(p) else ""


def sha_of(pdf_dir, url, cache):
    if url not in cache:
        h = hashlib.sha256()
        with open(os.path.join(pdf_dir, md5(url) + ".pdf"), "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
        cache[url] = h.hexdigest()
    return cache[url]


def why(index, url):
    if not url:
        return "no URL"
    h = host_of(url)
    if h == "mm.digikey.com":
        return "DigiKey copy, mm.digikey.com"
    if "rocelec" in h:
        return f"Rochester copy, {h}"
    if "snapeda" in h:
        return f"SnapEDA, {h}"
    if "waldom" in h:
        return f"Waldom copy, {h}"
    entry = index.get(md5(url))
    if entry is None:
        return f"not downloaded, {h}"
    code = entry[1]
    if code in ("000", "none", "", "timeout"):
        return f"no answer, {h}"
    if code.startswith("2") or code.startswith("3"):
        return f"not a PDF, {h}"
    return f"HTTP {code}, {h}"


def verdicts():
    out = {}
    if os.path.exists(VERDICTS):
        for v in csv.DictReader(open(VERDICTS), delimiter="\t"):
            out[(v["mpn"], v["pdf"])] = v
    return out


def decide(argv):
    tsv, pdf_dir = argv
    rows, cols = read(tsv)
    index = {}
    with open(os.path.join(pdf_dir, "index.tsv")) as f:
        for line in f:
            parts = line.rstrip("\n").split("\t")
            index[parts[0]] = parts
    # Maker-site PDF texts per maker, for the sibling check.
    texts = collections.defaultdict(dict)
    for r in rows:
        if r["decision"] in ("candidate", "check") and r["pdf"] == "1" and own_site(r["maker"], r["datasheet_url"]):
            m = SHORT[r["maker"]]
            if r["datasheet_url"] not in texts[m]:
                texts[m][r["datasheet_url"]] = norm(text(pdf_dir, r["datasheet_url"]))
    shas = {}
    canon = {}
    for m in texts:
        by_sha = collections.defaultdict(list)
        for u in texts[m]:
            by_sha[sha_of(pdf_dir, u, shas)].append(u)
        for group in by_sha.values():
            for u in group:
                canon[u] = sorted(group)[0]
    v = verdicts()
    unresolved = []
    for r in rows:
        if r["decision"] not in ("candidate", "check"):
            continue
        note = "check; mounting from the datasheet" if r["decision"] == "check" else ""
        m = SHORT[r["maker"]]
        if not (r["pdf"] == "1" and own_site(r["maker"], r["datasheet_url"])):
            key = norm(r["mpn"])
            sib = sorted(u for u, t in texts[m].items() if key.strip() and key in t)
            if not sib:
                r["decision"] = "skip"
                r["reason"] = "; ".join(x for x in (note, "no datasheet on the maker's site ("
                                                    + why(index, r["datasheet_url"]) + ")") if x)
                continue
            body = texts[m][sib[0]]
            at = body.find(key)
            r["reason"] = "; ".join(x for x in (note, "datasheet from a same-maker PDF (DigiKey link: "
                                                + (host_of(r["datasheet_url"]) or "none") + ")") if x)
            r["datasheet_url"] = sib[0]
            r["pdf"], r["hit"] = "1", "1"
            r["dead"] = "1" if DEAD.search(body[max(0, at - 300):at + 300]) else "0"
        else:
            r["reason"] = note
        r["datasheet_url"] = canon.get(r["datasheet_url"], r["datasheet_url"])
        if r["hit"] == "1" and r["dead"] == "1":
            verdict = v.get((r["mpn"], r["datasheet_url"]))
            if verdict is None:
                unresolved.append(r)
                r["decision"] = "candidate" if not note else "check"
                continue
            if verdict["verdict"] == "out":
                r["decision"] = "out"
                r["reason"] = "; ".join(x for x in (r["reason"], f"datasheet status: {verdict['word']} (p.{verdict['page']})") if x)
            else:
                r["decision"] = "add"
        elif r["hit"] == "1":
            r["decision"] = "add"
        else:
            r["decision"] = "pending"
    write(tsv, rows, cols)
    counts = collections.Counter(r["decision"] for r in rows)
    for k in sorted(counts):
        print(counts[k], k)
    for r in unresolved:
        print("no dead verdict:", r["mpn"], r["datasheet_url"])
    used = {r["datasheet_url"] for r in rows if r["decision"] in ("add", "pending")}
    tsv_placed = os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")
    if os.path.exists(tsv_placed):
        placed = list(csv.DictReader(open(tsv_placed), delimiter="\t"))
        keep = {p["path"] for p in placed if p["url"] in used}
        for p in sorted({p["path"] for p in placed} - keep):
            print("unused PDF:", p)
    return 1 if unresolved else 0


def dead_report(argv):
    """The text around each unresolved dead row's part number, page by page."""
    import subprocess
    tsv, pdf_dir = argv
    rows, _ = read(tsv)
    v = verdicts()
    for r in rows:
        if r["decision"] not in ("candidate", "check") or r["hit"] != "1" or r["dead"] != "1":
            continue
        if (r["mpn"], r["datasheet_url"]) in v:
            continue
        pdf = os.path.join(pdf_dir, md5(r["datasheet_url"]) + ".pdf")
        pages = subprocess.run(["pdftotext", "-layout", pdf, "-"], capture_output=True,
                               text=True, errors="replace").stdout.split("\f")
        key = norm(r["mpn"])
        for i, page in enumerate(pages, 1):
            body = norm(page)
            if key in body:
                for line in page.splitlines():
                    if DEAD.search(line.upper()) or norm(line).find(key) >= 0:
                        print(f"{r['mpn']}\t{r['datasheet_url']}\tp.{i}\t{line.strip()[:160]}")
    return 0


def section(rows):
    groups = collections.defaultdict(list)
    for r in rows:
        if r["decision"] == "skip":
            groups[(SHORT[r["maker"]], r["leaf"], r["reason"])].append(r)
    lines = [HEAD, "",
             "In-stock Active / Last Time Buy diodes of the three makers of plan-29 (Vishay",
             "General Semiconductor - Diodes Division, onsemi, Taiwan Semiconductor;",
             "`planning/plan-29-data/candidates.tsv`, DigiKey categories Single Diodes, Diode",
             "Arrays, Bridge Rectifiers, Single Zener Diodes, Zener Diode Arrays) left out.",
             "This section is written by `planning/plan-29-data/census/decide.py --skipped`.",
             "", "| Part(s) | Maker | Would have gone into | Why skipped |", "|---|---|---|---|"]
    for (maker, leaf, reason) in sorted(groups):
        g = sorted(groups[(maker, leaf, reason)], key=lambda r: r["mpn"])
        parts = ", ".join(r["mpn"] for r in g)
        lines.append(f"| {parts} | {maker} | `diode` / Discrete → Diodes → {leaf} | {reason} |")
    return "\n".join(lines) + "\n"


def write_skipped():
    rows, _ = read(CAND)
    body = open(SKIPPED).read()
    new = section(rows)
    if HEAD in body:
        start = body.index(HEAD)
        nxt = body.find("\n## ", start + len(HEAD))
        body = body[:start] + new + ("" if nxt < 0 else "\n" + body[nxt + 1:])
    else:
        body = body.rstrip("\n") + "\n\n" + new
    open(SKIPPED, "w").write(body)
    print(sum(1 for r in rows if r["decision"] == "skip"), "skip rows written")
    return 0


def verify_skipped():
    rows, _ = read(CAND)
    body = open(SKIPPED).read()
    if HEAD not in body:
        print("no section", HEAD)
        return 1
    start = body.index(HEAD)
    nxt = body.find("\n## ", start + len(HEAD))
    part = body[start:] if nxt < 0 else body[start:nxt]
    names = set()
    for line in part.splitlines():
        if line.startswith("| ") and not line.startswith("| Part"):
            names.update(n.strip() for n in line.split("|")[1].split(","))
    missing = [r["mpn"] for r in rows if r["decision"] == "skip" and r["mpn"] not in names]
    for m in missing:
        print("missing", m)
    print(f"{len(missing)} skip rows missing from skipped-parts.md")
    return 1 if missing else 0


if __name__ == "__main__":
    a = sys.argv[1:]
    if a == ["--skipped"]:
        sys.exit(write_skipped())
    if a == ["--verify-skipped"]:
        sys.exit(verify_skipped())
    if a and a[0] == "--dead":
        sys.exit(dead_report(a[1:]))
    sys.exit(decide(a))
