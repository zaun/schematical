# plan-42-B: Voltage regulators — ordering tables, datasheet status, verdicts, the final pick, skipped parts

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-42-A

B reads every download-set PDF that A placed for its ordering table and its
status wording, gives every download-set row a verdict (`add`, `skip`,
`out`), runs the final pick, writes the skipped parts and the obsolete PDFs,
regenerates `footprints-needed.txt` from the `add` rows and prunes the PDFs
to those an `add` row uses. The behavioral outcome: `candidates.tsv` has a
final `reason` for every row (`add` at most 600, every one in stock, in its
datasheet's ordering table and not marked dead by it), every `skip` group is
a row of `datasheets/skipped-parts.md`, and `datasheets/regulator/` holds
only PDFs that an `add` row cites.

References:

- plan-42-A (§ 3 selection rules, § 4 placement, § 5 stock check).
- `planning/completed/plan-40-B-transistor-verdicts.md` and
  `planning/plan-40-data/census/` (`specs.py`, `scheme-check.py`,
  `specs-fill.py`, `need-groups.py`, `mark-unread.py`, `dead-merge.py`,
  `verdicts.py`, `prune.py`, `select.py --verdicts --carry`: the scripts B
  adapts).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`
  (their table formats).
- `AGENTS.md` § Datasheets (in stock; obsolete PDFs; partly obsolete PDFs
  flag their obsolete parts).

## Prerequisites

plan-42-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-42-A complete | `ls planning/completed/plan-42-A-*.md` → exists | MET (2026-11-21: `planning/completed/plan-42-A-regulator-data.md`) |
| A's outputs exist | `ls planning/plan-42-data/stock-check.tsv planning/plan-42-data/specs.tsv datasheets/regulator/datasheets.tsv` → three files | MET (2026-11-21: all three exist) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `scheme-check.py --maker <m>` → `0 download-set rows without a scheme`
  for each of the three makers.
- `select.py … --stock … --verdicts planning/plan-42-data/verdicts.tsv
  --report` exits 0; `add` ≤ 600; each (family, maker) share full or its
  shortfall recorded in § Corrections with the rows that caused it.
- `datasheets/skipped-parts.md` has a `## Voltage regulators (plan-42)`
  section covering every `skip` row at or above its (family, maker)'s floor;
  `datasheets/obsolete-not-included.md` has one row per PDF whose
  download-set rows are all dead by the datasheet (and that PDF is deleted).
- `footprints-needed.txt` is regenerated from the `add` rows.
- `place.py --prune` leaves only PDFs an `add` row cites; the pruned tree
  and `datasheets.tsv` are committed.

### Non-goals (explicit constraints)

- No pinouts here (C) and no footprints (D–F); no change to `Parts/`,
  `backend/` or the app.
- No new DigiKey listing and no new stock look-up: the download set was
  checked part by part in A (plan-42-A § Decisions: refills come only from
  the download set).

## 2. Current State

- A leaves `candidates.tsv` with `pdf`, `hit`, `dead` and `stock_checked`
  filled for the download set, `specs.tsv` with one row per (maker, series,
  PDF, Package / Case) and empty `scheme` / `label`, and the placed PDFs
  uncommitted in `datasheets/regulator/`.
- At planning the download set has **633** (maker, URL, Package / Case)
  pairs: TI 207, Analog Devices 218, Microchip 208; the picked rows alone
  have **444** (TI 140, ADI 149, Microchip 155) (`awk -F'\t' 'NR==FNR{s[$1]=1;next}
  ($1 in s){print $2"|"$28"|"$12}' planning/plan-42-data/download-set.tsv
  planning/plan-42-data/candidates.tsv | sort -u | cut -d'|' -f1 | sort |
  uniq -c`; for the picks `$29=="candidate"`). A's `specs.tsv` row count is
  the real number (one series may share a URL).

### Verified properties

- **Each maker's ordering table has a stable name** (read at planning with
  `pdftotext -layout` in the three PDFs of plan-42-A § 2's download probe).
  TI: `lm1117.pdf` ends with "PACKAGE OPTION ADDENDUM" / "PACKAGING
  INFORMATION" whose first column is "Orderable part number"
  (`lm1117.txt:994–1003`); `LM1117MPX-3.3/NOPB` is printed 4 times. ADI:
  `ADP7118.pdf` has an "Ordering Guide" (contents: p.22) and prints
  `ADP7118ARDZ` 15 times. Microchip: `MCP1700-…-20001826E.pdf` has a
  "Product Identification System" page: output-voltage codes (`120 = 1.2V`
  … `500 = 5.0V`), tolerance `2`, temperature `E`, packages `MAY`, `MB`,
  `TO`, `TT`, and example names (`MCP1700T-3302E/TT` is not printed whole;
  `MCP1700T-3302E/T` and `MCP1700T-3302E/M` are) — so Microchip rows are
  matched by a scheme regex, TI and ADI rows mostly verbatim.

## 3. Design

### 3.1 `specs.tsv` columns B fills

| Column | Value |
|---|---|
| `scheme` | a full-match regex of the orderable names this (series, PDF, package) covers, from the datasheet's identification page, e.g. `MCP1700T?-(120\|180\|250\|280\|290\|300\|330\|500)2E/(TT\|MB\|TO\|MAY)`; empty when every row is printed verbatim |
| `label` | the series name the drawer and the generator use (`LM1117`, `TPS7A20`, `MCP1700`) |

`scheme-check.py --maker <m>` prints, per maker, `<n> rows with a PDF; <v>
verbatim, <s> by scheme, <x> in neither, <u> not read (n/r)` and `<k>
download-set rows without a scheme` (a row with a PDF, not verbatim, whose
spec row has no `scheme` and is not marked `n/r`).

### 3.2 Verdicts (`verdicts.py` → `verdicts.tsv`)

One row per download-set row: `mpn`, `verdict` (`add`, `skip`, `out`),
`reason`, `pdf`:

1. No PDF (A § 3 rule 5) → `skip`, `no datasheet on the maker's site
   (<host>)`.
2. PDF, part number printed verbatim or a full match of its spec row's
   `scheme` → may be `add`; else `skip`, `not in the datasheet's ordering
   table`.
3. `dead` = 1 confirmed by hand (`dead-*.tsv`) → `out`, `datasheet status`;
   when only some rows of a PDF are dead, those rows are `out` and the PDF
   stays (AGENTS.md: a partly obsolete PDF keeps its live parts; the
   `out` rows are never added, so no part carries `"flags": ["obsolete"]`
   from this plan).
4. Everything else → `add` candidate.

`select.py --verdicts` then re-runs duplicates and the quota over the rows
whose verdict is `add`, so a skipped winner's slot goes to its duplicate or
to the next row of its (family, maker) in the download set; rows outside the
download set are `out:below the pick (not read)`.

### 3.3 Where the risk is

A wrong scheme regex lets a part in that the datasheet does not list. The
mitigation is the full-match rule, `scheme-check.py`'s count of rows matched
by scheme per maker, and a hand check of one `add` row per scheme against
its PDF's table (Phase 4), recorded in § Corrections.

Rejected alternatives: (a) trusting DigiKey's datasheet link as proof of
orderability (a family PDF lists variants DigiKey stocks under other
suffixes, and some DigiKey part numbers are not in any table, plan-21's TI
lesson); (b) reading the maker's product page instead of the PDF (403 to
automated clients at microchip.com, A § 2).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Texas Instruments (with National and Burr Brown)

