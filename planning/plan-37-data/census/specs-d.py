#!/usr/bin/env python3
"""plan-37-D: writes planning/plan-37-data/specs/<device>.json for the 17 narrow
register devices (gen-parts.py reads them). Pinouts, groups and timing are
transcribed from the datasheets named in each device's note and timing notes.
usage: python3 -I specs-d.py   (from the repository root)"""
import json
from pathlib import Path

OUT = Path('planning/plan-37-data/specs')
G, V, NC = ('GND', 'ground'), ('VCC', 'power'), ('NC', 'nc')
VSS, VDD = ('VSS', 'ground'), ('VDD', 'power')


def I(*n):
    return [(x, 'input') for x in n]


def O(*n):
    return [(x, 'output') for x in n]


def pins(spec):
    return [{'number': i + 1, 'name': n, 'role': r} for i, (n, r) in enumerate(spec)]


def fk16(p16):
    """TI's 16-pin die in the 20-terminal FK carrier: 1 NC, 2-5 = 1-4, 6 NC,
    7-10 = 5-8, 11 NC, 12-15 = 9-12, 16 NC, 17-20 = 13-16."""
    return [NC] + p16[0:4] + [NC] + p16[4:8] + [NC] + p16[8:12] + [NC] + p16[12:16]


def lo(p):
    return {'pin': p, 'active': 'low'}


def hi(p):
    return {'pin': p, 'active': 'high'}


def clk(p='CLK'):
    return {'pin': p, 'edge': 'rising'}


def seq(fmt, n, base=1):
    return [fmt % (i + base) for i in range(n)]


def t(rise, fall, vcc, note, input_max=None):
    e = {'rise': rise, 'fall': fall, 'vcc': vcc}
    if input_max:
        e['inputMax'] = input_max
    e['note'] = note
    return e


def device(id_, desc, note, pinouts, groups, timing, part_vcc=None):
    d = {'id': id_, 'function': 'Register', 'description': desc, 'drawer': ['Logic', 'Registers'],
         'note': note, 'model': 'parallel', 'pinouts': pinouts, 'groups': groups, 'timing': timing}
    if part_vcc:
        d['partVcc'] = part_vcc
    (OUT / (id_ + '.json')).write_text(json.dumps(d, indent=1, ensure_ascii=False) + '\n')


V5 = [4.5, 5.5]
V5C = [4.75, 5.25]
VHC = [2, 6]
VLVC = [1.65, 3.6]

# ---- 74_173 -------------------------------------------------------------------
p173 = I('M', 'N') + O('1Q', '2Q', '3Q', '4Q') + I('CLK') + [G] + I('G1', 'G2') + I('4D', '3D', '2D', '1D', 'CLR') + [V]
device('74_173', '4-bit D-type registers with 3-state outputs',
       'Pinout and function table: sn54173 (SN54173, SN54LS173A, SN74173, SN74LS173A) p.1-2, the FK carrier '
       'p.1; cd54hc173 (CD74HC173, CD74HCT173: OE1, OE2 = M, N; E1, E2 = G1, G2; MR = CLR; CP = CLK; Dx, Qx); '
       '74hc_hct173 (Nexperia 74HC173, 74HCT173: OE1, OE2, E1, E2, MR, CP, Dn, Qn) Table 2. M or N high: Q = Z; '
       'G1 or G2 high: hold at the clock edge; CLR high clears at once.',
       {'p16': pins(p173), 'lccc20': pins(fk16(p173))},
       [{'d': seq('%dD', 4), 'clk': clk(), 'ce': [lo('G1'), lo('G2')],
         'clear': {'pin': 'CLR', 'active': 'high', 'when': 'async'},
         'outputs': [{'q': seq('%dQ', 4), 'oe': [lo('M'), lo('N')]}]}],
       {'TTL': t(28, 19, V5C, 'CLK to Q typ, 5 V 50 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn54173 p.3-4)'),
        'LS': t(17, 22, V5C, 'CLK to Q typ, 5 V 45 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn54173 p.5-6)'),
        'HC': t(17, 17, VHC, 'CLK to Q typ, 5 V 15 pF; VCC 2-6 V (cd54hc173 § 5.5)'),
        'HCT': t(17, 17, V5, 'CLK to Q typ, 5 V 15 pF; VCC 4.5-5.5 V (cd54hc173 § 5.5)')})

