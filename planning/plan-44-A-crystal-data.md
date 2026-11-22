# plan-44-A: Crystals and ceramic resonators — part-by-part stock check, datasheet downloads

Last updated: 2026-11-16
Overall Effort: huge (> 3d) — plan-44 is A → B → C → D → E → F → G → H
Effort: large (3h–1d; plus one DigiKey quota day)
Depends on: nothing (§ Prerequisites)

plan-44 adds **crystals** and **ceramic resonators** to the parts library and
the app, from the top three makers at DigiKey by in-stock count over the
Crystals (171) and Resonators (174) categories: **Abracon**, **ECS** and
**Suntsu** (§ 2). Scope: MHz crystals (fundamental mode, up to 100 MHz),
kHz tuning-fork crystals, and ceramic resonators with two pins and with three
pins (built-in load capacitors); two-pin and four-pad surface-mount packages
and through-hole packages (HC-49/U, HC-49/US, cylinder and radial); the
AEC-Q200 grade is in; plus four generic parts (`GENERIC-CRYSTAL-32768`,
`GENERIC-CRYSTAL-8M`, `GENERIC-CRYSTAL-16M`, `GENERIC-RESONATOR-8M`). They sit
in the parts drawer under **Crystal → kHz / MHz / Ceramic resonator →
<frequency band>**. In play mode a crystal or resonator is a clock source on
its own two pins: it drives a weak square wave at its frequency on one
terminal and the complement on the other, so a chip's clock input (an XTAL1
pin, a counter's CLK) wired to either terminal counts it (plan-44-E); the
sidebar shows frequency, load capacitance, tolerance, stability and ESR.

