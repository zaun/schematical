# plan-45-A: Terminal blocks and JST headers — stock re-check, datasheet downloads

Last updated: 2026-11-16
Overall Effort: huge (> 3d) — plan-45 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d; plus 145 DigiKey calls)
Depends on: nothing (§ Prerequisites)

plan-45 adds **terminal blocks** and **JST board headers** to the parts library
and the app. Terminal blocks are the PCB-mount (wire-to-board) kinds with a
screw, a spring-cage or a push-in clamp, and the board-side header of a
plug-in terminal block, at the pitches 3.50, 3.81, 5.00, 5.08, 7.50 and 7.62
mm and 2 to 12 positions in one level, from the top three makers (§ 2):
**Phoenix Contact**, **Weidmüller** and **Würth Elektronik**. JST board
headers are the XH (2.50 mm), PH (2.00 mm), EH, ZH, SH/SR, GH and VH series
(through hole and surface mount, top and side entry), 2 to 12 positions, from
**JST** (the other makers of XH/PH-style headers have no datasheet PDF, § 2).
Plus 88 generic parts (`GENERIC-TERMINAL-<pitch>-<NN>`, `GENERIC-JST-XH-<NN>`,
`GENERIC-JST-PH-<NN>`). They sit in the parts drawer under **Connector →
Terminal block → <pitch> mm** and **Connector → JST → <series>**, in the
library's existing `connector` kind (plan-45-F adds three device keys), as
passive terminals like the USB connectors; the sidebar shows pitch, clamp,
rating and positions; the Board Preview window draws a terminal block and a
JST header.

The single behavioral outcome of plan-45: loading the part files gives the
baseline P0 parts (§ Prerequisites) plus the 88 generics plus one `connector`
part per `add` row of `planning/plan-45-data/candidates.tsv` (521 picked at
planning time: Phoenix Contact 75, Weidmüller 155, Würth Elektronik 99, JST
192); the app lists them under the drawer paths above; a placed
`GENERIC-TERMINAL-5.08-03` has three pads at 5.08 mm; two of its pins on one
net join nothing in play mode; `ctest` passes T0 programs (the extended
`connector_test` and `library_check`).

| Letter | Content | Effort |
|---|---|---|
| **A** | stock re-check of the 521 candidates, datasheet downloads and placement, text scan, `specs.tsv` skeleton | large |
| B | ordering-table check, datasheet status, facts and pinouts, verdicts, the final pick, skipped parts, obsolete PDFs, generic representatives, the final `footprints-needed.txt` | large |
| C | footprints: fixed (one-piece) terminal blocks, and the generic terminal footprints | large |
| D | footprints: board-side headers of plug-in terminal blocks | large |
| E | footprints: JST headers, and the generic JST footprints | large |
| F | backend: `terminal`, `jst`, `pitch` on the `connector` kind, tests, docs | medium |
| G | parts: the generators, the 88 generics, every `add` row, datasheets, goldens | large |
| H | app: drawer, sidebar facts, `kindName`, Board Preview looks, the Part Editor's connector fields, the harness | large |
| I | landing tier | small |

