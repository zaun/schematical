# usage: font.py <pdfdir> : per PDF, which "Font type"/"Font Set" option is chosen
import subprocess,sys,os,re
CHK={'','þ','','■','☑',''}
UNCHK={'','o','','□','☐'}
for f in sorted(os.listdir(sys.argv[1])):
    if not f.endswith('.pdf'): continue
    t=subprocess.run(['pdftotext','-raw',os.path.join(sys.argv[1],f),'-'],capture_output=True,text=True).stdout
    res=''
    m=re.search(r'Font Set\s+([^\n]+)',t)
    if m: res='set:'+m.group(1).strip()
    else:
        i=t.find('Font type')
        if i>=0 and 'English-Jap' in t[i:i+200]:
            seg=t[i+9:i+160]
            parts=re.findall(r'(\S)\s*(English-[A-Za-z\n]+|Other|other)',seg)
            chosen=[p[1].replace('\n','') for p in parts if p[0] in CHK]
            res='box:'+','.join(chosen) if parts else 'box:?'+repr(seg[:80])
            if parts and not chosen: res='box:none '+repr([p[0] for p in parts])
        else:
            m=re.search(r'ST7066U?-0([A-Z])',t)
            res='ctrl-rom:0'+m.group(1) if m else ('JT-std' if 'English and Japanese Standard Font' in t else '-')
    print(f'{f}\t{res}')