- [x] `census/specs.py`, `scheme-check.py`, `specs-fill.py`,
      ~~`need-groups.py`, `mark-unread.py`~~ (plan-40's, adapted: the regulator
      columns, the three makers). Smoke run on the skeleton:
      `scheme-check.py --maker "Texas Instruments"` prints its count line. Observed: `Texas Instruments: 291 rows with a PDF; 285 verbatim, 0 by scheme, 0 in neither, 0 not read (n/r)` / `65 download-set rows without a scheme` (all makers, before the reading). `need-groups.py` and `mark-unread.py` are moot: every non-verbatim spec group (151) was read, so no spec row is `n/r` (`scheme-check.py` → `0 not read (n/r)` for every maker); `census/auto-scheme.py` (new) proposes the schemes and the labels, see Corrections 2.
- [x] Read each TI PDF's Package Option Addendum (`pdftotext -layout`); fill
      `scheme` (only where a row is not verbatim) and `label` for every TI
      spec row the pick can reach (`need-groups.py`: rows at or above the
      floor, or in a short share); `specs-ti.tsv`, merged by `specs-fill.py`. Done for every TI spec row (not only the reachable ones): 6 TI groups were not verbatim; `auto-scheme.py` + the manual files `specs-b-manual.tsv` / `specs-b-manual2.tsv` filled them; `specs-fill.py` → `0 errors`.

