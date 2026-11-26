# plan-81 planning (2026-11-25): the in-scope-maker rows of the OLED census pages.
#   python3 -I planning/plan-81-data/census-rows.py planning/plan-76-data/raw/o107.json planning/plan-76-data/raw/o99.json > planning/plan-81-data/census-rows.tsv
import json, sys
MAKERS = ('Newhaven', 'Display Visions', 'Orient', 'DLC')
print('category\tmaker\tpart\tin_stock\tcontroller\tformat\tinterface\tdisplay_type\tdatasheet_url')
for f in sys.argv[1:]:
    cat = '107' if f.endswith('o107.json') else '99'
    for p in json.load(open(f))['p']:
        v, par = {}, {}
        for x in p:
            v.setdefault(x['type'], x['value'])
            if x['type'] in ('string', 'stringList', 'link'):
                val = x['value']
                par[x['id']] = val if isinstance(val, str) else (val.get('value') if isinstance(val, dict) else json.dumps(val))
        pd = v['productDetail']
        m = pd['manufacturer']['value']['label']
        if not any(k in m for k in MAKERS):
            continue
        q = [y for y in v.get('qtyAvailable') or [] if y['label'] == 'In Stock']
        fmt = par.get('418') if cat == '107' else par.get('410')
        u = pd.get('datasheetUrl') or ''
        if u.startswith('//'):
            u = 'https:' + u
        print('\t'.join(str(s) for s in (cat, m, pd['productNumber'], (q[0]['quantity'] if q else '0').replace(',', ''),
                                          par.get('455', ''), fmt or '', par.get('154', ''), par.get('38', ''), u)))
