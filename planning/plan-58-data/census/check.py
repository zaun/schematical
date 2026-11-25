#!/usr/bin/env python3
"""plan-58-A: validates one census page: python3 -I check.py <file.json> <maker> <circuit>.

Exit 0 and prints "<maker> <circuit> count N rows M" when the page is what its URL asked for: every product
row has the Circuit and a manufacturer name of that maker, and the row count is DigiKey's count (a page holds
at most 100). A page of 0 results is {"n":"0","f":[]}. Exit 1 otherwise: Safari sometimes hands back another
tab's page, so list.sh retries it.
"""
import json
import sys

MAKER_NAMES = {'C&K': ('C&K', 'Littelfuse/C&K'), 'NKK': ('NKK Switches',), 'CIT': ('CIT Relay and Switch',),
               'E-Switch': ('E-Switch',)}


def main(path, maker, circuit):
    try:
        j = json.load(open(path))
    except (OSError, ValueError):
        return 1
    rows = j.get('p', [])
    if int(j['n'].replace(',', '')) != len(rows) or len(rows) > 100:
        return 1
    for p in rows:
        name = p[0]['value']['manufacturer']['name']
        cell = p[8]['value']['value']       # Circuit
        if name not in MAKER_NAMES[maker] or cell != circuit:
            return 1
    print(maker, circuit, 'count', j['n'], 'rows', len(rows))
    return 0


sys.exit(main(*sys.argv[1:4]))
