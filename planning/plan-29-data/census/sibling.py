"""plan-29 census, step 7: a maker-site datasheet for rows whose own link has none.

usage: python3 -I sibling.py <candidates.tsv>   (reads the <pdf-dir> = /tmp/p29/pdf text cache)

A candidate / check row whose DigiKey datasheet_url is not on its maker's
own site (vishay.com; onsemi.com; taiwansemi.com, ts.com.tw), or did not
download as a PDF, has no datasheet of its own.  This counts, per maker, how
many of those rows are printed verbatim (token boundaries, as textscan.py)
in a downloaded maker-site PDF of the same maker: that PDF becomes the row's
datasheet (plan-29-A section 3 rule 3).  The rest are skipped.
"""
import csv, collections, hashlib, os, re, sys
PDF = "/tmp/p29/pdf"
r=list(csv.DictReader(open(sys.argv[1]),delimiter='\t'))
S={"Vishay General Semiconductor - Diodes Division":"Vishay","Taiwan Semiconductor Corporation":"TSC","onsemi":"onsemi"}
OWN={'Vishay':('www.vishay.com',),'TSC':('services.taiwansemi.com','www.taiwansemi.com','services.ts.com.tw'),'onsemi':('www.onsemi.com',)}
host=lambda u:u.split('/')[2] if '//' in u else ''
norm=lambda s:" "+re.sub(r"[^0-9A-Z]+"," ",s.upper()).strip()+" "
c=[x for x in r if x['decision'] in('candidate','check')]
texts=collections.defaultdict(dict)
for x in c:
    m=S[x['maker']]; u=x['datasheet_url']
    if x['pdf']=='1' and host(u) in OWN[m] and u not in texts[m]:
        t='pdf/'+hashlib.md5(u.encode()).hexdigest()+'.txt'
        texts[m][u]=norm(open(os.path.join(PDF, os.path.basename(t)),errors='replace').read())
need=[x for x in c if not (x['pdf']=='1' and host(x['datasheet_url']) in OWN[S[x['maker']]])]
got=collections.Counter(); tot=collections.Counter()
for x in need:
    m=S[x['maker']]; k=norm(x['mpn']); tot[m]+=1
    if any(k in t for t in texts[m].values()): got[m]+=1
print('need',dict(tot),'found verbatim in a same-maker PDF',dict(got))
