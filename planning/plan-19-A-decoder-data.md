# plan-19-A: Decoders and encoders — datasheets, part data, families, footprints

Last updated: 2026-11-07
Overall Effort: x-large (1d–3d) — plan-19 is A → B → C → D
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-19 adds every Texas Instruments decoder, demultiplexer, priority
encoder, BCD-to-decimal decoder, BCD-to-7-segment decoder/driver and the
SN74BCT2414 memory decoder that DigiKey has in stock, as a new part kind
`decoder` (CD4511B, a latch/decoder, joins the existing `latch` kind). Each
part simulates its datasheet function table, names a footprint, carries its
supply range and shows in the app's Logic drawer.

The single behavioral outcome of plan-19: loading the part files gives the
baseline part count + N19 parts (N19 = the rows of
`planning/plan-19-data/parts.tsv` marked `add`), every new device runs its
datasheet behavior in `decoder_test`, and `ctest` passes T0 + 1 programs.

| Letter | Content | Effort |
|---|---|---|
| **A** | download the datasheets; build `plan-19-data/parts.tsv` and the device spec sections; record skipped parts; families `FCT`, `L`, `ALV`; new footprints | large |
| B | `decoder` kind: models `binary`, `priority`, `seg7`; `decoder_test` | large |
| C | the parts: `decoder.json` and CD4511B in `latch.json`; goldens | medium |
| D | app: the `decoder` kind in the drawer and sidebar; plan-19 landing tier | medium |

Outcome of A: `datasheets/decoder/` and `datasheets/encoder/` hold every TI
PDF the candidate parts cite; `planning/plan-19-data/parts.tsv` has one row
per candidate part with its device, family, package code, footprint and an
`add`/`skip` status and reason; `planning/plan-19-data/specs.md` has one
section per device (pinout, function table, timing, supply); the three
family codes and the new footprints load; goldens change only by added
footprint lines.

References:

- `AGENTS.md` (datasheet rules: in-stock, obsolete, part files, goldens).
- `docs/parts-format.md` (whole file; § Values, § Supply voltage,
  § Footprints, § `gate`, § Sequential kinds / `bcd7seg`), `docs/backend.md`
  (adding a kind, tests, goldens).
- `planning/plan-19-data/candidates.tsv` — the census this plan was sized from.
- `planning/completed/plan-10-A-groundwork.md` §3.4–§3.5 (data conventions
  and signal rules this plan reuses), `plan-10-J-app.md` (app pattern).
- Precedent for the inclusion rule: commits 7022ae9 (TI inverter datasheets,
  skipped parts recorded in `datasheets/skipped-parts.md` § TI inverters) and
  0655d09 (the inverter devices).

## Prerequisites

