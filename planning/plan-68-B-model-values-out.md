# plan-68-B: App — the value types out of `SchematicModel.swift`

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-68-A

`SchematicModel.swift` (5,606 code lines) holds two different things: 1,297
code lines of value types that every part of the app uses (the tools, the
part symbols and their pin layout, placed parts, wires, the orthogonal
router), and the 4,309-line `SchematicDocument` class. This letter moves the
value types into their own files, one per type family, so that what remains
is only the document, which letters C–F cover and take apart. Pure code
motion.

Prerequisites: plan-68-A's table, and plan-68-A complete (`ls
scripts/source-size.js scripts/coverage.js scripts/coverage-ratchet.tsv` →
all three).

References:

- `schematical/schematical/Canvas/SchematicModel.swift` (the `// MARK: -`
  sections named below).
- The test classes that use these types (§ Phase 1 Check): every test file
  naming `ChipLayout`, `OrthogonalRoute`, `RouteAxis`, `SupplyChoice` or
  `SchematicTool` (`grep -lE` over `schematical/schematicalTests` → 19 files).

## 1. Goal

- `SchematicModel.swift` holds `SchematicDocument` and nothing else (its
  first declaration is `final class SchematicDocument`).
- The value types are in the files of § 3, each at or under 800 code lines.
- Every test passes as before.

### Non-goals

- No declaration changes — names, access levels, `nonisolated`,
  conformances stay — with one exception: `ChipLayout.init(size:slots:)` goes
  from `fileprivate` to internal, because `PlacedChip.layout` (line 1362,
  moving to `PlacedChip.swift`) builds a split part's combined layout with
  it. That is the only cross-section use of a file-private member (`grep -nE
  'fileprivate' SchematicModel.swift` → line 701 and two inside the document;
  `ChipLayout(size:` is called at lines 651, 669, 677, 809 inside
  `ChipLayout` and at 1362; the document sets none of `PlacedChip`'s
  `private(set)` members: `awk 'NR>=1858' … | grep -E
  '\.(unitPlacements|symbol|unitKey|blockOrigin|baseLayout) *(=|\.append)'`
  → no output).

## 2. Current State

Measured (`sed -n '<a>,<b>p' … | node planning/plan-68-data/loc.js`), code
lines of `SchematicModel.swift` at `e2f6811a`:

| Lines | What | Code |
|---|---|---|
| 1–142 | `SchematicGrid`, `SchematicTool`, `SupplyChoice` (MARK Grid, Tools) | 104 |
| 143–1037 | `PartSymbol`, `ChipLayout` (MARK Block geometry) | 635 |
| 1038–1542 | `MemoryFile`, `ChipSocket`, `UnitPlacement`, `UnitRef`, `PlacedChip` with its Units mark (MARK Placed items) | 332 |
| 1543–1606 | `PinRef`, `BusPinRef`, `WireEnd`, `Junction`, `LineAnchor`, `Wire` | 42 |
| 1607–1857 | `RouteAxis`, `OrthogonalRoute` (MARK Orthogonal routing) | 184 |
| 1858–end | `SchematicDocument` (MARK Document) | 4,309 |

## 3. Design

| New file (in `schematical/schematical/Canvas/`) | Contents | Code |
|---|---|---|
| `SchematicTool.swift` | lines 1–142 | 104 |
| `ChipLayout.swift` | `PartSymbol`, `ChipLayout` | 635 |
| `PlacedChip.swift` | `MemoryFile` … `PlacedChip` | 332 |
| `Wire.swift` | `PinRef` … `Wire` | 42 |
| `OrthogonalRoute.swift` | `RouteAxis`, `OrthogonalRoute` | 184 |
| `SchematicModel.swift` | `SchematicDocument` | 4,309 (its size row stays until plan-68-F) |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — move the value types

- [x] Create the five files of § 3 by moving the line ranges verbatim (each
  file gets `import Foundation`/`import CoreGraphics` as the moved code
  needs); `SchematicModel.swift` keeps the document.
- [x] `scripts/source-size-ratchet.tsv`: lower the `SchematicModel.swift` row
  to its new count (`SZ` reports it stale until then).

Acceptance: the app and the tests build and the users of the moved types
pass.
  Check: `XB -only-testing:schematicalTests/ChipLayoutSideTests
  -only-testing:schematicalTests/SchematicRotateTests
  -only-testing:schematicalTests/WireSettleTests
  -only-testing:schematicalTests/WireRefitTests
  -only-testing:schematicalTests/RouteLegTests
  -only-testing:schematicalTests/SplitPartModelTests
  -only-testing:schematicalTests/SlideSwitchTests
  -only-testing:schematicalTests/CharacterLCDTests
  -only-testing:schematicalTests/SelectionModelTests` → all pass;
  `head -20 schematical/schematical/Canvas/SchematicModel.swift | grep -c
  'final class SchematicDocument'` → 1; `SZ` → exit 0 (est. 5 min).
  Observed: the nine classes pass (`ChipLayoutSideTests` 6, `SchematicRotateTests` 22, `WireSettleTests`, `WireRefitTests`, `RouteLegTests`, `SplitPartModelTests`, `SlideSwitchTests`, `CharacterLCDTests`, `SelectionModelTests`: every one of those suites `0 failures` in the run that also ran plan-68-C's new classes); `head -20 … | grep -c` → 1; `SZ` → exit 0 with the row at 4312 (`planning/plan-68-data/split_b.py` did the move).
Commit: —

## Validation Plan

- Final gate: the Phase 1 classes; `SZ`; `COV --app` total uncovered
  unchanged (est. one coverage run of those classes).

## Decisions

- (author) `ChipLayout` stays one file (635 ≤ 800), so none of its `private
  static` builders changes access.

## Corrections

(none yet)

## Summary

The safe first cut: values out, the class left alone for S.
