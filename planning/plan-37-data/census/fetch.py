#!/usr/bin/env python3
# plan-37-A Phase 2: one vendor PDF per distinct source (plan-37-A § 3 rule 3).
# usage: python3 -I fetch.py <repo root> <rawdir> [--offline]
#   Reads and rewrites planning/plan-37-data/candidates.tsv in place: fills
#   `pdf` (path under datasheets/) on every in-scope row whose verdict is not
#   yet `out`, or sets `verdict` skip with the reason in `ordering`.
#   Writes planning/plan-37-data/datasheets.tsv: file url bytes sha256
#   (file = the path under datasheets/).
# Sources:
#   TI        the product folder: the longest prefix of the orderable, down to
#             the device number, whose ti.com/product/<prefix> page answers 200
#             (SNJ54 orderables also try the SN54 prefixes; military numbers
#             the SN54/CD54 types of candidates.py MIL); the PDF is
#             ti.com/lit/ds/symlink/<folder>.pdf
#   Nexperia  DigiKey's assets.nexperia.com URL; else the URL of another
#             Nexperia row of the same device number and family; else the
#             data-sheet names Nexperia uses (74HC_HCT173.pdf, 74LVC273.pdf …)
#   onsemi    DigiKey's onsemi.com URL; else another onsemi row's of the same
#             device, family and part-number stem; else
#             onsemi.com/download/data-sheet/pdf/<stem>-d.pdf
# Probes and downloads are cached in /tmp/p37/fetch-cache.json; files with the
# same bytes are kept once (the first file name wins).
import csv, hashlib, json, os, re, subprocess, sys, time
from pathlib import Path

root, rawdir = Path(sys.argv[1]), Path(sys.argv[2])
data = root / 'planning/plan-37-data'
cand = data / 'candidates.tsv'
sheets = root / 'datasheets'
VENV = '/tmp/p37venv/bin/python'
NXGET = str(data / 'census/nxget.py')
UA = ('Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 '
      '(KHTML, like Gecko) Version/17.0 Safari/605.1.15')
CACHE = Path('/tmp/p37/fetch-cache.json')
cache = json.loads(CACHE.read_text()) if CACHE.exists() else {}
TI, ON, NX = 'Texas Instruments', 'onsemi', 'Nexperia USA Inc.'
last = {}


def save_cache():
    CACHE.write_text(json.dumps(cache, indent=0))


def pace(host):
    t = last.get(host, 0)
    if time.time() - t < 1:
        time.sleep(1 - (time.time() - t))
    last[host] = time.time()


def ti_page(prefix):
    k = 'ti-page:' + prefix
    if k not in cache:
        pace('ti')
        r = subprocess.run(['curl', '-s', '-o', '/dev/null', '-A', UA, '-w', '%{http_code}', '-L',
                            f'https://www.ti.com/product/{prefix}'], capture_output=True, text=True)
        cache[k] = r.stdout.strip()
        save_cache()
    return cache[k] == '200'


def download(url, out, nexperia=False):
    """Fetches url to the temp file out; returns True when it is a PDF."""
    k = 'get:' + url
    if k in cache and cache[k] is False:
        return False
    tmp = Path('/tmp/p37/dl') / hashlib.sha1(url.encode()).hexdigest()
    tmp.parent.mkdir(parents=True, exist_ok=True)
    if not tmp.exists():
        host = url.split('/')[2]
        pace(host)
        if nexperia:
            r = subprocess.run([VENV, '-I', NXGET, url, str(tmp)], capture_output=True, text=True)
            ok = r.returncode == 0
        else:
            r = subprocess.run(['curl', '-sL', '--max-time', '180', '-o', str(tmp), url])
            ok = r.returncode == 0 and tmp.exists() and tmp.read_bytes()[:5] == b'%PDF-'
            if ok:
                ok = subprocess.run(['pdfinfo', str(tmp)], capture_output=True).returncode == 0
        if not ok:
            tmp.unlink(missing_ok=True)
            cache[k] = False
            save_cache()
            return False
        cache[k] = True
        save_cache()
    out.write_bytes(tmp.read_bytes())
    return True


PAT = re.compile(r'^(?:NLV(?=14)|NLV|NL)?(?:SNJ54|SN54|SN74|CD54|CD74|CY74|MM74|MC74|MC54|M74|MC|CLV|CD|HEF|HEC|74|54|)'
                 r'([A-Z]*?)(\d[GT]\d+|\d{2,6})')
MIL = {'5962-8682501EA': ('173', 'HC'), '5962-87807012A': ('377', 'HC'),
       '5962-8976901RA': ('377', 'HCT'), '5962-8992501EA': ('378', 'LS'),
       '5962-9314801Q2A': ('377', 'ABT'), '5962-9314801QRA': ('377', 'ABT'),
       '8400102RA': ('576', 'ALS'), 'JM38510/32504BRA': ('377', 'LS'),
       'JM38510/32504BSA': ('377', 'LS'),
       # found by plan-37-A's full TI listing (16- and 20-pin pieces)
       'JM38510/36101BEA': ('173', 'TTL'), 'JM38510/30109BEA': ('109', 'LS'),
       'JM38510/37201BEA': ('174', 'ALS'), '5962-9222203MRA': ('574', 'FCT'),
       '5962-9221804MRA': ('374', 'FCT'), '5962-9221806M2A': ('374', 'FCT')}


