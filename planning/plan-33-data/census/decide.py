"""plan-33-B section 3.3: the pick walk.

usage: python3 -I decide.py <candidates.tsv> <pdf-dir>   decide every bucket in place
       python3 -I decide.py --skipped                    rewrite the "Capacitors (plan-33)"
                                                         section of datasheets/skipped-parts.md
       python3 -I decide.py --verify-skipped             every skip row is in that section
       python3 -I decide.py --empty                      write buckets-empty.tsv

Per bucket, in `rank` order over the rows scheme-check.py checked:
  check != ok                      -> skip ("no datasheet on the maker's site (<why>)" or
                                      "not in the datasheet's ordering table (<test>, <pdf>)")
  check == ok, a datasheet-status verdict (census/dead-verdicts.tsv) of `out`
                                   -> out ("datasheet status: <word> (p.<page>)")
  check == ok otherwise            -> add; every lower-ranked row -> alt
The walk restarts from the rows' first decision each run (rows decided
add / alt / skip / out by an earlier run go back to candidate first), so it
can be run again after more ranks are scanned.  A bucket whose checked rows
all fail is listed as `deeper: <bucket> (<n> checked, <m> unchecked)` when
unchecked rows remain (scan ranks 11-50: fetch.py / textscan.py
--max-rank 50), or `more: <bucket>` when every listed row failed and the
bucket's DigiKey count is above the rows listed (bucket-more.mjs), else
`empty: <bucket>`.

Run the walk before `place.py --prune`: it looks rows' PDFs up in
datasheets/capacitor/datasheets.tsv, and a pruned PDF's rows would lose their
status verdict (census/military-out.py changes decisions after the prune
without a walk).

dead-verdicts.tsv: `pdf` (the datasheet path), `pattern` (a regex the part
number must fullmatch; `.*` for every part of the PDF), `page`, `word`,
`verdict` (`out` or `add`).  Every `ok` row whose PDF has a dead word
(OBSOLETE / DISCONTINUED / NOT RECOMMENDED FOR NEW / END OF LIFE / NRND,
dead-scan.py) must match a verdict line; otherwise it is listed as
`no dead verdict` and the bucket is left undecided (exit 1).
"""
import collections
import csv
import hashlib
import os
import re
import sys
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
CAND = os.path.join(DATA, "candidates.tsv")
VERDICTS = os.path.join(HERE, "dead-verdicts.tsv")
DEADPDFS = os.path.join(HERE, "dead-pdfs.txt")
PLACED = os.path.join(ROOT, "datasheets", "capacitor", "datasheets.tsv")
SKIPPED = os.path.join(ROOT, "datasheets", "skipped-parts.md")
EMPTY = os.path.join(DATA, "buckets-empty.tsv")
HEAD = "## Capacitors (plan-33)"
SHORT = {"KEMET": "KEMET", "Cal-Chip Electronics": "Cal-Chip", "KYOCERA AVX": "KYOCERA AVX"}
TYPE_LEAF = {"ceramic": "Ceramic", "aluminum": "Electrolytic", "al-polymer": "Electrolytic",
             "tantalum": "Tantalum", "ta-polymer": "Tantalum", "niobium": "Tantalum"}
DECIDED = ("add", "alt", "skip", "out")
MILITARY = re.compile(r"\bMIL\b|MIL-|Military|^CWR\d")


def full(url):
    return ("https:" + url if url.startswith("//") else url).replace(" ", "%20")


def host_of(url):
    return urllib.parse.urlparse(full(url)).netloc.lower()


def md5(url):
    return hashlib.md5(url.encode()).hexdigest()


def read(path):
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        return list(reader), list(reader.fieldnames)


def write(path, rows, cols):
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, cols, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)


def why(index, url):
    if not url:
        return "no URL"
    h = host_of(url)
    if h == "mm.digikey.com":
        return "DigiKey copy, mm.digikey.com"
    if "waldom" in h:
        return f"Waldom copy, {h}"
    entry = index.get(md5(url))
    if entry is None:
        return f"not downloaded, {h}"
    if entry[3] == "%PDF":
        return f"a copy on another site, {h}"
    code = entry[1]
    if code in ("000", "none", "", "timeout"):
        return f"no answer, {h}"
    if code.startswith("2") or code.startswith("3"):
        return f"not a PDF, {h}"
    return f"HTTP {code}, {h}"


def verdicts():
    out = collections.defaultdict(list)
    if os.path.exists(VERDICTS):
        for v in csv.DictReader(open(VERDICTS), delimiter="\t"):
            out[v["pdf"]].append(v)
    return out


def dead_pdfs():
    if not os.path.exists(DEADPDFS):
        return set()
    return {line.strip() for line in open(DEADPDFS) if line.strip()}


