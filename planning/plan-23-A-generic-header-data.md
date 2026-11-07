# plan-23-A: Generic header parts, footprints and backend kind

Last updated: 2026-11-07
Overall Effort: x-large (1d–3d)
Effort: large (3h–1d)
Depends on: nothing

plan-23 adds a generic 2.54 mm pin header under **Connector ▸ Header**, built
like plan-18's `GENERIC-LED`. A placed header has four options: **rows** (1 or
2), **pins per row** (1–40), **Male/Female**, and **footprint** (vertical or
right angle). All four are saved in the file and set in the Schematic sidebar.
Settings sets what a new header starts as: 1×2, Male, vertical.

This sub-plan does the backend and the data. A new passive part kind,
`header`, takes the device keys `rows` and `genders`. A new footprint style,
`header`, draws a zigzag-numbered pin header from a handful of numbers. A
kept generator script writes `Parts/header.json` (80 parts,
`GENERIC-HEADER-1X01` … `GENERIC-HEADER-2X40`) and the 160 footprints
(`PINHDR-<r>X<nn>-V` and `-RA`) from the Würth WR-PHD drawings. plan-23-B
does the app's catalog, placed-part model, file and symbol. plan-23-C adds the
sidebar pickers and the Settings defaults.

Outcome: `listing_check --print` has 80 `GENERIC-HEADER-` lines, each with
`|choices=Straight:PINHDR-…-V,Right angle:PINHDR-…-RA`.
`circuit_add_part(c, "GENERIC-HEADER-2X20", …)` works and passes a signal
through a wire to its pin. `footprint_check` passes with the 160 new
footprints in use.

References:

- `docs/parts-format.md` § Parts, § Footprints (style sections, § The
  `"footprint"` part key, the `footprintChoices` rules from plan-18), § Kinds
- `docs/backend.md` § Tests (golden regeneration)
- `planning/completed/plan-18-A-generic-led-data.md` (the `footprintChoices`
  loader and the GENERIC-LED pattern this mirrors)
- `planning/completed/plan-03-B-logic-footprints.md` §3 (pad rules R1–R6)
- `backend/src/library/footprints.c` (`load_dual` :414, `load_explicit`
  :613, the style switch :749)
