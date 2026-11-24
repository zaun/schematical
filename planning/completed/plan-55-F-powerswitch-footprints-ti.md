# plan-55-F: Footprints — Texas Instruments power-switch packages

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-55-E

F draws the land patterns of the Texas Instruments packages of the final pick (the `F` lines of
`footprints-needed.txt`: at most 68 DigiKey `Package / Case` labels across the three families,
plan-55-A § 2) from the package drawings of the datasheets B placed, reusing a library footprint when
its pads equal the drawing's. It writes `planning/plan-55-data/footprints-F.json` (ready objects,
not yet in `Parts/footprints.json`: H and I merge the ones their parts use, because
`footprint_check` fails on a footprint no part uses) and F's rows of `footprint-map.tsv`, and the
tools F and G share. The behavioral outcome: every `F` line of `footprints-needed.txt` has a
footprint id in `footprint-map.tsv`, and the new objects load through `footprint_check`.

References:

- `docs/parts-format.md` § Footprints (`dual`, `quad`, `header`, `explicit`, `"centre"` for an
  exposed pad; § The `"footprint"` part key: the rules, mid-values, no overlapping pads).
- `planning/completed/plan-42-D-regulator-footprints-leaded.md` and
  `plan-42-E-regulator-footprints-leadless-ti-microchip.md` (the method F follows: ids, reuse,
  shared ids, ready objects, pin numbers are positions) and `planning/plan-42-data/census/`
  (`fp-load.sh`, `fp-merge.py`, `fp-check.py`: the tools F copies).
- `planning/plan-55-data/footprints-needed.txt` (E).

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-E complete | `ls planning/completed/plan-55-E-*.md` → exists | MET (2026-11-23: archived at 0ba82f54) |
| `footprints-needed.txt` is final | `grep -c '^F' planning/plan-55-data/footprints-needed.txt` → E's recorded `F` count | MET (2026-11-23: 51) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-55-data/census/fp-check.py F` prints `<n> lines, 0 without a footprint`,
  `<n>` = `awk -F'\t' '$1=="F"' planning/plan-55-data/footprints-needed.txt | wc -l`.
- `planning/plan-55-data/census/fp-load.sh planning/plan-55-data/footprints-F.json` prints
  `<k> new footprints: load ok`.
- `footprint-map.tsv` has, for every `F` line, the footprint id and the pinouts it serves.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, any `Parts/*.json`, `backend/` or the app; the merge is H's and I's.
- No footprint for a package no `add` row uses.

## 2. Current State

`Parts/footprints.json` has 1895 footprints at planning (`wc -l < backend/tests/golden/footprints.txt`
→ 1895 at 7c7b4d31), among them the leaded and leadless TI packages plan-42 drew for the regulators
(`SOT23-5`, `SOT23-6`, `SC70-5`, `SOT563-6`, `SOT89-3`, `SOT223-4-*`, `DPAK-3`, `SOIC-8`,
`MSOP8-DIODES`, `HTSSOP-16-PWP` and others). A footprint is reused when its pads equal the drawing's
(count, pitch, pad size, span), within the datasheet's tolerance; otherwise F draws one.

### Measured populations

| What | Count | Command |
|---|---|---|
| TI `Package / Case` labels at most, three families | 9 + 22 + 37 = 68 | `grep '^pick.*Texas' planning/plan-55-data/families.out` → `packages(case)` 9, 22, 37 |
| `F` lines after E | the distinct (company, package, supplier package) of the TI `add` rows | `awk -F'\t' '$1=="F"' planning/plan-55-data/footprints-needed.txt \| wc -l` (recorded here when F starts: **51**; 52 units once the pin count splits a line) |

## 3. Design

### 3.1 Ids, reuse and the map

Ids follow the library's (`SOT23-5`, `SC70-6-ONSEMI-MAA06A`: package family, pin count, a maker and
drawing code when the pads differ from the generic one); two lines with the same pads share one id.
`footprint-map.tsv` columns: `letter`, `company`, `package`, `supplier_package`, `footprint`, `source`
(`reuse` or `new`), `drawing` (the datasheet's package code and page), `pinouts` (the spec rows served).

### 3.2 The two classes

*Leaded* lines (`package` naming `SOT`, `SC-70`, `SOIC`, `SO`, `TSSOP`, `MSOP`, `SSOP`, `DIP`, `TO-`)
are Phase 2's; *leadless* lines (`DFN`, `QFN`, `WSON`, `UFDFN`, `X2SON`, `WLCSP`, `DSBGA`, `BGA`,
`LGA`) are Phase 3's.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The tools and the reuse pass

- [x] Copy `fp-check.py`, `fp-load.sh`, `fp-merge.py` from `planning/plan-42-data/census/` and adapt the
      paths (`planning/plan-55-data/`, `footprints-<letter>.json`). Beside them `census/fp-units.py` (the units and the reader packets), `fp-find.py` (reuse search), `fp-combine.py` (merges the packets' files) and `FOOTPRINT-GUIDE.md` (Correction 1).
- [x] Map every `F` line to a library footprint whose pads equal its package drawing's (`source`
      `reuse`); the lines left are Phase 2's and 3's. Observed: 33 of the 60 map rows are `reuse`.

