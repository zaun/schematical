# plan-33-A: Capacitors — census, datasheet downloads, first decisions

Last updated: 2026-11-11
Overall Effort: huge (> 3d) — plan-33 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-33 adds capacitors to the parts library and the app: ceramic,
aluminum electrolytic (with aluminum polymer) and tantalum (with tantalum
polymer and niobium oxide) capacitors from the three makers with the most
capacitors in stock at DigiKey (KEMET, Cal-Chip Electronics, KYOCERA AVX),
plus a `GENERIC-CAPACITOR` (ceramic disc, 0.1 µF by default, any value).
They sit in the parts drawer under **Capacitors → Ceramic / Electrolytic /
Tantalum → <capacitance>**. The owner wants 1k–2k parts and no
near-duplicates, so the census does not take every in-stock part (50948 of
them): it takes **one part per bucket**, a bucket being (type, package,
E6 capacitance, and the dielectric class for ceramics or the rated voltage
for the polarized types) — 1048 buckets (§ 2).

The single behavioral outcome of plan-33: loading the part files gives the
baseline 26729 parts (P0, § Prerequisites) plus `GENERIC-CAPACITOR` plus one
`capacitor` part per `add` row of `planning/plan-33-data/candidates.tsv`
(at most one per bucket, at most 1048); the app lists them under the three
drawer leaves in capacitance order; a placed generic capacitor is `C1`,
0.1 µF, on a disc footprint; a 6.3 V tantalum on a 12 V net is refused by
the voltage check; `ctest` passes T0 + 1 programs (`capacitor_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | census (one quota day for the rest of the buckets), selection order, datasheet downloads, verbatim scan, placement, `specs.tsv` skeleton | large |
| B | ordering-code schemes per datasheet, the pick walk (`add` / `alt` / `skip` / `out`), skipped parts, obsolete PDFs, `fpgroups.tsv`, `footprints-needed.txt` | large |
| C | footprints: chips (MLCC, molded tantalum / polymer / niobium, polymer 7343), the shared capacitor footprint rules, the four generic chip footprints | large |
| D | footprints: leaded and can parts (radial cans, SMD cans, leaded MLCC, disc, axial, dipped tantalum), the two generic leaded footprints | large |
| E | backend: the `capacitor` kind, the over-rating voltage check, `capacitor_test`, docs | large |
| F | parts: the generator, `GENERIC-CAPACITOR`, ceramic capacitors, goldens | large |
| G | parts: electrolytic and tantalum capacitors, goldens | medium |
| H | app: drawer order, symbol, facts, `kindName`, `C<n>` references, the generic's Capacitance field, file `farads`, BOM | large |
| I | landing tier | small |

Outcome of A: `datasheets/capacitor/<maker>/` holds every maker-site PDF a
candidate row of rank ≤ 10 cites, with `datasheets/capacitor/datasheets.tsv`;
`candidates.tsv` lists every bucket's 50 most-stocked parts in pick order
(`rank`), with `pdf`, `hit` and `dead` filled; `specs.tsv` has one row per
PDF, its scheme columns empty (B fills them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-33-data/` — `census/queries.mjs`, `buckets.mjs`,
  `buckets.py`, `bucket-parts.mjs`, `select.py`; `buckets.jsonl`,
  `facets.jsonl`, `buckets.tsv`, `raw/ceramic-2026-11-11.jsonl.gz`,
  `candidates.tsv`, `footprints-needed.txt`, `README.md` (written at
  planning time, 2026-11-11).
- `planning/completed/plan-29-A-diode-data.md` and `planning/plan-29-data/census/`
  (`fetch.py`, `textscan.py`, `place.py`, `specs-skeleton.py`: the scripts
  A adapts).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p33 && cmake --build /tmp/logic-p33 -j8 && ctest --test-dir /tmp/logic-p33` → `100% tests passed` | MET 2026-11-11 at 8b1770b: `100% tests passed, 0 tests failed out of 30`; **T0 = 30** (`grep -c '^logic_test' backend/CMakeLists.txt` → 30). Re-run at execution (656b6d1, worktree-P-33, build dir `/tmp/logic-p33wt`, § Corrections 1): `100% tests passed, 0 tests failed out of 30`; 30 |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` | MET: **P0 = 26729**, **R0 = 26729**, **F0 = 1212** (2026-11-11 at 8b1770b; re-run at 656b6d1: 26729, 26729, 1212) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p33-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-11 at 8b1770b; re-run at 656b6d1: `** BUILD SUCCEEDED **`) |
