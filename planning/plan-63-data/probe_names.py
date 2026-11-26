#!/usr/bin/env python3 -I
"""Probe: which KiCad 10.0.8 rule fires when a label and a power symbol, or two power symbols, or two labels
of one name share a wire. Usage: python3 -I probe_names.py <outdir>"""
import subprocess, json, os, re, sys
K = os.path.expanduser("~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli")
d = sys.argv[1]; os.makedirs(d, exist_ok=True)
n = [0]
lib = '''(symbol "P:passive" (exclude_from_sim no) (in_bom yes) (on_board yes)
 (property "Reference" "R" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (property "Value" "p" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (symbol "passive_1_1" (pin passive line (at 0 0 0) (length 2.54) (name "p" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))))
(symbol "P:power" (power) (exclude_from_sim no) (in_bom no) (on_board no)
 (property "Reference" "#PWR" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
 (property "Value" "p" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (symbol "power_1_1" (pin power_in line (at 0 0 0) (length 0) hide (name "p" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))))'''
def sym(l, x, y, val, ref):
    n[0] += 1; k = n[0]
    return f'''(symbol (lib_id "P:{l}") (at {x} {y} 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no) (uuid "00000000-0000-4000-8000-{k:012d}")
 (property "Reference" "{ref}" (at {x} {y-2} 0) (effects (font (size 1.27 1.27)))) (property "Value" "{val}" (at {x} {y+2} 0) (effects (font (size 1.27 1.27))))
 (pin "1" (uuid "10000000-0000-4000-8000-{k:012d}")) (instances (project "names" (path "/00000000-0000-4000-8000-ffffffffffff" (reference "{ref}") (unit 1)))))'''
def wire(x1, y1, x2, y2):
    n[0] += 1; return f'(wire (pts (xy {x1} {y1}) (xy {x2} {y2})) (stroke (width 0) (type default)) (uuid "20000000-0000-4000-8000-{n[0]:012d}"))'
def label(name, x, y):
    n[0] += 1; return f'(label "{name}" (at {x} {y} 0) (effects (font (size 1.27 1.27))) (uuid "30000000-0000-4000-8000-{n[0]:012d}"))'
rows = [("power +5V and label VCC on one wire", [sym("power", 50.8, 0, "+5V", "#PWR01"), wire(50.8, 0, 71.12, 0), sym("passive", 71.12, 0, "p", "R1"), label("VCC", 60.96, 0)]),
        ("power +5V and power +3V3 on one wire", [sym("power", 50.8, 0, "+5V", "#PWR02"), wire(50.8, 0, 71.12, 0), sym("power", 71.12, 0, "+3V3", "#PWR03")]),
        ("two labels X and Y on one wire (see probe_wires_labels)", [sym("passive", 50.8, 0, "p", "R2"), wire(50.8, 0, 71.12, 0), sym("passive", 71.12, 0, "p", "R3"), label("X", 55.88, 0), label("Y", 66.04, 0)]),
        ("two labels of one name X on one wire", [sym("passive", 50.8, 0, "p", "R4"), wire(50.8, 0, 71.12, 0), sym("passive", 71.12, 0, "p", "R5"), label("X", 55.88, 0), label("X", 66.04, 0)]),
        ("label +5V alone on a wire with a pin (the power symbol +5V is on another row)", [sym("passive", 50.8, 0, "p", "R6"), wire(50.8, 0, 71.12, 0), label("+5V", 71.12, 0)])]
body = []
for k, (desc, items) in enumerate(rows):
    dy = 30.48 + 25.4 * k
    for it in items:
        body.append(re.sub(r'(\(at |\(xy )(-?[\d.]+) (-?[\d.]+)', lambda m: f'{m.group(1)}{m.group(2)} {float(m.group(3)) + dy:.4f}', it))
p = d + "/names.kicad_sch"
open(p, "w").write(f'(kicad_sch (version 20231120) (generator "p") (uuid "00000000-0000-4000-8000-ffffffffffff") (paper "A0") (lib_symbols {lib}) {" ".join(body)} (sheet_instances (path "/" (page "1"))))')
subprocess.run([K, "sch", "erc", "--format", "json", "--severity-all", "-o", d + "/o.json", p], check=True, capture_output=True)
j = json.load(open(d + "/o.json"))
for k, (desc, _) in enumerate(rows):
    y0 = 30.48 + 25.4 * k
    got = sorted({(v["type"], v["severity"], v["description"]) for s in j["sheets"] for v in s["violations"]
                  if v["type"] not in ("lib_symbol_issues", "endpoint_off_grid") and y0 - 3 <= v["items"][0]["pos"]["y"] <= y0 + 3})
    print(f"{desc}:")
    for g in got: print("   ", g)
