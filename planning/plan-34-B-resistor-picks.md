# plan-34-B: Resistors — ordering-code verdicts, the final pick, skipped parts

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-34-A

B settles which eligible rows are in their maker datasheet's ordering
table, then runs the final pick. For each `specs.tsv` row (one per maker,
series and PDF) the reader writes the datasheet's ordering-code regex and
the field check; `census/verdicts.py` turns that into a verdict for every
eligible row; `select.py --verdicts` re-runs § 3 rules 3–4 of plan-34-A on
the rows that pass, so a skipped part's slot goes to the next one. The
outcome: `candidates.tsv` has `add` for every picked row (about 2000), the
rest `out` or `skip` with a reason; the skipped parts a pick would have
taken are in `datasheets/skipped-parts.md`; `footprints-needed.txt` is
regenerated from the `add` rows for C and D.

References:

- plan-34-A § 3 (the selection rules), § 5 (the stock check), § 6 (scripts).
- `planning/completed/plan-29-B-diode-footprints-vishay-single.md` § 4.5
  (settling pattern ordering tables with a regex: the precedent).
- `planning/plan-34-data/specs.tsv`, `candidates.tsv`, `stock-check.tsv`
  (plan-34-A).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.

## Prerequisites

plan-34-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| Phase 1: `specs.tsv` skeleton exists | `test -s planning/plan-34-data/specs.tsv && echo ok` → `ok` | MET (2026-11-13 at abb3652: `ok`, 109 rows) |
| Phase 2 onward: plan-34-A complete | `ls planning/completed/plan-34-A-*.md` → exists | NOT MET (2026-11-13: A Phase 2's stock check runs over about two more quota days; § Corrections 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `specs.tsv` row has a non-empty `mpn_regex` and `fields_checked`.
- Every eligible row of `candidates.tsv` (7531 at planning time) with a
  PDF or a no-datasheet reason has a verdict in
  `planning/plan-34-data/verdicts.tsv`: `add` (printed verbatim, or a full
  regex match whose fields agree with DigiKey's), `skip` or `out`, with a
  reason.
- `python3 -I census/select.py /tmp/p34/raw planning/plan-34-data/candidates.tsv
  --stock planning/plan-34-data/stock-check.tsv --verdicts
  planning/plan-34-data/verdicts.tsv --report` runs clean: every picked row
  is `add`; the counts are chip 1500, through-hole 400, arrays all their
  passing rows (≤ 100); every picked row from a cut piece is in
  `stock-check.tsv` with `found` = 1.
- `datasheets/skipped-parts.md` has a "Resistors (plan-34)" section listing
  every `skip` row whose stock is at or above its category's floor (the
  parts the pick would otherwise have taken).
- `datasheets/obsolete-not-included.md` has a row for each PDF whose every
  row is `out` for datasheet status; that PDF is deleted.
- `footprints-needed.txt` lists the `add` rows' land-pattern lines (86 at
  planning time, plan-34-A § 2).

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- A regex never admits a code the datasheet does not print as a choice of
  its field (a packing code, reel code or TCR code only DigiKey shows is
  `skip`, as plan-29-B § 4.5).
- No new DigiKey listing: only the part-by-part check of newly picked rows
  from cut pieces (plan-34-A § 5 rule 2).

## 2. Current State

At planning time 2 of the 1994 picked part numbers are printed verbatim in
their PDFs (plan-34-A § 2); every other row needs a regex. The picked rows
cite 41 PDFs, the eligible rows 85 (§ 2 of A). The series of the picked
rows (50: `awk -F'\t' '$22=="candidate"{print $2"|"$18}' planning/plan-34-data/candidates.tsv | sort -u | wc -l` → 50; 89 for the eligible rows) are mostly
YAGEO `RC_L` (1168 picked), `MFR` (237), `FMP` (57), `MF0` (29), `RSF` (27)
and Vishay Dale `CRCW-C` (147), `*` (96, DigiKey's empty series),
`CRCW0201 e3` (44). YAGEO's chip sheets print "GLOBAL PART NUMBER RC XXXX X
X X XX XXXX L" with each field's codes listed (§ 2 of A, Verified
properties); Vishay's CRCW sheets print the "GLOBAL PART NUMBER
INFORMATION" boxes (`CRCW0603` + value + tolerance + TCR + packaging).