# ---- 4076B --------------------------------------------------------------------
p4076 = I('M', 'N') + O('Q1', 'Q2', 'Q3', 'Q4') + I('CLOCK') + [VSS] + I('G1', 'G2') + I('D4', 'D3', 'D2', 'D1', 'RESET') + [VDD]
device('4076B', 'CMOS 4-bit D-type register with 3-state outputs',
       'Pinout, truth table: cd4076b p.1-2 (DATA 1-4 = D1-D4; OUTPUT DISABLE M, N; DATA INPUT DISABLE G1, G2); '
       'mc14076b (onsemi MC14076B: A, B output disable; A, B data disable; C clock; R reset; D0-D3, Q0-Q3), same '
       'pin numbers. M or N high: Q = Z; G1 or G2 high: hold at the clock edge; RESET high clears at once.',
       {'p16': pins(p4076)},
       [{'d': seq('D%d', 4), 'clk': clk('CLOCK'), 'ce': [lo('G1'), lo('G2')],
         'clear': {'pin': 'RESET', 'active': 'high', 'when': 'async'},
         'outputs': [{'q': seq('Q%d', 4), 'oe': [lo('M'), lo('N')]}]}],
       {'C': t(300, 300, [3, 18], 'Clock to Q typ, VDD 5 V 50 pF; VDD 3-18 V (cd4076b p.1, 3)')})

# ---- 40174B -------------------------------------------------------------------
p40174 = I('CLEAR') + O('Q1') + I('D1', 'D2') + O('Q2') + I('D3') + O('Q3') + [VSS] + I('CLOCK') + O('Q4') + \
    I('D4') + O('Q5') + I('D5', 'D6') + O('Q6') + [VDD]
device('40174B', 'CMOS hex D-type flip-flop with common clock and clear',
       'Pinout, truth table: cd40174b p.1, 4 (terminal assignment); mc14174b (onsemi MC14174B: R, C, D0-D5, '
       'Q0-Q5), same pin numbers. CLEAR low clears at once.',
       {'p16': pins(p40174)},
       [{'d': seq('D%d', 6), 'clk': clk('CLOCK'), 'clear': {'pin': 'CLEAR', 'active': 'low', 'when': 'async'},
         'outputs': [{'q': seq('Q%d', 6)}]}],
       {'C': t(150, 150, [3, 18], 'Clock to output typ, VDD 5 V 50 pF; VDD 3-18 V (cd40174b p.2, 4)')})

# ---- 74_377 / 378 / 379 (sn54ls378 p.1-2) -----------------------------------
p377 = I('G') + O('1Q') + I('1D', '2D') + O('2Q', '3Q') + I('3D', '4D') + O('4Q') + [G] + I('CLK') + O('5Q') + \
    I('5D', '6D') + O('6Q', '7Q') + I('7D', '8D') + O('8Q') + [V]
