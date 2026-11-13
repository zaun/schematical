#!/usr/bin/env python3
"""plan-37-F: writes planning/plan-37-data/specs/dff-additions.json, the
timing entries and note text the onsemi and Nexperia parts add to the six
dff register devices. Values: CLK (CP) to Q typ at the family's nominal
supply (max where the sheet gives no typ), from the PDFs named in each note.
usage: python3 -I specs-f.py   (from the repository root)"""
import json


def e(rise, fall, vcc, note, input_max=None):
    d = {'rise': rise, 'fall': fall, 'clearWins': False, 'vcc': vcc}
    if input_max:
        d['inputMax'] = input_max
    d['note'] = note
    return d


V5, VAC, VHC = [4.5, 5.5], [2, 6], [2, 5.5]
LVC = ([1.65, 3.6], 5.5)
add = {
    '74_174': {
        'note': ('plan-37-F: Nexperia 74HC174 / 74HCT174 (74hc_hct174 Table 2: MR, CP, D0-D5, Q0-Q5) and onsemi '
                 'MC74HC174A (mc74hc174a-d: Reset, Clock, D0-D5, Q0-Q5) use these pin numbers.'),
        'timing': {}},
    '74_175': {
        'note': ('plan-37-F: Nexperia 74HC175 / 74HCT175 (74hc_hct175 Table 2: MR, CP, D0-D3, Q0-Q3, Q0-Q3 '
                 'barred) and onsemi MC74HC175A, MM74HC175, 74VHC175 use these pin numbers.'),
        'timing': {
            'VHC': e(4.8, 4.8, VHC, 'CP to Qn typ, 5 V 15 pF; VCC 2.0-5.5 V (onsemi 74vhc175-d AC characteristics)'),
        }},
    '74_273': {
        'note': ('plan-37-F: Nexperia 74HC273, 74HCT273, 74AHC273, 74AHCT273, 74LVC273 (MR, CP, D0-D7, Q0-Q7) '
                 'and onsemi MC74HC273A, MM74HC273, MM74HCT273, MC74AC273, MC74ACT273, 74VHC273, 74LVX273 use '
                 'these pin numbers; Nexperia DHVQFN20 parts through the DHVQFN-20 footprint\'s slots.'),
        'timing': {
            'LVC': e(4.1, 4.1, LVC[0], 'CP to Qn typ, 3.0-3.6 V; VCC 1.65-3.6 V, inputs 5.5 V (Nexperia 74lvc273 '
                     'dynamic characteristics)', LVC[1]),
            'LVX': e(7.1, 7.1, [2, 3.6], 'CP to Qn typ, 3.3 V 15 pF; VCC 2.0-3.6 V, inputs 5.5 V (onsemi 74lvx273-d '
                     'AC characteristics)', 5.5),
            'VHC': e(5.8, 5.8, VHC, 'CK to Q typ, 5 V 15 pF; VCC 2.0-5.5 V (onsemi 74vhc273-d AC characteristics)'),
        }},
    '74_374': {
        'note': ('plan-37-F: Nexperia 74HC374, 74HCT374, 74AHC374, 74AHCT374, 74LVC374A (OE, CP, D0-D7, Q0-Q7) '
                 'and onsemi MC74HC374A, MM74HC374, MM74HCT374, MC74AC374, MC74ACT374, MC74LCX374, 74LCX374, '
                 'MC74LVX374, MC74VHC374, 74VHC374, 74VHCT374A (OE, CP, D0-D7, O0-O7 / Q0-Q7) use these pin '
                 'numbers; 74LCX374BQX through the DQFN-20-ONSEMI footprint\'s slots.'),
        'timing': {
            'AC': e(8.0, 7.0, VAC, 'CP to On typ, 5 V 50 pF; VCC 2-6 V (onsemi mc74ac374-d AC characteristics)'),
            'ACT': e(8.5, 8.0, V5, 'CP to On typ, 5 V 50 pF; VCC 4.5-5.5 V (onsemi mc74ac374-d 74ACT AC '
                     'characteristics)'),
            'AHC': e(4.4, 4.4, VHC, 'CP to Qn typ, 5 V 15 pF; VCC 2.0-5.5 V (Nexperia 74ahc_ahct374 dynamic '
                     'characteristics)'),
            'LCX': e(8.5, 8.5, [2, 3.6], 'CP to On max (no typ given), 3.0-3.6 V 50 pF; VCC 2.0-3.6 V, inputs 5.5 V '
                     '(onsemi 74lcx374-d AC characteristics)', 5.5),
            'LVC': e(3.3, 3.3, LVC[0], 'CP to Qn typ, 3.0-3.6 V; VCC 1.65-3.6 V, inputs 5.5 V (Nexperia 74lvc374a '
                     'dynamic characteristics)', LVC[1]),
            'LVX': e(6.7, 6.7, [2, 3.6], 'CP to O typ, 3.3 V 15 pF; VCC 2.0-3.6 V, inputs 5.5 V (onsemi mc74lvx374-d '
                     'AC characteristics)', 5.5),
            'VHC': e(5.4, 5.4, VHC, 'CP to On typ, 5 V 15 pF; VCC 2.0-5.5 V (onsemi 74vhc374-d AC characteristics)'),
            'VHCT': e(4.1, 4.1, V5, 'CP to On typ, 5 V 15 pF; VCC 4.5-5.5 V (onsemi 74vhct374a-d AC '
                      'characteristics)'),
        }},
    '74_574': {
        'note': ('plan-37-F: Nexperia 74HC574, 74HCT574, 74AHC574, 74AHCT574, 74ALVC574, 74LVC574A (OE, CP, '
                 'D0-D7, Q0-Q7) and onsemi MC74HC574A, MM74HC574, MM74HCT574, MC74AC574, MC74ACT574, MC74LCX574, '
                 'MC74LVX574, MC74VHC574, 74VHC574, 74VHCT574A (OE, CP, D0-D7, O0-O7 / Q0-Q7) use these pin '
                 'numbers; Nexperia DHVQFN20 parts through the DHVQFN-20 footprint\'s slots.'),
        'timing': {
            'ACT': e(11, 10, V5, 'CP to On max (no typ given), 5 V 50 pF; VCC 4.5-5.5 V (onsemi mc74ac574-d 74ACT AC '
                     'characteristics)'),
            'AHC': e(4.4, 4.4, VHC, 'CP to Qn typ, 5 V 15 pF; VCC 2.0-5.5 V (Nexperia 74ahc_ahct574 dynamic '
                     'characteristics)'),
            'AHCT': e(4.4, 4.4, V5, 'CP to Qn typ, 5 V 15 pF; VCC 4.5-5.5 V (Nexperia 74ahc_ahct574 dynamic '
                      'characteristics)'),
            'ALVC': e(2.5, 2.5, [1.65, 3.6], 'CP to Qn typ, 3.0-3.6 V; VCC 1.65-3.6 V (Nexperia 74alvc574 dynamic '
                      'characteristics)'),
            'LCX': e(8.5, 8.5, [1.65, 5.5], 'CP to On max (no typ given), 3.0-3.6 V; VCC 1.65-5.5 V, inputs 5.5 V '
                     '(onsemi mc74lcx574-d AC characteristics)', 5.5),
            'LVC': e(3.5, 3.5, LVC[0], 'CP to Qn typ, 3.0-3.6 V; VCC 1.65-3.6 V, inputs 5.5 V (Nexperia 74lvc574a '
                     'dynamic characteristics)', LVC[1]),
            'LVX': e(8.5, 8.5, [2, 3.6], 'CP to O typ, 3.3 V 15 pF; VCC 2.0-3.6 V, inputs 5.5 V (onsemi mc74lvx574-d '
                     'AC characteristics)', 5.5),
            'VHC': e(5.6, 5.6, VHC, 'CP to On typ, 5 V 15 pF; VCC 2.0-5.5 V (onsemi 74vhc574-d AC characteristics)'),
            'VHCT': e(4.1, 4.1, V5, 'CP to On typ, 5 V 15 pF; VCC 4.5-5.5 V (onsemi 74vhct574a-d AC '
                      'characteristics)'),
        }},
    '40175B': {
        'note': ('plan-37-F: Nexperia HEF40175B (hef40175b: MR, CP, D0-D3, Q0-Q3, Q0-Q3 barred) and onsemi '
                 'MC14175B (mc14175b-d: R, C, D0-D3, Q0-Q3, Q0-Q3 barred) use these pin numbers.'),
        'timing': {}},
}
json.dump(add, open('planning/plan-37-data/specs/dff-additions.json', 'w'), indent=1, ensure_ascii=False)
print(sum(len(v['timing']) for v in add.values()), 'timing entries')