def ti_prefixes(mpn):
    if mpn in MIL:
        num, fam = MIL[mpn]
        out = []
        for pre in ('SN54', 'CD54'):
            for suf in ('B', 'A', ''):
                out.append(f"{pre}{'' if fam == 'TTL' else fam}{num}{suf}")
        return out
    m = PAT.match(mpn)
    if not m:
        return []
    names = [mpn]
    if mpn.startswith('SNJ54'):
        names.append('SN54' + mpn[5:])
    out = []
    for n in names:
        end = m.end(2) - (1 if n is not mpn else 0)
        out += [n[:i] for i in range(len(n), end - 1, -1)]
    # Folders the prefixes cannot reach (plan-37-A § Corrections): TI files
    # CY74FCT…A/B/C…T parts under CY74FCT<n>T, unprefixed 74… parts under
    # SN74…[A], CLV…EP parts under SN74…-EP, and some parts under the
    # device's A/B revision folder (SN74ABT377N -> SN74ABT377A).
    base = mpn[:m.end(2)]
    if mpn.startswith(('CY74FCT', '74FCT')):
        out.append('CY' + base[base.index('74'):] + 'T')
    elif mpn.startswith('74'):
        out += ['SN' + base + 'A', 'SN' + base]
    elif mpn.startswith('CLV') and mpn.endswith('EP'):
        out.append('SN74' + base[1:] + '-EP')
    else:
        for n in names:
            b = n[:m.end(2) - (1 if n is not mpn else 0)]
            out += [b + 'A', b + 'B']
    return list(dict.fromkeys(out))


def ti_source(row):
    for p in ti_prefixes(row['mpn']):
        if ti_page(p):
            return p.lower(), f'https://www.ti.com/lit/ds/symlink/{p.lower()}.pdf'
    return None, None


def nx_type(mpn):
    t = mpn.split(',')[0]
    return t


def nx_guesses(row):
    fam, num = row['family'], None
    m = PAT.match(row['mpn'])
    if m:
        num = m.group(2)
    if not num:
        return []
    g = []
    if row['mpn'].startswith('HEF'):
        g += [f'HEF{num}B']
    else:
        pairs = {'HC': 'HC_HCT', 'HCT': 'HC_HCT', 'AHC': 'AHC_AHCT', 'AHCT': 'AHC_AHCT',
                 'LV': 'LV', 'LVC': 'LVC', 'ALVC': 'ALVC', 'ALVT': 'ALVT', 'ABT': 'ABT',
                 'AC': 'AC_ACT', 'ACT': 'AC_ACT', 'LVT': 'LVT'}
        stem = pairs.get(fam, fam)
        for name in (stem + num, fam + num, stem + num + 'A', fam + num + 'A'):
            g.append('74' + name)
    m2 = re.match(r'^(.*?\d{2,6}[A-Z]?)', row['mpn'])
    if m2:   # the type through its device number: 74ALVCH16823DL,512 -> 74ALVCH16823
        g += [m2.group(1)[:m.end(2)], m2.group(1)]
    return [f'https://assets.nexperia.com/documents/data-sheet/{x}.pdf' for x in dict.fromkeys(g)]


def stem(mpn):
    # the part number without its package / packing letters: MC74HC374ADWR2G -> MC74HC374A
    m = PAT.match(mpn)
    if not m:
        return mpn
    rest = mpn[m.end(2):]
    a = re.match(r'[AB](?=[A-Z])', rest) if not mpn.startswith('74') else None   # 74LCX374BQX: B is the package
    return mpn[:m.end(2)] + (a.group(0) if a else '')


def series(mpn):
    """onsemi's series prefix: MC, MM, '' (74LCX…, Fairchild); NLV parts are MC sheets."""
    p = mpn[:mpn.find('74')] if '74' in mpn else mpn[:2]
    return 'MC' if p.startswith('NLV') else p


rows = list(csv.DictReader(cand.open(), delimiter='\t'))
cols = list(rows[0].keys())
known_url = {}  # (maker, device, family[, stem]) -> URL on the maker's own host
for r in rows:
    u = r['datasheet_url']
    if r['maker'] == NX and 'assets.nexperia.com' in u:
        q = '_Q100' in u.upper()   # a Q100 sheet only for -Q100 parts
        known_url.setdefault((NX, r['device'], r['family'], q), u)
    if r['maker'] == ON and 'onsemi.com' in u:
        known_url.setdefault((ON, r['device'], r['family'], stem(r['mpn'])), u)
        known_url.setdefault((ON, r['device'], r['family'], series(r['mpn'])), u)
        known_url.setdefault((ON, r['device'], r['family']), u)


def target_dir(row):
    return {'register': 'register', 'dff': 'ff-d', 'mux': 'mux'}[row['kind']]