| No capacitor support yet | `grep -c 'PART_CAPACITOR' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/capacitor.json` → no such file; `ls datasheets/capacitor` → no such directory | MET (2026-11-11 at 8b1770b: 0, no file, no directory; re-run at 656b6d1: the same) |
| No other open plan adds capacitors | `grep -l -i '"capacitor"\|PART_CAPACITOR\|datasheets/capacitor' planning/plan-*.md \| grep -v plan-33 \| wc -l` → 0 | MET (2026-11-11: 0; re-run at 656b6d1: 1, `plan-34-E-resistor-kind.md`, whose only hit (:82) says plan-33-E appends `PART_CAPACITOR` first: a coordination note, not a plan that adds capacitors) |
| DigiKey credentials and quota | `DIGIKEY_CLIENT_ID`/`_SECRET` exported in `~/.zshrc`; Phase 1 needs at most 779 calls (775 unlisted buckets, one call each, plus one re-login retry per 10 minutes of run; § 2), a day allows about 1000 (`scripts/digikey.js --help`), shared with other sessions | MET (2026-11-11: credentials present; planning used 649 calls; at execution the probe printed `999 left today`). Start Phase 1 on a day whose first call reports ≥ 800 left (`scripts/digikey.js --category 67 --manufacturer 478 --in-stock --pages 1 --json` prints `… N left today`) |

