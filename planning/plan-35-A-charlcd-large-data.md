# plan-35-A: 20x4 / 40x2 / 40x4 LCDs — stock re-check, datasheet downloads, skipped parts

Last updated: 2026-11-11
Overall Effort: huge (> 3d) — plan-35 is A → B → C → D → E → F → G → H
Effort: medium (1h–2h)
Depends on: nothing (§ Prerequisites)

plan-35 adds 20-character × 4-line, 40 × 2 and 40 × 4 character LCD modules
to the parts library and the app: 67 parts from the three makers with the
most such parts in stock at DigiKey (Focus LCDs, Newhaven Display, Midas
Displays). They are parallel HD44780-type modules like the five 16 × 2
parts plan-11 added, and they use the same `charlcd` kind. That kind
gains three things: four-line displays; 40 × 4 modules, which have two
controllers with one enable pin each (E1 for the top two lines, E2 for the
bottom two); and backlights with up to three LEDs (red, green, blue). It
also gains two panel types, STN gray and FSTN white. The drawer files
character LCDs under **Displays → Character → 16x2 / 20x4 / 40x2 / 40x4**.

The single behavioral outcome of plan-35: loading the part files gives the
baseline parts (P0, § Prerequisites) plus one `charlcd` part per `add` row
of `planning/plan-35-data/candidates.tsv` (67). A 20 × 4 module wired to a
CPU board shows four lines of text in play mode. A 40 × 4 module shows
lines 1–2 through E1 and lines 3–4 through E2. An RGB module's face takes
the color of the LEDs that are lit. `ctest` passes T0 programs (no new
program; `charlcd_test` grows).

| Letter | Content | Effort |
|---|---|---|
| **A** | part-by-part stock re-check, the 67 module PDFs and `ST7066U.pdf` into `datasheets/module-disp-char/`, `datasheets.json` entries, `skipped-parts.md` | medium |
| B | footprints: the 16 Focus module footprints | large |
| C | footprints: the 10 Midas and 7 Newhaven module footprints | large |
| D | backend: `charlcd` gains 4 rows, two enables, multi-LED backlights, panels `gray` and `white`; `charlcd_test`; docs | large |
| E | parts: the 22 Newhaven and 13 Midas devices; the 16 × 2 parts move to the `16x2` drawer leaf; goldens; the every-part check and the 20 × 4 / 40 × 4 CPU-board tests | large |
| F | parts: the 32 Focus devices; goldens | large |
| G | app: panel colors, facts, the E1/E2 pin column, RGB backlight blending, `CharacterLCDTests` | medium |
| H | landing tier | small |

Outcome of A: `planning/plan-35-data/stock-check.tsv` shows every `add` row
in stock with status Active or Last Time Buy (or the row is now `skip`,
with the reason). `datasheets/module-disp-char/` holds the 67 module PDFs
plus `ST7066U.pdf`. `datasheets.json` has one entry per module PDF.
`datasheets/skipped-parts.md` lists the 59 `skip` rows.

References:

