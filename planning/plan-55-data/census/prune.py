#!/usr/bin/env python3
"""plan-55-E § 3.2: the PDFs that stay in datasheets/powerswitch/.

usage: python3 -I prune.py [--dry-run]

Keeps each PDF an `add` row of verdicts.tsv cites. Deletes every other PDF of datasheets/powerswitch/<company>/ and
rewrites datasheets.tsv to the kept files' lines (a deleted PDF stays re-downloadable from the URL recorded in
candidates.tsv's `maker_url`). A PDF whose every part is obsolete would be deleted and recorded in
datasheets/obsolete-not-included.md; DigiKey lists only Active parts, so none is expected. Prints the counts.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402


def main():
    dry = '--dry-run' in sys.argv
    files = set()
    for company in common.COMPANIES:
        d = os.path.join(common.TREE, company)
        for name in os.listdir(d) if os.path.isdir(d) else []:
            if name.lower().endswith('.pdf'):
                files.add(os.path.join('datasheets', 'powerswitch', company, name))
    keep = {v['pdf'] for v in common.read_tsv(common.VERDICTS) if v['verdict'] == 'add' and v['pdf']}
    missing = sorted(p for p in keep if p not in files)
    for p in missing:
        print('missing (used but not in the tree):', p)
    drop = sorted(files - keep)
    print(f'{len(files)} files; {len(keep)} used by add rows; keep {len(keep & files)}, delete {len(drop)}')
    if dry:
        return 1 if missing else 0
    for p in drop:
        os.remove(os.path.join(common.ROOT, p))
    lines = open(common.PLACED).read().splitlines()
    kept = [x for x in lines if x.startswith('file\t') or x.split('\t')[0] in keep]
    with open(common.PLACED, 'w') as out:
        out.write('\n'.join(kept) + '\n')
    print('datasheets.tsv:', len(kept) - 1, 'lines')
    return 1 if missing else 0


if __name__ == '__main__':
    sys.exit(main())
