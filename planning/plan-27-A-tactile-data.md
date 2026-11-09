# plan-27-A: Tactile switches — census, datasheet downloads, first decisions

Last updated: 2026-11-07
Overall Effort: huge (> 3d) — plan-27 is A → B → C → E → F → G → H → I → J → K, and A → D → E (D needs no B/C output: plan-27-D Correction 1)
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-27 adds tactile (push-button) switches to the parts library and the
app. They sit in the parts drawer under **Switch → Tactile → Normal Open**
and **Switch → Tactile → Normal Close**. A tactile switch is a momentary
contact: held down in play mode it closes (Normal Open) or opens (Normal
Close) one contact between its two sides; released, it springs back.

The single behavioral outcome of plan-27: loading the part files gives the
baseline 36 switch parts plus every row of `planning/plan-27-data/candidates.tsv`
whose `decision` ends as `add`; the app lists them under the two drawer
leaves; in play mode a mouse-down on a placed tactile switch closes (NO) or
opens (NC) its contact for as long as the button is held; `ctest` passes
T0 + 1 programs (`tactile_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | census, datasheet downloads, verbatim-match decisions, skipped parts | large |
| B | footprints and specs for C&K, E-Switch, Omron (Aratas) | large |
| C | footprints and specs for Panasonic, Alps, Same Sky, TE, APEM, NKK, Würth | large |
| D | backend: `switch` kind gains `momentary`, `normallyClosed`, `joined`; `tactile_test`; docs | medium |
| E | parts: C&K (`gen-parts.py`, devices, goldens) | large |
| F | parts: E-Switch, Omron (Aratas) | large |
| G | parts: Panasonic, Alps | medium |
| H | parts: Same Sky, TE | large |
| I | parts: APEM, NKK, Würth | large |
| J | app: drawer, symbol, press interaction, sidebar, kindName, BOM, file | medium |
| K | landing tier: full gates, harness, runtime proof | small |

Outcome of A: `datasheets/switch/tactile/<maker>/` holds every PDF a part
of `candidates.tsv` needs; the `decision` column is `add` for every
verbatim-match row, `skip` for every row with no usable PDF, `out` for the
rest of the DigiKey exclusions, and `pending` for the rows B and C decide.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § `switch`).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-27-data/` — `census/*.py|sh`, `candidates.tsv`,
  `footprints-needed.txt` (written at planning time, 2026-11-07).
- `planning/completed/plan-26-A-pull-resistor-data.md`,
  `planning/completed/plan-23-A-generic-header-data.md` (precedent: a kind
  reused with new device keys, data-only first letter).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p27 && cmake --build /tmp/logic-p27 -j8 && ctest --test-dir /tmp/logic-p27` → `100% tests passed` | MET 2026-11-07 at ff209fb: `100% tests passed, 0 tests failed out of 25`; **T0 = 25** (`grep -c '^logic_test'` → 25; plan-22 added one since authoring) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `traces.txt`; `footprints.txt` | MET: **P0 = 11233**, **R0 = 11233**, **F0 = 423** (2026-11-07 at ff209fb; plan-22 added 107 parts) |
| Switch baseline recorded | `grep -c '"id"' schematical/schematical/Parts/switch.json` → 27 devices; `grep -c '|switch|' backend/tests/golden/parts.txt` → 36 parts | MET: **S0 = 36** parts, 27 devices (re-measured 2026-11-07 at ff209fb) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p27-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-07 at ff209fb) |
| No tactile support yet | `grep -c 'momentary' backend/src/models/switch.c` → 0; `ls planning/plan-27-[B-K]-* 2>/dev/null` → only files of this plan | MET (re-run 2026-11-07: 0; 10 files, all plan-27) |
| No open plan touches switches | `grep -l -i 'tactile\|momentary\|SW-TACT' planning/plan-22-* planning/plan-24-* \| wc -l` → 0 | MET (re-run 2026-11-07: 0) |
| DigiKey credentials and quota | `DIGIKEY_CLIENT_ID`/`_SECRET` exported in `~/.zshrc`; the census needs 229 calls (§2) and DigiKey allows about 1000 a day; the last run left 475 | **NOT MET** (2026-11-07): credentials present, but the quota was ~173 at the start (C&K's 71 calls left 102); the run stopped at Same Sky with `0 left today` (C&K 572/573, E-Switch 418/418 complete in `/tmp/p27/raw`; Same Sky 300/405; the seven others 0). Re-run after the daily reset with ≥ 229 calls left |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `datasheets/switch/tactile/<maker>/` holds each datasheet PDF that at least
  one `add` row of `candidates.tsv` needs, with `datasheets.tsv` mapping
  each file to its URL, size and SHA-256.
- `candidates.tsv` has no row with `decision` = `candidate` or `check`:
  every row is `add`, `pending`, `skip` or `out` with a `reason`.
- `datasheets/skipped-parts.md` has a "Tactile switches (plan-27)" section
  listing every `skip` row; `obsolete-not-included.md` gets a row only for a
  PDF whose every part is obsolete (none expected: § 2, `dead` = 0).
- `specs.tsv` exists with one header line and one row per
  (maker, series, mounting) of the `add` and `pending` rows, its drawing
  columns empty (B and C fill them).

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part is added in this letter; `pending` rows are decided in B and C.
- No new DigiKey census beyond the re-run of § Phase 1.

## 2. Current State

The part library has one switch kind and no momentary switch:
`backend/src/models/switch.c` (DIP switches; "Every position starts OFF"),
`Parts/switch.json` (27 devices, 36 parts, all drawer `["Switch","DIP"]`).
There is no tactile footprint (the 15 `SW-` footprints are DIP switches).

### Measured populations

| What | Count | Command |
|---|---|---|
| Tactile Switches in stock at DigiKey, Active | 4010 | `node scripts/digikey.js --category 197 --in-stock --active-only --pages 1 --json` → `ProductsCount` 4010 |
| …of which SPST-NO / SPST-NC / SPDT / SPST x 2 | 3768 / 4 / 18 / 7 | same call, `Circuit` facet |
| Parts of the ten chosen makers, in stock, Active or Last Time Buy (distinct maker + part number) | 2750 | `planning/plan-27-data/census/list.sh /tmp/p27/raw` then `python3 -I census/build.py /tmp/p27/raw all.tsv` → `2750 parts`; 229 API calls |
| Rows, by maker | C&K 572, E-Switch 418, Same Sky 405, TE ALCOSWITCH 268, Aratas (Omron) 251, Panasonic 227, NKK 167, Alps 152, APEM 147, Würth 143 | `list.sh` summary lines |
| `candidate` after DigiKey rules | 2337 | `awk -F'\t' '$18=="candidate"' planning/plan-27-data/candidates.tsv \| wc -l` → 2337 |
| `check` (DigiKey parameters blank) | 30 | same, `"check"` → 30 |
| `out`: circuit 28, illuminated 349, mounting 5, stock 1 | 383 | `python3 -I census/select.py all.tsv cand.tsv` |
| Last Time Buy rows | 17, all `candidate` | `awk -F'\t' '$3=="Last Time Buy" && $18=="candidate"' …candidates.tsv \| wc -l` → 17 |
| SPST-NC candidates | 4 (C&K ATS2D1G NC LFS, ATS2D3G NC LFG, KSR223G NC LFG; Same Sky TS10-63-26-BE-250-SMT-TR) | `awk -F'\t' '$6=="SPST-NC"' …candidates.tsv \| wc -l` → 4 |
| Distinct datasheet URLs of candidate and check rows | 739 | `census/fetch.py` → `739 urls` |
| …that downloaded as a PDF | 681 | `census/fetch.py` → `681 are PDFs` (the 58 others are product pages, dead links or no URL) |
| `candidate` rows with a PDF / without | 2186 / 151 | `awk -F'\t' '$18=="candidate" && $20==1'` → 2186; `$20==0` → 151 |
| …with the part number verbatim in the PDF text | 1112 | `$21==1` → 1112 |
| …with a PDF but the number not verbatim | 1074 | `$20==1 && $21==0` → 1074, in 167 (maker, series) groups |
| Verbatim rows with an obsolete / discontinued / NRND / end-of-life word within 300 characters | 0 | `awk -F'\t' '$22==1' …candidates.tsv \| wc -l` → 0 |
| `check` rows with a PDF / verbatim | 29 / 11 | `awk -F'\t' '$18=="check" && $20==1'` → 29; `$21==1` → 11 |
| Distinct PDFs of the rows with a PDF | 619, 600 MB | `awk -F'\t' '$18=="candidate" && $20==1 {print $17}' … \| sort -u \| wc -l` → 619; `du -ch` of the files → 600M |
| Candidate rows with a PDF, by group (B: C&K 487, E-Switch 330, Aratas 148 = 965; C: Panasonic 193, Alps 120, Same Sky 379, TE 176, APEM 129, NKK 88, Würth 136 = 1221) | 2186 | `awk -F'\t' '$18=="candidate" && $20==1 && index($1,"C&K")==1'` etc. |
| Footprint proxy lines (maker, series, mounting) with a PDF | 314 | `grep -vc '^#' planning/plan-27-data/footprints-needed.txt` → 314 |

### Verified properties

- **Why scripts failed, and what works.** `urllib` and a bare `curl` get 403
  from littelfuse.com, te.com, ckswitches.com, alpsalpine.com and
  components.omron.com (Akamai). `curl` with Safari's request headers
  (`User-Agent` Safari 18.6, `Accept`, `Accept-Language`, `Accept-Encoding:
  gzip, deflate, br`, `Sec-Fetch-Site: none`, `Sec-Fetch-Mode: navigate`,
  `Sec-Fetch-Dest: document`, `Priority: u=0, i`), `--http2`, `--compressed`
  and the site's origin as Referer gets `200 application/pdf` from all of
  them (littelfuse PTS636: 435917 bytes; te.com drawing 1571300: 1603388
  bytes with the cookie `country_info=usa; geo_info=usa; language_id=1;
  site_info=usa|en`). we-online.com answers only `User-Agent: Claude-User`
  (637887 bytes). te.com tarpits parallel requests (8 at once: 7 files in two
  minutes; one at a time: 4 s each). All of this is in `census/fetch.py`,
  whose run gave 681 PDFs of 739 URLs (2026-11-07).
- **The 58 URLs that stay unfetched** are `tech.alpsalpine.com` 27 (19 return
  a product page, 8 return 403), `www3.panasonic.biz` 10 and
  `industrial.panasonic.com` 3 (product pages / timeouts), `omronfs.omron.com`
  4 (S3 `AccessDenied`), `spec_sheets.e-switch.com` 4 (no DNS answer),
  `www.we-online.com` 5, `www.apem.com` 2, `www.e-switch.com` 1,
  `configured-product-images.s3.amazonaws.com` 1, `www.te.com` 1
  (`python3 -I` over `/tmp/p27/pdf/index.tsv`, 2026-11-07).
- **Mid-checking `dead`.** `census/textscan.py` strips non-alphanumerics and
  looks for the part number in the PDF text; `dead` = 1 when OBSOLETE,
  DISCONTINU, NOTRECOMMENDEDFORNEW, ENDOFLIFE or NRND is within 300
  characters. 0 of 1112 verbatim rows have it.
- **DigiKey's `--limit` default is 500.** `list.sh` passes `--limit 5000`;
  without it C&K came back as 500 of 573.

## 3. Selection rules (the inclusion rule, applied)

A `candidate` or `check` row becomes `add` only when all of these hold; any
other outcome is `skip` or `out` with the reason in the `reason` column.

1. **In stock** (quantity above zero) and **status Active or Last Time Buy**
   at DigiKey. Obsolete, Discontinued at DigiKey and Not For New Designs are
   never listed (`list.sh` asks only for statuses 0 and 4). Last Time Buy
   parts are added without a flag (the flags list has no such flag).
2. **Circuit** SPST-NO or SPST-NC; **not illuminated**; **mounting** surface
   mount, through hole, surface-mount or through-hole right angle, or edge
   mount (`select.py`). A blank-parameter (`check`) row takes circuit,
   mounting and illumination from its datasheet in B/C; a row that is not
   non-illuminated SPST-NO/NC there is `out` (reason `circuit` or `lit`).
3. **A usable vendor datasheet.** The row's `datasheet_url` downloaded as a
   PDF from the maker's own site, DigiKey (`mm.digikey.com`) or Waldom
   (`waldomemea.com`), which host unmodified maker PDFs (the first page shows
   the maker's name or logo). No URL, a product page, a dead link: `skip`,
   reason `no datasheet PDF` (and the host's failure).
4. **In the datasheet's ordering table.** The part number is printed
   verbatim in the PDF (`hit` = 1) → `add` now. A row with a PDF and no
   verbatim hit is `pending`: B or C reads that series' part-number key and
   records a regular expression in `schemes.tsv`; `scheme-check.py` then sets
   `add` for every pending row whose part number fully matches it and `skip`
   (reason `not in the datasheet's ordering table`) for the rest.
5. **Not marked obsolete, discontinued, NRND or end of life** by the
   datasheet itself next to the part (`dead` = 0 → no effect; a `dead` = 1
   row is `out`, reason `datasheet status`; the PDF is deleted and listed in
   `obsolete-not-included.md` only if every part it covers is out).

## 4. Datasheet placement

`census/place.py` (written in Phase 2) copies each needed PDF from
`/tmp/p27/pdf/<md5>.pdf` to `datasheets/switch/tactile/<maker-slug>/<name>.pdf`
where `<maker-slug>` is `ck`, `eswitch`, `omron`, `panasonic`, `alps`,
`samesky`, `te`, `apem`, `nkk`, `wurth` and `<name>` is the URL path's last
segment (query string dropped, `.pdf` appended when missing, characters
outside `A-Za-z0-9._-` replaced by `-`); a clash gets `-<first 6 hex of the
md5>`. It writes `datasheets/switch/tactile/datasheets.tsv`: path, URL,
bytes, SHA-256. A PDF no `add` or `pending` row uses is not copied.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Re-run the census

The numbers in § 2 are from 2026-11-07; stock moves. Re-measure before
anything else.

- [x] Run `planning/plan-27-data/census/list.sh /tmp/p27/raw` (load
      `DIGIKEY_*` from `~/.zshrc` first; 229 calls), then
      `python3 -I census/build.py /tmp/p27/raw /tmp/p27/all.tsv` and
      `python3 -I census/select.py /tmp/p27/all.tsv /tmp/p27/cand.tsv`.
      Run 2026-11-08 one maker at a time with retries (Correction 5:
      `/tmp/p27/scripts/seq.sh`, `samesky.sh`, the same `digikey.js` call):
      C&K `572 of 573 parts, 71 pages`, E-Switch `418 of 418`, Same Sky `405 of
      405` (8th try after DNS failures), TE `268 of 268`, Aratas `251 of 251`,
      Panasonic `227 of 227`, Alps `152 of 152`, Würth `143 of 143`, NKK `167 of
      167`, APEM `147 of 147`; `build.py` → `2750 parts`; `select.py` → `2337
      ('candidate', '')`, `30 ('check', …)`, out `28` circuit, `349` lit, `5`
      mounting, `1` stock.
- [x] Compare with § 2. Where a count differs, write the new count and the
      date in § Corrections and carry it into B–I's goal numbers; the design
      is unchanged. Every § 2 census count is equal (2750 parts; 2337 / 30 /
      383; per maker 572 … 147). The (maker, part) set and every first-pass
      decision are identical to the planning file (`diff` of `cut -f1,2,18` →
      no lines); 19 rows differ only in other columns (stock).
- [ ] `python3 -I census/fetch.py /tmp/p27/cand.tsv /tmp/p27/pdf` then
      `python3 -I census/textscan.py /tmp/p27/cand.tsv /tmp/p27/pdf
      planning/plan-27-data/candidates.tsv` (≈ 15 min: te.com is serial).
- [ ] `python3 -I planning/plan-27-data/census/footprint-lines.py
      planning/plan-27-data/candidates.tsv > /tmp/p27/fp.txt`, then diff the
      data lines of `/tmp/p27/fp.txt` against `footprints-needed.txt`.

Acceptance: `candidates.tsv` is current and its counts are in § Corrections or
equal to § 2.
  Check: `awk -F'\t' '$18=="candidate"' planning/plan-27-data/candidates.tsv | wc -l` → the § 2 number or the corrected one (est. 20 min incl. 229 DigiKey calls and the downloads; nothing smaller re-measures stock).
Commit: —

### Phase 2 — Place the datasheets

- [ ] Write `planning/plan-27-data/census/place.py` per § 4.
- [ ] Run it for every row with `pdf` = 1; the PDFs land under
      `datasheets/switch/tactile/<maker>/` and `datasheets.tsv` is written.
- [ ] `git add` only those files.

Acceptance: every row with `pdf` = 1 has its PDF in the tree.
  Check: `python3 -I planning/plan-27-data/census/place.py --verify planning/plan-27-data/candidates.tsv` → `0 missing` and `ls datasheets/switch/tactile/*/*.pdf | wc -l` equals `awk -F'\t' '$20==1 {print $17}' planning/plan-27-data/candidates.tsv | sort -u | wc -l` (619 at planning time) (est. 2 min).
Commit: —

### Phase 3 — First decisions

- [ ] Write `planning/plan-27-data/census/decide.py` applying § 3 rules 1–5 to
      `candidates.tsv`: `pdf` = 1 and `hit` = 1 and `dead` = 0 → `add`;
      `pdf` = 1 and `hit` = 0 → `pending`; `pdf` = 0 → `skip` with the
      reason from `/tmp/p27/pdf/index.tsv` (`no URL`, `product page`,
      `HTTP 403`, `HTTP 404`, `timeout`, `S3 AccessDenied`); `dead` = 1 →
      `out`. `check` rows follow the same mapping (circuit and lit are
      checked in B/C).
- [ ] Run it; record the counts of each `decision` in § Summary.
- [ ] `datasheets/skipped-parts.md`: add the section "Tactile switches
      (plan-27)" with one table row per skip reason: the part numbers, the
      series, "would have gone into `switch` / Switch → Tactile", the reason
      (format of the existing sections).
- [ ] Delete from the tree any PDF no `add` or `pending` row uses
      (`decide.py` prints them); list in `obsolete-not-included.md` a PDF
      whose every part is `out` for `datasheet status` (expected: none).

Acceptance: no row is `candidate` or `check`; every `skip` is in
`skipped-parts.md`.
  Check: `awk -F'\t' 'NR>1 && ($18=="candidate" || $18=="check")' planning/plan-27-data/candidates.tsv | wc -l` → 0, and `python3 -I planning/plan-27-data/census/decide.py --verify-skipped` → `0 skip rows missing from skipped-parts.md` (est. 1 min).
Commit: —

### Phase 5 — Maker-hosted datasheet URLs (added, Correction 4)

- [ ] For every distinct `datasheet_url` of the `add` and `pending` rows whose
      host is not the maker's own site (`mm.digikey.com`, `waldomemea.com`;
      `census/maker-urls.py --list` prints them), look for the same document
      on the maker's own site and record it in
      `planning/plan-27-data/maker-urls.tsv` (`dk_url maker_url how`):
      `maker_url` the maker's URL that serves a PDF of the same document (or
      its current revision), or empty when none is found (then the part keeps
      the DigiKey URL: owner, 2026-11-08). Maker hosts: `littelfuse.com` /
      `ckswitches.com` (C&K), `e-switch.com` and its S3 buckets,
      `omron.com` (Aratas), `panasonic` hosts, `alpsalpine.com`,
      `sameskydevices.com`, `te.com`, `apem.com`, `nkkswitches.com`,
      `we-online.com`.
