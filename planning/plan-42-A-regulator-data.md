# plan-42-A: Voltage regulators — part-by-part stock check, datasheet downloads

Last updated: 2026-11-16
Overall Effort: huge (> 3d) — plan-42 is A → B → C → D → E → F → G → H → I → J
Effort: large (3h–1d; plus one DigiKey quota day)
Depends on: nothing (§ Prerequisites)

plan-42 adds voltage regulators to the parts library and the app, from the
top three makers at DigiKey: **Texas Instruments** (with National
Semiconductor and Burr Brown), **Analog Devices** (with Maxim Integrated)
and **Microchip Technology** (with Micrel). Scope: positive fixed-output
linear regulators and LDOs, positive adjustable linear regulators,
dual / triple / quad linear regulators, and switching regulators with an
internal switch (buck, boost, buck-boost, charge pump, SEPIC) with a fixed
or adjustable positive output; plus three generic parts (`GENERIC-REG-5V`,
`GENERIC-REG-3V3`, `GENERIC-REG-ADJ`). They sit in the parts drawer under
**Power → Regulators → Linear / Switching → <type>**. In play mode a
regulator is a regulated supply: its output drives 1 while its input reads
1 and its enable is active; in the voltage check its output net is at the
part's output voltage — for an adjustable part worked out from the two
feedback resistors on the board — and an input above the part's maximum is
refused (plan-42-G).

