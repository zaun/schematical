# plan-29-G: Footprints and specs — bridge rectifiers (all three makers)

Last updated: 2026-11-09
Effort: large (3h–1d)
Depends on: plan-29-F

G does for bridge rectifiers (category 299) of Vishay, onsemi and Taiwan Semi what plan-29-B does for Vishay single diodes,
with B's rules (plan-29-B § 4: rule choice, pin numbers, package names,
ids and reuse, ready objects, pending rows). Its ready objects go in
`planning/plan-29-data/footprints-G.json`.

References: plan-29-B § 4 (the rules), `docs/parts-format.md` § Footprints,
`planning/plan-29-data/footprints-needed.txt`, `specs.tsv`,
`candidates.tsv`.

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-F complete | `ls planning/completed/plan-29-F-*.md` → exists | moot (2026-11-09: B–G ran in parallel by package owner, plan-29-B § Corrections) |
| B's tools exist | `test -x planning/plan-29-data/census/fp-load.sh && test -s planning/plan-29-data/census/specs-check.py && echo ok` → `ok` | MET (2026-11-09: `ok`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- rows with `category` = `bridge`: no row is `pending`; each is `add`, `skip` or `out`.
- Every `add` row in scope has a complete `specs.tsv` row whose footprint is
  in `Parts/footprints.json` or a `footprints-[B-G].json`.
- `census/fp-load.sh` over `footprints-B.json` … `footprints-G.json`
  reports only `extra footprint` / `unused footprint` failures, for exactly
  the new ids.

Planning-time scope (2026-11-08):

| What | Count | Command |
|---|---|---|
| Bridge rectifiers in stock, Active: Vishay / onsemi / TSC | 382 / 101 / 377 | `scripts/digikey.js --category 299 --manufacturer <112\|488\|1801> --in-stock --status 0 --status 4 --pages 1 --json` → `ProductsCount` |
| …Chassis Mount or QC Terminal (`out`) | 93 / 25 / 38 | same calls, `Mounting Type` facet (Vishay 27 + 66, onsemi 25, TSC 38) |
| …Surface Mount / Through Hole (in scope) | 52 + 237 / 18 + 54 / 98 + 241 | same facet |
| Supplier packages (facet) | 29 / 8 / 20 | same calls, `Supplier Device Package` facet |

A's re-run (§ Corrections of A) replaces these numbers.

### Non-goals (explicit constraints)

- No shipped file changes (`Parts/*.json`, goldens, backend, app); no part
  is added.
- No new footprint for a package an earlier letter already drew, unless
  this maker's drawing differs (plan-29-B § 4.4 reuse).

## 2. Current State

- Bridge packages: DFS, MBS, ABS, TBS, KBP, KBL, KBU, GBU, GBL, GBJ, D3K,
  WOG, … (four leads, `AC1`, `AC2`, `+`, `-`, plan-29-B § 4.2). In-line
  through-hole bridges (KBP, GBU) are `explicit` or `dual` footprints with
  one row; their drawings give the lead pitch (R5).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Surface-mount bridges

- [x] For each SMD bridge (maker, package) line: reuse or add the footprint, fill the spec rows (pins `AC1`, `AC2`, `+`, `-` per the datasheet), settle pending rows (plan-29-B § 4).
      Done 2026-11-09 by parallel owner agents (§ Corrections of plan-29-B: the work split); results checked: specs-check --category bridge --mounting smd → `specs ok (164 add rows)`.

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --category bridge --mounting smd` → `specs ok` (est. 4 min).
Commit: —

### Phase 2 — Through-hole bridges

- [x] The same for the Through Hole lines; an in-line package is `explicit` (one row of four pads, pad 1 `rect`).
      Done 2026-11-09 by parallel owner agents (§ Corrections of plan-29-B: the work split); results checked: specs-check --category bridge → `specs ok (652 add rows)`; fp-load footprints-[B-G].json → `212 new footprints: load ok`; every in-line bridge footprint is `explicit` with pad 1 `rect` (GBU, KBU, KBL, GBL, KBP, KBPF, D3K, D-44, GSIB-5S, TS-6P, TS4K, 2KBB); square / dual-row ones are `dual`.

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --category bridge && planning/plan-29-data/census/fp-load.sh planning/plan-29-data/footprints-[B-G].json` → `specs ok`, then only extra/unused failures for the new ids` (est. 4 min).
Commit: —


## Validation Plan

- Tests: the last phase's `fp-load.sh` run; `specs-check.py`.
- Coverage check: `awk -F'\t' '$1=="bridge" && $24=="pending"' planning/plan-29-data/candidates.tsv | wc -l` → 0.
- Runtime proof: not applicable (no part loads yet).
- Doc sync: none.
- Final gate: the last phase's check (est. 4 min).

## Decisions

- Rules, ids and reuse as plan-29-B § Decisions.
- Bridge pins are named `AC1`, `AC2`, `+`, `-` whatever the datasheet prints (`~`, `AC`), so every bridge shares one device shape per pin order.

## Corrections

- See plan-29-B § Corrections (2026-11-09): B–G ran in parallel by package
  owner; per-PDF overrides; counts.

## Summary

G is the smallest footprint letter but the most varied package set (through-hole blocks with in-line leads). Chassis and QC-terminal bridges are out.
