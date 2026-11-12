#!/usr/bin/env python3
# plan-36 census -> candidates.tsv: one row per listed DigiKey part (AVR, Espressif, RP2040).
# usage: python3 -I candidates.py <raw dir> <atdf dir> <out.tsv>
# Columns A..G fill later: pdf, hit, dead (A), package_code, verdict, reason (A), pinout_src (B), footprint (C-E).
import json, glob, re, sys, os, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
os.environ['ATDF'] = sys.argv[2]; os.environ['RAW'] = sys.argv[1]
import atdfmatch as A
raw, out = sys.argv[1], sys.argv[3]
atdf = A.load_atdf()
COLS = ['mpn','maker','family','device','scope','reason','atdf_device','atdf_pinout','dk_package','dk_supplier_package',
        'stock','status','grade','dk_vcc','dk_speed','dk_flash','dk_ram','dk_eeprom','datasheet_url',
        'pdf','hit','dead','package_code','verdict','pinout_src','footprint']
def family(dev):
    d = dev.upper()
    if d.startswith('ATXMEGA'): return 'AVR XMEGA'
    if d.startswith('AT90'): return 'AVR AT90'
    m = re.match(r'AVR\d+([A-Z]{2})\d+$', d)
    if m: return {'DA':'AVR Dx','DB':'AVR Dx','DD':'AVR Dx','DU':'AVR Dx','EA':'AVR Ex','EB':'AVR Ex',
                  'LA':'AVR Lx/Sx','SD':'AVR Lx/Sx'}[m.group(1)]
    if re.match(r'ATMEGA(808|809|1608|1609|3208|3209|4808|4809)$', d): return 'AVR megaAVR 0'
    if re.match(r'ATTINY(202|204|212|214|402|404|406|412|414|416|417|424|426|427|804|806|807|814|816|817|824|826|827|'
                r'1604|1606|1607|1614|1616|1617|1624|1626|1627|3216|3217|3224|3226|3227)$', d): return 'AVR tinyAVR 0/1/2'
    if d.startswith('ATMEGA'): return 'AVR ATmega'
    if d.startswith('ATTINY'): return 'AVR ATtiny'
    return '?'
ESP_IN = re.compile(r'^ESP32(-S2|-S3|-C3|-C6)?-(WROOM|WROVER|MINI|SOLO)')
def esp_reason(m):
    if ESP_IN.match(m): return ''
    if re.match(r'^ESP32(-[A-Z0-9]+)?(F[HN]\d+)?(R\d+)?$', m): return 'bare SoC (owner: modules only)'
    if 'PICO' in m: return 'PICO system-in-package / PICO module (owner: WROOM/WROVER/MINI modules only)'
    if m.startswith('ESP32-C61') or m.startswith('ESP32-H2') or m.startswith('ESP8684'):
        return 'family not in scope (owner: ESP32, S2, S3, C3, C6)'
    return 'not a WROOM/WROVER/MINI/SOLO module'
def esp_device(m):
    m2 = re.match(r'^(ESP32(?:-S2|-S3|-C3|-C6)?-(?:WROOM-32|WROOM-DA|WROOM-02|WROOM-1|WROVER|MINI-2|MINI-1|SOLO-2|SOLO))', m)
    return m2.group(1) if m2 else m
rows = []
for f in sorted(glob.glob(raw + '/*.jsonl')):
    for l in open(f):
        p = json.loads(l); P = p['params']
        r = {c: '' for c in COLS}
        r.update(mpn=p['mpn'], maker=p['maker'], dk_package=P.get('Package / Case',''),
                 dk_supplier_package=P.get('Supplier Device Package',''), stock=str(p.get('stock','')), status=p['status'] or '',
                 grade=('automotive' if 'AEC' in (P.get('Qualification') or '') or P.get('Grade') == 'Automotive' else ''),
                 dk_vcc=P.get('Voltage - Supply (Vcc/Vdd)', P.get('Voltage - Supply','')), dk_speed=P.get('Speed',''),
                 dk_flash=P.get('Program Memory Size', P.get('Memory Size','')), dk_ram=P.get('RAM Size',''),
                 dk_eeprom=P.get('EEPROM Size',''), datasheet_url=re.sub(r'^//','https://',p['datasheet'] or ''), scope='in')
        if p['maker'] == 'Espressif Systems':
            r['family'] = 'ESP32'; r['device'] = esp_device(p['mpn']); why = esp_reason(p['mpn'])
            if why: r['scope'], r['reason'] = 'out', why
        elif p['maker'] == 'Raspberry Pi':
            r['family'] = 'RP2040'; r['device'] = 'RP2040'
            if not p['mpn'].startswith('SC0914'):
                r['family'] = 'RP2350'; r['scope'], r['reason'] = 'out', 'RP2350 / RP2354 (owner: RP2040 only)'
        else:
            s = A.device(p['mpn']); hit = next((x for x in A.candidates(s) if x in atdf), None)
            dev = hit or s; r['device'] = dev; r['family'] = family(dev)
            if hit and atdf[hit][1]:
                m = A.match(atdf, hit, r['dk_package'])
                r['atdf_device'] = atdf[hit][0]; r['atdf_pinout'] = m[0] if m else ''
            elif hit: r['atdf_device'] = atdf[hit][0]
        rows.append(r)
rows.sort(key=lambda r: (r['family'], r['device'], r['mpn']))
with open(out, 'w') as o:
    o.write('\t'.join(COLS) + '\n')
    for r in rows: o.write('\t'.join(r[c].replace('\t',' ') for c in COLS) + '\n')
c = collections.Counter((r['family'], r['scope']) for r in rows)
for k in sorted(c): print(k, c[k])
print('in-scope rows with no top-level stock (A checks part by part):', sum(r['scope']=='in' and r['stock']=='' for r in rows))
print('rows', len(rows), 'in', sum(r['scope']=='in' for r in rows), 'devices in', len({r['device'] for r in rows if r['scope']=='in'}))
