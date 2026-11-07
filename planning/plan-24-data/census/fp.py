import sys,collections,re
M={'14-PDIP':'PDIP-14','16-PDIP':'PDIP-16','20-PDIP':'PDIP-20','24-PDIP':'PDIP-24','8-PDIP':'PDIP-8','14-SOIC':'SOIC-14','16-SOIC':'SOIC-16','20-SOIC':'SOIC-20W','24-SOIC':'SOIC-24W','28-SOIC':'SOIC-28W','8-SOIC':'SOIC-8',
'14-SO':'SOP-14','16-SO':'SOP-16','20-SO':'SOP-20','24-SO':'SOP-24','8-SO':'SOP-8','14-SSOP':'SSOP-14','16-SSOP':'SSOP-16','20-SSOP':'SSOP-20','24-SSOP':'SSOP-24','48-SSOP':'SSOP-48','56-SSOP':'SSOP-56','28-SSOP':'SSOP-28',
'14-TSSOP':'TSSOP-14','16-TSSOP':'TSSOP-16','20-TSSOP':'TSSOP-20','24-TSSOP':'TSSOP-24','28-TSSOP':'TSSOP-28','48-TSSOP':'TSSOP-48','56-TSSOP':'TSSOP-56','8-TSSOP':'TSSOP-8','64-TSSOP':'TSSOP-64',
'14-TVSOP':'TVSOP-14','16-TVSOP':'TVSOP-16','20-TVSOP':'TVSOP-20','24-TVSOP':'TVSOP-24','48-TVSOP':'TVSOP-48','56-TVSOP':'TVSOP-56','80-TVSOP':'TVSOP-80','100-TVSOP':'TVSOP-100',
'SOT-23-5':'SOT23-5','SOT-23-6':'SOT23-6','SC-70-5':'SC70-5','SC-70-6':'SC70-6','8-VSSOP':'VSSOP-8','20-VSSOP':'VSSOP-20','14-WQFN (3x2.5)':'WQFN-14','16-WQFN (2.5x3.5)':'WQFN-16','14-VQFN (3.5x3.5)':'VQFN-14','16-VQFN (4x3.5)':'VQFN-16','20-VQFN (3.5x4.5)':'VQFN-20-RGY','20-VQFN (2.5x4.5)':'VQFN-20','20-VQFN (4.5x2.5)':'VQFN-20',
'20-CDIP':'CDIP-20','16-CDIP':'CDIP-16','14-CDIP':'CDIP-14','24-CDIP':'CDIP-24','20-LCCC':'LCCC-20','28-LCCC':'LCCC-28','20-CFP':'CFP-20','16-CFP':'CFP-16','14-CFP':'CFP-14','5-DSBGA (1.4x0.9)':'DSBGA-5-YZP','6-DSBGA':'DSBGA-6-YZP','8-DSBGA':'DSBGA-8-YZP',
'6-SON (1x1)':'SON-6-DSF','6-SON (1.45x1)':'SON-6-DRY','8-X2SON (1.4x1)':'X2SON-8','5-X2SON (0.8x0.8)':'X2SON-5-DPW','8-UQFN (1.5x1.5)':'UQFN-8','SOT-5':'SOT5X3-5','16-SOT-23-THIN':None,'14-SOT-23-THIN':'SOT23-THIN-14','96-LFBGA (13.5x5.5)':'NFBGA-96'}
import json
have={f['id'] for f in json.load(open('/Users/justinzaun/Development/logic/schematical/schematical/Parts/footprints.json'))['footprints']}
for k in sys.argv[1:]:
  c=collections.Counter(); nm=collections.Counter()
  for l in open(f'p{k}.tsv'):
    r=l.rstrip('\n').split('\t'); p=r[7]; c[p]+=1
    fp=M.get(p)
    if not p: nm['(blank: ceramic/military; map from TI code)']+=1
    elif fp is None or fp not in have: nm[p+(' -> '+fp if fp else '')]+=1
  print('== plan',k, 'packages',len(c)); 
  for p,n in nm.most_common(): print(f'  {n:4} {p}')
