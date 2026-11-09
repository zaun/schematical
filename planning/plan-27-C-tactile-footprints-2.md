# plan-27-C: Tactile switch footprints and specs — Panasonic, Alps, Same Sky, TE, APEM, NKK, Würth

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-27-B

B fixed the design (plan-27-B § 4: footprint rules, sharing and ids,
`schemes.tsv` / `specs.tsv` formats, the three checkers) and did C&K,
E-Switch and Omron. C does the other seven makers the same way: the part-number
key, the land pattern, the pinout.

Outcome: every Panasonic, Alps, Same Sky, TE, APEM, NKK and Würth row of
`candidates.tsv` is `add`, `skip` or `out`; `footprints-C.json` holds one ready
footprint object per land pattern their `add` rows use that
`footprints-B.json` does not already hold; their `specs.tsv` rows are complete.

References:

- `planning/plan-27-B-tactile-footprints-1.md` § 4 (the design, in full),
  `planning/plan-27-A-tactile-data.md` § 3.
- `planning/plan-27-data/candidates.tsv`, `footprints-needed.txt`, `specs.tsv`,
  `schemes.tsv`, `footprints-B.json`.
- Datasheets: `datasheets/switch/tactile/{panasonic,alps,samesky,te,apem,nkk,wurth}/`.

## Prerequisites

The gate for the whole feature is in plan-27-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-27-B is complete | `ls planning/completed/plan-27-B-*` → one file; `ls planning/plan-27-data/census/{scheme-check.py,specs-check.py,fp-load.sh}` → three files | MET 2026-11-08 (a03d720; the three files exist) |

If plan-27-B is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The seven makers: no row `pending`; each is `add`, `skip` or `out`.
- `footprints-C.json` holds the new footprints; `footprint_check` loads them
  together with `footprints-B.json` (plan-27-B § 4.4).
- `specs-check.py` passes for each of the seven makers.

### Non-goals (explicit constraints)

- `Parts/footprints.json`, `Parts/switch.json`, goldens, backend, app untouched.
- No footprint that `footprints-B.json` already holds is redrawn (§ 4.2 of B).

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidate rows with a PDF: Panasonic 193, Alps 120, Same Sky 379, TE 176, APEM 129, NKK 88, Würth 136 | 1221 | `awk -F'\t' '$18=="candidate" && $20==1 && index($1,"Panasonic")==1' planning/plan-27-data/candidates.tsv \| wc -l` etc. |
| …verbatim (`hit`=1) | 183 / 41 / 11 / 117 / 18 / 7 / 136 = 513 | `awk … $21==1` |
| …`pending` | 10 / 79 / 368 / 59 / 111 / 81 / 0 = 708 | `awk … $21==0` |
| Footprint proxy lines | 40 / 38 / 36 / 28 / 12 / 7 / 5 = 166 | `grep -c '^ *[0-9]*  Panasonic' planning/plan-27-data/footprints-needed.txt` etc. |
| Distinct datasheet URLs of those rows | 49 / 95 / 36 / 124 / 8 / 6 / 136 | `awk … {print $17}` `\| sort -u \| wc -l` |
| `check` rows (blank DigiKey parameters) of C's makers: APEM 5, Same Sky 2, TE 1 (B has C&K 17, Aratas 5) | 8 | `awk -F'\t' '$18=="check"' planning/plan-27-data/candidates.tsv \| cut -c1-10 \| sort \| uniq -c` |

### Verified properties

- TE's datasheets are one customer drawing per part number
  (`www.te.com/commerce/DocumentDelivery/DDEController?Action=…DocId=Customer
  +Drawing…`; 124 URLs for 176 rows) and Würth's one PDF per article number
  (`we-online.com/catalog/datasheet/430151005826.pdf`; 136 for 136): for these
  two makers the footprint source of a (series, mounting) is the drawing of
  its first part number (plan-27-B Decisions).
- Same Sky (`sameskydevices.com/product/resource/ts02.pdf`) and APEM publish
  series datasheets with a part-number key; their 368 and 111 `pending` rows
  need the key (B § 4.3).

## 3. Design Overview

Identical to plan-27-B § 3 and § 4. Risk concentrates in Same Sky (368
`pending` rows decided by a regular expression) and TE (a drawing per code).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

