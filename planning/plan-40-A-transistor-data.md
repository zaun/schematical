# plan-40-A: Transistors — census, part-by-part stock check, datasheet downloads

Last updated: 2026-11-14
Overall Effort: huge (> 3d) — plan-40 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d; plus two DigiKey quota days of waiting)
Depends on: nothing (§ Prerequisites)

plan-40 adds bipolar transistors and MOSFETs to the parts library and the
app, from the top three makers of each family at DigiKey: BJTs from onsemi
(with Fairchild), Nexperia and Diodes Incorporated, MOSFETs from onsemi (with
Fairchild), Infineon (with International Rectifier) and Vishay Siliconix.
Scope: NPN and PNP small-signal and power BJTs, Darlingtons, pre-biased
("digital") BJTs, N- and P-channel MOSFETs (logic-level and standard), the
dual and complementary array packages of both, and four generic parts
(`GENERIC-NPN`, `GENERIC-PNP`, `GENERIC-NMOS`, `GENERIC-PMOS`). They sit in
the parts drawer under **Discrete → Transistors → BJT / MOSFET → <type>**,
with **Discrete → Transistors → Generic** beside them. In play mode a
transistor is a digital switch: the base or gate level joins or opens the
collector–emitter or drain–source pins, so a low-side NPN driving an LED
lights it; the sidebar shows the part's ratings; a net above a rating is
refused by the voltage check (plan-40-F).

