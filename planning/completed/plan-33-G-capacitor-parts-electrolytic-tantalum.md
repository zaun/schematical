# plan-33-G: Parts — electrolytic and tantalum capacitors, goldens

Last updated: 2026-11-11
Effort: medium (1h–2h)
Depends on: plan-33-F

G runs F's generator for the polarized types: `aluminum` and
`aluminum-polymer` (drawer **Capacitors → Electrolytic**), `tantalum`,
`tantalum-polymer` and `niobium-oxide` (**Capacitors → Tantalum**), and
regenerates the goldens.

The single behavioral outcome: `listing_check` lists one `capacitor` line
per `add` row of `candidates.tsv` (`N_F` + `N_G` + the generic), each
polarized part with pins `+` and `-` and a `volts` rating; the goldens only
grew.

References:

- plan-33-F § 3 (devices, parts, value text) and its generator
  `planning/plan-33-data/census/gen-parts.py`.
- plan-33-B § Summary (`N_G`), plan-33-C and D (the footprints these rows
  name in `fpgroups.tsv`).

## Prerequisites

plan-33-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-33-F complete | `ls planning/completed/plan-33-F-*.md` → exists | MET (2026-11-11, at execution: d12f5c4) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `grep -c '|capacitor|' backend/tests/golden/parts.txt` → the `add` rows
  of `candidates.tsv` + 1 (`awk -F'\t' '$25=="add"'
  planning/plan-33-data/candidates.tsv | wc -l`, plus `GENERIC-CAPACITOR`).
- `grep '|capacitor|' backend/tests/golden/parts.txt | grep -c
  'type=\(aluminum\|tantalum\|niobium\)'` → `N_G`, and every one of those
  lines has `rated=` above 0.
- `git diff <F's first commit>^ -- backend/tests/golden | grep -c '^-[^-]'`
  → 0.

### Non-goals (explicit constraints)

- No backend, app or ceramic-part change.

## 2. Current State

After F: `Parts/capacitor.json` holds the generic and the ceramic devices;
the polarized rows are `add` in `candidates.tsv` (types aluminum,
al-polymer, tantalum, ta-polymer, niobium) with their footprints in
`footprints-C.json` (molded chips) and `footprints-D.json` (cans, dipped).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Electrolytic

- [x] `python3 -I gen-parts.py --type aluminum,aluminum-polymer`;
      regenerate the goldens; the diff only adds lines. Observed: `323 parts
      written; 81 devices, 48 new footprints`; numstat parts 323 / 0, traces
      323 / 0, footprints 48 / 0.

Acceptance: every electrolytic `add` row is a part.
  Check: `ctest --test-dir /tmp/logic-p33 -R 'listing_check|trace_check|footprint_check'` → 3 passed; the electrolytic line count equals `awk -F'\t' '$25=="add" && ($2=="aluminum" || $2=="al-polymer")' planning/plan-33-data/candidates.tsv | wc -l` (est. 4 min).
  Observed: `100% tests passed, 0 tests failed out of 3`; `type=aluminum` lines 323 = 323 add rows.
Commit: 0aac36b

### Phase 2 — Tantalum and niobium oxide

- [x] `python3 -I gen-parts.py --type tantalum,tantalum-polymer,niobium-oxide`;
      regenerate the goldens; the diff only adds lines. Observed: `reused
      CAPM-7343-D` (shipped by Phase 1's 2917 aluminum polymer chips), `332
      parts written; 138 devices, 41 new footprints`; numstat parts 332 / 0,
      traces 332 / 0, footprints 41 / 0.

Acceptance: every `add` row is a part.
  Check: `ctest --test-dir /tmp/logic-p33` → `100% tests passed … out of 31`; § 1's three counts (est. 5 min).
  Observed: `100% tests passed, 0 tests failed out of 31`; capacitor lines 1032 = 1031 add rows + 1; polarized lines 655 (`N_G`), none `rated=0`; `git diff cea454b^ -- backend/tests/golden | grep -c '^-[^-]'` → 0.
Commit: (this commit; hash in the next)

## Validation Plan

- Tests: the goldens (`listing_check`, `trace_check`, `footprint_check`).
- Coverage check: § 1's count equality across all `add` rows.
- Runtime proof: plan-33-H's harness (a tantalum part's 12 V refusal).
- Doc sync: none.
- Final gate: plan-33-I.

## Decisions

- **Electrolytic = aluminum + aluminum polymer; Tantalum = tantalum +
  tantalum polymer + niobium oxide** — author: the owner picked the types
  "Electrolytic" and "Tantalum" (plan-33-A § Decisions); polymer and
  niobium-oxide parts are those types' low-ESR variants, filed beside them.

## Corrections

1. **`N_G` is 655** (aluminum 188, aluminum polymer 135, tantalum 228,
   tantalum polymer 84, niobium oxide 20): the four PHH/PHV crown rows are
   `out` (plan-33-D § Corrections 2), so aluminum polymer has 135 picks, not
   137.
2. **An `http://` datasheet link.** DigiKey's TAP link is
   `http://datasheets.avx.com/tap.pdf`; `datasheets.json` takes https only
   (the loader: `"url" must start with https://`), so gen-parts.py writes
   the URL the same PDF answers at, `https://datasheets.kyocera-avx.com/tap.pdf`
   (curl: `200`, `application/pdf`). The first attempt wrote the http URL and
   the golden run failed to load; the part files and goldens were restored
   to Phase 1's commit and the scope re-run.

## Summary

G is two generator runs. Untouched: everything F shipped.
