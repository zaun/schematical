# plan-80 (planning, 2026-11-25): prints a saved census page's rows: part | maker | own In Stock | parameter values.
#   python3 -I planning/plan-80-data/census/rows.py <page.json> [param ids, comma-separated, e.g. 154,46]
import json,sys
# rows.py file.json [param ids comma] : prints part|maker|instock|params
d=json.load(open(sys.argv[1])); want=sys.argv[2].split(',') if len(sys.argv)>2 else []
for p in d['p']:
    v={}; par={}
    for x in p:
        v.setdefault(x['type'],x['value'])
        if x['type'] in ('string','stringList','link'): par[x['id']]=x['value'] if isinstance(x['value'],str) else json.dumps(x['value'])[:60]
    pd=v['productDetail']; q=[y for y in v.get('qtyAvailable') or [] if y['label']=='In Stock']
    print(pd['productNumber'],'|',pd['manufacturer']['value']['label'][:20],'|',q[0]['quantity'] if q else 0,'|',' | '.join(str(par.get(w,'')) for w in want))
