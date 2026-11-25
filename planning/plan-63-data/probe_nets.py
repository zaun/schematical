#!/usr/bin/env python3 -I
"""Probe: how many violations KiCad 10.0.8 reports on a net of three pins, and what a No Connect flag
exempts. Usage: python3 -I probe_nets.py <outdir>"""
import subprocess, json, os, sys
K = os.path.expanduser("~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli")
d = sys.argv[1]; os.makedirs(d, exist_ok=True)
n = [0]
def lib(t): return f'''(symbol "P:{t}" (exclude_from_sim no) (in_bom yes) (on_board yes)
 (property "Reference" "R" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (property "Value" "p" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (symbol "{t}_1_1" (pin {t} line (at 0 0 0) (length 2.54) (name "p" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))))'''
def part(t, x, y):
    n[0] += 1; k = n[0]
    return f'''(symbol (lib_id "P:{t}") (at {x} {y} 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no) (uuid "00000000-0000-4000-8000-{k:012d}")
 (property "Reference" "U{k}" (at {x} {y-2} 0) (effects (font (size 1.27 1.27)))) (property "Value" "p" (at {x} {y+2} 0) (effects (font (size 1.27 1.27))))
 (pin "1" (uuid "10000000-0000-4000-8000-{k:012d}")) (instances (project "nets" (path "/00000000-0000-4000-8000-ffffffffffff" (reference "U{k}") (unit 1)))))'''
def wire(x1, y1, x2, y2):
    n[0] += 1; return f'(wire (pts (xy {x1} {y1}) (xy {x2} {y2})) (stroke (width 0) (type default)) (uuid "20000000-0000-4000-8000-{n[0]:012d}"))'
def nc(x, y):
    n[0] += 1; return f'(no_connect (at {x} {y}) (uuid "40000000-0000-4000-8000-{n[0]:012d}"))'
def junction(x, y):
    n[0] += 1; return f'(junction (at {x} {y}) (diameter 0) (color 0 0 0 0) (uuid "50000000-0000-4000-8000-{n[0]:012d}"))'
rows = []
for desc, ts in [("3 outputs on one net", ["output"] * 3), ("2 outputs + 1 input", ["output", "output", "input"]),
                 ("3 power_out", ["power_out"] * 3), ("3 inputs", ["input"] * 3),
                 ("2 power_in + 1 input", ["power_in", "power_in", "input"]), ("3 power_in", ["power_in"] * 3)]:
    items = []
    for i, t in enumerate(ts):
        py = 7.62 * i
        items += [part(t, 50.8, py), wire(50.8, py, 100.33, py)]
    items.append(wire(100.33, 0, 100.33, 7.62 * (len(ts) - 1)))
    items += [junction(100.33, 7.62 * i) for i in range(1, len(ts) - 1)]
    rows.append((desc, items))
for desc, items in [
    ("input + NC flag on it", [part("input", 50.8, 0), nc(50.8, 0)]),
    ("power_in + NC flag on it", [part("power_in", 50.8, 0), nc(50.8, 0)]),
    ("output + NC flag on it", [part("output", 50.8, 0), nc(50.8, 0)]),
    ("input wired to an output, NC flag on the input", [part("input", 50.8, 0), nc(50.8, 0), wire(50.8, 0, 71.12, 0), part("output", 71.12, 0)]),
    ("input -> wire -> NC flag at the far end", [part("input", 50.8, 0), wire(50.8, 0, 71.12, 0), nc(71.12, 0)]),
    ("two inputs wired together, NC flag on one", [part("input", 50.8, 0), nc(50.8, 0), wire(50.8, 0, 71.12, 0), part("input", 71.12, 0)])]:
    rows.append((desc, items))
import re
body = []
for k, (desc, items) in enumerate(rows):
    dy = 30.48 + 30.48 * k
    for it in items:
        body.append(re.sub(r'(\(at |\(xy )(-?[\d.]+) (-?[\d.]+)', lambda m: f'{m.group(1)}{m.group(2)} {float(m.group(3)) + dy:.4f}', it))
types = ["input", "output", "power_in", "power_out"]
path = os.path.join(d, "nets.kicad_sch")
open(path, "w").write(f'(kicad_sch (version 20231120) (generator "p") (uuid "00000000-0000-4000-8000-ffffffffffff") (paper "A0") (lib_symbols {" ".join(lib(t) for t in types)}) {" ".join(body)} (sheet_instances (path "/" (page "1"))))')
subprocess.run([K, "sch", "erc", "--format", "json", "--severity-all", "-o", d + "/nets.json", path], check=True, capture_output=True)
j = json.load(open(d + "/nets.json"))
for k, (desc, _) in enumerate(rows):
    y0 = 30.48 + 30.48 * k
    got = [(v["type"], v["severity"]) for s in j["sheets"] for v in s["violations"]
           if v["type"] not in ("lib_symbol_issues", "endpoint_off_grid") and y0 - 3 <= v["items"][0]["pos"]["y"] <= y0 + 25]
    print(f"{desc}: {sorted(got)}")