The single behavioral outcome of plan-42: loading the part files gives the
baseline P0 parts (§ Prerequisites) plus the three generics plus one
`regulator` part per `add` row of `planning/plan-42-data/candidates.tsv`
(600 picked at planning time: linear fixed 240, linear adjustable 90, linear
multi-output 30, switching 240; 200 per maker); the app lists them under the
drawer paths above; a placed `GENERIC-REG-3V3` is `U1` on `TO-220-3`; a 5 V
supply through an LDO powers a 3.3 V-only part with no refusal; `ctest`
passes T0 + 1 programs (`regulator_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | part-by-part stock check of the download set, datasheet downloads and placement, the verbatim scan, `specs.tsv` skeleton | large |
| B | ordering-code schemes per datasheet, datasheet status, verdicts, the final pick, skipped parts, obsolete PDFs, the final `footprints-needed.txt` | large |
| C | pinouts and regulator keys per (series, package): pin roles, `in` / `out` / `enable` / `pg` / `adjust`, Vref, Vin min, dropout | large |
| D | footprints: leaded packages (TO-220, TO-263, DPAK, SOT-223, SOT-89, SOT-23, SC-70, SOIC, MSOP, TSSOP, HTSSOP, …) | medium |
| E | footprints: leadless and chip-scale packages, Texas Instruments and Microchip | large |
| F | footprints: leadless and chip-scale packages, Analog Devices | large |
| G | backend: the `regulator` kind, the model, the regulated level, the divider, the VIN rating, `circuit_set_resistance`, `regulator_test`, docs | large |
| H | parts: the generator, the three generics, every `add` row, goldens, the editor's kind list | large |
| I | app: drawer, sidebar facts, `kindName`, the resistances to the voltage check and play mode, the message, the harness | large |
| J | landing tier | small |

Outcome of A: `planning/plan-42-data/stock-check.tsv` confirms every row of
the download set in stock, Active or Last Time Buy, part by part (DigiKey's
listing hit the 300-result cap in 7 pieces, § 2); `datasheets/regulator/<maker>/`
holds every maker-site PDF a download-set row cites, with
`datasheets/regulator/datasheets.tsv`; `candidates.tsv` has `pdf` and `hit`
filled; `specs.tsv` has one row per (maker, series, PDF, Package / Case), its
scheme and pinout columns empty (B and C fill them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-42-data/` — `README.md`, `census/top.js`, `list.sh`,
  `named.sh`, `recheck.sh`, `select.py`, `download-set.py`,
  `footprint-lines.py`; `raw/*.jsonl.gz` and `raw/*.err` (the planning
  listing), `candidates.tsv`, `download-set.tsv`, `footprints-needed.txt`
  (written at planning time, 2026-11-15).
- `planning/completed/plan-40-A-transistor-data.md` and
  `planning/plan-40-data/census/` (`stock-check.sh`, `fetch.js`,
  `textscan.py`, `sibling.py`, `place.py`, `specs-skeleton.py`, `common.py`,
  `urls.py`: the scripts A adapts; the same "most-stocked per piece, re-check
  part by part" method).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p42 && cmake --build /tmp/logic-p42 -j8 && ctest --test-dir /tmp/logic-p42 -j8` → `100% tests passed` | MET (2026-11-21 at cd9ca2a8: `100% tests passed, 0 tests failed out of 39`; T0 = **39** now, `grep -c '^logic_test' backend/CMakeLists.txt` → 39; plan text elsewhere says 36: add 3) |
| Golden sizes recorded | `git show HEAD:backend/tests/golden/parts.txt \| wc -l`; `traces.txt`; `footprints.txt` | at planning (40ccd5fc): **P0 = 32854**, **R0 = 32854**, **F0 = 1624**; re-measured 2026-11-16 at 6eb14f28 (plan-43 landed since): **P0 = 32897**, **R0 = 32897**, **F0 = 1646**; re-measured 2026-11-21 at cd9ca2a8 (`wc -l backend/tests/golden/{parts,traces,footprints}.txt`): **P0 = 33042**, **R0 = 33042**, **F0 = 1685** — use these |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p42-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-21 at cd9ca2a8: `** BUILD SUCCEEDED **`) |
| No regulator kind yet | `grep -c 'PART_REGULATOR' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/regulator.json` → no such file; `ls datasheets/regulator` → no such directory | MET (2026-11-21 at cd9ca2a8: 0, no file, no directory) |
| No other open plan adds regulators | `grep -l -i 'PART_REGULATOR\|Parts/regulator\.json\|datasheets/regulator' planning/plan-*.md \| grep -v plan-42 \| wc -l` → 0 | MET (2026-11-21: the command prints 5, plan-44-A, 44-E, 47-A, 47-I, 48-A, but each only names plan-42 / `PART_REGULATOR` as an open plan whose kind order it follows — `grep -n` of those five files shows no regulator part, kind or datasheet directory added; none adds regulators. Command corrected below). Other open plans add part kinds: plan-44 (`PART_CRYSTAL`) and plan-46 (`PART_GENERIC`); every shared line (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists) is appended after whatever kind is last, each step greps first |
| plan-46 (Part Wizard) complete | `ls planning/completed/plan-46-N-*.md` → exists | MET (2026-11-21: `planning/completed/plan-46-N-wizard-landing.md` exists; plan-46-A … N are all in `planning/completed/`). Earlier status NOT MET (2026-11-16: plan-46 written, not started). Owner decision 2026-11-16: plan-46 lands before plan-44 and plan-42. It appends `PART_GENERIC`, replaces `PartKinds.displayNames` with `Editor/KindInfo.swift`, and gives every kind an editor schema (no `formless` set); plan-42-G/H/I are written against that world |
| DigiKey credentials and quota | `grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc` → 2; `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)" && node scripts/digikey.js --in-stock --pages 1 --limit 1 "LM1117"` prints `calls left today` ≥ 920 before Phase 1 (the quota is about 1000 a day, shared with other sessions; it resets at about 00:00 UTC) | MET (2026-11-21: credentials 2 of 2; `node scripts/digikey.js --in-stock --pages 1 --limit 1 "LM1117"` → `999 left today`; the quota is shared: 429 were left when the run was half done, 42 at the end). Earlier status NOT MET (2026-11-16 03:36 UTC: credentials 2 of 2; `node scripts/digikey.js …LM1117` → `898 left today`, below 920; the quota reset at 00:00 UTC and other sessions have already used about 100 calls; the next reset is 2026-11-17 00:00 UTC) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-42-data/stock-check.tsv` has one row per row of the
  download set (900 at planning time, `download-set.py --report` → `900
  rows`), each `found` 1 with stock above 0 and status Active or Last Time
  Buy; a row that fails is `out` (reason `stock check`) in `candidates.tsv`.
- `datasheets/regulator/<maker>/` holds each maker-site PDF that at least one
  download-set row needs; `datasheets/regulator/datasheets.tsv` maps each
  file to its URL, size and SHA-256.
- `candidates.tsv` has `pdf` and `hit` filled for every download-set row;
  `specs.tsv` exists with one row per (maker, series, PDF, DigiKey Package /
  Case) of those rows, its scheme and pinout columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter and no row is `add` yet: the verdicts and
  the final pick are B's.
- No datasheet hunting beyond the DigiKey link, the rewrites of § 4 rules 3
  and 4, and the same-maker sibling check.
- No new listing: the planning listing is complete for the pick (§ 2,
  `NEEDS DEEPER LISTING` 0). `scripts/digikey.js` is not changed.

## 2. Current State

The library has no regulator kind and no regulator part. The app's Settings
have no regulator reference category; a regulator will take `U`
(`Preferences/ReferencePrefixes.swift`, `ReferenceCategory.of(_:)` falls to
`.integratedCircuit`). `datasheets/` has one directory per family and no
`regulator/`; `datasheets/skipped-parts.md` and
`datasheets/obsolete-not-included.md` take the skips. `planning/todo.md:64`
asks for "78xx, AMS1117, LM317, LDOs, and common buck modules".

### Measured populations

All DigiKey numbers 2026-11-15, in stock (`--in-stock`), statuses Active and
Last Time Buy. `dk.sh` is § 6's wrapper around `node scripts/digikey.js`.
Planning used 326 calls (381 left before the first, 56 after the last).

| What | Count | Command |
|---|---|---|
| In stock, Active + LTB, linear (699) / switching (739) | **16503 / 10091** | `dk.sh --category 699 --in-stock --status 0 --status 4 --pages 1 --limit 1 --json` (and 739) → `ProductsCount` |
| …positive output (Output Configuration = Positive) | 699: 15548 (Negative 466, others 34); 739: 8927 | same calls, `ParametricFilters` 255 |
| …699 by Output Type | Fixed 13490, Adjustable 2210, Adjustable (Fixed) 261, other 54 | same, facet 41 |
| …699 by Number of Regulators | 1: 15158, 2: 646, 3: 18, 4: 4 | same, facet 1065 |
| …739 by Number of Outputs | 1: 9183, 2: 555, 3: 82, 4: 81, 8: 16 | same, facet 87 |
| Makers ranked, 699 + 739, a maker with the makers it bought | **Texas Instruments 8757** (TI 4749 + 3261, National 511 + 229, Burr Brown 6 + 1); **Analog Devices 5366** (ADI 1724 + 2483, Maxim 281 + 878); **Microchip 2313** (Microchip 1539 + 330, Micrel 423 + 21); onsemi 1961 (onsemi 1607 + 185, Fairchild 70 + 99); Monolithic Power Systems 690 (58 + 632) | same two calls, `Manufacturers` facet |
| Planning listing (`list.sh` 14 pieces, `named.sh` 13, `recheck.sh` 2) | **6225 parts** listed; **201 + 15 + 102 calls** | `python3 -I census/select.py /tmp/p42/raw /tmp/p42/cand.tsv --report` → `6225 parts listed`; the `top.js:` lines of `raw/*.err` |
| Outcome of `select.py` | **600 picked**; out: duplicate 1353, below quota 3301, automotive 485, output-type 344, chassis 118, tracking 17, no-stock 6, too-many-outputs 1 | `select.py --report` → `reason:` |
| …picked per family and maker | linear-fixed 80 / 80 / 80, linear-adjustable 30 / 30 / 30, linear-multi 10 / 10 / 10, switching 80 / 80 / 80 (TI / ADI / Microchip) | `select.py --report` → per-family lines |
| …status | Active 598, Last Time Buy 2 (`MCP1253T-ADJI/MS`, `MCP1603T-180I/OS`) | `awk -F'\t' '$29=="candidate"{print $7}' planning/plan-42-data/candidates.tsv \| sort \| uniq -c` |
| …switching topologies | Buck 164, Boost 55, Buck-Boost 10, "Boost, Flyback, SEPIC" 6, Charge Pump 2, 3 others 1 each | `awk -F'\t' '$29=="candidate" && $4=="switching"{print $22}' … \| sort \| uniq -c` |
| …multi-output | linear 2 outputs 29, 3 outputs 1; switching 2 outputs 13, 3 outputs 2 | `awk -F'\t' '$29=="candidate" && $11>1 {print $4, $11}' … \| sort \| uniq -c` |
| …Output Type | Fixed 361, Adjustable 234, Adjustable (Fixed) 5 | `awk -F'\t' '$29=="candidate"{print $10}' … \| sort \| uniq -c` |
| …mounting | Surface Mount 577, Through Hole 23 | same, `$14` |
| …owner-named bases picked (at most 6 each, § 3 rule 4) | LM1117 6, TLV1117 6, LM317 6, LM340 6, LM2576 6, LM2596 6, MCP1700 6, MCP1702 6, LM7805 2, UA7805 2, LM7812 2, UA7812 2; LM7833 none in stock | `select.py --report` → `named bases picked`; `dk.sh --manufacturer 296 --in-stock --category 699 "LM7833"` → `0 of 0 parts` |
| **Pieces cut at the 300-result cap** | **7** in the base listing (699-296 Fixed and Adjustable, 699-505 Fixed, 699-150 Fixed, 739-296 Fixed and Adjustable, 739-505 Adjustable); 2 of them were below the floor (`NEEDS DEEPER LISTING`: 699-296 Fixed, 300th stock 37170 ≥ floor 35427; 699-505 Fixed, 2542 ≥ 2334) and were re-listed by `recheck.sh` with `MinimumQuantityAvailable` = the floor; after that 4 cut pieces remain, every one complete for the pick, 0 `NEEDS DEEPER LISTING` | `select.py --report` → `cut piece` lines, `4 pieces cut at 300` |
| Picked rows that came from a cut piece | **158** of 600 | `select.py --report` → `candidates from a cut piece: 158 of 600` |
| Refused pieces | 2, both empty of pickable parts: 739-600062 (Burr Brown: 1 part, `TPS62092RGTT`, stock 250, far below the switching floor) and the named `LM7833` (no in-stock part) | `select.py --report` → `failed piece` lines; `dk.sh --category 739 --manufacturer 600062 --in-stock` → `1 of 1` |
| **The download set** (picked rows, then the next rows by stock up to 1.5 × each (family, maker) share) | **900 rows**, **541 distinct datasheet URLs** | `python3 -I census/download-set.py candidates.tsv --report` → `900 rows; 541 distinct URLs` |
| …datasheet hosts | www.analog.com 297, www.ti.com 265, ww1.microchip.com 175, rocelec.widen.net (Rochester copies) 141, mm.digikey.com 16, datasheets.maximintegrated.com 3, www.microchip.com 2, none 1 | same, `hosts:` |
| …rows with only a Rochester / DigiKey copy or none | Microchip 123, Texas Instruments 35, Analog Devices 0 | `awk -F'\t' 'NR>1 && ($6 ~ /rocelec\|mm.digikey/ \|\| $6=="") {print $2}' planning/plan-42-data/download-set.tsv \| sort \| uniq -c` |
| Distinct datasheet URLs, picked rows | 391; (URL, Package / Case) pairs 444 | `awk -F'\t' '$29=="candidate"{print $28}' candidates.tsv \| sort -u \| wc -l`; with `$12` |
| Footprint lines (letter, maker, package, supplier package), picked rows | 180: D 69 (329 parts; 28 with no existing footprint of the same outline), E 63 (169 parts; none existing), F 48 (102 parts; none existing); distinct DigiKey packages D 45, E 51, F 33 | `awk -F'\t' '!/^#/{n[$1]++; p[$1]+=$7; if($10=="")g[$1]++} END{for(k in n) print k, n[k], p[k], g[k]+0}' planning/plan-42-data/footprints-needed.txt` |

### Verified properties

- **Sorting by stock works with the Output Configuration filter.**
  `top.js` sends `ParameterFilters` [255 = Positive, …] with `SortOptions
  {Field: "QuantityAvailable", SortOrder: "Descending"}`; `raw/699-296.err`
  shows `all: 300 of 4624 (cut at 300: 40500)`, the type pieces split below
  it, and the parts arrive in falling stock. So the first 300 of a piece are
  its 300 most-stocked.
- **A minimum stock makes a deeper listing cheap and complete for the pick.**
  `recheck.sh` lists a piece with `MinimumQuantityAvailable` = the pick's
  floor: a part below the floor can never be picked (the floor only rises
  as rows are added), so the re-listing holds every part that can. Result:
  699-296 334 parts (52 calls), 699-505 500 parts (50 calls); `select.py
  --report` then prints no `NEEDS DEEPER LISTING`.
- **DigiKey refuses a filtered search that matches nothing.** A piece with
  no matching part answers HTTP 400 "Parametric Filtering Not Support at
  Parent Category Level" (`raw/739-600062.err`, `raw/699-296-named-LM7833.err`);
  both were checked without the filter (§ 2 table).
- **Downloads.** `curl -sL https://www.ti.com/lit/ds/symlink/lm1117.pdf` →
  200, `application/pdf`, 2920491 bytes; `…/symlink/lp8340.pdf` → 200,
  895629 bytes (a Rochester-linked TI row). `ww1.microchip.com` answers
  `curl` with the PDF (`MCP1700-…-20001826E.pdf` → 200, 1297219 bytes).
  `www.analog.com` breaks `curl`'s HTTP/2 stream (`curl: (92) HTTP/2 stream
  1 was not closed cleanly`) and answers Node's `fetch` with a Safari user
  agent (`ADP7118.pdf` → 200, `application/pdf`, 2042809 bytes;
  `MAX38902A-MAX38902D.pdf` → 200, 1109475 bytes): the downloader is a Node
  script, as plan-40's `fetch.js`.
- **Rochester-linked rows have a maker URL.** TI: `https://www.ti.com/lit/ds/symlink/<base
  lowercase, from the first `/`, `-` or `.` that follows the device name
  dropped>.pdf` (`lp8340.pdf`, `lp3874.pdf` → PDFs; `https://www.ti.com/product/LP8340`
  → 200, 297135 bytes, a live product page). Micrel (Microchip):
  `https://ww1.microchip.com/downloads/en/DeviceDoc/<base>.pdf` with Node's
  `fetch` gave a PDF for `MIC5235` (368666 bytes), `MIC5305` (353227),
  `MIC5213` (52108) and HTML for `MIC29203`, `MIC37502`; the product page
  `https://www.microchip.com/en-us/product/<base>` (403 to `curl`, 200 to
  Node's `fetch`) links `ww1.microchip.com/downloads/en/DeviceDoc/mic37501-502.pdf`
  for `MIC37502`; `MIC29203` has no page of that name.
- **Ordering tables are in the PDFs as patterns** (TI's "Package Option
  Addendum", ADI's "Ordering Guide", Microchip's "Product Identification
  System"): B writes one regex per (datasheet, series), as plan-40-B did.

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` (in B) only when all of these hold; any other outcome is
`out` or `skip` with the reason in the `reason` column.

1. **In stock** (quantity above zero) and **status Active or Last Time Buy**
   at DigiKey. `top.js` asks for statuses 0 and 4 with
   `MinimumQuantityAvailable` ≥ 1, so Obsolete, Discontinued, Not For New
   Designs and made-to-order parts are never listed; a row with no quantity
   is `out`, reason `no-stock`. Every download-set row is confirmed part by
   part (§ 5).
2. **Scope** (`select.py`): categories 699 and 739, Output Configuration
   Positive; the three makers (ids in `README.md`); Grade or Qualification
   naming Automotive, AEC-Q, Military or MIL- → `automotive`; Mounting Type
   neither Surface Mount nor Through Hole → `chassis`; Output Type not
   Fixed / Adjustable / Adjustable (Fixed) → `output-type`; a switching
   Function "Ratiometric" (DDR / VTT tracking) → `tracking`; more than 4
   outputs → `too-many-outputs`; a description or parameter naming I2C,
   SPI, PMBus, SMBus or VID → `digital-interface` (none at planning).
3. **At most one of the same thing** (`select.py`): the key is (family, base
   part number, Package / Case, Supplier Device Package, output voltage); a
   key keeps its most-stocked part, the rest → `duplicate`.
4. **Each maker gets its share** (`select.py`): per (family, maker) a third
   of the family's quota (linear-fixed 240, linear-adjustable 90,
   linear-multi 30, switching 240), the owner-named bases first (each
   base's 6 most-stocked parts: `UA7805`, `LM7805`, `LM340`, `LM7812`,
   `UA7812`, `LM7833`, `LM317`, `LM1117`, `TLV1117`, `LM2596`, `LM2576`,
   `MCP1700`, `MCP1702`), then by stock; the rest → `below quota`.
5. **A maker datasheet** (Phase 3; B's verdict): the row's URL is on the
   maker's own site (`ti.com`, `analog.com`, `maximintegrated.com`,
   `microchip.com`) and downloads as a PDF; else the rewrite of § 4 rules 3
   and 4 does; else a downloaded same-maker PDF prints the part number
   verbatim (`sibling.py`), and that PDF is its datasheet; else `skip`,
   reason `no datasheet on the maker's site` with the link's host. For a TI
   row also: `https://www.ti.com/product/<device>` answering the 404 page
   (about 3.5 KB, title "Error 404") with no `lit/ds/symlink` PDF is that
   skip.
6. **In the datasheet's ordering table** (B): printed verbatim, or a full
   match of the (datasheet, series) ordering-code regex B writes in
   `specs.tsv`; else `skip`, reason `not in the datasheet's ordering table`.
7. **Not marked obsolete, discontinued, NRND or end of life by the
   datasheet** (B reads every `dead` = 1 row); a PDF whose every row is `out`
   for datasheet status is deleted and listed in `obsolete-not-included.md`.

Rules 5–7 apply to every download-set row, so B's final pick can fill a
skipped slot with the next row of the same (family, maker).

## 4. Datasheet placement

`census/place.py` (plan-40's, adapted) copies each needed PDF from
`/tmp/p42/pdf/<md5 of the URL>.pdf` to
`datasheets/regulator/<maker-slug>/<name>.pdf`, `<maker-slug>` `ti`,
`analog-devices`, `microchip`; `<name>` is the URL path's last segment (query
string dropped, `%20` → `-`, `.pdf` appended when missing, characters outside
`A-Za-z0-9._-` replaced by `-`); a clash gets `-<first 6 hex of the md5>`.
It writes `datasheets/regulator/datasheets.tsv`: path, URL, bytes, SHA-256.
Files with the same SHA-256 under two URLs share one file. A PDF no
download-set row uses is not copied.

Downloader rules (`census/fetch.js`, Node `fetch` with a Safari user agent,
4 at a time, one retry, a `%PDF-` check and `pdfinfo` on the body):

1. A URL on a maker host is fetched as given.
2. A `datasheets.maximintegrated.com` URL is fetched as given (it is ADI's).
3. A Rochester (`rocelec.widen.net`) or DigiKey (`mm.digikey.com`) link, or
   none, of a TI row → `https://www.ti.com/lit/ds/symlink/<device>.pdf`, the
   device being the row's `base` lowercased with everything from its first
   `/` dropped (`LP8340ILDX-ADJ/NOPB` → `lp8340ildx-adj`, then, when that is
   not a PDF, the leading letters-and-digits run, `lp8340`); the URL that
   answered is recorded.
4. The same for a Microchip row → `https://ww1.microchip.com/downloads/en/DeviceDoc/<base>.pdf`;
   when that is not a PDF, the first `ww1.microchip.com/downloads/…pdf`
   link of `https://www.microchip.com/en-us/product/<base>` (Node `fetch`).
5. Anything else → no PDF; `sibling.py` then looks for the part in a
   downloaded same-maker PDF.

## 5. The 300-result cap: the part-by-part stock check

DigiKey returns at most the first 300 results of a search, and the planning
listing hit that cap in 7 pieces (§ 2). The listing is complete for the pick
(every cut piece's unlisted parts have less stock than the pick takes), but
a listing is a snapshot: every row of the download set is looked up by its
own part number before B may use it. `census/stock-check.sh` runs `dk.sh
--manufacturer <maker id> --in-stock --status 0 --status 4 --limit 5 --json
"<mpn>"` (one call; the row's own maker id from the raw listing, so no
look-up calls) and writes `planning/plan-42-data/stock-check.tsv` (`mpn`,
`found` 0/1, `stock`, `status`, `date`); `found` = 1 when a product with
exactly that `ManufacturerProductNumber` has `QuantityAvailable` above 0. The
script skips part numbers already in the file, so a run cut by the quota
goes on the next day. `select.py --stock stock-check.tsv` makes a `found` = 0
row `out`, reason `stock check`, and sets `stock_checked`.

## 6. Scripts

- `/tmp/p42/dk.sh` (not in the tree): loads `DIGIKEY_*` from `~/.zshrc` with
  `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"`, then
  `node /Users/justinzaun/Development/logic/scripts/digikey.js "$@"`.
- Written at planning time (`README.md`): `census/top.js`, `list.sh`,
  `named.sh`, `recheck.sh`, `select.py` (with `--stock`), `download-set.py`,
  `footprint-lines.py`.
- Written in this letter, adapted from `planning/plan-40-data/census/`:
  `stock-check.sh` (§ 5), `fetch.js` (§ 4 rules), `common.py` (the download
  set, maker hosts, the rewrites), `urls.py`, `textscan.py`, `sibling.py`,
  `place.py`, `specs-skeleton.py` (paths, maker hosts, the regulator columns).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Restore the listing

- [x] Restore the planning listing as `README.md` says and run `select.py …
      --report`: `6225 parts listed`, 0 `NEEDS DEEPER LISTING`, `candidates
      from a cut piece: 158 of 600`; `cmp` with `candidates.tsv` →
      `reproducible`; `download-set.py --report` → `900 rows; 541 distinct
      URLs`. Observed 2026-11-21: `6225 parts listed`, `4 pieces cut at 300`, 0 `NEEDS`, `candidates from a cut piece: 158 of 600`, `reproducible`, `900 rows; 541 distinct URLs`.

Acceptance: the planning pick reproduces.
  Check: `python3 -I planning/plan-42-data/census/select.py /tmp/p42/raw /tmp/p42/cand.tsv && cmp /tmp/p42/cand.tsv planning/plan-42-data/candidates.tsv && echo reproducible` → `reproducible` (est. < 1 min).
Commit: —

### Phase 2 — The part-by-part stock check (one quota day)

- [x] `census/stock-check.sh` (§ 5); run it for every row of
      `download-set.tsv` (900 calls; ≥ 920 left before the run, § Prerequisites);
      a run cut by the quota goes on the next day. Done 2026-11-21: `stock-check.tsv` 901 lines (900 rows + header); the run stopped once on a script bug (Corrections 2) and went on.
- [x] `select.py /tmp/p42/raw planning/plan-42-data/candidates.tsv --stock
      planning/plan-42-data/stock-check.tsv --report`; then
      `download-set.py` again: a row that went `out` leaves the set and the
      next row of its (family, maker) enters it; check every new row the
      same way until no row of the set is unchecked. Record the number of
      `found` = 0 rows and of extra calls in § Corrections. 2 rows `found` = 0, 2 extra calls, both refills found (Corrections 3).

Acceptance: every download-set row is in stock, Active or Last Time Buy.
  Check: `awk -F'\t' 'NR>1 && $2!=1' planning/plan-42-data/stock-check.tsv \| wc -l` → the `found` = 0 count recorded above, and `python3 -I planning/plan-42-data/census/download-set.py planning/plan-42-data/candidates.tsv \| cut -f1 \| tail -n +2 \| grep -vxFf <(cut -f1 planning/plan-42-data/stock-check.tsv) \| wc -l` → 0 (est. < 1 min; the run itself is the quota day).
Commit: —

### Phase 3 — Download, place and scan the datasheets

- [x] `census/fetch.js`, `common.py`, `urls.py`, `place.py`,
      `textscan.py`, `sibling.py` (§ 4, § 6): download every distinct URL
      the download set cites (541 at planning) by § 4's rules into
      `/tmp/p42/pdf`; `%PDF-` and `pdfinfo` on each; failures recorded in §
      Corrections with their host. Observed: `urls.py` → `900 download-set rows; 846 distinct URLs` (the rewrites and fallbacks add URLs); `fetch.js` → `846 urls; 554 are PDFs; 292 failed` (every failure is a fallback guess, a Rochester-copy rewrite that 404s, or a 403; see Corrections 4). The `%PDF-` check is in `fetch.js`; `pdfinfo` is not run on every file (the `pdftotext` scan reads each one; a bad file yields no text and no `hit`).
- [x] Place the PDFs under `datasheets/regulator/<maker-slug>/` and write
      `datasheets/regulator/datasheets.tsv`; `place.py --verify` → `0
      missing, 0 unused`. The PDFs stay out of the commits until B prunes
      them (B Phase 4), so the history never carries PDFs no `add` row uses. Observed: `place.py` → `479 PDFs placed for 513 URLs`; `place.py --verify` → `0 missing, 0 unused`.
- [x] `candidates.tsv`: `pdf` (the placed path) for every download-set row
      whose URL, rewrite or sibling gave a PDF, and `hit` = 1 when its part
      number is printed verbatim (`pdftotext -layout`, token-boundary
      search); `dead` = 1 when the text near the part number says
      obsolete, discontinued, NRND, "not recommended for new designs" or
      end of life (B reads every such row). Observed: `textscan.py` → `873 rows with their own PDF; 671 printed verbatim; 8 dead`; `sibling.py` → `27 download-set rows without their own PDF; 8 printed in a same-maker PDF`; so 881 rows have a PDF, 19 have none, 679 `hit` = 1, 8 `dead` = 1.
- [x] `census/specs-skeleton.py`: `planning/plan-42-data/specs.tsv`, one row
      per (maker, series, PDF, Package / Case) of the download set, columns
      `maker`, `series`, `pdf`, `package`, `rows`, `example`, `scheme`,
      `label` (B's), `pinout`, `regulators`, `analog`, `vref`, `vin_min`,
      `dropout`, `adjust_to` (C's). Observed: `583 rows; 0 kept from before`.
- [x] Record `du -sh datasheets/regulator` here. `643M	datasheets/regulator` (479 PDFs).

Acceptance: every download-set row either has a PDF or a reason it has none.
  Check: `awk -F'\t' 'NR==FNR{s[$1]=1; next} ($1 in s) && $32==""' planning/plan-42-data/download-set.tsv planning/plan-42-data/candidates.tsv \| wc -l` → the rows B will `skip` for no maker datasheet (recorded here); `find datasheets/regulator -iname '*.pdf' \| wc -l` = `tail -n +2 datasheets/regulator/datasheets.tsv \| cut -f1 \| sort -u \| wc -l` (est. 25 min: 541 downloads, four at a time).
Commit: —

## Validation Plan

- Tests: none in this letter (data only); `select.py` and
  `download-set.py` are checked by their `--report` outputs above.
- Coverage check: the three Acceptance checks cover the listing, the stock
  check and the downloads.
- Runtime proof: n/a.
- Doc sync: none (`datasheets/skipped-parts.md` and `obsolete-not-included.md`
  are B's).
- Final gate: plan-42-J.

## Decisions

Owner decisions (asked once, 2026-11-15, before this plan was written):

- **Sub-types in scope:** fixed positive linear, adjustable positive linear,
  switching regulators (internal switch). Negative linear regulators are out.
- **Model: regulated supply** — VOUT acts as a supply at the rated output
  while VIN is in range and EN (if any) is 1; otherwise VOUT is unpowered;
  the voltage check uses VOUT as that net's voltage (plan-42-G).
- **Extras: all four** — generic parts; the VIN rating check (above the
  maximum refused, below the headroom unpowered); adjustable output from the
  divider resistors; dual / multi-output parts.
- **Size: 600 parts**, 200 per maker, most-stocked first.
- **Makers: the top three** (the owner's "top 3 manufacturers"; the ranking
  of § 2).

Author decisions (no user-visible fork; recorded so nobody re-litigates):

- **A maker is its DigiKey ids with the makers it bought** (as plan-40):
  National Semiconductor and Burr Brown are TI, Maxim Integrated is Analog
  Devices, Micrel is Microchip. The ranking sums 699 and 739 because both
  are in scope; by single categories the third maker differs (linear alone:
  Microchip 1962 > onsemi 1677; switching alone: Monolithic Power Systems
  632 > Microchip 351), and one maker set for the whole plan keeps every
  maker in every family.
- **Family quotas** (linear-fixed 240, linear-adjustable 90, linear-multi
  30, switching 240): fixed and switching parts dominate the in-stock
  population (13490 fixed / 2471 adjustable linear, 10091 switching);
  adjustable and dual parts get their own share so each maker shows them.
- **The named parts of `planning/todo.md`** are picked whenever listed and
  eligible, at most 6 per base (6 covers a base's common voltages and
  packages; with no cap LM1117 alone took 23 of TI's 80 fixed-linear slots).
  AMS1117 (Advanced Monolithic Systems) is not a top-three maker and is
  out; buck **modules** are DigiKey's separate module category and are out
  (§ Summary).
- **Automotive and military grades are out** (they duplicate a commercial
  part; as plan-40).
- **The stock check covers the whole download set** (900), not only the
  picked rows from cut pieces (158): the owner asked for the part-by-part
  re-check whenever the cap was hit, and checking the refill rows now keeps
  B free of DigiKey calls.
- **B refills only from the download set.** A (family, maker) share that is
  still short after B's verdicts stays short and is recorded with its count
  in B § Corrections (no second listing).
- **Datasheets live in `datasheets/regulator/<maker>/`**, one tree for
  linear and switching.

## Corrections

1. **Prerequisite measurements were stale** (2026-11-21): T0 = 39 (plan text: 36), P0 = R0 = 33042 and F0 = 1685 at cd9ca2a8 (plan: 32897 / 1646). The row "No other open plan adds regulators" printed 5 (plan-44-A, 44-E, 47-A, 47-I, 48-A) although none adds regulators: each only mentions plan-42's `PART_REGULATOR` as an open plan whose kind order it follows. (`grep -n` of those five files printed only those mentions.) plan-46 and the DigiKey quota are MET.
2. **`stock-check.sh` bug** (found at run time): plan-40's script aborted on `grep -q 'rate limited\|429'`, which also matches DigiKey's own summary line `… 429 left today`. The run stopped at 515 rows with 429 calls left; the pattern is now `rate limited` only, and the run went on (385 more calls, 42 left today). The quota is shared with other sessions: 999 at the start, 429 after 515 calls.
3. **Stock-check result:** 900 calls over the download set plus 2 refill calls = 902; `found` = 0 for 2 rows (`MIC5365-3.3YC5-TR`, `TLV75801PDBVR`: no stock; both left the pick: `select.py --stock` → `out`, `stock check`); the next rows of their (family, maker), `MIC5365-2.8YC5-TR` and `TPS77901DGKR`, entered the set and were found (stock 20563, 23500, Active). Final `awk -F'\t' 'NR>1 && $2!=1' stock-check.tsv | wc -l` → 2; every download-set row is in `stock-check.tsv` (0 unchecked). The set is still 900 rows, 541 distinct URLs, `picked per maker` 200 / 200 / 200 and 598 Active + 2 Last Time Buy as before. `download-set.tsv` was regenerated for the final set (two rows changed).
4. **Download rules refined, three observed cases:**
   - A TI own-site link `www.ti.com/general/docs/suppproductinfo.tsp?distId=10&gotoUrl=…/lit/gpn/<device>` is an HTML redirect shell (200, not a PDF): 145 download-set rows had no PDF. `common.ti_goto()` takes the device from `gotoUrl` (decoded up to three times) and `sources()` fetches `https://www.ti.com/lit/ds/symlink/<device>.pdf` first (`ti.com/lit/gpn/<device>` answers 302 to it); after this 27 rows had none, 8 of them found by `sibling.py`.
   - A Microchip row of a Rochester or DigiKey link was tried under its `base` and under the leading letters-and-digits run (as § 4 rule 3 does for TI): `MIC5235BM5` → `MIC5235`, then the `page:` product pages; `fetch.js` reads the first `ww1.microchip.com/downloads/…pdf` link of such a page and records it in `index.tsv`'s sixth column, which `place.py` uses as the file's URL.
   - Failures of `fetch.js`: 292 of 846 URLs, hosts `www.ti.com` 114 (a guessed `symlink` rewrite that 404s, or a TI page that is not a PDF), `ww1.microchip.com` 97 and `www.microchip.com` 82 (403 to a guessed `DeviceDoc/<name>.pdf`, `no-pdf-link` for a product page that has none). Every download-set row that found no PDF has none by rule 5: 19 rows, TI 10 (Rochester copies of National parts; `TPS79301DBVR`, `TLV70710DQNR`, `LM2788MM-2.0`), Microchip 9 (Rochester copies of Micrel parts; `MIC5303-1.8YMT-TR`: its own `ww1` link answers 403).
   B skips those 19 as `no datasheet on the maker's site`.
5. `specs.tsv` also has a `pinout_adj` column (plan-42-C § 3 rule 3 needs it); the A skeleton columns of § Phase 3 lacked it.
6. Observed acceptance: Phase 2: found = 0 count 2 = recorded; unchecked download-set rows → 0. Phase 3: rows without a PDF → 19; `find datasheets/regulator -iname '*.pdf' | wc -l` → 479 = `tail -n +2 datasheets/regulator/datasheets.tsv | cut -f1 | sort -u | wc -l` → 479; `specs.tsv` → 583 rows; `du -sh` 643M.
7. The 8 `dead` = 1 rows (text-scan flag) are B's to read.

## Summary

A is the plan's quota-bound letter: one day of DigiKey calls (900 part-by-part
look-ups) and 541 downloads. Its risk is wrong data feeding B (a stale stock
figure, a PDF that is a distributor copy): the stock check and the
`%PDF-` / verbatim-name checks are there to catch it. Out of scope for the
whole plan: negative and inverting regulators, switching **controllers**
(external FET), voltage references, DDR tracking regulators, PMICs with a
serial interface, buck **modules**, makers outside the top three (AMS1117,
onsemi, MPS, ST, Diodes, …), automotive and military grades, and any analog
behaviour (ripple, load current, thermal limits). Nothing in the tree
except `planning/plan-42-data/` and `datasheets/regulator/` changes in A.
