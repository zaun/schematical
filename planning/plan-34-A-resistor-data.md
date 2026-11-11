# plan-34-A: Resistors — census, part-by-part stock check, datasheet downloads

Last updated: 2026-11-11
Overall Effort: huge (> 3d) — plan-34 is A → B → C → D → E → F → G → H
Effort: large (3h–1d; plus two to three DigiKey quota days of waiting)
Depends on: nothing (§ Prerequisites)

plan-34 adds fixed resistors to the parts library and the app: chip
resistors, through-hole (axial) resistors and resistor networks/arrays from
the three makers with the most resistors in stock at DigiKey (Viking Tech,
YAGEO, Vishay Dale), plus a `GENERIC-RESISTOR` (axial, 1 kΩ by default, any
value). They sit in the parts drawer under **Resistors → Chip / Through-hole
/ Array → <ohms group>**, the groups `< 1 kΩ`, `1 kΩ – 5 kΩ`, `5 kΩ – 10 kΩ`,
`10 kΩ – 50 kΩ`, `50 kΩ – 100 kΩ`, `100 kΩ – 500 kΩ`, `500 kΩ – 1 MΩ`,
`≥ 1 MΩ` in that order, with **Resistors → Generic** beside them. In play
mode a resistor is a weak two-way link: each end sees the other end's level
as a weak drive, so a resistor to VCC works like a pull-up and a real driver
overrides it.

The owner wants about 2000 parts, the most-stocked ones, with no more than
two that are basically the same thing. So the census does not take every
in-stock part (85807 of them, § 2): it lists the most-stocked parts of each
slice, sorted by stock, and `census/select.py` keeps at most two parts per
(category, ohms, tolerance, size) and fills a quota per category, highest
stock first (§ 3).

