# plan-47-A: Inductors, fuses, PTC resettable fuses, relays and buzzers — part-by-part stock check, datasheet downloads

Last updated: 2026-11-16
Overall Effort: huge (> 3d) — plan-47 is A → B → C → D → E → F → G → H → I → J → K → L → M → N → O
Effort: large (3h–1d of work; plus four DigiKey quota days)
Depends on: nothing (§ Prerequisites)

plan-47 adds **inductors**, **fuses**, **PTC resettable fuses (polyfuses)**,
**relays** and **buzzers** to the parts library and the app (`planning/todo.md:72`),
from the top three makers of each type at DigiKey by in-stock count (§ 2):

| Type | Makers (rank order) | In scope |
|---|---|---|
| inductor | **Bourns**, **Murata**, **Würth Elektronik** | two-terminal fixed inductors, surface mount and through hole, AEC-Q200 included |
| fuse | **Littelfuse**, **Eaton**, **SCHURTER** | board-mount fuses (surface mount, radial and axial through hole) and 5 × 20 mm / 6.3 × 32 mm cartridge fuses on PCB fuse clips |
| PTC | **Littelfuse**, **Bourns**, **Bel Fuse** | polymeric PTC resettable fuses, surface mount and radial, AEC-Q200 included |
| relay | **TE Connectivity**, **Franjobaim**, **Panasonic** | board-mount signal, power, reed, solid-state (photo) and automotive relays, DC coils, one to five poles, latching included, military grade included |
| buzzer | **Same Sky**, **PUI Audio**, **Mallory Sonalert** | board-mount buzzers, with a built-in driver (indicators) and without (transducers), piezo and magnetic |

Each maker gets up to 200 parts of a type: up to 3000 parts. In play mode an
inductor and an intact fuse conduct like a wire; a fuse can be blown and
restored with a click; a relay's contacts change over 1 ms after its coil is
driven; a buzzer that is driven draws sound waves and plays a tone. The four
new kinds are `inductor`, `fuse` (fuses and PTCs), `relay` and `buzzer`.

