# usage: outline.py keep.tsv txtdir -> pn, module outline LxW (from the datasheet text), pin count of the interface
import sys,re,os
for line in open(sys.argv[1]):
    mfr,pn,url=line.rstrip('\n').split('\t')
    t=open(os.path.join(sys.argv[2],url.split('/')[-1].replace('.pdf','.txt')),errors='replace').read()
    s=re.sub(r'[ \t]+',' ',t)
    m=re.search(r'(?i)(?:module size|outline dimension[s]?|module dimension|overall dimension|Outline\s*\(?W\s*x\s*H|Dimensional Outline)[^\n0-9]{0,40}([0-9]{2,3}(?:\.[0-9]+)?)\s*(?:mm)?\s*(?:\(?[LW]\)?)?\s*[x×X\*]\s*([0-9]{2,3}(?:\.[0-9]+)?)',s)
    if not m:
        m=re.search(r'(?i)\b(1[0-9]{2}\.[0-9]|[5-9][0-9]\.[0-9])\s*(?:mm)?\s*(?:\(?[LW]\)?)?\s*[x×X\*]\s*([3-9][0-9]\.[0-9])\s*(?:mm)?\s*(?:\(?[WH]\)?)?\s*[x×X\*]',s)
    print(f'{mfr.split()[0]}\t{pn}\t{m.group(1)+"x"+m.group(2) if m else "?"}')
