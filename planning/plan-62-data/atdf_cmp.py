import glob, os, re, sys
import xml.etree.ElementTree as ET

# Usage: python3 atdf_cmp.py <atdf dir of Microchip.ATmega_DFP.3.6.299.atpack>
D = sys.argv[1]
names = ['ATmega48', 'ATmega48A', 'ATmega48P', 'ATmega48PA', 'ATmega48PB',
         'ATmega88', 'ATmega88A', 'ATmega88P', 'ATmega88PA', 'ATmega88PB',
         'ATmega168', 'ATmega168A', 'ATmega168P', 'ATmega168PA', 'ATmega168PB',
         'ATmega328', 'ATmega328P', 'ATmega328PB']


def load(n):
    r = ET.parse(os.path.join(D, n + '.atdf')).getroot()
    dev = r.find('devices/device')
    mem = {}
    for seg in dev.iter('memory-segment'):
        mem[seg.get('name')] = (int(seg.get('start'), 0), int(seg.get('size'), 0))
    ints = {int(i.get('index')): i.get('name') for i in dev.iter('interrupt')}
    regs = {}
    for m in r.find('modules'):
        for rg in m.iter('register-group'):
            for reg in rg.findall('register'):
                regs[reg.get('name')] = (int(reg.get('offset'), 0), int(reg.get('size')))
    return mem, ints, regs


base = load('ATmega328P')
print('328P mem', {k: v for k, v in base[0].items()})
print('328P ints', base[1])
print('328P regs', len(base[2]))
for n in names:
    mem, ints, regs = load(n)
    d_int = {k: v for k, v in ints.items() if base[1].get(k) != v}
    miss_int = {k: v for k, v in base[1].items() if k not in ints}
    d_reg = {k: v for k, v in regs.items() if base[2].get(k) != v}
    miss_reg = sorted(k for k in base[2] if k not in regs)
    print('==', n, 'flash', mem.get('FLASH'), 'iram', mem.get('IRAM'), 'eeprom', mem.get('EEPROM'),
          'boot', [k for k in mem if 'BOOT' in k][:1])
    print('   ints differ:', d_int, 'missing:', miss_int)
    print('   regs differ/extra:', dict(sorted(d_reg.items(), key=lambda kv: kv[1])), 'missing:', miss_reg)
