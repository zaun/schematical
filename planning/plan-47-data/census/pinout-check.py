#!/usr/bin/env python3
"""plan-47-B section 3.3: the structure of every specs row's pinout, label and terminals.

usage: python3 -I pinout-check.py [--maker <maker>] <type>...
       (type: inductor, fuse, ptc, relay, buzzer)

Relays (plan-47-C section 3.2): `label` is THT, SMD or a solid-state package family
(DIP, SOP, SSOP, VSSOP, SON, SIP); `rtype` electromechanical, reed or solid-state; the
`coils` column (`A1-A2`, `S1-S2,R1-R2`, `S-C,R-C`) and the `contacts` column
(`COM/NO/NC` per pole, `;` separated; a missing throw is empty; `@<n>` as a fourth
field names the channel of a multi-channel part) name pins that are in the pinout; every
pinout name that is in no coil or contact is `NC`; the pole count equals the DigiKey
contact form's; the pin count equals `pads`; `latching` and `polarized` are 0 or 1.
Buzzers (section 3.3): `label` THT or SMD; names `+`, `-` (once each), `F` and `NC`;
the pin count equals `pads`; `driver` indicator or transducer; `technology` piezo or magnetic.

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


_FORMS = None


def form_of(s):
    """The DigiKey contact form of the first download-set row of a specs row."""
    global _FORMS
    if _FORMS is None:
        _FORMS = {}
        for r in sp.read(sp.CANDIDATES):
            if sp.in_set(r):
                _FORMS.setdefault(sp.key(r), r['form'])
    return _FORMS.get((s['type'], s['maker'], s['series'], s['pdf'], s['package']), '')


def poles_of(form):
    """The number of poles of a DigiKey contact form, None when it cannot be read."""
    f = form.strip()
    m = re.match(r'(\d+) ×\s*(.*)', f)
    if m:
        inner = poles_of(m.group(2))
        return int(m.group(1)) * (inner or 1)
    m = re.match(r'(\d+) NO \+ (\d+) NC', f)
    if m:
        return int(m.group(1)) + int(m.group(2))
    m = re.search(r'\((\d+) Form', f)
    if m:
        return int(m.group(1))
    m = re.match(r'(SP|DP|3P|4P|5P|6P)', f)
    if m:
        return {'SP': 1, 'DP': 2, '3P': 3, '4P': 4, '5P': 5, '6P': 6}[m.group(1)]
    if f.startswith('1 Form'):
        return 1
    return None


def pin_list(pinout, errors):
    designators, names = [], []
    for item in pinout.split(','):
        if '=' not in item:
            errors.append(f"pin {item!r} is not <designator>=<name>")
            continue
        d, n = item.split('=', 1)
        designators.append(d)
        names.append(n)
    if len(set(designators)) != len(designators):
        errors.append('a designator is used twice')
    return names


def check_relay(s):
    errors = []
    if s['label'] not in sp.RELAY_LABELS:
        errors.append(f"label {s['label']!r} is not THT, SMD or a solid-state family")
    if s['rtype'] not in ('electromechanical', 'reed', 'solid-state'):
        errors.append(f"rtype {s['rtype']!r}")
    for c in ('polarized', 'latching'):
        if s[c] not in ('0', '1'):
            errors.append(f"{c} {s[c]!r}, not 0 or 1")
    pinout = s['pinout']
    if pinout in ('-', 'out'):
        return errors
    if not pinout:
        return errors + ['pinout empty']
    names = pin_list(pinout, errors)
    named = set()
    for coil in [c for c in s['coils'].split(',') if c]:
        pair = coil.split('-')
        if len(pair) != 2:
            errors.append(f"coil {coil!r} is not <a>-<b>")
            continue
        named.update(pair)
    poles = [c for c in s['contacts'].split(';') if c]
    for pole in poles:
        f = pole.split('/')
        if f[-1].startswith('@'):
            f = f[:-1]
        if len(f) != 3:
            errors.append(f"contact {pole!r} is not COM/NO/NC")
            continue
        named.update(x for x in f if x)
        if not f[0] or not (f[1] or f[2]):
            errors.append(f"contact {pole!r} needs a common and a throw")
    if not named:
        errors.append('no coil or contact named')
    for n in sorted(named):
        if n not in names:
            errors.append(f"{n!r} is not in the pinout")
    for n in sorted(set(names) - named):
        if n != 'NC':
            errors.append(f"pin {n!r} is in no coil or contact (it should be NC)")
    expect = poles_of(form_of(s))
    if expect is not None and len(poles) != expect:
        errors.append(f"{len(poles)} poles, the contact form {form_of(s)!r} has {expect}")
    if s['pads'] in ('', '-'):
        errors.append('pads empty')
    elif int(s['pads']) != len(names):
        errors.append(f"{len(names)} pins, pads {s['pads']}")
    return errors


def check_buzzer(s):
    errors = []
    if s['label'] not in sp.BUZZER_LABELS:
        errors.append(f"label {s['label']!r} is not THT or SMD")
    if s['driver'] not in ('indicator', 'transducer'):
        errors.append(f"driver {s['driver']!r}")
    if s['technology'] not in ('piezo', 'magnetic'):
        errors.append(f"technology {s['technology']!r}")
    pinout = s['pinout']
    if pinout in ('-', 'out'):
        return errors
    if not pinout:
        return errors + ['pinout empty']
    names = pin_list(pinout, errors)
    for n in names:
        if n not in ('+', '-', 'F', 'NC'):
            errors.append(f"pin name {n!r} is not +, -, F or NC")
    for n in ('+', '-'):
        if names.count(n) != 1:
            errors.append(f"{n!r} is on {names.count(n)} pins")
    if s['pads'] in ('', '-'):
        errors.append('pads empty')
    elif int(s['pads']) != len(names):
        errors.append(f"{len(names)} pins, pads {s['pads']}")
    return errors


def check(s):
    if s['type'] == 'relay':
        return check_relay(s)
    if s['type'] == 'buzzer':
        return check_buzzer(s)
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
