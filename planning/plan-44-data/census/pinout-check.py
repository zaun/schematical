#!/usr/bin/env python3
"""plan-44-B section 3.3: the structure of every spec row's label, pinout,
terminals and capacitors.

usage: python3 -I pinout-check.py [--maker <maker>] [<specs.tsv>]

Per spec row (of the maker): `label` is one of SMD, HC-49/US, HC-49/U,
HC-46/X, SOJ, Cylinder; `terminals` is `X1,X2`; `capacitors` is 0 or 1;
`pinout` is `<designator>=<name>` items in pad order with unique designators,
each name X1, X2, NC or GND; exactly one X1 and one X2, at most one GND (the
common pad of a three-pad resonator, and only on a three-pad part), every
other pin NC; the pin count equals a pad count the DigiKey Package / Case
names (`specs.pad_counts`, when it names one); `capacitors` 1 only on a part
with a GND pad. Prints each error and `<n> rows, <e> bad`.
"""
import csv
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs  # noqa: E402


# The datasheet shows a pad count the DigiKey Package / Case text does not name
# (plan-44-B Corrections): (maker, series, DigiKey package) -> pad counts.
DATASHEET_PADS = {
    ('Abracon', 'ABSM3A', 'HC-49/US'): {4},   # an SMD HC-49/US with four pads (ABSM3A.pdf, land pattern #1-#4)
}


def check(s):
    errors = []
    if s['label'] not in specs.LABELS:
        errors.append(f"label {s['label']!r} is not one of {', '.join(specs.LABELS)}")
    if s['terminals'] != 'X1,X2':
        errors.append(f"terminals {s['terminals']!r} is not X1,X2")
    if s['capacitors'] not in ('0', '1'):
        errors.append(f"capacitors {s['capacitors']!r} is not 0 or 1")
    try:
        items = specs.pinout_items(s['pinout'])
    except ValueError as e:
        return errors + [f'pinout item {e} is not <designator>=<name>']
    designators = [d for d, _ in items]
    names = [n for _, n in items]
    if len(set(designators)) != len(designators) or '' in designators:
        errors.append('designators are not unique')
    for n in names:
        if n not in specs.NAMES:
            errors.append(f'pin name {n!r} is not one of {", ".join(specs.NAMES)}')
    for want in ('X1', 'X2'):
        if names.count(want) != 1:
            errors.append(f'{names.count(want)} pins named {want}')
    if names.count('GND') > 1:
        errors.append('more than one GND pin')
    if s['capacitors'] == '1' and 'GND' not in names:
        errors.append('built-in capacitors but no GND pad')
    counts = DATASHEET_PADS.get((s['maker'], s['series'], s['package'])) or specs.pad_counts(s['package'])
    if counts and len(names) not in counts:
        errors.append(f"{len(names)} pins, the package names {sorted(counts)}")
    return errors


def main(argv):
    maker = None
    if '--maker' in argv:
        i = argv.index('--maker')
        maker = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    path = argv[0] if argv else specs.SPECS
    rows = bad = 0
    for s in csv.DictReader(open(path), delimiter='\t'):
        if maker and s['maker'] != maker:
            continue
        rows += 1
        errs = check(s)
        bad += 1 if errs else 0
        for e in errs:
            print(f"{s['maker']} | {s['series']} | {s['pdf']} | {s['package']}: {e}")
    print(f'{rows} rows, {bad} bad')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