The single behavioral outcome of plan-40: loading the part files gives the
baseline 31608 parts (P0, § Prerequisites) plus the four generics plus one
`transistor` part per `add` row of `planning/plan-40-data/candidates.tsv`
(1287 at planning time: BJT 389, BJT array 53, pre-biased 120, pre-biased
array 42, MOSFET 601, MOSFET array 82); the app lists them under the drawer
leaves above; a placed `GENERIC-NPN` is `Q1` on `TO-92`; an NPN with a
pull-up and an LED lights the LED while its base is 1; `ctest` passes T0 + 1
programs (`transistor_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | re-listing of the capped pieces, the part-by-part stock check, datasheet downloads and placement, the verbatim scan, `specs.tsv` skeleton | large |
| B | ordering-code schemes and pinouts per datasheet, verdicts, the final pick, skipped parts, obsolete PDFs, `footprints-needed.txt` | large |
| C | footprints: through-hole packages (TO-92, TO-126, TO-220, TO-247, IPAK, …) | medium |
| D | footprints: small-signal surface-mount packages (SOT-23, SC-70, SOT-563, SOT-723, … and the chip-scale ones) | large |
| E | footprints: power and leadless surface-mount packages (DPAK, D2PAK, SOT-223, SO-8, PowerPAK, DFN, DirectFET, …) | large |
| F | backend: the `transistor` kind, gate-controlled channels, the rating test, `transistor_test`, docs | large |
| G | parts: the generator, the four generics, every `add` row, goldens | large |
| H | app: drawer order, the transistor symbols, facts, `kindName`, `Q<n>` references, the generics' footprint picker, BOM | large |
| I | landing tier | small |

Outcome of A: `/tmp/p40/raw` holds a listing whose every cut piece is either
complete for the pick (its 300th part has less stock than the lowest stock
the pick takes from that family and maker, § 5) or re-listed;
`planning/plan-40-data/stock-check.tsv` confirms each picked part that came
from a cut piece in stock part by part; `datasheets/transistor/<maker>/`
holds every maker-site PDF an eligible row cites, with
`datasheets/transistor/datasheets.tsv`; `specs.tsv` has one row per (maker,
series, datasheet, package), its scheme columns empty (B fills them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-40-data/` — `README.md`, `census/top.js`, `list.sh`,
  `recheck.sh`, `select.py`, `footprint-lines.py`; `raw/*.jsonl.gz` and
  `raw/*.err` (the planning listing), `candidates.tsv`,
  `footprints-needed.txt` (written at planning time, 2026-11-14).
- `planning/completed/plan-34-A-resistor-data.md` and
  `planning/plan-34-data/census/` (`stock-check.sh`, `fetch.py`,
  `textscan.py`, `sibling.py`, `place.py`, `specs-skeleton.py`: the scripts A
  adapts; the same "most-stocked per piece, re-check the capped pieces part
  by part" method).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p40 && cmake --build /tmp/logic-p40 -j8 && ctest --test-dir /tmp/logic-p40 -j8` → `100% tests passed` | MET (2026-11-14 19:45 UTC at 8913617d, in the P-40 worktree, build dir `/tmp/logic-p40w`: `100% tests passed, 0 tests failed out of 35`; T0 = 35) |
| Golden sizes recorded | `git show HEAD:backend/tests/golden/parts.txt \| wc -l`; `traces.txt`; `footprints.txt` | MET at the start: **P0 = 31608**, **R0 = 31608**, **F0 = 1497** (re-measured 2026-11-14 at 8913617d: 31608, 31608, 1497) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p40-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-14 at 8913617d: `** BUILD SUCCEEDED **`) |
| No transistor kind yet | `grep -c 'PART_TRANSISTOR' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/transistor.json` → no such file; `ls datasheets/transistor` → no such directory | MET (2026-11-14 at 8913617d: 0, no file, no directory) |
| No other open plan adds transistors or a part kind | `grep -l -i 'PART_TRANSISTOR\|Parts/transistor\.json\|datasheets/transistor' planning/plan-*.md \| grep -v plan-40 \| wc -l` → 0; `grep -lE 'PART_[A-Z]+\b\|footprints\.json\|ReferenceCategory\|PartKinds' planning/plan-*.md \| grep -v plan-40 \| wc -l` → 0 | MET (2026-11-14 at 8913617d: 0 and 0) |
| DigiKey credentials and quota | `DIGIKEY_CLIENT_ID`/`_SECRET` exported in `~/.zshrc` (`grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc` → 2); `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)" && node scripts/digikey.js --in-stock --pages 1 --limit 1 "2N3904"` prints `calls left today` ≥ 400 before Phase 1 and before Phase 2 (a day allows about 1000, shared with other sessions) | **NOT MET** (2026-11-14 19:49 UTC: credentials 2 of 2, but `1 API calls, 22 left today`; the quota resets about 00:00 UTC, so re-check after 2026-11-15 00:00 UTC) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- Every piece of the listing is complete for the pick: `python3 -I
  census/select.py /tmp/p40/raw /tmp/p40/cand.tsv --report` prints no
  `NEEDS DEEPER LISTING` line (§ 5).
- `planning/plan-40-data/stock-check.tsv` has one row per picked row from a
  cut piece (555 at planning time), each `found` with stock above 0 and
  status Active or Last Time Buy; a row that fails is `out` (reason
  `stock check`) and the pick is re-run until every picked row from a cut
  piece passes (§ 5).
- `datasheets/transistor/<maker>/` holds each maker-site PDF that at least
  one eligible row needs; `datasheets/transistor/datasheets.tsv` maps each
  file to its URL, size and SHA-256.
- `candidates.tsv` has the columns `pdf` and `hit` filled for every eligible
  row; `specs.tsv` exists with one row per (maker, series, PDF, DigiKey
  Package / Case) of the eligible rows, its scheme columns empty (B fills
  them).

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter and no row is `add` yet: the datasheet
  verdicts and the final pick are B's.
- No datasheet hunting beyond the DigiKey link, the onsemi URL rewrite of
  § 4 rule 3 and the same-maker sibling check.
- `scripts/digikey.js` is not changed: the census needs sorting by stock and
  facet splits, which it does not offer, so `census/top.js` calls the API
  itself (§ 6).

## 2. Current State

The library has no transistor kind and no transistor part. The app's
Settings already name a reference category `.transistor` with the prefix `Q`
(`Preferences/ReferencePrefixes.swift`, `grep -c 'transistor'
schematical/schematical/Preferences/ReferencePrefixes.swift` → 3); nothing
yet maps a part kind to it. `datasheets/` holds 6947 PDFs in 5.6 GB (`find
datasheets -name '*.pdf' -o -name '*.PDF' | wc -l` → 6947, `du -sh
datasheets` → 5.6G), one directory per family; `datasheets/skipped-parts.md`
and `datasheets/obsolete-not-included.md` take the skips.

### Measured populations

All DigiKey numbers 2026-11-14, in stock (`--in-stock`). Categories: 276
Single Bipolar Transistors, 277 Bipolar Transistor Arrays, 292 Single
Pre-Biased Bipolar Transistors, 293 Bipolar Transistor Arrays Pre-Biased, 278
Single FETs / MOSFETs, 289 FET / MOSFET Arrays. `dk.sh` is § 6's wrapper
around `node scripts/digikey.js`.

| What | Count | Command |
|---|---|---|
| In stock, all statuses, single BJT / single FET | 6707 / 21994 | `dk.sh --category 276 --in-stock --pages 1 --limit 1 --json` (and 278) → `ProductsCount` |
| In stock, Active + Last Time Buy, per category | 276: 5359; 277: 622; 292: 1677; 293: 723; 278: 18514; 289: 2329 | `dk.sh --category <c> --in-stock --status 0 --status 4 --pages 1 --limit 1 --json` (6 calls) → `ProductsCount` |
| Makers ranked by in-stock single BJTs (Active + LTB, category 276) | onsemi 1543 (+ Fairchild 182), Nexperia 569, Diodes Incorporated 522, Rohm 316, Renesas 214 | same calls, `Manufacturers` facet |
| Makers ranked by in-stock single FETs (Active + LTB, category 278) | onsemi 2233 (+ Fairchild 338), Infineon 2173 (+ International Rectifier 218, Infineon Americas 8), Vishay Siliconix 1952 (+ Siliconix 3), Rohm 898, STMicroelectronics 864, Diodes Incorporated 838 | same calls |
| …BJT family counting every BJT category (276 + 277 + 292 + 293) | onsemi 2103, Nexperia 1213, Rohm 901, Diodes Incorporated 790 | same calls, facets summed (Rohm's pre-biased parts lift it above Diodes; § Decisions: the ranking is by single parts) |
| Planning listing (`census/list.sh`, 30 pieces) | **6603 parts** listed, **251 calls** | `python3 -I census/select.py /tmp/p40/raw /tmp/p40/cand.tsv --report` → `6603 parts listed`; the `top.js:` lines of `raw/*.err` summed |
| Outcome of `select.py` | **1287 picked**; out: automotive 1946, not-a-transistor 579, duplicate 587, below quota 1966, chassis 158, no-stock 62, driver-ic 13, has-diode 5 | `select.py --report` → `reason:` |
| …picked per family | BJT 389, BJT array 53, pre-biased 120, pre-biased array 42, MOSFET 601, MOSFET array 82 | `awk -F'\t' '$24=="candidate"{c[$4]++} END{for(k in c) print k, c[k]}' planning/plan-40-data/candidates.tsv` |
| …picked per maker | onsemi 461, Infineon 227, Vishay Siliconix 227, Diodes Incorporated 224, Nexperia 148 | same, `$2` |
| …status | Active 1279, Last Time Buy 8 | same, `$7` |
| …logic-level MOSFETs (first gate drive of Rds(on) at or below 4.5 V) | 360 of 601 | `awk -F'\t' '$24=="candidate" && $4=="mosfet" && $19!="" && $19<=4.5' planning/plan-40-data/candidates.tsv \| wc -l` |
| **Pieces cut at the 300-result cap** | 10 of the listing's pieces (`cut piece` lines): onsemi 276 whole, NPN, PNP; Infineon 278 whole, N-channel; onsemi 278 whole, N-channel; Vishay Siliconix 278 whole, N-channel, P-channel | `select.py --report` → `10 pieces cut at 300` |
| …of them above the pick's floor (`NEEDS DEEPER LISTING`) | 3: onsemi 276 whole (1543 parts, 300th stock 39000, floor 11000), onsemi 278 whole (2233, 28556, 18647), Vishay Siliconix 278 whole (1952, 12224, 11990) | `select.py --report` → `NEEDS DEEPER LISTING` lines |
| Picked rows that came from a cut piece | **555** of 1287 | `select.py --report` → `candidates from a cut piece` |
| Cost of a deeper re-listing | 34 calls for 569 parts (Nexperia 276, split by Package / Case, same 565 unique part numbers as the base listing) | `node census/top.js 276 1727 /tmp/p40/x/test.jsonl 16` at planning → `top.js: 276/1727: 565 parts, 34 calls`; the three pieces hold 5728 parts (1543 + 2233 + 1952), so 5728 × 34 / 569 = 342 calls at that ratio (Phase 1 records the real number) |
| Datasheet hosts of the picked rows | www.onsemi.com 249, rocelec.widen.net (Rochester copies) 260, www.vishay.com 221 (3 are `doc?NNNNN` links), www.diodes.com 216, www.infineon.com 196, assets.nexperia.com 109, mm.digikey.com 22, none 14 | `awk -F'\t' '$24=="candidate" {print $23}' planning/plan-40-data/candidates.tsv \| sed 's/?.*//' \| awk -F/ '{print $3}' \| sort \| uniq -c` |
| Distinct datasheet URLs, picked rows (query strings kept) | 1189 | `awk -F'\t' '$24=="candidate" {print $23}' planning/plan-40-data/candidates.tsv \| sort -u \| wc -l` |
| Footprint lines (letter, maker, package), picked rows | 164: C 28, D 63, E 73; with no existing footprint of the same outline: C 10, D 29, E 56 | `grep -vc '^#' planning/plan-40-data/footprints-needed.txt` → 164; `awk -F'\t' '!/^#/{n[$1]++; if($10=="")g[$1]++} END{for(k in n) print k, n[k], g[k]+0}' planning/plan-40-data/footprints-needed.txt` |

### Verified properties

- **Sorting by stock works with the facet filters.** A KeywordSearch with
  `SortOptions {Field: "QuantityAvailable", SortOrder: "Descending"}`, a
  category, a maker, `MinimumQuantityAvailable` 1 and statuses 0 and 4
  returns products in falling stock (`raw/276-488.err`: onsemi 276 `300 of
  1543 (cut at 300: 39000)`, the first part `BC807-25LT1G` with 3933590). So
  the first 300 of a piece are its 300 most-stocked parts, and a cut piece is
  complete for the pick when its 300th part's stock is below the lowest stock
  the pick takes (§ 5).
- **The type facet does not cover every part.** The `Transistor Type` facet of
  category 276 sums to 4870 of 5359 parts (`scope276.json` at planning:
  NPN 2585, PNP 2004, NPN - Darlington 184, PNP - Darlington 79, 5 more,
  `-` 10), so `top.js` also lists each split piece's whole 300 most-stocked
  parts, and `select.py` counts the remainder (`cut_pieces`).
- **The empty parameter filter hides the facets.** A request that carries a
  `ParameterFilterRequest` with no filters returns no `ParametricFilters`, so
  `top.js` omits the field when it has none (`top.js:search`).
- **A piece of 301–600 parts can be listed whole.** Descending by stock for
  the first 300, ascending for the rest (`top.js` "both ends"): Nexperia 276
  (569 parts) listed 565 unique part numbers, the same as the Package / Case
  split above (the 4 missing are repeated part numbers: DigiKey lists one
  part once per packing, and `top.js` keeps one line per part number).
- **Downloads.** `curl -sL` gets a PDF from www.onsemi.com
  (`https://www.onsemi.com/pdf/datasheet/2n3904-d.pdf` → 200, `%PDF-`, 324812
  bytes; `ksa992-d.pdf` → 200, 279854 bytes) and www.diodes.com
  (`https://www.diodes.com/assets/Datasheets/MMBT3904.pdf` → 200, 349108
  bytes). `assets.nexperia.com` answers `curl` with HTTP 403 whatever the
  headers, and Node's `fetch` gets `PDTC115E_SERIES.pdf` (200,
  `application/pdf`, 186647 bytes): the downloader is a Node script
  (`census/fetch.js`). `www.infineon.com` answers a request with no user agent with 202 and an
  empty body and the same request with a Safari user agent with 200,
  `%PDF-` (`/assets/row/public/documents/24/49/infineon-irlml6244-datasheet-en.pdf`:
  202 with `curl`, 200 and 206758 bytes with `curl -A` and with Node's
  `fetch`; `/dgdl/irfhs8242pbf.pdf?fileId=5546d462533600a401535623903b1f5d`:
  202, then 200 and 242907 bytes): the downloader sends a browser user agent.
  rocelec.widen.net links answer an HTML page for part of the rows
  (`PHGLS19641-1.pdf?t.download=true` → `text/html`, 24513 bytes) and are
  distributor copies, never the maker's own site.
- **onsemi has a stable URL.** `https://www.onsemi.com/pdf/datasheet/<base
  part number lowercase>-d.pdf` answers a PDF for both a Rochester-linked
  Fairchild part (`KSA992FBTA` → `ksa992-d.pdf`) and a plain onsemi one.
- **Ordering tables are in the PDFs as patterns.** B writes one regex per
  (datasheet, series), as plan-34-B did.

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` (in B) only when all of these hold; any other outcome is
`out` or `skip` with the reason in the `reason` column.

1. **In stock** (quantity above zero) and **status Active or Last Time Buy**
   at DigiKey. `top.js` asks for statuses 0 and 4 with
   `MinimumQuantityAvailable` 1, so Obsolete, Discontinued at DigiKey, Not For
   New Designs and made-to-order parts are never listed; a row with no
   quantity figure is `out`, reason `no-stock`. A row from a cut piece is
   confirmed part by part (§ 5).
2. **Scope** (`select.py`): categories 276, 277, 292, 293, 278, 289 only; the
   five makers (maker ids in `README.md`); Grade or Qualification naming
   `Automotive` or `AEC-Q` → `out`, reason `automotive`; no transistor
   parameters → `not-a-transistor`; Mounting Type neither Surface Mount nor
   Through Hole → `chassis`; a type naming a built-in diode or Zener →
   `has-diode`; an array of more than 4 elements (the ULN2003 family) →
   `driver-ic`.
3. **At most one of the same thing** (`select.py`): the key is (family, base
   part number, Package / Case, Supplier Device Package); a key keeps its
   most-stocked part, the rest → `out`, reason `duplicate`. (Part numbers
   that differ by their packing suffix with the same base and package are one
   thing; B's ordering-table check lists the orderable names.)
4. **Each maker gets its share first** (`select.py`): per (family, maker) the
   family's quota divided by three (BJT 450, array 60, pre-biased 120,
   pre-biased array 40, MOSFET 600, MOSFET array 80), highest stock first; the
   rest → `out`, reason `below quota`. The owner's named parts (`2N3904`,
   `2N3906`, `BC547B`, `BC557B`, `2N7000`, `BS170`, `2N7002`) are picked
   whenever listed.
5. **A maker datasheet** (Phase 3; B's verdict): the row's `datasheet_url` is
   on the maker's own site (`onsemi.com`, `nexperia.com`, `diodes.com`,
   `infineon.com`, `vishay.com`) and downloads as a PDF; else for onsemi the
   rewritten URL of § 4 rule 3 does; else a downloaded same-maker PDF prints
   the part number verbatim (`sibling.py`), and that PDF is its datasheet;
   else `skip`, reason `no datasheet on the maker's site` with the link's
   host (Rochester copy `rocelec.widen.net`, DigiKey copy `mm.digikey.com`,
   none).
6. **In the datasheet's ordering table** (B): printed verbatim, or a full
   match of the (datasheet, series) ordering-code regex B writes in
   `specs.tsv`; else `skip`, reason `not in the datasheet's ordering table`.
7. **Not marked obsolete, discontinued, NRND or end of life by the
   datasheet** (B reads every `dead` = 1 row); a PDF whose every row is `out`
   for datasheet status is deleted and listed in `obsolete-not-included.md`.

Rules 5–7 are applied to every eligible row (reason `candidate`, `duplicate`
or `below quota`: 3840 at planning time, `awk -F'\t' '$24=="candidate" ||
$24=="duplicate" || $24=="below quota"' planning/plan-40-data/candidates.tsv
\| wc -l`), not only to the picked ones, so that B's final pick can fill a
skipped slot with the next part of the same (family, maker).

## 4. Datasheet placement

`census/place.py` (adapted from plan-34's in Phase 3) copies each needed PDF
from `/tmp/p40/pdf/<md5 of the URL>.pdf` to
`datasheets/transistor/<maker-slug>/<name>.pdf`, `<maker-slug>` `onsemi`,
`nexperia`, `diodes`, `infineon`, `vishay-siliconix`; `<name>` is the URL
path's last segment (query string dropped, `%20` → `-`, `.pdf` appended when
missing, characters outside `A-Za-z0-9._-` replaced by `-`; a Vishay
`doc?<number>` link takes `<number>.pdf`); a clash gets `-<first 6 hex of the
md5>`. It writes `datasheets/transistor/datasheets.tsv`: path, URL, bytes,
SHA-256. Files with the same SHA-256 under two URLs share one file. A PDF no
eligible row uses is not copied.

Downloader rules (`census/fetch.js`, Node `fetch`, 4 at a time, one retry,
a `%PDF-` check on the body):

1. A URL on a maker host is fetched as given.
2. A `doc?<number>` Vishay link is fetched as given.
3. A Rochester (`rocelec.widen.net`) or DigiKey (`mm.digikey.com`) link, and
   an empty link, of an onsemi row (maker onsemi, including the Fairchild
   ids) is fetched as `https://www.onsemi.com/pdf/datasheet/<base lowercase>-d.pdf`
   (`<base>` the row's `base` column); that URL is the one recorded. Other
   makers' Rochester and DigiKey links are not fetched: the row is `skip`
   (rule 5 of § 3).

## 5. The 300-result cap: re-list, then re-check part by part

DigiKey returns at most the first 300 results of a search. Because `top.js`
sorts by stock, a piece cut at 300 still holds its 300 most-stocked parts;
the parts it did not list all have stock at or below its 300th part's
(`cut at 300: <stock>` in the `.err` line). `select.py --report` compares that
stock with the lowest stock the pick takes from the same (family, maker): below
it → the piece is complete for the pick; at or above it → `NEEDS DEEPER
LISTING`.

1. **Re-list.** `recheck.sh` lists the three `NEEDS DEEPER LISTING` pieces
   again, split by the type facet and then Package / Case (`top.js … 74,16` or
   `606,16`), into `<category>-<maker id>-re.jsonl`; `select.py` reads a `-re`
   file before its base and does not count the base file's whole piece. Re-run
   `select.py --report` after each re-listing: the floor moves as rows are
   added, so a new `NEEDS DEEPER LISTING` piece gets its own `run` line in
   `recheck.sh`. A piece no facet splits below 300 with its 300th part at or
   above the floor is written in § Corrections with its count; the pick takes
   its listed parts (they are the most-stocked) and the unlisted ones cannot
   be picked.
2. **Re-check part by part** (the owner's rule for a capped listing). Every
   picked row whose piece was cut at 300 (`cut` = 1; 555 at planning time) is
   looked up by its own part number: `census/stock-check.sh` runs `dk.sh
   --manufacturer <maker id> --in-stock --status 0 --status 4 --limit 5 --json
   "<mpn>"` (one call; ids, so no lookup calls) and writes
   `planning/plan-40-data/stock-check.tsv` (`mpn`, `found` 0/1, `stock`,
   `status`, `date`); `found` = 1 when a product with exactly that
   `ManufacturerProductNumber` has `QuantityAvailable` above 0. The script
   skips part numbers already in the file, so a run cut by the quota goes on
   the next day. A row with `found` = 0 is `out`, reason `stock check`
   (`select.py --stock stock-check.tsv` excludes them, and sets
   `stock_checked`), and the pick is re-run; newly picked rows from cut pieces
   are checked the same way until none is unchecked.

## 6. Scripts

- `/tmp/p40/dk.sh` (not in the tree; one line per run): loads `DIGIKEY_*` from
  `~/.zshrc` with `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`,
  then `node /Users/justinzaun/Development/logic/scripts/digikey.js "$@"`.
- `census/top.js <category id> <maker id> <out.jsonl> [<split facet ids>]`,
  `census/list.sh <raw>`, `census/recheck.sh <raw>`, `census/select.py <raw>
  <out.tsv> [--stock <tsv>] [--report]`, `census/footprint-lines.py
  <candidates.tsv>`: written at planning time (`README.md`); Phase 2 adds
  `select.py`'s `--stock` option.
- Written in this letter: `census/stock-check.sh` (§ 5 rule 2),
  `census/fetch.js`, `census/textscan.py`, `census/sibling.py`,
  `census/place.py`, `census/specs-skeleton.py` (plan-34's, adapted: paths,
  maker hosts, the transistor columns, Node for the downloads).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Complete the listing (one quota day)

- [ ] Restore the planning listing as `README.md` says (`gunzip` every
      `raw/*.jsonl.gz` into `/tmp/p40/raw`, copy the `.err` files) and run
      `select.py … --report`: `6603 parts listed`, 3 `NEEDS DEEPER LISTING`
      lines, `candidates from a cut piece: 555 of 1287`.
- [ ] Run `census/recheck.sh /tmp/p40/raw` (the three deeper re-listings,
      342 calls at the planning-time ratio; a run cut by the quota goes on the next day), then
      `select.py /tmp/p40/raw planning/plan-40-data/candidates.tsv --report`
      until it prints no `NEEDS DEEPER LISTING` line; copy the new `-re`
      files into `planning/plan-40-data/raw/` (gzipped, with their `.err`).
- [ ] Record the real call count of the re-listings and any piece that no
      facet split below 300 in § Corrections.

Acceptance: the pick is complete for every piece.
  Check: `python3 -I planning/plan-40-data/census/select.py /tmp/p40/raw /tmp/p40/cand.tsv --report \| grep -c 'NEEDS DEEPER LISTING'` → 0 (est. 2 min for the check; the listing is the quota day).
Commit: —

### Phase 2 — Re-check the picked rows from cut pieces part by part (one quota day)

- [ ] `census/stock-check.sh` (§ 5 rule 2) and `select.py`'s `--stock
      <stock-check.tsv>` option (a `found` = 0 row → `out`, reason `stock
      check`; every checked row gets `stock_checked` = 1).
- [ ] Run it for every picked row with `cut` = 1 (555 at planning time); a
      run cut by the quota goes on the next day. Re-run the pick; check any
      newly picked row from a cut piece the same way until none is
      unchecked. Write `planning/plan-40-data/stock-check.tsv`.

Acceptance: every picked row from a cut piece is `found`.
  Check: `awk -F'\t' '$24=="candidate" && $25==1 && $26!=1' planning/plan-40-data/candidates.tsv \| wc -l` → 0 (est. < 1 min; the one-call-per-part run is the quota day).
Commit: —

### Phase 3 — Download, place and scan the datasheets

- [ ] `census/fetch.js`, `place.py`, `textscan.py`, `sibling.py` (§ 4, § 6):
      download every distinct URL the eligible rows cite (§ 4 rules 1–3) into
      `/tmp/p40/pdf`, check `%PDF-` and `pdfinfo` on each; failures are
      recorded in § Corrections with their host.
- [ ] Place the PDFs under `datasheets/transistor/<maker-slug>/` and write
      `datasheets/transistor/datasheets.tsv`.
- [ ] `candidates.tsv`: fill `pdf` (the placed path) for every eligible row
      whose URL, rewrite or sibling gave a PDF, and `hit` = 1 when its part
      number is printed verbatim in the PDF text (`pdftotext -layout`, token
      boundary search).
- [ ] `census/specs-skeleton.py`: `planning/plan-40-data/specs.tsv`, one row
      per (maker, series, PDF, DigiKey Package / Case) of the eligible rows,
      columns `maker`, `series`, `pdf`, `package`, `rows`, `example`,
      `scheme`, `label`, `pinout`, `elements` (the last four empty, B's).
- [ ] Check the total size: `du -sh datasheets/transistor` recorded here.

Acceptance: every eligible row either has a PDF or a reason it has none.
  Check: `awk -F'\t' '($24=="candidate"||$24=="duplicate"||$24=="below quota") && $27==""' planning/plan-40-data/candidates.tsv \| wc -l` → the number of rows B will `skip` for no maker datasheet (recorded here); `find datasheets/transistor -name '*.pdf' \| wc -l` → the distinct PDFs of `datasheets.tsv` (est. 40 min: about a thousand downloads, four at a time).
Commit: —

## Validation Plan

- Tests: none in this letter (data only). `select.py` is checked by its
  `--report` outputs above.
- Coverage check: the three Acceptance checks cover the listing, the stock
  check and the downloads.
- Runtime proof: n/a.
- Doc sync: none (`datasheets/skipped-parts.md` and `obsolete-not-included.md`
  are B's).
- Final gate: plan-40-I.

## Decisions

Owner decisions (asked once, 2026-11-14, before this plan was written):

- **Sub-types in scope: all of them** — NPN and PNP small-signal BJTs,
  N-channel MOSFETs including logic-level, P-channel MOSFETs, Darlington and
  pre-biased BJTs, power BJTs (the owner ticked every option).
- **Simulation: the switch model** — a placed transistor opens and closes its
  channel from the base or gate level in play mode (plan-40-F).
- **Makers: the top three per family** — BJT onsemi, Nexperia, Diodes
  Incorporated; MOSFET onsemi, Infineon, Vishay Siliconix, ranked by in-stock
  single parts (the counts of § 2).
- **Extras: all four** — generic parts, rating facts in the inspector, the
  voltage rating check, dual and complementary array packages.

Author decisions (no user-visible fork; recorded so nobody re-litigates):

- **The ranking is by single parts.** Counting pre-biased BJTs and arrays too
  puts Rohm (901) above Diodes (790); the owner chose the maker list above
  against the single-part counts, so Rohm stays out.
- **A maker is its DigiKey ids.** Fairchild Semiconductor is onsemi (the
  buyer, parts-format `manufacturer` rule: the part number stayed the same),
  International Rectifier and Infineon Americas are Infineon, Siliconix is
  Vishay Siliconix; NXP-branded BJTs (954, 568: 21 + 6 single parts) are not
  Nexperia's own listings and are left out.
- **Automotive parts are out** (reason `automotive`): they duplicate the
  commercial part and the parts format has no use for the flag here.
- **Equal share per maker, then by stock.** The quota of each family divides
  by three so every named maker appears in every family where it has parts.
- **The kind models transistors only.** A part with a built-in diode or Zener,
  and an array of more than four elements, is out.
- **Datasheets live in `datasheets/transistor/<maker>/`**, one tree for BJT
  and MOSFET (the pick of a PDF does not depend on the family).
- **Nexperia downloads use Node's `fetch`** (§ 2 Verified properties).

## Corrections

- 2026-11-14, Prerequisites, backend row: `/tmp/logic-p40` holds a CMake
  cache made from the main checkout (`CMake Error: The source
  ".../.claude/worktrees/P-40/backend/CMakeLists.txt" does not match the
  source ".../logic/backend/CMakeLists.txt" used to generate cache`), so a
  worktree run uses its own build dir, `/tmp/logic-p40w`.
- 2026-11-14 19:49 UTC, the gate stopped the plan: the DigiKey row is NOT MET
  (22 calls left, not 400). Every other row is MET. No phase was started.

## Summary

A is the plan's quota-bound letter: two days of DigiKey calls (the deeper
re-listing and the 555 part-by-part checks) and about a thousand downloads.
Its risk is wrong data feeding B (a stale stock figure, a PDF that is a
distributor copy): the stock check and the `%PDF-`/verbatim-name checks are
there to catch it. Nothing in the tree except `planning/plan-40-data/`,
`datasheets/transistor/` and the three files named above changes.
