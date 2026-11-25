# plan-58-A: Slide switches — census, datasheet downloads, decisions

Last updated: 2026-11-24
Overall Effort: x-large (1d–3d) — plan-58 is A → B → C → D → E → F → G → H
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-58 adds slide switches to the parts library and the app: the SPST, SPDT,
DPST and DPDT slide switches of the four makers that have the most of them in
stock at DigiKey (C&K / Littelfuse, NKK, CIT Relay & Switch, E-Switch). They
sit in the parts drawer under **Switches & Relays → Slide**, one leaf per
circuit and action (`SPDT On-On`, `SPDT On-Off-On`, `SPST On-Off`, …). In play
mode a click on a placed slide switch moves its knob to the next position
(two positions, or three for a center-off switch); exactly one position is
active, and the simulation and the voltage check see its contacts.

The single behavioral outcome of plan-58: loading the part files gives the
baseline switch parts plus every row of `planning/plan-58-data/candidates.tsv`
whose `decision` ends as `add`; the app lists them under the Slide leaves; in
play mode a click on a placed SPDT On-On switch wired 5 V → common, throw A →
LED 1, throw B → LED 2 moves the 5 V from LED 1 to LED 2 and back; `ctest`
passes T0 + 1 programs (`slide_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | census re-run, datasheet downloads, verbatim-match decisions, skipped parts, `specs.tsv` skeleton | large |
| B | backend: the `switch` kind gains `states` / `rest`; `circuit_set_switch_state`; `slide_test`; docs | medium |
| C | C&K: part-number schemes, footprints, `gen-parts.py`, parts, goldens | large |
| D | NKK: schemes, footprints, parts, goldens | medium |
| E | E-Switch: schemes, footprints, parts, goldens | medium |
| F | CIT Relay & Switch: schemes, footprints, parts, goldens | medium |
| G | app: drawer, symbol, click-to-move, sidebar, file, BOM, automation | large |
| H | landing tier: full gates, harness, runtime proof | small |

Outcome of A: `datasheets/switch/slide/<maker>/` holds every PDF a part of
`candidates.tsv` needs; the `decision` column is `add` (part number verbatim in
its PDF), `pending` (PDF present, number not verbatim: C–F decide it by the
datasheet's part-number key), `skip` (no usable PDF) or `out`; `specs.tsv`
exists with its drawing columns empty (C–F fill them).

References:

- `AGENTS.md` (§ Datasheets, § Git); `docs/parts-format.md` (§ Parts, § `switch`,
  § Datasheets, § Footprints); `docs/backend.md` (§ tests, goldens).
- `planning/plan-58-data/README.md`, `planning/plan-58-data/census/` — written
  at planning time, 2026-11-24.
- Precedent, read before starting: `planning/completed/plan-27-A-tactile-data.md`
  (the same shape for tactile switches), `planning/plan-27-data/README.md`
  (its census scripts: `fetch.py`, `textscan.py`, `decide.py`, `place.py`,
  `maker-urls.py` are the model for A's scripts; copy and adapt, do not call them).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p58 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8` → `100% tests passed` | MET (re-run 2026-11-24 in P-58: `100% tests passed, 0 tests failed out of 61`; planning-time note:): plan-51-J's gate on this tree was `100% tests passed, 0 tests failed out of 61`; **T0 = 61** (`grep -c '^logic_test' backend/CMakeLists.txt` → 61) |
| Switch baseline recorded | `grep -c '|switch|' backend/tests/golden/parts.txt` → 2070 parts; `grep -c '"id"' schematical/schematical/Parts/switch.json` → 186 devices | MET: **S0 = 2070** parts, 186 devices (2026-11-24) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` → 37090 / 37090 / 2929 | MET: **P0 = R0 = 37090**, **F0 = 2929** (2026-11-24) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p58-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET at plan-51-J (297 app tests green, 2026-11-24); not re-run at planning |
| No slide-switch support yet | `grep -c 'states' backend/src/models/switch.c` → 0; `ls planning/plan-58-[B-H]-* \| wc -l` → 7 (this plan's files) | MET (2026-11-24) |
| Safari has a DigiKey page open and passes Cloudflare | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/slide-switches/213" 8 'document.title'` → `Slide Switches \| Switches \| DigiKey` | MET again (2026-11-24 at P-58: the first four runs returned `Just a moment...`; after the owner ticked the box, `Slide Switches | Switches | DigiKey`); it passed during the planning census, then about 100 page loads later Cloudflare challenged the window (`Just a moment...`): tick "Verify you are human" once in the DigiKey tab, then re-run this command |
| Tools exist | `pdftotext -v`, `pdfinfo -v`, `python3 --version`, `node --version` each exit 0 | MET (2026-11-24: all used during planning) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `candidates.tsv` is rebuilt from a fresh DigiKey census and has no row with
  `decision` = `check`: every row is `add`, `pending`, `skip` or `out` with a `reason`.
- `datasheets/switch/slide/<maker>/` holds each datasheet PDF that at least one
  `add` or `pending` row needs, with `datasheets/switch/slide/datasheets.tsv`
  mapping each file to its URL, size and SHA-256.
- `datasheets/skipped-parts.md` has a "Slide switches (plan-58)" section listing
  every `skip` row; `obsolete-not-included.md` gets a row only for a PDF whose
  every part is obsolete.
- `planning/plan-58-data/specs.tsv` has one header line and one row per
  (maker, series, mounting, circuit) of the kept rows, its drawing columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter; `pending` rows are decided in C–F.
- The scope decisions of § Decisions are not re-opened here.

## 2. Current State

The library has one switch kind (`backend/src/models/switch.c`: DIP and tactile
switches), 186 devices / 2070 parts in `Parts/switch.json`, drawers
`["Switches & Relays","DIP"]` and `["Switches & Relays","Tactile"]`. There is
no slide switch. `planning/plan-58-data/` holds the census scripts and the
planning-time `candidates.tsv`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Slide Switches at DigiKey (category 213), all | 3601 | `scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/slide-switches/213" 10 'document.body.innerText.match(/Results:\s*[\d,]+/)[0]'` → `Results: 3,601` |
| …In Stock + Active | 940 | same URL with `?s=N4IgjCBcpgnAHLKoDGUBmBDANgZwKYA0IA9lANogAMIAugL7EC0ATMiGpFnkaRSAFY69EUA` → `Results: 940` |
| Parts of the four makers, SPST/SPDT/DPST/DPDT, In Stock + Active (distinct maker + part number) | 422 | `census/list.sh /tmp/p58/raw && python3 -I census/build.py /tmp/p58/raw planning/plan-58-data/candidates.tsv` → `422 parts` (16 DigiKey pages); re-run at P-58, 2026-11-24: **423 parts** (see Corrections) |
| …by maker | C&K 185 → 186 at P-58 (DigiKey names it `C&K` and `Littelfuse/C&K`), NKK 91, CIT 75, E-Switch 71 | `awk -F'\t' 'NR>1{print $1}' planning/plan-58-data/candidates.tsv \| sort \| uniq -c` |
| `add` after DigiKey rules | 296 (C&K 119, NKK 71, E-Switch 63, CIT 43) | `awk -F'\t' '$18=="add"' planning/plan-58-data/candidates.tsv \| wc -l` → 296 |
| `out`: panel mount 90, no DigiKey stock 21, line-voltage selector 7, On-Mom 3, illuminated 3, On-Off-On/On-On-On 2 | 126 (C&K 66, CIT 32, NKK 20, E-Switch 8); at P-58 **127** (panel mount 91, C&K 67) | `awk -F'\t' '$18=="out"{print $19}' planning/plan-58-data/candidates.tsv \| cut -c1-30 \| sort \| uniq -c` |
| `add` by circuit and action | SPDT On-On 130, DPDT On-On 103, SPDT On-Off-On 24, DPDT On-Off-On 18, SPST On-Off 15, DPST On-Off 6; at P-58 SPDT On-On 131, SPDT On-Off-On 23, the rest the same (296 total, C&K 119, NKK 71, E-Switch 63, CIT 43) | `awk -F'\t' '$18=="add"{print $4, $5}' … \| sort \| uniq -c` |
| Distinct (maker, series) / (series, mounting) / (series, mounting, circuit) of `add` rows | 36 / 66 / 108 | a python count over `candidates.tsv` (planning-time probe) |
| Distinct datasheet URLs of `add` rows | 40 (littelfuse.com 12 + 1 DigiKey, nkkswitches.com 8, citrelay.com 11, E-Switch 7 on `configured-product-images.s3.amazonaws.com` + 1 DigiKey) | `awk -F'\t' '$18=="add"{print $16}' … \| sort -u \| wc -l` → 40 |
| …that download as a PDF with Safari's request headers | 38 of 40 (the 2 `mm.digikey.com` ones, protocol-relative `//mm.digikey.com/…`, fail from `curl`: Cloudflare; `scripts/safari-digikey-pdf.sh` fetches them) | planning-time probe: `curl -sL --http2 --compressed` with the plan-27 header set → `%PDF-` for 38 |
| `add` rows whose part number is verbatim in its PDF's text | 102 of 296 (C&K 54 of 119, NKK 18 of 71, E-Switch 20 of 63, CIT 10 of 43) | planning-time probe: `pdftotext -layout` + a case-blind substring test |
| ctest programs T0 | 61 | `grep -c '^logic_test' backend/CMakeLists.txt` → 61 |

### Verified properties

- **DigiKey listing.** The category page's own `__NEXT_DATA__` carries every
  filter and up to 100 products with their parameters; the `s=` filter value is
  lz-string of `{"1": {...}}` (documented in `census/make-urls.js`). Parametric
  filters need `"pt":"String"`, otherwise DigiKey ignores them (probe:
  Circuit SPDT without `pt` → 940 results, with it → 451). Clicking filters or
  the next-page button from a script does not work (Safari ignores the events
  in the temporary tab), so the census asks for one (maker, circuit) page at a time;
  every page has fewer than 100 products (largest 88). Safari sometimes returns another
  tab's page: `check.py` rejects a page whose rows do not match the request and
  `list.sh` retries.
- **Cloudflare.** The census's 16 pages load without a challenge (4 min); a long
  session of page loads (about 100 in an hour at planning) gets the Safari window
  challenged again, and `safari-digikey.sh` then returns `Just a moment...`. The
  fix is the one tick in the DigiKey tab (`scripts/safari-digikey.sh` header).
- **"In Stock" is DigiKey's own line only.** DigiKey's "In Stock" facet also
  matches parts that have only Marketplace or Factory stock; the product cell
  then has no line labelled `In Stock`, or one with quantity 0 (21 parts:
  `S202091MS02G`, `L102011SS03Q`, `AS22BW`, …). `build.py` counts only the
  `In Stock` line (AGENTS.md: "a quantity available above zero").
- **Datasheet hosts.** `www.littelfuse.com` answers 403 to `curl` and `urllib`
  and `200 application/pdf` to `curl --http2 --compressed` with Safari 18.6's
  full header set (`Accept`, `Accept-Language`, `Accept-Encoding: gzip, deflate,
  br`, `Sec-Fetch-Site: none`, `Sec-Fetch-Mode: navigate`, `Sec-Fetch-Dest:
  document`, `Priority: u=0, i`) and the site's origin as Referer (probe, 12 of 12).
  `www.nkkswitches.com`, `www.citrelay.com` and the E-Switch S3 bucket answer a
  plain `curl` with a PDF.
- **E-Switch's datasheets are not on the maker's site** (`configured-product-images.s3.amazonaws.com`
  and `mm.digikey.com`), so `docs/parts-format.md` § Datasheets' "maker's own
  site" rule needs `maker-urls.tsv` (the plan-27 mechanism: the same document
  on the maker's site, or the DigiKey URL stays; owner, 2026-11-08, plan-27-A).
- **Part numbers.** A slide switch's number is built from an ordering key
  (C&K `OS 1 0 2 0 1 1 M S 2 Q N 1`, NKK `M2012SS1W01`), so a datasheet often
  prints the key and a few examples, not every part: the 194 rows without a
  verbatim hit are the `pending` rows that C–F decide against the key.

## 3. Design Overview

A is data work only: re-run the census, fetch and read the datasheets, decide
each row, and leave a skeleton for C–F. It mirrors plan-27-A with these
differences: DigiKey is read through Safari (AGENTS.md), the census is 16 pages
instead of 229 API calls, and there are four makers instead of ten. Risk is
small; the one trap is DigiKey's "In Stock" meaning (§ 2). The whole population
is `candidates.tsv`; later letters never re-run the census (they read
`candidates.tsv`), so a part that drops out of stock after A stays in scope.

Rejected: using `scripts/digikey.js` (AGENTS.md: stock lookups go through
Safari); one census page per maker (DigiKey returns at most 100 products and
C&K has 119 `add` rows, so a per-circuit page is the smallest filter that stays
under 100 for every maker); including panel-mount parts (owner, § Decisions).

## 4. Detailed Design

### 4.1 `candidates.tsv`

Columns (written by `census/build.py`): `maker mpn series circuit function
timing current vac vdc actuator mounting termination features stock packaging
datasheet detail decision reason`. A appends `pdf` (1 when a PDF was
downloaded and opens with `pdfinfo`), `hit` (1 when the part number is verbatim
in the PDF text, whitespace and case ignored), `dead` (1 when "obsolete",
"discontinued", "NRND" or "end of life" stands within 300 characters of the
number). `decision` rules, in this order, for rows `build.py` left `add`:

1. `pdf` = 0 → `skip`, reason `no datasheet`.
2. `hit` = 1 → `add`.
3. `hit` = 0 → `pending`, reason `number not in datasheet text`.

A PDF whose every row has `dead` = 1 is deleted and recorded in
`datasheets/obsolete-not-included.md`; its rows become `out`, reason `obsolete`.
A row with only `dead` = 1 stays `add` and is flagged `"flags": ["obsolete"]`
by the parts letter.

### 4.2 `specs.tsv`

One row per (maker, series, mounting, circuit) of the `add` and `pending`
rows. Columns: `maker series mounting circuit mpn_regex terminals pins footprint
note`. A writes the first four, `terminals` (the DigiKey `termination` values,
for the reader) and leaves `mpn_regex`, `pins`, `footprint`, `note` empty. C–F
fill them: `mpn_regex` is an anchored Python regular expression over the part
number that selects the rows of this spec (the series key's mounting and
circuit digits); `pins` is `designator=name;…` using the names of § 4 of plan-58-B
(`C`, `A`, `B`, `C1`, `A1`, `B1`, `C2`, `A2`, `B2`, or `A`, `B` / `A1`, `B1`,
`A2`, `B2` for SPST / DPST); `footprint` is a `footprints.json` id.

### 4.3 Where files go

- `datasheets/switch/slide/<maker>/<original file name>.pdf` with
  `maker` = `ck`, `nkk`, `cit`, `eswitch`; `datasheets/switch/slide/datasheets.tsv`
  (`file`, `url`, `bytes`, `sha256`) as `datasheets/switch/tactile/datasheets.tsv`.
- `planning/plan-58-data/census/{fetch,textscan,decide,place,maker-urls}.py`:
  the scripts of Phase 2–4, each with a `--help` docstring and a `--selftest` or
  a `--verify` mode, as plan-27's.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` for partially done; `- [x] ~~text~~ —
> moot: <evidence>` for a moot task; fill the phase's `Commit:` line when it
> lands. An unticked box means NOT DONE.

### Phase 1 — re-run the census

- [x] Re-run every Prerequisites command; update the Status column. (backend `100% tests passed, 0 tests failed out of 61`; S0 2070 / 186; P0 = R0 37090, F0 2929; `states` 0; 7 other plan files; Safari `Slide Switches | Switches | DigiKey`; tools exit 0. The app-build row was not re-run: plan-51-J's gate stands.)
- [x] `census/list.sh /tmp/p58/raw` (16 pages; the planning-time run took 4 min, Safari) → one line per
      page, no `FAILED` (16 lines, `count N rows N` equal on each, e.g. `C&K SPDT count 88 rows 88`); if a page fails after its 5 tries, re-run just it:
      `census/list.sh /tmp/p58/raw <maker> <circuit>`.
- [x] `python3 -I census/build.py /tmp/p58/raw planning/plan-58-data/candidates.tsv` → `423 parts`.
- [x] Record the new populations in § 2's table (and in Corrections where a
      count differs from the planning-time one).

Acceptance: `candidates.tsv` has a header and one row per part; every row's
`decision` is `add`, `out` or `check`; each (maker, circuit) page's count equals
its row count.
  Check: `awk -F'\t' 'NR>1{n++; d[$18]++} END{print n, d["add"], d["out"], d["check"]}' planning/plan-58-data/candidates.tsv` prints the four counts, and `awk -F'\t' 'NR>1 && $18!="add" && $18!="out" && $18!="check"' planning/plan-58-data/candidates.tsv | wc -l` → 0 (est. 6 min). Measured: `423 296 127 ` (check column empty) and `0`.
Commit: ca7a70d3

### Phase 2 — download the datasheets

- [x] `census/fetch.py <candidates.tsv> /tmp/p58/pdf` → `40 urls; 40 are PDFs; 0 missing`, exit 0 (the 2 `mm.digikey.com` ones through Safari) : the distinct `datasheet`
      URLs of the `add` rows (a `//host/…` URL gets `https:`). Headers: Safari
      18.6's full set for `www.littelfuse.com`; plain `curl`-style for the
      others; `mm.digikey.com` through `scripts/safari-digikey-pdf.sh`, one at a
      time. A body that does not start with `%PDF-` is not kept; `pdfinfo` must
      open the file. Writes `/tmp/p58/pdf/index.tsv` (md5 of URL, status, bytes,
      `%PDF`, url). Exit 1 lists missing files.
- [x] `census/textscan.py <candidates.tsv> /tmp/p58/pdf /tmp/p58/scan.tsv` → 296 `add` rows with `pdf` = 1, `hit` = 1 on 93 (C&K 54, NKK 19, E-Switch 20, CIT 0), `dead` = 1 on 0: adds `pdf`,
      `hit`, `dead` (§ 4.1) with `pdftotext -layout`.
- [x] Tests (both print `ok`): `fetch.py --selftest` (header set, `%PDF-` check, protocol-relative
      URL) and `textscan.py --selftest` (verbatim / spaced / obsolete cases on
      three inline strings).

Acceptance: every `add` row has `pdf` and `hit` filled; the number of distinct
downloaded PDFs equals the number of distinct URLs minus the ones listed as
missing.
  Check: `python3 -I planning/plan-58-data/census/textscan.py --selftest` →
  `ok`, and `awk -F'\t' 'NR>1 && $18=="add" && $20==""' /tmp/p58/scan.tsv | wc -l` → 0 (est. 5 min). Measured: `ok`, `ok`, `0`.
Commit: 54cf8eee

### Phase 3 — decide, place, document

- [x] `census/decide.py <candidates.tsv> /tmp/p58/pdf` (after copying `scan.tsv` over `candidates.tsv`, which adds `pdf`, `hit`, `dead`) → `93 add`, `127 out`, `203 pending`, no `skip`: § 4.1's rules;
      `--skipped` writes the "Slide switches (plan-58)" section of
      `datasheets/skipped-parts.md` (one line per `skip` row: maker, part
      number, reason); `--verify-skipped` re-reads that section (`0 skip rows written`; the section says `None: every kept row's datasheet downloaded as a PDF.`; `--verify-skipped` → `0 skip rows missing …`, exit 0).
- [x] `census/place.py <candidates.tsv> /tmp/p58/pdf` → `40 PDFs placed for 40 URLs`; `--verify` → `40 URLs, 40 PDFs, 0 problems`: copies the PDFs the
      `add` / `pending` rows need to `datasheets/switch/slide/<maker>/`,
      writes `datasheets/switch/slide/datasheets.tsv`; `--verify` recomputes
      the hashes.
- [x] `census/maker-urls.py --list` → 2 URLs (see Corrections: not 8; the S3 bucket is E-Switch's own host): the datasheet URLs not on the maker's own
      site; for each, find the same document on the maker's site
      and write `planning/plan-58-data/maker-urls.tsv` (`url`, `maker_url`, empty
      `maker_url` when the maker's site has none: the DigiKey URL then stays).
      `--verify` fetches each `maker_url` and requires `%PDF-` (`2 non-maker URLs, 1 with a maker URL, 0 missing, 0 not PDFs`).
- [x] `census/specs-skeleton.py <candidates.tsv> planning/plan-58-data/specs.tsv` → `108 rows` (= distinct (maker, series, mounting, circuit) of the 296 `add` + `pending` rows).
- [x] Obsolete check (AGENTS.md): any PDF whose every row has `dead` = 1 →
      delete it, add its line to `datasheets/obsolete-not-included.md`. None: `dead` = 0 on all 296 rows (`awk -F'\t' 'NR>1 && $18=="add"{d[$22]++} END{for(k in d)print k,d[k]}' /tmp/p58/scan.tsv` → `0 296`), so no PDF is deleted and `obsolete-not-included.md` is untouched.
- [x] Update `planning/plan-58-data/README.md` with the scripts of Phase 2–3.

Acceptance: `decision` has no `check`; `decide.py --verify-skipped` and
`place.py --verify` and `maker-urls.py --verify` all exit 0; `specs.tsv` has
one row per distinct (maker, series, mounting, circuit) of the `add` and
`pending` rows.
  Check: `python3 -I planning/plan-58-data/census/place.py --verify && python3 -I
  planning/plan-58-data/census/decide.py --verify-skipped && python3 -I
  planning/plan-58-data/census/maker-urls.py --verify` → each exits 0 (est. 6 min). Measured: `40 URLs, 40 PDFs, 0 problems` / `0 skip rows missing from skipped-parts.md, 0 rows `check`` / `2 non-maker URLs, 1 with a maker URL, 0 missing, 0 not PDFs`, all exit 0.
Commit: —

## Validation Plan

- Tests: the three scripts' `--selftest` / `--verify` modes (above); no backend
  or app test changes in A.
- Coverage check: `git diff --stat` shows only `planning/`, `datasheets/` files.
- Runtime proof: not applicable (data only).
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`
  (when a PDF is dropped), `planning/plan-58-data/README.md`.
- Final gate (run ONCE, after Phase 3): `python3 -I planning/plan-58-data/census/place.py --verify`
  and `git status --short` lists only the paths above (est. 1 min). Per-phase
  checks stay scoped.

## Decisions

- Makers: C&K / Littelfuse, NKK, CIT Relay & Switch and E-Switch — decided by
  the owner, 2026-11-24. The top three by SPST/SPDT/DPST/DPDT parts in stock
  and Active are C&K, NKK, CIT (the planning-time counts 185 / 91 / 75; E-Switch
  71, all rows of `candidates.tsv`); the owner took four. (§ 2)
- Actions: latching only — On-On, On-Off-On, On-Off (and Off-On, none in stock
  at planning) — decided by the owner, 2026-11-24. Momentary (On-Mom,
  Mom-On-Mom, On-On-Mom), line-voltage selectors and `On-Off-On/On-On-On` are `out`. (§ 4.1, `build.py`)
- Panel-mount parts (Mounting Type `Panel Mount`, `Panel Mount, Snap-In`) are
  left out — decided by the owner, 2026-11-24 (90 rows `out` at planning).
- Illuminated actuators are left out — the author's call: a lamp's extra
  terminals are not modelled (3 rows at planning).
- "In stock" is DigiKey's own `In Stock` line above zero; Marketplace-only and
  Factory-only parts are `out` — the author's reading of AGENTS.md (21 rows at planning).
- Datasheet URLs follow plan-27's rule: the maker's own site when the same
  document is there, else DigiKey's URL — owner, 2026-11-08 (plan-27-A).

## Corrections

- **2026-11-24, Phase 1: the census differs from the planning-time one by two parts.** `build.py` on the fresh pages gives 423 parts (planning-time 422): `diff <(cut -f1,2 old) <(cut -f1,2 new)` shows `C&K S802C21S207DQ` (new, Panel Mount, Snap-In: `out`) and `NKK AS12AP` (new, `add`) and `NKK MSN13ASG40` gone (out of stock since). `add` stays 296; by action SPDT On-On is 131 (was 130) and SPDT On-Off-On 23 (was 24); `out` is 127 (was 126). No later letter's scope is derived from these counts, so none is re-scoped. § 2's table carries both numbers.
- **2026-11-24, Phase 2: `hit` is 93 of 296, not 102; CIT has 0 hits, not 10.** The planning probe was a case-blind substring test; `textscan.py` also requires the number to stand on its own (plan-27-A Correction 6). The 10 CIT "hits" of the probe (`MS1202L6`, `MS1206L6`, `MS1247L4`, `MS2201L3`, `MS2205L6`, `MS2208L4`, `MS2210L4`, `MS2211L4`, `MS2219L4`, `MS2241L9`) are not part numbers in the PDF: `pdftotext -layout` of `Switch-MS1202.pdf` line 17 reads `1. Series  MS1202   L6`, the series and the actuator length printed in separate cells of the ordering-key table, which the substring test joined. They are `pending` like the other 203 (C–F read the key). C&K 54, NKK 19 (probe 18: `AS12AP` is new), E-Switch 20 match the probe. § 2's table row "`add` rows whose part number is verbatim" is 93 of 296 (C&K 54 of 119, NKK 19 of 71, E-Switch 20 of 63, CIT 0 of 43), so `pending` is 203 (was 194).
- **2026-11-24, Phase 3: 2 datasheet URLs are not the maker's, not 8.** § 2 and Phase 3 counted E-Switch's S3 bucket (`configured-product-images.s3.amazonaws.com`, 7 URLs) as not the maker's site. It is: E-Switch's own product pages link their datasheets there (`curl` of `e-switch.com/product/500-series-miniature-slide-switch/`, `…/500a-…`, `…/500r-…`, `…/eg-series-subminiature-slide-switch/` shows `https://configured-product-images.s3.amazonaws.com/Datasheets/500.pdf`, `500A.pdf`, `500R.pdf`, `EG.pdf`), and plan-27 treated the bucket as the maker's host too. So only the two `mm.digikey.com` URLs need `maker-urls.tsv`: E-Switch EG → `S3 Datasheets/EG.pdf` (same 15-page document, newer revision); C&K `SS-12D07-VG 4 NS GA PA` → no maker copy (DigiKey's file is a one-page 2006 COMAX customer drawing; the SS series datasheet on littelfuse.com is a different document, left to plan-58-C: the one row is `pending`). The S3 `EG.pdf` and DigiKey's `EG_Series.pdf` are both placed (rows name both URLs).
- **2026-11-24, Phase 3: `decide.py --skipped` wiped `skipped-parts.md` on its first run** (it opened the file for writing before reading it: 1192 lines deleted). Restored from the last commit, the read moved ahead of the open, re-run: `datasheets/skipped-parts.md | 10 ++++++++++`. No other file was touched by it.

## Summary

The risk in A is DigiKey, not the code: a census page that silently comes from
another tab (`check.py` catches it), "In Stock" counting third-party stock
(`build.py` counts only DigiKey's own line), and the 194 part numbers the
datasheets print only as an ordering key (decided in C–F). Everything in
`backend/`, `schematical/` and `Parts/` is untouched.
