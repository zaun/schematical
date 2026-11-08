#!/usr/bin/env python3
"""Parse TI's Package Option Addendum (orderable part, status, package, code, pins)
out of `pdftotext -layout` text files.

usage: poa.py <txt>...   -> TSV on stdout: pdf  part  status  pkgtype  code  pins
"""
import os, re, sys

# "ADS1115IDGSR   Active   Production   VSSOP (DGS) | 10   2500 | LARGE T&R ..."
NEW = re.compile(r'^\s*([A-Z0-9][A-Z0-9\-/.#+]*)\s+(Active|Obsolete|NRND|Preview|Last Time Buy|Last|LIFEBUY|Discontinued)\s+(?:Production|Preproduction|Pre-production|Samples)?\s*([A-Za-z0-9\-/ ]+?)\s+\(([A-Z0-9]+)\)\s*\|\s*(\d+)', re.I)
# older format: "ADS1115IDGSR  ACTIVE  VSSOP  DGS  10  2500 ..."
HEAD = re.compile(r'^\s*([A-Z0-9][A-Z0-9\-/.#+]*)\s+(Active|Obsolete|NRND|Preview|Last|LIFEBUY|Discontinued)\s+(?:Production|Preproduction|Pre-production|Samples)\s+([A-Za-z0-9\-/]+)\s{2,}\d', re.I)
TAIL = re.compile(r'^\s*([A-Za-z0-9\-/ ]*?)\s*\(([A-Z0-9]+)\)\s*\|\s*(\d+)')
OLD = re.compile(r'^\s*([A-Z0-9][A-Z0-9\-/.#+]*)\s+(ACTIVE|OBSOLETE|NRND|PREVIEW|LIFEBUY|LTB)\s+([A-Z0-9\-]+(?: [A-Z0-9\-]+)?)\s+([A-Z0-9]{1,5})\s+(\d+)\s+\d+', re.I)


def parse(path):
    rows = []
    txt = open(path, errors='replace').read()
    i = txt.find('PACKAGE OPTION ADDENDUM')
    if i < 0:
        i = txt.find('Orderable part number')
    if i < 0:
        i = txt.find('Orderable Device')
    if i < 0:
        return rows
    lines = txt[i:].splitlines()
    for li, line in enumerate(lines):
        m = NEW.match(line) or OLD.match(line)
        if not m:
            # the package name wrapped: "SOT-23-" here, "THIN (DDC) | 6" below
            h = HEAD.match(line)
            if h:
                for nxt in lines[li + 1:li + 4]:
                    t = TAIL.search(nxt)
                    if t:
                        part, st, pt1 = h.groups()
                        pt = (pt1 + ('' if pt1.endswith('-') else ' ') + t.group(1)).strip()
                        m = True
                        code, pins = t.group(2), t.group(3)
                        break
            if m is not True:
                continue
        else:
            part, st, pt, code, pins = m.groups()
        if True:
            if part.endswith('.B') or part.endswith('.A'):
                continue
            st = {'last': 'Last Time Buy', 'ltb': 'Last Time Buy', 'lifebuy': 'Last Time Buy'}.get(st.lower(), st.capitalize() if st.isupper() else st)
            rows.append((part, st, pt.strip(), code, pins))
    return rows


def main():
    for f in sys.argv[1:]:
        pdf = os.path.basename(f).rsplit('.', 1)[0] + '.pdf'
        seen = set()
        for r in parse(f):
            if r[0] in seen:
                continue
            seen.add(r[0])
            print('\t'.join((pdf,) + r))


if __name__ == '__main__':
    main()
