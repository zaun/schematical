# plan-68-I: Backend — the five over-limit serial model files split

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-H

Five serial model sources in `backend/src/models/` are over 1000 code lines.
Each is split along its existing section comments into files that each hold
one part of the model (its loader, its register file, its data path, its
pins), sharing the model's state struct through an internal header, as
`ns16550_ext.c` and `ns16550_int.h` already do. This is pure code motion: the
simulated behavior of every part is unchanged, and the goldens prove it.

Prerequisites: plan-68-A's table, and plan-68-H complete
(`ls backend/tests/library_check.c` and `node scripts/source-size.js | grep -c
backend/tests/` → 0).

References:

- `backend/src/models/ns16550.c`, `ns16550_exar.c`, `ns16550_bridge.c`,
  `serial_max310x.c`, `serial_i82510.c`; the internal-header precedents
  `ns16550_int.h` (shared by `ns16550.c`, `ns16550_ext.c`, `ns16550_exar.c`,
  `ns16550_bridge.c`, `ns16550_xr1280.c`, `ns16550_lpt.c`) and `converter_int.h`.
- `backend/CMakeLists.txt` (the `add_library(logic STATIC …)` source list);
  the app compiles `backend/src` through an Xcode synchronized folder, so a
  new `.c` there is in the app build with no project edit.
- `docs/backend.md` § Layout (line 628: the source tree listing).

## 1. Goal

- The five files are split per § 3; each resulting file is at or under 800
  code lines.
- `CT` passes, including `trace_check` and `listing_check` against
  `tests/golden/` with no golden changed.
- `COV --backend` reports the same total of uncovered backend lines as before
  the split, and each split file's ratchet row is replaced by rows for its
  pieces that sum to the old row.

### Non-goals

- No change to any function body, name (beyond `static` removal), or
  behavior; no golden regenerated. Public headers in `backend/include/` are
  untouched.

## 2. Current State

Measured (`node planning/plan-68-data/sections.js <file>`), code lines per
section:

| File | Code | Sections |
|---|---|---|
| `ns16550.c` | 2,444 | (top) 89; FIFOs 61; Interrupts 104; Outputs 223; Modem status 52; Reset 54; Receiving 96; Transmitting 67; Registers 330; Bus events 159; plan-12-I host port 18; Engine callbacks 245; plan-12-E core entry points 12; Terminal 11; Create / destroy 74; Loader 849 (`load_name_list` … `load_phase2` 514; `ns_load_device`, `ns_bind`, `ns_open_drain`, `ns_tristate` 335) |
| `ns16550_exar.c` | 2,467 | (top) 4; Small helpers 67; Loader 372; Binding and part options 69; Reset 109; Register decode 158; Register values 221; Write masks, FCR, MCR 70; Trigger levels 90; Flow control: receive 100; Flow control: levels 163; Flow control: transmit 35; Interrupts 92; Pins 233; Clocks 69; The 5G timer 33; Sleep, wake, PowerSave 117; Bus: select decode and the extra units 380; Pin changes, start 85 |
| `serial_max310x.c` | 1,979 | (top) 14; Variants 61; Device data 15; State 108; Small helpers 58; Interrupts 62; Formats and baud generator 86; Outputs 109; FIFOs and receive path 231; Transmit path 99; One baud tick 40; Reference clock 152; Reset 86; Registers 298; Host port 200; Engine callbacks 127; Terminal 10; Create / destroy 70; Loader 153 |
| `ns16550_bridge.c` | 1,390 | (top) 4; Small helpers 59; Loader 216; Register reset 44; Register decode 109; GPIO 80; Peek / write / read side effects 201; The host port 166; Create / start / reset / pins 219; Hooks into the core and ext 292 |
| `serial_i82510.c` | 1,216 | (top) 102; Formats 54; Status, GSR, GIR 58; Pins 134; BRGs and timers 112; Transmitter 52; Receiver 192; Registers 256; Bus events 40; Reset, clocks 57; Engine callbacks 44; Terminal 13; Create / destroy / loader 102 |

