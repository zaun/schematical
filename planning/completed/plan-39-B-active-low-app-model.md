# plan-39-B: Active-low pins — the app's pin model, Part Editor and agent control

Last updated: 2026-11-13
Effort: medium (1h–2h)
Depends on: plan-39-A

B carries the flag from the loader into the app. Its outcome: a part file
with `"activeLow"` loads in the app and `ChipPin` knows which alternatives
of its name are active low; the Part Editor shows, edits, loads and saves
the flag without losing it; agent control reports it. Nothing is drawn
differently yet (C draws).

References:

- plan-39-A § 3 (the key's rules), § Prerequisites (the gate for all of
  plan-39).
- `schematical/schematical/Backend/PartCatalog.swift` (`ChipPin`,
  the pin conversion at line 674).
- `schematical/schematical/Editor/PartDraft.swift` (`PinDraft`, `pinKeys`,
  the pin read at line 295 and write at lines 334–342),
  `Editor/PartEditorWindow.swift` (the pin grid at lines 360–385).
- `schematical/schematical/Automation/AutomationRead.swift` (`get_design`
  JSON at line 212, `get_part` text at line 358), `docs/agent-control.md`.

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-A complete | `grep -c 'active_low' backend/include/parts.h` → 1, and A's phases are ticked | MET (2026-11-14: 1; A ticked) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue. If you stop, report all
> prerequisites.

## 1. Goal

- `ChipPin.activeLowMask: UInt32` is filled from `PartPin.active_low`, and
  `ChipPin.alternatives` yields the name's slash alternatives, each with its
  polarity and its drawn text.
- The Part Editor's pin grid has an **Active low** control per signal pin;
  loading a part file and saving it back keeps the flag exactly (`true` stays
  `true`, a list stays a list).
- `get_design` (JSON) and `get_part` (text) report the flag.

### Non-goals (explicit constraints)

- No drawing change (plan-39-C).
- `get_design`'s text form is unchanged: its `designator:name` tokens are
  what agents copy back as pin references.
- No part file edits (D–K).
- `ChipPin` stays `Hashable, Sendable`; the new field is a plain value.

## 2. Current State

- `ChipPin` (`PartCatalog.swift:41–63`) holds `index designator name role
  group side`; the app builds it in one place (`PartCatalog.swift:674`) from
  the C `PartPin` (`grep -rn 'ChipPin(' schematical --include='*.swift' | wc -l`
  → 2: that site and a test helper).
- `PinDraft` (`PartDraft.swift:4–27`) is `Hashable` with hand-written `==`
  and `hash` that list every field but `id`. `pinKeys`
  (`PartDraft.swift:152`) is the set of pin keys the editor can keep; a pin
  with another key makes the whole pinout unloadable in the editor.
- The editor's grid (`PartEditorWindow.swift:360–385`) has columns Pin, Name,
  Role, Group, Side.
- `get_design` JSON writes one dictionary per pin
  (`AutomationRead.swift:212`: `designator name wired side x y`); `get_part`
  writes `designator=name [role] group` per pin (`:358`).

### Verified properties

| Claim | How verified |
|---|---|
| Only two sites build `ChipPin`, and the new field has a default so both compile unchanged | `grep -rn 'ChipPin(' schematical --include='*.swift'` → `PartCatalog.swift:674` and `schematicalTests/ChipLayoutSideTests.swift:8` |
| New files in `schematicalTests/` join the test target without a project edit | `grep -n PBXFileSystemSynchronizedRootGroup schematical/schematical.xcodeproj/project.pbxproj` → two groups (app and tests) |
| Editor saves through `PartDraft` → JSON (`JSONValue`), so the key must be in `pinKeys` or the editor drops pinouts that have it | read `PartDraft.swift:152, 295, 334–342` |

## 3. Design Overview

`ChipPin`:

```swift
let activeLowMask: UInt32            // bit i: alternative i of name is active low
struct Alternative { let text: String; let isActiveLow: Bool }
var alternatives: [Alternative]      // name split at "/", flags from the mask
var isActiveLow: Bool { activeLowMask != 0 }
```

`Alternative.text` is the name part as written. C draws it with one trailing
`#` removed when the alternative is active low and longer than one
character (`plan-39-C`); B only exposes the parts.

`PinDraft.activeLow: Set<String>` holds the alternatives (as written) that
are active low. Reading `true` fills it with every alternative of the pin's
current name; reading a list fills it with the list. Writing: `true` when the
set equals all alternatives of the name, the list (in name order) when it is
a proper non-empty subset, nothing when empty. Entries that are no longer an
alternative (the name was edited) are dropped on write; a pin whose role is
not input, output or inout writes nothing. This keeps the file canonical
(plan-39-A § 3) whatever the user typed.

Editor control: a `Menu` titled "Low" in a new grid column, one checkable
item per alternative of the name (a name with no `/` has one item), disabled
for non-signal roles; a marked item shows a check, and the column header is
"Active low".

Agent control: JSON pin dictionaries get `"activeLow": true` (all) or
`"activeLow": ["CS"]` (some) when set, absent otherwise; `get_part` appends
` low` or ` low=CS` after the role (and group).

**Where risk is.** The editor round trip (a flag lost or a non-canonical
file written). It gets a test that opens a flagged part, saves it unedited,
and compares the pin JSON.

**Gate class.** Behaviour-adding; tests, not byte identity.

**Rejected alternatives.** *A per-pin `Bool` in `ChipPin`* — cannot carry the
per-alternative flag the owner chose. *A free-text field in the editor* — one
more syntax to parse; the menu cannot produce an invalid list.

