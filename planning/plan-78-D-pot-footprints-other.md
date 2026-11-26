# plan-78-D: Potentiometer footprints: Amphenol Piher, Vishay Sfernice, Same Sky

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-78-C

D draws the land pattern of every `POT-PIHER-`, `POT-SFERNICE-` and `POT-SAMESKY-` id of
`planning/plan-78-data/footprints-needed.txt` into `planning/plan-78-data/footprints-other.json`,
with C's tools and method, and proves they load together with C's file. H merges them.

References:

- plan-78-C § 3 (style, pins, geometry, origin, renders: D follows it unchanged).
- plan-78-B § 3.4 (`specs.tsv`).
- `docs/parts-format.md` § Footprints, § `explicit`.

## Prerequisites

plan-78-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-78-C complete | `ls planning/completed/plan-78-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-25: C not started) |
| Shared footprint ids are added once | `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` → 0 before adding an id | a rule, per footprint |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `footprints-other.json` holds one footprint per id of `footprints-needed.txt` that does not start
  `POT-BOURNS-`, each with a `note` naming its PDF and page.
- `footprints-bourns.json` and `footprints-other.json` load together through `footprint_check` on a
  scratch copy: only `extra footprint` / `unused footprint` lines, one pair per id of
  `footprints-needed.txt`.
- `footprints.json` and the goldens do not change in D.

### Non-goals (explicit constraints)

- No part is added and nothing is merged (H). No new footprint style or key.

## 2. Current State

After C: `footprints-bourns.json` holds the Bourns ids; no other pot footprint exists. The series
are the `specs.tsv` rows whose `maker` is `piher`, `sfernice` or `samesky` (B).

## 3. Design

As plan-78-C § 3, unchanged: the drawing's recommended PCB layout where the PDF prints one, else
pin positions from the dimension drawing with C's drill and pad rule, and the `note` says which.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Amphenol Piher

- [ ] Entries in `footprints-other.json` for every `POT-PIHER-` id; renders looked at.

Acceptance: the Piher entries load.
  Check: `planning/plan-78-data/census/fp-load.sh planning/plan-78-data/footprints-other.json` → exit 0, only `extra footprint` / `unused footprint` lines for the `POT-PIHER-` ids (est. 4 min).
  Result:
Commit:

### Phase 2 — Vishay Sfernice and Same Sky

- [ ] Entries for every `POT-SFERNICE-` and `POT-SAMESKY-` id; renders looked at.
- [ ] Every non-Bourns id of `footprints-needed.txt` is in `footprints-other.json`.

Acceptance: both files load together and cover `footprints-needed.txt` exactly.
  Check: `planning/plan-78-data/census/fp-load.sh planning/plan-78-data/footprints-bourns.json planning/plan-78-data/footprints-other.json` → exit 0, only `extra footprint` / `unused footprint` lines, one pair per id (`wc -l < planning/plan-78-data/footprints-needed.txt`) (est. 4 min).
  Result:
Commit:

## Validation Plan

- Per-phase checks above. Final gate: plan-78-I.

## Decisions

- Author: **one file for the three smaller makers**, so the footprint work splits by effort
  (Bourns's series count against the other three's), not by maker.

## Corrections

## Summary

The land patterns of every Amphenol Piher, Vishay Sfernice and Same Sky pot series in scope, checked
to load with Bourns's.
