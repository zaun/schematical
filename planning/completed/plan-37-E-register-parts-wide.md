# plan-37-E: Parts — the wide registers (48-, 56- and 64-pin)

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-37-D

E adds the 9 wide register devices to `Parts/register.json` with their
`add` parts, merges `TSSOP-64` from `footprints-B.json` with its user, adds
the `datasheets.json` entries and regenerates the goldens. It uses D's
`specs/` + `gen-parts.py` flow unchanged.

Outcome of E: `grep -c '|register|' backend/tests/golden/parts.txt` = D's
count + E's `add` rows (76 in-scope rows at planning, before A's verdicts);
`git diff backend/tests/golden` shows only added lines; `register_test`'s
`check_real` also passes on SN74LVC16374ADGGR (two groups) and
SN74ALVCH16823DGGR (CLKEN and CLR per group).

References:

- plan-37-A § Prerequisites; C § 3.2–3.3; D § 3 (specs, generator,
  device facts, parts, `check_real`).
- `schematical/schematical/Parts/latch.json` 74_16373 (the 48-pin
  two-group precedent: `1OE`/`2OE`, `1LE`/`2LE`, pinout `p48`).
- `docs/parts-format.md` § Values (LCX, ALVT, added by C).

## Prerequisites

plan-37-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-37-D complete | `ls planning/completed/plan-37-D-*.md` → exists | MET (2026-11-13: `plan-37-D-register-parts-narrow.md`) |
| LCX and ALVT are known families | `grep -o '"LCX"\|"ALVT"' backend/src/library/kind_util.c \| wc -l` → 2 | MET (2026-11-13: 2) |

## 1. Goal

- `register.json` holds one device per E device with an `add` row, one
  part per `add` row.
- `ctest` passes; `register_test` `check_real` covers the two parts above.
- `git diff <E base> -- backend/tests/golden | grep -c '^-[^-]'` → 0.
- `node scripts/check-datasheets.js <every new id>` → every id answers PDF.

### Non-goals (explicit constraints)

- No narrow device change (D), no `dff` part (F), no app change (G).
- No 32-bit register (no in-stock part, plan-37-A § 3 rule 2).
- Bus hold and series-damping resistors (ALVCH, LVCH, LVTH, the 162xxx
  "-2" outputs) are not modeled: 74_162374 has 74_16374's logic.

## 2. Current State

E's rows (`candidates.tsv`, `scope` = `in`, `kind` `register`, device
`74_16…`): **76** (TI 62, Nexperia 11, onsemi 3), 9 devices
(`awk -F'\t' '$6=="in" && $3~/^74_16/' candidates.tsv | cut -f3 | sort | uniq -c`):

| Device | Rows | Shape (C § 3.2) | Pinout |
|---|---|---|---|
| 74_16374 | 43 | 2 groups × 8: 1CLK / 1OE, 2CLK / 2OE | p48 |
| 74_162374 | 7 | as 74_16374 | p48 |
| 74_16821 | 5 | 2 groups × 10: own CLK, OE | p56 |
| 74_16823 | 13 | 2 groups × 9: own CLK, `ce` CLKEN low, `clear` CLR low async, OE | p56 |
| 74_162823 | 1 | as 74_16823 | p56 |
| 74_16721 | 3 | 1 group × 20: CLK, `ce` CLKEN low, OE | p56 |
| 74_162721 | 2 | as 74_16721 | p56 |
| 74_16820 | 1 | 1 group × 10; banks 1Q (`oe` 1OE low) and 2Q (`oe` 2OE low) — `sn74alvch16820.pdf` function table "nOE CLK D → Qn, n = 1, 2" | p56 |
| 74_16722 | 1 | 1 group × 22: CLK, `ce` CLKEN low, OE | p64 |

