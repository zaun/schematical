"""plan-47 census: shared code. Reads the listing (raw/*.jsonl[.gz]), names
each row's company, class and scope, drops duplicates and resolves the
maker-site datasheet URL."""
import collections
import glob
import gzip
import json
import os
import re
import urllib.parse

TYPES = ['inductor', 'fuse', 'ptc', 'relay', 'buzzer']

# DigiKey maker name prefix -> company slug (the DigiKey ids of one company
# share a slug).
COMPANIES = [('Bourns', 'bourns'), ('Murata', 'murata'), ('Würth', 'wurth'), ('Littelfuse', 'littelfuse'),
             ('Eaton', 'eaton'), ('SCHURTER', 'schurter'), ('Bel Fuse', 'belfuse'), ('TE Connectivity', 'te'),
             ('Franjobaim', 'franjobaim'), ('Panasonic', 'panasonic'), ('Same Sky', 'samesky'),
             ('PUI Audio', 'pui'), ('Mallory', 'mallory')]

# The three companies of each type, in rank order (plan-47-A section 2).
MAKERS = {'inductor': ['bourns', 'murata', 'wurth'], 'fuse': ['littelfuse', 'eaton', 'schurter'],
          'ptc': ['littelfuse', 'bourns', 'belfuse'], 'relay': ['te', 'franjobaim', 'panasonic'],
          'buzzer': ['samesky', 'pui', 'mallory']}

# Class quotas per (type): each company gets 200 rows of a type; a class
# short of rows hands its unused quota to the other classes in this order.
QUOTA = {'inductor': [('smd', 140), ('tht', 60)],
         'fuse': [('smd', 70), ('tht', 60), ('cartridge', 70)],
         'ptc': [('smd', 110), ('tht', 90)],
         'relay': [('power', 70), ('signal', 60), ('ssr', 40), ('auto', 15), ('reed', 15)],
         'buzzer': [('indicator', 100), ('transducer', 100)]}
PER_MAKER = 200
RELAY_CLASS = {'189': 'signal', '188': 'power', '964': 'reed', '183': 'ssr', '962': 'auto'}
# Relay terminations that solder to a board.
RELAY_TERMS = {'PC Pin', 'Gull Wing', 'PCB', 'SMD (SMT) Tab', 'J Lead', 'PC Pin, Quick Connect - 0.250" (6.3mm)'}

PANASONIC_DIR = {'signal': 'signal', 'power': 'power', 'ssr': 'photomos', 'auto': 'vehicle', 'reed': 'signal'}
PANASONIC_DIRS = ['signal', 'power', 'photomos', 'vehicle', 'interception']

MAKER_HOSTS = {'bourns': ['www.bourns.com'], 'murata': ['pim.murata.com', 'www.murata.com'],
               'wurth': ['www.we-online.com'], 'littelfuse': ['www.littelfuse.com'], 'eaton': ['www.eaton.com'],
               'schurter': ['www.schurter.com'], 'belfuse': ['www.belfuse.com'], 'te': ['www.te.com'],
               'franjobaim': ['assets.franjobaim.com'],
               'panasonic': ['industry.panasonic.com', 'mediap.industry.panasonic.eu'],
               'samesky': ['www.sameskydevices.com'], 'pui': ['puiaudio.com', 'api.puiaudio.com'],
               'mallory': ['mspindy.com']}


def company(maker):
    for prefix, slug in COMPANIES:
        if maker.startswith(prefix):
            return slug
    return ''


def read_raw(raw_dir):
    """Every listed row; `cut` = 1 when its piece hit the 300-result cap."""
    rows = []
    for path in sorted(glob.glob(os.path.join(raw_dir, '*.jsonl')) + glob.glob(os.path.join(raw_dir, '*.jsonl.gz'))):
        piece = os.path.basename(path).split('.')[0]
        err = os.path.join(raw_dir, piece + '.err')
        cut = int(os.path.exists(err) and 'cut at' in open(err, encoding='utf-8').read())
        opener = gzip.open if path.endswith('.gz') else open
        with opener(path, 'rt', encoding='utf-8') as f:
            for line in f:
                r = json.loads(line)
                r['cut'] = cut
                r['stock'] = r.get('stock') or 0
                r['company'] = company(r['maker'])
                rows.append(r)
    return rows


