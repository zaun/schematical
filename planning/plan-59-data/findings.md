# plan-59 findings

Filled in while plan-59-A, -B and -C run. Verdicts and their rules are in
`probes.md`. A cell is `—` until its probe has run. An evidence cell holds one
command with its output, or `evidence/<probe>-<candidate>.png`.

## Baselines

| Baseline | Value | Command |
|---|---|---|
| B1 edit-lag median / p90 | label name 13.8 / 14.3 ms; label type 13.7 / 14.8 ms; bus wire name 16.5 / 18.3 ms; add wire + undo 34.1 / 35.6 ms; pan 2.0 / 16.0 ms (each the median of three runs; Debug app, unoptimized harness; screen locked) | `<scratch>/h-edit/H.app/Contents/MacOS/harness examples/6502.schematical both` ×3, output in `evidence/B1-edit-lag-base.txt` |
| B1 drag-timing median / p90 | moveChip 23.08 / 24.97 ms; dragBusLine 11.97 / 13.35 ms; pan 2.12 / 16.01 ms (median of three runs) | `<scratch>/h-drag/H.app/Contents/MacOS/harness examples/6502.schematical both` ×3, output in `evidence/B1-drag-timing-base.txt` |
| B2 relative `memoryFile.path` for Save As into a second folder | `../A/rom.bin` (load file `…/A/rom.bin`, schematic `…/B/design.schematical`); `rom.bin` when both are in one folder; the absolute path when the schematic has no URL | `scripts/baselines.swift` through `build.sh` on an export of 2d10a029 → `B2 part AT28C64B-15JU memoryFile.path: ../A/rom.bin`, `B2 same-folder memoryFile.path: rom.bin`, `B2 no-URL memoryFile.path: /tmp/…/b2/A/rom.bin` |
| B3 `SchematicFile(document:fileURL:)` main-thread median of 20 | 14.726 ms (min 14.606, max 15.528; 103 parts; unoptimized harness) | same run → `B3 snapshot ms: median 14.726 min 14.606 max 15.528 (n=20, parts 103)` |

## Verdicts

| ID | Probe | G | G evidence | H | H evidence |
|---|---|---|---|---|---|
| F1 | File URL while reading | — | — | — | — |
| F2 | File URL while writing | — | — | — | — |
| F3 | Round trip | — | — | — | — |
| F4 | Snapshot cost | — | — | — | — |
| F5 | Catalog ready on cold open | — | — | — | — |
| F6 | Sandbox and other writers | — | — | — | — |
| F7 | Part files (info) | — | — | — | — |
| U1 | Edit marks the document edited | — | — | — | — |
| U2 | Undo back to clean | — | — | — | — |
| U3 | One `NSUndoManager` per document | — | — | — | — |
| U4 | Text field undo | — | — | — | — |
| U5 | Simulation lock | — | — | — | — |
| A1 | File menu | — | — | — | — |
| A2 | Commands reach the focused document | — | — | — | — |
| A3 | Exports from the Export menu | — | — | — | — |
| A4 | Print | — | — | — | — |
| A5 | Child windows | — | — | — | — |
| A6 | Lifetime | — | — | — | — |
| A7 | Title bar | — | — | — | — |
| A8 | Tabs | — | — | — | — |
| A9 | Opening paths | — | — | — | — |
| A10 | Restoration | — | — | — | — |
| A11 | Close and quit prompts | — | — | — | — |
| A12 | Versions | — | — | — | — |
| A13 | Automation | — | — | — | — |
| A14 | Performance | — | — | — | — |
| A15 | Other scenes (info) | — | — | — | — |

## A16 — test seams

| Test file | Constructor sites | Controller members used | Replacement seam |
|---|---|---|---|
| — | — | — | — |

## Notes per candidate

Anything a probe taught that the verdict cell cannot hold: API names used by a
workaround, a surprising system behavior, the code size of the candidate.
