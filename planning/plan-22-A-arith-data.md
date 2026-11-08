# plan-22-A: Comparators, adders, ALUs, parity, multipliers — datasheets and part data

Last updated: 2026-11-07
Overall Effort: x-large (1d–3d) — plan-22 is A → B → C → D → E
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-22 adds the Texas Instruments magnitude and identity comparators,
adders, ALUs, carry-lookahead generators, parity generators/checkers and
multipliers that DigiKey has in stock **and** that have a datasheet, as a new
part kind `arith`. Each part simulates its datasheet function table, names a
footprint, carries its supply range and shows in the Logic drawer.

The owner also chose the SN74ACT8832 (32-bit ALU) and SN74ACT8847
(floating-point unit), the '381/'382/'881/'882 ALUs, the '183/'385/'80
adders and the '261/'384 multipliers. No datasheet for any of them can be
fetched from TI (§2 Verified properties; Rochester Electronics copies exist, see Corrections). Under AGENTS.md and the inverter
precedent (a part with no datasheet has no pinout, timing or package
drawing), A records them in `datasheets/skipped-parts.md` with that reason.
A later plan adds them if the owner supplies the PDFs.

The single behavioral outcome of plan-22: loading the part files gives the
baseline count + N22 parts (`add` rows of `planning/plan-22-data/parts.tsv`),
every new device runs its datasheet behavior in `arith_test`, and `ctest`
passes T0 + 1 programs.

| Letter | Content | Effort |
|---|---|---|
| **A** | datasheets; `plan-22-data/parts.tsv`, `specs.md`; skipped parts; families `FCT`, `H` check; footprints | large |
| B | `arith` kind; models `compare` and `parity`; `arith_test` | large |
| C | models `adder`, `alu181`, `lookahead`, `wallace` | large |
| D | the parts: `arith.json`; goldens | medium |
| E | app: the `arith` kind; plan-22 landing tier | medium |

References:

- `AGENTS.md`; `docs/parts-format.md`; `docs/backend.md`.
- `planning/plan-22-data/candidates.tsv` (census: DigiKey categories 773
  Comparators, 703 Specialty Logic, 720 Parity Generators and Checkers, 743
  for '261/'384; TI, in stock, 2026-11-07) and `planning/plan-22-data/census/`.
