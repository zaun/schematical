# plan-11-A: Character LCD modules — groundwork

Last updated: 2026-11-01
Overall Effort: x-large (1d–3d) — plan-11 is A → B → C → D
Effort: medium (1h–2h)
Depends on: nothing (§ Prerequisites)

plan-11 adds 16×2 character LCD modules: HD44780-compatible modules with a
built-in controller, character ROM, glass and backlight. They appear in the
parts drawer under **Displays → Character**. Each simulates its controller
from the datasheet: the 8- and 4-bit bus, the instruction set, the busy
flag, bus timing and the character ROM. The schematic symbol shows the live
display face while the simulation runs. On the PCB side, each module's
footprint has a **Style** option: **Full Module** (the module's outline,
mounting holes and header holes) or **Connector Only** (a plain 1×16 header,
pins 1–16 in order).

The single behavioral outcome of plan-11: a schematic with an MPS6502, an
AT28C256 holding a program, two HC gates and an NHD-0216K1Z-NSW-BBW-L shows
`HELLO` on the LCD's face in the running app. The same module's PCB
footprint switches between the 80 × 36 mm module and the 1×16 header from
the PCB sidebar, and the choice survives save and open.

This sub-plan does the groundwork no new kind needs yet:

- the datasheet folder: drop the two Lumex drawing sheets (owner's choice)
  and record them; commit the five PDFs that stay;
- the footprint format: a pin may have **more than one pad** (the
  NHD-0216K1Z has two header rows carrying the same 16 pins);
- a new common part key, `"connectorFootprint"`: the second footprint the
  PCB Style option switches to. It goes into the `Part` struct, the listing
  and `footprint_check`.

Outcome of A: `ctest` 17 of 17; `parts.txt`, `traces.txt` and
`footprints.txt` are byte-identical to today's (no part names either new
thing yet).

References:

- `AGENTS.md` (datasheet rules, part files, footprints, goldens).
- `docs/parts-format.md` § Footprints (§ `explicit`, § The `"footprint"`
  part key), § Parts; `docs/backend.md` § Tests.
- `planning/completed/plan-10-A-groundwork.md` (the last groundwork
  sub-plan; same shape).

## Prerequisites

This table is the gate for all of plan-11. B–D point here and add their own
rows.

| Must be true | Command | Status |
|---|---|---|
| plan-09 complete | `ls planning/plan-09-* 2>/dev/null` → no output; `ls planning/completed/plan-09-E-cpu-app.md` → exists | MET (2026-11-01 at 01c8377) |
| plan-10 complete | `ls planning/plan-10-* 2>/dev/null` → no output | MET (2026-11-01 at 01c8377) |
| Backend tests green | `cmake -S backend -B /tmp/logic-p11 && cmake --build /tmp/logic-p11 -j8 && ctest --test-dir /tmp/logic-p11` → `100% tests passed, 0 tests failed out of 17` | MET (2026-11-01 at 01c8377: 17 of 17) |
| Goldens at their measured size | `wc -l < backend/tests/golden/parts.txt` → 7157; `traces.txt` → 7157; `footprints.txt` → 195 | MET (2026-11-01 at 01c8377: 7157/7157/195) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p11-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01 at 01c8377) |
| No character LCD kind yet | `ls schematical/schematical/Parts/charlcd.json backend/src/models/charlcd*.c backend/include/lcd_state.h 2>/dev/null` → no output | MET (2026-11-01 at 01c8377) |
| The datasheets are present | `ls datasheets/module-disp-char/*.pdf \| wc -l` → 7 | MET (2026-11-01: 7) |
| The five parts are in stock | `source ~/.zshrc >/dev/null 2>&1; for p in NHD-0216BZ-FL-YBW NHD-0216K1Z-FL-YBW NHD-0216K1Z-NSW-BBW-L "162H BC BW" AMC1602GR-B-Y6WFDY; do node scripts/digikey.js --in-stock "$p" \| grep -c Active; done` → five lines of `1` | MET (2026-11-01: 4045, 7648, 3688, 114, 118 in stock) |