Each phase's tasks, per maker: (0) a maker's `check` rows (the table in § 2) are
read from the same datasheet entry: a row that is not a non-illuminated
SPST-NO/NC → `out` (`circuit` or `lit`), else it follows (a)–(c) with the rest
and its blank `circuit`, `actuator` and `rating` columns are filled from the
datasheet (plan-27-B § 3);
(a) add `schemes.tsv` rows for series with
`pending` rows and run `scheme-check.py --maker <M>`; (b) draw each
(series, mounting) land pattern with an `add` row into `footprints-C.json`,
skipping ids that exist in `footprints-B.json` (moot with `grep -c
'"id":"<ID>"' planning/plan-27-data/footprints-*.json`); (c) add the
`specs.tsv` rows.

### Phase 1 — Panasonic (40 lines, 193 rows) and Alps (38 lines, 120 rows)

- [ ] Tasks (a)–(c) for `panasonic/` and `alps/`.

Acceptance: no Panasonic or Alps row is `pending`; checks pass.
  Check: `python3 -I planning/plan-27-data/census/specs-check.py --maker Panasonic && python3 -I planning/plan-27-data/census/specs-check.py --maker Alps && planning/plan-27-data/census/fp-load.sh planning/plan-27-data/footprints-B.json planning/plan-27-data/footprints-C.json` → `specs ok` twice, only extra/unused failures for the new ids (est. 4 min).
Commit: —

### Phase 2 — Same Sky (36 lines, 379 rows)

- [x] Tasks (a)–(c) for `samesky/` (`TS02`, `TS04`, `TS06`, `TS10`, `TS11`,
      `TS14`, … from `footprints-needed.txt`). Same Sky reader (`work/samesky/`,
      three passes: most PDFs were truncated downloads at first, plan-27-A
      Correction 7); `merge-work.py C samesky` → 29 footprints drawn, 24 new
      (5 identical to B's: `TS01 -> PTS645-THT`, `TS13 -> PTS125-THT`, `TS20 ->
      PTS810`, `TS34 -> TL6800`, `TS44 -> TL6150-J`); `scheme-check.py --maker
      'Same Sky'`: `371 add, 2 skip, 0 still pending` (the two TS06 `-SMT-T`
      check rows, not in the key); 5 `skip` from `cannot-draw.tsv` (TS12, a
      5-way navigation switch with 6 terminals); TS41 (3 terminals) is a
      2-pin footprint with pin 1 on two pads (plan-27-B Correction 3).

Acceptance: no Same Sky row is `pending`; checks pass; the NC row
`TS10-63-26-BE-250-SMT-TR` is `add` or `skip` with a reason.
  Check: the Phase 1 command with `--maker 'Same Sky'` (est. 3 min).
  Result 2026-11-08: `specs ok (374 add rows)`; `fp-load.sh footprints-B.json footprints-C.json` → `24 new footprints: load ok`; TS10-63-26-BE-250-SMT-TR (SPST-NC) is `add` (ts10.pdf p.1 "normally closed", p.2 schematic). Same Sky rows: 374 add, 7 skip, 24 out.
Commit: —

### Phase 3 — TE (28 lines, 176 rows)

- [ ] Tasks (a)–(c) for `te/`; for series with per-part drawings use the
      first part number of each (series, mounting).

Acceptance: no TE row is `pending`; checks pass.
  Check: the Phase 1 command with `--maker 'TE Conn'` (est. 3 min).
Commit: —

### Phase 4 — APEM (12 lines), NKK (7), Würth (5)

- [ ] Tasks (a)–(c) for `apem/`, `nkk/`, `wurth/`.
- [ ] § Summary: add / skip counts per maker, the count of footprints in
      `footprints-C.json`, and the total across B and C.

Acceptance: no row of the seven makers is `pending`/`candidate`/`check`.
  Check: `awk -F'\t' 'NR>1 && ($18=="pending" || $18=="candidate" || $18=="check")' planning/plan-27-data/candidates.tsv | wc -l` → 0 (est. 1 min).
Commit: —

## Validation Plan

- Tests: `specs-check.py` per maker; `fp-load.sh` over both footprint files.
- Coverage check: Phase 4's `awk` → 0 across the whole file.
- Runtime proof: none (data only).
- Doc sync: `datasheets/skipped-parts.md` gains the rows decided here.
- Final gate (once, after Phase 4): `ctest --test-dir /tmp/logic-p27` → `100% tests passed` (est. 5 min; library unchanged).

## Decisions

- Design, scope and the one-pattern-per-series rule: plan-27-B § 4 and
  Decisions; owner's scope, 2026-11-07 (plan-27-A § Decisions).
- **Würth and TE use the first code's drawing per (series, mounting)** — author.

## Corrections

<!-- filled in during execution -->

## Summary

Risk: Same Sky's and APEM's `pending` rows ride on hand-written regular
expressions. `scheme-check.py` prints each accepted and rejected part number so
the expression can be read against the key. Untouched: all library and app files.
