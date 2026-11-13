#!/usr/bin/env python3
# plan-37-C: prints the FIXTURE and FIXTURE_FOOTPRINTS C literals of
# backend/tests/register_test.c (single-quote JSON, as latch_test's).
# usage: python3 -I register-fixture.py > /tmp/fixture.inc
# Pin names follow the TI datasheets (sn74ls173a, sn74hc377, sn74ls379,
# sn74hc534, sn74als575a, sn74abt823, sn74alvch16820, sn74alvch16823);
# NC pins pad each pinout to an even count for the dual fixture footprints.
import json


def lo(pin):
    return {'pin': pin, 'active': 'low'}


def hi(pin):
    return {'pin': pin, 'active': 'high'}


def pinout(spec):
    """spec: list of (name, role); returns the pinout list, padded with NC to even."""
    pins = [{'number': i + 1, 'name': n, 'role': r} for i, (n, r) in enumerate(spec)]
    if len(pins) % 2:
        pins.append({'number': len(pins) + 1, 'name': 'NC%d' % (len(pins) + 1), 'role': 'nc'})
    return pins


def I(*names):
    return [(n, 'input') for n in names]


def O(*names):
    return [(n, 'output') for n in names]


P = [('GND', 'ground'), ('VCC', 'power')]
devices = []
counts = set()


def device(id_, desc, spec, groups):
    pins = pinout(spec)
    counts.add(len(pins))
    devices.append({
        'id': 'fx-' + id_, 'function': 'Register', 'description': desc, 'drawer': [],
        'pinouts': {'p%d' % len(pins): pins}, 'model': 'parallel', 'groups': groups,
        'timing': {'HC': {'rise': 10, 'fall': 10, 'vcc': [2, 6]}},
        'parts': [{'name': 'FX-' + id_, 'package': 'DIP', 'pinout': 'p%d' % len(pins),
                   'footprint': 'FX%d' % len(pins), 'family': 'HC'}]})


def bits(fmt, n, base=1):
    return [fmt % (i + base) for i in range(n)]


# sn74ls173a p.1: 1 M, 2 N, 3-6 1Q-4Q, 7 CLK, 8 GND, 9 G1, 10 G2, 11-14 4D-1D, 15 CLR, 16 VCC
device('74_173', '4-bit D-type registers with 3-state outputs',
       I('M', 'N') + O('1Q', '2Q', '3Q', '4Q') + I('CLK') + [P[0]] + I('G1', 'G2') +
       I('4D', '3D', '2D', '1D') + I('CLR') + [P[1]],
       [{'d': bits('%dD', 4), 'clk': {'pin': 'CLK', 'edge': 'rising'}, 'ce': [lo('G1'), lo('G2')],
         'clear': {'pin': 'CLR', 'active': 'high', 'when': 'async'},
         'outputs': [{'q': bits('%dQ', 4), 'oe': [lo('M'), lo('N')]}]}])
# sn74hc377: 1 G, 2 1Q, 3 1D, 4 2D, 5 2Q, 6 3Q, 7 3D, 8 4D, 9 4Q, 10 GND, 11 CLK, 12 5Q ... 20 VCC
s377 = (I('G') + O('1Q') + I('1D', '2D') + O('2Q', '3Q') + I('3D', '4D') + O('4Q') + [P[0]] +
        I('CLK') + O('5Q') + I('5D', '6D') + O('6Q', '7Q') + I('7D', '8D') + O('8Q') + [P[1]])
device('74_377', 'Octal D-type flip-flops with clock enable', s377,
       [{'d': bits('%dD', 8), 'clk': {'pin': 'CLK', 'edge': 'rising'}, 'ce': lo('G'),
         'outputs': [{'q': bits('%dQ', 8)}]}])
# sn74ls379: 1 G, 2 1Q, 3 1QN, 4 1D, 5 2D, 6 2QN, 7 2Q, 8 GND, 9 CLK, 10 3Q, 11 3QN, 12 3D, 13 4D, 14 4QN, 15 4Q, 16 VCC
device('74_379', 'Quadruple D-type flip-flops with clock enable',
       I('G') + O('1Q', '1QN') + I('1D', '2D') + O('2QN', '2Q') + [P[0]] + I('CLK') +
       O('3Q', '3QN') + I('3D', '4D') + O('4QN', '4Q') + [P[1]],
       [{'d': bits('%dD', 4), 'clk': {'pin': 'CLK', 'edge': 'rising'}, 'ce': lo('G'),
         'outputs': [{'q': bits('%dQ', 4)}, {'q': bits('%dQN', 4), 'invert': True}]}])
