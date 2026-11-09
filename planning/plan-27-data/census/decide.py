"""plan-27 census, step 8: first decisions (plan-27-A section 3, rules 1-5).

usage: python3 -I decide.py <candidates.tsv> <pdf-dir>   rewrite `decision`/`reason` in place
       python3 -I decide.py --skipped                      rewrite the plan-27 section of
                                                           datasheets/skipped-parts.md from the
                                                           skip rows of candidates.tsv
       python3 -I decide.py --verify-skipped               every skip row is in that section

Mapping of a `candidate` / `check` row (rows already add/pending/skip/out are kept):
  pdf = 1, dead = 1           -> out      datasheet status
  pdf = 1, hit = 1            -> add
  pdf = 1, hit = 0            -> pending  (plan-27-B / C read the part-number key)
  pdf = 0                     -> skip     no datasheet PDF (<why>, from <pdf-dir>/index.tsv)
A `check` row keeps its "DigiKey parameters blank" note in the reason
(`check; ...`) so B and C can find it; its circuit and lighting are read there.
"""
import collections
import csv
import hashlib
import os
import sys
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
CAND = os.path.join(ROOT, "planning", "plan-27-data", "candidates.tsv")
SKIPPED = os.path.join(ROOT, "datasheets", "skipped-parts.md")
HEAD = "## Tactile switches (plan-27)"

# plan-27-A section 3 rule 3: the maker's own site, DigiKey or Waldom.
HOSTS = ("littelfuse.com", "ckswitches.com", "e-switch.com", "configured-product-images.s3.amazonaws.com",
         "sten-eswitch-13110800-production.s3.amazonaws.com", "omron.com", "panasonic.com",
         "panasonic.biz", "panasonic.eu", "alpsalpine.com", "sameskydevices.com", "cuidevices.com",
         "te.com", "apem.com", "nkkswitches.com", "we-online.com", "mm.digikey.com", "waldomemea.com")


def host_of(url):
    return urllib.parse.urlparse("https:" + url if url.startswith("//") else url).netloc


def allowed(url):
    h = host_of(url)
    return any(h == x or h.endswith("." + x) for x in HOSTS)


def read(path):
    with open(path) as f:
        rows = list(csv.DictReader(f, delimiter="\t"))
    return rows, list(rows[0].keys())


def write(path, rows, cols):
    with open(path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")


def why(index, url):
    if not url:
        return "no URL"
    entry = index.get(hashlib.md5(url.encode()).hexdigest())
    host = urllib.parse.urlparse("https:" + url if url.startswith("//") else url).netloc
    if entry is None:
        return f"not downloaded, {host}"
    code = entry[1]
    if code == "timeout":
        return f"timeout, {host}"
    if code in ("000", "none", ""):
        return f"no answer, {host}"
    if code.startswith("2") or code.startswith("3"):
        return f"product page, not a PDF, {host}"
    if code == "403" and "omronfs" in host:
        return f"S3 AccessDenied, {host}"
    return f"HTTP {code}, {host}"


def decide(argv):
    tsv, pdf_dir = argv
    rows, cols = read(tsv)
    index = {}
    with open(os.path.join(pdf_dir, "index.tsv")) as f:
        for line in f:
            parts = line.rstrip("\n").split("\t")
            index[parts[0]] = parts
    for r in rows:
        if r["decision"] not in ("candidate", "check"):
            continue
        note = "check; " if r["decision"] == "check" else ""
        if r["pdf"] == "1" and not allowed(r["datasheet_url"]):
            r["decision"], r["reason"] = "skip", note + "distributor copy (" + host_of(r["datasheet_url"]) + "), not the maker's, DigiKey's or Waldom's"
        elif r["pdf"] == "1" and r["dead"] == "1":
            r["decision"], r["reason"] = "out", note + "datasheet status"
        elif r["pdf"] == "1" and r["hit"] == "1":
            r["decision"], r["reason"] = "add", note.rstrip("; ")
        elif r["pdf"] == "1":
            r["decision"], r["reason"] = "pending", note.rstrip("; ")
        else:
            r["decision"], r["reason"] = "skip", note + "no datasheet PDF (" + why(index, r["datasheet_url"]) + ")"
    write(tsv, rows, cols)
    counts = collections.Counter(r["decision"] for r in rows)
    for k in sorted(counts):
        print(counts[k], k)
    return 0


def maker_name(m):
    return m.split(" (")[0]


def section(rows):
    groups = collections.defaultdict(list)
    for r in rows:
        if r["decision"] == "skip":
            groups[(maker_name(r["maker"]), r["reason"])].append(r)
    lines = [HEAD, "",
             "In-stock Active / Last Time Buy tactile switches of the ten makers of plan-27",
             "(`planning/plan-27-data/candidates.tsv`, DigiKey category Tactile Switches) left",
             "out. This section is written by `planning/plan-27-data/census/decide.py --skipped`.",
             "", "| Part(s) | Series | Would have gone into | Why skipped |", "|---|---|---|---|"]
    for (maker, reason) in sorted(groups):
        g = sorted(groups[(maker, reason)], key=lambda r: r["mpn"])
        series = ", ".join(sorted({r["series"] for r in g}))
        parts = ", ".join(r["mpn"] for r in g)
        lines.append(f"| {parts} | {maker} {series} | `switch` / Switch → Tactile | {reason} |")
    return "\n".join(lines) + "\n"


def write_skipped():
    rows, _ = read(CAND)
    text = open(SKIPPED).read()
    new = section(rows)
    if HEAD in text:
        start = text.index(HEAD)
        nxt = text.find("\n## ", start + len(HEAD))
        text = text[:start] + new + ("" if nxt < 0 else "\n" + text[nxt + 1:])
    else:
        text = text.rstrip("\n") + "\n\n" + new
    open(SKIPPED, "w").write(text)
    print(sum(1 for r in rows if r["decision"] == "skip"), "skip rows written")
    return 0


def verify_skipped():
    rows, _ = read(CAND)
    text = open(SKIPPED).read()
    if HEAD not in text:
        print("no section", HEAD)
        return 1
    start = text.index(HEAD)
    nxt = text.find("\n## ", start + len(HEAD))
    body = text[start:] if nxt < 0 else text[start:nxt]
    names = set()
    for line in body.splitlines():
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
    sys.exit(decide(a))