The single behavioral outcome of plan-34: loading the part files gives the
baseline 26729 parts (P0, § Prerequisites) plus `GENERIC-RESISTOR` plus one
`resistor` part per `add` row of `planning/plan-34-data/candidates.tsv`
(about 2000: chip 1500, through-hole 400, arrays up to 100); the app lists
them under the drawer leaves above in ohms order; a placed generic resistor
is `R1`, 1 kΩ, on `RES-AXIAL-10.16`; an inverter driving an LED through a
resistor lights the LED; `ctest` passes T0 + 1 programs (`resistor_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | census re-listing of the capped pieces, the part-by-part stock check, the first pick, datasheet downloads and placement, the verbatim scan, `specs.tsv` skeleton | large |
| B | ordering-code schemes per datasheet, verdicts, the final pick, skipped parts, obsolete PDFs, `footprints-needed.txt` | large |
| C | footprints: chip resistors (every size, every maker's land pattern), the shared resistor footprint rules | large |
| D | footprints: axial through-hole resistors and resistor arrays (chip arrays, SIP, SOIC) | large |
| E | backend: resistor links in the engine, the `resistor` kind, the voltage carry, `resistor_test`, docs | large |
| F | parts: the generator, `GENERIC-RESISTOR`, every `add` row, goldens | large |
| G | app: drawer order, the resistor symbol, facts, `kindName`, `R<n>` references, the generic's Ohms field, file `ohms`, BOM | large |
| H | landing tier | small |

Outcome of A: `/tmp/p34/raw` holds a listing whose every cut piece is
either complete for the pick (its 300th part has less stock than the
category's floor, § 5) or re-listed; `planning/plan-34-data/stock-check.tsv`
confirms each picked part in stock part by part; `datasheets/resistor/<maker>/`
holds every maker-site PDF an eligible row cites, with
`datasheets/resistor/datasheets.tsv`; `specs.tsv` has one row per
(maker, series, datasheet), its scheme columns empty (B fills them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-34-data/` — `census/top.js`, `list.sh`, `recheck.sh`,
  `select.py`, `footprint-lines.py`; `raw/*.jsonl.gz` and `raw/*.err` (the
  planning listing), `candidates.tsv`, `footprints-needed.txt`, `README.md`
  (written at planning time, 2026-11-11).
- `planning/completed/plan-29-A-diode-data.md` and `planning/plan-29-data/census/`
  (`fetch.py`, `textscan.py`, `sibling.py`, `place.py`, `specs-skeleton.py`:
  the scripts A adapts).
- `planning/plan-33-A-capacitor-data.md` (open, committed at 00e79a9 while
  this plan was researched; the same "most-stocked per bucket" idea for
  capacitors, § Decisions "Shared quota").

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p34 && cmake --build /tmp/logic-p34 -j8 && ctest --test-dir /tmp/logic-p34 -j8` → `100% tests passed` | MET 2026-11-11 at 8b1770b: `100% tests passed, 0 tests failed out of 30`; **T0 = 30** (`grep -c '^logic_test' backend/CMakeLists.txt` → 30) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` | MET: **P0 = 26729**, **R0 = 26729**, **F0 = 1212** (2026-11-11 at 8b1770b) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p34-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-11 at 8b1770b: `** BUILD SUCCEEDED **`) |
| No resistor kind yet | `grep -c 'PART_RESISTOR' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/resistor.json` → no such file; `ls datasheets/resistor` → no such directory | MET (2026-11-11 at 8b1770b: 0, no file, no directory) |
| No other open plan adds resistors | `grep -l -i '"resistor"\|PART_RESISTOR\|datasheets/resistor\|GENERIC-RESISTOR' planning/plan-*.md \| grep -v plan-34 \| wc -l` → 0 | MET (2026-11-11: 0) |
| DigiKey credentials and quota | `DIGIKEY_CLIENT_ID`/`_SECRET` exported in `~/.zshrc`; Phase 1 re-lists five pieces (the one re-listed at planning time cost 189 calls before the quota cut it, `52-dale-re-1.err`; `recheck.sh` skips finished pieces, so a cut run goes on the next day) and Phase 2 costs one call per checked part, 1778 at planning time (§ 2); a day allows about 1000, shared with other sessions | **NOT MET today** (2026-11-11: the planning census ended at `0 left today`; credentials present). Start Phase 1 on a day whose first call reports ≥ 600 left (`/tmp/p34/dk.sh --category 50 --manufacturer 2577 --in-stock --pages 1 --json` prints `… N left today`; `dk.sh` is § 6's wrapper) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- Every piece of the listing is complete for the pick: `python3 -I
  census/select.py /tmp/p34/raw /tmp/p34/cand.tsv --report` prints no
  `NEEDS DEEPER LISTING` line (§ 5).
- `planning/plan-34-data/stock-check.tsv` has one row per picked row that
  came from a cut piece (1778 at planning time), each `found` with stock
  above 0, status Active or Last Time Buy; a row that fails is `out`
  (reason `stock check`) and the pick is re-run until every picked row from
  a cut piece passes (§ 5).
- `datasheets/resistor/<maker>/` holds each maker-site PDF that at least one
  eligible row needs (85 distinct URLs at planning time);
  `datasheets/resistor/datasheets.tsv` maps each file to its URL, size and
  SHA-256.
- `candidates.tsv` has the columns `pdf`, `hit`, `dead` filled for every
  eligible row; `specs.tsv` exists with one row per (maker, series, PDF) of
  the eligible rows, its scheme columns empty (B fills them).

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter, and no row is `add` yet: the datasheet
  verdicts and the final pick are B's.
- No datasheet hunting on maker sites beyond the DigiKey link, the
  same-maker sibling check and the one YAGEO URL rewrite of § 4 rule 3.
- `scripts/digikey.js` is not changed: the census needs sorting by stock and
  parameter filters, which it does not offer, so `census/top.js` calls the
  API itself (§ 6).

## 2. Current State

The library has no resistor kind and no resistor part. The seven pull parts
(`PULLUP-5V` … `PULLDOWN`, `supply.json`, `"pull": true`) are drawn as
resistors and are real resistors on the board since plan-26: each placed
pull has `ohms` (default 1000) and one of six generic footprints,
`RES-0402`, `RES-0603`, `RES-0805`, `RES-1206`, `RES-AXIAL-10.16`,
`RES-RADIAL-2.54` (`grep -c '"id":"RES-' schematical/schematical/Parts/footprints.json`
→ 6, plan-26-A § 4: generic, not from a datasheet). The app's Ohms field
(`Drawer/PartOptionsSection.swift:OhmsField`), `Canvas/ResistorValue.swift`,
the file's `ohms` (`docs/file-format.md`, pulls only), the BOM's `RESISTOR`
lines and the `R<n>` references (`Canvas/SchematicModel.swift:takeReference`)
exist for the pulls; G widens them to the generic resistor.

### Measured populations

All DigiKey numbers 2026-11-11, in stock (`--in-stock`). Category ids: 52
Chip Resistor - Surface Mount, 53 Through Hole Resistors, 50 Resistor
Networks, Arrays, 54 Chassis Mount Resistors, 55 Specialized Resistors.
Maker ids: 2577 Viking Tech, 13 YAGEO, 541 Vishay Dale. `dk.sh` is § 6's
wrapper around `node scripts/digikey.js`.

| What | Count | Command |
|---|---|---|
| In stock per category, all makers, all statuses | 52: 177573; 53: 35914; 50: 4240; 54: 8374; 55: 1012 | `dk.sh --category <c> --in-stock --pages 1 --json` (5 calls) → `ProductsCount` |
| Makers ranked by in-stock chip + through-hole parts (all statuses) | Viking Tech 36600, YAGEO 26280, Vishay Dale 23929, then TE Connectivity 18334, Panasonic 17502, KOA 15187 | same 5 calls, `Manufacturers` facet of 52 + 53 summed |
| …Vishay as one company (all its DigiKey divisions) | 32122 (Dale 23929, Vishay 3709, Beyschlag 3984, Sfernice 891, Techno 687, Dale Thin Film 718, others 204) | same facets |
| In scope (Active + Last Time Buy), per maker, 52 / 53 | Viking 36595 / 5; YAGEO 20642 / 5622; Dale 16984 / 5959 | `dk.sh --category <c> --manufacturer <m> --in-stock --status 0 --status 4 --pages 1 --json` (6 calls) → `ProductsCount` |
| Total in scope, chip + through-hole | **85807** | sum of the row above |
| …of which ±1% or ±5% | 47795 (Viking 7250, YAGEO 21319, Dale 19226) | same 6 calls, `Tolerance` facet (±1% 1131, ±5% 2503) |
| …Jumper (0 Ω) | Viking 1, YAGEO 96, Dale 103 | same, `Tolerance` facet `Jumper` (362150) |
| Arrays in stock, all statuses | Viking 10, YAGEO 556, Dale 101 | the category 50 call, `Manufacturers` facet |
| Planning listing (`census/list.sh`, ±1% / ±5% / Jumper, top 300 by stock per piece) | **14882 parts**: 52 Viking 1259, YAGEO 5112, Dale 703 + 6110 re-listed (§ 5); 53 Viking 5, YAGEO 848, Dale 617; 50 Viking 10, YAGEO 553, Dale 15 | `planning/plan-34-data/raw/*.err` summary lines; `select.py --report` → `14882 parts listed` |
| Listing cost | 241 calls for the nine base pieces; 189 more for Dale chip ±1%'s re-listing, cut by the quota (`top.js: rate limited`) | the `top.js:` lines of `raw/*.err` |
| `select.py` at planning time | **1994 picked** (chip 1500, through-hole 400, arrays 94); out: automotive 7017, current-sense 334, duplicate 994, below quota 4543 | `python3 -I census/select.py /tmp/p34/raw /tmp/p34/cand.tsv --report` |
| …picked per maker | YAGEO 1648 (chip 1188, through-hole 384, arrays 76); Vishay Dale 322 (299, 15, 8); Viking Tech 24 (13, 1, 10) | same, `kept per category/maker` |
| …picked per ohms group, chip / through-hole / arrays | < 1 kΩ 563 / 170 / 36; 1–5 k 266 / 66 / 21; 5–10 k 92 / 25 / 5; 10–50 k 236 / 61 / 25; 50–100 k 87 / 17 / 0; 100–500 k 157 / 47 / 7; 500 k–1 M 36 / 4 / 0; ≥ 1 M 63 / 10 / 0 | same, `kept per bucket` |
| …tolerance | ±1% 1447, ±5% 530, Jumper 17 | `awk -F'\t' '$22=="candidate"{print $9}' planning/plan-34-data/candidates.tsv \| sort \| uniq -c` |
| …military | 4 | `awk -F'\t' '$22=="candidate" && $19=="military"' planning/plan-34-data/candidates.tsv \| wc -l` |
| …status | Active 1994, Last Time Buy 0 | same filter, `print $6` |
| Floor (stock of the last picked part) | chip 72373, through-hole 8982, arrays 26 | `select.py --report` → `floor` |
| **Cut by the 300-result cap above the floor** | 52 YAGEO ±1%/0402 (300th stock 150184), ±1%/0603 (104527); 53 YAGEO ±1%/Axial (12170), ±5%/Axial (11308); 52 Dale ±1% (its re-listing stopped at the quota) | `select.py --report` → `NEEDS DEEPER LISTING` lines |
| Picked rows that came from a cut piece | 1778 of 1994 | the pieces of `raw/*.err` with `cut at 300`, matched to each picked row's `piece` |
| Eligible rows (not out for scope) | 7531 (chip 6157, through-hole 1277, arrays 97) | `candidates.tsv` rows whose `reason` is not `automotive` / `current-sense` |
| Distinct datasheet URLs, eligible rows | 85: vishay.com 36, yageogroup.com 27, mm.digikey.com 8, viking.com.tw 7, waldom.com 4, yageo.com 3 | the `datasheet_url` column of those rows, `sort -u` |
| Distinct datasheet URLs, picked rows | 41; a bare `curl -sL` gets 40 PDFs; `yageo-zor_datasheet.pdf` (yageo.com) answers an HTML page | `/tmp/p34/fetch.log` (`%PDF-` 40, `<!DOC` 1); `pdfinfo` on each |
| Picked part numbers printed verbatim in their PDF | 2 of 1994 (one `RC_L`, one `YC158T`) | token-boundary search of `pdftotext -layout` text, as `plan-29-data/census/textscan.py` |
| Footprint lines (category, maker, datasheet, package, body), picked rows | 86: chip 34, through-hole 34, arrays 18 | `grep -vc '^#' planning/plan-34-data/footprints-needed.txt` → 86 |

### Verified properties

- **Sorting by stock works with parameter filters.** A KeywordSearch with
  `SortOptions {Field: "QuantityAvailable", SortOrder: "Descending"}` and a
  `ParameterFilterRequest` on Tolerance (category 52, YAGEO, ±1% + ±5%)
  returned `ProductsCount` 15783 and products in falling stock
  (`AC0603JR-131K5L` 35000000 first; offset 250 → `RC0402FR-0797K6L`
  530958) (`/tmp/p33probe/sortprobe.js`, 2026-11-11). So the first 300 of
  any piece are its 300 most-stocked parts, and a cut piece is complete for
  the pick when its 300th part's stock is below the floor (§ 5).
- **Facet value ids are global.** Tolerance `±1%` is 1131, `±5%` 2503,
  `Jumper` 362150 in categories 52 and 53 alike (the `Tolerance` facets of
  the six per-maker calls).
- **Package / Case does not always cover a piece.** Dale chip ±1% (12018
  parts) has a Package facet whose counts sum to less than the piece, so
  `top.js` did not split it and listed 300 (`52-dale.err`: `piece ±1%: 300
  of 12018`); the series → Features → Composition → Power → TCR order of
  `top.js`'s `SPLITS` split it (`52-dale-re-1.err`, 76 pieces before the
  quota ran out).
- **Downloads.** vishay.com, yageogroup.com and viking.com.tw answer a bare
  `curl -sL` with a PDF (40 of 41 picked URLs, each passes `pdfinfo`);
  `https://www.yageo.com/upload/media/product/app/datasheet/lr/yageo-zor_datasheet.pdf`
  answers YAGEO's HTML home page, while
  `https://yageogroup.com/content/datasheet/asset/file/YAGEO-ZOR_DATASHEET`
  answers `%PDF-` (1367292 bytes).
- **Ordering tables are patterns.** YAGEO `PYU-RC_GROUP_51_ROHS_L` prints
  "GLOBAL PART NUMBER RC XXXX X X X XX XXXX L" with the size, tolerance
  (B/D/F/J), packing (R/K/S), TCR, reel (07/10/13/7W/7D) and value fields
  (`pdftotext -layout`, lines 23–53). So B writes one regex per
  (datasheet, series), as plan-29-B § 4.5 did.

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` (in B) only when all of these hold; any other outcome
is `out` or `skip` with the reason in the `reason` column.

1. **In stock** (quantity above zero) and **status Active or Last Time Buy**
   at DigiKey. `top.js` asks for statuses 0 and 4 with
   `MinimumQuantityAvailable` 1, so Obsolete, Discontinued at DigiKey, Not
   For New Designs and made-to-order parts are never listed. A row from a
   cut piece is confirmed part by part (§ 5).
2. **Scope** (`select.py`): categories 52, 53, 50 only; tolerance ±1%, ±5%
   or Jumper only (`top.js`'s three slices). Features, Ratings, Grade or
   Qualification naming `Automotive` or `AEC-Q` → `out`, reason
   `automotive`; Features `Current Sense` or Composition `Metal Element` →
   `out`, reason `current-sense`. Features `Military` → flag `military`.
3. **At most two of the same thing** (`select.py`): the key is (category,
   ohms, tolerance, size); size is a chip's Package / Case, a through-hole
   part's Package / Case plus its power, an array's Package / Case plus its
   circuit type plus its element count. A key keeps its most-stocked part,
   then the most-stocked part of another maker if the key has one, else
   its next most-stocked part; the rest → `out`, reason `duplicate`.
4. **Large supplies first** (`select.py`): per category, highest stock
   first, the quota chip 1500, through-hole 400, arrays 100; the rest →
   `out`, reason `below quota`. Arrays have fewer eligible rows than their
   quota (97 at planning time, 94 after rule 3), so every one is taken.
5. **A maker datasheet** (Phase 3 here; B's verdict): the row's
   `datasheet_url` is on the maker's own site (`yageogroup.com`,
   `yageo.com`, `vishay.com`, `viking.com.tw`) and downloads as a PDF; else
   a downloaded same-maker PDF prints the part number verbatim
   (`sibling.py`), and that PDF is its datasheet; else `skip`, reason `no
   datasheet on the maker's site` with the link's host (DigiKey copy
   `mm.digikey.com`, `waldom.com`, none).
6. **In the datasheet's ordering table** (B): printed verbatim, or a full
   match of the (datasheet, series) ordering-code regex B writes in
   `specs.tsv`; else `skip`, reason `not in the datasheet's ordering table`.
7. **Not marked obsolete, discontinued, NRND or end of life by the
   datasheet** (B reads every `dead` = 1 row); a PDF whose every row is
   `out` for datasheet status is deleted and listed in
   `obsolete-not-included.md`.

Rules 5–7 are applied to every eligible row (7531 at planning time), not
only to the picked ones, so that B's final pick (`select.py --verdicts`)
can fill a skipped slot with the next part.

## 4. Datasheet placement

`census/place.py` (adapted from plan-29's in Phase 3) copies each needed PDF
from `/tmp/p34/pdf/<md5 of the URL>.pdf` to
`datasheets/resistor/<maker-slug>/<name>.pdf`, `<maker-slug>` `yageo`,
`vishay-dale`, `viking`; `<name>` the URL path's last segment (query string
dropped, `%20` → `-`, `.pdf` appended when missing, characters outside
`A-Za-z0-9._-` replaced by `-`); a clash gets `-<first 6 hex of the md5>`.
It writes `datasheets/resistor/datasheets.tsv`: path, URL, bytes, SHA-256.
Files with the same SHA-256 under two URLs share one file. A PDF no
eligible row uses is not copied.

Rule 3 of `census/fetch.py`: a `www.yageo.com/upload/media/product/app/datasheet/lr/yageo-<series>_datasheet.pdf`
URL that answers HTML is fetched as
`https://yageogroup.com/content/datasheet/asset/file/YAGEO-<SERIES>_DATASHEET`
(the ZOR case, § 2 Verified properties); that URL is the one recorded.

## 5. The 300-result cap: re-list, then re-check part by part

DigiKey returns at most the first 300 results of a search. Because `top.js`
sorts by stock, a piece cut at 300 still holds its 300 most-stocked parts;
the parts it did not list all have stock at or below its 300th part's
(`cut at 300: <stock>` in the `.err` line). `select.py --report` compares
that stock with the category's floor (the stock of the last picked part):
below the floor → the piece is complete for the pick; at or above →
`NEEDS DEEPER LISTING`.

1. **Re-list.** Each `NEEDS DEEPER LISTING` piece gets a `recheck.sh`
   line (`run <cat>-<maker>-re-<n> <cat> <maker id> <tolerance> [<package
   value id> <package name>]`): `top.js` lists that piece again, split by
   the first of Package / Case, Series, Features, Composition, Power,
   Temperature Coefficient whose values cover it, skipping Features values
   that name Automotive or Current Sense (those parts are out, rule 2).
   `select.py` treats a base piece as superseded by a re-listing whose
   piece labels start with it. The five planning-time lines are in
   `recheck.sh`. Re-run `select.py --report` after each re-listing: the
   floor moves as rows are added, so a new `NEEDS DEEPER LISTING` piece
   gets its own line. A piece no facet splits below 300 with its 300th part
   at or above the floor is written in § Corrections with its count; the
   pick takes its listed parts (they are the most-stocked) and the unlisted
   ones cannot be picked.
2. **Re-check part by part** (the owner's rule for a capped listing). Every
   picked row whose piece was cut at 300 (1778 at planning time) is looked
   up by its own part number: `census/stock-check.sh` runs `dk.sh
   --manufacturer <maker id> --in-stock --status 0 --status 4 --limit 5
   --json "<mpn>"` (one call, ids so no lookup calls) and writes
   `planning/plan-34-data/stock-check.tsv` (`mpn`, `found` 0/1, `stock`,
   `status`, `date`); `found` = 1 when a product with exactly that
   `ManufacturerProductNumber` has `QuantityAvailable` above 0. The script
   skips part numbers already in the file, so a run cut by the quota goes
   on the next day. A row with `found` = 0 is `out`, reason `stock check`
   (`select.py` reads `stock-check.tsv` and excludes them), and the pick is
   re-run; newly picked rows from cut pieces are checked the same way until
   none is unchecked.

## 6. Scripts

- `/tmp/p34/dk.sh` (not in the tree; one line per run): loads `DIGIKEY_*`
  from `~/.zshrc` with `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`,
  then `node /Users/justinzaun/Development/logic/scripts/digikey.js "$@"`.
- `census/top.js <cat> <maker id> <out.jsonl> [<tolerance> [<package value id> <name>]]`:
  KeywordSearch sorted by stock, statuses 0 and 4, in stock; slices
  ±1% / ±5% / Jumper; splits a piece above 300 (§ 5); writes one JSON line
  per product (`mpn`, `maker`, `category`, `piece`, `stock`, `status`,
  `datasheet`, `series`, `base`, `description`, `params`). Its own login
  (the same token cache as `digikey.js`).
- `census/list.sh <raw>`: `top.js` for the nine (category, maker) pairs;
  skips a pair whose `.done` exists.
- `census/recheck.sh <raw>`: § 5 rule 1's re-listings.
- `census/select.py <raw> <out.tsv> [--verdicts <tsv>] [--report]`: § 3
  rules 2–4 (and with `--verdicts`, B's rules 5–7); writes `candidates.tsv`.
- `census/footprint-lines.py <candidates.tsv>`: `footprints-needed.txt`.
- Written in this letter: `census/stock-check.sh` (§ 5 rule 2),
  `census/fetch.py`, `census/textscan.py`, `census/sibling.py`,
  `census/place.py`, `census/specs-skeleton.py` (plan-29's, adapted: paths,
  maker hosts, the resistor columns).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Complete the listing (one quota day)

- [ ] Restore the planning listing: `mkdir -p /tmp/p34/raw && for f in
      planning/plan-34-data/raw/*.jsonl.gz; do gunzip -c $f >
      /tmp/p34/raw/$(basename $f .gz); done; cp planning/plan-34-data/raw/*.err
      /tmp/p34/raw/`; `touch` a `.done` for the nine base pieces (their
      listings are complete: each `.err` ends with a `top.js:` summary line);
      delete the four `-re-` files, whose runs the quota cut.
- [ ] On a day with ≥ 600 calls left: `planning/plan-34-data/census/recheck.sh
      /tmp/p34/raw` (the five lines of § 2's cut row).
- [ ] `python3 -I planning/plan-34-data/census/select.py /tmp/p34/raw
      /tmp/p34/cand.tsv --report`; for each `NEEDS DEEPER LISTING` line add a
      `recheck.sh` line (§ 5 rule 1) and run it; repeat until there is none,
      or record the unsplittable piece in § Corrections.
- [ ] Compare the counts with § 2; a different count goes into § Corrections
      with the date and is carried into B's goal table.

Acceptance: every piece is complete for the pick.
  Check: `python3 -I planning/plan-34-data/census/select.py /tmp/p34/raw /tmp/p34/cand.tsv --report | grep -c 'NEEDS DEEPER'` → 0, or each remaining line is in § Corrections (est. 1 quota day; nothing smaller completes the listing).
Commit: —

### Phase 2 — Part-by-part stock check (one to two quota days)

- [ ] Write `census/stock-check.sh <candidates.tsv> <stock-check.tsv>`
      (§ 5 rule 2) and give `select.py` a `--stock <tsv>` option: a row with
      `found` = 0 is `out`, reason `stock check`, before rule 3.
- [ ] Run it on the picked rows from cut pieces (1778 at planning time);
      re-run `select.py --stock`; check the newly picked rows from cut
      pieces; repeat until none is unchecked. Copy the result to
      `planning/plan-34-data/stock-check.tsv`.

Acceptance: every picked row from a cut piece is confirmed in stock.
  Check: `python3 -I planning/plan-34-data/census/select.py /tmp/p34/raw /tmp/p34/cand.tsv --stock planning/plan-34-data/stock-check.tsv --report` and an `awk` join of the picked rows from cut pieces with `stock-check.tsv` → 0 unchecked, 0 picked with `found` = 0 (est. 1–2 quota days at one call per part).
Commit: —

### Phase 3 — Downloads, scans, placement

- [ ] `census/fetch.py /tmp/p34/cand.tsv /tmp/p34/pdf` for every eligible
      row's URL (85 at planning time; § 4's YAGEO rewrite; bare `curl -sL`
      first, as plan-29's `fetch.py`); `census/textscan.py` fills `pdf`,
      `hit`, `dead`; `census/sibling.py` gives a no-PDF row a same-maker PDF
      that prints its number verbatim.
- [ ] `census/place.py` per § 4; `git add` only `datasheets/resistor/`.
- [ ] Copy `/tmp/p34/cand.tsv` to `planning/plan-34-data/candidates.tsv`;
      `census/footprint-lines.py` → `footprints-needed.txt`.

Acceptance: every eligible row with a maker datasheet has its PDF in the tree.
  Check: `python3 -I planning/plan-34-data/census/place.py --verify planning/plan-34-data/candidates.tsv` → `0 missing, 0 unused` (est. 2 min after a 10-minute download).
Commit: —

### Phase 4 — `specs.tsv` skeleton and the data README

- [ ] `census/specs-skeleton.py` → `planning/plan-34-data/specs.tsv`: header
      `maker series datasheet mpn_regex fields_checked package pinout
      footprint note`, one row per distinct (maker, series, PDF) of the
      eligible rows with a PDF; `mpn_regex` empty (B), `package`,
      `pinout`, `footprint` empty (C, D).
- [ ] `planning/plan-34-data/README.md`: update the file map.

Acceptance: one spec row per group.
  Check: `awk -F'\t' 'NR>1 && $21!="" && $23!="automotive" && $23!="current-sense" {print $2"|"$18"|"$21}' planning/plan-34-data/candidates.tsv | sort -u | wc -l` equals `awk 'END{print NR-1}' planning/plan-34-data/specs.tsv` (est. 1 min; `$21` is the `pdf` column Phase 3 fills, `$23` the reason).
Commit: —

## Validation Plan

- Tests: none (data only). The scripts check themselves (`--report`,
  `--verify`).
- Coverage check: Phase 4's equality shows every eligible row with a PDF
  has a spec row.
- Runtime proof: not applicable; no part is loaded.
- Doc sync: none in A (`skipped-parts.md` and `obsolete-not-included.md`
  are written by B, once the verdicts exist).
- Final gate (once, after Phase 4): `cmake --build /tmp/logic-p34 -j8 &&
  ctest --test-dir /tmp/logic-p34` → `100% tests passed` (est. 1 min; the
  data-only letter changed nothing the tests read).

## Decisions

- **Makers** — owner, 2026-11-11 ("use only the top 3 manufacturers"):
  Viking Tech, YAGEO, Vishay Dale, the top 3 by DigiKey maker id of in-stock
  chip + through-hole resistors (§ 2). Counting Vishay's divisions as one
  company ranks the same three makers (Vishay 32122); only the Vishay Dale
  division is taken, as plan-29 took one Vishay division.
- **Sub-types** — owner, 2026-11-11: chip (52), through-hole (53),
  networks / arrays (50). Chassis-mount (54) and specialized (55) are out.
- **Population** — owner, 2026-11-11: ±1% and ±5% only, "about 2k total",
  "only pick resistors that are large supplies", "not more than two that
  are basically the same thing". Settled as § 3 rules 3–4.
- **Extras** — owner, 2026-11-11: 0 Ω jumpers in (the `< 1 kΩ` group);
  military parts in, flagged `military`; automotive (AEC-Q200) parts out
  (not chosen); current-sense / metal-element shunts out (not chosen).
- **Simulation** — owner, 2026-11-11: weak two-way link (plan-34-E).
- **Generic resistor** — owner, 2026-11-11: `GENERIC-RESISTOR`, Axial by
  default, 1 kΩ by default (plan-34-F, G).
- **Ohms groups** — owner, 2026-11-11 ("< 1k, 1k-5k, …"); the author
  carried the 1–5 pattern on: `< 1 kΩ`, `1 kΩ – 5 kΩ`, `5 kΩ – 10 kΩ`,
  `10 kΩ – 50 kΩ`, `50 kΩ – 100 kΩ`, `100 kΩ – 500 kΩ`, `500 kΩ – 1 MΩ`,
  `≥ 1 MΩ`, each lower bound in its group (1 kΩ is in `1 kΩ – 5 kΩ`,
  4.99 kΩ too, 5.1 kΩ in `5 kΩ – 10 kΩ`).
- **Inclusion rule** — owner, 2026-11-11: in stock, in the datasheet's
  ordering table, Active or Last Time Buy (§ 3).
- **Cap handling** — owner, 2026-11-11 ("if a listing hits DigiKey's
  300-result cap, … make phase A re-check stock part by part"): § 5 rule 2.
- **Quotas chip 1500 / through-hole 400 / arrays 100** — author: "about 2k"
  split by what a breadboard-and-board user places (chips dominate stock
  by 5× over through-hole, § 2), keeping through-hole parts in the set
  instead of letting stock alone drop them (the through-hole floor, 8982,
  is far below the chip floor, 72373). Rejected: one global top-2000 by
  stock (it takes no through-hole part).
- **The second slot prefers another maker** — author: a second source for
  the same resistor is the useful duplicate; without it the set was 94%
  YAGEO (planning run before the change: YAGEO 1813 of 1964).
- **Stock-sorted listing instead of a full census** — author: 85807 parts
  at plan-29's 0.08 calls per part is 6865 calls, 7 quota days; the top 300 per
  piece, sorted by stock, costs 241 calls and is exact for the pick (§ 5).
- **`top.js` beside `digikey.js`** — author: `digikey.js` sorts by part
  number and has no parameter filter; adding both to it would change a
  shared tool for one census. Rejected: extending `digikey.js`.
- **Maker-site datasheets only, with the sibling check** — author:
  `docs/parts-format.md` § Datasheets. The yageo.com → yageogroup.com
  rewrite (§ 4) is the same maker's site under its group domain.
- **Shared quota** — author: the day's quota went from 999 to 0 during
  planning (2026-11-11: this census's 430 listing calls, its probes, and
  the capacitor session's census, plan-33, together). Phase 1 starts only on a day with ≥ 600
  left; Phase 2 may span days.
- **Plan number** — author: `plan-34` was free on 2026-11-11 (`plan-33` was
  being written in the working tree by another session and landed at
  00e79a9 before this commit; `git log --all --oneline | grep -ciE 'plan[- ]34'`
  → 0; re-checked before writing and before committing). 00e79a9 changed
  only `planning/`, so the baselines measured at 8b1770b stand. The planning census first wrote into `/tmp/p33/raw`,
  the capacitor session's folder; its files were moved to `/tmp/p34/raw`.

## Corrections

(none yet)

## Summary

A's risk is the quota, not design: one day to finish the re-listing of
five cut pieces and up to two for the part-by-part check of 1778 picked
parts (planning count), on a quota other sessions share. The pick itself is
deterministic (`select.py`), and the cap is handled by a check that proves
completeness (§ 5) instead of a full listing. Left untouched: every part,
footprint, backend and app file.