The single behavioral outcome of plan-44: loading the part files gives the
baseline P0 parts (§ Prerequisites) plus the four generics plus one `crystal`
part per `add` row of `planning/plan-44-data/candidates.tsv` (600 picked at
planning time: Abracon 200, ECS 200, Suntsu 200; kHz 120, MHz 414, ceramic
resonator 66); the app lists them under the drawer paths above; a placed
`GENERIC-CRYSTAL-8M` is `Y1` on an HC-49/US footprint; a 32.768 kHz
crystal wired to an inverter's input makes the inverter's output (and an LED
behind it) toggle in play mode; `ctest` passes T0 + 1 programs
(`crystal_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | part-by-part stock check of the download set, datasheet downloads and placement, the verbatim scan, `specs.tsv` skeleton | large |
| B | ordering-code schemes per datasheet, datasheet status, pinouts and facts, verdicts, the final pick, skipped parts, obsolete PDFs, the final `footprints-needed.txt` | large |
| C | footprints: surface-mount crystals (two-pad, four-pad, HC-49/US SMD, cylinder SMD) | large |
| D | footprints: through-hole crystals (HC-49/U, HC-49/US, cylinder, HC-46/X) and every ceramic resonator | medium |
| E | backend: the `crystal` kind, the clock model, `crystal_test`, docs | large |
| F | parts: the generator, the four generics, every `add` row, datasheets, goldens, the editor's kind list | large |
| G | app: drawer, sidebar facts, `kindName`, the crystal symbol, the reference prefix, the harness | large |
| H | landing tier | small |

Outcome of A: `planning/plan-44-data/stock-check.tsv` checks every row of
the download set in stock, Active or Last Time Buy, part by part (the Suntsu
listing hit DigiKey's 300-result cap in 9 series pieces, § 2);
`datasheets/crystal/<maker>/` holds every maker-site PDF a download-set row
cites, with `datasheets/crystal/datasheets.tsv`; `candidates.tsv` has `pdf`
and `hit` filled; `specs.tsv` has one row per (maker, series, PDF, Package /
Case), its scheme and pinout columns empty (B fills them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets,
  § Footprints); `docs/backend.md` (§ Adding a kind).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-44-data/` — `README.md`, `census/count.js`, `list.js`,
  `list.sh`, `common.py`, `urls.py`, `urlcheck.js`, `select.py`,
  `footprint-lines.py`; `raw/*.jsonl.gz` and `raw/*.err` (the planning
  listing), `urls-needed.tsv`, `url-check.tsv`, `candidates.tsv`,
  `footprints-needed.txt` (written at planning time, 2026-11-16).
- `planning/completed/plan-40-A-transistor-data.md`,
  `planning/plan-40-data/census/` (`stock-check.sh`, `fetch.js`,
  `textscan.py`, `sibling.py`, `place.py`, `specs-skeleton.py`: the scripts A
  adapts) and `planning/plan-42-A-regulator-data.md` (the same method, the
  same layout of `datasheets/<family>/<maker>/`).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p44 && cmake --build /tmp/logic-p44 -j8 && ctest --test-dir /tmp/logic-p44 -j8` → `100% tests passed` | MET (2026-11-22 at f442d31a: `100% tests passed, 0 tests failed out of 40`; T0 = 40. At planning, 2026-11-16 at 7108a13e: 36 of 36) |
| Golden sizes recorded | `git show HEAD:backend/tests/golden/parts.txt \| wc -l`; `traces.txt`; `footprints.txt` | at planning (7108a13e): **P0 = 32897**, **R0 = 32897**, **F0 = 1646**; E, F and C re-measure them at their start and use their own. Re-measured 2026-11-22 at f442d31a: `parts.txt` 33771, `traces.txt` 33771, `footprints.txt` 1823 (main moved on; not a use here, E/F/C re-measure) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p44-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-22 at f442d31a: `** BUILD SUCCEEDED **`) |
| No crystal kind yet | `grep -c 'PART_CRYSTAL' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/crystal.json` → no such file; `ls datasheets/crystal` → no such directory | MET (2026-11-16 at 7108a13e: 0, no file, no directory) |
| plan-46 (Part Wizard) complete | `ls planning/completed/plan-46-N-*.md` → exists | MET (2026-11-22: `planning/completed/plan-46-A…N-*.md` all 14 letters exist, `PART_GENERIC` is in `parts.h`, `Editor/KindInfo.swift` exists). Was NOT MET at planning. Owner decision 2026-11-16: plan-46 lands first. It appends `PART_GENERIC` after `PART_TRANSISTOR`, replaces `PartKinds.displayNames` with `Editor/KindInfo.swift`, and gives every kind an editor schema (no `formless` set); plan-44-E/F/G are written against that world |
| No other open plan adds crystals or a part kind of the same name | `grep -l -i 'PART_CRYSTAL\|Parts/crystal\.json\|datasheets/crystal' planning/plan-*.md \| grep -v plan-44 \| wc -l` → 0 | MET (2026-11-16: 0; 2026-11-22: the command counts 3, plan-47-A, plan-47-I and plan-48-A, but each only names plan-44's `PART_CRYSTAL` / `datasheets/crystal` in its own coordination note, none adds a crystal kind; see Corrections). plan-42 (regulators) is open and adds `PART_REGULATOR`; the lines both plans touch (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists, `ReferenceCategory.of`) are added only if absent, each step greps first (plan-44-E Phase 1, plan-44-F Phase 1, plan-44-G Phase 1) |
| DigiKey credentials and quota | `grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc` → 2; `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)" && node scripts/digikey.js --in-stock --pages 1 --limit 1 --category 171 --manufacturer 2388` prints `calls left today` ≥ 800 before Phase 2 (the quota is about 1000 a day, shared with other sessions; it resets at about 00:00 UTC) | MET (2026-11-22 06:15 UTC: credentials 2 of 2; `998 left today`; the quota reset at 00:00 UTC). Was NOT MET at planning (`589 left today` after the planning listing's 283 calls) |
| The Internet Archive answers (Suntsu's PDFs) | `curl -s 'http://archive.org/wayback/available?url=suntsu.com/wp-content/uploads/SAW212-Series.pdf'` → contains `"available": true` | MET (2026-11-16: snapshot 20240706163019) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-44-data/stock-check.tsv` has one row per row of the download
  set (744 at planning time, `awk -F'\t' '$25!="out" && NR>1' planning/plan-44-data/candidates.tsv \| wc -l`
  → 744), each `found` 1 with stock above 0 and status Active or Last Time
  Buy; a row that fails is `out` (reason `stock check`) in `candidates.tsv`
  and the next row of its (maker, class) enters the set.
- `datasheets/crystal/<maker>/` holds each maker-site PDF that at least one
  download-set row needs (191 distinct URLs at planning time);
  `datasheets/crystal/datasheets.tsv` maps each file to its URL, size and
  SHA-256.
- `candidates.tsv` has `pdf` and `hit` filled for every download-set row;
  `specs.tsv` exists with one row per (maker, series, PDF, DigiKey Package /
  Case) of those rows, its scheme and pinout columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter and no row is `add` yet: the verdicts and
  the final pick are B's.
- No datasheet hunting beyond the DigiKey link, the rewrites of § 4 and the
  same-maker sibling check.
- No new listing: the planning listing is complete for the pick (§ 2,
  complete-for-the-pick check). `scripts/digikey.js` is not changed.

## 2. Current State

The library has no crystal kind and no crystal part. The app's Settings
already have a crystal reference category with the prefix `Y`
(`Preferences/ReferencePrefixes.swift`: `case crystal`, `.crystal: "Crystal"`,
`.crystal: "Y"`), but `ReferenceCategory.of(_:)` has no case that returns it.
`footprints.json` has no crystal footprint (`python3 -I -c` over its ids for
`XTAL`, `CRYSTAL`, `HC49`, `HC-49`, `HC45`, `RESON` → none, 1646 ids); its
oscillator footprints (`OSC-…`) are 4- and 6-pad active oscillators. `datasheets/`
has one directory per family (`osc`, `capacitor`, …) and no `crystal/`;
`datasheets/skipped-parts.md` and `datasheets/obsolete-not-included.md` take
the skips. `planning/todo.md:71` asks to "Add crystals and resonators
(oscillators exist, crystals don't)"; its reference-board list has "crystal:
8 parts" (plan-43 left them out).

### Measured populations

All DigiKey numbers 2026-11-16, in stock (`--in-stock`), statuses Active and
Last Time Buy. Planning used 283 listing calls (878 left before the first,
589 after the last check).

| What | Count | Command |
|---|---|---|
| In stock, Active + LTB: Crystals (171) / Resonators (174) | **80783 / 673** | `node scripts/digikey.js --category Crystals --in-stock --status 0 --status 4 --pages 1 --limit 1 --json` (and `Resonators`) → `ProductsCount` |
| …Crystals by Type | MHz crystal 78960, kHz crystal (tuning fork) 1287, MHz crystal with thermistor 24 | same, facet 183 |
| …Crystals by Operating Mode | Fundamental 80228, 3rd overtone 58, 5th overtone 1 | same, facet 538 |
| …Resonators by Type / Features | Ceramic 457, SAW 211; built-in capacitor 349, without capacitor 14 | same, facets 183 and 5 |
| Makers ranked, Crystals + Resonators | **Suntsu 64666** (64666 + 0); **Abracon 2138** (2070 + 68); **ECS 2057** (2023 + 34); Suzhou Hangjing 1767, TGS 1165, Jauch Quartz 769, Aker 652, Geyer 649 | same two calls, `Manufacturers` facet; `node scripts/digikey.js --category 174 --manufacturer 2388 --in-stock --status 0 --status 4 --pages 1 --limit 1` → `0 of 0 parts` |
| The three makers, Crystals | Suntsu 64666 (kHz 68, MHz 64598, AEC-Q200 2313, all surface mount); Abracon 2070 (kHz 79, MHz 1982, 3rd overtone 11, through hole 132, AEC-Q200 137); ECS 2023 (kHz 114, MHz 1904, thermistor 5, 3rd overtone 8, through hole 265, AEC-Q200 280) | `node planning/plan-44-data/census/count.js 171 <maker id>` (535, 50, 2388) |
| The three makers, Resonators | Abracon 68 (ceramic 66, SAW 2, built-in capacitor 59); ECS 34 (ceramic 31, SAW 3, built-in capacitor 22, without 7); Suntsu 0 | `node planning/plan-44-data/census/count.js 174 <maker id>` |
| Series of the three makers (pieces) | Abracon 72 + 11, ECS 85 + 6, Suntsu 19 (Crystals + Resonators) | `list.js` summary lines `maker <id> category <id>: <n> parts, <k> series` in `raw/*.err` |
| Planning listing (`list.sh`, 5 pieces) | **7019 parts** listed (Abracon 2062 + 68, ECS 2023 + 34, Suntsu 2832); **283 calls** (94 + 104 + 66 + 12 + 7) | `python3 -I census/select.py raw /tmp/p44-cand.tsv --urls url-check.tsv --report` → `7019 parts listed`; the `list.js:` lines of `raw/*.err` |
| **Pieces cut at the 300-result cap** | **9**, all Suntsu: SXT324 (26876 parts), SXT224 (13838), SXT214 (11916), SXT114 (3990), SXTHM2 (2412), SXT144 (1914), SXT834 (1320), SAX324 (1188), SAX214 (1080); every Abracon and ECS series is complete (the largest, ABLS, is 281) | `grep -h 'cut at' planning/plan-44-data/raw/*.err` → 9 lines |
| Complete for the pick | the download set takes at most 30 rows of any series (`SAW312`), a tenth of the 300 listed, so every cut series' unlisted parts (rank above 300 by stock) are far below the pick | `awk -F'\t' '$25!="out" && NR>1 {print $2, $4}' planning/plan-44-data/candidates.tsv \| sort \| uniq -c \| sort -rn \| head -1` → `30 Suntsu SAW312` |
| Outcome of `select.py` | **600 picked**; out: below quota 5703, no datasheet 381, duplicate 157, 3rd/5th overtone 19, mode 5, SAW 5, thermistor 5 | `python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --report` → `out:` line |
| …picked per maker and class | Abracon 40 kHz / 120 MHz / 40 resonator; ECS 40 / 134 / 26; Suntsu 40 / 160 / 0 | same, per-class lines; `awk -F'\t' '$25=="candidate"{print $2,$3}' candidates.tsv \| sort \| uniq -c` |
| …status, mounting, grade | Active 600 (Last Time Buy 0); Surface Mount 565, Through Hole 35; AEC-Q200 131 | `awk -F'\t' '$25=="candidate"{print $6}' …`, `$13`, `$18` with `sort \| uniq -c` |
| …frequency | 32000 Hz to 76800000 Hz; kHz class: 118 × 32768 Hz and 2 × 32000 Hz | `awk -F'\t' '$25=="candidate"{print $7}' … \| sort -n \| sed -n '1p;$p'` |
| …ceramic resonators | Built in Capacitor 51 (the 3-pin parts), Without Capacitor 7, `-` 8 | `awk -F'\t' '$25=="candidate" && $3=="resonator"{print $19}' … \| sort \| uniq -c` |
| …series | Abracon 80, ECS 88, Suntsu 12 (the seven Suntsu series with no datasheet copy are out, below) | `awk -F'\t' '$25=="candidate"{print $2, $4}' … \| sort -u \| awk '{print $1}' \| uniq -c` |
| Datasheet URLs: `urlcheck.js` | **239** in-scope maker-site URLs: 219 of Abracon and ECS answer a PDF (every one); Suntsu 20 URLs (`archive`), **12** with an Internet Archive snapshot, 8 without (SWS112 under two paths, SWS212, SWS312, SWS412, SWS144, SXT534, SXT144: 7 series); one Suntsu row has no link at all | `cut -f2,4 planning/plan-44-data/url-check.tsv \| sort \| uniq -c` → `219 direct 1`, `12 archive 1`, `8 archive 0` |
| **The download set** (picked rows, then the next rows of each (maker, class) up to a quarter more) | **744 rows**, **191 distinct URLs** (Abracon 81, ECS 98, Suntsu 12); candidate 600 + reserve 144 | `python3 -I census/select.py … --report` → `download set: 744 rows; 191 distinct URLs` |
| …rows whose DigiKey link is a distributor copy or none (rewritten, § 4) | Abracon 37, ECS 2, Suntsu 4 (`suntsucompany.wpengine.com`) | `awk -F'\t' '$25!="out" && NR>1 && $21!="abracon.com" && $21!="ecsxtal.com" && $21!="suntsu.com" && $21!="www.ecsxtal.com" {print $2}' planning/plan-44-data/candidates.tsv \| sort \| uniq -c` |
| …rows from a cut piece | 200 (160 of the 600 picked) | `awk -F'\t' '$25!="out" && NR>1 && $24==1' …\| wc -l` → 200 |
| Footprint lines (letter, maker, package, supplier package, size), download set | 99: C 70 (620 parts; 30 with no existing footprint of the same body size code), D 29 (124 parts; 27 with none); 63 distinct (letter, pad count, body size) outlines | `awk -F'\t' '!/^#/{n[$1]++; p[$1]+=$8; if($11=="")g[$1]++} END{for(k in n) print k, n[k], p[k], g[k]+0}' planning/plan-44-data/footprints-needed.txt` |

### Verified properties

- **Sorting by stock works inside a series.** `list.js` sends `SortOptions
  {Field: "QuantityAvailable", SortOrder: "Descending"}` with `SeriesFilter`;
  `raw/171-2388.err` shows `series SXT324: 300 of 26876 (cut at 300: 23206)`
  and the parts arrive in falling stock, so the first 300 of a series are its
  300 most-stocked.
- **Downloads, Abracon and ECS.** `curl -sL https://abracon.com/Resonators/ABM7.pdf`
  → 200, `application/pdf`, 500045 bytes; `…/Resonators/ABS13.pdf`,
  `ABM8.pdf`, `ABLS-LR.pdf`, `AB26TRQ.pdf`, `ABS05W.pdf`, `ABM12.pdf`,
  `ABM8W.pdf` → 200 PDFs (§ 4 rule 3's guess); `…/Resonators/AWSCR-CPLA.pdf` →
  404, `…/datasheets/AWSCR-CPLA.pdf` (DigiKey's own link) is the PDF.
  `curl -sL https://ecsxtal.com/store/pdf/ECS-271.2-CDX-2616.pdf` → 200,
  704125 bytes; `https://www.ecsxtal.com/store/pdf/csm-7x.pdf` → 200,
  415769 bytes; `https://ecsxtal.com/store/pdf/ECS-CTP.pdf` → 200. All 219
  Abracon and ECS URLs of `url-check.tsv` answer a PDF (`urlcheck.js`).
- **Downloads, Suntsu.** `suntsu.com` is behind Cloudflare: `curl -sL
  https://suntsu.com/wp-content/uploads/SAW212-Series.pdf` → 403 (also with a
  Safari user agent, with Node's `fetch`, with a Referer, and for the home
  page). The Internet Archive holds the maker's own file:
  `curl -sL "https://web.archive.org/web/2024/https://suntsu.com/wp-content/uploads/SAW212-Series.pdf"`
  → 200, `application/pdf`, 329757 bytes, `pdftotext` first lines "Ceramic SMD
  2 Pad Automotive Watch Crystal / SAW212 Series / 2.0mm x 1.2mm … Part
  Numbering Guide". The archive refuses a burst of page fetches with HTTP 429
  (`urlcheck.js` first run: 19 of 20 → 429 at 4 at a time), so the
  downloader fetches Suntsu one file at a time, 3 s apart.
- **A Suntsu URL with no snapshot cannot be downloaded.** The
  availability API answers `{"archived_snapshots": {}}` for the 8 URLs of §
  2; `select.py --urls` makes their rows `out` (reason `no datasheet`) and
  the pick refills from the other series. The API also answers an empty
  object when throttled; `urlcheck.js` asks 4 times, 10 s apart, before it
  believes `none`.
- **Datasheet URL = maker's own site, file from the archive.** `docs/parts-format.md`
  § Datasheets requires `datasheets.json` `url` to be on the maker's own site
  (no archive copies) and "the same document as the local PDF or its current
  revision": Suntsu's entries therefore name `https://suntsu.com/…` and the
  local PDF is the archive's copy of that URL. `node scripts/check-datasheets.js`
  will report these URLs as `HTTP 403` (suntsu.com refuses it); F records the
  expected failures in `docs/parts-format.md` § Datasheets.
- **The ordering tables are in the PDFs as part-numbering guides** (Abracon:
  "Part Numbering" / "Ordering Information"; ECS: "Ordering Information";
  Suntsu: "Part Numbering Guide"): B writes one regex per (datasheet,
  series), as plan-40-B did.

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` (in B) only when all of these hold; any other outcome is
`out` or `skip` with the reason in the `reason` column.

1. **In stock** (quantity above zero) and **status Active or Last Time Buy**
   at DigiKey. `list.js` asks for statuses 0 and 4 with
   `MinimumQuantityAvailable` 1, so Obsolete, Discontinued, Not For New
   Designs and made-to-order parts are never listed. Every download-set row
   is checked part by part (§ 5).
2. **Scope** (`common.scope`): crystals (category 171) with Operating Mode
   Fundamental and Type "MHz Crystal" or "kHz Crystal (Tuning Fork)"; ceramic
   resonators (category 174, Type Ceramic); frequency 1 Hz … 100 MHz. Out:
   `overtone` (3rd / 5th overtone), `thermistor`, `saw` (SAW resonators),
   `mode` (a crystal whose Operating Mode is not Fundamental), `frequency`
   (none or above 100 MHz). The AEC-Q200 grade and through-hole packages are
   in.
3. **At most one of the same thing** (`common.dedupe`): the key is (maker,
   class, series, frequency, load capacitance, tolerance, stability, ESR,
   temperature range, package, supplier package, rating, features); a key
   keeps its most-stocked part (the others differ only in packing), the rest
   → `duplicate`.
4. **A datasheet on the maker's site** (`select.py --urls`): the row's
   resolved URL (§ 4) answers a PDF in `url-check.tsv`; else `no datasheet`.
5. **Each maker gets 200 rows** (`select.py`): up to 40 kHz crystals, up to
   40 ceramic resonators, the rest MHz crystals (Suntsu has no resonator and
   only 68 kHz crystals of which fewer have a datasheet, so its 200 are 40
   kHz + 160 MHz). Inside a class the rows go round-robin over the series
   (each series' most-stocked row first, then each series' second, …, the
   series with more stock first in a round), so every series is shown and no
   series takes more than its turn; the first quota rows are `candidate`, the
   next quarter of the class `reserve`, the rest `below quota`.
6. **A maker datasheet** (Phase 3; B's verdict): the PDF downloads (§ 4).
   Else B makes the row `skip`, reason `no datasheet on the maker's site`.
7. **In the datasheet's ordering table** (B): printed verbatim, or a full
   match of the (datasheet, series) ordering-code regex B writes in
   `specs.tsv`; else `skip`, reason `not in the datasheet's ordering table`.
8. **Not marked obsolete, discontinued, NRND or end of life by the
   datasheet** (B reads every `dead` = 1 row); a PDF whose every row is `out`
   for datasheet status is deleted and listed in `obsolete-not-included.md`.

Rules 6–8 apply to every download-set row, so B's final pick can fill a
skipped slot with the next row of the same (maker, class).

## 4. Datasheet placement

`census/place.py` (plan-40's, adapted) copies each needed PDF from
`/tmp/p44/pdf/<md5 of the URL>.pdf` to
`datasheets/crystal/<maker-slug>/<name>.pdf`, `<maker-slug>` `abracon`,
`ecs`, `suntsu`; `<name>` is the URL path's last segment (query string
dropped, `%20` → `-`, `.pdf` appended when missing, characters outside
`A-Za-z0-9._-` replaced by `-`); a clash gets `-<first 6 hex of the md5>`.
It writes `datasheets/crystal/datasheets.tsv`: path, URL, bytes, SHA-256, via
(`direct` or `archive`), snapshot (the archive timestamp or empty). Files
with the same SHA-256 under two URLs share one file. A PDF no download-set
row uses is not copied.

The URL of a row is `url` of `candidates.tsv` (`common.resolve_urls`):

1. A DigiKey link on the maker's own site is the URL (`abracon.com`,
   `ecsxtal.com`, `suntsu.com`; `www.` dropped for Abracon and Suntsu;
   `suntsucompany.wpengine.com` → `suntsu.com`, same path).
2. A Waldom, DigiKey (`mm.digikey.com`) or missing link → the commonest
   maker-site link among the listed rows of the same (maker, series).
3. Else Abracon `https://abracon.com/Resonators/<series>.pdf`, ECS
   `https://ecsxtal.com/store/pdf/<series>.pdf` (Suntsu has none: no sibling
   means no PDF).
4. Anything else → no PDF; `sibling.py` then looks for the part number in a
   downloaded same-maker PDF.

Downloader rules (`census/fetch.js`, Node `fetch` with a Safari user agent,
4 at a time, one retry, a `%PDF-` check and `pdfinfo` on the body): a `direct`
URL is fetched as given; an `archive` URL (Suntsu) is fetched from
`https://web.archive.org/web/<timestamp>if_/<url>` with the timestamp of the
snapshot in `url-check.tsv`, one file at a time, 3 s apart, a 429 waited out
(30 s, up to 4 tries). A failed archive fetch makes the rows of that URL `out`
(reason `no datasheet`), recorded in § Corrections.

## 5. The 300-result cap: the part-by-part stock check

DigiKey returns at most the first 300 results of a search, and the planning
listing hit that cap in 9 Suntsu series (§ 2). The listing is complete for
the pick, but a listing is a snapshot, and 200 of the 744 download-set rows
come from a cut series: every row of the download set is looked up by its own
part number before B may use it. `census/stock-check.sh` runs
`node scripts/digikey.js --manufacturer <maker id> --in-stock --status 0
--status 4 --limit 5 --json "<mpn>"` (one call; the maker id from the raw
listing, so no look-up calls) and writes `planning/plan-44-data/stock-check.tsv`
(`mpn`, `found` 0/1, `stock`, `status`, `date`); `found` = 1 when a product
with exactly that `ManufacturerProductNumber` has `QuantityAvailable` above 0.
The script skips part numbers already in the file, so a run cut by the quota
goes on the next day. `select.py --stock stock-check.tsv` makes a `found` = 0
row `out`, reason `stock check`, sets `stock_checked`, and refills the class
from the next rows.

## 6. Scripts

- Written at planning time (`README.md`): `census/count.js`, `list.js`,
  `list.sh`, `common.py`, `urls.py`, `urlcheck.js`, `select.py` (with
  `--urls`, `--stock`), `footprint-lines.py`.
- Written in this letter, adapted from `planning/plan-40-data/census/`:
  `stock-check.sh` (§ 5), `fetch.js` (§ 4 rules), `textscan.py`,
  `sibling.py`, `place.py` (§ 4; `--verify` and, for B, `--prune`),
  `specs-skeleton.py` (paths, the three maker hosts, the crystal columns).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Restore the listing

- [x] Run `python3 -I planning/plan-44-data/census/select.py planning/plan-44-data/raw /tmp/p44-cand.tsv --urls planning/plan-44-data/url-check.tsv --report`:
      `7019 parts listed`, `candidates: 600; from a cut piece: 160`,
      `download set: 744 rows; 191 distinct URLs`; `cmp` with `candidates.tsv`
      → `reproducible`. Output 2026-11-22: `7019 parts listed`, `candidates: 600; from a cut piece: 160`, `download set: 744 rows; 191 distinct URLs`, `reproducible`.

Acceptance: the planning pick reproduces.
  Check: `python3 -I planning/plan-44-data/census/select.py planning/plan-44-data/raw /tmp/p44-cand.tsv --urls planning/plan-44-data/url-check.tsv && cmp /tmp/p44-cand.tsv planning/plan-44-data/candidates.tsv && echo reproducible` → `reproducible` (est. < 1 min). Output 2026-11-22: `reproducible`.
Commit: a3b45155

### Phase 2 — The part-by-part stock check (one quota day)

- [x] `census/stock-check.sh` (§ 5); run it for every row of the download
      set (744 calls; ≥ 800 left before the run, § Prerequisites); a run cut
      by the quota goes on the next day. Done 2026-11-22 (local date 11-21 in
      `stock-check.tsv`): 998 calls left before, 252 after; `stock-check.tsv`
      has 745 rows, `found` = 1 for 744, all `Active`.
- [x] `select.py … --stock planning/plan-44-data/stock-check.tsv --report` writes
      `candidates.tsv`; a row that went `out` leaves the set and the next row
      of its (maker, class) enters it; check every new row the same way until
      no row of the set is unchecked. Record the number of `found` = 0 rows
      and of extra calls in § Corrections. Result: 1 `found` = 0 row
      (`ECS-240-12-37BQ-CKY-TR`), 1 refill row checked (`ECS-320-10-37BQ-CKY-TR`,
      `found` 1); `unchecked.py` → `unchecked: 744 download-set rows, 0 unchecked`;
      the set is still 744 rows, 191 distinct URLs.

Acceptance: every download-set row is in stock, Active or Last Time Buy.
  Check: `awk -F'\t' 'NR>1 && $2!=1' planning/plan-44-data/stock-check.tsv \| wc -l` → the `found` = 0 count recorded above, and `awk -F'\t' '$25!="out" && NR>1 && $31==""' planning/plan-44-data/candidates.tsv \| wc -l` → 0 (est. < 1 min; the run itself is the quota day, about 35 min at 23 calls a minute). Output 2026-11-22: `1` and `0`.
Commit: b757909b

### Phase 3 — Download, place and scan the datasheets

- [x] `census/fetch.js`, `place.py`, `textscan.py`, `sibling.py` (§ 4, § 6):
      download every distinct URL the download set cites (191 at planning)
      by § 4's rules into `/tmp/p44/pdf`; `%PDF-` and `pdfinfo` on each;
      failures recorded in § Corrections with their host. Output
      2026-11-22: `191 urls; 191 are PDFs; 0 failed` (179 direct, 12 Suntsu
      from the Internet Archive); `textscan.py`: `744 rows with their own PDF;
      46 printed verbatim; 2 dead`; `sibling.py`: `0 eligible rows without
      their own PDF`.
- [x] Place the PDFs under `datasheets/crystal/<maker-slug>/` and write
      `datasheets/crystal/datasheets.tsv`; `place.py --verify` → `0
      missing, 0 unused`. The PDFs stay out of the commits until B prunes
      them (B Phase 4), so the history never carries PDFs no `add` row uses.
      Output: `190 PDFs placed for 191 URLs` (two URLs are one file by
      SHA-256), `0 missing, 0 unused`. `datasheets/crystal/` (the PDFs and
      `datasheets.tsv`) is untracked in the worktree until B Phase 4.
- [x] `candidates.tsv`: `pdf` (the placed path) for every download-set row
      whose URL, rewrite or sibling gave a PDF, and `hit` = 1 when its part
      number is printed verbatim (`pdftotext -layout`, token-boundary
      search); `dead` = 1 when the text near the part number says obsolete,
      discontinued, NRND, "not recommended for new designs" or end of life (B
      reads every such row). `hit` = 1 on 46 rows (Abracon 8, ECS 38,
      Suntsu 0: Suntsu's PDFs print part-numbering guides, not part
      numbers), `dead` = 1 on 2 rows (`ABS05-32.768KHZ-X-T5`,
      `ABS05-32.768KHZ-X-3-T3`: `ABS05.pdf`).
- [x] `census/specs-skeleton.py`: `planning/plan-44-data/specs.tsv`, one row
      per (maker, series, PDF, Package / Case) of the download set, columns
      `maker`, `series`, `pdf`, `package`, `rows`, `example`, `scheme`,
      `label` (B's), `pinout`, `terminals`, `capacitors` (B's). Output:
      `196 rows; 0 kept from before`.
- [x] Record `du -sh datasheets/crystal` here. `133M datasheets/crystal`
      (2026-11-22, 190 PDFs, before B's prune).

Acceptance: every download-set row either has a PDF or a reason it has none.
  Check: `awk -F'\t' '$25!="out" && NR>1 && $28==""' planning/plan-44-data/candidates.tsv \| wc -l` → the rows B will `skip` for no maker datasheet (recorded here); `find datasheets/crystal -iname '*.pdf' \| wc -l` = `tail -n +2 datasheets/crystal/datasheets.tsv \| cut -f1 \| sort -u \| wc -l` (est. 15 min: 191 downloads, 12 of them one at a time from the archive). Output 2026-11-22: `0` rows without a PDF (so B has no `no datasheet on the maker's site` skip from A's side); `190` = `190`.
Commit: —

## Validation Plan

- Tests: none in this letter (data only); `select.py` is checked by its
  `--report` outputs above.
- Coverage check: the three Acceptance checks cover the listing, the stock
  check and the downloads.
- Runtime proof: n/a.
- Doc sync: none (`datasheets/skipped-parts.md` and
  `datasheets/obsolete-not-included.md` are B's).
- Final gate: plan-44-H.

## Decisions

Owner decisions (asked once, 2026-11-16, before this plan was written):

- **Sub-types in scope:** MHz crystals, kHz tuning-fork crystals, ceramic
  resonators. SAW resonators, overtone crystals and thermistor crystals are
  out.
- **Pins, mounting, grades in scope:** two-pin and four-pad SMD crystals,
  through-hole crystals (HC-49/U, HC-49/US, cylinder, radial), AEC-Q200
  automotive grade, and three-pin resonators with built-in capacitors.
- **Size: 600 parts**, 200 per maker, the top three makers by DigiKey in-stock
  count over crystals and resonators (Abracon, ECS, Suntsu).
- **Extras:** generic parts with a footprint picker; frequency, load
  capacitance, tolerance, stability and ESR facts in the sidebar and drawer
  sorting; a simulated clock from a crystal (plan-44-E). No part-editor form
  for the kind.

Author decisions (no user-visible fork; recorded so nobody re-litigates):

- **The ranking sums the two categories** (the sub-types in scope span both);
  Suntsu's 64666 is mostly frequency and tolerance variants of 19 series,
  so each maker's share is capped at 200 rows and the rows are spread over
  the series round-robin (§ 3 rule 5) instead of taken by stock alone, which
  would give 200 rows of one series.
- **The quotas** (kHz at most 40, resonators at most 40, MHz the rest of
  200): MHz crystals are 98 % of the stock; kHz and resonator parts get their
  own share so each maker shows them. A class short of rows hands its unused
  quota to the MHz crystals.
- **Maximum frequency 100 MHz** for a pick: the clock model halves the period
  in picoseconds and a faster part adds event cost for no schematic use; the
  loader accepts up to 1 GHz (plan-44-E).
- **Suntsu's PDFs come from the Internet Archive's copy of the maker's URL**
  (the maker's site refuses every automated fetch); the part's `datasheet`
  entry names the maker's URL. A Suntsu series with no archived copy is out.
- **The stock check covers the whole download set** (744), not only the rows
  from cut pieces (200): the owner asked for the part-by-part re-check
  whenever the cap was hit, and checking the refill rows now keeps B free of
  DigiKey calls.
- **B refills only from the download set.** A (maker, class) share that is
  still short after B's verdicts stays short and is recorded with its count
  in B § Corrections (no second listing).
- **Datasheets live in `datasheets/crystal/<maker>/`**, one tree for
  crystals and resonators.

## Corrections

- **2026-11-22, Prerequisites row "No other open plan adds crystals":** the
  command counts 3 (plan-47-A, plan-47-I, plan-48-A), not 0. `grep -n` shows
  each only names plan-44's `PART_CRYSTAL` / `datasheets/crystal` in its own
  coordination note (plan-47-I line 84: "append other kinds (plan-42
  `PART_REGULATOR`, plan-44 `PART_CRYSTAL`)"); none creates a crystal kind or
  part. The row is MET; its command over-counts mentions. Use `grep -l …
  planning/plan-*.md | grep -v 'plan-44\|plan-47\|plan-48'` → 0 from now on.
- **2026-11-22, § 5 / § 6 script:** plan-40's `cut-picked.py` lists only the
  picked rows of cut pieces and reads only `*.jsonl`; § 5 checks the whole
  download set and the planning listing is gzipped, so A adds
  `census/unchecked.py` (every row whose `state` is not `out`, skipping part
  numbers already in `stock-check.tsv`, reading `*.jsonl*`) and
  `stock-check.sh` calls it.

- **2026-11-22, Phase 2 result:** `found` = 0 rows: **1** (`ECS-240-12-37BQ-CKY-TR`,
  not in stock); extra calls for the refill: **1** (`ECS-320-10-37BQ-CKY-TR`,
  in stock, Active). The planning snapshot (7019 parts, 2026-11-16) was nearly
  exact for the pick: 743 of 744 rows still in stock six days later. Calls
  spent: 746 (744 rows + 1 refill + 1 repeated), 998 → 252 left.
- **2026-11-22, `stock-check.sh` bug (inherited from plan-40's copy):** its
  rate-limit test was `grep -q 'rate limited\|429' $last`; `$last` ends with
  `… N API calls, 429 left today`, so the run stopped with exit 1 when the
  quota count reached 429 (after 569 rows). `scripts/digikey.js` throws
  `rate limited by DigiKey; …` on a real 429, so the test is now
  `grep -q 'rate limited'`. The failed run lost nothing but one repeated
  call (the row being checked was not yet written); the rerun went on from
  `stock-check.tsv`.

## Summary

A is the plan's quota-bound letter: one day of DigiKey calls (744 part-by-part
look-ups) and 191 downloads, 12 of them from the Internet Archive. Its risk is
wrong data feeding B (a stale stock figure, a PDF that is a distributor copy,
a Suntsu file the archive serves in an old revision): the stock check and the
`%PDF-` / verbatim-name checks are there to catch it. Out of scope for the
whole plan: SAW and RF resonators, overtone crystals, thermistor crystals,
oscillators (active, kind `oscillator`), crystal arrays and filters, makers
outside the top three (Suzhou Hangjing, TGS, Murata, Epson, NDK, TXC, Jauch,
…), frequencies above 100 MHz, a part-editor form, and any analog behavior
(load-capacitance matching, drive level, aging). Nothing in the tree except
`planning/plan-44-data/` and `datasheets/crystal/` changes in A.
