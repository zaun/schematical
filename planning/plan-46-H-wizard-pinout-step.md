# plan-46-H: Part Wizard — step 4 (pins): paste a pin table, map its columns, review the guesses

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-46-G

Typing a pin table row by row is the most tedious, error-prone part of
making a part. H lets the user copy the datasheet's pin table (from the
PDF pane or any PDF viewer) and paste it. The wizard splits it into
columns, guesses which column is the pin number (or ball), which is the
name and which is the type, and shows the rows in a grid with a column
picker over each column so the user can correct the guess. Applying it
fills the pinout with guessed roles (power, ground, input, output, …) and
active-low flags, which the user reviews in the existing pin table. Pin
names edited afterwards are carried into every kind field that uses them
(D's `renamePin`).

Behavioral outcome: pasting a TI "Pin Functions" table for a 14-pin quad
NAND (one row per pin: name, number, I/O, description) produces 14 pins
numbered 1–14, `VCC` power, `GND` ground, the `A`/`B` pins input and the
`Y` pins output; renaming `1A` to `A1` afterwards renames it in the gate's
`inputs`.

References:

- `Editor/PartEditorWindow.swift` `PinoutsSection` / `PinoutEditor`
  (:305–421, the pin grid), `Editor/PartDraft.swift` `PinDraft` (`roles`
  :18 = input, output, inout, power, ground, nc, absent; `signalRoles`
  :35; `alternatives` :38, the `/`-separated name alternatives;
  `activeLow`), `PinoutDraft.Style` (numbered, ball).
- `docs/parts-format.md` § Pinouts and pins (:106–193: designators, roles,
  names kept as the datasheet writes them, active-low rules).
- `PartDraft.renamePin(from:to:)` (D).
- `Datasheets/gate-and/cd74hc08.md` § Pin functions (the shape of a
  datasheet pin list: names grouped by function).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-G complete | `ls planning/completed/plan-46-G-*.md` → exists | NOT MET (2026-11-16: not started) |
| `renamePin` exists (D) | `grep -c 'func renamePin' schematical/schematical/Editor/PartDraft.swift` → 1 | NOT MET (2026-11-16: 0) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Editor/Wizard/PinTableParser.swift`:
  - `parse(_ text: String) -> PinTable` — rows split on newlines (blank
    lines dropped); columns split on tabs when any line has a tab, else on
    runs of 2+ spaces, else on single spaces; cells trimmed.
  - `guessColumns(_:) -> [PinColumn]` with `PinColumn` = `.number`,
    `.ball`, `.name`, `.type`, `.ignore`: a column whose non-empty cells
    are ≥ 80 % `^\d+(\s*,\s*\d+)*$` is a number column (the first such is
    `.number`, later ones `.ignore` — multi-package tables list one column
    per package); ≥ 80 % `^[A-Z]{1,2}\d{1,2}$` → `.ball`; ≥ 80 % type
    tokens (below) → `.type`; the first remaining column with letters →
    `.name`; the rest `.ignore`.
  - `pins(_ table:, columns:) -> (pins: [PinDraft], skipped: Int)` — a row
    with no designator in the number/ball column is skipped (headers,
    section rows) and counted; a cell `4, 7` gives two pins with the same
    name; roles and active-low from `PinRoleGuesser`.
- `Editor/Wizard/PinRoleGuesser.swift`:
  - type tokens (case-insensitive): `I`, `IN`, `INPUT` → input; `O`,
    `OUT`, `OUTPUT` → output; `I/O`, `IO`, `B`, `BIDIR` → inout; `P`,
    `PWR`, `POWER`, `S`, `SUPPLY` → power; `G`, `GND`, `GROUND` → ground;
    `NC`, `—`, `-` → nc.
  - names override the type for supplies: `VCC`, `VDD`, `VDDA`, `AVDD`,
    `DVDD`, `VIN`, `VBAT`, `V+`, `VS` (and these with a digit or `_x`
    suffix) → power; `GND`, `VSS`, `AGND`, `DGND`, `PGND`, `V-`, `EP`,
    `PAD`, `THERMAL PAD` → ground; `NC`, `N.C.`, `DNC` → nc.
  - no type and no name rule → input (the user reviews).
  - active low: per alternative of the name (`/`-separated), flagged when
    it ends with `#`, `_N`, `_B` or `*`, starts with `~`, starts with
    lowercase `n` followed by an uppercase letter (`nCS`), or carries a
    combining overline (U+0305) or a macron (U+0304) on any letter (a
    leading `/` is not a marker: `/` splits alternatives,
    `docs/parts-format.md:136–139`); roles outside `signalRoles` are never
    flagged. The name is kept exactly as pasted (§ Pinouts: names stay as
    the datasheet writes them, `:151`), except that combining overlines and
    macrons are removed: the flag carries the overbar, and the block symbol
    draws it (`:153–155`).
- Step 4 page `PinoutStepPage`: a paste box with "Paste the pin table from
  the datasheet"; after a paste, the parsed grid with a column picker
  above each column, "N pins, M rows skipped", and Apply (asks before
  replacing a non-empty pinout); "Start with N numbered pins" for users
  without a table; below, the existing `PinoutsSection` for review.
- `PinoutEditor`'s name field commits renames through `renamePin` on
  submit and on focus loss (never per keystroke); a status line says
  "Renamed in 2 settings".

### Non-goals (explicit constraints)

- **Pin designators and names are never invented**: a row the parser
  cannot place is skipped and counted, not guessed.
- **No OCR or PDF table extraction** — the user copies the text.
- **The All fields pin table is unchanged** apart from the rename commit.

## 2. Current State

- Pins are added one at a time (`PinoutEditor.addPin`, numbered next or
  `A<n>` for balls) and typed in a grid of designator, name, role, group,
  side, active-low (`PartEditorWindow.swift:340–421`).
- A renamed pin leaves kind fields pointing at the old name; the backend
  then rejects the draft (`unknown pin`).

## 3. Test fixtures (in `PinTableParserTests`, written as literal strings)

1. TI pin functions, tab-separated (what Preview copies from a TI table):
   `NAME\tNO.\tI/O\tDESCRIPTION`, then `1A\t1\tI\tInput`, … `GND\t7\t—\tGround`,
   … `VCC\t14\t—\tPower` — 14 pins; `GND` ground and `VCC` power by name
   (their type cell `—` would say nc); header row skipped (1).
2. Multi-package: `NAME\tD, N\tPW\tI/O` with two number columns → the first
   is `.number`, the second `.ignore`.
3. Space-aligned columns (2+ spaces) with a `4, 7` cell → two `GND` pins.
4. Ball table: `A1  VDD`, `A2  CS#`, `B1  GND`, `B2  nRESET` → style ball,
   `CS#` and `nRESET` active low, `VDD` power.
5. Overbar: `C̅S̅` (combining overline) → name `CS`, active low.
6. A section row (`Power pins`) inside the table → skipped and counted.

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Parser and guesser

- [ ] `PinTableParser.swift`, `PinRoleGuesser.swift` per § 1.
- [ ] `schematicalTests/PinTableParserTests.swift`: § 3's six fixtures, plus
      a role table test (each token and name rule once) and an active-low
      table test (each marker once, and `GND#`-style non-signal names
      unflagged).

Acceptance: the six fixtures give exactly the pins listed.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/PinTableParserTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — The page and the rename commit

- [ ] `Editor/Wizard/PinoutStepPage.swift`; `PinoutEditor` rename commit.
- [ ] Runtime check (run skill): gate wizard → Pins → paste fixture 1 →
      Apply → 14 pins with the roles above → rename `1A` to `A1` after the
      gates step has `1A` in `inputs` → the gates step shows `A1`.
      Pointer to (0, 0).

Acceptance: a pasted table becomes a reviewed pinout, and renames carry.
  Check: the walk above (est. 10 min).
Commit: —

## Validation Plan

- Tests: `PinTableParserTests`; D's `PinRenameTests` cover the rewrite.
- Runtime proof: Phase 2's walk.
- Doc sync: N (help: pasting pin tables).
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions. Author's calls (no user-visible trade-off beyond
the rules above): skipped rows are counted rather than guessed; a pasted
overbar becomes the active-low flag, which is how the library records one.

## Corrections

None yet.

## Summary

Text parsing behind six fixtures; the guesses are shown for review before
they are applied, so a wrong guess costs a click, not a broken part.
