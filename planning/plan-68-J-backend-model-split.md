# plan-68-J: Backend — the other eleven over-limit backend files split

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-I

The last eleven over-limit files in `backend/src`: the converter loader and
model, the part library loader, the serial memories, the counters, the shift
registers, the VIA, the engine's contacts, the character LCD and the EEPROM.
Same method as plan-68-I: pieces by job, an internal header per model, pure
code motion proved by the goldens. After this letter no backend file is over
1000 code lines.

Prerequisites: plan-68-A's table, and plan-68-I complete (`node
scripts/source-size.js | grep -cE 'models/(ns16550|serial_)'` → 0).

References:

- The eleven files below; internal-header precedents `converter_int.h`,
  `library_internal.h`, `ns16550_int.h`.
- `backend/CMakeLists.txt`; `docs/backend.md` § Layout; plan-68-I § 3 (the
  method and its byte-identity gate, which this letter reuses unchanged).

## 1. Goal

- Each file is split per § 3, every piece at or under 800 code lines.
- `CT` passes with no golden changed; `COV --backend`'s total uncovered is
  unchanged; the eleven size rows are gone from the ratchet.
- `node scripts/source-size.js | grep -c '^[0-9].*backend/'` → 0.

### Non-goals

As plan-68-I: no function body or behavior changes; public headers untouched.

## 2. Current State

Measured (`node planning/plan-68-data/sections.js <file>`; function ranges
inside one section counted with `sed -n '<a>,<b>p' | node
planning/plan-68-data/loc.js`), code lines:

| File | Code | Sections |
|---|---|---|
| `converter_load.c` | 3,573 | (top) 116; Fields and conditions 123; Patterns 186; Registers 171; Commands 231; Protocol 185; Buses 202; Conversion, ready, pins, DAC, choices 506; The rtc kind's clock 358; The display section 1,165 (`load_display_ram` … `load_display_ports` 652; `lcd_mode_slots` … `load_display` 513); The device 165; Pinouts and parts 165 |
| `converter.c` | 1,596 | (top) 6; Small helpers 113; Ready pins 49; Conversions 115; DAC registers 34; Register writes 89; Reset 43; Actions 202; bits in 170; bits out 211; SPI front end 157; I2C front end 97; The model 286; The kind 24 |
| `library.c` | 1,578 | (top) 8; Name sets 81; The library 114; Loading 1,049 (`file_add_part` … `load_part` 530; `load_device` … `load_parts` 519); Single part files 326 |
| `serialmem.c` | 1,523 | (top) 39; Device data 23; State 63; Inputs 12; Protection 37; Memory 38; Status 79; Busy 70; Commands 252; End of a transaction 326; Pins 81; Inspection hooks 44; Create / destroy 86; Loader 373 |
| `counter.c` | 1,845 | (top) 6; Shared pieces 101; sync 153; sync: loading 599; sync: counting 270; sync: outputs 107; sync: evaluation 136; ripple 334; The kind 79; Engine callbacks 60 |
| `counter_div.c` | 1,074 | (top) 64; johnson 186; seg7 459; divn 173; rate 192 |
| `shift.c` | 1,149 | (top) 6; Shared pieces 88; The shift model 105; loading 429; running 384; The kind 137 |
| `pio_via6522.c` | 1,115 | (top) 100; Three-valued helpers 33; Pins 100; Reset 40; Reads 93; Writes 163; Timers 154; Shift register 121; Cycle hooks 64; Control line inputs 68; Engine hooks 142; Loader 37 |
| `engine/sim.c` | 1,099 | (top, the event engine) 485; Contacts 614 |
| `charlcd.c` | 1,038 | (top) 48; Loader 260; Model 83; Addresses 57; Instructions and data 154; Reads 95; Bus events 231; Lights 13; The image 97 |
| `eeprom.c` | 1,001 | (top) 16; Instance state 90; Inputs 90; The load window 154; The write cycle 69; Writes 85; Reads 106; Model callbacks 38; Inspection hooks 38; Kind: devices, pinouts, parts 251; Model creation 64 |

## 3. Design

| File | Pieces (sections) → code | Header |
|---|---|---|
| `converter_load.c` | `converter_load.c`: (top), Fields, Patterns, The device, Pinouts and parts → 755. `converter_load_proto.c`: Registers, Commands, Protocol, Buses → 789. `converter_load_conv.c`: Conversion, ready, pins, DAC, choices → 506. `converter_load_rtc.c`: the rtc clock → 358. `converter_load_display.c`: `load_display_ram` … `load_display_ports` → 652. `converter_load_lcd.c`: `lcd_mode_slots` … `load_display` → 513 | new `converter_load_int.h` (the `Ld`/`Dl` loader contexts and the shared `dl_*` helpers) |
| `converter.c` | `converter.c`: (top), Small helpers, Ready pins, Conversions, DAC registers, Register writes, Reset, Actions, The kind → 675. `converter_proto.c`: bits in, bits out, SPI front end, I2C front end → 635. `converter_model.c`: The model → 286 | `converter_int.h` (existing) |
| `library.c` | `library.c`: (top), Name sets, The library, Single part files → 529. `library_load.c`: `file_add_part` … `load_part` → 530. `library_load_device.c`: `load_device` … `load_parts` → 519 | `library_internal.h` (existing) |
| `serialmem.c` | `serialmem.c`: (top), Device data, State, Inputs, Protection, Memory, Status, Busy, Pins, Inspection hooks, Create / destroy → 572. `serialmem_cmd.c`: Commands, End of a transaction → 578. `serialmem_load.c`: Loader → 373 | new `serialmem_int.h` |
| `counter.c` | `counter.c`: (top), Shared pieces, The kind, Engine callbacks → 246. `counter_sync.c`: sync, counting, outputs, evaluation → 666. `counter_sync_load.c`: sync: loading → 599. `counter_ripple.c`: ripple → 334 | `counter.h` (existing; the sync/ripple entry points join it) |
| `counter_div.c` | `counter_div.c`: (top), johnson, divn, rate → 615. `counter_seg7.c`: seg7 → 459 | `counter.h` |
| `shift.c` | `shift.c`: (top), Shared pieces, The shift model, running, The kind → 720. `shift_load.c`: loading → 429 | `shift.h` (existing) |
| `pio_via6522.c` | `pio_via6522.c`: (top), Three-valued helpers, Pins, Reset, Cycle hooks, Control line inputs, Engine hooks, Loader → 584. `pio_via6522_regs.c`: Reads, Writes, Timers, Shift register → 531 | new `pio_via6522_int.h` |
| `engine/sim.c` | `sim.c`: the event engine → 485. `sim_contacts.c`: Contacts → 614 | `engine/sim.h` / a new `engine/sim_int.h` for the `Sim` internals both use |
| `charlcd.c` | `charlcd.c`: everything but the Loader → 778. `charlcd_load.c`: Loader → 260 | new `charlcd_int.h` |
| `eeprom.c` | `eeprom.c`: (top) … Inspection hooks → 686. `eeprom_kind.c`: Kind, Model creation → 315 | new `eeprom_int.h` |

Byte-identity class, gate and failure handling exactly as plan-68-I § 3.

### Where the risk is

`engine/sim.c`: the event engine is in every test. `sim_contacts.c` needs the
`Sim` struct's internals, which today are file-private to `sim.c`; they move
to `engine/sim_int.h`, included only by the two engine files.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — converter_load.c, converter.c, library.c, serialmem.c

- [ ] Split per § 3; CMake; ratchet rows (size removed, coverage renamed).

Acceptance: behavior and coverage identical.
  Check: `CT` → all pass, `git status --short backend/tests/golden` → no
  output; `COV --backend` total uncovered unchanged (est. 2 min).
Commit: —

### Phase 2 — counter.c, counter_div.c, shift.c, pio_via6522.c

- [ ] Split per § 3; CMake; ratchet rows.

Acceptance: as Phase 1.
  Check: as Phase 1 (est. 2 min).
Commit: —

### Phase 3 — sim.c, charlcd.c, eeprom.c

- [ ] Split per § 3; CMake; ratchet rows.
- [ ] `docs/backend.md` § Layout: every file and header added in D and E.
- [ ] The app build (plan-68-I Phase 2's command).

Acceptance: as Phase 1, and nothing in `backend/` over 800.
  Check: as Phase 1; `node scripts/source-size.js --all | grep backend/ |
  grep -v third_party | awk '$1>800'` → no output; the app build → `** BUILD
  SUCCEEDED **` (est. 6 min).
Commit: —

## Validation Plan

- Final gate (once, after Phase 3): `CT`; `SZ`; `COV --backend --check`; the
  app build (est. 6 min).

## Decisions

- (author) As plan-68-I.

## Corrections

(none yet)

## Summary

Code motion in eleven backend files; `engine/sim.c` carries the blast radius
and goes last, behind the goldens.