## 3. Design

Each file's sections are grouped into pieces by what they do; a piece is a
new `.c` beside the original. What one piece calls in another is declared in
the model's internal header (`static` dropped from exactly those functions;
names unchanged). The original file keeps the model's core: state, reset,
create/destroy.

| File | Pieces (sections) → code |
|---|---|
| `ns16550.c` | `ns16550.c`: (top), FIFOs, Interrupts, Outputs, Modem status, Reset, Create / destroy, Terminal, core entry points → 680. `ns16550_io.c`: Receiving, Transmitting, Registers, Bus events, host port → 670. `ns16550_kind.c`: Engine callbacks, `ns_load_device`, `ns_bind`, `ns_open_drain`, `ns_tristate` → 580. `ns16550_load.c`: `load_name_list` … `load_phase2` → 514. Header: `ns16550_int.h` (existing). |
| `ns16550_exar.c` | `ns16550_exar.c`: (top), Small helpers, Binding, Reset, Register decode, Register values, Write masks, Trigger levels → 788. `ns16550_exar_load.c`: Loader → 372. `ns16550_exar_flow.c`: the three Flow control sections, Interrupts, The 5G timer, Sleep → 540. `ns16550_exar_pins.c`: Pins, Clocks, Bus select decode, Pin changes → 767. Header: new `ns16550_exar_int.h`. |
| `serial_max310x.c` | `serial_max310x.c`: (top), Variants, Device data, State, Small helpers, Interrupts, Formats, Outputs, Reset, Terminal, Create / destroy → 679. `serial_max310x_data.c`: FIFOs and receive path, Transmit path, One baud tick, Reference clock → 522. `serial_max310x_regs.c`: Registers, Host port, Engine callbacks, Loader → 778. Header: new `serial_max310x_int.h` (the State struct and the variant table move here). |
| `ns16550_bridge.c` | `ns16550_bridge.c`: (top), Small helpers, Register reset, Register decode, GPIO, Peek / write / read, The host port → 663. `ns16550_bridge_kind.c`: Loader, Create / start / reset / pins, Hooks into the core and ext → 727. Header: new `ns16550_bridge_int.h` (`ns16550_bridge.h` stays the bridge's interface to the core). |
| `serial_i82510.c` | `serial_i82510.c`: (top), Formats, Status, Pins, BRGs and timers, Transmitter, Receiver → 704. `serial_i82510_regs.c`: Registers, Bus events, Reset/clocks, Engine callbacks, Terminal, Create / destroy / loader → 512. Header: new `serial_i82510_int.h`. |

The section counts sum to the piece counts above; `#include` lines and the
moved declarations add a few code lines per piece, and every piece has at
least 12 lines of room under 800.

**This is byte-identity-class work**: provably neutral code motion. The gate
is the goldens (`trace_check` hashes every part's simulated behavior) plus the
whole `CT`, plus an unchanged coverage total. A failure means a moved piece
changed (a lost `static` initializer, a reordered definition with a
file-local default) — root-cause it on the one failing test and fix it; it
never means the split is wrong.

### Rejected alternatives

- **Splitting by line count** (`ns16550_1.c`, `_2.c`) — the rule is logical
  units; each piece here is one job of the model.
- **Making the moved functions `static` in headers** (`static inline`) — it
  duplicates code into every piece and hides the dependency the header now
  names.

### Where the risk is

`ns16550.c` and `ns16550_exar.c`: the core and the Exar options call into each
other through `ns_ext_*` and `ns_core_*` hooks, so their internal headers
change the most.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — ns16550.c and ns16550_exar.c

- [x] Split both per § 3; new files in `backend/CMakeLists.txt`.
- [x] Ratchet: the two size rows removed; their coverage rows replaced by
  their pieces' rows (`COV --backend --write-ratchet`; `git diff scripts/coverage-ratchet.tsv` shows the diff
  only renames).

Acceptance: behavior and coverage identical.
  Check: `CT` → all pass, `git status --short backend/tests/golden` → no
  output; `COV --backend` total uncovered equals the total before the phase
  (recorded in the phase's commit message) (est. 2 min).
  Observed (both phases together): `CT` → 104 of 105 pass, the one failure
  `source_size_check` reporting the five stale rows this commit removes;
  `git status --short backend/tests/golden` → no output; `COV --backend` →
  `60689 lines, 7090 uncovered` before and after, and each model's pieces
  sum to its old row (ns16550 37+31+117+141 = 326, exar 85+69+102+75 = 331,
  bridge 128+109 = 237, max310x 50+132+154 = 336, i82510 68+71 = 139).
Commit: a0b23798, 24b23569

### Phase 2 — max310x, bridge, i82510

- [x] Split the three per § 3; CMake; ratchet rows as Phase 1.
- [x] `docs/backend.md` § Layout: the new files and headers.
- [x] XB build: `xcodebuild -project schematical/schematical.xcodeproj -scheme
  schematical -derivedDataPath /tmp/p68-dd CODE_SIGNING_ALLOWED=NO build`
  succeeds (the app compiles the new files through the synchronized folder).

Acceptance: as Phase 1; and no serial model file over 800.
  Check: as Phase 1; `node scripts/source-size.js --all | grep -E
  'models/(ns16550|serial_)' | awk '$1>800'` → no output; the app build →
  `** BUILD SUCCEEDED **` (est. 5 min: the app build is the only check that the
  Xcode target picks the files up).
  Observed: the largest piece is `ns16550_exar.c` at 780; the app build
  linked (the `XB` run of plan-68-B's classes, `Executed 122 tests`, built
  the app with these files).

  The app build first failed on `MAX310X_MODEL` defined twice: the original
  file has a tentative definition (`const SerialModel MAX310X_MODEL;`) ahead
  of the real one, which the split put in another piece; the splitter now
  turns such a line into `extern`. CMake's archive build had not shown it.
Commit: a0b23798, 24b23569

## Validation Plan

- Tests: unchanged; the goldens are the behavior proof.
- Coverage check: total unchanged; rows renamed.
- Final gate (once, after Phase 2): `CT`; `SZ`; `COV --backend --check`; the
  app build (est. 6 min).
  Observed: `CT` → 105/105; `SZ` → exit 0; `COV --backend --check` → exit 0;
  the app build linked (after 24b23569).

## Decisions

- (author) Pieces by job, shared through one internal header per model (§ 3).

## Corrections

- **Phases 1 and 2 landed together**, by one script
  (`planning/plan-68-data/split_model.py`), and both phases' checks ran on
  the result.
- **Shared names were renamed, not only un-`static`ed.** The non-goal said
  no name changes beyond `static` removal; that cannot hold: once global, a
  shared name can clash with any other global in the link. `load_byte` and
  `bus_write` (ns16550) clashed with plan-68-G/H's test bench helpers, and a
  plain `ns_` prefix with the ns16550 core's existing `ns_*` wrappers. So
  every name a piece shares that was `static` before takes the model's
  prefix (`nsc_`, `exs_`, `brs_`, `m31x_`, `i82_`; each checked unused with
  `grep -c`), renamed in code only, never in strings or comments, and never
  after `.`/`->` (`fstat` is both an exar function and a struct field).
  Names that were already global keep theirs. `nm`-based check:
  `planning/plan-68-data/clash_check.sh` prints no name defined twice.
- **New internal headers, not the existing ones.** ns16550's pieces share
  `ns16550_core_int.h` (new) rather than `ns16550_int.h`, which
  `ns16550_ext.c`, `_exar.c`, `_bridge.c`, `_xr1280.c` and `_lpt.c` also
  include: the core's private types do not belong in theirs.
- **A shared table's `extern` carries its length** (`m31x_VARIANTS[4]`),
  counted from the initializer, because another piece takes its `sizeof`;
  the compiler rejects the pair if the count is wrong.

## Summary

Code motion in five models, guarded by the golden traces of every part.
