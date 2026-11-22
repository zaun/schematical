# plan-54-A: Card slots — stock re-check, datasheet downloads, text scan, specs skeleton

Last updated: 2026-11-22
Overall Effort: huge (> 3d) — plan-54 is A → B → C → D → E → F → G → H → I → J → K → L → M
Effort: large (3h–1d; plus 162 DigiKey calls)
Depends on: nothing (§ Prerequisites)

plan-54 adds **card slots** to the parts library and the app, from the top three
makers of each family, **Active parts only**, and **no generic part**:

- **memory and SIM card sockets** (DigiKey "Memory Connectors – PC Card Sockets",
  category 414: SD, microSD, SIM, Micro / Nano SIM, smart-card sockets) from
  **Same Sky**, **Amphenol ICC** and **GCT**;
- **memory-module sockets** (DigiKey "Memory Connectors – Inline Module Sockets",
  413: DIMM, SO-DIMM, SIMM, MiniDIMM, SOCAMM) from **TE Connectivity**,
  **Amphenol ICC** and **Molex**;
- **card-edge sockets** (DigiKey "Card Edge Connectors – Edgeboard Connectors",
  303: PCI Express, M.2, MicroTCA and dual / single edge sockets) from **Samtec**,
  **TE Connectivity** and **Amphenol ICC**.

They sit in the parts drawer under **Connector → Card slot → …**, in the
library's existing `connector` kind (plan-54-I adds two device keys), as
passive terminals like the USB connectors; the sidebar shows card, pitch and
positions; the Board Preview window draws a card slot.

