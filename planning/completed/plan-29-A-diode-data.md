# plan-29-A: Diodes — census, datasheet downloads, first decisions

Last updated: 2026-11-09
Overall Effort: huge (> 3d) — plan-29 is A → B → C → D → E → F → G → I → J → K → L → M → N, with H after A only (H → I; corrected 2026-11-09, H § Corrections)
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-29 adds diodes to the parts library and the app: rectifier, fast
recovery, switching and Schottky diodes, Zeners, diode arrays and bridge
rectifiers from the three makers with the most diodes in stock at DigiKey
(Vishay, onsemi, Taiwan Semiconductor). They sit in the parts drawer under
**Discrete → Diodes → Rectifier / Fast recovery / Switching / Schottky /
Zener / Bridge**. In play mode a diode passes logic one way: a 1 from anode
to cathode, a 0 from cathode to anode, so diode-OR and diode-AND gates work.

The single behavioral outcome of plan-29: loading the part files gives the
baseline 15214 parts (P0, § Prerequisites) plus one `diode` part per `add` row of
`planning/plan-29-data/candidates.tsv`; the app lists them under the six
drawer leaves; in play mode two inverters driving a diode-OR into a
pull-down light an LED while either output is 1; `ctest` passes T0 + 1
programs (`diode_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | census (two quota days), datasheet downloads, verbatim and status decisions, skipped parts, `specs.tsv` skeleton | large |
| B | footprints and specs: Vishay single diodes; the shared footprint rules | large |
| C | footprints and specs: onsemi single diodes | large |
| D | footprints and specs: Taiwan Semi single diodes | large |
| E | footprints and specs: Zeners and Zener arrays (all makers) | large |
| F | footprints and specs: diode arrays (all makers) | large |
| G | footprints and specs: bridge rectifiers (all makers) | large |
| H | backend: one-way diode links in the engine, the `diode` kind, voltage carry, `diode_test`, docs | large |
| I | parts: the generator; Vishay single diodes; goldens | large |
| J | parts: Vishay Zeners, arrays, bridges | large |
| K | parts: onsemi | large |
| L | parts: Taiwan Semi | large |
| M | app: catalog details, diode symbol marks, sidebar facts, `kindName` | medium |
| N | landing tier | small |

Outcome of A: `datasheets/diode/<maker>/` holds every PDF an `add` or
`pending` row needs, with `datasheets/diode/datasheets.tsv`; the `decision`
column is `add` for every verbatim row, `skip` for every row with no maker
datasheet, `out` for DigiKey and datasheet-status exclusions, and `pending`
for the rows B–G settle from ordering-code schemes.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-29-data/` — `census/*.py|sh`, `candidates.tsv`,
  `footprints-needed.txt`, `README.md` (written at planning time, 2026-11-08).
- `planning/plan-27-A-tactile-data.md` (open; the census precedent these
  scripts copy).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p29 && cmake --build /tmp/logic-p29 -j8 && ctest --test-dir /tmp/logic-p29` → `100% tests passed` | MET 2026-11-09 14:10 at f68e13f (main merged in; build dir /tmp/logic-p29w): `100% tests passed, 0 tests failed out of 29`; **T0 = 29** (`grep -c '^logic_test' backend/CMakeLists.txt` → 29) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` | MET: **P0 = 15214**, **R0 = 15214**, **F0 = 772** (2026-11-09 at f68e13f) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p29-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-09 at f68e13f, derived data /tmp/p29w-dd: `** BUILD SUCCEEDED **`) |
