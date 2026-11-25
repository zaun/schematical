# plan-65-H: The DXF Reader

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-65-G (complete)

The board can be cut and shaped by hand (A–G); this sub-plan starts the DXF import with its pure half: read an ASCII DXF, flatten its curves, join its pieces into closed loops, and sort the loops into an outline,
cutouts and ignored ones. It touches no document, no window and no file dialog (plan-65-I does). Everything is checked against files written by other programs: KiCad's own DXF export and five files written by ezdxf, with
the areas ezdxf's own flattening gives.

The single outcome: `DXFReader.parse(text)` and `DXFLoops.build(…)` turn each fixture of `planning/plan-65-data` into the loops listed in `expected.json` (count, area within 1 %, bounding box within 0.02 mm), and `DXFImportPlan.make(…)` turns a set of loops into an outline and cutouts (or cutouts only) with every ignored loop counted by reason.

References:

- `planning/plan-65-data/README.md`, `make_fixtures.py`, `DXFFixtures.swift`, `fixtures/expected.json`, `kicad_edge_probe.dxf` (KiCad writes `LINE`, `ARC`, `CIRCLE`, `$INSUNITS` 4); Autodesk's DXF reference for the group codes used below
- plan-65-A § 4.1 (`flatness`, the flattening rule, `BoardGeometry.crossesItself`, `BoardGeometry.clockwise`), plan-65-B § 4.1 (`BoardSettings.problem(outline:)`)
- `Routing/CopperGeometry.swift` (`segmentsCross`, `:116`); `BoardEdge.contains(_:in:)` (A § 4.2)
- `Document/ZipArchive.swift` (the nearest existing pure file-format code, as a style reference), `schematicalTests/BoardCutoutTests.swift`

## Prerequisites

The shared gate is in plan-65-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-65-G is complete | `ls planning/completed \| grep -c '^plan-65-G'` → 1 | MET (2026-11-25: G landed, 1) |
| the fixtures exist | `ls planning/plan-65-data/DXFFixtures.swift planning/plan-65-data/fixtures/expected.json` → both listed | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `DXFReader.parse(_ text: String) throws -> DXFDrawing`: the `$INSUNITS` code and the `LINE`, `ARC`, `CIRCLE`, `LWPOLYLINE`, `POLYLINE` (with its `VERTEX`es), `ELLIPSE` and `SPLINE` entities of the `ENTITIES` section, each with its layer; every other entity type is counted by name in `skipped`; blocks and `INSERT` are not expanded (counted `INSERT`).
- `DXFLoops.build(_ drawing: DXFDrawing, layers: Set<String>, scale: Double) -> DXFLoopSet`: the closed loops (mm, y up as in the DXF) and the number of open chains, from the entities on `layers`, units scaled by `scale`.
- `DXFImportPlan.make(loops: [DXFLoop], mode: DXFImportMode) -> DXFImportPlan`: `outline: [CGPoint]?`, `cutouts: [[CGPoint]]` and `ignored: [DXFIgnored]` (reason and count).
- `DXFUnits` (from `$INSUNITS`: 1 inch, 2 foot, 4 mm, 5 cm, 6 m → `scale`; 0, missing or anything else → nil) and `DXFLayerPick.defaultSelection(layers:) -> Set<String>`.

### Non-goals (explicit constraints)

- No document, file dialog, sheet or menu (I); no automation (J).
- No binary DXF (refused with a message), no `BLOCKS`/`INSERT` expansion, no 3D entities, no `HATCH`, `TEXT` or `DIMENSION` (all counted as skipped), no DXF writer.
- No change to any existing file: all new files.

## 2. Current State

- No DXF code exists: `rg -il "dxf" schematical/schematical --glob '*.swift'` → no output, exit 1 (checked 2026-11-25).
- The tools it builds on exist: `BoardGeometry.crossesItself` and `clockwise` (A), `BoardEdge.contains(_:in:)` (A), `CopperGeometry.segmentsCross` (`Routing/CopperGeometry.swift:116`), the 0.01 mm sagitta rule (A § 4.1).

