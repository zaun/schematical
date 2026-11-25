# plan-58-D: Slide switches — NKK: schemes, footprints, parts

Last updated: 2026-11-24
Effort: medium (1h–2h)
Depends on: plan-58-C

D adds the NKK slide switches to the library with the tools plan-58-C
built: it decides the maker's `pending` rows against the datasheets'
part-number keys, draws one footprint per distinct recommended layout, fills
the maker's rows of `specs.tsv`, and runs `gen-parts.py` for it.

Outcome: every kept NKK row of `candidates.tsv` is a part
(`grep -c 'states=' backend/tests/golden/parts.txt` rises by the number of NKK
`add` rows); `Parts/footprints.json`, `Parts/datasheets.json` and the three
goldens gain exactly the new lines; `ctest` passes.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` § `switch`, § Footprints;
  `docs/backend.md` (goldens).
- `planning/plan-58-C-slide-switch-parts-ck.md` § 4 (schemes, specs, footprint
  rules, generator): this letter uses them unchanged. `planning/plan-58-B-slide-switch-kind.md`
  § 4.3 (pin names).
- NKK datasheets: `datasheets/switch/slide/nkk/` (8 PDFs at planning, plan-58-A).

## Prerequisites

The gate for the whole feature is in plan-58-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-58-C is complete | `ls planning/completed/plan-58-C-* \| wc -l` → 1 | MET (2026-11-24, P-58: plan-58-C archived, 98c5cefb) |
| The generator exists | `ls planning/plan-58-data/gen-parts.py planning/plan-58-data/census/scheme-check.py planning/plan-58-data/census/specs-check.py planning/plan-58-data/census/fp-load.sh \| wc -l` → 4 | MET (2026-11-24, P-58: plan-58-C archived, 98c5cefb) |
| NKK's PDFs are placed | `ls datasheets/switch/slide/nkk/*.pdf \| wc -l` → 8 (or fewer, when A skipped a PDF's rows) | MET (2026-11-24) |
| No NKK slide parts yet | `python3 -I -c "import csv;t=open('schematical/schematical/Parts/switch.json').read();m=[r['mpn'] for r in csv.DictReader(open('planning/plan-58-data/candidates.tsv'),delimiter='\t') if r['maker']=='NKK' and r['decision']=='add'];print(sum(('\"name\":\"'+x+'\"') in t for x in m))"` → 0 | MET (2026-11-24) |

If plan-58-C is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `schemes.tsv` has a row per NKK series that has `pending` rows; no NKK
  row is `pending`.
- `specs.tsv` has `mpn_regex`, `pins`, `footprint`, `note` for every NKK row;
  `census/specs-check.py --maker 'NKK'` exits 0.
- `Parts/switch.json` has the NKK devices and parts, `Parts/footprints.json`
  the footprints they name (moved from `planning/plan-58-data/footprints-D.json`),
  `Parts/datasheets.json` an entry per datasheet the parts name; the goldens have
  exactly the new lines added.

### Non-goals (explicit constraints)

- Every existing device, part and golden line (including C&K's) is byte-identical.
- No backend code, no app code, no other maker's rows. A part not in stock at
  DigiKey, or whose number the datasheet does not support, is `skip` (AGENTS.md).

## 2. Current State

C landed C&K's parts and `gen-parts.py`; `Parts/switch.json` holds the `slide_`
region before the tactile one. `candidates.tsv` has the NKK rows; `specs.tsv`
has their skeleton rows (plan-58-A).

### Measured populations

| What | Count | Command |
|---|---|---|
| NKK rows kept by DigiKey rules (`add` at planning) | 71 | `awk -F'\t' '$1=="NKK" && $18=="add"' planning/plan-58-data/candidates.tsv \| wc -l` |
| …verbatim in the PDF text | 18 | planning-time probe (`pdftotext` + substring); A records the real `hit` column |
| …series / (series, mounting) / (series, mounting, circuit) | 7 / 10 / 19 | `awk -F'\t' '$1=="NKK" && $18=="add"{print $3}' … \| sort -u \| wc -l`; likewise `$3,$11` and `$3,$11,$4` |
| …by series | AS 31, M 20, SS 13, CS 2, SM 2, SS3 2, FS 1 | `awk … \| sort \| uniq -c` |
| …by circuit and action | SPDT On-On 27, DPDT On-On 21, DPDT On-Off-On 10, SPDT On-Off-On 7, SPST On-Off 6 | `awk -F'\t' '$1=="NKK" && $18=="add"{print $4,$5}' … \| sort \| uniq -c` |
| Distinct datasheet URLs | 8 (all `www.nkkswitches.com`) | `awk -F'\t' '$1=="NKK" && $18=="add"{print $16}' … \| sort -u \| wc -l` |
| Rated volts (DigiKey DC rating, else AC) | 12, 28, 30; one part has no DC rating and 125 V AC | `awk -F'\t' '$1=="NKK" && $18=="add"{print $9, $8}' … \| sort -u` |

### Verified properties

- The formats, rules and checks are plan-58-C § 2 and § 4 (including: `footprint_check`
  fails on an unused footprint, so the footprints stay staged in
  `planning/plan-58-data/footprints-D.json` until `gen-parts.py` moves them with
  their parts).
- `www.nkkswitches.com` serves a PDF to a plain `curl` (plan-58-A probe: 8 of 8).
- A part with no DC rating takes its AC rating (plan-58-C § 4.4); NKK has one such part at planning (125 V AC).

## 3. Design Overview

The same four pieces as plan-58-C § 3, with the same risk (a wrong pad is valid
data; the image-against-drawing comparison catches it) and the same gate
(every other golden line byte-identical). NKK's series are drawn per series with one page of PCB layouts for several poles/throws, so one drawing often serves several of the 19 lines.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` partial; moot tasks stay with
> evidence; fill each phase's `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — decide the pending rows

- [x] Read each NKK PDF's part-number key; add the NKK rows to
      `planning/plan-58-data/schemes.tsv` (plan-58-C § 4.1).
- [x] `python3 -I planning/plan-58-data/census/scheme-check.py --maker 'NKK'`; commit
      `candidates.tsv` and the skipped-parts section.

Acceptance: no NKK row is `pending`; each `skip` reason is listed in
`datasheets/skipped-parts.md`.
  Check: `awk -F'\t' '$1=="NKK" && $18=="pending"' planning/plan-58-data/candidates.tsv \| wc -l` → 0 (est. 3 min).
  Measured: `scheme-check.py --maker 'NKK'` → `NKK 67 add, 4 skip, 0 pending`; `awk` pending count → 0; `decide.py --verify-skipped` exit 0
Commit: 5cba86ae

### Phase 2 — footprints and specs

- [x] For each of NKK's (series, mounting, circuit) lines (19 at planning): read
      the PDF drawing, write the footprint (plan-58-C § 4.3) as a ready object in
      `planning/plan-58-data/footprints-D.json` and its `specs.tsv` row (§ 4.2).
- [x] `python3 -I planning/plan-58-data/census/specs-check.py --maker 'NKK'` and
      `planning/plan-58-data/census/fp-load.sh planning/plan-58-data/footprints-D.json`.
- [x] For every new footprint: a rendered image beside the datasheet page, read
      pair by pair, fixing any mismatch.

Acceptance: every NKK row has a spec whose footprint is staged and has its
pads; the staged footprints load; each image matches its drawing.
  Check: `python3 -I planning/plan-58-data/census/specs-check.py --maker 'NKK' && planning/plan-58-data/census/fp-load.sh planning/plan-58-data/footprints-D.json` → `specs ok`, then only `unused footprint` lines for the staged ids and `load ok` (est. 6 min).
  Measured: `specs-check.py --maker 'NKK'` → `specs ok (67 rows, 29 specs)`; `fp-load.sh footprints-D.json` → `28 footprints appended`, `28 new footprints: load ok`
Commit: —

### Phase 3 — parts

- [ ] `python3 -I planning/plan-58-data/gen-parts.py --maker 'NKK'` (the earlier
      makers' parts stay: the generator rebuilds every maker already in the region).
- [ ] Regenerate the goldens (`listing_check --print`, `trace_check --print`,
      `footprint_check --print`) and review: only added lines, in each file.
- [ ] `node scripts/check-datasheets.js --host www.nkkswitches.com` → exit 0.

Acceptance: `grep -c 'states=' backend/tests/golden/parts.txt` equals the previous
total plus the number of NKK `add` rows; the generator run is idempotent; `ctest`
passes (the library pass of `slide_test` covers the new devices).
  Check: `python3 -I planning/plan-58-data/gen-parts.py --maker 'NKK' --dry-run && cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8 -R 'slide_test|listing_check|trace_check|footprint_check|library_check'` → no change, `100% tests passed` (est. 12 min).
Commit: —

## Validation Plan

- Tests: `scheme-check.py`, `specs-check.py`, `fp-load.sh`, `gen-parts.py --dry-run`;
  `slide_test`'s library pass; the three goldens.
- Coverage check: the new parts are in `listing_check`'s and `trace_check`'s
  denominators (both iterate every library part): `grep -c 'states=' backend/tests/golden/parts.txt` rises by the NKK `add` count.
- Runtime proof: the footprint images against the datasheet pages (Phase 2).
- Doc sync: `datasheets/skipped-parts.md`.
- Final gate (once, after Phase 3): `cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8`
  → `100% tests passed, 0 tests failed out of 62` (est. 15 min).

## Decisions

- Everything this letter does follows plan-58-A § Decisions and plan-58-C § Decisions;
  no choice is left to this letter.

## Corrections

- **2026-11-24, Phase 1: 4 pending rows do not fit their key and are `skip`** (AS: AS11BW, AS12CV/A, AS13AW; M: MSN12ANA03); NKK has 67 `add` rows after Phase 1. `schemes.tsv` regexes are written from each datasheet's ordering key (positions and listed letters only).
- **2026-11-24, Phase 2: the footprints were drawn by parallel sub-agents, one per group of series** (AS; M + FS + CS; SS + SS3 + SM), each reading its PDFs' drawings as page images, rendering every footprint with `census/fp-render.py` and comparing it with the drawing; the lead merged the fragments (`merge-specs.py`, `fp-load.sh`: 28 footprints, 29 spec rows for 67 `add` rows). Open questions of each sub-agent are in the footprints' `note` fields (top vs bottom view, the body outline's position against the pads where the drawing gives no tie, terminals renumbered to 1..n where a datasheet skips numbers). Images are rendered from the JSON (see plan-58-C Corrections), not by the app.

## Summary

The risk is the footprints (drawn from PDF drawings, checked only by eye against
the drawing); the generator and the goldens are mechanical. Untouched: the
backend, the app, every other maker, every existing golden line.
