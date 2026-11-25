#!/usr/bin/env python3
"""Reads a KiCad netlist (kicad-cli sch export netlist, s-expression form) and
prints, for one reference, how many components carry it, its units with their
pins, and every net with the nodes on it.

usage: python3 check_netlist.py FILE.net [REF]     (REF defaults to U1)

plan-67-F: the split sample must show ONE component U1 with five units and
its 14 pins on the nets the app's own netlist has.
"""
import re
import sys


def parse(text):
    tokens = re.findall(r'\(|\)|"(?:[^"\\]|\\.)*"|[^\s()]+', text)
    pos = 0

    def read():
        nonlocal pos
        tok = tokens[pos]
        pos += 1
        if tok == "(":
            items = []
            while tokens[pos] != ")":
                items.append(read())
            pos += 1
            return items
        return tok.strip('"')

    return read()


def find(node, name):
    return [c for c in node if isinstance(c, list) and c and c[0] == name]


def value(node, name):
    hit = find(node, name)
    return hit[0][1] if hit and len(hit[0]) > 1 else ""


def main():
    path = sys.argv[1]
    ref = sys.argv[2] if len(sys.argv) > 2 else "U1"
    tree = parse(open(path).read())
    comps = find(find(tree, "components")[0], "comp")
    mine = [c for c in comps if value(c, "ref") == ref]
    print(f"components: {len(comps)}; with ref {ref}: {len(mine)}")
    for comp in mine:
        print(f"  value {value(comp, 'value')}  footprint {value(comp, 'footprint')}")
        for unit in find(find(comp, "units")[0], "unit") if find(comp, "units") else []:
            pins = [value(p, "num") for p in find(find(unit, "pins")[0], "pin")]
            print(f"  unit {value(unit, 'name')}: pins {' '.join(sorted(pins, key=int))}")
    seen = set()
    for net in find(find(tree, "nets")[0], "net"):
        nodes = [(value(n, "ref"), value(n, "pin")) for n in find(net, "node")]
        for node in nodes:
            if node[0] == ref:
                seen.add(node[1])
        print(f"net {value(net, 'name')}: " + " ".join(f"{r}.{p}" for r, p in sorted(nodes)))
    print(f"{ref} pins on nets: {len(seen)} ({' '.join(sorted(seen, key=int))})")


main()