### Measured populations

| What | Count | Command |
|---|---|---|
| fixture DXF files and the loops expected | 6 files, 12 loops | `python3 -c "import json; d=json.load(open('planning/plan-65-data/fixtures/expected.json')); print(len(d), sum(len(v['loops']) for v in d.values()))"` → `6 12` (`chain_board` 1, `curves` 2, `inch_board` 2, `kicad_edge_probe` 3, `open_chain` 1, `rounded_board` 3) |
| entity types the reader must handle | 7 | `LINE`, `ARC`, `CIRCLE`, `LWPOLYLINE`, `POLYLINE`, `ELLIPSE`, `SPLINE` |
| entity types seen in the fixtures | `ARC`, `CIRCLE`, `ELLIPSE`, `LINE`, `LWPOLYLINE`, `POLYLINE`, `SPLINE`, `TEXT` | the ezdxf listing of `DXFFixtures.swift` (planning/plan-65-data/README.md) |

### Verified properties

- The reduced fixtures (header `$INSUNITS` + `ENTITIES` only) are readable by ezdxf and have the same entity types as the full files (`planning/plan-65-data/README.md`).
- ezdxf's flattening at 0.01 mm gives the expected areas; the Swift flattening is the same rule (a 0.01 mm sagitta), so areas agree within the polygon error: for the 1.6 mm circle ezdxf's 7.9931 mm² is 0.6 % under π·1.6² = 8.0425 mm², which is why the test tolerance is 1 %.
- KiCad's DXF has y up with the board at negative y (`kicad_edge_probe.dxf`: lines from (0, 0) to (40, −30)); the reader keeps the DXF's coordinates and I flips them.

## 3. Design Overview

Pure value types in `Document/DXFReader.swift` and `Document/DXFLoops.swift` (`nonisolated`, `Sendable`), no UI types.

**Parse.** The file is group-code/value line pairs. Text is read as UTF-8, or Latin-1 when that fails. A first line starting `AutoCAD Binary DXF` throws `DXFError.binary` ("Binary DXF isn't supported; save the drawing as ASCII DXF"); a file with no `ENTITIES` section throws `DXFError.noEntities`; an odd number of lines throws `DXFError.truncated`. Only the `HEADER` variable `$INSUNITS` (group 70) and the `ENTITIES` section are read; `TABLES`, `BLOCKS`, `CLASSES`, `OBJECTS` are skipped by section name. Entities by group code: all have `8` layer (default `0`).

| Entity | Codes |
|---|---|
| `LINE` | `10 20` start, `11 21` end |
| `ARC` | `10 20` centre, `40` radius, `50` start angle, `51` end angle, degrees, counter-clockwise from start to end |
| `CIRCLE` | `10 20`, `40` |
| `LWPOLYLINE` | `90` count, `70` flags (bit 1 closed), repeated `10 20` vertex with optional `42` bulge after it |
| `POLYLINE` | `70` flags (bit 1 closed), then `VERTEX` entities (`10 20`, `42` bulge) until `SEQEND`; 3D polyline and mesh flags (bits 8, 16, 32) are skipped and counted `POLYLINE` |
| `ELLIPSE` | `10 20` centre, `11 21` major-axis end point relative to the centre, `40` minor/major ratio, `41` `42` start and end parameter in radians (default 0 and 2π) |
| `SPLINE` | `70` flags (bit 1 closed, 4 rational), `71` degree, `40` knots, `10 20` control points, `41` weights, `11 21` fit points |

**Flatten** (all in mm after `scale`, sagitta `BoardCutout.flatness` = 0.01 mm, at least 4 segments per quarter turn): a bulge `b` on a chord `c` is an arc of included angle `4·atan(b)` (positive counter-clockwise), radius `(c²/4 + s²)/(2s)` with sagitta `s = |b|·c/2`; an ellipse point at parameter `t` is `centre + major·cos t + minor·sin t` with `minor = ratio·(−major.y, major.x)`; a spline is evaluated as a NURBS from its knots, control points and weights (weights 1 when absent), each parameter span subdivided until its midpoint is within the flatness of the chord (depth at most 10); a spline with fit points and no control points is the polyline through the fit points.