Acceptance: the tools run and every reusable line is mapped.
  Check: `python3 -I planning/plan-55-data/census/fp-check.py F` → prints the count of lines still without a footprint (est. 1 min).
Commit: c3380d83

### Phase 2 — Leaded and through-hole packages

- [x] Draw each remaining leaded `F` line from its package drawing into `footprints-F.json` (the
      `dual`, `quad`, `explicit` styles, § Footprints rules), `source` `new`. Observed: the readers' packets f01 to f08 (Correction 1) drew 27 new footprints in all (leaded and leadless together); `fp-load.sh planning/plan-55-data/footprints-F.json` → `27 new footprints: load ok`.

Acceptance: every leaded `F` line has a footprint and the objects load.
  Check: `planning/plan-55-data/census/fp-load.sh planning/plan-55-data/footprints-F.json` → `<k> new footprints: load ok` (est. 1 min).
Commit: c3380d83

### Phase 3 — Leadless and ball-grid packages

- [x] The same for the leadless `F` lines (`"centre"` for an exposed pad; `explicit` for WLCSP and
      DSBGA balls). Observed: `fp-check.py F` → `51 lines, 0 without a footprint`.

Acceptance: every `F` line has a footprint and the objects load.
  Check: `python3 -I planning/plan-55-data/census/fp-check.py F` → `<n> lines, 0 without a footprint`; `fp-load.sh planning/plan-55-data/footprints-F.json` → `<k> new footprints: load ok` (est. 2 min).
Commit: c3380d83

## Validation Plan

- Tests: `fp-check.py` and `fp-load.sh` (the loader's own checks on the new objects); no backend or
  app test is touched.
- Coverage check: `fp-check.py F` counts every `F` line once.
- Runtime proof: none; the footprints are drawn on a board in plan-55-J's harness.
- Doc sync: none (`docs/parts-format.md` § Footprints already describes the format).
- Final gate: plan-55-K.

## Decisions

- Author: **reuse beats drawing**: a line whose pads equal a library footprint's takes it.
- Author: **the footprints stay in the data directory until a parts letter uses them**
  (`footprint_check` rejects an unused one).

## Corrections

1. **The readers work in packets by unit.** A unit is (letter, company, `Package / Case`, `Supplier Device Package`, pin count): 52 for TI (a pin count splits a line: the 16-pin and the 17-pin HTSSOP; the numbered pad is the 17th pin), in eight packets (`census/fp-units.py --packets`, reader guide `planning/plan-55-data/FOOTPRINT-GUIDE.md`, outputs `footprints.json` and `map.tsv` per packet, merged by `fp-combine.py F`). `fp-check.py` checks every (PDF, pin count, part number) of a line, so `footprint-map.tsv` carries `pdf` (a row for one PDF wins) and, new here, `mpn` (a regex a part number must fully match, a row with it wins).
2. **A numbered exposed pad that is a pin** (plan-55-C Correction 6) is the `centre` pad with `slots` 0 for its pin: `HTSSOP-17-TI-PWP0016M` (TPS1HA08-Q1's VBB). The unnumbered pads (the usual exposed pad) stay unnamed.
3. **`tps22921.pdf` covers YFP (0.4 mm pitch) and YZP (0.5 mm) parts under one pair of labels**: `footprint-map-fixes.tsv` maps `TPS2292[12]B?YZPR` to `DSBGA-6-YZP`; the rest of the PDF is `DSBGA-6-YFP`.
4. **For a reviewer** (from the readers' reports; all load, none overlaps): the YZV0004 land pattern of tps22910a / tps22934 is taken from tps22930a p.21 (those PDFs have no drawing) and equals the library's `WLCSP-4-ADI-CB-4-4` pads (its body is 0.95 x 1.03, larger than the die); `HTSSOP-14-PWP` and `SOIC-16` are reused for PDFs with no 14-pin / 16-pin land pattern of their own (same family drawings); `SOT583-8-TI-DYC0008A` draws a stepped land as its 0.85 x 0.3 bounding rectangle; the RPW, DML, RYZ, CHU, RXP and RXN lands are read off rendered pages; `WSON-8-TI-DNY0008A` draws the 0.6 x 1.6 land of pins 6 to 8 as three abutting pads; `HVSSOP-8-TI-DGN0008`'s exposed pad is the midpoint of four DGN drawings (within 0.05 mm of each); `HTSSOP-20-TI-PWP0020T`'s and `UQFN`/`WQFN` pads omit the drawings' R0.05 corners, as the library's TI footprints do; TPS254900-Q1's RVC0020A drawing comes from the TPS25940 datasheet (`slvscf3a.pdf` p.40-41, downloaded from ti.com), not from the placed PDF, which has none.

## Summary

Drawing land patterns is slow and exact; the loader's checks (no overlapping pads, mid-values) and
the pin-count match of `fp-check.py` catch a wrong one, and no part exists yet to depend on it.
