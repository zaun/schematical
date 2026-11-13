"""plan-36-A Phase 2 (§ 3 rule 3): a datasheet for the rows with no maker PDF.

usage: python3 -I sibling.py <cand.tsv> <txt-dir>

For each `scope` = in row whose `pdf` is empty after textscan.py (a
product-page URL, a distributor copy, no URL, or a download that failed):

  1. the PDFs other rows of the same device use (same maker), preferring
     one in which the part number prints verbatim (textscan.py's rule);
  2. otherwise any downloaded PDF of the same maker whose first page prints
     the device name, a Microchip list title included (`AVR32LA14/20/28/32`
     names AVR32LA28).

The first PDF found becomes the row's `pdf` (an md5, as textscan.py writes
it), with `hit` / `package_code` / `dead` from that PDF. A row with none is
`verdict` = skip, reason `no datasheet on the maker's site`. Rewrites
<cand.tsv> in place and prints the counts.
"""
import csv
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import textscan as T  # noqa: E402


def title_devices(page):
    """Device names a first page prints, with Microchip's list titles expanded:
    `AVR32LA14/20/28/32` and `AVR16/32DD28/32` name every combination."""
    out = set()
    for m in re.finditer(r"AVR((?:\d+/)*\d+)([A-Z]{2})((?:\d+/)*\d+)", page):
        for f in m.group(1).split("/"):
            for p in m.group(3).split("/"):
                out.add(f"AVR{f}{m.group(2)}{p}")
    return out


def main(tsv, txt_dir):
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    cols = list(rows[0].keys())
    bodies = {}

    def body(h):
        if h not in bodies:
            p = os.path.join(txt_dir, h + ".txt")
            b = open(p, errors="replace").read() if os.path.exists(p) else None
            bodies[h] = None if b is None else (b, T.legend(b))
        return bodies[h]

    by_device, by_maker = {}, {}
    for r in rows:
        if r["scope"] == "in" and r["pdf"]:
            by_device.setdefault((r["maker"], r["device"].upper()), []).append(r["pdf"])
            by_maker.setdefault(r["maker"], set()).add(r["pdf"])
    placed = skipped = 0
    for r in rows:
        if r["scope"] != "in" or r["pdf"]:
            continue
        choice = None
        same = sorted(set(by_device.get((r["maker"], r["device"].upper()), [])))
        for h in same:
            b = body(h)
            if b and T.scan(r, b[0], {})[0] == "verbatim":
                choice = h
                break
        if choice is None and same:
            choice = same[0]
        if choice is None:
            dev = re.compile(r"(?<![A-Z0-9])" + re.escape(r["device"].upper()) + r"(?![0-9])")
            for h in sorted(by_maker.get(r["maker"], ())):
                b = body(h)
                page = b[0].split("\f", 1)[0].upper() if b else ""
                if b and (dev.search(page) or r["device"].upper() in title_devices(page)):
                    choice = h
                    break
        if choice is None:
            r["verdict"], r["reason"] = "skip", "no datasheet on the maker's site"
            skipped += 1
            continue
        r["pdf"] = choice
        b = body(choice)
        r["hit"], r["package_code"], r["dead"] = T.scan(r, b[0], b[1] if r["family"] in T.CLASSIC else {})
        placed += 1
    with open(tsv, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    print(f"sibling PDFs given {placed}, skipped (no datasheet on the maker's site) {skipped}")


if __name__ == "__main__":
    main(*sys.argv[1:3])
