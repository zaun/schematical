# plan-82-B: Indicator LEDs: datasheet downloads, obsolete check, `specs.tsv` skeleton

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-82-A

B downloads the datasheet of every `add` row of `planning/plan-82-data/verdicts.tsv`, checks each
PDF against the obsolete rule of AGENTS.md, and writes the `specs.tsv` skeleton that C, D and E
fill: one row per (maker, datasheet, package drawing).

References: plan-82-A (§ Prerequisites, § 3.4 verdict rules); `AGENTS.md` § Datasheets;
`docs/parts-format.md` § Datasheets (`datasheets.json`, the `"datasheet"` part key);
`planning/plan-74-A-ethernet-data.md` Phase 4 (the download and `datasheets.tsv` precedent);
`datasheets/obsolete-not-included.md`, `datasheets/skipped-parts.md` (formats).

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-82-A complete | `ls planning/completed/plan-82-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| The maker hosts serve PDFs | `curl -sL -A "Mozilla/5.0" -o /tmp/p82.pdf -w '%{http_code}' <url>; head -c5 /tmp/p82.pdf` for one URL per maker host of `candidates.tsv` → `200 %PDF-` | MET for Kingbright (`www.KingbrightUSA.com`, APT1608EC.pdf, 455,813 B) and SunLED (`www.sunledusa.com`, XZMG53W-1.pdf, 258,126 B), 2026-11-25. Broadcom and QTB hosts are read off A's `candidates.tsv`; Phase 1 probes them first |

## 1. Goal

- `datasheets/led/{kingbright,broadcom,sunled,qtb}/` hold every PDF an `add` row cites, each folder
  with a `datasheets.tsv` (file, URL, bytes, SHA-256).
- Every PDF opens (`pdfinfo` exit 0) and starts `%PDF-`.
- A PDF all of whose parts DigiKey or the maker marks obsolete is deleted and recorded in
  `datasheets/obsolete-not-included.md` (PDF name; family "LED", category "LED Indication -
  Discrete", group = the maker); its rows become `skip` (`obsolete`).
- `specs.tsv` has one row per (maker, datasheet, package drawing) of the `add` rows, columns
  `maker datasheet parts package mount pins dies colors wavelength mcd vf if viewingAngle lens
  footprint drawing_page note`, with every column after `mount` empty.

### Non-goals

- No change to `Parts/*.json`, `footprints.json`, `datasheets.json`, `backend/` or the app (H adds
  the `datasheets.json` entries with the parts).
- No PDF of a `skip` row is kept.

## 2. Current State

There is no `datasheets/led/` folder (`ls datasheets/led` → no such directory, 2026-11-25). A's
`candidates.tsv` carries each row's `datasheet_url`. Kingbright publishes one PDF per part number
(the census URL is `…/SPEC/<part>.pdf`), so Kingbright's 300 rows give up to 300 PDFs; SunLED's
URL is `…/products/spec/<part>.pdf`, also one per part. Broadcom and QTB group a series per PDF
(their census rows cite series PDFs); B measures the distinct-URL count of each in Phase 1.

## 3. Design

- `census/fetch.py` (Python `urllib`, Safari user agent and `Accept`/`Accept-Language` headers, as
  `planning/plan-74-data/census/fetch.py`), one request per distinct URL, 1 s apart, 3 retries.
  A DigiKey-hosted URL (`mm.digikey.com`) goes through `scripts/safari-digikey-pdf.sh` instead.
- File name: the URL's last path segment, lower-cased, `%20` → `-`; a name collision within a
  maker folder gets the part number prefixed.
- Obsolete check: a PDF is obsolete when every part it covers (its rows in `candidates.tsv`, plus
  every part number its ordering table lists) is marked obsolete by DigiKey or by the PDF itself.
  Since every A `in` row is Active or Last Time Buy, no PDF is expected to be all-obsolete; the
  check still runs and records its count.
- The skeleton: one row per (datasheet, package drawing) — a Kingbright or SunLED PDF is one
  drawing; a Broadcom / QTB series PDF has one row per package option its ordering table names.

## 4. Phases

### Phase 1: downloads

- [ ] Write `census/fetch.py`; probe one URL per host (`--probe`).
- [ ] Download every `add` row's PDF; write the four `datasheets.tsv`.

Acceptance: `python3 -I planning/plan-82-data/census/fetch.py --verify` prints `missing 0 bad 0`
and the PDF count per maker — ~60 min (at most 1,200 downloads at 1 s: Kingbright and SunLED one PDF per part, Broadcom and QTB fewer).

Commit:

### Phase 2: obsolete check, verdict updates

- [ ] Apply § 3's obsolete rule; update `verdicts.tsv` (`datasheet 404`, `obsolete`) and
      `datasheets/obsolete-not-included.md`.

Acceptance: `python3 -I planning/plan-82-data/census/fetch.py --verify` still `missing 0 bad 0`;
`grep -c 'plan-82' datasheets/obsolete-not-included.md` equals the `obsolete` count of
`verdicts.tsv` — 1 min.

Commit:

### Phase 3: `specs.tsv` skeleton

- [ ] Write the skeleton (§ 3).

Acceptance: `python3 -I -c "import csv;r=list(csv.DictReader(open('planning/plan-82-data/specs.tsv'),delimiter='\t'));print(len(r),sum(1 for x in r if x['pins']))"`
prints the row count and `0` — 1 min.

Commit:

## 5. Validation Plan

B ships no code: its gate is Phase 1's `--verify`. The landing tier is J's.