- `AGENTS.md` § Datasheets (stock rule, obsolete rule, footprint rule, goldens).
- `docs/parts-format.md` § Datasheets (`id` and `url` rules: the URL is the
  maker's own site).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`
  (their table formats).
- `planning/plan-35-data/README.md`: every census file and count.
- `planning/completed/plan-11-A-groundwork.md`, `plan-11-C-charlcd-parts.md`
  (the 16 × 2 precedent: one device per datasheet, Connector Only header).

## Prerequisites

This gate covers all of plan-35. Sub-plans B–H point here, plus their own
"previous letter complete" row.

| Must be true | Command | Status |
|---|---|---|
| The census data is in the tree | `ls planning/plan-35-data/candidates.tsv planning/plan-35-data/footprints-needed.txt` → both listed | MET (2026-11-11, written with this plan) |
| DigiKey credentials are available | `grep -c '^export DIGIKEY_CLIENT_\(ID\|SECRET\)=' ~/.zshrc` → 2 | MET (2026-11-11: 2) |
| DigiKey quota for the re-check | the stderr summary of any `node scripts/digikey.js` call shows ≥ 67 calls left today | MET (2026-11-11: 874 left after the census) |
| Poppler and ImageMagick are installed | `which pdftotext pdftoppm pdfinfo magick` → four paths | MET (2026-11-11) |
| The `charlcd` kind exists (plan-11) | `grep -c 'CHARLCD_KIND' backend/src/library/kinds.c` → 2 | MET (2026-11-11 at 656b6d1: 2) |
| Baseline recorded: P0 parts, F0 footprints, T0 tests | `wc -l < backend/tests/golden/parts.txt`; `wc -l < backend/tests/golden/footprints.txt`; `grep -c '^logic_test' backend/CMakeLists.txt` | recorded, not a gate (2026-11-11 at 656b6d1: 26729 / 1212 / 30). Re-measure when E starts: plans 33 and 34 add parts, footprints and tests before or after this plan, and every count below is relative |

plan-35 does not depend on any other open plan. Plans 33 (capacitors) and
34 (resistors) add other kinds; they share only `footprints.json`,
`datasheets.json` and the goldens, where each plan only appends its own
lines. plan-32-J (part editor) builds a `charlcd` form from today's keys.
This plan adds a Corrections note to plan-32-J (same commit) saying which
keys plan-35 adds; plan-35 does not wait for plan-32.

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `awk -F'\t' '$4=="add"' planning/plan-35-data/candidates.tsv | wc -l` →
  67, minus any row the re-check moved to `skip` (each listed in
  Corrections with its stock line).
- `wc -l < planning/plan-35-data/stock-check.tsv` → 67;
  `awk -F'\t' '$3=="" || $3==0 || ($2!="Active" && $2!="Last Time Buy")' planning/plan-35-data/stock-check.tsv`
  → only the rows moved to `skip`.
- `ls datasheets/module-disp-char/*.pdf | wc -l` → 7 (today) + 68 = 75.
- Each of the 59 `skip` part names appears exactly once in the new
  plan-35 section of `datasheets/skipped-parts.md` (Phase 3's check).
- `node scripts/check-datasheets.js` (its `--help` names the option that limits
  it to new ids) answers PDF for all 67 new ids.

### Non-goals (explicit constraints)

- No part, footprint, kind or app change (B–G).
- No PDF of a `skip` row in the tree (`skipped-parts.md`: "Their PDFs are
  not in the tree").
- `obsolete-not-included.md` gets no entry: no downloaded PDF has only
  obsolete parts (every `add` and `skip` part is Active; the 16 obsolete
  Newhaven parts had no stock and were never downloaded, see § 2).

## 2. Current State

### The census (2026-11-11), `planning/plan-35-data/`

| What | Count | Command |
|---|---|---|
| In-stock parts in DigiKey category 99, every format | 810 | `census/list.sh` → `raw/cat99-instock.err`: "810 of 810 parts, 39 pages, 39 API calls" |
| … of format 20 x 4, 4 x 20, 40 x 2, 40 x 4 | 189 | `python3 -I census/census.py raw/cat99-instock.jsonl.gz` (stderr) |
| Top three makers by that count | Focus LCDs 75, Newhaven 34, Midas 17 (next: Display Visions 11, Winstar 11) | same |
| Their parts in any stock/status | Focus 75 (all Active, stocked); Newhaven 52 (34 stocked, 2 Active unstocked, 16 Obsolete unstocked); Midas 20 (17 stocked, 3 Active unstocked) | `python3 -I census/allstat.py raw/cat99-m*-all.jsonl.gz` |
| Rows in `candidates.tsv` | 126 (67 `add`, 59 `skip`) | `awk -F'\t' 'NR>1{print $4}' candidates.tsv \| sort \| uniq -c` |
| `add` by size | 20x4 41, 40x2 14, 40x4 12 | `awk -F'\t' '$4=="add"{print $3}' candidates.tsv \| sort \| uniq -c` |
| `add` by maker | Focus 32, Newhaven 22, Midas 13 | `… {print $1} …` |
| `skip` by reason | 32 no font stated, 12 serial/I2C/SPI front end, 8 OLED, 4 English/European font, 1 overview-only PDF, 1 FPC tail, 1 no own-site datasheet | `awk -F'\t' '$4=="skip"{print substr($5,1,30)}' candidates.tsv \| sort \| uniq -c` |

**The 300-result cap.** The category listing (810 parts) is over DigiKey's
300-result cap. digikey.js split it into pieces, and no piece was cut:
`raw/cat99-instock.err` holds only the summary line, with no "cut short"
warning. The per-maker listings (251, 250, 64) are under the cap. Stock
changes from day to day, so phase 1 still re-checks every `add` row
part by part.

### Inclusion rule, applied (owner, 2026-11-11)

- **Makers:** Focus LCDs, Newhaven Display Intl and Midas Displays, the top
  three by in-stock 20x4/40x2/40x4 count.
- **In:** parallel HD44780-type LCDs, including modules without a
  backlight, 3.3 V modules and RGB-backlight modules.
- **Out (owner):**
  - character OLEDs (8);
  - modules with a serial front end (12): the PIC-based Focus C204AS and
    NHD-0420D3Z, and the IST3602 NHD-C0420 modules;
  - any font other than ROM A00 (English/Japanese).
- **Stock and status:** quantity > 0, status Active or Last Time Buy (all
  67 `add` rows are Active).
- **Must be in the datasheet:** each module PDF covers exactly one part
  and prints its part number. Verified for all 67: the part number,
  without a trailing `#`, is found in the PDF text.
- **Font check, per PDF:**
  - Midas states it on "Font Set";
  - newer Focus PDFs tick a "Font type" box (U+F0FE ticked, U+F06F empty;
    `census/font.py`);
  - other Focus PDFs say "ST7066U-0A" (ROM code 0A = A00);
  - Newhaven PDFs and the rest print a font table, checked by eye on
    rendered pages (11 templates, all A00: katakana at A1–DF);
  - C204AXBSYLY6WT's table is titled "English/European" but shows the
    A00 glyphs, so the table counts;
  - C204ALBSYLY6WM55PAA ticks both "English-Japanese" and "Other"; its
    font table on p.11 is A00.
- **No font stated → skip.** 32 Focus PDFs share one 10-page template
  (C204A/B/C-…-LW6x, C204A1, C402A-…, C404A-…) that has no font table, no
  ROM code and no font box (`grep -i -E 'ROM Code|ST7066U?-0[A-Z]|English[-/ ]'`
  finds nothing in them; all 10 pages viewed). A00 cannot be shown for
  them, so they go to `skipped-parts.md`.
- **No own-site datasheet → skip.** `docs/parts-format.md` § Datasheets
  wants the maker's own URL. DigiKey links Midas PDFs on its media server.
  `census/midas.sh` finds each Midas part's product page on
  midasdisplays.com and the PDF that page links to:
  - 13 found (`census/midas-urls.tsv`); 10 are byte-identical to DigiKey's
    copy, and 3 differ only in page layout (`pdftotext` diff: 0 lines for
    the two MC42005, 19 layout lines for MC42004A6W-SPR-V2);
  - MC42008A6W-SPR has no product page and no PDF there (404), so it is a
    skip.

### Datasheets

- `datasheets/module-disp-char/` holds 7 PDFs today (`ls … | wc -l` → 7).
- The 67 module PDFs, as read for this plan, are listed with sha256, size
  and page count in `planning/plan-35-data/pdf-sha256.tsv`. URLs are in
  `candidates.tsv` column `datasheet_url`:
  - Focus: focuslcds.com;
  - Newhaven: newhavendisplay.com;
  - Midas: midasdisplays.com.
- **`ST7066U.pdf`** is Sitronix's ST7066U datasheet V2.5 (45 pages),
  from `https://focuslcds.com/wp-content/uploads/Drivers/ST7066U.pdf`.
  - It is needed because the Midas modules print no bus timing and no
    execution times. Their page 1 says to use "the ST7066U specification
    (Provided Separately)".
  - Three Newhaven PDFs print no instruction table: NHD-0240AZ-FSW-FBW,
    NHD-0240AZ-NSW-BBW and NHD-0440AZ-RN-FBW. Each names "Built-in
    ST7066U Controller".
  - It has the AC timing tables (p.35 at VCC = 2.7 V, p.36 at VCC = 5 V;
    "TC Enable Cycle Time Pin E 1200 ns") and the instruction table with
    execution times (p.20: 37 µs; 1.52 ms for Clear and Home).
  - Newhaven's `newhavendisplay.com/app_notes/ST7066U.pdf` is the older
    V2.2 (42 pages); the V2.5 copy is used.
  - It is a controller datasheet that no part names in `"datasheet"`, so
    it gets no `datasheets.json` entry. Device notes cite it by file name,
    the way plan-11 notes cite module pages.
- **Datasheet ids**: the PDF's name in lower case without `.pdf`, as in
  plan-11 (`nhd-0216bz-fl-ybw`): `C204ALBFWSW6WT33XAA_Spec.pdf` →
  `c204albfwsw6wt33xaa_spec`, `NHD-0440WH-ATFH-JT.pdf` →
  `nhd-0440wh-atfh-jt` (the part keeps its `#`). `candidates.tsv` column `pdf` is the local
  file name.

### Verified properties

- **No new datasheet id collides.** None of the 67 lower-case PDF names is
  an id in `datasheets.json` today (`grep -c` of each → 0; checked with the
  `candidates.tsv` `pdf` column, 2026-11-11).
- **Every add row has a maker-site URL.** `awk -F'\t' '$4=="add" && $14 ~ /digikey/' candidates.tsv | wc -l` → 0.

## Phases

> Tick `- [x]` in the same commit as the work; `- [~]` for partial with one
> line of what remains. Fill each `Commit:` line when the phase lands. **An
> unticked box means NOT DONE.**

### Phase 1 — stock re-check, part by part

- [ ] Run `planning/plan-35-data/census/recheck.sh planning/plan-35-data/candidates.tsv planning/plan-35-data/stock-check.tsv`
      (67 API calls; about 2 minutes). It prints the rows that fail the
      stock or status rule.
- [ ] For each printed row: set its `candidates.tsv` verdict to `skip`
      with the reason `out of stock / <status> on <date>`, remove the part
      from `census/footprint-groups.tsv` (drop the group if it empties),
      re-run `census/footprints.py`, and record the row in Corrections.
      The counts in B–H are `add` rows, so they follow.

Check: `wc -l < planning/plan-35-data/stock-check.tsv` → 67 and the awk
of § 1 prints only rows now marked `skip` (est. 2 min).

Commit: —

### Phase 2 — datasheets into the tree

- [ ] Download every `add` row's `datasheet_url` with
      `planning/plan-35-data/census/dl.sh` (input: `maker\tpart\turl`
      from `candidates.tsv` columns 1, 2, 14) into a new empty
      `/tmp/p35-pdf/`, plus `ST7066U.pdf` from the focuslcds.com URL above.
- [ ] Compare with `pdf-sha256.tsv` (`shasum -a 256`). A changed file is
      the maker's newer revision: keep it, re-read the pages B–F cite from
      it (outline, pins, font, timing), and record what changed in
      Corrections. A file that is no longer a PDF moves its part to
      `skip` (no datasheet) as in Phase 1.
- [ ] Copy them to `datasheets/module-disp-char/` under the `pdf` column's
      name (the Midas files keep their own names; the DigiKey `3086_`
      prefix is gone because the own-site URL has none).
- [ ] Append 67 entries to `schematical/schematical/Parts/datasheets.json`
      (`{"id": <lower-case pdf name without .pdf>, "url": <datasheet_url>}`),
      in the file's sorted position.

Check: `ls datasheets/module-disp-char/*.pdf | wc -l` → 75;
`python3 -c` count of `datasheets.json` ids that match a `candidates.tsv`
`add` row → 67; `node scripts/check-datasheets.js` on the new ids → all
PDF (est. 3 min).

Commit: —

### Phase 3 — skipped parts

- [ ] Add a section `## Character LCDs, 20x4 / 40x2 / 40x4 (plan-35)` to
      `datasheets/skipped-parts.md` with the file's table columns (Part(s) |
      Device | Would have gone into | Why skipped). Group rows by reason:
      one row per reason and family, listing every part. Every `skip` row
      of `candidates.tsv` appears exactly once. "Would have gone into" is
      `charlcd` / Displays → Character → <size>. Why: the `reason` column,
      with "owner, 2026-11-11" on the scope rows.

Check: every `skip` part name is found once in the new section (a
`grep -c -F` loop over the 59 names → 59 ones; est. 1 min).

Commit: —

## Validation Plan

Per-phase checks only. The landing tier is plan-35-H.

## Decisions

- **Makers: Focus LCDs, Newhaven, Midas** (owner, 2026-11-11): the top
  three by in-stock 20x4/40x2/40x4 parts (75 / 34 / 17).
- **Parallel LCDs only; OLED and serial front ends out** (owner,
  2026-11-11). This removes 20 parts.
- **No-backlight, 3.3 V and RGB-backlight modules in** (owner,
  2026-11-11): 4 no-backlight parts, 19 parts at 3.3 V, 4 RGB parts.
- **Non-A00 fonts out** (owner, 2026-11-11); they go to
  `skipped-parts.md`.
- **A PDF with no font information counts as not-A00** (author). The
  inclusion rule needs the datasheet to show the part is A00, and the 32
  template PDFs show nothing. The ST7066U has six ROM codes (`ST7066U.pdf`
  pp.13–18: 0A, 0B, 0E, 0R, 0T, 1G), so the controller name alone does not
  settle it.
- **C204IXBSGN06WR50XAC skipped** (author): its p.2 drawing is an FPC tail
  ("Conductive side / Stiffener side"), not header pins. Its footprint
  would need an FPC connector part that the library does not have.
- **C404A-FTB-LW65 skipped** (author): Focus's PDF is a 2-page overview
  with no pin table, timing or drawing.
- **Midas datasheets from midasdisplays.com** (author): this is the rule in
  `docs/parts-format.md`. MC42008A6W-SPR has no PDF there and is skipped.
- **`ST7066U.pdf` from focuslcds.com, V2.5** (author): the newest copy
  found on a module maker's site.

## Corrections

(none yet)

## Summary

Low risk: data only. The one judgment call, how to treat a PDF with no
font information, was decided before writing (§ Decisions). Phase 1 can
shrink the part count, and every later sub-plan counts `add` rows, so they
follow it.
