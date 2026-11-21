#!/usr/bin/env python3
"""plan-42-B Phase 1-3: propose `scheme` and `label` for the spec rows.

usage: python3 -I auto-scheme.py <candidates.tsv> <specs.tsv> <out.tsv> [--list]

For every spec row with a download-set row that is not printed verbatim: the
orderable name of such a row is looked up in its PDF's text after removing a
packing suffix the maker uses (ADI / Linear `#TRPBF`, `#PBF`, `-R7`; Maxim `+T`,
`+`; Microchip `-TR`, ` TR`, `T` of `MCP…T-`; TI `/NOPB`, a trailing reel
letter): when that shorter name stands in the text between token boundaries,
the row is in the datasheet's ordering table. The spec row's `scheme` is then
the regex `(?:<core>|<core>…)(?:<packing suffix class>)?` over every core found
for the group; a group whose non-verbatim rows are not all found is not filled
(they are listed with --list, for reading by hand). `label` is the DigiKey
base part number of the group's most-stocked row when that is a clean device
name (letters, digits, `-`), else the leading letters-and-digits run of its
part number. Writes <out.tsv> in specs-fill.py's format (pdf, package, series,
scheme, label, note).
"""
import collections
import os
import re
import subprocess
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

PACK = {
    "Analog Devices": [r"#TRM?PBF$", r"#PBF$", r"#TRM?$", r"-R[257]$", r"-REEL7?$", r"-RL7?$", r"\+T$", r"\+$",
                       r"-TR7?$", r"-T$"],
    "Microchip Technology": [r"[ -]?TR$", r"-T$", r"[ -]?T&R$", r"T$"],
    "Texas Instruments": [r"/NOPB$", r"[RTX]$"],
}
PACK_RE = {
    "Analog Devices": r"(?:#TRM?PBF|#PBF|#TRM?|-R[257]|-REEL7?|-RL7?|\+T|\+|-TR7?|-T)?",
    "Microchip Technology": r"(?:[ -]?TR|-T|[ -]?T&R|T)?",
    "Texas Instruments": r"(?:/NOPB)?[RTX]?",
}
TXT = "/tmp/p42/txt"


def text_of(pdf):
    os.makedirs(TXT, exist_ok=True)
    out = os.path.join(TXT, pdf.replace("/", "_") + ".txt")
    if not os.path.exists(out):
        subprocess.run(["pdftotext", "-q", "-layout", os.path.join(common.ROOT, pdf), out], check=False, timeout=240)
    return common.norm(open(out, errors="replace").read()) if os.path.exists(out) else ""


def cores(mpn, maker):
    """mpn and what remains after removing packing suffixes in any order, plus
    the MCP `T-` form without its T."""
    seen = {mpn}
    todo = [mpn]
    while todo:
        s = todo.pop()
        for pat in PACK[maker]:
            t = re.sub(pat, "", s)
            if t != s and len(t) >= 4 and t not in seen:
                seen.add(t)
                todo.append(t)
        m = re.match(r"^(MCP\d+)T(-.*)$", s)
        if m and m.group(1) + m.group(2) not in seen:
            seen.add(m.group(1) + m.group(2))
            todo.append(m.group(1) + m.group(2))
    return sorted(seen, key=lambda c: -len(c))


def label_of(rs):
    r = max(rs, key=lambda r: (int(r["stock"] or 0), r["mpn"]))
    base = r["base"].strip()
    if re.fullmatch(r"[A-Za-z0-9-]+", base) and len(base) <= 16:
        return base.upper()
    m = re.match(r"[A-Za-z]+[0-9]+[A-Za-z]?", r["mpn"])
    return (m.group(0) if m else base).upper()


def main(cand, spec_path, out_path, listing):
    rows = common.load(cand)
    common.init(rows)
    table = specs.load(spec_path)
    groups = collections.defaultdict(list)
    for r in rows:
        if common.eligible(r) and r["pdf"]:
            groups[specs.key(r)].append(r)
    out = ["\t".join(["pdf", "package", "series", "scheme", "label", "note"])]
    solved = unsolved = 0
    for k in sorted(groups):
        rs = groups[k]
        label = label_of(rs)
        bad = [r for r in rs if r["hit"] != "1"]
        scheme = ""
        note = ""
        if bad:
            text = text_of(k[2])
            found = set()
            miss = []
            for r in bad:
                hit = [c for c in cores(r["mpn"], k[0]) if common.norm(c) in text]
                if hit:
                    found.add(hit[0])
                else:
                    miss.append(r["mpn"])
            if miss:
                unsolved += 1
                note = "unresolved: " + " ".join(miss[:6])
                if listing:
                    print("unresolved", k[0][:3], k[2].split("/")[-1], "|", k[3], "|", " ".join(miss[:4]))
            else:
                solved += 1
                scheme = "(?:" + "|".join(re.escape(c) for c in sorted(found)) + ")" + PACK_RE[k[0]]
                if not all(specs.matches({"scheme": scheme}, r["mpn"]) for r in bad):
                    scheme = ""
                    note = "regex does not cover"
                    unsolved += 1
                    solved -= 1
        out.append("\t".join([k[2], k[3], k[1], scheme, label, note]))
    with open(out_path, "w") as fh:
        fh.write("\n".join(out) + "\n")
    print(len(groups), "spec rows;", solved, "schemes proposed;", unsolved, "groups to read by hand")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], sys.argv[3], "--list" in sys.argv)
