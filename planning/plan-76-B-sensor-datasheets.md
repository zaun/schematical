# plan-76-B: Sensors: datasheet downloads, text verdicts, obsolete checks, spec skeletons

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-A

B downloads the datasheet of every `in` row of `candidates.tsv`, gives each row a verdict from the
PDF's text, records skipped and obsolete parts as AGENTS.md asks, and writes the seven
`specs-<family>.tsv` skeletons that C–K fill.

References:

- plan-76-A § 3.1 (`candidates.tsv` columns), § Decisions; `planning/plan-76-data/plan-76.md`
  (the planning download probe, one URL per host).
- `AGENTS.md` § Datasheets (obsolete rules); `docs/parts-format.md` § Datasheets
  (`datasheets.json` entries are added by F, G, I, J, L, M, not here).
- Precedent: `planning/plan-74-A-ethernet-data.md` § 3.4–3.5 (`fetch.py`, `textscan.py`, reserves,
  skeletons).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-A complete | `ls planning/completed/plan-76-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| The maker sites serve their PDFs to Python | `python3 -I planning/plan-76-data/census/fetch76.py --probe` (Phase 1 writes it from the planning probe) → 21 lines `200 … %PDF-` | MET as a probe at planning (2026-11-25: 21 URLs gave a PDF, `planning/plan-76-data/plan-76.md`; Broadcom needs `/docs/` → `/doc/`; one All Sensors `allsensors.com` link was a 404) |
| `scripts/safari-digikey-pdf.sh` for DigiKey-hosted copies | `scripts/safari-digikey-pdf.sh https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/323/MXC400xXC_Rev.B_4-24-15.pdf /tmp/p76-mxc.pdf && head -c5 /tmp/p76-mxc.pdf` → `%PDF-` | the AGENTS.md route; run at Phase 1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `datasheets/sensor/<family>/<maker>/` holds each PDF an `add` row cites, with one
  `datasheets.tsv` per family folder (file, URL, bytes, SHA-256).
- `verdicts.tsv` has one row per `in` row of `candidates.tsv`: `add` or `skip` with a reason
  (§ 3.2), and `candidates.tsv`'s `verdict`, `verdict_reason` and `pdf` columns are filled.
- `datasheets/skipped-parts.md` has a section "Sensors (plan-76)" listing every `skip` row.
  `datasheets/obsolete-not-included.md` gets a row for each PDF all of whose parts are obsolete
  (that PDF is deleted). A part the PDF marks obsolete while others of it are not gets
  `obsolete=1` in `verdicts.tsv`, which M turns into `"flags": ["obsolete"]`.
- `specs-<family>.tsv` exist (7 files), one row per (maker, datasheet, package) of the `add` rows,
  with their pinout and facts columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `datasheets.json`, `backend/` or the app.
- No pin table read here (C–K).

## 2. Current State

`candidates.tsv` and `stock-check.tsv` exist from A. Datasheet hosts of the census pages' first
rows per maker (planning probe): `www.analog.com`, `www.ti.com` (DigiKey links
`suppproductinfo.tsp?…gotoUrl=…/lit/gpn/<part>`; 5 of TI's first 52 temperature rows link
`rocelec.widen.net`, Rochester copies), `ww1.microchip.com`, `www.nxp.com`, `sensirion.com`,
`www.amphenol-sensors.com`, `www.te.com` (DDEController), `www.ist-ag.com`,
`428059.fs1.hubspotusercontent-na1.net` and `www.allsensors.com` (All Sensors),
`prod-edam.honeywell.com`, `www.st.com`, `www.memsic.com`, `www.allegromicro.com`,
`www.diodes.com`, `www.infineon.com`, `look.ams-osram.com` and `ams.com`, `www.vishay.com`,
`docs.broadcom.com`, `www.lem.com`, `www.melexis.com`, and DigiKey copies `mm.digikey.com`
(Memsic 5 of 9, Honeywell 10 of 25, IST 3 of 7, Allegro 7 of 51 first rows).

## 3. Design

### 3.1 Download (`census/fetch76.py`)

- Python `urllib` with a Safari user agent, `Accept: application/pdf,*/*` and
  `Accept-Language: en-US,en;q=0.9` (the planning probe's headers). A body that does not start
  `%PDF-` is a failure.
- Rewrites:
  - TI `suppproductinfo.tsp?…gotoUrl=…/lit/gpn/<part>` → `https://www.ti.com/lit/ds/symlink/<part>.pdf`;
    a TI row that links `rocelec.widen.net` → the same symlink URL of its base part (TI's current
    datasheet, not the Rochester copy; memory: TI no-datasheet check — a 404 there is `skip`,
    `no PDF`).
  - Broadcom `docs.broadcom.com/docs/<id>` → `docs.broadcom.com/doc/<id>`.
  - `//host/…` → `https://host/…`.
- `mm.digikey.com` links go through `scripts/safari-digikey-pdf.sh` (AGENTS.md).
- English PDFs only. DigiKey links some non-English copies (Infineon's first 554 row:
  `infineon-tle493d-w2b6-datasheet-cn.pdf`, planning probe). A file name ending `-cn`, `-zh`,
  `-jp` or `-ja`, or a text layer that is mostly CJK, is replaced by the same URL with `-en`; when
  that fails, the row is `skip` (`no English PDF`).
- A failed download (HTTP error, not a PDF) after the rewrites: the row's DigiKey product page is
  loaded through `scripts/safari-digikey.sh` and its other `Datasheet` link is tried once; then the
  row is `skip` (`no PDF`).
- Files are named by the maker's file name in lower case. Several part numbers that serve
  byte-identical PDFs (by SHA-256) share one file.
- `--probe`: downloads the 21 planning URLs of `plan-76.md` that gave a PDF (one per host, Broadcom in its `/doc/` form)
  and prints `200 <bytes> %PDF- <url>` or the failure.

### 3.2 Verdicts (`census/textscan76.py`, `pdftotext -layout`)

- `add`: the part number, less DigiKey's packaging suffix (`-TR`, `-R`, `/TR`, `-REEL`, `T` reel
  forms, `-T&R`, `-CT`, Microchip's leading `T`), or its ordering-code pattern with the
  datasheet's option letters (`TMP117xxDRVR` style tables are matched by the regular expression the
  ordering table prints), appears in its PDF's text, and the PDF prints no obsolete / not
  recommended for new designs / discontinued status for it.
- `skip` reasons: `no PDF`, `no English PDF`, `not in PDF`, `PDF obsolete`.
- **Reserves.** A `skip` frees a capped maker's slot: the next `cap` row of that (family, maker)
  becomes `in`, gets its stock check (A § 3.3, from the saved row) and its verdict, until 300 are
  `add` or no row is left (only ADI temperature, All Sensors and Honeywell pressure have reserves).
- An image-only PDF (`pdftotext … | wc -w` under 200) is read by eye on its ordering page; the
  verdict records `by eye p.<n>`.

### 3.3 Spec skeletons

`specs-<family>.tsv`: `maker  datasheet  package  footprint  parts  measures  output  ranges
accuracy  address  extra  vcc  pinout  note`. One row per (maker, PDF, DigiKey package) of the
family's `add` rows; `parts` is the comma list of part numbers. `measures` and `output` are
pre-filled from DigiKey's Sensor Type and Output Type with the mapping of plan-76-L § 3.1; C–K
check them against the datasheet. The other columns are empty.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Downloads

- [ ] `planning/plan-76-data/census/fetch76.py` per § 3.1, with `--probe`.
- [ ] Download every `in` row's PDF into `datasheets/sensor/<family>/<maker>/`, with the seven
      `datasheets.tsv` files.

Acceptance: every `in` row has a local PDF or a `no PDF` reason.
  Check: `python3 -I planning/plan-76-data/census/fetch76.py --verify` → `<n> rows, <p> PDFs, <k> no PDF, 0 unaccounted` (est. 45 min: the downloads; Safari only for DigiKey copies).
  Result:
Commit:

### Phase 2 — Verdicts, obsolete checks, skeletons

- [ ] `census/textscan76.py` per § 3.2; `verdicts.tsv`; reserves moved up; `candidates.tsv`'s
      verdict columns filled.
- [ ] `datasheets/skipped-parts.md` § "Sensors (plan-76)"; `datasheets/obsolete-not-included.md`
      rows (PDF name, family `sensor` / group `<family>`) and those PDFs deleted.
- [ ] The seven `specs-<family>.tsv` skeletons per § 3.3.
- [ ] Record the `add` counts by family and maker here
      (`awk -F'\t' 'NR>1 && $16=="add"{print $1,$2}' planning/plan-76-data/candidates.tsv | sort | uniq -c`).

Acceptance: every `add` row has a local PDF, every `skip` row is listed in `skipped-parts.md`, and
no (family, maker) has more than 300 `add` rows.
  Check: `python3 -I planning/plan-76-data/census/textscan76.py --verify` → `<a> add, <s> skip; 0 add without a PDF; 0 skip missing from skipped-parts.md; cap ok`; `find datasheets/sensor -name '*.pdf' | wc -l` equals the distinct `pdf` values of the `add` rows (est. 30 min: one `pdftotext` per PDF and the by-eye pages).
  Result:
Commit:

## Validation Plan

- B's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **one folder per family and maker**, `datasheets/sensor/<family>/<maker>/`, since a
  maker's PDFs are counted and checked per family (TI and Allegro are in two or three families).
- Author: **TI's current datasheet, not a Rochester copy**, for TI rows DigiKey links to
  `rocelec.widen.net`: the symlink PDF is TI's own and carries the current ordering table.
- Author: **reserves move up on a skip**, as plan-48-A and plan-74-A, so a capped maker still
  reaches 300 parts when some of its top rows have no usable datasheet.

## Corrections

## Summary

B downloads every in-scope sensor datasheet, keeps each part whose number its PDF prints, records
the rest in `skipped-parts.md`, and hands C–K one specs skeleton per family.
