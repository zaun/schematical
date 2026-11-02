# plan-11-B: The `charlcd` kind — loader, HD44780 model, character ROM, state API

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-11-A

This sub-plan adds the `charlcd` part kind and its model. It has no
catalog parts yet (plan-11-C ships them), so every behavior here is tested
on an inline fixture device in `backend/tests/charlcd_test.c`.

- the loader for `charlcd` devices and `PartCharLcd` facts;
- the HD44780-compatible controller model:
  - bus: RS, R/W, E and DB0–7, in 8- and 4-bit modes;
  - the instruction set, the busy flag and execution times;
  - power-on reset, bus timing checks and X rules;
- the character ROM A00 (the 5×8 glyphs), transcribed from the datasheet;
- the backlight as a light; the VDD range and the backlight in the voltage
  check;
- a public state read-back, `circuit_lcd_state` (`lcd_state.h`): the dot
  image the display shows now, plus RAM and flags for tests.

Outcome of B: `ctest` 18 of 18 (T0 + 1, the new `charlcd_test`); goldens
byte-identical (no part uses the kind yet).

References:

- plan-11-A § Prerequisites (the gate for all of plan-11), § Decisions.
- `docs/backend.md` § Adding a kind (the five steps this follows).
- `docs/parts-format.md` § Pinouts and pins, § Values, § Supply voltage,
  § `led` (the light rule the backlight copies), § Sequential kinds (the X
  and Z signal rules this kind copies).
