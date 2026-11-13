# plan-37-A: Registers — census refresh, datasheets, ordering-table verdicts

Last updated: 2026-11-12
Overall Effort: huge (> 3d) — plan-37 is A → B → C → D → E → F → G → H
Effort: large (3h–1d; one DigiKey quota day for Phase 1)
Depends on: nothing (§ Prerequisites)

plan-37 adds **registers** to the parts library and the app, from the top
three makers of in-stock flip-flops at DigiKey (Texas Instruments, onsemi,
Nexperia). It has two halves:

1. A new part kind, **`register`**, for the multi-bit D registers the
   library does not have yet: the 74x173 class (4-bit, 3-state, two data
   enables, two output enables, reset; CD4076B), the clock-enable registers
   (74x377, 378, 379), the bus-interface and inverting registers (74x534,
   564, 575, 576, 577, 821, 823, 825, 874, 29821, 11374, CD40174B), the wide
   16-, 18-, 20- and 22-bit registers (74x16374, 162374, 16721, 162721,
   16722, 16820, 16821, 16823, 162823). They join the existing parts-drawer
   category **Logic → Registers** (today: the shift registers and the
   74x670).
2. The onsemi and Nexperia parts of the register devices the `dff` kind
   already has (74_174, 74_175, 74_273, 74_374, 74_574, 40175B), added to
   those devices, and the two in-stock SN74LS670 parts the existing `mux`
   device 74_670 (model `regfile`, plan-21) lacks.

The single behavioral outcome of plan-37: loading the part files gives the
baseline 27828 parts (P0, § Prerequisites) plus one part per `add` row of
`planning/plan-37-data/candidates.tsv` (472 in-scope rows at planning time,
before the datasheet verdicts: 272 `register`, 198 `dff`, 2 `mux`). Each `register`
part lists with kind `register` and the function `Register` (or `Register
file`). An SN74LS173AN driven through its function table (D loaded on a
rising CLK with G1 = G2 = 0, held with G1 = 1, cleared by CLR = 1, Z with M
= 1) shows the datasheet's outputs in `register_test`. `ctest` passes T0 +
1 programs (`register_test`). In the app, the drawer shows Logic →
Registers, and the sidebar names the kind "Register".