- `backend/src/models/connector.c` (the passive kind this mirrors)
- `backend/src/library/kinds.c:26` (`KINDS[]`), `backend/include/parts.h:27` (`PartKind`)

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-18 (the `footprintChoices` part key) is on main | `ls planning/completed/plan-18-A-* planning/completed/plan-18-B-*` → 2 files; `grep -c footprintChoices backend/src/library/library.c` → ≥ 1 | MET (2026-11-07: 2 files; 11) |
| No `header` kind or `PINHDR-` footprint exists yet | `grep -c '"kind": "header"' schematical/schematical/Parts/*.json \| grep -v ':0'` → no lines; `grep -c PINHDR schematical/schematical/Parts/footprints.json` → 0 | MET (2026-11-07) |
| Backend tests pass before starting | `rm -rf /tmp/p23-build && cmake -S backend -B /tmp/p23-build && cmake --build /tmp/p23-build -j8 && ctest --test-dir /tmp/p23-build -j8` → 21/21 | MET (2026-11-07, as `/tmp/p18-build`: 21/21) |
| Node runs the generator | `node --version` → any version | MET (2026-11-07: `/Users/justinzaun/local/bin/node`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The library has 80 header parts, `GENERIC-HEADER-<R>X<NN>` for R = 1, 2 and
  NN = 01 … 40, kind `header`, drawer `["Connector","Header"]`, each with a
  vertical (`Straight`) and a right-angle (`Right angle`) footprint choice.
- The backend simulates a header as passive terminals: wires through a
  header's pin join the pin's net and nothing else.
- Each footprint matches the Würth WR-PHD drawing for its type.

### Non-goals (explicit constraints)

- No app changes beyond naming the kind (`PartFacts.kindName`, which has a
  `default` arm, so the app builds either way); the catalog, symbol and file
  are plan-23-B.
- No SMD footprints and no pitches other than 2.54 mm (owner decision).
- Male and Female use the **same footprint** (owner decision: gender changes
  only the symbol and label). The kind lists the genders; nothing in the
  backend acts on them.
- Existing footprints, including the LCD's `HDR-1X16-P2.54`, are not changed.
  `footprints.txt` only gains lines.
- The in-stock rule in `AGENTS.md` is for adding orderable parts from a
  datasheet. A generic header is not an orderable part number, like
  `GENERIC-LED`. Its footprints come from datasheet drawings as the
  footprint rules require.

## 2. Current State

- **Connector kind.** `backend/src/models/connector.c` is the only passive
  multi-pin kind. Device keys `centerPin`, `ratedVolts`, `ratedAmps` and
  `contacts` are barrel-jack specific, and `connector_bind_pinout` rejects
  fewer than 2 pins (`a connector has at least two pins`). Its model
  `CONNECTOR_MODEL` has `connector_pin_count`/`connector_create` and no
  pin-change hook. `circuit.c:151, 327, 356` special-case `PART_CONNECTOR`
  for the jack contacts. A 1×1 header has 1 pin, so headers can't reuse this
  kind.
- **Kinds.** `KINDS[]` in `backend/src/library/kinds.c:26` lists each
  `PartKindOps`, and `PartKind` (`parts.h:25-28`) ends with `PART_PIO`. New
  backend sources are listed by hand in `backend/CMakeLists.txt` (`src/models/*.c`
  from line 18). The Xcode project compiles `../backend/src` as a
  synchronized folder (`project.pbxproj:30-33`), so it needs no edit.
- **Footprint styles.** `footprints.c` has `dual`, `quad`, `grid` and
  `explicit` (switch at :749). `dual` numbers pins counter-clockwise (down
  the left column, up the right), the DIP order, and needs two columns. A
  pin header numbers zigzag (pin 2 across from pin 1). The only existing
  header footprint, `HDR-1X16-P2.54` (`footprints.json:208`), is an
  `explicit` list of 16 pads.
- **Footprint choices.** plan-18 added `footprintChoices` (`library.c:
  load_footprint_choices`). A choice must have a pad for every pin, and the
  part's own `footprint` must be one of them with the part's `package`.
- **Goldens.** `listing_check --print` writes `parts.txt`, one line per part
  sorted by name. `trace_check --print` writes `traces.txt`.
  `footprints.txt` has one line per footprint, and `footprint_check`
  compares it and fails on a footprint no part uses (`footprint_check.c`,
  which counts `footprint_choices` since plan-18).
- **Datasheets.** `datasheets/connector-barrel/` holds the barrel-jack PDFs.
  There is no header datasheet in the repo.

### The WR-PHD drawings (read 2026-11-07, page 1 of each)

| Order code | Type | URL | Numbers used |
|---|---|---|---|
| 61300111121 | 1 row, straight, male | `https://www.we-online.com/components/products/datasheet/61300111121.pdf` | hole Ø1.10 ±0.15, pitch 2.54, body depth 2.54 ±0.15, L = pins × 2.54 |
| 61300421121 | 2 rows, straight, male | `…/61300421121.pdf` | hole Ø1.10 ±0.15, pitch and row pitch 2.54, body depth 5.08 ±0.15, pins 1 and 2 across the rows |
| 61300211021 | 1 row, right angle, male | `…/61300211021.pdf` | hole Ø1.1 ±0.20, pitch 2.54, housing 2.54 ±0.15 deep, its near face 1.5 ±0.2 from the hole row |
| 61300421021 | 2 rows, right angle, male | `…/61300421021.pdf` | hole Ø1.02 ±0.05, pitch and row pitch 2.54, housing 2.54 deep, near face 1.5 ±0.25 from row 1 (the row with pin 1), row 2 on the far side |

No drawing gives a pad diameter, so pads follow R5 (`plan-03-B` §3: round,
drill + 0.7, at least 1.5): Ø1.80 on a 1.10 drill and Ø1.72 on a 1.02 drill.
Pin 1's pad is square, as on the drawings' pin-1 marks and `HDR-1X16-P2.54`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts added | 80 | 2 row counts × 40 sizes (owner decision: 1×N and 2×N, N 1–40) |
| Footprints added | 160 | 80 sizes × 2 (vertical, right angle) |
| `parts.txt` / `traces.txt` lines today | 8022 / 8022 | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt` |
| `footprints.txt` lines today | 237 | `wc -l backend/tests/golden/footprints.txt` |
| `GENERIC-HEADER` or `PINHDR` anywhere today | 0 | `grep -rc 'GENERIC-HEADER\|PINHDR' schematical/schematical/Parts backend` → all 0 |

### Verified properties

- **A choice footprint may have more pads than pins, but no header needs that.**
  Every header footprint has exactly rows × perRow pads, one per pin, so the
  default (vertical) choice also passes `load_part_footprint`'s strict pad
  count. (Read `library.c:load_part_footprint` and `check_pads`.)
- **Pads may sit off the body centre.** A footprint's pads are in mm from the
  body centre (`parts.h:FootprintPad`), and `explicit` footprints already put
  pads anywhere. The app draws body and pads from those numbers
  (`BoardModel.swift:boardPads`, `boardBody`), so a right-angle footprint
  whose holes sit beside its housing draws correctly. (Read both.)
- **`PART_HEADER` is safe to append.** `PartKind` is a C enum whose values the
  app reads by name (`PART_LED` etc.; `grep -rn 'rawValue ==' schematical/schematical | grep -i kind`
  → no lines), and no file stores a kind number.

## 3. Design Overview

Three pieces, in risk order: the footprint style (with tests on fixtures),
the kind (with tests on fixtures), then the generated data and goldens.

**Gate class:** behavior change. Goldens gain lines only:
`parts.txt` +80, `traces.txt` +80, `footprints.txt` +160, and no existing
line changes. Any other diff is a bug to root-cause
(`listing_check --verbose`, `footprint_check --verbose`); it never means the
design is wrong.

### Footprint style `header`

```json
{"id":"PINHDR-2X08-RA","description":"2x8 pin header, 2.54 mm pitch, right angle",
 "note":"…","style":"header","pins":16,"rows":2,
 "body":{"width":20.32,"height":2.54},
 "pitch":2.54,"rowPitch":-2.54,"firstRowY":-2.77,
 "pad":{"width":1.72,"height":1.72,"shape":"round"},"drill":1.02,"pin1Shape":"rect"}
```

- Keys (besides the common ones): `rows` (1 or 2), `pitch` (> 0), `rowPitch`
  (non-zero, required when `rows` is 2, forbidden when 1), `firstRowY`
  (number), `pad`, `drill` (> 0), `pin1Shape` (optional, as on `dual`).
- `pins` must be a multiple of `rows`. perRow = pins / rows.
- Pin p (1-based): column c = (p − 1) / rows, row k = (p − 1) % rows
  (zigzag: 1 and 2 across the rows, 3 and 4 next). Pad centre:
  x = (c − (perRow − 1) / 2) · pitch, y = firstRowY + k · rowPitch.
- Errors: `"rows" must be 1 or 2`, `"pins" 7 is not a multiple of "rows" 2`,
  `"rowPitch" is required with 2 rows`, `"rowPitch" is not allowed with 1 row`.

The four shapes (body centre at the origin, y down):

| Footprint | body w × h | firstRowY | rowPitch | drill / pad |
|---|---|---|---|---|
| `PINHDR-1XNN-V` | N·2.54 × 2.54 | 0 | — | 1.10 / 1.80 |
| `PINHDR-2XNN-V` | N·2.54 × 5.08 | −1.27 | +2.54 | 1.10 / 1.80 |
| `PINHDR-1XNN-RA` | N·2.54 × 2.54 | −2.77 | — | 1.10 / 1.80 |
| `PINHDR-2XNN-RA` | N·2.54 × 2.54 | −2.77 | −2.54 | 1.02 / 1.72 |

For right angle, the body is the housing on the board. Its near face is
1.5 mm from row 1, so row 1 sits 1.27 + 1.5 = 2.77 mm above the body centre,
and row 2 a further 2.54 mm away. The pins' free ends past the housing are
not drawn, and neither is `HDR-1X16-P2.54`'s.

### Kind `header`

- Device keys `rows` (integer 1 or 2) and `genders` (non-empty array of
  strings, the first is the default; read with `kind_names`). No part keys.
- Bind: pin count ≥ 1 and a multiple of `rows`
  (`a header's pin count must be a multiple of its rows`).
- `PartHeader { unsigned rows; const char *const *genders; size_t gender_count; }`,
  `Part.header`. `PART_HEADER` appended to `PartKind`.
- Model: passive, like a connector with no contacts: `pin_count`, `create`
  (an empty state), no hooks. `circuit.c` needs no change: its connector
  special cases test `PART_CONNECTOR` only.
- `listing.c` writes `rows=<r> genders=Male,Female`.

### Data: `Parts/header.json` and the generator

`scripts/gen-headers.js` (kept) writes both files, and running it twice
changes nothing:

- `schematical/schematical/Parts/header.json`: two devices, `header-1row`
  (`rows` 1) and `header-2row` (`rows` 2). Each has `function` `"Header"`,
  `description` `"Generic pin header, 1 row"` / `"…, 2 rows"`, drawer
  `["Connector","Header"]`, `genders` `["Male","Female"]`, one pinout per
  size (`"1x08"`: pins `{"number":n,"name":"<n>","role":"inout"}`), and one
  part per size:
  `{"name":"GENERIC-HEADER-1X08","package":"Straight","pinout":"1x08","footprint":"PINHDR-1X08-V","footprintChoices":[{"package":"Straight","footprint":"PINHDR-1X08-V"},{"package":"Right angle","footprint":"PINHDR-1X08-RA"}]}`.
- `footprints.json`: every line whose id starts with `PINHDR-` is replaced
  by the 160 generated lines, appended at the end of the array when there
  are none yet. Every other line stays byte-identical. Each `note` names the
  order code, file and page, for example `"Würth WR-PHD 61300421021 p.1:
  hole Ø1.02, row pitch 2.54, housing 1.5 from row 1; pad R5 (drill + 0.7)"`.

Rejected alternatives:

- **80 `explicit` footprints per type.** 160 footprints × up to 80 pads
  would be 6,480 pad objects written out. The `header` style keeps each
  footprint to one short line, and the zigzag numbering lives in one tested
  function.
- **Reusing `dual`.** Its counter-clockwise numbering is wrong for headers,
  and it can't do one row.
- **Reusing the `connector` kind.** Its device keys are barrel-jack ratings,
  and it rejects one-pin parts.
- **One backend part with a pin-count option.** The circuit API adds a part
  by name with the part's single pinout (`circuit_add_part`), and the app's
  layout, netlist and wire ends come from that pinout. One part per size
  keeps all of that unchanged. plan-23-B swaps the placed part when the size
  changes.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial, with what remains. Mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`. Fill each `Commit:` line as it lands.
> An unticked box means NOT DONE.

### Phase 1 — Footprint style `header`

- [ ] `backend/src/library/footprints.c`: `HEADER_KEYS`, `load_header`
      (§ Footprint style) and its `"header"` case in the style switch.
- [ ] `backend/tests/footprint_check.c` (or the file holding the footprint
      fixture cases, `library_check.c:check_footprint_cases`): a valid
      `rows` 2, `pins` 6 fixture whose pads 1–6 sit at
      (−2.54, −1.27), (−2.54, 1.27), (0, −1.27), (0, 1.27), (2.54, −1.27),
      (2.54, 1.27), with pad 1 rect and the rest round; a `rows` 1 fixture with
      `firstRowY` −2.77; one `expect_fail` per § error message (4).
- [ ] `docs/parts-format.md` § Footprints: a `### header` section with the key
      table, the numbering formula and an ASCII sketch, beside `dual`.

Acceptance: the style lays out zigzag pads and rejects bad keys. Nothing else changes.
  Check: `cmake --build /tmp/p23-build -j8 && ctest --test-dir /tmp/p23-build -j8`
  → 21/21; `/tmp/p23-build/library_check | grep -c '^FAIL'` → 0, and the 6
  new cases print `ok`; `git diff --stat backend/tests/golden` → empty (est. 1 min).
Commit: —

### Phase 2 — Kind `header`

- [ ] `backend/include/parts.h`: `PART_HEADER` after `PART_PIO`; `PartHeader`;
      `const PartHeader *header` on `Part`.
- [ ] New `backend/src/models/header.c` (§ Kind); `HEADER_KIND` in
      `kinds.c` (extern + `KINDS[]`); `src/models/header.c` in
      `backend/CMakeLists.txt`.
- [ ] `backend/tests/support/listing.c`: `case PART_HEADER` → `rows=… genders=…`.
- [ ] `schematical/schematical/Backend/PartFacts.swift:kindName`:
      `case PART_HEADER: return "Header"` (otherwise the `default` arm
      shows an empty Kind row).
- [ ] `backend/tests/library_check.c`: a valid 2-row device with parts of 2
      and 4 pins (`part->header->rows == 2`, 2 genders, `genders[0]` Male);
      `expect_fail`: `rows` 3; empty `genders`; a 3-pin part on a 2-row
      device.
- [ ] `docs/parts-format.md` § Kinds: a `### header` section (device keys,
      no part keys, passive, the bind rule).

Acceptance: the kind loads and rejects as specified, and existing data is unchanged.
  Check: the Phase 1 command → 21/21, 0 FAIL, the 4 new cases `ok`;
  `git diff --stat backend/tests/golden` → empty (est. 1 min).
Commit: —

### Phase 3 — Data, generator, datasheets and goldens

- [ ] `datasheets/connector-header/`: download the four PDFs of § The WR-PHD
      drawings (`curl -fLo datasheets/connector-header/<code>.pdf <url>`).
      Each file is a PDF (`file … → PDF document`).
- [ ] New `scripts/gen-headers.js` (§ Data). Run it:
      `node scripts/gen-headers.js` writes `Parts/header.json` and the
      `PINHDR-` lines of `footprints.json`.
- [ ] Run it a second time: `git diff --stat` shows no change from the
      second run.
- [ ] Regenerate: `/tmp/p23-build/listing_check --print > backend/tests/golden/parts.txt`,
      `/tmp/p23-build/trace_check --print > backend/tests/golden/traces.txt`,
      and `/tmp/p23-build/footprint_check --print > backend/tests/golden/footprints.txt`
      (`docs/backend.md:260`).
- [ ] `backend/tests/circuit_check.c`: a `GENERIC-HEADER-2X20` whose pin 1
      and pin 40 are on nets from a `5V` and a `GND`, and a `GENERIC-LED`
      wired from pin 1 (A) to pin 40 (K). After `circuit_advance`, the LED
      is lit. That shows header pins pass nets through.
- [ ] `docs/parts-format.md` § `header`: name `GENERIC-HEADER-<R>X<NN>`, the
      generator, and the two footprint choices.

Acceptance: 80 header parts and 160 footprints load, every footprint is
used, and the golden diffs only add lines.
  Check: `ctest --test-dir /tmp/p23-build -j8` → 21/21.
  `git diff -U0 backend/tests/golden | grep -c '^-[^-]'` → 0 (no line removed or changed);
  `git diff -U0 backend/tests/golden/parts.txt | grep -c '^+GENERIC-HEADER-'` → 80;
  `… traces.txt | grep -c '^+GENERIC-HEADER-'` → 80;
  `… footprints.txt | grep -c '^+PINHDR-'` → 160.
  `/tmp/p23-build/listing_check --print | grep '^GENERIC-HEADER-2X08|'` →
  one line with `rows=2 genders=Male,Female`, 16 pins and
  `choices=Straight:PINHDR-2X08-V,Right angle:PINHDR-2X08-RA` (est. 2 min).
Commit: —

## Validation Plan

- Tests: `library_check` covers the style and the kind, accepting and
  rejecting. `circuit_check` runs a header in a circuit. The goldens pin all
  80 parts and 160 footprints.
- Coverage check: `footprint_check` reports no unused footprint, so every
  `-RA` footprint is reached through `footprintChoices`.
- Doc sync: `docs/parts-format.md` (style, kind, part naming).
- Final gate (once, after Phase 3):
  `cmake --build /tmp/p23-build -j8 && ctest --test-dir /tmp/p23-build -j8` → 21/21;
  `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p23-dd CODE_SIGNING_ALLOWED=NO build`
  → `BUILD SUCCEEDED` (the app compiles with `PART_HEADER`) (est. 4 min).

## Decisions

- One header family, 1×N and 2×N with N 1–40. Owner, 2026-11-07.
- Footprints: 2.54 mm through-hole, vertical and right angle. Owner,
  2026-11-07. Settles the 160 footprints and the two choices.
- Male/Female changes only the symbol and label; both use the same
  footprint. Owner, 2026-11-07. So `genders` is data for the app only.
- Pins zigzag: pin 2 across the rows from pin 1. Owner, 2026-11-07. Settles
  the `header` style's numbering.
- New headers start as 1×2, Male, vertical, configurable in Settings.
  Owner, 2026-11-07 (plan-23-C). Here: `Straight` is each part's default
  footprint and `Male` the first gender.
- Shrinking a header removes wires on pins that go away. Owner, 2026-11-07
  (plan-23-B).

## Corrections

## Summary

The risk here is low and contained: one footprint function with a formula,
tested on fixtures, and a passive kind. The 160 footprints and 80 parts are
generated, so a wrong number is fixed once in the script. The goldens only
gain lines, and `footprint_check` proves every footprint is used.
