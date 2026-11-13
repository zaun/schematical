# plan-36: match each AVR part to an ATDF pinout (package type + pin count).
import json,glob,collections,re,sys,os
import xml.etree.ElementTree as ET
TYPES=[('DIP',r'DIP'),('SOIC',r'SOIC|(?<![TS])SO(?!T)'),('TSSOP',r'TSSOP'),('SSOP',r'(?<!T)SSOP'),
       ('QFP',r'QFP'),('QFN',r'QFN|MLF|VQFN'),('DFN',r'DFN|MLF10|USON'),('BGA',r'BGA'),('WLCSP',r'WLCSP|CSP'),
       ('SOT23',r'SOT-?23'),('PLCC',r'PLCC|LCC')]
def ptypes(s):
  return {t for t,rx in TYPES if re.search(rx,s.upper())}
def load_atdf():
  atdf={}
  for f in glob.glob(os.environ.get('ATDF','/tmp/p36/packs/atdf')+'/*.atdf'):
    r=ET.parse(f).getroot(); d=r.find('devices/device'); name=d.get('name').upper()
    pos={po.get('name'):[(p.get('position'),p.get('pad')) for p in po.findall('pin')] for po in r.iter('pinout')}
    atdf[name]=(os.path.basename(f),pos)
  return atdf
def device(m):
  s=m.split('-')[0]
  if re.match(r'AVR\d+[A-Z]{2}\d+T$',s): s=s[:-1]
  return s
def candidates(s):
  n=[s]; m=re.match(r'(AT(MEGA|TINY|90)\w*?)(V|L)$',s)
  if m: n.append(m.group(1))
  if s.endswith('RC'): n.append(s[:-2])
  return n
def geom(s):
  u=s.upper()
  if 'DRQFN' in u or 'DUAL ROWS' in u: return 'drqfn'
  if re.search(r'PLCC|LCC',u): return 'plcc'
  if re.search(r'BGA|WLCSP|CSP',u): return 'grid'
  if re.search(r'MLF10|10-VFDFN|DFN|USON|SOT',u): return 'dual'
  if re.search(r'QFP|QFN|MLF|QUAD',u): return 'quad'
  if re.search(r'DIP|SOIC|SSOP|TSSOP|SO',u): return 'dual'
  return None
def count(name,pins):
  m=re.findall(r'(\d+)',name)
  if m: return int(m[-1])
  nums=[int(p[0]) for p in pins if p[0].isdigit()]
  return max(nums) if nums else len(pins)
def match(atdf,dev,pkg):
  g=geom(pkg); m=re.match(r'(\d+)',pkg or ''); n=int(m.group(1)) if m else (6 if 'SOT-23-6' in (pkg or '') else None)
  out=[]
  for name,pins in atdf[dev][1].items():
    if geom(name)==g and (n is None or count(name,pins)==n): out.append(name)
  return sorted(out)
if __name__=='__main__':
  atdf=load_atdf(); rows={}
  for f in sorted(glob.glob(os.environ.get('RAW','/tmp/p36/raw')+'/*.jsonl')):
    if 'espressif' in f: continue
    for l in open(f):
      p=json.loads(l); rows.setdefault(p['mpn'],p)
  res=collections.Counter(); misses=collections.Counter(); units=set(); multi=collections.Counter()
  for p in rows.values():
    s=device(p['mpn']); hit=next((x for x in candidates(s) if x in atdf),None)
    pk=p['params'].get('Package / Case') or ''
    if not hit: res['no ATDF device']+=1; misses[(s,pk,'no device')]+=1; continue
    if not atdf[hit][1]: res['ATDF has no pinouts']+=1; misses[(hit,pk,'empty')]+=1; continue
    m=match(atdf,hit,pk)
    if len(m)==1: res['one pinout']+=1; units.add((hit,m[0]))
    elif m: res['several']+=1; multi[(hit,pk,tuple(m))]+=1; units.add((hit,m[0]))
    else: res['no pinout of that package']+=1; misses[(hit,pk,tuple(atdf[hit][1]))]+=1
  print(dict(res),'distinct (ATDF device, pinout) used',len(units))
  print('devices with empty ATDF pinouts:',sorted({k[0] for k in misses if k[2]=='empty'}))
  for k,n in sorted(misses.items()):
    if k[2]!='empty': print(' miss',n,k)
  for k,n in multi.items(): print(' multi',n,k)
