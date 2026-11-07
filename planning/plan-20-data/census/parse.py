import json,sys,collections,re
rows={}
for f in sys.argv[1:]:
  for l in open(f):
    d=json.loads(l)
    for p in d.get('Products',[]):
      prm={x['ParameterText']:x['ValueText'] for x in p.get('Parameters',[])}
      n=p['ManufacturerProductNumber']
      rows[n]=dict(name=n,base=(p.get('BaseProductNumber') or {}).get('Name',''),series=(p.get('Series') or {}).get('Name',''),
        status=(p.get('ProductStatus') or {}).get('Status',''),qty=p.get('QuantityAvailable',0),
        pkg=prm.get('Supplier Device Package',''),case=prm.get('Package / Case',''),
        typ=prm.get('Type') or prm.get('Logic Type',''),circ=prm.get('Circuit','') or prm.get('Number of Bits per Element',''),
        out=prm.get('Output Type',''),vs=prm.get('Voltage Supply Source',''),ds=p.get('DatasheetUrl',''),desc=(p.get('Description') or {}).get('ProductDescription',''))
w=open(sys.argv[1].replace('.jsonl','.tsv'),'w')
for r in rows.values(): w.write('\t'.join(str(r[k]) for k in ['name','base','series','status','qty','pkg','case','typ','circ','out','vs','ds','desc'])+'\n')
print(len(rows))
