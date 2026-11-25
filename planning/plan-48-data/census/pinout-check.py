#!/usr/bin/env python3
"""plan-48-B section 3.2: the structure of every spec row's scheme, label,
pinout and facts.
usage: python3 -I pinout-check.py [--type timer|opamp] [--company ti|adi|st|renesas]
                                  [--unfilled] [<specs.tsv>]

Per spec row: `label`, `scheme`, `facts` are filled; `pinout` is filled when
the scheme is `explicit` and is `-` when the PDF shows no pinout for the
package (the row is then skipped by final.py). A named scheme's pin count
equals the package's; an explicit pinout is `<designator>=<name>` pairs in
pin order, designators 1…n without a gap or ball names (JEDEC rows), n equal
to the package's pin count or one more (a numbered exposed pad `EP`). A timer
row names TRIG, THRES, DISCH, OUT (with a digit when `circuits` is 2) once
per circuit, one VCC and one GND; an op-amp row names IN<k>+, IN<k>-, OUT<k>
once per circuit (no digit for a single) and at least one V+ and one V-.
`vcc` is min-max with 0 < min < max <= 100; a timer's `process`, `maxHz`
(1..100000000) and `tpd` (1..100000) are present. `--unfilled` lists the rows
with no scheme yet instead of reporting them as errors. Prints each error and
`<n> rows, <m> errors`."""
import csv
import os
import re
import sys

LABELS = {'DIP', 'SOIC', 'SOP', 'SSOP', 'TSSOP', 'TVSOP', 'VSSOP', 'SOT-23', 'SC-70', 'TO-263', 'SON',
          'QFN', 'LGA', 'DSBGA'}
SCHEMES = {
    '555-8': ['GND', 'TRIG', 'OUT', 'RESET', 'CONT', 'THRES', 'DISCH', 'VCC'],
    '556-14': ['DISCH1', 'THRES1', 'CONT1', 'RESET1', 'OUT1', 'TRIG1', 'GND', 'TRIG2', 'OUT2', 'RESET2',
               'CONT2', 'THRES2', 'DISCH2', 'VCC'],
    'single-8': ['NC', 'IN-', 'IN+', 'V-', 'NC', 'OUT', 'V+', 'NC'],
    'dual-8': ['OUT1', 'IN1-', 'IN1+', 'V-', 'IN2+', 'IN2-', 'OUT2', 'V+'],
    'quad-14': ['OUT1', 'IN1-', 'IN1+', 'V+', 'IN2+', 'IN2-', 'OUT2', 'OUT3', 'IN3-', 'IN3+', 'V-', 'IN4+',
                'IN4-', 'OUT4'],
    'single-5a': ['OUT', 'V-', 'IN+', 'IN-', 'V+'],
    'single-5b': ['IN+', 'V-', 'IN-', 'OUT', 'V+'],
}
# the circuits each named scheme is for, by type
SCHEME_FOR = {'555-8': ('timer', '1'), '556-14': ('timer', '2'), 'single-8': ('opamp', '1'),
              'dual-8': ('opamp', '2'), 'quad-14': ('opamp', '4'), 'single-5a': ('opamp', '1'),
              'single-5b': ('opamp', '1')}
EXTRA = re.compile(r'^(NC|ISET|BAL[12]|FLAG|OFFSET[12]|SHDN\d?|EN\d?|REF|COMP\d?|EP|VCC2|GND2|CONT\d?|RESET\d?)$')
BALL = re.compile(r'^[ABCDEFGHJKLMNPRTUVWY][1-9][0-9]?$')


def pin_count(package, supplier):
    """Lead count named by the supplier package or the package / case."""
    for s in (supplier, package):
        m = re.search(r'DDPAK-(\d+)', s)
        if m:
            return int(m.group(1))
        m = re.match(r'^(\d+)-', s) or re.search(r'(?:-|DDPAK-)(\d+)\)?(?:\s|$|,)', s)
        if m:
            return int(m.group(1))
    return None


def parse_pinout(text):
    pairs = []
    for item in text.split(','):
        d, _, n = item.partition('=')
        pairs.append((d.strip(), n.strip()))
    return pairs


def names_ok(typ, circuits, names):
    errors = []
    c = int(circuits)
    count = lambda n: names.count(n)
    if typ == 'timer':
        for k in (('',) if c == 1 else ('1', '2')):
            for n in ('TRIG', 'THRES', 'DISCH', 'OUT'):
                if count(n + k) != 1:
                    errors.append('%s%s on %d pins' % (n, k, count(n + k)))
        for n in ('VCC', 'GND'):
            if count(n) != 1:
                errors.append('%s on %d pins' % (n, count(n)))
        for n in names:
            if not (re.match(r'^(TRIG|THRES|DISCH|OUT)[12]?$', n) or n in ('VCC', 'GND') or EXTRA.match(n)):
                errors.append('timer pin name %r' % n)
    else:
        for k in (('',) if c == 1 else tuple(str(i) for i in range(1, c + 1))):
            for n in ('IN%s+', 'IN%s-', 'OUT%s'):
                n = n % k
                if count(n) != 1:
                    errors.append('%s on %d pins' % (n, count(n)))
        if count('V+') < 1 or count('V-') < 1:
            errors.append('needs a V+ and a V-')
        for n in names:
            if not (re.match(r'^(IN[1-4]?[+-]|OUT[1-4]?)$', n) or n in ('V+', 'V-') or EXTRA.match(n)):
                errors.append('op-amp pin name %r' % n)
    return errors


