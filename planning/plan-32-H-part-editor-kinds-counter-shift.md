# plan-32-H: Part editor kinds: counter, shift

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-32-G

This sub-plan adds the forms for the two kinds with the most keys per shipped part of the
model-dependent kinds: the counter (20 key arrays) and the shift register (12). It uses the same
two checks as plan-32-F and plan-32-G: the drift test against the backend's key arrays, and the
round trip over every shipped part with empty `extras`.

Kinds and measured populations (2026-11-11):

| Kind | Part file | Model source (KEYS arrays) | Shipped parts | Device keys that choose a model |
|---|---|---|---|---|
| counter | `counter.json` | `backend/src/models/counter.c` → 20 (plus `counter_div.c`, 5 arrays, the divider models) | 730 | `model` (`DEVICE_KEYS` at line 1947: `model`, `timing`, `units`, `byteBus`, `sections`, …) |
| shift | `shift.json` | `backend/src/models/shift.c` → 12 (plus `shift_special.c`, 6 arrays) | 583 | `model` (`DEVICE_KEYS` at line 1112: `model`, `timing`, `units`, …) |

Sum: 32 KEYS arrays in the two kinds' main files (`counter.c` 20, `shift.c` 12), 1313 shipped parts.
The divider models (`counter_div.c`) and the special shift models (`shift_special.c`) are part of the
same kinds and their arrays are part of the drift test's list (`PartKindKeysTests` parses every `KEYS[]`
array in the kind's own source files, so the list includes them).

References:

- plan-32-C — `PartDraft`, extras, `PartValidator`, `BundledPartFile`.
- plan-32-F — the drift test and round-trip test; the model-dependent pattern (`MuxDraft`).
- plan-32-G — the same pattern for latch, buffer, arith.
- `docs/parts-format.md` — § Parts, § Values (the logic families `LV`, `ALV`, …).
- `backend/src/models/{counter,counter_div,shift,shift_special}.c` — the key arrays.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-G complete: latch, buffer and arith pass their round trip | `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed` with the latch, buffer and arith lines | MET (run 2026-11-11 in the P-32 worktree: `PartKindRoundTripTests` passed with the latch, buffer and arith lines 0/0/0) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- File › New Part… lists `counter` and `shift` as enabled kinds; each opens an editor whose form
  fills every key of the device and part objects for the chosen `model`.
- Every shipped part of the two kinds (1313) round-trips with an empty `extras`, and the backend
  accepts each encoded file.

### Non-goals (explicit constraints)

- **Only these two kinds change.** Shipped files and goldens are untouched.
- **The counter's `sections` and `byteBus` keys are written as the backend reads them.** The editor
  does not check a section's bit widths beyond the backend's own rules.
- **No model the backend lacks.**

## 2. Current State

- `counter.json`: 730 shipped parts, 57 devices (`rg -o '"function"' … | wc -l`). Its device keys
  begin `model`, `timing`, `units`, `byteBus`, `sections` (`counter.c:1947`).
- `shift.json`: 583 shipped parts, 36 devices. Its device keys begin `model`, `timing`, `units`
  (`shift.c:1112`).
- Both files use `model` (`rg -c '"model"'`: counter 4 lines, shift 4 lines in their model sources).

### Verified properties

- **counter's device keys begin as cited.** Read `backend/src/models/counter.c:1947–1948`.
- **shift's device keys begin as cited.** Read `backend/src/models/shift.c:1112–1113`.

## 3. Design Overview

1. **Drift test**: `PartKindKeysTests` gains `counter` and `shift`.
2. **Drafts**: `CounterDraft` and `ShiftDraft`, each with a `model` and a typed sub-draft per model
   (the counter's `sections` as a list of section drafts; the shift's `units` as a list).
3. **Round trip**: `PartKindRoundTripTests` gains the two kinds.

Where correctness risk concentrates: the counter's `sections` (a list of bit sections, each with its
own keys), the largest nested structure of any kind in the editor. The round trip over all 730
counter parts checks that each section round-trips in order and unchanged.

### Rejected alternatives

- **Showing `sections` as JSON text.** Rejected: the form's value is structured editing, and a text
  field would let the user write a section the backend rejects with no place to point at.

## 4. Detailed Design

- **counter** (`counter.c`, 20 arrays): device `model` selects the counter variant; `byteBus` and
  `sections` (line 1947) are the variant's bus and section keys. The divider variants use the arrays
  of `counter_div.c` (5). The form shows the chosen model's sections; `sections` is a list of
  section drafts, each with the section's own keys.
- **shift** (`shift.c`, 12 arrays): device `model` (line 1112) selects the shift register variant;
  the special variants use `shift_special.c` (6 arrays). The form shows the chosen model's sections.

The per-array keys are the drift test's list, derived from the source (`rg -n 'KEYS\[\] = '`
over `counter.c`, `counter_div.c`, `shift.c`, `shift_special.c`), not from this document.

## Phases

### Phase 1 — Drift test: counter and shift

One line: the drift test fails listing every backend key of the two kinds that the form lacks.