This table is the gate for all of plan-19; B–D point here and add their own rows.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p19 && cmake --build /tmp/logic-p19 -j8 && ctest --test-dir /tmp/logic-p19` → `100% tests passed` | MET; **T0 = 21** (2026-11-07 at c7d2ba0 in `.claude/worktrees/P-19`: `100% tests passed, 0 tests failed out of 21`) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `footprints.txt` | MET: **P0 = 8022**, **F0 = 237** (re-measured 2026-11-07 at c7d2ba0) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p19-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-07 at c7d2ba0: `** BUILD SUCCEEDED **`) |
| No `decoder` kind yet | `ls schematical/schematical/Parts/decoder.json backend/src/models/decoder*.c 2>/dev/null` → no output | MET (2026-11-07 at c7d2ba0: no output) |
| DigiKey script works | `node scripts/digikey.js --in-stock --manufacturer 296 --limit 1 SN74HC138N` → a part line (credentials: memory `digikey-credentials`; a wrapper that evals the two `export DIGIKEY_CLIENT_*` lines of `~/.zshrc` then runs node) | MET (2026-11-07: `SN74HC138N  Active  5208  74HC …`) |

plan-19 does not wait for any other plan (owner: plans 19–22 run in any
order, possibly in parallel worktrees). Everything below is written against
the world where these rows hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of *all* rows.

## 1. Goal

- `ls datasheets/decoder/*.pdf datasheets/encoder/*.pdf | wc -l` equals the
  number of distinct `pdf` values in `parts.tsv`.
- `awk -F'\t' 'NR>1' planning/plan-19-data/parts.tsv | wc -l` → 505 (one row
  per candidate), every row `add` or `skip` with a reason.
- `kind_logic_family` accepts `FCT`, `L`, `ALV`; the new footprints load;
  `ctest` passes T0 of T0; `git diff backend/tests/golden` adds only
  footprint lines.

### Non-goals (explicit constraints)

- No existing part, listing line or trace changes (`git diff
  backend/tests/golden | grep -c '^-[^-]'` → 0).
- No new kind, model or part in A.
- Not in scope at all: TI analog and FET switches (category 743's bus
  switches belong to nobody), multiplexers (plan-21), parts already in
  `schematical/schematical/Parts/*.json`.

## 2. Current State

- Census: `planning/plan-19-data/candidates.tsv` — every TI in-stock part
  (DigiKey `--in-stock --manufacturer 296`, categories 743 "Signal Switches,
  Multiplexers, Decoders" and 703 "Specialty Logic", 2026-11-07) whose
  function is in plan-19's scope. Columns: `function family name base series
  status qty supplier_pkg case type circuit output supply datasheet_url
  description`.
- Families: `LOGIC_FAMILIES` in `backend/src/library/kind_util.c:435-438`
  (25 codes, `HC` … `TPIC`); `docs/parts-format.md` § Values lists them.
- Footprints: `schematical/schematical/Parts/footprints.json`, 237 ids
  (`python3 -c "import json;print(len(json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 237).
- TI package code → footprint mapping used by earlier plans:
  `planning/plan-10-data/parts.tsv` columns `code`, `pins`, `footprint`
  (e.g. `N 16 PDIP-16`, `D 16 SOIC-16`, `PW 16 TSSOP-16`, `NS 16 SOP-16`,
  `DB 16 SSOP-16`, `DGV 16 TVSOP-16`, `BQB 16 WQFN-16`, `RGY 16 VQFN-16`,
  `J 16 CDIP-16`, `W 16 CFP-16`, `FK 20 LCCC-20`), plus the gate parts'
  `DBV SOT23-5/6`, `DCK SC70-5/6`, `DRL SOT5X3-6`, `YZP DSBGA-*-YZP`,
  `DCU VSSOP-8`, `DRY/DSF SON-6-*`.
- The existing BCD-to-7-segment latch: `latch.json` device `74_4511`
  (CD74HC4511, model `bcd7seg`, `docs/parts-format.md` § Model `bcd7seg`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidate parts | 505 | `tail -n +2 planning/plan-19-data/candidates.tsv \| wc -l` |
| Candidate functions | 37 | `tail -n +2 planning/plan-19-data/candidates.tsv \| cut -f1 \| sort -u \| wc -l` |
| … listed obsolete by DigiKey | 102 | `awk -F'\t' '$6=="Obsolete"' planning/plan-19-data/candidates.tsv \| wc -l` |
| … addable by status (Active or Last Time Buy) | 395 (110 excluded: Obsolete, Discontinued at DigiKey, Not For New Designs) | `awk -F'\t' 'NR>1 && ($6=="Active"||$6=="Last Time Buy")' planning/plan-19-data/candidates.tsv \| wc -l` |
| Per function | 138: 166, 139: 108, 238: 34, 1G19: 15, 42: 18, 137: 16, 148: 12, 237: 11, 4028: 8, 4555: 8, 156: 8, 1G18: 8, 11138: 8, 1G29: 7, 145: 7, 147: 7, 155: 7, 4515: 7, 4532: 7, 1G139: 6, 11139: 6, 4514: 5, 154: 3, 348: 3, 4014 (CD40147): 3, 4556: 3, 131: 2, 278: 2, 2414: 2, 45: 1, 47: 1, 49: 1, 159: 1, 445: 1, 4056: 1, 4511: 1, 11238: 1 | `tail -n +2 planning/plan-19-data/candidates.tsv \| cut -f1 \| sort \| uniq -c` |
| Families in the candidates | HC, HCT, HCS, LS, ALS, AS, F, S, AC, ACT, AHC, AHCT, LV, LVC, AUC, CD4 (→ `C`), FCT, L, ALV, standard TTL (blank → `TTL`) | `cut -f2 planning/plan-19-data/candidates.tsv \| sort -u` |
| Candidates with no DigiKey package (ceramic / military; mapped from the TI code) | 53 | `awk -F"\t" 'NR>1 && $8==""' planning/plan-19-data/candidates.tsv | wc -l` → 53 |
| Candidate packages with no existing footprint | 16-SOT-23-THIN (5), 24-PDIP (4), SM8 (4), 20-BGA MicroStar Junior 4×3 (3), 16-UQFN 2.6×1.8 (1) | `planning/plan-19-data/census/fp.py` (supplier package → footprint map vs `footprints.json`; run in a directory holding `p19.tsv` = candidates without header) |

### Verified properties

| Claim | How verified |
|---|---|
| None of the 505 candidates is already in the catalog | python set of every `name` in `schematical/schematical/Parts/*.json` ∩ candidate names → 0 (2026-11-07) |
| The SN74BCT2414 is a dual 2-to-4 decoder with enables SD, G, 1G, 2G and a VCC-monitor output VS (VCC > 3.65 V) | `sn74bct2414.pdf` p.2 logic symbol (downloaded to `/tmp/p19/pdf/`, `pdftotext -layout`) |
| No TI datasheet is published for the '278 | `curl -sfL -o /dev/null https://www.ti.com/lit/ds/symlink/sn54278.pdf` → exit 22 (2026-11-07) |
| CD4056B is a BCD-to-7-segment decoder/driver with a strobed latch and a display-frequency input | `cd4056b.pdf` downloaded (854858 bytes); first page title |

## 3. Design Overview (all of plan-19)

### 3.1 Where each part goes

| Kind | Model | Functions | Sub-plan |
|---|---|---|---|
| `decoder` (new) | `binary` | 131, 137, 138, 139, 154, 155, 156, 159, 237, 238, 1G139, 1G18, 1G19, 1G29, 11138, 11139, 11238, 4028, 4514, 4515, 4555, 4556, 42, 45, 145, 445, 2414 | B, C |
| `decoder` | `priority` | 147, 148, 348, 4532, 40147 | B, C |
| `decoder` | `seg7` | 47 (SN7447A), 49, 4056 | B, C |
| `latch` (existing) | `bcd7seg` | 4511 (CD4511B) | C |
| skipped | — | 278 (no TI datasheet) | A |

### 3.2 Why one kind with three models

The decoders, encoders and 7-segment drivers are all combinational (or
combinational behind an input latch): one `PartKindOps` and one
`SimComponentType`, a `"model"` device key, the plan-09-B / plan-10
precedent ("one `SimComponentType` per kind; a kind with several behaviours
dispatches inside its model"). The models reuse `models/seq.h` (timing
entries, controls, level reading, change-suppressed inertial outputs with
3-state and open-drain).

Rejected alternatives:

- **Decoders in the `gate` kind** as a sum of AND gates: no enables with
  priority, no address latch, no 3-state, and the 4-to-16 parts would need 16
  gates of 6 inputs each with inverted inputs the gate kind cannot express.
- **One model per function** (`138`, `139`, …): 27 binary functions share one
  machine (address in, one-of-N out, ANDed enables).
- **CD4511B in `decoder`**: the existing `bcd7seg` latch model already
  implements the CD74HC4511's identical function table; a second copy would
  diverge.

### 3.3 Data conventions

plan-10-A §3.4 applies unchanged (parts from `parts.tsv`, flags `obsolete` /
`military` / `automotive`, device ids `74_<function>` / `<number>B`,
pinout names `p16` …, family codes with bus-hold folded into the base, one
`{rise, fall}` per family at the reference condition with a `note`, SN54 vs
SN74 `vcc`, `"drawer": ["Logic"]`, a `note` citing the PDF and spec
section). The drawer `function` is `"Decoder"` for `binary`, `"Encoder"` for
`priority`, `"7-segment decoder"` for `seg7`.

Additional rules:

- **Inclusion** (AGENTS.md, inverter precedent): a part is `add` only when it
  is in stock (`--in-stock`) on the day A runs **and** appears in its PDF's
  ordering/package-option table. In-stock parts missing from the table, and
  parts whose datasheet TI does not publish, are `skip` and go into
  `datasheets/skipped-parts.md` § TI decoders (2026-MM-DD) with the reason.
  A PDF whose every part is obsolete is deleted and recorded in
  `datasheets/obsolete-not-included.md`.
- **Automotive variants named outside the SN74 pattern** (CAHCT138, CLVC138,
  CAHCT139, CLVC139) are parts of the `74_138` / `74_139` devices with the
  `automotive` flag; CY74FCT138 parts are `74_138` with family `FCT`.
- **Military SMD numbers** (5962-…, JM38510/…, 7-digit numbers): the PDF's
  orderable-part table names the SN54 part each one is; the row takes that
  part's device and family, flag `military`.

### 3.4 Signal rules

plan-10-A §3.5 applies (power-on state 0 for stored bits; Z on a data/select
input reads X; X on an enable makes every output it gates X; outputs
change-suppressed, inertial, fall delay for 0, rise for 1 and Z, longer for
X; open-drain drives 0 and releases to Z). Specific to decoders: an X
address bit makes X exactly the outputs whose state depends on it (an output
that is inactive for every completion of the address stays inactive).

### 3.5 Where correctness risk concentrates

- The `binary` model carries 467 of the 505 candidate parts (the 27 binary
  functions' counts above sum to 467). Its enable-polarity table and the
  partial-output rule (BCD-to-decimal: codes 10–15 leave every output
  inactive) are tested case by case in `decoder_test` (B) before any part
  data lands (C).
- The 7447's bidirectional BI/RBO pin (input that blanks, open-drain output
  that signals ripple blanking) is the only bidirectional pin in the kind; B
  tests both directions.
- Byte-identity is the gate for existing parts only. New parts are proven by
  `decoder_test` and spot checks, not by goldens.

## 4. Detailed Design (this sub-plan)

### 4.1 Datasheets

- Folders: `datasheets/decoder/` (binary, BCD-to-decimal, 7-segment, memory
  decoder PDFs) and `datasheets/encoder/` (priority encoders).
- One PDF per distinct TI datasheet the candidates cite: the `datasheet_url`
  column (DigiKey) names it; the file is
  `https://www.ti.com/lit/ds/symlink/<lowercase base name>.pdf`, saved as
  `<lowercase base name>.pdf` (`curl -sfL -o`). A URL that fails is retried
  with the SN54/CD54 or `-ep`/`-q1` name the DigiKey URL gives; a part whose
  datasheet cannot be fetched at all is `skip` ("no TI datasheet").
- The '278 rows (2) are `skip` ("no TI datasheet", §2 probe).

### 4.2 `planning/plan-19-data/`

- `parts.tsv`: columns `name device family package code pins footprint
  flags status pdf decision reason` (plan-10's columns plus `decision` and
  `reason`). One row per `candidates.tsv` row (505).
- `specs.md`: one section per device id, each with: the PDF(s) and pages;
  the pinout per package style (pin number → name, names per the repo
  convention: no overbars, `BAR` only for true/complement pairs); the
  device's model keys (§ plan-19-B §4 key tables) filled from the function
  table; timing per family (main path: address to output for `binary`,
  input to output for `priority`, BCD input to segment for `seg7`); VCC and
  input-max per family; polarity of every input and output as printed.
- `README.md`: what each file is and the census date.

### 4.3 Families

`LOGIC_FAMILIES` gains `FCT` (TI/Cypress FCT CMOS), `L` (low-power TTL,
SN74L42), `ALV` (SN74ALV parts); `docs/parts-format.md` § Values gets one
line each. Each is added only if absent — plans 19–22 run in any order and
`FCT` is shared: if `grep -c '"FCT"' backend/src/library/kind_util.c` → 1
before the edit, that code's task is moot (record the grep output).

### 4.4 Footprints

New ids, each drawn from the TI package drawing in the downloaded PDF
(`docs/parts-format.md` § Footprints, pad rules of plan-03-B §3):
`SOT23-THIN-16` (DYY, 16 pins), `PDIP-24` (N, 24 pins, 300 mil),
`SM8-DCT` (DCT, 8 pins), `BGA-20-MICROSTAR-JR` (20-ball MicroStar Junior,
4 × 3 mm), `UQFN-16-RSV` (RSV, 2.6 × 1.8 mm). Same rule as §4.3: an id
another plan already added is reused (task moot with the grep). Each row of
`parts.tsv` names an existing or one of these ids; any further package found
while mapping gets a footprint the same way and a line in Corrections.

## Phases

> **Status discipline (every phase).** Tick a box only when its check has
> run, with the command and output on the task line; fill `Commit:` when the
> phase lands; differences go in Corrections. Commits on the current branch
> (AGENTS.md); touch only this plan's files.

### Phase 1 — Datasheets and part data

- [x] Stock re-check: for each function, `node scripts/digikey.js --in-stock --manufacturer 296 --list "<base name>"` (budget: one call per base name; about 60 calls) → refresh `qty`/`status` per row; parts no longer in stock become `skip` ("not in stock on <date>"), parts whose status is not Active or Last Time Buy become `skip` ("status <status>"). Done 2026-11-07 with `--json` (for qty): 82 base-name searches + 30 exact-name searches for the candidates with no DigiKey base → `candidates 505 found 503 missing 2` (missing: SN74278N, SNJ54278J, skipped as no TI datasheet anyway); no status changed. Status skips: 102 Obsolete, 8 Discontinued at DigiKey.
- [x] Download the PDFs of §4.1 into `datasheets/decoder/` and `datasheets/encoder/`. Check: `ls datasheets/decoder/*.pdf datasheets/encoder/*.pdf | wc -l` = `cut -f10 planning/plan-19-data/parts.tsv | tail -n +2 | sort -u | grep -c pdf` → `89` = `89` (83 decoder, 6 encoder).
- [x] Write `parts.tsv` (§4.2) from each PDF's ordering table; `specs.md`; `README.md`. Check: `tail -n +2 parts.tsv | wc -l` → 505; `awk -F'\t' 'NR>1 && $11!="add" && $11!="skip"' parts.tsv | wc -l` → 0; every `add` row has a footprint: `awk -F'\t' 'NR>1 && $11=="add" && $7==""' parts.tsv | wc -l` → 0. Measured: `505`, `0`, `0`; **N19 = 322** add rows (`awk -F'\t' 'NR>1 && $11=="add"' parts.tsv | wc -l`), **n4511 = 0**; 32 devices, `grep -c '^## ' specs.md` → 32. Each device's JSON was load-checked against the decoder kind (`/tmp/p19/scripts/check_device.py` → `ok` for all 32).
- [x] `datasheets/skipped-parts.md` § TI decoders: one table of the `skip` rows grouped by reason; `obsolete-not-included.md` rows for any all-obsolete PDF (and the PDF deleted). Section `## TI decoders (2026-11-07)`, 4 reason rows covering all 183 skip rows. No PDF is all-obsolete (`sheets only on skip rows: []`). Two downloaded PDFs have no addable part for other reasons (cd4511b, cd4056b): not copied in, recorded in `datasheets/not-included.md`.

Acceptance: the four checks above (minutes).
Commit: —

### Phase 2 — Families and footprints

- [ ] §4.3 families + docs; `library_check` case: a gate part with `"family": "FCT"` and an `FCT` timing entry loads (`ok   gate part with family FCT`).
- [ ] §4.4 footprints into `footprints.json`; regenerate `backend/tests/golden/footprints.txt` (docs/backend.md § Tests).

Acceptance: `ctest --test-dir /tmp/logic-p19` → T0 of T0 passed;
`git diff backend/tests/golden | grep -c '^-[^-]'` → 0;
`git diff backend/tests/golden/footprints.txt | grep -c '^+[^+]'` = the
number of new footprint ids (seconds).
Commit: —

## Validation Plan

The landing tier for plan-19 runs once, at the end of D. For A: Phase 2's
`ctest` (about 13 s) plus the data checks of Phase 1.

## Decisions

- **Only Active and Last Time Buy parts are added** (owner, 2026-11-07): a part whose DigiKey status is Obsolete, Discontinued at DigiKey or Not For New Designs is `skip` with reason "status <status>", even when it is in stock and in the datasheet's ordering table. This overrides, for this plan, AGENTS.md's "add obsolete parts with the `obsolete` flag"; no part of this plan carries the `obsolete` flag. A PDF left with no addable part is deleted and recorded in `datasheets/obsolete-not-included.md` (all its parts obsolete) or `datasheets/not-included.md` (other statuses).
- **Scope** (owner, 2026-11-07): TI binary decoders/demultiplexers plus
  priority encoders, BCD-to-decimal decoders, BCD-to-7-segment drivers
  (SN7447A; CD4511B into the existing latch kind) and the SN74BCT2414 memory
  decoder.
- **Plans 19–22 run in any order, possibly in parallel** (owner, 2026-11-07):
  no prerequisite on another plan; shared additions (family codes, footprint
  ids, enum slots, docs rows) are made by whichever plan lands first and are
  moot in the others.
- **Inclusion rule**: in stock and in the datasheet's ordering table; others
  recorded (AGENTS.md; inverter precedent 7022ae9).
- **The '278 is skipped**: TI publishes no datasheet (§2 probe).
- **Military numbers and FCT/automotive-named parts** join their SN74
  device (§3.3; plan-10 precedent).

## Corrections

- **The census's CD4511B and CD4056B parts are not addable** (2026-11-07):
  the census holds only CD4511BNS and CD4056BNS, and neither is in its
  datasheet's packaging table (cd4511b lists CD4511BNSR; cd4056b lists
  CD4056BE, BF3A, BM96, BPW; `grep` of `/tmp/p19/txt/cd4511b.txt`). So
  n4511 = 0: no `4511B` latch device and no `4056B` device; plan-19-C's
  CD4511B task is moot on this evidence. The other §3.1 functions with no
  added part: '131, '159, '445, 11238 (no TI datasheet: `curl` of
  `sn74als131`, `sn74as131a`, `sn74ls445`, `74ac11238` symlinks returns no
  PDF; the 74_159 sheet `sn74159` returns an HTML page), '278 (as planned).
- **PDF names** (§4.1): files are named after the TI datasheet
  (`sn74hc138.pdf`, `cd74hc138.pdf`, `sn74ls139a.pdf` …), not the DigiKey
  base number; byte-identical downloads (e.g. `cd54hc138`, `cd74hc238`,
  `cd74hct138` are one file) are kept once under the alphabetically first
  SN74/CD74/CD4 name. A part's PDF is the sheet whose packaging table lists
  it, preferring the sheet named after its own series.
- **Stock re-check cost** (Phase 1): 112 calls (82 base names + 30 exact
  names), not ~60; a first run of the script also spent 82 calls because its
  output directory did not exist yet (results discarded).
- **Footprints** (§4.4), measured on the `add` rows of `parts.tsv`:
  - `SM8-DCT` moot: TI's DCT (SM8) package already has the footprint
    `SSOP-8` ("8-pin SSOP (DCT)", from DCT0008A); the 3 DCT add rows use it.
  - `BGA-20-MICROSTAR-JR` moot: its 3 candidates (SN74LVC138AGQNR,
    SN74LVC138AZQNR, SN74LVC139AGQNR) are Obsolete → `skip`.
  - `PDIP-24` moot: its 4 candidates (CD74HCT4514E, CD4515BE, CD74HC4515E,
    CD74HC4515EN) are Obsolete → `skip`.
  - CY74FCT138 QSOP (TI code DBQ) uses the existing `QSOP-16` (same JEDEC
    MO-137 body and 0.635 mm pitch).
  - New: `SOT23-THIN-16` (DYY; 5 add rows), `UQFN-16-RSV` (RSV; 1 add row).
- **74_154 enables are E1, E2** (only CD74HC/HCT154 parts are added, so the
  device uses the CD74 names), not G1/G2 (specs.md § 74_154).
- **Design corrections found by the spec read** (repaired in plan-19-B):
  the SN74BCT2414's two units have separate address pins (they share only
  G and SD), and its VS output is open-collector and low while VCC is good;
  '1G29 code 3 does select an output (A1 = L selects Y0 whatever A0 is), so
  it is two units sharing G; the 7447A's BI wins over LT (blank > lamp test).

## Summary

A turns the DigiKey census into datasheets on disk and a reviewed part table,
adds three family codes and the missing footprints. No behavior changes.