## 4. Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; `- [~]` partial; moot tasks stay with a reason; fill `Commit:`
> when a phase lands; an unticked box means NOT DONE.

### Phase 1 — `ChipPin`

- [x] `PartCatalog.swift`: add `activeLowMask` (default 0) and the
      `Alternative` struct and `alternatives` / `isActiveLow` to `ChipPin`;
      pass `pin.active_low` at the conversion (line 674).
      `ChipLayoutSideTests.swift:8` keeps compiling (the field has a default). — done: `activeLowMask` from `pin.active_low`; `ChipLayoutSideTests` compiles unchanged
- [x] Tests: `schematicalTests/ChipPinActiveLowTests.swift` (new): register a
      part (the way `PartEditorWindowTests` registers a custom part) with
      pins `CE#` `true`, `A0/CS` `["CS"]`, `D0` unflagged, `AIN7/GPIO3/DRDY/CLK`
      `["DRDY"]`; assert each pin's `alternatives` texts and flags, and
      `isActiveLow`. — done: registered as a single part file (6-pin NAND, SOT23-6) via `PartCatalogStore.register`, as `PartRegistrationTests` does; 2 tests

Acceptance: a catalog part's pins report the file's flags.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p39-dd test -only-testing:schematicalTests/ChipPinActiveLowTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min: first app build; nothing smaller compiles the app).
  Observed 2026-11-14: `Executed 2 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: bd2e825e

### Phase 2 — Part Editor

- [x] `PartDraft.swift`: add `"activeLow"` to `pinKeys`; add
      `activeLow: Set<String>` to `PinDraft` (in `==` and `hash`); read it in
      the pin parser (a `.bool(true)` or an array of strings, anything else
      → the part is not loadable in the editor, as for a bad `side`); write
      it as § 3 says. — done: `activeLowValue` writes the canonical form; a bad value leaves the pinouts an extra
- [x] `PartEditorWindow.swift`: the "Active low" menu column in the pin grid
      (§ 3); the Pin/Name/Role header row gains the column. — done: `ActiveLowMenu` (Toggle per unique alternative; label Low / Low ✓ / Low: CS; — and disabled for non-signal roles)
- [x] Tests: `PartEditorWindowTests`: open a part whose pins are `true`, a
      list and unflagged, save it with no edit, and assert the written pin
      objects equal the input (`true` stays `true`, the list stays the
      list); rename `A0/CS` to `A0/CSB` and assert the saved pin has no
      `activeLow`; change a flagged pin's role to `power` and assert it is
      dropped; tick every alternative in the menu model and assert `true`. — done: `testActiveLowRoundTrips`, `testActiveLowIsWrittenCanonically`, `testMalformedActiveLowIsNotLoadedIntoTheGrid`

Acceptance: the editor round-trips the flag and writes only canonical forms.
  Check: `xcodebuild … test -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartKindRoundTripTests` → `** TEST SUCCEEDED **` (est. 6 min).
  Observed 2026-11-14: PartEditorWindowTests `Executed 8 tests, with 0 failures`, PartKindRoundTripTests `Executed 26 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: aa0d824b

### Phase 3 — agent control

- [x] `AutomationRead.swift`: pin dictionaries in `get_design` JSON gain
      `activeLow` (§ 3); `get_part` text appends ` low` / ` low=A;B`. — done: `activeLowJSON` / `activeLowText`; the `get_part` header now reads `pins (designator=name [role] group low):`
- [x] `docs/agent-control.md`: where `get_design` JSON and `get_part` are
      described, one sentence each. — done: one paragraph after the Read tools covering both
- [x] Tests: `AutomationTests`: place a flagged test part (registered as in
      Phase 1), assert `get_design` JSON has `activeLow` and the text form
      is unchanged (`designator:name` tokens identical to the unflagged
      case), and `get_part` shows ` low`. — done: `testActiveLowIsReported` (registers into `PartCatalogStore.shared`, which `place_part` reads, as `PartFileSaveOpenTests` does; compares `get_design` text with an unflagged copy line for line)

Acceptance: agents can see the flag; their pin references are unchanged.
  Check: `xcodebuild … test -only-testing:schematicalTests/AutomationTests` → `** TEST SUCCEEDED **` (est. 5 min).
  Observed 2026-11-14: `Executed 29 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: —

## Validation Plan

- Tests: `ChipPinActiveLowTests`, `PartEditorWindowTests`,
  `PartKindRoundTripTests`, `AutomationTests` (above).
- Coverage check: the new test file is in the `schematicalTests` target
  automatically (a synchronized group; Phase 1's `xcodebuild … -only-testing:schematicalTests/ChipPinActiveLowTests` runs ≥ 1 test, not "0 tests").
- Runtime proof: none beyond the tests; C shows the flag on screen.
- Doc sync: `docs/agent-control.md`; `docs/parts-format.md` was done in A.
- Final gate: plan-39-L, once.

## Decisions

- Per-alternative flag, names kept — owner, 2026-11-13 (plan-39-A).
- Author's calls: `get_design` text stays unchanged; the editor edits the
  flag through a menu; non-canonical input is normalised on save.

## Corrections

1. Phase 1's `ChipPinActiveLowTests.fileText` gained `flagged:` (default
   true) so Phase 3 can register an unflagged twin of the same part; the
   plan had Phase 3 "registered as in Phase 1" without saying how the
   unflagged case is built.

## Summary

The risk is the editor round trip. Nothing visible changes in the canvas.