If the goldens or the test count differ at the start (another change landed
in between), record the new values as **P0** (`parts.txt` = `traces.txt`
lines), **F0** (`footprints.txt` lines) and **T0** (ctest count) here.
Every later count in plan-11 is P0 + n, F0 + n, T0 + n.

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `ls datasheets/module-disp-char/*.pdf | wc -l` → 5, and
  `datasheets/not-included.md` has the two Lumex rows of §4.1.
- `ctest --test-dir /tmp/logic-p11` → `100% tests passed, 0 tests failed out of 17`.
- `git diff --exit-code backend/tests/golden` → exit 0 (byte-identical).
- `library_check` has new cases: a footprint whose pin has two pads loads;
  `connectorFootprint` loads and is checked like `footprint` (§4.3).

### Non-goals (explicit constraints)

- No existing footprint, part, listing line or trace hash changes. This
  sub-plan is **provably neutral** for the catalog: byte-identical goldens
  are its gate.
- `"footprint"` keeps every rule it has. `"connectorFootprint"` is optional;
  no existing part gets one.
- The pad order the loader produces for every existing footprint is
  unchanged (§4.2).
- No new kind and no app change. The app compiles `backend/src/` directly,
  so it must still build: the new `Part` field is appended.

## 2. Current State

- **Datasheets.** `datasheets/module-disp-char/` is untracked
  (`git status --short` → `?? datasheets/module-disp-char/`). It holds 7 PDFs:
  `162H-BC-BW-Displaytech-Spec.pdf`, `AMC1602G-full.pdf`,
  `LCM-X01602DXX-D.pdf`, `LCM-x01602Dxx+A-page1[1].pdf`,
  `NHD-0216BZ-FL-YBW.pdf`, `NHD-0216K1Z-FL-YBW.pdf` and
  `NHD-0216K1Z-NSW-BBW-L.pdf`. `datasheets/not-included.md` lists PDFs the
  owner left out, one table row each (`| PDF | Family | Category / group | Notes |`).
- **Explicit footprints, one pad per pin.** `load_explicit`
  (`backend/src/library/footprints.c:613-690`) keeps one slot per pin
  (`where[number]`). A second pad for the same pin fails:
  `"pin %u has two pads"` (`footprints.c:665-667`). The output order is pin
  1…N, then the unnumbered pads (`pin: 0`) in file order (`footprints.c:676-688`).
  `parts.h:PartFootprint` documents that order (`"1"…"N" order (grid:
  row-major), unnumbered last`). `pins` is the count of designated pins
  (1–1000), and `load_part_footprint` (`library.c:361-391`) requires
  `footprint->pins == part->pin_count` and a pad for every pin's designator.
- **The `footprint` part key.** `PART_KEYS` (`library.c:188`) is
  `name package pinout flags footprint`. `load_part_footprint` resolves and
  checks it, and sets `part->footprint`. Supplies reject it.
- **Listing.** `tests/support/listing.c:151-152` ends a part's line with
  `|footprint=<id>` when it has one.
- **footprint_check** (`backend/tests/footprint_check.c:141-155`): every
  footprint must be some part's `->footprint`, else `unused footprint: <id>`.
- **The app reads `Part`** through the bridging header and copies it in
  `CatalogPart.init` (`schematical/schematical/Backend/PartCatalog.swift`).
  A new field appended to `Part` does not change any existing read.
- **Duplicate pads in the app.** The app reads pads only through
  `footprint.pads` (`grep -rn '\.pads' --include='*.swift'
  schematical/schematical` → 5 sites: the copy in `PartCatalog.swift:320`
  and 4 in `BoardModel.swift`). `BoardModel.swift:96` maps every pad
  on its own. `:127` (pin-1 mark) and `:177` (pad-1 origin) take the
  *first* pad with the designator; `:151` (bounds) unions all pads. None
  keys a dictionary by designator, so a pin with two pads draws both and
  uses the first for pin 1.

### Measured populations