| No diode support yet | `grep -c 'PART_DIODE' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/diode.json` → no such file; `ls datasheets/diode` → no such directory | MET (2026-11-09 at f68e13f: 0, no file, no directory) |
| No other open plan adds diodes | `grep -l -i '"diode"\|PART_DIODE\|datasheets/diode' planning/plan-*.md \| grep -v plan-29 \| wc -l` → 0 | MET (2026-11-09 at f68e13f: 0) |
| DigiKey credentials and quota | `DIGIKEY_CLIENT_ID`/`_SECRET` exported in `~/.zshrc`; Phase 1 needs about 1250 calls over two days (§ 2); a day allows about 1000, shared with other sessions | **MET** (2026-11-09 14:05 HST: the command printed `999 left today`; 159 at the morning re-run; credentials present). Planning census left 163. Start Phase 1 on a day whose first call reports ≥ 600 left (`scripts/digikey.js --category 295 --manufacturer 1801 --in-stock --pages 1 --json` prints `… N left today`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `candidates.tsv` lists every in-stock, Active or Last Time Buy part of the
  five DigiKey categories for the three makers, with no listing cut short by
  the 300-result cap (§ 5), and has no row with `decision` = `candidate` or
  `check`: every row is `add`, `pending`, `skip` or `out` with a `reason`.
- `datasheets/diode/<maker>/` holds each maker-site PDF that at least one
  `add` or `pending` row needs; `datasheets/diode/datasheets.tsv` maps each
  file to its URL, size and SHA-256.
- `datasheets/skipped-parts.md` has a "Diodes (plan-29)" section listing
  every `skip` row; `obsolete-not-included.md` has a row for each PDF whose
  every part is `out` for `datasheet status`.
- `specs.tsv` exists with one row per (maker, category, datasheet PDF,
  supplier package) of the `add` and `pending` rows, its drawing columns
  empty (B–G fill them).

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter; `pending` rows are decided in B–G.
- No datasheet hunting on maker sites beyond the DigiKey link and the
  same-maker sibling check (§ 3 rule 3).

## 2. Current State

The library has no diode: no `diode` kind, no diode footprint
(`grep -c -i 'SOD\|SMA\|DO-\|MELF' backend/tests/golden/footprints.txt` → 0),
no `datasheets/diode/`. The only diode-like part is `GENERIC-LED` (kind
`led`, passive: it drives and passes nothing, `backend/src/models/led.c`).
The app already draws a diode symbol for the LED (`DiodeSymbolView.swift`).

### Measured populations

All DigiKey numbers 2026-11-08, in stock (`--in-stock`), status Active or
Last Time Buy (`--status 0 --status 4`). Category ids: 280 Single Diodes,
286 Diode Arrays, 299 Bridge Rectifiers, 287 Single Zener Diodes, 295 Zener
Diode Arrays. Maker ids: 112 Vishay General Semiconductor - Diodes Division,
488 onsemi, 1801 Taiwan Semiconductor Corporation.

| What | Count | Command |
|---|---|---|
| Makers ranked by in-stock parts over the four diode groups (all statuses) | Vishay 7482, onsemi 3752, Taiwan Semi 3369, then Nexperia 2821, Renesas 2274 | `scripts/digikey.js --category <280\|286\|299\|287\|295> --in-stock --pages 1 --json` (5 calls), `Manufacturers` facet summed |
| In scope, per category: Vishay / onsemi / TSC | 280: 3682 / 1113 / 1884; 286: 693 / 259 / 291; 299: 382 / 101 / 377; 287: 2242 / 1547 / 596; 295: 39 / 27 / 30 | `scripts/digikey.js --category <c> --manufacturer <m> --in-stock --status 0 --status 4 --pages 1 --json` (15 calls) → `ProductsCount` |
| Total in scope | **13263** (Vishay 7038, onsemi 3047, TSC 3178) | sum of the row above |
| …of which Last Time Buy | 0 | same 15 calls, `Status` facet: only `Active` |
| …of which Military grade | 0 | same calls, `Grade` facet: only `-` and `Automotive` |
| …Automotive grade, per category V / o / T | 280: 1251 / 271 / 864; 286: 212 / 49 / 145; 299: 0 / 0 / 129; 287: 1089 / 535 / 100; 295: 2 / 13 / 0 | same calls, `Grade` facet |
| …`out` by mounting (chassis, stud, QC terminal) | 280: 188 / 0 / 0; 286: 76 / 0 / 0; 299: 93 / 25 / 38; 287, 295: 0 | same calls, `Mounting Type` facet (280 Vishay: Chassis 32 + Chassis, Stud 153 + Stud 3) |
| …`out` as SiC | 280: 83 / 94 / 11; 286: 17 / 12 / 0 | same calls, `Technology` facet |
| Listed in full at planning time | 280 Vishay 3682 of 3682 (270 calls), 280 TSC 1884 of 1884 (164 calls) | `planning/plan-29-data/census/list.sh /tmp/p29/raw` summary lines |
| **Cut by the 300-result cap** | 280 onsemi: 462 of 1113; the piece `series=-` (950 parts) cannot be split | `list.sh` → `digikey: series=-: 950 parts and no way to split it; only the first 300 listed` (every parametric facet of that piece covers 931–942 of the 950, so `scripts/digikey.js`'s `bestSplit` refuses all) |
| Not yet listed | 286, 299, 287, 295 (all makers); 6584 parts | the listing was stopped to leave the shared quota (163 calls) to other sessions |
| Listing cost | 0.073 and 0.087 calls per part (Vishay, TSC singles) | 270 / 3682, 164 / 1884 |
| Phase 1 call budget | about 1250: 13263 parts × 0.087 = 1154 (968 at Vishay's rate), plus § 5's 49 prefix searches at 1–2 calls each | arithmetic on the rows above |
| Single-diode rows after `select.py` (planning time, onsemi partial) | 6028: `candidate` 5576, `check` 94 (mounting blank), `out` 358 (mounting 188, SiC 170) | `python3 -I census/build.py /tmp/p29/raw /tmp/p29/all.tsv` → `6028 parts`; `census/select.py` summary |
| …by leaf | Fast recovery 2332, Schottky 1921, Rectifier 1221, Switching 196 | `awk -F'\t' '$24=="candidate"\|\|$24=="check" {print $26}' planning/plan-29-data/candidates.tsv \| sort \| uniq -c` |
| …automotive | 2093 | same, `$27=="automotive"` |
| Distinct datasheet URLs of those rows | 2207; 2125 downloaded as a PDF | `census/fetch.py` → `2207 urls; 2125 are PDFs` |
| Rows whose link is the maker's own site and downloaded | 5311 (2051 distinct PDFs, 490 MB) | `census/textscan.py` then the host test of § 3 rule 3 |
| …part number printed verbatim | 3936 (Vishay 2237, TSC 1424, onsemi 275) | `$28==1 && $29==1` on maker-site rows |
| …not verbatim (→ `pending`) | 1375 (Vishay 1078, TSC 289, onsemi 8) | `$28==1 && $29==0` on maker-site rows |
| Rows with no maker-site PDF of their own | 359 (TSC 160, Vishay 96, onsemi 103): no URL 116, `mm.digikey.com` 101, `rocelec.widen.net` (Rochester) 59, `waldom.com` 22, `snapeda.com` 11, `vishay.ru` 1, maker-site link that failed 49 | `census/sibling.py planning/plan-29-data/candidates.tsv` and the host counts |
| …printed verbatim in another same-maker PDF (sibling) | 54 (TSC 31, Vishay 16, onsemi 7) | `census/sibling.py` → `found verbatim in a same-maker PDF {'TSC': 31, 'Vishay': 16, 'onsemi': 7}` |
| Verbatim rows with an obsolete / discontinued / NRND / end-of-life word within 300 characters | 44 (40 onsemi, e.g. `1N4003G`, `MUR220G`; 4 Vishay `VS-…ETL06…`) | `awk -F'\t' '$30==1' planning/plan-29-data/candidates.tsv \| wc -l` → 44 |
| Footprint proxy lines (maker, supplier package, mounting) with a PDF, singles | 135 (Vishay 58, onsemi 35, TSC 42), 108 distinct packages | `grep -vc '^#' planning/plan-29-data/footprints-needed.txt` → 135 |
| Supplier packages of the other categories (facets), V / o / T | 286: 27 / 25 / 9; 299: 29 / 8 / 20; 287: 20 / 21 / 14; 295: 1 / 2 / 1; 66 package names that no single diode uses | the 15 count calls, `Supplier Device Package` facet |

### Verified properties

- **Downloads.** vishay.com, onsemi.com and taiwansemi.com answer a bare
  `curl -sL` with `200 application/pdf` (`1n4001.pdf` 74632 bytes,
  `mbr0520lt1-d.pdf` 193198, `pdf.php?pn=SS14` → `SS12 SERIES_R2305.pdf`
  301142). onsemi.com answers plan-27's Safari-header and `Claude-User`
  attempts with 403 (118 URLs in the first run), so `census/fetch.py` tries a
  bare curl first. taiwansemi.com is slow under 8 parallel requests (about
  3 s per file alone); the 2207 URLs took about 70 minutes.
- **Prefix keywords narrow a listing.** `scripts/digikey.js --category 280
  --manufacturer 488 --in-stock --status 0 --pages 1 --json "MBRS"` → 22
  parts, every one `MBRS…` (1 call). § 5 relies on this.
- **Ordering tables are often patterns** (read with `pdftotext -layout`):
  Vishay `1n4001.pdf` "ORDERING INFORMATION (Example)" shows only
  `1N4004-E3/54` and `1N4004-E3/73` (package codes 54, 73; code 53 is
  printed nowhere in the PDF, so `1N4001-E3/53` is not in its table); Taiwan
  Semi `SS12 SERIES_R2305.pdf` prints `SS1x`, "x defines voltage from
  20V(SS12) to 150V(SS115)"; onsemi `mbr0520lt1-d.pdf` lists every number
  (`MBR0520LT1G`, `MBR0520LT3G`, `SBR80520LT1G`, …). Hence `pending` and
  the regex step in B–G (plan-29-B § 4.5).
- **Token-boundary matching.** `census/textscan.py` matches the part number
  between non-alphanumeric boundaries, so `SS14` does not hit inside
  `SS140` and `1N4001-E3/53` does not hit `1N4001-E3/54`.
- **Status ids**: DigiKey's `Status` facet on category 280 lists Active 0,
  Obsolete 1, Discontinued at DigiKey 2, Last Time Buy 4, Not For New
  Designs 7 (`scripts/digikey.js --category 280 --in-stock --pages 1
  --json`), so `--status 0 --status 4` is exactly Active + Last Time Buy; no
  in-scope part is Last Time Buy today.

## 3. Selection rules (the inclusion rule, applied)

A `candidate` or `check` row becomes `add` only when all of these hold; any
other outcome is `skip` or `out` with the reason in the `reason` column.

1. **In stock** (quantity above zero) and **status Active or Last Time Buy**
   at DigiKey. Obsolete, Discontinued at DigiKey, Not For New Designs and
   made-to-order parts are never listed (`list.sh` asks for statuses 0 and 4
   with `--in-stock`).
2. **Scope** (`census/select.py`): mounting Surface Mount, Surface Mount
   Wettable Flank or Through Hole (else `out`, reason `mounting`);
   technology not SiC (`out`, `sic`); a blank mounting is `check`, decided
   from the datasheet in B–G. Drawer leaf (`leaf`): Zener categories →
   Zener; bridges → Bridge; Schottky, Super Barrier and FERD technology →
   Schottky; speed "Small Signal" → Switching; any other "Fast"/"Recovery
   =<" speed → Fast recovery; else Rectifier. Flags (`flags`): Grade
   Automotive → `automotive`, Military → `military`.
3. **A maker datasheet.** The row's `datasheet_url` is on the maker's own
   site (vishay.com; onsemi.com; taiwansemi.com, ts.com.tw) and downloaded
   as a PDF; else a downloaded maker-site PDF of the same maker prints the
   part number verbatim (`census/sibling.py`), and that PDF is its
   datasheet; else `skip`, reason `no datasheet on the maker's site` with the
   link's host (DigiKey copy, Rochester, SnapEDA, none, or the failed
   maker URL).
4. **In the datasheet's ordering table.** The part number is printed
   verbatim (`hit` = 1) → `add`. A row with a PDF and no verbatim hit is
   `pending`: B–G write the datasheet's ordering-code regex in
   `specs.tsv`, and `census/scheme-check.py` sets `add` for a full match and
   `skip` (reason `not in the datasheet's ordering table`) for the rest.
5. **Not marked obsolete, discontinued, NRND or end of life by the
   datasheet.** A `dead` = 1 row is read in its PDF: when its own ordering
   line (or a note it points to) carries the mark it is `out`, reason
   `datasheet status: <the word>`; when the word belongs to another part it
   stays `add`. A PDF whose every row is `out` for datasheet status is
   deleted and listed in `obsolete-not-included.md`.

## 4. Datasheet placement

`census/place.py` (written in Phase 3) copies each needed PDF from
`/tmp/p29/pdf/<md5>.pdf` to `datasheets/diode/<maker-slug>/<name>.pdf`,
`<maker-slug>` `vishay`, `onsemi`, `tsc`, `<name>` the URL path's last
segment (query string dropped; taiwansemi's `pdf.php?pn=SS14` uses the file
name of the redirect target, `SS12 SERIES_R2305.pdf`; `.pdf` appended when
missing; characters outside `A-Za-z0-9._-` replaced by `-`); a clash gets
`-<first 6 hex of the md5>`. It writes `datasheets/diode/datasheets.tsv`:
path, URL, bytes, SHA-256. A PDF no `add` or `pending` row uses is not
copied.

## 5. The 300-result cap: re-check part by part

Any `list.sh` run whose `.err` holds `only the first 300 listed` is cut
short; at planning time that is 280 onsemi `series=-`, whose first 300 by
part number end at `FFSP4065BDN-F085`. For each cut piece, Phase 1 runs
`census/recheck.sh <category> <maker id> <prefix>…`: one keyword search per
part-number prefix (`--category <c> --manufacturer <m> --in-stock --status
0 --status 4 --limit 5000 --json "<prefix>"`), appended to the raw folder.
For 280 onsemi the prefixes are those of onsemi's rectifier part families
that sort after `FFSP`: `FFSD FFSH FFSM FFSP FFSB` are SiC (`out`) and are
not searched; searched are `MBR MBRA MBRB MBRD MBRF MBRM MBRS MMBD MMSD
MSB MUR MURA MURD MURF MURH MURS NRVB NRVA NRVHP NRVUS NSR NSVR NTSB NTST
NTSV RB RFN RHRD RHRG RHRP RS1 RURD RURG RURP S1 S2 S3 SB SBR SBS SBT SMB SS1
SS2 SS3 SUR TUR UF US1` (49 prefixes). Then each base number in onsemi's ordering
tables (the downloaded PDFs) that is still not in the listing gets one
keyword search of its own (part by part). The re-check ends when the onsemi
280 listing reaches DigiKey's `ProductsCount` for that category, or when the
ordering-table numbers are exhausted; any shortfall is written in
§ Corrections with the count, and those parts are not added (they cannot be
shown to be in stock). Any other cut piece found in Phase 1 gets the same
treatment with the prefixes of its own listed rows' part families.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The census (two quota days)

- [x] Day 1 (≥ 600 calls left): `planning/plan-29-data/census/list.sh
      /tmp/p29/raw` (load `DIGIKEY_*` from `~/.zshrc`, as the script does).
      It skips pieces whose `.done` file exists, so a piece cut by the
      quota is re-run whole the next day: delete that piece's `.jsonl` /
      `.err` first. Ran 2026-11-09 (999 calls left): the 12 pieces not
      listed on 2026-11-08 (`/tmp/p29/list-day2.log`); the three 280 pieces
      kept their planning-day listing (§ Corrections).
- [x] ~~Day 2: the same command again, until all 15 pieces have a `.done`.~~
      — moot: all 15 pieces have a `.done` after Day 1
      (`ls /tmp/p29/raw/2??-{vishay,onsemi,tsc}.done | wc -l` → 15).
- [x] Every piece's summary `N of M parts` has N = M, or the piece is cut
      (§ 5): write `census/recheck.sh` and run it for each cut piece.
      Short pieces: 280 onsemi (cap, 462 of 1113), 287 onsemi (cap,
      `series=-: 1080 parts`, 766 of 1546), 287 TSC (DigiKey answered one
      page with HTTP 500: 563 of 596), 299 onsemi (100 of 101: one part
      number listed twice, 101 product rows, 100 distinct). `recheck.sh`
      runs (`/tmp/p29/recheck.log`, `recheck287.log`): 280 onsemi with
      § 5's 49 prefixes plus 24 package keywords (AXIAL, SOD123, SMA,
      DO41, …) → 1107 of 1110; 287 onsemi with 43 Zener family prefixes →
      1541 of 1546; 287 TSC with 26 → 594 of 596. Shortfall 10 parts
      (§ Corrections).
- [x] Compare each piece's count with § 2; a different count goes into
      § Corrections with the date and is carried into B–L's goal tables.

Acceptance: every in-scope part is listed.
  Check: `python3 -I planning/plan-29-data/census/build.py /tmp/p29/raw /tmp/p29/all.tsv` → `<N> parts` with N = the sum of the 15 `ProductsCount` values of the day (13263 at planning time) minus the § 5 shortfall in § Corrections (est. 2 days of quota; nothing smaller re-measures stock).
  Result 2026-11-09: `13249 parts`; the 15 counts sum to 13260 (Vishay
  7039, onsemi 3043, TSC 3178; 280 onsemi re-counted today: 1110); 13260 −
  10 unlisted − 1 duplicate part number = 13249.

### Phase 2 — Downloads and scans

- [x] `python3 -I census/select.py /tmp/p29/all.tsv /tmp/p29/cand.tsv`;
      `census/fetch.py /tmp/p29/cand.tsv /tmp/p29/pdf` (the 2125 PDFs of
      the planning run are cached if `/tmp/p29/pdf` survives; else about
      70 minutes for singles and more for the other categories);
      `census/textscan.py /tmp/p29/cand.tsv /tmp/p29/pdf
      planning/plan-29-data/candidates.tsv`; `census/sibling.py`.
      Results 2026-11-09: select → 12419 candidate, 197 check, 416 out
      (mounting), 217 out (SiC); fetch → `3563 urls; 3386 are PDFs`;
      textscan (candidate + check, pdf/hit/dead) → 6442 hit, 148 hit and
      dead, 5465 PDF without a verbatim hit, 561 no PDF; sibling → need
      TSC 285, Vishay 497, onsemi 404, found verbatim in a same-maker PDF
      TSC 39, Vishay 30, onsemi 64.
- [x] `python3 -I census/footprint-lines.py planning/plan-29-data/candidates.tsv
      > planning/plan-29-data/footprints-needed.txt`; write the per-letter
      line counts (B: Vishay single; C: onsemi single; D: TSC single; E:
      zener, zener-array; F: array; G: bridge) into § Corrections when they
      differ from B–G's tables. Lines: B 58, C 59, D 42, E 52, F 57, G 45
      (313; § Corrections).

Acceptance: `candidates.tsv` covers every listed row.
  Check: `awk 'END{print NR-1}' planning/plan-29-data/candidates.tsv` equals the Phase 1 `<N>` (est. 2 h, the downloads; nothing smaller fetches the PDFs).
  Result: 13249 = N.
Commit: 75a9190

### Phase 3 — Place the datasheets

- [x] Write `census/place.py` per § 4 (with `--verify`; also `--prune`, used in
      Phase 4). Files with the same SHA-256 under two URLs share one file.
- [x] Run it for every row that § 3 rule 3 gives a PDF; `git add` only
      `datasheets/diode/`. `3049 PDFs placed for 3229 URLs` (730 MB; the
      tree already holds 3.7 GB of datasheet PDFs as plain files).

Acceptance: every row with a maker datasheet has its PDF in the tree.
  Check: `python3 -I planning/plan-29-data/census/place.py --verify planning/plan-29-data/candidates.tsv` → `0 missing` (est. 2 min).
  Result: `3229 URLs, 3049 PDFs, 0 missing, 0 unused`.
Commit: 9543c80

### Phase 4 — First decisions

- [x] Write `census/decide.py` applying § 3 rules 1–5: maker PDF and `hit`
      and not dead → `add`; maker PDF and no hit → `pending`; no maker PDF →
      `skip` with the host reason; `dead` → read (rule 5) and record each
      verdict in `census/dead-verdicts.tsv` (mpn, PDF, page, word, `out` or
      `add`), which `decide.py` reads. The verdicts are written by a new
      `census/dead-scan.py` (§ Corrections: the `dead` flag misses parts),
      406 verdicts, 13 `out`; checked by hand on s2m-d.pdf (S2A/G/J/K under
      "DISCONTINUED (Note 2)", S2B/D/M live), mur8100e-d.pdf, mbr1060-d.pdf
      (MBR1060G live, MBR1060 discontinued), 1N4001-D.PDF (every …G live).
- [x] Write `census/scheme-check.py` (used by B–G): applies `specs.tsv`'s
      `mpn_regex` to the `pending` rows of each spec row.
- [x] `datasheets/skipped-parts.md`: a "Diodes (plan-29)" section, one table
      row per (maker, reason): the part numbers, "would have gone into
      `diode` / Discrete → Diodes", the reason (format of the existing
      sections).
