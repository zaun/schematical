#!/usr/bin/env python3
"""plan-47-D section 3.2: footprint objects from a dimension table.

usage: python3 -I fp-gen.py <dims-X.tsv> > footprints-X.json

One two-pad `dual` object per row (comment lines start with `#`). Columns:
  id        <PREFIX>-... (IND, FUSE, PTC, RELAY, BUZZER; the id scheme of plan-47-D section 3.1)
  family    chip (surface mount), radial (standing, leaded), axial (leaded, lying)
  body_w    the body's size along the pad row, mm (the pads sit at x = -span/2 and +span/2)
  body_h    the body's size across the pad row, mm
  round     1 for a cylinder standing on the board (the body is drawn as a circle / ellipse)
  span      pad-centre distance, mm
  pad_w     pad size along the pad row, mm; pad_h across it
  shape     rect or round
  drill     the through-hole diameter, mm; empty for surface mount
  polarized optional: 1 gives pad 1 `pin1Shape` rect (a polarized part: the + pin of a buzzer)
  pdf       the datasheet's path; page: its page; arith: the arithmetic or the table row (goes to `note`)
Pin 1 is the left pad; there is no pin-1 mark (an unpolarized part).
"""
import csv
import json
import sys

KIND = {'IND': 'inductor', 'FUSE': 'fuse', 'PTC': 'PTC resettable fuse', 'RELAY': 'relay', 'BUZZER': 'buzzer'}


def num(text):
    return round(float(text), 4)


def obj(r):
    prefix = r['id'].split('-')[0]
    what = KIND.get(prefix, 'part')
    family = {'chip': 'surface-mount', 'radial': 'radial-lead', 'axial': 'axial-lead'}[r['family']]
    pad = {'width': num(r['pad_w']), 'height': num(r['pad_h']), 'shape': r['shape']}
    body = {'width': num(r['body_w']), 'height': num(r['body_h'])}
    if r.get('round') == '1':
        body['round'] = True
    o = {
        'id': r['id'],
        'description': f"Two-pad {family} {what}, {r['body_w']} x {r['body_h']} mm body, pads {r['span']} mm apart",
        'note': f"{r['pdf']} p.{r['page']}: {r['arith']}",
        'style': 'dual', 'pins': 2, 'perSide': 1, 'body': body, 'pitch': 1, 'span': num(r['span']), 'pad': pad,
    }
    if r.get('drill'):
        o['drill'] = num(r['drill'])
    if r.get('polarized') == '1':
        o['pin1Shape'] = 'rect'            # a polarized part (a buzzer): pad 1 is square, the + pin
    return o


def main(path):
    rows = [r for r in csv.DictReader((l for l in open(path, encoding='utf-8') if not l.startswith('#')),
                                      delimiter='\t', quoting=csv.QUOTE_NONE)]
    ids = [r['id'] for r in rows]
    dup = sorted({i for i in ids if ids.count(i) > 1})
    if dup:
        print('duplicate ids:', ', '.join(dup), file=sys.stderr)
        return 1
    objs = [obj(r) for r in rows]
    print('{"format":"schematical-footprints","version":1,"footprints":[')
    print(',\n'.join('  ' + json.dumps(o, ensure_ascii=False, separators=(',', ':')) for o in objs))
    print(']}')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1]))
