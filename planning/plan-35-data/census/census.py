import json,sys,gzip,collections
seen={}
for l in (gzip.open(sys.argv[1],'rt') if sys.argv[1].endswith('.gz') else open(sys.argv[1])):
    d=json.loads(l)
    for p in d.get('Products',[]):
        pn=p['ManufacturerProductNumber']
        if pn in seen: continue
        params={x['ParameterText']:x['ValueText'] for x in p.get('Parameters',[])}
        seen[pn]=dict(pn=pn,mfr=p['Manufacturer']['Name'],status=p['ProductStatus']['Status'],
            stock=p.get('QuantityAvailable',0),fmt=params.get('Display Format',''),
            type=params.get('Display Type',''),mode=params.get('Display Mode',''),bl=params.get('Backlight',''),
            iface=params.get('Interface',''),ctrl=params.get('Controller Type',''),v=params.get('Voltage - Supply',''),
            outline=params.get('Outline L x W x H',''),charfmt=params.get('Character Format',''),
            ds=p.get('DatasheetUrl',''),desc=p['Description']['ProductDescription'],series=p['Series']['Name'])
want={'20 x 4':'20x4','4 x 20':'20x4','40 x 2':'40x2','40 x 4':'40x4'}
rows=[r for r in seen.values() if r['fmt'] in want]
for r in rows: r['size']=want[r['fmt']]
print('unique in-stock cat99:',len(seen),' target formats:',len(rows),file=sys.stderr)
c=collections.Counter(r['mfr'] for r in rows)
for m,n in c.most_common(): print(f'{n:4} {m}',' '.join(f"{s}:{sum(1 for r in rows if r['mfr']==m and r['size']==s)}" for s in ('20x4','40x2','40x4')),file=sys.stderr)
cols=['pn','mfr','size','status','stock','type','mode','bl','iface','ctrl','v','outline','charfmt','series','ds','desc']
print('\t'.join(cols))
for r in sorted(rows,key=lambda r:(r['mfr'],r['size'],r['pn'])): print('\t'.join(str(r[k]) for k in cols))
