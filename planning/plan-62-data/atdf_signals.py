import sys
import os
import xml.etree.ElementTree as ET

# Usage: python3 atdf_signals.py <atdf dir of Microchip.ATmega_DFP.3.6.299.atpack>
D = sys.argv[1]
WANT = ('OC', 'ICP', 'T0', 'T1', 'T3', 'T4', 'RXD', 'TXD', 'XCK', 'MISO', 'MOSI', 'SCK', 'SS',
        'SDA', 'SCL', 'INT', 'ADC', 'XTAL', 'RESET', 'AIN', 'PCINT')


def signals(n):
    r = ET.parse(os.path.join(D, n + '.atdf')).getroot()
    out = {}
    for m in r.find('devices/device/peripherals'):
        for inst in m.findall('instance'):
            for s in inst.iter('signal'):
                g = s.get('group')
                idx = s.get('index')
                name = g + (idx if idx is not None and g in ('ADC', 'PCINT', 'INT', 'AIN') else '')
                if any(name.startswith(w) for w in WANT):
                    out.setdefault((inst.get('name'), name), s.get('pad'))
    return out


base = signals('ATmega328P')
print('ATmega328P:')
for k in sorted(base):
    print('  %-8s %-8s %s' % (k[0], k[1], base[k]))
for n in ['ATmega48', 'ATmega48A', 'ATmega48P', 'ATmega48PA', 'ATmega88', 'ATmega88A', 'ATmega88P',
          'ATmega88PA', 'ATmega168', 'ATmega168A', 'ATmega168P', 'ATmega168PA', 'ATmega328',
          'ATmega48PB', 'ATmega88PB', 'ATmega168PB', 'ATmega328PB']:
    s = signals(n)
    diff = {k: v for k, v in s.items() if base.get(k) != v}
    miss = [k for k in base if k not in s]
    print(n, 'differs/extra:', len(diff), 'missing:', len(miss))
    if n.endswith('PB'):
        for k in sorted(diff):
            print('  %-8s %-8s %s' % (k[0], k[1], diff[k]))
        print('  missing', sorted(miss))