## 3. Design

### 3.1 The regex and the field check

For each `specs.tsv` row the reader reads the PDF's ordering information
(`pdftotext -layout`, the page cited in `note`) and writes:

- `mpn_regex`: an anchored regex over the whole part number, one
  alternation per field, built only from the codes the PDF prints for that
  field (YAGEO RC: `^RC(0201|0402|0603|0805|1206|1210|1218|2010|2512)(F|J)(R|K)-(07|10|13|7W|7D)([0-9RKM]{1,5})L$`
  — the exact alternation from the PDF; the value field as the PDF
  defines it).
- `fields_checked`: the named groups the regex captures and the DigiKey
  column each is checked against: size → `package`, tolerance letter →
  `tolerance`, value code → `ohms` (decoded the PDF's way: `4K7` = 4700,
  `100R` = 100, `0R` = 0). A row whose fields disagree with DigiKey's is
  `skip`, reason `ordering code disagrees with DigiKey (<field>)`.

### 3.2 Verdicts

`census/verdicts.py <candidates.tsv> <specs.tsv> <verdicts.tsv>` (written in
Phase 1): for every eligible row with a PDF: `hit` = 1 → `add`; else the
spec row's regex and field check → `add` or `skip`; a row with no PDF →
`skip`, reason `no datasheet on the maker's site` with the link's host
(plan-34-A § 3 rule 5). `dead` = 1 rows are read in their PDF (plan-29-A
§ 3 rule 5) and recorded in `census/dead-verdicts.tsv` (`mpn`, PDF, page,
word, `out` or `add`), which `verdicts.py` reads: a row read as `out` gets
verdict `out`, reason `datasheet status: <word>`. Verdicts are `add`,
`skip` or `out`; `select.py` treats `out` like `skip` (never picked) but
writes decision `out` (Phase 1 adds that branch).

### 3.3 The final pick

`select.py --stock … --verdicts …` (A § 6): rows with verdict `skip` are
`skip` and never picked; a datasheet-status row is `out`; then rules 3–4 as
in A. Newly picked rows from cut pieces that are not in `stock-check.tsv`
are checked with `census/stock-check.sh` (A § 5 rule 2) and the pick is
re-run until none is unchecked.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Schemes

- [x] `census/select.py`: verdict `out` → decision `out` with its reason
      (§ 3.2).
