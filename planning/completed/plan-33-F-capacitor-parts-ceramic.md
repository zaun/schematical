# plan-33-F: Parts — the generator, GENERIC-CAPACITOR, the ceramic capacitors, goldens

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-33-E

F writes `census/gen-parts.py`, which turns the `add` rows of
`planning/plan-33-data/candidates.tsv` into `Parts/capacitor.json` devices
and parts (merging their datasheets into `Parts/datasheets.json` and the
footprints they use into `Parts/footprints.json`), and runs it for
`GENERIC-CAPACITOR` and every ceramic `add` row. The goldens are
regenerated and only grow.

The single behavioral outcome: `listing_check` lists `GENERIC-CAPACITOR`
plus one `capacitor` line per ceramic `add` row (`N_F`, B § Summary; 390
buckets at most, plan-33-A § 2), every one with a footprint and (except the
generic) a datasheet; `footprint_check` passes (every new footprint used).

References:

- `planning/plan-29-data/census/gen-parts.py` (the diode generator this
  adapts: footprint-id merge, datasheet merge, re-run safety).
- `planning/plan-33-data/fpgroups.tsv`, `footprints-C.json`,
  `footprints-D.json`, `footprints-generic.json` (C and D), `candidates.tsv`
  (B), `datasheets/capacitor/datasheets.tsv` (A).
- `docs/parts-format.md` § Parts, § capacitor (plan-33-E), § The
  `"footprint"` part key (`footprintChoices`); `GENERIC-LED` in
  `Parts/led.json` (the generic-part precedent).
- `docs/backend.md` § Tests (regenerating goldens).

## Prerequisites

plan-33-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-33-E complete | `ls planning/completed/plan-33-E-*.md` → exists | MET (2026-11-11, at execution: d97e95d) |
| The kind loads an empty file | `grep -c '"devices": \[\]' schematical/schematical/Parts/capacitor.json` → 1 | MET (at execution: 1) |
| No other plan wrote capacitor parts | `grep -c '"name"' schematical/schematical/Parts/capacitor.json` → 0 | MET (at execution: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `grep -c '|capacitor|' backend/tests/golden/parts.txt` → `N_F` + 1 (the
  ceramic `add` rows plus the generic).
- `git diff <F's first commit>^ -- backend/tests/golden | grep -c '^-[^-]'`
  → 0: no existing line changed.
- `ctest` passes T0 + 1 = 31 programs.

### Non-goals (explicit constraints)

- No electrolytic or tantalum part (plan-33-G).
- No backend or app code change.
- An existing footprint or datasheet entry is never edited; the generator
  only appends.

## 2. Current State

- After E, `Parts/capacitor.json` has no devices; F's rows come from B's
  `add` decisions (`awk -F'\t' '$2=="ceramic" && $25=="add"'
  planning/plan-33-data/candidates.tsv | wc -l` → `N_F`).
- plan-29's `gen-parts.py` already does the footprint-id merge (identical
  geometry → one footprint; the first of a base id takes the bare id), the
  datasheet merge into `datasheets.json`, and the `reused <id>` report.
  F copies it to `planning/plan-33-data/census/gen-parts.py` and changes
  only what is capacitor-specific (§ 3).

## 3. Design

### 3.1 Devices

`candidates.tsv`'s `type` column names the kind's `type`: `ceramic` →
`ceramic`, `aluminum` → `aluminum`, `al-polymer` → `aluminum-polymer`,
`tantalum` → `tantalum`, `ta-polymer` → `tantalum-polymer`, `niobium` →
`niobium-oxide`; `--type` takes the kind's names.

One device per (type, capacitance): id `<type>-<value>` with the value
written without spaces and `u` for `µ` (`ceramic-100nF`,
`tantalum-4.7uF`), `function` the value text (§ 3.3), `description`
`<Type name> capacitor, <value>` (`Ceramic capacitor, 100 nF`), `drawer`
`["Capacitors", "Ceramic"]` (G: `"Electrolytic"` for `aluminum` and
`aluminum-polymer`, `"Tantalum"` for `tantalum`, `tantalum-polymer`,
`niobium-oxide`), `type`, `farads`, and for polarized types `positive`
`"+"`, `negative` `"-"`.

Pinouts: ceramic `"two-pin"`: `{"number":1,"name":"1","role":"input"}`,
`{"number":2,"name":"2","role":"input"}`; polarized `"polar"`:
`{"number":1,"name":"+","role":"input"}`, `{"number":2,"name":"-","role":"input"}`.

### 3.2 Parts

`{"name": <mpn>, "manufacturer": <maker>, "datasheet": <id>, "package":
<fpgroups.tsv package>, "pinout": "two-pin"|"polar", "footprint": <id>,
"volts": <rated>, "dielectric": <code>, "tolerance": <text>, "flags":
["automotive"]}` — `dielectric` for ceramic only (the DigiKey code before
the first comma: `C0G, NP0` → `C0G`), `flags` only when set. Makers are
written `KEMET`, `Cal-Chip Electronics`, `KYOCERA AVX`.

### 3.3 Value text

pF below 1 nF, nF below 1 µF, µF from 1 µF; at most three significant
digits, no trailing zeros: 1e-11 → `10 pF`, 4.7e-9 → `4.7 nF`, 1e-7 →
`100 nF`, 2.2e-5 → `22 µF`, 4.7e-3 → `4700 µF`. The app's
`CapacitorValue.text` (plan-33-H) uses the same rule.

