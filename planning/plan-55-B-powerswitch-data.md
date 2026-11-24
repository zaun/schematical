# plan-55-B: Data — the listing, stock re-check, datasheet downloads, text scan, specs skeleton

Last updated: 2026-11-22
Effort: large (3h–1d; plus two listings of at least 30 DigiKey calls each)
Depends on: plan-55-A

B lists every in-stock **Active** part of the nine (family, maker) pieces of plan-55-A § 2
(ideal diodes: Texas Instruments, Analog Devices, Monolithic Power Systems; load switches: Texas
Instruments, onsemi, Microchip Technology; current-limited and USB switches: Texas Instruments,
Microchip Technology, Diodes Incorporated), re-checks the stock, downloads and places the maker
datasheets, scans them for the part numbers, and writes the `specs.tsv` skeleton the verdict
letters fill. The behavioral outcome: `planning/plan-55-data/candidates.tsv` has one row per listed
part (at most 1037, plan-55-A § 2), every row records its stock re-check, its maker-datasheet state and
its PDF; `datasheets/powerswitch/<maker>/` holds every PDF a
candidate row needs; no part file, footprint, backend or app file changes.

References:

- `planning/completed/plan-55-A-powerswitch-kind.md` (§ 2 populations, § 3.2 conventions);
  `AGENTS.md` (§ Datasheets); `docs/parts-format.md` § Datasheets.
