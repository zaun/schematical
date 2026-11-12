# plan-35 candidates.tsv builder. usage:
#   candidates.py <census.py output tsv> verdicts.tsv footprint-groups.tsv drawing-pages.tsv midas-urls.tsv > ../candidates.tsv
# midas-urls.tsv: the Midas parts' own-site PDF URLs (midas.sh); they replace DigiKey's media links.
# One row per top-3 in-stock 20x4/40x2/40x4 part (126). For "add" rows it adds
# the facts read from the datasheets on 2026-11-11 (panel, backlight, pinout,
# footprint, connector, drawing page). DigiKey columns are DigiKey's, a cross-check only.
import sys
cen={}
lines=open(sys.argv[1]).read().splitlines()
head=lines[0].split('\t')
for l in lines[1:]:
    r=dict(zip(head,l.split('\t'))); cen[r['pn']]=r
ver=[l.split('\t') for l in open(sys.argv[2]).read().splitlines()[1:]]
fp={}
for l in open(sys.argv[3]).read().splitlines():
    gid,members=l.split('\t')
    for m in members.split(): fp[m]=gid
page={l.split('\t')[0]:l.split('\t')[2] for l in open(sys.argv[4]).read().splitlines()}
own={l.split('\t')[0]:l.split('\t')[1] for l in open(sys.argv[5]).read().splitlines()} if len(sys.argv)>5 else {}

def pinout(pn):
    if pn in ('C204AXBSGN06WR50XAA','MC42004A6W-SPR-V2','MC42008A6W-SPR','C402ALBSGN06WT50PAB'): return 'p16-nc',16
    if pn.startswith(('C404ALB','MC44005','NHD-0440WH')): return 'p18-dual',18
    if pn=='NHD-0440AZ-RN-FBW': return 'p18-dual-nc',18
    if pn.startswith(('MD42004','MD42005C')): return 'p18-rgb',18
    if pn.startswith('MD24005'): return 'p18-rgb-pads',18
    if pn.startswith('MD44005'): return 'p20-rgb-dual',20
    if pn=='C204HLBSYLY6WM': return 'p20-fgnd',20
    return 'p16',16
def panel(pn):
    if pn.startswith('NHD-'):
        code=pn.split('-')[2]
        return {'FL':None,'FSW':None,'NSW':None,'RN':'white','ATFH':'white','ATMI':'blue-negative'}.get(code) or \
               {'G':'gray','Y':'yellow-green','F':'white','B':'blue-negative'}[pn.split('-')[3][0]]
    if pn.startswith(('MC','MD')):
        return 'blue-negative' if 'BNMLW' in pn else 'white' if '-FPTL' in pn else 'yellow-green'
    if pn in ('C204A-BW-LW25','C402A3-BW-LW65'): return 'blue-negative'
    if pn=='C204ADBFGSW6WT55PAB': return 'white'
    k=pn[5:9] if pn[4] in 'ABCDH' and pn[5:7] in ('LB','XB','BB','DB','SB') else ''
    tail=pn[7:9]
    return {'SB':'blue-negative','SY':'yellow-green','SG':'gray','FW':'white','FK':'white','FG':'white'}[tail]
def backlight(pn,dk):
    if 'RGB' in pn: return 'Red,Green,Blue'
    if pinout(pn)[0] in ('p16-nc','p18-dual-nc'): return '-'
    return {'LED - White':'White','LED - Yellow/Green':'Yellow-green','LED - Yellow':'Yellow-green','LED - Blue':'Blue'}[dk]
def connector(n): return 'HDR-1X16-P2.54' if n==16 else 'PINHDR-1X%02d-V'%n
cols=['maker','part','size','verdict','reason','panel','backlight','pinout','pins','footprint','connector','drawing_page','pdf','datasheet_url','dk_status','dk_stock','dk_controller','dk_vcc','dk_outline']
print('\t'.join(cols))
for mfr,pn,v,why in ver:
    r=cen[pn]; url=r['ds'] if not r['ds'].startswith('//') else 'https:'+r['ds']
    url=own.get(pn,url)
    pdf=url.split('/')[-1].replace('3086_','')
    row=[mfr,pn,r['size'],v,why]
    if v=='add':
        po,n=pinout(pn)
        row+=[panel(pn),backlight(pn,r['bl']),po,str(n),fp[pn],connector(n),'p'+page[pn.replace('#','')] if pn.replace('#','') in page else 'p'+page[pn],pdf]
    else: row+=['']*7+[pdf]
    row+=[url,r['status'],r['stock'],r['ctrl'],r['v'],r['outline']]
    print('\t'.join(row))
