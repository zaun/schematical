"""plan-29-B–G: fill specs.tsv's drawing columns from the owners' package lines.

usage: python3 -I assign.py [--dry-run]

For every spec row: the package line (packages-[B-G].tsv) of its maker,
supplier package and mounting (the mounting of the group's add rows) gives
`package` and `footprint` (the first id when the line names several,
`A|B`); `pins` is the line's map for the group's configuration: single and
zener rows `single`, arrays and zener arrays by DigiKey's configuration
(1 Pair Common Cathode → cc, … Common Anode → ca, … Series Connection →
series, N Independent → ind or the configuration text), bridges `bridge`.
Then every line of work/assign-overrides-*.tsv (datasheet, supplier_package,
footprint, pins, note; supplier_package `*` = every row of that PDF)
replaces the row's footprint and pins.  A group whose add rows have several
configurations gets one map per configuration: pins `cc=<map>;series=<map>`
and pinout `<name>;<name>` in the same order (gen-parts.py picks by the
row's configuration).  A `single` row with no single map on its line takes
the `cc` map (dual-die parts DigiKey files as single).  `pinout` is named from the map:
`two-pin` for 1:K,2:A, else `p<N>-<name>-<name>…` in pin order (lower case;
`+` → `pos`, `-` → `neg`).  Prints the rows it cannot fill.
"""
import collections
import csv
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
SHORT = {"Vishay General Semiconductor - Diodes Division": "Vishay", "onsemi": "onsemi",
         "Taiwan Semiconductor Corporation": "TSC"}
CONF = {"": "single", "1 Pair Common Cathode": "cc", "1 Pair Common Anode": "ca",
        "1 Pair Series Connection": "series", "2 Independent": "ind"}


def maps(text):
    out = {}
    for part in text.split(";"):
        if "=" in part:
            k, v = part.split("=", 1)
            out[k.strip()] = v.strip()
    return out


def pinout_name(pins):
    if pins == "1:K,2:A":
        return "two-pin"
    names = [p.split(":")[1].lower().replace("+", "pos").replace("-", "neg") for p in pins.split(",")]
    return f"p{len(names)}-" + "-".join(names)


def main(dry):
    paths = {r["url"]: r["path"] for r in csv.DictReader(open(os.path.join(ROOT, "datasheets", "diode", "datasheets.tsv")), delimiter="\t")}
    lines = {}
    for p in sorted(glob.glob(os.path.join(DATA, "packages-[B-G].tsv"))):
        for l in csv.DictReader(open(p), delimiter="\t"):
            lines.setdefault((l["maker"], l["supplier_package"], l["mounting"]), l)
            if l["mounting"] == "":
                pass
    groups = collections.defaultdict(list)
    for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv")), delimiter="\t"):
        if r["decision"] == "add":
            groups[(r["maker"], r["category"], paths[r["datasheet_url"]], r["supplier_package"])].append(r)
    over = {}
    for p in sorted(glob.glob(os.path.join(DATA, "work", "assign-overrides-*.tsv"))):
        for o in csv.DictReader(open(p), delimiter="\t"):
            over[(o["datasheet"], o["supplier_package"])] = o
    spec = os.path.join(DATA, "specs.tsv")
    specs = list(csv.DictReader(open(spec), delimiter="\t"))
    cols = list(specs[0].keys())
    bad = []
    for s in specs:
        g = groups.get((s["maker"], s["category"], s["datasheet"], s["supplier_package"]), [])
        if not g:
            bad.append(f"{s['datasheet']} {s['supplier_package']}: no add rows")
            continue
        m = SHORT[s["maker"]]
        mount = collections.Counter(r["mounting"] for r in g).most_common(1)[0][0]
        line = lines.get((m, s["supplier_package"], mount)) or lines.get((m, s["supplier_package"], ""))
        if line is None:
            line = next((l for (lm, lsp, _), l in lines.items() if lm == m and lsp == s["supplier_package"]), None)
        if line is None:
            bad.append(f"{s['datasheet']} {s['supplier_package']} ({mount}): no package line")
            continue
        confs = sorted({r["configuration"] for r in g})
        mp = maps(line["pins"])
        if s["category"] in ("single", "zener"):
            keys = ["single"]
        elif s["category"] == "bridge":
            keys = ["bridge"]
        else:
            keys = [CONF.get(c, c) for c in confs]

        def pick(k):
            got = mp.get(k)
            if got is None and k == "single" and s["category"] == "single":
                got = mp.get("cc")  # dual-die parts DigiKey files as single (TO-220AB, ITO-220AB)
            return got
        found = {k: pick(k) for k in keys}
        if len(keys) == 1:
            pins = found[keys[0]]
            key = keys[0]
        else:
            # Several configurations on one PDF and package: one map per configuration.
            key = ",".join(keys)
            pins = ";".join(f"{k}={v}" for k, v in found.items()) if all(found.values()) else None
        footprint = line["footprint"].split("|")[0]
        o = over.get((s["datasheet"], s["supplier_package"])) or over.get((s["datasheet"], "*"))
        if o:
            footprint = o["footprint"] or footprint
            pins = o["pins"] or pins
        if not pins or footprint in ("", "NONE"):
            bad.append(f"{s['datasheet']} {s['supplier_package']}: no {'pins for ' + key if not pins else 'footprint'} (line maps: {sorted(mp)})")
            continue
        s["package"] = line["package"]
        s["footprint"] = footprint
        s["pins"] = pins
        s["pinout"] = ";".join(pinout_name(x.split("=", 1)[-1]) for x in pins.split(";"))
    for b in bad:
        print(b)
    print(f"{len(specs) - len(bad)} rows filled, {len(bad)} not")
    if not dry:
        with open(spec, "w") as out:
            out.write("\t".join(cols) + "\n")
            for s in specs:
                out.write("\t".join(s[c] for c in cols) + "\n")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main("--dry-run" in sys.argv))
