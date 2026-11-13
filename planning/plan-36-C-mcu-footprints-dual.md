# plan-36-C: Microcontrollers — dual-row and small footprints (Microchip)

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-36-B

C draws or reuses the land pattern for every dual-row and small package
code the AVR `add` rows use: PDIP, SOIC (150 mil and 208 mil), SSOP,
TSSOP, DFN/UDFN/MLF10 and SOT-23-6. New footprints are kept as ready
objects in `planning/plan-36-data/footprints-C.json`. plan-36-G merges
them into `footprints.json` together with the parts that use them,
because `footprint_check` fails on a footprint no part uses (the plan-35-B
precedent). C also fills the `footprint` column of `package-codes.tsv`
for these codes.

Outcome of C: every dual-row and small code in `package-codes.tsv` names
a footprint, either an existing id or one of `footprints-C.json`, and
`census/fp-load.sh planning/plan-36-data/footprints-C.json` reports only
`extra footprint` / `unused footprint` for exactly the new ids.

References:

- plan-36-A § Prerequisites (the gate for all of plan-36).
- `docs/parts-format.md` § Footprints (`dual` style, `centre`, `slots`,
  mid-values, the `note`), § The `"footprint"` part key.
- `planning/completed/plan-03-B-logic-footprints.md` § 3 (pad rules R1–R6).
- `planning/plan-35-data/census/fp-load.sh`, `fp-merge.py` (copied here,
  paths changed).
- `planning/plan-36-data/package-codes.tsv` (B), `footprints-needed.txt`
  (planning proxy).

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-36-B complete | `ls planning/completed/plan-36-B-*.md` → exists | NOT MET (2026-11-12: plan written) |
| The loader probe works | `echo '{"footprints":[]}' > /tmp/p36-fp0.json && planning/plan-36-data/census/fp-load.sh /tmp/p36-fp0.json` → `0 new footprints: load ok` (after Phase 1's copy) | NOT MET until Phase 1 |

## 1. Goal

- Each `package-codes.tsv` row of geometry `dual` has a `footprint`.
  Planning saw 22 dual-row and small (DigiKey package, supplier package)
  lines over 487 AVR parts (`footprints-needed.txt`, rows whose package
  matches `DIP|SOIC|SSOP|TSSOP|DFN|SOT`, plus the `-` line). The code list
  from A replaces that proxy.
- `fp-load.sh footprints-C.json` → only `extra` / `unused` lines for the
  new ids, and no load error.

### Non-goals (explicit constraints)

- `Parts/footprints.json`, the part files and the goldens are not touched
  here (G merges).
- No quad, grid or module footprint (D, E).
- No change to the footprint format.

## 2. Current State

- Existing dual-row footprints a Microchip code may reuse (`grep -o '"id": "[^"]*"' schematical/schematical/Parts/footprints.json`):
  `PDIP-8`, `PDIP-14`, `PDIP-20`, `PDIP-28-600`, `PDIP-40-600`, `SOIC-8`,
  `SOIC-14`, `SOIC-20W`, `SOIC-24W`, `SOIC-28W`, `SSOP-20`, `SSOP-28`,
  `TSSOP-8`, `TSSOP-14`, `TSSOP-20`, `SOT23-6`. All are TI drawings
  (plan-03-B § 3). There is no 300-mil PDIP-28 (`grep -c '"id": "PDIP-28"' …` → 0).
- Microchip datasheets end with a "Packaging Information" chapter. Each
  drawing gives dimensions and, in the Microchip format (C04-xxx
  drawings), a "Recommended Land Pattern" page with C (row distance), E
  (pitch), X1 (pad width) and Y1 (pad length). Atmel-era datasheets (`28P3`,
  `8S2`, `20S2`) give the outline only.

## 3. Design

- **Reuse rule** (decided at planning; applied per code): a code reuses an
  existing footprint when the pin count, pitch and row distance (span)
  are the same, and each pad side is within 0.10 mm of what R1/R2
  (below) gives for the Microchip drawing. Otherwise the code gets a new
  footprint. Two Microchip codes with identical results share one new
  footprint, which is the first code's.
- **Pad rules**: R1 when the drawing has a Recommended Land Pattern (use
  C, E, X1, Y1 as printed: span = C, pad = Y1 × X1). R2 (gull-wing outline
  only), R4 (DFN/UDFN/MLF10, leadless), R5 (PDIP through-hole), with
  mid-values as in `docs/parts-format.md` § The `"footprint"` part key. A
  DFN's exposed pad is a `centre` pad with no pin (an unnumbered pad, B
  § Decisions).