- [ ] `python3 -I planning/plan-27-data/census/maker-urls.py --verify` →
      every non-maker URL has a row.

Acceptance: every non-maker-hosted datasheet of a kept row has a decided URL.
  Check: `python3 -I planning/plan-27-data/census/maker-urls.py --verify` → `0 missing` (est. 1 min).
Commit: —

### Phase 4 — `specs.tsv` skeleton

- [ ] Write `planning/plan-27-data/specs.tsv`: header `maker series mounting
      mpn_regex terminals pins footprint note`, then one row per distinct
      (maker, series, mounting) of the `add` and `pending` rows, `mpn_regex`
      `.*`, the last four columns empty (B and C fill `terminals`, `pins`,
      `footprint`; formats in plan-27-B § 4.3).
- [ ] `planning/plan-27-data/README.md`: the file map of this folder.

Acceptance: `specs.tsv` has one row per (maker, series, mounting) group.
  Check: `awk -F'\t' '$18=="add" || $18=="pending" {print $1"|"$5"|"$8}' planning/plan-27-data/candidates.tsv | sort -u | wc -l` equals `awk 'END{print NR-1}' planning/plan-27-data/specs.tsv` (est. 1 min).
Commit: —

## Validation Plan

- Tests: none (data only). The scripts are checked by their own `--verify`
  modes (Phases 2, 3).
