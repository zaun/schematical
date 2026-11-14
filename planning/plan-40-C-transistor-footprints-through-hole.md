# plan-40-C: Footprints — through-hole transistor packages

Last updated: 2026-11-14
Effort: medium (1h–2h)
Depends on: plan-40-B

C draws the land patterns of the through-hole packages of the final pick
(TO-92 and its variants, TO-126, TO-220 and TO-247 families, IPAK, I2PAK,
TO-3P, TO-274, E-Line, SSIP, DIP) from the package drawings of the
datasheets plan-40-A downloaded, reusing a footprint of the library when its
pads equal the drawing's. It writes `planning/plan-40-data/footprints-C.json`
(ready objects, not yet in `Parts/footprints.json`: G merges them with the
parts, because `footprint_check` fails on a footprint no part uses) and the
through-hole rows of `planning/plan-40-data/footprint-map.tsv`. It also
writes the tools C, D and E share. The behavioral outcome: every
through-hole line of `footprints-needed.txt` has a footprint id in
`footprint-map.tsv`, and the new objects load through `footprint_check`.

References:

- `docs/parts-format.md` § Footprints (`dual`, `header`, `explicit`; § The
  `"footprint"` part key: the rules R1–R9, mid-values, no overlapping pads).
- `planning/completed/plan-03-B-logic-footprints.md` § 3 (pad rules R1–R6).
- `planning/completed/plan-34-C-resistor-footprints-chip.md` § 4.3 (ids,
  reuse, shared ids, ready objects) and `planning/plan-34-data/census/`
  (`fp-load.sh`, `fp-merge.py`, `fp-check.py`: the tools C copies).
- `schematical/schematical/Parts/footprints.json` precedents (all `header`
  style, through-hole): `TO-220-3-TSC`, `ITO-220-3-TSC`, `TO-247-3-TSC`;
  explicit: `TO-220-3-ONSEMI`, `TO-3P-3`.

## Prerequisites

plan-40-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-40-B complete | `ls planning/completed/plan-40-B-*.md` → exists | NOT MET (B not started) |
| `footprints-needed.txt` is final | `awk -F'\t' '$24=="add"' planning/plan-40-data/candidates.tsv \| wc -l` → N; `grep -c '^C' planning/plan-40-data/footprints-needed.txt` → the C lines (28 at planning time) | NOT MET (B not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-40-data/census/fp-check.py C` prints `<n> lines,
  0 without a footprint`, `<n>` = `awk -F'\t' '$1=="C"' planning/plan-40-data/footprints-needed.txt | wc -l`
  (28 at planning time).
- `planning/plan-40-data/census/fp-load.sh planning/plan-40-data/footprints-C.json`
  prints `<k> new footprints: load ok`, `<k>` the objects of the file
  (10 at most at planning time: the lines with no same-outline footprint, § 2).
- `planning/plan-40-data/footprint-map.tsv` has, for every C line, the
  footprint id and the pinouts it serves.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json` or any `Parts/*.json`, `backend/` or
  the app (G merges the objects).
- No existing footprint changes: a footprint of the library is reused or a
  new one is added, never edited.
- No footprint for a package no `add` row uses.

## 2. Current State

`Parts/footprints.json` holds 1497 footprints (`git show HEAD:backend/tests/golden/footprints.txt | wc -l`
→ 1497). Through-hole transistor-shaped ones came with the diode plans:
three-lead `header` footprints (`TO-220-3-TSC`, `ITO-220-3-TSC`, `TO-247-3-TSC`)
and `explicit` ones (`TO-220-3-ONSEMI`, `TO-220-FULL-PACK-3`, `TO-247-3-ONSEMI`,
`TO-3P-3`); none for TO-92, TO-126, E-Line, SSIP or DIP-14 transistors.

### Measured populations

| What | Count | Command |
|---|---|---|
| C lines (maker, DigiKey Package / Case), picked rows | 28 (157 parts) | `awk -F'\t' '!/^#/ && $1=="C"' planning/plan-40-data/footprints-needed.txt \| wc -l` |
| …with no existing footprint of the same outline | 10: TO-92 ×4 (all four package variants), TO-126, E-Line ×2, SC-71, 3-SSIP, 14-DIP | the same lines with an empty 10th column |
| …by package | TO-220-3 3, TO-220-3 Full Pack 2 (+1 formed), TO-247-3 3, TO-251/IPAK 4, TO-262/I2PAK 2, TO-3P 2, TO-274AA 1, TO-92 4, TO-126 1, E-Line 2, SC-71 1, 3-SSIP 1, 14-DIP 1 | `awk -F'\t' '!/^#/ && $1=="C" {print $4}' planning/plan-40-data/footprints-needed.txt \| sort \| uniq -c` |

### Verified properties

- **A pin number is a position.** A part's pins are matched to pads by
  designator (docs/parts-format.md § The `"footprint"` part key), so a
  transistor reuses an existing footprint only when its pad `n` sits where
  the datasheet's pin `n` does. The existing diode footprints number a
  tab package's pads leads first and the tab last (`DPAK-3` pads 1, 2, 3 =
  lead, lead, tab), while a transistor datasheet numbers a DPAK 1 gate, 2
  drain ("Pin 2, Tab"), 3 source (`pdftotext -layout` of
  `https://www.infineon.com/assets/row/public/documents/24/49/infineon-ipd95r2k0p7-datasheet-en.pdf`,
  lines 20, 25, 32); the reuse rule below compares the pad positions, so
  that case is never reused by mistake.