- **Ids**: `<family>-<pins>[W]-MC-<code>`, where `<family>` is `PDIP`,
  `SOIC`, `SSOP`, `TSSOP`, `UDFN`, `VDFN` or `SOT23` and `<code>` is the
  drawing code with characters outside `A-Z0-9` → `-`. Examples:
  `PDIP-28-MC-28P3`, `SOIC-8-MC-8S2`, `UDFN-8-MC-8MA2`, `SOIC-8W-MC-S2A`
  (208 mil). The `note` names the drawing, the PDF and the page, plus the
  mid-values (`lead 0.46 mid (0.38-0.53)`).
- **`package` value** (G writes it on each part): `DIP`, `SOIC`, `SSOP`,
  `TSSOP`, `DFN`, `SOT` (the `docs/parts-format.md` § Parts `package`
  rules: no pin count, no body size).

Where the risk is: pad sizes copied wrong. Every new object is loaded by
the real loader (`fp-load.sh`), and Phase 2 compares each one with its
drawing.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Probe scripts and the code list

- [ ] Copy `planning/plan-35-data/census/fp-load.sh` and `fp-merge.py` to
      `planning/plan-36-data/census/`. Change `p35` → `p36` in the temp
      names, then run the empty-file probe (§ Prerequisites).
- [ ] `census/fp-codes.py`: list the `dual` rows of `package-codes.tsv`
      with the PDF and page of each code's drawing (the "Packaging
      Information" page whose text has the code). Write them to
      `fp-pages.tsv`.

Acceptance: every dual code has a drawing page.
  Check: `awk -F'\t' '$2=="dual" && $5==""' planning/plan-36-data/fp-pages.tsv | wc -l` → 0 (`$5` is the page) (est. 2 min).
Commit: —

### Phase 2 — Footprints

- [ ] For each code: apply the reuse rule, or write a new object into
      `footprints-C.json` (R1/R2/R4/R5). Then fill the `footprint` column of
      `package-codes.tsv`.
- [ ] Load: `planning/plan-36-data/census/fp-load.sh planning/plan-36-data/footprints-C.json`.

Acceptance: the new footprints load, and every dual code has one.
  Check: `fp-load.sh footprints-C.json` → only `extra footprint` / `unused footprint` lines naming exactly the ids in `footprints-C.json`; `awk -F'\t' '$2=="dual" && $6==""' planning/plan-36-data/package-codes.tsv | wc -l` → 0 (`$6` is `footprint`) (est. 3 min).
Commit: —

## Validation Plan

- Tests: the real loader through `fp-load.sh`; `footprint_check` in G
  (every footprint used, golden lines).
- Coverage check: Phase 2's empty-`footprint` count.
- Runtime proof: G's loaded parts, I's board check.
- Doc sync: none (no format change).
- Final gate: plan-36-I.

## Decisions

- **Reuse within 0.10 mm** — author: land patterns of JEDEC packages with
  the same pitch and span differ by a few hundredths between makers'
  recommendations. Prompt rule 4 says to reuse what exists, and a 0.10 mm
  bound keeps a reused pad inside both makers' tolerance.
- **The `-MC-` id infix** — author: Microchip's drawing codes (`28P3`,
  `S2A`) would collide with other makers' codes in a bare `PDIP-28-<code>`
  name.

## Corrections

## Summary

These are routine footprints from drawings, for packages the library mostly
knows already. The risk is transcription, and both the loader and the
drawing comparison check it.