- Coverage check: Phase 4's count equality shows every kept row has a spec row.
- Runtime proof: not applicable; no part is loaded.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate (once, after Phase 4): `cmake --build /tmp/logic-p27 -j8 && ctest --test-dir /tmp/logic-p27` → `100% tests passed` (est. 5 min; proves the data-only letter changed nothing).

## Decisions

- **Scope** — decided by the owner, 2026-11-07: SPST-NO and SPST-NC only;
  ten makers (C&K, E-Switch, Same Sky, TE ALCOSWITCH, Omron/Aratas, Panasonic,
  Alps Alpine, Würth, NKK, APEM); surface mount, through hole, right angle and
  edge mount in; illuminated (LED) variants out; a switch is a momentary
  hold-to-press button. Settles § 3 rule 2 and the whole drawer shape.
- **Drawer** — owner, 2026-11-07: Switch → Tactile → Normal Open / Normal
  Close. As data: `"drawer": ["Switch","Tactile"]`, `"function": "Normal
  Open"` or `"Normal Close"` (the loader allows two drawer levels; the
  function is the leaf).
- **Reuse the `switch` kind** — author. Same part kind, model and Swift
  `CatalogPart.Details` family; plan-27-D adds the three device keys.
  Rejected: a new `tactile` kind (duplicates the contact model and the voltage
  check's `switch_contact_pins`).
- **Datasheet source of truth** — author: the DigiKey `DatasheetUrl` of the row
  (§ 3 rule 3); no other search. Rejected: hunting maker sites for a better PDF.
- **Ordering-table test** — author: verbatim match, else a regex from the
  datasheet's own key (§ 3 rule 4). Rejected: counting a DigiKey-only code as
  in the table (the existing rule for TI parts, `skipped-parts.md`).
- **Download method** — author: `census/fetch.py` (§ 2 Verified properties).
- **PDFs are committed** for the `add` and `pending` rows (600 MB at most),
  like every other `datasheets/` family.
- **Datasheet URL of a part** — owner, 2026-11-08 (asked when main's
  "manufacturer's own site" rule for `datasheets.json` met rule 3's DigiKey
  and Waldom copies): use the maker's URL of the same document when it can
  be found; when it cannot, keep the DigiKey URL. Rejected: skipping those
  parts; adding them with no `"datasheet"`.

## Corrections

1. 2026-11-07 — gate measured at ff209fb: T0 = 25 (not 24) and P0 = R0 =
   11233 (not 11126): plan-22 landed one test program and 107 parts after this
   plan was written. Later letters' "25 programs" become **26** (T0 + 1) and
   the P_E / P0 baselines start from 11233.
2. 2026-11-07 — the DigiKey quota row was NOT MET: the census exhausted the
   shared daily quota after 175 of 229 calls (`samesky: … 300 of 405 parts,
   20 API calls, 0 left today`). The plan stops at the gate; resume after the
   reset with the full `list.sh` run (the partial raw files are not used).
3. 2026-11-08 — **main moved** (87 commits, merged as 55140df): T0 = 27
   (`converter_test`, `rtc_test` from plan-24), so plan-27's ctest total is
   **28**; goldens P0 = R0 = **13158**, F0 = **508** (`wc -l`, after the
   merge). Main also made `"manufacturer"` required on every shipped part and
   `"datasheet"` (an id in `Parts/datasheets.json`, URL on the maker's own
   site) expected (`docs/parts-format.md` § Parts, § Datasheets;
   `listing_check`). plan-27-E § 4.1 now writes both.
4. 2026-11-08 — **new Phase 5** (maker-hosted URLs) for the owner's decision
   above. Rows by host in the planning-time file (`/tmp/p27/scripts/hosts.py`):
   `mm.digikey.com` APEM 118, Panasonic 71, Alps 36, Aratas 24, C&K 17,
   E-Switch 9, TE 2; `waldomemea.com` C&K 9, APEM 5, Aratas 5;
   `ariat-tech.tw` APEM 2 (a distributor, not DigiKey or Waldom: rule 3
   makes those two `skip`, reason `distributor copy`).
6. 2026-11-08 — **rule 4's "printed verbatim" was a substring test.**
   `textscan.py` stripped punctuation and searched the whole text, so a
   number that is only the prefix of a longer printed code counted
   (`B3SE-1002` where only `B3SE-1002P` is printed; found by the Omron
   reader). `textscan.py` now also requires the number to stand on its own
   (its characters in order, at most two punctuation / space characters
   between them, no letter or digit right before or after) and reads the
   text NFKC-normalised (Alps prints numbers in full-width characters, which
   the old test missed). Measured on the pre-census copy: 8 of 775 verbatim
   hits were not bounded (`/tmp/p27/scripts/bound.py`: Aratas 3, E-Switch 2,
   C&K 1, NKK 1, Same Sky 1); they become `pending` and their series' key
   decides them.
5. 2026-11-08 — the census hit a degraded network: DigiKey connects timed out
   (`UND_ERR_CONNECT_TIMEOUT`) while the 600 MB of PDF downloads ran, and
   searches took 76 s each (`curl`, `/products/v4/search/keyword`, 5
   results). `scripts/digikey.js` now cuts a request after 180 s and retries
   it 3 times (b1aa8e7, 51801bb); the census runs after the downloads, one
   maker at a time (`/tmp/p27/scripts/one.sh`, the same call as `list.sh`).

## Summary

The risk in A is volume, not design: 2337 candidate rows, 619 PDFs, 167
series whose part numbers come from an ordering-code key rather than a list.
A decides nothing that needs reading a drawing; B and C read each datasheet
once for scheme, drawing and pinout together. Left untouched: every part,
footprint, backend and app file.
