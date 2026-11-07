import sys,re,collections; sys.path.insert(0,'/tmp/p19')
from fn import fn
def rows(f):
  for l in open(f):
    r=l.rstrip('\n').split('\t'); yield r
P19=set('42 45 47 49 131 137 138 139 145 147 148 154 155 156 159 237 238 278 348 445 1G139 1G18 1G19 1G29 11138 11139 11238 4028 4514 4515 4532 4014 4555 4556 4511 4056 2414'.split())
P21=set('151 153 157 158 251 253 257 258 298 352 353 354 355 356 399 604 2G157 1G157 1G158 1T157 1T158 11151 11157 11251 11257 4512 4025 2257 250 850 851 857 870 670 16460 162460'.split())
P22=set('11181 181 182 183 264 275 283 381 382 385 80 8832 8847 881 882 261 384'.split())
EXCL20=re.compile(r'^(FB|GTL|GTLP|SSTL|SSTV|SSTVF|SSTU|SSTUB|SSQE|CBT|TVC|CB|ABTE)')
def special(n):
  for k,f in [('CAHCT138','138'),('CAHCT139','139'),('CLVC138','138'),('CLVC139','139'),('CY74FCT138','138'),('CAHCT157','157'),('CAHCT257','257'),('CLVC257','257'),('CY74FCT2257','2257')]:
    if n.startswith(k): return f
out={19:[],20:[],21:[],22:[]}
for f in ['cat743.tsv','cat703.tsv']:
  for r in rows(f):
    fam,fu=fn(r[0],r[1]); s=special(r[0]) or fu
    if s in P19: out[19].append((s,fam,r))
    elif s in P21: out[21].append((s,fam,r))
    elif s in P22: out[22].append((s,fam,r))
for f in ['cat773.tsv','cat720.tsv']:
  for r in rows(f):
    fam,fu=fn(r[0],r[1]); out[22].append((fu,fam,r))
scan=re.compile(r'^(8\d{3}|18\d{3}|182\d{3}|1284|161284|1071|1073|10\d\d|16222|3010|3306)$')
for r in rows('all704.tsv'):
  fam,fu=fn(r[0],r[1])
  if EXCL20.match(fam) or scan.match(fu) or 'Translat' in r[7] and False: continue
  out[20].append((fu,fam,r))
for k,v in out.items():
  w=open(f'p{k}.tsv','w')
  for fu,fam,r in v: w.write('\t'.join([fu,fam]+r)+'\n')
  devs=collections.Counter(fu for fu,_,_ in v)
  mil=sum(1 for fu,_,_ in v if not re.match(r'^[0-9]+[GT]?[0-9]*$',fu) )
  print(k,'parts',len(v),'functions',len(devs),'obsolete',sum(1 for *_,r in v if r[3]=='Obsolete'),'unmapped-numbers',mil)
