# plan-29-F: Footprints and specs — rectifier diode arrays (all three makers)

Last updated: 2026-11-08
Effort: large (3h–1d)
Depends on: plan-29-E

F does for rectifier / Schottky / switching diode arrays (category 286) of Vishay, onsemi and Taiwan Semi what plan-29-B does for Vishay single diodes,
with B's rules (plan-29-B § 4: rule choice, pin numbers, package names,
ids and reuse, ready objects, pending rows). Its ready objects go in
`planning/plan-29-data/footprints-F.json`.

References: plan-29-B § 4 (the rules), `docs/parts-format.md` § Footprints,
`planning/plan-29-data/footprints-needed.txt`, `specs.tsv`,
`candidates.tsv`.

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-E complete | `ls planning/completed/plan-29-E-*.md` → exists | NOT MET (2026-11-08: plan written) |
| B's tools exist | `test -x planning/plan-29-data/census/fp-load.sh && test -s planning/plan-29-data/census/specs-check.py && echo ok` → `ok` | NOT MET (plan-29-B Phase 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- rows with `category` = `array`: no row is `pending`; each is `add`, `skip` or `out`.
- Every `add` row in scope has a complete `specs.tsv` row whose footprint is
  in `Parts/footprints.json` or a `footprints-[B-F].json`.
- `census/fp-load.sh` over `footprints-B.json` … `footprints-F.json`
  reports only `extra footprint` / `unused footprint` failures, for exactly
  the new ids.

Planning-time scope (2026-11-08):

| What | Count | Command |
|---|---|---|
| Diode arrays in stock, Active: Vishay / onsemi / TSC | 693 / 259 / 291 | `scripts/digikey.js --category 286 --manufacturer <112\|488\|1801> --in-stock --status 0 --status 4 --pages 1 --json` → `ProductsCount` |
| …Chassis Mount (`out`) | 76 / 0 / 0 | same calls, `Mounting Type` facet |
| …SiC (`out`) | 17 / 12 / 0 | same calls, `Technology` facet |
| Supplier packages (facet) | 27 / 25 / 9 | same calls, `Supplier Device Package` facet |

A's re-run (§ Corrections of A) replaces these numbers.

### Non-goals (explicit constraints)

- No shipped file changes (`Parts/*.json`, goldens, backend, app); no part
  is added.
- No new footprint for a package an earlier letter already drew, unless
  this maker's drawing differs (plan-29-B § 4.4 reuse).

## 2. Current State

- Array packages are multi-lead (SOT-23, SOT-323, SC-70, SOT-363,
  TO-220AB, TO-247AD, TO-263AB, ITO-220AB, FlatPAK 5x6 Dual, SO-8FL Dual):
  numbered leads, plan-29-B § 4.2's second rule.
- A configuration (DigiKey `Diode Configuration`: 1 Pair Common Cathode,
  1 Pair Common Anode, 1 Pair Series Connection, 2 Independent, …) becomes
  a device's `diodes` list in plan-29-H's format; F records it per spec row
  (`note`) with the pin map.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Surface-mount arrays

- [ ] For each SMD array (maker, package) line: reuse or add the footprint, fill the spec rows with the pin map and configuration, settle pending rows (plan-29-B § 4).

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --category array --mounting smd` → `specs ok` (est. 4 min).
Commit: —

### Phase 2 — Through-hole arrays

- [ ] The same for the Through Hole lines (TO-220AB, TO-247AD, ITO-220AB: R5).

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --category array && planning/plan-29-data/census/fp-load.sh planning/plan-29-data/footprints-[B-F].json` → `specs ok`, then only extra/unused failures for the new ids` (est. 4 min).
Commit: —


## Validation Plan

- Tests: the last phase's `fp-load.sh` run; `specs-check.py`.
- Coverage check: `awk -F'\t' '$1=="array" && $24=="pending"' planning/plan-29-data/candidates.tsv | wc -l` → 0.
- Runtime proof: not applicable (no part loads yet).
- Doc sync: none.
- Final gate: the last phase's check (est. 4 min).

## Decisions

- Rules, ids and reuse as plan-29-B § Decisions.
- An array's drawer leaf follows its technology (Schottky, Fast recovery, Switching, Rectifier), like a single diode's (plan-29-A § 3).

## Corrections

## Summary

F's new risk is the pin map: an array whose anodes and cathodes are swapped still loads. Each pin map is copied from the datasheet's pin diagram and read twice.
