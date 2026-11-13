#!/usr/bin/env python3
"""plan-37-D/E/F: register.json devices (and dff.json additions) from specs/ + candidates.tsv.

usage: gen-parts.py <specs dir> <candidates.tsv> [--device RE] [--check] [--write <register.json>]
       gen-parts.py --dff <dff.json> <specs dir> <candidates.tsv> [--check] [--write]

  default   print every spec'd device (id matching RE) with its parts, one
            per `add` row of candidates.tsv (kind register, that device),
            sorted by name, in the part files' layout
  --check   every add row's family is a timing key, its footprint's pin
            count has a pinout, every group pin is exactly one pin of every
            pinout, the footprint's package name is known; print
            "<n> rows ok" or the problems (exit 1)
  --write   replace (or append) those devices in register.json
  --dff     the six dff devices: specs/dff-additions.json gives per device
            {"timing": {family: entry}, "note": text}; the add rows (kind dff)
            become parts appended to the device (by name order with the
            existing ones), the timing entries are added; --check validates
            them against the device, --write rewrites dff.json

specs/<device>.json: the device object without "parts" (id, function,
description, drawer, note, model, pinouts, groups, timing), plus optional
"partVcc": {maker: {family: [min, max]}} (not written to the part file): that
maker's parts of the family get a part "vcc".
"""
import csv
import json
import re
import sys
from pathlib import Path

MAKER = {'Texas Instruments': 'Texas Instruments', 'onsemi': 'onsemi', 'Nexperia USA Inc.': 'Nexperia'}
PACKAGE = [('SOIC', 'SOIC'), ('SOP', 'SOP'), ('SSOP', 'SSOP'), ('QSOP', 'QSOP'), ('TSSOP', 'TSSOP'),
           ('TVSOP', 'TVSOP'), ('PDIP', 'DIP'), ('CDIP', 'DIP'), ('LCCC', 'LCCC'), ('CFP', 'CFP'),
           ('DHVQFN', 'DHVQFN'), ('DQFN', 'DQFN')]
CERAMIC = ('CDIP', 'LCCC', 'CFP')


def package(fp):
    for prefix, name in PACKAGE:
        if fp.split('-')[0] == prefix:
            return name
    raise KeyError(fp)


def pins_of(fp):
    m = re.search(r'-(\d+)', fp)
    return int(m.group(1))


def pinout_key(fp, dev):
    """p<n>; an LCCC carrier of a device with an lccc<n> pinout (a 16-pin
    device in a 20-terminal FK) uses that one."""
    if fp.startswith('LCCC-') and 'lccc%d' % pins_of(fp) in dev['pinouts']:
        return 'lccc%d' % pins_of(fp)
    return 'p%d' % pins_of(fp)


def flags(row):
    if row['grade'] == 'military':
        return ['military', 'ceramic'] if row['footprint'].startswith(CERAMIC) else ['military']
    if row['grade'] == 'automotive':
        return ['automotive']
    return []


def part(row, dev):
    p = {'name': row['mpn'], 'manufacturer': MAKER[row['maker']],
         'datasheet': Path(row['pdf']).stem, 'package': package(row['footprint']),
         'pinout': pinout_key(row['footprint'], dev), 'footprint': row['footprint'], 'family': row['family']}
    f = flags(row)
    if f:
        p['flags'] = f
    # spec "partVcc": {maker: {family: [min, max]}}: that maker's parts of the
    # family take their own datasheet's supply range (D § 3.2)
    v = dev.get('partVcc', {}).get(MAKER[row['maker']], {}).get(row['family'])
    if v:
        p['vcc'] = v
    return p


def group_pins(dev):
    out = []
    for g in dev.get('groups', []):
        out += g['d'] + [g['clk']['pin']]
        for key in ('ce',):
            c = g.get(key)
            out += [x['pin'] for x in (c if isinstance(c, list) else [c] if c else [])]
        if g.get('clear'):
            out.append(g['clear']['pin'])
        for b in g['outputs']:
            out += b['q']
            c = b.get('oe')
            out += [x['pin'] for x in (c if isinstance(c, list) else [c] if c else [])]
    return out


def problems(dev, rows):
    out = []
    for name, pins in dev['pinouts'].items():
        names = [p['name'] for p in pins]
        nums = [p['number'] for p in pins]
        if nums != list(range(1, len(pins) + 1)):
            out.append('%s: pinout %s numbers are not 1..%d' % (dev['id'], name, len(pins)))
        for n in group_pins(dev):
            if names.count(n) != 1:
                out.append('%s: pin %r is named %d times in pinout %s' % (dev['id'], n, names.count(n), name))
    for r in rows:
        p = part(r, dev)
        if r['family'] not in dev['timing']:
            out.append('%s: family %s has no timing entry' % (r['mpn'], r['family']))
        po = dev['pinouts'].get(p['pinout'])
        if po is None:
            out.append('%s: no pinout %s' % (r['mpn'], p['pinout']))
        elif len(po) != pins_of(r['footprint']):
            out.append('%s: pinout %s has %d pins, footprint %s %d' % (
                r['mpn'], p['pinout'], len(po), r['footprint'], pins_of(r['footprint'])))
    return out


