#!/usr/bin/env python3
"""plan-47-B section 3: the verdicts of the inductor, fuse and PTC rows of the download set.

usage: python3 -I scheme-check.py [--facts] [--apply] [--list] [--sample] <type>...
       (type: inductor, fuse, ptc)

For every download-set row (candidates.tsv state `candidate` or `reserve`) of
the types, in the order the checks apply:

  1. no specs row, or its `scheme` empty: unchecked (not a verdict);
  2. no PDF: skip, `no datasheet on the maker's site`;
  3. `dead` = 1, or the PDF is in obsolete-series.tsv (the datasheet says the whole series is
     obsolete or not recommended for new designs): out, `obsolete` (B clears `dead` to 0 where
     the datasheet says the part is alive);
  4. the specs row's `pinout` is `-`: skip, `no pinout in the datasheet`; `out`
     (more than two terminals) is written as `pinout` = `out`;
  5. the part number is not printed verbatim (`hit` 0) and its specs row's scheme
     does not match (or is `verbatim` or `-`): skip, `not in the datasheet's
     ordering table`; a scheme value group (specs47.py) that disagrees with the row's
     DigiKey value: skip, `not in the datasheet's ordering table`;
  6. with --facts, a DigiKey fact the specs row contradicts: skip, `fact not in
     datasheet`; a required fact DigiKey leaves blank: skip, `fact not given`.

Prints `<n> rows: <k> ok, <u> unchecked` (and the skip / out counts by reason).
--apply writes the verdicts to candidates.tsv columns `verdict` (ok / skip / out)
and `vreason`; --list prints each row that is not ok; --sample prints 20 ok rows
per maker and type with their scheme (a hand check against the PDF).
"""
import collections
import os
import random
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import specs47 as sp  # noqa: E402

NOT_IN_TABLE = "not in the datasheet's ordering table"


def fact_problem(r, s, params):
    """A reason when a fact of the row contradicts the specs row, else ''."""
    t = r['type']
    v = sp.row_value(r)
    if v is None:
        return 'fact not given'
    if not sp.in_values(s['values'], v):
        return 'fact not in datasheet'
    if t == 'inductor':
        tol = params.get('Tolerance', '')
        if s['tols'] not in ('', '-') and tol not in ('', '-') and tol not in s['tols'].split('|'):
            return 'fact not in datasheet'
        for col, name in (('amps_max', 'Current Rating (Amps)'), ('isat_max', 'Current - Saturation (Isat)'),
                          ('dcr_max', 'DC Resistance (DCR)')):
            figure = sp.parse(params.get(name))
            if s[col] not in ('', '-') and figure is not None and figure > float(s[col]) * (1 + 1e-6):
                return 'fact not in datasheet'
        shield = params.get('Shielding', '')
        if s['shield'] not in ('', '-') and shield not in ('', '-') and shield.lower() not in s['shield'].split('|'):
            return 'fact not in datasheet'
    elif t == 'fuse':
        speed = {'Fast Blow': 'fast', 'Slow Blow': 'slow', 'Medium Blow': 'medium'}.get(params.get('Response Time', ''), '')
        if speed and s['speed'] not in ('', '-') and speed not in s['speed'].split('|'):
            return 'fact not in datasheet'
        for col, name in (('vac', 'Voltage Rating - AC'), ('vdc', 'Voltage Rating - DC'),
                          ('break_max', 'Breaking Capacity @ Rated Voltage')):
            figure = sp.parse(params.get(name))
            if s[col] not in ('', '-') and figure is not None and figure > float(s[col]) * (1 + 1e-6):
                return 'fact not in datasheet'
    else:
        for col, name in (('vmax', 'Voltage - Max'), ('imax', 'Current - Max')):
            figure = sp.parse(params.get(name))
            if s[col] not in ('', '-') and figure is not None and figure > float(s[col]) * (1 + 1e-6):
                return 'fact not in datasheet'
        if sp.parse(params.get('Current - Trip (It)')) is None:
            return 'fact not given'
    return ''


