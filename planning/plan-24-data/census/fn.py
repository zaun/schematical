import re,sys,collections
def fn(name,base):
  for s in (name,base):
    m=re.match(r'^(?:SNJ|SNV|SN|CD|M|JM|)(?:54|74)([A-Z]*?)(\d+[GT]?\d*)',s)
    if m: return m.group(1),m.group(2)
    m=re.match(r'^CD(4\d{3})',s)
    if m: return 'CD4',m.group(1)
  return '?',base or name
