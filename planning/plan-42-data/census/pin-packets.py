#!/usr/bin/env python3
"""plan-42-C: split the spec rows the `add` rows use into reading packets.

usage: python3 -I pin-packets.py <out dir> [<groups per packet>] [--missing]

--missing: only the spec rows whose columns an `add` row needs and are still
empty (the groups plan-42-C's first pass did not cover: refills after a
verdict override).

Writes <out dir>/<maker slug>-<nn>.txt (the groups of one packet with their
`add` rows' DigiKey facts), <maker slug>-<nn>.only (pdf TAB package TAB series
per group, for pinout-check.py --only) and prints one line per packet. Groups
of one PDF stay in one packet.
"""
import collections
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

FACTS = ["mpn", "family", "output_type", "outputs", "supplier_package", "vout_v", "vout_max_v", "vin_min_v",
         "vin_max_v", "iout_a", "dropout_v", "topology", "function", "fsw_hz"]


def missing(s, rs):
    has_fixed = any(specs.variant_of(r) == "fixed" for r in rs)
    has_adj = any(specs.variant_of(r) == "adj" for r in rs)
    mixed = has_fixed and has_adj
    if has_fixed and not s["pinout"]:
        return True
    if has_adj and not (s["pinout_adj"] if mixed else s["pinout"]):
        return True
    return False


def main(out, size, only_missing=False):
    rows = common.load(specs.CANDIDATES)
    table = specs.load()
    groups = collections.defaultdict(list)
    for r in rows:
        if r["reason"] == "add":
            groups[specs.key(r)].append(r)
    if only_missing:
        groups = {k: v for k, v in groups.items() if missing(table[k], v)}
    os.makedirs(out, exist_ok=True)
    by_maker = collections.defaultdict(lambda: collections.defaultdict(list))
    for k in sorted(groups, key=lambda k: (k[0], k[2], k[3], k[1])):
        by_maker[k[0]][k[2]].append(k)
    for maker, pdfs in sorted(by_maker.items()):
        slug = common.SLUG[maker]
        packets = [[]]
        for pdf, keys in sorted(pdfs.items()):
            if packets[-1] and len(packets[-1]) + len(keys) > size:
                packets.append([])
            packets[-1].extend(keys)
        for n, keys in enumerate(packets, 1):
            name = f"{slug}-{n:02d}"
            with open(os.path.join(out, name + ".txt"), "w") as f, open(os.path.join(out, name + ".only"), "w") as only:
                for k in keys:
                    s = table[k]
                    f.write(f"### GROUP pdf={k[2]} | package={k[3]} | series={k[1]} | label={s['label']}\n")
                    for r in sorted(groups[k], key=lambda r: -int(r["stock"])):
                        f.write("  " + " | ".join(f"{c}={r[c]}" for c in FACTS if r[c] != "") + "\n")
                    f.write("\n")
                    only.write(f"{k[2]}\t{k[3]}\t{k[1]}\n")
            print(name, len(keys), "groups,", sum(len(groups[k]) for k in keys), "add rows,",
                  len({k[2] for k in keys}), "pdfs")


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if a != "--missing"]
    main(args[0], int(args[1]) if len(args) > 1 else 22, "--missing" in sys.argv)
