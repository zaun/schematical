#!/usr/bin/env python3
# plan-37 census: the footprint each in-scope candidate takes, and the
# footprints-needed.txt summary.
# usage: python3 -I footprints.py <candidates.tsv> <footprints.json> <footprints-needed.txt>
# Fills the `footprint` column of candidates.tsv in place (in-scope rows only)
# and writes one line per (footprint id, maker, DigiKey package) with its part
# count; a footprint id not in footprints.json is marked NEW.
#
# The mapping is by package outline. Nexperia (SOT109-1, SOT163-1, SOT338-1,
# SOT339-1, SOT403-1, SOT360-1, SOT370-1, SOT362-1, SOT371-1, SOT364-1) and
# onsemi (751B, 751D, 948F, 948E, 738, 966/967 SOEIAJ, MTD48) gull-wing and DIP
# outlines are the same JEDEC / JEITA outlines as the TI packages the existing
# ids were drawn from (MS-012, MS-013, MO-150, MO-153, MO-118, MS-001, EIAJ
# type II), so they reuse them. Leadless packages (Nexperia DHVQFN, onsemi DQFN)
# and the TI packages no existing id covers (NT 24-pin 300 mil PDIP, DGG 64)
# get a new footprint, drawn in plan-37-B from the maker's package drawing.
import csv, json, sys
from collections import Counter

cand_path, fp_path, out_path = sys.argv[1:4]

# (DigiKey "Package / Case" prefix, supplier package substring or '') -> id
BY_PACKAGE = [
    ('16-SOIC (0.154"', '', 'SOIC-16'),
    ('16-SOIC (0.209"', '', 'SOP-16'),
    ('16-SSOP (0.209"', '', 'SSOP-16'),
    ('16-TSSOP', '', 'TSSOP-16'),
    ('16-DIP', '', 'PDIP-16'),
    ('16-CDIP', '', 'CDIP-16'),
    ('20-SOIC (0.295"', '', 'SOIC-20W'),
    ('20-SOIC (0.209"', '', 'SOP-20'),
    ('20-SSOP (0.209"', '', 'SSOP-20'),
    ('20-SSOP (0.154"', '', 'QSOP-20'),
    ('20-TSSOP', '', 'TSSOP-20'),
    ('20-DIP', '', 'PDIP-20'),
    ('20-CLCC', '', 'LCCC-20'),
    ('20-VFQFN Exposed Pad', 'DHVQFN', 'DHVQFN-20'),
    ('20-WFQFN Exposed Pad', 'DQFN', 'DQFN-20-ONSEMI'),
    ('24-SOIC (0.295"', '', 'SOIC-24W'),
    ('24-SSOP (0.209"', '', 'SSOP-24'),
    ('24-SSOP (0.154"', '', 'QSOP-24'),
    ('24-TFSOP (0.173"', '', 'TVSOP-24'),
    ('24-TSSOP', '', 'TSSOP-24'),
    ('24-CDIP (0.300"', '', 'CDIP-24'),
    ('24-VFQFN Exposed Pad', 'DHVQFN', 'DHVQFN-24'),
    ('48-BSSOP', '', 'SSOP-48'),
    ('48-TFSOP (0.240"', '', 'TSSOP-48'),
    ('48-TFSOP (0.173"', '', 'TVSOP-48'),
    ('56-BSSOP', '', 'SSOP-56'),
    ('56-TFSOP (0.240"', '', 'TSSOP-56'),
    ('56-TFSOP (0.173"', '', 'TVSOP-56'),
    ('64-TFSOP (0.240"', '', 'TSSOP-64'),
]

