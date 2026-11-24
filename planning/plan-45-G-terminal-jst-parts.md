# plan-45-G: Parts — the generics, every `add` row, the datasheets, the goldens

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-45-F

G adds the parts: the 88 generics (`GENERIC-TERMINAL-<pitch>-<NN>`, 66;
`GENERIC-JST-XH-<NN>` and `GENERIC-JST-PH-<NN>`, 22) and one `connector` part
per `add` row of `candidates.tsv` (521 at planning time before B's skips), the
footprints they name (written by `scripts/gen-terminals.js --write`), the
datasheet entries, the regenerated goldens and the real-library tests. The
behavioral outcome: the library loads with P0 + 88 + (the `add` rows) parts,
every new part has a manufacturer, a datasheet (the generics none) and a
footprint, `ctest` passes, and the goldens only gain lines.

References:

- plan-45-B § 4 (the final pick, the generic representatives), C § 3 (ids),
  F § 3 (the keys); `docs/parts-format.md` § Parts, § Datasheets, § Footprints,
  § `connector`; `docs/backend.md` § Tests (golden regeneration).
- `scripts/gen-headers.js` (the idempotent generator precedent) and
  `planning/completed/plan-40-G-transistor-parts.md` (`census/gen-parts.py`,
  the phases per maker).

## Prerequisites

plan-45-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-45-F complete | `ls planning/completed/plan-45-F-*.md` → exists | MET (2026-11-24: not archived, every F box ticked: `grep -c '\[ \]' planning/plan-45-F-connector-kind.md` → 0; F's commit c1afdc4a) |
| The pick is final | `awk -F'\t' '$22=="add"' planning/plan-45-data/candidates.tsv \| wc -l` → the `add` rows (521 at planning time, `candidate`) | MET (2026-11-24: **501** `add` (Phoenix Contact 68, Weidmüller 149, Würth Elektronik 93, JST 191), 12 `skip` (6, 4, 2: B and D Corrections), 0 `candidate`) |
| The footprint tables are done | `python3 -I planning/plan-45-data/census/fp-check.py C && python3 -I planning/plan-45-data/census/fp-check.py D && python3 -I planning/plan-45-data/census/fp-check.py E` → each `0 without a footprint` | MET (2026-11-24: `19 lines, 0 without`, `24 lines, 0 without`, `21 lines, 0 without`) |
| The goldens are P0 / R0 / F0 | `wc -l < backend/tests/golden/parts.txt` etc. re-measured at the start; G uses its own | MET (2026-11-24 at c1afdc4a: **35921 / 35921 / 2293**; after Phase 1 36009 / 36009 / 2381) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- 88 generic parts and one part per `add` row are in `Parts/connector.json`.
- `Parts/footprints.json` gains every `TB-` / `JST-` footprint those parts
  name and no other (`footprint_check` passes: none unused, none missing).
- `Parts/datasheets.json` gains one entry per distinct maker URL of the `add`
  rows (340 at planning time); the PDFs are in the tree (A).
- `backend/tests/golden/{parts,traces,footprints}.txt` only gain lines.
- `connector_test` shows a placed `GENERIC-TERMINAL-5.08-03` and
  `GENERIC-JST-XH-04` joining nothing.

### Non-goals (explicit constraints)

- No backend change (F did it); no app change (H); no change to the 224
  existing connector devices (their text stays byte for byte).
- No part outside the `add` rows and the generics.

## 2. Current State

`connector.json` is 2894 lines: 7 barrel and 217 USB devices, each a block of
lines under `"devices": [` indented four spaces (`    {` … `    },`), pinouts
and parts on single lines. `datasheets.json` is sorted by id, one entry a
line (7993). `footprints.json` has 1646 footprints one per line. The loader
checks every device (F § 3 for the new keys) and `listing_check`,
`trace_check`, `footprint_check` compare against the goldens.

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts to add | 88 generics (6 pitches × 11 position counts + 2 series × 11) + the `add` rows (521 `candidate` at planning time) | `awk -F'\t' '$22=="add"' planning/plan-45-data/candidates.tsv \| wc -l` |
| Devices the `add` rows make: one per (variant, rated voltage, rated current) | 71 at planning time (`candidate` rows; amps / volts as DigiKey gives them) | `python3 -I -c "import csv,collections;r=[x for x in csv.DictReader(open('planning/plan-45-data/candidates.tsv'),delimiter='\t') if x['state']=='candidate'];print(len({(x['variant'],x['amps'],x['volts']) for x in r}))"` → 71 |
| Datasheet entries to add | 340 at planning time | A § 2 |
| Footprints to add | 450 at planning time (variant × position count) plus B's extras for the generic representatives | A § 2 |
| Existing datasheet ids that clash with the candidates' (`mpn` lower-case, `exh`, `eph`, `eeh`, `ezh`, `esh`, `egh`, `evh`, `esr_sz`) | 0 | `python3 -I -c` over `datasheets.json` and `candidates.tsv`, 2026-11-16 |
| Backend goldens | P0 32897, R0 32897, F0 1646 | A § Prerequisites |

### Verified properties

- `datasheets.json`'s `url` must be the maker's own site and the same
  document as the local PDF (`docs/parts-format.md` § Datasheets); a Phoenix
  Contact entry names `https://www.phoenixcontact.com/us/products/<item>/pdf`,
  which answers 403 to automated fetches (A § 2), like Suntsu's in plan-44.
- `jst.com` answers 403 to Node's `fetch` (2026-11-16 probe: a Node `fetch` of
  `https://www.jst.com/wp-content/uploads/2025/06/eSR_SZ.pdf` → 403, the same
  URL through Python `urllib` with a Safari User-Agent → 200), so
  `scripts/check-datasheets.js` will report the two `jst.com` entries.

## 3. Design

### 3.1 The generator

`planning/plan-45-data/census/gen-parts.py` (idempotent; `--check` exits 1 when
`connector.json` or `datasheets.json` differ from what it would write): splits
`connector.json` into its device blocks by their four-space braces, drops the
blocks whose `id` starts with `tb-`, `jst-` or `generic-`, keeps every other
block byte for byte, and appends the generated blocks; inserts the generated
`datasheets.json` lines in id order and removes none; writes the generics.
Then `node scripts/gen-terminals.js --write` writes the footprints named by
the connector parts' `footprint` ids (the ids that start `TB-` or `JST-`).

### 3.2 Devices, parts, names

| Field | Rule |
|---|---|
| device `id` | `tb-<label lower>-<amps>a-<volts>v` for a terminal variant (`tb-pho-combicon-pt-5.00-ht-17.5a-400v`; `nr` for a missing rating), `jst-<label lower>-<amps>a-<volts>v` for JST; `generic-terminal-<pitch>` (`generic-terminal-5.08`), `generic-jst-xh`, `generic-jst-ph` |
| `drawer` | terminals `["Connector", "Terminal block", "<pitch> mm"]` (`"5.08 mm"`); JST `["Connector", "JST", "<series>"]` (`"SH-SR"` for the SH/SR series) |
| `function` | terminals `<Clamp>, <orientation>`: `Screw`, `Spring`, `Push-in` or `Plug-in header`; `vertical`, `horizontal` or `angled`; JST `Top entry` (vertical) or `Side entry` (horizontal); generics `Generic` |
| `terminal` / `jst` / `pitch` | F § 1: `screw`, `spring`, `push-in`, `pluggable` (the class); the series (`SH` for SH/SR); the pitch in mm |
| `ratedVolts`, `ratedAmps` | B's `volts`, `amps` columns; omitted when empty; omitted on generics |
| `note` | `<maker> <series>, <datasheet file>` |
| pinouts | `p02` … `p12`, only those the device's parts use: `n` pins numbered 1…n, name `"n"`, role `inout` |
| part `name` | the DigiKey number (B § 3); generics `GENERIC-TERMINAL-<pitch>-<NN>` (`GENERIC-TERMINAL-3.50-02` … `-7.62-12`), `GENERIC-JST-XH-<NN>`, `GENERIC-JST-PH-<NN>` |
| part `manufacturer` | `Phoenix Contact`, `Weidmüller`, `Würth Elektronik`, `JST`; generics `Generic` |
| part `datasheet` | the id of the part's PDF (the placed file's name lower-case, `.pdf` dropped: `1935161`, `691137710002`, `exh`); generics none |
| part `package` | `Through hole vertical`, `Through hole right angle` (horizontal), `Through hole angled`, `SMD vertical`, `SMD right angle`; generics: the representative's |
| part `footprint` | `TB-<label>-<NN>` or `<label>-<NN>` (C § 3.1); generics: the representative variant's footprint of the same position count (B's `generic-reps.tsv`) |
| `datasheets.json` entry | `{"id", "url"}`: the part's maker URL (`url` of `candidates.tsv`) |