- `planning/completed/plan-10-A-groundwork.md` §3.4–§3.5; `plan-10-J-app.md`;
  inverter precedent 7022ae9 (`datasheets/skipped-parts.md` § TI inverters:
  "No TI datasheet available … DigiKey links only a Rochester viewer page").

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p22 && cmake --build /tmp/logic-p22 -j8 && ctest --test-dir /tmp/logic-p22` → `100% tests passed` | MET; **T0 = 24** (2026-11-07 16:30 HST at 76e4f46: `100% tests passed, 0 tests failed out of 24`) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `footprints.txt` | **P0 = 11126**, **F0 = 423** (2026-11-07 at 76e4f46) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p22-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-07 at 76e4f46: `** BUILD SUCCEEDED **`) |
| No `arith` kind yet | `ls schematical/schematical/Parts/arith.json backend/src/models/arith*.c 2>/dev/null` → no output | MET (2026-11-07 at 76e4f46: no output) |
| DigiKey script works | `node scripts/digikey.js --in-stock --manufacturer 296 --limit 1 SN74LS283N` → a part line | MET (2026-11-07 16:25 HST: `SN74LS283N Active 2285`, 474 calls left today) |

plan-22 waits for no other plan (owner: 19–22 in any order, possibly in
parallel).

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all rows.

## 1. Goal

- `ls datasheets/comparator/*.pdf datasheets/arith/*.pdf datasheets/parity/*.pdf | wc -l` = the distinct `pdf` values of `parts.tsv` `add` rows.
- `tail -n +2 planning/plan-22-data/parts.tsv | wc -l` → 225; each row `add` or `skip` with a reason; every function of §2 "no datasheet" is `skip` with reason "no datasheet".
- `FCT` accepted; new footprints load; `ctest` T0 of T0; goldens add only footprint lines.

### Non-goals

- No existing line changes. Rate multipliers 4089/4527/'97 are already in
  `counter.json` and are not candidates. EDAC parts ('630/'632/'637) and
  memory mappers ('610/'612) are not in the owner's scope.

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidate parts | 225 | `tail -n +2 planning/plan-22-data/candidates.tsv \| wc -l` |
| … addable by status (Active or Last Time Buy) | 179 (46 excluded: Obsolete, Discontinued at DigiKey, Not For New Designs) | `awk -F'\t' 'NR>1 && ($6=="Active"||$6=="Last Time Buy")' planning/plan-22-data/candidates.tsv \| wc -l` |
| … comparators | 109 | `tail -n +2 … \| cut -f1 \| grep -cxE '85\|518\|519\|520\|521\|527\|528\|677\|679\|680\|682\|684\|685\|686\|688\|689\|866\|885\|4063\|4585\|11521\|29806'` |
| … adders ('283, '183, '385, '80) | 32 | same, `'80\|83\|283\|183\|385'` |
| … ALUs ('181, '381, '382, '881, 11181) | 14 | same |
| … lookahead ('182, '882, '264) | 3 | same |
| … parity ('280, '180, '286, 11286, FCT480, SN700526) | 51 | same |
| … multipliers ('261, '275, '384) | 5 | same |
| … ACT8832, ACT8847 | 2 | same |
| … military SMD numbers (mapped to devices in A) | 9 | the rest: `5962-8685701RA`, `8415301RA`, `8601301EA`, six `JM38510/…` |
| Families | LS 46, HC 32, ALS 30, AS 23, F 22, S 15, HCT 11, ACT 10, CD4 (→ `C`) 9, AC 6, FCT 4, H 4, blank (`TTL`) 3, unparsed 10 | `tail -n +2 … \| cut -f2 \| sort \| uniq -c` |
| No DigiKey package (ceramic/military) | 91 | `awk -F'\t' 'NR>1 && $8==""' … \| wc -l` |
| Packages with no existing footprint | 24-PDIP (5) | `census/fp.py` |

### Verified properties — datasheet availability (2026-11-07)

`curl -s -o /dev/null -w '%{http_code}:%{content_type}' -L https://www.ti.com/lit/ds/symlink/<name>.pdf`
(script `planning/plan-22-data/census/probe.sh <names…>`, rerun in A Phase 1):

| `application/pdf` (datasheet available) | `404` or an HTML page (no datasheet) |
|---|---|
| sn74ls85, sn74s85, cd74hc85, sn74ls682, sn74hc682, sn74ls684, sn74ls688, sn74hc688, sn74als688, cd74hct688, sn74als518, sn74als520, sn74als521, sn74f521, sn74as885, cd4063b, cd4585b, sn74ls283, sn74s283, sn74f283, cd74hc283, cd74ac283, sn54ls181, sn54s181, sn54s182, sn74ls280, sn74as280, sn74f280b, sn54s280, cd74hc280, cd74ac280, sn54180, sn74as286, 74act11286 | sn74ls685, sn74ls686, sn74ls689, sn74f518 / sn54f518, sn74als677, sn74als679, sn74als680, sn74as866, sn74ac11521, sn5480, sn74h183 / sn54h183, sn74ls183, sn74ls385, sn74ls381 / sn74s381 / sn54s381, sn74ls382, sn74as881a, sn74as882, sn74ac11181, sn74as264, sn74ls261, sn74ls384, sn74act8832, sn74act8847, sn74als527/528 and sn74als29806 (Rochester viewer links only), sn74als280 (HTML), sn74s280 (HTML), sn74ls181 / sn74as181a (HTML; the SN54 sheets cover them), cy74fct480t (HTML) |

- DigiKey's datasheet links for SN74ACT8832AGB and SN74ACT8847-30GA are
  `rocelec.widen.net` viewer pages: `curl -sfL` saves 24519 bytes, `file` →
  `HTML document` for both. The page's `previews…` link does yield a PDF
  (Rochester Electronics' copies of TI's scanned data manuals: 192 and 261
  pages), but TI itself publishes none (no product page, no `lit/ds` PDF) —
  corrected 2026-11-07, see Corrections.
- SN54LS275J's DigiKey link
  (`https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/5028/SNx4%28L%29S27x.PDF`)
  is a 10-page scanned TI datasheet (`file` → `PDF document, version 1.5, 10
  pages`; image-only, `pdfimages -list` → one CCITT page image per page).
  The '275 is therefore available: A transcribes it from the page images.
- No candidate is already in the catalog (name set ∩ `Parts/*.json` → 0).

## 3. Design Overview (all of plan-22)

### 3.1 Models

| Model | Functions (datasheet available) | Sub-plan |
|---|---|---|
| `compare` | '85, 4063, 4585 (cascadable magnitude), '682, '684 (P>Q, P=Q), '688, '518, '520, '521 (identity with enable; open-drain variants), '885 (8-bit magnitude with latch, per specs.md) | B |
| `parity` | '280 (9-bit even/odd), '180 (8-bit with even/odd inputs), '286 and 11286 (9-bit with parity I/O transceiver and XMIT) | B |
| `adder` | '283 (4-bit binary full adder, fast carry) | C |
| `alu181` | '181 (16 logic + 16 arithmetic functions, M, Cn, P, G, A=B open-drain, Cn+4) | C |
| `lookahead` | '182 (four-stage carry lookahead) | C |
| `wallace` | '275 (7-bit-slice Wallace tree, per its scanned sheet), 3-state outputs | C |

One kind `arith`, one `SimComponentType`, a `"model"` key (plan-10
precedent). Rejected: comparators in `gate` (no cascade inputs, no
magnitude); one kind per function family (six kinds of one or two devices
each); a generic expression language (the '181 function table and the
cascade rules are clearer as code with a test per row).

### 3.2 Data conventions

plan-10-A §3.4 plus plan-19-A §3.3 (inclusion rule; military numbers to
their SN54 device; FCT family). Drawer `function`: `"Comparator"`,
`"Adder"`, `"ALU"`, `"Carry lookahead"`, `"Parity"`, `"Multiplier"`.

### 3.3 Signal rules

plan-10-A §3.5. Arithmetic X rule: an unknown input bit makes X every output
bit whose value depends on it (computed by evaluating both completions of
each unknown bit, up to 2^k for k ≤ 8 unknown bits; more → every dependent
output X). Outputs are combinational except the '885 latch.

### 3.4 Risk

The '181 (32 function rows × carry in) and the '182 carry equations are the
easiest places for a silent wrong answer; C tests every row of both tables
exhaustively against a reference written from the datasheet equations.

## 4. Detailed Design (this sub-plan)

- Datasheets: `datasheets/comparator/`, `datasheets/arith/` (adders, ALU,
  lookahead, multiplier), `datasheets/parity/`; TI symlink PDFs of §2 via
  `curl -sfL -o`; the '275 from its DigiKey PDF link. Every function in the
  right column of §2 → `skip`, reason "no datasheet (ti.com 404/HTML;
  DigiKey link is a viewer page)".
- `planning/plan-22-data/parts.tsv`, `specs.md`, `README.md` as plan-19-A
  §4.2; timing main paths: A/B → output for `compare`, `adder`, `alu181`;
  data → ΣE/ΣO for `parity`; P/G → C for `lookahead`; input → output for
  `wallace`.
- Family `FCT` only if absent (`grep -c '"FCT"' backend/src/library/kind_util.c` → 0; else moot with that output — plans 19–22 run in any order). `H` already exists (`kind_util.c:437`).
- Footprint `PDIP-24` (N, 300 mil), only if absent (same rule).

## Phases

> **Status discipline:** tick with command and output; `Commit:` when landed;
> Corrections for differences; current branch; only this plan's files.

### Phase 1 — Datasheets and part data

- [x] Re-run the §2 availability probe; stock re-check per base name (`node scripts/digikey.js --in-stock --manufacturer 296 --list "<base>"`; about 40 calls); rows whose status is not Active or Last Time Buy `skip`. Probe (`/tmp/p22/scripts/probe2.sh`, 2026-11-07 16:30 HST): every §2 "available" name → `200:application/pdf`, every §2 "no datasheet" name → 404 or HTML, plus `https://www.ti.com/product/<base>` → 404 (3542 bytes) for all of them; the TI literature numbers printed on the Rochester copies of the no-datasheet parts (sdls168, sdls169, sdas235, sdas183a, sdas209b, sdas012c, sdas003c, sdas051a, scas022a) → 404 (`/tmp/p22/scripts/litprobe.sh`). Stock re-check (`--json`, `/tmp/p22/scripts/stock.sh`): 113 searches (88 base names + 25 exact names; not about 40 — Corrections), `346 left today`, 499 in-stock TI parts; 222 of 225 candidates found with their census status (0 status changes), SN74ALS677ADW, SN74ALS679DW, SN74ALS679N not found → `skip` "not in stock on 2026-11-07".
- [x] Download the available PDFs. Check: `ls … | wc -l` = distinct `pdf` values of `add` rows. 72 files fetched (symlinks + TI literature numbers), 29 distinct; the 26 named by `add` rows copied: `ls datasheets/comparator/*.pdf datasheets/arith/*.pdf datasheets/parity/*.pdf | wc -l` → 26; `awk -F'\t' 'NR>1 && $11=="add"{print $10}' planning/plan-22-data/parts.tsv | sort -u | wc -l` → 26.
- [x] `parts.tsv` (225 rows), `specs.md`, `README.md`. Checks: 0 undecided rows; 0 `add` rows without a footprint; every no-datasheet function's rows `skip`. `tail -n +2 parts.tsv | wc -l` → 225; undecided or reasonless skips → 0; `add` rows without a footprint → 0; **N22 = 107** `add` rows over 16 devices; skips: 41 Obsolete, 3 Discontinued at DigiKey, 33 not in the datasheet ordering table, 38 no TI datasheet (every §2 no-datasheet function), 3 not in stock. `specs.md`: 16 sections (five parallel readers + the '275 from its page images), each device JSON passing `/tmp/p22/scripts/check_device.py` (`ok` ×16).
- [x] `datasheets/skipped-parts.md` § TI comparators and arithmetic: the `skip` rows grouped by reason, the ACT8832/ACT8847 row first with "owner chose them; no datasheet can be fetched (2026-11-07)". 118 skip rows: the ACT8832/8847 row first, then one row per reason (`/tmp/p22/scripts/skipped.py`). Sheets fetched but kept for no part: `obsolete-not-included.md` row for `parity/sccs025b.pdf` (CY74FCT480T, all Obsolete); `not-included.md` rows for `parity/sn54180.pdf` and `arith/sn54s182.pdf` (only SN54 parts in their tables, none in stock).

Acceptance: the checks above (minutes). Met: every Phase 1 check output is on its task line.
Commit: —

### Phase 2 — Family and footprint

- [x] ~~`FCT` (or moot) with docs line and `library_check` case; `PDIP-24` (or moot); regenerate `footprints.txt`~~ — moot: `grep -c '"FCT"' backend/src/library/kind_util.c` → 1 (plan-19) and `library_check` prints `ok   gate part with family FCT`; no `add` row needs `PDIP-24` (the only 24-pin DIPs, SN74AS885NT/NT3, are Obsolete or not in the ordering table; `awk … $7=="PDIP-24"` → 0) and every `add` footprint exists (missing → `[]`), so `footprints.json` and `footprints.txt` are unchanged.

Acceptance: `ctest --test-dir /tmp/logic-p22` → T0 of T0; no golden line removed (seconds). Met: `100% tests passed, 0 tests failed out of 24` (T0 = 24); `git diff --stat backend/tests/golden` → empty.
Commit: —

## Validation Plan

Landing tier at the end of E. For A: Phase 2's `ctest` and Phase 1's checks.

## Decisions

- **Only Active and Last Time Buy parts are added** (owner, 2026-11-07): a part whose DigiKey status is Obsolete, Discontinued at DigiKey or Not For New Designs is `skip` with reason "status <status>", even when it is in stock and in the datasheet's ordering table. This overrides, for this plan, AGENTS.md's "add obsolete parts with the `obsolete` flag"; no part of this plan carries the `obsolete` flag. A PDF left with no addable part is deleted and recorded in `datasheets/obsolete-not-included.md` (all its parts obsolete) or `datasheets/not-included.md` (other statuses).
- **Scope** (owner, 2026-11-07): comparators, adders/ALUs, carry lookahead,
  parity generators, multipliers, and the ACT8832/ACT8847.
- **Parts without a datasheet are skipped** (AGENTS.md and the inverter
  precedent): this removes the ACT8832, ACT8847, '381, '382, '881, '882,
  '183, '385, '80, '264, '261, '384, '685, '686, '689, '518 (F), '677, '679,
  '680, '866, '527, '528, 29806 (DigiKey links are Rochester viewer pages), 11181, 11521 and the ALS/S '280 and FCT480 sheets that resolve
  to HTML (§2). Their rows are recorded with the reason.
- **Plans 19–22 in any order** (owner, 2026-11-07).

## Corrections

- **"No datasheet can be fetched" is false as written** (2026-11-07). The
  Rochester `widen.net` viewer pages DigiKey links lead to real PDFs (the
  plan-22 authoring session's own `/tmp/p22/roc.sh` saved them: ACT8832 192
  pages, ACT8847 261 pages, '381/'382 (SDLS168), '384 (SDLS169), '882
  (SDAS235), '677/'679, '866, AS181 and others are TI sheets with TI
  literature numbers). What is true is that **TI publishes none of them**:
  `https://www.ti.com/product/<part>` → 404 and `lit/ds/<number>` → 404 for
  every one of those literature numbers. The design rests on "no TI
  datasheet", the rule plan-21 applied to the same kind of Rochester copies
  (`planning/plan-21-data/README.md` step 2), so the skips stand; the
  reason in `parts.tsv` is "no TI datasheet" (plan-21's wording) rather
  than "no datasheet (…viewer page)", and `skipped-parts.md` says the
  copies exist. The '275 stays the owner's stated exception (a DigiKey-hosted
  TI scan).
- **Stock re-check took 113 calls, not about 40** (88 base names + 25
  candidates with no base name; `--json` rather than `--list` to get
  status per part).
- **The '180 and the '182 add no part** (2026-11-07). TI's sheets (sdls071,
  sdls206) list only SN54 J/W/FK parts, none in stock; the in-stock SN74180N,
  SNJ54HC180J and SN74S182N are in no TI ordering table. Likewise only
  SNJ54LS181J of the '181s is in its table. So plan-22-C's `lookahead`
  model and the `parity` model's '180 EVEN/ODD inputs have no part: they are
  not built (plan-22-B/C Corrections), as plan-21 left its part-less models.
- **The '885, '286 and CD4585B need more than plan-22-B §3 wrote**: '286
  XMIT is active low (transmit while low) and PARITY ERROR flags an even
  total (sn74as286 p.1 table); the CD4585B gates A>B out with its A>B
  cascade input also for unequal words (cd4585b p.2 Fig. 4). B's `compare`
  takes `cascadeTable` rows with an optional `"when"` relation, and
  `parity` takes `bus.drives` and `errorOn` (plan-22-B Corrections).
- **The 4063B and 4585B truth tables omit cascade combinations**: the
  missing equal-word rows are read from each sheet's logic diagram
  (specs.md § 4063B, § 4585B).
- **The '275 sheet prints no truth table**: the model is its p.2 block
  diagram (five full adders), and C's test checks that netlist against the
  weighted sum for all 512 inputs (plan-22-C Corrections).
- **N22 = 107** (`add` rows), 16 devices; plan-22-B…E use it.

## Summary

A fixes plan-22's real population: the parts with a datasheet. The two big
parts the owner wanted have none, and that is recorded, not silently dropped.
