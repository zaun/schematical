#!/usr/bin/env python3
"""plan-47-B section 3.3: the structure of every specs row's pinout, label and terminals.

usage: python3 -I pinout-check.py [--maker <maker>] <type>...
       (type: inductor, fuse, ptc)

Per specs row of the types (one per (type, maker, series, pdf, package)):
`label` is a chip code (four digits), `SMD`, `Radial`, `Axial` or `Cartridge`;
`terminals` is `1,2`; `pinout` is `<designator>=<name>` joined by commas (or `-`,
the PDF gives no pinout, or `out`, more than two terminals: not checked further),
the designators are distinct, the only names are `1`, `2` and `NC` (the two
terminal names, and the pins that are no terminal), both terminal names occur,
and `pads` (the pad or lead count of the datasheet's land pattern or lead drawing)
equals the pin count. Prints each error and `<n> specs rows: <e> errors`.
"""
import os
import re
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs47 as sp  # noqa: E402


def check(s):
    errors = []
    label = s['label']
    if not (re.fullmatch(r'\d{4}', label) or label in sp.LABELS):
        errors.append(f"label {label!r} is not a chip code, SMD, Radial, Axial or Cartridge")
    if s['terminals'] != '1,2':
        errors.append(f"terminals {s['terminals']!r}, not '1,2'")
    pinout = s['pinout']
    if pinout in ('-', 'out'):
        return errors
    if not pinout:
        return errors + ['pinout empty']
    designators = []
    names = []
    for item in pinout.split(','):
        if '=' not in item:
            errors.append(f"pin {item!r} is not <designator>=<name>")
            continue
        d, n = item.split('=', 1)
        designators.append(d)
        names.append(n)
        if n not in ('1', '2', 'NC'):
            errors.append(f"pin name {n!r} is not 1, 2 or NC")
    if len(set(designators)) != len(designators):
        errors.append('a designator is used twice')
    for n in ('1', '2'):
        if n not in names:
            errors.append(f"terminal {n} is on no pin")
    if s['pads'] in ('', '-'):
        errors.append('pads empty')
    elif int(s['pads']) != len(names):
        errors.append(f"{len(names)} pins, pads {s['pads']}")
    return errors


def main(argv):
    maker = None
    if '--maker' in argv:
        i = argv.index('--maker')
        maker = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    types = [a for a in argv if a in sp.TYPES]
    if not types or len(types) != len(argv):
        print('usage: pinout-check.py [--maker <maker>] inductor|fuse|ptc ...')
        return 2
    rows = [s for s in sp.read(sp.SPECS) if s['type'] in types and (not maker or s['maker'] == maker)]
    bad = 0
    for s in rows:
        for e in check(s):
            bad += 1
            print(f"{s['type']} {s['maker']} | {s['pdf']} | {s['package']}: {e}")
    print(f"{len(rows)} specs rows: {bad} errors")
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
