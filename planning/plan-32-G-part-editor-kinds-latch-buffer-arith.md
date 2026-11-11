# plan-32-G: Part editor kinds: latch, buffer, arith

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-32-F

This sub-plan adds the forms for three model-dependent kinds, and enables them in the kind
picker, using plan-32-F's two checks: the drift test against the backend's key arrays, and the
round trip over every shipped part with empty `extras`.

Kinds and measured populations (2026-11-11):

| Kind | Part file | Model source (KEYS arrays) | Shipped parts | Device keys that choose a model |
|---|---|---|---|---|
| latch | `latch.json` | `backend/src/models/latch.c` → 11 | 750 | `model` (`DEVICE_KEYS` at line 986) |
| buffer | `buffer.json` | `backend/src/models/buffer.c` → 11 | 2135 | `model` (`DEVICE_KEYS` at line 644: `model`, `timing`, `units`, `invert`, `output`, `domains`) |
| arith | `arith.json` | `backend/src/models/arith.c` → 7 | 107 | `model` (`DEVICE_KEYS` at line 776: `model`, `timing`, `output`, `openDrain`, …) |

Sum: 29 KEYS arrays, 2992 shipped parts.

References:

- plan-32-C — `PartDraft`, the extras rule, `PartValidator`, `BundledPartFile`.
- plan-32-F — the drift test (`PartKindKeysTests`) and the round-trip test (`PartKindRoundTripTests`)
  this sub-plan extends, and the model-dependent form pattern (`MuxDraft`, `DecoderDraft`).
