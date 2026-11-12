# plan-33-B: Capacitors — ordering-code schemes, the pick walk, skipped and obsolete parts

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-33-A

B decides every bucket. It writes each placed datasheet's part-number scheme
into `specs.tsv`, checks every ranked candidate against it, and walks each
bucket in `rank` order: the first candidate that passes the inclusion rule
(plan-33-A § 3 rule 3) is `add`; the ones ranked above it that failed are
`skip` or `out` with a reason; the ones below are `alt` (never added: the
owner's no-near-duplicates rule). It lists the skipped parts, records
all-obsolete PDFs, prunes PDFs no `add` row cites, and writes
`fpgroups.tsv` and `footprints-needed.txt` for C and D.

The single behavioral outcome: every bucket of `buckets.tsv` (1048) has
exactly one `add` row, or is listed in `buckets-empty.tsv` with the reason
none of its parts passed; no row is `candidate`.

References:

- plan-33-A § 3 (rules), § 2 (the measured populations).
- `planning/plan-29-data/census/scheme-check.py`, `decide.py`,
  `dead-scan.py`, `footprint-lines.py` (the scripts B adapts) and
  plan-29-B § 4.5 (a regex built only from what the datasheet prints).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).

## Prerequisites

plan-33-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-33-A complete | `ls planning/completed/plan-33-A-*.md` → exists | MET (2026-11-11, at execution: `planning/completed/plan-33-A-capacitor-data.md`, a7ddd66) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `specs.tsv`: every row has `mpn_regex`, `cap_group` and `volt_map` filled
  (`awk -F'\t' 'NR>1 && ($4=="" || $5=="" || $7=="")' specs.tsv | wc -l` → 0).
- `candidates.tsv`: no `candidate` row; per bucket exactly one `add` or the
  bucket in `buckets-empty.tsv` (`awk -F'\t' '$25=="add" {print $1}'
  candidates.tsv | sort -u | wc -l` + `awk 'END{print NR-1}'
  buckets-empty.tsv` → 1048, and `… | sort | uniq -d | wc -l` → 0).
- `datasheets/skipped-parts.md` has a "Capacitors (plan-33)" section with
  every `skip` row; `obsolete-not-included.md` a row per all-obsolete PDF.
- `datasheets/capacitor/` holds exactly the PDFs the `add` rows cite.
- `fpgroups.tsv` and `footprints-needed.txt` list the `add` rows' footprint
  groups, each tagged C or D.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- A scheme regex uses only what its datasheet prints (plan-29-B § 4.5);
  a DigiKey-only suffix or packaging code is not in the table.

## 2. Current State

After A: `candidates.tsv` (all 1048 buckets, `rank`, `pdf`, `hit`,
`dead` for rank ≤ 10), `specs.tsv` (one row per placed PDF, scheme columns
empty), `datasheets/capacitor/<maker>/`. At planning time the ceramic
rank-1 rows of the 273 listed buckets cited 15 distinct datasheets and the rank ≤ 10 rows
52 (plan-33-A § 2); no sampled part was printed verbatim
(plan-33-A § 2 Verified properties), so nearly every decision goes through
a scheme.

## 3. Design

### 3.1 Scheme columns (`specs.tsv`)

| Column | Value |
|---|---|
| `mpn_regex` | a Python regex the whole part number must match, with named groups `cap` and `volt`, built from the datasheet's part-number guide (e.g. Cal-Chip `GMC-Series-3.pdf` "PART NUMBER GUIDE" p.2: `GMC(?P<case>01\|02\|04\|10\|21\|31\|32\|40\|43\|45\|55\|57)(?P<diel>CG\|X7R\|X5R\|X7S\|X7T\|X6S\|Y5V\|Z5U)(?P<cap>[0-9R]{3})(?P<tol>[ABCDFGJKMZ])(?P<volt>4R0\|6R3\|10\|16\|25\|35\|50\|63\|100\|200)(?P<term>NT\|PT)(?P<pack>D\|G\|Q)?`, each alternative printed in the guide) |
| `cap_group` | how `cap` decodes: `eia` (two digits then a multiplier digit, pF; `R` the decimal point: `104` = 100000 pF, `1R0` = 1 pF; also the µF codes of electrolytics, `107` = 100 µF) or `map:<code>=<farads>;…` when the datasheet prints a table |
| `volt_group` | `map` |
| `volt_map` | the datasheet's voltage codes, `5=50;3=25;4=16;8=10` (KEMET MLCC), `6R3=6.3;10=10;…` (Cal-Chip) |
| `case_group` | optional, the case code group's map when a datasheet covers several package sizes; checked against DigiKey's `package` |
| `note` | page of the guide, e.g. `PART NUMBER GUIDE p.2` |

