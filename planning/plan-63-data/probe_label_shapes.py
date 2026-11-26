#!/usr/bin/env python3 -I
"""Probe: does a label's shape (input / output / bidirectional / tri_state / passive) change what
KiCad 10.0.8's ERC reports? One net per (pin type, global-label shape), the pin wired to the label.
Usage: python3 -I probe_label_shapes.py <outdir>"""
import json, subprocess, sys, os
KICAD = os.path.expanduser("~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli")
out = sys.argv[1]; os.makedirs(out, exist_ok=True)
PINS = ["input", "output", "passive", "power_in"]
SHAPES = ["input", "output", "bidirectional", "tri_state", "passive"]
lib = "\n".join(f'''(symbol "P:{t}" (exclude_from_sim no) (in_bom yes) (on_board yes)
 (property "Reference" "R" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (property "Value" "{t}" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (symbol "{t}_1_1" (pin {t} line (at 0 0 0) (length 2.54) (name "p" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))))''' for t in PINS)
body = []; n = 0; cases = []
for i, t in enumerate(PINS):
    for j, shape in enumerate(SHAPES):
        n += 1; y = 30.48 + 15.24 * (i * len(SHAPES) + j)
        cases.append((t, shape, y))
        body.append(f'''(symbol (lib_id "P:{t}") (at 50.8 {y} 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no)
 (uuid "00000000-0000-4000-8000-{n:012d}")
 (property "Reference" "U{n}" (at 50.8 {y-2} 0) (effects (font (size 1.27 1.27))))
 (property "Value" "{t}" (at 50.8 {y+2} 0) (effects (font (size 1.27 1.27))))
 (pin "1" (uuid "10000000-0000-4000-8000-{n:012d}"))
 (instances (project "labels" (path "/00000000-0000-4000-8000-ffffffffffff" (reference "U{n}") (unit 1)))))''')
        body.append(f'(wire (pts (xy 50.8 {y}) (xy 71.12 {y})) (stroke (width 0) (type default)) (uuid "20000000-0000-4000-8000-{n:012d}"))')
        body.append(f'(global_label "N{n}" (shape {shape}) (at 71.12 {y} 0) (effects (font (size 1.27 1.27))) (uuid "30000000-0000-4000-8000-{n:012d}") (property "Intersheetrefs" "" (at 71.12 {y} 0) (effects (font (size 1.27 1.27)) hide)))')
sch = f'''(kicad_sch (version 20231120) (generator "probe") (uuid "00000000-0000-4000-8000-ffffffffffff") (paper "A0")
 (lib_symbols {lib})
 {chr(10).join(body)}
 (sheet_instances (path "/" (page "1"))))'''
path = os.path.join(out, "labels.kicad_sch"); open(path, "w").write(sch)
rep = os.path.join(out, "labels.json")
subprocess.run([KICAD, "sch", "erc", "--format", "json", "--severity-all", "-o", rep, path], check=True, capture_output=True)
data = json.load(open(rep))
rows = {}
for s in data["sheets"]:
    for v in s["violations"]:
        if v["type"] in ("endpoint_off_grid", "lib_symbol_issues"): continue
        rows.setdefault(round(v["items"][0]["pos"]["y"], 1), set()).add((v["type"], v["severity"]))
for t, shape, y in cases:
    print(f"{t:9s} wired to a {shape:13s} global label: {sorted(rows.get(round(y,1), []))}")
