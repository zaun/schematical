# plan-58-E: Slide switches — E-Switch: schemes, footprints, parts

Last updated: 2026-11-24
Effort: medium (1h–2h)
Depends on: plan-58-D

E adds the E-Switch slide switches to the library with the tools plan-58-C
built: it decides the maker's `pending` rows against the datasheets'
part-number keys, draws one footprint per distinct recommended layout, fills
the maker's rows of `specs.tsv`, and runs `gen-parts.py` for it.

Outcome: every kept E-Switch row of `candidates.tsv` is a part
(`grep -c 'states=' backend/tests/golden/parts.txt` rises by the number of E-Switch
`add` rows); `Parts/footprints.json`, `Parts/datasheets.json` and the three
goldens gain exactly the new lines; `ctest` passes.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` § `switch`, § Footprints;
  `docs/backend.md` (goldens).
- `planning/plan-58-C-slide-switch-parts-ck.md` § 4 (schemes, specs, footprint
  rules, generator): this letter uses them unchanged. `planning/plan-58-B-slide-switch-kind.md`
  § 4.3 (pin names).
- E-Switch datasheets: `datasheets/switch/slide/eswitch/` (8 PDFs at planning, plan-58-A).

## Prerequisites

The gate for the whole feature is in plan-58-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-58-D is complete | `ls planning/completed/plan-58-D-* \| wc -l` → 1 | MET (2026-11-24, P-58) |
| The generator exists | `ls planning/plan-58-data/gen-parts.py planning/plan-58-data/census/scheme-check.py planning/plan-58-data/census/specs-check.py planning/plan-58-data/census/fp-load.sh \| wc -l` → 4 | MET (2026-11-24, P-58: plan-58-C archived, 98c5cefb; plan-58-D archived, 7b2c459b) |
| E-Switch's PDFs are placed | `ls datasheets/switch/slide/eswitch/*.pdf \| wc -l` → 8 (or fewer, when A skipped a PDF's rows) | MET (2026-11-24) |
| No E-Switch slide parts yet | `python3 -I -c "import csv;t=open('schematical/schematical/Parts/switch.json').read();m=[r['mpn'] for r in csv.DictReader(open('planning/plan-58-data/candidates.tsv'),delimiter='\t') if r['maker']=='E-Switch' and r['decision']=='add'];print(sum(('\"name\":\"'+x+'\"') in t for x in m))"` → 0 | MET (2026-11-24) |

If plan-58-D is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `schemes.tsv` has a row per E-Switch series that has `pending` rows; no E-Switch
  row is `pending`.
- `specs.tsv` has `mpn_regex`, `pins`, `footprint`, `note` for every E-Switch row;
  `census/specs-check.py --maker 'E-Switch'` exits 0.
- `Parts/switch.json` has the E-Switch devices and parts, `Parts/footprints.json`
  the footprints they name (moved from `planning/plan-58-data/footprints-E.json`),
  `Parts/datasheets.json` an entry per datasheet the parts name; the goldens have
  exactly the new lines added.

### Non-goals (explicit constraints)

- Every existing device, part and golden line (including C&K's) is byte-identical.
- No backend code, no app code, no other maker's rows. A part not in stock at
  DigiKey, or whose number the datasheet does not support, is `skip` (AGENTS.md).

## 2. Current State

C landed C&K's parts and `gen-parts.py`; `Parts/switch.json` holds the `slide_`
region before the tactile one. `candidates.tsv` has the E-Switch rows; `specs.tsv`
has their skeleton rows (plan-58-A).

### Measured populations

| What | Count | Command |
|---|---|---|
| E-Switch rows kept by DigiKey rules (`add` at planning) | 63 | `awk -F'\t' '$1=="E-Switch" && $18=="add"' planning/plan-58-data/candidates.tsv \| wc -l` |
| …verbatim in the PDF text | 20 | planning-time probe (`pdftotext` + substring); A records the real `hit` column |
| …series / (series, mounting) / (series, mounting, circuit) | 6 / 16 / 23 | `awk -F'\t' '$1=="E-Switch" && $18=="add"{print $3}' … \| sort -u \| wc -l`; likewise `$3,$11` and `$3,$11,$4` |
| …by series | EG 28, 500 17, 500A 7, 500R 4, EGJ1210 4, 600 3 | `awk … \| sort \| uniq -c` |
| …by circuit and action | SPDT On-On 36, DPDT On-On 21, SPDT On-Off-On 5, DPDT On-Off-On 1 | `awk -F'\t' '$1=="E-Switch" && $18=="add"{print $4,$5}' … \| sort \| uniq -c` |
| Distinct datasheet URLs | 8 (7 on `configured-product-images.s3.amazonaws.com`, 1 on `mm.digikey.com`) | `awk -F'\t' '$1=="E-Switch" && $18=="add"{print $16}' … \| sort -u \| wc -l` |
| Rated volts (DigiKey DC rating, else AC) | 4, 6, 12, 15, 20, 24, 28, 30 | `awk -F'\t' '$1=="E-Switch" && $18=="add"{print $9, $8}' … \| sort -u` |

### Verified properties

- The formats, rules and checks are plan-58-C § 2 and § 4 (including: `footprint_check`
  fails on an unused footprint, so the footprints stay staged in
  `planning/plan-58-data/footprints-E.json` until `gen-parts.py` moves them with
  their parts).
- E-Switch's datasheet URLs are not on the maker's own site (DigiKey's S3 bucket and `mm.digikey.com`); `maker-urls.tsv` (plan-58-A Phase 3) holds each one's `maker_url` or leaves it empty, and `gen-parts.py` writes the `maker_url` when set, else the DigiKey URL (precedent: the `06-ts35m943160tr` entry of `datasheets.json`).

## 3. Design Overview

The same four pieces as plan-58-C § 3, with the same risk (a wrong pad is valid
data; the image-against-drawing comparison catches it) and the same gate
(every other golden line byte-identical). E-Switch's PDFs hold several series each, so a `specs.tsv` row names the PDF page of its drawing in `note`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` partial; moot tasks stay with
> evidence; fill each phase's `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — decide the pending rows

- [x] Read each E-Switch PDF's part-number key; add the E-Switch rows to
      `planning/plan-58-data/schemes.tsv` (plan-58-C § 4.1).
- [x] `python3 -I planning/plan-58-data/census/scheme-check.py --maker 'E-Switch'`; commit
      `candidates.tsv` and the skipped-parts section.

Acceptance: no E-Switch row is `pending`; each `skip` reason is listed in
`datasheets/skipped-parts.md`.
  Check: `awk -F'\t' '$1=="E-Switch" && $18=="pending"' planning/plan-58-data/candidates.tsv \| wc -l` → 0 (est. 3 min).
  Measured: `scheme-check.py --maker 'E-Switch'` → `E-Switch 56 add, 7 skip, 0 pending`; `awk` pending count → 0; `decide.py --verify-skipped` exit 0
Commit: 18db34d0

### Phase 2 — footprints and specs

- [x] For each of E-Switch's (series, mounting, circuit) lines (23 at planning): read
      the PDF drawing, write the footprint (plan-58-C § 4.3) as a ready object in
      `planning/plan-58-data/footprints-E.json` and its `specs.tsv` row (§ 4.2).
- [x] `python3 -I planning/plan-58-data/census/specs-check.py --maker 'E-Switch'` and
      `planning/plan-58-data/census/fp-load.sh planning/plan-58-data/footprints-E.json`.
- [x] For every new footprint: a rendered image beside the datasheet page, read
      pair by pair, fixing any mismatch.

Acceptance: every E-Switch row has a spec whose footprint is staged and has its
pads; the staged footprints load; each image matches its drawing.
  Check: `python3 -I planning/plan-58-data/census/specs-check.py --maker 'E-Switch' && planning/plan-58-data/census/fp-load.sh planning/plan-58-data/footprints-E.json` → `specs ok`, then only `unused footprint` lines for the staged ids and `load ok` (est. 6 min).
  Measured: `specs-check.py --maker 'E-Switch'` → `specs ok (56 rows, 33 specs)`; `fp-load.sh footprints-E.json` → `33 footprints appended`, `33 new footprints: load ok`
Commit: 0fd23f62

### Phase 3 — parts

- [x] `python3 -I planning/plan-58-data/gen-parts.py --maker 'E-Switch'` (the earlier
      makers' parts stay: the generator rebuilds every maker already in the region).
- [x] Regenerate the goldens (`listing_check --print`, `trace_check --print`,
      `footprint_check --print`) and review: only added lines, in each file.
- [x] `node scripts/check-datasheets.js --host <each host of the new datasheets.json URLs>` → exit 0 (the hosts: `grep -h '"url"' schematical/schematical/Parts/datasheets.json` lines for the new ids).

Acceptance: `grep -c 'states=' backend/tests/golden/parts.txt` equals the previous
total plus the number of E-Switch `add` rows; the generator run is idempotent; `ctest`
passes (the library pass of `slide_test` covers the new devices).
  Check: `python3 -I planning/plan-58-data/gen-parts.py --maker 'E-Switch' --dry-run && cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8 -R 'slide_test|listing_check|trace_check|footprint_check|library_check'` → no change, `100% tests passed` (est. 12 min).
  Measured: `gen-parts.py --maker 'E-Switch'` written, then `--dry-run` → `no changes`; goldens regenerated: no golden line removed, `grep -c 'states=' backend/tests/golden/parts.txt` → 228; `ctest -j8` → `100% tests passed, 0 tests failed out of 62`; `check-datasheets.js --host configured-product-images.s3.amazonaws.com` → `0 failed`
Commit: 4a869484

## Validation Plan

- Tests: `scheme-check.py`, `specs-check.py`, `fp-load.sh`, `gen-parts.py --dry-run`;
  `slide_test`'s library pass; the three goldens.
- Coverage check: the new parts are in `listing_check`'s and `trace_check`'s
  denominators (both iterate every library part): `grep -c 'states=' backend/tests/golden/parts.txt` rises by the E-Switch `add` count.
- Runtime proof: the footprint images against the datasheet pages (Phase 2).
- Doc sync: `datasheets/skipped-parts.md`.
- Final gate (once, after Phase 3): `cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8`
  → `100% tests passed, 0 tests failed out of 62` (est. 15 min).

## Decisions

- Everything this letter does follows plan-58-A § Decisions and plan-58-C § Decisions;
  no choice is left to this letter.

## Corrections

- **2026-11-24, Phase 1: 7 pending rows do not fit their key and are `skip`** (EG: EG1249, EG1257, EG1270, EG1271, EG1271A, EG2210A-REDACTUATOR, EG2219); E-Switch has 56 `add` rows after Phase 1. `schemes.tsv` regexes are written from each datasheet's ordering key (positions and listed letters only).
- **2026-11-24, Phase 1: `EG_Series.pdf` (DigiKey's copy of the EG datasheet) is deleted.** Its only two rows (`EG1257`, `EG2210A-REDACTUATOR`) are `skip` (no EG datasheet prints their numbers and the EG family has no key), so no `add` / `pending` row needs the PDF any more: `place.py --prune` (new) removed it and its `datasheets.tsv` line, and its `maker-urls.tsv` row (the S3 `EG.pdf` mapping) went with it; `place.py --verify` → `39 URLs, 39 PDFs, 0 problems`, `maker-urls.py --verify` → `1 non-maker URLs, 0 with a maker URL, 0 missing`. The `EG` scheme keeps only `EG1215AA`: the EG datasheet (EG.pdf) lists the part numbers it prints and gives no ordering key, so every other pending EG number (`EG1249`, `EG1270`, `EG1271`, `EG1271A`, `EG2219`) is `skip` as well (7 skips: those 5, `EG1257`, `EG2210A-REDACTUATOR`).
- **2026-11-24, Phase 2: the footprints were drawn by parallel sub-agents, one per group of series** (500; 500A + 500R + 600; EG + EGJ1210), each reading its PDFs' drawings as page images, rendering every footprint with `census/fp-render.py` and comparing it with the drawing; the lead merged the fragments (`merge-specs.py`, `fp-load.sh`: 33 footprints, 33 spec rows for 56 `add` rows). Open questions of each sub-agent are in the footprints' `note` fields (top vs bottom view, the body outline's position against the pads where the drawing gives no tie, terminals renumbered to 1..n where a datasheet skips numbers). Images are rendered from the JSON (see plan-58-C Corrections), not by the app.
- **2026-11-24, Phase 3: E-Switch's datasheets are on E-Switch's own host** (`configured-product-images.s3.amazonaws.com`, linked from e-switch.com's product pages; see plan-58-A Corrections), so `check-datasheets.js --host configured-product-images.s3.amazonaws.com` (69 of 69 URLs serve a PDF) is the host check; the new lines add no pinout rename (plan-58-D Corrections).

## Summary

The risk is the footprints (drawn from PDF drawings, checked only by eye against
the drawing); the generator and the goldens are mechanical. Untouched: the
backend, the app, every other maker, every existing golden line.
