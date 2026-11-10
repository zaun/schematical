# plan-29-I: Parts — Vishay single diodes (and the part generator)

Last updated: 2026-11-09
Effort: large (3h–1d)
Depends on: plan-29-H

I writes `planning/plan-29-data/census/gen-parts.py`, which turns the `add`
rows of `candidates.tsv` and their `specs.tsv` rows into devices and parts in
`Parts/diode.json`, the datasheet entries in `Parts/datasheets.json`, and
moves the footprints those parts use from the `footprints-*.json` ready
objects into `Parts/footprints.json`. It runs it for the Vishay single
diodes and regenerates the three goldens. J, K and L run the same script for
their scopes.

References:

- plan-29-H § 5.1 (the `diode` file format), plan-29-B § 4 (pinouts,
  package names, footprint ids).
- `docs/parts-format.md` § Parts (`name`, `manufacturer`, `datasheet`,
  `package`, `flags`), § Datasheets, § Footprints.
- `docs/backend.md` (regenerating the goldens).
- `planning/plan-29-data/candidates.tsv`, `specs.tsv`,
  `footprints-[B-G].json`, `datasheets/diode/datasheets.tsv`.

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-H complete | `ls planning/completed/plan-29-H-*.md` → exists | MET (2026-11-09) |
| The `diode` kind loads | `grep -c 'PART_DIODE' backend/include/parts.h` → ≥ 1 | MET (2026-11-09: 1) |
| Every footprint letter done | `ls planning/completed/plan-29-[B-G]-*.md \| wc -l` → 6 | MET (2026-11-09: 6) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/diode.json` holds one part per `add` row with `category` =
  `single` and `maker` = Vishay General Semiconductor - Diodes Division.
  Planning time: 3410 `candidate` + 1 `check` rows before the datasheet
  rules (`awk -F'\t' '$1=="single" && $2 ~ /^Vishay/ && ($24=="candidate"||$24=="check")' planning/plan-29-data/candidates.tsv | wc -l` → 3411); A's decisions and B's schemes set the final number **N_I**.
- `parts.txt` and `traces.txt` grow by exactly N_I lines each, `footprints.txt`
  by the number of footprints the parts use that were not shipped before;
  no existing golden line changes
  (`git diff backend/tests/golden | grep -c '^-[^-]'` → 0).
- `ctest` passes (T0 + 1 programs).

### Non-goals (explicit constraints)

- No change to any non-diode part, footprint or golden line.
- No app change (plan-29-M).

## 2. Current State

- After H, `Parts/diode.json` is `"devices": []` and the kind loads it.
- The ready footprints are in `planning/plan-29-data/footprints-[B-G].json`;
  `specs.tsv` names, per `add` row group, `package`, `pinout`, `pins`,
  `footprint`.
- `datasheets.json` has 2621 entries
  (`grep -c '"id"' schematical/schematical/Parts/datasheets.json` → 2621,
  2026-11-08); none is a diode datasheet.

## 3. Design — `gen-parts.py`

`python3 -I planning/plan-29-data/census/gen-parts.py --maker <prefix of
the maker column> --category <c>[,<c>…]` reads `candidates.tsv` (`decision` = `add`),
`specs.tsv`, `datasheets/diode/datasheets.tsv` and the `footprints-*.json`
files, and rewrites `Parts/diode.json`, `Parts/datasheets.json` and
`Parts/footprints.json` with the scope's parts merged in (existing content
kept, sorted as below). Re-running for the same scope gives the same files.

### 3.1 Devices

One device per (type, diode shape):

| Field | Value |
|---|---|
| `id` | `<type>-<shape>`: `schottky-1`, `zener-1`, `fast-cc2` (common cathode pair), `fast-ca2`, `switching-ser2` (series pair), `rectifier-ind2` (two independent), `switching-cc3`…, `bridge-4` |
| `type` | from the row's `leaf`: Rectifier → `rectifier`, Fast recovery → `fast`, Switching → `switching`, Schottky → `schottky`, Zener → `zener`, Bridge → `bridge` |
| `function` | the `leaf` (`"Schottky"`, `"Fast recovery"`, …) |
| `description` | `"<Type> diode"`, `"Dual common-cathode <type> diode"`, `"Bridge rectifier"`, … |
| `drawer` | `["Discrete", "Diodes"]` |
| `diodes` | single: `[{"anode":"A","cathode":"K"}]`; arrays from the spec row's configuration (`cc2`: A1→K, A2→K); bridge: `[{"anode":"AC1","cathode":"+"},{"anode":"AC2","cathode":"+"},{"anode":"-","cathode":"AC1"},{"anode":"-","cathode":"AC2"}]` |
| `pinouts` | every distinct (`pinout`, `pins`) of the device's spec rows; all pins `inout` except the map's `NC` (`nc`) |

### 3.2 Parts

| Key | Value |
|---|---|
| `name` | the row's `mpn` |
| `manufacturer` | `"Vishay"`, `"onsemi"`, `"Taiwan Semiconductor"` |
| `datasheet` | the id of the row's PDF in `datasheets.json`: the PDF's file name, lower case, without `.pdf` (`1n4001`); `gen-parts.py` adds `{"id", "url"}` (the row's maker-site `datasheet_url`) when absent |
| `package`, `pinout`, `footprint` | the spec row's |
| `flags` | the row's `flags` (`automotive`; `military` when DigiKey's Grade is Military) |
| `vr`, `io` | from DigiKey's `vr` (`"50 V"` → 50) and `io` (`"1A"` → 1, `"500mA"` → 0.5), every type but zener |
| `vz`, `pd` | Zeners: `vz` (`"5.1 V"` → 5.1), `pd` from `power` (`"500 mW"` → 0.5) |

The DigiKey values are checked once per spec row against the datasheet's
maximum-ratings table for the first part of the group (`specs.tsv` `note`
records `vr/io checked p.N`); a mismatch takes the datasheet's value for
the whole group and is listed in § Corrections.

### 3.3 Footprints

Each footprint id the scope's parts name that is not yet in
`Parts/footprints.json` is copied from its `footprints-*.json` object.
Shared ids (`SOT23-3`, `SC70-3`, `SOT363-6`, …, plan-29-B § 4.4) are added
only when absent: `gen-parts.py` prints `reused <id>` for one that is
already there, and the step is moot for it.

### 3.4 Order and size

Devices sorted by `id`, parts by `name` within a device, as the other part
files. The file stays one JSON document; `Parts/diode.json` will hold every
diode (J–L add to it).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generator

- [x] `planning/plan-29-data/census/gen-parts.py` (§ 3), with `--dry-run`
      (prints device, part and footprint counts, writes nothing). Also
      `--ids` (the footprint id map), the footprint id merge (§ Corrections),
      `work/part-overrides.tsv`, and the ratings of `vrio-check.py` /
      `work/values.tsv`. Checked: `--maker Vishay --category single
      --dry-run` → `3179 parts, 0 without spec, 0 unknown footprint; 6
      devices, 56 new footprints, 1238 new datasheets`; N_I = 3179.

Acceptance: a dry run over I's scope names N_I parts and no missing spec.
  Check: `python3 -I planning/plan-29-data/census/gen-parts.py --maker Vishay --category single --dry-run` → `N_I parts, 0 without spec, 0 unknown footprint` (est. 1 min).
Commit: 0396b84

### Phase 2 — Vishay single diodes

- [x] Run it for `--maker Vishay --category single`. Same counts as the dry
      run; `library_check` exit 0 after the `vr` limit fix (§ Corrections).
- [x] `docs/parts-format.md` § Footprints: R7 (plan-29-B § 4.1) beside the
      R-rule pointer, now that axial footprints ship (a bullet after
      "Mid-values": R7 is the one rule that takes max values).
- [x] Regenerate the goldens (`listing_check --print`, `trace_check
      --print`, `footprint_check --print`, `docs/backend.md`) and review:
      only added lines. `git diff --stat backend/tests/golden` →
      footprints.txt +56, parts.txt +3179, traces.txt +3179; `git diff
      backend/tests/golden | grep -c '^-[^-]'` → 0. Runtime proof:
      `trace_check --verbose 1N4001-E3/54` → `126713 pin 1 -> X`, `126713
      pin 2 -> X` (weak 1 on A, weak 0 on K: forward bias) and `148292 pin 1
      -> 1` with pin 2 at 1 (the 1 passed to the cathode).

Acceptance: the parts load and the goldens only grow.
  Check: `cmake --build /tmp/logic-p29 -j8 && ctest --test-dir /tmp/logic-p29` → `100% tests passed`; `grep -c '|diode|' backend/tests/golden/parts.txt` → N_I; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 6 min).
  Result 2026-11-09: `100% tests passed, 0 tests failed out of 30`; 3179; 0.
Commit: 0396b84

## Validation Plan

- Tests: `listing_check`, `trace_check`, `footprint_check`, `library_check`.
- Coverage check: `grep -c '|diode|' backend/tests/golden/parts.txt` equals
  the scope's `add` count.
- Runtime proof: `/tmp/logic-p29/trace_check --verbose 1N4001-E3/54` (or
  the first `add` part) prints its net changes: whenever the weak stimulus
  drives A to 1 and K to 0, both nets read X (forward bias between two weak
  drivers, plan-29-H § 4.3 step 4), which an unlinked pair never shows.
- Doc sync: `docs/parts-format.md` (R7).
- Final gate: the Phase 2 check (est. 6 min).

## Decisions

- **Manufacturer names** — author: `Vishay`, `onsemi`, `Taiwan
  Semiconductor` (the company, not DigiKey's division string).
- **Drawer `Discrete → Diodes → <leaf>`** — author: no `Discrete` group exists
  (`cut -d'|' -f4 backend/tests/golden/parts.txt | sort -u` shows none);
  diodes are not indicators.
- **DigiKey values, checked per group** — author (§ 3.2).
- **One generator for I–L** — author.

## Corrections

- **Footprint ids are merged in gen-parts.py** — the owners' provisional
  `<BASE>-<MAKER>` ids: identical objects collapse to one, and per BASE the
  first in letter order takes the bare id (`DIODE-SMA`, `SOT23-3`,
  `TO-220-3`); a different land pattern of the same package keeps its maker
  tag (`DIODE-SMA-TSC`). `gen-parts.py --ids` prints the map.
- **Datasheet ids** are the PDF's file name, lower case, without `.pdf`;
  when another maker's PDF already has that id, `<maker>-<name>`.
- **`vr` above 3000 V** — GP02-35 / GP02-40 (3500 / 4000 V) failed the
  kind's range; the limit is now 10000 V (plan-29-H § Corrections).
- **Ratings** come from DigiKey, checked per spec row by
  `census/vrio-check.py` (every spec row's first part's numbers must be
  printed in its PDF); 118 values read from the datasheets where DigiKey
  gave none or disagreed are in `work/values.tsv` (e.g. GSD2004 io 0.225 →
  0.35 A, BZT52 pd 0.41 → 0.5 W, V5NM63 vr 65 → 60 V).
- **N_I = 3179** (Vishay single add rows, 2026-11-09).

## Summary

I is where the plan's data first ships: the generator is the risk (a wrong
spec join puts a part on the wrong pinout or footprint), checked by the dry
run's zero counts and by every golden line being an addition.
