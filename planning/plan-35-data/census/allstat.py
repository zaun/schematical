import json,sys,gzip,collections
want={'20 x 4':'20x4','4 x 20':'20x4','40 x 2':'40x2','40 x 4':'40x4'}
for f in sys.argv[1:]:
    seen={}
    for l in (gzip.open(f,'rt') if f.endswith('.gz') else open(f)):
        for p in json.loads(l).get('Products',[]):
            params={x['ParameterText']:x['ValueText'] for x in p.get('Parameters',[])}
            fm=params.get('Display Format','')
            if fm in want: seen[p['ManufacturerProductNumber']]=(want[fm],p['ProductStatus']['Status'],p.get('QuantityAvailable',0))
    c=collections.Counter((s, 'stock' if q>0 else 'nostock') for _,s,q in seen.values())
    print(f,len(seen),dict(c))
