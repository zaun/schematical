# plan-58-F: Slide switches — CIT Relay & Switch: schemes, footprints, parts

Last updated: 2026-11-24
Effort: medium (1h–2h)
Depends on: plan-58-E

F adds the CIT Relay & Switch slide switches to the library with the tools plan-58-C
built: it decides the maker's `pending` rows against the datasheets'
part-number keys, draws one footprint per distinct recommended layout, fills
the maker's rows of `specs.tsv`, and runs `gen-parts.py` for it.

Outcome: every kept CIT Relay & Switch row of `candidates.tsv` is a part
(`grep -c 'states=' backend/tests/golden/parts.txt` rises by the number of CIT Relay & Switch
`add` rows); `Parts/footprints.json`, `Parts/datasheets.json` and the three
goldens gain exactly the new lines; `ctest` passes.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` § `switch`, § Footprints;
  `docs/backend.md` (goldens).
- `planning/plan-58-C-slide-switch-parts-ck.md` § 4 (schemes, specs, footprint
  rules, generator): this letter uses them unchanged. `planning/plan-58-B-slide-switch-kind.md`
  § 4.3 (pin names).
- CIT Relay & Switch datasheets: `datasheets/switch/slide/cit/` (11 PDFs at planning, plan-58-A).

## Prerequisites

The gate for the whole feature is in plan-58-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-58-E is complete | `ls planning/completed/plan-58-E-* \| wc -l` → 1 | NOT MET until E lands |
| The generator exists | `ls planning/plan-58-data/gen-parts.py planning/plan-58-data/census/scheme-check.py planning/plan-58-data/census/specs-check.py planning/plan-58-data/census/fp-load.sh \| wc -l` → 4 | NOT MET until C lands |
| CIT Relay & Switch's PDFs are placed | `ls datasheets/switch/slide/cit/*.pdf \| wc -l` → 11 (or fewer, when A skipped a PDF's rows) | NOT MET until A lands |
| No CIT slide parts yet | `python3 -I -c "import csv;t=open('schematical/schematical/Parts/switch.json').read();m=[r['mpn'] for r in csv.DictReader(open('planning/plan-58-data/candidates.tsv'),delimiter='\t') if r['maker']=='CIT' and r['decision']=='add'];print(sum(('\"name\":\"'+x+'\"') in t for x in m))"` → 0 | MET (2026-11-24) |

If plan-58-E is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `schemes.tsv` has a row per CIT Relay & Switch series that has `pending` rows; no CIT Relay & Switch
  row is `pending`.
- `specs.tsv` has `mpn_regex`, `pins`, `footprint`, `note` for every CIT Relay & Switch row;
  `census/specs-check.py --maker 'CIT'` exits 0.
- `Parts/switch.json` has the CIT Relay & Switch devices and parts, `Parts/footprints.json`
  the footprints they name (moved from `planning/plan-58-data/footprints-F.json`),
  `Parts/datasheets.json` an entry per datasheet the parts name; the goldens have
  exactly the new lines added.

### Non-goals (explicit constraints)

- Every existing device, part and golden line (including C&K's) is byte-identical.
- No backend code, no app code, no other maker's rows. A part not in stock at
  DigiKey, or whose number the datasheet does not support, is `skip` (AGENTS.md).

## 2. Current State

C landed C&K's parts and `gen-parts.py`; `Parts/switch.json` holds the `slide_`
region before the tactile one. `candidates.tsv` has the CIT Relay & Switch rows; `specs.tsv`
has their skeleton rows (plan-58-A).

### Measured populations

| What | Count | Command |
|---|---|---|
| CIT Relay & Switch rows kept by DigiKey rules (`add` at planning) | 43 | `awk -F'\t' '$1=="CIT" && $18=="add"' planning/plan-58-data/candidates.tsv \| wc -l` |
| …verbatim in the PDF text | 10 | planning-time probe (`pdftotext` + substring); A records the real `hit` column |
| …series / (series, mounting) / (series, mounting, circuit) | 11 / 13 / 15 | `awk -F'\t' '$1=="CIT" && $18=="add"{print $3}' … \| sort -u \| wc -l`; likewise `$3,$11` and `$3,$11,$4` |
| …by series | 1000 20, MS2201 5, MS1247 3, MS2205 3, MS2241 3, MS1202 2, MS1206 2, MS2210 2, MS2208 1, MS2211 1, MS2219 1 | `awk … \| sort \| uniq -c` |
| …by circuit and action | DPDT On-On 20, SPDT On-On 16, DPDT On-Off-On 4, SPDT On-Off-On 3 | `awk -F'\t' '$1=="CIT" && $18=="add"{print $4,$5}' … \| sort \| uniq -c` |
| Distinct datasheet URLs | 11 (all `www.citrelay.com`) | `awk -F'\t' '$1=="CIT" && $18=="add"{print $16}' … \| sort -u \| wc -l` |
| Rated volts (DigiKey DC rating, else AC) | 20, 28, 30 | `awk -F'\t' '$1=="CIT" && $18=="add"{print $9, $8}' … \| sort -u` |

### Verified properties

- The formats, rules and checks are plan-58-C § 2 and § 4 (including: `footprint_check`
  fails on an unused footprint, so the footprints stay staged in
  `planning/plan-58-data/footprints-F.json` until `gen-parts.py` moves them with
  their parts).
- `www.citrelay.com` serves a PDF to a plain `curl` (plan-58-A probe: 11 of 11); each PDF is one page with the dimensions and schematic of one series.

## 3. Design Overview

The same four pieces as plan-58-C § 3, with the same risk (a wrong pad is valid
data; the image-against-drawing comparison catches it) and the same gate
(every other golden line byte-identical). CIT's one-page datasheets draw the PC layout beside the schematic; a series with several mountings has one drawing per mounting.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` partial; moot tasks stay with
> evidence; fill each phase's `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — decide the pending rows

- [ ] Read each CIT Relay & Switch PDF's part-number key; add the CIT Relay & Switch rows to
      `planning/plan-58-data/schemes.tsv` (plan-58-C § 4.1).
- [ ] `python3 -I planning/plan-58-data/census/scheme-check.py --maker 'CIT'`; commit
      `candidates.tsv` and the skipped-parts section.

Acceptance: no CIT Relay & Switch row is `pending`; each `skip` reason is listed in
`datasheets/skipped-parts.md`.
  Check: `awk -F'\t' '$1=="CIT" && $18=="pending"' planning/plan-58-data/candidates.tsv \| wc -l` → 0 (est. 3 min).
Commit: —

### Phase 2 — footprints and specs

- [ ] For each of CIT Relay & Switch's (series, mounting, circuit) lines (15 at planning): read
      the PDF drawing, write the footprint (plan-58-C § 4.3) as a ready object in
      `planning/plan-58-data/footprints-F.json` and its `specs.tsv` row (§ 4.2).
- [ ] `python3 -I planning/plan-58-data/census/specs-check.py --maker 'CIT'` and
      `planning/plan-58-data/census/fp-load.sh planning/plan-58-data/footprints-F.json`.
- [ ] For every new footprint: a rendered image beside the datasheet page, read
      pair by pair, fixing any mismatch.

Acceptance: every CIT Relay & Switch row has a spec whose footprint is staged and has its
pads; the staged footprints load; each image matches its drawing.
  Check: `python3 -I planning/plan-58-data/census/specs-check.py --maker 'CIT' && planning/plan-58-data/census/fp-load.sh planning/plan-58-data/footprints-F.json` → `specs ok`, then only `unused footprint` lines for the staged ids and `load ok` (est. 6 min).
Commit: —

### Phase 3 — parts

- [ ] `python3 -I planning/plan-58-data/gen-parts.py --maker 'CIT'` (the earlier
      makers' parts stay: the generator rebuilds every maker already in the region).
- [ ] Regenerate the goldens (`listing_check --print`, `trace_check --print`,
      `footprint_check --print`) and review: only added lines, in each file.
- [ ] `node scripts/check-datasheets.js --host www.citrelay.com` → exit 0.

Acceptance: `grep -c 'states=' backend/tests/golden/parts.txt` equals the previous
total plus the number of CIT Relay & Switch `add` rows; the generator run is idempotent; `ctest`
passes (the library pass of `slide_test` covers the new devices).
  Check: `python3 -I planning/plan-58-data/gen-parts.py --maker 'CIT' --dry-run && cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8 -R 'slide_test|listing_check|trace_check|footprint_check|library_check'` → no change, `100% tests passed` (est. 12 min).
Commit: —

## Validation Plan

- Tests: `scheme-check.py`, `specs-check.py`, `fp-load.sh`, `gen-parts.py --dry-run`;
  `slide_test`'s library pass; the three goldens.
- Coverage check: the new parts are in `listing_check`'s and `trace_check`'s
  denominators (both iterate every library part): `grep -c 'states=' backend/tests/golden/parts.txt` rises by the CIT Relay & Switch `add` count.
- Runtime proof: the footprint images against the datasheet pages (Phase 2).
- Doc sync: `datasheets/skipped-parts.md`.
- Final gate (once, after Phase 3): `cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8`
  → `100% tests passed, 0 tests failed out of 62` (est. 15 min).

## Decisions

- Everything this letter does follows plan-58-A § Decisions and plan-58-C § Decisions;
  no choice is left to this letter.

## Corrections

(Filled in during execution.)

## Summary

The risk is the footprints (drawn from PDF drawings, checked only by eye against
the drawing); the generator and the goldens are mechanical. Untouched: the
backend, the app, every other maker, every existing golden line.