Acceptance: every TI download-set row has a scheme or is a skip.
  Check: `python3 -I planning/plan-42-data/census/scheme-check.py --maker "Texas Instruments"` → `0 download-set rows without a scheme` (est. 1 min; the reading is the work). Observed 2026-11-21: `Texas Instruments: 297 rows with a PDF; 289 verbatim, 0 by scheme, 8 in neither, 0 not read (n/r)` / `0 download-set rows without a scheme`.
Commit: 44c18a29

### Phase 2 — Analog Devices (with Maxim)

- [x] As Phase 1 for the ADI PDFs (Ordering Guide; Maxim-origin
      "Ordering Information" with `+`/`+T` suffixes); `specs-adi.tsv`. Done (the ADI rows are in `specs-auto.tsv` and `specs-b-manual.tsv`).

Acceptance: as Phase 1.
  Check: `scheme-check.py --maker "Analog Devices"` → `0 download-set rows without a scheme` (est. 1 min). Observed: `Analog Devices: 300 rows with a PDF; 224 verbatim, 74 by scheme, 2 in neither, 0 not read (n/r)` / `0 download-set rows without a scheme`.
Commit: 44c18a29

### Phase 3 — Microchip (with Micrel)

- [x] As Phase 1 for the Microchip PDFs (Product Identification System
      schemes; Micrel-origin "Ordering Information" tables); `specs-microchip.tsv`. Done (the Microchip rows are in `specs-auto.tsv`, `specs-b-manual.tsv`, `specs-b-manual2.tsv`).

Acceptance: as Phase 1.
  Check: `scheme-check.py --maker "Microchip Technology"` → `0 download-set rows without a scheme` (est. 1 min). Observed: `Microchip Technology: 294 rows with a PDF; 171 verbatim, 92 by scheme, 31 in neither, 0 not read (n/r)` / `0 download-set rows without a scheme`.
Commit: 44c18a29

### Phase 4 — Datasheet status, verdicts, the final pick

- [x] Read every download-set row with `dead` = 1 in its PDF (the text scan's
      flag) and confirm or clear it by hand (`dead-<maker>.tsv`: `mpn`,
      `dead`, `page`, `words`; merged by `dead-merge.py`); record each PDF
      whose download-set rows are all dead. Done: `dead-merge.py` → `7` rows `dead` = 1 (TPS7101QDR, TPS72501KTTR, SM72238TDX-5.0/NOPB, TPS61032PWPR, MIC4724YMME, MIC5245-3.1BM5, MIC5245-3.3BM5); PDFs whose download-set rows are all dead: `MIC4724-Data-Sheet-Obsolete-Device-EOL.pdf`, `MIC5245.pdf` (listed in `obsolete-not-included.md`), and three TI family PDFs with one NRND package row each (Corrections 4).
