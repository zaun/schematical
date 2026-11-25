#!/usr/bin/env python3 -I
"""Probe: which KiCad 10.0.8 ERC rule fires for each wire / label / No Connect / reference case.

One schematic, one case per 20 mm row; prints the rule types (and severities) each row
produced. Usage: python3 -I probe_wires_labels.py <outdir>
"""
import json, subprocess, sys, os
KICAD = os.path.expanduser("~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli")
out = sys.argv[1]; os.makedirs(out, exist_ok=True)
U = [0]
def sym(lib, x, y, ref):
    U[0] += 1; n = U[0]
    return f'''(symbol (lib_id "P:{lib}") (at {x} {y} 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no)
 (uuid "00000000-0000-4000-8000-{n:012d}")
 (property "Reference" "{ref}" (at {x} {y-2} 0) (effects (font (size 1.27 1.27))))
 (property "Value" "{lib}" (at {x} {y+2} 0) (effects (font (size 1.27 1.27))))
 (pin "1" (uuid "10000000-0000-4000-8000-{n:012d}"))
 (instances (project "wires" (path "/00000000-0000-4000-8000-ffffffffffff" (reference "{ref}") (unit 1)))))'''
W = [0]
def wire(x1, y1, x2, y2):
    W[0] += 1
    return f'(wire (pts (xy {x1} {y1}) (xy {x2} {y2})) (stroke (width 0) (type default)) (uuid "20000000-0000-4000-8000-{W[0]:012d}"))'
def label(name, x, y, kind="label"):
    W[0] += 1
    return f'({kind} "{name}" (at {x} {y} 0) (effects (font (size 1.27 1.27))) (uuid "30000000-0000-4000-8000-{W[0]:012d}"))'
def nc(x, y):
    W[0] += 1
    return f'(no_connect (at {x} {y}) (uuid "40000000-0000-4000-8000-{W[0]:012d}"))'
def power(name, x, y):
    U[0] += 1; n = U[0]
    return f'''(symbol (lib_id "P:power") (at {x} {y} 0) (unit 1) (exclude_from_sim no) (in_bom no) (on_board no) (dnp no)
 (uuid "00000000-0000-4000-8000-{n:012d}")
 (property "Reference" "#PWR{n:02d}" (at {x} {y-2} 0) (effects (font (size 1.27 1.27)) hide))
 (property "Value" "{name}" (at {x} {y+2} 0) (effects (font (size 1.27 1.27))))
 (pin "1" (uuid "10000000-0000-4000-8000-{n:012d}"))
 (instances (project "wires" (path "/00000000-0000-4000-8000-ffffffffffff" (reference "#PWR{n:02d}") (unit 1)))))'''