device('74_377', 'Octal D-type flip-flops with clock enable',
       'Pinout and function table: sn54ls378 (SN54/74LS377) p.1, sn54hc377, sn74hct377, sn54abt377, sn74f377a, '
       'cd54hct377 (CD74HC377: E = G, CP = CLK, Dn, Qn), cy74fct377t (CY74FCT377T: CE, CP, D0-D7, O0-O7); '
       'Nexperia 74hc_hct377, 74ahc_ahct377, 74lvc377 (E, CP, D0-D7, Q0-Q7); onsemi mc74ac377 (CE, CP, D0-D7, '
       'Q0-Q7). All the same pin numbers; FK carriers number 1:1. G low: CLK up loads; G high: hold.',
       {'p20': pins(p377)},
       [{'d': seq('%dD', 8), 'clk': clk(), 'ce': lo('G'), 'outputs': [{'q': seq('%dQ', 8)}]}],
       {'ABT': t(4.5, 5.3, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn54abt377 switching characteristics)'),
        'AC': t(9, 10, VHC, 'CP to Qn max (no typ given), 5 V 50 pF; VCC 2-6 V (mc74ac377 AC characteristics)'),
        'ACT': t(9, 10, V5, 'CP to Qn max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (mc74ac377 ACT characteristics)'),
        'AHCT': t(4.0, 4.0, V5, 'CP to Qn typ, 5 V 15 pF; VCC 4.5-5.5 V (Nexperia 74ahc_ahct377 Table 8)'),
        'F': t(6.5, 7, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74f377a switching characteristics)'),
        'FCT': t(7.2, 7.2, V5C, 'CP to O max, A grade (CY74FCT377AT), 5 V 50 pF (no typ given); VCC 4.75-5.25 V '
                 '(cy74fct377t switching characteristics)'),
        'HC': t(15, 15, VHC, 'CLK to Q typ, 4.5 V 50 pF; VCC 2-6 V (sn54hc377 § 6.6)'),
        'HCT': t(15, 15, V5, 'CLK to Q typ, 4.5 V 50 pF; VCC 4.5-5.5 V (sn74hct377 switching characteristics)'),
        'LS': t(17, 18, V5C, 'CLK to Q typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn54ls378 p.4)'),
        'LVC': t(4.0, 4.0, VLVC, 'CP to Qn typ, 3.0-3.6 V; VCC 1.65-3.6 V, inputs 5.5 V (Nexperia 74lvc377 '
                 'dynamic characteristics)', 5.5)})

p378 = I('G') + O('1Q') + I('1D', '2D') + O('2Q') + I('3D') + O('3Q') + [G] + I('CLK') + O('4Q') + I('4D') + \
    O('5Q') + I('5D', '6D') + O('6Q') + [V]
device('74_378', 'Hex D-type flip-flops with clock enable',
       'Pinout and function table: sn54ls378 (SN54/74LS378) p.1; mc74f378 / SN74F378 would be the same pins. '
       'G low: CLK up loads; G high: hold.',
       {'p16': pins(p378)},
       [{'d': seq('%dD', 6), 'clk': clk(), 'ce': lo('G'), 'outputs': [{'q': seq('%dQ', 6)}]}],
       {'LS': t(17, 18, V5C, 'CLK to Q typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn54ls378 p.4)')})

p379 = I('G') + O('1Q', '1QBAR') + I('1D', '2D') + O('2QBAR', '2Q') + [G] + I('CLK') + O('3Q', '3QBAR') + \
    I('3D', '4D') + O('4QBAR', '4Q') + [V]
device('74_379', 'Quadruple D-type flip-flops with clock enable and complementary outputs',
       'Pinout and function table: sn54ls378 (SN54/74LS379) p.1-2, the FK carrier p.2. G low: CLK up loads; '
       'G high: hold; Q and QBAR are complementary.',
       {'p16': pins(p379), 'lccc20': pins(fk16(p379))},
       [{'d': seq('%dD', 4), 'clk': clk(), 'ce': lo('G'),
         'outputs': [{'q': seq('%dQ', 4)}, {'q': seq('%dQBAR', 4), 'invert': True}]}],
       {'LS': t(17, 18, V5C, 'CLK to Q typ, 5 V 15 pF; VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (sn54ls378 p.4)')})

