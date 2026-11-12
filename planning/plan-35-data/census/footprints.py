# plan-35 footprints-needed.txt builder. usage: footprints.py ../candidates.tsv footprint-groups.tsv > ../footprints-needed.txt
# HDR: the header layout seen on each group's drawing (70-110 dpi renders, 2026-11-11).
import csv,sys
HDR={
'LCM-FOCUS-C204A-BW':'1x16 top-left','LCM-FOCUS-C204ABB':'1x16 (drawing rotated 90 deg)','LCM-FOCUS-C204ADB':'1x16 top-left',
'LCM-FOCUS-C204ALB-98':'1x16 top-left','LCM-FOCUS-C204AXB':'1x16 top','LCM-FOCUS-C204BLB-146':'1x16 top-left',
'LCM-FOCUS-C204DLB':'1x16 top-left','LCM-FOCUS-C204ALB-155':'1x16 top-left','LCM-FOCUS-C204BLB-155':'1x16 top-left',
'LCM-FOCUS-C204CXB':'1x16 top-left','LCM-FOCUS-C204HLB':'1x20 bottom','LCM-FOCUS-C402ALB-285':'2x8 left (15-16 NC)',
'LCM-FOCUS-C402A3':'2x8 left','LCM-FOCUS-C402ALB':'2x8 left','LCM-FOCUS-C402ASB':'2x8 left (mechanical diagram p.2)',
'LCM-FOCUS-C404ALB':'2x9 left','LCM-MIDAS-MC42004':'1x16 (drawing rotated 90 deg)','LCM-MIDAS-MC42004-SPR':'1x16 (rotated)',
'LCM-MIDAS-MD42004':'1x18 (rotated)','LCM-MIDAS-MC42005':'1x16 (rotated)','LCM-MIDAS-MD42005C':'1x18 (rotated)',
'LCM-MIDAS-MC42008':'1x16 (rotated)','LCM-MIDAS-MD24005':'2x7 interface plus 4 backlight pads A, K(R), K(G), K(B) (rotated)',
'LCM-MIDAS-MC44005':'2x9 (rotated)','LCM-MIDAS-MC44005-SPTLY':'2x9 (rotated)','LCM-MIDAS-MD44005':'2x10 (rotated)',
'LCM-NHD-0420AZ':'1x16 top-left','LCM-NHD-0420DZ':'1x16 top-left','LCM-NHD-0420E2Z':'1x16 top-left','LCM-NHD-0420E2Z-NSW-BBW':'1x18 top-left (17-18 NC, unnumbered pads)',
'LCM-NHD-0420H1Z':'two 1x8 columns: pins 1-8 left edge, 9-16 right edge','LCM-NHD-0240AZ':'2x8 left','LCM-NHD-0440AZ':'2x9 left','LCM-NHD-0440WH':'2x9 left'}
rows=[r for r in csv.DictReader(open(sys.argv[1]),delimiter='\t') if r['verdict']=='add']
groups={}
for r in rows: groups.setdefault(r['footprint'],[]).append(r)
order=[l.split('\t')[0] for l in open(sys.argv[2]).read().splitlines()]
assert set(order)==set(groups), set(order)^set(groups)
print(f'# plan-35 footprints needed (2026-11-11): {len(order)} module footprints for {len(rows)} parts; 0 connector footprints.')
print('# Connector footprints are reused: HDR-1X16-P2.54 (16 pins), PINHDR-1X18-V, PINHDR-1X20-V')
print('# (all three already in footprints.json; Connector Only = a plain 1xN header in pin order, plan-11 rule).')
print('# One module footprint per (maker, outline, drawing sheet); Newhaven: one per model code.')
print('# Columns: id | outline (DigiKey L x W) | pins | header on the drawing | drawing (pdf page) | parts')
for g in order:
    m=groups[g]; r0=m[0]
    print(f"{g}\t{r0['dk_outline'].rsplit(' x ',1)[0]}\t{r0['pins']}\t{HDR[g]}\t{r0['pdf']} {r0['drawing_page']}\t{' '.join(x['part'] for x in m)}")
