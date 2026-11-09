# plan-29-C: Footprints and specs — onsemi single diodes

Last updated: 2026-11-08
Effort: large (3h–1d)
Depends on: plan-29-B

C does for onsemi single diodes (DigiKey category 280) what plan-29-B does for Vishay single diodes,
with B's rules (plan-29-B § 4: rule choice, pin numbers, package names,
ids and reuse, ready objects, pending rows). Its ready objects go in
`planning/plan-29-data/footprints-C.json`.

References: plan-29-B § 4 (the rules), `docs/parts-format.md` § Footprints,
`planning/plan-29-data/footprints-needed.txt`, `specs.tsv`,
`candidates.tsv`.

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-B complete | `ls planning/completed/plan-29-B-*.md` → exists | NOT MET (2026-11-08: plan written) |
| B's tools exist | `test -x planning/plan-29-data/census/fp-load.sh && test -s planning/plan-29-data/census/specs-check.py && echo ok` → `ok` | NOT MET (plan-29-B Phase 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- onsemi single diodes (`category` = `single`, `maker` = onsemi): no row is `pending`; each is `add`, `skip` or `out`.
- Every `add` row in scope has a complete `specs.tsv` row whose footprint is
  in `Parts/footprints.json` or a `footprints-[B-C].json`.
- `census/fp-load.sh` over `footprints-B.json` … `footprints-C.json`
  reports only `extra footprint` / `unused footprint` failures, for exactly
  the new ids.

Planning-time scope (2026-11-08):

| What | Count | Command |
|---|---|---|
| onsemi singles in stock, Active (DigiKey) | 1113 | `scripts/digikey.js --category 280 --manufacturer 488 --in-stock --status 0 --status 4 --pages 1 --json` → `ProductsCount` 1113 |
| …listed at planning time (the 300-result cap, plan-29-A § 2) | 462 | `list.sh` summary `280 onsemi: 462 of 1113 parts` |
| Supplier packages, all 1113 (DigiKey facet) | 70 | same call, `Supplier Device Package` facet → 70 values |
| (maker, package, mounting) lines with a PDF, of the 462 listed | 35 | `grep -c ' onsemi / ' planning/plan-29-data/footprints-needed.txt` → 35 |

A's re-run (§ Corrections of A) replaces these numbers.

### Non-goals (explicit constraints)

- No shipped file changes (`Parts/*.json`, goldens, backend, app); no part
  is added.
- No new footprint for a package an earlier letter already drew, unless
  this maker's drawing differs (plan-29-B § 4.4 reuse).

## 2. Current State

- onsemi part datasheets end with a "MECHANICAL CASE OUTLINE" per case
  (`mbr0520lt1-d.pdf` p.4: CASE 425, SOD-123, "STYLE 1: PIN 1. CATHODE 2.
  ANODE"); dimensions sit in the drawing image, so pages are read as images.
- onsemi ordering tables list every orderable number (`MBR0520LT1G`,
  `MBR0520LT3G`, `SBR80520LT1G`, …), so most rows are verbatim hits; the
  `NRV`/`SBR`/`SZ` automotive twins are printed beside their base parts.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Surface-mount packages

- [ ] For each onsemi single SMD line of `footprints-needed.txt`, in descending part count: read the case outline, reuse or add the footprint (plan-29-B § 4.1, § 4.4), fill the spec rows (§ 4.2, § 4.3), settle pending rows (§ 4.5).

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --maker onsemi --category single --mounting smd` → `specs ok` (est. 4 min).
Commit: —

### Phase 2 — Through-hole packages

- [ ] The same for the Through Hole lines (axial R7; TO-220 / TO-247 R5).

Acceptance: every spec row of this phase's scope is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --maker onsemi --category single && planning/plan-29-data/census/fp-load.sh planning/plan-29-data/footprints-[B-C].json` → `specs ok`, then only extra/unused failures for the new ids` (est. 4 min).
Commit: —


## Validation Plan

- Tests: the last phase's `fp-load.sh` run; `specs-check.py`.
- Coverage check: `awk -F'\t' '$1=="single" && $2=="onsemi" && $24=="pending"' planning/plan-29-data/candidates.tsv | wc -l` → 0.
- Runtime proof: not applicable (no part loads yet).
- Doc sync: none.
- Final gate: the last phase's check (est. 4 min).

## Decisions

- Rules, ids and reuse as plan-29-B § Decisions.

## Corrections

## Summary

C is the same work as B for a smaller maker set whose drawings are images; the risk is copying numbers from an image, read twice per footprint.