# ---- 74_534 ('374 pinout, inverting) / 74_564 ('574 pinout, inverting) ------
p374 = I('OE') + p377[1:]
device('74_534', 'Octal edge-triggered D-type flip-flops with inverted 3-state outputs',
       'Pinout and function table: sn74abt534a, sn74ac534, sn74act534, sn74als534a (the 74x374 pinout with OE '
       'on pin 1 and inverted outputs 1Q-8Q), cd74ac534, cd74hc564 (CD74HCT534). OE high: Q = Z; Q = NOT D '
       'after CLK up.',
       {'p20': pins(p374)},
       [{'d': seq('%dD', 8), 'clk': clk(), 'outputs': [{'q': seq('%dQ', 8), 'invert': True, 'oe': lo('OE')}]}],
       {'ABT': t(4.5, 5.5, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74abt534a switching characteristics)'),
        'AC': t(10.5, 9.5, VHC, 'CLK to Q max (no typ given), 5 V 50 pF; VCC 2-6 V (sn74ac534 switching characteristics)'),
        'ACT': t(11.5, 10.5, V5, 'CLK to Q max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (sn74act534 switching characteristics)'),
        'ALS': t(12, 16, V5, 'CLK to Q SN74ALS534A max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (sn74als534a switching characteristics)'),
        'HCT': t(14, 14, V5, 'Clock to output typ, 4.5 V 50 pF; VCC 4.5-5.5 V (cd74hc564, CD74HCT534 switching characteristics)')})

p574 = I('OE') + I(*seq('%dD', 8)) + [G] + I('CLK') + O(*reversed(seq('%dQ', 8))) + [V]
device('74_564', 'Octal edge-triggered D-type flip-flops with inverted 3-state outputs',
       'Pinout and function table: sn74ac564, sn74act564, sn74als564b, cd74hc564 (CD74HC564, CD74HCT564): the '
       '74x574 flow-through pinout with inverted outputs. OE high: Q = Z; Q = NOT D after CLK up.',
       {'p20': pins(p574)},
       [{'d': seq('%dD', 8), 'clk': clk(), 'outputs': [{'q': seq('%dQ', 8), 'invert': True, 'oe': lo('OE')}]}],
       {'AC': t(4.9, 5, VHC, 'CLK to Q typ, 5 V 50 pF; VCC 2-6 V (sn74ac564 switching characteristics)'),
        'ACT': t(6.5, 6, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74act564 switching characteristics)'),
        'ALS': t(14, 14, V5, 'CLK to Q SN74ALS564B max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (sn74als564b switching characteristics)'),
        'HC': t(13, 13, VHC, 'Clock to output typ, 4.5 V 50 pF; VCC 2-6 V (cd74hc564 switching characteristics)'),
        'HCT': t(14, 14, V5, 'Clock to output typ, 4.5 V 50 pF; VCC 4.5-5.5 V (cd74hc564 switching characteristics)')})

device('74_576', 'Octal edge-triggered D-type flip-flops with inverted 3-state outputs',
       'Pinout and function table: sn54als576b (SN54/74ALS576B) p.1: the 74x574 pinout with inverted outputs. '
       'OE high: Q = Z; Q = NOT D after CLK up.',
       {'p20': pins(p574)},
       [{'d': seq('%dD', 8), 'clk': clk(), 'outputs': [{'q': seq('%dQ', 8), 'invert': True, 'oe': lo('OE')}]}],
       {'ALS': t(14, 14, V5, 'CLK to Q SN74ALS576B max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (sn54als576b switching characteristics)')})

p575 = I('CLR', 'OE') + I(*seq('%dD', 8)) + [NC, G, NC] + I('CLK') + O(*reversed(seq('%dQ', 8))) + [NC, V]
device('74_575', 'Octal D-type edge-triggered flip-flops with synchronous clear and 3-state outputs',
       'Pinout and function table: sn74als575a (SN74ALS575A, 24-pin DW / NT) p.1. CLR low at CLK up: Q = 0; '
       'OE high: Q = Z.',
       {'p24': pins(p575)},
       [{'d': seq('%dD', 8), 'clk': clk(), 'clear': {'pin': 'CLR', 'active': 'low', 'when': 'clock'},
         'outputs': [{'q': seq('%dQ', 8), 'oe': lo('OE')}]}],
       {'ALS': t(14, 14, V5, 'CLK to Q SN74ALS575A max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (sn74als575a switching characteristics)')})

