#!/usr/bin/env python3
"""The census of plan-43: every reference-board part with a datasheet and
DigiKey stock, one row per distinct part number, with its family and where
it goes.

usage: python3 -I census/classify.py <datasheets-log.tsv>   (from planning/plan-43-data)

Input: the lookup log of the reference boards' missing parts
(routing-example/datasheets-log.tsv, not in the repository; its rows with
result `downloaded` or `already have`).  Output: candidates.tsv (columns
below) and the counts the plan quotes.

where = `plan-43` (fits a kind the library has; listed in census/fit.tsv),
        `skipped` (fits, but has no maker-site datasheet; census/skip.tsv),
        `plan-42` (a regulator), or `later` (its family needs a new kind or
        a kind extension; plan-43-A § 5 names the family's plan).
"""
import collections, csv, os, sys
HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# family by the folder the datasheet was filed in (the DigiKey leaf category)
FOLDER = {
    'regulator-linear': 'regulator', 'regulator-switching': 'regulator',
    'dc-dc-converters': 'power-module', 'ac-dc-converters': 'power-module',
    'inductor': 'inductor', 'ferrite-bead': 'inductor', 'fuse': 'fuse',
    'osc': 'crystal', 'ceramic-capacitors': 'capacitor',
    'instrumentation-op-amps-buffer-amps': 'amplifier', 'comparator': 'amplifier',
    'audio-amplifiers': 'amplifier', 'codecs': 'amplifier',
    'eeprom': 'serial-memory', 'memory': 'serial-memory',
    'drivers-receivers-transceivers': 'transceiver', 'digital-isolators': 'transceiver',
    'translator': 'bus-switch', 'analog-switches-multiplexers-demultiplex': 'bus-switch',
    'analog-switches-special-purpose': 'bus-switch', 'signal-switches-multiplexers-decoders': 'bus-switch',
    'signal-buffers-repeaters-splitters': 'bus-switch', 'hot-swap-controllers': 'bus-switch',
    'specialized': 'bus-switch',
    'battery-charger': 'power-management', 'battery-management': 'power-management',
    'power-distribution-switches-load-drivers': 'power-management', 'supervisors': 'power-management',
    'power-over-ethernet-poe-controllers': 'power-management', 'power-management-specialized': 'power-management',
    'ambient-light-ir-uv-sensors': 'sensor', 'analog-and-digital-output': 'sensor',
    'angle-linear-position-measuring': 'sensor', 'gas-sensors': 'sensor',
    'sensor-and-detector-interfaces': 'sensor', 'energy-metering': 'sensor',
    'thermal-management': 'sensor', 'touch-screen-controllers': 'sensor',
    'led-drivers': 'driver', 'motor-drivers-controllers': 'driver', 'i-o-expanders': 'driver',
    'mcu': 'mcu', 'microprocessors': 'mcu', 'fpgas-field-programmable-gate-array': 'mcu',
    'embedded-complex-logic-fpga-cpld-evaluat': 'mcu', 'rf-transceiver-ics': 'mcu',
    'specialized-ics': 'driver',
    'connector-header': 'connector', 'connector-usb': 'connector', 'connector-barrel': 'connector',
    'coaxial-connector-rf-assemblies': 'connector', 'headers-plugs-and-sockets': 'connector',
    'modular-ethernet-connector-rj45-jacks-wi': 'connector', 'accessories': 'connector',
    'spring-loaded': 'connector',
    'balun': 'rf-passive', 'rf-antennas': 'rf-passive', 'saw-filters': 'rf-passive',
    'rf-front-end-lna-pa': 'rf-passive',
    'switch': 'electromechanical', 'encoders': 'electromechanical', 'trimmer-potentiometers': 'electromechanical',
    'speakers': 'electromechanical', 'motors-ac-dc': 'electromechanical',
    'lcd-oled-graphic': 'display', 'led-character-and-numeric': 'display', 'optoelectronics-accessories': 'display',
    'switches-solid-state': 'optocoupler',
    'diode': 'protection', 'transistor': 'driver',
    'adc': 'converter', 'dac': 'converter', 'rtc': 'rtc', 'buffer': 'buffer',
    'buffers-drivers-receivers-transceivers': 'buffer',
}
# part numbers whose family is not their folder's
MPN = {
    'MAX40200AUK+T': 'power-management', 'VBPW34FASR': 'sensor', 'VO617A-9X017T': 'optocoupler',
    'LTW-482DS5': 'display', 'ATECC508A-SSHDA-T': 'serial-memory', 'AP1511B': 'regulator',
}

def main():
    fit = {r['mpn']: r['kind'] for r in csv.DictReader(open(os.path.join(HERE, 'census/fit.tsv')), delimiter='\t')}
    skip = {r['mpn']: r for r in csv.DictReader(open(os.path.join(HERE, 'census/skip.tsv')), delimiter='\t')}
    rename = {r['listed']: r['mpn'] for r in csv.DictReader(open(os.path.join(HERE, 'census/rename.tsv')), delimiter='\t')}
    seen, out = {}, []
    for r in csv.DictReader(open(sys.argv[1]), delimiter='\t'):
        if r['result'] not in ('downloaded', 'already have'): continue
        mpn = rename.get(r['mpn'], r['mpn'])
        if mpn in seen:
            seen[mpn][7] = ', '.join(sorted(set(seen[mpn][7].split(', ') + [r['value']]))); continue
        folder = r['path'].split('/')[1]
        if mpn in fit: family, where = fit[mpn], 'plan-43'
        elif mpn in skip: family, where = skip[mpn]['kind'], 'skipped'
        else:
            family = MPN.get(mpn) or FOLDER[folder]
            where = 'plan-42' if family == 'regulator' else 'later'
        row = [family, where, mpn, r['manufacturer'], r['stock'], r['status'], r['path'], r['value'], r['category']]
        seen[mpn] = row; out.append(row)
    out.sort(key=lambda o: (o[1] != 'plan-43', o[1], o[0], o[3], o[2]))
    w = csv.writer(open(os.path.join(HERE, 'candidates.tsv'), 'w'), delimiter='\t', lineterminator='\n')
    w.writerow(['family', 'where', 'mpn', 'maker', 'stock', 'status', 'pdf', 'board_values', 'digikey_category'])
    w.writerows(out)
    print(len(out), 'parts')
    for where, n in sorted(collections.Counter(o[1] for o in out).items()): print(' ', where, n)
    for (where, fam), n in sorted(collections.Counter((o[1], o[0]) for o in out).items()): print('   ', where, fam, n)
    missing = set(fit) - {o[2] for o in out}
    if missing: print('fit.tsv rows not in the log:', sorted(missing))

main()