- [x] Delete from `datasheets/diode/` any PDF no `add` or `pending` row
      uses (`decide.py` prints them); add to `obsolete-not-included.md` each
      PDF whose every part is `out` for datasheet status (PDF name, `diode`,
      Discrete → Diodes → <leaf>, the word and page, the URL). One:
      `onsemi/mur8100e-d.pdf` (its one in-stock part, MUR880EG, is under
      "DISCONTINUED (Note 1)"); `place.py --prune` → `1 PDFs deleted, 3048
      kept`; `place.py --verify` → `3048 URLs, 3048 PDFs, 0 missing, 0 unused`.
- [x] Record the counts of each `decision` in § Summary.

Acceptance: no row is `candidate` or `check`; every `skip` is listed.
  Check: `awk -F'\t' 'NR>1 && ($24=="candidate" || $24=="check")' planning/plan-29-data/candidates.tsv | wc -l` → 0, and `python3 -I planning/plan-29-data/census/decide.py --verify-skipped` → `0 skip rows missing from skipped-parts.md` (est. 1 min; the `dead` reading is about 44 rows × 1 min).
  Result 2026-11-09: 0; `0 skip rows missing from skipped-parts.md`.
Commit: e1e1cce

### Phase 5 — `specs.tsv` skeleton and the data README