device('74_577', 'Octal D-type edge-triggered flip-flops with synchronous clear and inverted 3-state outputs',
       'Pinout and function table: sn54als576b (SN74ALS577A, 24-pin DW / NT) p.1. CLR low at CLK up: Q = 1 '
       '(the stored bits clear; the outputs invert); OE high: Q = Z.',
       {'p24': pins(p575)},
       [{'d': seq('%dD', 8), 'clk': clk(), 'clear': {'pin': 'CLR', 'active': 'low', 'when': 'clock'},
         'outputs': [{'q': seq('%dQ', 8), 'invert': True, 'oe': lo('OE')}]}],
       {'ALS': t(14, 14, V5, 'CLK to Q SN74ALS577A max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (sn54als576b switching characteristics)')})

# ---- 24-pin bus-interface registers -----------------------------------------
p821 = I('OE') + I(*seq('%dD', 10)) + [G] + I('CLK') + O(*reversed(seq('%dQ', 10))) + [V]
device('74_821', '10-bit bus-interface flip-flops with 3-state outputs',
       'Pinout and function table: sn74abt821a p.1, sn74lvc821a, cy74fct821t (CY74FCT821T: OE, D0-D9, CP, '
       'Y0-Y9), same pin numbers. OE high: Q = Z.',
       {'p24': pins(p821)},
       [{'d': seq('%dD', 10), 'clk': clk(), 'outputs': [{'q': seq('%dQ', 10), 'oe': lo('OE')}]}],
       {'ABT': t(4.1, 4.6, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74abt821a switching characteristics)'),
        'FCT': t(10, 10, V5C, 'CP to Y max, A grade (CY74FCT821AT; B 7.5, C 6), 5 V 50 pF (no typ given); VCC '
                 '4.75-5.25 V (cy74fct821t switching characteristics)'),
        'LVC': t(7.3, 7.3, VLVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V, inputs 5.5 V '
                 '(sn74lvc821a switching characteristics)', 5.5)})

p29821 = p821
device('74_29821', '10-bit bus-interface flip-flops with 3-state outputs',
       'Pinout and function table: sn74als29821 p.1 (the 74x821 pinout). OE high: Q = Z.',
       {'p24': pins(p29821)},
       [{'d': seq('%dD', 10), 'clk': clk(), 'outputs': [{'q': seq('%dQ', 10), 'oe': lo('OE')}]}],
       {'ALS': t(10, 10, V5C, 'CLK to Q SN74ALS29821 max (no typ given), 5 V 50 pF; VCC SN74 4.75-5.25 V '
                 '(sn74als29821 switching characteristics)')})

p823 = I('OE') + I(*seq('%dD', 9)) + I('CLR') + [G] + I('CLK', 'CLKEN') + O(*reversed(seq('%dQ', 9))) + [V]
device('74_823', '9-bit bus-interface flip-flops with clock enable, clear and 3-state outputs',
       'Pinout and function table: sn74abt823 p.1, sn74lvc823a, cy74fct823t (CY74FCT823T: OE, D0-D8, CLR, CP, '
       'EN, Y0-Y8); Nexperia 74lvc823a (OE, D0-D8, MR, CP, CE, Q0-Q8, also DHVQFN24), same pin numbers. CLKEN '
       'high: hold at the clock edge; CLR low clears at once; OE high: Q = Z.',
       {'p24': pins(p823)},
       [{'d': seq('%dD', 9), 'clk': clk(), 'ce': lo('CLKEN'), 'clear': {'pin': 'CLR', 'active': 'low', 'when': 'async'},
         'outputs': [{'q': seq('%dQ', 9), 'oe': lo('OE')}]}],
       {'ABT': t(4.3, 4.4, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74abt823 switching characteristics)'),
        'FCT': t(10, 10, V5C, 'CP to Y max, A grade (CY74FCT823AT; C 6), 5 V 50 pF (no typ given); VCC '
                 '4.75-5.25 V (cy74fct823t switching characteristics)'),
        'LVC': t(8, 8, VLVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V, inputs 5.5 V '
                 '(sn74lvc823a switching characteristics)', 5.5)})

