"""plan-65 DXF fixtures: real DXF files written by ezdxf 1.4.5 (an independent writer), and the oracle numbers the
Swift reader (plan-65-H) must reproduce.  Run: python3 -m venv v; v/bin/pip install ezdxf; v/bin/python make_fixtures.py
Writes fixtures/*.dxf and fixtures/expected.json (per fixture: unit scale to mm, and each closed loop's |area| in mm^2
and bounding box, from ezdxf's own flattening at 0.01 mm)."""
import json, math, os
import ezdxf
from ezdxf import path as ezpath

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "fixtures")
os.makedirs(OUT, exist_ok=True)

def area(pts):
    return abs(sum(a[0]*b[1]-b[0]*a[1] for a, b in zip(pts, pts[1:]+pts[:1]))) / 2

def loops_of(entities, scale):
    """Closed loops from entities that are each a closed loop by themselves (the chain case is checked by count and area)."""
    out = []
    for e in entities:
        p = ezpath.make_path(e)
        pts = [(v.x*scale, v.y*scale) for v in p.flattening(0.01 / scale)]
        out.append({"area": round(area(pts), 4),
                    "box": [round(min(x for x, _ in pts), 3), round(min(y for _, y in pts), 3),
                            round(max(x for x, _ in pts), 3), round(max(y for _, y in pts), 3)]})
    return out

expected = {}

def save(name, doc, scale, loops, skipped=None):
    doc.saveas(os.path.join(OUT, name))
    expected[name] = {"scale_to_mm": scale, "loops": loops, "skipped": skipped or {}}

# 1. lwpolyline: 60 x 40 board with 5 mm rounded corners (bulge = tan(pi/8) on a quarter arc), a 3.2 mm circle and a
#    6 x 2 mm slot (two semicircle ends: bulge 1), units mm
doc = ezdxf.new("R2010", setup=True); doc.header["$INSUNITS"] = 4
msp = doc.modelspace()
b = math.tan(math.pi/8)
outline = msp.add_lwpolyline([(5,0,0,0,0),(55,0,0,0,b),(60,5,0,0,0),(60,35,0,0,b),(55,40,0,0,0),(5,40,0,0,b),(0,35,0,0,0),(0,5,0,0,b)],
                             format="xyseb", close=True, dxfattribs={"layer": "Edge"})
hole = msp.add_circle((15, 20), 1.6, dxfattribs={"layer": "Edge"})
slot = msp.add_lwpolyline([(31,19,0,0,0),(35,19,0,0,1),(35,21,0,0,0),(31,21,0,0,1)], format="xyseb", close=True,
                          dxfattribs={"layer": "Edge"})
msp.add_text("dimension", dxfattribs={"layer": "Dims"})
save("rounded_board.dxf", doc, 1.0, loops_of([outline, hole, slot], 1.0), {"TEXT": 1})

# 2. heavy POLYLINE + VERTEX (what older CAD writes), inches ($INSUNITS 1): 2 x 1 in board, a 0.25 in circle
doc = ezdxf.new("R2000"); doc.header["$INSUNITS"] = 1
msp = doc.modelspace()
poly = msp.add_polyline2d([(0,0),(2,0),(2,1),(0,1)], close=True)
hole = msp.add_circle((1, 0.5), 0.125)
save("inch_board.dxf", doc, 25.4, loops_of([poly, hole], 25.4))

# 3. a board as separate LINE and ARC entities, out of order and with reversed ends (what KiCad and Inkscape write):
#    40 x 20 with one 5 mm radius corner arc; three 1e-4 mm gaps at the joins
doc = ezdxf.new("R2000"); doc.header["$INSUNITS"] = 4
msp = doc.modelspace()
msp.add_line((40, 20), (0, 20))                       # reversed, third
msp.add_line((0, 20.0001), (0, 0))
msp.add_line((0, 0), (35, 0))
msp.add_arc((35, 5), 5, -90, 0)                       # the corner, from (35,0) to (40,5)
msp.add_line((40, 5), (40, 20.0001))
# one loop: the 5 mm corner arc on an otherwise 40 x 20 rectangle
exp_area = 40*20 - (5*5 - math.pi*25/4)
save("chain_board.dxf", doc, 1.0, [{"area": round(exp_area, 4), "box": [0, 0, 40, 20]}])

# 4. SPLINE (degree 3, clamped knots, 7 control points whose last equals its first, so the curve closes on itself)
#    and ELLIPSE (full, 10 x 6 mm)
doc = ezdxf.new("R2010"); doc.header["$INSUNITS"] = 4
msp = doc.modelspace()
ctrl = [(20*math.cos(t), 12*math.sin(t)) for t in [i*2*math.pi/6 for i in range(6)]]
spline = msp.add_open_spline(ctrl + [ctrl[0]], degree=3)
ell = msp.add_ellipse((60, 0), major_axis=(5, 0), ratio=0.6)
save("curves.dxf", doc, 1.0, loops_of([spline, ell], 1.0))

