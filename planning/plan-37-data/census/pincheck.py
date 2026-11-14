#!/usr/bin/env python3
"""plan-37-F Phase 1: each (maker, device) datasheet's pin table against the
dff device's pinout. A pin is classified by its function (D n, Q n, QBAR n,
clock, clear, output enable, ground, power, NC) from either side's name, and
compared pin number by pin number; the datasheets number data bits 0-based
(D0, Q0 / On) or 1-based (1D, D1), which the class normalises.
usage: python3 -I pincheck.py   (from the repository root)
Reads the pin tables as text: Nexperia "Pin description" tables
(names, then pin numbers) and two-column top views ("name n m name")."""
import csv, json, re, subprocess, collections

rows = [r for r in csv.DictReader(open('planning/plan-37-data/candidates.tsv'), delimiter='\t')
        if r['verdict'] == 'add' and r['kind'] == 'dff']
devs = {d['id']: d for d in json.load(open('schematical/schematical/Parts/dff.json'))['devices']}


def cls(name, zero_based):
    n = name.upper().replace('_', '').replace('̅', '')
    bar = n.endswith(('BAR', 'N')) and not n.startswith(('NC', 'GND')) and re.match(r'^\d?Q', n)
    n2 = re.sub(r'(BAR|N)$', '', n) if bar else n
    if n2 in ('GND', 'VSS'):
        return ('gnd',)
    if n2 in ('VCC', 'VDD'):
        return ('vcc',)
    if n2 in ('NC', 'N.C.'):
        return ('nc',)
    if n2 in ('CP', 'CLK', 'CK', 'C', 'CLOCK'):
        return ('clk',)
    if n2 in ('MR', 'CLR', 'R', 'RESET', 'CLEAR'):
        return ('clr',)
    if n2 in ('OE',):
        return ('oe',)
    m = re.match(r'^(\d)?([DQO])(\d)?$', n2)
    if m and (m.group(1) or m.group(3)):
        idx = int(m.group(1) or m.group(3)) + (1 if (zero_based and m.group(3)) else 0)
        k = 'd' if m.group(2) == 'D' else 'q'
        return (k + ('bar' if bar else ''), idx)
    return ('?', name)


def table(pdf):
    t = subprocess.run(['pdftotext', '-layout', 'datasheets/' + pdf, '-'], capture_output=True, text=True).stdout
    pins = {}
    # Nexperia: "D0, D1, D2  3, 4, 7" or "CP  11"
    i = t.find('Pin description')
    if i >= 0:
        for l in t[i:i + 4000].splitlines():
            m = re.match(r'^\s*([A-Za-z][A-Za-z0-9, ]*?)\s{2,}(\d[\d, ]*)\s{2,}\S', l)
            if m:
                names = [x.strip() for x in m.group(1).split(',') if x.strip()]
                nums = [int(x) for x in re.findall(r'\d+', m.group(2))]
                if len(names) == len(nums):
                    for a, b in zip(names, nums):
                        pins.setdefault(b, a)
                elif len(names) == 1:
                    for b in nums:
                        pins.setdefault(b, names[0])
    if len(pins) < 10:
        for l in t.splitlines():
            m = re.match(r'^\s*(\S+)\s+(\d{1,2})\s+(\d{1,2})\s+(\S+)\s*$', l)
            if m:
                pins.setdefault(int(m.group(2)), m.group(1))
                pins.setdefault(int(m.group(3)), m.group(4))
    return pins


pairs = collections.defaultdict(set)
for r in rows:
    pairs[(r['device'], r['pdf'])].add(r['maker'][:3])
bad = 0
for (dev, pdf), makers in sorted(pairs.items()):
    po = next(p for k, p in devs[dev]['pinouts'].items() if k.startswith('p'))
    want = {p['number']: cls(p['name'], False) for p in po}
    got_names = table(pdf)
    zero = any(re.match(r'^[DQO]0$', v) for v in got_names.values())
    got = {k: cls(v, zero) for k, v in got_names.items() if k in want}
    diff = [(k, got_names[k], want[k]) for k in sorted(got) if got[k] != want[k] and got[k][0] not in ('gnd', 'vcc')]
    status = 'ok' if len(got) >= len(want) - 2 and not diff else 'CHECK'
    if status != 'ok':
        bad += 1
    print('%-7s %-28s %-4s read %2d/%2d %s %s' % (dev, pdf, ','.join(sorted(makers)), len(got), len(want), status,
                                                  diff[:4]))
print(bad, 'to check by eye')