def decide(argv):
    tsv, pdf_dir = argv
    rows, cols = read(tsv)
    index = {}
    with open(os.path.join(pdf_dir, "index.tsv")) as f:
        for line in f:
            parts = line.rstrip("\n").split("\t")
            index[parts[0]] = parts
    paths = {p["url"]: p["path"] for p in csv.DictReader(open(PLACED), delimiter="\t")}
    v = verdicts()
    dead = dead_pdfs()
    raw_counts = {}
    buckets = collections.defaultdict(list)
    for r in rows:
        if not r["rank"]:
            continue  # out before ranking (select.py: military, wet tantalum, bi-polar)
        if MILITARY.search(r["series"] + " " + r["ratings"]):
            # Military by series name too (`Military, MIL-C-11015, CK12`, `CWR11`), which
            # select.py's Ratings test missed (plan-33-D section Corrections).
            r["decision"], r["reason"] = "out", "military"
            continue
        if r["decision"] in DECIDED:
            r["decision"], r["reason"] = "candidate", ""
        if r["decision"] == "candidate":
            buckets[r["bucket"]].append(r)
    unresolved, notes = [], []
    for bucket, group in buckets.items():
        group.sort(key=lambda r: int(r["rank"]))
        picked = False
        checked = 0
        for r in group:
            if picked:
                r["decision"], r["reason"] = "alt", ""
                continue
            c = r.get("check", "")
            if c == "":
                break
            checked += 1
            if c == "no maker pdf":
                r["decision"] = "skip"
                r["reason"] = "no datasheet on the maker's site (" + why(index, r["datasheet_url"]) + ")"
                continue
            path = paths.get(r["datasheet_url"], "")
            if c != "ok":
                r["decision"] = "skip"
                r["reason"] = f"not in the datasheet's ordering table ({c}, {os.path.basename(path)})"
                continue
            if path in dead:
                match = [x for x in v.get(path, []) if re.fullmatch(x["pattern"], r["mpn"])]
                if not match:
                    unresolved.append((r["mpn"], path))
                    r["decision"], r["reason"] = "candidate", ""
                    break
                if match[0]["verdict"] == "out":
                    r["decision"] = "out"
                    r["reason"] = f"datasheet status: {match[0]['word']} (p.{match[0]['page']})"
                    continue
            r["decision"], r["reason"] = "add", ""
            picked = True
        if not picked:
            undecided = [r for r in group if r["decision"] == "candidate"]
            if undecided and undecided[0].get("check", "") == "":
                notes.append(f"deeper: {bucket} ({checked} checked, {len(undecided)} unchecked)")
            elif not undecided:
                notes.append(f"failed: {bucket} ({checked} checked)")
    write(tsv, rows, cols)
    counts = collections.Counter(r["decision"] for r in rows)
    print(", ".join(f"{counts[k]} {k}" for k in sorted(counts)))
    print(len({r['bucket'] for r in rows if r['decision'] == 'add'}), "buckets with an add row")
    for n in sorted(notes):
        print(n)
    for m, p in unresolved:
        print("no dead verdict:", m, p)
    return 1 if unresolved else 0


def section(rows):
    groups = collections.defaultdict(list)
    for r in rows:
        if r["decision"] == "skip":
            groups[(SHORT[r["maker"]], TYPE_LEAF[r["type"]], r["reason"])].append(r)
    lines = [HEAD, "",
             "In-stock Active / Last Time Buy capacitors of the three makers of plan-33",
             "(KEMET, Cal-Chip Electronics, KYOCERA AVX; ceramic, aluminum electrolytic and",
             "polymer, tantalum, tantalum polymer and niobium oxide) left out. plan-33 takes one",
             "part per bucket (type, package, E6 capacitance, and the dielectric class or rated",
             "voltage; `planning/plan-33-data/candidates.tsv`): a bucket's parts are tried in",
             "pick order and the first that passes is taken, so only the parts ranked above a",
             "bucket's pick were tried, and only those are listed here. This section is written",
             "by `planning/plan-33-data/census/decide.py --skipped`.",
             "", "| Part(s) | Maker | Would have gone into | Why skipped |", "|---|---|---|---|"]
    for (maker, leaf, reason) in sorted(groups):
        g = sorted({r["mpn"] for r in groups[(maker, leaf, reason)]})
        lines.append(f"| {', '.join(g)} | {maker} | `capacitor` / Capacitors → {leaf} | {reason} |")
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


def write_empty():
    rows, _ = read(CAND)
    by = collections.defaultdict(list)
    for line in open(os.path.join(DATA, "buckets.tsv")).read().splitlines()[1:]:
        if line.strip():
            label, cap = line.split("\t")[:2]
            by[f"{label}|{cap}"] = []
    for r in rows:
        by[r["bucket"]].append(r)
    with open(EMPTY, "w") as out:
        out.write("bucket\ttried\treasons\n")
        n = 0
        for b in sorted(by):
            g = by[b]
            if any(r["decision"] == "add" for r in g):
                continue
            tried = [r for r in g if r["decision"] in ("skip", "out")]
            reasons = collections.Counter(re.sub(r" \(.*", "", r["reason"]) for r in tried)
            text = "; ".join(f"{k} x{c}" for k, c in reasons.most_common()) or "no part listed"
            out.write(f"{b}\t{len(tried)}\t{text}\n")
            n += 1
    print(n, "empty buckets")
    return 0


if __name__ == "__main__":
    a = sys.argv[1:]
    if a == ["--skipped"]:
        sys.exit(write_skipped())
    if a == ["--verify-skipped"]:
        sys.exit(verify_skipped())
    if a == ["--empty"]:
        sys.exit(write_empty())
    sys.exit(decide(a))