**Loops.** Each entity gives one polyline. Circles, closed polylines/ellipses/splines, and open pieces whose two ends are within the join tolerance (0.01 mm) are loops by themselves. The rest are chained: start from the first unused piece in drawing order, repeatedly append the unused piece with an end nearest the chain's tail within 0.01 mm (reversed if it is its far end; ties go to the earlier piece in drawing order), until none; the chain is a loop when its two ends are within 0.01 mm and it has at least 3 points, else it counts as an open chain. A loop with a repeated consecutive point drops the repeat; a loop that `crossesItself` or has area under 1e-6 mm² is dropped and counted `crossesItself`.

**Plan.** Depth of a loop = how many other loops contain all its points (`BoardEdge.contains(_:in:)`); a loop whose edges cross another loop's edges (`CopperGeometry.segmentsCross`) is `ignored(.crossesAnotherLoop)` and takes no part in depth. *Outline plus cutouts*: the outline is the depth-0 loop with the largest area; every other depth-0 loop is ignored as `.outsideTheOutline`; depth-1 loops inside the outline are cutouts; depth 2 and deeper are ignored as `.insideACutout`. *Cutouts only*: every depth-0 loop is a cutout; deeper loops are ignored as `.insideACutout`. Every polygon is returned counter-clockwise in the DXF's y-up frame (positive shoelace area); I flips y, which makes it clockwise in the board's y-down frame, and applies `BoardGeometry.clockwise` anyway.

**Default layers.** `DXFLayerPick.defaultSelection`: the layers whose lower-cased name contains `edge`, `outline`, `board`, `profile` or `cut` when there are any; else every layer that has a supported entity.

Where the risk concentrates: the chain walk at a point where three pieces meet (decided above: nearest, then drawing order; the unchosen pieces are left over and counted as open chains), and unit scale. Byte-identity is irrelevant (a new reader); the gate is the fixtures' numbers.

Rejected: a third-party DXF library (none is in the project and the entity set is small); expanding `INSERT` (CAD board outlines are drawn in model space; a count of skipped inserts tells the person); a global join tolerance larger than 0.01 mm (it would fuse two features 0.02 mm apart).

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes**; use `- [~]` for partially done and say what remains; mark a moot task `- [x] ~~text~~ — moot: <evidence>`; fill the phase's `Commit:` line with the hash(es) the moment it lands; add any task you discover you needed.
>
> **An unticked box means NOT DONE.**

### Phase 1 — Parse and flatten

- [x] Copy `planning/plan-65-data/DXFFixtures.swift` to `schematical/schematicalTests/DXFFixtures.swift` (unchanged).
- [x] `Document/DXFReader.swift`: `DXFDrawing`, `DXFEntity`, `DXFUnits`, `DXFError`, `DXFReader.parse`; `Document/DXFLoops.swift`: the flatteners (bulge, ellipse, NURBS).
- [x] Tests `schematicalTests/DXFReaderTests.swift`: every fixture parses; entity types and counts per fixture (`chainBoard`: 4 `LINE` + 1 `ARC`; `curves`: 1 `SPLINE` + 1 `ELLIPSE`; `inchBoard`: `POLYLINE` + `CIRCLE`, units 1; `roundedBoard`: `LWPOLYLINE` ×2, `CIRCLE`, `skipped == ["TEXT": 1]`; `openChain` units code 0 → `DXFUnits` nil; `kicadEdgeProbe`: 5 `LINE`, 1 `CIRCLE`, 1 `ARC`, units 4); a binary-DXF header throws `.binary`; a text file with an odd line count throws `.truncated`; a file with no `ENTITIES` throws `.noEntities`; Latin-1 layer name reads; a quarter-circle bulge `b = tan(π/8)` from (10, 0) to (0, 10) has every flattened point 10 ± 0.01 from the origin and a negative bulge goes the other way; the degree-2 NURBS quarter circle (control points (1,0), (1,1), (0,1), weights 1, √2/2, 1, knots 0,0,0,1,1,1) has every flattened point 1 ± 0.01 from the origin; an ellipse with major (5, 0), ratio 0.6 has area 47.06 ± 1 %.