### 3.2 The check (`census/scheme-check.py`)

A row passes rule 3 when its `pdf` is a placed maker-site PDF and either
`hit` = 1, or `mpn_regex` full-matches its part number and the decoded
`cap` equals the bucket's capacitance (3 significant digits) and the decoded
`volt` equals DigiKey's rated voltage (and `case`, when the row has a
`case_group`, equals DigiKey's package). The result goes in a new column
`check` (`ok`, or the first failing test: `no maker pdf`, `regex`, `cap`,
`volt`, `case`).

### 3.3 The walk (`census/decide.py`)

Per bucket in rank order: the first row with `check` = `ok` and no datasheet
status mark (`dead` read as plan-29-A § 3 rule 5, verdicts in
`census/dead-verdicts.tsv`) → `add`. Rows ranked above it: `skip` (reason
`no datasheet on the maker's site (<host>)` or `not in the datasheet's
ordering table (<test>)`) or `out` (`datasheet status: <word>`). Rows
below: `alt`. A bucket whose first 10 ranks all fail: A's downloads and
scans run for its ranks 11–50, then the walk again. A bucket whose 50 all
fail and whose `count` is above 50: `census/bucket-more.mjs` lists its next
pages (`Offset` 50, 100, … 250; one call each; the 300-result cap is never
passed) in stock order, and the walk continues there. A bucket with no
passing part: `buckets-empty.tsv` (bucket, rows tried, the reasons).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Schemes

- [x] For each `specs.tsv` row, read the PDF's part-number guide and ratings
      table (`pdftotext -layout`) and write § 3.1's columns. A datasheet
      with no guide or table that decodes its part numbers gets
      `mpn_regex` `NONE` and its rows fail with `regex` (recorded in
      § Corrections with the PDF name). Done in walk order
      (`census/specs-needed.py`, § Corrections 1): the schemes of 100 PDFs
      in `census/specs-fill.py`; one `NONE` (`C114K101K1X5CA.pdf`, a
      one-page spec sheet whose part is printed verbatim). The 102 PDFs no
      `add` row cites left with `place.py --prune` and their spec rows with
      `census/specs-prune.py` (Phase 3).