_OBSOLETE = None


def obsolete_pdfs():
    """The PDFs whose datasheet marks the whole series obsolete or not recommended for new
    designs (obsolete-series.tsv, with the quoted text): every row of such a PDF is out."""
    global _OBSOLETE
    if _OBSOLETE is None:
        path = os.path.join(os.path.dirname(sp.CANDIDATES), 'obsolete-series.tsv')
        _OBSOLETE = {r['pdf'] for r in sp.read(path)} if os.path.exists(path) else set()
    return _OBSOLETE


def verdict(r, s, facts, params):
    """(verdict, reason) of a row with a specs row: ok, skip or out."""
    if not r['pdf']:
        return 'skip', "no datasheet on the maker's site"
    if r['dead'] == '1' or r['pdf'] in obsolete_pdfs():
        return 'out', 'obsolete'
    if s['pinout'] == 'out':
        return 'out', 'more than two terminals'
    if s['pinout'] == '-':
        return 'skip', 'no pinout in the datasheet'
    scheme = s['scheme']
    if r['hit'] != '1':
        if scheme in ('verbatim', '-'):
            return 'skip', NOT_IN_TABLE
        matched, value = sp.scheme_value(scheme, r['mpn'])
        if not matched:
            return 'skip', NOT_IN_TABLE
        rv = sp.row_value(r)
        if value is not None and rv is not None and not sp.close(value, rv):
            return 'skip', NOT_IN_TABLE
    if facts:
        reason = fact_problem(r, s, params)
        if reason:
            return 'skip', reason
    return 'ok', ''


def main(argv):
    flags = {a for a in argv if a.startswith('--')}
    types = [a for a in argv if not a.startswith('--')]
    bad = [t for t in types if t not in sp.TYPES]
    if not types or bad:
        print('usage: scheme-check.py [--facts] [--apply] [--list] [--sample] inductor|fuse|ptc ...')
        return 2
    rows = sp.read(sp.CANDIDATES)
    cols = list(rows[0].keys())
    for c in ('verdict', 'vreason'):
        if c not in cols:
            cols.append(c)
    specs = sp.load_specs()
    params = sp.params_by_mpn() if '--facts' in flags else {}
    total = ok = unchecked = 0
    reasons = collections.Counter()
    by_group = collections.defaultdict(list)
    for r in rows:
        r.setdefault('verdict', '')
        r.setdefault('vreason', '')
        if r['type'] not in types or not sp.in_set(r):
            continue
        total += 1
        s = specs.get(sp.key(r))
        if s is None or not s['scheme']:
            unchecked += 1
            r['verdict'] = r['vreason'] = ''
            if '--list' in flags:
                print('unchecked', r['mpn'], '|', r['series'], '|', r['pdf'], '|', r['package'])
            continue
        r['verdict'], r['vreason'] = verdict(r, s, '--facts' in flags, params.get(r['mpn'], {}))
        if r['verdict'] == 'ok':
            ok += 1
            by_group[(r['company'], r['type'])].append((r, s))
        else:
            reasons[(r['verdict'], r['vreason'])] += 1
            if '--list' in flags:
                print(r['verdict'], r['vreason'], '|', r['mpn'], '|', r['pdf'], '|', s['scheme'])
    summary = sys.stderr if '--sample' in flags else sys.stdout   # --sample: only the sample lines on stdout
    print(f"{total} rows: {ok} ok, {unchecked} unchecked", file=summary)
    for (v, reason), n in sorted(reasons.items()):
        print(f"  {v}: {reason}: {n}", file=summary)
    if '--sample' in flags:
        rng = random.Random(47)
        for k in sorted(by_group):
            for r, s in rng.sample(by_group[k], min(20, len(by_group[k]))):
                print(f"sample {r['company']} {r['type']} {r['mpn']} hit={r['hit']} value={r['value']!r} "
                      f"scheme={s['scheme']!r} page={s['page']}")
    if '--apply' in flags:
        sp.write(sp.CANDIDATES, rows, cols)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