- Datasheets (the behavior's source, one family):
  - `datasheets/module-disp-char/NHD-0216K1Z-NSW-BBW-L.pdf` (PDF page
    numbers): instruction table p.7, AC timing pp.8–9, font table p.10,
    initialization examples p.11 (8-bit) and p.12 (4-bit);
  - `162H-BC-BW-Displaytech-Spec.pdf`: execution times p.13, AC timing
    pp.14–15, font table p.12;
  - `AMC1602G-full.pdf`: initialization flow p.18 ("Wait for more than
    40 ms after VDD rises to 4.5 V").
- `planning/completed/plan-04-A-eeprom-kind.md` (bus timing checks and
  "a violation stores X") and `planning/completed/plan-09-B-cpu-kind-6502.md`
  (`circuit_cpu_state` and `cpu_state.h`: the state API this copies).

## Prerequisites

The gate is plan-11-A § Prerequisites. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-11-A complete | `grep -c 'Commit: —' planning/plan-11-A-groundwork.md` → 0 (or the file is in `planning/completed/`) | MET (2026-11-01 at bffe730: 0) |
| `Part.connector_footprint` exists | `grep -n 'connector_footprint' backend/include/parts.h` → one line | MET (2026-11-01 at bffe730: parts.h:119) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `ctest --test-dir /tmp/logic-p11` → `100% tests passed, 0 tests failed out of 18`.
- `git diff --exit-code backend/tests/golden` → exit 0.
- `charlcd_test` passes every case of §6, including both NHD
  initialization examples (8-bit and 4-bit, their bytes and waits),
  followed by text.
- The ROM spot checks of §5 pass, and the glyph sheet that
  `charlcd_test --font` prints matches the datasheet's font table on a
  side-by-side look (§5.2).

### Non-goals (explicit constraints)

- No catalog part, footprint or golden changes (plan-11-C does those).
- No app change apart from what compiles automatically. The app compiles
  `backend/src/`, so it must still build (new model files, an appended
  `PartKind` value, an appended `Part` field, an appended `SimComponentType`
  hook).
- Existing kinds behave exactly as before. The voltage check's results for
  every existing circuit are unchanged (`voltage_test` passes unmodified).
- Not modeled, on purpose:
  - the LCD drive voltage and contrast (V0 is a plain input the model ignores);
  - the 5×10 font (the F bit is stored; these modules have 5×8 glass);
  - the second ROM codes (A02);
  - the edge rate limits tR/tF;
  - serial interfaces.

## 2. Current State

- **The kind table.** `KINDS[]` (`backend/src/library/kinds.c:22-38`) has
  14 kinds. The last is `CPU_KIND` (plan-09). `PartKind`
  (`backend/include/parts.h:24-28`) ends `PART_SHIFT, PART_CPU`. `Part`
  carries one info pointer per kind (`parts.h`, `Part` struct: `logic`,
  `sram`, …, `cpu`). `docs/backend.md` § Adding a kind gives the five steps.
- **Lights.** `lights.{h,c}` (`backend/src/models/lights.h`) tracks on-time;
  `led.c` reports one light (`led_lights`, `led_light_time`,
  `led.c:114-129`). `circuit_part_lights` and `circuit_part_light_time`
  (`circuit.c:397`) dispatch through `SimComponentType.lights` /
  `.light_time`, so any kind with those hooks is a light part. The app
  finds light parts by asking `circuit_part_light_time(…, nil, 0) > 0`
  (`Simulation.swift:235-237`).
- **Voltage check, LEDs.** `voltage.c:208` handles `PART_LED` through
  `led_pins` (`models/model_pins.h:14`): it carries the anode's level to the
  cathode net, then `continue`s past the `vcc` check (LEDs have no `vcc`).
- **State read-back precedent.** `circuit_cpu_state` (`circuit.c:491-506`)
  calls `SimComponentType.cpu_state` (`sim.h:152-153`) and fails with
  `part %zu is not a processor` for any other kind. `cpu_state.h` is a
  public header included by `circuit.h:25`.
- **Bus timing precedent.** `eeprom.c` checks setup and hold windows and
  stores X on a violation (`eeprom.c:1-30`, "Where the last load went, so a
  hold violation can turn it X", `eeprom.c:69`).
- **Test bench.** `tests/support/bench.{h,c}` drives any part's input pins
  by name and reads its outputs. `latch_test`, `counter_test` and
  `shift_test` use it. `logic_test(<name>)` in `backend/CMakeLists.txt:79-86`
  registers a test. There are 17 today (`grep -c '^logic_test'
  backend/CMakeLists.txt` → 17).
- **Datasheet facts** (read 2026-11-01 for this plan; full extract in
  plan-11-C §2):
  - all five modules are HD44780-compatible (ST7066U or S6A0069);
  - every font table is HD44780 ROM A00 (Japanese standard): ¥ at 5C, → at
    7E, ← at 7F, katakana A1–DF, Greek and math E0–FF, 10–1F and 80–A0
    blank. The three Newhaven tables are identical; Displaytech and Orient
    match the glyph set;
  - every bus timing table is identical (ns): tC 1200, tPW 140, tAS 0,
    tAH 10, tDSW 40, tH 10, tDDR 100 max, tDHR 10;
  - execution times at fOSC 270 kHz: Newhaven 37 µs for every instruction
    and data access, 1.52 ms for Clear and Home; Displaytech and Orient
    39 µs for instructions, 43 µs for data, 1.53 ms for Clear and Home;
  - power-on: every module's initialization flow waits more than 40 ms
    after VDD reaches 4.5 V.

### Measured populations

| What | Count | Command |
|---|---|---|
| Kinds today | 14 | `grep -c '^    &' backend/src/library/kinds.c` |
| Registered tests | 17 | `grep -c '^logic_test' backend/CMakeLists.txt` |
| ROM glyphs to transcribe | 191 | codes 20–7F (96) + A1–FF (95); 00–0F are CGRAM, 10–1F, 80–A0 blank |
| `SimComponentType` hooks | 13 | `sed -n '/^typedef struct {/,/} SimComponentType;/p' backend/src/engine/sim.h \| grep -c '(\*'` |

### Verified properties

- **The voltage check runs `vcc` checks only for parts with `vcc`**
  (`voltage.c`, the loop after line 236: `if (part->vcc == NULL) continue;`).
  So the LCD's VDD range is checked once `PartCharLcd` parts carry a `vcc`,
  as long as the backlight branch does not `continue` (§4.6).
- **Every time key fits `kind_ns`.** The longest, the 40 ms power-on wait,
  is 4·10¹⁰ ps, well inside `uint64_t`.
- **The bench can watch an inout bus.** `bench.c` reads any pin's value,
  and the EEPROM tests already read DATA# polling through it
  (`eeprom_test.c`).

## 3. Design Overview

Layers, lowest risk first:

1. **Loader and facts** (§4.1–4.2): `PART_CHARLCD`, `PartCharLcd`, device
   keys, pin binding. Tested by the loader's accept and reject cases.
2. **Character ROM** (§5): a const table, used only by the renderer.
3. **Controller model** (§4.3–4.5): one state machine driven by E edges and
   timers. Both bus widths share one byte path, which works on whole bytes;
   4-bit mode only assembles nibbles in front of it.
4. **State read-back and lights** (§4.7): `circuit_lcd_state` renders the
   dot image from RAM, registers and sim time. The backlight is one light.
5. **Voltage** (§4.6): VDD range through `vcc`; the backlight carries like
   an LED.

**Where the risk is:**

- the X and LOST rules (§4.5), which decide what a bad write does;
- the 4-bit nibble phase;
- DDRAM address wrap and display shift.

All three sit behind `charlcd_test` cases that run the datasheet's own
init sequences. The ROM transcription (191 glyphs) is the other risk: a
wrong dot is silent. It gets spot checks against a few known glyphs plus a
printed glyph sheet compared with the datasheet page (§5.2).

**Correctness gate:** behavior tests (`charlcd_test`). Goldens must stay
byte-identical because no part uses the kind yet. A golden diff here is a
bug in shared code (voltage, circuit, loader); root-cause it.

**Rejected alternatives**

- *Rendering the font in the app.* Rejected: the dot image would then be
  untestable in C, and the ROM, cursor, blink and shift rules would be
  split across two languages. The backend owns the image; the app paints
  it.
- *A pixel hook in `lights`* (every dot a light). Rejected: 1280 lights per
  display, with on-time tracking the app does not need. LCD dots are
  state, not pulsed LEDs.
- *Ignoring bus timing* (a perfect bus). Rejected: plan-04 and plan-09 check
  datasheet timing, and an LCD wired straight to a fast CPU is the classic
  timing mistake (plan-11-C §4.6 tests exactly that).
- *Treating a write while busy as LOST.* Rejected: the HD44780 datasheet
  says the next instruction "will not be accepted" while BF = 1, so the
  model ignores it.

## 4. Detailed Design

### 4.1 Facts (`parts.h`)

```c
/* panel: "yellow-green" (STN positive: dark dots on yellow-green) or
   "blue-negative" (STN negative: light dots on blue). backlight_color: NULL
   when the module has no backlight. */
typedef struct {
    unsigned columns, rows;
    const char *controller, *rom, *panel, *backlight_color;
    const size_t *bus_pins; size_t bus_pin_count;  /* RS, R/W, E, DB0-DB7: indices into Part.pins */
} PartCharLcd;
```

`bus_pins` lets the app draw the bus on the symbol's left side, the way
`PartDisplay.segment_pins` does for 7-segment displays (plan-11-D).

- `PartKind` gains `PART_CHARLCD` after `PART_CPU`.
- `Part` gains `const PartCharLcd *charlcd;`, appended after
  `connector_footprint` (plan-11-A) and before `internal`, so every
  existing field keeps its offset.
- `Part.vcc` comment: character LCDs have one.

### 4.2 Part file (`Parts/charlcd.json`, kind `charlcd`)

Device keys:

| Key | Value |
|---|---|
| `"columns"`, `"rows"` | characters per line (1–40) and lines (1–2) |
| `"controller"` | shown as is, e.g. `"ST7066U"` |
| `"rom"` | `"A00"`, the only character ROM the model has |
| `"panel"` | `"yellow-green"` or `"blue-negative"` |
| `"bus"` | `{"rs", "rw", "e", "data"}`: pin names; `data` is 8 names, DB0 first. Each names exactly one pin |
| `"contrast"` | the V0 pin's name (one pin); the model ignores it |
| `"backlight"` | optional `{"anode", "cathode", "color"}`: pin names (one pin each) and the shown color |
| `"timing"` | ns: `tc tpw tas tah tdsw th tddr tdhr`, every key required, each ≥ 0; `tc` and `tpw` > 0 |
| `"exec"` | ns: `instruction` (every instruction but Clear and Home), `data` (a data read or write), `clearHome`, `powerOn` (busy time after power-up), each > 0 |
| `"vcc"`, `"inputMax"` | the supply range (§ Supply voltage); also allowed on the part |

Part keys: `"vcc"`, `"inputMax"` (optional; replace the device's).

Loader errors (exact text, after the usual path prefix):

- `columns` outside 1–40 → `"columns" must be 1 to 40`;
- `rows` outside 1–2 → `"rows" must be 1 or 2`;
- `rom` other than A00 → `unknown rom "A02"`;
- `panel` other than the two → `"panel" must be yellow-green or blue-negative`;
- `bus.data` not 8 names → `"data" must be 8 pin names, DB0 first`;
- a pin name with no or several pins → the existing `kind_pin` messages;
- `exec` value ≤ 0 → `"exec.powerOn" must be more than 0`;
- `tc` or `tpw` ≤ 0 → `"timing.tpw" must be more than 0`;
- missing `vcc` at both levels → the existing `kind_part_voltage` message.

`docs/parts-format.md` gains a § `charlcd`. It has the key table above, the
behavior summary of §4.3–4.5 and the example device of plan-11-C §4.1. The
file table at the top gains `charlcd.json`, and § Supply voltage adds
charlcd devices and parts to where `vcc` and `inputMax` go.

### 4.3 The bus

Pins: RS, R/W and E are inputs; DB0–7 are inout; V0 and the backlight pins
are inputs. Signal rules, as for the sequential kinds: **Z reads as X on
every input.** Times are the device's `timing` and `exec` (ps after
`kind_ns`).

- **Access start, E rising** (E moves into 1 from anything else):
  - record the rise time, RS and R/W, and whether RS or R/W changed less
    than `tas` before it (a setup violation);
  - `tc` check: less than `tc` since the previous E rise is a violation;
  - R/W = 1 (read): drive DB X now; then at + `tddr` the read data
    (§4.4), or X when the read is violated, unknown or the controller is
    LOST;
  - R/W X: drive DB X now, and treat the access as unknown (§4.5).
- **Access end, E falling:**
  - `tpw` check: E high less than `tpw` is a violation;
  - a write (R/W = 0 at the rise and now): if any DB pin the current mode
    reads changed less than `tdsw` before the fall, it is a violation. The
    byte or nibble is sampled now, and so are RS and R/W;
  - a read: DB goes to Z at + `tdhr`.
- **Hold window.** A write takes effect at E fall + max(`th`, `tah`). It
  becomes a violated write if, inside that window, a sampled DB pin
  changes within `th` of the fall, or RS or R/W changes within `tah`.
  Then §4.5 applies.
- **E reads X** (X or Z) at any time: the controller becomes LOST (§4.5)
  and any read drive goes X until its normal Z time.
- **4-bit mode** (DL = 0): only DB4–7 are read and driven; DB0–3 stay Z.
  A byte is two accesses, high nibble first. A nibble phase (high or low)
  says which comes next:
  - a write's two nibbles form one byte, which then takes effect;
  - a read returns the high nibble of the byte, then the low one. AC moves
    after the second nibble of a data read.
  - At power-up and after any Function Set, the phase is high.
- **Busy.**
  - A write (instruction or data, the whole byte in 4-bit mode) that takes
    effect sets BF = 1 for its exec time: `clearHome` for Clear and Home,
    `instruction` for other instructions, `data` for data.
  - While BF = 1, a write is not accepted: no state changes, and in
    4-bit mode the nibble phase does not move.
  - Reads always work. A busy-flag read returns BF; a data read while
    busy returns X.
- **Power-up.** From the start for `exec.powerOn`, BF = 1 (the internal
  reset). The power-up state is in place from time 0:
  - DDRAM all 20 (space), CGRAM all X;
  - DL = 1, N = 0, F = 0, D = 0, C = 0, B = 0, I/D = 1, S = 0;
  - AC = 0 in DDRAM, shift 0, nibble phase high.
  These are the HD44780's internal-reset values; the modules' datasheets
  call for the same wait.

### 4.4 Instructions and data

RS = 0 and R/W = 0: an instruction byte. Its highest 1 bit decides which
instruction it is:

| Byte | Instruction | Effect |
|---|---|---|
| `0000 0001` | Clear display | DDRAM all 20; AC = 0 (DDRAM); shift = 0; I/D = 1 |
| `0000 001x` | Return home | AC = 0 (DDRAM); shift = 0 |
| `0000 01 I/D S` | Entry mode set | I/D, S |
| `0000 1 D C B` | Display on/off | D, C, B |
| `0001 S/C R/L xx` | Cursor/display shift | S/C = 1: shift ± 1 (R/L = 1 right: shift − 1; left: + 1); S/C = 0: AC ± 1 (R/L) |
| `001 DL N F xx` | Function set | DL, N, F; nibble phase high |
| `01 aaaaaa` | Set CGRAM address | AC = a (6 bits), RAM = CGRAM |
| `1 aaaaaaa` | Set DDRAM address | AC = a (7 bits), RAM = DDRAM |
| `0000 0000` | (none) | no effect, no busy time |

The other three bus states:

- **RS = 1, R/W = 0: write data.** RAM[AC] = byte; then AC moves (I/D = 1:
  + 1, 0: − 1). For a DDRAM write with S = 1, the display shifts too
  (I/D = 1: shift + 1, I/D = 0: shift − 1).
- **RS = 1, R/W = 1: read data.** Returns RAM[AC]; AC moves as for a write,
  at the end of the read (E fall; in 4-bit mode, the second nibble's fall).
  The display does not shift.
- **RS = 0, R/W = 1: read BF and AC.** DB7 = BF; DB6–0 = AC (7 bits; DB6 is
  0 for a CGRAM address).

**Addresses.**

- DDRAM has 80 cells, at physical index 0–79.
  - 2-line mode (N = 1): addresses 00–27 are indices 0–39, and 40–67 are
    40–79. AC + 1 wraps 27 → 40 and 67 → 00; AC − 1 wraps 00 → 67 and
    40 → 27.
  - 1-line mode (N = 0): addresses 00–4F are indices 0–79; AC wraps
    4F ↔ 00.
  - Set DDRAM Address to an address with no cell (28–3F or 68–7F in 2-line
    mode; 50–7F in 1-line mode): AC holds it. A data write there changes
    nothing, a data read returns X, and AC still moves by the wrap rules
    (an address with no cell + 1 is the next address, mod 128).
- CGRAM has 64 cells (6-bit AC, wrapping 3F ↔ 00); each cell's low 5 bits
  are one dot row.
- The shift counts modulo 40 in 2-line mode and 80 in 1-line mode.

### 4.5 X, violations and LOST

**The rule:** an access whose effect is unknown makes everything it could
change X. Registers are tracked one field at a time, each known or X. The
fields are DL, N, F, D, C, B, I/D, S, AC, RAM select (DDRAM or CGRAM),
shift and nibble phase; BF is known or X too.

- **Data write, everything known but some data bits X**: those bits of the
  cell are X. AC moves.
- **Data write with a timing violation**: the cell at AC becomes X (all 8
  bits). AC moves.
- **Data write with AC or RAM select X**: every cell of the RAM it could
  have hit becomes X (both RAMs when the select is X). AC becomes X, and
  the shift becomes X if S is 1 or X.
- **Data read with AC X**: drives X; AC stays X.
- **Instruction write with some bits X but the instruction known** (the
  highest 1 bit is known and every bit above it is a known 0): only the
  fields with X bits become X, e.g. `0000 1 1 X 0` makes C X. Shifts or
  AC moves with an X direction make their target X.
- **LOST.** Any of these makes every register field X, every DDRAM and
  CGRAM cell X, and BF X:
  - an instruction write whose instruction is not known;
  - an instruction write with a timing violation;
  - RS or R/W X at a write;
  - an unknown access (R/W X at E rise);
  - E reading X.
  "LOST" is not a separate mode: it is all fields X. While BF is X, every
  write is taken (it might have been accepted).
- **Leaving LOST** happens one field at a time: each instruction with every
  bit known sets its own fields known. Clear, Home and the address sets
  make AC (and the shift) known again. A data write makes its cell known
  when its address is known.
- **Resync.** When DL or the nibble phase is X, a write with RS = 0 and
  DB7–4 = 0011 is Function Set with DL = 1: 8-bit mode, phase high, N and
  F from DB3 and DB2 (X if X). This is the datasheet's "initializing by
  instruction" sequence (three `0x3` writes). It is the only way to know
  the nibble phase again, as on the real chip.
- **BF X** (after LOST) becomes known when the next write's exec time ends.
  A BF/AC read while BF is X drives X on DB7.

### 4.6 Voltage and the backlight

- `vcc` / `inputMax` work as on other kinds: VDD is a `power` pin, VSS a
  `ground` pin, and the other pins are inputs or inouts checked against
  input max.
- `led_pins` (`models/model_pins.h`) answers for `charlcd` parts with a
  backlight: anode and cathode.
- `voltage.c:208`: the LED branch runs for `PART_LED` and `PART_CHARLCD`
  (when `led_pins` answers). It `continue`s only for `PART_LED`, so a
  charlcd's `vcc` checks still run.
- **The light.** A device with `backlight` has one light, on while the
  anode reads 1 and the cathode reads 0 (the `led` rule), tracked with
  `lights.c`. `lights` and `light_time` hooks report it. A device without
  a backlight has no light hooks (NULL).

### 4.7 State read-back (`lcd_state.h`, `circuit_lcd_state`)

New public header `backend/include/lcd_state.h`, included by `circuit.h`:

```c
#define LCD_STATE_MAX_COLUMNS 40
#define LCD_STATE_MAX_ROWS 2
#define LCD_STATE_CELL_WIDTH 5
#define LCD_STATE_CELL_HEIGHT 8
enum { LCD_DOT_OFF = 0, LCD_DOT_ON = 1, LCD_DOT_X = 2 };

typedef struct LcdState {
    unsigned columns, rows;           /* the part's, from PartCharLcd */
    int busy;                         /* 1, 0, or -1 for X */
    uint8_t ddram[80], ddram_known[80];   /* by physical index (§4.4); known: 1 bit per defined bit */
    uint8_t cgram[64], cgram_known[64];
    /* dots[y][x]: y = row * 8 + dot row, x = column * 5 + dot column */
    uint8_t dots[LCD_STATE_MAX_ROWS * LCD_STATE_CELL_HEIGHT]
                [LCD_STATE_MAX_COLUMNS * LCD_STATE_CELL_WIDTH];
} LcdState;
```

- `sim.h` `SimComponentType` gains, appended after `cpu_state`:
  `int (*lcd_state)(const Sim *sim, const void *state, struct LcdState *out);`
  It takes the `Sim` for the blink phase (`sim_now`). Every existing
  initializer leaves it 0 (C zero-fills trailing members).
- `circuit.h`:
  `int circuit_lcd_state(const Circuit *circuit, size_t part, LcdState *out);`
  It returns 0, or −1 with circuit_error `part N is not a character LCD`
  (copied from `circuit_cpu_state`, `circuit.c:491-506`).

**The image.** Each cell is 5 × 8 dots; dot row 7 is the cursor line.
Dots outside `columns` × `rows` are `LCD_DOT_OFF`. A dot is `LCD_DOT_X`
when anything it depends on is X.

- D = 0: every dot off.
- D = 1, N = 1: row r, column c shows DDRAM address
  `r * 0x40 + (shift + c) mod 40`.
- D = 1, N = 0: row 0 shows address `(shift + c) mod 80`; row 1 is off.
- Character code k:
  - 00–0F: the CGRAM glyph `k & 7`, cells `(k & 7) * 8 + y`, low 5 bits
    (bit 4 = left dot);
  - 10–FF: ROM A00 glyph k, row y (§5).
  - An X bit in k makes every dot of the cell X; X bits in a CGRAM row
    make those dots X.
- **Cursor**: C = 1, RAM = DDRAM, and AC's address shown in a cell: that
  cell's dot row 7 is all on.
- **Blink**: B = 1 and AC's cell is shown. While the blink phase is on,
  all 40 dots of the cell are on. The phase is on while
  `floor(sim_now / 379 259 259 ps)` is even. That is 409.6 ms × 250 kHz /
  270 kHz: the HD44780 blink interval scaled to the modules' fOSC.
- The image is the controller's output: it does not depend on the
  backlight or on V0. The app dims the face by the backlight (plan-11-D).

### 4.8 Where the code goes

- `backend/src/models/charlcd.c`: `PartKindOps CHARLCD_KIND` (loader) and
  the engine model.
- `backend/src/models/charlcd_font.c` + `charlcd_font.h`:
  `const uint8_t CHARLCD_ROM_A00[256][8]`.
- `backend/src/library/kinds.c`: `&CHARLCD_KIND` after `&CPU_KIND`.
- `tests/support/listing.c`:
  - `kind_name`: `PART_CHARLCD` → `"charlcd"`;
  - facts: `columns=16 rows=2 controller=ST7066U rom=A00 panel=yellow-green backlight=Yellow-green bus=4,5,6,7,8,9,10,11,12,13,14`
    (`backlight=none` without one; `bus` lists designators, as the 7seg
    `segmentPins` fact does).

## 5. Character ROM A00

### 5.1 Content

- `CHARLCD_ROM_A00[k][y]` (k = 0–255, y = 0–7): the 5 dots of row y,
  bit 4 = left. Row 7 is 0 for every glyph except FF (all 1s) and the
  eight 5×10 glyphs whose descender starts in row 7: E2 E4 E6 E7 EA F0 F1
  F9 (see Corrections).
- Codes 00–0F are never read from the ROM (they select CGRAM); their rows
  are 0.
- 10–1F, 80–A0 are blank (all 0), as in the tables.
- The 191 glyphs 20–7F and A1–FF are transcribed from the font table on
  PDF page 10 of `NHD-0216K1Z-NSW-BBW-L.pdf`. Render it with
  `pdftoppm -png -r 300 -f 10 -l 10 <pdf> /tmp/p11-font/p` and work in a
  scratch directory.
- The table draws every cell as a 5 × 11 grid, the HD44780 A00 layout with
  5×10 glyphs in E0–FF. Rows 0–7 of each cell, as drawn, are the glyph's
  8 rows. Anything below row 7 (the descenders of E0–FF, and the bottom of
  the FF block) is dropped, since a 5×8 controller reads 8 rows. This was
  verified 2026-11-01 on the 300 dpi render: `0` and `A` use rows 0–6, and
  FF fills every row.
- `charlcd_font.c` starts with a comment naming the source page.

### 5.2 Checks

- **Spot checks** in `charlcd_test`, each exact (rows top to bottom, hex).
  Each was read off the 300 dpi render of PDF page 10 on 2026-11-01:
  - `A` (41): `0E 11 11 11 1F 11 11 00`;
  - `0` (30): `0E 11 13 15 19 11 0E 00`;
  - `¥` (5C): `11 0A 1F 04 1F 04 04 00`;
  - `→` (7E): `00 04 02 1F 02 04 00 00`;
  - `←` (7F): `00 04 08 1F 08 04 00 00`;
  - space (20): all 0; FF: all `1F`.
- **The glyph sheet.** `charlcd_test --font` prints all 256 glyphs as a
  16 × 16 grid of `#`/`.` art: rows are the code's high nibble, columns the
  low nibble, the same layout as the datasheet table. Compare it with the
  rendered page, one row of 16 glyphs at a time. Record "sheet matches,
  0 differences" in the commit message of Phase 2; fix any difference
  before committing.

## 6. Tests (`backend/tests/charlcd_test.c`)

The test is registered with `logic_test(charlcd_test)`. It uses an inline
fixture `charlcd` device copied from the NHD-0216K1Z-NSW-BBW-L facts
(plan-11-C §4.1): 16×2, the NHD timing and exec values, and a fixture
footprint. It drives the fixture through `tests/support/bench`. Bus helper:
`write(rs, byte)` and `read(rs)` run one access with E high 500 ns, every
setup and hold met, and 2 µs between accesses. In 4-bit mode they do two
nibble accesses.

Cases (each an `expect` with its own message):

1. **Power-up.** At 1 ms a BF read gives DB7 = 1; at 41 ms DB7 = 0 and
   AC = 0. A write at 1 ms is not accepted: DDRAM stays all 20.
2. **8-bit init.** The NHD 8-bit example (PDF p.11), starting 100 ms after
   power-up: 30, wait 5 ms, 30, wait 160 µs, 30, wait 160 µs, then 38, 10,
   0C, 06. The example sends the last four back to back on its slow port;
   the test leaves 40 µs between them, since the model does not accept a
   write while busy.
   - After the sequence AC = 67: the example's 10 moves the cursor left
     from 00, and that wraps in 2-line mode.
   - Then 01 (Clear), wait 2 ms, then `HELLO` as data: `ddram[0..4]` =
     48 45 4C 4C 4F, AC = 05, the dots of cell 0 equal ROM 48, and row 1
     is all off (spaces).
3. **4-bit init.** The NHD 4-bit example (PDF p.12): 100 ms, then the
   nibble 3 three times (5 ms, 160 µs, 160 µs apart), then the nibble 2,
   then 28, 10, 0F, 06 as nibble pairs, high nibble first, with 40 µs
   between bytes. Then 01, wait 2 ms, data `Hi`, and a BF/AC read in two
   nibbles:
   - BF 0 and AC 02, read as 0 then 2;
   - DB0–3 stay Z through every read;
   - 0F turned on the blinking cursor, so cell 2 blinks (case 10's rule).
4. **Busy.** A data write 10 µs after another (inside 37 µs) is not
   accepted. Clear sets BF for 1.52 ms: BF is 1 at + 1.5 ms and 0 at
   + 1.53 ms.
5. **Clear and home.** After Clear, DDRAM is all 20, AC 0, I/D 1. After a
   shift, Home brings the shift back to 0 and DDRAM is unchanged.
6. **Entry mode and shift.** I/D = 0 writes go leftward. S = 1 shifts the
   display on each DDRAM write: after 3 writes the image's column 0 shows
   address 03.
7. **2-line wrap.** Set DDRAM 27, write 2 bytes: they land at indices 39
   and 40 (addresses 27 and 40). Decrement from 00 lands at 67.
8. **No-cell address.** Set DDRAM 28 (2-line), write: no cell changes;
   AC = 29; a data read there drives X.
9. **CGRAM.** Set CGRAM 00 and write 8 rows. Codes 00 and 08 both render
   that glyph; a CGRAM row with an X bit renders that dot X.
10. **Cursor and blink.** C = 1: cell AC's row 7 is on. B = 1: all dots of
    the cell are on at t where `floor(t / 379259259 ps)` is even, and the
    glyph shows when it is odd.
11. **Display off.** D = 0: every dot off, DDRAM kept.
12. **1-line mode.** N = 0: row 0 shows 16 cells from address 00; row 1 off;
    AC wraps 4F → 00.
13. **Read timing.** DB is X from E rise until + 100 ns, the data from then
    until E fall + 10 ns, and Z after.
14. **Write violations.**
    - E high 100 ns (< 140) on a data write: the cell is X and its dots are
      X; AC moves.
    - Data changing 20 ns before E fall (< 40): the cell is X.
    - Data changing 5 ns after E fall (< 10): the cell is X.
    - E rising 1000 ns after the last rise (< 1200): violated.
15. **LOST and resync.**
    - An instruction write with RS X makes every field X: every dot X, BF
      read X.
    - Then the 8-bit init sequence brings every field back: the image is
      blank (DDRAM X cells show X until written or cleared), and after
      Clear every dot is off and `HELLO` shows.
16. **Partial X instruction.** `0000 11X0` makes only C X: the cursor cell's
    row 7 is X and every other dot is right.
17. **Backlight.** With anode 1 and cathode 0, `circuit_part_lights` gives
    1 light, on; anode 0 gives off. A fixture without `backlight` gives 0
    lights.
18. **Voltage.**
    - VDD on `3V3` is refused at `circuit_start` with
      `voltage: 3.3 V on pin 2 (VDD) of <fixture>; it needs 4.5-5.5 V`.
    - On `5V` it starts.
    - Backlight anode on `5V` and cathode on `GND` starts (no smoke, no
      voltage problem).
19. **API.** `circuit_lcd_state` on a gate part gives −1 and
    `part 0 is not a character LCD`.
20. **Loader errors.** Every message of §4.2 for an inline bad device, and
    the fixture loads.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the
> same commit as the work it describes; `- [~]` for partial with one line of
> what remains; mark moot tasks `- [x] ~~text~~ — moot: <evidence>`; fill
> each `Commit:` line when the phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Kind, loader and facts

Part facts load; the model is a stub that drives nothing (DB Z), and BF is
not yet readable.

- [x] `backend/include/parts.h`: `PART_CHARLCD`, `PartCharLcd`, `Part.charlcd` (§4.1).
- [x] `backend/src/models/charlcd.c`: `CHARLCD_KIND` with device keys,
      `load_device`, `bind_pinout`, `load_part` (§4.2); a model with
      `pin_count`, `create`, `destroy` and no-op `start`/`pin_changed`.
- [x] `backend/src/library/kinds.c`: `&CHARLCD_KIND` after `&CPU_KIND`.
- [x] `backend/tests/support/listing.c`: kind name and facts (§4.8).
- [x] `backend/tests/charlcd_test.c` + `logic_test(charlcd_test)` in
      `backend/CMakeLists.txt`: the fixture device and case 20 (loader errors).
      Also `src/models/charlcd.c` in the `logic` library's source list (the
      CMake list is explicit). Ran: `charlcd_test` → 18 PASS lines for case 20
      (facts, bus pins, vcc, no-backlight device, and 13 bad devices), `all passed`.
- [x] `docs/parts-format.md`: the § `charlcd` key table (§4.2), the file
      table row, § Supply voltage. `docs/backend.md` § Layout: `charlcd.c`,
      `charlcd_font.c`.

Acceptance: the fixture loads with the right facts and every bad device is
rejected with its message; the goldens are unchanged.
  Check: `cmake --build /tmp/logic-p11 -j8 && ctest --test-dir /tmp/logic-p11` → `out of 18`, 100% passed; `git diff --exit-code backend/tests/golden` → 0 (est. 1 min).
  Ran: `100% tests passed, 0 tests failed out of 18`; golden diff → exit 0.
Commit: 70ff814

### Phase 2 — Character ROM

- [x] `backend/src/models/charlcd_font.{c,h}`: `CHARLCD_ROM_A00` (§5.1).
      Transcribed by sampling every dot centre of the 300 dpi render
      (`/tmp/p11-scripts/font_extract.py`: cell lines found from the table
      rules, dots on a 14.6 px pitch); 0 dots in rows 0–7 fell in the
      ambiguous 70–170 grey band. 189 non-blank glyphs (FE is blank in the
      table). Added to the `logic` source list in CMake.
- [x] `charlcd_test`: the spot checks (§5.2) and the `--font` sheet printer.
      Ran: 7 spot-check PASS lines plus `rom: 00-1F and 80-A0 are blank`.
- [x] Compare the sheet with PDF page 10 at 300 dpi, row by row (§5.2).
      The `--font` sheet was redrawn in the table's layout next to crops of
      the page (columns 2–5, 6–7, A–C, D–F; 0, 1, 8, 9 are blank and checked
      by the test) and looked at row by row: 16 rows, 0 differences.

Acceptance: spot checks pass, and the sheet matches the datasheet.
  Ran: `charlcd_test` → `all passed`; sheet compared, 0 differences.
  Check: `/tmp/logic-p11/charlcd_test` → passes; `/tmp/logic-p11/charlcd_test --font` printed and compared with `/tmp/p11-font/p-10.png`, 16 rows, 0 differences recorded in the commit message (est. 20 min of looking; a dot-level transcription can only be checked by looking, and the spot checks cover only 7 glyphs).
Commit: a1425c1

### Phase 3 — Controller: bus, instructions, busy, power-up

- [x] `charlcd.c`: §4.3 (without the violation and X rules of §4.5),
      §4.4, power-up; `lcd_state` hook and image (§4.7).
      The §4.5 code (timing checks, X fields, LOST, resync) landed in the
      same commit, since the field-by-field X tracking is the register
      representation; its cases (14–16) are Phase 4 (see Corrections).
- [x] `backend/include/lcd_state.h`, `circuit.h` (`circuit_lcd_state`),
      `circuit.c`, `engine/sim.h` (the hook) (§4.7).
- [x] `charlcd_test` cases 1–13 and 19.
      The bench gained `bench_sim` and `bench_state` so the test can call
      the `lcd_state` hook. Ran: 41 PASS lines for cases 1–13 and 19
      (both init examples, busy, clear/home, entry mode, wrap, no-cell,
      CGRAM, cursor/blink, display off, 1-line, read timing, API).

Acceptance: both datasheet init sequences drive the fixture to `HELLO` / `Hi`,
and every instruction case passes.
  Check: `ctest --test-dir /tmp/logic-p11 -R charlcd_test` → passes (est. 30 s).
  Ran: full ctest → `100% tests passed, 0 tests failed out of 18`; goldens
  unchanged.
Commit: 96ce450

### Phase 4 — Violations, X and LOST

- [x] `charlcd.c`: the timing checks of §4.3 (setup, `tc`, `tpw`, `tdsw`,
      the hold window) and §4.5. (In 96ce450, see Corrections.)
- [x] `charlcd_test` cases 14–16.
      Ran: 12 PASS lines: tpw, tdsw, th, tah and tc violations; LOST by RS X
      and resync to HELLO; LOST by E X; E X during power-on ignored; the
      partial-X instruction `0000 11X0`.

Acceptance: every violation and X case gives the stated cells and dots, and
cases 1–13 still pass.
  Check: `ctest --test-dir /tmp/logic-p11 -R charlcd_test` → passes (est. 30 s).
  Ran: `charlcd_test` → `all passed` (cases 1–16, 19, 20).
Commit: 2dd9dae

### Phase 5 — Backlight and voltage

- [x] `charlcd.c`: the light (`lights`, `light_time`) for devices with a
      backlight (§4.6). The hooks landed in 96ce450; a device without a
      backlight answers 0 lights (see Corrections).
- [x] `models/model_pins.h` / `charlcd.c`: `led_pins` answers for charlcd.
      (`led_pins` in `led.c` forwards to `charlcd_backlight_pins`.)
- [x] `backend/src/voltage.c:208`: the LED branch for both kinds; `continue`
      only for `PART_LED` (§4.6).
- [x] `charlcd_test` cases 17–18.
      The fixture library also loads the real `supply.json` (5V, 3V3, GND).
      Ran: 6 PASS lines, including the exact message
      `voltage: 3.3 V on pin 2 (VDD) of FX-LCD; it needs 4.5-5.5 V`.
- [x] `docs/parts-format.md` § `charlcd`: behavior summary (§4.3–4.7);
      `docs/backend.md` § Using it: `circuit_lcd_state` and the
      backlight light.

Acceptance: the backlight lights, the VDD range is enforced, and nothing
else in the voltage check changed.
  Check: `ctest --test-dir /tmp/logic-p11` → `out of 18`, 100% passed (voltage_test unchanged); `git diff --exit-code backend/tests/golden` → 0 (est. 1 min).
  Ran: `100% tests passed, 0 tests failed out of 18` (voltage_test.c not
  modified); golden diff → exit 0; final gate app build →
  `** BUILD SUCCEEDED **`.
Commit: bab0b68

## Validation Plan

- Tests: `charlcd_test` cases 1–20 (§6) and the ROM checks (§5.2); the
  negative paths are cases 4, 8, 14–16, 18 and 20.
- Coverage check: the kind has no catalog part yet, so the goldens cannot
  see it. `charlcd_test` is the only test of every line here, and each
  phase's cases run before it lands.
- Runtime proof: plan-11-C §4.6 (a real CPU board) and plan-11-D (the app).
- Doc sync: `docs/parts-format.md` § `charlcd`, file table, § Supply
  voltage; `docs/backend.md` § Layout, § Using it.
- Final gate (once, after Phase 5): `cmake --build /tmp/logic-p11 -j8 &&
  ctest --test-dir /tmp/logic-p11` → 18 of 18;
  `git diff --exit-code backend/tests/golden` → 0. Then the app build
  (plan-11-A Phase 3 command) → `** BUILD SUCCEEDED **` (est. 5 min).

## Decisions

- Behavior follows the HD44780 datasheet family as the module datasheets
  print it: their timing, exec times and init flows (author, from the
  datasheets; § 2).
- A write while busy is not accepted (HD44780 datasheet text) (author). (§4.3)
- An unknown or violated write makes everything it could change X, and
  `0011` with RS = 0 resyncs (the datasheet's init-by-instruction) (author;
  the repo's signal rule). (§4.5)
- The backend renders the dot image; the app paints it (author). (§3)
- V0 is a plain input the model ignores (author: the simulator is logic-only
  and the catalog has no resistors or potentiometers). (§4.6)
- No RAM window for the LCD in the app (owner, 2026-11-01); `LcdState`
  still carries RAM for the tests. (§4.7)

## Corrections

- **E or R/W X during the power-on reset does not make the controller
  LOST.** §4.3/§4.5 say E reading X at any time is LOST. Before the host
  drives E (a bench at time 0, or the CPU board of plan-11-C §4.6, where E
  comes from an HC11 whose inputs are X until the CPU clocks), E reads X.
  LOST then would make BF X, the 6502's first `BIT $4000` would read X,
  and the CPU would go LOST, so §4.6's HELLO could never print. The
  internal reset ignores the bus anyway (no write is accepted while
  BF = 1). Rule as built: E X, or an unknown access (R/W X at E rise),
  before `exec.powerOn` only drives the bus X; after it, LOST as written.
- **A write while BF = 1 is ignored before any LOST check.** A write with
  RS X or a timing violation that arrives while busy is "not accepted"
  (§4.3), so it changes nothing, rather than making the controller LOST.
- **A device without a backlight has light hooks that answer 0.** §4.6
  says such a device has no light hooks (NULL). A kind has one
  `SimComponentType`, so the hooks are shared and return 0 lights when
  the module has no backlight. `circuit_part_lights` and
  `circuit_part_light_time(…, NULL, 0)` both give 0, which is what the
  app's light-part test reads (case 17 checks both).
- **The §4.5 code landed with Phase 3.** Registers are stored as
  known-or-X fields from the start, so the X rules are part of the
  representation. Phase 4 adds cases 14–16, which exercise them.

- **§5.1 "Row 7 is 0 for every glyph except FF" is false.** The table's
  E0–FF cells are 5×10 drawings; with rows 0–7 taken as drawn (the rule
  §5.1 also states), eight glyphs have dots in row 7 because their
  descender starts there: E2 β (`10`), E4 μ (`10`), E6 ρ (`10`), E7 g
  (`01`), EA j (`02`), F0 p (`10`), F1 q (`01`), F9 y (`01`). Measured
  with `font_extract.py` on the 300 dpi render. The "as drawn" rule is
  kept, matching a real 5×8 HD44780, which shows the top 8 rows of these
  glyphs. Nothing else rested on the row-7 claim (the cursor ORs row 7).
- **§5.1 "5 × 11 grid".** The table draws 5 × 10 cells: a sample at row
  index 10 lands on the cell's lower rule in every cell. Rows 0–7 are
  unaffected.
- **§2 "191 glyphs"** counts cells (20–7F, A1–FF). Two of them are blank
  in the table: 20 (space) and FE. So 189 glyphs carry dots.

## Summary

The risk is the controller model's X and LOST rules and the 4-bit
phase, and every one of them sits behind a named test case. The ROM is a
transcription; spot checks plus a side-by-side sheet check guard it. No
catalog or app behavior changes in B.
