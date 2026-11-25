# plan-45-B: Terminal blocks and JST headers — ordering tables, status, facts, verdicts, the final pick

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-45-A

B reads what A placed and decides, row by row, what becomes a part. It
checks each candidate row against its datasheet (the printed ordering code,
the datasheet's own status), reads the facts the library keeps (pitch, rated
voltage and current, positions), records what is skipped and why, and writes
the final pick and the final `footprints-needed.txt` that C, D and E draw
from. Nothing in `Parts/`, `footprints.json`, the backend or the app changes.

References:

- plan-45-A § 3 (the inclusion rule), § 4 (placement), § 2 (the numbers).
- `planning/plan-45-data/candidates.tsv`, `specs.tsv`, `footprints-needed.txt`.
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- `planning/completed/plan-40-B-transistor-verdicts.md` and
  `planning/plan-40-data/census/` (`scheme-check.py`, `verdicts.py`,
  `specs-fill.py`, `pinout-check.py`, `dead-merge.py`: the scripts B adapts).

## Prerequisites

plan-45-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-45-A complete | `ls planning/completed/plan-45-A-*.md` → exists, or every A box ticked | MET (2026-11-24: not archived yet (`ls planning/completed/plan-45-A-*.md` → no match), but every A box is ticked: `grep -c '\[ \]' planning/plan-45-A-terminal-jst-data.md` → 0; A's commits 248d40f4, ced62f5e, febbaa58, 185524c9) |
| Every candidate row has a placed PDF | `awk -F'\t' '$22=="candidate" && $24==""' planning/plan-45-data/candidates.tsv \| wc -l` → 0 | MET (2026-11-24: 0; 513 candidate rows, 320 placed files) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every candidate row of `candidates.tsv` has `state` `add` or `skip` (a
  `skip` row has a `reason`), and no row stays `candidate`.
- `specs.tsv` has, for every (maker, variant, PDF): the ordering scheme (a
  regular expression over the part number), the rated voltage and current
  the datasheet prints (empty when it prints none), and the pin rule.
- `datasheets/skipped-parts.md` lists every skip group and every `skip` row;
  `datasheets/obsolete-not-included.md` lists every PDF whose parts are all
  obsolete (and the PDF is deleted).
- `planning/plan-45-data/generic-reps.tsv` names, for each of the six terminal
  pitches, the variant whose land pattern the generic parts of that pitch use,
  and for JST XH and PH their top-entry through-hole variant.
- `footprints-needed.txt` is rewritten from the `add` rows: C, D and E draw
  exactly those (variant, position count) pairs plus the generics' (§ 4).

### Non-goals (explicit constraints)

- No new listing and no DigiKey call.
- No datasheet hunting: a row with no usable PDF is `skip`.
- No footprint drawn: a row whose land pattern C, D or E cannot draw is made
  `skip` there, in that letter's § Corrections.

## 2. Current State

At planning time (2026-11-16) `candidates.tsv` has 521 `candidate` rows and
54 variants; `specs.tsv` does not exist (A writes its skeleton). The
datasheets are of two kinds, which sets the verdict rules:

| Maker | PDF | Rows per PDF | Ordering table |
|---|---|---|---|
| Phoenix Contact | per-item (DigiKey's copy; first page names the item and `https://www.phoenixcontact.com/us/products/<item>`) | 1 | the item is the part; its number is printed (`pdftotext 1935161.pdf` → 12 hits) |
| Weidmüller | per-item (`datasheet.weidmueller.com/pdf/en/<item>/scope/2/`) | 1 | the item number is printed (`pdftotext 1825640000.pdf` → 1 hit) |
| Würth Elektronik | per-item (`we-online.com/catalog/datasheet/<item>.pdf`) | 1 | the order code is printed (`pdftotext 691137710002.pdf` → 5 hits) |
| JST | per-series (`eXH.pdf` …), one per series and revision | up to 22 | the "Part No." table / the numbering rule (`B*B-XH-A` top entry, `S*B-XH-A` side entry) |

Würth and Phoenix part numbers can carry a trailing packing letter
(`691412320002M`, `691412120002B`: 14 of the 99 Würth candidates): the file
name on the maker site includes it.

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidate rows | 521 (Phoenix 75, Weidmüller 155, Würth 99, JST 192) | `awk -F'\t' '$22=="candidate"{n[$2]++} END{for(k in n) print k, n[k]}' planning/plan-45-data/candidates.tsv` |
| Rows whose part number ends in a letter (Würth 14) | 201 of 521 (JST's `-A`, `-K-S`, `-TB` endings included) | `awk -F'\t' '$22=="candidate" && $1 ~ /[A-Za-z]$/' planning/plan-45-data/candidates.tsv \| wc -l` |
| Distinct PDFs | 340 | `awk -F'\t' '$22=="candidate"{print $19}' planning/plan-45-data/candidates.tsv \| sort -u \| wc -l` |
| JST series PDFs | 11 URLs: `eEH`, `eGH`, `ePH`, `eSH`, `eVH`, `eXH`, `eZH` at jst-mfg.com (`eEH` and `eVH` once more with a query string) and two revisions of `eSR_SZ` at jst.com (the SH/SR series) | `awk -F'\t' '$22=="candidate" && $2=="JST"{print $19}' planning/plan-45-data/candidates.tsv \| sort -u` |

### Verified properties

- A per-item PDF prints its item: the three probes above (Phoenix 12,
  Weidmüller 1, Würth 5 hits for the part number).
- A Weidmüller per-item PDF has `Drawings` on its last pages and a `Solder
  eyelet hole diameter` line (plan-45-A § 2); a Phoenix PDF a `Drilling
  diagram`; a Würth PDF a `Recommended Hole Pattern`; a JST series PDF a
  `PC board layout and Assembly layout`: every land pattern C, D and E draw is
  in a placed PDF.

## 3. Rules

1. **Printed in the PDF** (`hit`): the part number, or the part number
   without its trailing packing letter, is in the PDF text (per-item
   makers). JST: the part number matches the (PDF, series) scheme, one
   regular expression per (JST PDF, variant) over the numbering rule (`B(\d+)B-XH-A`),
   with the position count taken from the match; `scheme-check.py` (plan-40's)
   enumerates the matches and every candidate row must match exactly one
   scheme. No match → `skip`, reason `not in the datasheet's ordering table`.
2. **Status** (`dead`): `textscan.py`'s hits (`obsolete`, `discontinued`, `not
   recommended for new design`, `end of life`, `EOL`, `last time buy`) are
   read; a row whose datasheet marks that very part obsolete, discontinued,
   NRND or end of life is `skip`, reason `obsolete in the datasheet`. A PDF
   whose every row is such is deleted from the tree and listed in
   `obsolete-not-included.md`.
3. **Facts**: pitch (must equal DigiKey's), rated voltage and rated current:
   the datasheet's IEC rating when it prints both IEC and UL values, else the
   one it prints; the datasheet's value replaces DigiKey's (`volts`, `amps`
   columns) and an empty one stays empty (the kind's ratings are optional,
   F). Pitch or positions that disagree with the PDF → `skip`, reason
   `datasheet disagrees`.
4. **Pins**: `positions` pins, numbered 1…n, named `"1"`…`"n"`, role
   `inout`. The pin-1 end is each footprint's (C, D, E read the drawing);
   rows whose drawing numbers pins in a way that is not 1…n along the row are
   `skip` there.
5. **Names**: the part name is the DigiKey `ManufacturerProductNumber`
   verbatim. Two rows whose numbers differ only by a trailing packing letter
   and share a PDF stay one part (the most-stocked, which `dedupe` already
   keeps).

## 4. The final pick

- `state` `add` for every row that passes 1–4 and keeps its stock check.
- **Generic representatives** (`generic-reps.tsv`: pitch, variant, reason):
  for each terminal pitch the `add` fixed-block variant (class `screw`,
  `spring` or `push-in`, mounting `tht`) with the largest summed stock; for
  JST XH and PH `JST|XH|2.50|V|tht` and `JST|PH|2.00|V|tht`. These variants'
  footprints are drawn for every position count 2…12 (the generics use
  them); every other variant's footprints only for the position counts of
  its `add` rows.
- `footprints-needed.txt` is rewritten from the `add` rows with those extra
  position counts: one line per variant, letter C / D / E as in A.
- **Skip groups** for `datasheets/skipped-parts.md` (a section "Terminal
  blocks and JST headers (plan-45)"): Reliance North America's single-level
  in-scope DigiKey parts (3759 at planning time, § A 2: no maker-site
  datasheet); Pololu's `JST PH-Style` (10) and `JST SH-Style` (12) headers
  (the datasheet link is an HTML product page); Phoenix Contact's 735
  in-scope rows with no per-item copy; Weidmüller's 57 and Würth's 22 rows
  with no maker link; every `skip` row of B with its reason.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — JST ordering schemes

- [x] `census/scheme-check.py` (adapted): one scheme per (JST PDF, variant) in
      `specs.tsv`; run it: every JST candidate row matches exactly one
      scheme, with its position count.
      Output 2026-11-24: `census/jst-schemes.py` wrote the 22 schemes (one per (JST PDF, variant): the sheets' "Model number allocation" page and part-number tables, page numbers in each `note`); `scheme-check.py` → `jst: 191 rows, 0 unmatched, 0 ambiguous; 0 wrong spec, 0 position mismatches, 0 unread`. The 11 rows the PDFs do not print verbatim are each made by a documented option of a printed form: PH SMT top entry `B*B-PH-SM4-TB` + suction tape `T` (6 rows, allocation p5), SR top entry `BM*B-SRSS-TB` + `T` (3, eSR_SZ allocation p4), VH `B2P-VH-FB-B` (printed glued to its footnote marker: "B2P-VH-FB-BNote2", p4), ZH `B*B-ZR-SM4-TF` + `T` (1, allocation p6); each base form is printed (`pdftotext … \| grep -c` → 1).

Acceptance: every JST candidate row is covered by one scheme.
  Check: `python3 -I planning/plan-45-data/census/scheme-check.py planning/plan-45-data/specs.tsv planning/plan-45-data/candidates.tsv` → `jst: 192 rows, 0 unmatched, 0 ambiguous` (est. 1 h: reading 11 numbering tables). Result: `jst: 191 rows, 0 unmatched, 0 ambiguous` (191, not 192: A's stock check removed `S11B-XH-A`).
Commit: ec922b72

### Phase 2 — Per-item verdicts

- [x] `census/verdicts.py` (adapted): `hit` and `dead` for the 329 terminal
      rows from `textscan.py`'s output; read every `dead` = 1 row and set
      `skip` or keep it; record the readings in `specs.tsv`'s `note`.
      Output 2026-11-24: 322 terminal rows (Phoenix 74, Weidmüller 153, Würth 95). `hit` 1 for 309; the other 13 are Würth rows of two series-level sheets whose title block prints the numbering rule (`census/series-schemes.py`: `6913114001(?P<n>\d\d)` and `6914121200(?P<n>\d\d)B` in `specs.tsv`, Corrections), each with `n` equal to DigiKey's `positions` (13 of 13). `dead` 1: 0 rows, so no reading to record (the dict `READINGS` in `verdicts.py` is empty). `verdicts.py` → `terminal: 322 rows: 322 add, 0 skip, 0 undecided`.
- [x] ~~Delete the PDF of a per-item row marked obsolete and list it in
      `datasheets/obsolete-not-included.md` (the family column
      `connector`, category `Connector → Terminal block`).~~ — moot: no PDF names an obsolescence word anywhere. A scan of all 320 placed files (`pdftotext -layout`, words OBSOLE…, DISCONTINUED, DISCONTINUATION, NOT RECOMMENDED FOR NEW, END OF LIFE, EOL, LAST TIME BUY, NRND, PHASE-OUT, NOT FOR NEW DESIGN, SUPERSEDED, REPLACED BY, SUCCESSOR, not only on the pages that name the part) → `320 files scanned; 0 with a hit`; DigiKey lists every candidate Active (A's stock check).

Acceptance: every terminal row has a verdict and a reason when skipped.
  Check: `python3 -I planning/plan-45-data/census/verdicts.py planning/plan-45-data/candidates.tsv --check` → `terminal: 329 rows: <add> add, <skip> skip, 0 undecided` (est. 1 h). Result: `terminal: 322 rows: 322 add, 0 skip, 0 undecided` (after Phase 3 the same command says `320 add, 2 skip`: the two stub sheets).
Commit: 41b89bed

### Phase 3 — Facts, pins, the final pick

- [x] `census/specs-fill.py`: rated voltage and current from each PDF into
      `specs.tsv` and `candidates.tsv` (`volts`, `amps`); `pinout-check.py`:
      the pin rule of § 3 holds for every `add` row.
      Output 2026-11-24: `specs-fill.py --write` read 513 rows: JST 191 (specification box), Phoenix 74 (71 by the headline "Nominal current …, Rated voltage (III/2) …", 3 by the IEC table: `Rated voltage (III/3)` or the one the sheet prints), Weidmüller 153 (`Product data IEC: <V> V / <A> A`), Würth 95 (12 VDE section, 68 the VDE column of a "cULus / UL, VDE" table, 13 the two series sheets, 2 none). Rule: the IEC value when the sheet prints IEC and UL (Würth: VDE), else the one it prints. Ratings the sheets do not print stay empty (`add` rows with no `amps` or `volts`: 0, the two empties are the two skipped stubs). Pitch compared on 482 rows (JST 191, Phoenix 70, Weidmüller 153, Würth 68; the sheets of the other 31, Phoenix 4 and Würth 27, print none) and positions on 234 (Phoenix 70, Weidmüller 153, Würth 11): **0 disagreements**. Spot-checked against the PDFs: Phoenix 1935815 32 A / 400 V, 5441278 160 V (DigiKey says 320: the II/2 figure), Würth 950011202000000 VDE 24 A / 750 V (DigiKey 14 A / 300 V), Weidmüller 8000078321 IEC 1000 V / 30.4 A, 2741400000 26.8 A (DigiKey: none). `pinout-check.py --fill` → `pinout: 511 rows, 0 bad` (every row `1..n`, 2 to 12 positions).
- [x] `generic-reps.tsv` (§ 4); `census/final.py`: `state` `add` / `skip`,
      `footprints-needed.txt` rewritten.
      Output: `final.py --write` → `191 JST rows decided; 511 add, 2 skip; 8 generic representatives`; no row stays `candidate`. Representatives: 3.50 WDM Omnimate-LSF-SMT H spring, 3.81 WE WR-TBL-Series-4123 A spring, 5.00 WE 1377 H screw, 5.08 WDM Omnimate-LM H screw, 7.50 WE 0011 H screw, 7.62 WE 4114 H spring, JST XH and PH `V|tht`. `footprints-needed.txt` per letter: C 17 variants (123 parts) 133 footprints; D 18 (197) 139; E 19 (191) 184; the `existing` column is empty for every variant (`awk -F'\t' '!/^#/ && $10!=""' … \| wc -l` → 0).
- [x] `datasheets/skipped-parts.md`: the section of § 4; `final.py
      --verify-skipped` finds every `skip` row in it.
      Section "Terminal blocks and JST headers (plan-45)": the five groups of § 4 (Reliance North America, Pololu, Phoenix 735, Weidmüller 57, Würth 22), the one JST row with only a DigiKey copy, the 2 `skip` rows and the 8 rows A's stock check removed. `final.py --verify-skipped` → `2 skip rows, 0 missing from skipped-parts.md`.

Acceptance: the pick is final and its skips are listed.
  Check: `python3 -I planning/plan-45-data/census/final.py --verify-skipped` → `<n> skip rows, 0 missing from skipped-parts.md`; `awk -F'\t' '$22=="candidate"' planning/plan-45-data/candidates.tsv \| wc -l` → 0 (est. 30 min). Result: `2 skip rows, 0 missing from skipped-parts.md`; `0`.
Commit: d5d472ee

## Validation Plan

- Tests: the scripts' own `--check` runs above.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate: none; the landing tier is I's.

## Decisions

- Author: **A per-item PDF is its own ordering table**: Phoenix, Weidmüller
  and Würth publish one datasheet per orderable number, so "in the
  datasheet's ordering table" is "the number is printed in it".
- Author: **The datasheet's rating wins over DigiKey's** (42 candidates have
  no current and 40 no voltage at DigiKey); an empty rating stays empty.
- Author: **The part name is the DigiKey number verbatim** (plan-40 and
  plan-44's rule), so the golden lines and the BOM match what a buyer types.

## Corrections

- **2026-11-24, populations after A.** A's stock check took the candidates from 521 to **513** (Phoenix 74, Weidmüller 153, Würth 95, JST 191) and the distinct URLs from 340 to 333 (320 placed files); the "329 terminal rows" below are **322** (74 + 153 + 95), the JST rows 191. § 2's numbers are planning-time figures; the checks below are the truth.
- **2026-11-24, Phase 1.**
  - *One scheme read in several PDFs is not ambiguous.* The SH/SR top and side entry numbering (`BM*B-SRSS-TB`, `SM*B-SRSS-TB`) is printed in `eSH.pdf`, `eSR_SZ.pdf` and `eSR_SZ-new.pdf`, so a part matched the same variant's scheme in two or three PDFs (17 rows) and the first run said `17 ambiguous`. `scheme-check.py` now calls a row ambiguous only when the schemes of more than one *variant* match, and requires the row's own (variant, PDF) scheme among the matches.
  - *`scheme-check.py` and `jst-schemes.py` are new files, not adapted copies:* plan-40's and plan-44's schemes cover a different spec columns set (frequency, loads, …); plan-45's is the regex over the part number with `n` = positions.

- **2026-11-24, Phase 2.**
  - *§ 2 said every Würth sheet is per-item; two are series-level.* `691311400102.pdf` (series 3114, 7.62 mm close vertical header) and `691412120002B.pdf` (series 412 B, 3.5 mm screwless 45°) are one file each shared by 7 and 6 candidate rows (same SHA-256 under several URLs, `datasheets.tsv`). They print "WERI PART NO: 691 311 400 1xx" / "691 412 120 0xx B" (and 412 B "A = (3.5 x NB PINS) + 1.5"), not the part numbers, so `hit` was 0 for those 13 rows. Rule 1 for a per-item maker now reads: the number is printed, or a numbering rule printed in the sheet makes it (the JST rule). The two rules are in `specs.tsv` (`scheme`, `note`), checked per row with `n` = `positions` (13 of 13), and `verdicts.py` applies them.
  - *Terminal rows are `add` now, JST rows still `candidate`.* `verdicts.py` sets `state` for the terminal makers only; Phase 3's `final.py` decides the JST rows and the facts step may still turn an `add` into a `skip` (`datasheet disagrees`). `place.py --verify`, `textscan.py` and `specs-skeleton.py` treat `candidate` and `add` alike (they said `candidate` only).
  - *No obsolete PDF.* The whole-file scan (task 2 above) found no obsolescence wording in any of the 320 files, so nothing is deleted and `datasheets/obsolete-not-included.md` is untouched.

- **2026-11-24, Phase 3.**
  - *Two Würth sheets are stubs, so their rows are `skip`.* `950018205000000.pdf` and `950018207000000.pdf` (the `/components/products/datasheet/` copies DigiKey links) are 3 pages: soldering profile, cautions, disclaimer: no rating, pitch, drawing or land pattern (page 1 and 2 looked at as images); their siblings 2, 3, 4, 6, 8 of the same series are 9-page full sheets from `/catalog/datasheet/`. § Non-goals: no datasheet hunting, a row with no usable PDF is `skip`. The `/catalog/…` form of the two items answered 403 on 2026-11-24 (so did a control item that had downloaded the same day: the throttle A's § 4 describes), so it was not used. Rule in `specs-fill.py`: a placed sheet that prints no rating, no voltage and no pitch → `skip`, reason `no technical data in the datasheet`. Both PDFs are deleted from the tree (`place.py --prune`: 318 files left, `ok: 318 files, 0 rows without a file, 0 unused`). `verdicts.py` leaves a row another step skipped for its own reason (it would have turned both back into `add`).
  - *Four Phoenix headers and the Würth 3114 sheet print no drilling or hole wording.* A scan for land-pattern wording over the 318 placed files (`Drilling diagram`, `Hole diameter` (Phoenix); `Solder eyelet hole diameter`, `Drawings` (Weidmüller); `Hole Pattern`, `PCB LAYOUT`, `Footprint`, … (Würth); `PC board layout` (JST)) finds it in every file except Phoenix `1941650`, `1966509`, `5452093`, `5452094` and Würth `691311400102` (the 3114 series sheet, 1 page, a drawing without those words). § 2's "every land pattern C, D and E draw is in a placed PDF" is therefore **not** established for those five sheets (11 rows: Phoenix 4 rows, Würth 3114 7 rows). They stay `add`: C, D and E read the drawing and, per § Non-goals, make a row `skip` in their own Corrections when no land pattern can be drawn (the footprint list only shrinks).
  - *Ratings: IEC over UL.* Phoenix's headline is the IEC figure (III/2); a header whose headline names no voltage gets the IEC table's `III/3` (400 V, 320 V), else `II/2`. Würth prints UL and VDE ([IEC 61984]) columns: VDE. DigiKey's value often differs (Phoenix 5441278: 160 V against 320 V (II/2); Würth 950011202000000: 24 A / 750 V against 14 A / 300 V): 20 JST, 33 Phoenix, 76 Weidmüller and 57 Würth `amps` and 63 / 70 / 74 `volts` differ from DigiKey's. JST VH is two ratings in one sheet: 10 A for the standard header (AWG #16), 7 A for the shrouded `-FB-B` (AWG #18), set per part; SR (`eSR_SZ`) 0.7 A against SH's 1.0 A: each row takes its own PDF's.
  - *Positions can be compared only where the sheet ties the part to a count.* Phoenix and Weidmüller print it (`Number of positions`, `Number of poles`): compared on 223 rows; Würth on 11 (a table row naming the part); JST's `n` equals DigiKey's `positions` on all 191 (`scheme-check.py`), and the `xx` of the 13 Würth series-sheet rows does the same: the remaining 69 Würth rows, 4 Phoenix rows and the 2 stubs rely on DigiKey's count, as § 3 allows when the sheet prints none.
  - *Footprint numbers moved.* `footprints-needed.txt` is now from the `add` rows plus the generics' counts 2…12: **C 17 variants / 133 footprints, D 18 / 139, E 19 / 184** (A's planning pick: 17 / 124, 18 / 142, 19 / 184 for 450 in all, and A's § 2 line count 54 is unchanged). C, D and E re-measure these at their start.

## Summary

B's risk is reading: 340 PDFs, of which 329 are one-part files a script scans
and 11 are JST tables a person reads. The pick it writes sets C, D and E's
footprint counts, so a late `skip` there only shrinks them.