# 5. units unknown ($INSUNITS 0), and an open chain that must be reported, not imported
doc = ezdxf.new("R2000"); doc.header["$INSUNITS"] = 0
msp = doc.modelspace()
sq = msp.add_lwpolyline([(0,0),(10,0),(10,10),(0,10)], close=True)
msp.add_line((20, 0), (30, 0)); msp.add_line((30, 0), (30, 5))
save("open_chain.dxf", doc, 1.0, loops_of([sq], 1.0), {"open_chains": 1})

# 6. KiCad's own DXF (kicad_edge_probe.dxf, written by kicad-cli, kept whole): a 40 x 30 rectangle of four LINEs, a 3 mm circle
#    and a half disc of one ARC and one LINE
kdoc = ezdxf.readfile(os.path.join(os.path.dirname(os.path.abspath(__file__)), "kicad_edge_probe.dxf"))
circle = [e for e in kdoc.modelspace() if e.dxftype() == "CIRCLE"][0]
arc = [e for e in kdoc.modelspace() if e.dxftype() == "ARC"][0]
arc_pts = [(v.x, v.y) for v in ezpath.make_path(arc).flattening(0.01)]
expected["kicad_edge_probe.dxf"] = {
    "scale_to_mm": 1.0, "skipped": {},
    "loops": [{"area": 1200.0, "box": [0, -30, 40, 0]}] + loops_of([circle], 1.0)  # KiCad's y is down, the DXF's up: y is negated
             + [{"area": round(area(arc_pts), 4), "box": [round(min(x for x, _ in arc_pts), 3), round(min(y for _, y in arc_pts), 3),
                                                          round(max(x for x, _ in arc_pts), 3), round(max(y for _, y in arc_pts), 3)]}]}

with open(os.path.join(OUT, "expected.json"), "w") as f:
    json.dump(expected, f, indent=1, sort_keys=True)
print(json.dumps(expected, indent=1, sort_keys=True))

# Swift source for plan-65-H's tests (the test host is sandboxed, so the fixtures are string literals, not files).
# ezdxf writes every table and class of a DXF R2010 file (about 3000 lines); the Swift copy keeps only the header's $INSUNITS and the
# ENTITIES section, which is all a reader needs. The KiCad file is kept whole: it is the real-world file with its tables and blocks.
def swift_name(file):
    parts = os.path.splitext(file)[0].split("_")
    return parts[0] + "".join(p.title() for p in parts[1:])

def minimal(text):
    rows = text.split("\n")
    pairs = [(rows[i].strip(), rows[i + 1].rstrip()) for i in range(0, len(rows) - 1, 2)]
    keep, section, header_var = [], None, None
    for k, v in pairs:
        if k == "0" and v == "SECTION":
            section = "?"
        elif section == "?" and k == "2":
            section = v.strip()
            if section == "ENTITIES":
                keep += [("0", "SECTION"), ("2", "ENTITIES")]
        elif k == "0" and v == "ENDSEC":
            if section == "ENTITIES":
                keep.append(("0", "ENDSEC"))
            section = None
        elif section == "HEADER":
            if k == "9":
                header_var = v.strip()
            elif header_var == "$INSUNITS" and k == "70":
                keep = [("0", "SECTION"), ("2", "HEADER"), ("9", "$INSUNITS"), ("70", v.strip()), ("0", "ENDSEC")] + keep
                header_var = None
        elif section == "ENTITIES":
            keep.append((k, v))
    keep.append(("0", "EOF"))
    return "\n".join("%3s\n%s" % (k, v) for k, v in keep) + "\n"

with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "DXFFixtures.swift"), "w") as out:
    out.write("// Generated by planning/plan-65-data/make_fixtures.py (ezdxf 1.4.5; KiCad 10.0.8 for kicadEdgeProbe). Do not edit.\n")
    out.write("// Each fixture is an ASCII DXF; `expectedJSON` has, per file name, the unit scale to mm, the area (mm^2) and\n")
    out.write("// bounding box [minX, minY, maxX, maxY] of each closed loop, and the entities a reader must skip or report.\n")
    out.write("enum DXFFixtures {\n")
    names = sorted(expected)
    for name in names:
        if name == "kicad_edge_probe.dxf":
            continue
        text = minimal(open(os.path.join(OUT, name)).read())
        assert '"""#' not in text
        out.write('    static let %s = #"""\n%s"""#\n\n' % (swift_name(name), text))
    kicad = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "kicad_edge_probe.dxf")).read()
    out.write('    static let kicadEdgeProbe = #"""\n%s"""#\n\n' % kicad)
    out.write("    static let all: [(name: String, text: String)] = [\n")
    for name in names:
        out.write('        ("%s", %s),\n' % (name, swift_name(name)))
    out.write("    ]\n\n")
    out.write('    static let expectedJSON = #"""\n%s\n"""#\n}\n' % json.dumps(expected, indent=1, sort_keys=True))