p825 = I('OE1', 'OE2') + I(*seq('D%d', 8, 0)) + I('CLR') + [G] + I('CP', 'EN') + O(*reversed(seq('Y%d', 8, 0))) + \
    I('OE3') + [V]
device('74_825', '8-bit bus-interface register with clock enable, clear and three output enables',
       'Pinout and function table: cy74fct825t (CY74FCT825T) p.1. EN high: hold at the clock edge; CLR low '
       'clears at once; any of OE1, OE2, OE3 high: Y = Z.',
       {'p24': pins(p825)},
       [{'d': seq('D%d', 8, 0), 'clk': clk('CP'), 'ce': lo('EN'), 'clear': {'pin': 'CLR', 'active': 'low', 'when': 'async'},
         'outputs': [{'q': seq('Y%d', 8, 0), 'oe': [lo('OE1'), lo('OE2'), lo('OE3')]}]}],
       {'FCT': t(10, 10, V5C, 'CP to Y max, A grade (CY74FCT825AT; C 6), 5 V 50 pF (no typ given); VCC '
                 '4.75-5.25 V (cy74fct825t switching characteristics)')})

p874 = I('1CLR', '1OE') + I(*seq('1D%d', 4)) + I(*seq('2D%d', 4)) + I('2OE') + [G] + I('2CLR', '2CLK') + \
    O(*reversed(seq('2Q%d', 4))) + O(*reversed(seq('1Q%d', 4))) + I('1CLK') + [V]
device('74_874', 'Dual 4-bit D-type edge-triggered flip-flops with clear and 3-state outputs',
       'Pinout and function table: sn54als874b (SN54/74ALS874B, SN74AS874) p.1. Each half: nCLR low clears at '
       'once; nOE high: nQ = Z.',
       {'p24': pins(p874)},
       [{'d': seq('%dD%%d' % n, 4), 'clk': clk('%dCLK' % n),
         'clear': {'pin': '%dCLR' % n, 'active': 'low', 'when': 'async'},
         'outputs': [{'q': seq('%dQ%%d' % n, 4), 'oe': lo('%dOE' % n)}]} for n in (1, 2)],
       {'ALS': t(14, 14, V5, 'CLK to Q SN74ALS874B max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (sn54als874b switching characteristics)'),
        'AS': t(8.5, 10.5, V5, 'CLK to Q SN74AS874 max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (sn54als874b switching characteristics)')})

p11374 = O('1Q', '2Q', '3Q', '4Q') + [G, G, G, G] + O('5Q', '6Q', '7Q', '8Q') + I('CLK', '8D', '7D', '6D', '5D') + \
    [V, V] + I('4D', '3D', '2D', '1D', 'OE')
device('74_11374', 'Octal edge-triggered D-type flip-flop with 3-state outputs (flow-through, center-pin power)',
       'Pinout and function table: 74act11374 p.1 (GND 5-8, VCC 18-19). OE high: Q = Z.',
       {'p24': pins(p11374)},
       [{'d': seq('%dD', 8), 'clk': clk(), 'outputs': [{'q': seq('%dQ', 8), 'oe': lo('OE')}]}],
       {'ACT': t(8.5, 8.5, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (74act11374 switching characteristics)')})
print('written', len(list(OUT.glob('*.json'))), 'specs')