Outcome of A: `planning/plan-45-data/stock-check.tsv` re-checks every
candidate row in stock, Active or Last Time Buy (no listing piece hit
DigiKey's 300-result cap, so the re-check is one fresh listing, § 5);
`datasheets/connector-terminal/<maker>/` and `datasheets/connector-jst/` hold
every maker PDF a candidate row cites, with a `datasheets.tsv` in each;
`candidates.tsv` has `pdf`, `hit`, `dead` and `stock_checked` filled;
`specs.tsv` has one row per (maker, variant, PDF), its scheme and pinout
columns empty (B fills them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets,
  § Footprints, § `connector`, § `header`); `docs/backend.md`.
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-45-data/` — `README.md`, `census/count.js`, `sample.js`,
  `list.js`, `list.sh`, `common.py`, `select.py`, `urls.py`, `urlcheck.py`,
  `footprint-lines.py`; `raw/*.jsonl`, `raw/*.err` (the planning listing),
  `urls-needed.tsv`, `url-check.tsv`, `candidates.tsv`, `footprints-needed.txt`
  (written at planning time, 2026-11-16).
- `planning/completed/plan-40-A-transistor-data.md` and
  `planning/plan-40-data/census/` (`stock-check.sh`, `fetch.js`, `textscan.py`,
  `place.py`, `specs-skeleton.py`: the scripts A adapts, `fetch.js` as `fetch.py`);
  `planning/completed/plan-44-A-crystal-data.md` (the same method, the same layout).
- `planning/plan-43-data/candidates.tsv` (family `connector`): seven of its
  22 pending parts are JST headers that plan-45 adds (§ 2).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p45 && cmake --build /tmp/logic-p45 -j8 && ctest --test-dir /tmp/logic-p45 -j8` → `100% tests passed` | MET (2026-11-24 at ed9b9765: `100% tests passed, 0 tests failed out of 57`; T0 = 57, `grep -c '^logic_test' backend/CMakeLists.txt` → 57; was 36 at 5bccd0d7) |
| Golden sizes recorded | `git show HEAD:backend/tests/golden/parts.txt \| wc -l`; `traces.txt`; `footprints.txt` | at planning (5bccd0d7): **P0 = 32897**, **R0 = 32897**, **F0 = 1646**; at 2026-11-24 (ed9b9765, after plans 42 to 44 and the pld kind landed): **P0 = 35921**, **R0 = 35921**, **F0 = 2293**; F, C and G re-measure them at their start and use their own |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p45-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-24 at ed9b9765: `** BUILD SUCCEEDED **`) |
| No terminal / JST parts yet | `grep -c '"terminal"\|"jst"' schematical/schematical/Parts/connector.json` → 0; `ls datasheets/connector-terminal datasheets/connector-jst` → no such directory | MET (2026-11-24 at ed9b9765: 0, no directories) |
| No other open plan adds them | `grep -i 'datasheets/connector-terminal\|datasheets/connector-jst\|terminal block' planning/plan-*.md \| grep -v '^planning/plan-45' \| grep -v 'grep -l' \| wc -l` → 0 (the `grep -l` filter drops plans 47-A and 48-A, whose own Prerequisites quote these words inside their grep commands; the plain `grep -l` count is 2 there, Corrections) | MET (2026-11-24: 0). plan-42 (regulators) and plan-44 (crystals) are open; the lines they and plan-45 touch (`PartFacts.swift` and `PartCatalog.swift` `switch` statements, `footprints.json`, `datasheets.json`, the goldens) are edited by hunk; each plan-45 step that adds a line another plan also adds greps first (F Phase 1, G Phase 1, H Phase 1) |
| DigiKey credentials and quota | `grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc` → 2; `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)" && node planning/plan-45-data/census/count.js 314 455 jst` prints `calls left today` ≥ 200 before Phase 2 (about 1000 a day, shared with other sessions; it resets at about 00:00 UTC) | MET (2026-11-24, **second key** per the owner's "using opt2": `grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)2=' ~/.zshrc` → 2; `count.js 314 455 jst` through Safari → `calls left today 583`. The first key is spent for the day (429, resets 2026-11-25 00:00 UTC). The census scripts use the second key unless `DK_OPT2=0` and go through `scripts/digikey.js`'s Safari fallback: `census/dk.js`, Corrections) |
| Weidmüller, JST and DigiKey's Phoenix copies answer (Würth throttles; § 4) | `(head -1 planning/plan-45-data/urls-needed.tsv; grep -v 'we-online.com' planning/plan-45-data/urls-needed.tsv \| tail -n +2) > /tmp/p45-urls.tsv && python3 -I planning/plan-45-data/census/urlcheck.py /tmp/p45-urls.tsv \| awk -F'\t' 'NR>1{n[$4]++} END{for(k in n) print k, n[k]}'` → `1 241` (2026-11-24: `0 75` and `1 166`, see below) | MET (2026-11-24, command corrected: Weidmüller, JST and the other maker hosts answer `1 166` (155 + 9 + 2); DigiKey's 75 Phoenix copies on `mm.digikey.com` now answer 403 `cf-mitigated: challenge` to scripts, and download through Safari: `scripts/safari-digikey-pdf.sh <first mm.digikey.com url> /tmp/p45-safari-test/one.pdf` → `%PDF-1.4`, `pdfinfo` 4 pages. Phase 3's `fetch.py` fetches them that way; Corrections) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-45-data/stock-check.tsv` has one row per candidate row (521
  at planning time, `awk -F'\t' '$22=="candidate"' planning/plan-45-data/candidates.tsv \| wc -l`
  → 521), each `found` 1 (in stock, Active or Last Time Buy); a row that
  fails is `out` (reason `stock check`) in `candidates.tsv`.
- `datasheets/connector-terminal/<maker>/` (`phoenix`, `weidmuller`, `wurth`)
  and `datasheets/connector-jst/` hold each maker PDF that at least one
  candidate row needs (340 distinct URLs at planning time); each folder has a
  `datasheets.tsv` mapping file → URL, bytes, SHA-256.
- `candidates.tsv` has `pdf` and `hit` filled for every candidate row;
  `specs.tsv` exists with one row per (maker, variant, PDF), its scheme and
  pinout columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter and no row is `add` yet: B's verdicts and
  the final pick.
- No datasheet hunting beyond § 4's rules; a row with no downloadable PDF
  stays `out`.
- No new listing for the pick: the planning listing is complete (§ 2); the
  re-check in Phase 2 only re-checks stock.

## 2. Current State

The library has the `connector` kind with 224 devices and 376 parts: 7 barrel
jacks and 217 USB connectors (§ Measured populations). It has no terminal
block and no JST header; the `header` kind's only devices are the two generic
pin headers (`GENERIC-HEADER-<R>X<NN>`, 2.54 mm). `footprints.json` has no
terminal-block or JST footprint (1646 ids; `PINHDR-` 160 are the only
header-style ones). `datasheets/` has `connector-barrel`, `connector-header`
and `connector-usb`, and `datasheets/reference-boards/connector/` (plan-43)
with four JST PDFs (`ePH.pdf`, `eSH.pdf`, `eSR_SZ.pdf`, `eGH.pdf`) that serve
seven JST parts plan-43 left out (`B4B-PH-K-S`, `BM03B-SRSS-TBT`,
`S2B-PH-SM4-TB`, `SM03B-SRSS-TB`, `SM06B-GHS-TB`, `SM10B-GHS-TB`,
`SM10B-SRSS-TB`: all seven are candidates of plan-45). `planning/todo.md:73`
asks for "screw terminals and JST-XH/PH connectors (check what the 222
existing connectors already cover first)": the connector kind covers barrel
and USB only (224 devices, not 222), so nothing of the request exists.

### Measured populations

All DigiKey numbers 2026-11-16, in stock, statuses Active and Last Time Buy
(`count.js` and `list.js` send `MinimumQuantityAvailable` 1 and statuses 0
and 4). Planning used 145 listing calls (list.sh) plus single counting calls (count.js, sample.js).

| What | Count | Command |
|---|---|---|
| Existing connector kind | **224** devices (Barrel 7, USB 217), **376** parts | `python3 -I -c "import json;d=json.load(open('schematical/schematical/Parts/connector.json'));print(len(d['devices']), sum(len(x['parts']) for x in d['devices']))"` |
| Existing pin-header kind | 2 devices, 80 parts (all `GENERIC-HEADER-…`) | same, `header.json` |
| In stock: Wire to Board (371) / Headers, Plugs and Sockets (370) / Barrier Blocks (368) / DIN Rail (369) | **23707 / 18147 / 2158 / 4870** | `node planning/plan-45-data/census/count.js <category>` → `all` |
| …in scope (pitch 3.50 / 3.81 / 5.00 / 5.08 / 7.50 / 7.62 mm, 2–12 positions per level; 370: the header Types only) | 371: **10019**, 370: **5721** | `node planning/plan-45-data/census/count.js 371 0 t`, `… 370 0 t` → `all` |
| …of those in one level | 371: 8454, 370: 5069 | same, `levels 1` lines |
| Makers ranked by single-level in-scope parts, 371 + 370 | Reliance North America **3759** (371 only); Phoenix Contact **1661** (772 + 889); Weidmüller **1425** (505 + 920); Würth Elektronik **1319** (780 + 539) | `node planning/plan-45-data/census/count.js <371\|370> <4915\|277\|281\|732> t` → `levels 1` |
| **Reliance North America is not a candidate maker** | its 50 most-stocked parts: 40 link to `mm.digikey.com` (a distributor copy), 10 to nothing; the maker's own catalog `003_PCB_BLOCK_2027.pdf` (linked from reliancenorthamerica.com/terminal-blocks, 153 pages) is images: `pdftotext -layout` gives 12910 bytes, all of it the index | `node planning/plan-45-data/census/sample.js 371 4915 50`; `curl -sL -o /tmp/rna.pdf http://testrna.info/2027_Catalogs/003_PCB_BLOCK_2027.pdf && pdfinfo /tmp/rna.pdf \| grep Pages && pdftotext -layout /tmp/rna.pdf - \| wc -c` → `153`, `12910` |
| The three makers | **Phoenix Contact, Weidmüller, Würth Elektronik** (the first three that have a maker-site datasheet URL) | the two rows above |
| JST headers in the seven series (cat 314) | **312** (PH 72, XH 66, ZH 53, SH/SR 37, VH 36, EH 28, GH 20) | `node planning/plan-45-data/census/count.js 314 455 jst` → `all`; `SERIES=40 node census/count.js 314 455` → `series` lines |
| Other makers of XH / PH-style headers | only **Pololu**: series `JST PH-Style` 10, `JST SH-Style` 12; no `JST XH-Style`; its datasheet link is an HTML page | `SERIES=5000 node planning/plan-45-data/census/count.js 314 \| grep -i 'jst'`; `SERIESF=139857 node planning/plan-45-data/census/count.js 314` → `mfr 2183 Pololu 10`; `curl -sIL https://www.pololu.com/product-info-merged/4362 \| grep -i content-type` → `text/html` |
| Planning listing (`list.sh`, 7 pieces) | **5295 parts** (JST 312, Phoenix 371 833 + 370 1098, Weidmüller 536 + 1101, Würth 802 + 613); **145 calls** | `python3 -I planning/plan-45-data/census/select.py planning/plan-45-data/raw /tmp/p45-c.tsv --report` → `5295 parts listed`; `grep -h 'list.js:' planning/plan-45-data/raw/*.err` |
| **Pieces cut at the 300-result cap** | **0**: `list.js` splits a piece above 300 by facet until every piece is at most 300 | `grep -h 'cut at' planning/plan-45-data/raw/*.err \| wc -l` → 0 |
| Rows with no `QuantityAvailable` in the listing | 107, all Weidmüller | `gzip -dc planning/plan-45-data/raw/37*-281-t.jsonl.gz \| grep -c -v '"stock"'` → 107 |
| Outcome of `select.py` | **521 picked**; out: below quota 2150, duplicate 1040, no datasheet 815, levels 562, no-stock 107, positions 53, mounting 38, termination 9 | `python3 -I planning/plan-45-data/census/select.py planning/plan-45-data/raw planning/plan-45-data/candidates.tsv --urls planning/plan-45-data/url-check.tsv --report` → the `out:` line |
| …picked per maker | Phoenix Contact 75 (11 variants, 10 series), Weidmüller 155 (12, 6), Würth Elektronik 99 (12, 12), JST 192 (19, 7) | same, the per-maker lines |
| …Phoenix Contact rows with no downloadable file | 735 of its in-scope rows (Phoenix answers 403 to every automated fetch; only the rows with DigiKey's per-item copy `mm.digikey.com/…/<item>.pdf` have a file) | `awk -F'\t' '$2=="Phoenix Contact" && $23=="no datasheet"' planning/plan-45-data/candidates.tsv \| wc -l` → 735 |
| …terminal blocks by class | pluggable header 201, spring-cage 72, screw 44, push-in 12 | `awk -F'\t' '$22=="candidate" && $3=="terminal"{print $6}' planning/plan-45-data/candidates.tsv \| sort \| uniq -c` |
| …mounting, orientation | through hole 440, surface mount 81; vertical 255, horizontal 222, angled 44 | `$10`, `$9` of the same |
| …positions | 2: 58, 3: 59, 4: 58, 5: 57, 6: 57, 7: 40, 8: 49, 9: 40, 10: 36, 11: 26, 12: 41 | `$8` of the same |
| …status | Active 521, Last Time Buy 0 | `$16` |
| **The download set** | **521 rows, 340 distinct URLs** (Weidmüller 155, Würth 99, Phoenix 75 — one per-item PDF per row — JST 11, one per series and revision) | `python3 -I planning/plan-45-data/census/select.py … --report` → `candidates: 521; distinct URLs 340`; `awk -F'\t' 'NR>1{n[$3]++} END{for(k in n) print k, n[k]}' planning/plan-45-data/urls-needed.tsv` |
| Footprints (variant × position count) of the candidates | **450**: C 124, D 142, E 184 (521 parts share them) | `awk -F'\t' '$22=="candidate"{print $21 "\|" $8}' planning/plan-45-data/candidates.tsv \| sort -u \| wc -l` → 450 |
| Footprint lines (letter, variant), candidates | **54**: C 17 (128 parts), D 18 (201), E 19 (192); 0 with an existing footprint of the same pitch | `awk -F'\t' '!/^#/{n[$1]++; p[$1]+=$9} END{for(k in n) print k, n[k], p[k]}' planning/plan-45-data/footprints-needed.txt`; `awk -F'\t' '!/^#/ && $10!=""' … \| wc -l` → 0 |

### Verified properties

- **The listing is complete, not capped.** `list.js` reads a piece of at
  most 300 parts in full and splits a bigger one by the facet (Pitch,
  Positions Per Level, Mounting Type, Termination Style, Type, Current) whose
  largest value is smallest; `raw/*.err` shows 0 `cut at` lines (above). Each
  piece's listed parts equal its facet count (`371-277-t.err`: `list.js: 833
  parts` = `count.js 371 277 t` → `all 833`); the distinct-MPN counts differ
  from the product counts by one for two pieces (536 against 537, 1101
  against 1102: a part listed under two packings).
- **Downloads.** `urlcheck.py` (4 at a time, first 5 bytes) → 340 URLs, all
  `%PDF-`: Weidmüller `https://datasheet.weidmueller.com/pdf/en/<item>/scope/2/`
  answers with `Mozilla/5.0`; Würth `https://www.we-online.com/catalog/datasheet/<item>.pdf`
  needs `Mozilla/5.0` (curl's own and Safari's agents get 403); JST
  `https://www.jst-mfg.com/product/pdf/eng/eXH.pdf` and
  `https://www.jst.com/wp-content/uploads/…` answer a Safari User-Agent only;
  Phoenix Contact's own `https://www.phoenixcontact.com/us/products/<item>/pdf`
  answers 403 to curl, Node and Python with any User-Agent (`curl -sL -o
  /dev/null -w '%{http_code}' https://www.phoenixcontact.com/us/products/1729128/pdf`
  → 403), while DigiKey's copy of the same per-item document
  (`https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/909/1935161.pdf`
  → 200, 84413 bytes, its first page names `https://www.phoenixcontact.com/us/products/1935161`)
  downloads. A Phoenix row therefore downloads DigiKey's copy and names the
  maker's per-item URL, as `docs/parts-format.md` § Datasheets allows for a
  maker that refuses fetches (Suntsu in plan-44).
- **Each PDF holds a land pattern.** Phoenix per-item PDF
  (`1935161.pdf`, 6 pages): `Hole diameter 1.3 mm` (line 51) and a `Drilling
  diagram` (line 264); Würth `691137710002.pdf` (5 pages): `Recommended Hole
  Pattern: [mm]` on page 1; Weidmüller `1825640000.pdf` (8 pages): `Solder
  eyelet hole diameter (D) 1.1 mm` and the `Drawings` pages 7–8 (raster
  dimensional drawings); JST `eXH.pdf` (9 pages): `PC board layout and
  Assembly layout` (line 49).
- **No name clashes.** No candidate part number is a part of the library:
  `python3 -I -c` over `backend/tests/golden/parts.txt` names and the 521 `mpn`
  values → 0 in common (521 distinct).

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` (in B) only when all of these hold; any other outcome is
`out` or `skip` with the reason in the `reason` column.

1. **In stock and Active or Last Time Buy** at DigiKey. `list.js` asks for
   statuses 0 and 4 with `MinimumQuantityAvailable` 1, so Obsolete,
   Discontinued, Not For New Designs and made-to-order parts are never
   listed. Phase 2 re-checks every candidate (§ 5).
2. **Scope** (`common.scope`): terminal blocks (DigiKey 371, and 370 with the
   Types Header, Header Blade Pins, Header Male Pins (shrouded or not), Header
   Female Sockets, Receptacle Male Pins) with pitch 3.50, 3.81, 5.00, 5.08,
   7.50 or 7.62 mm; one level; 2 to 12 positions; mounting through hole
   (including kinked pins) or surface mount; a wire clamp that is a screw, a
   spring cage or a push-in (`wire_class`), or a pluggable header. Out:
   `levels`, `pitch`, `positions`, `mounting` (press-fit, SKEDD, panel, DIN
   rail, free hanging), `termination` (IDC, quick connect). JST: the seven
   series, 2 to 12 positions. Out: > 12 positions (53 rows). Barrier blocks
   (368), DIN-rail blocks (369), the plug halves of plug-in blocks, housings,
   crimp contacts, cable assemblies and AEC-Q200 or over-30 A grades are not
   listed.
3. **At most one of the same thing** (`common.dedupe`): the key is (maker,
   series, pitch, positions, orientation, mounting, class, current, voltage,
   type, shrouding); a key keeps its most-stocked part (the others differ in
   colour or packing), the rest → `duplicate`.
4. **A maker datasheet** (`select.py --urls`): `resolve_urls` gives a URL and
   a download source (§ 4) and `url-check.tsv` says it answers a PDF; else
   `no datasheet`.
5. **Variants** (`common.variant`: maker, series, pitch, orientation,
   mounting, clamp class, and for a pluggable header its Type and shrouding):
   a variant is one footprint family and has one drawing per position count.
   Terminal blocks: per (maker, pitch) the fixed-block variant and the
   pluggable-header variant with the most summed stock among those with at
   least 4 in-scope rows are picked; all their in-scope rows are
   `candidate`. JST: every variant with at least 2 in-scope rows. The rest are
   `out`, reason `below quota`.
6. **A datasheet that prints the part** (Phase 3 and B): the number is in the
   PDF (§ 4); else `skip`.
7. **In the datasheet's ordering table** (B): the number is printed in a
   Phoenix, Weidmüller or Würth per-item PDF (the item is the part), or a
   JST ordering-table pattern matches it; else `skip`.
8. **Not obsolete, discontinued, NRND or end of life by the datasheet** (B
   reads every `dead` = 1 row); a PDF whose every row is out for datasheet
   status is deleted and listed in `obsolete-not-included.md`.

## 4. Datasheet placement

`census/place.py` (plan-40's, adapted) copies each needed PDF from
`/tmp/p45/pdf/<md5 of the URL>.pdf` to `datasheets/connector-terminal/<maker-slug>/<name>.pdf`
(`phoenix`, `weidmuller`, `wurth`) or `datasheets/connector-jst/<name>.pdf`;
`<name>` is the part number for the per-item makers (the Phoenix item, the
Weidmüller item, the Würth item) and the URL path's last segment for JST
(query string dropped, `.pdf` appended when missing, characters outside
`A-Za-z0-9._-` replaced by `-`); a clash gets `-<first 6 hex of the md5>`. It
writes `datasheets.tsv` in each folder: path, URL, bytes, SHA-256, source
(`maker` or `digikey-copy`). Files with the same SHA-256 under two URLs share
one file. A PDF no candidate row uses is not copied.

The URL and the source of a row are `url` and `src` of `candidates.tsv`
(`common.resolve_urls`):

1. Phoenix Contact: `src` is the row's DigiKey link when it is
   `mm.digikey.com/…/<item>.pdf` (the per-item copy); `url` is
   `https://www.phoenixcontact.com/us/products/<item>/pdf`. A row whose
   DigiKey link is anything else has none: `out`, `no datasheet`.
2. Weidmüller: the row's link on `datasheet.weidmueller.com`, `url` = `src`.
3. Würth Elektronik: the row's link on `www.we-online.com`, `url` = `src`.
4. JST: the row's link on `www.jst-mfg.com` or `www.jst.com`, `url` = `src`.

Downloader rules (`census/fetch.py`, plan-40's `fetch.js` ported to Python's
`urllib`, because `www.jst.com` answers 403 to Node's `fetch` and to curl with
either User-Agent and 200 to `urllib` with the Safari one (2026-11-16 probes);
a `%PDF-` check and `pdfinfo` on the body): the `User-Agent` is `Mozilla/5.0`
for every host except `jst-mfg.com` and `jst.com` (the Safari string of
`urlcheck.py`). Hosts are fetched in parallel with each other but one file at
a time within a host: `datasheet.weidmueller.com`, `mm.digikey.com`,
`jst-mfg.com` and `jst.com` 1 s apart; `www.we-online.com` 5 s apart, because
it throttles: after `urlcheck.py` made its 99 Würth requests (4 at a time),
the next request for the same URLs got 403 (2026-11-16: three tries 10 s
apart, and again minutes later, all 403). A 403 from we-online.com is waited
out (120 s) and retried, up to 5 tries; any other host gets one retry. The four JST PDFs that
`datasheets/reference-boards/connector/jst-sales-america-inc/` already holds
(`ePH.pdf`, `eSH.pdf`, `eSR_SZ.pdf`, `eGH.pdf`) are downloaded again like the
others; those copies stay where plan-43 put them. A failed download makes the
rows of that URL `out` (`no datasheet`), recorded in § Corrections.

## 5. The stock re-check (no piece hit the cap)

The rule "a listing that hits the 300-result cap is re-checked part by part"
does not apply: no piece was cut (§ 2). The listing is still a snapshot from
2026-11-16, so Phase 2 re-lists the same seven pieces (145 calls, cheaper
than 521 single lookups) into `/tmp/p45-raw2` and `census/recheck.py` writes
`stock-check.tsv` (`mpn`, `found` 0/1, `stock`, `status`, `date`): `found` 1
when the part number is in the new listing with stock above 0 and status
Active or Last Time Buy. `select.py --stock stock-check.tsv` makes a `found`
0 row `out` (`stock check`) and sets `stock_checked`. A variant that loses
rows keeps the rest (no refill: each row is its own part).

## 6. Scripts

- Written at planning time (`README.md`): `census/count.js`, `sample.js`,
  `list.js`, `list.sh`, `common.py`, `select.py`, `urls.py`, `urlcheck.py`,
  `footprint-lines.py`.
- Written in this letter, adapted from `planning/plan-40-data/census/`:
  `recheck.py` (§ 5), `fetch.py`, `place.py` (§ 4; `--verify`, and for B
  `--prune`), `textscan.py` (the part number and the words `obsolete`,
  `discontinued`, `not recommended for new design`, `end of life`, `EOL`,
  `last time buy` per PDF), `specs-skeleton.py` (one row per (maker, variant,
  PDF)).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Restore the listing

- [x] Run `python3 -I planning/plan-45-data/census/select.py planning/plan-45-data/raw /tmp/p45-cand.tsv --urls planning/plan-45-data/url-check.tsv --report`:
      `5295 parts listed`, `candidates: 521; distinct URLs 340; variants 54`;
      `cmp` with `candidates.tsv` → `reproducible`.
      Output 2026-11-24: `5295 parts listed`, `candidates: 521; distinct URLs 340; variants 54`, `cmp` → `reproducible`.

Acceptance: the planning pick reproduces.
  Check: `python3 -I planning/plan-45-data/census/select.py planning/plan-45-data/raw /tmp/p45-cand.tsv --urls planning/plan-45-data/url-check.tsv && cmp /tmp/p45-cand.tsv planning/plan-45-data/candidates.tsv && echo reproducible` → `reproducible` (est. < 1 min).
Commit: ced62f5e (the gate and the call scripts: 248d40f4)

### Phase 2 — The stock re-check (145 DigiKey calls)

- [x] `census/recheck.py` (§ 5): `census/list.sh /tmp/p45-raw2` (145 calls),
      then `python3 -I census/recheck.py /tmp/p45-raw2 candidates.tsv > stock-check.tsv`.
      Output 2026-11-24 (second key; 7 pieces, 0 `cut at` lines, 5271 parts in the fresh listing; 148 calls: 145 + the 3 retries after the `371-732-t` failure, Corrections): `recheck.py: 513 found, 8 not found`.
- [x] `python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --stock stock-check.tsv --report`:
      every row `stock_checked` 1 or `out` (`stock check`); record the number
      out under § Corrections.
      Output: `out: … stock check 8`, `candidates: 513; distinct URLs 333; variants 54` (Phoenix 74, Weidmuller 153, Wurth 95, JST 191); 8 out, listed in Corrections.
- [x] Update `candidates.tsv`; `planning/plan-45-data/README.md` names
      `stock-check.tsv`. (`candidates.tsv` rewritten; README § Added by plan-45-A.)

Acceptance: every candidate row was seen in stock on the day of this phase.
  Check: `awk -F'\t' '$22=="candidate" && $27!=1' planning/plan-45-data/candidates.tsv \| wc -l` → 0 (est. 15 min: the 145-call listing; nothing smaller covers 521 rows).
  Result 2026-11-24: `0`; also no candidate that was not in the planning pick (`comm -13` of the two candidate lists → 0).
Commit: febbaa58

### Phase 3 — Download and place the datasheets

- [x] `census/fetch.py` and `census/place.py` (§ 4): download the distinct
      `src` of the candidate rows (340 at planning time) and place them;
      write the four `datasheets.tsv`.
      Output 2026-11-24: `fetch.py` 340 of 340 files (the 521-row set; Weidmüller 155, Würth 99, JST 11 on the first try, no 403 wait; Phoenix 75 through Safari, `safari-1` each); `place.py` on the 513 rows (333 URLs): `320 files placed for 333 URLs; 0 rows out (no datasheet)`: Phoenix 74 files / 74 `datasheets.tsv` rows, Weidmüller 153 / 153, Würth 84 / 95, JST 9 / 11 (same SHA-256 under two URLs shares a file).
- [x] `pdfinfo` every file: `Pages` ≥ 1; `place.py --verify` → every URL of the
      candidate rows has a file, every file a row.
      Output: `pdfinfo: 320 files, 0 bad`; `place.py --verify` → `ok: 320 files, 0 rows without a file, 0 unused, 0 changed`.
- [x] Fill `pdf` (the placed path) and `hit` (1 when `textscan.py` finds the
      part number in the PDF text; the JST series PDFs are checked in B) in
      `candidates.tsv`; fill `dead` (1 when `textscan.py` finds an
      obsolescence word on a page that names the part).
      Output: Phoenix 74 rows, 74 printed verbatim, 0 dead; Weidmüller 153, 153, 0; Würth 95, 82, 0 (13 not printed: B's ordering-table reading); JST 191, 180, 0 (11 not printed: series numbering tables, B). `awk … ($24=="" \|\| $25=="" \|\| $26=="")` over the candidates → 0.
- [x] `census/specs-skeleton.py` → `planning/plan-45-data/specs.tsv`.
      Output: `333 rows` (one per (maker, variant, PDF)), scheme / volts / amps / pinout / note empty.
- [x] Tests: none (data only).

Acceptance: every candidate row has a placed PDF and the verbatim scan.
  Check: `python3 -I planning/plan-45-data/census/place.py --verify` → `ok: <n> files, 0 rows without a file` (est. 30 min: 99 Würth files 5 s apart plus any 120 s waits dominate; the other hosts run beside them).
  Result 2026-11-24: `ok: 320 files, 0 rows without a file, 0 unused, 0 changed`.
Commit: this commit (hash in the next one)

## Validation Plan

- Tests: none run in this letter; its data is checked by the commands above.
- Doc sync: none (no part, footprint or doc changes).
- Final gate: none; the landing tier is I's.

## Decisions

- Author: **Reliance North America is left out of the ranking.** It has the
  most in-scope parts (3759) but publishes no datasheet on its own site that
  a part can be checked against: DigiKey links to its own copy, and the
  maker catalog is images. `docs/parts-format.md` § Datasheets wants the
  maker's site. The three makers are the first three of the ranking that
  have one. Reliance is recorded in `datasheets/skipped-parts.md` by B.
- Author: **The 12-position cap and the six pitches** (owner, 2026-11-16,
  scope answers: "limit terminal pitch to 3.5–7.62 mm" and "cap at 12
  positions per block"; the six named pitches are the author's reading of the
  list the owner saw). Pitches 4.00, 6.00, 6.35, 6.50 and 7.00 mm are out.
  The cap is applied to JST headers too (53 of 312 have more).
- Author: **One level only.** 2- to 4-level (stacked) blocks are out (562
  rows): their footprint has more rows and `pinouts` for them would double
  the work for little use.
- Owner (2026-11-16): fixed PCB blocks, board-side headers of pluggable
  blocks and spring-cage / push-in clamps are in; barrier, DIN-rail and
  panel-mount blocks are out; JST XH and PH are in with their SMD variants
  and the related families EH, ZH, SH, GH and VH; housings and crimp contacts
  are out; top 3 makers per group; generic parts are in; AEC-Q200 and
  high-current grades are not asked for, so they are out.
- Author: **XH/PH second and third makers.** The "top 3 per group" for
  XH/PH is JST plus the two makers with most XH/PH-style headers: Pololu is
  the only other one (its datasheet link is an HTML product page), so JST is
  the group's only maker with parts. Pololu's 22 headers go to
  `datasheets/skipped-parts.md` (B).
- Author: **Phoenix rows without DigiKey's per-item copy are out** (735
  rows): the maker blocks automated downloads, so a part could not be
  checked against its datasheet. Weidmüller's 57 and Würth's 22 rows with no
  maker link are out for the same reason.
- Author: **Variant choice is per (maker, pitch) with at least 4 rows**
  (`MIN_ROWS` 4, `K_FIXED` 1, `K_PLUGGABLE` 1 in `select.py`): the smallest
  set that still gives every maker a fixed-block and a pluggable variant at
  the pitches where it has a well-stocked one, and keeps the footprints to
  450 (variant × position count) drawn from 54 table rows. The first try (two
  fixed variants per pitch, no minimum) gave 604 candidates, 73 variants and
  537 footprints and was dropped for the footprint count.

## Corrections

- **2026-11-24, Prerequisites re-run (worktree `worktree-P-45` from ed9b9765).**
  - *Test and golden counts moved.* `ctest` → `100% tests passed, 0 tests failed out of 57` (T0 = 57, was 36); P0 = R0 = 35921, F0 = 2293 (were 32897 / 32897 / 1646: plans 42 to 44 and the pld kind landed since). Each letter re-measures at its start anyway.
  - *"No other open plan" command had two false hits.* `grep -l … \| grep -v plan-45 \| wc -l` gave 2: `plan-47-A` and `plan-48-A` only quote these words in their own Prerequisites `grep -l` commands. Command corrected (drops lines containing `grep -l`) → 0.
  - *Cloudflare answers scripts with a challenge since 2026-11-23* (AGENTS.md / `scripts/digikey.js`). `count.js`, `sample.js` and `list.js` called `fetch` directly, so they could not run. They now take their login and transport from the new `census/dk.js`, which wraps `scripts/digikey.js`'s `credentials`, `token` and `fetchRetry` (the pattern of `planning/plan-55-data/census/common.js`): Node first, Safari when challenged. Key: the **second** key (`DIGIKEY_CLIENT_ID2` / `_SECRET2`, its own quota) unless `DK_OPT2=0`, per the owner's "using opt2" (the first key's quota was already spent by another session: 429, resets 2026-11-25 00:00 UTC). `list.sh` exports both keys. Output and arguments of the three scripts are unchanged.
  - *DigiKey's copies of the Phoenix PDFs are behind the same challenge.* `urlcheck.py` on the 241 non-Würth URLs → `0 75` (all 75 `mm.digikey.com` files: HTTP 403, `cf-mitigated: challenge`, `server: cloudflare`) and `1 166`; on 2026-11-16 all 241 were `1`. Safari passes: `scripts/safari-digikey-pdf.sh` fetched `…/docus/20/5441278.pdf` → 97716 bytes, `%PDF-1.4`, 4 pages. Phase 3's `fetch.py` therefore fetches every `mm.digikey.com` URL through `safari-digikey-pdf.sh` (one at a time; it opens a tab in Safari's DigiKey window, so a DigiKey page must be open there) and every other host as § 4 says. `url-check.tsv` stays the planning snapshot (75 rows `ok`); a Phoenix file that does not download makes its row `out` (`no datasheet`) as § 4 says.
  - *Live JST count drifted.* `count.js 314 455 jst` → `all 318` (312 at planning): stock moves; Phase 2's fresh listing re-measures, and a part that appeared since is not added (the pick is the planning pick, § 5).

- **2026-11-24, Phase 2.**
  - *`select.py --stock` refilled the pick.* As written it applied the stock check before the duplicate and variant-quota choices, so the 8 lost rows changed their winners: a first run gave 517 candidates, with four rows the planning pick did not have (`S11B-XH-A-1` took `S11B-XH-A`'s duplicate slot, `3124680000` … `3124740000` became a Weidmüller variant) and a variant lost to the quota. § 5 says the opposite ("no refill: each row is its own part") and the Phase 2 check needs every candidate stock-checked. `select.py` now applies the stock check last, to the finished pick: a `candidate` that is `found` 0 becomes `out`, `stock check`; nothing else moves. Without `--stock` its output is unchanged (`cmp` → `reproducible` again after the edit).
  - *8 rows out for stock check* (`found` 0: not in the fresh listing with stock above 0 and status Active or Last Time Buy): JST `S11B-XH-A`; Phoenix Contact `5430904`; Weidmüller `2473010000`, `1124280000`; Würth `691412120005B`, `691412120006B`, `691412120008B`, `691412120009B`. All 54 variants keep at least one row (variants 54 before and after). Candidates 521 → **513** (Phoenix 74, Weidmüller 153, Würth 95, JST 191); distinct URLs 340 → **333**: the Phase 3 download set is 333, not 340.
  - *Calls.* The listing took 148 calls, not 145: `371-732-t` failed once (Safari request timeouts, then DigiKey HTTP 500) and `list.sh` re-ran it from its start. Second key 573 left after the first piece; 421 at the lowest point (shared with other sessions).
  - *Dates.* `recheck.py` stamps the UTC date (2026-11-24), the date the plans use.

- **2026-11-24, Phase 3.**
  - *Download set.* 333 URLs after the stock check (the downloads started before it ran, on the 521-row set: 340 files fetched, the Phoenix `5430904` copy of the one Phoenix row that went out is in `/tmp/p45/pdf` only, not placed). 333 URLs → 320 placed files: Würth 95 URLs → 84 files, JST 11 → 9, by identical SHA-256 (`place.py` shares one file).
  - *Phoenix copies through Safari.* Every `mm.digikey.com` file came down through `scripts/safari-digikey-pdf.sh` (first try each, about 20 s a file, 75 files in about 30 minutes, serial; the listing and the downloads cannot share Safari, so they ran one after the other). `fetch.py` decides by host, `--skip <host>` leaves one for a later run.
  - *`textscan.py` false positive.* The first run flagged 82 of 95 Würth rows `dead`: every Würth datasheet says "we also reserve the right to **discontinue** production and delivery of products". The word list used the bare `DISCONTINUE`; the plan's word is "discontinued". It is `DISCONTINUED|DISCONTINUATION` now (and `OBSOLETE`, `NOT RECOMMENDED FOR NEW`, `END OF LIFE`, `EOL`, `LAST TIME BUY`, `NRND`); rerun → `dead` 0 in all four makers. B still reads the PDFs of rows it keeps, the zero is a scan result, not a status.
  - *Column order of `specs.tsv`.* § Goal said "its scheme and pinout columns empty": the columns are `maker, variant, pdf, rows, example, scheme, volts, amps, pinout, note` (B's `note` is Phase 2's, `volts` / `amps` its Phase 3 facts); B may add columns.

## Summary

A is mechanical: a fresh listing to re-check stock, 340 downloads, a text
scan. The risks are the maker sites (Phoenix's 403 wall, Würth's and JST's
User-Agent rules) and the quota day. Nothing in `Parts/`, the backend or the
app is touched.