def fmt_value(v):
    return json.dumps(v, ensure_ascii=False, separators=(',', ':'))


def fmt_device(dev, indent='    '):
    """The part files' layout: one key per line, one pin / part / timing entry per line."""
    i1, i2, i3 = indent + '  ', indent + '    ', indent + '      '
    lines = [indent + '{']
    keys = [k for k in dev if k != 'parts']
    body = []
    for k in keys:
        v = dev[k]
        if k == 'pinouts':
            po = []
            for name, pins in v.items():
                po.append(i2 + json.dumps(name) + ': [\n' +
                          ',\n'.join(i3 + fmt_value(p) for p in pins) + '\n' + i2 + ']')
            body.append(i1 + '"pinouts": {\n' + ',\n'.join(po) + '\n' + i1 + '}')
        elif k in ('groups', 'units'):
            body.append(i1 + json.dumps(k) + ': [\n' + ',\n'.join(i2 + fmt_value(g) for g in v) + '\n' + i1 + ']')
        elif k == 'timing':
            body.append(i1 + '"timing": {\n' + ',\n'.join(
                i2 + json.dumps(f) + ': ' + fmt_value(e) for f, e in v.items()) + '\n' + i1 + '}')
        else:
            body.append(i1 + json.dumps(k) + ': ' + json.dumps(v, ensure_ascii=False))
    body.append(i1 + '"parts": [\n' + ',\n'.join(i2 + fmt_value(p) for p in dev['parts']) + '\n' + i1 + ']')
    lines.append(',\n'.join(body))
    lines.append(indent + '}')
    return '\n'.join(lines)


def fmt_file(data):
    head = '{\n  "format": %s,\n  "version": %d,\n  "kind": %s,\n  "devices": [\n' % (
        json.dumps(data['format']), data['version'], json.dumps(data['kind']))
    return head + ',\n'.join(fmt_device(d) for d in data['devices']) + '\n  ]\n}\n'


def load_rows(path, kind):
    return [r for r in csv.DictReader(open(path), delimiter='\t')
            if r['scope'] == 'in' and r['kind'] == kind and r['verdict'] == 'add']


def main_register(args):
    specs, cand = Path(args[0]), args[1]
    rex = re.compile(args[args.index('--device') + 1]) if '--device' in args else re.compile('')
    rows = load_rows(cand, 'register')
    devs = []
    bad = []
    n = 0
    for f in sorted(specs.glob('*.json')):
        if f.name.startswith('dff-'):
            continue
        dev = json.load(open(f))
        if not rex.search(dev['id']):
            continue
        mine = sorted((r for r in rows if r['device'] == dev['id']), key=lambda r: r['mpn'])
        if not mine:
            bad.append('%s: no add row' % dev['id'])
        bad += problems(dev, mine)
        dev = dict(dev)
        dev['parts'] = [part(r, dev) for r in mine]
        dev.pop('partVcc', None)
        devs.append(dev)
        n += len(mine)
    have = {d['id'] for d in devs}
    for r in rows:
        if rex.search(r['device']) and r['device'] not in have:
            bad.append('%s: device %s has no spec' % (r['mpn'], r['device']))
    if '--check' in args:
        for b in bad:
            print(b)
        print('%d rows ok' % n if not bad else '%d problems' % len(bad))
        return 1 if bad else 0
    if '--write' in args:
        path = args[args.index('--write') + 1]
        data = json.load(open(path))
        by_id = {d['id']: i for i, d in enumerate(data['devices'])}
        for d in devs:
            if d['id'] in by_id:
                data['devices'][by_id[d['id']]] = d
            else:
                data['devices'].append(d)
        open(path, 'w').write(fmt_file(data))
        print('%d devices, %d parts written to %s' % (len(devs), n, path))
        return 0
    for d in devs:
        print(fmt_device(d) + ',')
    return 0


