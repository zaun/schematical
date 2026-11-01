# plan-02-B: Schematic file version 2, pins by designator

Last updated: 2026-10-31
Effort: medium (1h–2h)
Depends on: plan-02-A

Moves `.schematical` to version 2, where a wire end names its pin by
designator: `{"part": <id>, "pin": "14"}` or `{"part": <id>, "pin": "A1"}`.
Version 1 files (`"pin": 14`) still open and are converted on load. Version
1 pin n is the part's n-th pin in its pinout. Saving always writes version 2.

Outcome: opening `test.schematical` (version 1) and saving it gives the same
schematic with `"version": 2` and every `"pin"` a string of the same pin.
Reopening the saved file gives the same wiring, and Play gives the same
result.

References:

- `docs/file-format.md` ("Changing the format": renaming or changing a field's
  meaning bumps `version`, older versions are converted on open, and newer
  versions are refused).
- `planning/plan-02-A-pin-designators.md` §4.6 (`PinRef.index`, `ChipPin.designator`).
- `schematical/schematical/Document/SchematicFile.swift`.

## Prerequisites

See plan-02-A for the plan-wide prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-02-A complete | `ls planning/completed/plan-02-A-* && ! ls planning/plan-02-A-* 2>/dev/null` → the completed path only | NOT MET (2026-10-31) |

## 1. Goal

- The app saves version 2 with string `"pin"` designators, opens version 2,
  and converts version 1. A version 1 `"pin": n` becomes the part's pin at
  position n − 1.

### Non-goals (explicit constraints)

- No other field changes: parts, junctions, bends, ids, `switches`,
  `nextReference` keep their meaning and encoding.
- The app still refuses files newer than it supports (now `version > 2`).
- No backend or part data change. Ball data is plan-02-C.
- No mixed files: a version 1 file must use integer pins, a version 2 file
  string pins.

## 2. Current State (after plan-02-A)

- `SchematicFile.currentVersion = 1` (`SchematicFile.swift:13`).
- `End.pin(part: UUID, number: Int)` decodes and encodes an integer `"pin"`
  (`SchematicFile.swift:54-83`).
- `decode(_:)` checks the header's `format`/`version`, then decodes the whole
  file (`:170-190`). `apply(to:catalog:)` resolves ends
  (`:236-252`). After plan-02-A, `"pin": n` maps to `PinRef(index: n - 1)`
  after checking a drawn slot has that index.
- `missingPin(wire:reference:pin: Int)` reads "…connects to pin \(pin) of
  \(reference), which has no such pin." (`:112, :129-130`).
- `docs/file-format.md` documents version 1 and integer pins (sections "Wires",
  "Validation on open", "Example").
- `test.schematical` (repo root) is a version 1 file with 5 parts and 9 integer
  pins (`grep -c '"pin" :' test.schematical` → 9).

### Measured populations

| What | Count | Command |
|---|---|---|
| Wire-end pins in `test.schematical` | 9 | `grep -c '"pin" :' test.schematical` → 9 |
| Swift files touched | 1 (+ doc) | only `SchematicFile.swift` encodes/decodes ends (`grep -rln "SchematicFile.End\|case pin(part" schematical/schematical` → `Document/SchematicFile.swift`) |

### Verified properties

- **Version 1 pin n means position n − 1 for every part.** Before plan-02-C,
  every pinout is numbered 1..N in order (loader rule, `library.c:load_pinout`).
  Plan-02-C keeps each converted BGA pinout in the position order of the
  numbered pinout it replaces (plan-02-C §3). So version 1 files saved before
  plan-02-C still mean the same pin on every part.

## 3. Design

- `currentVersion = 2`.
- The file's pin value is `enum PinKey: Equatable, Sendable { case designator(String), position(Int) }`.
  `End.pin(part: UUID, pin: PinKey)` decodes a string as `.designator` and an
  integer as `.position`; any other type is a decoding error. Encoding
  `.designator(s)` writes the string. The app only ever builds
  `.designator` for saving.
- `decode(_:)`, after the full decode, rejects mixed files:
  - version 1 with a `.designator` end throws
    `unreadable("version 1 files name pins by number")`;
  - version 2 with a `.position` end throws
    `unreadable("version 2 files name pins by designator, such as \"A1\"")`.
- `apply(to:catalog:)` resolves:
  - `.position(n)` to the slot whose `pin.index == n - 1`;
  - `.designator(s)` to the slot whose `pin.designator == s`.

  Only drawn slots count, as today, and either case gives `PinRef(index:)`.
  `missingPin` takes `pin: String`: the designator, or `"\(n)"` for a
  version 1 number.
