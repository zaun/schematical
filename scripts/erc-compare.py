#!/usr/bin/env python3
"""Compare the app's electrical rules check with KiCad's, on the same designs (plan-63-G).

usage: python3 scripts/erc-compare.py <xcodebuild log> [--kicad-cli PATH] [--verbose]

The log is the output of the test host run with `TEST_RUNNER_ERC_OUT=1`
(ErcCompareTests.testPrintSamplesWhenAsked): per design, its KiCad export
(`KICAD_FILE erc/<design>/<file> <base64>`) and the violations the app's ERC
reports (`ERC_RESULT <design> <base64 JSON>`). The script writes each export to a
temporary folder, switches on the two rules KiCad leaves off (single_global_label,
four_way_junction), runs `kicad-cli sch erc` on it, and compares by rule over the
rules the app implements:

  * pin rules (pin_to_pin, pin_not_connected, pin_not_driven, power_pin_not_driven,
    no_connect_connected): the sorted list of (rule, pins), a pin as `U1.14`;
  * every other rule: how many.

A difference is printed unless DELIBERATE names it (rule, design) with its reason.
Exits 1 when a difference is left, else 0. Needs KiCad 10 and Python 3 only.
"""

import argparse
import base64
import collections
import json
import os
import re
import subprocess
import sys
import tempfile

KICAD_CLI = os.path.expanduser('~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli')

PIN_RULES = {'pin_to_pin', 'pin_not_connected', 'pin_not_driven', 'power_pin_not_driven', 'no_connect_connected'}
RULES = PIN_RULES | {
    'no_connect_dangling', 'wire_dangling', 'unconnected_wire_endpoint', 'label_dangling', 'isolated_pin_label',
    'multiple_net_names', 'similar_labels', 'similar_label_and_power', 'duplicate_reference', 'single_global_label',
    'four_way_junction',
}

# (rule, design or '*') -> why the two differ on purpose, each read off the comparison's own output.
DELIBERATE = {
    ('duplicate_reference', '*'): 'kicad-cli 10.0.8 does not report duplicate references (plan-63-data/README.md)',
    ('single_global_label', '*'): 'the export writes local labels and KiCad applies this rule to global labels only; '
                                  'its meaning was measured on global labels (plan-63-data/README.md)',
    ('multiple_net_names', 'AFU'): 'the export adds labels such as 5V_2 and GND_5 for bus wires named like a rail; '
                                   'the app has no such label (KiCadExport.busText)',
    ('multiple_net_names', 'LFU'): 'the same export labels as in AFU',
    ('four_way_junction', 'AFU'): 'at the point KiCad names four wire runs end, two of them the same segment: a wire '
                                  'bends at a junction without ending there; the app joins wires by their ends, '
                                  'KiCad by where they touch',
}

# The card edge connector the export adds beside the drawing (EDGE1, HEADER1: a symbol of passive pins) is not a part
# of the schematic. The app counts the edge bus's ports as its passive pins on their nets and reports nothing on
# them, so KiCad's reports on the connector's own pins are dropped.
EXPORT_ONLY = re.compile(r'^(EDGE|HEADER)\d+\.')

PIN = re.compile(r'Symbol (\S+) Pin (\S+)')


def read_log(path):
    exports, results = collections.defaultdict(dict), {}
    with open(path, errors='replace') as fh:
        for line in fh:
            parts = line.rstrip('\n').split(' ')
            if parts[0] == 'KICAD_FILE' and len(parts) == 3 and parts[1].startswith('erc/'):
                _, design, name = parts[1].split('/', 2)
                exports[design][name] = base64.b64decode(parts[2])
            elif parts[0] == 'ERC_RESULT' and len(parts) == 3:
                results[parts[1]] = json.loads(base64.b64decode(parts[2]))
    return exports, results


