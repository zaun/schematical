import json,sys
d=json.load(open(sys.argv[1])); d.setdefault('n',d.get('productCount')); d.setdefault('f',d.get('filters')); want=sys.argv[2:] 
print('TOTAL',d['n'])
for g in d['f']:
    if not want or g['name'] in want or g['id'] in want:
        o=sorted(g['o'],key=lambda x:-int(str(x[2]).replace(',','') or 0))
        print('##',g['id'],g['name'],len(o)); 
        for t in o[:40]: print('   ',t[2],t[0],'(%s)'%t[1])
    elif not want: pass
if not want: print([ (g['id'],g['name']) for g in d['f']])
