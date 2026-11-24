#!/usr/bin/env python3
"""plan-55-C: the `pdftotext -layout` text of every placed datasheet, for the readers of the specs.

usage: python3 -I texts.py [<text dir>]      default /tmp/p55/text; writes <dir>/<company>/<name>.txt (a copy of the
                                             placed PDF's path with .txt), skipping a file already there
"""
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

out = sys.argv[1] if len(sys.argv) > 1 else '/tmp/p55/text'
n = 0
for r in common.read_tsv(common.PLACED):
    pdf = os.path.join(common.ROOT, r['file'])
    txt = os.path.join(out, r['file'][len('datasheets/powerswitch/'):][:-4] + '.txt')
    if os.path.exists(txt):
        continue
    os.makedirs(os.path.dirname(txt), exist_ok=True)
    subprocess.run(['pdftotext', '-q', '-layout', pdf, txt], check=False, timeout=300)
    n += 1
print(n, 'texts written to', out)
