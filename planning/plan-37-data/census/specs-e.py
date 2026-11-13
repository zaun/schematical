#!/usr/bin/env python3
"""plan-37-E: writes planning/plan-37-data/specs/<device>.json for the 9 wide
register devices (48-, 56-, 64-pin). Uses specs-d.py's helpers.
usage: python3 -I specs-e.py   (from the repository root)"""
import importlib.util
from pathlib import Path

spec = importlib.util.spec_from_file_location('d', str(Path(__file__).with_name('specs-d.py')))
src = Path(__file__).with_name('specs-d.py').read_text()
# the helpers only (everything before the first device)
exec(compile(src[:src.index('# ---- 74_173')], 'specs-d.py', 'exec'))

V33 = [2.7, 3.6]
VALVC = [1.65, 3.6]

# ---- 48-pin: 74_16374, 74_162374 (sn74lvc16374a p.3 pin functions) --------
p48 = []
for half in (1, 2):
    q = ['%dQ%d' % (half, i) for i in range(1, 9)]
    p48 += I('%dOE' % half) if half == 1 else []
p48 = (I('1OE') + O('1Q1', '1Q2') + [G] + O('1Q3', '1Q4') + [V] + O('1Q5', '1Q6') + [G] + O('1Q7', '1Q8') +
       O('2Q1', '2Q2') + [G] + O('2Q3', '2Q4') + [V] + O('2Q5', '2Q6') + [G] + O('2Q7', '2Q8') + I('2OE', '2CLK') +
       I('2D8', '2D7') + [G] + I('2D6', '2D5') + [V] + I('2D4', '2D3') + [G] + I('2D2', '2D1') + I('1D8', '1D7') +
       [G] + I('1D6', '1D5') + [V] + I('1D4', '1D3') + [G] + I('1D2', '1D1') + I('1CLK'))
g16374 = [{'d': seq('%dD%%d' % n, 8), 'clk': clk('%dCLK' % n),
           'outputs': [{'q': seq('%dQ%%d' % n, 8), 'oe': lo('%dOE' % n)}]} for n in (1, 2)]
NOTE48 = ('Nexperia (nOE, nCP, nDn, nQn) and onsemi (OEn, CPn, Dn, On) number the same pins. Each half: '
          'nCLK up loads nD; nOE high: nQ = Z. Bus hold (H types) and series output resistors (162xxx) are not '
          'modeled.')
