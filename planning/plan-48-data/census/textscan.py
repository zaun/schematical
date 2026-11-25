#!/usr/bin/env python3
"""plan-48-A phase 2: for each picked row (state `candidate`), whether its
datasheet prints the part number, and whether a status word stands next to it.
usage: python3 -I textscan.py <candidates.tsv> <pdf cache dir> <textscan.tsv> [states]
`states` (default `candidate`) is a comma list of candidates.tsv states to scan.

Text = `pdftotext -layout` of the cached PDF (`<md5 of url>.txt`), whitespace
removed, upper case (the same as select.py's `hit`). Columns:
  mpn, hit     1 = the part number stands verbatim
  scheme       the packing rule that matched when `hit` is 0 (plan-48-A § 4
               rule 2), else empty:
                 tr    ADI / Maxim: the number without `+` and without a
                       trailing T, -T, -REEL, -REEL7, -RL, -RL7, -R7, -R2
                       stands in the text, and the text has TAPEANDREEL or
                       DENOTESALEAD
                 pbf   ADI (Linear Technology): `#TRPBF` / `#TRMPBF` read as `#PBF`
                 nopb  TI: a `/NOPB` number matches without the slash
                 dash  ADI / Maxim: a `<base>+T` number whose `<base>-T` stands
                       in the text (the printed `-T` is the same part, tape
                       and reel; plan-48-A Corrections 2)
                 slash ST: an order-code shorthand `TL061CD/CDT` stands for
                       `TL061CD` and `TL061CDT` (the later part replaces the
                       tail it repeats; plan-48-A Corrections 2)
  dead         1 = OBSOLETE, DISCONTINUED, NRND, NOT RECOMMENDED FOR NEW
               (DESIGNS), END OF LIFE or LIFEBUY stands within 300 characters
               after an occurrence of the matched number (rule 3); for TI only
               right after it (`<part>[.A]<STATUS>`), since the addendum lists
               a row per orderable and a later word is the next row's
  context      the 80 characters around the first match (the first dead match
               when dead is 1), for the by-hand read of phase 2
Rows without a cached PDF get hit, scheme and dead empty."""
import csv
import os
import re
import subprocess
import sys

DEAD = re.compile('OBSOLETE|DISCONTINU|NRND|NOTRECOMMENDEDFORNEW|ENDOFLIFE|LIFEBUY')
SUFFIX = ('-REEL7', '-REEL', '-RL7', '-RL', '-R7', '-R2', '-T', 'T')


def text_of(cache, name):
    pdf = os.path.join(cache, name)
    txt = pdf[:-4] + '.txt'
    if not os.path.exists(txt):
        subprocess.run(['pdftotext', '-layout', pdf, txt], check=False, stderr=subprocess.DEVNULL)
    try:
        return ''.join(open(txt, encoding='utf-8', errors='replace').read().split()).upper()
    except OSError:
        return ''


def slash_codes(raw):
    """{expanded order code: the printed shorthand} for the tokens `A/B[/C]`
    of a raw text: B (C) repeats the tail of A and goes on, so the code is A
    with its longest tail that B starts with replaced by B."""
    out = {}
    for tok in re.findall(r'[A-Za-z0-9\-]+(?:/[A-Za-z0-9\-]+)+', raw):
        parts = tok.upper().split('/')
        a = parts[0]
        out.setdefault(a, tok.upper())
        for b in parts[1:]:
            for k in range(min(len(a), len(b)) - 1, 0, -1):
                if a.endswith(b[:k]):
                    out.setdefault(a + b[k:], tok.upper())
                    break
            else:
                out.setdefault(a + b, tok.upper())
    return out


def raw_of(cache, name):
    txt = os.path.join(cache, name)[:-4] + '.txt'
    try:
        return open(txt, encoding='utf-8', errors='replace').read()
    except OSError:
        return ''


def norm(s):
    return ''.join(s.split()).upper()


def variants(company, mpn):
    """(scheme, key) pairs, tried in order after the verbatim number."""
    m = norm(mpn)
    out = []
    if company == 'adi':
        base = m.replace('+', '')
        if '#TR' in base:
            out.append(('pbf', re.sub(r'#TRM?PBF$', '#PBF', base)))
        for suf in SUFFIX:
            if base.endswith(suf) and len(base) > len(suf) + 2:
                out.append(('tr', base[:-len(suf)]))
        out.append(('tr', base))
    elif company == 'ti' and '/' in m:
        out.append(('nopb', m.replace('/', '')))
    return out


def scan(text, company, mpn, slash=None):
    key = norm(mpn)
    at = text.find(key) if key else -1
    hit, scheme, used = 0, '', key
    if at >= 0:
        hit = 1
    else:
        if company == 'adi' and key.endswith('+T') and key[:-2] + '-T' in text:
            at, scheme, used = text.find(key[:-2] + '-T'), 'dash', key[:-2] + '-T'
        elif company == 'st' and slash and key in slash and text.find(slash[key]) >= 0:
            at, scheme, used = text.find(slash[key]), 'slash', slash[key]
    if at < 0:
        for name, k in variants(company, mpn):
            if not k:
                continue
            if name == 'tr' and not ('TAPEANDREEL' in text or 'DENOTESALEAD' in text):
                continue
            at = text.find(k)
            if at >= 0:
                scheme, used = name, k
                break
    if at < 0:
        return 0, '', 0, ''
    ctx_at = at
    dead = 0
    start = 0
    while True:
        i = text.find(used, start)
        if i < 0:
            break
        after = text[i + len(used):i + len(used) + 300]
        if company == 'ti':
            # a TI orderable-addendum row reads `<part>[.A]<STATUS>PRODUCTION...`:
            # the status word stands right after the part; a word further on
            # is the next row's (or another orderable that starts with this one)
            d = re.match(r'(\.[A-Z])?(' + DEAD.pattern + ')', after)
        else:
            d = DEAD.search(after)
        if d:
            dead, ctx_at = 1, i
            break
        start = i + 1
    lo = max(0, ctx_at - 40)
    return hit, scheme, dead, text[lo:ctx_at + len(used) + 40]


def main():
    cand, cache, out = sys.argv[1:4]
    states = set((sys.argv[4] if len(sys.argv) > 4 else 'candidate').split(','))
    texts, slashes = {}, {}
    n = 0
    with open(out, 'w', encoding='utf-8', newline='') as f:
        f.write('mpn\thit\tscheme\tdead\tcontext\n')
        for r in csv.DictReader(open(cand, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE):
            if r['state'] not in states:
                continue
            n += 1
            name = r['pdf']
            if not name or not os.path.exists(os.path.join(cache, name)):
                f.write('%s\t\t\t\t\n' % r['mpn'])
                continue
            if name not in texts:
                texts[name] = text_of(cache, name)
            if name not in slashes:
                slashes[name] = slash_codes(raw_of(cache, name)) if r['company'] == 'st' else None
            hit, scheme, dead, ctx = scan(texts[name], r['company'], r['mpn'], slashes[name])
            f.write('%s\t%d\t%s\t%d\t%s\n' % (r['mpn'], hit, scheme, dead, ctx.replace('\t', ' ')))
    print('%d rows scanned' % n)


if __name__ == '__main__':
    main()