def run_kicad(cli, folder, design, files):
    for name, data in files.items():
        with open(os.path.join(folder, name), 'wb') as fh:
            fh.write(data)
    pro = os.path.join(folder, design + '.kicad_pro')
    settings = json.load(open(pro)) if os.path.exists(pro) else {}
    severities = settings.setdefault('erc', {}).setdefault('rule_severities', {})
    severities['single_global_label'] = 'warning'
    severities['four_way_junction'] = 'warning'
    with open(pro, 'w') as fh:
        json.dump(settings, fh)
    out = os.path.join(folder, 'erc.json')
    subprocess.run([cli, 'sch', 'erc', '--format', 'json', '--severity-all', '-o', out,
                    os.path.join(folder, design + '.kicad_sch')], check=True, capture_output=True)
    return json.load(open(out))


def kicad_summary(report):
    """{rule: sorted list of pin tuples, or a count}."""
    found = collections.defaultdict(list)
    for sheet in report['sheets']:
        for v in sheet['violations']:
            rule = v['type']
            if rule not in RULES:
                continue
            pins = sorted('%s.%s' % m for item in v['items'] for m in PIN.findall(item['description']))
            if rule in PIN_RULES and any(EXPORT_ONLY.match(p) for p in pins):
                continue
            found[rule].append(tuple(pins))
    return found


def ours_summary(result):
    found = collections.defaultdict(list)
    for v in result['violations']:
        rule = v['rule']
        pins = sorted(p.strip() for p in v['reference'].split(',')) if rule in PIN_RULES else ()
        found[rule].append(tuple(pins))
    return found


def main():
    ap = argparse.ArgumentParser(description="Compare the app's ERC with KiCad's.")
    ap.add_argument('log')
    ap.add_argument('--kicad-cli', default=KICAD_CLI)
    ap.add_argument('--verbose', action='store_true')
    args = ap.parse_args()
    exports, results = read_log(args.log)
    if not results:
        sys.exit('no ERC_RESULT lines in %s (run the test with TEST_RUNNER_ERC_OUT=1)' % args.log)
    differences, rows, deliberate = [], [], []
    with tempfile.TemporaryDirectory() as root:
        for design in sorted(results):
            folder = os.path.join(root, design)
            os.makedirs(folder)
            theirs = kicad_summary(run_kicad(args.kicad_cli, folder, design, exports[design]))
            ours = ours_summary(results[design])
            for rule in sorted(RULES):
                a, b = ours.get(rule, []), theirs.get(rule, [])
                if rule in ('pin_to_pin', 'no_connect_connected'):
                    # How many, and for pin_to_pin which pins are involved: KiCad pairs the pins of a net
                    # differently (three outputs on one net give two pairs either way), and it names a pin for
                    # a wired No Connect flag (the flag sits at the wire's far end, not on a pin) that is not
                    # always the one the app names.
                    same = len(a) == len(b) and (rule == 'no_connect_connected'
                                                 or {p for t in a for p in t} == {p for t in b for p in t})
                elif rule in PIN_RULES:
                    same = sorted(a) == sorted(b)
                else:
                    same = len(a) == len(b)
                rows.append((design, rule, len(a), len(b), same))
                if same:
                    continue
                why = DELIBERATE.get((rule, design)) or DELIBERATE.get((rule, '*'))
                if why:
                    deliberate.append((design, rule, len(a), len(b), why))
                else:
                    differences.append((design, rule, a, b))
    print('%-22s %-26s %6s %6s' % ('design', 'rule', 'ours', 'KiCad'))
    for design, rule, a, b, same in rows:
        if args.verbose or a or b:
            print('%-22s %-26s %6d %6d%s' % (design, rule, a, b, '' if same else '   *'))
    print('\ndesigns: %d; rows compared: %d' % (len(results), len(rows)))
    for design, rule, a, b, why in deliberate:
        print('deliberate: %s / %s: ours %d, KiCad %d: %s' % (design, rule, a, b, why))
    for design, rule, a, b in differences:
        print('DIFFERENCE %s / %s:\n    ours : %s\n    KiCad: %s' % (design, rule, sorted(a), sorted(b)))
    print('\n%d differences' % len(differences))
    sys.exit(1 if differences else 0)


if __name__ == '__main__':
    main()
