"""plan-48 census: shared code. Reads the listing (raw/*.jsonl[.gz]), names
each row's company, type and class, applies the scope rules and resolves the
maker-site datasheet URL of a row."""
import glob
import gzip
import json
import os
import re
import urllib.parse

# DigiKey maker name -> company. The legacy labels are Rochester Electronics
# stock (every datasheet link of theirs is rocelec.widen.net, plan-48-A § 2).
COMPANY = {
    'Texas Instruments': 'ti',
    'Analog Devices Inc.': 'adi',
    'Analog Devices Inc./Maxim Integrated': 'adi',
    'STMicroelectronics': 'st',
    'Renesas Electronics Corporation': 'renesas',
}
LEGACY = ('Harris Corporation', 'National Semiconductor')
# The three makers of each type, by in-stock count (plan-48-A § 2).
MAKERS = {'timer': ('ti', 'adi', 'renesas'), 'opamp': ('ti', 'adi', 'st')}
QUOTA = 200          # op-amps per maker; timers have no cap (62 rows)
HIREL = re.compile(r'CDIP|CERDIP|Metal Can|TO-99|TO-5|CFP|Flatpack|Flat Pack|CLCC|LCCC|Ceramic', re.I)
CLASS = {'Standard (General Purpose)': 'gp', 'J-FET': 'jfet', 'CMOS': 'cmos'}


def read_raw(raw_dir):
    rows = []
    for path in sorted(glob.glob(os.path.join(raw_dir, '*.jsonl*'))):
        op = gzip.open if path.endswith('.gz') else open
        cut = 0
        err = path.split('.jsonl')[0] + '.err'
        if os.path.exists(err) and 'cut at 300' in open(err).read():
            cut = 1
        with op(path, 'rt', encoding='utf-8') as f:
            for line in f:
                r = json.loads(line)
                r['cut'] = cut
                rows.append(r)
    return rows


def classify(r):
    """Sets company, class, circuits; returns '' or the reason the row is out."""
    p = r['params']
    r['company'] = COMPANY.get(r['maker'], '')
    if r['type'] == 'timer':
        r['class'] = 'dual' if 'Dual' in p.get('Type', '') else 'single'
        r['circuits'] = '2' if r['class'] == 'dual' else '1'
    else:
        r['class'] = CLASS.get(p.get('Amplifier Type', ''), '')
        r['circuits'] = p.get('Number of Circuits', '')
    if r['maker'] in LEGACY:
        return 'legacy label'
    if r['company'] not in MAKERS[r['type']]:
        return 'maker'
    if r['piece'] == 'opamp-legacy':
        return 'legacy label'
    if 'stock' not in r or r['stock'] is None:
        return 'no stock figure'
    if r['status'] not in ('Active', 'Last Time Buy'):
        return 'status'
    if p.get('Grade') == 'Military' or 'MIL' in p.get('Qualification', ''):
        return 'military'
    if HIREL.search(p.get('Package / Case', '') + ' ' + p.get('Supplier Device Package', '')):
        return 'hi-rel package'
    if r['type'] == 'opamp' and (not r['class'] or r['circuits'] not in ('1', '2', '4')):
        return 'type'
    return ''


def _norm(url):
    url = (url or '').strip()
    return 'https:' + url if url.startswith('//') else url


def ti_gpn(mpn):
    m = re.match(r'[A-Z]+\d+', mpn)
    return m.group(0).lower() if m else ''


def st_base(mpn):
    m = re.match(r'[A-Z]+\d+', mpn)
    return m.group(0).lower() if m else ''


def resolve(r):
    """(via, url): the maker's own datasheet URL of a row. via is 'link'
    (DigiKey's link, rewritten to its plain form), 'guess' (DigiKey links a
    Rochester or DigiKey-hosted copy, or nothing: the maker's URL scheme
    with the part's base name) or '' (none)."""
    url = _norm(r['datasheet'])
    u = urllib.parse.urlparse(url)
    host, c, mpn = u.netloc.lower(), r['company'], r['mpn']
    if c == 'ti':
        if host.endswith('ti.com') or host.endswith('tij.co.jp'):
            if 'suppproductinfo' in u.path:
                goto = urllib.parse.parse_qs(u.query).get('gotoUrl', [''])[0]
                for _ in range(2):
                    goto = urllib.parse.unquote(goto)
                m = re.search(r'/lit/gpn/([A-Za-z0-9\-]+)', goto)
                if m:
                    return 'link', 'https://www.ti.com/lit/gpn/' + m.group(1).lower()
            elif u.path.endswith('.pdf'):
                return 'link', 'https://www.ti.com' + u.path
        g = ti_gpn(mpn)
        return ('guess', 'https://www.ti.com/lit/gpn/' + g) if g else ('', '')
    if c == 'adi':
        if host == 'www.analog.com':
            return 'link', 'https://www.analog.com' + u.path
        if host == 'datasheets.maximintegrated.com':
            return 'link', 'https://www.analog.com/media/en/technical-documentation/data-sheets/' + u.path.rsplit('/', 1)[1]
        return '', ''
    if c == 'st':
        if host == 'www.st.com':
            return 'link', url
        b = st_base(mpn)
        return ('guess', 'https://www.st.com/resource/en/datasheet/%s.pdf' % b) if b else ('', '')
    if c == 'renesas':
        if host == 'www.renesas.com':
            return 'link', url
        return '', ''
    return '', ''
