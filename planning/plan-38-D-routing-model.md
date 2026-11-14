# plan-38-D: Routing model — shared copper geometry, `RoutingProblem`, `RoutingOptions`, fixtures

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-38-C

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A, B, C) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

The engine (E–J) must not touch the document, so it needs a value-type picture
of the board. This letter builds it: the DRC's copper-shape geometry becomes a
shared file the router also uses (a provably neutral move, gated by DRC output
being identical), the three example boards become test fixtures, and
`SchematicDocument.routingProblem(options:)` turns the document, a scope and the
options into a `RoutingProblem` — nets to route, their rules, and every other
piece of copper as an obstacle. No search yet.

References:

- plan-38-A § Decisions; plan-38-B § 3 (layer rules, groups).
- `DesignRuleCheck/DesignRuleCheck.swift` (`Shape` `:366`, `shape(of:)` `:404`,
  `copperShapes` `:424`, `nearPairs` `:444`, `distance` / `segmentDistance` /
  `pointDistance` / `rectDistance` / `segmentsCross` `:793-833`, `checkEdge`
  `:516`, `checkConnections` `:151`); `Canvas/Ratsnest.swift:connectivity`;
  `Preferences/DesignRulePreferences.swift:DesignRules`.
- `planning/plan-38-data/spike/` (the numbers below), `planning/plan-38-data/fixtures/`.

## 1. Goal

- `Routing/CopperGeometry.swift` holds the copper-shape types and distance
  functions the DRC used privately; the DRC calls them and its output on the
  three fixtures is byte-identical before and after.
- `schematicalTests/Fixtures/{AFU,LFU,RegFile}.schematical` are exact copies of
  `planning/plan-38-data/fixtures/` (sha256 checked by a test).
- `SchematicDocument.routingProblem(options:) -> RoutingProblem` gives, for each
  fixture with `scope: .all`: AFU 53 nets / 115 connections, LFU 59 / 188,
  RegFile 8 / 24 (the spike's census), with each net's width, clearance, via
  size, allowed layers and group resolved from plan-38-A/B.

### Non-goals (explicit constraints)

- No search, no document edits, no UI. `routingProblem` is read-only.
- The DRC's behavior and issue text do not change (Phase 2's gate).
- `RoutingProblem` and everything in it are `nonisolated` and `Sendable`: the
  engine runs off the main actor on this snapshot.

## 2. Current State

- The DRC's copper types are inside `private struct DesignRuleChecker`
  (`DesignRuleCheck.swift:51`): `Shape` (core segment or rect grown by a radius,
  on some layers, with a connectivity node), `shape(of: PlacedPad…)`,
  `nearPairs` (sweep along x), the distance functions. `grep -rn
  "DesignRuleChecker" schematical schematicalTests` finds uses only in
  `DesignRuleCheck.swift` itself (3 lines), so nothing outside depends on them.
- Edge fingers and board holes are exempt from the copper-to-edge rule
  (`DesignRuleCheck.swift:checkEdge`, "Fingers belong at the edge"); the spike's
  RegFile run showed a router that doesn't exempt them can't enter a finger
  (`results.txt`, RegFile 12/24).
- Connectivity: pads of one pin are one node; a Plane joins every plated
  through-hole pad and via on its net (`Ratsnest.swift`, "A Plane joins …"); each
  net's copper groups are `BoardConnectivity.netGroups()` — the unit the router
  connects.
