#!/usr/bin/env python3
"""Compare the ERC pin types of the part library with KiCad's stock symbols.

usage: python3 scripts/kicad-pin-types.py [--kicad DIR] [--golden FILE] [--pair OURS:KICAD]...

Reads backend/tests/golden/erc_pins.txt (one line per part, one letter per pin:
what `erc_pin_type` says, plan-63-B/C) and, for every part that
scripts/kicad-compare.py matches to a KiCad symbol of the same part number and
the same pin count, compares pin by pin number our type with the symbol's pin
type. Prints (1) how many parts and pins were compared, (2) a table of the
(our type, KiCad's type) pairs, and (3) for tri_state against output, the
kinds where we and KiCad disagree, with the first three part names of each.

--pair OURS:KICAD (repeatable, e.g. power_in:passive) also lists, for that pair, the pins by kind
and five example pins (part, pin number, name).

--kicad is KiCad's SharedSupport directory (default as kicad-compare.py).
Takes about a minute; needs only Python 3. Writes nothing.
"""

import argparse
import collections
import importlib.util
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, '..')

spec = importlib.util.spec_from_file_location('kicad_compare', os.path.join(HERE, 'kicad-compare.py'))
kc = importlib.util.module_from_spec(spec)
spec.loader.exec_module(kc)

# erc_pins_check's letters, in ErcPinType order.
OURS = {'i': 'input', 'o': 'output', 'b': 'bidirectional', 't': 'tri_state', 'p': 'passive', 'f': 'free',
        'u': 'unspecified', 'P': 'power_in', 'O': 'power_out', 'c': 'open_collector', 'e': 'open_emitter',
        'n': 'no_connect'}
# KiCad's own spellings that differ from ours.
KICAD = {'unconnected': 'no_connect'}


def read_golden(path):
    golden = {}
    with open(path) as fh:
        for line in fh.read().splitlines():
            name, kind, letters = line.split('|')
            golden[name] = (kind, letters)
    return golden


def main():
    ap = argparse.ArgumentParser(description="Compare the ERC pin types with KiCad's symbols.")
    ap.add_argument('--kicad', help="KiCad's SharedSupport directory")
    ap.add_argument('--pair', action='append', default=[], help='OURS:KICAD pair to list examples of')
    ap.add_argument('--golden', default=os.path.join(REPO, 'backend', 'tests', 'golden', 'erc_pins.txt'))
    args = ap.parse_args()
    shared = kc.find_kicad(args.kicad)
    golden = read_golden(args.golden)

    print('reading KiCad symbols…', file=sys.stderr)
    symbols = kc.load_symbols(shared)
    keys = kc.build_symbol_keys(symbols)
    _, parts = kc.load_ours()

    pairs = collections.Counter()
    examples = collections.defaultdict(list)
    disagree = collections.defaultdict(lambda: collections.defaultdict(list))
    matched = compared = skipped = pins_compared = 0
    for p in parts:
        syms, _mil = kc.find_symbols(keys, p['name'], 'military' in p['flags'])
        if not syms or p['name'] not in golden:
            continue
        matched += 1
        kind, letters = golden[p['name']]
        # The symbol whose pin count equals ours; none: package numbering differs, skip.
        live = [i for i, ch in enumerate(letters) if ch != '-']
        choice = next((s for s in syms if len(symbols[s]['pins']) == len(live)), None)
        if choice is None:
            skipped += 1
            continue
        compared += 1
        kpins = symbols[choice]['pins']
        differ = False
        for i in live:
            pin = p['pins'][i]
            designator = str(pin.get('number', pin.get('ball')))
            if designator not in kpins:
                continue
            ours = OURS[letters[i]]
            theirs = KICAD.get(kpins[designator][1], kpins[designator][1])
            pairs[(ours, theirs)] += 1
            pins_compared += 1
            examples[(ours, theirs)].append((kind, p['name'], designator, pin.get('name', '')))
            if {ours, theirs} == {'tri_state', 'output'}:
                differ = True
        if differ:
            disagree[kind]['n'].append(p['name'])

    print('parts matched to a KiCad symbol: %d; compared (same pin count): %d; skipped (another package): %d; '
          'pins compared: %d' % (matched, compared, skipped, pins_compared))
    print('\n%-16s %-16s %8s' % ('ours', 'KiCad', 'pins'))
    for (ours, theirs), n in sorted(pairs.items(), key=lambda kv: -kv[1]):
        print('%-16s %-16s %8d%s' % (ours, theirs, n, '' if ours == theirs else '   *'))
    for spec_pair in args.pair:
        ours, theirs = spec_pair.split(':')
        found = examples[(ours, theirs)]
        by_kind = collections.Counter(k for k, _, _, _ in found)
        print('\n%s (ours) vs %s (KiCad): %d pins; by kind: %s' % (ours, theirs, len(found),
              ', '.join('%s %d' % kv for kv in by_kind.most_common())))
        seen = set()
        for kind, name, number, pin_name in found:
            if name in seen:
                continue
            seen.add(name)
            print('    %s %s pin %s %s' % (kind, name, number, pin_name))
            if len(seen) >= 5:
                break
    print('\ntri_state vs output disagreements, by kind (parts, first three):')
    for kind, d in sorted(disagree.items()):
        names = d['n']
        print('  %-10s %5d  %s' % (kind, len(names), ', '.join(names[:3])))
    if not disagree:
        print('  none')


if __name__ == '__main__':
    main()
