#!/usr/bin/env python3
"""plan-40-B section 3.1: the structure of every spec row's pinout and elements.

usage: python3 -I pinout-check.py [--maker <maker>] [<specs.tsv>]

Per spec row (of the maker): `label`, `pinout` and `elements` are filled
(`-` in `pinout` and `elements`: the PDF gives no pinout for the package; B
skips its rows; a row whose scheme is `n/r` is not read and not checked);
every pinout name is B, C, E, G, D or S with an optional element digit, or
NC; each element is `npn`/`pnp` with B, C, E names or
`nmos`/`pmos` with G, D, S names, each name in the pinout (the control on
exactly one pin); every pinout name is used by an element; the pin count
equals the DigiKey Package / Case's (when it names one), or is one more
when the last pin is a numbered tab repeating a channel name. Prints each error
and `<n> pinout errors`.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402

LETTERS = {"npn": "BCE", "pnp": "BCE", "nmos": "GDS", "pmos": "GDS"}


def check(s):
    errors = []
    if s["scheme"] == "n/r":
        return errors
    if not s["label"]:
        errors.append("no label")
    if s["pinout"] == "-" and s["elements"] == "-":
        return errors
    if not s["pinout"] or not s["elements"]:
        return errors + ["pinout or elements empty"]
    names = specs.pinout_names(s["pinout"])
    for n in names:
        if not specs.NAMES.match(n):
            errors.append(f"pin name {n!r} is not B/C/E/G/D/S[digit]")
    used = set()
    try:
        els = specs.elements(s["elements"])
    except ValueError:
        return errors + [f"elements {s['elements']!r} unreadable"]
    for e in els:
        if len(e) != 4 or e[0] not in LETTERS:
            errors.append(f"element {e!r}: type:control,a,b with type npn/pnp/nmos/pmos")
            continue
        t, control, a, b = e
        for name, letter in zip((control, a, b), LETTERS[t]):
            if specs.NAMES.match(name) is None or name[0] != letter:
                errors.append(f"element {t}: {name!r} where {letter} belongs")
            if name not in names:
                errors.append(f"element {t}: {name!r} not in the pinout")
            used.add(name)
        if names.count(control) != 1:
            errors.append(f"control {control!r} on {names.count(control)} pins")
    for n in sorted(set(names) - used - {"NC"}):
        errors.append(f"pin {n!r} in no element")
    count = specs.pin_count(s["package"])
    # A numbered tab: the datasheet gives the tab its own number, one more
    # than DigiKey's count, and it repeats a channel name (plan-40-B Corrections 2).
    tab = len(names) == (count or 0) + 1 and names[-1] in names[:-1] and names[-1][0] in "CDES"
    if count is not None and count != len(names) and not tab:
        errors.append(f"{len(names)} pins, the package names {count}")
    return errors


def main(argv):
    maker = None
    if "--maker" in argv:
        i = argv.index("--maker")
        maker = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    path = argv[0] if argv else specs.SPECS
    bad = 0
    for s in csv.DictReader(open(path), delimiter="\t"):
        if maker and s["maker"] != maker:
            continue
        for e in check(s):
            bad += 1
            print(f"{s['maker']} | {s['pdf']} | {s['package']}: {e}")
    print(f"{bad} pinout errors")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