cases = []  # (description, [items])
def case(desc, *items): cases.append((desc, items))
x0 = 50.8
case("wire, both ends free", wire(x0, 0, x0 + 20.32, 0))
case("passive pin -> wire -> free end", sym("passive", x0, 0, "R1"), wire(x0, 0, x0 + 20.32, 0))
case("passive pin -> wire -> passive pin", sym("passive", x0, 0, "R2"), wire(x0, 0, x0 + 20.32, 0), sym("passive", x0 + 20.32, 0, "R3"))
case("label alone", label("A", x0, 0))
case("label on a wire whose other end is free", label("B", x0, 0), wire(x0, 0, x0 + 20.32, 0))
case("label on a wire between two passive pins", sym("passive", x0, 0, "R4"), wire(x0, 0, x0 + 20.32, 0), sym("passive", x0 + 20.32, 0, "R5"), label("C", x0 + 10.16, 0))
case("labels C1 and C2 on one wire (multiple names)", sym("passive", x0, 0, "R6"), wire(x0, 0, x0 + 20.32, 0), sym("passive", x0 + 20.32, 0, "R7"), label("C1", x0 + 5.08, 0), label("C2", x0 + 15.24, 0))
case("label on two wires meeting at its point", sym("passive", x0, 0, "R8"), wire(x0, 0, x0 + 10.16, 0), label("D", x0 + 10.16, 0), wire(x0 + 10.16, 0, x0 + 20.32, 0), sym("passive", x0 + 20.32, 0, "R9"))
case("two nets labelled CLK and clk", sym("passive", x0, 0, "R10"), wire(x0, 0, x0 + 10.16, 0), label("CLK", x0 + 10.16, 0), sym("passive", x0, 5.08, "R11"), wire(x0, 5.08, x0 + 10.16, 5.08), label("clk", x0 + 10.16, 5.08))
case("power symbols +5V and +5v on separate nets", power("+5V", x0, 0), power("+5v", x0 + 20.32, 0))
case("label 'gnd' on a net, power symbol GND on another", power("GND", x0, 0), sym("passive", x0 + 20.32, 0, "R12"), wire(x0 + 20.32, 0, x0 + 30.48, 0), label("gnd", x0 + 30.48, 0))
case("No Connect on an unwired pin", sym("passive", x0, 0, "R13"), nc(x0, 0))
case("No Connect on a wired pin", sym("passive", x0, 0, "R14"), nc(x0, 0), wire(x0, 0, x0 + 20.32, 0), sym("passive", x0 + 20.32, 0, "R15"))
case("No Connect alone", nc(x0, 0))
case("two parts both U1", sym("passive", x0, 0, "U1"), sym("passive", x0 + 20.32, 0, "U1"))
case("part U? (unannotated)", sym("passive", x0, 0, "U?"))
case("wire T-junction without a junction dot", sym("passive", x0, -5.08, "R16"), wire(x0, -5.08, x0, 5.08), sym("passive", x0 + 10.16, 0, "R17"), wire(x0, 0, x0 + 10.16, 0))
case("two same-name labels J, each on a free-ended wire, no pins", wire(x0, 0, x0 + 10.16, 0), label("J", x0 + 10.16, 0), wire(x0 + 40.64, 0, x0 + 50.8, 0), label("J", x0 + 40.64, 0))
case("two same-name labels K alone, no wires, no pins", label("K", x0, 0), label("K", x0 + 30.48, 0))
case("two labels L1 and L2 joined by one wire, no pins", label("L1", x0, 0), wire(x0, 0, x0 + 20.32, 0), label("L2", x0 + 20.32, 0))
lib = f'''(symbol "P:passive" (exclude_from_sim no) (in_bom yes) (on_board yes)
 (property "Reference" "R" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (property "Value" "p" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (symbol "passive_1_1" (pin passive line (at 0 0 0) (length 2.54) (name "p" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))))
(symbol "P:power" (power) (exclude_from_sim no) (in_bom no) (on_board no)
 (property "Reference" "#PWR" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
 (property "Value" "p" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (symbol "power_1_1" (pin power_in line (at 0 0 0) (length 0) hide (name "p" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))))'''
# lay each case out 25.4 mm apart; shift y by moving items: simplest is regenerating with y offset
body = []
for k, (desc, items) in enumerate(cases):
    dy = 30.48 + 25.4 * k
    import re
    for it in items:
        def shift(m):
            return f'{m.group(1)}{float(m.group(2)):.4f} {float(m.group(3)) + dy:.4f}{m.group(4)}'
        it = re.sub(r'(\(at |\(xy )(-?[\d.]+) (-?[\d.]+)(\b)', shift, it)
        body.append(it)
sch = f'''(kicad_sch (version 20231120) (generator "probe") (uuid "00000000-0000-4000-8000-ffffffffffff") (paper "A0")
 (lib_symbols {lib})
 {chr(10).join(body)}
 (sheet_instances (path "/" (page "1"))))'''
path = os.path.join(out, "wires.kicad_sch"); open(path, "w").write(sch)
rep = os.path.join(out, "wires.json")
subprocess.run([KICAD, "sch", "erc", "--format", "json", "--severity-all", "-o", rep, path], check=True, capture_output=True)
data = json.load(open(rep))
rows = {}
for s in data["sheets"]:
    for v in s["violations"]:
        for it in v["items"][:1]:
            y = it["pos"]["y"]
            k = round((y - 30.48) / 25.4)
            rows.setdefault(k, []).append((v["type"], v["severity"]))
for k, (desc, _) in enumerate(cases):
    r = sorted(set(x for x in rows.get(k, []) if x[0] not in ("endpoint_off_grid", "lib_symbol_issues")))
    print(f"{desc}:\n    {r}")
