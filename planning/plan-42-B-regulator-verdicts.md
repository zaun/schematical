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
| plan-42-A complete | `ls planning/completed/plan-42-A-*.md` → exists | NOT MET (2026-11-15: not started) |
| A's outputs exist | `ls planning/plan-42-data/stock-check.tsv planning/plan-42-data/specs.tsv datasheets/regulator/datasheets.tsv` → three files | NOT MET (2026-11-15: none) |

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

- [ ] `census/specs.py`, `scheme-check.py`, `specs-fill.py`,
      `need-groups.py`, `mark-unread.py` (plan-40's, adapted: the regulator
      columns, the three makers). Smoke run on the skeleton:
      `scheme-check.py --maker "Texas Instruments"` prints its count line.
- [ ] Read each TI PDF's Package Option Addendum (`pdftotext -layout`); fill
      `scheme` (only where a row is not verbatim) and `label` for every TI
      spec row the pick can reach (`need-groups.py`: rows at or above the
      floor, or in a short share); `specs-ti.tsv`, merged by `specs-fill.py`.

Acceptance: every TI download-set row has a scheme or is a skip.
  Check: `python3 -I planning/plan-42-data/census/scheme-check.py --maker "Texas Instruments"` → `0 download-set rows without a scheme` (est. 1 min; the reading is the work).
Commit: —

### Phase 2 — Analog Devices (with Maxim)

- [ ] As Phase 1 for the ADI PDFs (Ordering Guide; Maxim-origin
      "Ordering Information" with `+`/`+T` suffixes); `specs-adi.tsv`.

Acceptance: as Phase 1.
  Check: `scheme-check.py --maker "Analog Devices"` → `0 download-set rows without a scheme` (est. 1 min).
Commit: —

### Phase 3 — Microchip (with Micrel)

- [ ] As Phase 1 for the Microchip PDFs (Product Identification System
      schemes; Micrel-origin "Ordering Information" tables); `specs-microchip.tsv`.

Acceptance: as Phase 1.
  Check: `scheme-check.py --maker "Microchip Technology"` → `0 download-set rows without a scheme` (est. 1 min).
Commit: —

### Phase 4 — Datasheet status, verdicts, the final pick

- [ ] Read every download-set row with `dead` = 1 in its PDF (the text scan's
      flag) and confirm or clear it by hand (`dead-<maker>.tsv`: `mpn`,
      `dead`, `page`, `words`; merged by `dead-merge.py`); record each PDF
      whose download-set rows are all dead.
- [ ] `census/verdicts.py` (§ 3.2): `verdicts.tsv`; `select.py --verdicts`
      (plan-40's option, adapted); re-run the pick to the final
      `candidates.tsv`.
- [ ] Hand-check one `add` row per distinct `scheme` against its PDF's
      ordering table (row, page, result) in § Corrections.

Acceptance: the verdicts and the pick hold, and the named parts are in.
  Check: `awk -F'\t' '$29=="add"' planning/plan-42-data/candidates.tsv \| wc -l` → N (recorded here; at most 600); `python3 -I planning/plan-42-data/census/select.py /tmp/p42/raw /tmp/p42/cand.tsv --stock planning/plan-42-data/stock-check.tsv --verdicts planning/plan-42-data/verdicts.tsv --report` → exits 0 with the `add` per (family, maker) lines; `for n in LM1117 TLV1117 LM317 LM340 LM7805 UA7805 LM2576 LM2596 MCP1700 MCP1702; do echo $n $(awk -F'\t' -v n=$n '$29=="add" && toupper($8)==n' planning/plan-42-data/candidates.tsv \| wc -l); done` → each at least 1, a 0 written in § Corrections with its rows' verdicts (est. 3 min).
Commit: —

### Phase 5 — Skips, obsolete PDFs, footprint lines, prune

- [ ] `datasheets/skipped-parts.md`: a `## Voltage regulators (plan-42)`
      section, one table row per (device or series, reason) group of the
      `skip` rows at or above their (family, maker)'s floor, in the file's
      four-column format (`Part(s) | Device | Would have gone into | Why
      skipped`; "Would have gone into" `regulator` / `Power → Regulators →
      Linear` or `… → Switching`); `verdicts.py --write-skipped` prints the
      counts, recorded here, and `verdicts.py --verify-skipped` → `0 skip
      rows missing from skipped-parts.md` (plan-42-J re-runs it).
- [ ] `datasheets/obsolete-not-included.md`: one row per all-dead PDF (`PDF |
      Family | Category / group | Notes`: `regulator`, the drawer path, the
      datasheet's wording and page, the URL); delete those PDFs.
- [ ] `census/footprint-lines.py planning/plan-42-data/candidates.tsv >
      planning/plan-42-data/footprints-needed.txt` from the `add` rows;
      record the per-letter line counts (planning: D 69, E 63, F 48).
- [ ] `census/prune.py`: delete every placed PDF no `add` row cites, rewrite
      `datasheets.tsv`; `place.py --verify` → `0 missing, 0 unused`; commit
      `datasheets/regulator/`.

Acceptance: the tree holds exactly the datasheets the parts will cite.
  Check: `python3 -I planning/plan-42-data/census/place.py --verify` → `0 missing, 0 unused`; `grep -c '^| ' datasheets/skipped-parts.md` grew by the recorded row count; `du -sh datasheets/regulator` recorded here (est. 2 min).
Commit: —

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

## Summary

B is reading work: three makers' ordering tables and status wording for
about 633 (series, package) rows. The risk is a loose scheme regex letting a
part in; the full-match rule and one hand check per scheme guard it. Nothing
outside `planning/plan-42-data/` and `datasheets/` changes.
