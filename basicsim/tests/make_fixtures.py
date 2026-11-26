"""Writes basicsim's failure fixtures from the app's gate.json (plan-68-L).

Run from the repo root: python3 basicsim/tests/make_fixtures.py
- one_gate.json: only the AND (SN74LS08N), so the NAND part is missing.
- slow_gates.json: both parts, the LS family's NAND fall delay 7 -> 20 ns, so
  the NAND is not yet 0 at the 28 ns checkpoint.
"""
import json
import os

here = os.path.dirname(os.path.abspath(__file__))
gates = json.load(open(os.path.join(here, '..', '..', 'schematical', 'schematical', 'Parts', 'gate.json')))
keep = {'74_08': 'SN74LS08N', '74_00': 'SN74LS00N'}
devices = []
for device in gates['devices']:
    if device['id'] in keep:
        device = dict(device)
        device['parts'] = [p for p in device['parts'] if p['name'] == keep[device['id']]]
        devices.append(device)


def write(name, chosen):
    out = {k: v for k, v in gates.items() if k != 'devices'}
    out['devices'] = chosen
    with open(os.path.join(here, name), 'w') as f:
        json.dump(out, f, indent=1)
        f.write('\n')


write('one_gate.json', [d for d in devices if d['id'] == '74_08'])
slow = [json.loads(json.dumps(d)) for d in devices]
for d in slow:
    if d['id'] == '74_00':
        d['timing']['LS']['fall'] = 20
write('slow_gates.json', slow)
