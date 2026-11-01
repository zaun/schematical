#!/usr/bin/env python3
"""notes.py <part.json> [<part.json> ...]

Replaces each "note": "@<key>" placeholder written by export_parts with the
datasheet comment the C compiler dropped, and removes note keys whose source
has no comment. Comment sources (all in backend/, plan-01-B section 3.6):

  1. Rows `{LOGIC_FAMILY_X, ...}, /* c */` inside a
     `static const (GtFamily|FfFamily) NAME[] = {` block   -> key NAME/X
  2. `case LOGIC_FAMILY_X:` followed by a `/* c */` line inside
     `family_timing` of 74_*.c                              -> key <file>/X
  3. A `/* c */` line directly above a declaration of NAME:
     `static const <type> NAME...` or `#define NAME(...)`   -> key NAME/
  4. `/* ---- NAME: text ---- */` section headers            -> key NAME/

The JSON file is rewritten in place; only note lines change. Exits non-zero
when a placeholder names a key no rule produced (a typo in an exporter), so
`grep -c '"@'` is 0 afterwards.
"""

import glob
import json
import os
import re
import sys

BACKEND = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
COMMENT = re.compile(r'/\*\s*(.*?)\s*\*/')


def clean(text):
    return ' '.join(text.split())


def harvest():
    notes = {}
    known = set()   # keys that exist even without a comment
    paths = sorted(glob.glob(os.path.join(BACKEND, '*.c')) + glob.glob(os.path.join(BACKEND, '*.inc')))
    for path in paths:
        name = os.path.basename(path)
        lines = open(path, encoding='utf-8').read().split('\n')
        block = None
        in_timing = False
        pending_case = None
        for i, line in enumerate(lines):
            # 1. family table rows
            m = re.match(r'\s*static const (?:GtFamily|FfFamily) (\w+)\[\] = \{', line)
            if m:
                block = m.group(1)
                continue
            if block is not None:
                if line.strip().startswith('};'):
                    block = None
                    continue
                m = re.match(r'\s*\{LOGIC_FAMILY_([A-Z]+),', line)
                if m:
                    key = '%s/%s' % (block, m.group(1))
                    known.add(key)
                    c = COMMENT.search(line)
                    if c:
                        notes[key] = clean(c.group(1))
                continue
            # 2. family_timing cases in the hand-written gate files
            if re.match(r'static \w+ family_timing\(', line):
                in_timing = True
            elif in_timing and line.startswith('}'):
                in_timing = False
            if in_timing:
                m = re.match(r'\s*case LOGIC_FAMILY_([A-Z]+):', line)
                if m:
                    pending_case = '%s/%s' % (name, m.group(1))
                    known.add(pending_case)
                    continue
                if pending_case is not None:
                    c = re.match(r'\s*/\*\s*(.*?)\s*\*/\s*$', line)
                    if c:
                        notes[pending_case] = clean(c.group(1))
                    if line.strip():
                        pending_case = None
                continue
            # 4. section headers
            m = re.match(r'\s*/\* ---- (\w+): (.*?) ---- \*/\s*$', line)
            if m:
                notes[m.group(1) + '/'] = clean(m.group(1) + ': ' + m.group(2))
                continue
            # 3. a comment line directly above a declaration
            m = re.match(r'(?:static const [\w ]+?\b(\w+)\s*(?:\[|=)|#define (\w+)\()', line)
            if m:
                decl = m.group(1) or m.group(2)
                known.add(decl + '/')
                prev = lines[i - 1] if i > 0 else ''
                c = re.match(r'\s*/\*\s*(.*?)\s*\*/\s*$', prev)
                if c and not prev.strip().startswith('/* ----'):
                    notes.setdefault(decl + '/', clean(c.group(1)))
    return notes, known


def main():
    notes, known = harvest()
    status = 0
    for path in sys.argv[1:]:
        out = []
        replaced = removed = 0
        for line in open(path, encoding='utf-8').read().split('\n'):
            m = re.search(r'"note":\s*"@([^"]*)"', line)
            if m:
                key = m.group(1)
                if key in notes:
                    line = line.replace(m.group(0), '"note": ' + json.dumps(notes[key], ensure_ascii=False))
                    replaced += 1
                elif key in known:
                    # no comment: drop the key (inline `, "note": ...` or a whole line)
                    new = re.sub(r',\s*"note":\s*"@[^"]*"', '', line)
                    if new == line:
                        new = re.sub(r'"note":\s*"@[^"]*",?\s*', '', line)
                        if not new.strip():
                            removed += 1
                            continue
                    line = new
                    removed += 1
                else:
                    print('%s: unknown note key %s' % (path, key), file=sys.stderr)
                    status = 1
            out.append(line)
        open(path, 'w', encoding='utf-8').write('\n'.join(out))
        print('%s: %d notes, %d removed' % (path, replaced, removed))
    return status


if __name__ == '__main__':
    sys.exit(main())
