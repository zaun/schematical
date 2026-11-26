# plan-79-B: Battery holders: datasheet and drawing downloads, the obsolete check, `drawings.tsv`

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-79-A

B downloads every PDF an `add` or `pending` row of `verdicts.tsv` cites, plus the Keystone catalog
pages those PDFs point to for their PCB layout, and records how each drawing reads, so D–H know
where every number comes from before they start.

References: plan-79-A (§ 2 Verified properties: the two hosts, the "Refer to page 41" note, MPD's
outlined drawings); `AGENTS.md` § Datasheets (the obsolete rule); `docs/parts-format.md`
§ Datasheets (`datasheets.json` entries); `planning/plan-74-A-ethernet-data.md` Phase 4 (the
download precedent); `datasheets/obsolete-not-included.md` (format).

## Prerequisites

plan-79-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-79-A complete | `ls planning/completed/plan-79-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| The maker hosts serve PDFs to Python | `python3 -I planning/plan-79-data/fetch.py --probe` (Phase 1 writes it from the planning probe) → 2 lines `200 … %PDF-` | MET as a probe at planning (2026-11-25: `K75p16.pdf` 632,180 B, `BCAA-datasheet.pdf` 141,724 B; plan-79-A § 2) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `datasheets/connector-battery/{keystone,mpd,adam-tech,assmann}/` hold every PDF an `add` or
  `pending` row cites, and every Keystone layout page those PDFs refer to, each folder with a
  `datasheets.tsv` (file, URL, bytes, SHA-256).
- `planning/plan-79-data/drawings.tsv` has one row per PDF: `file pages words kind layout_page note`,
  `kind` = `text` (dimensions are text), `outlined` (dimensions are vector outlines: read from a
  600 dpi `pdftoppm` render) or `image` (a raster drawing: read from the image at its own resolution).
- `specs.tsv`'s `layout_page` and `drawing_kind` columns are filled for every row.
- Every PDF whose parts are all obsolete is deleted and recorded in
  `datasheets/obsolete-not-included.md` (family "Connector → Battery"), and its rows are `skip`
  (`obsolete`), each promoting a reserve row (plan-79-A § 3.3).

### Non-goals

- No part, footprint or code change. No pin or pad is read here (D–H).

## 2. Design

- `fetch.py <url> <out>`: Python `urllib`, Safari user agent plus `Accept` and `Accept-Language`
  (plan-74-A's header set), 3 tries; writes the file only when it starts with `%PDF-`; prints
  `200 <bytes> %PDF-`. `--probe` fetches the two planning URLs to `/tmp`. A DigiKey copy
  (`mm.digikey.com`) is fetched with `scripts/safari-digikey-pdf.sh` (AGENTS.md).
- **DigiKey-copy rows** (`pending`, plan-79-A § 3.4): fetch the maker's own pattern URL; on
  `%PDF-` the row is `add` with that URL, else `skip` (`no maker datasheet`), promoting a reserve row.
- File names: the URL's last path element, lower case; a clash gets the part number prefixed.
- **Keystone layout pages.** `pdftotext` each Keystone PDF and grep `Refer to page (\d+)`; each
  page number `N` is fetched as `K75p<N>.pdf` from the same folder (`K75p16.pdf` → `K75p41.pdf`),
  the catalog's own naming. A referenced page that is not served (`fetch.py` → not `%PDF-`) makes
  the rows that need it `skip` (`no layout page`).
- **Drawing kind.** `pdftotext <pdf> - | wc -w`: 100 words or more with dimension numbers next to
  the drawing → `text`; fewer and `pdfimages -list` shows no full-page image → `outlined`; a
  page-sized image → `image`. The rule is checked by hand on the first two of each maker.
- **Obsolete check** (AGENTS.md): a PDF is obsolete-only when every part it covers (its ordering
  table, or the parts DigiKey links to it) is Obsolete or Discontinued at DigiKey or the maker
  marks it so. The listing rows carry DigiKey's status; the maker's mark is read in the PDF text.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Downloads

- [ ] `planning/plan-79-data/fetch.py` per § 2; `--probe` passes.
- [ ] Fetch every `add` / `pending` row's PDF and every Keystone layout page; the four
      `datasheets.tsv` files.

Acceptance: every cited PDF is on disk and is a PDF.
  Check: `python3 -I planning/plan-79-data/census/build.py --pdfs` → `ok: <n> PDFs, every add/pending row's file present, every file starts %PDF-` (est. 20 min of downloads; the check < 1 min).
  Result:
Commit:

### Phase 2 — Drawing kinds and the obsolete check

- [ ] `drawings.tsv` per § 2; `specs.tsv`'s `layout_page` and `drawing_kind`.
- [ ] Obsolete check; deletions and `obsolete-not-included.md` rows; promotions.

Acceptance: every PDF has a kind and every `specs.tsv` row a layout page or a `skip`.
  Check: `python3 -I planning/plan-79-data/census/build.py --drawings` → `ok: <n> PDFs classified (text <a>, outlined <b>, image <c>), every specs row has layout_page` (est. < 1 min).
  Result:
Commit:

## Validation Plan

- Phase checks above. Final gate: plan-79-J.

## Decisions

- Author: **the Keystone catalog's own page files are the layout source.** A Keystone product PDF
  is one catalog page and points to the page that holds the PCB layout; fetching that page by the
  catalog's naming keeps every number in a Keystone document.
- Author: **classify before reading.** MPD's drawings carry their numbers as outlines (plan-79-A
  § 2), so the reading method is decided per PDF here, once.

## Corrections

## Summary

Every battery-holder PDF on disk with its layout page and reading method recorded, obsolete-only
PDFs removed.