- The unconnected-copper DRC rules a router's output must satisfy: a trace end
  must land on a pad on its layer, a via or another trace on its layer; a via
  touched by only one trace or pad is "Goes nowhere"
  (`DesignRuleCheck.swift:checkConnections`, `endIsConnected`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Connections / nets to route, AFU | 115 / 53 | `planning/plan-38-data/spike/results.txt` § Connections per net |
| Connections / nets to route, LFU | 188 / 59 | same |
| Connections / nets to route, RegFile | 24 / 8 | same |
| Pads: AFU, LFU, RegFile | 177, 254, 116 | same (`DumpScene`) |
| References to `DesignRuleChecker` outside `DesignRuleCheck.swift` | 0 | `grep -rn "DesignRuleChecker" schematical schematicalTests \| grep -v "schematical/DesignRuleCheck/DesignRuleCheck.swift" \| wc -l` → 0 |

### Verified properties

- A test can open an example by path and run the DRC on it: the spike's
  `DumpScene` did `SchematicFile.decode(data).apply(to: doc, catalog:
  TestSupport.catalog, fileURL: nil)` then `doc.connectivity()`, `doc.ratsnest()`
  and `doc.designRuleCheck(rules: DesignRules.customDefault)` on all three
  (output: `PROBE AFU pads 177 unrouted 115 drc 131`, `PROBE LFU pads 254 unrouted
  188 drc 188`, `PROBE RegFile pads 116 unrouted 24 drc 32`).

## 3. Design Overview

Types (all in `schematical/schematical/Routing/`, `nonisolated`, `Sendable`):

- `CopperShape { core: segment | rect, radius, layers: Set<CopperLayer>?, node:
  Int }` + `CopperGeometry` (the DRC's functions, unchanged): `shape(of:layers:node:)`,
  `gap(_:_:)`, `nearPairs`, `segmentDistance`, `pointDistance`, `rectDistance`,
  `segmentsCross`. A **trace segment** is a `.segment` core with radius width/2; a
  **via** is a zero-length segment with radius diameter/2, `layers: nil`.
- `RoutingOptions: Codable, Hashable` — see § 4.
- `RoutingProblem` — see § 4: `outline`, `layers`, `routable`, `planes: [CopperLayer:
  Int]`, `rules: DesignRules`, `options`, `nets: [RoutingNet]`, `obstacles:
  [RoutingObstacle]`, `groups: [RoutingGroup]`, `removedTraces: Set<UUID>`,
  `removedVias: Set<UUID>`.

Scope semantics (author decisions, stated as the router's contract):

- **Scope** picks the nets to route: `.all`; `.nets([PlaneNet])`; `.footprints([UUID])`
  (every net on a pad of those parts); `.connection(from: CGPoint, to: CGPoint)`
  (one ratsnest line: its two end points name two components of one net).
- **Include** (`.incompleteNets`, default, or `.untouchedNets`): a net is routed
  when it has ≥ 2 components; `.untouchedNets` additionally requires it to have no
  trace or via at all.
- **Existing copper** (`.keepAll` default, `.clearAuto`, `.clearUnlocked`): locked
  copper is always kept; `.clearAuto` removes unlocked `auto` traces and vias;
  `.clearUnlocked` removes all unlocked traces and vias. Removal applies to
  copper of nets in scope; copper that belongs to no net (traces that touch no
  pad) is removed only by `.clearUnlocked` with scope `.all`. Removed copper is
  absent from the problem (`removedTraces`, `removedVias` name it for plan-38-G).
  Kept copper of a routed net is part of its components; kept copper of every
  other net is an obstacle with that net's id.
- Every net out of scope, and every in-scope net with a single component, is an
  obstacle set tagged with its net (so a router may pass over its own net's
  copper and never another's).

Where correctness risk concentrates: Phase 2 (moving code the DRC depends on)
— hence its byte-identity gate; and the scope/clear rules in Phase 4, which plan-38-G
turns into deletions.

Byte-identity IS the gate for Phase 2 (a provably neutral move): the DRC issue
text for the three fixtures, captured before the move, equals the text after.
Everything else is behavior-gated.

Rejected: giving the router the DRC's `DesignRuleChecker` as is (it is `private`
and `@MainActor`); copying the geometry (two copies would drift, and plan-38-H's
verification must agree with the DRC to the last 0.0001 mm).

## 4. Detailed Design

`RoutingOptions` (defaults in parentheses): `scope`, `include` (`.incompleteNets`),
`existing` (`.keepAll`), `layers: [CopperLayer: LayerOption]` (every routable layer
enabled, direction `.auto`; `LayerOption { enabled: Bool, direction: .auto |
.horizontal | .vertical }`), `vias: .allowed | .none` (`.allowed`), `angle:
.octilinear | .orthogonal` (`.octilinear`), `strategy: .fast | .balanced |
.thorough` (`.balanced`), `grid: Double` (0.1 mm, one of 0.05, 0.1, 0.127, 0.254),
`widthOverride: Int?` (nil, mil), `clearanceOverride: Double?` (nil, mm),
`fanoutPlanes: Bool` (true), `tuneGroups: Bool` (true), `routePairs: Bool` (true),
`seed: UInt64` (1), `timeLimit: Double?` (nil, seconds). Only the options other
than `scope` are persisted between runs (plan-38-K); `scope` comes from the
selection.

`RoutingNet { id: Int, name: String, key: PlaneNet, width: Double (mm), clearance:
Double, viaDrill: Double, viaDiameter: Double, allowed: Set<CopperLayer>,
preferred: Set<CopperLayer>, components: [RoutingComponent], planeLayer:
CopperLayer?, group: UUID?, impossible: String? }`. `width`/`clearance`/via come
from the net's class, then the overrides in the options, then the Default class;
`clearance` is never below `rules.minClearance`, and `width` never below
`rules.minTraceWidth`. `allowed`/`preferred` are `effectiveLayers` (plan-38-B)
intersected with the options' enabled layers. `impossible` is set when an **Only**
set leaves a pad unreachable (a surface-mount pad whose side layer is not in
`allowed`) or the set is empty, with a sentence naming the pad.
`RoutingComponent { pads: [CopperShape], traces: [RoutedTrace], vias: [RoutedVia] }`
(kept copper). `RoutingObstacle { shape: CopperShape, net: Int? }`: `net` nil is a
non-plated hole, a board hole or the outline's keep-out; a plated pad, a kept
trace or a via carries its net. `RoutingGroup { id, mode, nets: [Int], tolerance,
gap, maxUncoupled, maxMeander }`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — fixtures and the DRC golden (before any move)

- [x] Copy `planning/plan-38-data/fixtures/*.schematical` to `schematical/schematicalTests/Fixtures/`; `schematicalTests/FixtureSupport.swift`: `enum Fixture { static func open(_ name: String) throws -> SchematicDocument }` reading `URL(fileURLWithPath: #filePath).deletingLastPathComponent().appendingPathComponent("Fixtures/\(name).schematical")` and `SchematicFile.decode(...).apply(to:catalog: TestSupport.catalog, fileURL: nil)`.
- [x] `schematicalTests/FixtureTests.swift`: sha256 of each file equals `planning/plan-38-data/fixtures/SHA256SUMS`; `ratsnest().count` is 115 / 188 / 24.
- [x] `schematicalTests/DesignRuleGoldenTests.swift`: for each fixture, `designRuleCheck(rules: .customDefault)` rendered one line per issue as `"\(index)\t\(reference)\t\(issue)"`, compared to `Fixtures/drc-<name>.txt`; generate the three `.txt` files **at this commit, before Phase 2** (first run writes them; commit them). — 131 / 188 / 32 lines (the spike's DRC counts). Plus a fourth golden, `drc-AFU-copper.txt` (125 lines), see Corrections.
- [x] (added) `drc-AFU-copper.txt`: AFU with deterministic copper (`DesignRuleGoldenTests.afuWithCopper`), so the gate reaches the geometry Phase 2 moves. — `-only-testing:FixtureTests -only-testing:DesignRuleGoldenTests` → first run writes the golden and fails by design, second run `** TEST SUCCEEDED **` (3 tests, 0 failures).

Acceptance: fixtures match their hashes and counts; the DRC golden passes against itself.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/FixtureTests -only-testing:schematicalTests/DesignRuleGoldenTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 2 min; the spike's `DumpScene`, which ran connectivity, ratsnest and one DRC on all three fixtures, took 0.697 s).
Commit: aec77008

### Phase 2 — extract the copper geometry (byte-identical)

- [x] `Routing/CopperGeometry.swift` (new): move `Shape` (renamed `CopperShape`), `Shape.Core`, `shape(of:layers:node:)`, `nearPairs`, `distance(_:_:)`, `segmentDistance` (both), `pointDistance`, `rectDistance`, `segmentsCross`. `outlineSegments` and `referenceSize` stay in the DRC (silkscreen only). (`SchematicDocument.distance(from:to:)`, `distance(from:toSegment:_:)` and `segment(_:_:touches:)`, which the moved code calls, became `nonisolated static`: pure functions the off-main-actor engine needs too; no body changed.)
- [x] `DesignRuleCheck.swift`: replace the removed members with calls to `CopperGeometry` / `CopperShape`; nothing else changes.
- [x] Tests: none new; `DesignRuleGoldenTests` and `DesignRuleCheckTests` are the gate. — `-only-testing:DesignRuleGoldenTests -only-testing:DesignRuleCheckTests` → 16 tests, 0 failures, `** TEST SUCCEEDED **`, 6 runs of the golden after the determinism fix (8f2b2d2b) all pass; see Corrections for the one failed run before it.

Acceptance: DRC output identical on the three fixtures; the 11 DRC tests pass.
  Check: `-only-testing:schematicalTests/DesignRuleGoldenTests -only-testing:schematicalTests/DesignRuleCheckTests` → `** TEST SUCCEEDED **` (est. 2 min). A diff in a golden is a bug in the move: diff the failing fixture's text, fix the move, re-run — not a reason to regenerate the golden.
Commit: 96fda6f9

### Phase 3 — options and problem types

- [x] `Routing/RoutingOptions.swift`: `RoutingOptions` and its enums per § 4, `Codable` (stable raw values), `Hashable`, with `static let standard`. (`PlaneNet` gains `Codable` as its file text, for `.nets` scopes; layers are keyed by `CopperLayer.rawValue`; a grid off the four values fails to decode.)
- [x] `Routing/RoutingProblem.swift`: `RoutingProblem`, `RoutingNet`, `RoutingComponent`, `RoutedTrace { layer, width (mm), points }`, `RoutedVia { centre, drill, diameter }`, `RoutingObstacle`, `RoutingGroup`.
- [x] Tests: `schematicalTests/RoutingOptionsTests.swift` — `standard` round-trips through JSON; an unknown `strategy` string fails to decode; grid is one of the four values. — `-only-testing:RoutingOptionsTests` → 4 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: round trip and rejection.
  Check: `-only-testing:schematicalTests/RoutingOptionsTests` → `** TEST SUCCEEDED **` (est. 1–2 min).
Commit: —

### Phase 4 — `routingProblem(options:)`

- [ ] `Routing/RoutingProblemBuilder.swift`: `extension SchematicDocument { func routingProblem(options: RoutingOptions) -> RoutingProblem }` (`@MainActor`, reads `connectivity()`, `netRows`-style keys, `netClass`, `effectiveLayers`, `group`): scope and include, existing copper, components from `BoardConnectivity.root`, obstacles from every pad (plated pads carry their net; non-plated holes and board holes net nil), the outline, planes, rules from `DesignRules.current()` (a parameter `rules:` with that default so tests pass `.customDefault`).
- [ ] Edge fingers and header pads are obstacles with their net and flagged `exemptFromEdge` (the router must not apply the copper-to-edge keep-out to them, as `checkEdge`).
- [ ] Tests: `schematicalTests/RoutingProblemTests.swift` — per fixture with `.all`: net and connection counts of § 1; `.nets([one GND key])` gives 1 net; `.footprints([U1])` gives exactly the nets on U1's pads; `.clearAuto` on a document with one auto and one hand trace lists only the auto one in `removedTraces`; a locked auto trace is kept; `.untouchedNets` skips a net that has a trace; a net class's width and a Top-only rule appear in `RoutingNet`; an Only {Top} net with a Bottom SMD pad is `impossible`; `.connection` names two components of one net.

Acceptance: all cases pass on the fixtures.
  Check: `-only-testing:schematicalTests/RoutingProblemTests -only-testing:schematicalTests/FixtureTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

## Validation Plan

- Tests: `FixtureTests`, `DesignRuleGoldenTests`, `DesignRuleCheckTests`, `RoutingOptionsTests`, `RoutingProblemTests`.
- Coverage check: every new file under `Routing/` is hit by a test; the goldens would fail if `CopperGeometry` changed a gap by 0.0001 mm.
- Runtime proof: none (no UI).
- Doc sync: none (no format change). `docs/` gets the router's description in plan-38-K/M.
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions. Author decisions: clear applies to in-scope nets only; four grid values; options other than scope persist; goldens are generated before the move and are never regenerated to make the move pass.

## Corrections

- **The Phase 2 gate was too weak as written.** The three fixtures have no
  traces and no vias (spike README: "0 traces, 0 vias"), so their DRC output
  is only "Unrouted" and silkscreen lines (`cut -f3 drc-RegFile.txt | sort |
  uniq -c`: 24 Unrouted, 8 Reference over pad). The clearance, short, edge and
  via checks, which are exactly the `nearPairs` / distance code Phase 2 moves,
  never run on them, so "the goldens would fail if `CopperGeometry` changed a
  gap by 0.0001 mm" (Validation Plan) was false. Strengthened: a fourth golden,
  `drc-AFU-copper.txt`, from AFU plus deterministic copper (a straight trace
  along each of its first 60 unrouted lines, alternating Top/Bottom, 5–30 mil;
  a via at every fourth line's midpoint; a trace along the top edge; one
  0.15 / 0.3 mm via). Its 125 lines include "Clearance … min …", "Copper
  touches copper of another net", "Copper … from the board edge", four short
  circuits, "Goes nowhere" and "Drill 0.15 mm under the 0.2 mm minimum".
  Generated at this commit, before the move; it passed a second run unchanged.
- **`connectivity()` was not deterministic across runs** (found by the Phase 2
  gate). After the move, `DesignRuleGoldenTests` failed 1 run in 4
  (`AFU-copper: 125 lines vs golden 127; first difference at line 104`) and
  passed the other 3, unchanged code. Root cause, not the move: "pads of one
  pin are one node" unioned `padsOfPin.values`, a dictionary whose order is
  seeded per process, so which pad became a group's root varied, so
  `netGroups()`' group order and `spanningLines`' ties varied, so the
  fixture's copper (drawn along the first 60 ratsnest lines) varied. Fixed in
  `Ratsnest.swift`: union in sorted key order. This is a prerequisite of plan-38-F's
  determinism (same seed → same result), which `routingProblem` inherits from
  `connectivity()`. Proof the golden and the move both stand: the pre-move DRC
  (aec77008) plus only this fix, in a detached checkout → golden passed 3/3;
  the moved DRC plus the fix → passed 6/6.

## Summary

The risk is Phase 2: shared geometry the DRC depends on. The golden before the
move makes it checkable to the line. Everything after is a read-only snapshot.
