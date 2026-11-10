# plan-29-D: Footprints and specs — Taiwan Semiconductor single diodes

Last updated: 2026-11-09
Effort: large (3h–1d)
Depends on: plan-29-C

D does for Taiwan Semiconductor single diodes (DigiKey category 280) what plan-29-B does for Vishay single diodes,
with B's rules (plan-29-B § 4: rule choice, pin numbers, package names,
ids and reuse, ready objects, pending rows). Its ready objects go in
`planning/plan-29-data/footprints-D.json`.

References: plan-29-B § 4 (the rules), `docs/parts-format.md` § Footprints,
`planning/plan-29-data/footprints-needed.txt`, `specs.tsv`,
`candidates.tsv`.

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-C complete | `ls planning/completed/plan-29-C-*.md` → exists | moot (2026-11-09: B–G ran in parallel by package owner, plan-29-B § Corrections) |
| B's tools exist | `test -x planning/plan-29-data/census/fp-load.sh && test -s planning/plan-29-data/census/specs-check.py && echo ok` → `ok` | MET (2026-11-09: `ok`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Taiwan Semiconductor single diodes (`category` = `single`): no row is `pending`; each is `add`, `skip` or `out`.
- Every `add` row in scope has a complete `specs.tsv` row whose footprint is
  in `Parts/footprints.json` or a `footprints-[B-D].json`.
- `census/fp-load.sh` over `footprints-B.json` … `footprints-D.json`
  reports only `extra footprint` / `unused footprint` failures, for exactly
  the new ids.

Planning-time scope (2026-11-08):

| What | Count | Command |
|---|---|---|
| TSC singles in stock, Active (DigiKey) | 1884 | `scripts/digikey.js --category 280 --manufacturer 1801 --in-stock --status 0 --status 4 --pages 1 --json` → `ProductsCount` 1884 |
| …`candidate` after the DigiKey rules | 1873 | `awk -F'\t' '$1=="single" && $2 ~ /^Taiwan/ && $24=="candidate"' planning/plan-29-data/candidates.tsv \| wc -l` → 1873 |
| (maker, package, mounting) lines with a PDF | 42 | `grep -c ' TSC / ' planning/plan-29-data/footprints-needed.txt` → 42 |

A's re-run (§ Corrections of A) replaces these numbers.

### Non-goals (explicit constraints)

- No shipped file changes (`Parts/*.json`, goldens, backend, app); no part
  is added.
- No new footprint for a package an earlier letter already drew, unless
  this maker's drawing differs (plan-29-B § 4.4 reuse).

## 2. Current State

- Taiwan Semi datasheets give a "SUGGESTED PAD LAYOUT" per package
  (`SS12 SERIES_R2305.pdf`, SMA), so most footprints are R1.
- Their ordering tables are patterns: "SS1x … x defines voltage from
  20V(SS12) to 150V(SS115)" with packing codes; DigiKey numbers add
  suffixes (`SS14M RSG`, `SS14LWH`) that the pattern must print to count
  (plan-29-B § 4.5).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Surface-mount packages

- [x] For each TSC single SMD line of `footprints-needed.txt`, in descending part count: read the drawing and pad layout, reuse or add the footprint, fill the spec rows, settle pending rows (plan-29-B § 4).
      Done 2026-11-09 by parallel owner agents (§ Corrections of plan-29-B: the work split); results checked: specs-check --maker Taiwan --category single --mounting smd → `specs ok (1254 add rows)`.

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --maker "Taiwan Semiconductor" --category single --mounting smd` → `specs ok` (est. 4 min).
Commit: —

### Phase 2 — Through-hole packages

- [x] The same for the Through Hole lines.
      Done 2026-11-09 by parallel owner agents (§ Corrections of plan-29-B: the work split); results checked: specs-check --maker Taiwan --category single → `specs ok (1658 add rows)`; fp-load footprints-[B-D].json → `154 new footprints: load ok`.

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --maker "Taiwan Semiconductor" --category single && planning/plan-29-data/census/fp-load.sh planning/plan-29-data/footprints-[B-D].json` → `specs ok`, then only extra/unused failures for the new ids` (est. 4 min).
Commit: —


## Validation Plan

- Tests: the last phase's `fp-load.sh` run; `specs-check.py`.
- Coverage check: `awk -F'\t' '$1=="single" && $2 ~ /^Taiwan/ && $24=="pending"' planning/plan-29-data/candidates.tsv | wc -l` → 0.
- Runtime proof: not applicable (no part loads yet).
- Doc sync: none.
- Final gate: the last phase's check (est. 4 min).

## Decisions

- Rules, ids and reuse as plan-29-B § Decisions; a TSC drawing that matches a Vishay or onsemi land pattern reuses that id.

## Corrections

- See plan-29-B § Corrections (2026-11-09): B–G ran in parallel by package
  owner; per-PDF overrides; counts.

## Summary

D carries the most pattern-style ordering tables; the risk is a regex wider than the datasheet prints. Each regex is built only from printed bases and codes (plan-29-B § 4.5).