Families in E: ABT, ACT, AHC, AHCT, ALVC (ALVCH), ALVT, AUC (AUCH), AVC,
FCT, LCX, LVC (LVCH), LVT (LVTH). Footprints (`cut -f20` of E's rows):
TSSOP-48 22, SSOP-48 21, SSOP-56 15, TSSOP-56 8, TVSOP-48 7, TVSOP-56 2,
TSSOP-64 1 (new, B). The 74_16722 is 22 bits, within C's 64-bit limit.

## 3. Design

D § 3 throughout, with:

- **Pin names**: TI's `1D1 … 1D8, 1Q1 … 1Q8, 1CLK, 1OE, …` (Nexperia's
  `1Dn / 1Qn / 1CP / 1OE`, onsemi's `Dn / On / CPn / OEn` in the `note`).
- **Timing**: CLK → Q typ at 3.3 V for LVC, LVT, ALVC, ALVT, LCX and AVC,
  at 1.8 V for AUC (its datasheet's nominal), at 5 V for ABT, ACT, AHC,
  AHCT, FCT; `inputMax` for the 5 V-tolerant
  3.3 V families as the datasheet's input maximum.
- **`check_real`**: the `check_16823` steps of C on SN74ALVCH16823DGGR and
  a two-group load / Z step on SN74LVC16374ADGGR (a part A did not make
  `add` is replaced as D § 3.4 says).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Specs

- [x] `specs/` for the 9 devices of § 2 that have an `add` row (all 9),
      written by `census/specs-e.py`; every pin the text of the TI sheets
      prints was compared with the specs (`sn74avc16722` all 64,
      `sn74lvc16374a` all 48, 14–24 per 56-pin sheet): no mismatch.

Check: `python3 -I planning/plan-37-data/census/gen-parts.py planning/plan-37-data/specs planning/plan-37-data/candidates.tsv --check --device '^74_16'`
→ `<n> rows ok`, n = E's `add` rows (est. 1 min).
Observed: `67 rows ok`.

Commit: 97316a9

### Phase 2 — Part file, footprint, datasheets

- [x] Append the generated devices to `register.json` (`9 devices, 67
      parts written`).
- [x] Merge every `footprints-B.json` id an E `add` row names: `TSSOP-64`
      (SN74AVC16722DGGR), `1 footprints appended`.
- [x] `datasheets.json` entries for the new PDFs: `33 datasheets added`;
      `check-datasheets.js` on the 33 → `33 of 33 URLs serve a PDF`.
- [x] `register_test` `check_real` additions (§ 3): case 34
      (SN74LVC16374ADGGR, two groups) and 35–36 (SN74ALVCH16823DGGR, the
      16823 steps through a shared `run_16823`), all PASS.
- [x] Regenerate the three goldens; review (added lines only): +67, +67,
      +1, 0 removed.

Check: `cmake --build /tmp/logic-p37 -j8 && ctest --test-dir /tmp/logic-p37 -j8`
→ `100% tests passed`; `git diff -- backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 6 min).
Observed: `100% tests passed, 0 tests failed out of 33`; `0`;
`|register|` → 220 (D 153 + E 67); `parts.txt` 28048 lines.

Commit: 97316a9

## Decisions

- **The 162xxx devices are their own devices** (74_162374, 74_162721,
  74_162823), not parts of the 16xxx ones: the catalog keeps TI's device
  numbers apart (latch.json has both 74_16373 and 74_162373).
- **74_16820 is one group with two banks** (C § Decisions): one D drives two
  outputs; the function table's "n = 1, 2" is the two banks.

## Corrections

- **2026-11-13 — E's counts after A.** 67 `add` rows (TI 60, Nexperia 5,
  onsemi 2), not 76 in-scope rows: 74_16374 36, 74_162374 7, 74_16823 11,
  74_16821 5, 74_16721 3, 74_162721 2, 74_162823 1, 74_16820 1, 74_16722 1.
  Both § 3 `check_real` parts are `add` rows.
- **2026-11-13 — timing where the datasheet gives no typical**: as D
  (the max at the nominal supply, said in the `note`): ALVC, AVC, LVC,
  LCX, FCT. AUC is at 1.8 V, its nominal (§ 3). The CY74FCT16374 entry is
  the C grade, the grade of its one `add` row (74FCT16374CTPVCG4).
- **2026-11-13 — the 74_16820 banks.** `sn74alvch16820.pdf` p.1 names the
  outputs nQ1 and nQ2 (n = 1–10); bank 1 is nQ1 with 1OE, bank 2 nQ2 with
  2OE, as § 2 says (`1Q` / `2Q` there).

## Summary

E ships the 9 wide registers and the TSSOP-64 footprint, adding only new
golden lines.
