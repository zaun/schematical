# plan-70-B: Datasheet URLs, downloads, text scan, verdicts, placement, the specs skeleton

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-70-A

B fetches the datasheet of every `candidate` row, reads its text, and sets
each row's verdict: `add` (the part number is printed in its PDF), `pending`
(the PDF prints an ordering key, which C and D decide), `skip` or `out`. A
`skip` or `out` moves the next `reserve` row of its (type, maker) up, and
that row runs through the same steps. B places the PDFs under
`datasheets/isolator/<maker>/` and writes the `specs.tsv` skeleton that C and
D fill.

References:

- plan-70-A (`candidates.tsv`: `datasheet_url`, `pick`, `rank`).
- `planning/plan-58-data/census/` (`fetch.py`, `textscan.py`, `decide.py`,
  `place.py`, `maker-urls.py`, `specs-skeleton.py`): the scripts B copies
  and adapts. `planning/plan-48-data/census/verdicts.py` provides the
  status-word rule.
- `docs/parts-format.md` § Datasheets (the maker's own site);
  `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`
  (formats); AGENTS.md § Datasheets (obsolete PDFs).

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-A complete | `ls planning/completed/plan-70-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| The candidates exist | `awk -F'\t' 'NR>1 && $17=="candidate"' planning/plan-70-data/candidates.tsv \| wc -l` → at least 1, at most 2103 | NOT MET (2026-11-25: no file) |
| Maker hosts answer a download | `python3 -I planning/plan-70-data/census/fetch.py --probe` → `10 hosts, 10 PDFs` | rule. The planning probes (2026-11-25, Python `urllib` with a Safari user agent plus `Accept` and `Accept-Language`, or `curl -sL -A "Mozilla/5.0"`) got a PDF from each of the ten makers (§ 2) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `candidate` row has a verdict in `candidates.tsv` (`decision`
  `add`, `pending`, `skip` or `out`, with a `reason`). The refill loop has
  run until each (type, maker) has min(200, its kept rows) `add` + `pending`
  rows, or no `reserve` row is left.
- `datasheets/isolator/<maker>/` holds each PDF at least one `add` or
  `pending` row cites. Each folder's `datasheets.tsv` maps the file to its
  URL, size and SHA-256.
- `datasheets/skipped-parts.md` has an "Optocouplers and digital isolators
  (plan-70)" section listing every `skip` row.
  `datasheets/obsolete-not-included.md` gets a row only for a PDF whose
  every listed part is obsolete.
- `planning/plan-70-data/specs.tsv` has one row per (type, maker, PDF,
  package, channels) of the `add` and `pending` rows. Its pinout and fact
  columns are empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- `pending` rows are not decided here (C and D read the ordering keys).
- No datasheet hunt beyond DigiKey's link, the maker-site rewrites of § 3.1,
  and `maker-urls.tsv`.

## 2. Current State

No PDF of these parts is in the tree (`ls datasheets/isolator` → no such
directory). The reference-board copy
`datasheets/transistor/vishay-semiconductor-opto-divi/vo617a.pdf` (plan-43)
is one of Vishay's transistor-output datasheets. B places its own copy
under `datasheets/isolator/vishay/` when a candidate cites it, as plan-48
placed its own PDFs.

### Verified properties (planning probes, 2026-11-25, PDFs saved in the scratchpad)

| Maker | Probe URL | Result |
|---|---|---|
| Isocom | `https://isocom.com/wp-content/uploads/2026/04/de93286-h11aax-150622.pdf` (linked from `https://isocom.com/products/h11aa1-ac-input-6-pin-single/`; `https://isocom.com/product-sitemap.xml` lists 508 product pages) | `curl -sL -A "Mozilla/5.0"` → 200 `application/pdf`, 2,203,681 bytes, 11 pages; its ORDER INFORMATION prints `H11AA1SMT&R`, `H11AA1XG` |
| Vishay | `https://www.vishay.com/docs/83725/4n25.pdf` | `urllib` → 200, 130,719 bytes, `%PDF-` |
| onsemi | `https://www.onsemi.com/pdf/datasheet/moc3023m-d.pdf` | `urllib` → 200, 343,504 bytes, title "MOC3023M - 6-Pin DIP Random-Phase Triac Driver Output Optocoupler" |
| Broadcom | `https://docs.broadcom.com/doc/AV01-0545EN` | `curl -sL -A "Mozilla/5.0 (Macintosh) Safari/605.1.15"` → 200 `application/pdf` (title "4N45"); a wrong document id returns 302 → `/docs/404`, so URLs come from DigiKey's link |
| Toshiba | `https://toshiba.semicon-storage.com/info/docget.jsp?did=30107&prodName=TCS40DPR` (the plan-43 URL) | `urllib` → 200, 221,271 bytes (redirected to `…/TCS40DPR_datasheet_en_20150403.pdf?did=30107…`); bare `curl` gets 403 |
| Renesas | `https://www.renesas.com/us/en/document/dst/ps2801-1-ps2801-4-datasheet` | `urllib` → 200, 378,698 bytes (redirected to `/en/document/dst/…`), title "PS2801-1, PS2801-4 Data Sheet" |
| Würth Elektronik | `https://www.we-online.com/components/products/datasheet/140816144000.pdf` | `urllib` with `Accept` + `Accept-Language` → 200, 2,437,742 bytes, "WL-OCPT Optocoupler Phototransistor", 14 pages with a "Recommended Land Pattern" page; one PDF per part number |
| TI | `https://www.ti.com/lit/gpn/iso7721` | `urllib` → 200, 2,464,766 bytes, redirected to `…/lit/ds/symlink/iso7721.pdf` |
| ADI | `https://www.analog.com/media/en/technical-documentation/data-sheets/ADuM1200_1201.pdf` | `urllib` → 200, 520,933 bytes, "ADuM1200/ADuM1201 (Rev.L)" |
| Skyworks | `https://www.skyworksinc.com/-/media/Skyworks/SL/documents/public/data-sheets/si861x-2x-datasheet.pdf` | `urllib` → 200, 753,193 bytes, "Si861x/2x … Digital Isolators", 30 pages |

- **Default output is an ordering option** (Skyworks Si86xx p.5 Table 3
  note 6; TI ISO772x p.1: "default output is high for devices without
  suffix F"). The part number, not the device, says which default a part
  has (G § 3.4: the part key `failsafe`).
- **Isocom prints an ordering key** (`H11AA1` + `G`, `SM`, `SMT&R`, `X`, `XG`).
  Each listed number is the base plus suffixes, so most Isocom rows are
  `pending` until C reads the key.

## 3. Design

### 3.1 URLs (`census/fetch.py`)

- The URL is the row's `datasheet_url` (`//host/…` gets `https:`).
- `ti.com` links become `https://www.ti.com/lit/ds/symlink/<name>.pdf`, the
  form `docs/parts-format.md` § Datasheets uses. A DigiKey-hosted or
  Rochester link of a TI row becomes `https://www.ti.com/lit/gpn/<base>`
  (plan-48-A § 3).
- `datasheets.maximintegrated.com` links become
  `https://www.analog.com/media/en/technical-documentation/data-sheets/<file>`
  (plan-48-A § 3).
- A `mm.digikey.com` link is fetched with `scripts/safari-digikey-pdf.sh`.
  It goes to `maker-urls.py --list`, and the same document on the maker's
  site replaces it in `maker-urls.tsv` (plan-58-A Phase 3).
- Headers: a Safari user agent with `Accept` and `Accept-Language` for every
  host. `toshiba.semicon-storage.com` and `we-online.com` refuse a bare
  request (§ 2).
- A body that does not start with `%PDF-`, or that `pdfinfo` cannot open,
  is not kept. The row is `skip`, `no datasheet`.

### 3.2 Verdict rules (`census/decide.py`)

Per `candidate` row, in this order; the first that applies decides:

1. No PDF → `skip`, `no datasheet`.
2. The part number in the PDF's text (`pdftotext -layout`, spaces removed,
   upper case), verbatim, with these packing forms. Toshiba's `(TPL,E` and
   `(TP,F` style suffixes are matched as the base before `(`, when the PDF
   prints that base with the same suffix in its ordering table. A TI
   `R`/`T` reel suffix matches the orderable addendum's row. An ADI `-RL7`
   / `-RL` / `-R7` matches the number without it when the text has
   `REEL`. Then `hit` 1 → `add`.
3. No hit, but the PDF has an ordering or part-number key section
   (`ORDER INFORMATION`, `ORDERING INFORMATION`, `ORDERING GUIDE`, `PART
   NUMBER SYSTEM`, `ORDERING CODE`) → `pending`, `ordering key`. C or D
   decides it against the key.
4. No hit and no key → `skip`, `not in the datasheet`.
5. Status words (`OBSOLETE`, `DISCONTINUED`, `NRND`, `NOT RECOMMENDED FOR
   NEW DESIGNS`, `END OF LIFE`, `LIFEBUY`) at the part's own ordering row
   (plan-48-A Corrections 2: read at the part, not in a window) → `dead` 1.
   An `add` or `pending` row with `dead` 1 stays and is flagged
   `"flags": ["obsolete"]` by H. A PDF whose every listed row has `dead` 1
   is deleted, recorded in `obsolete-not-included.md`, and its rows are
   `out`, `obsolete`.

`textscan.py` writes `textscan.tsv` (`mpn hit scheme dead context`). The
80 characters around a match are read for every `scheme` and `dead` row. A
wrong match goes into `verdict-overrides.tsv` (`mpn verdict reason why`),
applied last.

### 3.3 Placement and skeleton

- `place.py` copies each distinct PDF (by SHA-256) of an `add` or `pending`
  row to `datasheets/isolator/<maker>/<name>.pdf`. `<name>` is the URL's
  last path segment in lower case. For Toshiba `docget.jsp`, Renesas
  document pages and Broadcom `doc/<id>`, it is the redirected file name or
  the document id, with `.pdf` added. `place.py --verify` re-hashes.
- `specs-skeleton.py`: `specs.tsv` columns `type maker pdf package
  supplier_package channels rows example pinout facts footprint note`. A
  writes the first eight (`example` is the most-stocked part) and leaves the
  last four empty.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: Downloads

- [ ] `census/fetch.py` per § 3.1; `--selftest` (rewrites, header set,
      `%PDF-` check) prints `ok`; `--probe` prints `10 hosts, 10 PDFs`.
- [ ] `python3 -I census/fetch.py planning/plan-70-data/candidates.tsv /tmp/p70/pdf`
      → `<u> urls; <p> are PDFs; <m> missing`; writes `url-check.tsv`.
- [ ] `census/maker-urls.py --list` and `maker-urls.tsv` for the
      DigiKey-hosted URLs; `--verify` prints `0 missing, 0 not PDFs`.

Acceptance: every `candidate` URL is fetched or listed as missing.
  Check: `awk -F'\t' 'NR>1' planning/plan-70-data/url-check.tsv | wc -l`
  equals the distinct `datasheet_url` count of the candidates
  (`awk -F'\t' 'NR>1 && $17=="candidate"{print $18}' planning/plan-70-data/candidates.tsv | sort -u | wc -l`).
  About 30 min of downloads; the URL count is set by A.
Commit:

### Phase 2: Text scan, verdicts, the refill

- [ ] `census/textscan.py` and `census/decide.py` per § 3.2; `--selftest`
      (verbatim, Toshiba suffix, ADI reel, key section, a status word at the
      part) prints `ok`.
- [ ] The loop `build.py --refill` → `fetch.py` → `textscan.py` →
      `decide.py` until `decide.py --report` prints `non-final groups 0`.
- [ ] Read `context` for every `scheme` and `dead` row; write
      `verdict-overrides.tsv` for wrong matches.
- [ ] `decide.py --skipped` writes the plan-70 section of
      `datasheets/skipped-parts.md`. It reads the file before opening it
      for writing (plan-58-A Corrections 4). `--verify-skipped` prints
      `0 skip rows missing`.

Acceptance: no `candidate` row is without a verdict.
  Check: `awk -F'\t' 'NR>1 && $17=="candidate" && $23==""' planning/plan-70-data/candidates.tsv | wc -l`
  → 0 (column 23 is `decision`, after A's 19 and B's `pdf hit dead`).
  Takes about 10 min (`pdftotext` cached).
Commit:

### Phase 3: Placement, obsolete PDFs, the skeleton

- [ ] `census/place.py` per § 3.3; `--verify` → `<n> PDFs, 0 problems`.
- [ ] Obsolete PDFs (§ 3.2 rule 5): delete them, add the lines to
      `datasheets/obsolete-not-included.md` (PDF name; family "Optocouplers
      and digital isolators", category Interface, group Optocoupler or
      Digital isolator).
- [ ] `census/specs-skeleton.py` → `<n> rows`.
- [ ] Update `planning/plan-70-data/README.md`.

Acceptance: every kept row's PDF is in the tree, and each has a spec row.
  Check: `python3 -I planning/plan-70-data/census/place.py --verify` →
  `0 missing, 0 unused`. The `rows` column sum of `specs.tsv`
  (`awk -F'\t' 'NR>1{s+=$7} END{print s}' planning/plan-70-data/specs.tsv`)
  equals the `add` + `pending` count. Takes about 2 min.
Commit:

## Validation Plan

- Tests: the `--selftest` / `--verify` modes; the three acceptance checks.
- Coverage check: `git status --short` lists only `planning/`, `datasheets/`.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`,
  `planning/plan-70-data/README.md`.
- Final gate: plan-70-J.

## Decisions

- Author: **`pending` for ordering-key datasheets** (plan-58's rule). Isocom,
  Vishay, onsemi and Würth print keys more often than full numbers (§ 2).
- Author: **one folder per maker under `datasheets/isolator/`**, named by
  the kind, as `datasheets/timer/` (plan-48).
- Author: **a status word is read at the part's own row** (plan-48-A
  Corrections 2), not in a character window.
- Author: **`maker-urls.tsv` replaces DigiKey-hosted links with the maker's
  own copy when there is one** (plan-27-A, owner 2026-11-08). Otherwise the
  DigiKey URL stays.

## Corrections

## Summary

B is file work. The care is in the matching rules: Toshiba's bracketed
suffixes, Isocom's suffix keys, and status words read at the right row.