The single behavioral outcome of plan-47: loading the part files gives the
baseline P0 parts (§ Prerequisites) plus one part per `add` row of
`planning/plan-47-data/candidates.tsv` (2953 picked at planning time:
inductor 600, fuse 600, PTC 600, relay 600, buzzer 553); the app lists them
under **Discrete → Inductors**, **Discrete → Fuses**, **Discrete →
Resettable fuses**, **Switches & Relays → Relay** and **Displays & Indicators → Buzzers**; in play mode a
gate's output driving a relay coil closes the relay's normally-open contact
and lights an LED behind it, the same LED behind a fuse goes dark when the
fuse is clicked, and a buzzer on the gate's output sounds; `ctest` passes T0
+ 4 programs (`inductor_test`, `fuse_test`, `relay_test`, `buzzer_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | part-by-part stock check of the download set, datasheet downloads and placement, the verbatim scan, `specs.tsv` skeleton | large + 4 quota days |
| B | verdicts for inductors, fuses and PTCs: ordering tables, datasheet status, pinouts and facts | large |
| C | verdicts for relays and buzzers; the final pick, skipped parts, obsolete PDFs, the final `footprints-needed.txt` | large |
| D | footprints: inductors | large |
| E | footprints: fuses, and the six cartridge-fuse clip pairs | large |
| F | footprints: PTC resettable fuses | large |
| G | footprints: relays | large |
| H | footprints: buzzers | large |
| I | backend: the `inductor` and `fuse` kinds, fixed contacts, the blowable fuse, tests, docs | large |
| J | backend: the `relay` and `buzzer` kinds, tests, docs | large |
| K | parts: inductors, fuses, PTCs; datasheets, footprints, goldens, the editor's kind lists | large |
| L | parts: relays and buzzers; goldens | large |
| M | app: drawer, sidebar facts, `kindName`, references, the inductor and fuse symbols, blowing a fuse | large |
| N | app: the relay and buzzer symbols, the relay's play state, the buzzer's sound, the harness | large |
| O | landing tier | small |

Outcome of A: `planning/plan-47-data/stock-check.tsv` checks every row of
the download set in stock, Active or Last Time Buy, part by part (19 of the
36 listing pieces hit DigiKey's 300-result cap, § 2);
`datasheets/{inductor,fuse,relay,buzzer}/<maker>/` hold every maker-site PDF
a download-set row cites, with one `datasheets.tsv` per family;
`candidates.tsv` has `pdf` and `hit` filled; `specs.tsv` has one row per
(type, maker, series, PDF, package), its scheme and pinout columns empty (B
and C fill them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets,
  § Footprints); `docs/backend.md` (§ Adding a kind).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-47-data/` — `README.md`; `census/dk.js`, `count.js`,
  `list.js`, `list.sh`, `common.py`, `urls.py`, `urlcheck.py`, `select.py`,
  `footprint-lines.py`; `raw/*.jsonl.gz` and `raw/*.err` (the planning
  listing), `clips/` (the fuse-clip listing), `urls-needed.tsv`,
  `url-check.tsv`, `candidates.tsv`, `footprints-needed.txt` (written at
  planning time, 2026-11-16).
- `planning/completed/plan-40-A-transistor-data.md` and
  `planning/plan-40-data/census/` (`stock-check.sh`, `fetch.js`,
  `textscan.py`, `sibling.py`, `place.py`, `specs-skeleton.py`: the scripts A
  adapts); `planning/completed/plan-44-A-crystal-data.md` (the same method, the same
  layout of `datasheets/<family>/<maker>/`).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p47 && cmake --build /tmp/logic-p47 -j8 && ctest --test-dir /tmp/logic-p47 -j8` → `100% tests passed` | MET (2026-11-16 at 349f25b3: `100% tests passed, 0 tests failed out of 37`; T0 = 37, `grep -c '^logic_test' backend/CMakeLists.txt` → 37) |
| Golden sizes recorded | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt` | at planning (349f25b3): **P0 = 32897**, **R0 = 32897**, **F0 = 1646**; I, J, K and L re-measure them at their start and use their own |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-16 at 349f25b3: `** BUILD SUCCEEDED **`) |
| None of the four kinds exists | `grep -c 'PART_INDUCTOR\|PART_FUSE\|PART_RELAY\|PART_BUZZER' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/{inductor,fuse,relay,buzzer}.json` → no such file; `find datasheets/inductor datasheets/fuse -type f \| wc -l` → 0; `ls datasheets/relay datasheets/buzzer` → no such directory | MET (2026-11-16 at 349f25b3: 0; no file; 0 files in the two empty directories; no directory) |
| No other open plan adds these parts or kinds | `grep -l -i 'PART_INDUCTOR\|PART_FUSE\|PART_RELAY\|PART_BUZZER\|Parts/inductor\.json\|Parts/fuse\.json\|Parts/relay\.json\|Parts/buzzer\.json\|datasheets/relay\|datasheets/buzzer' planning/plan-*.md \| grep -v plan-47 \| wc -l` → 0 | MET (2026-11-16: 0). plan-42 (regulators, `PART_REGULATOR`), plan-44 (crystals, `PART_CRYSTAL`) and plan-45 (terminal blocks, the existing `connector` kind) are open: the lines several plans touch (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists, `ReferenceCategory.of`, `PartSymbol`) are added only if absent and after whatever is last; each such step greps first (plan-47-I Phase 1, J Phase 1, K Phase 1, M Phase 1) |
| No open plan uses this plan's footprint id prefixes | `grep -l '"IND-\|"FUSE-\|"PTC-\|"RELAY-\|"BUZZER-' planning/plan-4[245]-*.md planning/plan-4[245]-data/*.json 2>/dev/null \| wc -l` → 0; `python3 -I -c "import json;print(sum(f['id'].startswith(('IND-','FUSE-','PTC-','RELAY-','BUZZER-')) for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-16: 0 and 0; the library's one id with `IND` in it is `THINDPAK-3`) |
| DigiKey credentials and quota | `grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc` → 2; `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)" && node scripts/digikey.js --in-stock --pages 1 --limit 1 --category 150 --manufacturer 507` prints `calls left today` ≥ 900 before each day's run of Phase 2 (about 1000 a day, shared with other sessions; it resets at about 00:00 UTC) | NOT MET (2026-11-16: credentials 2 of 2; `148 calls left today` after planning's 205 calls; the next reset is 2026-11-17 00:00 UTC) |
| The maker sites answer Python's `urllib` | `python3 -I planning/plan-47-data/census/urlcheck.py <(head -3 planning/plan-47-data/urls-needed.tsv) /tmp/p47/pdf \| cut -f3` → `ok`, `1`, `1` | MET (2026-11-16: 1745 of 1866 URLs answered a PDF, § 2) |
| `pdftotext`, `pdfinfo` | `which pdftotext pdfinfo` → two paths | MET (2026-11-16) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-47-data/stock-check.tsv` has one row per row of the download
  set (3522 at planning time, `awk -F'\t' 'NR>1 && ($23=="candidate" || $23=="reserve")' planning/plan-47-data/candidates.tsv \| wc -l`
  → 3522) and one per fuse clip (6, § 6), each `found` 1 with stock above 0
  and status Active or Last Time Buy; a row that fails is `out` (reason
  `stock check`) in `candidates.tsv` and the next row of its (type, maker,
  class) enters the set.
- `datasheets/inductor/<maker>/`, `datasheets/fuse/<maker>/` (fuses, PTCs and
  the clip datasheets), `datasheets/relay/<maker>/` and
  `datasheets/buzzer/<maker>/` hold each maker-site PDF that at least one
  download-set row needs (1461 distinct URLs at planning time, plus 5 clip
  PDFs); each family's `datasheets.tsv` maps each file to its URL, size and
  SHA-256.
- `candidates.tsv` has `pdf` and `hit` filled for every download-set row;
  `specs.tsv` exists with one row per (type, maker, series, PDF, package) of
  those rows, its scheme and pinout columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter and no row is `add` yet: the verdicts are
  B's and C's, the final pick C's.
- No datasheet hunting beyond the DigiKey link, the rewrites of § 4 and the
  same-maker sibling check.
- No new listing: the planning listing is complete for the pick (§ 2).
  `scripts/digikey.js` is not changed.

## 2. Current State

The library has no inductor, fuse, relay or buzzer kind and no such part
(`planning/plan-42-C-regulator-pinouts.md:93`: "The library has no inductor"). The app's Settings
already have the reference categories with their prefixes
(`Preferences/ReferencePrefixes.swift`: `fuse` "F", `relay` "K", `inductor`
"L", `speaker` "Speaker / Buzzer" "LS"), but `ReferenceCategory.of(_:)` has
no case that returns them. `footprints.json` (1646 footprints) has chip
footprints for resistors and capacitors (`RES-0603`, `CAP-0805-KEMET`, …)
and no inductor, fuse, relay or buzzer footprint. `datasheets/inductor/` and
`datasheets/fuse/` exist and are empty; `datasheets/relay/` and
`datasheets/buzzer/` do not exist. `planning/todo.md:72` asks to "Add
inductors, fuses / polyfuses, relays and buzzers."

### Measured populations

All DigiKey numbers 2026-11-16, in stock (`MinimumQuantityAvailable` 1),
statuses Active (0) and Last Time Buy (4). `C` =
`node planning/plan-47-data/census/count.js` (one call each; credentials as
in § Prerequisites); `PCB` = `--param 69=409393,411897` (Mounting Type
Surface Mount or Through Hole). Planning used 205 calls (353 left before the
first, 148 after the last).

| What | Count | Command |
|---|---|---|
| In stock, Active + LTB, any mounting: Fixed Inductors (71) / Fuses (139) / PTC Resettable Fuses (150) | **44245 / 9965 / 2726** | `C 71`, `C 139`, `C 150` → `all` |
| …Signal Relays (189) / Power Relays (188) / Reed Relays (964) / Solid State Relays (183) / Automotive Relays (962) | **2954 / 10377 / 529 / 3818 / 1106** | `C <id>` → `all` |
| …Alarms, Buzzers, and Sirens (157) / Buzzer Elements, Piezo Benders (160, out of scope) | **3114 / 201** | `C 157`, `C 160` |
| Board mount (`PCB`): 71 / 150 / 189 / 188 / 964 / 183 / 962 / 157 | 43449 (surface mount 38601, through hole 4848) / 2648 / 2366 / 4062 / 465 / 1576 / 511 / 1248 | `C <id> PCB` → `all` |
| Fuses in scope: board mount + cartridge on a holder (5 × 20 mm, 3AB/3AG 6.3 × 32 mm) | 5003 + 2649 = **7652** | `C 139 PCB`; `C 139 --param 69=395953 --param 16=248782,201760` |
| PTCs in scope: polymeric, board mount | **2212** (ceramic 234 and untyped 183 are out) | `C 150 PCB --facets 183`; `C 150 PCB --param 183=388650` |
| **Makers ranked, inductors** (71, `PCB`) | **Bourns 4710**, **Murata 4070**, **Würth 3770**; TDK 3601, Central Technologies 2544, Pulse 2443, Taiyo Yuden 2063 | `C 71 PCB` → `maker:` lines |
| **Makers ranked, fuses** (139, board mount + cartridge) | **Littelfuse 2863** (1934 + 929), **Eaton 1510** (657 + 853), **SCHURTER 839** (557 + 282); Bel Fuse 625 (448 + 177), Bourns 498, OptiFuse 427 (112 + 315) | the two fuse commands above → `maker:` lines |
| **Makers ranked, PTCs** (150, `PCB`, polymeric) | **Littelfuse 947**, **Bourns 449**, **Bel Fuse 272**; YAGEO 165, Eaton 139, SCHURTER 64 | `C 150 PCB --param 183=388650` |
| **Makers ranked, relays** (189 + 188 + 964 + 183 + 962, `PCB`, the DigiKey maker ids of one company summed) | **TE Connectivity 2564** (ids 844: 1563, 41: 876, 994: 75, 17: 50), **Franjobaim 1152**, **Panasonic 1146**; Aratas + Omron 1083 (1023 + 60), CIT Relay and Switch 483, American Zettler 312 | `C <id> PCB --facets 0 --makers 80` for the five ids, the `maker:` lines summed per id |
| **Makers ranked, buzzers** (157, `PCB`) | **Same Sky 231**, **PUI Audio 215**, **Mallory Sonalert 206**; DB Products 156, Soberton 79. Floyd Bell (1100 with every mounting) has no board-mount part in the first twelve | `C 157 PCB` |
| Planning listing (`list.sh`, 36 pieces: type × maker × mounting or relay category) | **8075 parts** listed (inductor 1500, fuse 2431, PTC 1321, relay 2171, buzzer 652); **171 calls** | `python3 -I census/select.py raw /tmp/p47-cand.tsv --urls url-check.tsv --report` → `8075 parts listed`; `grep -h 'list.js:' raw/*.err \| awk '{c+=$4; p+=$2} END{print p, c}'` → `8075 171` |
| **Pieces cut at the 300-result cap** | **19 of 36**: `ind-bourns-smd` (3791 parts), `ind-murata-smd` (4069), `ind-wurth-smd` (3377), `ind-bourns-tht` (919), `ind-wurth-tht` (393), `fuse-littelfuse-smd` (672), `-tht` (1262), `-cart` (929), `fuse-eaton-tht` (434), `-cart` (853), `fuse-schurter-smd` (431), `ptc-littelfuse-smd` (433), `-tht` (514), `relay-te-signal` (1423), `relay-te-power` (1103), `relay-franjobaim-power` (826), `relay-panasonic-signal` (346), `-power` (353), `-ssr` (406). The other 17 are complete (every buzzer piece, every Bel Fuse and Bourns PTC piece) | `grep -h 'cut at' planning/plan-47-data/raw/*.err` → 19 lines |
| Complete for the pick | a cut piece holds its 300 most-stocked parts (the listing is sorted by stock, § Verified); the pick takes rows in stock order inside each series, so an unlisted part (rank above 300 in its piece) could only replace a listed one of lower stock in the same piece | `grep -h 'cut at' raw/*.err` shows the stock of each piece's 300th part (from 24 for `relay-panasonic-power` to 41896 for `ind-murata-smd`) |
| Outcome of `select.py` | **2953 picked**, 569 reserve; out: below quota 2892, duplicate 1202, no datasheet 312, termination 93, coil voltage 28, more than two terminals 19, no stock figure 6, contact form 1 | `python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --report` |
| …picked per maker and class | inductor: Bourns 140 smd + 60 tht, Murata 200 smd (it has no through-hole part: `ind-murata-tht: 0 of 0`), Würth 140 + 60. fuse: 70 smd + 60 tht + 70 cartridge for each of the three. PTC: 110 smd + 90 tht for each of the three. relay: TE power 117, signal 60, ssr 4, auto 14, reed 5; Franjobaim power 151, signal 34, auto 15; Panasonic power 85, signal 60, ssr 40, auto 15. buzzer: Same Sky indicator 73 + transducer 127; PUI 67 + 133; **Mallory 82 + 71 = 153** (every in-scope Mallory row with a datasheet) | same, the per-share lines |
| …status, grade | download set: Active 3520, Last Time Buy 2. Picked: AEC-Q200 428 (PTC 296, inductor 132), Military 19 (relays) | `awk -F'\t' '$23=="candidate"{print $3, $14}' candidates.tsv \| sort \| uniq -c` |
| Datasheet URLs: `urlcheck.py` | **1866** in-scope maker-site URLs, **1745** answer a PDF; 121 do not: Würth 100 (HTTP 404), PUI 8, Panasonic 6, TE 5 (HTTP 500), Mallory 1, Same Sky 1 | `cut -f1,3 planning/plan-47-data/url-check.tsv \| sort \| uniq -c` |
| **The download set** (picked rows, then the next quarter of each class) | **3522 rows**, **1461 distinct URLs** (inductor 416, fuse 204, PTC 101, relay 171, buzzer 569); Würth, Same Sky, PUI and Mallory publish one PDF per part (Würth 250 rows, 250 URLs) | `select.py … --report` → `download set: 3522 rows; 1461 distinct URLs` |
| …rows whose URL is not DigiKey's link | sibling 70, guess 120 (§ 4) | `awk -F'\t' 'NR>1 && ($23=="candidate"\|\|$23=="reserve"){print $21}' candidates.tsv \| sort \| uniq -c` |
| …rows from a cut piece | 2146 (1713 of the 2953 picked) | `select.py … --report` → `from a cut piece` |
| Footprint lines, download set | **1076**: D inductor 229 (750 parts), E fuse 103 (753), F PTC 287 (710), G relay 267 (738; 157 distinct (maker, series)), H buzzer 190 (571) | `awk -F'\t' '!/^#/{n[$1]++; p[$1]+=$10} END{for(k in n) print k, n[k], p[k]}' planning/plan-47-data/footprints-needed.txt` |
| PCB fuse clips of the three fuse makers | 88 in stock (clip type, board mount); the most-stocked through-hole clip per (maker, fuse diameter) is § 6's table | `node census/list.js clips/clips.jsonl fuseclip fuse-clips 140 --maker 18,283,486 --param 1724=331018 --param 69=409393,411897` → `piece fuse-clips: 88 of 88` |
| Size of the downloaded PDFs | 1.0 GB for the 1745 URLs that answered (the download set's 1461 are a subset; the tree's `datasheets/` is 6.5 GB, 8353 PDFs) | `du -sh /tmp/p47/pdf`; `du -sh datasheets`; `find datasheets -iname '*.pdf' \| wc -l` |

### Verified properties

- **Sorting by stock works with parametric filters.** `list.js` sends
  `SortOptions {Field: "QuantityAvailable", SortOrder: "Descending"}` with
  `ParameterFilterRequest`; `raw/ind-murata-smd.err` shows `piece
  ind-murata-smd: 300 of 4069 (cut at 300: 41896)` and the parts arrive in
  falling stock.
- **Six listed rows carry no stock figure** (`04941.25NRHF`, `37311000000`,
  `37311250000`, `38511250000`, `0603L200SLYR`, `TRF250-120US`: DigiKey
  returned no `QuantityAvailable`); `common.in_scope` makes them `out`, reason `no stock
  figure`.
- **DigiKey gives the automotive relays (962) no Termination Style**: the
  scope rule accepts an empty termination for that category only
  (`common.classify`).
- **Downloads need Python's `urllib` with three headers.** `curl` (bare, with
  a Safari or Chrome user agent, HTTP/1.1, browser `Sec-Fetch` headers) gets
  HTTP 403 from `www.we-online.com`, `www.littelfuse.com` and `www.te.com`,
  no answer from `www.eaton.com`, and 403 from `www.schurter.com` and
  `belfuse.com` without a user agent. `urllib.request` with a Safari
  `User-Agent`, `Accept` and `Accept-Language` gets the PDF from all
  thirteen maker sites (`urlcheck.py`: 1745 PDFs); `www.littelfuse.com`
  answers 403 to the same request without `Accept` and `Accept-Language`.
- **TE's document URL needs a part context.** `…DDEController?Action=srchrtrv&DocNm=V23076-X0000-A001&DocType=Data%20Sheet&DocLang=English&DocFormat=pdf&PartCntxt=1393277-4`
  → PDF (238898 bytes); the same without `&PartCntxt=…` → no PDF.
  `common.resolve_urls` keeps one full URL per `DocNm` (the first in
  alphabetical order), so 245 TE rows share 93 URLs.
- **Panasonic's file names survive in DigiKey's copies.** 638 of the 940
  listed Panasonic rows link `mm.digikey.com`; a file named
  `mech_eng_<series>.pdf` or `semi_eng_<series>.pdf` is on the maker's CDN at
  `https://industry.panasonic.com/ac/cdn/e/control/relay/<dir>/catalog/<name>`,
  `<dir>` one of `signal`, `power`, `photomos`, `vehicle`, `interception`
  (`mech_eng_tq.pdf` → `signal`, `mech_eng_lq.pdf` → `power`,
  `semi_eng_ge1a_aqy21_e.pdf` → `photomos`, `mech_eng_cb.pdf` → `vehicle`,
  `mech_eng_hev.pdf` → `interception`: all PDFs). `na.industrial.panasonic.com/file-download/…`,
  `api.pim.na.industrial.panasonic.com/…` and `www3.panasonic.biz/ac/e_download/…`
  answer HTML. 87 of 93 Panasonic URLs answer.
- **The ordering information is in the PDFs**: each maker's datasheet has a
  part-numbering or ordering table ("How to Order", "Ordering Information",
  "Part Numbering System", "Product Code"); B and C write one regex per
  (datasheet, series), as plan-40-B did.

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` (in C) only when all of these hold; any other outcome is
`out` or `skip` with the reason in the `reason` column.

1. **In stock** (quantity above zero) and **status Active or Last Time Buy**
   at DigiKey. `list.js` asks for statuses 0 and 4 with
   `MinimumQuantityAvailable` 1, so Obsolete, Discontinued, Not For New
   Designs and made-to-order parts are never listed. Every download-set row
   is checked part by part (§ 5).
2. **Maker**: one of the type's three (`common.MAKERS`).
3. **Scope** (`common.classify`), with the class it gives:
   - inductor: `smd` or `tht`; out: a "4 Leads" package (`more than two
     terminals`);
   - fuse: `smd`, `tht` (radial, axial, and cartridges with axial leads) or
     `cartridge` (5 × 20 mm and 6.3 × 32 mm, Requires Holder);
   - PTC: polymeric, `smd` or `tht`; out: `4-SMD` packages (`more than two
     terminals`);
   - relay: `signal`, `power`, `reed`, `ssr` or `auto`, by DigiKey category;
     out: a termination that does not solder to a board (wire leads, plug-in,
     socketable, solder hook: `termination`), an AC coil or none given (`coil
     voltage`), no contact form (`contact form`), a solid-state `Module`;
   - buzzer: `indicator` (Driver Circuitry "Indicator…") or `transducer`;
     out: wire leads (`termination`).
4. **At most one of the same thing** (`common.dedupe_key`): rows of one
   maker, type and series that are equal in every DigiKey parameter differ
   only in packing; the key keeps its most-stocked part, the rest →
   `duplicate`.
5. **A datasheet on the maker's site** (`select.py --urls`): the row's
   resolved URL (§ 4) answers a PDF in `url-check.tsv`; else `no datasheet`.
6. **Each maker gets 200 rows of a type** (`common.QUOTA`), by class:
   inductor `smd` 140, `tht` 60; fuse `smd` 70, `tht` 60, `cartridge` 70; PTC
   `smd` 110, `tht` 90; relay `power` 70, `signal` 60, `ssr` 40, `auto` 15,
   `reed` 15; buzzer `indicator` 100, `transducer` 100. A class short of rows
   hands its unused quota to the other classes in that order. Inside a class
   the rows go round-robin over the series (each series' most-stocked row
   first, then each series' second, …, the series with more stock first in a
   round); the first quota rows are `candidate`, the next quarter of the
   class `reserve`, the rest `below quota`.
7. **A maker datasheet** (Phase 3; B's and C's verdict): the PDF downloads
   (§ 4). Else the row is `skip`, reason `no datasheet on the maker's site`,
   and goes to `datasheets/skipped-parts.md`.
8. **In the datasheet's ordering table** (B, C): printed verbatim, or a full
   match of the (datasheet, series) ordering-code regex in `specs.tsv`; else
   `skip`, reason `not in the datasheet's ordering table`.
9. **Not marked obsolete, discontinued, NRND or end of life by the
   datasheet** (B, C read every `dead` = 1 row); a PDF whose every row is
   `out` for datasheet status is deleted and listed in
   `datasheets/obsolete-not-included.md`.

Rules 7–9 apply to every download-set row, so C's final pick can fill a
skipped slot with the next row of the same (type, maker, class).

## 4. Datasheet placement

`census/place.py` (plan-40's, adapted) copies each needed PDF from
`/tmp/p47/pdf/<md5 of the URL>.pdf` to
`datasheets/<family>/<maker-slug>/<name>.pdf`: `<family>` `inductor`, `fuse`
(fuses, PTCs and fuse clips), `relay` or `buzzer`; `<maker-slug>` the
`company` column (`bourns`, `murata`, `wurth`, `littelfuse`, `eaton`,
`schurter`, `belfuse`, `te`, `franjobaim`, `panasonic`, `samesky`, `pui`,
`mallory`); `<name>` the URL path's last segment (query string dropped,
`%20` → `-`, `.pdf` appended when missing, characters outside `A-Za-z0-9._-`
replaced by `-`), for a TE URL its `DocNm`; a clash gets `-<first 6 hex of
the md5>`. It writes `datasheets/<family>/datasheets.tsv`: path, URL, bytes,
SHA-256. Files with the same SHA-256 under two URLs share one file. A PDF no
download-set row uses is not copied.

The URL of a row is `url` of `candidates.tsv` (`common.resolve_urls`), and
`via` says how it was found:

1. `link`: DigiKey's link, on the maker's own site, normalized
   (`common.own_url`): Bourns `https://www.bourns.com<path>` (query dropped);
   Murata `pim.murata.com` and `www.murata.com` as given; Würth
   `www.we-online.com` as given, `katalog.we-online.de` and
   `www.we-online.de` → `https://www.we-online.com/components/products/datasheet/<file>`;
   Littelfuse `www.littelfuse.com` as given (the `assetguid` query is part of
   it); Eaton `www.eaton.com` (and `.com.cn`, same path); SCHURTER
   `us.schurter.com` → `www.schurter.com`; Bel Fuse `belfuse.com` →
   `www.belfuse.com`; TE one URL per `DocNm` (§ 2); Franjobaim
   `assets.franjobaim.com`; Panasonic the CDN rule of § 2 (the row's class
   picks the first directory, `urlcheck.py` tries the other four and records
   the one that answered in `final`); Same Sky `www.cuidevices.com` →
   `www.sameskydevices.com`; PUI `puiaudio.com`, `api.puiaudio.com`; Mallory
   `mspindy.com`.
2. `guess`: a maker that publishes one PDF per part or series, from the part
   number: Würth `…/components/products/datasheet/<mpn>.pdf`, PUI
   `https://puiaudio.com/file/specs-<mpn>.pdf`, Mallory
   `https://mspindy.com/spec-sheets/<mpn>.pdf`, Same Sky
   `…/product/resource/<mpn lower case>.pdf`, Bourns inductors
   `https://www.bourns.com/docs/Product-Datasheets/<series>.pdf`, Panasonic
   `mech_eng_<series code>.pdf` (a series DigiKey names by its part-number
   prefix drops the leading A: `ADJ` → `mech_eng_dj.pdf`).
3. `sibling`: the commonest maker-site URL among the listed rows of the same
   (maker, type, series).
4. Anything else (a Waldom, DigiKey or missing link with no sibling and no
   guess) → no URL → `no datasheet`.

Downloader rules (`census/fetch.py`, § 7): Python `urllib` with the three
headers of § 2, 8 at a time, one retry, a `%PDF-` check and `pdfinfo` on the
body; a file already in `/tmp/p47/pdf` from `urlcheck.py` is reused. A
failed fetch makes the rows of that URL `out` (reason `no datasheet`),
recorded in § Corrections.

## 5. The 300-result cap: the part-by-part stock check

DigiKey returns at most the first 300 results of a search, and the planning
listing hit that cap in 19 of its 36 pieces (§ 2); 2146 of the 3522
download-set rows come from a cut piece. A listing is also a snapshot.
Every row of the download set is therefore looked up by its own part number
before B or C may use it. `census/stock-check.sh` runs
`node scripts/digikey.js --manufacturer <maker id> --in-stock --status 0
--status 4 --limit 5 --json "<mpn>"` (one call; the maker id is the row's
own DigiKey id from the raw listing, so no look-up calls) and writes
`planning/plan-47-data/stock-check.tsv` (`mpn`, `found` 0/1, `stock`,
`status`, `date`); `found` = 1 when a product with exactly that
`ManufacturerProductNumber` has `QuantityAvailable` above 0. The script
skips part numbers already in the file and stops cleanly on HTTP 429, so a
run cut by the quota goes on the next day. `select.py --stock stock-check.tsv
--keep candidates.tsv` makes a `found` = 0 row `out`, reason `stock check`,
sets `stock_checked`, and refills the class from the next rows.

3522 rows + 6 clips = 3528 calls: four quota days at 900 calls a day (the
quota is 1000 a day and shared).

## 6. The cartridge-fuse clips

A cartridge fuse needs a holder; its footprint is a pair of PCB fuse clips
(plan-47-E draws it). Per (fuse maker, fuse diameter) the clip is that
maker's most-stocked in-stock through-hole clip (`clips/clips.jsonl`, sorted
by stock), each printed in its datasheet (`pdftotext` on the downloaded
file, 2026-11-16):

| Maker | Fuse | Clip | Stock | Datasheet (maker site) |
|---|---|---|---|---|
| Littelfuse | 5 × 20 mm | `01110501Z` (111 series) | 274257 | `https://www.littelfuse.com/assetdocs/littelfuse-fuse-clip-111-datasheet?assetguid=B1CA8750-5406-48D2-B921-B86728E90ADF` |
| Littelfuse | 6.3 × 32 mm | `01020071Z` (102 series) | 73290 | `https://www.littelfuse.com/assetdocs/102-bt-series-datasheet?assetguid=896A24D8-F3D9-44D3-91AA-0A751A3C9FC6` |
| Eaton | 5 × 20 mm | `BK/1A5018-10-R` | 47538 | `https://www.eaton.com/content/dam/eaton/products/electronic-components/resources/data-sheet/eaton-1axxxx-pcb-fuse-clips-atc-atm-blade-data-sheet.pdf` |
| Eaton | 6.3 × 32 mm | `BK/1A1907-06-R` | 47284 | the same PDF |
| SCHURTER | 5 × 20 mm | `0751.0052` (OG) | 28110 | `https://www.schurter.com/en/datasheet/typ_OG__Clip__5x20.pdf` |
| SCHURTER | 6.3 × 32 mm | `0751.0056` (OG) | 18395 | `https://www.schurter.com/en/datasheet/typ_OG__Clip__5x20___6.3x32.pdf` |

The five PDFs go to `datasheets/fuse/<maker>/` with the rest; the clips are
stock-checked in Phase 2. A clip is not a library part: it is named in the
footprint's `note` and in the cartridge fuse devices' `note` (plan-47-K).

## 7. Scripts

- Written at planning time (`README.md`): `census/dk.js`, `count.js`,
  `list.js`, `list.sh`, `common.py`, `urls.py`, `urlcheck.py`, `select.py`
  (with `--urls`, `--stock`, `--keep`), `footprint-lines.py`.
- Written in this letter, adapted from `planning/plan-40-data/census/`:
  `stock-check.sh` (§ 5), `fetch.py` (§ 4: `urlcheck.py`'s `fetch_one` over
  the download set's URLs and the clip URLs; plan-40's `fetch.js` is not used
  because Node's `fetch` was not tested against these sites), `textscan.py`,
  `sibling.py`, `place.py` (§ 4; `--verify` and, for C, `--prune`),
  `specs-skeleton.py` (the four families, the thirteen maker slugs, the
  columns of this plan's `candidates.tsv`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Restore the listing

- [ ] Run `python3 -I planning/plan-47-data/census/select.py planning/plan-47-data/raw /tmp/p47-cand.tsv --urls planning/plan-47-data/url-check.tsv --report`:
      `8075 parts listed`, `candidates: 2953; reserve: 569`, `download set:
      3522 rows; 1461 distinct URLs`; `cmp` with `candidates.tsv` →
      `reproducible`.

Acceptance: the planning pick reproduces.
  Check: `python3 -I planning/plan-47-data/census/select.py planning/plan-47-data/raw /tmp/p47-cand.tsv --urls planning/plan-47-data/url-check.tsv && cmp /tmp/p47-cand.tsv planning/plan-47-data/candidates.tsv && echo reproducible` → `reproducible` (est. < 1 min).
Commit: —

### Phase 2 — The part-by-part stock check (four quota days)

- [ ] `census/stock-check.sh` (§ 5); run it for every row of the download
      set and the six clips of § 6 (3528 calls; ≥ 900 left before each day's
      run, § Prerequisites); a run cut by the quota goes on the next day.
- [ ] `select.py … --stock planning/plan-47-data/stock-check.tsv --keep
      planning/plan-47-data/candidates.tsv --report` writes `candidates.tsv`;
      a row that went `out` leaves the set and the next row of its (type,
      maker, class) enters it; check every new row the same way until no row
      of the set is unchecked. Record the number of `found` = 0 rows and of
      extra calls in § Corrections. A clip that fails is replaced by the
      next clip of its maker and diameter in `clips/clips.jsonl` and § 6's
      table is corrected.

Acceptance: every download-set row and every clip is in stock, Active or Last Time Buy.
  Check: `awk -F'\t' 'NR>1 && $2!=1' planning/plan-47-data/stock-check.tsv \| wc -l` → the `found` = 0 count recorded above, and `awk -F'\t' 'NR>1 && ($23=="candidate" \|\| $23=="reserve") && $29==""' planning/plan-47-data/candidates.tsv \| wc -l` → 0 (est. < 1 min; the run itself is the four quota days, about 40 min a day at 23 calls a minute).
Commit: —

### Phase 3 — Download, place and scan the datasheets

- [ ] `census/fetch.py`, `place.py`, `textscan.py`, `sibling.py` (§ 4, § 7):
      download every distinct URL the download set cites (1461 at planning)
      and the five clip PDFs into `/tmp/p47/pdf`; `%PDF-` and `pdfinfo` on
      each; failures recorded in § Corrections with their host.
- [ ] Place the PDFs under `datasheets/<family>/<maker-slug>/` and write each
      family's `datasheets.tsv`; `place.py --verify` → `0 missing, 0
      unused`. The PDFs stay out of the commits until C prunes them (C Phase
      4), so the history never carries PDFs no `add` row uses.
- [ ] `candidates.tsv`: `pdf` (the placed path) for every download-set row
      whose URL gave a PDF, and `hit` = 1 when its part number is printed
      verbatim (`pdftotext -layout`, token-boundary search); `dead` = 1 when
      the text near the part number says obsolete, discontinued, NRND, "not
      recommended for new designs" or end of life (B and C read every such
      row).
- [ ] `census/specs-skeleton.py`: `planning/plan-47-data/specs.tsv`, one row
      per (type, maker, series, PDF, package) of the download set, columns
      `type`, `maker`, `series`, `pdf`, `package`, `rows`, `example`,
      `scheme`, `label`, `page` (B's and C's), `pinout`, `terminals` (B's and
      C's).
- [ ] Record `du -sh datasheets/inductor datasheets/fuse datasheets/relay
      datasheets/buzzer` here.

Acceptance: every download-set row either has a PDF or a reason it has none.
  Check: `awk -F'\t' 'NR>1 && ($23=="candidate" \|\| $23=="reserve") && $26==""' planning/plan-47-data/candidates.tsv \| wc -l` → the rows B and C will `skip` for no maker datasheet (recorded here); for each family `find datasheets/<family> -iname '*.pdf' \| wc -l` = `tail -n +2 datasheets/<family>/datasheets.tsv \| cut -f1 \| sort -u \| wc -l` (est. 30 min: the PDFs `urlcheck.py` already fetched are reused; a cold cache is 1466 downloads, about 45 min).
Commit: —

## Validation Plan

- Tests: none in this letter (data only); `select.py` is checked by its
  `--report` outputs above.
- Coverage check: the three Acceptance checks cover the listing, the stock
  check and the downloads.
- Runtime proof: n/a.
- Doc sync: none (`datasheets/skipped-parts.md` and
  `datasheets/obsolete-not-included.md` are C's).
- Final gate: plan-47-O.

## Decisions

Owner decisions (asked once, 2026-11-16, before this plan was written):

- **Sub-types in scope**, beyond the core (two-terminal fixed inductors;
  board-mount fuses and polymeric PTC resettable fuses; board-mount
  electromechanical relays; board-mount buzzers with or without a driver):
  cartridge fuses on PCB clips; reed and solid-state relays; latching relays
  and relays of three and four poles; AEC-Q200 automotive and military
  grades. Out: panel, chassis and DIN-rail parts, sirens, common-mode
  chokes, transformers, circuit breakers, contactors.
- **Makers: the top three per type, fuses and PTC resettable fuses ranked
  separately** (five rankings), by in-stock count of the in-scope parts, the
  DigiKey maker ids of one company summed.
- **Size: 200 parts per maker per type** (up to 3000).
- **Extras:** a relay switches in play mode; a buzzer sounds in play mode; a
  fuse can be blown. No generic parts (`GENERIC-INDUCTOR` and the like were
  offered and not chosen).

Author decisions (no user-visible fork; recorded so nobody re-litigates):

- **The ranking counts board-mount parts** (surface mount and through hole;
  fuses also the two cartridge sizes on a holder): the panel-mount leaders
  (Floyd Bell, 1100 alarms) have no part the library can place.
- **One relay ranking over the five relay categories**: the owner's relay
  scope spans them; TE's four DigiKey maker ids are one company.
- **Franjobaim is in** as the second relay maker by count (1152 against
  Panasonic's 1146 and Aratas + Omron's 1083); its datasheets are on its own
  site (`assets.franjobaim.com`, 11 URLs, all PDFs). Aratas (formerly Omron
  Components) and Omron Automation and Safety are summed and still fourth.
- **The listing takes each piece's 300 most-stocked parts** instead of
  every part (43449 inductors alone): the pick is by stock, 171 calls
  listed 8075 parts, and the cap is declared in § 2 and answered by § 5.
- **Class quotas** (rule 6) so each maker shows its through-hole, cartridge,
  solid-state and automotive parts and not only its most-stocked chips; the
  series round-robin so no series takes more than its turn.
- **Polymeric PTCs only**: "polyfuse" names the polymeric kind; ceramic PTC
  thermistors are sold in the same category for other uses.
- **DC coils only, and only terminations that solder to a board**: the logic
  simulation drives a coil from a DC level, and a plug-in relay's footprint
  is its socket's.
- **Buzzer Elements, Piezo Benders (160) are out**: bare discs with wire
  leads, no enclosure and no board footprint.
- **Mallory's share stays at 153**: it has no more in-scope parts with a
  datasheet; the shortfall is not filled from a fourth maker.
- **The stock check covers the whole download set** (3522), not only the
  rows from cut pieces (2146): the owner asked for the part-by-part re-check
  whenever the cap was hit, and checking the reserve rows now keeps B and C
  free of DigiKey calls.
- **C refills only from the download set.** A share that is still short
  after the verdicts stays short and is recorded with its count in C §
  Corrections (no second listing).
- **Datasheets live in `datasheets/{inductor,fuse,relay,buzzer}/<maker>/`**;
  PTCs and the clip datasheets share `fuse/`.
- **A cartridge fuse's footprint is its maker's most-stocked through-hole
  clip pair** (§ 6); the clip is not a library part and is not in the Bill
  of Materials.

## Corrections

## Summary

A is the plan's quota-bound letter: four days of DigiKey calls (3528
part-by-part look-ups) and 1466 downloads through Python's `urllib`, the one
client all thirteen maker sites answer. Its risk is wrong data feeding B and
C (a stale stock figure, a PDF that is not the part's, a guessed URL that
serves another series): the stock check, the `%PDF-` check and the verbatim
part-number scan are there to catch it. Out of scope for the whole plan:
generic parts; panel-, chassis- and DIN-rail-mount parts; sirens and bare
piezo elements; common-mode chokes, coupled inductors, transformers and
adjustable inductors; ceramic PTCs, automotive blade fuses, circuit breakers
and thermal cutoffs; AC-coil, plug-in and socketed relays, contactors and
time-delay relays; makers outside each type's top three (TDK, Coilcraft,
Vishay, Bel Fuse fuses, Omron, CIT, DB Products, …); every analog behavior
(inductance, current, a fuse that blows by itself, coil voltage thresholds,
sound pressure). Nothing in the tree except `planning/plan-47-data/` and
the four `datasheets/` families changes in A.
