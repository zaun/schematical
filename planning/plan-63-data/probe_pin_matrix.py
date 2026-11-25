#!/usr/bin/env python3 -I
"""Probe: KiCad 10.0.8's pin-to-pin conflict matrix, measured through `kicad-cli sch erc`.

Writes one schematic with a 1-pin symbol per KiCad pin type (12 types), and for every
pair (A, B) of types (78 pairs, A <= B) one wire joining a pin of A to a pin of B on a
net of its own, then runs the ERC and prints, per pair, what `pin_to_pin` reported:
ok / warning / error. Usage: python3 -I probe_pin_matrix.py <outdir>
"""
import json, subprocess, sys, os

KICAD = os.path.expanduser("~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli")
TYPES = ["input", "output", "bidirectional", "tri_state", "passive", "free", "unspecified",
         "power_in", "power_out", "open_collector", "open_emitter", "no_connect"]

out = sys.argv[1]
os.makedirs(out, exist_ok=True)
lib = []
for t in TYPES:
    lib.append(f'''(symbol "P:{t}" (exclude_from_sim no) (in_bom yes) (on_board yes)
 (property "Reference" "U" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (property "Value" "{t}" (at 0 0 0) (effects (font (size 1.27 1.27))))
 (symbol "{t}_1_1" (pin {t} line (at 0 0 0) (length 2.54)
   (name "p" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))))''')
body = []
pairs = [(a, b) for i, a in enumerate(TYPES) for b in TYPES[i:]]
n = 0
for k, (a, b) in enumerate(pairs):
    y = 20 + 10.16 * k
    for side, t, x in (("a", a, 50.8), ("b", b, 76.2)):
        n += 1
        body.append(f'''(symbol (lib_id "P:{t}") (at {x} {y} 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no)
 (uuid "00000000-0000-4000-8000-{n:012d}")
 (property "Reference" "U{n}" (at {x} {y-2} 0) (effects (font (size 1.27 1.27))))
 (property "Value" "{t}" (at {x} {y+2} 0) (effects (font (size 1.27 1.27))))
 (pin "1" (uuid "10000000-0000-4000-8000-{n:012d}"))
 (instances (project "probe" (path "/00000000-0000-4000-8000-ffffffffffff" (reference "U{n}") (unit 1)))))''')
    body.append(f'(wire (pts (xy 50.8 {y}) (xy 76.2 {y})) (stroke (width 0) (type default)) (uuid "20000000-0000-4000-8000-{k:012d}"))')
singles = []
for k, t in enumerate(TYPES):
    y = 20 + 10.16 * (len(pairs) + 2 + k)
    n += 1
    singles.append((t, y))
    body.append(f'''(symbol (lib_id "P:{t}") (at 50.8 {y} 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no)
 (uuid "00000000-0000-4000-8000-{n:012d}")
 (property "Reference" "U{n}" (at 50.8 {y-2} 0) (effects (font (size 1.27 1.27))))
 (property "Value" "{t}" (at 50.8 {y+2} 0) (effects (font (size 1.27 1.27))))
 (pin "1" (uuid "10000000-0000-4000-8000-{n:012d}"))
 (instances (project "probe" (path "/00000000-0000-4000-8000-ffffffffffff" (reference "U{n}") (unit 1)))))''')
sch = f'''(kicad_sch (version 20231120) (generator "probe") (uuid "00000000-0000-4000-8000-ffffffffffff") (paper "A0")
 (lib_symbols {" ".join(lib)})
 {chr(10).join(body)}
 (sheet_instances (path "/" (page "1"))))
'''
path = os.path.join(out, "probe.kicad_sch")
open(path, "w").write(sch)
rep = os.path.join(out, "erc.json")
subprocess.run([KICAD, "sch", "erc", "--format", "json", "--severity-all", "-o", rep, path],
               check=True, capture_output=True)
data = json.load(open(rep))
found = {}
at = {}   # y -> {rule: severity}
for sheet in data.get("sheets", []):
    for v in sheet.get("violations", []):
        for i in v["items"][:1]:
            at.setdefault(round(i["pos"]["y"], 2), {})[v["type"]] = v["severity"]
        if v["type"] != "pin_to_pin":
            continue
        ys = [round(i["pos"]["y"] * 1.0, 2) for i in v["items"]]
        found[round(ys[0], 2)] = v["severity"]
print("type counts:", {t: sum(1 for s in data["sheets"] for v in s["violations"] if v["type"] == t)
                       for t in sorted({v["type"] for s in data["sheets"] for v in s["violations"]})})
matrix = {}
for k, (a, b) in enumerate(pairs):
    y = round(20 + 10.16 * k, 2)
    sev = next((s for yy, s in found.items() if abs(yy - y) < 0.01), "ok")
    matrix[(a, b)] = matrix[(b, a)] = {"ok": 0, "warning": 1, "error": 2}[sev]
print("row/col order:", TYPES)
for a in TYPES:
    print(f"{a:15s}", [matrix[(a, b)] for b in TYPES])

SKIP = {"endpoint_off_grid", "lib_symbol_issues", "pin_to_pin"}
print("\nper pair (net of 2 pins): rules other than pin_to_pin")
for k, (a, b) in enumerate(pairs):
    y = round(20 + 10.16 * k, 2)
    r = {x: v for x, v in at.get(y, {}).items() if x not in SKIP}
    if r:
        print(f"  {a} + {b}: {r}")
print("\nper single pin, no wire:")
for t, y in singles:
    r = {x: v for x, v in at.get(round(y, 2), {}).items() if x not in SKIP}
    print(f"  {t}: {r}")