- [x] `census/verdicts.py` (§ 3.2): `verdicts.tsv`; `select.py --verdicts`
      (plan-40's option, adapted); re-run the pick to the final
      `candidates.tsv`. Done: `verdicts.py` → `843 add`, `7 out datasheet status`, `8 + 1 skip no datasheet on the maker's site`, `41 skip not in the datasheet's ordering table` (900 rows); `select.py … --verdicts … --report` → `add total: 600`, every (family, maker) share full.
- [x] Hand-check one `add` row per distinct `scheme` against its PDF's
      ordering table (row, page, result) in § Corrections. Done: `scheme-handcheck.py` → `scheme-handcheck.txt`: 99 distinct (maker, scheme), 126 add rows by scheme; 71 with the page and line that print the row's name after an ordering heading; the other 28 read by hand (Corrections 5).

Acceptance: the verdicts and the pick hold, and the named parts are in.
  Observed 2026-11-21: `add` N = **600**; the named bases: LM1117 6, TLV1117 6, LM317 3, LM340 6, LM7805 2, UA7805 2, LM2576 6, LM2596 6, MCP1700 6, MCP1702 6 (each ≥ 1).
  Check: `awk -F'\t' '$29=="add"' planning/plan-42-data/candidates.tsv \| wc -l` → N (recorded here; at most 600); `python3 -I planning/plan-42-data/census/select.py /tmp/p42/raw /tmp/p42/cand.tsv --stock planning/plan-42-data/stock-check.tsv --verdicts planning/plan-42-data/verdicts.tsv --report` → exits 0 with the `add` per (family, maker) lines; `for n in LM1117 TLV1117 LM317 LM340 LM7805 UA7805 LM2576 LM2596 MCP1700 MCP1702; do echo $n $(awk -F'\t' -v n=$n '$29=="add" && toupper($8)==n' planning/plan-42-data/candidates.tsv \| wc -l); done` → each at least 1, a 0 written in § Corrections with its rows' verdicts (est. 3 min).
Commit: 44c18a29

### Phase 5 — Skips, obsolete PDFs, footprint lines, prune

- [x] `datasheets/skipped-parts.md`: a `## Voltage regulators (plan-42)`
      section, one table row per (device or series, reason) group of the
      `skip` rows at or above their (family, maker)'s floor, in the file's
      four-column format (`Part(s) | Device | Would have gone into | Why
      skipped`; "Would have gone into" `regulator` / `Power → Regulators →
      Linear` or `… → Switching`); `verdicts.py --write-skipped` prints the
      counts, recorded here, and `verdicts.py --verify-skipped` → `0 skip
      rows missing from skipped-parts.md` (plan-42-J re-runs it). Observed: `--write-skipped` → `43 skip rows in 10 table rows`; `--verify-skipped` → `0 skip rows missing from skipped-parts.md`; `grep -c '^| ' datasheets/skipped-parts.md` 692 → 703 (+11: 10 rows and the table header).
- [x] `datasheets/obsolete-not-included.md`: one row per all-dead PDF (`PDF |
      Family | Category / group | Notes`: `regulator`, the drawer path, the
      datasheet's wording and page, the URL); delete those PDFs. Two rows: `MIC4724-Data-Sheet-Obsolete-Device-EOL.pdf`, `MIC5245.pdf`; the PDFs are not in the tree after the prune.
- [x] `census/footprint-lines.py planning/plan-42-data/candidates.tsv >
      planning/plan-42-data/footprints-needed.txt` from the `add` rows;
      record the per-letter line counts (planning: D 69, E 63, F 48). Observed: D 68 lines (320 parts, 25 with no existing footprint of the same outline), E 64 (179 parts; 64 new), F 47 (101 parts; 47 new): 179 lines.
- [x] `census/prune.py`: delete every placed PDF no `add` row cites, rewrite
      `datasheets.tsv`; `place.py --verify` → `0 missing, 0 unused`; commit
      `datasheets/regulator/`. Observed: `prune.py` → `487 files; 357 used by add rows; keep 357, delete 130`, `datasheets.tsv: 387 lines`; `place.py --verify-add` (`--verify` checks the download set, Corrections 6) → `0 missing, 0 unused`.

Acceptance: the tree holds exactly the datasheets the parts will cite.
  Observed: `du -sh datasheets/regulator` → `480M` (357 PDFs).
  Check: `python3 -I planning/plan-42-data/census/place.py --verify` → `0 missing, 0 unused`; `grep -c '^| ' datasheets/skipped-parts.md` grew by the recorded row count; `du -sh datasheets/regulator` recorded here (est. 2 min).
Commit: 78ffc1bc

## Validation Plan

- Tests: none (data); the scripts' `--report` / `--verify` outputs are the
  checks.
- Coverage check: every download-set row has a verdict (`verdicts.py`
  exits non-zero on a row without one; `select.py --verdicts` refuses a
  missing verdict, as plan-40's).
- Runtime proof: n/a.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate: plan-42-J.

## Decisions

- Owner rules (2026-11-15, the plan request): a part must be in stock, in its
  datasheet's ordering table, Active or Last Time Buy; parts with no vendor
  datasheet (`ti.com/product/<PART>` 404, or only Rochester copies) go in
  `skipped-parts.md`; PDFs whose every part is obsolete go in
  `obsolete-not-included.md`.
- Author: **refills come only from the download set** (plan-42-A §
  Decisions); a share left short is recorded, not re-listed.
- Author: **dead rows are `out`, not added with a flag**: AGENTS.md asks for
  `"flags": ["obsolete"]` only on parts that are added; the owner's rule for
  this plan leaves out every obsolete, discontinued or NRND part.

## Corrections

1. **A's final data differed slightly from the plan text** (2026-11-21): the download set is 900 rows (two refills after the stock check, plan-42-A Corrections 3); `download-set.tsv` is now the fixed definition of the set (`common.init()` reads it; the `reason` column changes to `add` / `skip:` / `out:` and no longer defines it). The part-number scan counts: 881 download-set rows with a PDF at A; after the extra TI / Microchip rewrites of Corrections 3 below: 890 (683 verbatim), 10 without.
2. **`census/auto-scheme.py` replaces the by-hand pass for most groups** (new script): 593 spec rows (583 at A + 10 new groups after the rewrites); 151 had a row not printed verbatim. The script removes the maker's packing suffix (ADI / Linear `#TRPBF`, `#PBF`, `-R7`; Maxim `+T`, `+`; Microchip `-TR`, ` TR`, `T`; TI `/NOPB`, reel letter) and looks the shorter name up in the PDF text (token boundaries); 106 groups got `(?:<cores>)<suffix class>?` as scheme, the other 50 were read by a subagent (`specs-b-manual.tsv`, 46 groups: 35 regex from the datasheet's Product Identification System, 11 `-` = no ordering table supports the rows) and by me (`specs-b-manual2.tsv`, 4 groups, `-`). `label` = DigiKey's base part number when it is a clean device name, else the leading letters-and-digits run of the part number (every spec row has one; the generator in plan-42-H may refine a drawer name, not the label's key).
3. **The TI / Microchip rewrite of plan-42-A § 4 rule 3 / 4 was too narrow**: it tried the full device name and the leading letters-and-digits run (`lp8340ildx`), not the family (`lp8340`): `common.device_runs()` now also tries the name up to the first `-`, the letters + digits, and the digits cut to 3 (`tps79301` → `tps793`). After that 13 more PDFs downloaded (`urls.py` → 965 URLs, `fetch.js` → `965 urls; 567 are PDFs`) and 10 download-set rows have no PDF (TI Rochester copies whose product page answers 404: `LM3204TLX/NOPB`, `LM2788MM-2.0`, …; Micrel `MIC5303-1.8YMT-TR`). Rule 5 evidence for the TI ones: `curl -L https://www.ti.com/product/LM3204` and `…/LM2788` → HTTP 404 (3385 bytes, "Error 404"), and no `lit/ds/symlink/lm3204.pdf` / `lm2788.pdf` PDF (404 text/html).
4. **Datasheet status.** `dead-<maker>.tsv` (16 rows read): 7 confirmed dead (TI Package Option Addendum "NRND": `TPS7101QDR`, `TPS72501KTTR`, `SM72238TDX-5.0/NOPB`, `TPS61032PWPR`; Microchip: `MIC4724YMME` p.1 "The MIC4724 device is obsolete", `MIC5245-3.1BM5` and `-3.3BM5` p.1 "Not recommended for new designs, see MIC5255"), 9 cleared (the NRND / Obsolete word in a TI addendum belongs to a neighbouring orderable such as `REG102NA-A/250G4`; MIC2951 / MIC2954 only the TO-92 is discontinued; LT1129 "OBSOLETE PACKAGE" labels the TSSOP). The text scan only looked at the first occurrence of a part number; `TPS72501KTTR` was found by an all-occurrences scan (`part number` then `NRND|Obsolete|…` in the TI addendum), and a page 1–2 sweep of every placed PDF for family-wide wording (`not recommended for new`, `obsolete`, …) found only MIC4724 and MIC5245. `obsolete-not-included.md` lists the two PDFs whose every part is obsolete (`MIC4724-…-Obsolete-Device-EOL.pdf`, `MIC5245.pdf`); `slvs341e.pdf` (TPS725xx), `sm72238.pdf` and `tps61030.pdf` have all their *download-set* rows dead, but each is a family datasheet with other packages / parts that are not NRND, so they are not listed as all-obsolete and are simply pruned (no `add` row uses them).
5. **Hand checks** (`scheme-handcheck.txt`; 99 distinct (maker, scheme), 126 add rows by scheme). 71: the page and line of the PDF that prints the name after an ordering heading (e.g. `MAX38902CANT+T` → `MAX38902CANT+` p.16; `MIC5235-5.0YM5-TR` → `MIC5235-5.0YM5` p.2; `ADP1710AUJZ-2.5-R7` → p.14). 28 read by hand: LT / LTC old datasheets print "PACKAGE/ORDER INFORMATION" with the extraction garbled (`ORDER I FOR ATIO`) and the name in the "ORDER PART NUMBER" column, "Order Options Tape and Reel: Add #TR Lead Free: Add #PBF" (LT1615ES5-1, LT3027IDD, LT1930AES5, LT3464ETS8, LT1935ES5, LT1618EDD, LT1956EGN-5, LTC1844ES5-BYP: confirmed lines 84–104 of their text); pattern documents: `MCP1702T-1802E/CB` (22008E.pdf, Product Identification System p.23: voltage codes 12 … 50, tolerance 2, `E`, `CB` / `MB` / `TO`, examples `MCP1702T-1202E/CB`), `MCP1703T-3302E/DB` and `-5002E/CB` (p.30: `CB` SOT-23A, `DB` SOT-223, `VAO`), `MCP1825ST-3302E/DB`, `MCP1826ST-3302E/DB` (Product Identification, `S` variant, `DB`), `TC1262-3.3VDBTR` (`TC1262-xxVDB`, p.1 ordering table), `MIC94310-PYMT-TR` (voltage letters `4 F G D J L M N P S`, `YMT`, `TR` / `T5`), `MIC49150YMM-TR` (`MIC49150-0.9YMM`, `MM` = 8-lead power MSOP, `-TR`), `MIC5209-5.0YS-TR` (`MIC5209-x.xYS`), `MIC5205-5.0YM5-TR` (p.17: Y grade, `M5`, `TR` / `TX`, 2.5 … 5.0), `MIC5504-1.8YM5-TR` (p.19: per-device voltage list, `YM5`, `TR`), `MAX8559ETAAA+T` (`MAX8559ETAxy*+T`), `MAX77597ETBB+T` (`MAX77597ETBB+`, fixed 3.3 V), `ADP1713AUJZ-3.0-R7`, `ADPL44002AUJZ-5.0-R7`, `MAX8510EXK45+T` (`EXKxy+T`), `MAX5023LASA+T`, `MIC37150-1.8WR`, `MIC37151-1.8WR`, `MCP16331T-E/MNYVAO`, `MIC5205`. All confirmed. The B-grade (Micrel, non-RoHS) rows `MIC5205-…BM5`, `MIC5207`, `MIC5209…B…`, `MIC5233-3.0BM5`, `MIC37102BMTR` are not in the new Microchip PDFs (Y / W grade only): skipped, not in the ordering table.
6. **Commands corrected.** `place.py --verify` reads the download set, so after the prune the check is `place.py --verify-add` (the `add` rows' PDFs against the tree): `0 missing, 0 unused`. The plan-40 `need-groups.py` / `mark-unread.py` steps and the `pinout` check of `verdicts.py` are not part of B (pinouts are C's).
7. **Wrong-product PDFs found by the reading** (skipped as not in the ordering table, no change to the pick): `50001894.pdf` is Microchip's development-tools catalog (`MCP1257T-E/UN`, `TC1185-…`), `MCP16411-…-Evaluation-Board-Users-Guide-DS50003000A.pdf` is an evaluation-board guide (MCP16413 / 14 / 15 / 18), `lm317.pdf` does not list LM317M parts, `lp2985.pdf` (2025) lists only the SOT-23 DBV package.
8. **Shares.** Every (family, maker) share is full (`add linear-fixed … 80 of 80`, `linear-adjustable 30 of 30`, `linear-multi 10 of 10`, `switching 80 of 80`, for each of the three makers): 600 `add`, 597 Active + 3 Last Time Buy; `candidates from a cut piece: 164 of 600` (A's pick had 158). `LM317` has 3 add rows, not 6: DigiKey's base `LM317` lumps LM317, LM317L, LM317M and LM317A; the 6 most-stocked of the base were mostly LM317M / LM317T-LF01 parts that are not in `lm317.pdf`'s ordering table (3 skips) or in the download set's outside (`out:below the pick (not read)`), and the 3 added are LM317L parts (`LM317LIDR`, `LM317LIPK`, `LM317LCLP`); the classic TO-220 `LM317KCSE3` is outside the download set (stock below the share's floor) and stays out. `LM7805` has 2 and `UA7805` 2 as in the planning pick.

## Summary

B is reading work: three makers' ordering tables and status wording for
about 633 (series, package) rows. The risk is a loose scheme regex letting a
part in; the full-match rule and one hand check per scheme guard it. Nothing
outside `planning/plan-42-data/` and `datasheets/` changes.