| What | Count | Command |
|---|---|---|
| PDFs in the folder | 7 | `ls datasheets/module-disp-char/*.pdf \| wc -l` |
| PDFs dropped (§4.1) | 2 | §4.1 table |
| Explicit footprints today | 14 | `grep -c '"style":"explicit"' schematical/schematical/Parts/footprints.json` |
| Footprints with an unnumbered pad today | 0 | `grep -c '"pin":0' schematical/schematical/Parts/footprints.json` |
| App reads of `.pads` | 5 (1 copy in `PartCatalog.swift:320`, 4 in `BoardModel.swift`) | `grep -rn '\.pads' --include='*.swift' schematical/schematical \| wc -l` |

### Verified properties

- **Today no explicit footprint lists a pin twice.** The loader refuses it
  (`footprints.c:665`) and `footprints.json` loads, so the new rule (§4.2)
  cannot change any existing footprint's pads or order.
- **The Lumex sheets lack what a part needs.** A datasheet read
  (2026-11-01, pages rendered at 200/400 dpi) found:
  - neither sheet names a controller, prints a font table or an AC timing
    table, or gives execution times;
  - `LCM-x01602Dxx+A-page1[1].pdf` is page 1 of 2, and its callouts
    ("14-Ø1.00", "P2.54 × 13") contradict its 16-hole drawing.
  The owner chose to leave both out (§ Decisions).

## 3. Design

Three independent pieces, all neutral for today's catalog:

1. **Datasheet housekeeping** (§4.1): delete two PDFs, add two
   `not-included.md` rows, commit the folder.
2. **Several pads per pin** (§4.2): `explicit` footprints may list a pin more
   than once. Pads come out grouped by pin: pin 1's pads in file order, then
   pin 2's, … then the unnumbered pads. Every existing footprint has one pad
   per pin, so its order is unchanged.
3. **`connectorFootprint`** (§4.3): an optional common part key, resolved
   and checked exactly like `footprint` (same helper), stored in a new
   `Part.connector_footprint`. The listing prints `|connector=<id>` after
   `|footprint=<id>`, only when set. `footprint_check` counts a footprint as
   used when it is a part's `footprint` or `connector_footprint`.

**Correctness gate:** byte-identical goldens. This sub-plan is the
provably-neutral class: nothing in the catalog uses either feature yet. A
golden diff is a bug in this sub-plan. Root-cause the one changed line and
fix it; a diff never means the design is wrong.

**Rejected alternatives**

- *A footprint-level `connector` reference* (the module footprint naming its
  connector footprint). Rejected: the connector footprint is a fact of the
  orderable part (which pins, which order), and the part file already names
  footprints. A part key keeps footprints.json free of cross-references.
- *Making `connectorFootprint` a `charlcd` kind key.* Rejected: footprint
  resolution lives in `library.c` for every kind, and the PCB Style option
  is generic (it shows for any part that has a connector footprint).
- *Allowing duplicate pins in `dual`/`quad`/`grid` too.* Not needed: only
  `explicit` describes the K1Z module (mounting holes, two rows).

## 4. Detailed Design

### 4.1 Datasheets