def facts_of(text):
    out = {}
    for item in text.split(';'):
        if item.strip():
            k, _, v = item.partition('=')
            out[k.strip()] = v.strip()
    return out


def check(s):
    e = []
    if not s['label']:
        e.append('no label')
    elif s['label'] not in LABELS:
        e.append('label %r' % s['label'])
    if not s['scheme']:
        return e + ['no scheme']
    typ, circuits = s['type'], s['circuits']
    n = pin_count(s['package'], s['supplier_package'])
    if s['scheme'] == 'explicit':
        if s['pinout'] in ('', '-'):
            return e + (['explicit scheme without a pinout'] if s['pinout'] == '' else [])
        pairs = parse_pinout(s['pinout'])
        ds = [d for d, _ in pairs]
        names = [x for _, x in pairs]
        if all(d.isdigit() for d in ds):
            if [int(d) for d in ds] != list(range(1, len(ds) + 1)):
                e.append('designators are not 1..%d' % len(ds))
        elif not all(BALL.match(d) for d in ds) or len(set(ds)) != len(ds):
            e.append('designators are neither 1..n nor ball names')
        if n is not None and len(ds) not in (n, n + 1):
            e.append('%d pins, the package names %d' % (len(ds), n))
        if len(ds) == (n or 0) + 1 and 'EP' not in names:
            e.append('one more pin than the package and no EP')
        e += names_ok(typ, circuits, names)
    else:
        if s['scheme'] not in SCHEMES:
            return e + ['scheme %r' % s['scheme']]
        if SCHEME_FOR[s['scheme']] != (typ, circuits):
            e.append('scheme %s is for %s with %s circuits' % ((s['scheme'],) + SCHEME_FOR[s['scheme']]))
        if n is not None and len(SCHEMES[s['scheme']]) != n:
            e.append('scheme has %d pins, the package names %d' % (len(SCHEMES[s['scheme']]), n))
        if s['pinout'] not in ('',):
            e.append('named scheme with a pinout')
    f = facts_of(s['facts'])
    vcc = re.fullmatch(r'(\d+(?:\.\d+)?)-(\d+(?:\.\d+)?)', f.get('vcc', ''))
    if not vcc or not 0 < float(vcc.group(1)) < float(vcc.group(2)) <= 100:
        e.append('vcc %r' % f.get('vcc'))
    if typ == 'timer':
        if f.get('process') not in ('bipolar', 'cmos'):
            e.append('process %r' % f.get('process'))
        for k, lo, hi in (('maxHz', 1, 100000000), ('tpd', 1, 100000)):
            if not re.fullmatch(r'\d+(?:\.\d+)?', f.get(k, '')) or not lo <= float(f[k]) <= hi:
                e.append('%s %r' % (k, f.get(k)))
    elif 'inputMax' in f and not re.fullmatch(r'\d+(?:\.\d+)?', f['inputMax']):
        e.append('inputMax %r' % f['inputMax'])
    for k in f:
        if k not in ('vcc', 'process', 'maxHz', 'tpd', 'inputMax'):
            e.append('unknown fact %r' % k)
    return e


def main(argv):
    opts, rest = {}, []
    i = 0
    while i < len(argv):
        if argv[i] in ('--type', '--company'):
            opts[argv[i]] = argv[i + 1]
            i += 2
        elif argv[i] == '--unfilled':
            opts['--unfilled'] = True
            i += 1
        else:
            rest.append(argv[i])
            i += 1
    path = rest[0] if rest else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'specs.tsv')
    rows = bad = unfilled = 0
    for s in csv.DictReader(open(path, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
        if opts.get('--type') and s['type'] != opts['--type']:
            continue
        if opts.get('--company') and s['company'] != opts['--company']:
            continue
        rows += 1
        if opts.get('--unfilled') and not s['scheme']:
            unfilled += 1
            print('unfilled |', s['company'], '|', s['pdf'], '|', s['package'], '|', s['circuits'])
            continue
        for e in check(s):
            bad += 1
            print('%s | %s | %s | %s circuits: %s' % (s['company'], s['pdf'], s['package'], s['circuits'], e))
    print('%d rows, %d errors%s' % (rows, bad, ', %d unfilled' % unfilled if unfilled else ''))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