# Rows DigiKey lists without parameters: the package from the part number's
# package code (TI: N, NS, DB, DW, PW, J, FK, W, D; SMD "E"/"R"/"2A" case
# letters; onsemi: D, DW, DT, MEL, SJ, MTC, MTD).
BY_NAME = {
    '5962-8682501EA': 'CDIP-16', '5962-87807012A': 'LCCC-20', '5962-8976901RA': 'CDIP-20',
    '5962-8992501EA': 'CDIP-16', '5962-9314801Q2A': 'LCCC-20', '5962-9314801QRA': 'CDIP-20',
    '8400102RA': 'CDIP-20', 'CD40174BF3A': 'CDIP-16', 'CD4076BF': 'CDIP-16',
    'CD54HC377F3A': 'CDIP-20', 'JM38510/32504BRA': 'CDIP-20', 'JM38510/32504BSA': 'CFP-20',
    'SN54HC534J': 'CDIP-20', 'SN74173N': 'PDIP-16', 'SN74ABT377ANS': 'SOP-20',
    'SN74ABT377N': 'PDIP-20', 'SN74ABT534ADB': 'SSOP-20', 'SN74ABT534ADBLE': 'SSOP-20',
    'SN74ABT534APWLE': 'TSSOP-20', 'SN74ABT534DWR': 'SOIC-20W', 'SN74AC534NS': 'SOP-20',
    'SN74ACT534NS': 'SOP-20', 'SN74ALS534ADB': 'SSOP-20', 'SN74ALS576BNS': 'SOP-20',
    'SN74F378DR': 'SOIC-16', 'SN74F534ADWR': 'SOIC-20W', 'SN74F534AN': 'PDIP-20',
    'SN74LS377NS': 'SOP-20', 'SN74LS378NS': 'SOP-16', 'SN74LS379N': 'PDIP-16',
    'SNJ54ALS564BJ': 'CDIP-20', 'SNJ54ALS576BJ': 'CDIP-20', 'SNJ54F534FK': 'LCCC-20',
    'SNJ54HC377J': 'CDIP-20', 'SNJ54LS173AFK': 'LCCC-20', 'SNJ54LS377FK': 'LCCC-20',
    'SNJ54LS378W': 'CFP-16', 'SNJ54LS379FK': 'LCCC-20', 'SNJ54LS379W': 'CFP-16',
    '74LCX162374MTDX': 'TSSOP-48', '74LCX374MTC': 'TSSOP-20', '74VHC574SJX': 'SOP-20',
    'MC74AC174D': 'SOIC-16', 'MC74AC273DT': 'TSSOP-20', 'MC74ACT273DT': 'TSSOP-20',
    'MC74ACT273MELG': 'SOP-20', 'MC74ACT574DT': 'TSSOP-20', 'MC74F378D': 'SOIC-16',
    'MC74LCX374DW': 'SOIC-20W', 'MC74VHC574MEL': 'SOP-20', 'MC74VHCT574ADWR2G': 'SOIC-20W',
}


def footprint(row):
    if row['mpn'] in BY_NAME:
        return BY_NAME[row['mpn']]
    for prefix, supplier, fid in BY_PACKAGE:
        if row['dk_package'].startswith(prefix) and supplier in row['dk_supplier_package']:
            return fid
    raise SystemExit(f"footprints.py: no footprint for {row['mpn']} ({row['dk_package']!r}, "
                     f"{row['dk_supplier_package']!r})")


known = {f['id'] for f in json.load(open(fp_path))['footprints']}
with open(cand_path) as f:
    reader = csv.DictReader(f, delimiter='\t')
    cols, rows = reader.fieldnames, list(reader)
lines = Counter()
for row in rows:
    if row['scope'] != 'in':
        continue
    row['footprint'] = footprint(row)
    lines[(row['footprint'], row['maker'], row['dk_package'] or '(no DigiKey parameters)')] += 1
with open(cand_path, 'w', newline='') as f:
    w = csv.DictWriter(f, fieldnames=cols, delimiter='\t', lineterminator='\n')
    w.writeheader()
    w.writerows(rows)
with open(out_path, 'w') as out:
    out.write('# plan-37: in-scope parts per (footprint id, maker, DigiKey package). '
              'NEW = not in Parts/footprints.json (plan-37-B adds it).\n')
    for (fid, maker, pkg), n in sorted(lines.items()):
        out.write(f"{fid}{'  NEW' if fid not in known else ''}\t{maker}\t{pkg}\t{n}\n")
