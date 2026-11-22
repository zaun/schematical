# plan-52-B: Pours and Keep-outs in the File

Last updated: 2026-11-21
Effort: small (<1h)
Depends on: plan-52-A (complete)

plan-52-A gave the document pours and keep-outs. This sub-plan saves and opens them: `board.pours` and `board.keepouts`, written
next to `board.holes`, validated on open, and described in `docs/file-format.md`.

The single outcome: a document with pours and keep-outs saves, opens and compares equal; a file with none is byte-identical to
before; a bad entry fails the open with a sentence that names it.

References:

- plan-52-A § 3 and § 4 (the types), `planning/plan-52-data/README.md`
- `docs/file-format.md` § Holes (line 229) and § Validation on open (line 563), "Changing the format" (line 736: an optional field
  keeps `version`)
- `schematical/schematical/Document/SchematicFile.swift`: `holes` (`:196`), `HoleEntry` (`:311`), the encode of holes (`:820`),
  the decode and checks (`:1158`), `PlaneNet(fileValue:)` use (`:1169`)
- Line numbers in this plan are as of 2026-11-21 and drift as other sessions edit these files; the symbol named beside each is the reference.

## Prerequisites

The shared gate is in plan-52-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-52-A is complete | `ls planning/completed \| grep -c '^plan-52-A'` → 1 | MET (2026-11-21: plan-52-A landed as 319d14c3; the letters are archived together when the whole plan lands) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update every status
> before you continue, and again before you decide to stop. If you stop, report the current status of *all* prerequisites.

## 1. Goal

- `board.pours`: one entry per pour, drawing order, written only when there are some; `board.keepouts` likewise.
- Opening a file rebuilds `pours` and `keepouts` equal to what was saved (ids, rects, layers, nets, every setting).
- A file with neither key opens as before and a document with neither saves with neither key: the bytes equal a save from before
  this plan.
- Opening fails with "The PCB settings are not valid: <detail>." for each bad entry listed in § 4.

### Non-goals (explicit constraints)

- `version` does not change: the two keys are optional, an older build ignores them (`docs/file-format.md` § Changing the format).
- No change to `layerStyles`, `holes`, `traces` or any existing key.
- No drawing, tool or export.

## 2. Current State

- `Board` (`SchematicFile.swift`, from `layerStyles` at `:188`) lists optional arrays `texts`, `traces`, `vias`, `holes`, `netClasses`… in that order; each is
  written only when non-empty (`holes: document.holes.isEmpty ? nil : …`, `:820`).
- `HoleEntry` (`:311`) is the precedent: `id`, `x`, `y`, optionals with defaults, `net` as `PlaneNet.fileValue`, `locked` written
  only when true. Decode (`:1158`) builds a `BoardHole`, checks `PlaneNet(fileValue:)` (`:1169`) and the range (`:1174`), appends.
- The "ID used more than once" check spans parts, junctions, wires, buses, texts, traces, notes and labels
  (`docs/file-format.md:567`); it has to cover the new ids.
- `plan-49` and `plan-50` kept goldens of saved text with a new key stripped (`TestSupport` helper at `TestSupport.swift:36`);
  `SchematicFileV5Tests` is the file round-trip suite.

### Measured populations

| What | Count | Command |
|---|---|---|
| places `SchematicFile.swift` names holes | 9 | `grep -c "holes\|HoleEntry" schematical/schematical/Document/SchematicFile.swift` → 9 |

### Verified properties

- Old builds ignore unknown keys of `board`: `docs/file-format.md` states it for `vias` tenting ("older builds ignore the two keys",
  plan-50-C) and the decoder is `Codable` with optionals, so an extra key is skipped.

## 3. Design Overview

Mirror `HoleEntry` exactly. Where the correctness risk is: the id-uniqueness check and the validation order (a bad entry must fail
before `apply` changes the document, "the canvas is left unchanged"). Byte-identity IS the gate for the no-pour case (class:
provably-neutral for old files), a failure there is a bug to root-cause (`diff` the two saved texts), not a stop. For files with
pours the gate is the round-trip equality test.

## 4. Detailed Design

```json
"pours": [ { "id": "...", "x": 10, "y": 5, "width": 40, "height": 30, "layer": "top", "net": "ground",
             "clearance": 0.3, "connection": "solid", "thermalGap": 0.6, "spokeWidth": 0.4, "minWidth": 0.25, "locked": true } ],
"keepouts": [ { "id": "...", "x": 20, "y": 10, "width": 8, "height": 6, "layers": ["top", "bottom"], "locked": true } ]
```

| `pours` field | Meaning (missing =) |
|---|---|
| `id` | UUID, unique across the file's other ids |
| `x`, `y`, `width`, `height` | the rectangle in mm; `width` and `height` each at least 0.5 |
| `layer` | `"top"`, `"inner2"`…`"inner5"`, `"bottom"`; on the board and not a Plane |
| `net` | as a Plane's (`"ground"`, `"power:<mV>"`, `"pin:<part UUID>/<designator>"`); none |
| `clearance` | mm, 0.05 to 10; the DRC's min clearance |
| `connection` | `"thermal"` or `"solid"`; `"thermal"` |
| `thermalGap`, `spokeWidth` | mm, 0.1 to 5; 0.5 each |
| `minWidth` | mm, 0.05 to 2; 0.2 |
| `locked` | written only when true; false |