- `docs/parts-format.md` — § Parts, § Supply voltage, § Two supply domains (`domains`, `vccB`, `inputMaxB`
  for `buffer`'s translator model).
- `backend/src/models/{latch,buffer,arith}.c` — the `KEYS` arrays.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-F complete: the flip-flop and decoder forms pass their round trip | `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed` | NOT MET at writing; starts after plan-32-F |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- File › New Part… lists `latch`, `buffer` and `arith` as enabled kinds; each opens an editor
  whose form fills every key of the kind's device and part objects for the chosen `model`.
- Every shipped part of the three kinds (2992) round-trips with an empty `extras`, and the
  backend accepts each encoded file.

### Non-goals (explicit constraints)

- **Only these three kinds change.** Shipped files and goldens are untouched.
- **The buffer's two-domain form is the backend's.** The `domains` key is written as the
  backend reads it (`docs/parts-format.md` § Two supply domains); the editor does not compute voltages.
- **No model the backend lacks.** A model name is taken from the backend's `model` handling in each
  kind's source; the form lists exactly those.

## 2. Current State

- All three kinds take a `model` key (`rg -c '"model"'` in each file: latch 4, buffer 3, arith 4 lines).
- `buffer.json` has 2135 shipped parts, the largest of the three; its models include the
  dual-supply translator, whose keys `domains`, `vccB` and `inputMaxB` are in
  `docs/parts-format.md` § Two supply domains.
- Shipped sample: `rg -o '"function"' schematical/schematical/Parts/buffer.json | wc -l` → 152 devices;
  `latch.json` → 28; `arith.json` → 16.

### Verified properties

- **`buffer`'s device keys are as cited.** Read `backend/src/models/buffer.c:644–645`:
  `{"model", "timing", "units", "invert", "output", "domains"}`.
- **`arith`'s device keys begin `model`, `timing`, `output`, `openDrain`.** Read
  `backend/src/models/arith.c:776–777`.
- **`latch`'s device keys begin `model`, `timing`.** Read `backend/src/models/latch.c:986–987`.

## 3. Design Overview

1. **Drift test**: `PartKindKeysTests` gains the three kinds (its parser already reads every
   `KEYS[]` array in a kind's model file, so it needs only the kind names added).
2. **Model-dependent drafts**: `LatchDraft`, `BufferDraft`, `ArithDraft`, each with a `model`
   and one typed sub-draft per model, as `MuxDraft` in plan-32-F.
3. **Round trip**: `PartKindRoundTripTests` gains the three kinds (the same test loop).

Where correctness risk concentrates: `buffer`'s translator model (the two-domain keys). A wrong
`domains` split is accepted by the form and rejected by the backend's rule "every pin of every
pinout except ground, NC and absent pins belongs to exactly one domain" (`docs/parts-format.md`).
The round trip over all 2135 buffer parts is the check that the form writes the split as the file has it.

### Rejected alternatives

- **One generic draft for all three kinds.** Rejected: the three models have different keys
  (`latch` has its own unit and timing keys, `arith` its operation list); one draft would hold
  most keys in `extras` and lose the form's value.

## 4. Detailed Design

### 4.1 Models and their forms

- **latch** (`latch.c`, 11 arrays): device `model` (line 986) selects the latch variant; each
  variant's unit and timing keys are its own arrays. The form shows the chosen variant's sections.
- **buffer** (`buffer.c`, 11 arrays): device `model` (line 644) selects the buffer's model; the
  translator is one of them (`docs/parts-format.md` § Two supply domains: "Today only the `buffer`
  kind's model `translator` takes these keys"). The translator's device has `domains` (exactly two, each `{power, pins}`)
  and each timing entry has `vccB` and `inputMaxB` (`docs/parts-format.md` § Two supply domains).
  Its form shows the two domain lists as two pin-name pickers, one per domain.
- **arith** (`arith.c`, 7 arrays): device `model` (line 776) selects among the arithmetic
  models; `openDrain` and `output` are the output keys. The form shows the chosen model's sections.

The per-array key names are the drift test's list (Phase 1), derived from the source
(`rg -n 'KEYS\[\] = ' backend/src/models/{latch,buffer,arith}.c`), so the form is checked
against the backend, not against this document.

## Phases

### Phase 1 — Drift test: the three kinds

One line: the drift test fails listing every backend key of the three kinds that the form lacks.

- [ ] `schematical/schematicalTests/PartKindKeysTests.swift`: add latch, buffer and arith to the kinds the test covers.
- [ ] Run it; it fails listing the missing keys (RED).

Acceptance: the test fails with the missing keys of the three kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `failed`, listing the keys (est. 3 min).
Commit: —

### Phase 2 — Forms for latch, buffer and arith

One line: each kind's form covers every key of Phase 1 for each of its models.

- [ ] `Editor/LatchDraft.swift`, `BufferDraft.swift`, `ArithDraft.swift`: each with `model`, one sub-draft per model, `init(device:part:)` and `encoded()`; unknown keys stay in `extras`.
- [ ] `BufferDraft`: the translator's `domains` as two pin-name pickers (power pin name and the domain's other pins), and `vccB`/`inputMaxB` on the timing entries.
- [ ] `PartEditorWindow.swift`: a kind section per kind; the chosen model's sections only.
- [ ] `PartDraft.enabledKinds` gains `latch`, `buffer`, `arith`.

Acceptance: `PartKindKeysTests` passes for the three kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `passed` (est. 5 min).
Commit: —

### Phase 3 — Round trip over every shipped part

One line: all 2992 shipped parts round-trip with no key in `extras`, and the backend accepts each.

- [ ] `PartKindRoundTripTests.swift`: the three kinds added to the loop of plan-32-F Phase 3.

Acceptance: zero mismatched, zero with extras, zero rejected, for latch (750), buffer (2135) and arith (107).
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed`, with the lines `checked 750 parts of latch, 0 mismatched, 0 with extras, 0 rejected`, `checked 2135 parts of buffer, …`, `checked 107 parts of arith, …` (est. 10 min; the populations are the whole of each kind).
Commit: —

### Phase 4 — Runtime proof

One line: one part of each kind is created, saved and loaded back in the app.

- [ ] No code. From New Part…, create a latch, a buffer (translator model, two domains) and an arith part; save and load each with File › Load Part….

Acceptance: each loads back into the drawer.
  Check: the run skill's session (est. 25 min).
Commit: —

## Validation Plan

- Tests: `PartKindKeysTests` (Phases 1–2), `PartKindRoundTripTests` (Phase 3).
- Coverage check: `rg -c 'KEYS\[\] = ' backend/src/models/{latch,buffer,arith}.c` → 11, 11, 7; the test prints the array count it parsed.
- Runtime proof: Phase 4.
- Doc sync: none in `docs/parts-format.md` (the format is the backend's, unchanged).
- Final gate (once, after Phase 4): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (est. 15 min, background per the project's tiers).

## Decisions

- **The three kinds take the same model-dependent form pattern as plan-32-F's mux and decoder** — the author's choice (§ 3).
- **Translator `domains` are edited as two pin-name lists** — the author's choice: the backend's rule is per domain (`docs/parts-format.md`), so the form shows the two domains as the file does.

## Corrections

(Filled in during execution.)

## Summary

The risk is `buffer`'s two-domain split; the 2135-part round trip checks it. Untouched: shipped
files, the backend, and the other kinds.
