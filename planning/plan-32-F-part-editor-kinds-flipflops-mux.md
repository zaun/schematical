# plan-32-F: Part editor kinds: dff, jkff, mux, decoder

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-32-E

This sub-plan adds the form for four kinds to the editor, and enables them in the kind
picker. It follows plan-32-C's pattern: a kind section that edits the kind's device and
part keys, with every other key kept in `extras`. Each kind's form is checked two ways:
against the backend's own key list (so a key the backend accepts has a control), and
against every shipped part of the kind (so a shipped part's file round-trips with no key
left in `extras`).

Kinds and measured populations (2026-11-11):

| Kind | Part file | Model source (KEYS arrays, `rg -c 'KEYS\[\] = ' <file>`) | Shipped parts (`rg -o '"manufacturer"' <file> \| wc -l`) |
|---|---|---|---|
| dff | `dff.json` | `backend/src/models/dff.c` → 4 | 706 |
| jkff | `jkff.json` | `backend/src/models/jkff.c` → 4 | 186 |
| mux | `mux.json` | `backend/src/models/mux.c` → 4 | 346 |
| decoder | `decoder.json` | `backend/src/models/decoder.c` → 6 | 322 |

Sum: 18 KEYS arrays, 1560 shipped parts. The four kinds are plan-32's first group after gate
(plan-32-C). The other 20 kinds are plan-32-G, H, I and J (sizes in those files).

References:

- plan-32-C — `PartDraft`, `GateDraft`, the extras rule (§ 4.2), `PartValidator` (§ 4.7),
  `BundledPartFile` (§ 4.5), and the kind picker (`PartDraft.enabledKinds`).