- [x] `planning/plan-29-data/specs.tsv`: header `maker category datasheet
      supplier_package mpn_regex package pinout pins footprint vr_io_checked
      note`, one row per distinct (maker, category, datasheet PDF, supplier
      package) of the `add` and `pending` rows, `mpn_regex` `.*` for
      groups with no `pending` row, the drawing columns empty. Written by
      `census/specs-skeleton.py`: `3198 rows`, 643 with an empty
      `mpn_regex` (they have pending rows). `datasheet` is the PDF's path.
- [x] `planning/plan-29-data/README.md`: update the file map.

Acceptance: one spec row per group.
  Check: `awk -F'\t' '$24=="add" || $24=="pending" {print $2"|"$1"|"$23"|"$22}' planning/plan-29-data/candidates.tsv | sort -u | wc -l` equals `awk 'END{print NR-1}' planning/plan-29-data/specs.tsv` (est. 1 min; for sibling rows `$23` is the sibling PDF's URL, which `decide.py` writes into `datasheet_url`).
  Result: 3198 = 3198.
Commit: 2dbcd2b

## Validation Plan

- Tests: none (data only). The scripts check themselves (`--verify` modes,
  Phases 3–4).
- Coverage check: Phase 5's equality shows every kept row has a spec row.
- Runtime proof: not applicable; no part is loaded.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate (once, after Phase 5): `cmake --build /tmp/logic-p29 -j8 &&
  ctest --test-dir /tmp/logic-p29` → `100% tests passed` (est. 1 min; the
  data-only letter changed nothing the tests read).

## Decisions

- **Makers** — owner, 2026-11-08: Vishay, onsemi, Taiwan Semiconductor (the
  measured top 3, § 2).
- **Sub-types** — owner, 2026-11-08 ("based on what you already have pick
  the top 3 makers"): everything those makers have in the four diode groups
  DigiKey files as diodes: single diodes, Zeners, Zener arrays, diode arrays,
  bridge rectifiers. TVS diodes (a circuit-protection category, 144) are out.
- **Extras** — owner, 2026-11-08: automotive parts in, flagged
  `automotive`; military parts in, flagged `military` (none exist today,
  § 2); chassis / stud / QC-terminal mounting out; SiC Schottky out.
- **Simulation** — owner, 2026-11-08: one-way logic (plan-29-H).
- **Inclusion rule** — owner, 2026-11-08: in stock, in the datasheet's
  ordering table, Active or Last Time Buy (§ 3).
- **Maker-site datasheets only, with the sibling check** — author: matches
  `docs/parts-format.md` § Datasheets ("the manufacturer's own site … no
  distributor or archive copies"); a DigiKey or Rochester copy is not a
  vendor datasheet. Rejected: searching maker sites for each skipped part.
- **Pattern ordering tables count only what they print** — author (§ 3 rule
  4; plan-29-B § 4.5). Rejected: counting a DigiKey-only package code.
- **Stop the planning census at 163 calls** — author: the quota is shared
  (plan-27's census also waits on it); Phase 1 re-lists everything anyway.
- **Plan number** — author: `plan-29` was free on 2026-11-08 (highest plan
  in `planning/` and `planning/completed/` was `plan-28`; `git log --all
  --oneline | grep -c plan-29` → 0; re-checked before writing and before
  committing).

## Corrections

- **Baseline moved before the plan started** — the Prerequisites were measured at
  8cce726 (T0 = 27, P0 = R0 = 13158, F0 = 508); main took plan-27's merge
  (d12a1bb) before plan-29 was committed. Re-measured at 0f87917: T0 = 28,
  P0 = R0 = 15189, F0 = 766. Main moved again before Phase 1 (merged at
  f68e13f, 2026-11-09): T0 = 29, P0 = R0 = 15214, F0 = 772
  (`grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l` of the three
  goldens). Every letter's "T0 + 1" and golden-growth checks use these
  values; H § 1 and N's table are corrected to them.
- **Counts moved** (2026-11-09 against § 2's 2026-11-08 values): Vishay
  286 693 → 692, 287 2242 → 2243, 299 382 → 383; onsemi 280 1113 → 1110
  (`--pages 1` count), 287 1547 → 1546. Total in scope 13263 → 13260. The
  per-letter goal tables (B–L) take their numbers from Phase 2's tables,
  which replace the planning-time ones.
- **A third short piece, and a duplicate** — 287 onsemi was capped too
  (`series=-: 1080 parts and no way to split it`), and 287 TSC lost pages
  to a DigiKey HTTP 500 (`POST /search/keyword failed (500)`); both were
  re-checked by prefix (§ 5's method). 299 onsemi's "100 of 101" is one
  part number DigiKey lists as two products, not a gap.
- **§ 5 shortfall: 10 parts** (2026-11-09): 280 onsemi 3 (1107 of 1110),
  287 onsemi 5 (1541 of 1546), 287 TSC 2 (594 of 596). The part-by-part
  search of every ordering-table base number was not run: it costs one
  call per number (thousands), and 141 calls were left. These 10 parts
  are not listed, so they are not added (they cannot be shown in stock).
  Package keywords (`AXIAL` alone found 17 onsemi rectifiers the `UF`
  prefix missed) were the better splitter; one, `SMA`, cost 79 calls.
- **Footprint lines per letter** (Phase 2, 2026-11-09, `footprint-lines.py`,
  candidate + check rows with a PDF): B 58 (Vishay single; planning 58),
  C 59 (onsemi single; planning 35 from a partial listing), D 42 (TSC
  single; planning 42), E 52 (zener + zener-array), F 57 (array), G 45
  (bridge). B–G's scope numbers are these.
- **The `dead` flag misses parts** (Phase 4) — textscan.py's `dead` looks
  only within 300 characters of the *first* place a part number is
  printed, so onsemi's S2A (first in the ratings table, again under
  "DISCONTINUED (Note 2)" on p.2 of s2m-d.pdf) read as `add`. Rule 5 is
  applied by `census/dead-scan.py` to every hit row of a PDF with a dead
  word: onsemi's DISCONTINUED sections (read in reading order: on
  two-column pages `-layout` splits the heading onto another column's
  line), plus revision-history lines that mark OPNs discontinued; case
  outline notes ("267-04 OBSOLETE, NEW STANDARD 267-05"), Fairchild's status
  legend ("Obsolete / Not In Production") and Vishay's "Discontinuous Mode
  PFC" name no part. 148 rows had `dead` = 1; 13 parts are `out`. The plan's
  example `1N4003G` is live (1N4001-D.PDF p.5 lists it above the
  DISCONTINUED heading, under which only `1N4003` / `1N4003RL` sit).
- **`place.py` matched `.pdf` case-sensitively** (found by plan-29-N's
  datasheet gate): `--prune` and `--verify` skipped onsemi's `.PDF` files,
  so `onsemi/NSR0530H-D.PDF` stayed after its one row became skip. Fixed
  (`f.lower().endswith(".pdf")`) and pruned: 3010 PDFs for 3010 URLs.
- **Phase 1 reuses the planning-day 280 listings** — `list.sh` skips a piece
  whose `.done` exists, and `/tmp/p29/raw` kept `280-vishay`, `280-tsc` and
  `280-onsemi` from 2026-11-08, so Day 1 (2026-11-09) listed the other 12
  pieces. Their stock is a day older than the rest; Phase 1's count check
  compares them with the planning-day `ProductsCount` (§ 2).

## Summary

Decisions (2026-11-09, `candidates.tsv`, 13249 rows): **add 6559**,
**pending 4991**, **skip 1053** (no maker-site datasheet: DigiKey copy
440, HTTP 404 133, no URL 172, Rochester 177, a copy on another site 68,
Waldom 35, SnapEDA 21, other 7), **out 646** (mounting 416, SiC 217,
datasheet status 13). 133 rows take a same-maker sibling PDF.

A's risk is volume and the quota, not design: 13263 parts to list over two
days, one piece already known to be capped (onsemi singles, re-checked by
prefix and part by part), about 2000+ PDFs, and about a quarter of the rows
behind pattern ordering tables. Left untouched: every part, footprint,
backend and app file.
