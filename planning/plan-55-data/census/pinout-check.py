#!/usr/bin/env python3
"""plan-55-C § 3.3: the pinout, switches and facts columns of specs.tsv, checked by rule.

usage: python3 -I pinout-check.py [--family F]... [--company C]... [--specs specs.tsv] [--all] [--complete] [--list]

Checks every filled, non-skip spec row of specs.tsv (or --specs <file>: a packet's output) that a candidate row of
candidates.tsv uses, restricted to the given families and companies (the candidate rows' `family` and `company`;
with --all every filled row of the file). --complete also reports a used row that is neither filled nor a skip
row. A skip row's `skip` must be one of common.SKIP_REASONS. Per row:

  pinout      JSON `[[number, name, role], ...]`: numbers 1..N once each (a ball pinout: ball names `A1` once each),
              roles input, output, inout, power, ground, nc, absent; N equals the lead count of DigiKey's Package /
              Case (common.pin_count; an exposed pad is not a pin; pinout-count-overrides.tsv corrects a label)
  type        load, limited or ideal-diode
  switches    JSON list of 1 to 4 objects with `in`, `out` and optionally `enable`, `enableActive` (high / low),
              `enableRef` (logic / input; input needs low), `flag`, `flagWhen` (fault / disabled); every name is a
              pin name of the pinout; `in` pins have role power, `out` output, `enable` input, `flag` output
  analog      JSON list of pin names of the pinout
  ordering    a regex (may be empty) that compiles
  facts       vin_min, vin_max, iout, ilimit, ron, iq: numbers in the kind's ranges (vin_max >= vin_min); `ilimit`
              only for a `limited` row
Every pin of the pinout is `ground`, `nc` or `absent`, or is named by a switch (in, out, enable, flag) or in
`analog`. Prints one line per error, then `<n> pinout errors`; --list also prints each row checked.
Exit status 1 when there is an error.
"""
import collections
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

ROLES = {'power', 'ground', 'input', 'output', 'inout', 'nc', 'absent'}
TYPES = {'load', 'limited', 'ideal-diode'}
KEYS = {'in', 'out', 'enable', 'enableActive', 'enableRef', 'flag', 'flagWhen'}
FACTS = {'vin_min': 100, 'vin_max': 100, 'iout': 100, 'ilimit': 100, 'ron': 10, 'iq': 1}
FLAGS = {'automotive', 'military', 'enhanced'}


def parse(text, what, errs, tag):
    if not text:
        errs.append(f'{tag}: {what} is empty')
        return None
    try:
        return json.loads(text)
    except ValueError as e:
        errs.append(f'{tag}: {what} is not JSON ({e})')
        return None


