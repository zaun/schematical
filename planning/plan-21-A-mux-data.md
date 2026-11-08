# plan-21-A: Multiplexers — datasheets, part data, families, footprints

Last updated: 2026-11-07
Overall Effort: x-large (1d–3d) — plan-21 is A → B → C → D → E
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-21 adds every Texas Instruments digital multiplexer / data selector
DigiKey has in stock — plain select muxes, muxes with input or output
storage, the '670/'870 register files and the ALS/AS specials ('250,
'850/'851, '857) and the ABT 4-to-1 multiplexed bus transceivers
('16460/'162460) — as a new part kind `mux`. Each part simulates its
datasheet function table, names a footprint, carries its supply range and
shows in the Logic drawer.

The single behavioral outcome of plan-21: loading the part files gives the
baseline count + N21 parts (N21 = `add` rows of
`planning/plan-21-data/parts.tsv`), every new device runs its datasheet
behavior in `mux_test`, and `ctest` passes T0 + 1 programs.

| Letter | Content | Effort |
|---|---|---|
| **A** | download the datasheets; `plan-21-data/parts.tsv` and `specs.md`; skipped parts; family `FCT`; footprints | large |
| B | `mux` kind; model `select` (583 candidates); `mux_test` | large |
| C | models `stored` ('298/'399/'354/'355/'356/LS604), `regfile` ('670/'870), `universal` ('857), `bus4` ('16460/'162460) | large |
| D | the parts: `mux.json`; goldens | medium |
| E | app: the `mux` kind in the drawer and sidebar; plan-21 landing tier | medium |

Outcome of A: `datasheets/mux/` holds every TI PDF the candidates cite;
`parts.tsv` has one row per candidate with `add`/`skip`; `specs.md` one
section per device; `FCT` and the new footprints load; goldens change only by
added footprint lines.

References:

- `AGENTS.md`; `docs/parts-format.md` (§ Values, § Supply voltage,
  § Footprints, § Sequential kinds); `docs/backend.md`.
- `planning/plan-21-data/candidates.tsv` (the census) and
  `planning/plan-21-data/census/` (the scripts that produced it).
- `planning/completed/plan-10-A-groundwork.md` §3.4–§3.5,
  `plan-10-J-app.md`; inverter precedent commits 7022ae9, 0655d09.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p21 && cmake --build /tmp/logic-p21 -j8 && ctest --test-dir /tmp/logic-p21` → `100% tests passed` | MET at start of execution (1b780ae, worktree-P-21): `100% tests passed, 0 tests failed out of 22` — **T0 = 22** (plan-19 added `decoder_test`) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `footprints.txt` | MET at 1b780ae: **P0 = 8344**, **F0 = 239** |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p21-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET at 1b780ae: `** BUILD SUCCEEDED **` |
| No `mux` kind yet | `ls schematical/schematical/Parts/mux.json backend/src/models/mux*.c 2>/dev/null` → no output | MET at 1b780ae: no output |
| DigiKey script works | `node scripts/digikey.js --in-stock --manufacturer 296 --limit 1 SN74HC157N` → a part line | MET at 1b780ae: `SN74HC157N Active 11876 74HC IC MULTIPLEXER 4 X 2:1 16-PDIP` (7 calls left today). NOT MET 2026-11-07 21:13–23:12 UTC (`429 Daily Ratelimit exceeded`, `0 left today`, reset 2026-11-08 00:00 UTC); **MET again 2026-11-08 00:01 UTC** (`SN74HC251N`, `771 left today` after the re-check) |

plan-21 waits for no other plan (owner: plans 19–22 run in any order,
possibly in parallel). Everything below assumes these rows hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all rows.

## 1. Goal

- `ls datasheets/mux/*.pdf | wc -l` = the distinct `pdf` values of `parts.tsv`.
- `tail -n +2 planning/plan-21-data/parts.tsv | wc -l` → 634, each `add` or `skip` with a reason.
- `FCT` accepted; new footprints load; `ctest` T0 of T0; goldens add only footprint lines.

### Non-goals

- No existing listing line or trace changes.
- Out of scope (owner): analog and FET switches (CBT, CBTLV, CBTU, CBTK,
  CB3T, TS5/TS3, TMUX, CD4051–CD4067, 4066, PCA/TCA I2C muxes) — they are not
  in `candidates.tsv`; the '261 and LS384 multipliers (plan-22); decoders
  (plan-19).

## 2. Current State

- Census: `planning/plan-21-data/candidates.tsv` — TI in-stock parts
  (`--in-stock --manufacturer 296`, categories 743 and 703, 2026-11-07) in
  plan-21's functions; columns as plan-19-A §2.
- `SN74LVC1G157`/`1G158`: none in stock (`node scripts/digikey.js --in-stock --manufacturer 296 SN74LVC1G157` → `0 of 0 parts`, 2026-11-07), so not candidates.
- Families: `kind_util.c:435-438`, 25 codes; footprints: 237 ids.

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidate parts | 634 | `tail -n +2 planning/plan-21-data/candidates.tsv \| wc -l` |
| Functions | 34 | `tail -n +2 … \| cut -f1 \| sort -u \| wc -l` |
| … DigiKey obsolete | 156 | `awk -F'\t' '$6=="Obsolete"' … \| wc -l` |
| … addable by status (Active or Last Time Buy) | 455 (179 excluded: Obsolete, Discontinued at DigiKey, Not For New Designs) | `awk -F'\t' 'NR>1 && ($6=="Active"||$6=="Last Time Buy")' planning/plan-21-data/candidates.tsv \| wc -l` |
| Per function | 157: 137, 257: 97, 153: 67, 151: 56, 158: 44, 251: 43, 253: 41, 258: 33, 670: 14, 2G157: 12, 11257: 11, 298: 10, 399: 8, 4512: 8, 2257: 6, 353: 5, 4025 (CD40257): 5, 352: 4, 354: 4, 162460: 3, 250: 3, 857: 3, 870: 3, 11151: 2, 11157: 2, 11251: 2, 16460: 2, 355: 2, 850: 2, 1T157: 1, 1T158: 1, 356: 1, 604: 1, 851: 1 | `tail -n +2 … \| cut -f1 \| sort \| uniq -c` |
| … `select` model (B) | 583 | `tail -n +2 … \| cut -f1 \| grep -cxE '151\|153\|157\|158\|251\|253\|257\|258\|352\|353\|2G157\|1T157\|1T158\|11151\|11157\|11251\|11257\|4512\|4025\|2257\|250\|850\|851'` → 583 |
| … `stored` (C) | 26 | functions 298, 354, 355, 356, 399, 604 |
| … `regfile` (C) | 17 | 670 (14) + 870 (3) |
| … `universal` (C) | 3 | 857 |
| … `bus4` (C) | 5 | 16460 (2) + 162460 (3) |
| Families | HC 110, LS 65, LVC 55, ALS 53, ACT 45, AS 45, F 40, HCT 40, AC 37, FCT 31, AHC 28, HCS 18, AHCT 16, S 16, CD4 (→ `C`) 13, LV 8, ABTH (→ `ABT`) 4, AUP 2, ABT 1; blank (standard TTL, `TTL`) 4; unparsed names 3 | `tail -n +2 … \| cut -f2 \| sort \| uniq -c` |
| No DigiKey package (ceramic/military; from the TI code) | 97 | `awk -F'\t' 'NR>1 && $8==""' … \| wc -l` |
| Packages with no existing footprint | 24-PDIP (3), SM8 (2) | `census/fp.py` |

### Verified properties

| Claim | How verified |
|---|---|
| No candidate is already in the catalog | names ∩ every `name` in `Parts/*.json` → 0 (2026-11-07) |
| The DigiKey "20-LCCC (8.89x8.89)" and "8-DSBGA (1.9x0.9)" packages are the existing `LCCC-20` and `DSBGA-8-YZP` | footprint dimensions in `footprints.json` match (8.89 mm body; YZP 1.9 × 0.9 mm) |

## 3. Design Overview (all of plan-21)

### 3.1 Models

| Model | Functions | Sub-plan |
|---|---|---|
| `select` | 151, 153, 157, 158, 251, 253, 257, 258, 352, 353, 2G157, 1T157, 1T158, 11151, 11157, 11251, 11257, 4512, 40257, 2257, 250, 850, 851 | B |
| `stored` | 298, 399 (register after the select), 354, 355 (transparent data + select latches), 356 (edge-triggered data register + select latch), LS604 (two 8-bit input latches, select, 3-state) | C |
| `regfile` | 670 (4 × 4), 870 (dual 16 × 4) | C |
| `universal` | 857 (hex 2-to-1 with true/complement/zero/one control) | C |
| `bus4` | 16460, 162460 (4-to-1 multiplexed/demultiplexed registered bus transceivers) | C |

One kind `mux`, one `SimComponentType`, a `"model"` device key (plan-10
precedent). Models reuse `models/seq.h`.

Rejected alternatives:

- **Muxes in `gate`**: no select decode, no strobe, no 3-state.
- **Storage muxes in `latch`/`shift`**: their storage sits in front of or
  behind the selector; splitting one chip across two kinds is not possible
  (one part = one kind).
- **The register files in `sram`**: `sram` is a single-port array with one
  address bus; the '670 has independent read and write addresses and
  enables.

### 3.2 Data conventions

plan-10-A §3.4 unchanged, with the plan-19-A §3.3 additions (inclusion rule,
automotive-named CAHCT/CLVC parts into their `74_157`/`74_257` devices,
CY74FCT parts with family `FCT`, military numbers to their SN54 device).
Drawer `function`: `"Multiplexer"` for `select`, `stored`, `universal`,
`bus4`; `"Register file"` for `regfile`.

### 3.3 Signal rules

plan-10-A §3.5. For selects: an X select bit makes the output X unless every
data input it chooses between has the same known value.

### 3.4 Risk

`select` carries 583 of 634 candidates; its strobe/3-state/complement
combinations are tested row by row in B before any data lands. `bus4` is the
only bidirectional model (both ports read and drive); C tests both
directions and the turnaround.

## 4. Detailed Design (this sub-plan)

- Datasheets: `datasheets/mux/`, one PDF per distinct TI datasheet,
  `https://www.ti.com/lit/ds/symlink/<lowercase base>.pdf` via `curl -sfL -o`
  (plan-14-A precedent); fallback to the DigiKey `datasheet_url`'s TI name; a
  part whose datasheet TI does not publish is `skip`.
- `planning/plan-21-data/parts.tsv` (columns as plan-19-A §4.2), `specs.md`
  (per device: PDF pages, pinouts, the model keys of B/C filled in, timing per
  family — main path data → Y for `select`, clock → Q for `stored` registers,
  read address → Q for `regfile`, A → B for `bus4` — VCC and input max),
  `README.md`.
- Family `FCT` (added only if `grep -c '"FCT"' backend/src/library/kind_util.c` → 0; another plan of 19–22 may have added it — then moot with that output); docs § Values line; `library_check` case.
- Footprints `PDIP-24` (N, 300 mil) and `SM8-DCT` (DCT, 8 pins) from the PDF drawings; each added only if its id is absent (same rule).

## Phases

> **Status discipline:** tick with the command and output; `Commit:` when
> landed; differences in Corrections; commit on the current branch; touch
> only this plan's files.

### Phase 1 — Datasheets and part data

- [x] Stock re-check per base name (`node scripts/digikey.js --in-stock --manufacturer 296 --json "<base>"`, 114 base names, plus one exact-name search for each of the 119 candidates DigiKey gives no base name: 233 calls, not about 40 — Corrections); refresh `status`; out-of-stock rows and rows whose status is not Active or Last Time Buy `skip`. Run 2026-11-08 00:01 UTC after the daily quota reset (`/tmp/p21/scripts/stock.sh`): 233 searches, 0 errors, `703` in-stock TI parts parsed, `771 left today`; every one of the 634 candidates found in stock with its census status, so rebuilding `parts.tsv` from the re-check changed 0 rows (346 add; skips 156 Obsolete, 23 Discontinued at DigiKey, 81 not in the ordering table, 28 no TI datasheet; 0 out of stock).
- [x] Download the PDFs. TI symlinks for 461 names derived from the part numbers (+ SN54/-Q1/-EP/lit-number retries: sdls098, sdas219b, sdas137a), byte-identical sheets kept once. Check: `ls datasheets/mux/*.pdf | wc -l` → 102 = distinct `pdf` values of `parts.tsv` → 102.
- [x] `parts.tsv`, `specs.md` (20 device sections, read by seven parallel readers and loaded through the real loader: `check_device.py` → `ok` for all 20), `README.md`. Check: `tail -n +2 parts.tsv | wc -l` → 634; rows without a decision (or a skip without a reason) → 0; `add` rows without a footprint → 0; `add` rows → **N21 = 346** (20 devices).
- [x] `datasheets/skipped-parts.md` § TI multiplexers (288 parts in four reason rows: Obsolete 156, Discontinued at DigiKey 23, not in the ordering table 81, no TI datasheet 28); `obsolete-not-included.md` rows for `mux/sn54als857.pdf` (SN74ALS857: all Obsolete) and `mux/sdas137a.pdf` (SN74AS250A: all Obsolete) — neither PDF is kept in `datasheets/mux/`.

Acceptance: the checks above (minutes).
Commit: —

### Phase 2 — Family and footprints

- [x] ~~`FCT`~~ — moot: already present (`grep -c '"FCT"' backend/src/library/kind_util.c` → 1, added by plan-19); `library_check` prints `ok   gate part with family FCT`.
- [x] ~~Footprints `PDIP-24` and `SM8-DCT`; regenerate `footprints.txt`~~ — moot: SM8 (TI DCT, 8 pins) is the existing `SSOP-8` ("8-pin SSOP (DCT)", from the DCT0008A drawing), used by SN74LVC2G157DCTR; the only 24-pin DIP candidates (SN74AS250ANT, SN74ALS857NT, SN74ALS870NT) are all Obsolete, so no `add` row needs `PDIP-24` (every add row's footprint is an existing id: `add` rows without a footprint → 0). `git diff --stat backend/tests/golden/footprints.txt schematical/schematical/Parts/footprints.json` → empty.

Acceptance: `ctest --test-dir /tmp/logic-p21` → T0 of T0; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (seconds).
Commit: —

## Validation Plan

Landing tier at the end of E. For A: Phase 2's `ctest` and Phase 1's data checks.

## Decisions

- **Only Active and Last Time Buy parts are added** (owner, 2026-11-07): a part whose DigiKey status is Obsolete, Discontinued at DigiKey or Not For New Designs is `skip` with reason "status <status>", even when it is in stock and in the datasheet's ordering table. This overrides, for this plan, AGENTS.md's "add obsolete parts with the `obsolete` flag"; no part of this plan carries the `obsolete` flag. A PDF left with no addable part is deleted and recorded in `datasheets/obsolete-not-included.md` (all its parts obsolete) or `datasheets/not-included.md` (other statuses).
- **Scope** (owner, 2026-11-07): digital muxes plus muxes with storage, the
  '670 register file (and the '870, the same function doubled), and the
  ALS/AS data-generator specials; analog/FET switches out.
- **Plans 19–22 in any order** (owner, 2026-11-07); shared additions moot
  when present.
- **Inclusion rule**: in stock and in the ordering table (AGENTS.md;
  7022ae9).

## Corrections

- **T0, P0, F0** (2026-11-07, at 1b780ae): T0 = 22, not 21; P0 = 8344, not 8022; F0 = 239, not 237 — plan-19 (decoders) landed between the plan's writing and its execution. Every later "T0 + 1" means 23.
- **Stock re-check size**: the census has 114 distinct DigiKey base names plus 119 candidates with no base name (`tail -n +2 candidates.tsv | cut -f4 | sort -u` → 114 non-empty; `awk -F'\t' '$4==""'` → 119), so the re-check is about 233 calls, not "about 40". At execution start DigiKey had 7 calls left for the day (prerequisite row output), so the re-check ran after the daily quota reset (Phase 1 first box).
- **No TI datasheet for 28 candidates** (step 2 of `plan-21-data/README.md`): TI has no product page (`https://www.ti.com/product/<part>` → 404) and no `ti.com/lit` PDF for the '352, '353, '850, '851, LS354, LS355, LS604, 74ACT11151/11251, 74AC/ACT11157, ABT162460 and AHC2G157; DigiKey links only Rochester Electronics or DigiKey-hosted copies. They are `skip` "no TI datasheet", as plan-19's '131/'278. With the Obsolete/Discontinued '250, '857, '870, '16460 and ABTH162460 parts, **no addable part uses plan-21-C's models `universal` ('857) or `bus4` ('16460/'162460)**, nor `select`'s `selectLatch` ('850/'851); see plan-21-B and C Corrections.
- **Devices added**: 20, not the 34 functions of § Measured populations: `select` 15 devices (74_151, 153, 157, 158, 251, 253, 257, 258, 2G157, 1T157, 1T158, 11257, 4512B, 40257B, 2257 — 327 parts), `stored` 4 (74_298, 399, 354, 356 — 12 parts), `regfile` 1 (74_670 — 7 parts): 346 `add` rows.
- **CAHCT157QWBQBRQ1** is family `AHC`, not `AHCT` as the plan-19-A §3.3 automotive rule would give: it is listed only in the SN74AHC157-Q1 ordering table (marking AT157Q) with AHC levels (spec reader's finding, checked: `grep CAHCT157 /tmp/p21/txt/*.txt` → `sn74ahc157-q1.txt` only).
- **SN74298N** is `skip` "not in the datasheet ordering table": its sheet sdls098 lists SN74LS298N only (the builder first said "no TI datasheet" because the scanned sheet has no `74298` text).
- **Footprints**: neither `PDIP-24` nor `SM8-DCT` is needed (Phase 2); SM8 was already `SSOP-8`.

## Summary

A converts the census into datasheets and a reviewed part table, plus one
family code and two footprints.