- `planning/plan-55-data/` — `README.md`, `census/families.js`, `families.out`.
- `planning/plan-54-data/census/` (`list.js`, `list.sh`, `select.py`, `common.py`: the listing
  scripts B adapts) and `planning/plan-44-data/census/` (`stock-check.sh`, `fetch.js`, `place.py`,
  `textscan.py`, `sibling.py`, `specs-skeleton.py`: the download scripts B adapts);
  `planning/completed/plan-42-A-regulator-data.md` § 4 (the maker download rules).

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-A complete | `ls planning/completed/plan-55-A-*.md` → exists | MET (2026-11-23: archived at 29edf01f) |
| DigiKey quota | plan-55-A's quota row: `left` ≥ 40 before Phase 1 and again before Phase 2; **owner 2026-11-23: every call runs with `--opt2`** | MET for Phase 1 (2026-11-23: 750 left with key 2 before the listing, 677 after); Phase 2: re-measure |
| The census still shows nine pieces | `grep -c '^pick' planning/plan-55-data/families.out` → 9 | MET (2026-11-23: 9) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-55-data/raw/<family>-<maker id>.jsonl.gz` holds every part of the 14 pieces of § 3.1
  (a piece is one DigiKey maker label of a picked company) and `raw/*.err` shows no `cut at` line.
- `planning/plan-55-data/candidates.tsv` has one row per listed part with the § 3.2 columns;
  `state` is `candidate` or `out` with a `reason`.
- `planning/plan-55-data/stock-check.tsv` re-checks every candidate row; a row that is not in stock
  and Active is `out`, reason `stock check`.
- `datasheets/powerswitch/<maker slug>/` (`ti`, `analog-devices`, `monolithic-power-systems`,
  `onsemi`, `microchip`, `diodes`) holds each PDF at least one candidate row needs, with a
  `datasheets.tsv` (file, URL, bytes, SHA-256).
- `candidates.tsv` has `pdf`, `hit`, `dead` and `stock_checked` filled for every candidate row;
  `specs.tsv` has one row per (maker, series, PDF) with its pinout columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added and no row is `add`: the verdicts are C's, D's and E's.
- No datasheet hunting beyond § 3.3's rules; a row with no downloadable maker PDF is `out`.
- No generic part, no maker beyond the nine pieces' companies (plan-55-A § Decisions).

## 2. Current State

The library has no power switch and `datasheets/` has no `powerswitch` folder (plan-55-A §
Prerequisites). DigiKey's own links are not the maker's: a TI product's `DatasheetUrl` is
`https://www.ti.com/general/docs/suppproductinfo.tsp?distId=10&gotoUrl=https%3A%2F%2Fwww.ti.com%2Flit%2Fgpn%2Flm66100`
(read for `LM66100DCKT`, 2026-11-22), and the maker document behind it is
`https://www.ti.com/lit/ds/symlink/lm66100.pdf`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts per (family, maker) piece at planning | ideal 44 / 37 / 9; load 140 / 59 / 42; current-limited 366 / 173 / 167 (1037) | `grep '^pick' planning/plan-55-data/families.out` |
| Pieces above DigiKey's 300-result cap | 1: current-limited Texas Instruments (363 under label 296) | `grep '^pick' planning/plan-55-data/families.out` and plan-55-A § 2 |
| DigiKey calls of one listing, lower bound | **30**: ideal 4 (one call for each of the labels 296, 505, 175, 1589); load 7 (140 → 3; 35, 24, 36, 6 → 1 each); current-limited 19 (the 363-part piece: one call for its facets, then at least ceil(363 / 50) = 8 for its sub-pieces; 3 → 1; 167 → 4; 156 → 4; 17 → 1) | one call per 50 parts of a (sub-)piece, from the `pick` counts of `families.out`; the split's exact sub-pieces are known only when `list.js` runs, so B budgets **40** per listing |
| PDFs already under `datasheets/reference-boards/power-management/` | 13 (among them the Diodes `AP2161_71.pdf`; the TI `tps2116.pdf` is a power mux, out of scope) | `find datasheets/reference-boards/power-management -iname '*.pdf' \| wc -l` → 13; B downloads its own copies and does not touch that folder |

### Verified properties

- **TI documents answer a script at the symlink URL.** `curl -sL -A Mozilla/5.0
  https://www.ti.com/lit/ds/symlink/<name>.pdf` for `lm66100`, `lm66200`, `tps22918`, `tps2051b`
  → HTTP 200, `application/pdf`, 2105188 / 1957146 / 1072926 / 3664348 bytes (2026-11-22).
- **onsemi PDFs answer a bare `curl -sL`, not a browser-header request** (memory of plan-29: 403 to
  the Safari header, 200 to bare `curl`); `fetch.js` tries both.
- **A listing piece is complete below the cap.** `list.js` reads a piece of 300 parts or fewer in
  full and splits a bigger one by the facet whose largest value is smallest
  (`plan-54-data/census/list.js`: `splitFacet`); plan-54-A recorded 0 `cut at` lines the same way.

## 3. Design

### 3.1 The listing

`census/common.js` holds `FAMILIES` (the three DigiKey filter sets of `families.js`) and the
filter resolution; `families.js` requires it (its output is unchanged). `census/list.js
<family>-<maker id> <out.jsonl>` is plan-54's `list.js` with the piece's family filter applied: in
stock (`MinimumQuantityAvailable` 1), status Active (0), sorted by `QuantityAvailable` descending,
one JSON line per product (`mpn`, `maker`, `makerId`, `family`, `series`, `stock`, `status`,
`datasheet`, `description`, `params`). `census/list.sh` lists the 14 pieces: `ideal-296`,
`ideal-505`, `ideal-175`, `ideal-1589`, `load-296`, `load-488`, `load-600039`, `load-150`,
`load-576`, `limited-296`, `limited-14`, `limited-31`, `limited-150`, `limited-576`.

### 3.2 `candidates.tsv`

Columns: `family` (`ideal`, `load`, `limited`), `company` (`ti`, `analog-devices`,
`monolithic-power-systems`, `onsemi`, `microchip`, `diodes`: a company's DigiKey labels together),
`mpn`, `series`, `stock`, `status`, `package` (DigiKey `Package / Case`), `supplier_package`,
`grade` (DigiKey `Grade`: `Automotive` or empty), `datasheet_url` (DigiKey's link), `maker_url`
(§ 3.3), `state` (`candidate` or `out`), `reason`, `pdf`, `hit`, `dead`, `stock_checked`.
`select.py` writes it from the listing; a part listed under two labels or two packings is one
row (the first), the rest `out`, reason `duplicate`. A part is `out`, reason `no datasheet`, when
§ 3.3 gives it no maker URL. Automotive-grade, military and every other kind of part stay
candidates; C to E set the `flags`.

### 3.3 Datasheet sources

`maker_url` and the download follow plan-42-A § 4's rules, per company:

1. **Texas Instruments:** a link on `ti.com` whose `gotoUrl` ends `lit%2Fgpn%2F<name>` →
   `https://www.ti.com/lit/ds/symlink/<name>.pdf`; a Rochester (`rocelec.widen.net`) or DigiKey
   (`mm.digikey.com`) link, or none → the same URL from the part's base name, lowercased, cut at the
   first `/`, then at its leading letters-and-digits run when the first does not answer.
2. **Microchip, Analog Devices (including Maxim and Linear Technology), Monolithic Power Systems,
   Diodes Incorporated:** a link on the maker's own host (`microchip.com`, `ww1.microchip.com`,
   `analog.com`, `maximintegrated.com`, `monolithicpower.com`, `diodes.com`) is fetched as given;
   a Microchip row with none → `https://ww1.microchip.com/downloads/en/DeviceDoc/<base>.pdf`.
3. **onsemi (and Fairchild):** a link on `onsemi.com` or `fairchildsemi.com` is fetched as given,
   with a bare `curl -sL` when the browser-header request answers 403.
4. Anything else → no PDF; `sibling.py` then looks for the part number verbatim in a downloaded PDF
   of the same company, and that PDF is its datasheet; else the row is `out`, reason `no datasheet`
   with the link's host.

`datasheets.json`'s `url` must be the maker's own site and the same document as the local PDF
(`docs/parts-format.md` § Datasheets), so a distributor copy is never a `maker_url`.

### 3.4 Placement

`place.py` copies each fetched PDF to `datasheets/powerswitch/<company>/<name>.pdf`, `<name>` the URL
path's last segment (query dropped, `%20` → `-`, `.pdf` added when missing, characters outside
`A-Za-z0-9._-` replaced by `-`; a clash gets `-<first 6 hex of the SHA-256>`), and writes
`datasheets/powerswitch/datasheets.tsv`. Two URLs with one SHA-256 share one file. A PDF no candidate
row needs is not copied.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The listing

- [x] `census/common.js`, `census/list.js`, `census/list.sh` (§ 3.1); `families.js` requires
      `common.js`. (`scripts/digikey.js` exports its login and request path when `require`d, so the census
      scripts get its Safari fallback: Correction 1; `--opt2` is `list.js`'s and `list.sh`'s second argument.)
- [x] Run `census/list.sh planning/plan-55-data/raw` (30 to 40 calls, within one quota day);
      `gzip -9` the `.jsonl` files. Observed: `zsh planning/plan-55-data/census/list.sh planning/plan-55-data/raw --opt2` → `all 14 pieces done`; 1036 parts, 32 calls, no `cut at` (Corrections 2 and 3).
- [x] `census/common.py`, `census/select.py` (§ 3.2): the first pass, no datasheet judgement yet; `--carry`
      keeps the `pdf`, `hit`, `dead` and `stock_checked` of the existing `candidates.tsv` when it is rewritten
      (plan-42-J's Correction 1: a rewrite without it dropped them). Observed: `select.py raw /tmp/p55-c0.tsv --report` → `listed: 1036 (planning census pick sum 1037)`, `ideal: 90 (pick sum 90)`, `load: 241 (pick sum 241)`, `limited: 705 (pick sum 706)`; 1031 candidates, 5 `out` (no stock: the listing gives those five no quantity).

Acceptance: every piece is listed in full.
  Check: `grep -h 'list.js:' planning/plan-55-data/raw/*.err | awk '{s+=$6} END{print s}'` → at least 30 and at most 40 calls (est. < 1 min); `grep -h 'cut at' planning/plan-55-data/raw/*.err | wc -l` → 0; `python3 -I planning/plan-55-data/census/select.py planning/plan-55-data/raw /tmp/p55-c0.tsv --report` → `listed:` equal to the sum of the nine `pick` counts of `families.out` plus any part DigiKey added since, `ideal`, `load` and `limited` lines each within their `pick` sums (est. 2 min).
Commit: c8b2f11d

### Phase 2 — The datasheet sources, the stock re-check

- [x] `census/urls.py` → `urls-needed.tsv`: the distinct (maker URL, download source) pairs of the
      rows; `census/urlcheck.py` → `url-check.tsv`: which sources answer a PDF (one request a second
      per host). Observed: 679 chains; `url-check.tsv` 410 URLs: 196 ti.com, 12 analog.com, 50 diodes.com, 26 onsemi.com, 34 microchip.com answer a PDF; 85 ti.com and 1 diodes.com answer 404; 5 monolithicpower.com and 1 microchip.com answer a page, not a PDF (Correction 4).
- [x] `census/stock-check.sh` (adapted from plan-44-A): `census/list.sh planning/plan-55-data/raw-recheck` (30 to 40 calls),
      then `select.py --stock` marks every row found again (`stock_checked` 1) and a row not found `out`,
      reason `stock check`. Needs quota `left` ≥ 40 on its own day. Observed: `stock-check.sh … --opt2` → 30 calls (the facets files are copied, saving two), no `cut at`, `stock-check: 1031 rows, 1030 found again`; the one not found is `MIC2077-2BWM TR` (`out`, `stock check`).

Acceptance: every candidate row has `stock_checked` 1 or is `out`.
  Check: `python3 -I planning/plan-55-data/census/select.py planning/plan-55-data/raw planning/plan-55-data/candidates.tsv --urls planning/plan-55-data/url-check.tsv --stock planning/plan-55-data/stock-check.tsv --report` → exits 0 and prints `candidate` rows each with `stock_checked` 1 (est. 3 min). Observed (with `--carry` added): `candidate: 1030; out: no stock 5, stock check 1`; `candidate rows with stock_checked 1: 1030`.
Commit: 33ca1d56

### Phase 3 — Download, place and scan the datasheets

- [x] `census/fetch.js` (§ 3.3's rules, 4 at a time, one retry, a `%PDF-` check and `pdfinfo` on the
      body) → `/tmp/p55/pdf/`; `census/place.py` (§ 3.4) → `datasheets/powerswitch/<company>/` and
      `datasheets.tsv`. Observed: `fetch.js /tmp/p55-urls.tsv /tmp/p55/pdf` → `318 urls; 318 are PDFs; 0 failed`; `place.py` → `264 PDFs placed for 318 URLs`, `place.py --verify` → `0 missing, 0 unused`.
- [x] `census/textscan.py`: `pdftotext` each PDF and set, per candidate row, `hit` 1 when the part number
      (or the ordering-table form of its base) is in the text; `census/sibling.py` for a row with no
      PDF of its own (§ 3.3 rule 4); a row still without a PDF is `out`, reason `no datasheet`,
      `dead` 1. Observed: `textscan.py` → `883 candidate rows with a PDF; 776 print the part number; 107 do not`; `textscan.py --sibling` (the sibling step is a mode of `textscan.py`, Correction 7) → `147 candidate rows without their own PDF; 36 printed in a same-company PDF; 111 out`.
- [x] `census/specs-skeleton.py` → `specs.tsv`: one row per (company, series, PDF) of the
      candidate rows, the columns of plan-55-C § 3.1, the pinout columns empty. Observed: `372 rows` (per (company, series, pdf, package, supplier_package): Correction 8).

Acceptance: every candidate row has a PDF in the tree or is `out`.
  Check: `python3 -I planning/plan-55-data/census/select.py … --report` (as Phase 2) → `candidate` rows all with `pdf` non-empty and `hit` filled (observed: `candidate: 919; out: no datasheet 111, no stock 5, stock check 1`, 919 with pdf, 919 with hit); `find datasheets/powerswitch -iname '*.pdf' \| wc -l` equals `awk -F'\t' 'NR>1 && $1!=""' datasheets/powerswitch/datasheets.tsv \| wc -l` (est. 20 min: the downloads at one request a second per host are the cost, and no smaller check covers a download).
Commit: (filled by the next commit)

## Validation Plan

- Tests: the scripts' `--report` output is the check; no backend or app test is touched.
- Coverage check: `select.py --report` prints the candidate count per (family, company) next to the
  census's `pick` counts, so a piece that lost parts shows.
- Runtime proof: none (data only).
- Doc sync: `planning/plan-55-data/README.md` lists every script and file B adds.
- Final gate: plan-55-K.

## Decisions

- Owner, 2026-11-22 (plan-55-A § Decisions): the three makers per family, Active only, no
  generic part.
- Author: **every in-stock Active part of a picked piece is a candidate**, with no per-maker quota
  (plan-54's rule): the families are small enough that the verdict letters, not a quota, cut them.
- Author: **the stock re-check is a second listing**, not one call per part: no piece is cut at the
  cap (Phase 1's check), so a listing sees every row.
- Author: **Automotive and military parts stay candidates**; the library flags them
  (`docs/parts-format.md` § Parts, Flags).

## Corrections

1. **DigiKey's API answers scripts with a Cloudflare challenge since 2026-11-23** (`scripts/digikey.js` header; the memory `digikey-credentials`): `list.js` as plan-54 wrote it (plain `fetch`) would fail. `scripts/digikey.js` now `module.exports` its `credentials`, `token` and `fetchRetry` (the request path with the Safari fallback) when `require`d (the command line is unchanged: `require.main === module`), `census/common.js` uses them, and `list.js`, `list.sh` and `families.js` take `--opt2`. The token cache is `digikey.js`'s (one file per key).
2. **The split must be an exact partition.** plan-54's `splitFacet` took the facet whose largest value is smallest; for the 363-part Texas Instruments current-limited piece that was a facet (`Grade`, id 1977) that only 57 of the 363 parts have a value for, so the first run listed 57 parts and no `cut at` line. `splitFacet` now takes only a facet whose values sum to the piece's count (and the Series facet likewise) and, among those, the one that reads the piece in the fewest calls (a value of n parts costs ceil(n / 50), at least 1): `Switch Type` (279 + 83) → 9 calls; the first exact-partition version chose by smallest largest value and took 38 calls over 34 sub-pieces. The spent calls: 26 + 38 + 9 on the way, 32 in the kept run (the check's sum counts only the kept `.err` files).
3. **The listing is 1036 parts, not 1037**: DigiKey's Texas Instruments current-limited count went from 363 to 362 since planning (`limited: 705`); the checks read "each within their `pick` sums".
4. **Monolithic Power Systems' datasheet links are HTML pages behind a bot check** (`www.monolithicpower.com/en/documentview/…` answers 200 `text/html`, a challenge page, to `curl` and to Python): its 9 rows have no maker PDF a script can fetch, so by § 3.3 rule 4 (no same-company PDF either) they end `out`, reason `no datasheet` in Phase 3. The Microchip row whose `ww1.microchip.com` link is not a PDF is the same case.
5. **The re-check ran the same day as the listing** (2026-11-23), not on its own day: the quota was not the constraint (key 2 had 677 calls left), and the second listing still sees stock changes (one part gone).
6. **The chains of `urls.py` carry the alternatives**: a TI part's three document names are tried in order by `urlcheck.py` and `select.py --urls` takes the first that answered (the plan's "(maker URL, download source) pairs").
7. **`sibling.py` is `textscan.py --sibling`** (one script shares the text cache and the matching); the obsolete-word scan of plan-44's `textscan.py` is not carried over: a part DigiKey lists Active is not dead, and `dead` 1 means only "no datasheet we can fetch" (C's skip reason 1).
8. **`specs.tsv` is keyed (company, series, pdf, package, supplier_package)**, not (company, series, PDF): DigiKey's `series` is `-` for nearly every part and one PDF covers several packages whose pinouts differ, so a row per PDF would not carry a `package`. C to E split a row further when one package has two pin assignments or two enable polarities.
9. **The PDFs (362 MB, 264 files) stay untracked until plan-55-E's prune**: committing them now would keep ~300 MB of PDFs no `add` row cites in the repository's history for good. `datasheets.tsv` and the check read the working tree; E prunes to the `add` rows' PDFs and commits those with `git add`. Texas Instruments rows whose names found no `ti.com` document (44 rows) and Monolithic Power Systems' 9 (Correction 4) are `out`, `no datasheet`: 111 rows in all.
10. **`datasheets.tsv` is one line per file** (its first URL), so the Phase 3 check's two counts agree (264 and 264); a second URL of the same document is only in `candidates.tsv`'s `maker_url`.

## Summary

B is quota and bandwidth, not design: nothing it does can change a part file. The risk is a
datasheet link that answers a page instead of a PDF; the `%PDF-` check and `dead` 1 keep such a row
out.