def classify(r):
    """(class, '') for a row in scope, ('', reason) for one out of scope."""
    p = r['params']
    t = r['type']
    mount = p.get('Mounting Type', '')
    if t == 'inductor':
        if 'Leads' in p.get('Package / Case', '') and '4 Leads' in p.get('Package / Case', ''):
            return '', 'more than two terminals'
        return ('smd' if mount == 'Surface Mount' else 'tht'), ''
    if t == 'fuse':
        if mount == 'Requires Holder':
            return 'cartridge', ''
        return ('smd' if mount == 'Surface Mount' else 'tht'), ''
    if t == 'ptc':
        if p.get('Package / Case', '').startswith('4-SMD'):
            return '', 'more than two terminals'
        return ('smd' if mount == 'Surface Mount' else 'tht'), ''
    if t == 'relay':
        cls = RELAY_CLASS[str(r['category'])]
        if cls != 'ssr':
            term = p.get('Termination Style', '')
            # DigiKey gives the automotive relays (962) no termination; the
            # listing took only their through-hole and surface-mount parts.
            if term not in RELAY_TERMS and not (cls == 'auto' and term in ('', '-')):
                return '', 'termination'
            if 'VAC' in p.get('Coil Voltage', '') or p.get('Coil Voltage', '-') == '-':
                return '', 'coil voltage'
            if p.get('Contact Form', '-') == '-':
                return '', 'contact form'
        elif p.get('Package / Case', '') in ('Module', '-'):
            return '', 'termination'
        return cls, ''
    if t == 'buzzer':
        if p.get('Termination', '') == 'Wire Leads':
            return '', 'termination'
        return ('indicator' if p.get('Driver Circuitry', '').startswith('Indicator') else 'transducer'), ''
    return '', 'type'


# The parameter that names a row's value in candidates.tsv.
VALUE = {'inductor': 'Inductance', 'fuse': 'Current Rating (Amps)', 'ptc': 'Current - Hold (Ih) (Max)',
         'buzzer': 'Frequency'}


def value(r):
    p = r['params']
    if r['type'] == 'relay':
        return p.get('Coil Voltage') or p.get('Voltage - Input', '')
    return p.get(VALUE[r['type']], '')


def form(r):
    """A relay's contact form or circuit; a buzzer's driver; else ''."""
    p = r['params']
    if r['type'] == 'relay':
        return p.get('Contact Form') or p.get('Circuit', '')
    if r['type'] == 'buzzer':
        return p.get('Technology', '')
    if r['type'] == 'fuse':
        return p.get('Response Time', '')
    return ''


def dedupe_key(r):
    """Rows equal in every DigiKey parameter differ only in packing."""
    return (r['company'], r['type'], r['series'], json.dumps(r['params'], sort_keys=True, ensure_ascii=False))


def _norm(url):
    if url.startswith('//'):
        url = 'https:' + url
    return url.strip()


def own_url(r):
    """The row's DigiKey link rewritten to the maker's own site, or ''."""
    url = _norm(r['datasheet'])
    if not url:
        return ''
    u = urllib.parse.urlparse(url)
    host, path, c = u.netloc.lower(), u.path, r['company']
    if c == 'bourns' and host in ('www.bourns.com', 'bourns.com'):
        return 'https://www.bourns.com' + path
    if c == 'murata' and host in MAKER_HOSTS['murata']:
        return url
    if c == 'wurth':
        if host == 'www.we-online.com':
            return url
        if host in ('katalog.we-online.de', 'www.we-online.de'):
            return 'https://www.we-online.com/components/products/datasheet/' + path.rsplit('/', 1)[1]
        return ''
    if c == 'littelfuse' and host == 'www.littelfuse.com':
        return url
    if c == 'eaton' and host in ('www.eaton.com', 'www.eaton.com.cn'):
        return 'https://www.eaton.com' + path
    if c == 'schurter' and host in ('www.schurter.com', 'us.schurter.com'):
        return 'https://www.schurter.com' + path
    if c == 'belfuse' and host in ('www.belfuse.com', 'belfuse.com'):
        return 'https://www.belfuse.com' + path
    if c == 'te' and host in ('www.te.com', 'www-qa.te.com'):
        # resolve_urls keeps one URL per document (DocNm): te.com refuses a
        # document URL without a part context.
        return 'https://www.te.com' + path + ('?' + u.query if u.query else '')
    if c == 'franjobaim' and host == 'assets.franjobaim.com':
        return url
    if c == 'panasonic':
        # Panasonic's own file names (mech_eng_<series>.pdf, semi_eng_<series>.pdf)
        # survive in DigiKey's copies and on the old www3.panasonic.biz host;
        # the maker's CDN serves them under one of PANASONIC_DIRS. The class
        # picks the first directory; urlcheck.py tries the others.
        name = urllib.parse.unquote(path.rsplit('/', 1)[-1])
        # DigiKey's copies add a "255_" prefix and a " (1)" suffix.
        name = re.sub(r' \(\d+\)(?=\.pdf$)', '', re.sub(r'^\d+_', '', name))
        if re.fullmatch(r'(mech|semi)_eng_[a-z0-9_]+\.pdf', name):
            if host == 'mediap.industry.panasonic.eu':
                return url
            m = re.search(r'/control/relay/([a-z-]+)/catalog/', path)
            d = m.group(1) if m else PANASONIC_DIR[r.get('class') or RELAY_CLASS[str(r['category'])]]
            return 'https://industry.panasonic.com/ac/cdn/e/control/relay/' + d + '/catalog/' + name
        return ''
    if c == 'samesky' and host in ('www.sameskydevices.com', 'www.cuidevices.com'):
        return 'https://www.sameskydevices.com' + path.replace('/resource/digikeypdf/', '/resource/')
    if c == 'pui' and host in MAKER_HOSTS['pui']:
        return url
    if c == 'mallory' and host in ('mspindy.com', 'www.mspindy.com'):
        return 'https://mspindy.com' + path
    return ''