**Re-run after the landing (2026-11-11, main at 33e64ca, every row's command):**
backend tests `100% tests passed, 0 tests failed out of 31` (`grep -c '^logic_test'` → 31: T0 30 +
`capacitor_test`) — MET; golden sizes 27761 / 27761 / 1352 (P0 / R0 / F0 + plan-33's 1032 / 1032 / 140) —
MET; app `** BUILD SUCCEEDED **` — MET; "no capacitor support yet" → `PART_CAPACITOR` 1, `capacitor.json`
present, `datasheets/capacitor/` present — the entry gate's world, now changed by this plan as intended; "no
other open plan adds capacitors" → `plan-34-E-resistor-kind.md` only, whose hit (:82) orders its kind after
`PART_CAPACITOR` — MET; DigiKey quota — not needed after Phase 1 (no later letter calls DigiKey).

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-33-data/raw/` (gzipped) holds one listing line per bucket
  of `buckets.tsv` (1048), each the bucket's 50 most-stocked in-stock
  Active / Last Time Buy parts of the three makers (fewer when the bucket
  has fewer).
- `candidates.tsv` has one row per (bucket, listed part), `rank` in pick
  order (§ 3 rule 2), and no row other than `candidate` or `out`.
- `datasheets/capacitor/<maker>/` holds each maker-site PDF cited by a
  candidate row of rank ≤ 10; `datasheets/capacitor/datasheets.tsv` maps
  each file to its URL, size and SHA-256.
- `specs.tsv` has one row per placed PDF, the scheme columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is decided `add` here; B walks the buckets.
- No datasheet hunting on maker sites beyond the DigiKey link (§ 3 rule 3).

## 2. Current State

The library has no capacitor: no `capacitor` kind, no capacitor footprint
(`grep -c '^CAP' backend/tests/golden/footprints.txt` → 0), no
`datasheets/capacitor/`. The pull resistors are the only passives
(`supply.json`, `"pull": true`), with generic chip / axial / radial
footprints `RES-*` (plan-26-A § 4).

### Measured populations

All DigiKey numbers 2026-11-11, in stock (`--in-stock`, i.e.
`MinimumQuantityAvailable` 1), status Active or Last Time Buy (`--status 0
--status 4`). Category ids (one `GET /search/categories` call): 60 Ceramic,
58 Aluminum Electrolytic, 69 Aluminum - Polymer, 59 Tantalum, 70 Tantalum -
Polymer, 67 Niobium Oxide, 62 Film. Maker ids: 399 KEMET, 4713 Cal-Chip
Electronics, Inc., 478 KYOCERA AVX.

| What | Count | Command |
|---|---|---|
| In-stock capacitors, all makers, per category | 60: 81996; 62: 13853; 58: 27363; 59: 8545; 69: 4454; 70: 1864; 61 (EDLC): 1530; 64 (mica/PTFE): 1122; 66 (thin film): 321; 65 (trimmers): 245; 57 (arrays): 170; 68 (silicon): 60; 67: 34 | `scripts/digikey.js --category <c> --in-stock --status 0 --status 4 --pages 1 --json` (13 calls) → `ProductsCount` |
| Makers ranked by in-stock capacitors (those 13 categories) | KEMET 23921, Cal-Chip 19357, KYOCERA AVX 12420, then TDK 8474, Murata 8286, Nichicon 7073, Chemi-Con 6282, Panasonic 6165 | the same 13 calls, `Manufacturers` facet summed |
| In scope per category, KEMET / Cal-Chip / AVX | 60: 14429 / 18064 / 8440; 58: 1370 / 107 / 219; 69: 686 / 68 / 122; 59: 2485 / 1061 / 2583; 70: 804 / 47 / 429; 67: 0 / 0 / 34 | `scripts/digikey.js --category <c> --manufacturer <m> --in-stock --status 0 --status 4 --pages 1 --json` (18 calls) → `ProductsCount` |
| Total in scope | **50948** (KEMET 19774, Cal-Chip 19347, AVX 11827) | sum of the row above |
| Out of scope by type: film (62) | 4160 (KEMET 4036, AVX 124) | the 13 category calls, `Manufacturers` facet |
| **Buckets** (§ 3 rule 1) | **1048**: ceramic 378 (0402 56, 0603 59, 0805 65, 1206 66, Radial 65, Disc 17, Axial 50); aluminum 199 (Radial 107, SMD 92); aluminum polymer 138 (Radial 67, SMD 71); tantalum 228 (SMD 135, Radial 93); tantalum polymer 85 (SMD); niobium oxide 20 (SMD) | `node census/buckets.mjs buckets.jsonl` (73 calls, one per query of `census/queries.mjs`, `Capacitance` facet) → `python3 -I census/buckets.py buckets.jsonl buckets.tsv` → `total 1048 buckets (29 with more than one DigiKey value id)` |
| …distinct E6 values per type | ceramic 42, aluminum 23, aluminum polymer 16, tantalum 25, tantalum polymer 20, niobium oxide 12 | same `buckets.py` run |
| Ceramic buckets listed at planning time | 273 of 378 (477 calls in two runs: 390, whose last 187 answered HTTP 401, an expired token, and 87, which listed 86 of those again before the day's quota ran out) | `node census/bucket-parts.mjs <the ceramic lines of buckets.tsv> raw.jsonl` (2 runs; `wc -l` of the output → 273) |
| …candidate rows | 9653 (`products` summed: `gunzip -c raw/ceramic-2026-11-11.jsonl.gz \| jq '.products\|length' \| paste -sd+ - \| bc` → 9653) (0 `out`: military 0, wet tantalum 0) | `python3 -I census/select.py raw/ceramic-2026-11-11.jsonl.gz candidates.tsv` → `273 buckets, 9653 rows, 0 out` |
| …largest bucket (`ProductsCount`) | 309 (`ceramic\|0603\|other\|0.1 µF`); buckets with more than 50 parts: 140 | `gunzip -c raw/ceramic-2026-11-11.jsonl.gz \| jq -r .count \| sort -n \| tail -1`; `… \| jq -r 'select(.count>50)\|.count' \| wc -l` |
| …distinct datasheet URLs, rank ≤ 10 rows | 52 (rank 1 only: 15) | `awk -F'\t' '$2=="ceramic" && $7!="" && $7<=10 {print $23}' candidates.tsv \| sort -u \| wc -l`; `$7==1` for rank 1 |
| …rank-1 rows by datasheet host | calchip.com 201, yageogroup.com 64, datasheets.kyocera-avx.com 6, ele.kyocera.com 1, search.kemet.com 1 (rank ≤ 10 adds waldomemea.com 17, content.kemet.com 5, mm.digikey.com 3) | the same rows, `$23` host |
| …rank-1 rows by maker | Cal-Chip 201, KEMET 65, AVX 7; 1 automotive | the same rows, `$9` |
| Not yet listed | 775 buckets (ceramic 105: Radial 65, Disc 17, Axial 23; aluminum 199; aluminum polymer 138; tantalum 228; tantalum polymer 85; niobium oxide 20) | `buckets.tsv` lines minus the raw lines |
| Listing speed | 23.5 buckets per minute (one call each, 50 parts with all parameters) | the second ceramic run: 87 calls (86 buckets, then HTTP 429) in 3 min 42 s (`stat` of its input and log files, 13:26:15 → 13:29:57) |
| Phase 1 call budget | 779 at most: one call per unlisted bucket (775), plus one retry per token renewal (a token lasts 10 minutes; the run takes 35, so at most 4) | the two rows above |

The 300-result cap: **no listing hits it.** `bucket-parts.mjs` asks for one
page (`Offset` 0, `Limit` 50) per bucket, sorted by stock; DigiKey's cap
limits `Offset + Limit` to 300, which a single first page never reaches,
and the pick rule never needs more than the 50 most-stocked parts unless
every one of them fails (B § 3: then the bucket's next pages, `Offset`
50–250, are fetched one page per call in stock order, still under the cap). No
census count comes from paging, so nothing needs a part-by-part re-check.

### Verified properties

- **Sorting by stock works with parametric filters.** `bucket-parts.mjs` on
  three test buckets (`ceramic|0603|other` 0.1 µF, `aluminum|Radial|16 V`
  100 µF, `ceramic|Disc|other` 10000 pF; 3 calls) returned parts in
  descending `QuantityAvailable` (85544000, 85544000, …; 42483, 28743, …;
  13998, 12000, …) with `ProductsCount` 309, 11, 13.
- **One capacitance, several value ids.** DigiKey files 10 nF under `0.01
  µF`, `10 nF` and `10000 pF`, and 1000 µF under `1 mF` and `1000 µF`;
  `buckets.py` merges them (29 buckets) and `bucket-parts.mjs` sends all of
  a bucket's ids in one filter (OR). The first planning run had listed 16
  single-id pieces of merged buckets; they were dropped and re-listed.
- **Tokens expire after 10 minutes.** The first ceramic run got HTTP 401
  for its last 187 buckets; `bucket-parts.mjs` now logs in again on a 401
  and retries the bucket once.
- **Downloads.** `curl -sL` gets `200 application/pdf` from yageogroup.com
  (KEMET; the URLs have no `.pdf`: `…/KEM_A4009_ESW` 3350259 bytes) and
  calchip.com (`GMC-Series-3.pdf` 1447882 bytes); datasheets.kyocera-avx.com
  answers a bare curl with `406 text/html` and any `Mozilla/5.0` user agent
  with `200 application/pdf` (`cx5r.pdf`), including
  `scripts/check-datasheets.js`'s.
- **Ordering tables are part-number schemes**, read with `pdftotext
  -layout`, page by page: Cal-Chip `GMC-Series-3.pdf` "PART NUMBER GUIDE"
  (p.2); KEMET `KEM_C1127_CGP` "Ordering Information" (p.2) and
  `KEM_A4009_ESW` "Part Number System" (p.1) plus "Table 1 – Ratings & Part
  Number Reference" (base numbers with a `(1)` suffix placeholder,
  `ESW107M010AC3(1)`); AVX `cx5r.pdf` "PART NUMBER (SEE PAGE 4 FOR COMPLETE
  PART NUMBER EXPLANATION)" (p.1). None prints its sampled in-stock part
  verbatim on any page (`GMC10X7R104K50NT`, `CGP5C103MDWDAA7301`,
  `ESW107M016AE3EA`, `01016D101KAT2A`: 0 pages each), so B decodes schemes
  (B § 3).
- **Status ids**: `--status 0 --status 4` is Active + Last Time Buy
  (plan-29-A § 2's facet reading; the same DigiKey ids).

## 3. Rules

### Rule 1 — buckets

A bucket is one query of `census/queries.mjs` plus one capacitance:

| Type (`type` column) | DigiKey category | Query (package filter) | Third key | Capacitances (E6: 1.0 1.5 2.2 3.3 4.7 6.8) |
|---|---|---|---|---|
| `ceramic` | 60 | Package / Case 0402, 0603, 0805, 1206, Radial, Radial Disc, Axial | `C0G` (C0G, NP0) or `other` (every other temperature coefficient) | 10 pF – 100 µF |
| `aluminum` | 58 | Radial, Can (`Radial`) / Radial, Can - SMD (`SMD`) | rated voltage 6.3 10 16 25 35 50 63 100 V | 0.1 – 4700 µF |
| `al-polymer` | 69 | Radial, Can / Radial, Can - SMD or 2917 | the same voltages | 0.1 – 4700 µF |
| `tantalum` | 59 | SMD: Surface Mount, Molded, cases 0603 0805 1206 1411 2312 2917; Radial | rated voltage 4 6.3 10 16 20 25 35 50 V | 0.1 – 1000 µF |
| `ta-polymer` | 70 | Surface Mount, the same cases | the same voltages | 0.1 – 1000 µF |
| `niobium` | 67 | (all are SMD) | 4 6.3 10 V | 0.1 – 1000 µF |

Out by construction: every other package (0201, 1210, 1812 and larger
chips; snap-in, screw-terminal, solder-lug, press-fit and axial cans;
surface-mount tantalums that are not molded, such as conformal-coated
and hermetic ones; axial tantalums), every other voltage, every
non-E6 value. Each bucket keeps at most one part (B).

### Rule 2 — pick order (`census/select.py`, `rank`)

Within a bucket: not automotive (no AEC-Q200 in Ratings) before automotive;
for ceramic `other` buckets X7R, then X5R, then the rest; for ceramics the
lowest rated voltage at or above 50 V, then the highest below 50 V; more
DigiKey stock; part number. Military parts (Ratings or Grade naming MIL)
and wet tantalums are `out` before ranking. `flags` is `automotive` for
AEC-Q200 parts.

### Rule 3 — the inclusion rule (applied by B, data gathered here)

A part becomes `add` only when all hold:

1. **In stock and Active or Last Time Buy** at DigiKey (the listing asks for
   nothing else). Obsolete, Discontinued, Not For New Designs and
   made-to-order parts are never listed.
2. **A maker datasheet**: `datasheet_url` on the maker's own site
   (yageogroup.com or any kemet.com host for KEMET, calchip.com for
   Cal-Chip, kyocera-avx.com, avx.com or kyocera.com for AVX) and downloaded as a
   PDF (a link that answers HTML fails); else
   `skip`, reason `no datasheet on the maker's site (<host or none>)`.
   (`ti.com/product` 404s do not apply: no TI part is in scope.)
3. **In the datasheet's ordering table**: printed verbatim (`hit` 1), or a
   full match of the datasheet's part-number scheme whose decoded
   capacitance and voltage equal the bucket's (B § 3).
4. **Not marked obsolete, discontinued, NRND or end of life** in the
   datasheet (`dead`, read as plan-29-A § 3 rule 5).

### Rule 4 — placement (`census/place.py`, from plan-29)

`datasheets/capacitor/<maker-slug>/<name>.pdf`, slug `kemet`, `calchip`,
`avx`; `<name>` the URL path's last segment, `.pdf` appended when missing
(KEMET's `KEM_C1003_C0G_SMD` → `KEM_C1003_C0G_SMD.pdf`), characters outside
`A-Za-z0-9._-` replaced by `-`; a clash gets `-<first 6 hex of the md5>`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The census (one quota day)

- [x] `mkdir -p /tmp/p33/raw && gunzip -c planning/plan-33-data/raw/ceramic-2026-11-11.jsonl.gz > /tmp/p33/raw/parts.jsonl`,
      then (DIGIKEY_* loaded from `~/.zshrc` in a script file, as
      `planning/plan-33-data/README.md` shows) `node
      planning/plan-33-data/census/bucket-parts.mjs
      planning/plan-33-data/buckets.tsv /tmp/p33/raw/parts.jsonl`. It skips
      listed buckets and stops on HTTP 429; re-run the next day until done.
      Done in one day: run 1 → `777 calls, 225 calls left today`, 1047
      buckets (one answered HTTP 500, `ta-polymer|SMD|6.3 V` 10 µF); run 2
      → `1 calls, 224 calls left today`, 1048.
- [x] Any bucket whose listing now has `count` 0 (stock moved since
      2026-11-11) is recorded in § Corrections; it gets no part. None:
      every line has `count` > 0 and at least one product (18111 in all).
- [x] `gzip -c /tmp/p33/raw/parts.jsonl > planning/plan-33-data/raw/parts-<date>.jsonl.gz`
      and delete `raw/ceramic-2026-11-11.jsonl.gz` (its lines are in the new
      file).

Acceptance: every bucket is listed.
  Check: `gunzip -c planning/plan-33-data/raw/parts-*.jsonl.gz | wc -l` → 1048 (est. 35 min of wall time at 23.5 buckets/min; nothing smaller lists the buckets).
  Observed: `1048` (`raw/parts-2026-11-11.jsonl.gz`; about 30 minutes).
Commit: a7ddd66

### Phase 2 — Ranking, downloads and scans

- [x] `python3 -I census/select.py planning/plan-33-data/raw/parts-*.jsonl.gz
      planning/plan-33-data/candidates.tsv` (replaces the planning-time
      ceramic-only file). Observed: `1048 buckets, 18111 rows, 2 out`
      (both bi-polar, § Corrections 2).
- [x] `census/fetch.py` (from plan-29, with the AVX user agent of § 2: bare
      `curl -sL` first, then `-A 'Mozilla/5.0 (Macintosh)'`): download every
      distinct `datasheet_url` of the rows with `rank` ≤ 10 to
      `/tmp/p33/pdf/<md5>.pdf`. Observed: 231 URLs, 212 PDFs (`fetch.py
      --verify` → `231 urls, 212 PDFs, 0 untried`).
- [x] `census/textscan.py` (from plan-29): fill `pdf`, `hit` (part number
      between non-alphanumeric boundaries), `dead` (an obsolete /
      discontinued / NRND / end-of-life word within 300 characters of any
      hit) for the rank ≤ 10 rows. Observed: `6638 rows scanned, 6573 with
      a PDF, 378 hit` (3 `dead`).
- [x] Record in § Corrections: rows per type, the distinct URLs, the PDFs
      downloaded, the rows with a maker-site PDF, the `hit` rows.

Acceptance: every listed part has a row and every rank ≤ 10 URL was tried.
  Check: `awk 'END{print NR-1}' planning/plan-33-data/candidates.tsv` equals the summed `products` of the raw lines (`gunzip -c raw/parts-*.jsonl.gz | jq '.products|length' | paste -sd+ | bc`), and `fetch.py --verify` → `0 untried` (est. 1 h, the downloads).
  Observed: 18111 = 18111; `231 urls, 212 PDFs, 0 untried`.
Commit: a7ddd66

### Phase 3 — Place the datasheets

- [x] `census/place.py` (from plan-29, `.pdf` matched case-blind, plan-29-A
      § Corrections) for every rank ≤ 10 row with a maker-site PDF;
      `git add` only `datasheets/capacitor/`. Observed: `186 PDFs placed
      for 195 URLs`.

Acceptance: every such row's PDF is in the tree.
  Check: `python3 -I planning/plan-33-data/census/place.py --verify planning/plan-33-data/candidates.tsv` → `0 missing` (est. 2 min).
  Observed: `195 URLs, 186 PDFs, 0 missing, 0 unused`.
Commit: a7ddd66

### Phase 4 — `specs.tsv` skeleton and the README

- [x] `census/specs-skeleton.py`: `planning/plan-33-data/specs.tsv`, header
      `maker datasheet url mpn_regex cap_group volt_group volt_map
      case_group note`, one row per placed PDF, the scheme columns empty.
      Observed: `186 rows`.
- [x] `planning/plan-33-data/README.md`: the file map (A's files, and
      the files B–F add).

Acceptance: one spec row per placed PDF.
  Check: `ls datasheets/capacitor/*/*.pdf | wc -l` equals `awk 'END{print NR-1}' planning/plan-33-data/specs.tsv` (est. 1 min).
  Observed: 186 = 186.
Commit: a7ddd66

## Validation Plan

- Tests: none (data only); the scripts' `--verify` modes.
- Coverage check: Phase 1's 1048 = 1048 and Phase 2's row equality.
- Runtime proof: not applicable.
- Doc sync: none in A (B writes `skipped-parts.md` and
  `obsolete-not-included.md`).
- Final gate (once, after Phase 4): `cmake --build /tmp/logic-p33 -j8 &&
  ctest --test-dir /tmp/logic-p33` → `100% tests passed` (est. 1 min).

## Decisions

- **Makers** — owner, 2026-11-11: KEMET, Cal-Chip Electronics, KYOCERA AVX
  (the measured top 3, § 2).
- **Types** — owner, 2026-11-11: Ceramic, Electrolytic (aluminum and
  aluminum polymer), Tantalum (tantalum, tantalum polymer, niobium oxide).
  Film, supercapacitors, silicon, thin-film, mica / PTFE, arrays, trimmers
  and feed-through capacitors are out.
- **Drawer** — owner, 2026-11-11: **Capacitors → type → value** (e.g.
  Capacitors → Ceramic → 100 nF).
- **Generic capacitor** — owner, 2026-11-11: `GENERIC-CAPACITOR`, ceramic
  disc footprint by default, **0.1 µF** by default (the to-do's "0.1pf" was
  0.1 µF), value editable per placed part, footprint choices 0402, 0603,
  0805, 1206, disc, radial (plan-33-F § 3.4).
- **Size: 1k–2k parts, no near-duplicates** — owner, 2026-11-11. Settles
  rule 1 (one part per bucket) and rule 2 (which one). Rejected: every
  in-stock part (50948), and E12 values (more buckets of near-equal values).
- **Inclusion rule** — owner, 2026-11-11: in stock, in the datasheet's
  ordering table, Active or Last Time Buy (§ 3 rule 3).
- **Automotive parts only as a fallback, flagged** — author: an AEC-Q200
  part is a near-duplicate of its commercial twin; it is taken only when a
  bucket has no commercial part that passes, and is flagged `automotive`
  (the drawer hides those by default). Military parts are out.
- **Ceramic voltage: 50 V first** — author: 50 V is the commonest MLCC
  rating (`Voltage - Rated` facet, category 60: 50V 25279 of 81996, the
  largest value) and safe on every logic rail.
- **No part-editor form for the `capacitor` kind** — author: plan-32 (open)
  covers the 26 existing kinds (its owner decision); plan-33 does not edit
  plan-32's scope. plan-32-C's test that compares `PartDraft.kindNames` with
  the `Parts/*.json` file names sees `capacitor.json` once plan-33-E lands
  (noted in plan-32-A § Corrections by this commit).
- **Maker-site datasheets only** — author: `docs/parts-format.md`
  § Datasheets. yageogroup.com counts as KEMET's own site (Yageo owns
  KEMET; DigiKey's KEMET links point there).
- **Plan number** — author: `plan-33` was free on 2026-11-11 (highest plan
  in `planning/` and `planning/completed/` was `plan-32`; `git log --all
  --oneline | grep -ciE 'plan[- ]33'` → 0; re-checked before writing and
  before committing).

## Corrections

1. **Build directory** (2026-11-11). `/tmp/logic-p33` had been configured
   from the main checkout; the worktree builds in `/tmp/logic-p33wt` (the
   same commands). Every later letter uses it.
2. **Bi-polar electrolytics are `out`** (select.py). Cal-Chip's CVB series
   is bi-polar ("Bi-Polar", `CVB-SERIES.pdf` p.1), and the kind's
   `aluminum` type is polarized (`+`/`-`, plan-33-E § 3.2), so a bi-polar
   part cannot be one. `select.py` makes a row `out`, reason `bi-polar`,
   when its series is in `BIPOLAR` (`CVB`) or its description ends in `BP`
   / `NP`: 2 rows (`CVB-1V2R2MD55-R`, twice). Every rank ≤ 10 electrolytic
   PDF was searched for `bi-?polar|non-?polar`: the only other hit is
   Arcotronics' catalog on mm.digikey.com (not a maker-site PDF; its KEMET
   ESX rows' own datasheet, `KEM_A4073_ESX`, is a polarized series).
3. **Phase 2 counts.** Rows per type: ceramic 11432, aluminum 769,
   al-polymer 393, tantalum 4443, ta-polymer 1040, niobium 34 (18111).
   Distinct URLs at rank ≤ 10: 231; downloaded as PDFs: 212 (the 19 others:
   `content.kemet.com`, whose TLS handshake never completes — `curl -v`
   stops after "Client hello" — 16, two cloudfront links, and one
   `connect.kemet.com:7667` link that answers HTTP 500); rows with a
   maker-site PDF: 6533; verbatim `hit` rows: 378 (365 of them on a
   maker-site PDF); `dead`: 3.
4. **One DigiKey 500.** Run 1 got HTTP 500 ("ServiceCallout SC-Quota
   failed") for `ta-polymer|SMD|6.3 V` 10 µF and went on; run 2 listed it
   (1 call).
5. **The day's quota** began at 999 (not the ≤ 800 the gate expected);
   Phase 1 used 778 calls.

## Summary

A's risk is volume and the shared quota, not design: 775 buckets
still to list (one call each, about 35 minutes), the PDFs (52 distinct URLs at rank ≤ 10 in the 273 ceramic buckets
alone),
and every ordering table a scheme (B). Left untouched: every part,
footprint, backend and app file.