The single behavioral outcome of plan-54: loading the part files gives the
baseline P0 parts (§ Prerequisites) plus one `connector` part per `add` row of
`planning/plan-54-data/candidates.tsv` (1270 candidates at planning time:
memory / SIM sockets 134, module sockets 122, card-edge sockets 1014, before B
to D's skips); the app lists them under the drawer paths of J; a placed
part of each family (J and K record the most-stocked `add` row of each in
`examples.tsv`) has the pins and pads of its datasheet; two pins of one slot on one net join nothing in play mode; `ctest` passes
T0 programs (the extended `connector_test` and `library_check`).

| Letter | Content | Effort |
|---|---|---|
| **A** | stock re-check of the 1270 candidates, datasheet downloads and placement, text scan, `specs.tsv` skeleton | large |
| B | memory / SIM and module sockets: ordering tables, datasheet status, pin names, facts, verdicts, final pick | large |
| C | card-edge, Samtec and Amphenol ICC: the same | large |
| D | card-edge, TE Connectivity: the same | large |
| E | footprints: memory / SIM card sockets and module sockets | large |
| F | footprints: card-edge, Samtec | large |
| G | footprints: card-edge, TE Connectivity | large |
| H | footprints: card-edge, Amphenol ICC | large |
| I | backend: `card` and `cardPitch` on the `connector` kind, tests, docs | medium |
| J | parts: generator, memory / SIM and module sockets, datasheets, goldens | large |
| K | parts: card-edge sockets, datasheets, goldens | large |
| L | app: drawer, sidebar facts, `kindName`, Board Preview look, the Part Editor's connector fields, the harness | large |
| M | landing tier | small |

Outcome of A: `planning/plan-54-data/stock-check.tsv` re-checks every
candidate row in stock and Active (no listing piece hit DigiKey's 300-result
cap, so the re-check is one fresh listing, § 5);
`datasheets/connector-card-socket/<maker>/`, `datasheets/connector-memory-module/<maker>/`
and `datasheets/connector-card-edge/<maker>/` hold every maker PDF a candidate
row cites, with a `datasheets.tsv` in each; `candidates.tsv` has `pdf`, `hit`,
`dead` and `stock_checked` filled; `specs.tsv` has one row per (maker, series,
PDF), its scheme and pinout columns empty (B to D fill them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets,
  § Footprints, § `connector`); `docs/backend.md`.
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-54-data/` — `README.md`, `census/makers.js`, `stocked.js`,
  `list.js`, `list.sh`, `common.py`, `select.py`, `urls.py`, `urlcheck.py`,
  `report.py`; `raw/*.jsonl.gz`, `raw/*.err` (the planning listing),
  `urls-needed.tsv`, `url-check.tsv`, `candidates.tsv` (written at planning
  time, 2026-11-22).
- `planning/completed/plan-44-A-crystal-data.md` and `planning/plan-44-data/census/`
  (`stock-check.sh`, `fetch.js`, `place.py`, `textscan.py`, `sibling.py`,
  `specs-skeleton.py`: the scripts A adapts), `planning/plan-45-A-terminal-jst-data.md`
  (the same method for another connector family; a written plan, not a dependency).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p54 && cmake --build /tmp/logic-p54 -j8 && ctest --test-dir /tmp/logic-p54 -j8` → `100% tests passed` | MET (2026-11-22 at 7b449718: the plan-44-H landing run, `100% tests passed, 0 tests failed out of 41`; T0 = 41, `grep -c '^logic_test' backend/CMakeLists.txt` → 41) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` | at planning (7b449718): **P0 = 34375**, **R0 = 34375**, **F0 = 1895**; E, F, G, H, J and K re-measure them at their start and use their own |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p54-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-22: `** BUILD SUCCEEDED **` in plan-44-H's landing run on the tree HEAD 7b449718 holds) |
| No card-slot parts yet | `grep -c '"card"\|"cardPitch"' schematical/schematical/Parts/connector.json backend/src/models/connector.c` → `0` and `0`; `ls datasheets \| grep -c 'card\|memory-module'` → 0 | MET (2026-11-22 at 7b449718: 0, 0, 0) |
| No other open plan adds them | `grep -l -i 'datasheets/connector-card\|card slot\|card edge\|card-edge\|memory card' planning/plan-*.md \| grep -v plan-54 \| wc -l` → 0 | MET (2026-11-22: 0). plan-45 (terminal blocks and JST) and plan-53 are open and also edit the `connector` kind, `PartCatalog.swift` / `PartFacts.swift` `switch` statements, `footprints.json`, `datasheets.json` and the goldens; those lines are edited by hunk; each plan-54 step that adds a line another plan also adds greps first (I Phase 1, J Phase 1, L Phase 1) |
| DigiKey credentials and quota | `grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc` → 2; `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)" && node planning/plan-54-data/census/stocked.js 414:2223` prints `left` ≥ 220 before Phase 2 (about 1000 a day, shared with other sessions; it resets at about 00:00 UTC) | NOT MET (2026-11-22: credentials 2 of 2; `left 48` after planning's calls and other sessions'; the quota resets at about 00:00 UTC). Phases 1 and 3 need no quota |
| The download sources answer | `awk -F'\t' 'NR>1{n[$4]++} END{for(k in n) print k, n[k]}' planning/plan-54-data/url-check.tsv` → `1 506`, `0 11` (the 2026-11-22 result: Phase 3 fetches each source again) | MET (2026-11-22: `1 506`, `0 11`; the 11 are 6 te.com links answering 500, 3 molex.com links answering HTML, 1 cuidevices.com 404, 1 Amphenol S3 key answering 403) |

Everything below is written against the world where these hold; only the
quota row is open at planning time, and it gates Phase 2 alone.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-54-data/stock-check.tsv` has one row per candidate row
  (1270 at planning time, `awk -F'\t' '$20=="candidate"' planning/plan-54-data/candidates.tsv \| wc -l`
  → 1270), each `found` 1 (in stock, Active); a row that fails is `out`
  (reason `stock check`) in `candidates.tsv`.
- `datasheets/connector-card-socket/<maker>/` (`same-sky`, `amphenol`, `gct`),
  `datasheets/connector-memory-module/<maker>/` (`te`, `amphenol`, `molex`)
  and `datasheets/connector-card-edge/<maker>/` (`samtec`, `te`, `amphenol`)
  hold each maker PDF that at least one candidate row needs (498 distinct
  sources at planning time); each folder has a `datasheets.tsv` mapping file →
  URL, bytes, SHA-256, source.
- `candidates.tsv` has `pdf`, `hit`, `dead` and `stock_checked` filled for every
  candidate row; `specs.tsv` exists with one row per (maker, series, PDF), its
  scheme and pinout columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter and no row is `add` yet: the verdicts and
  the final pick are B's, C's and D's.
- No datasheet hunting beyond § 4's rules; a row with no downloadable PDF
  stays `out`.
- No new listing for the pick: the planning listing is complete (§ 2); the
  re-check in Phase 2 only re-checks stock.
- No generic part (owner, § Decisions).

## 2. Current State

The library has the `connector` kind with 224 devices and 376 parts: 7 barrel
jacks and 217 USB connectors (§ Measured populations). It has no card slot:
`footprints.json` has 1895 ids and none is a card socket, module socket or
card-edge footprint (the `PINHDR-` ones are pin headers). `datasheets/` has no
card folder. `planning/todo.md:66` asks "Add card edge sockets" (a line this
plan ticks, § Validation Plan).

### Measured populations

All DigiKey numbers 2026-11-22, in stock and status **Active** only
(`list.js` sends `MinimumQuantityAvailable` 1 and status 0; Last Time Buy is
not taken). Planning spent 164 listing calls (`list.sh`) plus single counting and
probing calls.

| What | Count | Command |
|---|---|---|
| Existing connector kind | **224** devices (Barrel 7, USB 217), **376** parts | `python3 -I -c "import json;d=json.load(open('schematical/schematical/Parts/connector.json'));print(len(d['devices']), sum(len(x['parts']) for x in d['devices']))"` |
| In stock, Active: PC Card Sockets (414) / Inline Module Sockets (413) / Edgeboard Connectors (303) | **397 / 180 / 1906** | `node planning/plan-54-data/census/makers.js 414 413 303` → the `== cat` lines |
| Makers of 414, by Active in-stock parts | Same Sky 76, ATTEND 41, 3M 32, Amphenol ICC (FCI) 32, GCT 31, Hirose 30, Molex 24, JAE 23, Amphenol ICC (Commercial) 19, TE AMP 18 | same, the `414` list |
| Makers of 413 | TE AMP 83, Amphenol ICC (FCI) 64, Molex 14, ATTEND 7, Walta 6, TE 4, JAE 2 | same |
| Makers of 303 | Samtec 465, TE AMP 363, Amphenol ICC (FCI) 327, EDAC 255, Sullins 156, Amphenol ICC (Commercial) 75, Molex 43, TE 42 | same |
| **A company's DigiKey labels count together** | Amphenol ICC = (FCI) + (Commercial Products); TE Connectivity = AMP Connectors + TE Connectivity | `census/common.py` `MAKER` |
| **ATTEND Technology has no DigiKey stock** | the 41 rows pass DigiKey's in-stock filter but state no `QuantityAvailable`; one product's variation says `QuantityAvailableforPackageType 0`, `ManufacturerPublicQuantity 0` | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` → `card   ATTEND Technology  with stock    0  without  41` |
| **3M is not a candidate maker** | 32 stocked parts, only 6 link to 3M's own site (`multimedia.3m.com`), the rest to DigiKey copies (24) or nothing (2) | same, `card   3M    6 of   32` |
| Rows whose link resolves to a maker URL, of the stocked rows | Same Sky 75 / 76, GCT 28 / 31, Amphenol ICC 35 / 50 (414); TE 76 / 85, Amphenol ICC 48 / 64, Molex 8 / 14 (413); Samtec 438 / 461, Amphenol ICC 327 / 401, TE 308 / 396 (303) | same, the `rows whose datasheet link resolves` block |
| The three makers | 414 **Same Sky, Amphenol ICC, GCT**; 413 **TE Connectivity, Amphenol ICC, Molex**; 303 **Samtec, TE Connectivity, Amphenol ICC** (the first three by stocked parts of which at least half link to the maker's own site; EDAC 255 is the fourth of 303) | the rows above |
| Planning listing (`list.sh`, 15 pieces) | **1651 parts** (1661 listed lines, 10 MPNs sit in two pieces) (414: Same Sky 76, Amphenol 32 + 19, GCT 31, 3M 32, ATTEND 41; 413: TE 83 + 4, Amphenol 64, Molex 14; 303: Samtec 461, TE 361 + 42, Amphenol 326 + 75); **164 calls** | `python3 -I planning/plan-54-data/census/report.py …` → `listed: 1651 parts`; `grep -h 'list.js:' planning/plan-54-data/raw/*.err \| awk '{s+=$6} END{print s}'` → 164 |
| **Pieces cut at the 300-result cap** | **0**: `list.js` splits a piece above 300 by the facet (any parametric one, then Series) whose largest value is smallest | `grep -h 'cut at' planning/plan-54-data/raw/*.err \| wc -l` → 0 |
| Listed rows against DigiKey's counts | 303-612 461 of 465, 303-17 361 of 363, 303-609 326 of 327: one MPN listed under two packings counts twice in DigiKey's number | `gzip -dc planning/plan-54-data/raw/303-612.jsonl.gz \| wc -l` → 461 |
| Outcome of `select.py` | **1270 candidates**; out: no datasheet 196, maker 73 (3M 32, ATTEND 41), no facts 59, duplicate 50, male 3 | `python3 -I planning/plan-54-data/census/select.py planning/plan-54-data/raw /tmp/p54-c.tsv --urls planning/plan-54-data/url-check.tsv --report` → the `out:` line |
| …candidates per family and maker | memory / SIM **134** (Same Sky 74, Amphenol ICC 32, GCT 28); module **122** (TE 75, Amphenol ICC 43, Molex 4); card-edge **1014** (Samtec 389, Amphenol ICC 325, TE 300) | `census/report.py` → the `card:`, `module:`, `edge:` lines |
| …variants (one footprint family) | **62** card, **21** module, **132** edge | same |
| …footprint keys (variant, positions) | **87** card, **30** module, **408** edge = **525**; 286 of them serve one part | same; `python3 -I -c` over `candidates.tsv` (parts per key: max 26) |
| …download sources | **498** (card 112, module 74, edge 312) | same, `download sources:` |
| …sources by host | te.com 291, s3.us-west-2.amazonaws.com (Amphenol's own `catsy.925` bucket) 73, cuidevices.com 55, suddendocs.samtec.com 44, gct.co 22, sameskydevices.com 20, molex.com 6, www-qa.te.com 4, literature.molex.com 1, www.gct.co 1 | `awk -F'\t' 'NR>1{n[$3]++} END{for(k in n) print n[k],k}' planning/plan-54-data/urls-needed.tsv` (517 rows, 509 distinct sources, 498 serve a candidate) |
| Positions | memory / SIM 6 to 23; module 72 to 694; card-edge 2 to 428; 5 module and 5 edge parts have more than 300 | `python3 -I -c` over `candidates.tsv`, `positions` |
| Mounting | memory / SIM: surface mount right angle 125, through hole right angle 5, mixed 4; module: surface mount 93, through hole 22, surface mount right angle 7; card-edge: surface mount 471, surface mount right angle 192, through hole 181, **board edge straddle mount 99**, through hole right angle 49, mixed 21, none 1 | `awk -F'\t' '$20=="candidate"{print $4 "\|" $11}' planning/plan-54-data/candidates.tsv \| sort \| uniq -c` |
| Status | Active 1270, Last Time Buy 0 | `awk -F'\t' '$20=="candidate"{print $7}' planning/plan-54-data/candidates.tsv \| sort \| uniq -c` |
| No name clashes | 0 candidate part numbers are library part names | `python3 -I -c` over `backend/tests/golden/parts.txt` names and the 1270 `mpn` values → 0 |

### Verified properties

- **The listing is complete, not capped.** `list.js` reads a piece of at most
  300 parts in full and splits a bigger one by facet; `raw/*.err` shows 0
  `cut at` lines (above). `303-612` (Samtec) is 40 pieces of at most 300 parts (`grep -h 'list.js:' planning/plan-54-data/raw/303-612.err`).
- **The maker sites answer a script only through some doors.** `curl` gets
  `HTTP 403` from `www.te.com` and `cdn.amphenol-cs.com` and nothing from
  `www.molex.com`; Python `urllib` with `User-Agent: Mozilla/5.0` gets a PDF
  from `www.te.com` (285 of 291 sources in `url-check.tsv`), and
  `cdn.amphenol-cs.com`, `cdn.amphenol-icc.com` and `www.amphenol-icc.com`
  answer 403 or 429 to every script. Amphenol ICC's own file store
  `https://s3.us-west-2.amazonaws.com/catsy.925/<basename>` (the same basename
  as the `cdn.amphenol-cs.com` URL) serves 72 of 73 sources as `%PDF-`
  (`url-check.tsv`). Samtec's DigiKey copies
  (`mm.digikey.com/…/<name>-mkt.pdf`) are Samtec marketing prints that Samtec
  serves itself at `https://suddendocs.samtec.com/prints/<name>-mkt.pdf`: 44 of
  44 such sources answer `%PDF-`, and for `pec-xx-xx-xx-s-xx-mkt.pdf` and
  `mec1-1xx-xx-xx-d-xx-xx-x-xx-mkt.pdf` the file is byte-identical to the
  DigiKey copy (`cmp`, 2026-11-22).
- **`datasheets.json`'s `url` must be the maker's own site and the same
  document as the local PDF** (`docs/parts-format.md` § Datasheets), so a row
  whose only link is a distributor copy (waldom.com, waldomemea.com,
  4donline.ihs.com, a DigiKey copy of a non-Samtec part) has no maker URL and
  is `out` (`no datasheet`: 196 rows); an Amphenol ICC row names the
  `cdn.amphenol-cs.com` URL DigiKey gives and downloads the `catsy.925` copy,
  as a Suntsu row names suntsu.com and was fetched from the Internet Archive
  (plan-44-F).
- **A part can be listed twice.** 50 MPNs are another MPN plus `-TR` (tape and
  reel; `common.py`/`select.py` rule `duplicate`).

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` (in B, C or D) only when all of these hold; any other
outcome is `out` or `skip` with the reason in the `reason` column.

1. **A listed maker.** The three makers of the row's family (§ 2); a company's
   DigiKey labels count together. Else `out`, reason `maker`.
2. **In stock and Active** at DigiKey. `list.js` asks for status 0 with
   `MinimumQuantityAvailable` 1, so Obsolete, Discontinued, Not For New
   Designs, Last Time Buy and made-to-order parts are never listed, and a row
   with no `QuantityAvailable` is `out` (`no stock`). Phase 2 re-checks every
   candidate (§ 5).
3. **Facts.** The row has a Number of Positions (else `no facts`: 59 rows, DigiKey
   gives them no parameters). Card-edge sockets are female: a row with Gender
   Male or a card type "Fits Female Edgecards" is `out` (`male`: 3 rows).
4. **One of the same part.** An MPN that is another listed MPN plus `-TR` is
   `out` (`duplicate`); there is no other de-duplication: every distinct
   stocked MPN of the three makers is a part (owner: everything the makers
   list, § Decisions).
5. **A maker datasheet** (`select.py --urls`): `common.src_for` gives a URL and
   a download source (§ 4) and `url-check.tsv` says the source answers a PDF;
   else `no datasheet`.
6. **A datasheet that prints the part** (Phase 3, B, C, D): the part number or
   its series' ordering-code scheme is in the PDF; else `skip`.
7. **In the datasheet's ordering table** (B, C, D): the scheme matches the part
   number; else `skip`.
8. **Active by the datasheet** (B, C, D read every `dead` = 1 row and every
   part whose PDF names a status): a part the datasheet marks obsolete,
   discontinued, not recommended for new designs, last time buy or end of life
   is `skip` (owner: only Active card slots, § Decisions); a PDF whose every
   row is out for datasheet status is deleted and listed in
   `obsolete-not-included.md`.
9. **A land pattern the footprint format can express** (B, C, D read the
   drawing; E to H confirm): the format has no pad on the bottom layer, no rotated
   slot and no keyhole shell slot (`docs/parts-format.md` § `connector`, the USB
   connectors' limit); a part whose drawing needs one is `skip`, reason `footprint
   not expressible: <feature>`. DigiKey's mounting type `Board Edge, Straddle
   Mount` (contacts on both faces of the board) is the expected case: 99
   candidate rows.

`variant` (the footprint family, `select.py` `variant()`): card-edge — maker,
series, mounting type, pitch, termination, rows; module — maker, series,
connector style, mounting, insertion angle; card socket — maker, series, card
type, mounting, insertion and removal method. A variant has one drawing per
position count (the footprint keys of § 2).

## 4. Datasheet placement

`census/place.py` (plan-44's, adapted) copies each needed PDF from
`/tmp/p54/pdf/<md5 of the source>.pdf` to
`datasheets/connector-card-socket/<maker-slug>/<name>.pdf`,
`datasheets/connector-memory-module/<maker-slug>/<name>.pdf` or
`datasheets/connector-card-edge/<maker-slug>/<name>.pdf` by the row's `family`
(`card`, `module`, `edge`); the maker slugs are `same-sky`, `amphenol`, `gct`,
`te`, `molex`, `samtec`. `<name>` is: for a te.com link the `DocNm` value of the
query (`2309407`, `108-60121-1`, `8-1773459-7_EXPRESS_MINI_CARD_QRG`) and, for
a `product-<number>.datasheet.pdf` link, the file's own last path segment; for
every other host the source's last path segment, query string dropped, `.pdf`
appended when missing, characters outside `A-Za-z0-9._-` replaced by `-`; a
clash gets `-<first 6 hex of the md5 of the source>`. It writes
`datasheets.tsv` in each folder: path, url (the maker URL of
`candidates.tsv`), bytes, SHA-256, source (the downloaded host). Files with the
same SHA-256 under two URLs share one file. A PDF no candidate row uses is not
copied.

The URL and the source of a row are `url` and `src` of `candidates.tsv`
(`common.src_for`):

1. Same Sky: the row's link on `www.cuidevices.com` or `www.sameskydevices.com`;
   `url` = `src`.
2. GCT: the row's link on `gct.co` or `www.gct.co`; `url` = `src`.
3. Samtec: the row's link on `suddendocs.samtec.com` (query string dropped); or,
   for a DigiKey copy `mm.digikey.com/…/<name>-mkt.pdf`,
   `https://suddendocs.samtec.com/prints/<name>-mkt.pdf`; `url` = `src`.
4. TE Connectivity: the row's link on `www.te.com` or `www-qa.te.com`; `url` =
   `src`.
5. Amphenol ICC: the row's link on `cdn.amphenol-cs.com`, `cdn.amphenol-icc.com`,
   `www.amphenol-cs.com` or `www.amphenol-icc.com`; `url` = that link, `src` =
   `https://s3.us-west-2.amazonaws.com/catsy.925/<basename>`.
6. Molex: the row's link on `www.molex.com` or `www.literature.molex.com`; `url`
   = `src`.
7. Any other row: none (`no datasheet`).

Downloader rules (`census/fetch.py`, plan-44's `fetch.js` ported to Python's
`urllib`, because `www.te.com` answers a PDF to `urllib` and 403 to Node's
`fetch` and curl; a `%PDF-` check and `pdfinfo` on the body): the
`User-Agent` is `Mozilla/5.0` for every host; one file at a time within a
host, hosts in parallel; each host 1 s apart; a 429 or 5xx is waited
out (60 s) and retried up to 5 tries, any other host gets one retry. A failed
download makes the rows of that source `out` (`no datasheet`), recorded in
§ Corrections.

## 5. The stock re-check (no piece hit the cap)

The rule "a listing that hits the 300-result cap is re-checked part by part"
does not apply: no piece was cut (§ 2). The listing is still a snapshot from
2026-11-22, so Phase 2 re-lists the thirteen pieces of the three makers per
family (`list.sh` with `PIECES` set to every piece except `414-19` and
`414-4827`: 162 calls, cheaper than 1270 single lookups) into `/tmp/p54-raw2`
and `census/recheck.py` writes `stock-check.tsv` (`mpn`, `found` 0/1, `stock`,
`status`, `date`): `found` 1 when the part number is in the new listing with
stock above 0 and status Active. `select.py --stock stock-check.tsv` makes a
`found` 0 row `out` (`stock check`) and sets `stock_checked`. A variant that
loses rows keeps the rest (no refill: each row is its own part).

## 6. Scripts

- Written at planning time (`README.md`): `census/makers.js`, `stocked.js`,
  `list.js`, `list.sh`, `common.py`, `select.py`, `urls.py`, `urlcheck.py`,
  `report.py`.
- Written in this letter, adapted from `planning/plan-44-data/census/`:
  `recheck.py` (§ 5), `fetch.py`, `place.py` (§ 4; `--verify`, and for B to D
  `--prune`), `textscan.py` (the part number and the words `obsolete`,
  `discontinued`, `not recommended for new design`, `end of life`, `EOL`,
  `last time buy` per PDF), `specs-skeleton.py` (one row per (maker, series,
  PDF)), and the `PIECES` variable of `list.sh`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Restore the listing and commit the planning data

- [ ] Run `python3 -I planning/plan-54-data/census/select.py planning/plan-54-data/raw /tmp/p54-cand.tsv --urls planning/plan-54-data/url-check.tsv --report`:
      `1651 parts listed`, `candidates: 1270; distinct URLs 498`; `cmp` with
      `candidates.tsv` → `reproducible`.
- [ ] `census/list.sh`: the `PIECES` environment variable replaces the default
      piece list when set (§ 5); `pieces=(…)` stays the default.

Acceptance: the planning pick reproduces.
  Check: `python3 -I planning/plan-54-data/census/select.py planning/plan-54-data/raw /tmp/p54-cand.tsv --urls planning/plan-54-data/url-check.tsv && cmp /tmp/p54-cand.tsv planning/plan-54-data/candidates.tsv && echo reproducible` → `reproducible` (est. < 1 min).
Commit: —

### Phase 2 — The stock re-check (one quota day)

- [ ] `census/recheck.py` (§ 5); run
      `PIECES="414-2223 414-609 414-664 414-2072 413-17 413-994 413-609 413-23 303-612 303-17 303-994 303-609 303-664" census/list.sh /tmp/p54-raw2`
      (162 calls; ≥ 220 left before the run, § Prerequisites), then
      `recheck.py /tmp/p54-raw2 planning/plan-54-data/candidates.tsv > planning/plan-54-data/stock-check.tsv`.
- [ ] `select.py … --stock planning/plan-54-data/stock-check.tsv --report` writes
      `candidates.tsv`; record the number of `found` = 0 rows in § Corrections.

Acceptance: every candidate row is in stock and Active.
  Check: `awk -F'\t' 'NR>1 && $2!=1' planning/plan-54-data/stock-check.tsv \| wc -l` → the `found` = 0 count recorded above, and `awk -F'\t' '$20=="candidate" && $25==""' planning/plan-54-data/candidates.tsv \| wc -l` → 0 (est. 8 min: 162 calls at about 23 a minute plus 1 min of scripts).
Commit: —

### Phase 3 — Download, place and scan the datasheets

- [ ] `census/fetch.py`, `place.py`, `textscan.py` (§ 4, § 6): download every
      distinct source the candidate rows cite (498 at planning) by § 4's rules
      into `/tmp/p54/pdf`; `%PDF-` and `pdfinfo` on each; failures recorded in
      § Corrections with their host.
- [ ] Place the PDFs under `datasheets/connector-card-socket/`,
      `datasheets/connector-memory-module/` and `datasheets/connector-card-edge/`
      and write each folder's `datasheets.tsv`; `place.py --verify` → `0
      missing, 0 unused`. The PDFs stay out of the commits until B, C and D
      prune them, so the history never carries PDFs no `add` row uses.
- [ ] `candidates.tsv`: `pdf` (the placed path) for every candidate row whose
      source gave a PDF, and `hit` = 1 when its part number is printed
      verbatim (`pdftotext -layout`, token-boundary search); `dead` = 1 when
      the text near the part number says obsolete, discontinued, NRND, "not
      recommended for new designs", last time buy or end of life (B, C and D
      read every such row).
- [ ] `census/specs-skeleton.py`: `planning/plan-54-data/specs.tsv`, one row per
      (maker, series, PDF) of the candidate rows (`specs rows` of `report.py`:
      card 119, module 75, edge 329 = 523 at planning), columns `family`,
      `maker`, `series`, `pdf`, `rows`, `example`, `scheme`, `label`,
      `pinout`, `positions`, `status` (B to D's).
- [ ] Record `du -sh datasheets/connector-card-socket datasheets/connector-memory-module datasheets/connector-card-edge`
      here, before the prune.

Acceptance: every candidate row either has a PDF or a reason it has none.
  Check: `awk -F'\t' '$20=="candidate" && $22==""' planning/plan-54-data/candidates.tsv \| wc -l` → the rows B to D will `skip` for no datasheet (recorded here); `find datasheets/connector-card-socket datasheets/connector-memory-module datasheets/connector-card-edge -iname '*.pdf' \| wc -l` = the distinct files the three `datasheets.tsv` list (est. 25 min: 498 downloads, about 4 files a second on the fast hosts and 1 a second at te.com).
Commit: —

## Validation Plan

- Tests: `place.py --verify` (Phase 3); `select.py` reproduces the pick
  (Phase 1). No backend or app code changes.
- Coverage check: not applicable (data only).
- Runtime proof: not applicable.
- Doc sync: none; `datasheets/connector-*/datasheets.tsv` are the record.
- Final gate: none; the landing tier is M's.

## Decisions

- Owner (2026-11-22): **card slots are all three DigiKey families** — memory /
  SIM card sockets, DIMM / SO-DIMM module sockets and card-edge sockets. It
  settles § 1.
- Owner (2026-11-22): **the top three makers per family**, not one set of three
  overall. It settles § 2's three makers of each category. The author reads
  "top" as most Active in-stock DigiKey parts, a company's DigiKey labels
  counted together, among the makers at least half of whose stocked rows link to
  the maker's own site (plan-45's precedent for Reliance North America); that
  drops ATTEND (no stock) and 3M (6 of 32 links) from the memory / SIM family
  and puts GCT in.
- Owner (2026-11-22): **everything the three makers list** in the card-edge
  family (1014 parts), not a subset by pitch or position count. It settles
  § 3 rule 4.
- Owner (2026-11-22): **Active only**. A part DigiKey lists as Last Time Buy, NRND
  or Obsolete is never a candidate (§ 3 rule 2), and a part its datasheet marks
  obsolete is `skip`, not added with an `obsolete` flag (§ 3 rule 8).
- Owner (2026-11-22): **no generic part** for any card slot. It settles that
  every part in the library comes from an `add` row and every test uses a real
  part.
- Owner (2026-11-22): **plan-54 is independent of plan-45**: it neither waits for
  nor assumes plan-45's terminal-block and JST keys; shared lines are edited by
  hunk (§ Prerequisites).
- Author: **Download copies are allowed only when the maker's own file store
  serves the same document.** Amphenol ICC rows download the `catsy.925` S3
  copy of the `cdn.amphenol-cs.com` file; Samtec rows download
  `suddendocs.samtec.com/prints/<name>-mkt.pdf`; no distributor copy (waldom,
  waldomemea, 4donline, DigiKey) is a source for a non-Samtec row. Rejected:
  taking every distributor copy (the `datasheets.json` rule needs the maker's
  own URL and a distributor file may be another revision).

## Corrections

## Summary

A is data work: it re-checks 1270 rows against DigiKey in one 162-call quota
day, downloads 498 PDFs through the doors the maker sites leave open (TE and
Samtec directly, Amphenol ICC through its own file store) and writes the
skeleton B, C and D read. The risk is volume and the sites that throttle
(`www.te.com` with 291 sources), not design; nothing in `Parts/`, the backend
or the app changes.
