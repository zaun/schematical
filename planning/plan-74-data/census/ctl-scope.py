# plan-74 planning: the in-scope single-port Ethernet controller rows of the planning census
# (planning/plan-74-data/raw/ethc2.json holds all 74 rows of DigiKey category 753, In Stock + Active,
# Protocol Ethernet, Function Controller / MAC Controller / Physical Layer Controller, host bus
# Parallel / SPI / SPI+Parallel / I2C+SPI / Bus). Rules (plan-74-A § Decisions): makers Microchip,
# WIZnet, Renesas, onsemi; DigiKey's own In Stock line above 0; EtherCAT slave controllers
# (LAN9252/LAN9253/LAN9254: two or three ports) out; onsemi NCN26010 (10BASE-T1S, single pair) out.
# usage: python3 -I census/ctl-scope.py planning/plan-74-data/raw/ethc2.json
import json, sys, collections
MAKERS = {'Microchip Technology': 'microchip', 'WIZnet': 'wiznet',
          'Renesas Electronics Corporation': 'renesas', 'onsemi': 'onsemi'}
d = json.load(open(sys.argv[1]))
n = collections.Counter(); out = collections.Counter()
for pn, maker, qty in d['firstPage']:
    stock = [int(q.replace(',', '')) for label, q in qty if label == 'In Stock' and q]
    if maker not in MAKERS or not stock or stock[0] <= 0:
        out['not a maker of the pick / no DigiKey stock'] += 1
    elif pn.startswith(('LAN9252', 'LAN9253', 'LAN9254')):
        out['EtherCAT (2-3 ports)'] += 1
    elif pn.startswith('NCN26010'):
        out['single pair (10BASE-T1S)'] += 1
    else:
        n[MAKERS[maker]] += 1
print('rows', len(d['firstPage']), 'in', sum(n.values()), dict(n), 'out', dict(out))
