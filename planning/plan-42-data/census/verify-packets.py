#!/usr/bin/env python3
"""plan-42-C Phase 4: the hand-check packets.

usage: python3 -I verify-packets.py <out dir> [<items per packet>]

One item per distinct (maker, pinout, regulators) combination of the spec rows
the `add` rows use (and per distinct (maker, pinout_adj, regulators_adj)): the
first spec row, in PDF / package order, that has it. Writes <out dir>/<maker
slug>-<nn>.txt with the stored columns of each item and the DigiKey facts of its
`add` rows, and prints one line per packet.
"""
import collections
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402


def compact(pinout):
    return " ".join(f"{n}:{name}/{role}" for n, name, role in json.loads(pinout))


def main(out, size):
    rows = common.load(specs.CANDIDATES)
    table = specs.load()
    used = collections.defaultdict(list)
    for r in rows:
        if r["reason"] == "add":
            used[specs.key(r)].append(r)
    seen = {}
    items = []
    for k in sorted(used, key=lambda k: (k[0], k[2], k[3], k[1])):
        s = table[k]
        has_fixed = any(r["output_type"] != "Adjustable" for r in used[k])
        has_adj = any(r["output_type"] == "Adjustable" for r in used[k])
        mixed = has_fixed and has_adj
        variants = []
        if has_fixed:
            variants.append(("fixed", s["pinout"], s["regulators"], s["analog"]))
        if has_adj:
            variants.append(("adj", s["pinout_adj"] if mixed else s["pinout"],
                             s["regulators_adj"] if mixed else s["regulators"],
                             s["analog_adj"] if mixed else s["analog"]))
        for name, pin, reg, ana in variants:
            sig = (k[0], pin, reg)
            if sig in seen:
                continue
            seen[sig] = k
            rs = [r for r in used[k] if (r["output_type"] == "Adjustable") == (name == "adj")]
            items.append((k, name, pin, reg, ana, s, rs))
    os.makedirs(out, exist_ok=True)
    by_maker = collections.defaultdict(list)
    for it in items:
        by_maker[it[0][0]].append(it)
    for maker, its in sorted(by_maker.items()):
        slug = common.SLUG[maker]
        for n in range(0, len(its), size):
            name = f"{slug}-{n // size + 1:02d}"
            with open(os.path.join(out, name + ".txt"), "w") as f:
                for k, variant, pin, reg, ana, s, rs in its[n:n + size]:
                    f.write(f"### ITEM pdf={k[2]} | package={k[3]} | series={k[1]} | variant={variant}\n")
                    f.write(f"  mpns: {', '.join(r['mpn'] for r in rs[:4])}  ({rs[0]['family']}, outputs={rs[0]['outputs']}, "
                            f"{rs[0]['topology'] or 'linear'})\n")
                    f.write(f"  stored pinout: {compact(pin)}\n")
                    f.write(f"  stored regulators: {reg}\n")
                    f.write(f"  stored analog: {ana}\n")
                    extra = []
                    if variant == "adj":
                        extra.append(f"vref={s['vref']} adjust_to={s['adjust_to']}")
                    extra.append(f"vin_min={s['vin_min']} dropout={s['dropout']}")
                    f.write("  stored values: " + " ".join(extra) + "\n\n")
            print(name, len(its[n:n + size]), "items")
    print(len(items), "items in total")


if __name__ == "__main__":
    main(sys.argv[1], int(sys.argv[2]) if len(sys.argv) > 2 else 28)
