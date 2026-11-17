"""Checks the stackup KiCad kept when it loaded and re-saved a board exported by the app: the copper
thicknesses, the dielectrics (type, thickness, epsilon_r), the copper finish and the general thickness, against the
standard 4-layer stackup (plan-50-A). Prints each mismatch (empty = equal).

    python compare_kicad_stackup.py saved.kicad_pcb

Run on the app's `fourLayer` board after `probe_kicad_stackup_roundtrip.py`; recorded output in plan-50-E: none."""
import re
import sys

text = open(sys.argv[1]).read()
expected_copper = {"F.Cu": 0.035, "In1.Cu": 0.0152, "In2.Cu": 0.0152, "B.Cu": 0.035}
expected_dielectrics = [("prepreg", 0.2104, 4.4), ("core", 1.065, 4.6), ("prepreg", 0.2104, 4.4)]
problems = []

stackup = text[text.index("(stackup"):]
stackup = stackup[: stackup.index("(copper_finish")]
copper = {}
dielectrics = []
for name, body in re.findall(r'\(layer "([^"]+)"(.*?)\n\t\t\t\)', stackup, re.S):
    thickness = re.search(r"\(thickness ([0-9.]+)\)", body)
    if name in expected_copper:
        copper[name] = float(thickness.group(1)) if thickness else None
    elif name.startswith("dielectric"):
        kind = re.search(r'\(type "([^"]+)"\)', body).group(1)
        dielectrics.append((kind, float(thickness.group(1)), float(re.search(r"\(epsilon_r ([0-9.]+)\)", body).group(1))))
for name, wanted in expected_copper.items():
    if copper.get(name) is None or abs(copper[name] - wanted) > 1e-9:
        problems.append(f"copper {name}: {copper.get(name)} != {wanted}")
if len(dielectrics) != len(expected_dielectrics):
    problems.append(f"dielectrics: {len(dielectrics)} != {len(expected_dielectrics)}")
for got, wanted in zip(dielectrics, expected_dielectrics):
    if got[0] != wanted[0] or abs(got[1] - wanted[1]) > 1e-9 or abs(got[2] - wanted[2]) > 1e-9:
        problems.append(f"dielectric {got} != {wanted}")
finish = re.search(r'\(copper_finish "([^"]+)"\)', text).group(1)
if finish != "HAL lead-free":
    problems.append(f"copper_finish {finish}")
general = re.search(r"\(general\s+\(thickness ([0-9.]+)\)", text).group(1)
if abs(float(general) - 1.6062) > 1e-9:
    problems.append(f"general thickness {general} != 1.6062")
print("\n".join(problems))