Acceptance: every fixture parses to the entities above and each flattener is within the 0.01 mm rule.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/DXFReaderTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min).
  Observed: DXFReaderTests 13, 0 failures, TEST SUCCEEDED.
Commit: —

### Phase 2 — Loops and the import plan

- [ ] `Document/DXFLoops.swift`: `DXFLoop`, `DXFLoopSet`, `DXFLoops.build` (join, chain, drop), `DXFLayerPick`; `Document/DXFImportPlan.swift`: `DXFImportMode` (`outlineAndCutouts`, `cutoutsOnly`), `DXFIgnored`, `DXFImportPlan.make`.
- [ ] Tests `schematicalTests/DXFLoopTests.swift`, against `DXFFixtures.expectedJSON`: for each fixture, `build(layers: all, scale: scale_to_mm)` gives the expected number of loops and each loop's area within 1 % and bounding box within 0.02 mm (matched by area order); `openChain` gives 1 loop and `openChains == 1`; `chainBoard` (ends 1e-4 apart, one reversed, out of order) is 1 loop; `kicadEdgeProbe` is 3 loops, from `LINE`s, a `CIRCLE` and an `ARC` plus `LINE`; `roundedBoard` with layers `["Dims"]` has no loops; two lines whose ends are 0.05 mm apart do not join; a bow-tie made of 4 lines is dropped as `crossesItself`; plan tests: `roundedBoard` in `outlineAndCutouts` gives the 60 × 40 outline and 2 cutouts and no ignored; the same loops plus a separate far square give `ignored == [(.outsideTheOutline, 1)]`; a loop inside a cutout is `.insideACutout`; two loops that cross are `.crossesAnotherLoop`; `cutoutsOnly` on `roundedBoard` gives the outline's loop and the other two as depth-0 and depth-1 → the 60 × 40 as a cutout and the two inside it ignored `.insideACutout`; `DXFLayerPick`: a drawing with layers `Edge`, `Dims` picks `["Edge"]`, with `A`, `B` picks both.

Acceptance: the loop numbers of all six fixtures match the oracle and the plan sorts them as above.
  Check: `xcodebuild … test -only-testing:schematicalTests/DXFLoopTests -only-testing:schematicalTests/DXFReaderTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 3 min).
Commit: —

## Validation Plan

- Tests: `DXFReaderTests`, `DXFLoopTests` (the negative cases: binary, truncated, no entities, open chain, a gap too wide to join, a bow-tie, crossing loops).
- Coverage check: the reader files are in the app target; the tests are in the test target; nothing else in the app calls them yet, which is the point of this sub-plan.
- Runtime proof: round trip with a real tool, run once and recorded in the Commit line: export the app's own `cutouts.kicad_pcb` sample (G) with `kicad-cli pcb export dxf -l Edge.Cuts --output-units mm`, and feed that DXF to `DXFLoops.build` in a test that has its text inlined: it gives the same loop count and areas (within 1 %) as the document's outline and cutouts. (est. 5 min)
- Doc sync: `docs/file-format.md` is B's; the import is documented in I and J.
- Final gate (run ONCE, after Phase 2): both phases' test classes (est. 4 min).

## Decisions

- DXF import takes the board's outline from the largest loop and its cutouts from the loops inside it, or only cutouts — decided by the owner, 2026-11-25 (§ 3, the plan's two modes).
- ASCII DXF only; the seven entity types above; no block expansion — decided by the author, 2026-11-25. KiCad, ezdxf and the usual CAD exporters write these types; the sheet reports what was skipped (I).

## Corrections

<Filled in during execution.>

## Summary

Risk: joining pieces (tolerance and the three-way point) and the unit scale; both have fixtures. Left untouched: the document, the UI and every other file.
