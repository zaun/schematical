#!/usr/bin/env python3
"""Every spec package's footprint has a pad for each pinout designator and the pin count.

usage: fpmatch.py <staging footprints.json>   (footprints.json is read too)
Pad designators come from the fpcheck golden lines (fpcheck prints them), so this runs
fpcheck on the staging file and parses its output.
"""
import json, os, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
lib = os.path.join(ROOT, 'schematical/schematical/Parts/footprints.json')
pads = {}
for path in (lib, sys.argv[1]):
    out = subprocess.run(['/tmp/p24/fpcheck/fpcheck', lib, path if path != lib else '/tmp/p24/fpcheck/empty.json'],
                         capture_output=True, text=True).stdout
    for line in out.splitlines():
        if '|pads=' in line:
            fid = line.split('|')[0]
            pads[fid] = {p.split(':')[0] for p in line.split('|pads=')[1].split(';')}
# library footprints: fpcheck prints only staging ones, so expand the library separately
for f in json.load(open(lib))['footprints']:
    if f['id'] not in pads:
        if f['style'] == 'grid':
            pads[f['id']] = set(f['balls'])
        elif f['style'] == 'explicit':
            pads[f['id']] = {str(p['pin']) for p in f['pads']}
        else:
            pads[f['id']] = {str(i) for i in range(1, f['pins'] + 1)}
bad = 0
for fn in sorted(os.listdir(os.path.join(DATA, 'specs'))):
    sp = json.load(open(os.path.join(DATA, 'specs', fn)))
    for code, pk in sp['packages'].items():
        pins = sp['device']['pinouts'][pk['pinout']]
        des = {str(p.get('number', p.get('ball'))) for p in pins}
        have = pads.get(pk['footprint'])
        if have is None:
            print(f'{fn}: {code}: footprint {pk["footprint"]} unknown'); bad += 1
        elif not des <= have:
            print(f'{fn}: {code}: {pk["footprint"]} lacks pads {sorted(des - have)[:8]}'); bad += 1
print(f'{bad} problems')
sys.exit(1 if bad else 0)