A generic JST header's `function` is `Generic` under `["Connector", "JST",
"XH"]`; its `package` is `Through hole vertical`.

### 3.3 Goldens and risk

Three goldens grow by the new parts and footprints; the expected diff is
added lines only. The risk is a clash or a wrong footprint on a part; both
surface as a load error or a `footprint_check` failure.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generics

- [x] `census/gen-parts.py` (§ 3.1, § 3.2): the generics only first; run it,
      then `node scripts/gen-terminals.js --write`.
      `gen-parts.py --generics-only` → `8 devices, 88 parts, 0 datasheet entries` (`git diff` of `connector.json`: 184 insertions, 0 deletions: every existing block byte for byte); `gen-terminals.js --write` → `88 TB-/JST- footprints written` (the 88 the generics name); `--check` modes of both → exit 0.
- [x] `connector_test.c`: `GENERIC-TERMINAL-5.08-03` pins 1 and 3 on two nets
      driven from `5V` and `GND`, and the same for `GENERIC-JST-XH-04`: after
      `circuit_start` and `circuit_advance(circuit, 1)` neither net is
      shorted.
      The card-slot helper became `part_joins_nothing(…, what)`; both parts also checked for kind, `terminal` / `jst`, `pitch_um` and a footprint with as many pads as pins: `PASS: GENERIC-TERMINAL-5.08-03 (a generic terminal block) joins nothing`, `PASS: GENERIC-JST-XH-04 (a generic JST header) joins nothing`.
- [x] Regenerate the goldens (`docs/backend.md` § Tests) and review: only added
      lines.
      `parts.txt` and `traces.txt` 35921 → 36009, `footprints.txt` 2293 → 2381: `264 insertions(+)`, 0 deletions.
- [x] Tests: `library_check`, `listing_check`, `trace_check`, `footprint_check`,
      `connector_test`.

Acceptance: 88 generics load with footprints; goldens only gained.
  Check: `cmake --build /tmp/logic-p45 -j8 && ctest --test-dir /tmp/logic-p45 -R 'connector_test|library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff backend/tests/golden \| grep -c '^-[^-]'` → 0; `grep -c 'GENERIC-TERMINAL\|GENERIC-JST' backend/tests/golden/parts.txt` → 88 (est. 15 min). Result: `100% tests passed, 0 tests failed out of 5`; `0`; `88`.
