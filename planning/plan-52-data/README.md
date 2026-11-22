# plan-52 data

Measurements the plan-52 sub-plans (A–G) rest on, taken on 2026-11-21 on macOS 15 / Swift 6.2.4 (`swiftc -O`). Nothing here is
code the app runs.

| File | What it is |
|---|---|
| `probe_boolean_correctness.swift` | `CGPath` boolean ops (`subtracting`, `union`, `componentsSeparated`) against an exact point test: 500 random pads and traces, each grown by a 0.2 mm clearance (the shape unioned with its round-join stroke), subtracted from a 100 × 80 mm board. |
| `probe_components.swift` | What `componentsSeparated` returns: loops per component, and whether the largest loop's box holds the others (the outer-loop-and-holes rule plan-52-G's Gerber output relies on). |
| `probe_thermal.swift` | One 2 × 2 mm pad, its ring (the pad grown by 0.5 mm) less two crossing spokes 0.5 mm wide, subtracted from the board, the pad added back. |
| `probe_fill_pipeline.swift` | The fill pipeline plan-52-B ships: other-net obstacles grown by the clearance, thermal rings with their spokes cut out of them, one subtraction from the pour's rectangle, `componentsSeparated`, then a sample-point test per piece. |
| `zone_thermal_keepout.kicad_pcb`, `zone_pouronly.kicad_pcb`, `probe_kicad_zones.py` | KiCad 10.0.8 loads the zone and rule-area syntax plan-52-F writes. |

## Results

Boolean correctness (`./verify`): `mismatch 0 of 200000` sample points between the subtracted path and the direct
"not inside any grown obstacle" test; two pads 1 mm apart, each grown by 0.2 mm, leave a 0.6 mm copper strip between them
(`gap centre in fill: true`, and no copper within 0.15 mm of a pad); the result has curve elements (`1757`), so Gerber output
must flatten them (`GerberOutline.contours` already does).

Fill pipeline (`./pipeline2`, one board 98 × 78 mm, milliseconds):

| other-net obstacles | same-net pads (4 spokes each) | ring cut | fill | separate | touch test | total |
|---|---|---|---|---|---|---|
| 300 | 100 | 2 | 12 | 10 | 0 | 24 |
| 700 | 300 | 10 | 48 | 50 | 1 | 110 |
| 1500 | 500 | 21 | 174 | 72 | 8 | 275 |

Thermal ring (`./therm`): copper in the west spoke (`spoke W true`), none in the corner gap beside the pad (`gap NW false`) or
above it (`gap above pad false`), copper in the pad and outside the ring, one component.

An earlier pipeline that subtracted the obstacles and rings first and added the spokes back clipped to the result took 67 / 412 /
1053 ms for the spokes step alone (rows 100/300/500 same-net pads), against 2 / 10 / 21 ms for cutting the spokes out of the rings
first; and testing which pad touches which piece with a boolean per pair took 56 / 399 / 1542 ms, against 0 / 1 / 8 ms by sample
points. The pipeline in `probe_fill_pipeline.swift` is the faster one of each.

The same pipeline with a width clean-up by erosion and dilation (complement stroked and subtracted) took 196 ms, 4232 ms
and 3524 ms: too slow to run on every edit, so plan-52 has no such step (a piece is dropped by its bounding box instead).

Components (`./components`): `components 121, with more than one loop 2, of those whose largest loop's box holds every other loop's box 2, most loops in one component 157` (800 random obstacles).

KiCad (`probe_kicad_zones.py`): `(connect_pads (clearance 0.3))` reads back as thermal relief, `(connect_pads yes …)` as
solid (`ZONE_CONNECTION_FULL`), `no` as none, `thru_hole_only` as through-hole-only thermal; `(thermal_gap 0.6)
(thermal_bridge_width 0.4) (min_thickness 0.25)` read back as 0.6, 0.4, 0.25 mm; a rule area with
`(keepout (tracks allowed) (vias allowed) (pads allowed) (copperpour not_allowed) (footprints allowed))` and
`(layers "F.Cu" "B.Cu")` reads back as blocking only copper pour on those two layers; `kicad-cli pcb drc` loads it
(`Found 1 violations`, the isolated-copper warning for the unconnected test zone); after `ZONE_FILLER.Fill` the GND fill has
no copper inside the keepout and none within the clearance of the trace.
