"""plan-36-A Phase 3: apply specs.tsv and the selection rules; fill verdict.

usage: python3 -I verdicts.py <cand.tsv> <specs.tsv> [--report] [--txt <txt-dir>] [--pdf <pdf-dir>]

specs.tsv: `pdf mpn_regex package_field note`, one or more rows per PDF (the
md5 textscan.py writes in `pdf`). For a row of that PDF:

  mpn_regex      matched in full against the part number, case aside; `-`
                 = no regex (the row only says where a verbatim hit's
                 package code comes from); a `V:` prefix = the regex only
                 reads the package of a verbatim hit (a table that prints
                 the package as a word, `QFN32`, or a code in its own
                 legend layout) and never makes a hit itself
  package_field  where the package code comes from:
                   pkg      the regex's named group `pkg`
                   pkg:A=X,B=Y   group `pkg` mapped to the datasheet's own
                            package codes (`AU=MD,MU=PS`)
                   base     the package code of the part number in group
                            `base`, which must print verbatim in the same PDF
                            with a code (a reel twin, `ATMEGA162-16AUR` of
                            `ATMEGA162-16AU`, where the datasheet says the R
                            suffix is tape and reel)
                   device   the row's device (the ESP32 modules: B § 3.1)
                   =CODE    this literal code for every row (`=QFN-56`)
  note           the datasheet page or figure the rule was read from

A regex with a `dev` group also needs that device name in the PDF's text
(textscan.names), so a scheme cannot claim a device the datasheet does not
cover.

Per in-scope row (verdict not already `skip` / `out`):
  1. a verbatim hit with a package code: `add`;
  2. else the first specs row of its PDF that applies: `hit` = `regex` (a
     verbatim hit stays `verbatim`), `package_code` from package_field: `add`;
  3. else the same tests on each other PDF a row of the same device uses,
     then on every other PDF of the same maker (row PDFs and extra-pdfs.tsv)
     whose text names the device (§ 3 rule 3's sibling rule, for a row whose
     DigiKey link is a different edition): the row's `pdf` becomes that PDF;
  4. else `skip`, reason `not in the datasheet's ordering table`.
package-overrides.tsv (beside specs.tsv) then replaces the package code of
the rows it lists. --report prints the counts only (no write); the last line
is `<n> undecided`: in-scope rows with no verdict.
"""
import csv
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import textscan as T  # noqa: E402

SKIP = "not in the datasheet's ordering table"


def load_specs(path):
    specs = {}
    for r in csv.DictReader(open(path), delimiter="\t"):
        if not r["pdf"] or r["pdf"].startswith("#"):
            continue
        text = r["mpn_regex"]
        only_verbatim = text.startswith("V:")
        text = text[2:] if only_verbatim else text
        rx = None if text in ("", "-") else re.compile(text, re.I)
        specs.setdefault(r["pdf"], []).append((rx, r["package_field"], r["note"], only_verbatim))
    return specs


class Texts:
    def __init__(self, txt_dir, pdf_dir):
        self.dir, self.pdf_dir, self.cache = txt_dir, pdf_dir, {}

    def get(self, h):
        if h not in self.cache:
            p = os.path.join(self.dir, h + ".txt")
            if not os.path.exists(p):
                T.text(self.pdf_dir, self.dir, h)
            if os.path.exists(p):
                body = T.dashes(open(p, errors="replace").read())
                self.cache[h] = (body, T.legend(body), T.names(body.upper()))
            else:
                self.cache[h] = None
        return self.cache[h]


def try_pdf(row, h, specs, texts):
    """(hit, package_code) of row against PDF h, or None."""
    got = texts.get(h)
    if got is None:
        return None
    body, codes, named = got
    hit, pkg, _ = T.scan(row, body, codes if row["family"] in T.CLASSIC else {})
    if hit and pkg:
        return hit, pkg
    for rx, field, _, only_verbatim in specs.get(h, []):
        m = None
        if only_verbatim and not hit:
            continue
        if rx is not None:
            m = rx.fullmatch(row["mpn"])
            if m is None:
                continue
            if "dev" in rx.groupindex and m.group("dev").upper() not in named:
                continue
        elif not hit:
            continue
        code = ""
        if field == "pkg" and m is not None:
            code = m.group("pkg").upper()
        elif field.startswith("pkg:") and m is not None:
            table = dict(x.split("=", 1) for x in field[4:].split(","))
            code = table.get(m.group("pkg").upper(), "")
        elif field == "base" and m is not None:
            base = dict(row, mpn=m.group("base"))
            bhit, bpkg, _ = T.scan(base, body, codes if row["family"] in T.CLASSIC else {})
            code = bpkg if bhit else ""
        elif field == "device":
            code = row["device"]
        elif field.startswith("="):
            code = field[1:]
        if code:
            return (hit or "regex"), code
    return None


