# plan-37-D: Parts — the 4- to 10-bit registers (16-, 20- and 24-pin) and two 74_670 parts

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-37-C

D fills `Parts/register.json` with the 17 narrow register devices and
their `add` parts, adds the two in-stock SN74LS670 parts the `mux` device
74_670 lacks, merges `DHVQFN-24` from `footprints-B.json` with its first
user, adds the `datasheets.json` entries, and regenerates the goldens.
It also builds the generator E and F reuse.

Outcome of D: `grep -c '|register|' backend/tests/golden/parts.txt` = the
D `add` rows (198 in-scope rows at planning, before A's verdicts);
`grep -c '|mux|' …` = 346 + the 74_670 `add` rows (2 at planning);
`git diff backend/tests/golden` shows only added lines; `register_test`'s
`check_real` passes on SN74LS173AN, SN74HC377ANSR and CD4076BE (§ 3.4).

References:

- plan-37-A § Prerequisites; § 3 (verdicts); C § 3.2–3.3 (the format and
  behavior); B (`footprints-B.json`).
- `docs/parts-format.md` § Devices, § Parts, § `register` (C), § Datasheets,
  § The `"footprint"` part key; `docs/backend.md` § Tests (regenerating the
  goldens: `listing_check --print`, `trace_check --print`,
  `footprint_check --print`).
- `planning/plan-35-data/census/gen-parts.py` (the generator precedent);
  `fp-merge.py` (B's copy).
- `schematical/schematical/Parts/latch.json` 74_373 / 74_16373 (note,
  drawer and timing-entry style), `mux.json` 74_670.

## Prerequisites

plan-37-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-37-C complete | `ls planning/completed/plan-37-C-*.md` → exists | NOT MET (2026-11-12: plan written) |
| `register.json` is empty | `python3 -c "import json;print(len(json.load(open('schematical/schematical/Parts/register.json'))['devices']))"` → 0 | NOT MET until C |

## 1. Goal

- `register.json` holds one device per D device with an `add` row and one
  part per `add` row; `mux.json` 74_670 gains its `add` rows.
- `ctest` passes (T + 1 programs, as after C); `register_test` has a
  `check_real` case on real parts (below) and all its cases pass.
- `git diff <D base> -- backend/tests/golden | grep -c '^-[^-]'` → 0.
- `node scripts/check-datasheets.js <every new id>` → every id answers PDF.

### Non-goals (explicit constraints)

- No wide device (E), no `dff` part (F), no app change (G).
- No change to an existing device's keys, pinouts or timing, 74_670's
  included (its parts are appended only).
- No device beyond the 17 below, even if a datasheet covers more types.

## 2. Current State

D's rows (`candidates.tsv`, `scope` = `in`, `kind` `register` with a
non-`74_16…` device, or `kind` `mux`): **198** (TI 168, Nexperia 20,
onsemi 10), 17 `register` devices + 74_670:

| Device | Rows | Shape (C § 3.2) | Pinouts |
|---|---|---|---|
| 74_173 | 16 | 1 group × 4; `ce` [G1, G2] low; `clear` CLR high async; bank `q`, `oe` [M, N] low | p16, lccc20 |
| 4076B | 6 | as 74_173 (CD4076B / MC14076B: M, N, G1, G2, Reset) | p16 |
| 40174B | 8 | 1 group × 6; `clear` CLR low async; bank `q` | p16 |
| 74_377 | 61 | 1 group × 8; `ce` G low; bank `q` | p20, lccc20 |
| 74_378 | 8 | 1 group × 6; `ce` G low; bank `q` | p16 |
| 74_379 | 3 | 1 group × 4; `ce` G low; banks `q`, inverted `qn` | p16, lccc20 |
| 74_534 | 34 | 1 group × 8 ('374 pinout); bank inverted, `oe` OE low | p20, lccc20 |
| 74_564 | 18 | as 74_534 on the '574 flow-through pinout | p20 |
| 74_575 | 1 | 1 group × 8; `clear` CLR low clocked; bank `oe` OE low | p24 |
| 74_576 | 6 | as 74_564 ('574 pinout, inverted) | p20 |
| 74_577 | 1 | as 74_575, inverted | p24 |
| 74_821 | 14 | 1 group × 10; bank `oe` OE low | p24 |
| 74_823 | 13 | 1 group × 9; `ce` CLKEN low; `clear` CLR low async; bank `oe` OE low | p24 |
| 74_825 | 2 | 1 group × 8; `ce` CLKEN low; `clear` CLR low async; bank `oe` [OE1, OE2, OE3] low | p24 |
| 74_874 | 3 | 2 groups × 4, each own CLK, `clear` CLR low async, bank `oe` OE low | p24 |
| 74_11374 | 1 | 1 group × 8; bank `oe` OE low (24-pin flow-through) | p24 |
| 74_29821 | 1 | 1 group × 10; bank `oe` OE low (`sn74als29821.pdf` function table: OE, CLK, D → Q) | p24 |
| 74_670 (`mux`) | 2 | existing device, `regfile` | existing p16 |

Families in D: ABT, AC, ACT, AHCT, ALS, AS, C, F, FCT, HC, HCT, LS, LVC,
TTL (all known before C). Footprints: every D row maps to an existing id
except one `DHVQFN-24` row (74LVC823ABQ,118): `awk -F'\t' '$6=="in" && $3!~/^74_16/ && ($5=="register"||$5=="mux"){print $20}' candidates.tsv | sort | uniq -c`.

The '670 rows: SN74LS670N3, SN74LS670NS (`sn74ls670`, already a
`datasheets.json` id: `grep -c '"sn74ls670"' schematical/schematical/Parts/datasheets.json` → 1).

## 3. Design

### 3.1 Specs and generator

- `planning/plan-37-data/specs/<device>.json`: one file per device, the
  device object without `parts` (id, function, description, drawer,
  note, model, pinouts, groups, timing), transcribed from the datasheets
  of its `add` rows.
- `census/gen-parts.py <specs dir> <candidates.tsv> [--device RE] [--check]`
  (from plan-35's): prints each device with its `parts` from the `add`
  rows, sorted by name; `--check` verifies every row's `family` is a
  `timing` key, its pinout exists with the footprint's pin count, every
  group pin is in every pinout, and each part's package/footprint pair.

### 3.2 Device facts

- `function`: `"Register"`. `drawer`: `["Logic", "Registers"]` (the
  category the shift registers and 74_670 use).
- `description`: the datasheet title, as the latch devices
  (e.g. "4-bit D-type registers with 3-state outputs").
- **Pin names** follow TI's sheet where TI makes the device, else the
  maker's. The `note` lists every datasheet (id, page of the pinout and
  function table) and the other makers' names for the same pins
  (Nexperia 74HC173: OE1, OE2, E1, E2, MR, CP, Dn, Qn).
- **Pinouts** by pin count, keyed `p16`, `p20`, `p24`; the FK / LCCC
  carriers as `lccc20` (the `dff` 74_174 precedent). A maker's package
  with the same numbering uses the same pinout (checked by `--check`
  against each datasheet's pin table in Phase 1).
- **Timing**, per family: CLK → Q `{rise, fall}` = tPLH / tPHL typical at
  the datasheet's nominal VCC (5 V, or 3.3 V for LVC), its stated load;
  max where no typical is given, said in the `note`; `vcc` from
  Recommended Operating Conditions (`[min, max]`); `inputMax` where the
  family is 5 V-tolerant (LVC). One entry per family; parts of another
  maker in the same family use it, and set their own part `vcc` when their
  datasheet's range differs.

### 3.3 Parts

From each `add` row: `name` (the part number), `manufacturer` (`Texas
Instruments`, `onsemi`, `Nexperia`), `datasheet` (the PDF id), `package`
(the footprint's family name: `DIP`, `SOIC`, `SOP`, `SSOP`, `QSOP`,
`TSSOP`, `TVSOP`, `LCCC`, `CFP`, `DHVQFN`), `pinout`, `footprint`,
`family`, `flags` (`military` + `ceramic` / `automotive` from A Phase 3's
`grade`).

### 3.4 `check_real` (in `register_test`)

Load the real library and run the `check_173` steps on SN74LS173AN, the
`check_377` steps on SN74HC377ANSR, and the `check_173` steps (with 4076B's
pin names) on CD4076BE: one in-scope planning row per device. A part A
did not make `add` is replaced by the first `add` part of the same device
by name, recorded in § Corrections. The steps are each device's own
function table.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Specs

- [ ] `census/gen-parts.py`.
- [ ] `specs/` for the 17 devices of § 2 that have an `add` row.

Check: `python3 -I planning/plan-37-data/census/gen-parts.py planning/plan-37-data/specs planning/plan-37-data/candidates.tsv --check --device '^(74_(173|377|378|379|534|564|575|576|577|821|823|825|874|11374|29821)|4076B|40174B)$'`
→ `<n> rows ok` with n = D's `add` rows less the 74_670 ones (est. 1 min).

Commit: —

### Phase 2 — Part files, footprint, datasheets

- [ ] Append the generated devices to `register.json`; append the 74_670
      `add` parts to `mux.json` 74_670 (by name order of the existing list).
- [ ] Merge into `footprints.json` (`fp-merge.py`) every `footprints-B.json`
      id a D `add` row names (planning: `DHVQFN-24`, for 74LVC823ABQ,118;
      B drew only ids with an `add` row).
- [ ] `datasheets.json`: one `{"id", "url"}` per new PDF (A's `datasheets.tsv`).
- [ ] `register_test` `check_real` (§ 3.4).
- [ ] Regenerate `parts.txt`, `traces.txt`, `footprints.txt`; review the
      diff (added lines only).

Check: `cmake --build /tmp/logic-p37 -j8 && ctest --test-dir /tmp/logic-p37 -j8`
→ `100% tests passed`; `git diff -- backend/tests/golden | grep -c '^-[^-]'` → 0;
`grep -c '|register|' backend/tests/golden/parts.txt` → D's `register` `add`
rows (est. 6 min).

Commit: —

## Decisions

- **74_670 parts go to `mux.json`**, the device that already models the
  '670 (plan-37-A § Decisions); no second register-file model.
- **Shared timing per family** across makers (§ 3.2): the catalog keys
  timing by (device, family), and a part has no timing key.
- **Package names**: the footprint id's family (`QSOP` for QSOP-20/24, as
  the 24 existing QSOP-20 parts that say `QSOP`), `DHVQFN` for Nexperia's
  leadless package (its own name, no pin count).
- **One `specs/` file per device**, transcribed once and generated into
  `register.json`, so E and F reuse the generator.

## Corrections

(none yet)

## Summary

D ships the narrow registers (17 devices) and two missing '670 parts with
their datasheets, and adds only new golden lines.
