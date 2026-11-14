#!/usr/bin/env python3 -I
"""plan-39 slices: the device/pinout slices plan-39-E..K review, with their sizes.
Run: python3 -I planning/plan-39-data/slices.py   (from the repo root)
`--devices A-B` of scripts/pin-polarity.js takes the 1-based indices printed here."""
import json, os, collections
root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..') + os.sep
P = root + 'schematical/schematical/Parts/'
SIG = ('input', 'output', 'inout')
def devs(k): return sorted(json.load(open(P + k + '.json'))['devices'], key=lambda d: d['id'])
def stats(k, lo, hi):
    ds = devs(k)[lo - 1:hi]
    pin = sum(len(d['pinouts']) for d in ds)
    sig = sum(1 for d in ds for p in d['pinouts'].values() for q in p if q['role'] in SIG)
    mux = sum(1 for d in ds for p in d['pinouts'].values() for q in p if q['role'] in SIG and '/' in q['name'])
    hsh = sum(1 for d in ds for p in d['pinouts'].values() for q in p if '#' in q['name'])
    return dict(kind=k, lo=lo, hi=hi, first=ds[0]['id'], last=ds[-1]['id'], dev=len(ds), pinouts=pin, sig=sig, mux=mux, hash=hsh)
def split(k, lo, hi, n):
    ds = devs(k)[lo - 1:hi]; tot = sum(len(d['pinouts']) for d in ds); acc = 0; start = lo; out = []; cum = 0
    for i, d in enumerate(ds, lo):
        acc += len(d['pinouts'])
        if acc >= tot / n * (len(out) + 1) and len(out) < n - 1:
            out.append((start, i)); start = i + 1
    out.append((start, hi))
    return [stats(k, a, b) for a, b in out]
def whole(k): return stats(k, 1, len(devs(k)))
N = lambda k: len(devs(k))

LETTERS = {
 'E': dict(name='logic-small', title='Counters, decoders, latches, registers, shifters, flip-flops, mux, arith, CPU, PIO, RTC',
           phases=[[whole('counter'), whole('decoder')], [whole('latch'), whole('register'), whole('shift')],
                   [whole('arith'), whole('dff'), whole('jkff'), whole('mux'), whole('cpu'), whole('pio'), whole('rtc')]]),
 'F': dict(name='buffer-osc', title='Buffers and oscillators',
           phases=[[s] for s in split('buffer', 1, N('buffer'), 2) + split('oscillator', 1, N('oscillator'), 2)]),
 'G': dict(name='memory-serial', title='SRAM, EEPROM and serial controllers',
           phases=[[whole('eeprom'), whole('sram')]] + [[s] for s in split('serial', 1, N('serial'), 2)]),
 'H': dict(name='converter-1', title='Converters, devices 1–379 (ADC0802 … ADS8548)',
           phases=[[s] for s in split('converter', 1, 379, 3)]),
 'I': dict(name='converter-2', title='Converters, devices 380–764 (ADS8555 … TLV571)',
           phases=[[s] for s in split('converter', 380, 764, 3)]),
 'J': dict(name='mcu-1', title='Microcontrollers, devices 1–145 (at90can128 … attiny3224)',
           phases=[[s] for s in split('mcu', 1, 145, 2)]),
 'K': dict(name='mcu-2', title='Microcontrollers, devices 146–303 (attiny3226 … rp2040)',
           phases=[[s] for s in split('mcu', 146, 303, 2)]),
}
if __name__ == '__main__':
    for L, d in LETTERS.items():
        tot = collections.Counter()
        print(L, d['title'])
        for i, ph in enumerate(d['phases'], 1):
            for s in ph:
                print('  ', i, s); tot['p'] += s['pinouts']; tot['s'] += s['sig']; tot['m'] += s['mux']; tot['h'] += s['hash']; tot['d'] += s['dev']
        print('  total', dict(tot))