- **A footprint's `pins` equals the pinout's pin count** (the loader's rule),
  and a tab that is its own pin is a pin of the pinout (`G,D,S,D`, B § 3.1).

## 3. Design

### 3.1 Ids, reuse, ready objects

- **Ids.** `<outline>-<MAKER>` with the outline's name as the package spells
  it and the maker in upper case: `TO-92-3-ONSEMI`, `TO-92-3-NEXPERIA`,
  `TO-126-3-ONSEMI`, `TO-220-3-INFINEON`, `TO-247-3-VISHAY`,
  `TO-251-3-INFINEON`; a second drawing of the same maker and outline appends
  the series or variant (`TO-92-3-ONSEMI-FORMED`, `TO-92-3-ONSEMI-LONG`).
  Characters `A-Z 0-9 . -` only.
- **Reuse.** A drawing whose land pattern equals an existing footprint's
  (same pin count, every pad's designator, centre and size within 0.05 mm, same
  drill, body within 0.1 mm) reuses it. C checks `Parts/footprints.json`
  first.
- **Shared ids.** Before adding an id: `grep -c '"id":"<id>"'
  schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json`;
  a hit means reuse it (checked as any reuse) and the add step is marked
  moot with that grep's output.
- **Ready objects.** `footprints-C.json` is `{"footprints":[…]}` in
  `footprints.json`'s format; each object has a `note` naming the drawing,
  the PDF path under `datasheets/transistor/` and the page, and the
  mid-values used.

### 3.2 Which style

Through-hole in-line packages (TO-92, TO-126, TO-220, TO-247, TO-3P, E-Line,
SSIP) use style `header` (one row of `pins` pads at the lead pitch, round pads,
`drill` from the lead width per rule R5, pin 1 `rect`), as the TSC TO-220
precedent; a package whose leads are staggered or formed to a different pitch
(a formed-lead TO-92, TO-251 with stub leads) uses `explicit` with the
drawing's lead coordinates; the DIP-14 uses `dual` as the library's other
14-pin DIPs do. Body = the drawing's package outline seen from above
(mid-values), the tab side up for tab packages. A tab that the datasheet
gives its own pin number is a pad of that number (the center lead of a
TO-220 is the tab: pad 2 only; the pinout `G,D,S`), and a tab with its own
lead beside it (the pinout `G,D,S,D`) is an `explicit` pad.

### 3.3 Where the risk is

A pad at the wrong place or a drill smaller than the lead. Mitigation: the
`note` records the mid-values and arithmetic, `fp-load.sh` runs
`footprint_check` (pins, designators, pad rules), and Phase 2 hand-checks
five objects against their drawings.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools, TO-92, TO-126 and the small leaded packages

- [ ] Copy `planning/plan-34-data/census/fp-load.sh` and `fp-merge.py` to
      `planning/plan-40-data/census/` (paths `/tmp/p40-fp`, the plan-40
      data directory, the golden `footprints.txt`).
- [ ] Write `census/fp-check.py <letter>`: joins the letter's lines of
      `footprints-needed.txt` with the ids of `footprint-map.tsv` (one row
      per land-pattern line: maker, DigiKey Package / Case, footprint id,
      the `label`), `footprints-C.json`, `-D.json`, `-E.json` and
      `Parts/footprints.json`; prints `<n> lines, <m> without a footprint`
      and each missing line.
- [ ] TO-92 (4 lines: standard, formed leads, long body, long body formed),
      TO-126, E-Line (2), SC-71, 3-SSIP, 14-DIP: one object per distinct land
      pattern (§ 3.1, § 3.2) from the datasheet drawing of the line's example
      part (`footprints-needed.txt` 8th column; the PDF is in
      `datasheets/transistor/`), the `footprint-map.tsv` rows; reuse noted
      where a drawing equals an existing footprint.

Acceptance: the Phase 1 objects load.
  Check: `planning/plan-40-data/census/fp-load.sh planning/plan-40-data/footprints-C.json` → only `extra footprint` / `unused footprint` failures, for exactly the new ids; no load error (est. 4 min: copies `backend/`, builds `footprint_check`).
Commit: —

### Phase 2 — TO-220, TO-247, IPAK, I2PAK, TO-3P, TO-274

- [ ] One object per distinct land pattern for the TO-220 (3, + full pack, +
      formed), TO-247, TO-251 (short and stub leads), TO-262, TO-3P (formed,
      full pack) and TO-274AA lines; the maker drawings that equal
      `TO-220-3-ONSEMI`, `TO-220-FULL-PACK-3`, `TO-247-3-ONSEMI`, `TO-3P-3` or
      the TSC/Vishay ones reuse them (every pad within 0.05 mm).
- [ ] Hand-check five objects (one per package family) against their drawing:
      pad centres, drill, body; recorded in § Corrections.

Acceptance: every C line has a footprint and the objects load.
  Check: `python3 -I planning/plan-40-data/census/fp-check.py C` → `<n> lines, 0 without a footprint` (n = the C lines); `fp-load.sh planning/plan-40-data/footprints-C.json` as Phase 1 (est. 5 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`.
- Coverage check: `fp-check.py C` joins every C line.
- Runtime proof: n/a here (G's goldens and H's harness render the parts).
- Doc sync: none (the footprint rules are unchanged).
- Final gate: plan-40-I.

## Decisions

- **Objects stay in `planning/plan-40-data/footprints-C.json` until G** —
  author: `footprint_check` fails on a footprint no part uses.
- **Reuse is by pad geometry, not by name** — author: pin numbering of tab
  packages differs between diode and transistor datasheets (§ 2).
- **One footprint per maker drawing** — author: the library's existing practice
  (`TO-220-3-ONSEMI` vs `TO-220-3-TSC`).
- Owner decisions: plan-40-A § Decisions.

## Corrections

(none yet)

## Summary

C is drawing work with small blast radius: nothing in the tree changes except
`planning/plan-40-data/`. The risk is a pad placed by the wrong rule; the
loader checks the structure and the hand check covers the geometry.
