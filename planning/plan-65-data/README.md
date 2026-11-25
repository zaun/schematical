# plan-65 data

Measurements the plan-65 sub-plans (A–J) rest on, taken on 2026-11-25 (macOS 15, Xcode 26.3, KiCad 10.0.8 CLI, ezdxf 1.4.5).
Nothing here is code the app runs.

| File | What it is |
|---|---|
| `kicad_edge_probe.kicad_pcb` | A KiCad 10 board whose Edge.Cuts is a 40 × 30 rectangle with a 3 mm-radius circle and a half-disc (an arc and a line) inside it. |
| `kicad_edge_probe.dxf` | What `kicad-cli pcb export dxf -l Edge.Cuts --output-units mm` writes for it. |
| `zone_edge_cuts.kicad_pcb`, `probe_zone_edge_cuts.py` | KiCad fills a whole-outline zone round an inner Edge.Cuts loop (result below). |
| `make_fixtures.py`, `fixtures/*.dxf`, `fixtures/expected.json` | Five DXF files written by ezdxf (an independent writer), and per file the unit scale and each closed loop's area and bounding box from ezdxf's own flattening at 0.01 mm. plan-65-H's reader tests reproduce these numbers. |

## Results

KiCad's DXF (`python3` over the file's group-code pairs): entities `LINE` 5, `CIRCLE` 1, `ARC` 1 (no polylines: the rectangle
is four separate `LINE`s, so a reader must join end points); header `$ACADVER` `AC1018`, `$INSUNITS` `4` (mm), `$MEASUREMENT` `1`.

KiCad's own check of nested Edge.Cuts loops (`kicad-cli pcb drc --format json kicad_edge_probe.kicad_pcb`):
`violations 0`, `unconnected_items 0`: a closed loop inside the outline is accepted as a cutout, with no "malformed outline" error.

ezdxf fixtures (`expected.json`): `rounded_board.dxf` (LWPOLYLINE with bulges: a 60 × 40 board with 5 mm corners, area 2378.4355;
a 3.2 mm circle, 7.9931; a 6 × 2 slot, 11.1223; a TEXT that must be skipped), `inch_board.dxf` (heavy POLYLINE, `$INSUNITS` 1,
scale 25.4), `chain_board.dxf` (five LINE/ARC entities out of order, ends reversed, two joins 1e-4 apart; area 794.635),
`curves.dxf` (a closed SPLINE 506.0902 and a full ELLIPSE 47.0613), `open_chain.dxf` (`$INSUNITS` 0, one 10 mm square and one
open chain of two lines).

KiCad's zone fill against an inner Edge.Cuts loop (`zone_edge_cuts.kicad_pcb`: 40 × 30 outline, a 10 × 10 inner loop at
15,10, a GND zone on B.Cu over the whole outline; `probe_zone_edge_cuts.py` with KiCad's bundled python, `ZONE_FILLER.Fill`):
`HitTestFilledArea` is `False` at the cutout's centre, `True` 5 mm left of it, `False` 0.1 mm left of its edge. So a zone
polygon that is the whole outline stays out of a cutout and keeps its edge clearance from it: plan-65-G leaves the Plane
zone's polygon alone.

`DXFFixtures.swift` is what `make_fixtures.py` also writes: the five ezdxf files reduced to their `$INSUNITS` and ENTITIES (ezdxf's tables and classes are
about 3000 lines each), the KiCad file whole, and `expectedJSON` (the same numbers as `fixtures/expected.json`, plus the KiCad file's three loops: a
1200 mm² rectangle, a 3 mm circle 28.2368 and a half disc 6.2529; KiCad's DXF has y up, so its board lies at negative y). plan-65-H copies it to
`schematical/schematicalTests/DXFFixtures.swift`; `swiftc -parse` accepts it (2172 lines), and ezdxf reads each reduced text back to the same entity types.