device('74_16374', '16-bit edge-triggered D-type flip-flops with 3-state outputs',
       'Pinout: sn74lvc16374a p.3 (Pin Functions), the same in sn74abt16374a, sn74ahc16374, sn74ahct16374, '
       'sn74alvch16374, sn74auc16374, sn74avc16374, sn74lvth16374, 74act16374, cy74fct16374t; Nexperia '
       '74lvc_lvch16374a, 74lvt_lvth16374a; onsemi 74lcx16374. ' + NOTE48,
       {'p48': pins(p48)}, g16374,
       {'ABT': t(4.3, 4.7, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74abt16374a switching characteristics)'),
        'ACT': t(8.8, 8.8, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (74act16374 switching characteristics)'),
        'AHC': t(5.4, 5.4, [2, 5.5], 'CLK to Q typ, 5 V 15 pF; VCC 2-5.5 V, inputs 5.5 V (sn74ahc16374 switching '
                 'characteristics)', 5.5),
        'AHCT': t(6.5, 6.5, V5, 'CLK to Q typ, 5 V 15 pF; VCC 4.5-5.5 V (sn74ahct16374 switching characteristics)'),
        'ALVC': t(4.2, 4.2, VALVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V (sn74alvch16374 '
                  'switching characteristics)'),
        'AUC': t(1.5, 1.5, [0.8, 2.7], 'CLK to Q typ, 1.8 V; VCC 0.8-2.7 V, inputs 3.6 V (sn74auc16374 switching '
                 'characteristics)', 3.6),
        'AVC': t(3.3, 3.3, [1.4, 3.6], 'CLK to Q max (no typ given), 3.3 V; VCC 1.4-3.6 V (sn74avc16374 switching '
                 'characteristics)'),
        'FCT': t(5.2, 5.2, V5, 'CLK to O max, C grade (CY74FCT16374CT; A 10, plain 6.5) (no typ given), 5 V 50 pF; '
                 'VCC 4.5-5.5 V (cy74fct16374t switching characteristics)'),
        'LCX': t(6.2, 6.2, [2, 3.6], 'CP to On max (no typ given), 3.0-3.6 V 50 pF; VCC 2.0-3.6 V, inputs 5.5 V '
                 '(onsemi 74lcx16374 AC characteristics)', 5.5),
        'LVC': t(4.5, 4.5, VALVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V, inputs 5.5 V '
                 '(sn74lvc16374a switching characteristics)', 5.5),
        'LVT': t(3, 2.9, V33, 'CLK to Q typ, 3.3 V 50 pF; VCC 2.7-3.6 V, inputs 5.5 V (sn74lvth16374 switching '
                 'characteristics)', 5.5)})

device('74_162374', '16-bit edge-triggered D-type flip-flops with series-damping-resistor 3-state outputs',
       'Pinout: as 74_16374 (sn74alvch162374, sn74lvth162374, Nexperia 74lvt162374, onsemi 74lcx162374). The '
       'outputs\' series resistors are not modeled: the logic is 74_16374\'s. ' + NOTE48,
       {'p48': pins(p48)}, g16374,
       {'ALVC': t(4.6, 4.6, VALVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V (sn74alvch162374 '
                  'switching characteristics)'),
        'LCX': t(7.0, 7.0, [2, 3.6], 'CP to On max (no typ given), 3.0-3.6 V 50 pF; VCC 2.0-3.6 V, inputs 5.5 V '
                 '(onsemi 74lcx162374 AC characteristics)', 5.5),
        'LVT': t(3.4, 3.3, V33, 'CLK to Q typ, 3.3 V 50 pF; VCC 2.7-3.6 V, inputs 5.5 V (sn74lvth162374 switching '
                 'characteristics)', 5.5)})

# ---- 56-pin, two 10-bit groups: 74_16821 (sn74abt16821 p.1) ----------------
p16821 = (I('1OE') + O('1Q1', '1Q2') + [G] + O('1Q3', '1Q4') + [V] + O('1Q5', '1Q6', '1Q7') + [G] +
          O('1Q8', '1Q9', '1Q10', '2Q1', '2Q2', '2Q3') + [G] + O('2Q4', '2Q5', '2Q6') + [V] + O('2Q7', '2Q8') + [G] +
          O('2Q9', '2Q10') + I('2OE', '2CLK', '2D10', '2D9') + [G] + I('2D8', '2D7') + [V] + I('2D6', '2D5', '2D4') +
          [G] + I('2D3', '2D2', '2D1', '1D10', '1D9', '1D8') + [G] + I('1D7', '1D6', '1D5') + [V] + I('1D4', '1D3') +
          [G] + I('1D2', '1D1', '1CLK'))
device('74_16821', '20-bit bus-interface flip-flops with 3-state outputs',
       'Pinout: sn74abt16821 p.1, the same in sn74alvch16821. Each half: nCLK up loads the ten nD; nOE high: nQ = Z.',
       {'p56': pins(p16821)},
       [{'d': seq('%dD%%d' % n, 10), 'clk': clk('%dCLK' % n),
         'outputs': [{'q': seq('%dQ%%d' % n, 10), 'oe': lo('%dOE' % n)}]} for n in (1, 2)],
       {'ABT': t(3.7, 3.9, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74abt16821 switching characteristics)'),
        'ALVC': t(4.5, 4.5, VALVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V (sn74alvch16821 '
                  'switching characteristics)')})

# ---- 56-pin, two 9-bit groups with CLR and CLKEN: 74_16823, 74_162823 -------
p16823 = (I('1CLR', '1OE') + O('1Q1') + [G] + O('1Q2', '1Q3') + [V] + O('1Q4', '1Q5', '1Q6') + [G] +
          O('1Q7', '1Q8', '1Q9', '2Q1', '2Q2', '2Q3') + [G] + O('2Q4', '2Q5', '2Q6') + [V] + O('2Q7', '2Q8') + [G] +
          O('2Q9') + I('2OE', '2CLR', '2CLK', '2CLKEN', '2D9') + [G] + I('2D8', '2D7') + [V] +
          I('2D6', '2D5', '2D4') + [G] + I('2D3', '2D2', '2D1', '1D9', '1D8', '1D7') + [G] + I('1D6', '1D5', '1D4') +
          [V] + I('1D3', '1D2') + [G] + I('1D1', '1CLKEN', '1CLK'))
g16823 = [{'d': seq('%dD%%d' % n, 9), 'clk': clk('%dCLK' % n), 'ce': lo('%dCLKEN' % n),
           'clear': {'pin': '%dCLR' % n, 'active': 'low', 'when': 'async'},
           'outputs': [{'q': seq('%dQ%%d' % n, 9), 'oe': lo('%dOE' % n)}]} for n in (1, 2)]
NOTE56 = ('Each half: nCLKEN high: hold at the clock edge; nCLR low clears at once; nOE high: nQ = Z. Nexperia '
          '(nOE, nMR, nCP, nCE, nDn, nQn) numbers the same pins.')
device('74_16823', '18-bit bus-interface flip-flops with clock enable, clear and 3-state outputs',
       'Pinout: sn74abt16823 p.1, the same in sn74abth16823, sn74alvch16823, 74act16823; Nexperia 74alvt16823. ' +
       NOTE56,
       {'p56': pins(p16823)}, g16823,
       {'ABT': t(3.9, 3.9, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74abt16823 switching characteristics)'),
        'ACT': t(7.5, 8.3, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (74act16823 switching characteristics)'),
        'ALVC': t(4.5, 4.5, VALVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V (sn74alvch16823 '
                  'switching characteristics)'),
        'ALVT': t(2.3, 2.1, [2.3, 3.6], 'nCP to nQn typ, 3.3 V; VCC 2.3-3.6 V, inputs 5.5 V (Nexperia 74alvt16823 '
                  'dynamic characteristics)', 5.5)})
device('74_162823', '18-bit bus-interface flip-flops with clock enable, clear and series-damping-resistor 3-state outputs',
       'Pinout: sn74abt162823a p.1 (as 74_16823). The outputs\' series resistors are not modeled. ' + NOTE56,
       {'p56': pins(p16823)}, g16823,
       {'ABT': t(4.6, 4.6, V5, 'CLK to Q typ, 5 V 50 pF; VCC 4.5-5.5 V (sn74abt162823a switching characteristics)')})

# ---- 56-pin, one 20-bit group with CLKEN: 74_16721, 74_162721 --------------
p16721 = (I('OE') + O('Q1', 'Q2') + [G] + O('Q3', 'Q4') + [V] + O('Q5', 'Q6', 'Q7') + [G] +
          O('Q8', 'Q9', 'Q10', 'Q11', 'Q12', 'Q13') + [G] + O('Q14', 'Q15', 'Q16') + [V] + O('Q17', 'Q18') + [G] +
          O('Q19', 'Q20') + [NC] + I('CLKEN', 'D20', 'D19') + [G] + I('D18', 'D17') + [V] + I('D16', 'D15', 'D14') +
          [G] + I('D13', 'D12', 'D11', 'D10', 'D9', 'D8') + [G] + I('D7', 'D6', 'D5') + [V] + I('D4', 'D3') + [G] +
          I('D2', 'D1', 'CLK'))
g16721 = [{'d': seq('D%d', 20), 'clk': clk(), 'ce': lo('CLKEN'), 'outputs': [{'q': seq('Q%d', 20), 'oe': lo('OE')}]}]
device('74_16721', '20-bit flip-flops with clock enable and 3-state outputs',
       'Pinout: sn74alvch16721 p.1 (NC 28). CLKEN high: hold at the clock edge; OE high: Q = Z.',
       {'p56': pins(p16721)}, g16721,
       {'ALVC': t(4.3, 4.3, VALVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V (sn74alvch16721 '
                  'switching characteristics)')})
device('74_162721', '20-bit flip-flops with clock enable and series-damping-resistor 3-state outputs',
       'Pinout: sn74alvch162721 p.1 (as 74_16721). The outputs\' series resistors are not modeled. CLKEN high: '
       'hold at the clock edge; OE high: Q = Z.',
       {'p56': pins(p16721)}, g16721,
       {'ALVC': t(5.3, 5.3, VALVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V (sn74alvch162721 '
                  'switching characteristics)')})

# ---- 56-pin, 10 bits with dual outputs: 74_16820 (sn74alvch16820 p.1) ------
p16820 = (I('1OE') + O('1Q1', '1Q2') + [G] + O('2Q1', '2Q2') + [V] + O('3Q1', '3Q2', '4Q1') + [G] +
          O('4Q2', '5Q1', '5Q2', '6Q1', '6Q2', '7Q1') + [G] + O('7Q2', '8Q1', '8Q2') + [V] + O('9Q1', '9Q2') + [G] +
          O('10Q1', '10Q2') + I('2OE') + [NC, NC] + I('D10') + [G, NC] + I('D9') + [V, NC] + I('D8') + [NC, G] +
          I('D7') + [NC] + I('D6') + [NC] + I('D5') + [NC, G] + I('D4') + [NC] + I('D3') + [V, NC] + I('D2') +
          [G, NC] + I('D1', 'CLK'))
device('74_16820', '10-bit flip-flops with dual 3-state outputs',
       'Pinout: sn74alvch16820 p.1. One D per bit, two outputs: nQ1 (enabled by 1OE) and nQ2 (by 2OE), '
       'n = 1-10. CLK up loads D; each OE high puts its outputs in Z.',
       {'p56': pins(p16820)},
       [{'d': seq('D%d', 10), 'clk': clk(),
         'outputs': [{'q': seq('%dQ1', 10), 'oe': lo('1OE')}, {'q': seq('%dQ2', 10), 'oe': lo('2OE')}]}],
       {'ALVC': t(4.8, 4.8, VALVC, 'CLK to Q max (no typ given), 3.3 V 50 pF; VCC 1.65-3.6 V (sn74alvch16820 '
                  'switching characteristics)')})

# ---- 64-pin, one 22-bit group with CLKEN: 74_16722 (sn74avc16722 p.1) ------
p16722 = (I('OE') + O('Q1', 'Q2') + [G] + O('Q3', 'Q4') + [V] + O('Q5', 'Q6', 'Q7') + [G] +
          O('Q8', 'Q9', 'Q10', 'Q11', 'Q12', 'Q13') + [G] + O('Q14', 'Q15', 'Q16') + [V] + O('Q17', 'Q18') + [G] +
          O('Q19', 'Q20') + [V] + O('Q21', 'Q22') + [G, NC] + I('CLKEN') + [G] + I('D22', 'D21') + [V] +
          I('D20', 'D19') + [G] + I('D18', 'D17') + [V] + I('D16', 'D15', 'D14') + [G] +
          I('D13', 'D12', 'D11', 'D10', 'D9', 'D8') + [G] + I('D7', 'D6', 'D5') + [V] + I('D4', 'D3') + [G] +
          I('D2', 'D1', 'CLK'))
device('74_16722', '22-bit flip-flops with clock enable and 3-state outputs',
       'Pinout: sn74avc16722 p.1 (NC 32). CLKEN high: hold at the clock edge; OE high: Q = Z.',
       {'p64': pins(p16722)},
       [{'d': seq('D%d', 22), 'clk': clk(), 'ce': lo('CLKEN'), 'outputs': [{'q': seq('Q%d', 22), 'oe': lo('OE')}]}],
       {'AVC': t(2.6, 2.6, [1.4, 3.6], 'CLK to Q max (no typ given), 3.3 V; VCC 1.4-3.6 V (sn74avc16722 switching '
                 'characteristics)')})
print('specs written')
