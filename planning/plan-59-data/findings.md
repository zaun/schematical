# plan-59 findings

Filled in while plan-59-A, -B and -C run. Verdicts and their rules are in
`probes.md`. A cell is `—` until its probe has run. An evidence cell holds one
command with its output, or `evidence/<probe>-<candidate>.png`.

## Baselines

| Baseline | Value | Command |
|---|---|---|
| B1 edit-lag median / p90 | — | — |
| B1 drag-timing median / p90 | — | — |
| B2 relative `memoryFile.path` for Save As into a second folder | — | — |
| B3 `SchematicFile(document:fileURL:)` main-thread median of 20 | — | — |

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