by_sha = {}      # sha -> path under datasheets/
tsv = {}         # path -> (url, bytes, sha)
existing_sha = {}
for d in ('register', 'ff-d', 'mux'):
    for f in (sheets / d).glob('*.pdf') if (sheets / d).exists() else []:
        existing_sha[hashlib.sha256(f.read_bytes()).hexdigest()] = f'{d}/{f.name}'
stops = []


def place(row, name, url, nexperia=False):
    """Downloads url as datasheets/<dir>/<name>; returns the path, or None."""
    d = target_dir(row)
    (sheets / d).mkdir(exist_ok=True)
    rel = f'{d}/{name}'
    for p, (u, _, _) in tsv.items():
        if u == url and p.startswith(d + '/'):
            return p
    tmp = Path('/tmp/p37/one.pdf')
    if not download(url, tmp, nexperia):
        return None
    body = tmp.read_bytes()
    sha = hashlib.sha256(body).hexdigest()
    if sha in by_sha and by_sha[sha].startswith(d + '/'):
        return by_sha[sha]
    dest = sheets / rel
    if dest.exists() and hashlib.sha256(dest.read_bytes()).hexdigest() != sha:
        if rel not in tsv:
            stops.append(f'{rel}: exists with different bytes ({url})')
            return rel
    dest.write_bytes(body)
    by_sha[sha] = rel
    tsv[rel] = (url, len(body), sha)
    return rel


for row in rows:
    if row['scope'] != 'in' or (row['verdict'] == 'out' and row['ordering'] == 'stock check'):
        continue
    row['pdf'] = ''
    row['verdict'], row['ordering'] = '', ''
    if row['verdict'] == 'skip':
        row['verdict'], row['ordering'] = '', ''
    if row['kind'] == 'mux' and row['device'] == '74_670':
        row['pdf'] = 'mux/sn74ls670.pdf'
        continue
    maker, url = row['maker'], row['datasheet_url']
    if maker == TI:
        got, folder = None, None
        for p in ti_prefixes(row['mpn']):   # a page whose symlink PDF is missing: the next shorter folder
            if not ti_page(p):
                continue
            folder = p.lower()
            got = place(row, folder + '.pdf', f'https://www.ti.com/lit/ds/symlink/{folder}.pdf')
            if got:
                break
        if folder is None:
            row['verdict'], row['ordering'] = 'skip', 'no TI product page'
            continue
        if got is None:
            row['verdict'], row['ordering'] = 'skip', ('only a Rochester copy' if 'rocelec' in url
                                                       else f'no TI datasheet ({folder})')
            continue
        row['pdf'] = got
    elif maker == NX:
        num = PAT.match(row['mpn']).group(2)
        srcs = []
        if 'assets.nexperia.com' in url:
            srcs.append(url)
        key = (NX, row['device'], row['family'], '-Q100' in row['mpn'])
        if key in known_url:
            srcs.append(known_url[key])
        srcs += nx_guesses(row)
        got = None
        for s in dict.fromkeys(srcs):
            got = place(row, s.rsplit('/', 1)[1].lower(), s, nexperia=True)
            if got:
                break
        if got is None:
            row['verdict'], row['ordering'] = 'skip', 'no Nexperia data sheet'
            continue
        row['pdf'] = got
    elif maker == ON:
        srcs = []
        if 'onsemi.com' in url:
            srcs.append(url)
        st = stem(row['mpn'])
        if (ON, row['device'], row['family'], st) in known_url:
            srcs.append(known_url[(ON, row['device'], row['family'], st)])
        srcs.append(f'https://www.onsemi.com/download/data-sheet/pdf/{st.lower()}-d.pdf')
        if (ON, row['device'], row['family'], series(row['mpn'])) in known_url:
            srcs.append(known_url[(ON, row['device'], row['family'], series(row['mpn']))])
        if (ON, row['device'], row['family']) in known_url:   # last resort; the ordering table decides
            srcs.append(known_url[(ON, row['device'], row['family'])])
        got = None
        for s in dict.fromkeys(srcs):
            name = s.rsplit('/', 1)[1].split('?')[0].lower()
            got = place(row, name, s)
            if got is None and '/pdf/datasheet/' in s:
                s2 = s.replace('/pdf/datasheet/', '/download/data-sheet/pdf/')
                got = place(row, name, s2)
            if got:
                break
        if got is None:
            row['verdict'], row['ordering'] = 'skip', 'no onsemi data sheet'
            continue
        row['pdf'] = got

with cand.open('w', newline='') as f:
    w = csv.DictWriter(f, fieldnames=cols, delimiter='\t', lineterminator='\n')
    w.writeheader()
    w.writerows(rows)
with (data / 'datasheets.tsv').open('w') as f:
    f.write('file\turl\tbytes\tsha256\n')
    for p in sorted(tsv):
        u, n, s = tsv[p]
        f.write(f'{p}\t{u}\t{n}\t{s}\n')
print(f'fetch.py: {len(tsv)} PDFs, '
      f'{sum(1 for r in rows if r["scope"] == "in" and r["pdf"])} rows with pdf, '
      f'{sum(1 for r in rows if r["scope"] == "in" and r["verdict"] == "skip")} skip')
for s in stops:
    print('STOP', s)