def decide(rows, specs, texts, extra):
    by_device, by_maker = {}, {}
    for r in rows:
        if r["scope"] == "in" and r["pdf"]:
            by_device.setdefault((r["maker"], r["device"].upper()), set()).add(r["pdf"])
            by_maker.setdefault(r["maker"], set()).add(r["pdf"])
    for url, maker in extra:
        by_maker.setdefault(maker, set()).add(T.md5(url))
    for r in rows:
        if r["scope"] != "in" or (r["verdict"] in ("skip", "out") and r["reason"] != SKIP):
            continue
        r["verdict"], r["reason"] = "", ""
        got = try_pdf(r, r["pdf"], specs, texts) if r["pdf"] else None
        if got is None:
            same = sorted(by_device.get((r["maker"], r["device"].upper()), set()) - {r["pdf"]})
            named = [h for h in sorted(by_maker.get(r["maker"], set()) - set(same) - {r["pdf"]})
                     if texts.get(h) is not None and r["device"].upper() in texts.get(h)[2]]
            for h in same + named:
                got = try_pdf(r, h, specs, texts)
                if got is not None:
                    r["pdf"] = h
                    r["dead"] = T.scan(r, texts.get(h)[0], {})[2]
                    break
        if got is None:
            r["verdict"], r["reason"], r["hit"], r["package_code"] = "skip", SKIP, "", ""
            continue
        r["hit"], r["package_code"] = got
        r["verdict"] = "add"


def overrides(rows, path):
    if not os.path.exists(path):
        return 0
    fix = {r["mpn"]: r["package_code"] for r in csv.DictReader(open(path), delimiter="\t")}
    n = 0
    for r in rows:
        if r["mpn"] in fix and r["verdict"] == "add":
            r["package_code"] = fix[r["mpn"]]
            n += 1
    return n


def report(rows):
    from collections import Counter
    c = Counter((r["family"], r["verdict"] or "undecided") for r in rows if r["scope"] == "in")
    for k in sorted(c):
        print(k[0], k[1], c[k])
    hits = Counter(r["hit"] for r in rows if r["verdict"] == "add")
    print("add by hit:", dict(hits))
    print("add", sum(r["verdict"] == "add" for r in rows), "skip", sum(r["verdict"] == "skip" and r["scope"] == "in" for r in rows))
    print(sum(1 for r in rows if r["scope"] == "in" and not r["verdict"]), "undecided")


def main(argv):
    tsv, spec_path = argv[0], argv[1]
    txt = argv[argv.index("--txt") + 1] if "--txt" in argv else "/tmp/p36/txt"
    pdf = argv[argv.index("--pdf") + 1] if "--pdf" in argv else "/tmp/p36/pdf"
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    if "--report" not in argv:
        here = os.path.dirname(os.path.abspath(spec_path))
        extra_path = os.path.join(here, "extra-pdfs.tsv")
        extra = [(r["url"], r["maker"]) for r in csv.DictReader(open(extra_path), delimiter="\t")] \
            if os.path.exists(extra_path) else []
        decide(rows, load_specs(spec_path), Texts(txt, pdf), extra)
        n = overrides(rows, os.path.join(os.path.dirname(os.path.abspath(spec_path)), "package-overrides.tsv"))
        cols = list(rows[0].keys())
        with open(tsv, "w") as out:
            out.write("\t".join(cols) + "\n")
            for r in rows:
                out.write("\t".join(r[c] for c in cols) + "\n")
        print("package overrides applied", n)
    report(rows)


if __name__ == "__main__":
    main(sys.argv[1:])
