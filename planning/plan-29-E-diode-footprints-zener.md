# plan-29-E: Footprints and specs — Zener diodes and Zener arrays (all three makers)

Last updated: 2026-11-09
Effort: large (3h–1d)
Depends on: plan-29-D

E does for single Zeners (category 287) and Zener arrays (category 295) of Vishay, onsemi and Taiwan Semi what plan-29-B does for Vishay single diodes,
with B's rules (plan-29-B § 4: rule choice, pin numbers, package names,
ids and reuse, ready objects, pending rows). Its ready objects go in
`planning/plan-29-data/footprints-E.json`.

References: plan-29-B § 4 (the rules), `docs/parts-format.md` § Footprints,
`planning/plan-29-data/footprints-needed.txt`, `specs.tsv`,
`candidates.tsv`.

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-D complete | `ls planning/completed/plan-29-D-*.md` → exists | moot (2026-11-09: B–G ran in parallel by package owner, plan-29-B § Corrections) |
| B's tools exist | `test -x planning/plan-29-data/census/fp-load.sh && test -s planning/plan-29-data/census/specs-check.py && echo ok` → `ok` | MET (2026-11-09: `ok`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- rows with `category` = `zener` or `zener-array`: no row is `pending`; each is `add`, `skip` or `out`.
- Every `add` row in scope has a complete `specs.tsv` row whose footprint is
  in `Parts/footprints.json` or a `footprints-[B-E].json`.
- `census/fp-load.sh` over `footprints-B.json` … `footprints-E.json`
  reports only `extra footprint` / `unused footprint` failures, for exactly
  the new ids.

Planning-time scope (2026-11-08):

| What | Count | Command |
|---|---|---|
| Single Zeners in stock, Active: Vishay / onsemi / TSC | 2242 / 1547 / 596 | `scripts/digikey.js --category 287 --manufacturer <112\|488\|1801> --in-stock --status 0 --status 4 --pages 1 --json` → `ProductsCount` |
| Zener arrays in stock, Active: Vishay / onsemi / TSC | 39 / 27 / 30 | same with `--category 295` |
| Supplier packages (DigiKey facet): 287 Vishay / onsemi / TSC; 295 | 20 / 21 / 14; 1 / 2 / 1 | same calls, `Supplier Device Package` facet |
| Mounting (facet), 287: Surface Mount / Wettable Flank / Through Hole | Vishay 1644 / 0 / 598; onsemi 1088 / 47 / 359; TSC 560 / 0 / 36 | same calls, `Mounting Type` facet (no chassis mount: none are `out` for mounting) |

A's re-run (§ Corrections of A) replaces these numbers.

### Non-goals (explicit constraints)

- No shipped file changes (`Parts/*.json`, goldens, backend, app); no part
  is added.
- No new footprint for a package an earlier letter already drew, unless
  this maker's drawing differs (plan-29-B § 4.4 reuse).

## 2. Current State

- Zener packages are mostly the single-diode ones (SOD-123, SOD-323,
  SOD-523, SMA, SMB, DO-35, DO-41, MiniMELF) that B–D drew; E reuses those
  ids and adds the rest (SOT-23 Zeners, SC-70, the arrays' SOT-23 / SC-88 /
  SOT-563 / SC-89-3).
- Zener arrays are common-anode or common-cathode pairs and quads: their
  pinouts name `A`, `K1`, `K2`… (§ 4.2 of plan-29-B).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Single Zeners

- [x] For each single-Zener (maker, package) line: reuse or add the footprint, fill the spec rows, settle pending rows (plan-29-B § 4).
      Done 2026-11-09 by parallel owner agents (§ Corrections of plan-29-B: the work split); results checked: specs-check --category zener → `specs ok (3766 add rows)`.

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --category zener` → `specs ok` (est. 4 min).
Commit: —

### Phase 2 — Zener arrays

- [x] The same for the Zener-array lines; each array spec row also records its configuration in `note` (common anode / common cathode / count).
      Done 2026-11-09 by parallel owner agents (§ Corrections of plan-29-B: the work split); results checked: specs-check --category zener-array → `specs ok (96 add rows)`; fp-load footprints-[B-E].json → `157 new footprints: load ok`.

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --category zener-array && planning/plan-29-data/census/fp-load.sh planning/plan-29-data/footprints-[B-E].json` → `specs ok`, then only extra/unused failures for the new ids` (est. 4 min).
Commit: —


## Validation Plan

- Tests: the last phase's `fp-load.sh` run; `specs-check.py`.
- Coverage check: `awk -F'\t' '($1=="zener" || $1=="zener-array") && $24=="pending"' planning/plan-29-data/candidates.tsv | wc -l` → 0.
- Runtime proof: not applicable (no part loads yet).
- Doc sync: none.
- Final gate: the last phase's check (est. 4 min).

## Decisions

- Rules, ids and reuse as plan-29-B § Decisions.
- Zener arrays share the `zener` type; the drawer leaf is Zener (plan-29-A § 3).

## Corrections

- See plan-29-B § Corrections (2026-11-09): B–G ran in parallel by package
  owner; per-PDF overrides; counts.

## Summary

E is mostly reuse: Zeners come in the single-diode packages. The new work is the SOT/SC-family multi-lead packages and the array pin maps.
