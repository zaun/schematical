#!/usr/bin/env python3
"""Builds the ns16550 fixture part file serial_test loads (plan-12-D section 3.3).

Each device is keys/<ID>.json (the model keys) merged into devices/<ID>.json
(the datasheet data: pinouts, grades, parts). The fixture gets one part per
pinout, SFX-<ID>-<pinout>, on the footprint the real parts of that pinout
use; SFX-<ID> names the first. A device whose data file is missing gets a
stand-in pinout of the pins its keys name, on a T<n> dual footprint, so its
model cases can run; the shipped data replaces it when it lands.

Run from the repo root:  python3 -I backend/tests/fixtures/serial/merge.py
Writes ns16550.json and footprints_test.json beside this script.
  merge.py --ship F,G   also writes Parts/serial.json with the devices of
                        those data sub-plans and their real parts.
"""
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUTPUTS = ('tx', 'int', 'rts', 'dtr', 'out1', 'out2', 'txrdy', 'rxrdy', 'op', 'mf')
STAND_IN_GRADE = {'trvd': 60, 'thz': 100, 'trdd': 60, 'tbld': 175, 'trint': 1000, 'thr': 175,
                  'tmdo': 200, 'tsim': 250, 'trim': 250, 'twxi': 195, 'twrr': 290, 'tir': 250,
                  'tcso': 50, 'vcc': [4.5, 5.5], 'note': 'stand-in grade (no device data yet)'}


def key_pins(keys):
    """(name, role) for every pin the keys name."""
    pins = []

    def add(name, role):
        if name and name not in [p[0] for p in pins]:
            pins.append((name, role))

    bus = keys['bus']
    for n in bus.get('data', []):
        add(n, 'inout')
    for n in bus.get('address', []) + bus.get('select', []) + bus.get('selectHigh', []):
        add(n, 'input')
    for k in ('read', 'readHigh', 'write', 'writeHigh', 'addressStrobe', 'reset'):
        add(bus.get(k), 'input')
    for k in ('driverDisable', 'selectOut'):
        add(bus.get(k), 'output')
    clock = keys['clock']
    for k in ('in', 'single', 'receive', 'select'):
        add(clock.get(k), 'input')
    for k in ('out', 'baudOut'):
        add(clock.get(k), 'output')
    for ch in keys['channels']:
        for k, v in ch.items():
            if k in ('select', 'selectHigh'):
                for n in v:
                    add(n, 'input')
            else:
                add(v, 'output' if k in OUTPUTS else 'input')
    add(keys.get('channelSelect', {}).get('pin'), 'input')
    add(keys.get('intControl'), 'input')
    add(keys.get('tristate'), 'input')
    for k in ('txrdy', 'rxrdy'):
        add(keys.get('dma', {}).get(k), 'output')
    lpt = keys.get('printerPort')
    if lpt:
        for n in lpt['data']:
            add(n, 'inout')
        add(lpt['select'], 'input')
        for k in ('strobe', 'autoFeed', 'init', 'selectIn'):
            add(lpt[k], 'inout')
        for k in ('ack', 'busy', 'paperEnd', 'slct', 'error', 'inputEnable', 'pemd', 'enirq'):
            add(lpt.get(k), 'input')
        add(lpt['irq'], 'output')
    add('VCC', 'power')
    add('GND', 'ground')
    if len(pins) % 2:
        pins.append(('NC', 'nc'))
    return pins


