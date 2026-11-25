"""plan-58-A Phase 3: decisions (plan-58-A section 4.1) and the skipped-parts section.

usage: python3 -I decide.py <candidates.tsv> <pdf-dir>   rewrite `decision` / `reason` in place
       python3 -I decide.py --skipped                      rewrite the "Slide switches (plan-58)"
                                                           section of datasheets/skipped-parts.md
       python3 -I decide.py --verify-skipped               every skip row is in that section
       python3 -I decide.py --selftest

<candidates.tsv> must carry the columns `pdf`, `hit`, `dead` (textscan.py's output).
Rules, in this order, for every row `build.py` left `add` (and, so the script
can run again, every row an earlier run made add / pending / skip; rows `out`
stay as they are):
  0. every row of one datasheet has dead = 1  -> out, reason `obsolete`
     (AGENTS.md: the PDF is then deleted and recorded in obsolete-not-included.md)
  1. pdf = 0                                  -> skip, reason `no datasheet (<why>)`
  2. hit = 1                                  -> add
  3. hit = 0                                  -> pending, reason `number not in datasheet text`
A row with dead = 1 whose datasheet has live rows too stays add / pending; the
parts letter flags it "obsolete".
"""
import collections
import csv
import hashlib
import os
import sys
import urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
CAND = os.path.join(ROOT, "planning", "plan-58-data", "candidates.tsv")
SKIPPED = os.path.join(ROOT, "datasheets", "skipped-parts.md")
HEAD = "## Slide switches (plan-58)"
PENDING = "number not in datasheet text"
OWN = ("add", "pending", "skip")  # decisions this script may rewrite


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
    host = urllib.parse.urlparse("https:" + url if url.startswith("//") else url).netloc
    if not url:
        return "no URL"
    entry = index.get(hashlib.md5(url.encode()).hexdigest())
    if entry is None:
        return f"not downloaded, {host}"
    code = entry[1]
    if code in ("000", "none", ""):
        return f"no answer, {host}"
    if code == "timeout":
        return f"timeout, {host}"
    if code == "truncated":
        return f"download cut off every time, {host}"
    if code[:1] in "23":
        return f"product page, not a PDF, {host}"
    return f"HTTP {code}, {host}"


def decide_rows(rows, index):
    mine = [r for r in rows if r["decision"] in OWN]
    by_url = collections.defaultdict(list)
    for r in mine:
        by_url[r["datasheet"]].append(r)
    for r in mine:
        group = by_url[r["datasheet"]]
        if r["pdf"] == "1" and all(g["dead"] == "1" for g in group):
            r["decision"], r["reason"] = "out", "obsolete"
        elif r["pdf"] != "1":
            r["decision"], r["reason"] = "skip", "no datasheet (" + why(index, r["datasheet"]) + ")"
        elif r["hit"] == "1":
            r["decision"], r["reason"] = "add", ""
        else:
            r["decision"], r["reason"] = "pending", PENDING
    return rows


def decide(tsv, pdf_dir):
    rows, cols = read(tsv)
    for need in ("pdf", "hit", "dead"):
        if need not in cols:
            print(f"{tsv} has no `{need}` column: copy textscan.py's output over it first")
            return 1
    index = {}
    with open(os.path.join(pdf_dir, "index.tsv")) as f:
        for line in f:
            parts = line.rstrip("\n").split("\t")
            index[parts[0]] = parts
    decide_rows(rows, index)
    write(tsv, rows, cols)
    counts = collections.Counter(r["decision"] for r in rows)
    for k in sorted(counts):
        print(counts[k], k)
    return 0


def section(rows):
    skips = [r for r in rows if r["decision"] == "skip"]
    lines = [HEAD, "",
             "In-stock Active SPST / SPDT / DPST / DPDT slide switches of C&K / Littelfuse, NKK, CIT Relay &",
             "Switch and E-Switch (`planning/plan-58-data/candidates.tsv`, DigiKey category Slide Switches)",
             "left out for want of a datasheet PDF. Written by `planning/plan-58-data/census/decide.py --skipped`.",
             "Rows left out by rule (panel mount, momentary, line-voltage, illuminated, no DigiKey stock) are",
             "`out` in that file and are not listed here.", ""]
    if not skips:
        lines.append("None: every kept row's datasheet downloaded as a PDF.")
        return "\n".join(lines) + "\n"
    groups = collections.defaultdict(list)
    for r in skips:
        groups[(r["maker"], r["series"], r["reason"])].append(r)
    lines += ["| Part(s) | Series | Would have gone into | Why skipped |", "|---|---|---|---|"]
    for (maker, series, reason) in sorted(groups):
        parts = ", ".join(sorted(r["mpn"] for r in groups[(maker, series, reason)]))
        lines.append(f"| {parts} | {maker} {series} | `switch` / Switches & Relays → Slide | {reason} |")
    return "\n".join(lines) + "\n"


def splice(text, new):
    if HEAD in text:
        start = text.index(HEAD)
        nxt = text.find("\n## ", start + len(HEAD))
        return text[:start] + new + ("" if nxt < 0 else "\n" + text[nxt + 1:])
    return text.rstrip("\n") + "\n\n" + new


def write_skipped():
    rows, _ = read(CAND)
    new_text = splice(open(SKIPPED).read(), section(rows))  # read BEFORE open(..., "w") truncates the file
    open(SKIPPED, "w").write(new_text)
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
    bad = [r["mpn"] for r in rows if r["decision"] == "check"]
    for m in bad:
        print("still `check`", m)
    print(f"{len(missing)} skip rows missing from skipped-parts.md, {len(bad)} rows `check`")
    return 1 if missing or bad else 0


def selftest():
    def row(mpn, url, pdf, hit, dead, decision="add"):
        return {"mpn": mpn, "datasheet": url, "pdf": pdf, "hit": hit, "dead": dead,
                "decision": decision, "reason": "", "maker": "M", "series": "S"}
    rows = [row("A", "u1", "1", "1", "0"), row("B", "u1", "1", "0", "0"),
            row("C", "u2", "0", "0", "0"), row("D", "u3", "1", "1", "1"),
            row("E", "u3", "1", "0", "1"), row("F", "u4", "1", "1", "1"), row("G", "u4", "1", "1", "0"),
            row("H", "u5", "1", "1", "0", decision="out")]
    decide_rows(rows, {})
    got = {r["mpn"]: r["decision"] for r in rows}
    assert got == {"A": "add", "B": "pending", "C": "skip", "D": "out", "E": "out", "F": "add", "G": "add", "H": "out"}, got
    decide_rows(rows, {})  # idempotent
    assert {r["mpn"]: r["decision"] for r in rows} == got
    assert "None" in section([r for r in rows if r["decision"] != "skip"])
    assert "| C |" in section(rows)
    print("ok")
    return 0


if __name__ == "__main__":
    a = sys.argv[1:]
    if a == ["--selftest"]:
        sys.exit(selftest())
    if a == ["--skipped"]:
        sys.exit(write_skipped())
    if a == ["--verify-skipped"]:
        sys.exit(verify_skipped())
    if len(a) != 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(decide(*a))