### 3.4 GENERIC-CAPACITOR

Device `ceramic-generic`: type `ceramic`, `farads` 1e-7, `function`
`Generic`, `drawer` `["Capacitors"]`, `description` `Generic capacitor`,
pinout `two-pin`. Part: `{"name": "GENERIC-CAPACITOR", "manufacturer":
"Generic", "package": "Disc", "pinout": "two-pin", "footprint":
"CAP-DISC-5.08", "footprintChoices": [0402 CAP-0402, 0603 CAP-0603, 0805
CAP-0805, 1206 CAP-1206, Disc CAP-DISC-5.08, Radial CAP-RADIAL-2.54]}` —
no datasheet, no `volts` (no rating check), no tolerance. The six
footprints come from `footprints-generic.json` (plan-33-C § 4.6, D § 4.6).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generator and the generic part

- [x] `planning/plan-33-data/census/gen-parts.py`: copied from plan-29's;
      `--type <t>[,<t>…]` scope, `--generic`, `--dry-run`, `--ids`; the
      reuse of a shipped footprint with identical geometry is limited to
      ids starting with `CAP` (plan-33-C § 4.6);
      devices, pinouts, parts and value text per § 3; footprints from
      `footprints-C.json`, `footprints-D.json`, `footprints-generic.json`.
- [x] `python3 -I gen-parts.py --generic`; regenerate the goldens
      (`docs/backend.md` § Tests) and review the diff: 1 new line in
      `parts.txt` and `traces.txt`, 6 in `footprints.txt`. Observed: `1 parts
      written; 1 devices, 6 new footprints`; `git diff --numstat` → 6 / 1 / 1
      added, 0 removed (26730, 26730, 1218 lines).

Acceptance: the generic part loads with its six choices.
  Check: `cmake --build /tmp/logic-p33 -j8 && ctest --test-dir /tmp/logic-p33 -R 'listing_check|trace_check|footprint_check'` → 3 passed; `grep -c '^GENERIC-CAPACITOR|capacitor|Disc|Capacitors|Generic' backend/tests/golden/parts.txt` → 1 (est. 3 min).
  Observed (build dir `/tmp/logic-p33wt`): `100% tests passed, 0 tests failed out of 3`; `1`.
Commit: cea454b

### Phase 2 — Ceramic capacitors

- [x] `python3 -I gen-parts.py --type ceramic` (it prints `reused <id>`
      for footprints already shipped and the count of parts written).
      Observed: `376 parts written; 43 devices, 45 new footprints` (no
      `reused`: no shipped CAP footprint has a maker drawing's geometry).
- [x] Regenerate the goldens; the diff only adds lines: `N_F` parts and
      traces, and one footprint line per new ceramic footprint (C and D's
      ceramic counts). Observed: `git diff --numstat` → parts.txt 376 / 0,
      traces.txt 376 / 0, footprints.txt 45 / 0.

Acceptance: every ceramic `add` row is a part.
  Check: `ctest --test-dir /tmp/logic-p33` → `100% tests passed … out of 31`; `grep -c '|capacitor|' backend/tests/golden/parts.txt` → `N_F` + 1; `git diff <Phase 1 commit>^ -- backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 5 min).
  Observed: `100% tests passed, 0 tests failed out of 31`; `377` (`N_F` 376 + 1); `git diff cea454b^ -- backend/tests/golden | grep -c '^-[^-]'` → `0`.
Commit: (this commit; hash in the next)

## Validation Plan

- Tests: `listing_check`, `trace_check`, `footprint_check` (goldens),
  `library_check` (the loader takes the file).
- Coverage check: the part-count equality of Phase 2.
- Runtime proof: plan-33-H's harness on these parts.
- Doc sync: none (E documented the kind).
- Final gate: plan-33-I.

## Decisions

- **One device per (type, value)** — author: the drawer leaf is the device's
  `function` (plan-33-E § 3 Rejected); every part of a leaf shares it.
- **Generic footprint choices 0402 / 0603 / 0805 / 1206 / Disc / Radial,
  Disc default** — owner (Ceramic Disc default; the six were in the question
  the owner answered, plan-33-A § Decisions).

## Corrections

1. **`N_F` is 376**, not B's first 377: `CK12BX101K` (a military part by its
   series name) left the ceramic picks (plan-33-B § Corrections 8).
2. **Footprints come from fpgroups.tsv by group key**, the key including
   the maker's case code (plan-33-D § Corrections 1); gen-parts.py imports
   `footprint-lines.py`'s `maker_case` so both agree.
3. **`volts` is DigiKey's `Voltage - Rated` number**, which the scheme check
   matched against the datasheet's voltage code (plan-33-B); for the AC-rated
   safety discs it is the X1 VAC rating (plan-33-B § Corrections 6).
4. **One datasheet entry per PDF**: two URLs that are one file (place.py)
   share the id of the first URL, so `datasheets.json` gains one line per
   kept PDF the scope cites.
5. **Build directory** `/tmp/logic-p33wt` (plan-33-A § Corrections 1).

## Summary

F is mechanical once B–D are done; the generator is plan-29's with a
different device key. The risk is a footprint id clash with an existing id,
which the generator's merge rule (reuse identical geometry, tag the rest)
already handles.