def main_dff(args):
    dff_path, specs, cand = args[0], Path(args[1]), args[2]
    add = json.load(open(specs / 'dff-additions.json'))
    data = json.load(open(dff_path))
    rows = load_rows(cand, 'dff')
    bad, n = [], 0
    by_id = {d['id']: d for d in data['devices']}
    for dev_id in sorted({r['device'] for r in rows}):
        dev = by_id[dev_id]
        extra = add.get(dev_id, {})
        timing = dict(dev['timing'])
        for fam, e in extra.get('timing', {}).items():
            if fam in timing:
                bad.append('%s: timing %s exists already' % (dev_id, fam))
            timing[fam] = e
        mine = sorted((r for r in rows if r['device'] == dev_id), key=lambda r: r['mpn'])
        names = {p['name'] for p in dev['parts']}
        for r in mine:
            p = part(r, dev)
            if r['mpn'] in names:
                bad.append('%s: already a part of %s' % (r['mpn'], dev_id))
            if r['family'] not in timing:
                bad.append('%s: family %s has no timing entry in %s' % (r['mpn'], r['family'], dev_id))
            po = dev['pinouts'].get(p['pinout'])
            if po is None or len(po) != pins_of(r['footprint']):
                bad.append('%s: %s has no %d-pin pinout %s' % (r['mpn'], dev_id, pins_of(r['footprint']),
                                                                p['pinout']))
        n += len(mine)
        if '--write' in args:
            plan.append((dev_id, extra.get('timing', {}), extra.get('note', ''), [part(r, dev) for r in mine]))
    if '--check' in args:
        for b in bad:
            print(b)
        print('%d rows ok' % n if not bad else '%d problems' % len(bad))
        return 1 if bad else 0
    if '--write' in args:
        if bad:
            for b in bad:
                print(b)
            return 1
        return write_dff(dff_path, plan)
    return 0


plan = []


def block_end(text, start, open_ch, close_ch):
    """Index of the bracket closing the one at text[start] (no brackets in strings assumed
    beyond balanced ones)."""
    depth, i, in_str = 0, start, False
    while i < len(text):
        c = text[i]
        if in_str:
            if c == '\\':
                i += 1
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
        elif c == open_ch:
            depth += 1
        elif c == close_ch:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError('unbalanced')


def write_dff(path, changes):
    """Inserts, per device, the new timing entries (one line each, at the end
    of "timing"), the note text (appended to "note", or a new "note" line
    after "drawer") and the new part lines (at the end of "parts"), keeping
    the file's own layout everywhere else."""
    text = open(path).read()
    for dev_id, timing, note, parts in changes:
        at = text.index('"id": %s' % json.dumps(dev_id))
        dev_end = block_end(text, text.rindex('{', 0, at), '{', '}')
        t0 = text.index('"timing": {', at, dev_end) + len('"timing": ')
        t1 = block_end(text, t0, '{', '}')
        if timing:
            inner = text[t0 + 1:t1].rstrip()
            indent = '        '
            new = ''.join(',\n%s%s: %s' % (indent, json.dumps(f), fmt_value(e)) for f, e in timing.items())
            if '\n' not in text[t0:t1]:
                raise ValueError('%s: one-line timing' % dev_id)
            text = text[:t0 + 1] + inner + new + '\n      ' + text[t1:]
        at = text.index('"id": %s' % json.dumps(dev_id))
        dev_end = block_end(text, text.rindex('{', 0, at), '{', '}')
        p0 = text.index('"parts": [', at, dev_end) + len('"parts": ')
        p1 = block_end(text, p0, '[', ']')
        inner = text[p0 + 1:p1].rstrip()
        new = ''.join(',\n        %s' % fmt_value(p) for p in parts)
        text = text[:p0 + 1] + inner + new + '\n      ' + text[p1:]
        if note:
            at = text.index('"id": %s' % json.dumps(dev_id))
            dev_end = block_end(text, text.rindex('{', 0, at), '{', '}')
            m = re.compile(r'"note": "((?:[^"\\]|\\.)*)"').search(text, at, dev_end)
            # the device's own note is the first "note" before "pinouts"
            pin_at = text.index('"pinouts"', at, dev_end)
            if m and m.start() < pin_at:
                old = json.loads('"%s"' % m.group(1))
                text = text[:m.start()] + '"note": ' + json.dumps(old + ' ' + note, ensure_ascii=False) + \
                    text[m.end():]
            else:
                d0 = text.index('"drawer": ', at, dev_end)
                d1 = text.index('\n', d0)
                text = text[:d1] + '\n      "note": ' + json.dumps(note, ensure_ascii=False) + ',' + text[d1:]
    json.loads(text)
    open(path, 'w').write(text)
    print('%d devices changed in %s' % (len(changes), path))
    return 0


if __name__ == '__main__':
    a = sys.argv[1:]
    if a and a[0] == '--dff':
        sys.exit(main_dff(a[1:]))
    sys.exit(main_register(a))
