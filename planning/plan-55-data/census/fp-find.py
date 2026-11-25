#!/usr/bin/env python3
"""plan-55-F/G: find library footprints that may serve a land pattern (for the reuse pass).

usage: python3 -I fp-find.py [--pins N] [--pitch P] [--span S] [--id REGEX] [--style dual|quad|grid|explicit|header]

Lists footprints of Parts/footprints.json (and planning/plan-55-data/footprints-[FG]*.json) whose `pins` equal N, `pitch`
equals P (within 0.01 mm) and `span` (dual) equals S (within 0.06 mm), and whose id matches the regex, one per line:
id, style, pins, pitch, span (spanX x spanY for a quad), the pad size, and `centre` when it has one.
"""
import glob
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402

a = sys.argv[1:]


def opt(name):
    return a[a.index(name) + 1] if name in a else None


pins, pitch, span, ident, style = opt('--pins'), opt('--pitch'), opt('--span'), opt('--id'), opt('--style')
paths = [os.path.join(common.ROOT, 'schematical', 'schematical', 'Parts', 'footprints.json')]
paths += sorted(glob.glob(os.path.join(common.DATA, 'footprints-[FG]*.json')))
for p in paths:
    for o in json.load(open(p))['footprints']:
        if pins and o['pins'] != int(pins):
            continue
        if style and o['style'] != style:
            continue
        if ident and not re.search(ident, o['id']):
            continue
        if pitch and abs(o.get('pitch', -9) - float(pitch)) > 0.011:
            continue
        sp = o.get('span', o.get('spanX'))
        if span and (sp is None or abs(sp - float(span)) > 0.06):
            continue
        pad = o.get('pad', {})
        print(o['id'], o['style'], o['pins'], 'pitch', o.get('pitch'), 'span', o.get('span', f"{o.get('spanX')}x{o.get('spanY')}"),
              'pad', f"{pad.get('width')}x{pad.get('height')}", 'centre' if 'centre' in o else '', 'slots' if 'slots' in o else '')
