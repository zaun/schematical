# plan-27-E: Tactile switch parts — the generator and C&K

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-27-D

D taught the backend the `momentary`, `normallyClosed` and `joined` keys; B
and C produced `candidates.tsv`, `specs.tsv` and the ready footprints. E writes
the generator that turns those into `Parts/switch.json` devices and parts
(§ 4), runs it for **C&K** (the first maker with an NC part), moves the
footprints C&K's parts use into `Parts/footprints.json`, and regenerates the
goldens. F–I run the same generator for the other makers.

Outcome: `Parts/switch.json` has the 36 DIP parts unchanged plus one part per
C&K row of `candidates.tsv` with `decision` `add` (Normal Open and Normal Close
devices under drawer `["Switch","Tactile"]`); `ctest` passes; the goldens
gain exactly those lines.

References:

- `planning/plan-27-D-tactile-kind.md` § 4 (the keys), `planning/plan-27-B-tactile-footprints-1.md` § 4
  (specs.tsv, footprints), `planning/plan-27-A-tactile-data.md` § 3.
- `docs/parts-format.md` § Parts, § `switch`, § Footprints; `docs/backend.md`
  (golden regeneration: `listing_check --print`, `trace_check --print`,
  `footprint_check --print`).
- `scripts/gen-headers.js` (precedent for a generator that owns part of a
  parts file).

## Prerequisites