- [x] Write `census/verdicts.py` (§ 3.2) and `census/scheme-check.py
      <specs.tsv> <candidates.tsv> <spec row>`: prints, for one spec row,
      the regex's matches and non-matches among its eligible rows, so the
      reader can see a regex that is too tight or too loose. The field
      check is `census/scheme.py` (named groups `size`, `tol`, `rkm`,
      `eia3`, `eia4`, `jumper`; a `sizemap=` for size codes that are not
      the imperial size, read from the PDF's dimension table). A size agrees
      when it is a whole token of DigiKey's Package / Case (`1218` in `Wide
      1812 (4532 Metric), 1218`). `scheme-check.py` also takes a trial
      regex and fields_checked.
- [x] Fill `mpn_regex`, `fields_checked` and `note` (PDF page of the
      ordering information) for every `specs.tsv` row, most-picked series
      first (`RC_L`, `MFR`, `CRCW-C`, `*`, `FMP`, …). 109 rows, read by four
      parallel readers, each regex run through `scheme-check.py`. Results
      (match / disagree / no match): RC_L 4054/0/0, MFR 1109/0/1 (a jumper
      with tolerance `-`, not printed), CRCW-C 711/0/0, CRCW `*` 311/0/0,
      FMP 278/0/0, CRCW0201 e3 621/0/0, RT 734/0/0, RC_P 351/0/0, RSF
      525/0/1. Every no-match uses a code its PDF does not print (e.g. AC
      and YC reel `10`, Viking CR value `1M`), or its PDF has no ordering
      information for the series: the YAGEO mounting guide
      `yageo_PYu-R_Mount_10_19050818_343.pdf` (AF, AF122, AF164, RT: 14
      rows), Vishay `dectable.pdf` (25), `cmfmil.pdf` for CW, `rsns.pdf`
      for CW. Those rows have `(?!)`, `none` and the reason in `note`.
      Disagreements: MMF25S ×2 (DigiKey MELF 0207, the PDF 0204),
      PNP1WVJR-52-5R1 (code 5.1 Ω, DigiKey 5 Ω).
- [x] ~~Read every `dead` = 1 row (§ 3.2) into `census/dead-verdicts.tsv`.~~
      — moot: no row is `dead` (`awk -F'\t' 'NR>1 && $25=="1"'
      planning/plan-34-data/candidates.tsv | wc -l` → 0, textscan.py: `0
      dead`); `verdicts.py` reads the file only when it exists.

Acceptance: every spec row has a scheme.
  Check: `awk -F'\t' 'NR>1 && ($4=="" || $5=="")' planning/plan-34-data/specs.tsv | wc -l` → 0 (est. 3–4 h of reading, 89 series; nothing smaller reads the ordering tables).
  Result (2026-11-13): 0.
Commit: 6556935

### Phase 2 — Verdicts and the final pick

- [x] `python3 -I census/verdicts.py planning/plan-34-data/candidates.tsv
      planning/plan-34-data/specs.tsv planning/plan-34-data/verdicts.tsv`.
      2026-11-13: 11700 add; skip: 188 no datasheet on the maker's site
      (mm.digikey.com), 7 (none), 4 (www.waldom.com), 1 (www.yageo.com), 83
      not in the datasheet's ordering table, 2 ordering code disagrees
      (size), 1 (value); 0 out. Verdicts do not depend on the stock check:
      it can only remove rows.
- [ ] `select.py --stock … --verdicts … --report` (§ 3.3); stock-check the
      newly picked rows from cut pieces (`census/stock-check.sh`, one call
      each); repeat until clean. Record the counts per decision and reason
      in § Summary. Provisional run (2026-11-13, stock check 172 of 1847
      done): `('add', ''): 1974`: chip 1500, through-hole 400, arrays 74.
      C, D, F and G ran on this pick. Remaining: the final run once A's stock
      check ends, written to `candidates.tsv`.

Acceptance: every picked row is `add` and confirmed.
  Check: `python3 -I planning/plan-34-data/census/select.py /tmp/p34/raw planning/plan-34-data/candidates.tsv --stock planning/plan-34-data/stock-check.tsv --verdicts planning/plan-34-data/verdicts.tsv --report` exits 0 and prints `kept per category/maker` summing to chip 1500, through-hole 400, arrays ≤ 100, and `awk -F'\t' '$22=="candidate"' planning/plan-34-data/candidates.tsv | wc -l` → 0 (est. 5 min plus any stock calls).
Commit: —

### Phase 3 — Skipped parts, obsolete PDFs, footprint lines

- [~] `datasheets/skipped-parts.md`: a "Resistors (plan-34)" section, one
      table row per (maker, reason) for the `skip` rows with stock at or
      above their category's floor: the part numbers, "would have gone into
      `resistor` / Resistors → <Chip|Through-hole|Array>", the reason (the
      existing sections' format). `verdicts.py --verify-skipped` prints
      `0 skip rows missing from skipped-parts.md`. Written on the
      provisional pick (`verdicts.py --write-skipped`, which replaces the
      section): `26 skip rows in 6 table rows`, verify `0 skip rows
      missing`. Remaining: re-write on the final pick.
- [~] Delete from `datasheets/resistor/` any PDF no `add` row uses
      (`place.py --prune`); add to `obsolete-not-included.md` each PDF whose
      every row is `out` for datasheet status (PDF name, `resistor`,
      Resistors → <leaf>, the word and page, the URL). Pruned on the
      provisional pick: 40 PDFs deleted, 32 left (the 32 the parts cite);
      `place.py --verify-add` → `0 missing, 0 unused`. No PDF is `out` for
      datasheet status (0 `dead` rows), so `obsolete-not-included.md` gets
      no row. Remaining: re-check on the final pick (a swap that needs a
      pruned PDF re-places it from the cache).
- [~] `python3 -I census/footprint-lines.py planning/plan-34-data/candidates.tsv
      > planning/plan-34-data/footprints-needed.txt` (now from `add` rows);
      write the per-letter line counts (C: chip; D: through-hole and
      arrays) into § Corrections when they differ from 34 and 52. On the
      provisional pick: 75 lines, chip 33, through-hole 29, arrays 13
      (§ Corrections 2). Remaining: re-run on the final pick.

Acceptance: the tree holds exactly the PDFs the `add` rows need, and every listed skip is recorded.
  Check: `python3 -I planning/plan-34-data/census/place.py --verify planning/plan-34-data/candidates.tsv` → `0 missing, 0 unused`, and `python3 -I planning/plan-34-data/census/verdicts.py --verify-skipped` → `0 skip rows missing` (est. 2 min).
Commit: —

## Validation Plan

- Tests: none (data only).
- Coverage check: Phase 2's `candidate` count of 0 shows every row is decided.
- Runtime proof: not applicable; no part is loaded.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate (once, after Phase 3): `cmake --build /tmp/logic-p34 -j8 &&
  ctest --test-dir /tmp/logic-p34` → `100% tests passed` (est. 1 min).

## Decisions

- **Verdicts on every eligible row, the pick after them** — author: a
  skipped part's slot goes to the next-stocked part, so the set stays at
  the quotas. Rejected: deciding only the 1994 planning picks (a skip
  would leave a hole).
- **Field check against DigiKey** — author: a regex match whose size or
  value code disagrees with DigiKey's columns is a wrong match, not a
  part; it is `skip`, not `add`.
- **skipped-parts.md lists only the skips a pick would have taken** —
  author: listing all skip rows of 7531 would bury the useful ones; a skip
  below the floor would not have been added anyway.
- Owner decisions: plan-34-A § Decisions.

## Corrections

1. **Phase 1 runs during A's stock check (2026-11-13).** Phase 1 reads
   only `specs.tsv` and the PDFs, which plan-34-A Phases 3–4 produced
   (abb3652). A Phase 2 (the part-by-part stock check) needs about two
   more quota days, and its outcome can only swap picked rows. So the
   gate is split: Phase 1 needs the skeleton, Phases 2–3 need A complete.
   A's added Phase 2 task re-runs the skeleton, which merges and keeps
   these schemes. A spec row it adds for a newly eligible group gets its
   scheme then.
2. **Line counts (2026-11-13, provisional pick).** `footprints-needed.txt`
   from the `add` rows: chip 33 (planning 34; no MELF), through-hole 29
   and arrays 13 (planning 34 + 18 = 52). Twenty arrays are skipped,
   because their only datasheet is a DigiKey copy (`AS42…`, `CN-21…`) or
   YAGEO's mounting guide (`AF122…`, `AF164…`: their DigiKey link is
   `yageo_PYu-R_Mount_10_19050818_343.pdf`, which has no ordering table).
   So arrays are 74 of their quota of 100.
3. **YAGEO AF / YC parts linked to the mounting guide (2026-11-13).**
   DigiKey links some AF122 / AF164 / YC124 / YC164 parts to YAGEO's
   mounting guide rather than their series sheet. The guide is on the
   maker's site, so § 3 rule 5 takes it as their datasheet; it has no
   ordering table, so they are `skip`. The series sheet
   (`PYU-AF122_124_162_164_51_ROHS_L.pdf`) covers them, but A's Non-goals
   allow no datasheet hunting beyond the link and the verbatim sibling
   check, and the sheet prints a pattern, not their part numbers. They
   are listed in `skipped-parts.md`.

## Summary

B's risk is a regex that admits a code the PDF does not print (a part
added that is not in the ordering table) or rejects a printed one (a
needless skip). `scheme-check.py`'s match / non-match listing per spec row
is the guard. Left untouched: every part, footprint, backend and app file.