Delete these two PDFs and add one row each to `datasheets/not-included.md`
(the owner's choice, not obsolete parts):

| PDF | Family | Category / group | Notes |
|---|---|---|---|
| `module-disp-char/LCM-X01602DXX-D.pdf` (Lumex LCM-S01602DSR/D) | charlcd | Displays → Character | 1998 drawing sheet: no controller, font, AC timing or execution times; left out (owner, 2026-11-01) |
| `module-disp-char/LCM-x01602Dxx+A-page1[1].pdf` (Lumex LCM-S01602DTR/A) | charlcd | Displays → Character | Page 1 of 2 only (no pinout or electrical data); "14-Ø1.00" callout contradicts its 16-hole drawing; left out (owner, 2026-11-01) |

The five that stay are committed in the same commit.

### 4.2 Several pads per pin (`explicit`)

`load_explicit` (`footprints.c`):

- Drop the `"pin %u has two pads"` error. Count the pads per pin instead.
- Every pin 1…`pins` still needs at least one pad (`"pin %u has no pad"`).
- Output order: for pin = 1…`pins`, that pin's pads in file order; then the
  unnumbered pads in file order. With one pad per pin this is exactly
  today's order.
- `fp->pins` stays the pin count (not the pad count), so
  `load_part_footprint`'s `pins == pin_count` check is unchanged.

`parts.h` `PartFootprint` comment: `"1"…"N" order (grid: row-major); a pin
with several pads (explicit only) lists them together, in file order;
unnumbered last`.

`docs/parts-format.md` § `explicit`: replace "Every pin 1…`pins` appears
exactly once" with: "Every pin 1…`pins` has at least one pad. A pin may have
several (the same connection in more places, e.g. a module's two header
rows); they are listed together, in file order. The app draws each and uses
the pin's first pad for the pin-1 mark and the `Pad 1 Center` origin."

### 4.3 The `connectorFootprint` part key

- `PART_KEYS` (`library.c:188`) gains `"connectorFootprint"`.
- `load_part_footprint` is split into a helper taking the key name, so one
  function resolves `footprint` (required, as today) and
  `connectorFootprint` (optional). Each gets the same checks and messages
  with its key in them: unknown id, pad count against the pinout, a pin
  without a pad. For example, `unknown connectorFootprint "HDR-1X17"` and
  `pin "3" has no pad in connectorFootprint "X"`. The existing `footprint`
  messages are byte-identical (`library_check` asserts them today).
- A supply with `connectorFootprint` is rejected: `a supply has no footprint`
  (the existing message).
- `parts.h` `Part`: append
  `const PartFootprint *connector_footprint;  /* NULL = none; the PCB Style option's "Connector Only" */`
  after `footprint` and before `internal`. The app sees it in plan-11-D.
- `listing.c`: after `|footprint=<id>`, print `|connector=<id>` when
  `connector_footprint != NULL`.
- `footprint_check.c:144-150`: a footprint is used when it is some part's
  `footprint` **or** `connector_footprint`.
- `docs/parts-format.md`:
  - § Parts table: a `"connectorFootprint"` row: "optional: a second
    footprint, the `id` of one in `footprints.json`, that the PCB Style
    option switches to (**Connector Only**). Same rules as `footprint`.
    Character LCD modules use it (§ `charlcd`)."
  - § The `"footprint"` part key: one line saying `connectorFootprint`
    follows the same rules and counts as a use for `footprint_check`.
- `docs/backend.md` § Tests, `footprint_check` bullet: "every footprint used
  by some part (`footprint` or `connectorFootprint`)"; the listing note gains
  `|connector=<id>`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the
> same commit as the work it describes; `- [~]` for partial with one line of
> what remains; mark moot tasks `- [x] ~~text~~ — moot: <evidence>`; fill
> each `Commit:` line when the phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Datasheets

Deletes two PDFs, records them and commits the folder. Touches no code.

- [ ] `git rm` is not needed (untracked): `rm "datasheets/module-disp-char/LCM-X01602DXX-D.pdf" "datasheets/module-disp-char/LCM-x01602Dxx+A-page1[1].pdf"`.
- [ ] `datasheets/not-included.md`: the two rows of §4.1, appended to the table.
- [ ] Commit `datasheets/module-disp-char/` (5 PDFs) and `not-included.md` together.

Acceptance: five PDFs remain and the two rows are recorded.
  Check: `ls datasheets/module-disp-char/*.pdf | wc -l` → 5;
  `grep -c 'module-disp-char/LCM-' datasheets/not-included.md` → 2 (est. 1 s).
Commit: —

### Phase 2 — Several pads per pin

Loader rule and doc; no footprint uses it yet.

- [ ] `backend/src/library/footprints.c` `load_explicit`: §4.2. Per-pin pad
      lists replace the single `where[]` slot; output grouped by pin, file
      order within a pin, unnumbered last.
- [ ] `backend/include/parts.h`: the `PartFootprint` pad-order comment (§4.2).
- [ ] `docs/parts-format.md` § `explicit`: the text of §4.2.
- [ ] `backend/tests/library_check.c`: the case `"explicit: pin repeated"`
      (`library_check.c:1226-1228`, expects `pads[2]: pin 1 has two pads`)
      becomes an accept case: it loads, and its pads come out `1, 1, 2`
      (pin 1's two pads in file order, then pin 2).
- [ ] `backend/tests/library_check.c`: a footprint with pin 1 listed twice
      (pads at x = −1 and x = 1) loads with `pad_count` = pins + 1, and
      `pads[0]` and `pads[1]` are both designator `"1"`, in file order. A pin
      with no pad still fails with `pin 2 has no pad`.

Acceptance: duplicate pins load in the documented order, and every existing
footprint is unchanged.
  Check: `cmake --build /tmp/logic-p11 -j8 && ctest --test-dir /tmp/logic-p11 -R 'library_check|footprint_check'` → both pass;
  `git diff --exit-code backend/tests/golden/footprints.txt` → exit 0 (est. 1 min).
Commit: —

### Phase 3 — `connectorFootprint`

The part key, the `Part` field, the listing and the used-footprint rule.

- [ ] `backend/src/library/library.c`: `PART_KEYS` gains
      `connectorFootprint`; the shared resolve-and-check helper (§4.3);
      supplies reject it.
- [ ] `backend/include/parts.h`: append `connector_footprint` to `Part` (§4.3).
- [ ] `backend/tests/support/listing.c`: `|connector=<id>` (§4.3).
- [ ] `backend/tests/footprint_check.c`: used = `footprint` or `connector_footprint`.
- [ ] `backend/tests/library_check.c`, using inline fixture files: a gate
      part with `connectorFootprint` loads and `part->connector_footprint->id`
      matches; the three errors with their exact messages (unknown id, pad
      count, missing pad); a supply with it is rejected.
- [ ] `docs/parts-format.md` (§ Parts table row, § The `"footprint"` part
      key) and `docs/backend.md` (§ Tests) per §4.3.
- [ ] The app still builds (the field is appended; nothing reads it yet).

Acceptance: the key loads and is checked, nothing in the catalog changes,
and the app still builds.
  Check: `cmake --build /tmp/logic-p11 -j8 && ctest --test-dir /tmp/logic-p11` → `100% tests passed, 0 tests failed out of 17`;
  `git diff --exit-code backend/tests/golden` → exit 0;
  `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p11-dd CODE_SIGNING_ALLOWED=NO build | tail -1` → `** BUILD SUCCEEDED **` (est. 4 min; the app build is the only check that the new `Part` field compiles in Swift's import).
Commit: —

## Validation Plan

- Tests: `library_check` cases of Phases 2 and 3 (both the accept and the
  error paths).
- Coverage check: the new cases exercise the changed code. A golden match
  alone only shows that nothing in the catalog changed, so the new
  `library_check` cases are what prove the new paths.
- Doc sync: `docs/parts-format.md` (§ `explicit`, § Parts, § The
  `"footprint"` part key), `docs/backend.md` (§ Tests).
- Final gate: the Phase 3 check. It is the whole backend suite plus the app
  build, and both are already scoped to this sub-plan (est. 5 min).

## Decisions

- Five parts, the ones with full datasheets: NHD-0216BZ-FL-YBW,
  NHD-0216K1Z-FL-YBW, NHD-0216K1Z-NSW-BBW-L, 162H BC BW,
  AMC1602GR-B-Y6WFDY. Both Lumex PDFs go to `not-included.md` (owner,
  2026-11-01). (§4.1)
- The K1Z modules' two header rows both carry pins 1–16 in the Full Module
  footprint (owner, 2026-11-01), so a pin may have several pads. (§4.2)
- The connector footprint is a separate footprint named per part (author):
  owner's choice that Connector Only is a plain 1×16 header (plan-11-C §4.3).
  (§4.3)
- plan-11 comes after plan-09 and plan-10, both complete (owner,
  2026-11-01). (§ Prerequisites)

## Corrections

## Summary

Low risk: two neutral format extensions and housekeeping, gated on
byte-identical goldens. The `Part` struct grows one appended field, which
the app build checks.