- `End.init(_ end: WireEnd)` gets the designator from the chip:
  `chip.part.pins[ref.index].designator`. The helper therefore takes the
  document's chips, or the `init(document:)` loop looks the chip up by
  `ref.chipID`.

**Gate class:** behavior changes on purpose (the saved bytes change).
Byte-identity is not the gate. The gate is the round-trip runtime proof plus
the error cases.

**Rejected alternatives**

- *Accept both pin types in any version.* Old builds would then hit a
  decoding error on version 1 files with string pins, instead of the clean
  "newer version" message the format rules require.
- *Store the pin index in version 2.* It is not readable, and a future
  pinout reorder would silently rewire files. A designator is stable and is
  what the datasheet prints.

## Compatibility / Format Impact

- `.schematical` **version 2**: `"pin"` is a string designator. Version 1
  files open and are converted, and saving writes version 2. Builds from
  before this plan refuse version 2 files with "saved by a newer version of
  Schematical (file format 2)".
- Unchanged: every other field.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. Use `- [~]` for partial work, plus one line on what
> remains. Mark moot tasks `- [x] ~~text~~ — moot: <evidence>`. Fill
> `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — Version 2 in the app

- [ ] `Document/SchematicFile.swift`: `currentVersion = 2`; `PinKey`; `End.pin(part:pin:)` coding (§3).
- [ ] `Document/SchematicFile.swift:decode`: reject mixed files with the two §3 messages.
- [ ] `Document/SchematicFile.swift:apply`: resolve `.position`/`.designator` to `PinRef(index:)`; `missingPin(… pin: String)`.
- [ ] `Document/SchematicFile.swift`: saving writes the designator of `chip.part.pins[ref.index]`; the doc comment on `SchematicFile` says version 2.

Acceptance: the app builds.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-bga-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 30 s). The behavior check is the runtime proof below; no smaller check exercises the decoder, because the app has no Swift test target (`find schematical -name '*Tests*'` → none).
Commit: —

### Phase 2 — Docs

- [ ] `docs/file-format.md`:
  - "describes version 2";
  - "Wires": an end is `{"part": <id>, "pin": "<designator>"}`, the pin's
    designator from the part library (`"14"`, `"A1"`);
  - a new "Version 1 files" section: `"pin"` is an integer, pin n is the
    part's n-th pin in its pinout, files are converted on open and saved as
    version 2;
  - "Validation on open": the mixed-file rule;
  - the Example: `"version": 2`, `"pin": "1"`, `"pin": "2"`.

Acceptance: the doc describes version 2 and the conversion.
  Check: `grep -nE '"version": 1|"pin": [0-9]' docs/file-format.md` → matches only inside the "Version 1 files" section (est. seconds).
Commit: —

## Validation Plan

- Tests: none automated (no Swift test target). The runtime proof covers
  decode, convert, encode and every error.
- Runtime proof (with the `run` skill, Debug build):
  1. Open `test.schematical` (v1). Save As `/tmp/plan02b.schematical`. Then
     `jq '.version, [.wires[] | .from.pin, .to.pin | select(. != null)]' /tmp/plan02b.schematical`
     prints `2` and nine strings, each the decimal of the v1 value at the same
     place.
  2. Reopen `/tmp/plan02b.schematical`. The wires attach to the same pins,
     and Play gives the lit LED, as with `test.schematical`.
  3. Edit copies in `/tmp`:
     - `jq '.wires[0].from.pin = "99"'` on the v2 file → open fails with
       "connects to pin 99 of U…, which has no such pin";
     - `jq '.wires[0].from.pin = 1'` on the v2 file → "version 2 files name
       pins by designator";
     - `jq '.wires[0].from.pin = "1"'` on `test.schematical` → "version 1
       files name pins by number";
     - `jq '.version = 3'` → "saved by a newer version of Schematical (file
       format 3)".
- Doc sync: `docs/file-format.md`.
- Final gate (run once, after Phase 2): the Phase 1 `xcodebuild` (est. 30 s)
  and the runtime proof (est. 15 min). The backend suite is untouched by this
  letter, so it is not rerun.

## Decisions

- Version 2 with string pins everywhere, version 1 converted on open (pin n = n-th pin). Decided by the owner, 2026-10-31. Old builds refuse v2 cleanly. (§3)
- No mixed files. Author's call: one meaning per version keeps the newer-version refusal honest. (§3)

## Corrections

## Summary

Small and contained: one Swift file and one doc. The risk is the version 1
conversion. It depends on plan-02-C keeping converted pinouts in their old
position order, and plan-02-C §3 makes that a rule. The runtime proof
round-trips `test.schematical`, the only version 1 file in the repo.