Acceptance: every spec row is filled.
  Check: `awk -F'\t' 'NR>1 && ($4=="" || $5=="" || $7=="")' planning/plan-33-data/specs.tsv | wc -l` → 0 (est. 3 min per PDF; the reading is the work).
  Observed (after Phase 3's prune): `0`; `census/specs-prune.py` → `102 rows dropped, 84 kept, 0 kept rows without a scheme`.
Commit: (this commit; hash in the next)

### Phase 2 — Check and walk

- [x] `census/scheme-check.py`, `census/dead-scan.py` (from plan-29),
      `census/decide.py`, `census/bucket-more.mjs` per § 3.2–3.3; run them
      until every bucket is decided. Observed: `scheme-check.py` → `3 cap,
      105 no maker pdf, 5786 ok, 686 regex, 41 table, 17 volt`;
      `dead-scan.py` → `132 PDFs read, 13 with a status word` (verdicts in
      `census/dead-verdicts.tsv`); `decide.py` → `1034 add, 17007 alt, 9
      out, 61 skip`, no `deeper:` bucket. `bucket-more.mjs` was not written:
      moot — every bucket left without a pick lists all its DigiKey parts
      (`count` = listed products for all 14: 1, 1, 1, 1, 4, 1, 2, 1, 2, 1,
      2, 1, 1, 1), so there is no further page to list.
- [x] `buckets-empty.tsv`; record the counts of each `decision` and the
      empty buckets per type in § Summary (`N_F` = ceramic `add` rows,
      `N_G` = the rest).

Acceptance: § 1's second bullet.
  Check: the two `awk` commands of § 1 → 1048 and 0 (est. 10 min of runs).
  Observed: 1034 + 14 = 1048; `uniq -d` → 0; no `candidate` row.
Commit: (this commit; hash in the next)

### Phase 3 — Skipped, obsolete, pruned

- [x] `datasheets/skipped-parts.md`: "## Capacitors (plan-33)", a short
      intro (the bucket rule; only parts ranked above a bucket's pick were
      tried), one table row per (maker, reason): the part numbers, "would
      have gone into `capacitor` / Capacitors → <type>", the reason.
      `decide.py --verify-skipped` → `0 skip rows missing from
      skipped-parts.md`. Observed: `61 skip rows written`; `0 skip rows
      missing from skipped-parts.md`.
- [x] `datasheets/obsolete-not-included.md`: a row per PDF whose every tried
      row is `out` for datasheet status (PDF name, `capacitor`, Capacitors →
      <type>, the word and page, the URL); delete those PDFs. Five KEMET
      safety-disc PDFs (`C901U102MYVDAAWL30`, `C901U100JZSDAAWL20`,
      `C971U472MVWDCAWL50`: "OBSOLETE" on every page; `C901U331KVYDBAWL45`,
      `C971U472MUWDBAWL40`: one-part sheets marked "Obsolete"); deleted by
      the prune below.
- [x] `census/place.py --prune`: delete every PDF no `add` row cites;
      `--verify` → `0 missing, 0 unused`. Observed: `102 PDFs deleted, 84
      kept`; `85 URLs, 84 PDFs, 0 missing, 0 unused`.

Acceptance: the tree holds exactly the `add` rows' PDFs and every skip is listed.
  Check: `ls datasheets/capacitor/*/*.pdf | wc -l` equals `awk -F'\t' '$25=="add" {print $27}' planning/plan-33-data/candidates.tsv | sort -u | wc -l`; `decide.py --verify-skipped` → `0 …` (est. 2 min).
  Corrected check (§ Corrections 4): `ls datasheets/capacitor/*/*.pdf | wc -l` → 84 equals the distinct placed paths of the `add` rows' URLs (`place.py --verify` → `85 URLs, 84 PDFs, 0 missing, 0 unused`: two URLs are one file); `0 skip rows missing`.
Commit: (this commit; hash in the next)

### Phase 4 — Footprint groups

- [x] `census/footprint-lines.py`: `fpgroups.tsv`, one row per (maker,
      type, DigiKey package, size, lead spacing, size code, PDF) of the
      `add` rows, columns `letter maker type package size spacing size_code
      height pdf rows footprint part_package pinout note` (`letter` C for
      chips: 0402–1206 MLCC, molded tantalum / polymer / niobium, the 2917
      aluminum polymer; D for the rest; the last four columns empty for C
      and D to fill); `footprints-needed.txt`, the same groups one per line.
- [x] Record the C and D line counts in § Summary; they replace the
      planning-time proxies of C § 2 and D § 2.

Acceptance: every `add` row is in a group.
  Check: `awk -F'\t' 'NR>1 {s+=$10} END{print s}' planning/plan-33-data/fpgroups.tsv` equals the `add` row count (est. 1 min).
  Observed: 1034 = 1034 (`302 groups (C 88, D 214), 1034 rows`).
Commit: (this commit; hash in the next)

## Validation Plan

- Tests: none (data only); `--verify` modes.
- Coverage check: § 1's 1048 equality.
- Runtime proof: not applicable.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate (once, after Phase 4; observed `100% tests passed, 0 tests failed out of 31`): `ctest --test-dir /tmp/logic-p33` →
  `100% tests passed` (est. 1 min; nothing the tests read changed).

## Decisions

- **Decoded scheme match counts as "in the ordering table"** — author:
  every sampled datasheet prints a part-number guide, not a list (plan-33-A
  § 2); plan-29-B § 4.5's regex method, plus the decoded value and voltage
  equal to DigiKey's, so a DigiKey listing error cannot slip a wrong value
  into a bucket.
- **Only parts ranked above the pick are tried and listed as skipped** —
  author: the rest were never candidates for inclusion under the one part
  per bucket rule; listing them would fill `skipped-parts.md` with
  near-duplicates.

## Corrections

1. **Schemes in walk order.** Reading all 186 placed PDFs would have read
   102 that no pick needs (they are pruned in Phase 3). `census/specs-needed.py`
   walks every bucket as `decide.py` does and lists the PDFs whose scheme
   decides the next row; schemes were written until it printed `0 PDFs block
   0 buckets`. A PDF reached only through verbatim hits still got its scheme
   when an `add` row cites it (9 of them), so every kept spec row is filled.
2. **Tables print placeholders.** Most ordering guides leave fields as "see
   table" (KEMET size codes, AVX ESR digits), so a regex alone would accept
   any shape there. Where the datasheet's ratings table prints the part
   numbers, the scheme also needs the part printed there: group `base`
   (`ESK107M016AE3(1)`), or pieces `b1 b2 b3` around a placeholder (AVX
   `TAJA475*006#NJ`, KEMET `T491A105(1)010A(2)`), with KEMET's `(n)` read as
   a gap (`scheme-check.py`, test `table`; 41 rows fail it, e.g.
   `ESW226M010AC3AA`, absent from `KEM_A4009_ESW` Table 1).
3. **EIA codes 8 and 9 are powers of ten** (`338` = 3300 µF): KEMET's MLCC
   small-value rule (9 = ×0.1) applies only below 10 pF, which no bucket
   holds. Cap group `eiauf` (µF codes: AVX and Cal-Chip aluminum) and `phx`
   (KEMET PHH/PHV: first digit = digit count) were added.
4. **Phase 3's check used column 27** (`pdf`, the 0/1 flag), not the
   datasheet; the PDF count is the distinct placed paths of the `add` rows'
   URLs (84; 85 URLs, two of them one file).
5. **Status marks read from font colour.** KEMET T52X and SSD mark NRND
   parts "in blue/orange font"; `pdftohtml -xml` gives each text run's colour
   (`#00aeef`, `#ee3824`): 6 + 6 part numbers, verdict `out` in
   `dead-verdicts.tsv`; the rest of each PDF `add`. PHH/PHV "End of Life
   Requirement" (a test limit) and T59X "End of Life Vehicle" (a directive)
   are not status marks.
6. **AC-rated safety discs.** KEMET C7/C9 safety discs are rated in VAC
   (X1 400 / 440 / 760); their voltage letter maps to that number, which is
   DigiKey's `Voltage - Rated`, so a disc's `volts` is its X1 AC rating.
7. **`scheme-check.py` re-checks every scanned ranked row** (not only
   `candidate` / `add`), so a scheme written after a walk re-decides rows the
   walk had skipped (2 `CGP` rows had been skipped before their scheme
   existed).

## Summary

Decisions (2026-11-11): 1034 `add`, 17007 `alt`, 61 `skip`, 9 `out`
(2 bi-polar before ranking, 7 datasheet status). **`N_F` = 377** (ceramic
`add` rows), **`N_G` = 657** (aluminum 188, aluminum polymer 137, tantalum
228, tantalum polymer 84, niobium oxide 20). Makers: KEMET 507, Cal-Chip
288, KYOCERA AVX 239; 9 automotive; 1001 picks are rank 1. Empty buckets
(14, `buckets-empty.tsv`): ceramic 1 (Disc 22 pF), aluminum 11, aluminum
polymer 1, tantalum polymer 1. Footprint groups (`fpgroups.tsv`): **C 88**,
**D 214** (302).

B's risk is a wrong regex taking a part outside its datasheet's table; the
decoded capacitance and voltage checks catch a regex that is too loose on
the two fields that define a bucket.