- `docs/parts-format.md` — § Parts, § Pinouts and pins, § Values (logic families), § Supply voltage.
- `backend/src/models/{dff,jkff,mux,decoder}.c` — the `KEYS` arrays.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-32-E complete (the editor's preview pane exists) | `xcodebuild test … -only-testing:schematicalTests/BlockPreviewViewTests` → `passed` | NOT MET at writing; starts after plan-32-E |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run it before starting, and report it if you stop.

## 1. Goal

- File › New Part… lists `dff`, `jkff`, `mux` and `decoder` as enabled kinds, and each opens
  an editor whose form fills every key of the kind's device and part objects.
- Every shipped part of the four kinds (1560) round-trips through the editor draft with
  an empty `extras`, and the backend accepts each encoded file.

### Non-goals (explicit constraints)

- **Only these four kinds change.** Other kinds stay disabled in the picker; their forms are
  plan-32-G through J.
- **Shipped files do not change.** `Parts/dff.json`, `jkff.json`, `mux.json`, `decoder.json` and
  the goldens are untouched.
- **The kind's behavior is the backend's.** The editor writes keys; it does not decide
  logic. A key's meaning is the backend model's (`dff.c`, etc.).

## 2. Current State

- plan-32-C's gate form is the pattern: a `GateDraft` for the kind's device and part keys,
  and `extras` for keys without a control.
- The kind's model reads its keys with `DEVICE_KEYS` / `PART_KEYS` / per-unit `KEYS` arrays
  (in `dff.c`, `jkff.c`, `mux.c`, `decoder.c`).
- Shipped sample: `rg -o '"function"' schematical/schematical/Parts/dff.json | wc -l` → 22 devices
  (measured), `jkff.json` → 6, `mux.json` → 20, `decoder.json` → 32.

### Verified properties

- **The key arrays are the backend's list of accepted keys.** Read `backend/src/models/dff.c`
  (lines 25–28 and the unit and timing keys at 110–111: `UNIT_KEYS = {d, clk, pre, clr, q, qn}`,
  `TIMING_KEYS = {rise, fall, clearWins, note, vcc, …}`), and the corresponding arrays in
  `jkff.c`, `mux.c`, `decoder.c` (the counts above).

## 3. Design Overview

1. **Drift test** (Phase 1): `PartKindKeysTests` parses the `KEYS[] = { … }` arrays from the
   kind's model source with a regular expression, and asserts that `PartDraft.formKeys(kind)`
   (the Swift set of keys the form has controls for) contains every one. This is the check that
   fails when the backend gains a key the form lacks.
2. **Kind forms**: `DffDraft`, `JkffDraft`, `MuxDraft`, `DecoderDraft` (typed, as `GateDraft`),
   each with its `encoded()` and `init(device:part:)`; `MuxDraft` and `DecoderDraft` hold a
   `model` and one typed sub-draft per variant (§ 4.1); the form sections in `PartEditorWindow`.
3. **Round trip over the shipped population** (Phase 3): for every shipped part of the kind,
   `copy → draft → encode` equals the copy, `extras` is empty, and the backend accepts the file.

Where correctness risk concentrates: the **`extras` must be empty** check. Without it, a missing
control is silently preserved by the round trip and the drift test is the only guard. Both are
required. Schedule: the drift test first (red), then the controls, then the population.

### Rejected alternatives

- **Typed drafts with no `extras` for these kinds.** Rejected: plan-32-C keeps `extras` so that
  any key is lossless; removing it for these kinds would make a future backend key break saved parts.
- **A generic key/value grid for every kind.** Rejected: the owner asked for a form, and a grid
  puts the burden of valid keys on the user.

## 4. Detailed Design

### 4.1 Per-kind sections

Each kind's section shows the device's `pinouts` (the generic pinout table of plan-32-C) and
its kind keys. Key names below are read from the model sources (2026-11-11):

- **dff** (`dff.c`): device keys `DEVICE_KEYS` (line 25) and part keys `PART_KEYS` (line 28,
  `family`, `vcc`, `inputMax`); the unit keys `UNIT_KEYS` (line 110): `d`, `clk`, `pre`, `clr`,
  `q`, `qn`; the timing keys `TIMING_KEYS` (line 111), each entry a `rise`/`fall` pair with `vcc`
  and `inputMax`.
- **jkff** (`jkff.c`, 4 arrays): device keys `units`, `kInverted`, `presetActiveHigh`,
  `clearActiveHigh`, `clockEdge`, `timing` (line 20); part keys `family`, `vcc`, `inputMax`
  (line 23); unit keys `j`, `k`, `clk`, `pre`, `clr`, `q`, `qn` (line 70); timing keys `rise`,
  `fall`, `note`, `vcc`, `inputMax` (line 71).
- **mux** (`mux.c`, 4 arrays): the device has a `model` key that picks a multiplexer variant.
  Device keys (line 355) include `model`, `timing`, `output`, `units`, `disabled`, `invert`,
  `selectLatch`, `dataLatch`, `dataClock`, `outputClock`, `oes`; the select-group keys `SELECT_KEYS`
  (line 244: `units`, `disabled`, `invert`, `selectLatch`, `output`); unit keys `data`, `select`,
  `y`, `w`, `strobe`, `oe` (line 99). The form shows the sections of the chosen model only.
- **decoder** (`decoder.c`, 6 arrays): the device has a `model` key with several variants.
  Device keys (line 895) include `model`, `timing`, `output` and every variant's keys; the variant
  arrays are `BINARY_KEYS` (line 158: `units`, `outputsActive`, `unselected`, `disabled`,
  `latchActive`, `output`, `monitor`), `UNIT_KEYS` (line 208: `address`, `outputs`, `enables`,
  `latch`, `clock`), `PRIORITY_KEYS` (line 456: `inputs`, `inputsActive`, `code`, `codeActive`,
  `offset`, `idle`, `enableIn`, `groupSelect`, `enableOut`, `disabled`, `output`) and `SEG7_KEYS`
  (line 655: `inputs`, `segments`, `segmentsActive`, `glyphs`, `lampTest`, `blank`, `rbi`, `rbo`,
  `latch`, `phase`, `output`). The form shows the sections of the chosen `model` only.

The form has a control for each key of each array above (the union across the variants);
Phase 1's test derives the same list from the source, so the list is checked, not trusted.

### 4.2 The kind's picker entry

`PartDraft.enabledKinds` gains `dff`, `jkff`, `mux`, `decoder`. The picker's rows use the
kind names of plan-32-C.

## Phases

### Phase 1 — Drift test: the form's keys cover each kind's backend keys

One line: a failing test lists every backend key the form has no control for, for the four kinds.

- [ ] `schematical/schematicalTests/PartKindKeysTests.swift`: for each kind in {dff, jkff, mux, decoder}, parse the `KEYS[] = { … }` arrays from `backend/src/models/<kind>.c` (path from `#filePath`), and assert `PartDraft.formKeys(kind)` (a `Set<String>`, empty until Phase 2) contains each key.
- [ ] Run the test; it fails with the missing key names listed (the RED state of this sub-plan).

Acceptance: the test fails listing the keys of the four kinds (RED); the failure output is the list used by Phase 2.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `failed`, with the message naming every missing key of the four kinds (est. 3 min).
Commit: —

### Phase 2 — Kind forms

One line: each of the four kinds has a typed draft and a form section covering every key of Phase 1.

- [ ] `schematical/schematical/Editor/DffDraft.swift`, `JkffDraft.swift`, `MuxDraft.swift`, `DecoderDraft.swift`: each with `init(device:part:)` and `encoded()`; each unknown key stays in `extras`.
- [ ] `PartEditorWindow.swift`: a kind section per kind (§ 4.1).
- [ ] `PartDraft.formKeys(kind)` returns the keys each draft has controls for; `PartDraft.enabledKinds` gains the four kinds.
- [ ] Tests: `PartKindKeysTests` passes for the four kinds.

Acceptance: `PartKindKeysTests` passes for all four kinds.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindKeysTests` → `passed` (est. 5 min).
Commit: —

### Phase 3 — Round trip over every shipped part of the four kinds

One line: all 1560 shipped parts round-trip with no key in `extras`, and the backend accepts each.

- [ ] `schematical/schematicalTests/PartKindRoundTripTests.swift`: for each kind, for every part of `Parts/<kind>.json`: copy (plan-32-C § 4.5) → draft → encode → equal to the copy with `uuid` excluded; `extras` empty; `PartValidator` accepts the encoded file.
- [ ] The test prints `checked <N> parts of <kind>, <m> mismatched, <k> with extras, <r> rejected` per kind.

Acceptance: for dff (706), jkff (186), mux (346) and decoder (322), zero mismatched, zero with extras, zero rejected.
  Check: `xcodebuild test … -only-testing:schematicalTests/PartKindRoundTripTests` → `passed` with the four count lines reading `checked 706 parts of dff, 0 mismatched, 0 with extras, 0 rejected` and the matching lines for the other three (est. 8 min; the population is the whole of each kind, the smallest set that exercises every key of the kind).
Commit: —

### Phase 4 — Runtime proof

One line: a part of each kind is created in the app and saved.

- [ ] No code. Create one part of each of the four kinds from New Part…, save each, and load each back with File › Load Part….

Acceptance: each of the four parts loads back and appears in the drawer.
  Check: the run skill's app session (est. 25 min for four kinds).
Commit: —

## Validation Plan

- Tests: `PartKindKeysTests` (Phase 1–2), `PartKindRoundTripTests` (Phase 3).
- Coverage check: `rg -c 'KEYS\[\] = ' backend/src/models/{dff,jkff,mux,decoder}.c` → 4, 4, 4, 6, and the test's parsed list has the same number of arrays (`PartKindKeysTests` prints the count).
- Runtime proof: Phase 4.
- Doc sync: `docs/parts-format.md` § Parts is unchanged (the format is the backend's); the Help page is unchanged.
- Final gate (once, after Phase 4): `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical` (est. 15 min, background per the project's tiers).

## Decisions

- **Each kind's form is checked against the backend's `KEYS` arrays and against the whole shipped population with empty `extras`** — the author's choice (both checks are needed; § 3).
- **F takes dff, jkff, mux and decoder** — the author's choice. dff and jkff are the two
  flip-flop kinds, which have the simplest key sets (one unit, one timing list); mux and decoder
  are multi-model kinds, the first in the editor to show a model-dependent form (§ 4.1). Doing
  the simple pair and the model-dependent pair in one sub-plan proves both shapes before the
  larger kinds in plan-32-G, H and J.

## Corrections

(Filled in during execution.)

## Summary

The risk is a backend key with no control; the drift test and the empty-`extras` check
together catch it. Untouched: shipped files, the backend, and the other kinds.
