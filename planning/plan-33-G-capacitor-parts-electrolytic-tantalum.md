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
| plan-33-F complete | `ls planning/completed/plan-33-F-*.md` → exists | NOT MET (2026-11-11: planning) |

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

- [ ] `python3 -I gen-parts.py --type aluminum,aluminum-polymer`;
      regenerate the goldens; the diff only adds lines.

Acceptance: every electrolytic `add` row is a part.
  Check: `ctest --test-dir /tmp/logic-p33 -R 'listing_check|trace_check|footprint_check'` → 3 passed; the electrolytic line count equals `awk -F'\t' '$25=="add" && ($2=="aluminum" || $2=="al-polymer")' planning/plan-33-data/candidates.tsv | wc -l` (est. 4 min).
Commit: —

### Phase 2 — Tantalum and niobium oxide

- [ ] `python3 -I gen-parts.py --type tantalum,tantalum-polymer,niobium-oxide`;
      regenerate the goldens; the diff only adds lines.

Acceptance: every `add` row is a part.
  Check: `ctest --test-dir /tmp/logic-p33` → `100% tests passed … out of 31`; § 1's three counts (est. 5 min).
Commit: —

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

(Filled in during execution.)

## Summary

G is two generator runs. Untouched: everything F shipped.