def check_row(r, errs, overrides):
    tag = f"{r['company']} {os.path.basename(r['pdf'])} [{r['package']} | {r['supplier_package']}] ord={r['ordering'][:30]!r}"
    pins = parse(r['pinout'], 'pinout', errs, tag)
    if pins is None:
        return
    if not isinstance(pins, list) or not pins:
        errs.append(f'{tag}: pinout is not a non-empty list')
        return
    names = collections.defaultdict(list)
    numbers = []
    for p in pins:
        if not (isinstance(p, list) and len(p) == 3 and isinstance(p[1], str) and p[1]):
            errs.append(f'{tag}: pin {p!r} is not [number, name, role]')
            return
        num, name, role = p
        if role not in ROLES:
            errs.append(f'{tag}: pin {num} {name}: role {role!r}')
        numbers.append(num)
        names[name].append(p)
    if all(isinstance(n, int) and not isinstance(n, bool) for n in numbers):
        if sorted(numbers) != list(range(1, len(numbers) + 1)):
            errs.append(f'{tag}: pin numbers are not 1..{len(numbers)} once each: {sorted(numbers)}')
    elif not all(isinstance(n, str) for n in numbers) or len(set(numbers)) != len(numbers):
        errs.append(f'{tag}: mixed or repeated pin numbers: {numbers}')
    expect = overrides.get((r['pdf'], r['package'], r['supplier_package']))
    if expect is None:
        expect = common.pin_count(r['package'], r['supplier_package'])
    if expect is not None and len(pins) != expect:
        errs.append(f"{tag}: {len(pins)} pins, the package label says {expect}")
    if r['type'] not in TYPES:
        errs.append(f"{tag}: type {r['type']!r}")
    sw = parse(r['switches'], 'switches', errs, tag)
    analog = parse(r['analog'] or '[]', 'analog', errs, tag)
    used = set()
    if isinstance(analog, list):
        for a in analog:
            if a not in names:
                errs.append(f'{tag}: analog name {a!r} is not a pin')
            used.add(a)
    else:
        errs.append(f'{tag}: analog must be a list')
    if isinstance(sw, list) and 1 <= len(sw) <= 4:
        for k, e in enumerate(sw, 1):
            if not isinstance(e, dict) or set(e) - KEYS:
                errs.append(f'{tag}: switch {k}: keys {sorted(set(e) - KEYS) if isinstance(e, dict) else e!r}')
                continue
            for key, roles in (('in', {'power'}), ('out', {'output'}), ('enable', {'input'}), ('flag', {'output'})):
                name = e.get(key)
                if name is None:
                    if key in ('in', 'out'):
                        errs.append(f'{tag}: switch {k} has no {key!r}')
                    continue
                if name not in names:
                    errs.append(f'{tag}: switch {k} {key} {name!r} is not a pin name')
                    continue
                used.add(name)
                for p in names[name]:
                    if p[2] not in roles:
                        errs.append(f'{tag}: switch {k} {key} pin {p[0]} {name}: role {p[2]!r}, want {sorted(roles)}')
            if 'enableActive' in e:
                if 'enable' not in e:
                    errs.append(f'{tag}: switch {k}: enableActive without enable')
                if e['enableActive'] not in ('high', 'low'):
                    errs.append(f"{tag}: switch {k}: enableActive {e['enableActive']!r}")
            if 'enableRef' in e:
                if 'enable' not in e:
                    errs.append(f'{tag}: switch {k}: enableRef without enable')
                if e['enableRef'] not in ('logic', 'input'):
                    errs.append(f"{tag}: switch {k}: enableRef {e['enableRef']!r}")
                if e['enableRef'] == 'input' and e.get('enableActive') != 'low':
                    errs.append(f'{tag}: switch {k}: enableRef input needs enableActive low')
            if 'flagWhen' in e:
                if 'flag' not in e:
                    errs.append(f'{tag}: switch {k}: flagWhen without flag')
                if e['flagWhen'] not in ('fault', 'disabled'):
                    errs.append(f"{tag}: switch {k}: flagWhen {e['flagWhen']!r}")
            if e.get('in') == e.get('out'):
                errs.append(f'{tag}: switch {k}: in and out have one name')
    elif sw is not None:
        errs.append(f'{tag}: switches must be a list of 1 to 4 objects')
    for name, ps in names.items():
        for p in ps:
            if p[2] not in ('ground', 'nc', 'absent') and name not in used:
                errs.append(f'{tag}: pin {p[0]} {name} ({p[2]}) is in no switch and not analog')
    try:
        re.compile(r['ordering'])
    except re.error as e:
        errs.append(f'{tag}: ordering is not a regex ({e})')
    facts = {}
    for key, top in FACTS.items():
        if r[key] == '':
            continue
        try:
            v = float(r[key])
        except ValueError:
            errs.append(f'{tag}: {key} {r[key]!r} is not a number')
            continue
        facts[key] = v
        if not 0 < v <= top:
            errs.append(f'{tag}: {key} {v} is not above 0 and at most {top}')
    if 'vin_min' in facts and 'vin_max' in facts and facts['vin_max'] < facts['vin_min']:
        errs.append(f'{tag}: vin_max below vin_min')
    if r['ilimit'] != '' and r['type'] != 'limited':
        errs.append(f'{tag}: ilimit on a {r["type"]} row')
    for f in filter(None, (x.strip() for x in r['flags'].split(','))):
        if f not in FLAGS:
            errs.append(f'{tag}: flag {f!r}')


def main():
    args = sys.argv[1:]
    fams = {args[i + 1] for i, a in enumerate(args) if a == '--family'}
    cos = {args[i + 1] for i, a in enumerate(args) if a == '--company'}
    path = args[args.index('--specs') + 1] if '--specs' in args else common.SPECS
    rows = common.read_tsv(path)
    used = collections.Counter()
    if '--all' not in args:
        for c in common.read_tsv(common.CANDIDATES):
            if common.eligible(c) and (not fams or c['family'] in fams) and (not cos or c['company'] in cos):
                used[common.spec_key(c)] += 1
    errs = []
    overrides = common.count_overrides()
    n = 0
    filtered = bool(fams or cos)
    for r in rows:
        if r['skip'] != '':
            if r['skip'] not in common.SKIP_REASONS:
                errs.append(f"{r['company']} {os.path.basename(r['pdf'])}: skip reason {r['skip']!r} is not one of {common.SKIP_REASONS}")
            continue
        is_used = used[common.spec_key(r)] > 0
        if '--all' not in args and filtered and not is_used:
            continue
        filled = any(r[c] for c in ('pinout', 'type', 'switches'))
        if not filled:
            if '--complete' in args and (is_used or '--all' in args):
                errs.append(f"{r['company']} {os.path.basename(r['pdf'])} [{r['package']} | {r['supplier_package']}]: not read (no pinout and no skip)")
            continue
        n += 1
        if '--list' in args:
            print('checking', r['company'], r['pdf'], r['package'], '|', r['supplier_package'])
        check_row(r, errs, overrides)
    for e in errs:
        print(e)
    print(f'{n} rows checked; {len(errs)} pinout errors')
    sys.exit(1 if errs else 0)


main()
