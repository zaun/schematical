# plan-03-B: Logic footprints (gate, dff, jkff)

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-03-A

Adds the 47 footprints the TI logic parts use to `Parts/footprints.json`
and names one on each of the 3,410 gate, D flip-flop and JK flip-flop parts.
Every footprint is taken from the TI package drawing in the repo's datasheets.

Outcome: every part in `gate.json`, `dff.json`, `jkff.json` carries a
`"footprint"`; `footprint_check` lists 58 footprints (11 LED + 47 logic);
the only `parts.txt` change is the `|footprint=` suffix on those 3,410 lines;
`traces.txt` is unchanged.

References:

- plan-03-A §4 (format, styles, formulas, goldens) — read first.
- `docs/parts-format.md` § Footprints (written in plan-03-A Phase 3).
- Datasheets: `datasheets/{ff-d,ff-jk,gate-and,gate-nand,gate-nor,gate-or,gate-xnor,gate-xor}/`.

## Prerequisites

See plan-03-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-03-A complete | `ls planning/completed/plan-03-A-footprint-format.md` → exists | MET (2026-11-01: exists, 5239f2f) |
| Loader in place | `ctest --test-dir /tmp/logic-plan03 -R footprint_check` → passes | MET (2026-11-01: `footprint_check … Passed`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- All 3,410 logic parts have a footprint; `footprints.txt` has 58 lines.
- `ctest` passes; `parts.txt` diff = the suffix only; `traces.txt` no diff.

### Non-goals

- No pinout, timing, family or flag change on any part.
- No footprint for a package not listed in §4.1.
- The loader still accepts parts without a footprint (plan-03-C flips it).

## 2. Current State

### Measured populations

The assignment table in §4.1 was produced on 2026-10-31 by a `python3 -I`
script over the three files: footprint = by `(package, pin count)`, except for
four pairs split by the TI designator in the part name (`DRY`/`DSF`,
`YZP`/`YFP`, `DPW`/`DTX`). Result: **47 footprints, 3,410 parts, 0 unmatched.**

Drawings in the repo, by TI drawing code (`pdftotext` of every logic
datasheet into `/tmp/p03txt/`, then `grep -l <code>` and `grep -l 'EXAMPLE
BOARD LAYOUT'`, files with code / files with code and a board layout):

| Code | Files | Code | Files | Code | Files |
|---|---|---|---|---|---|
| J0014 | 125/125 | DCK0005 | 58/58 | DCU0008 | 28/28 |
| W0014 | 165/165 | DCK0006 | 10/10 | DGS0020 | 4/4 |
| W0016 | 6/6 | DBV0005 | 52/52 | BQA0014 | 77/77 |
| W0020 | 15/15 | DBV0006 | 7/7 | DPW0005 | 10/10 |
| D0014 | 218/218 | DRL0005 | 22/22 | DTX0005 | 4/4 |
| DW0020 | 15/15 | DCT0008 | 15/15 | DQE0008 | 8/8 |
| NS0016 | 12/12 | DB0014/16/20 | 59/3/13 | DRY0006 | 18/18 |
| PW0014/16/20 | 165/6/14 | RSE0008 | 9/9 | DSF0006 | 15/15 |
| RKS0020 | 3/3 | YZP0005/6/8 | 15/7/13 | YFP0006/8 | 5/5 |

That is 34 drawings with text and a board layout. The other 13
(J0016, J0020, N0014, N0016, N0020, D0016, NS0014, NS0020, FK0020, DGV-14/16/20,
RGY-14) have no new-style text in any repo datasheet; they are old-style
drawings: `DGV (R-PDSO…` text pages in 24 files, `RGY (` in 20 files,
`LCCC - ` generic views in 93 files, and image-only pages (no text layer)
for N/NS/J in the addenda — for example `datasheets/gate-nand/sn74hc00.pdf`
pages 26, 27, 32 and 35 have under 400 characters of text each, between its
D/DB and J/PW drawings. Image pages are read by rendering them
(`pdftoppm -r 150 -f N -l N`, or the Read tool on the PDF page).

## 3. Design Overview

Pure data entry against plan-03-A's format, in two steps per footprint group:
write the footprint from the drawing, then tag the parts. The tagging is a
line-local edit (each part is one line, plan-03-A §2) done by a throwaway
script in `/tmp` that applies the §4.1 rule and inserts
`"footprint":"<id>"` right after `"pinout":"…"`.

Correctness risk: a wrong number in a footprint (shows as a misdrawn
footprint, caught by the hand check of each `footprints.txt` line against its
drawing) and a wrong assignment (caught by the 0-unmatched assignment script
and by the loader's pin-count check). Behavior cannot change: footprints do
not reach the simulator, and `traces.txt` must not diff.

### Pad rules (shared with plan-03-C)

- **R1 — board layout given** (TI "EXAMPLE BOARD LAYOUT"): use its pad size,
  pitch and pad-centre span. When it gives the outer extent X and pad length
  L instead of the span, span = X − L.
- **R2 — gull-wing or flat leads, outline only:** pad length = L(max) + 0.8;
  pad width = b(max) + 0.05; span = E(max, tip to tip) + 1.0 − pad length
  (pads reach 0.5 mm beyond the lead tips).
- **R3 — J-leads, outline only:** pad length 2.0; pad width = b(max) + 0.1;
  span = E(nom, tip to tip) − 1.0.
- **R4 — leadless terminals (SON, QFN, LCCC), outline only:** pad length =
  terminal length + 0.3; pad width = terminal width + 0.05; pads extend 0.3
  outward, so span = terminal-centre span + 0.3. Exposed pad → `centre`
  (rect) with the exposed pad's nominal size.
- **R5 — through-hole:** drill = largest lead cross-section dimension + 0.25,
  rounded up to 0.05; round pad, diameter = drill + 0.7 (at least 1.5);
  `pin1Shape` `rect`; span = the row spacing.
- **R6 — BGA:** round pad, diameter = 0.8 × nominal ball diameter, rounded to
  0.05.
- **Body:** nominal (mean of min and max) body size without leads, rounded to
  0.01 mm.
- Every footprint's `note` names the drawing code and the file and page used,
  e.g. `"TI D0014A board layout, gate-nand/sn74hc00.pdf p.24"`.

## 4. Detailed Design

### 4.1 Footprints and assignments

`dual` unless noted. Rule = the pad rule applied (§3).

| id | TI | Parts | Style / rule | Notes |
|---|---|---|---|---|
| `PDIP-14` / `-16` / `-20` | N | 223 / 35 / 25 | dual, R5 | span 7.62, pitch 2.54; package `DIP`; body from the N outline |
| `CDIP-14` | J | 369 | dual, R1 (J0014A layout) | |
| `CDIP-16` / `-20` | J | 71 / 41 | dual, R5 | span 7.62, pitch 2.54; body from the J outline |
| `CFP-14` / `-16` / `-20` | W | 171 / 18 / 15 | dual, R1 (W0014/16/20 layouts) | |
| `SOIC-14` | D | 616 | dual, R1 (D0014A) | |
| `SOIC-16` | D | 61 | dual, R1 with D0014A's pads, pitch and span (same lead geometry); body length from the 16-pin D outline | |
| `SOIC-20W` | DW | 44 | dual, R1 (DW0020A) | CD74 `M`/`M96` 20-pin parts are DW (`cd74hc273.pdf`: "DW (SOIC, 20)") |
| `SOP-14` / `-16` / `-20` | NS | 147 / 16 / 15 | dual, R1 with NS0016A's pads, pitch and span; body length = nominal "A" from the NS (R-PDSO-G**) family table | the family drawing is image-only, e.g. `sn74hc00.pdf` p.26 |
| `SSOP-8` | DCT | 27 | dual, R1 | |
| `SSOP-14` / `-16` / `-20` | DB | 76 / 5 / 21 | dual, R1 | |
| `TSSOP-14` / `-16` / `-20` | PW | 415 / 14 / 33 | dual, R1 | |
| `TVSOP-14` / `-16` / `-20` | DGV | 30 / 1 / 2 | dual, R2 from the `DGV (R-PDSO-G**)` outline | |
| `VSSOP-8` | DCU | 66 | dual, R1 | |
| `VSSOP-20` | DGS | 5 | dual, R1 | |
| `SOT23-5` | DBV | 133 | dual, R1, `perSide` 3, `slots` [1,2,3,4,6] | |
| `SOT23-6` | DBV | 17 | dual, R1 | |
| `SC70-5` | DCK | 164 | dual, R1, `perSide` 3, `slots` [1,2,3,4,6] | |
| `SC70-6` | DCK | 27 | dual, R1 | |
| `SOT5X3-5` | DRL | 28 | dual, R1, `perSide` 3, `slots` [1,2,3,4,6] | |
| `SON-6-DRY` | DRY | 36 | dual, R1 | `SON` 6-pin, name contains `DRY` |
| `SON-6-DSF` | DSF | 33 | dual, R1 | name contains `DSF` |
| `X2SON-5-DPW` | DPW | 10 | dual, R1, `perSide` 2, `slots` [1,2,0,3,4], round `centre` = pin 3 | layout: 1 and 5 on top, 2 and 4 below, 3 in the middle (DPW0005A, `sn74aup1g02.pdf`) |
| `X2SON-5-DTX` | DTX | 4 | dual, R1, `perSide` 2, `slots` [1,2,0,3,4], `centre` = pin 3 | same arrangement (DTX0005A, `sn74ahc1g32-q1.pdf`) |
| `X2SON-8` | DQE | 9 | dual, R1 | |
| `UQFN-8` | RSE | 12 | quad, R1 | |
| `VQFN-14` | RGY | 46 | quad, R4 from the `RGY (` outline; exposed pad as `centre` | |
| `VQFN-20` | RKS | 3 | quad, R1, `centre` | |
| `WQFN-14` | BQA | 79 | quad, R1, `centre` | |
| `LCCC-20` | FK | 204 | quad, R4; `sides` [5,5,5,5], `slots` putting pin 1 at the top-centre (pin n → slot ((n + 16) mod 20) + 1) | |
| `DSBGA-5-YZP` | YZP | 14 | grid 3 rows × 2 columns, R1 | `balls` A1 B1 C1 C2 A2 (plan-02-C §4 layout L5; B2 empty) |
| `DSBGA-6-YZP` / `-YFP` | YZP / YFP | 7 / 4 | grid 3 × 2, R1 | `balls` A1 B1 C1 C2 B2 A2 (layouts L6 / L6Y); name contains `YZP` / `YFP` |
| `DSBGA-8-YZP` / `-YFP` | YZP / YFP | 13 / 5 | grid 4 × 2, R1 | `balls` A1 B1 C1 D1 D2 C2 B2 A2 (layout L8); name contains `YZP` / `YFP` |

Totals: 47 footprints, 3,410 parts (sum of the Parts column).

R1 rows use the 34 drawings with a text board layout (§2). The 13 old-style
or image-only drawings are outline-only (rendered on 2026-10-31:
`sn74hc00.pdf` p.26 is the NS family outline, p.27 the old W outline — no land
pattern on either), hence R2/R4/R5 or a sibling's R1 pads for those rows.

### 4.2 Assignment script (throwaway, `/tmp/p03b-assign.py`)

Reads each of `gate.json`, `dff.json`, `jkff.json` as text and as JSON;
computes each part's footprint id by §4.1 (package + pin count; the four
split pairs by designator substring in the name); exits non-zero if any part
is unmatched; rewrites each part line by inserting
`,"footprint":"<id>"` after the `"pinout":"…"` field. It prints
`<file>: N parts tagged`; expected gate 2552, dff 672, jkff 186.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Through-hole and SOIC/SOP families (27 footprints)

PDIP, CDIP, CFP, SOIC, SOP, SSOP, TSSOP, TVSOP, VSSOP: the dual-row families.

- [x] `footprints.json`: `PDIP-14/16/20`, `CDIP-14/16/20`, `CFP-14/16/20`,
      `SOIC-14/16`, `SOIC-20W`, `SOP-14/16/20`, `SSOP-8/14/16/20`,
      `TSSOP-14/16/20`, `TVSOP-14/16/20`, `VSSOP-8`, `VSSOP-20` (27), each with
      its drawing `note`. — added (38 footprints total); CFP uses R2 and
      SOIC-16's body the D (R-PDSO-G16) outline (Corrections).
- [x] `/tmp/p03b-assign.py` (§4.2) restricted to these ids; tag the parts. —
      as `/tmp/p03/tag.py /tmp/p03/rules_b.py` with `P03_IDS` = the 27 ids:
      `gate.json: 1861 parts tagged, 691 skipped`, `dff.json: 529 …, 143
      skipped`, `jkff.json: 172 …, 14 skipped`; a full dry run over a copy
      matched all 3,410 parts to the 47 ids with 0 unmatched.
- [x] Regenerate `parts.txt` and `footprints.txt`; hand-check each new
      `footprints.txt` line's pitch, span and pad size against its drawing. —
      `/tmp/p03/goldcheck.py` on the 27 lines: pin 1 at x = −span/2, row
      step = pitch, pad = the drawing's pad, pin-1 shape — all as recorded;
      re-read D0014A, DW0020A (span 9.3), DCU0008A and J0014A (pad 1.6, hole
      1.0) in the text myself.

Acceptance: those parts carry these footprints and the geometry matches the
drawings.
  Check: `ctest --test-dir /tmp/logic-plan03 --output-on-failure` → all pass (est. 1 min). — 2026-11-01: `100% tests passed, 0 tests failed out of 8`.
  Check: `git diff --exit-code backend/tests/golden/traces.txt` → no diff (est. seconds). — 2026-11-01: exit 0, no output.
Commit: 30c9924

### Phase 2 — Small-outline, leadless and BGA (20 footprints)

- [x] `footprints.json`: `SOT23-5/6`, `SC70-5/6`, `SOT5X3-5`, `SON-6-DRY`,
      `SON-6-DSF`, `X2SON-5-DPW`, `X2SON-5-DTX`, `X2SON-8`, `UQFN-8`,
      `VQFN-14`, `VQFN-20`, `WQFN-14`, `LCCC-20`, `DSBGA-5-YZP`,
      `DSBGA-6-YZP`, `DSBGA-6-YFP`, `DSBGA-8-YZP`, `DSBGA-8-YFP` (20). —
      added; SON-6-DRY, X2SON-8 and UQFN-8 are `explicit`, VQFN-14 comes
      from RHL0014A (Corrections).
- [x] Run `/tmp/p03b-assign.py` over all ids; it reports 0 unmatched and
      2552 / 672 / 186 tagged. — `gate.json: 2552 parts tagged`,
      `dff.json: 672 parts tagged`, `jkff.json: 186 parts tagged`, exit 0.
- [x] Regenerate the goldens; hand-check the new lines (slot maps: pin 1 where
      the drawing puts it; DSBGA ball sets equal the parts' `dsbga` pinout
      designators from plan-02-C — the loader enforces this). — checked:
      SOT23-5/SC70-5/SOT5X3-5 pin 5 top-right with right-middle empty;
      X2SON-5 pin 3 at the centre; VQFN-14/20 and WQFN-14 pin 1 on the top
      edge left of pin N; LCCC-20 `3 2 1 20 19` across the top; DSBGA balls
      row-major on the drawing pitch, loader-matched to every part.

Acceptance: every logic part has a footprint; `footprints.txt` has 58 lines.
  Check: `ctest --test-dir /tmp/logic-plan03 --output-on-failure` → all pass (est. 1 min). — 2026-11-01: `100% tests passed, 0 tests failed out of 8`.
  Check: `grep -c '"footprint"' schematical/schematical/Parts/{gate,dff,jkff}.json` → 2552, 672, 186; `wc -l < backend/tests/golden/footprints.txt` → 58 (est. seconds). — 2026-11-01: 2552, 672, 186; 58.
  Check: a `python3 -I` one-off that strips `|footprint=…` from every line of `backend/tests/golden/parts.txt` and compares with the same strip of `git show <plan-03-A's last commit>:backend/tests/golden/parts.txt` → identical (est. seconds). — 2026-11-01, against 5239f2f: `identical 3818 3818`; `git diff --exit-code … traces.txt` exit 0.
Commit: 514a36e

## Validation Plan

- Tests: `footprint_check` (golden of all 58), `listing_check`,
  `trace_check`, `library_check`.
- Coverage check: the assignment script's 0-unmatched report plus
  `footprint_check`'s unused-footprint failure mean every id is used and
  every part is tagged.
- Runtime proof: none needed beyond the build — the app shows footprints only
  from plan-03-D. `xcodebuild` (plan-03-A Phase 2 command) → `** BUILD SUCCEEDED **`.
- Doc sync: none (format unchanged).
- Final gate (once): `cmake -S backend -B /tmp/logic-final && cmake --build /tmp/logic-final -j8 && ctest --test-dir /tmp/logic-final` (est. 1 min) + `xcodebuild` (est. 3 min).

## Final gate (run 2026-11-01)

- `cmake --build /tmp/logic-final-p03 -j8 && ctest --test-dir /tmp/logic-final-p03`
  → `100% tests passed, 0 tests failed out of 8` (the gate's `/tmp/logic-final`
  holds a stale plan-01 cache; see plan-03-A Corrections).
- `xcodebuild … build` → `** BUILD SUCCEEDED **`.

## Decisions

See plan-03-A §Decisions. Pad rules R1–R6 are this plan's (author) choice:
the view has no DRC and no fabrication output, so nominal land patterns are
enough, and the rules make every number reproducible from a drawing.

## Corrections

- 2026-11-01: §2's drawing census is wrong for W: `grep -l W0014` matches
  `PW0014A` (TSSOP). Every one of the 165 "W0014" hits is `PW0014A` (the
  only W0014 strings in /tmp/p03txt are "PW0014A SCALE 2.500" and "PW0014A
  TSSOP - 1.2 mm max height"); the same holds for W0016/W0020. TI's CFP (W)
  drawings are the old image-only `W (R-GDFP-F14/F16/F20)` outlines (e.g.
  `sn74hc00.pdf` p.27) with no land pattern. So CFP-14/16/20 use **R2**,
  not R1. With the CFP's long straight leads (6.35–9.14 mm) R2 gives pads
  ≈10 mm long whose inner ends reach the body edge (overlapping it by
  0.05–0.15 mm on CFP-14/16). Kept as R2 gives it: no DRC, and the rule is
  the plan's reproducible choice.
- 2026-11-01: no `D0016` drawing exists in any logic datasheet; SOIC-16's
  body length comes from the image-only `D (R-PDSO-G16)` outline
  4040047-6/M (`ff-d/scls119e.pdf` p.20), 9.9 mm; pads stay D0014A's, as
  §4.1 says.
- 2026-11-01: CFP-20 body length: the W outline gives only 13.72 max, so the
  nominal 13.09 comes from the datasheet's device-information table
  (`ff-d/scls136f.pdf` p.1, "CFP (20) 13.09 mm × 6.92 mm").
- 2026-11-01: three land patterns have mixed pad sizes, so they use the
  `explicit` style instead of §4.1's `dual`/`quad`: SON-6-DRY (DRY0006A,
  pin 1 pad 0.35 vs 0.3, `sn74aup1g02.pdf` p.27), X2SON-8 (DQE0008A, pin 1
  0.6 vs 0.5, `sn74aup1g74.pdf` p.27) and UQFN-8 (RSE0008A: corner side
  pads 0.55×0.25, middle 0.55×0.2, top/bottom 0.3×0.6, `sn74aup1g74.pdf`
  p.33). Explicit exists for exactly this (plan-03-A §4.2).
- 2026-11-01: VQFN-20 (RKS) and WQFN-14 (BQA) need `slots` that §4.1 left
  out: pin 1 is on the top edge, left of the last pin. VQFN-20: `sides`
  [8,2,8,2], `slots` [20,1…19]; WQFN-14: `sides` [5,2,5,2], `slots`
  [14,1…13].
- 2026-11-01: X2SON-5 centre pads are squares turned 45° on DPW0005A
  (□0.25) and DTX0005A (□0.35); pads are axis-aligned, so both are
  `round` of that size (§4.1 already said round for DPW).
- 2026-11-01: no RGY outline exists in the repo: all 20 `RGY (` hits §2
  counted are device-table rows ("RGY (VQFN, 14) 3.50mm × 3.50mm"), and
  every RGY datasheet (and TI's current ones, e.g. sn74lvc00a Rev. U) has
  only the generic RGY view. VQFN-14 instead uses TI's RHL0014A
  board layout (R1, 4228405/A, TPS54620 datasheet p.45 — "The RGY package
  is identical to the RHL package"): pads 0.6 × 0.24, spanX = spanY = 3.3
  (pad-centre to pad-centre, read from the rendered drawing), `sides`
  [5,4,5,4] with pins 1/14 and 7/8 on the outer top/bottom slots (±0.75),
  exposed pad 2.05. R1 replaces §4.1's R4 because a TI layout exists.
- 2026-11-01: SC70-6's layout pad is 0.9 × 0.4 (DCK0006A), not SC70-5's
  0.95 × 0.4.

## Summary

Data entry with two mechanical guards (assignment script, pin-count check)
and a hand check of each golden line against its drawing. No behavior or
format change.
