# plan-51-B: IC sockets — the full listing, stock re-check, datasheet downloads

Last updated: 2026-11-21
Effort: large (3h–1d; plus about 80 DigiKey calls)
Depends on: plan-51-A (letter order; B reads nothing A produces)

plan-51 adds DIP and PLCC IC sockets (see plan-51-A for the whole feature). B
produces the **data**: the complete list of candidate sockets of the three
makers, each re-checked in stock, and the maker datasheet PDFs they cite,
downloaded under `datasheets/ic-socket/`.

Outcome of B: `planning/plan-51-data/candidates.tsv` lists every candidate of
Mill-Max, Preci-Dip (DIP **and PLCC**) and Assmann with `stock_checked` filled;
`datasheets/ic-socket/<maker>/` holds the PDFs the rows cite, with a
`datasheets.tsv` in each; every row's `pdf` column names a downloaded file.

References:

- `AGENTS.md` (§ Datasheets: in-stock rule, obsolete rule);
  `docs/parts-format.md` (§ Datasheets: the `url` is on the manufacturer's own site).
- `planning/plan-51-data/` (README, `census/`), plan-51-A § 2 (the scope shapes).
- `planning/completed/plan-45-A-terminal-jst-data.md` (the precedent: download, URL check).
- Memory notes: DigiKey credentials come from `~/.zshrc` through
  `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"` in a script
  file; the quota is 1000 calls a day, shared with other sessions.

## Prerequisites

Rows 1 and 4 of plan-51-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-51-A complete | `ls planning/completed/plan-51-A-*.md` → exists | MET (2026-11-24: `planning/completed/plan-51-A-socket-kind.md`, commit 83da69bc) |
| The DigiKey quota is available | `node scripts/digikey.js --opt2 --limit 1 "74HC00"` (with the credentials) → a table, not HTTP 429 | MET on the second key (2026-11-24 18:01 UTC: a table, 399 calls left; the standard key is spent until 2026-11-25 00:00 UTC; plan-45's census is on the second key today too, so the quota is shared) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

- `python3 -I planning/plan-51-data/census/tally.py` prints the counts of § 2 with
  Preci-Dip's PLCC rows included, from complete listings of the three makers.
- Every candidate row has `stock_checked` = the date of a fresh in-stock check, and
  rows that no longer pass are marked `out` with the reason.
- Every PDF a candidate row cites is in `datasheets/ic-socket/<maker>/`, is a PDF
  (`file` says `PDF document`), and has a URL on the maker's own site in `datasheets.tsv`.

### Non-goals (explicit constraints)

- No part, footprint or code change. No part is judged here for its land pattern (C).
- The candidate scope is fixed (§ 2 rule); B does not add shapes or makers.
- DigiKey-hosted datasheet copies are never the `url` (the rule in `docs/parts-format.md`).

## 2. Current State

The planning census (2026-11-21) ran `census/list.sh`: Mill-Max is complete,
Preci-Dip stopped on HTTP 429 after its DIP types (`planning/plan-51-data/raw/1212.err`).
Scope rule (`census/common.py:scope`): category 409, in stock, Active or Last Time
Buy, Mounting Type exactly `Through Hole` or `Surface Mount`, Type `DIP, 0.3"` /
`DIP, 0.6"` (not ZIF) with `N (2 x M)` pins, mating pitch 0.100", N one of the
shape's pin counts; or Type `PLCC` with 28, 32, 44 or 68 pins.

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidates, Mill-Max / Preci-Dip (DIP only so far) / Assmann | 320 / 203 / 46 = 569 | `python3 -I planning/plan-51-data/census/tally.py` → `in scope per maker` (Mill-Max 320, Preci-Dip 203, Assmann 46) and `three makers: 569` |
| Other makers' candidates (not in scope) | Samtec 51, Aries 21, 3M 20, Adam Tech 20, TE AMP 9, Amphenol ICC 8 | same command, same line |
| `(maker, series, family, mounting, spacing)` groups of the three | 53 | same command → `groups: 53` |
| Preci-Dip PLCC parts in stock (a keyword search, not yet listed) | 7 | `node scripts/digikey.js --in-stock --active-only --category 409 --manufacturer 1212 --limit 1 --json "PLCC"` → `ProductsCount` 7 (2026-11-21) |
| Distinct DigiKey datasheet URLs of the three makers' candidates | 15 | same tally → `distinct datasheet URLs: 15` |
| Candidates whose DigiKey datasheet link is DigiKey-hosted | 319 (317 Mill-Max `mm.digikey.com`, 1 Assmann `mm.digikey.com`, 1 Preci-Dip `media.digikey.com`); 32 Preci-Dip rows have no link; 3 Mill-Max rows cite a `mill-max.com/catalog/download/…` URL that answers 404 | `cut -f1,10 planning/plan-51-data/candidates.tsv \| sort \| uniq -c` |

### Verified properties (2026-11-21)

| Claim | How verified |
|---|---|
| Mill-Max publishes its IC-socket catalog pages as one PDF on its own site | `curl -sL -A 'Mozilla/5.0' https://www.mill-max.com/sites/default/files/external/catalog/2018-12/088-145.pdf -o x.pdf -w '%{http_code} %{content_type} %{size_download}'` → `200 application/pdf 20612647`; `pdftotext -layout` → 6325 lines naming series 110, 111, 114, 115, 210, 214, 540 and 940 (the 540 and 940 are PLCC) |
| Mill-Max's per-part data sheet pages are not PDFs a script can fetch | `curl -sL https://www.mill-max.com/products/datasheet/110-41-308-41-001000` → `400 text/plain` |
| Preci-Dip's DIL socket catalog downloads | `curl … https://precidip.com/wp-content/uploads/2024/10/CATALOG-DILSOCKETS.pdf` → `200 application/pdf 2404695` |
| Assmann's datasheets download | `curl … https://www.assmann-wsw.com/uploads/datasheets/ASS_4852_CO.pdf` → `200 application/pdf 460261` |

## 3. Design

A data task. The risk is the datasheet URL rule: Mill-Max's DigiKey link is a
DigiKey copy, so the Mill-Max `url` is the maker's catalog PDF above (the same
document, on the maker's site); Preci-Dip's 32 rows with no link are looked up on
precidip.com (the catalog covers DIL sockets; a PLCC row needs its own page).
A candidate whose PDF does not show its land pattern is `out` with that
reason, and C moves it to `datasheets/skipped-parts.md` (its format), never into a part.

## 4. Phases

### Phase 1 — the listing (needs the quota)

- [x] `rm planning/plan-51-data/raw/1212.jsonl.gz` and run `planning/plan-51-data/census/list.sh planning/plan-51-data/raw` (credentials by the `eval` in a script file); it lists Preci-Dip again from the start and ends `list.sh: all 12 makers done`, or stops on 429 (rerun the next day: finished makers are skipped). — ran on the second key (§ Corrections: `list.js` moved to `scripts/digikey.js`'s transport first); the output ended `list.sh: all 12 makers done`; Preci-Dip 1007 parts in 32 calls, the other ten makers 14 calls, 358 calls left on the second key afterwards.
- [x] `gzip -9 planning/plan-51-data/raw/*.jsonl`. — done; `ls planning/plan-51-data/raw/*.jsonl 2>&1` → no match.
- [x] `census/tally.py --candidates` writes four more empty trailing columns (`pdf`, `state` = `candidate`, `reason`, `stock_checked`; columns 11–14); then `python3 -I planning/plan-51-data/census/tally.py --candidates planning/plan-51-data/candidates.tsv`; copy the new counts into § Corrections (the table above is the planning-time record). — counts in § Corrections.

Acceptance: `raw/1212.done` exists; `candidates.tsv` has Preci-Dip PLCC rows.
  Check: `ls planning/plan-51-data/raw/*.done \| wc -l` → 12 and `awk -F'\t' '$1=="Preci-Dip" && $4=="PLCC"' planning/plan-51-data/candidates.tsv | wc -l` → the number of Preci-Dip PLCC rows (est. 30 min of calls). Measured 2026-11-24: `ls planning/plan-51-data/raw/*.done | wc -l` → 12; the awk → 4 (`540-88-032-24-008`, `540-88-044-24-008`, `540-88-068-17-400-2`, `540-88-068-24-008`). VERIFIED.
Commit: (next commit)

### Phase 2 — datasheets

- [ ] Fill column 11 (`pdf`) of `candidates.tsv`: the downloaded file's path.
- [ ] `datasheets/ic-socket/mill-max/088-145.pdf` from the URL in § 2; `datasheets/ic-socket/preci-dip/` the DigiKey-listed Preci-Dip PDFs (`CATALOG-DILSOCKETS.pdf`, and the PDF of the `AppHost` link) and the PDF for each of the 32 rows with no link, found on precidip.com; `datasheets/ic-socket/assmann/ASS_*_CO.pdf` for each distinct URL of the Assmann rows.
- [ ] A `datasheets.tsv` in each folder: `file`, `url` (maker's own site), `bytes`, `parts` (candidate rows that cite it).
- [ ] Every Mill-Max series of the candidates (110, 111, 114, 115, 116, 121, 122, 123, 126, 146, 210, 214, 540, 605, 612, 614, 940, 104: the `series` column of `candidates.tsv`) is looked up in `088-145.pdf` with `pdftotext`; a series `pdftotext` does not find in it has its rows marked `out`, reason `no drawing`.

Acceptance: every candidate row's `pdf` is a file; each file is a PDF.
  Check: `for f in datasheets/ic-socket/*/*.pdf; do file "$f"; done \| grep -vc 'PDF document'` → 0 (est. 15 min).
Commit: —

### Phase 3 — stock re-check

- [ ] A fresh listing of the three makers (`list.sh` into `raw-recheck/`, deleted after): `stock_checked` (column 14) = the run date for every row still listed in stock and Active or Last Time Buy; the others `state` `out`, `reason` `no stock`.

Acceptance: no candidate row has an empty `stock_checked` or `state`.
  Check: `awk -F'\t' 'NR>1 && ($12=="" || $14=="")' planning/plan-51-data/candidates.tsv | wc -l` → 0 (est. 30 min of calls).
Commit: —

## Validation Plan

- Tests: none (data). Coverage: not applicable.
- Runtime proof: `tally.py` after Phase 3 prints the final counts, recorded in plan-51-C § 2.
- Doc sync: `datasheets/skipped-parts.md` only if a row is dropped for lack of a drawing (C does it).
- Final gate: plan-51-J.

## Decisions

Owner decisions: see plan-51-A § Decisions (the three makers, through hole and surface mount, DIP and PLCC).

Author decisions:

- **The datasheet of a Mill-Max part is the catalog PDF `088-145.pdf`**, since the per-part pages are not PDFs and the DigiKey copy is not the maker's site.
- **A candidate's `datasheet` id (F) is by the local PDF's name in lower case**, as `docs/parts-format.md` § Datasheets says: `mill-max-088-145`, `precidip-catalog-dilsockets`, `assmann-ass-4852-co`.

## Corrections

- **2026-11-24, `census/list.js` and `list.sh` changed before Phase 1 could run.** Cloudflare has challenged every Node request to `api.digikey.com` since 2026-11-23 (after the plan was written), so `list.js`'s own `fetch` could not log in. It now sends its requests through `scripts/digikey.js`'s `credentials`, `token` and `fetchRetry` (the Safari fallback; the module exports them for census scripts) and uses the second key unless `DK_OPT2=0` (owner, 2026-11-24: "using opt2"; the first key was spent). `list.sh` exports the second key's variables too. Same change as `planning/plan-45-data/census/dk.js`.
- **2026-11-24, Phase 1 counts** (`python3 -I planning/plan-51-data/census/tally.py`): in scope per maker Mill-Max 320, Preci-Dip 207 (was 203: +4 PLCC), Samtec 50, Assmann 46, Aries 21, 3M 20, Adam Tech 20, TE AMP 10, Amphenol ICC 8; `three makers: 573` (was 569); groups 55 (was 53); variants 207; distinct datasheet URLs 17 (was 15). The Preci-Dip PLCC rows are series 540: 032 TH, 044 TH, 068 TH and 068 SMT. The planning note of 7 Preci-Dip PLCC parts in stock was a keyword search (any PLCC, including types and pin counts out of scope); the scope rule keeps 4. Per shape: see the tally's `per shape` line (PLCC: 28 SMD 1, 28 TH 3, 32 SMD 3, 32 TH 4, 44 SMD 3, 44 TH 4, 68 SMD 2, 68 TH 4 across the three makers).
- **2026-11-24, Phase 3 re-lists only Mill-Max.** Phase 1's run listed Preci-Dip and Assmann today (the same date Phase 3 would stamp), so a second listing of them would spend about 35 calls for the same answer; Mill-Max's listing (`raw/54.jsonl.gz`) is from 2026-11-21 and is the one that is stale.

## Summary

B's risk is the quota and the 32 Preci-Dip rows with no datasheet link. Untouched:
all code and every part file.