Commit: b9a48fd2

### Phase 2 — Phoenix Contact and Weidmüller

- [x] `gen-parts.py --maker phoenix --maker weidmuller`: their `add` rows'
      devices, `datasheets.json` entries, then `gen-terminals.js --write`.
      `50 devices, 305 parts, 217 datasheet entries` (the 88 generics + 68 Phoenix + 149 Weidmüller parts, 217 datasheet files: one per part); `249 TB-/JST- footprints written` (88 generics' + 161).
- [x] Regenerate the goldens; review: only added lines.
      Against Phase 1: `parts.txt` +217, `traces.txt` +217, `footprints.txt` +161: `595 insertions(+)`, 0 deletions.

Acceptance: the two makers' parts load.
  Check: `cmake --build /tmp/logic-p45 -j8 && ctest --test-dir /tmp/logic-p45 -R 'library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff backend/tests/golden \| grep -c '^-[^-]'` → 0 (est. 15 min). Result: `100% tests passed, 0 tests failed out of 4`; `0`.
Commit: d4d8c847

### Phase 3 — Würth Elektronik and JST

- [x] `gen-parts.py` (all makers), `gen-terminals.js --write`; goldens;
      `node scripts/gen-terminals.js --check` and `python3 -I
      planning/plan-45-data/census/gen-parts.py --check` → both exit 0.
      `85 devices, 589 parts, 308 datasheet entries`; `480 TB-/JST- footprints written` (C 135 + D 152 + E 193: every footprint the parts name, all of them); goldens against Phase 2: `parts.txt` +284, `traces.txt` +284, `footprints.txt` +231, 0 deletions; `gen-terminals.js --check` → `ok: 480 TB-/JST- footprints`, `gen-parts.py --check` → exit 0.
- [x] Tests: the same targets as Phase 2.
      The whole suite: `100% tests passed, 0 tests failed out of 57`.

Acceptance: every `add` row is a part.
  Check: `grep -c '|connector|' backend/tests/golden/parts.txt` → 376 + 88 + the `add` rows; `awk -F'\t' '$22=="add"' planning/plan-45-data/candidates.tsv \| wc -l` gives the last term (est. 15 min). Result: `1541` = 952 + 88 + 501 (the connector kind held 952 parts, not 376: Corrections).
Commit: 96f98f8c

### Phase 4 — Datasheets and docs

- [x] `datasheets/connector-terminal/` and `datasheets/connector-jst/` hold
      exactly the PDFs the `datasheets.json` entries need (`place.py --verify`).
      `ok: 308 files, 0 rows without a file, 0 unused, 0 changed`.
- [x] `node scripts/check-datasheets.js --host www.phoenixcontact.com`,
      `--host datasheet.weidmueller.com`, `--host www.we-online.com`,
      `--host www.jst-mfg.com`, `--host www.jst.com`: record each host's
      result in `docs/parts-format.md` § Datasheets next to the Suntsu note
      (the hosts that refuse the script's client, with their HTTP status).
      phoenixcontact 68 of 68, weidmueller 149 of 149, we-online 219 of 219 (run alone), jst-mfg 7 of 7 serve a PDF; `www.jst.com` 0 of 2: `HTTP 403` for `eSR_SZ` and `eSR_SZ-new` (recorded in § Datasheets).
- [x] `docs/parts-format.md` § Footprints: the `TB-`/`JST-` paragraph names
      `gen-terminals.js --write`; § `connector` lists the 88 generics.
- [x] `docs/file-format.md` (the `reference` row): the `J` prefix reads
      "USB/barrel connectors, terminal blocks, JST headers and female headers".

Acceptance: the datasheet and doc state matches the tree.
  Check: `python3 -I planning/plan-45-data/census/place.py --verify` → `ok`; `node scripts/check-datasheets.js --host www.jst-mfg.com` → the result the doc records (est. 20 min). Result: `ok: 308 files ...`; `7 of 7 URLs serve a PDF; 0 failed.`
Commit: this commit (hash in the next one)

## Validation Plan

- Tests: the five backend targets above after each phase.
- Coverage check: the new parts are in `listing_check`'s and `trace_check`'s
  goldens (lines added), and every new footprint in `footprints.txt`.
- Runtime proof: `connector_test`'s generic parts join nothing.
- Doc sync: `docs/parts-format.md`, `docs/file-format.md`.
- Final gate: none; the landing tier is I's.

## Decisions

- Author: **One generator for devices, one for footprints**, both idempotent,
  so a B correction re-runs instead of editing 600 lines by hand.
- Author: **Footprints are written from the parts**, not from the tables:
  `footprint_check` forbids an unused footprint, so only the footprints the
  parts name exist.
- Author: **Device per (variant, rating)**: ratings are a device key, so rows of
  one variant with different ratings are different devices (71 at planning).
- Author: **The generic's footprint is its representative's** (B), so there
  is one land pattern per pitch and no duplicate objects.

## Corrections

- The connector kind held 952 parts before G, not 376 (the plan's figure was
  written before other plans' connector parts landed): the Phase 3 check is
  `1541 = 952 + 88 + 501`, measured with `grep -c '|connector|'`.
- 589 parts = 88 generics + 501 `add` rows (the 12 `skip` rows are in
  `datasheets/skipped-parts.md`).
- `datasheets.json` entries are one per placed file: 308, not the 340 of
  planning time (several PDFs are shared by one URL; `place.py --verify` is the
  proof).
- 480 footprints (C 135 + D 152 + E 193): `--write` emits only those the parts
  name, and all of the generated ones are named.
- Phase 1's goldens were regenerated against the previous library, so Phase 3's
  diff (+284 / +284 / +231 lines, 0 removed) is against Phase 2.
- `docs/parts-format.md` drawer text now shows the two-decimal pitch strings the
  generator writes (`5.00 mm`), not `5.0 mm`.
- `www.jst.com` answers a script `HTTP 403` (both entries); the other four hosts
  serve every PDF. Recorded in § Datasheets as expected.

## Summary

G is bulk and mechanical: two generators, three golden regenerations, 340
datasheet lines. The risk is a part naming a wrong footprint; the loader and
`footprint_check` catch a missing one, and C, D and E's hand-checks cover a
wrong one.