def guess_url(r):
    """A maker-site URL built from the part number, for the makers that
    publish one PDF per part."""
    c, mpn = r['company'], r['mpn']
    if c == 'wurth':
        return 'https://www.we-online.com/components/products/datasheet/' + mpn + '.pdf'
    if c == 'pui':
        return 'https://puiaudio.com/file/specs-' + mpn + '.pdf'
    if c == 'mallory':
        return 'https://mspindy.com/spec-sheets/' + mpn + '.pdf'
    if c == 'samesky':
        return 'https://www.sameskydevices.com/product/resource/' + mpn.lower() + '.pdf'
    if c == 'panasonic' and r.get('class') in ('signal', 'power', 'auto'):
        # mech_eng_<series>.pdf; a series DigiKey names by its part-number
        # prefix (ADJ, ALDP, ACJ) drops the leading A (mech_eng_dj.pdf).
        code = re.sub(r'[^a-z0-9]', '', r['series'].lower())
        if len(code) >= 3 and code.startswith('a'):
            code = code[1:]
        if code:
            return ('https://industry.panasonic.com/ac/cdn/e/control/relay/' + PANASONIC_DIR[r['class']]
                    + '/catalog/mech_eng_' + code + '.pdf')
    if c == 'bourns' and r['type'] == 'inductor' and re.fullmatch(r'[A-Za-z0-9-]+', r['series']):
        return 'https://www.bourns.com/docs/Product-Datasheets/' + r['series'] + '.pdf'
    return ''


def resolve_urls(rows):
    """Sets r['url'] and r['via'] ('link', 'sibling', 'guess' or '')."""
    by_series = collections.defaultdict(collections.Counter)
    te_doc = {}
    for r in rows:
        r['url'] = own_url(r)
        if r['company'] == 'te' and r['url']:
            doc = urllib.parse.parse_qs(urllib.parse.urlparse(r['url']).query).get('DocNm', [''])[0]
            if doc:
                te_doc[doc] = min(te_doc.get(doc, r['url']), r['url'])
                r['te_doc'] = doc
            else:
                r['url'] = ''
    for r in rows:
        if r.get('te_doc'):
            r['url'] = te_doc[r['te_doc']]
    for r in rows:
        r['via'] = 'link' if r['url'] else ''
        if r['url'] and r['series'] not in ('', '-', '*'):
            by_series[(r['company'], r['type'], r['series'])][r['url']] += 1
    for r in rows:
        if r['url']:
            continue
        g = guess_url(r)
        sib = by_series.get((r['company'], r['type'], r['series']))
        if g:
            r['url'], r['via'] = g, 'guess'
        elif sib:
            r['url'], r['via'] = sorted(sib.items(), key=lambda kv: (-kv[1], kv[0]))[0][0], 'sibling'


def in_scope(rows):
    """Rows of the fifteen (type, company) shares, classed, de-duplicated:
    sets r['class'], r['state'] ('' or 'out') and r['reason']."""
    best = {}
    for r in rows:
        r['class'], r['reason'] = classify(r)
        r['state'] = 'out' if r['reason'] else ''
        if r['company'] not in MAKERS[r['type']]:
            r['state'], r['reason'] = 'out', 'maker'
        if r['stock'] <= 0:
            r['state'], r['reason'] = 'out', 'no stock figure'
        if r['state']:
            continue
        k = dedupe_key(r)
        if k not in best or (r['stock'], r['mpn']) > (best[k]['stock'], best[k]['mpn']):
            best[k] = r
    for r in rows:
        if not r['state'] and best[dedupe_key(r)] is not r:
            r['state'], r['reason'] = 'out', 'duplicate'
    return rows
