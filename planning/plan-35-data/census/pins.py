# usage: pins.py txtfile... -> the pin table: longest run of consecutive "<n> <symbol>" lines (ranges like 7-14 expanded)
import sys,re
SYM=r'(V[Ss][Ss]|GND|V[Dd][Dd]|VCC|V[0Oo]|VEE|RS|R/W|RW|E1|E2|E|DB[0-7](?:-DB[0-7])?|D[0-7](?:-D[0-7])?|LED[+-]?(?:\([+-]\))?[RGB]?|LED-?[RGB]|BLA|BLK|NC|N\.C\.?|A|K|K[RGB]|[RGB]|LEDA|LEDK|BL[+-]|A\+|K-)'
for fn in sys.argv[1:]:
    t=re.sub(r'[ \t]+',' ',open(fn,errors='replace').read())
    best=[]
    lines=t.split('\n')
    for i in range(len(lines)):
        run=[];want=1;j=i
        while j<len(lines) and j<i+80:
            m=re.match(r'^ ?(?:PIN ?)?(\d{1,2})(?:\s*[-~]\s*(\d{1,2}))? '+SYM+r'(?:\b|$)',lines[j])
            if m and int(m.group(1))==want:
                lo=int(m.group(1)); hi=int(m.group(2) or lo)
                run.append((lo,hi,m.group(3))); want=hi+1
            j+=1
        if sum(h-l+1 for l,h,_ in run)>sum(h-l+1 for l,h,_ in best): best=run
    n=sum(h-l+1 for l,h,_ in best)
    print(f"{fn.split('/')[-1][:-4]:28} {n:2} " + ' '.join(f'{l}{"-"+str(h) if h!=l else ""}:{s}' for l,h,s in best))