| Letter | Content | Effort |
|---|---|---|
| **A** | census refresh (incl. TI's 16- and 20-pin packages in full), datasheet downloads, ordering-table verdicts, `skipped-parts.md` / `obsolete-not-included.md` | large |
| B | footprints: the 4 new land patterns (DHVQFN-20, DHVQFN-24, DQFN-20-ONSEMI, TSSOP-64), staged in `footprints-B.json` | medium |
| C | backend: the `register` kind (model `parallel`), the 5 new logic families, `register_test`, docs, the app's kind-list lines and generated Part Editor form | large |
| D | parts: the 17 4- to 10-bit register devices (16-, 20- and 24-pin) and the 2 missing 74_670 parts, goldens | large |
| E | parts: the 9 wide register devices (48-, 56- and 64-pin), goldens | large |
| F | parts: onsemi and Nexperia parts of the 6 `dff` register devices, their new timing families, goldens | large |
| G | app: catalog, drawer filters, `kindName`, sidebar facts, Part Editor form check | medium |
| H | landing tier | small |

Outcome of A: `candidates.tsv` reflects a listing made the day A ran, and
every in-scope row has a verdict: `add`, `skip` with a reason, or `out`
with a reason. Each `add` row names its datasheet PDF, which is in the
tree under `datasheets/register/` (new devices) or `datasheets/ff-d/` (the
`dff` devices; the two 74_670 parts reuse `sn74ls670`), and the
ordering-table line that lists it.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-37-data/` (written at planning time, 2026-11-12, at cf40403):
  `census/list.js`, `facets.js`, `run.sh`, `candidates.py`,
  `footprints.py`; `raw/*.jsonl.gz`, `raw/*.err`, `raw/ti-facets.tsv`;
  `candidates.tsv`; `footprints-needed.txt`; `README.md`.
- `planning/completed/plan-35-A-charlcd-large-data.md` and
  `planning/plan-36-A-mcu-data.md` (the census / verdict pattern A follows).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here and adds only its own rows.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p37 && cmake --build /tmp/logic-p37 -j8 && ctest --test-dir /tmp/logic-p37 -j8` → `100% tests passed` | MET (2026-11-13 00:10 UTC at 31f79ce, A start: `100% tests passed, 0 tests failed out of 32`): **T0 = 32** |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` | MET (A start, 31f79ce: 27828 / 27828 / 1386): **P0 = 27828**, **R0 = 27828**, **F0 = 1386** |
| No register kind yet | `grep -c 'PART_REGISTER' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/register.json` → no such file; `ls datasheets/register` → no such directory | MET (A start: 0, no file, no directory) |
| No other open plan adds registers | `grep -l 'PART_REGISTER\|Parts/register.json\|74_173\|74_377' planning/plan-*.md \| grep -v plan-37 \| wc -l` → 0 | MET (A start: 0) |
| DigiKey credentials and quota | `DIGIKEY_CLIENT_ID`/`_SECRET` exported in `~/.zshrc`. Phase 1 costs 90 calls: the 64 of the planning listing (`README.md`) plus 26 for TI's 16- and 20-pin pieces (§ 2) | Start Phase 1 on a day whose first call reports ≥ 120 left. MET (A start, `facets.js: 1272 parts, 1 call, 998 left today`) |
| A browser-TLS HTTP client for Nexperia | `python3 -m venv /tmp/p37venv && /tmp/p37venv/bin/pip install curl_cffi` → installs; `/tmp/p37venv/bin/python -I -c "from curl_cffi import requests; r = requests.get('https://assets.nexperia.com/documents/data-sheet/74HC_HCT173.pdf', impersonate='chrome'); print(r.status_code, r.headers['content-type'])"` → `200 application/pdf` | MET (A start: `200 application/pdf`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `candidates.tsv` reflects a listing made the day A ran: the planning
  pieces plus TI's 20 16- and 20-pin package pieces. Every in-scope row
  (`scope` = `in`) has stock above 0 and status Active or Last Time Buy at
  DigiKey. A planning row the new listing does not return is `out`, reason
  `stock check`.
- Every in-scope row has `verdict` `add`, `skip` or `out` and, for `skip`
  and `out`, a reason in the `ordering` column. Each `add` row has `pdf`
  (its PDF's path under `datasheets/`) and `ordering` (the ordering-table
  entry, verbatim, that lists it).
- `datasheets/register/` holds every PDF a new-device `add` row names;
  `datasheets/ff-d/` the PDFs of the `dff` rows. `planning/plan-37-data/datasheets.tsv`
  maps each file to its URL, bytes and SHA-256.
- `datasheets/skipped-parts.md` has a `## Registers (plan-37)` section
  with every `skip` row. Every PDF whose rows are all `out` for vendor
  status is deleted and listed in `datasheets/obsolete-not-included.md`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No footprint work (B), no pinout or timing transcription (D–F).
- No device outside the closed list of § 3 rule 2, whatever new rows the
  full TI listing brings: those rows are `out`, reason `not in device list`.
- `scripts/digikey.js` is not changed: the census needs package pieces with
  full parameters per product, so `census/list.js` calls the API itself (the
  plan-36 precedent).

## 2. Current State

The library has registers only inside `dff` (74_174, 74_175, 74_273, 74_374,
74_574, 40175B, plus the TPIC6273s), all Texas Instruments
(`python3 -c` over `dff.json` → every part's `manufacturer` is `Texas
Instruments`). There is no 74x173, '377, '534, '821 or 16-bit register
(`grep -l '"74_173"\|"74_377"\|"74_534"\|"74_821"\|"74_16374"' schematical/schematical/Parts/*.json`
→ no file). The 74x670 register file is the `mux` kind's 74_670 (model
`regfile`, drawer Logic → Registers, 7 TI parts). Logic PDFs live in flat group directories
(`datasheets/ff-d/`, `datasheets/latch/`); `datasheets/register/` is new.

### Measured populations

All DigiKey numbers 2026-11-12: category 706 Flip Flops, in stock
(`MinimumQuantityAvailable` 1). Maker ids 296 Texas Instruments, 488
onsemi, 1727 Nexperia USA Inc. Listing pieces add status Active or Last
Time Buy (`StatusFilter` 0, 4); their summaries are
`planning/plan-37-data/raw/<label>.err`.

| What | Count | Command |
|---|---|---|
| In-stock flip-flops, all makers | 3616 | `node scripts/digikey.js --json --limit 1 --category "Flip Flops" --in-stock` → `ProductsCount` |
| …top makers (in stock, any status) | TI 1646, onsemi 459, Nexperia 428; then Fairchild Semiconductor 355, National 158, Harris 139 | same call, `FilterOptions.Manufacturers` |
| TI, in stock, Active / LTB | 1271 | `node census/facets.js 706 296` → `facets.js: 1271 parts` |
| Nexperia, in stock, Active / LTB | 283 (296 listed, 28 package pieces, none cut at 300) | `raw/nexperia.err` → `list.js: 283 parts, 29 calls` |
| onsemi, in stock, Active / LTB | 255 (257 listed, one piece) | `raw/onsemi.err` → `list.js: 255 parts, 6 calls` |
| TI 24-, 48-, 56-, 64-pin, listed in full | 98 (14 package pieces) | `cat raw/ti-p*.err` → 1+1+21+2+3+2+6+19+7+13+13+2+7+1 |
| TI 16- and 20-pin, listed by device number | 288 rows over the 11 keyword pieces (keywords are fuzzy: `ti-k173` alone returned 183) | `tail -qn1 raw/ti-k*.err` |
| Pieces cut at DigiKey's 300-result cap | 0 | `grep -h 'cut at 300' raw/*.err \| wc -l` → 0 |
| Listed rows (unique maker + part) | **929** | `wc -l < candidates.tsv` → 930 (header) |
| …scope | **in 472**; out: 1-bit flip-flop 177, single/dual/translating gate 122, TI part of an existing `dff` device 93, ECL 61, already in the catalog 3 (SN74LS670D/N/NSR, `mux.json`), not in its ordering table 1 (SN74AS575NT, read at planning: `candidates.py` `PLANNING_OUT`) | `cut -f6 candidates.tsv \| sort \| uniq -c` |
| In-scope `register` rows | **272** (TI 228, Nexperia 31, onsemi 13), 26 devices | `awk -F'\t' '$6=="in" && $5=="register"' candidates.tsv \| wc -l` |
| In-scope `mux` rows (74_670) | **2** (SN74LS670N3, SN74LS670NS) | `awk -F'\t' '$6=="in" && $5=="mux"' candidates.tsv` |
| …4- to 10-bit devices + 74_670 (D) | 198 rows, 17 devices + 74_670 | `planning/plan-37-D-…md` § 2 |
| …wide devices (E) | 76 rows, 9 devices | `planning/plan-37-E-…md` § 2 |
| In-scope `dff` rows (F) | **198** (onsemi 112, Nexperia 86), 6 devices | `awk -F'\t' '$6=="in" && $5=="dff"' candidates.tsv \| wc -l` |
| …automotive (DigiKey Grade / AEC-Q100) | 21 | `awk -F'\t' '$6=="in" && $9=="automotive"' candidates.tsv \| wc -l` |
| Distinct DigiKey base products in scope | 163 (incl. one group of rows with no base number per maker) | `BaseProductNumber` of the in-scope rows, grouped by maker |
| Datasheet URLs DigiKey gives, in scope | 214 distinct: ti.com 65, Rochester (`rocelec.widen.net`) 76, onsemi.com 41, assets.nexperia.com 29, mm.digikey.com 2, none 1 | `awk -F'\t' '$6=="in"{print $16}' candidates.tsv \| sort -u \| awk -F/ '{print $3}' \| sort \| uniq -c` |
| Rows with no DigiKey parameters | 51 (TI 39, onsemi 12); their package is read from the part number (`census/footprints.py` `BY_NAME`) | `awk -F'\t' '$6=="in" && $10==""' candidates.tsv \| wc -l` |
| DigiKey calls the planning census cost | 64 | `README.md` |

TI's 16- and 20-pin packages (the ids in `raw/ti-facets.tsv`: 107808 107879
108614 108624 108656 108681 108695 108891 142706 142714 142845 143005 143702
143707 143724 143726 730773 143742 143757 143890, 615 parts) were listed by
device number because the day's quota ran out. Phase 1 lists them in full
(26 calls: one per 50 parts per piece).

### Verified properties

- **TI.** `https://www.ti.com/lit/ds/symlink/<folder>.pdf` answers `200
  application/pdf` to `curl -sIL` (sn74ls173a, cd74hc173, sn74hc377,
  sn74ls670, sn74alvch16721). `https://www.ti.com/product/<P>` with a
  browser user agent answers 200 for SN74LS173A, CD4076B, SN74LS670,
  SN74LS377, SN74LS378, 74ACT11374 and **404 for SN74173, SN74LS379,
  SN74ACT11374** (so the 74ACT11xxx folders have no `SN` prefix).
- **onsemi.** `https://www.onsemi.com/pdf/datasheet/mc74hc374a-d.pdf` and
  `mc14076b-d.pdf` answer `200 application/pdf`.
- **Nexperia blocks plain HTTP clients.** `curl` (with any user agent and
  browser headers), `assets.nexperia.cn` and `www.nexperia.com/documents/…`
  all answer `403 text/html` (425 bytes); so did the WebFetch tool.
  `curl_cffi` with `impersonate="chrome"` (a throwaway venv in `/tmp`)
  answers `200 application/pdf`, 284256 bytes, `%PDF-` for
  `74HC_HCT173.pdf`.
- **Nexperia ordering tables name type numbers, not orderables.**
  `pdftotext -layout 74HC_HCT173.pdf`: "Table 1. Ordering information" rows
  `74HC173D … SO16 … SOT109-1`, `74HCT173D`, `74HC173PW … TSSOP16 …
  SOT403-1`; the DigiKey part is `74HC173D,652` (packing suffix). The
  outline page says `SOT109-1 … MS-012`. Rev. 5 — 15 January 2024.
- **TI ordering tables** (PACKAGING INFORMATION, "Orderable part number"
  and "Status") are the plan-10 / plan-35 source; the Status values are
  `ACTIVE`, `LIFEBUY`, `NRND`, `OBSOLETE`, `PREVIEW`.

## 3. Selection rules (the inclusion rule, applied)

A row becomes `add` only when all of these hold. Any other outcome is
`out` or `skip`, with the reason in `ordering`.

1. **In stock and Active or Last Time Buy** at DigiKey in Phase 1's
   listing (statuses 0, 4; `MinimumQuantityAvailable` 1). A planning row
   the listing no longer returns is `out`, `stock check`. No piece hit the
   300 cap at planning; a Phase 1 piece that does (`cut at 300` in its
   `.err`) has each of its planning rows looked up by part number
   (`node scripts/digikey.js --manufacturer <id> --in-stock --limit 5 --json "<mpn>"`;
   kept when a product with exactly that number has `QuantityAvailable` > 0).
2. **Scope** (`census/candidates.py`): the 26 `register` devices of
   `REGISTER`, the 6 `dff` devices of `DFF` (onsemi and Nexperia rows
   only) and the `mux` device 74_670 (`MUX`). A part name any part file
   already has is `out:already-in-catalog`. Any other device number is
   `out`, `not in device list`. The two
   exceptions of the planning output stay `out`: 74_396 has no in-stock
   part (`ti-k396` returned 9 rows, none a '396), and no 32-bit register
   is in stock (no BGA piece in `ti-facets.tsv`).
3. **A vendor datasheet**:
   - TI: the product folder is the first of the orderable's prefixes,
     longest first, down to the device number, for which
     `https://www.ti.com/product/<prefix>` answers 200 (cache per prefix).
     Military numbers (`5962-…`, `JM38510/…`, `8400102RA`) use the folder of
     the SN54/CD54 type `candidates.py` `MIL` maps them to. No folder → `skip`,
     `no TI product page`. The PDF is `https://www.ti.com/lit/ds/symlink/<folder lower case>.pdf`.
     A folder whose only datasheet is a Rochester copy → `skip`,
     `only a Rochester copy`.
   - Nexperia: the `assets.nexperia.com/documents/data-sheet/<name>.pdf`
     URL DigiKey gives; for a Rochester URL, the data-sheet link of
     `https://www.nexperia.com/product/<type number>` (fetched with
     `census/nxget.py`). No Nexperia PDF → `skip`.
   - onsemi: the `onsemi.com` URL DigiKey gives; for another host,
     `https://www.onsemi.com/pdf/datasheet/<base lower case>-d.pdf`. 404 →
     `skip`.
4. **In the ordering table**: TI — the orderable is a PACKAGING INFORMATION
   row; onsemi — it is an ORDERING INFORMATION row; Nexperia — its type
   number (the part number up to the first `,`, with a trailing packing
   letter `J`/`X` or `-Q100J`/`-Q100X` reduced to the type, e.g.
   `74HC273D-Q100J` → `74HC273D-Q100`) is a "Table 1. Ordering information"
   row. Not listed → `out`, `not in ordering table`.
5. **Vendor status**: TI's Status column `ACTIVE` or `LIFEBUY` stays;
   `OBSOLETE`, `NRND` or a PDF marked "Not recommended for new designs" →
   `out`, `obsolete` / `nrnd`. onsemi and Nexperia rows take DigiKey's
   status (rule 1). Owner's rule: obsolete parts are left out, never added
   with an `obsolete` flag.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Census refresh

- [x] Copy `planning/plan-37-data/census/` to `/tmp/p37/census` and run
      every listing line of `README.md` into a fresh `/tmp/p37/raw`, plus
      the 20 TI pieces of § 2 (2026-11-13 00:10–00:40 UTC: 47 pieces, all
      `.done`; `facets.js: 1272 parts, 1 call, 998 left today` →
      `list.js: 28 parts, 1 calls, 798 left today` at the last piece):
      `for p in 107808 107879 108614 108624 108656 108681 108695 108891 142706 142714 142845 143005 143702 143707 143724 143726 730773 143742 143757 143890; do census/run.sh /tmp/p37/raw 706 296 $p ti-p$p; done`.
- [x] Rebuild `candidates.tsv` (`census/candidates.py`, then
      `census/footprints.py`). Set `verdict` = `out`, `ordering` = `stock check`
      for every planning in-scope row the new listing lacks (keep the row).
      Result: 1335 rows; every one of the 472 planning in-scope rows is
      listed again and still `in`, so no `stock check` row; one new
      in-scope row, `JM38510/36101BEA` (a 54173, § Corrections); 473 in
      scope (register 273: TI 229, Nexperia 31, onsemi 13; dff 198; mux 2).
- [x] Copy the new `raw/` files (gzipped) over `planning/plan-37-data/raw/`
      (95 files: 47 `.jsonl.gz`, 47 `.err`, `ti-facets.tsv`).

Check: `grep -h 'cut at 300' /tmp/p37/raw/*.err | wc -l` → 0, or rule 1's
lookups done for each cut piece; `awk -F'\t' '$6=="in"' candidates.tsv | wc -l`
recorded here (planning: 472) (est. 25 min, the API pace).
Observed: `cut at 300` → **0**; in-scope rows → **473**.

Commit: a1e355f

### Phase 2 — Datasheets

- [x] Write `census/nxget.py` (curl_cffi, `impersonate="chrome"`; usage
      `nxget.py <url> <out.pdf>`; prints status, type, bytes; exit 1 unless
      the body starts with `%PDF-`) and `census/fetch.py` (one PDF per
      distinct source of § 3 rule 3, into `datasheets/register/` or
      `datasheets/ff-d/` by the row's `kind`, file name = the URL's last
      path part in lower case; writes `planning/plan-37-data/datasheets.tsv`:
      `file url bytes sha256`). TI and onsemi with `curl -sL`, Nexperia
      with `nxget.py`, 1 s apart per host. (Downloads are cached in
      `/tmp/p37/dl`, probes in `/tmp/p37/fetch-cache.json`; files with
      equal bytes are kept once. Rule-3 additions: § Corrections.)
- [x] Fill `pdf` on every in-scope row, or `verdict` `skip` with its reason:
      448 rows with a `pdf`, 25 `skip` (TI 12, onsemi 12, Nexperia 1);
      129 PDFs fetched (`fetch.py: 129 PDFs, 448 rows with pdf, 25 skip`).
- [x] A file name that already exists in the target directory with
      different bytes is a stop: compare the two revisions' ordering tables
      and keep the newer (the plan-35 rule). — no such file (`fetch.py`
      prints `STOP` for one; none printed; every fetched name was new).
- [x] Delete the PDFs no `add` row names after Phase 3 (added task, §
      Corrections): `register/74alvch16374.pdf`,
      `register/74lvc_lvch16374a_q100.pdf`, `register/mc74lcx16374-d.pdf`;
      126 PDFs left in `datasheets.tsv`.

Check: `awk -F'\t' '$6=="in" && $19!="out" && $17=="" && $19!="skip"' candidates.tsv | wc -l` → 0;
`for f in $(cut -f1 planning/plan-37-data/datasheets.tsv | tail -n +2); do head -c5 datasheets/*/$f; done | grep -vc '%PDF-'` → 0 (est. 20 min).
Observed: the first command → **0**; the `%PDF-` count → **0** (the
`file` column holds `<group>/<name>`, so the loop reads `datasheets/$f`);
`pdfinfo` opens every one of the 126.

Commit: a789481

### Phase 3 — Ordering-table verdicts

- [x] Write `census/verdicts.py`: `pdftotext -layout` each PDF once; apply
      § 3 rules 4–5 per row; fill `ordering` (the matching table line,
      whitespace collapsed) and `verdict`. Result: **382 add** (TI 189,
      onsemi 94, Nexperia 99), **66 out** (TI 31, onsemi 18, Nexperia 17:
      62 not in the ordering table, 3 obsolete, 1 NRND), 25 skip.
      TI's tables print `Last` for Last Time Buy (§ Corrections).
- [x] Hand-check one row per maker per verdict value against the PDF page
      and record the six (maker, part, page) here:
      TI add SN74LS173AN (`register/sn54173.pdf` p.11, PACKAGING
      INFORMATION `SN74LS173AN Active … PDIP (N) | 16`); TI out CD74HC377M
      (`register/cd54hct377.pdf` p.9–10, `CD74HC377M Obsolete`); Nexperia
      add 74HC173D,652 (`register/74hc_hct173.pdf` p.1 Table 1 `74HC173D …
      SO16`); Nexperia out 74HC377DB,112 (`register/74hc_hct377.pdf` p.1
      Table 1 lists D and PW only, no DB); onsemi add MC74HC374ADWR2G
      (`ff-d/mc74hc374a-d.pdf` p.8 `MC74HC374ADWR2G HC374A SOIC-20 Wide`);
      onsemi out MC74AC273DT (`ff-d/mc74ac273-d.pdf` p.6 lists DWG, DWR2G,
      DTR2G only).
- [x] Flags for D–F, from the ordering table: `military` + `ceramic` for
      SNJ54, SN54…J/W/FK, CD54…F, `5962-`, `JM38510/`, `8400102RA`;
      `automotive` for `-Q1`, `-Q100`, `NLV`. Record them in the `grade`
      column (`military` / `automotive` / empty). Result: in-scope rows
      military 25 (19 add), automotive 19 (19 add).

Check: `cut -f19 planning/plan-37-data/candidates.tsv | sort | uniq -c` →
no empty value on an in-scope row (est. 15 min).
Observed: in scope `add` 382, `out` 66, `skip` 25 (473); no empty value.

Commit: a789481

### Phase 4 — Records

- [x] `datasheets/skipped-parts.md`: a `## Registers (plan-37)` section
      (date, the census command) with one table row per (`skip` reason,
      device): part names, device, "`register` / Logic → Registers" or
      "`dff` / Logic → Flip-flops & latches", why. (Written by
      `verdicts.py --skipped`: 25 rows in 11 table rows.)
- [x] Every PDF whose in-scope rows are all `out` for `obsolete` / `nrnd`:
      delete it, drop its `datasheets.tsv` line, add a row to
      `datasheets/obsolete-not-included.md` (PDF, family `register` or
      `dff`, group, note with the URL). — none: no PDF has only obsolete /
      NRND rows (the 4 obsolete / NRND rows share their PDFs with `add`
      rows), so `obsolete-not-included.md` is unchanged.

Check: `python3 -I census/verdicts.py --verify-records` → `0 skip rows
missing from skipped-parts.md, 0 obsolete PDFs left` (est. 2 min).
Observed: `0 skip rows missing from skipped-parts.md, 0 obsolete PDFs left`.

Commit: a789481

## Decisions

Owner's answers (2026-11-12, one batch):

- **Devices**: the 173 class, the clock-enable registers, the
  bus-interface / inverting registers and the wide 16/32-bit registers are
  all in, and the 74x670 register file is in.
- **onsemi and Nexperia parts of the existing `dff` register devices are
  added** to those devices (F). TI parts of those devices that the catalog
  lacks are not (the answer named only the other two makers): `out`,
  `TI part of an existing dff device` (93 rows, incl. SN74LV8T273/374/574).
- **Automotive and military variants are in**, flagged as the catalog
  already does.
- **No Register window**: register parts do not open the converter/RTC
  Register window.
- **A new `register` kind**; 174/175/273/374/574 stay in `dff`, so no
  existing golden line changes.

Author's decisions:

- **The top three makers** are the measured top three by in-stock
  flip-flop parts: TI 1646, onsemi 459, Nexperia 428 (§ 2). Fairchild
  Semiconductor (355) is a separate DigiKey maker and fourth; it is out.
- **The 74x670 needs no new model.** It is already the `mux` kind's
  74_670 (model `regfile`, `mux_stored.c`, drawer Logic → Registers, 7 TI
  parts: `grep -c '"74_670"' schematical/schematical/Parts/mux.json` → 1).
  The owner's "register files in" is met by adding its two missing
  in-stock parts there (D); onsemi and Nexperia list no '670.
- **The device list is closed** (§ 3 rule 2): 26 `register` devices, 6
  `dff` devices, the `mux` 74_670. 40175B (the CMOS '175, a `dff` device) counts with the
  five 74-series devices; it is the same register.
- **Every new device goes into `register`**, even where `dff`'s model
  would fit (40174B, 74_11374): one rule, and the drawer groups all
  registers together.
- **Out of scope by kind**: 1-bit flip-flops ('72, '73, '74, '107, '109,
  '112, 4013, 4027), single/dual gates (1G/2G/2T, NC7S, NL17SZ), ECL (MC10,
  MC100, NB4/NB7).
- **Part names** are DigiKey's part numbers verbatim (`74HC173D,652`,
  `MC74HC374ADWR2G`): the name a buyer orders. Packing twins stay separate
  parts, as TI's `…R` reels do.
- **Nexperia downloads** use `curl_cffi` (§ 2); no other Nexperia source.

## Corrections

- **2026-11-13, Phase 1 — one more in-scope row (473, not 472).** TI's
  16-pin pieces, listed in full for the first time, return six military
  numbers `candidates.py` `MIL` did not map (they fell into
  `out:keyword-noise`). By DigiKey's description and Rochester file name:
  `JM38510/36101BEA` = 54173 (4-bit, 16-CDIP: **in**, device 74_173, family
  TTL), `JM38510/30109BEA` = 54LS109A (JK, out), `JM38510/37201BEA` =
  54ALS174, `5962-9222203MRA` = CY54FCT574T, `5962-9221804MRA` /
  `5962-9221806M2A` = CY54FCT374T (TI parts of `dff` devices, out). All six
  are added to `MIL` in `candidates.py` and `fetch.py`. The four
  `CY74FCT2574…` rows (device '2574, not in the list) stay
  `out:keyword-noise` (= `not in device list`). Counts: D gains one row
  (74_173 17), A § 2's 472 / 272 / TI 228 become 473 / 273 / TI 229.
- **2026-11-13, Phase 2 — rule 3's TI folder search reaches more folders.**
  The prefix rule ("longest prefix down to the device number") misses
  folders TI names otherwise: `CY74FCT<n>T` for CY74FCT…A/B/C…T parts,
  `SN74…[A]` for unprefixed `74LVCH16374ADGVRE4`, `SN74LVTH16374-EP` for
  `CLVTH16374IDLREP`, the `A`/`B` revision folder for parts named without
  it (`SN74ABT377N` → `SN74ABT377A`), and `SN54…` for `SNJ54…` (no SNJ54
  page exists: `ti.com/product/SNJ54LS173A` → 404, `SN54LS173A` → 200).
  `fetch.py` `ti_prefixes` tries these after the prefixes (probes:
  `CY74FCT377T`, `CY74FCT821T`, `CY74FCT823T`, `CY74FCT825T`,
  `CY74FCT16374T`, `SN74LVCH16374A`, `SN74LVTH16374-EP`, `SN74ABT377A`,
  `SN74ABT534A` → 200). A folder page whose symlink PDF is missing
  (`sn74hc377a.pdf` → not a PDF) falls through to the next folder. The
  ordering table (rule 4) still decides each row.
- **2026-11-13, Phase 2 — rule 3's sources for Rochester-linked rows.**
  Nexperia product pages are rendered by script and name no PDF
  (`nxget.py https://www.nexperia.com/product/74HC173D -` → 100535 bytes,
  no `.pdf` link), so a Nexperia row whose DigiKey link is a Rochester copy
  takes the `assets.nexperia.com` URL of another row of the same device and
  family (a `_Q100` sheet only for `-Q100` parts), else Nexperia's sheet
  names (`74HC_HCT173.pdf`, `74LVC377.pdf`, `74ALVCH16823.pdf` …). onsemi:
  `onsemi.com/download/data-sheet/pdf/<stem>-d.pdf`, then another row's
  sheet of the same device and family (onsemi answers its "notFound" page,
  HTML, for a missing sheet: `mc74hct273a-d.pdf`, `mc74hc534a-d.pdf`,
  `mc74f378-d.pdf`).
- **2026-11-13, Phase 3 — TI status `Last`, onsemi `X` suffix.** TI's
  PACKAGING INFORMATION tables now print `Last` (Last Time Buy) where the
  plan says `LIFEBUY` (`sn54als576b.pdf`: `SN74ALS576BDWR Last
  Production`); `verdicts.py` keeps `Last` as rule 5 keeps LIFEBUY.
  Fairchild-heritage onsemi sheets list the tube part and say "Specify by
  appending the suffix letter “X” to the ordering code" for tape and reel
  (`74vhc273-d.pdf`, `74lcx374-d.pdf`): an `…X` part whose base is listed
  in such a sheet is in its ordering table.
- **2026-11-13, Phase 3 — no 74_670 part.** Both in-scope '670 rows are
  `out`: `sn74ls670.pdf`'s PACKAGING INFORMATION lists SN74LS670D, N and
  NSR only, not SN74LS670N3 or SN74LS670NS. D adds no `mux` part (D §
  Corrections).
- **2026-11-13, Phase 2 — unused PDFs deleted (added task).** Three
  fetched PDFs are named by no `add` row (their only rows are not in their
  ordering tables): `74alvch16374.pdf`, `74lvc_lvch16374a_q100.pdf`,
  `mc74lcx16374-d.pdf`. They are not obsolete (so not in
  `obsolete-not-included.md`); they are deleted so no PDF is in the tree
  without a part.
- **Counts after A** (`awk -F'\t' '$19=="add"' candidates.tsv`): D 153
  `register` rows (TI 129, Nexperia 16, onsemi 8; 17 devices) and 0 `mux`;
  E 67 (TI 60, Nexperia 5, onsemi 2; 9 devices); F 162 (onsemi 84,
  Nexperia 78; 6 devices). The four new footprints keep users: DHVQFN-20
  9, DHVQFN-24 1, DQFN-20-ONSEMI 1, TSSOP-64 1.

## Summary

A refreshes the census (adding TI's 16- and 20-pin packages in full),
downloads one vendor PDF per source, and gives each of the 472 in-scope
rows a verdict from its ordering table. B–F build on the `add` rows.
