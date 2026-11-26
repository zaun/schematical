# plan-69-F: Parts — Zener-type TVS diodes (Littelfuse, Vishay, YAGEO); datasheets, footprints, goldens

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-69-E

F writes `Parts/tvs.json` with every Zener-type `add` row (single TVS diodes and
the multi-channel Zener-type arrays of Littelfuse, Vishay and YAGEO), their
`datasheets.json` entries and the footprints of C and D they name, and regenerates
the goldens.

References:

- plan-69-A § 1 (the pick), plan-69-B § 3 (`specs.tsv`'s `pinout`, `elements`,
  `facts`, `page`), plan-69-C/D (`footprint-map.tsv`, `footprints-C.json`,
  `footprints-D.json`), plan-69-E § 3.1 (the kind's keys).
- `planning/completed/plan-48-G-timer-opamp-parts.md` (the precedent:
  `gen-parts.py`, `fp-merge.py --only`, goldens, `check-datasheets.js`);
  `planning/plan-48-data/census/gen-parts.py` (the script F copies as
  `census/tvs-gen-parts.py`).
- `docs/parts-format.md` § Parts (flags, `package`), § Datasheets, § `tvs`.

## Prerequisites

plan-69-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-69-E complete | `ls planning/completed/plan-69-E-*.md` → exists, or every E box ticked | NOT MET (2026-11-25: E not started) |
| Every `add` row has a footprint and a pinout | `python3 -I planning/plan-69-data/census/tvs-fp-check.py C` and `… D` → `0 without a footprint`; `python3 -I planning/plan-69-data/census/tvs-pinout-check.py` → `0 errors` | NOT MET (2026-11-25: B–D not started) |
| Golden sizes at F's start | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt backend/tests/golden/erc_pins.txt` | recorded here at F's start as **P1**, **R1**, **F1**, **E1** (P0 = R0 = E0 = 40832, F0 = 3993 at planning) |
| The part file is absent | `ls schematical/schematical/Parts/tvs.json` → no such file | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/tvs.json` holds one part per Zener-type `add` row (at most 600).
- `parts.txt`, `traces.txt` and `erc_pins.txt` grow by exactly those parts;
  `footprints.txt` by exactly the objects of `footprints-C.json` / `-D.json` the
  parts name; no golden line is removed or changed.
- `Parts/datasheets.json` has an entry for every PDF a Zener-type `add` row cites.

### Non-goals (explicit constraints)

- No change to an existing part, footprint or datasheet entry.
- No hand-written part: every part comes from `tvs-gen-parts.py`; a wrong part is
  fixed in `specs.tsv`, `footprint-map.tsv` or the script.
- No steering array (G); no editor change (G); no app change (H).

## 2. Current State

`Parts/` has no `tvs.json`. `docs/parts-format.md:255` lists `automotive` among a
part's `"flags"` (crystals and fuses use it for AEC-Q200 parts: `:1472`, `:1539`).
The drawer's `Discrete` category holds Capacitors, Diodes, Fuses, Inductors,
Resettable fuses, Resistors and Transistors (`python3 -I -c` over every
`Parts/*.json` device's `drawer[:2]` where `drawer[0] == "Discrete"`, 2026-11-25).

## 3. Design

### 3.1 Devices and parts (`census/tvs-gen-parts.py`)

- **One device per spec group**: (company, PDF, `elements`, standoff band). Its
  `pinouts` are the group's spec rows (one per package), keyed `<label lower
  case>-<pins>` (`smb-2`, `sod-123fl-2`, `sot-23-3`), with `-b`, `-c` when two
  packages of one label and pin count differ in pinout.
- **Ids**: `<datasheet id>-<uni|bi>-<n>` (`smbj-uni-1`, `sp0504s-uni-4`), with the
  band appended (`-lt5`, `-5`, `-12`, `-30`, `-100`) when one PDF spans several.
- **Standoff bands** (each part's `vrwm`): `< 5 V`, `5 V – 12 V` (5 ≤ v < 12),
  `12 V – 30 V`, `30 V – 100 V`, `≥ 100 V`.
- **Drawer and function**: one diode, unidirectional:
  `["Discrete", "TVS diodes", "Unidirectional"]`; one diode, bidirectional:
  `["Discrete", "TVS diodes", "Bidirectional"]`; the function is the band.
  Several diodes: `["Discrete", "TVS diodes", "ESD arrays"]`, function `"1–2
  lines"`, `"3–4 lines"`, `"5–8 lines"` or `"9+ lines"` by diode count.
  `description`: `"<series> <600 W> TVS diode, unidirectional"` (the `ppk` when
  every part of the device shares it, else without it) / `"<series> <n>-line ESD
  protection array"`.
- **Device keys**: `type` `diode`; `diodes` from `elements` (`K>A` → `{"anode":
  "A", "cathode": "K"}`; `1~2` → `{"terminals": ["1", "2"]}`).
- **Parts**: `name`, `manufacturer` (`Littelfuse`, `Vishay`, `YAGEO`), `datasheet`
  (the id), `package` (the spec row's label: `SMA`, `SMB`, `SMC`, `SOD-123FL`,
  `DO-15`, `P600`, `0402`, …), `pinout`, `footprint` (`footprint-map.tsv`), the
  facts `vrwm`, `vbr`, `vc`, `ipp`, `ippWave`, `ppk`, `cj` from the row's checked
  `facts` (B § 3.2), `"flags": ["automotive"]` for an AEC-Q101 / AEC-Q100 row.
  The device `note` names the PDF and B's `page`.
- **Order**: devices by company, then datasheet id; parts by name.

### 3.2 Datasheets and footprints

- `datasheets.json`: one entry per PDF an `add` row cites, `id` = the placed file's
  name without `.pdf`, `url` = its `datasheets.tsv` URL; an id that exists is not
  added again (`python3 -I -c` over `datasheets.json` ids before the run → the
  overlap goes in § Corrections).
- `footprints.json`: `tvs-fp-merge.py --only <ids>` appends the objects the
  phase's parts name; an id already in `footprints.json` (another plan added it) is
  skipped and recorded in § Corrections.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generator

- [ ] `census/tvs-gen-parts.py` per § 3.1–3.2 (from plan-48's `gen-parts.py`),
      with `--type zener|steering` and `--verify`.

Acceptance: a dry run writes the Zener-type devices with no load error.
  Check: `python3 -I planning/plan-69-data/census/tvs-gen-parts.py --type zener --dry-run` → `<d> devices, <n> parts` with `<n>` the Zener-type `add` count (`awk -F'\t' 'NR>1 && $2=="zener" && $24=="add"' planning/plan-69-data/candidates.tsv | wc -l`) (est. < 1 min).
Commit:

### Phase 2 — Littelfuse, Vishay, YAGEO

- [ ] `tvs-gen-parts.py --type zener`: `Parts/tvs.json`, the `datasheets.json`
      entries, `tvs-fp-merge.py --only` the footprints the parts name.
- [ ] Regenerate the goldens (`listing_check --print > backend/tests/golden/parts.txt`,
      `trace_check --print > …/traces.txt`, `footprint_check --print >
      …/footprints.txt`, `erc_pins_check --print > …/erc_pins.txt`) and review
      `git diff --stat backend/tests/golden`.
- [ ] `node scripts/check-datasheets.js --jobs 4 <the new ids>` → no failure.

Acceptance: the Zener-type pick loads and the goldens grow by exactly it.
  Check: `cmake -S backend -B /tmp/logic-p69-f && cmake --build /tmp/logic-p69-f -j8 && ctest --test-dir /tmp/logic-p69-f -j8` → `100% tests passed`; `grep -c '|tvs|' backend/tests/golden/parts.txt` → the Zener-type `add` count; `wc -l < backend/tests/golden/footprints.txt` → F1 + the objects merged; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 8 min: a build from scratch and the four goldens; nothing smaller loads every new part).
Commit:

## Validation Plan

- Tests: `listing_check`, `trace_check`, `footprint_check`, `erc_pins_check` with
  the regenerated goldens; `tvs_test`.
- Coverage check: `python3 -I planning/plan-69-data/census/tvs-gen-parts.py --verify` → `zener: <n> add rows, <n> parts`.
- Doc sync: the `### tvs` section's library paragraph (counts, drawer paths) is
  written in G, once both types are in.
- Final gate: plan-69-I.

## Decisions

- Author: **drawer Discrete → TVS diodes → Unidirectional / Bidirectional / ESD
  arrays**, beside Diodes and Fuses; the leaf is the standoff band, which is what
  a designer picks a TVS by.
- Author: **every orderable packing is its own part** (plan-48-A § 2).
- Author: **no generic TVS part**: the pick holds every common package and
  standoff, and a generic would carry no real rating for the standoff check.

## Corrections

## Summary

Generated files only. The check is arithmetic: the goldens grow by the Zener-type
`add` rows, and nothing else changes.