The gate for the whole feature is in plan-27-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-27-D is complete | `ls planning/completed/plan-27-D-*` → one file; `grep -c 'momentary' backend/src/models/switch.c` ≥ 1 | MET 2026-11-08 (c9dd9cd; 4 lines) |
| Golden sizes at start | `wc -l < backend/tests/golden/parts.txt` → **P_E** (11126 at planning time; 13158 after main's merge, plan-27-A Correction 3) | **P_E = 13158**, R = 13158, F = 508 (2026-11-08) |

If plan-27-D is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-27-data/gen-parts.py` and `append-footprints.py` exist and are
  idempotent.
- Every C&K row with `decision` `add` is a part of `Parts/switch.json`;
  `wc -l < backend/tests/golden/parts.txt` = P_E + N_ck and the same for
  `traces.txt`, where N_ck = `awk -F'\t' '$18=="add" && index($1,"C&K")==1'
  planning/plan-27-data/candidates.tsv | wc -l` (440 verbatim rows at planning
  time, plus the rows B decided from the key).
- The footprints those parts name are in `Parts/footprints.json` and
  `footprints.txt` gained exactly their lines.

### Non-goals (explicit constraints)

- The 36 DIP parts and their golden lines do not change (`git diff` of the
  goldens shows only added lines).
- No part of another maker is added.

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| C&K rows `add` (after B) | N_ck | `awk -F'\t' '$18=="add" && index($1,"C&K")==1' planning/plan-27-data/candidates.tsv \| wc -l` |
| C&K NC parts | 3 (ATS2D1G NC LFS, ATS2D3G NC LFG, KSR223G NC LFG) + any `check` row B decided NC | `awk -F'\t' '$18=="add" && index($1,"C&K")==1 && $6=="SPST-NC"' …candidates.tsv \| wc -l` |
| Distinct (circuit, actuator, volts) among C&K `add` rows | D_ck | `awk -F'\t' '$18=="add" && index($1,"C&K")==1 {print $6"\|"$9"\|"$13}' …candidates.tsv \| sort -u \| wc -l` (DigiKey's `rating` text carries the volts) |
| Ready footprints in `footprints-B.json` used by C&K | F_ck | `python3 -I planning/plan-27-data/gen-parts.py --maker 'C&K' --list-footprints \| wc -l` |

### Verified properties

- `Parts/switch.json` ends with the DIP devices and the closing `\n  ]\n}`;
  nothing else follows (`tail -3 schematical/schematical/Parts/switch.json`).
- The loader rejects a part name used twice across the library
  (`docs/parts-format.md` § Loading rules), so the generator refuses a name
  already in any `Parts/*.json` (`grep -c '"name":"<MPN>"'`).

## 3. Design Overview

One generator, three inputs (`candidates.tsv`, `specs.tsv`, the footprint
objects), one output region of `switch.json`. Correctness risk concentrates in
the join between a part and its `specs.tsv` row (a wrong pinout is silent);
`specs-check.py` (B) already proves each `add` part matches exactly one row,
and the generator re-asserts it. Byte-identity is the gate for the part of the
file the generator does not own (the DIP devices) and for a second run
(idempotence); new parts are gated by the loader and the goldens.

## 4. Detailed Design

### 4.1 `gen-parts.py [--maker M]… [--list-footprints] [--dry-run] [--selftest]`

For every `add` row of the named makers (all makers if none named), in
part-name order:

| Field | Value |
|---|---|
| device `id` | `tact_<no\|nc>_<T>t_<V>v_<slug>`: `no` for SPST-NO, `nc` for SPST-NC; `<T>` terminals; `<V>` the volts of the row's `rating` text (`0.05A @ 12VDC` → `12`); `<slug>` the row's `actuator` lowercased, spaces → `-` |
| `function` | `Normal Open` / `Normal Close` |
| `drawer` | `["Switch","Tactile"]` |
| `description` | `Tactile switch, SPST normally open\|closed, <T> terminals, <V> V, <actuator> actuator` |
| `note` | `Rating from DigiKey "Contact Rating @ Voltage" (<amps>A @ <V>VDC, DC switching); datasheets: <comma-separated paths under datasheets/switch/tactile/>` — the files of this device's parts |
| `pinouts` | per distinct `pins` string of its parts: name `p<T>_<A numbers joined by _>_<B numbers joined by _>`; pins 1…T in number order; role `inout`; names `A`,`B` (T = 2) or `A1`,`A2`,`B1`,`B2` in the order listed in `pins` |
| `actuator` | the row's DigiKey `actuator` (`Standard`, `Plunger for Cap`, …) |
| `contacts` | `[["A","B"]]` (T = 2) or `[["A1","B1"]]` (T = 4) |
| `joined` | T = 4 only: `[["A1","A2"],["B1","B2"]]` |
| `momentary` | `true` |
| `normallyClosed` | `true` for `nc` devices only |
| `ratedVolts` | `<V>` |
| part | `{"name": MPN, "manufacturer": M, "datasheet": D, "package": P, "pinout": <name>, "footprint": <specs footprint>}` (Correction 1); `M` from the maker table below; `D` the id of the row's datasheet in `Parts/datasheets.json`; `P` from the row's `mounting`: `Surface Mount` → `SMD`, `Through Hole` → `THT`, `Through Hole, Right Angle` → `THT right angle`, `Surface Mount, Right Angle` → `SMD right angle`, `Edge Mount, Right Angle` → `Edge mount`; no `flags` |

Manufacturer (`docs/parts-format.md` § Parts: the buyer when a maker was
bought and the numbers stayed): C&K → `Littelfuse` (C&K is Littelfuse's;
its datasheets are on `littelfuse.com`; the library's Zilog parts already
read `Littelfuse`), E-Switch → `E-Switch`, Aratas (Formerly Omron
Components) → `Aratas`, Panasonic Industry → `Panasonic`, Alps Alpine →
`Alps Alpine`, Same Sky (Formerly CUI Devices) → `Same Sky` (the library's
barrel jacks), TE Connectivity ALCOSWITCH Switches → `TE Connectivity` (the
DIP switches), APEM Inc. → `APEM`, NKK Switches → `NKK Switches`, Würth
Elektronik → `Würth Elektronik`.

Datasheets (`Parts/datasheets.json`): one entry per distinct datasheet of
the generated parts, `id` = the placed PDF's file name in lower case without
`.pdf` (the file's custom), `url` = `census/maker-urls.py --url
<datasheet_url>` (the maker's URL when plan-27-A Phase 5 found one, else
the DigiKey URL: owner, 2026-11-08). An id already in `datasheets.json`
with the same URL is reused; with another URL the generator refuses. The
generator adds its entries in id order and never touches the others; a
second run adds nothing.

The generator owns the text of `switch.json` from the first device whose `id`
starts `tact_` (or from the end of the array when there is none) to the end
of the `devices` array; it never touches the text before it, and writes its
region in the file's existing layout (2-space indent, one pin per line, one
part per line). Devices are sorted by `id`, parts by `name`; it merges the
region it finds with the `add` rows of the named makers, so running it for the
next maker keeps the earlier ones. `--dry-run` writes nothing and prints `no changes` or the new region's diff;
`--list-footprints` prints the footprint ids the `add` rows name;
`--selftest` is Phase 1's check. Refusals (exit non-zero, message names the
row): a part name already in any `Parts/*.json` outside the generated region;
an `add` row with zero or two `specs.tsv` matches; a footprint id in no
`footprints-*.json` and not in `Parts/footprints.json`.

### 4.2 `append-footprints.py <maker…>`

Appends to `Parts/footprints.json` (one line per footprint, the file's
layout) every footprint the generated region names that `Parts/footprints.json`
does not already hold (`grep -c '"id":"<ID>"'` → 0; otherwise the step is moot
for that id), taking the object from `footprints-B.json` / `footprints-C.json`.

### 4.3 Goldens

`cmake --build /tmp/logic-p27 -j8`, then `/tmp/logic-p27/listing_check --print
> backend/tests/golden/parts.txt`, `/tmp/logic-p27/trace_check --print >
backend/tests/golden/traces.txt`, `/tmp/logic-p27/footprint_check --print >
backend/tests/golden/footprints.txt`; review `git diff --stat
backend/tests/golden` (only added lines) and the first 5 added `parts.txt`
lines by hand (they read `…|switch|THT|Switch/Tactile|…positions=1
actuator=… rated=12000 momentary joined=2|…`).

### 4.4 Docs

`docs/parts-format.md` § `switch`: one paragraph — the library's tactile
switches are written by `planning/plan-27-data/gen-parts.py` (device ids,
drawer, function names, package values).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generator

- [x] `planning/plan-27-data/gen-parts.py` (§ 4.1) and
      `append-footprints.py` (§ 4.2).
- [x] `gen-parts.py --selftest`: builds a two-row `candidates.tsv` (one NO
      2-terminal, one NC 4-terminal) and a copy of `switch.json` in a temp
      directory, runs the generator twice and prints `idempotent ok` (second run
      byte-identical) and `prefix ok` (the text before the first `tact_`
      device byte-identical to the input). Also `datasheets ok` (the
      `datasheets.json` entry, in id order, and the part's `manufacturer` /
      `datasheet`). Result: `idempotent ok`, `prefix ok`, `datasheets ok`.

Acceptance: the generator is idempotent and leaves the DIP text alone.
  Check: `python3 -I planning/plan-27-data/gen-parts.py --selftest` → `idempotent ok` and `prefix ok` (est. 1 min). Result 2026-11-08: both, plus `datasheets ok`.
Commit: 42e0815

### Phase 2 — C&K parts, footprints, goldens

- [x] `python3 -I planning/plan-27-data/gen-parts.py --maker 'C&K'` (writes
      `Parts/switch.json`), then `append-footprints.py 'C&K'`.
      `26 tactile devices, 488 tactile parts written; 67 datasheets.json
      entries added`; a second run `no changes`; `55 footprints appended
      (C&K)`; `--list-footprints` → 55.
- [x] Docs: § 4.4 (`docs/parts-format.md` § `switch`, the generator paragraph).
- [x] Goldens: § 4.3. `git diff --numstat backend/tests/golden` → `55 0
      footprints.txt`, `488 0 parts.txt`, `488 0 traces.txt`. First lines:
      `ATS2D1G NC LFS|switch|SMD|Switch/Tactile|Normal Close|||positions=1
      actuator=Standard rated=32000 momentary nc|1:A:inout:default,2:B:inout:default|mfr=Littelfuse|datasheet=littelfuse-ck-ats-series-datasheet|footprint=SW-TACT-ATS`.
- [x] Tests: `ctest --test-dir /tmp/logic-p27 -R 'library_check|listing_check|trace_check|footprint_check|switch_test|tactile_test'` after the rebuild → `100% tests passed, 0 tests failed out of 6`.

Acceptance: the library loads with N_ck new parts; goldens grew by exactly N_ck
lines each in `parts.txt` and `traces.txt` and by F_ck lines in `footprints.txt`;
no existing golden line changed.
  Check: `echo $(( $(wc -l < backend/tests/golden/parts.txt) - P_E ))` equals N_ck; `git diff --numstat backend/tests/golden` shows `<added> 0 …` (zero deletions) for each file; `ctest --test-dir /tmp/logic-p27 -R 'library_check|listing_check|trace_check|footprint_check'` → `100% tests passed, 0 tests failed out of 4` (est. 8 min: full rebuild and the four golden sweeps; nothing smaller proves the loader and the goldens agree).
  Result 2026-11-08: 13646 − 13158 = 488 = N_ck (C&K `add` rows); zero deletions; the 6-test run above includes the four.
Commit: 42e0815

## Validation Plan

- Tests: the four golden checks plus `tactile_test` (Phase 2); idempotence (Phase 1).
- Coverage check: `grep -c '"id": "tact_' schematical/schematical/Parts/switch.json` equals D_ck and `grep -c 'Switch/Tactile' backend/tests/golden/parts.txt` equals N_ck.
- Runtime proof: none beyond the goldens (the app proof is K).
- Doc sync: `docs/parts-format.md` § `switch`.
- Final gate (once, after Phase 2): `ctest --test-dir /tmp/logic-p27 -j8` → `100% tests passed, 0 tests failed out of 28` (est. 6 min; 28 after main's merge, plan-27-A Correction 3). Result 2026-11-08: `100% tests passed, 0 tests failed out of 28`; `grep -c 'Switch/Tactile' …parts.txt` → 488; `grep -c '"id": "tact_'` → 26.

## Decisions

- **Device = (circuit, terminals, volts, actuator)** — author: `ratedVolts` and
  `actuator` are device keys (`docs/parts-format.md` § `switch`), so parts with
  different ratings or actuator labels need different devices. Rejected:
  moving them to the part (a loader change for a presentation detail).
- **Rating = DigiKey's voltage** — author: it is the DC switching rating
  (`0.05A @ 12VDC`), uniform across makers. Rejected: re-reading 619 datasheets
  for a number DigiKey already keys.
- **Package names** — author: `SMD`, `THT`, `THT right angle`, `SMD right
  angle`, `Edge mount` (the drawer's package filter reads them; the loader
  forbids pin counts and sizes in them).
- **Generator owns its region of `switch.json`** — author, after
  `scripts/gen-headers.js`; keeps the DIP diff empty.
- **Scope and drawer** — owner, 2026-11-07 (plan-27-A § Decisions).

## Corrections

1. 2026-11-08 — main made `"manufacturer"` required and `"datasheet"`
   expected on every shipped part (plan-27-A Correction 3). § 4.1 now writes
   both and the `datasheets.json` entries; the maker table and the URL rule
   (owner, plan-27-A § Decisions) are in § 4.1.
2. 2026-11-08 — **E needs B and D, not C.** E's parts use only C&K's
   footprints (B); `merge-work.py` compares C's land patterns with
   `Parts/footprints.json` too, so moving C&K's footprints there first does
   not stop C sharing them. E ran while C's readers worked.
3. 2026-11-08 — **ratings in VA.** 42 `add` rows give DigiKey's rating as
   `1VA @ 32VAC/DC`, `3VA @ 28VDC`, `0.4VA @ 28VAC/DC`, and 4 as `25mA @ 16VDC`
   (`awk … $13 | sort | uniq -c`), which § 4.1's `<amps>A @ <V>VDC` did not
   read (`refused: KT11B0JM34LFS: rating '1VA @ 32VAC/DC'`). The generator
   takes the volts after `@` for every form; the device note quotes DigiKey's
   text.
4. 2026-11-08 — **`datasheets.json` wants `https://`.** DigiKey gives
   `//mm.digikey.com/…` (protocol-relative); the loader refused it
   (`datasheets[1077]: … "url" must start with https://`). The generator
   writes `https:` + the URL (and `https://` for `http://`).

## Summary

Risk: a wrong `specs.tsv` row becomes a silent wrong pinout on up to hundreds
of parts; the regex-uniqueness check and the K images are the net. Untouched:
the DIP devices, every other maker's rows, the app.