| `keepouts` field | Meaning (missing =) |
|---|---|
| `id`, `x`, `y`, `width`, `height` | as a pour's |
| `layers` | array of layer names; a layer the board doesn't have is kept and ignored; missing is every layer. An empty array is kept as empty (no layer: it blocks nothing). |
| `locked` | as above |

Errors, each "The PCB settings are not valid: <detail>.": `pour <id> layer "middle" must be top, inner2 to inner5, or bottom`;
`pour <id> layer "inner4" is not on a 4-layer board`; `pour <id> layer inner2 is a plane`; `pour <id> net "vcc" must be ground,
power:<mV> or pin:<part id>/<pin>`; `pour <id> width 0.1 must be at least 0.5 mm` (same for `height`); `pour <id> clearance 20 must
be 0.05 to 10 mm`; `pour <id> connection "x" must be thermal or solid`; `pour <id> thermalGap 9 must be 0.1 to 5 mm` (same for
`spokeWidth`; `minWidth` 0.05 to 2); `keepout <id> layer "middle" must be top, inner2 to inner5, or bottom`; `keepout <id> width 0.1
must be at least 0.5 mm`.

## Compatibility / Format Impact

Adds two optional `board` keys. Unchanged: `version`, every existing key, every file without pours. An older build opens a file
with pours and shows no pour (it ignores them) and saves it back without them.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work; `- [~]` for partial; mark a moot
> task `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — Entries, encode, decode, validate

- [x] `SchematicFile.swift`: `Board.PourEntry` and `Board.KeepoutEntry` (`Codable, Equatable, Sendable`, fields per § 4, `locked`
      optional written only when true); `Board.pours`/`keepouts` optional arrays after `holes` (`:198`).
- [x] Encode (`:820`): `pours: document.pours.isEmpty ? nil : …`, `keepouts` likewise; `net` via `PlaneNet.fileValue`; `layers` as
      sorted raw names, `nil` when `layers == nil`.
- [x] Decode (`:1158`): build `BoardPour`/`BoardKeepout` with the defaults; run the checks of § 4 in that order using
      `BoardPour.problem(…)` from plan-52-A; include the ids in the cross-file uniqueness check; pass both arrays to the document
      init (`:1204`).
- [x] `docs/file-format.md`: a `### Pours` and a `### Keep-outs` section after Holes (`:229`) with the field tables of § 4, the
      `board` table rows, and the new errors in § Validation on open. Sentences about behavior (the fill) point to `docs/export.md`.
- [x] Tests: new `schematicalTests/PourFileTests.swift`: round trip with two pours (one every field set, one defaults) and two
      keep-outs (`layers` nil, `layers` set); each error of § 4 fails with its sentence and leaves the document unchanged; a saved
      document with none has neither key (`XCTAssertFalse(text.contains("\"pours\""))`); a file with a duplicate id between a pour
      and a trace fails.

Acceptance: round trip equal; every error asserted; a document with no pours saves byte-identical to the golden saved text the
existing suites keep.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/PourFileTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/FileDesignRulesTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min; the two existing suites hold the byte goldens of files with no pours). Observed: `Executed 31 tests, with 0 failures`, `** TEST SUCCEEDED **`; `grep -c '^### Pours\|^### Keep-outs' docs/file-format.md` → 2.
Commit: COMMIT_B

## Validation Plan

- Tests: `PourFileTests` (new); `SchematicFileV5Tests`, `FileDesignRulesTests` (existing goldens).
- Coverage check: `grep -c "PourEntry" schematical/schematical/Document/SchematicFile.swift` → at least 4 (type, encode, decode, uniqueness), all in the tested file path.
- Runtime proof: none; the first user-visible save is plan-52-E.
- Doc sync: `docs/file-format.md` (Pours, Keep-outs, Validation on open), checked by `grep -c '^### Pours\|^### Keep-outs' docs/file-format.md` → 2.
- Final gate: the Phase 1 check, once (est. 7 min).

## Decisions

- Optional `board.pours` and `board.keepouts`, `version` unchanged — decided by the author: "Adding an optional field: keep `version`" (`docs/file-format.md` § Changing the format).
- A keep-out's `layers` missing means every layer; an empty array means none — decided by the author: the two must not be confusable.

## Corrections

- `ls planning/completed | grep -c '^plan-52-A'` is 0 while plan-52 runs: a split plan's letters are archived together when the whole plan lands (plan-52-H's last task), so this row is read as "A's commit is on the integration branch" (`319d14c3`). Later letters' rows are read the same way.
- The cross-file id check needed no new code beyond `claim(…)` calls for pours and keep-outs, which `apply` already uses for every other id.

## Summary

Low risk: a copy of the hole entry. The one thing to watch is that validation runs before any state changes. Untouched: every
other key, the version, every export.