def main():
    devices = []
    footprints = {}
    for name in sorted(os.listdir(os.path.join(HERE, 'keys'))):
        dev_id = name[:-5]
        keys = json.load(open(os.path.join(HERE, 'keys', name)))
        data_path = os.path.join(HERE, 'devices', name)
        if os.path.exists(data_path):
            data = json.load(open(data_path))
            device = {k: v for k, v in data.items() if k != 'parts'}
            parts = []
            for pinout in data['pinouts']:
                real = [p for p in data['parts'] if p['pinout'] == pinout]
                if not real:
                    continue
                part = {'name': 'SFX-%s-%s' % (dev_id, pinout), 'package': real[0]['package'],
                        'pinout': pinout, 'footprint': real[0]['footprint'],
                        'grade': real[0]['grade']}
                for k in ('vcc', 'inputMax'):
                    if k in real[0]:
                        part[k] = real[0][k]
                parts.append(part)
            first = dict(parts[0])
            first['name'] = 'SFX-' + dev_id
            parts.insert(0, first)
        else:
            pins = key_pins(keys)
            fp = 'T%d' % len(pins)
            footprints[fp] = len(pins)
            device = {'id': dev_id, 'function': 'UART', 'description': 'stand-in fixture',
                      'drawer': ['Interface'],
                      'pinouts': {'test': [{'number': i + 1, 'name': n, 'role': r}
                                           for i, (n, r) in enumerate(pins)]},
                      'grades': {'T': STAND_IN_GRADE}}
            parts = [{'name': 'SFX-' + dev_id, 'package': 'DIP', 'pinout': 'test',
                      'footprint': fp, 'grade': 'T'}]
        device.update(keys)
        device['id'] = 'SFX-' + dev_id   # beside the shipped device of the same id
        device['parts'] = parts
        devices.append(device)
    out = {'format': 'schematical-parts', 'version': 1, 'kind': 'serial', 'devices': devices}
    with open(os.path.join(HERE, 'ns16550.json'), 'w') as f:
        json.dump(out, f, indent=1)
        f.write('\n')
    fps = [{'id': fp, 'description': 'test dual', 'style': 'dual', 'pins': n,
            'body': {'width': 10, 'height': 2 + n}, 'pitch': 1, 'span': 8,
            'pad': {'width': 0.5, 'height': 0.5, 'shape': 'rect'}}
           for fp, n in sorted(footprints.items())]
    with open(os.path.join(HERE, 'footprints_test.json'), 'w') as f:
        json.dump({'format': 'schematical-footprints', 'version': 1, 'footprints': fps}, f, indent=1)
        f.write('\n')
    print(len(devices), 'devices,', len(fps), 'stand-in footprints')


def group_of(dev_id, model, function):
    """plan-12-A section 3.4: which data sub-plan ships a device."""
    if model == 'ns16550' and function == 'UART':
        return 'F' if dev_id.startswith(('TL16', 'NS16', 'PC16', '82C50', 'COM92')) else 'G'
    if model in ('ay1013', 'i8251', 'i82510', 'acia6551', 'hs82c52', 'com78c80x'):
        return 'H'
    if model in ('max3100', 'max310x'):
        return 'J'
    return 'I'


def ship(groups):
    """Writes schematical/schematical/Parts/serial.json: every device of the
    given groups (and the groups already shipped) with its real parts."""
    import csv
    repo = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..'))
    rows = list(csv.DictReader(open(os.path.join(repo, 'planning', 'plan-12-data', 'devices.tsv')),
                               delimiter='\t'))
    out_path = os.path.join(repo, 'schematical', 'schematical', 'Parts', 'serial.json')
    devices = []
    for row in rows:
        dev_id = row['device']
        if group_of(dev_id, row['model'], row['function']) not in groups:
            continue
        keys_path = os.path.join(HERE, 'keys', dev_id + '.json')
        data_path = os.path.join(HERE, 'devices', dev_id + '.json')
        if not (os.path.exists(keys_path) and os.path.exists(data_path)):
            raise SystemExit('missing keys or data for ' + dev_id)
        data = json.load(open(data_path))
        device = {k: v for k, v in data.items() if k != 'parts'}
        device.update(json.load(open(keys_path)))
        device['parts'] = data['parts']
        devices.append(device)

    with open(out_path, 'w') as f:
        json.dump({'format': 'schematical-parts', 'version': 1, 'kind': 'serial',
                   'devices': devices}, f, indent=1)
        f.write('\n')
    print('serial.json:', len(devices), 'devices,', sum(len(d['parts']) for d in devices), 'parts')


if __name__ == '__main__':
    import sys
    if len(sys.argv) > 2 and sys.argv[1] == '--ship':
        ship(set(sys.argv[2].split(',')))
    else:
        main()
