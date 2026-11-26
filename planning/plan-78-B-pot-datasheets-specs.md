# plan-78-B: Potentiometers: datasheets, verdicts, reserves, `specs.tsv`

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-78-A

B downloads the datasheet of every `in` row of `candidates.tsv`, gives each row a verdict, moves
reserves up where a `skip` frees a capped slot, and writes `specs.tsv`: one row per (maker, series
drawing), the hand-off to the footprint letters (C, D) and the parts letters (G, H).

References:

- plan-78-A § 3 (the listing's columns and scope); `planning/plan-78-data/README.md`.
- `AGENTS.md` § Datasheets (obsolete PDFs); `datasheets/skipped-parts.md`,
  `datasheets/obsolete-not-included.md` (formats); `docs/parts-format.md` § Datasheets.
- `planning/plan-74-A-ethernet-data.md` § 3.4 (fetch, text scan, reserves: the precedent).
- `planning/plan-58-data/census/fetch.py`, `textscan.py`, `specs-skeleton.py` (copied and adapted).

## Prerequisites

plan-78-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-78-A complete | `ls planning/completed/plan-78-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| Bourns serves its PDFs to a plain client | `curl -sL -A "Mozilla/5.0" -o /tmp/p78-3296.pdf https://www.bourns.com/docs/product-datasheets/3296.pdf && head -c 5 /tmp/p78-3296.pdf` → `%PDF-` | MET (author, 2026-11-25: 200, 577,872 bytes, 3 pages, title "3296 3/8-inch Square Trimpot® Trimming Potentiometer") |
| The datasheet folders are absent | `ls datasheets/potentiometer` → no such directory | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `datasheets/potentiometer/{bourns,piher,sfernice,samesky}/` hold every PDF an `add` row cites,
  with one `datasheets/potentiometer/datasheets.tsv` (`path url bytes sha256 digikey_url`, the
  `datasheets/resistor/datasheets.tsv` format).
- `verdicts.tsv` (columns `part maker verdict reason pdf`) gives every `in` row `add` or `skip`
  with a reason; `candidates.tsv`'s `verdict`,
  `verdict_reason` and `pdf` columns are filled. A `skip` in a capped maker is replaced by the next
  `cap` row (§ 3.3) until the maker has 300 `add` rows or no row is left.
- `datasheets/skipped-parts.md` has a section "Potentiometers (plan-78)" listing every `skip` row;
  `datasheets/obsolete-not-included.md` gets a row only for a PDF all of whose parts are obsolete.
- `specs.tsv` has one row per (maker, series drawing) of the `add` rows, every column filled (§ 3.4).
- `footprints-needed.txt` lists every footprint id of `specs.tsv`, sorted, unique.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No footprint is drawn (C, D).

## 2. Current State

No pot datasheet is in the repo (`ls datasheets/potentiometer` → no such directory). Bourns's
datasheets are per series, e.g. `3296.pdf`, and print a "How To Order" scheme rather than every
part number: `pdftotext -layout 3296.pdf` shows `3296 W - 1 - 103 __ LF` under "How To Order", and
the style codes `3296W`, `3296X`, `3296W-1`, `3296X-1` in the dimension drawings (author's probe,
2026-11-25; 2,184 words of text, so the scan works on text, not images).

## 3. Design

### 3.1 Download (`census/fetch.py`)

- Each `in` row's `datasheet_url` (DigiKey's link) is fetched with Python `urllib`, a Safari user
  agent, `Accept` and `Accept-Language` (plan-74-A's probe headers). An answer that does not start
  `%PDF-` is fetched again through `scripts/safari-digikey-pdf.sh <url> <out>` (Safari passes the
  maker sites' bot checks). A row with neither is `skip` (`no PDF`).
- Files: `datasheets/potentiometer/<maker>/<name>.pdf`, `<name>` the URL's file name in lower case;
  rows whose URLs serve byte-identical PDFs (same SHA-256) share one file.
- `fetch.py --probe` fetches the first `datasheet_url` of each maker in `candidates.tsv` and prints
  `<status> <bytes> <first 5 bytes> <url>`: 4 lines.

### 3.2 Verdicts (`census/textscan.py`, `pdftotext -layout`)

- **Series token.** A part's token is its part number up to the first `-` (Bourns `3296W-1-103LF`
  → `3296W`; `PTV09A-4020F-B103` → `PTV09A`; Piher `PT10LV10-103A2020` → `PT10LV10`; Vishay
  `T93YB103KT20` → `T93YB`, the letters and digits before the resistance code where there is no
  dash: the longest prefix that appears in the PDF). Same Sky part numbers (`PTN09-…`) follow the
  dash rule.
- `add`: the token appears in the PDF text with spaces and dashes removed from both, the row's
  `ohms` lies inside the resistance range the PDF prints (B reads the range into `specs.tsv`,
  § 3.4, and the scan checks it), and the PDF prints no obsolete, discontinued or not-recommended
  status for that series.
- `skip` reasons: `no PDF`, `not in PDF` (token absent), `ohms out of range`, `PDF obsolete`.
- **Obsolete PDFs.** A PDF that marks every series it covers obsolete is deleted and recorded in
  `datasheets/obsolete-not-included.md` (PDF name, family "Potentiometers", category
  rotary/trimmer/slide/thumbwheel). A PDF that marks only some series obsolete: those rows are
  `add`, and `specs.tsv`'s `flags` column says `obsolete` (AGENTS.md).

### 3.3 Reserves

A `skip` frees a capped maker's slot. The next `cap` row of that maker (by `stock`) becomes `in`,
gets its stock check (plan-78-A § 3.3, from the saved page) and its verdict, until the maker has 300
`add` rows or no `cap` row is left. `verdicts.tsv` records each promotion as reason `reserve`.

### 3.4 `specs.tsv`

One row per (maker, PDF, drawing) of the `add` rows; a series whose PDF prints several pin
layouts (Bourns 3296 W / X / Y / P / Z) has one row per layout. Columns:

`maker  pdf  series  layout  category  parts  gangs  pins  wipers  pin_names  turns  taper
ohms_min  ohms_max  tolerance  power  footprint  drawing_page  flags  note`

- `layout`: the drawing's style code (`W`, `X`, `P`, …) or `-`.
- `pins`: the pad count of the drawing's PCB layout, mounting tabs and board locks included.
- `wipers`: one `a:w:b` triple per gang, pin names as the drawing prints them (Bourns `1:2:3`;
  a dual pot `1:2:3,4:5:6`); `pin_names`: every pin in pad order, `nc` for a mounting tab or
  board lock (`MT1`, `MT2`, …, the resistor kind's `nc` role).
- `turns`: 1 for a single-turn part, else the PDF's mechanical / electrical turns (Bourns 3296:
  25); a decimal where the PDF prints one (3.75).
- `taper`: `linear`, `log` (DigiKey "Logarithmic", the PDF's "A" / audio taper) or `reverse-log`;
  from the row's DigiKey Taper (rotary) or `linear` for trimmers, whose PDFs print linear only.
- `footprint`: the id C or D draws, `POT-<MAKER>-<SERIES>[-<LAYOUT>]` in upper case, e.g.
  `POT-BOURNS-3296W`, `POT-PIHER-PT10LV`, `POT-SFERNICE-T93YB`, `POT-SAMESKY-PTN09`. Two layouts
  with identical recommended PCB layouts share one id (the `note` lists both).
- `drawing_page`: the page with the recommended PCB layout (or the dimension drawing when the PDF
  prints none: § Decisions).
- `specs-check.py` (copied from `planning/plan-58-data/census/specs-check.py`) checks every row:
  every column filled, `pins` = the count of `pin_names`, every wiper name in `pin_names`, every
  footprint id unique to one drawing.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Downloads and verdicts

- [ ] `census/fetch.py` (with `--probe`) and `census/textscan.py` per § 3.1–3.2; download into
      `datasheets/potentiometer/<maker>/`; write `datasheets/potentiometer/datasheets.tsv`.
- [ ] `verdicts.tsv`; reserves per § 3.3; `candidates.tsv`'s verdict columns.
- [ ] `datasheets/skipped-parts.md` § "Potentiometers (plan-78)"; `obsolete-not-included.md` rows.
- [ ] Record the `add` / `skip` counts per maker in this section.

Acceptance: every `add` row has a local PDF and every `skip` row is in `skipped-parts.md`.
  Check: `python3 -I planning/plan-78-data/census/textscan.py --verify` → `<a> add, <s> skip; 0 add without a PDF; 0 skip missing from skipped-parts.md; makers ≤ 300 add` (est. 30 min with downloads; 1 min to verify).
  Result:
Commit:

### Phase 2 — `specs.tsv` and `footprints-needed.txt`

- [ ] `specs.tsv` per § 3.4, filled from each PDF's drawing page.
- [ ] `footprints-needed.txt`.

Acceptance: every `add` row is in exactly one `specs.tsv` row, and every row passes the check.
  Check: `python3 -I planning/plan-78-data/census/specs-check.py planning/plan-78-data/specs.tsv planning/plan-78-data/verdicts.tsv` → `rows ok; every add row covered once; <n> footprints` with `<n>` = `wc -l < planning/plan-78-data/footprints-needed.txt` (est. 1 min).
  Result:
Commit:

## Validation Plan

- B's checks are its two acceptance commands. Nothing in `backend/` or the app changes.
- Final gate: plan-78-I.

## Decisions

- Author: **verdicts match the series token, not the full part number.** Pot datasheets print an
  ordering scheme (`3296 W - 1 - 103 __ LF`), not each part number, so a verbatim match would skip
  every Bourns part. The token plus the resistance range proves the PDF covers the part.
- Author: **one footprint per drawing layout, named `POT-<MAKER>-<SERIES>[-<LAYOUT>]`.** Pot
  footprints are series-specific (pin triangle, mounting tabs), so no generic pot footprint is
  shared across makers.
- Author: **where a PDF prints no recommended PCB layout, the pads come from the dimension drawing**:
  the pin positions, a drill of the lead's maximum size + 0.2 mm, and an annular ring of 0.5 mm
  (IPC-2222 level B, the plan-48-C precedent of IPC nominals). The footprint's `note` says so.
- Author: **`datasheets/potentiometer/`, not the untracked empty `datasheets/trimmer-potentiometers/`**:
  one folder for all four categories, as `datasheets/resistor/` holds chip, axial and arrays.

## Corrections

## Summary

B fetches every in-scope pot datasheet, keeps each maker at 300 parts covered by their PDFs, and
reads each series drawing into `specs.tsv`: pins, wipers, turns, taper and the footprint id.
