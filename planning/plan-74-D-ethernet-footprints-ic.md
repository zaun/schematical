# plan-74-D: Ethernet IC footprints: every package of footprints-needed.txt

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-74-C

D draws every land pattern `planning/plan-74-data/footprints-needed.txt` lists. Each comes from its
maker's package drawing or recommended land pattern. D writes them to
`planning/plan-74-data/footprints-ic.json` and proves they load. F merges them into
`footprints.json` in the same commit as the parts that use them: `footprint_check` fails on a
footprint no part names (`backend/tests/footprint_check.c:179`, `unused footprint`).

References:

- `docs/parts-format.md` § Footprints (`dual`, `quad`, `grid`, `explicit`, corners, `centre`,
  `slots`).
- `planning/completed/plan-48-C-timer-opamp-footprints-leaded.md`,
  `plan-48-D-timer-opamp-footprints-leadless.md` (the method: one entry per drawing, the `note`
  names file, page and every number used; KiCad cross-check).
- `planning/plan-74-data/footprints-needed.txt` (C).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-C complete | `ls planning/completed/plan-74-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-25: C not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `footprints-ic.json` holds one footprint per id of `footprints-needed.txt`, in the
  `schematical-footprints` format, each with a `note` naming its drawing.
- Every entry loads through `footprint_check` on a scratch copy of the tree: the only messages are
  `extra footprint` and `unused footprint` for exactly the new ids, with no load error and no
  overlapping pads.
- `footprints.json` and the goldens do not change in D.

### Non-goals (explicit constraints)

- No part is added, and nothing is merged (F).
- No new footprint style or key: the four styles cover QFN / QFP / SSOP / SOIC / DIP / BGA.

## 2. Current State

`footprints.json` has 3993 footprints (F0). Quad-flat-no-lead footprints there use `quad` with a
`centre` exposed pad (`QFN-32-5X5`). Ball grids use `grid`. The golden `footprints.txt` has one
line per footprint, and `listing_check` compares it (`docs/backend.md` § goldens).

## 3. Design

- **Style** by package: QFN / DFN / VQFN / WQFN → `quad` (or `dual`) with `centre`; QFP / TQFP /
  LQFP → `quad`; SSOP / SOIC / TSSOP → `dual`; DIP → `dual` with `drill` and `pin1Shape` `rect`;
  BGA → `grid`.
- **Pads**: the drawing's recommended land pattern. Where a drawing prints only the package (no
  land pattern), the IPC-7351 nominal of the same body and lead: the precedent of plan-48-C § 3.
  The `note` says which.
- **`fp-merge.py` and `fp-load.sh`** (`planning/plan-74-data/census/`, copied from
  `planning/plan-58-data/census/fp-merge.py` and `fp-load.sh`, with `/tmp/p74-fp.XXXXXX`):
  - `fp-load.sh <footprints-X.json>…` merges the files into a scratch copy of `backend/` and
    `Parts/`, builds `footprint_check` there, and runs `fp-merge.py --expect`.
  - `fp-merge.py <footprints.json> <footprints-X.json>…` is the real merge, used by F, G, H and I.
    D adds one change: an id already in `footprints.json` (another plan's) is skipped and printed
    `present <id>`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Leaded packages (QFP, SSOP, SOIC, DIP)

- [ ] Entries in `footprints-ic.json` for every leaded id of `footprints-needed.txt`.

Acceptance: every leaded entry loads, and its pad count and pitch match its drawing.
  Check: `planning/plan-74-data/census/fp-load.sh planning/plan-74-data/footprints-ic.json` → exit 0, its output only `extra footprint` / `unused footprint` lines for the new leaded ids (est. 4 min: a scratch build of `footprint_check`).
  Result:
Commit:

### Phase 2 — Leadless and ball packages

- [ ] Entries for every QFN / DFN / BGA id.
- [ ] Every id of `footprints-needed.txt` is in `footprints-ic.json`.

Acceptance: the whole file loads.
  Check: `planning/plan-74-data/census/fp-load.sh planning/plan-74-data/footprints-ic.json` → exit 0, only `extra footprint` / `unused footprint` lines, one pair per id of `footprints-needed.txt` (`wc -l < planning/plan-74-data/footprints-needed.txt`) (est. 4 min).
  Result:
Commit:

## Validation Plan

- Per-phase checks above. Final gate: plan-74-K.

## Decisions

- Author: **IPC-7351 nominal only where the maker draws no land pattern**, and the `note` says so,
  as in plan-48-C.

## Corrections

## Summary

The IC packages the library lacks, drawn from their datasheets and merged once.