# sn74hc534: the '374 pinout, OE pin 1, CLK 11, inverted outputs
s534 = [('OE', 'input')] + s377[1:]
device('74_534', 'Octal edge-triggered D-type flip-flops with inverted 3-state outputs', s534,
       [{'d': bits('%dD', 8), 'clk': {'pin': 'CLK', 'edge': 'rising'},
         'outputs': [{'q': bits('%dQ', 8), 'invert': True, 'oe': lo('OE')}]}])
# sn74als575a: 1 CLR, 2-9 1D-8D, 10-11 NC, 12 GND, 13 CLK, 14 NC, 15-22 8Q-1Q, 23 OE, 24 VCC
device('74_575', 'Octal D-type edge-triggered flip-flops with synchronous clear and 3-state outputs',
       I('CLR') + I(*bits('%dD', 8)) + [('NC10', 'nc'), ('NC11', 'nc'), P[0]] + I('CLK') +
       [('NC14', 'nc')] + O(*reversed(bits('%dQ', 8))) + I('OE') + [P[1]],
       [{'d': bits('%dD', 8), 'clk': {'pin': 'CLK', 'edge': 'rising'},
         'clear': {'pin': 'CLR', 'active': 'low', 'when': 'clock'},
         'outputs': [{'q': bits('%dQ', 8), 'oe': lo('OE')}]}])
# sn74abt823: 1 OE, 2-10 1D-9D, 11 CLR, 12 GND, 13 CLK, 14 CLKEN, 15-23 9Q-1Q, 24 VCC
device('74_823', '9-bit bus-interface flip-flops with 3-state outputs',
       I('OE') + I(*bits('%dD', 9)) + I('CLR') + [P[0]] + I('CLK', 'CLKEN') +
       O(*reversed(bits('%dQ', 9))) + [P[1]],
       [{'d': bits('%dD', 9), 'clk': {'pin': 'CLK', 'edge': 'rising'}, 'ce': lo('CLKEN'),
         'clear': {'pin': 'CLR', 'active': 'low', 'when': 'async'},
         'outputs': [{'q': bits('%dQ', 9), 'oe': lo('OE')}]}])
# sn74alvch16820: one D per bit, two banks 1Q / 2Q with 1OE / 2OE
device('74_16820', '3.3-V 10-bit flip-flops with dual outputs and 3-state outputs',
       I('1OE', '2OE', 'CLK') + I(*bits('D%d', 10)) + O(*bits('1Q%d', 10)) +
       O(*bits('2Q%d', 10)) + P,
       [{'d': bits('D%d', 10), 'clk': {'pin': 'CLK', 'edge': 'rising'},
         'outputs': [{'q': bits('1Q%d', 10), 'oe': lo('1OE')},
                     {'q': bits('2Q%d', 10), 'oe': lo('2OE')}]}])
# sn74alvch16823: two 9-bit groups, each with CLK, CLKEN, CLR, OE
g16823 = []
s16823 = []
for n in (1, 2):
    s16823 += I('%dOE' % n, '%dCLR' % n, '%dCLK' % n, '%dCLKEN' % n) + \
        I(*bits('%dD%%d' % n, 9)) + O(*bits('%dQ%%d' % n, 9))
    g16823.append({'d': bits('%dD%%d' % n, 9), 'clk': {'pin': '%dCLK' % n, 'edge': 'rising'},
                   'ce': lo('%dCLKEN' % n),
                   'clear': {'pin': '%dCLR' % n, 'active': 'low', 'when': 'async'},
                   'outputs': [{'q': bits('%dQ%%d' % n, 9), 'oe': lo('%dOE' % n)}]})
device('74_16823', '18-bit bus-interface flip-flops with 3-state outputs', s16823 + P, g16823)

counts.add(6)   # the loader cases' p6 device (register_test check_loader)
parts = {'format': 'schematical-parts', 'version': 1, 'kind': 'register', 'devices': devices}
fps = {'format': 'schematical-footprints', 'version': 1, 'footprints': [
    {'id': 'FX%d' % n, 'description': 'd', 'style': 'dual', 'pins': n,
     'body': {'width': 1, 'height': 1}, 'pitch': 1, 'span': 2,
     'pad': {'width': 0.5, 'height': 0.5, 'shape': 'rect'}} for n in sorted(counts)]}


def literal(name, obj):
    text = json.dumps(obj, separators=(',', ':')).replace('"', "'")
    lines = [text[i:i + 88] for i in range(0, len(text), 88)]
    return 'static const char %s[] =\n%s;\n' % (name, '\n'.join('    "%s"' % l for l in lines))


print('/* Generated by planning/plan-37-data/census/register-fixture.py. */')
print(literal('FIXTURE', parts))
print('/* Dual-row fixture footprints FX<n>, one per pin count above. */')
print(literal('FIXTURE_FOOTPRINTS', fps))