- [x] `PartKindKeysTests.swift`: add counter and shift, with their `_div` and `_special` files.
- [x] Run it; RED.

Acceptance: the test fails listing the missing keys.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `failed`, listing the keys (est. 3 min).
  Result (2026-11-11, RED): failed, `["shift": ["a", "aEnable", "active", "address", "asyncSync", "b", "chipSelect", …], "counter": [...]]`; arrays parsed counter 31, shift 24 (Corrections).
Commit: 092efcb

### Phase 2 — Forms for counter and shift

One line: each kind's form covers every key of Phase 1 for each model.

- [x] `Editor/CounterDraft.swift`, `ShiftDraft.swift`: `model`, one sub-draft per model, `init(device:part:)`, `encoded()`; unknown keys stay in `extras`.
- [x] `CounterDraft.sections`: a list editor; each section's keys in its own sub-draft.
- [x] `PartEditorWindow.swift`: a kind section per kind; the chosen model's sections only.
- [x] `PartDraft.enabledKinds` gains `counter`, `shift`.

Acceptance: `PartKindKeysTests` passes for the two kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `passed` (est. 5 min).
  Result (2026-11-11): passed (`counter: 31 KEYS arrays, 89 keys, seq.c helper keys not read: ["when"]`, `shift: 24 KEYS arrays, 82 keys, seq.c helper keys not read: []`).
Commit: 092efcb

### Phase 3 — Round trip over every shipped part

One line: all 1313 shipped parts round-trip with no key in `extras`, and the backend accepts each.

- [x] `PartKindRoundTripTests.swift`: counter and shift added to the loop.

Acceptance: zero mismatched, zero with extras, zero rejected, for counter (730) and shift (583).
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed`, with the lines `checked 730 parts of counter, 0 mismatched, 0 with extras, 0 rejected` and `checked 583 parts of shift, …` (est. 6 min).
  Result (2026-11-11): passed: `checked 730 parts of counter, 0 mismatched, 0 with extras, 0 rejected`, `checked 583 parts of shift, 0 mismatched, 0 with extras, 0 rejected`. `sections` is a list of section entries (model `ripple`), each section's keys its own fields; `byteBus` an object (model `sync`).
Commit: 092efcb

### Phase 4 — Runtime proof

One line: a counter and a shift part are created, saved and loaded back in the app.

- [x] No code. From New Part…, create a counter (a multi-section variant) and a shift register; save and load each with File › Load Part….

Acceptance: each loads back into the drawer.
  Check: the run skill's session (est. 20 min).
  Result (2026-11-11, `/tmp/p32/scripts/guarded.sh`, this build frontmost before and after every cycle): New Part… → the kind's row opened the kind's editor, Open Part… a copy of a shipped part filled it (the window took the file's name), ⌘S rewrote the file (new mtime), and File › Load Part… registered it. The drawer count went 26,636 → 26,646 over the ten loads of this session (G's three, then H's two, I's three, J's two): ZZH-COUNTER (SN74LS90DR, model ripple, with `sections`) and ZZH-SHIFT (CD74AC164BQAR).
Commit: (this commit)

## Validation Plan

- Tests: `PartKindKeysTests` (Phases 1–2), `PartKindRoundTripTests` (Phase 3).
- Coverage check: `rg -c 'KEYS\[\] = ' backend/src/models/{counter,counter_div,shift,shift_special}.c` → 20, 5, 12, 6; the test prints the count it parsed.
- Runtime proof: Phase 4.
- Doc sync: none (the format is the backend's, unchanged).
- Final gate (once, after Phase 4): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (est. 15 min, background per the project's tiers).

## Decisions

- **Counter sections are structured in the form, not text** — the author's choice (§ 3, rejected: JSON text).
- **Divider and special models are in the same kind's form** — the author's choice: they are models of the same kinds (their keys are in the same `model` switch), so a separate kind would split one picker entry.

## Corrections

- **Arrays parsed** (2026-11-11): counter 31 (counter.c 20 + counter_div.c 5 + seq.c 6),
  shift 24 (shift.c 12 + shift_special.c 6 + seq.c 6). § Kinds counted 32 in the two main
  files; with their helper files the drift test parses 55.
- **Timing keys outside the `KEYS[]` arrays** (found 2026-11-11): a model's extra timing
  keys are tables of their own, `RIPPLE_EXTRA = {{"stage", 1}}` (counter.c) and
  `FIFO_EXTRA = {{"fallThrough", 1}}` (shift_special.c), passed to `seq_timing`. The
  generator reads `*_EXTRA[]` tables too, so the timing map's entries have `stage` /
  `fallThrough` fields (only ripple counters and FIFOs use them); without them the
  timing entries looked like free-form maps and `rise`/`fall`/`inputMax` had no control.
  Regenerating the F and G kinds with this change left their files unchanged.
- **Forms are generated** (plan-32-F Corrections): `CounterDraft`, `ShiftDraft`.

## Summary

The risk is the counter's nested sections; the 730-part round trip checks them. Untouched: shipped
files, the backend, and the other kinds.
