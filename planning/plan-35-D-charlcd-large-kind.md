# plan-35-D: The `charlcd` kind — four lines, two controllers, RGB backlights, gray and white panels

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-35-C

D extends the `charlcd` kind (plan-11-B) so it can describe and simulate
the 67 new modules:

- **4-line displays on one controller** (20 × 4): the HD44780 4-line
  DDRAM map;
- **two controllers** (40 × 4): `bus.e` names two enable pins, E1 for
  lines 1–2 and E2 for lines 3–4, each with its own controller state;
- **backlights with up to three LEDs** (the Midas RGB modules): one light
  per LED, and the voltage check carries each anode to its cathode;
- **panels `gray` and `white`** (STN gray positive, FSTN positive).

D adds no catalog part (E and F do), so every case runs on inline fixture
devices in `backend/tests/charlcd_test.c`, as in plan-11-B.

Outcome of D: `ctest` passes T0 programs (`charlcd_test` grows, no new
program); goldens byte-identical.

References:

- plan-35-A § Prerequisites, § Decisions.
- `planning/completed/plan-11-B-charlcd-kind.md` (the model this extends:
  §4.3 bus, §4.4 instructions and addresses, §4.5 X rules, §4.7 image).
- `docs/parts-format.md` § `charlcd` (line 2649), `docs/backend.md`
  (line 152, `circuit_lcd_state`).
- `datasheets/module-disp-char/NHD-0420DZ-FL-GBW.pdf` (the 20 × 4 DDRAM
  address table: line 1 00–13, line 2 40–53, line 3 14–27, line 4 54–67);
  `NHD-0440WH-ATFH-JT.pdf` (pin 9 E1 "for top 2 lines", pin 15 E2 "for
  bottom 2 lines"; example code driving E1 and E2 one at a time);
  `MD44005A6W-FPTLRGB.pdf` (pins 17 LED+, 18 K(B), 19 K(G), 20 K(R)).

## Prerequisites

The gate is plan-35-A § Prerequisites. D adds:

| Must be true | Command | Status |
|---|---|---|
| plan-35-C complete | `grep -c '^Commit: —' planning/plan-35-C-charlcd-large-footprints-midas-nhd.md` → 0 (or in `planning/completed/`) | NOT MET (2026-11-11 at a5b0fcf: → 2; C is being worked in parallel. D adds no part and no footprint, so it touches nothing C writes; D was started on the session owner's instruction with this row open) |
| The kind is plan-11's | `grep -n 'must be 1 or 2' backend/src/models/charlcd.c` → one line | MET at the start of D (2026-11-11 at a5b0fcf: charlcd.c:120; see Corrections for the command). D Phase 1 replaces the message, so the command now → 0 lines by design |
| Tests pass before D | `cmake -S backend -B /tmp/logic-p35 && cmake --build /tmp/logic-p35 -j8 && ctest --test-dir /tmp/logic-p35` → `100% tests passed` | MET (2026-11-11 at a5b0fcf, untouched tree: `100% tests passed, 0 tests failed out of 30`; `charlcd_test`: 100 PASS, 0 FAIL) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `ctest --test-dir /tmp/logic-p35` → `100% tests passed`, T0 programs.
- `git diff --exit-code backend/tests/golden` → exit 0.
- `/tmp/logic-p35/charlcd_test | grep -c '^PASS'` grows by the cases of § 5
  and `grep -c '^FAIL'` → 0.

### Non-goals (explicit constraints)

- The five 16 × 2 parts behave exactly as before. Their listing lines,
  trace hashes and every existing `charlcd_test` case are unchanged (the
  existing cases are only edited where `LcdState` fields become arrays, § 4.5,
  and where an existing loader case expects one of the two messages § 4.2
  replaces: `"rows" must be …` and `"panel" must be …`; see Corrections).
- `PartCharLcd` keeps every existing field and offset; new fields are
  appended. `bus_pins` keeps its order for one-enable parts.
- Not modeled (as in plan-11-B): contrast, the 5×10 font, other ROMs,
  edge rates, serial interfaces. The FGND (frame ground) pin of C204HLB is
  a plain `ground` pin.
- No app change (G), no part (E/F).

## 2. Current State

- **Loader** (`backend/src/models/charlcd.c`):
  - `charlcd_load_device` (`charlcd.c:97`) takes `rows` 1–2 (line 119:
    `"rows" must be 1 or 2`) and `panel` `yellow-green` / `blue-negative`
    (line 136);
  - `bus.e` is one string (line 152);
  - `backlight` is one object (lines 171–180, `BACKLIGHT_KEYS` = anode,
    cathode, color).
  - `charlcd_bind_pinout` (line 212) binds RS, R/W, E, DB0–7, V0 and the
    backlight pins into `CharLcdPins`, and fills `info.bus_pins` (RS,
    R/W, E, DB0–7: 11).
- **Model state** (`CharLcdState`, `charlcd.c:290-329`): one controller.
  Its registers, DDRAM/CGRAM, busy times, access record, read drive and
  pending write sit next to the shared input history (E, RS/R/W/DB change
  times) and one `Light`.
- **Image** (`image_dot`, `charlcd.c:983`): N = 1 gives row r address
  `r * 0x40 + (shift + c) mod 40`; N = 0 gives row 0
  `(shift + c) mod 80` and row 1 off.
- **State API** (`backend/include/lcd_state.h`):
  - `LCD_STATE_MAX_ROWS` is 2;
  - `busy`, `ddram[80]`, `ddram_known`, `cgram[64]` and `cgram_known`
    belong to the one controller;
  - `charlcd_lcd_state` (line 1029) fills them.
- **Lights**:
  - `charlcd_lights` / `charlcd_light_time` (lines 955, 964) report 1
    light when the part has a backlight;
  - `update_light` (line 817) sets light 0 from anode/cathode.
- **Voltage**:
  - `voltage.c:254` handles `PART_CHARLCD` with `led_pins`, which calls
    `charlcd_backlight_pins(part, &anode, &cathode)`
    (`models/model_pins.h:18`, `charlcd.c:263`);
  - one anode→cathode carry per part.
- **Listing** (`backend/tests/support/listing.c:173`) prints `columns rows
  controller rom panel backlight bus=`; `backlight` is
  `PartCharLcd.backlight_color` or `none` (see Corrections).
- **Tests**:
  - `charlcd_test.c` has 1519 lines;
  - 25 lines use `LcdState.ddram`, `.busy` or `.cgram`
    (`grep -n 'ddram\[\|\.busy\|cgram\[' backend/tests/charlcd_test.c | wc -l`
    → 25).

### Measured populations

| What | Count | Command |
|---|---|---|
| New parts with 4 rows, one E (20 × 4) | 41 | `awk -F'\t' '$4=="add" && $3=="20x4"' planning/plan-35-data/candidates.tsv \| wc -l` |
| New parts with two E (40 × 4) | 12 | `… $3=="40x4" …` (every one has pin names E1 and E2: `pinout` p18-dual / p18-dual-nc / p20-rgb-dual) |
| New 40 × 2 parts (already supported: 40 columns, 2 rows) | 14 | `… $3=="40x2" …` |
| New parts with an RGB backlight | 4 | `awk -F'\t' '$4=="add" && $7=="Red,Green,Blue"' … \| wc -l` |
| New parts per panel | yellow-green 20, white 19, blue-negative 15, gray 13 | `awk -F'\t' '$4=="add"{print $6}' … \| sort \| uniq -c` |
| `LcdState` uses in the app | 1 file reads it (`SimulationEngine.swift`: `LCDFrame.init` reads `columns`, `rows`, `dots`; line 249 calls `circuit_lcd_state`); the other hit is a doc comment in `CharacterLCDSymbolView.swift` | `grep -rn 'LcdState' schematical/schematical --include='*.swift'` → 4 lines |

### Verified properties

- **The app reads only `columns`, `rows` and `dots` from `LcdState`**
  (`SimulationEngine.swift:31-52`), so turning `busy`/`ddram`/`cgram` into
  per-controller arrays needs no app change. A larger
  `LCD_STATE_MAX_ROWS` is picked up through the imported constant.
- **No existing part uses `rows` 4, two enables or an array backlight**,
  so the existing five listing lines can stay byte-identical as long as
  one-enable, one-LED facts print as today (§ 4.6).
- **A 20 × 4 on one HD44780 is 2-line mode with 20-column halves.** Line 3
  continues line 1's 40-cell DDRAM line (addresses 14–27), and line 4
  continues line 2's (54–67). Source: the NHD-0420DZ address table above,
  and the same table in `C204ALBSYLY6WT33XAA_Spec.pdf` and
  `C204ABBSYSY6WT55PAB_Spec.pdf` (`grep -l '54'` hits).

## 3. Design Overview

1. **Facts and loader** (§ 4.1–4.2): rows 1, 2 or 4; `bus.e` a name or two
   names; `backlight` an object or an array of 1–3; panels `gray`,
   `white`. These are low risk: loader cases and goldens.
2. **Controllers** (§ 4.3): `CharLcdState` splits into the shared bus and
   `ctl[2]`, one record per controller. Every plan-11-B rule runs per
   controller, unchanged. For a one-enable part only `ctl[0]` exists, and
   it must behave byte-for-byte as today: the existing tests and goldens
   are the gate.
3. **Image** (§ 4.4): the 4-row map.
4. **State API** (§ 4.5): `controllers`, per-controller RAM and busy.
5. **Lights and voltage** (§ 4.6).

**Where the risk is:** step 2. It moves every field of the model's state,
and a slip changes 16 × 2 behavior. It is guarded by the unchanged
`charlcd_test` cases plus the trace goldens (5 charlcd hashes). Both must
pass unchanged before any new case is added (Phase 2 lands the move alone).

**Correctness gate:** behavior tests plus byte-identical goldens (no part
uses the new features yet). A golden diff is a bug in the move; root-cause
it on the one part whose hash changed.

**Rejected alternatives**

- *A new kind for 40 × 4.* Rejected: it is two copies of the same
  controller on one bus; a second kind would duplicate 1000 lines and
  every fix.
- *Modeling the 40 × 4 as one 80-column controller.* Rejected: the two
  controllers have separate busy flags, separate AC and separate init,
  and code that forgets to initialise E2 must show lines 3–4 blank.
- *An RGB backlight as one light with a color mix.* Rejected: the three
  LEDs have separate cathodes and can be driven separately (PWM per
  color). Separate lights let the app mix whatever is lit.

## 4. Detailed Design

### 4.1 Facts (`backend/include/parts.h`)

`PartCharLcd` gains, appended after `bus_pin_count`:

```c
    unsigned enables;                          /* 1, or 2: bus_pins is RS, R/W, E1, E2, DB0-DB7 */
    size_t backlight_count;                    /* 0-3 LEDs */
    const char *const *backlight_colors;       /* backlight_count colors; [0] == backlight_color */
```

`backlight_color` stays: it is the first LED's color, or NULL. The `panel`
comment adds `"gray"` (STN positive: dark dots on light gray) and
`"white"` (FSTN positive: black dots on white).

### 4.2 Part file keys (`docs/parts-format.md` § `charlcd`)

| Key | Change |
|---|---|
| `"rows"` | 1, 2 or 4. Error: `"rows" must be 1, 2 or 4` |
| `"columns"` | 1–40; with 4 rows and one enable, at most 20 (`4 rows on one controller need "columns" of 20 or less`) |
| `"bus".e` | a pin name, or an array of 2 names (E1, E2). Two need `"rows": 4` (`two enables need "rows": 4`); any other array length: `"e" must be a pin name or 2 pin names` |
| `"panel"` | `yellow-green`, `blue-negative`, `gray`, `white`. Error: `"panel" must be yellow-green, blue-negative, gray or white` |
| `"backlight"` | `{"anode", "cathode", "color"}` or an array of 1–3 such objects (`"backlight" must be an object or 1 to 3 objects`). LEDs may share the anode pin; each cathode is its own pin |

With two enables, controller 1 (E1) drives rows 0–1 and controller 2 (E2)
drives rows 2–3. Each is a 2-line controller of `columns` columns.

### 4.3 Two controllers (`charlcd.c`)

- `CharLcdPins` gets `e[2]`, `enables`, and `anode[3]`, `cathode[3]`
  with `leds` (replacing `anode`, `cathode`, `has_backlight`).
- `CharLcdState` keeps the shared parts: pins, timing, RS/R/W/DB change
  times, and `Light light[3]`. Everything per controller moves into
  `CharLcdCtl ctl[2]`:
  - registers and RAM;
  - busy and BF-X times;
  - nibble phase and high nibble;
  - E level, rise times and the access record, with its `seq` (see
    Corrections);
  - read drive;
  - the pending write.
- Every function that took the state now takes `(CharLcdState *, CharLcdCtl *)`,
  or only the `CharLcdCtl *` when it reads nothing shared (busy flag, LOST,
  AC and shift steps, DDRAM index, the byte a read returns, the image dot).
- `charlcd_pin_changed` routes an E pin's change to its controller. RS,
  R/W and DB changes go to both, since each checks its own windows.
- **Reads:** a controller drives DB as today. When both drive at the same
  time (E1 and E2 high on a read), each DB bit where they disagree, or
  either is X, is X. The model never shorts.
- **Timers** carry the controller number in the timer id's top bit.
- **Power-up:** each controller powers up as in plan-11-B §4.3 (BF = 1 for
  `exec.powerOn`).

### 4.4 The image (`image_dot`)

The display row r maps to a controller and a line:

- **two enables:** controller = r / 2, line = r % 2, offset = 0;
- **one enable:** controller 0, line = r % 2, offset = (r / 2) × `columns`
  (0 for 1- and 2-row parts, so they are unchanged).

Then, as today, for that controller:

- **N = 1:** address `line * 0x40 + (shift + offset + c) mod 40`, index
  `line * 40 + (shift + offset + c) mod 40`.
- **N = 0:** a row with line 1 is off; a row with line 0 shows
  `(shift + offset + c) mod 80`.
- **N = X:** take both and keep the dot only where they agree, as today.

The cursor and blink rules are unchanged; they use the row's controller.

### 4.5 State API (`backend/include/lcd_state.h`)

```c
#define LCD_STATE_MAX_ROWS 4
#define LCD_STATE_MAX_CONTROLLERS 2
typedef struct LcdState {
    unsigned columns, rows, controllers;
    int busy[LCD_STATE_MAX_CONTROLLERS];
    uint8_t ddram[LCD_STATE_MAX_CONTROLLERS][80], ddram_known[LCD_STATE_MAX_CONTROLLERS][80];
    uint8_t cgram[LCD_STATE_MAX_CONTROLLERS][64], cgram_known[LCD_STATE_MAX_CONTROLLERS][64];
    uint8_t dots[LCD_STATE_MAX_ROWS * LCD_STATE_CELL_HEIGHT][LCD_STATE_MAX_COLUMNS * LCD_STATE_CELL_WIDTH];
} LcdState;
```

The 25 existing test lines (28, see Corrections) become `[0]` uses. `docs/backend.md:152`
mentions `controllers`.

### 4.6 Lights, voltage, listing

- **Lights.** `charlcd_lights` / `charlcd_light_time` report `leds` lights,
  in backlight-array order. `update_light` sets light i from anode i and
  cathode i, with the LED rule as today.
- **Voltage.**
  - `charlcd_backlight_pins(part, index, &anode, &cathode)` returns 0
    past the last LED.
  - `voltage.c`'s `PART_CHARLCD` branch loops over the indices and carries
    each anode's level to its cathode net, as it does for the one LED today.
  - `led_pins` keeps its LED meaning; the charlcd case leaves it.
- **Listing** (`listing.c:173`):
  - `backlight=` prints the colors joined by `,`, so one LED prints as
    today;
  - ` enables=2` follows `rows=` only when there are two enables;
  - `bus=` lists `bus_pins`, 11 or 12 entries.
  The five existing lines are unchanged.

## 5. Tests (`backend/tests/charlcd_test.c`)

New inline fixtures:

- **`fixture_2004`:** the NHD-0420DZ facts: 20 × 4, p16 pinout, one E,
  timing from plan-11's fixture.
- **`fixture_4004`:** 40 × 4 on the C404ALB p18-dual pinout: E1 pin 9,
  E2 pin 15.
- **`fixture_rgb`:** 20 × 4, p18-rgb: LED+ 15, K(B) 16, K(G) 17, K(R) 18,
  `backlight` an array of three.

Cases:

1. **Loader:** every new message of § 4.2 on an inline bad device; rows 4,
   two enables, panels `gray` / `white`, and a 3-LED backlight all load;
   `PartCharLcd.enables`, `backlight_count`, `backlight_colors` and
   `bus_pin_count` (12) are right.
2. **20 × 4 map:** init in 8-bit mode, Function Set N = 1. Write 80 bytes
   from address 00: the DDRAM wrap runs 27 → 40 and 67 → 00 as in
   plan-11-B. Row 0's cells are bytes 0–19, row 1's bytes 40–59 (address
   40–53), row 2's bytes 20–39 (14–27), row 3's bytes 60–79 (54–67).
3. **20 × 4 shift:** shift the display left once; row 2, column 0 shows
   address 15 and row 0, column 19 shows address 14.
4. **20 × 4, 1-line mode:** N = 0; rows 1 and 3 are off, row 2 shows
   addresses 14–27.
5. **40 × 4:**
   - E1 init + "TOP" shows on row 0, and rows 2–3 stay blank;
   - then E2 init + "LOW" shows on row 2;
   - `LcdState.controllers` is 2, `ddram[1][0]` is 'L', and the two
     `busy[]` values are separate (E2 is busy while E1 is not);
   - E2 never initialised (BF = 1 during its power-on, writes ignored):
     rows 2–3 show spaces.
6. **40 × 4 read clash:** a BF read with E1 and E2 both high drives X on
   DB7 when the busy flags differ.
7. **40 × 4 timing per controller:** an E2 pulse shorter than `tpw` makes
   controller 2 LOST (its fields X) and leaves controller 1 known.
8. **RGB lights:** three lights; K(G) low and the others high gives lights
   0/1/0 (array order R, G, B: the fixture lists R first); light times
   add up per LED.
9. **RGB voltage:** VCC on LED+ and three cathode nets with no other
   driver; the voltage check carries VCC's level to all three (as it does
   for the single backlight).
10. **16 × 2 unchanged:** every existing case passes untouched apart from
    the `[0]` edits and the two replaced loader messages (rows 3, panel
    green).

## Phases

> Tick `- [x]` in the same commit as the work; `- [~]` for partial with one
> line of what remains. Fill each `Commit:` line when the phase lands. **An
> unticked box means NOT DONE.**

### Phase 1 — facts and loader

- [x] `parts.h`: append the three `PartCharLcd` fields (§ 4.1).
- [x] `charlcd.c` loader and binder: § 4.2 keys and messages; `enables`,
      `bus_pins` with E2, the backlight array; `charlcd_backlight_pins`
      with an index; `model_pins.h`, `led.c`, `voltage.c` callers.
      (The model still watches E1 and LED 0 only until Phase 3.)
- [x] `listing.c`: § 4.6 printing.
- [x] `charlcd_test`: case 1 (`check_loader_large`, "D1 loader" lines plus
      14 `expect_error` cases; fixtures FX-2004, FX-4004, FX-RGB and a
      fixture footprint FX18). Check run 2026-11-11:
      `cmake --build /tmp/logic-p35 -j8 && /tmp/logic-p35/charlcd_test | grep -c '^FAIL'`
      → `0` (`grep -c '^PASS'` → 119, was 100);
      `/tmp/logic-p35/listing_check && /tmp/logic-p35/trace_check` → both
      exit 0, each `26729 parts compared, 0 skipped`; also
      `ctest --test-dir /tmp/logic-p35` → `100% tests passed, 0 tests failed out of 30`
      and `git diff --exit-code backend/tests/golden` → exit 0.

Check: `cmake --build /tmp/logic-p35 -j8 && /tmp/logic-p35/charlcd_test | grep -c '^FAIL'`
→ 0; `/tmp/logic-p35/listing_check && /tmp/logic-p35/trace_check` pass
(est. 3 min).

Commit: d0131fd

### Phase 2 — split the controller state (no behavior change)

- [x] `CharLcdState` → shared part + `CharLcdCtl ctl[2]` (§ 4.3), with only
      `ctl[0]` used. `lcd_state.h` § 4.5 and the 25 test lines (28 lines
      in fact, see Corrections; no new test case). Check run 2026-11-11:
      `ctest --test-dir /tmp/logic-p35` → `100% tests passed, 0 tests failed out of 30`;
      `git diff --exit-code backend/tests/golden` → exit 0 (and
      `listing_check` / `trace_check` → `26729 parts compared, 0 skipped`);
      `charlcd_test`: 119 PASS, 0 FAIL, as after Phase 1.

Check: `ctest --test-dir /tmp/logic-p35` → `100% tests passed`;
`git diff --exit-code backend/tests/golden` → 0 (est. 4 min). Both must
hold with no new test case.

Commit: —

### Phase 3 — 4 rows, two controllers, RGB lights

- [ ] § 4.3 E routing, the read clash and timer ids; § 4.4 image; § 4.6
      lights and voltage.
- [ ] `charlcd_test` cases 2–9.
- [ ] `docs/parts-format.md` § `charlcd`: the key table of § 4.2, the
      row/controller map of § 4.4, the backlight array, the two panels.
      `docs/backend.md:152`: `LcdState.controllers` and the per-controller
      arrays.

Check: `ctest --test-dir /tmp/logic-p35` → `100% tests passed`, T0
programs; `git diff --exit-code backend/tests/golden` → 0;
`grep -c '"rows" must be 1, 2 or 4' docs/parts-format.md` and
`grep -c -F 'rows\" must be 1, 2 or 4' backend/src/models/charlcd.c`
→ 1 each (est. 5 min).

Commit: —

## Validation Plan

Per-phase checks. Phase 2's unchanged goldens are the gate for the risky
move. The landing tier is H.

## Decisions

- **Two controllers as one part with `ctl[2]`** (author; § 3 rejected
  alternatives).
- **`bus.e` takes an array of two names** (author): the part file names
  the pins the way the datasheets do (E1, E2), and the order is E1 first.
- **Backlight as an array of LED objects** (author): it matches the
  existing object form. The RGB modules share one anode (LED+/A) with
  three cathodes, and this form describes them as printed.
- **Panel names `gray` and `white`** (author): they name what the face
  looks like, as `yellow-green` does, and they stay short in the facts.
- **4 rows on one enable needs `columns` ≤ 20** (author): one HD44780 has
  80 DDRAM cells, so 4 rows × 20 is its maximum.

## Corrections

- **The `"rows"` greps on `charlcd.c`** (2026-11-11): the C source holds
  the message as `\"rows\" must be …` (escaped quotes), so
  `grep '"rows" must be 1 or 2' backend/src/models/charlcd.c` finds
  nothing (measured: 0 lines; `grep -n 'must be 1 or 2'` → charlcd.c:120).
  The prerequisite row, Phase 3's check and plan-35-E's prerequisite now
  grep the C file with `-F 'rows\" must be 1, 2 or 4'`; the docs file keeps
  the plain form.

- **No-backlight listing word** (2026-11-11, Phase 1): § 2 said the
  listing prints `-` for a module without a backlight; `listing.c:176` (at a5b0fcf)
  prints `none` (`l->backlight_color != NULL ? l->backlight_color : "none"`).
  § 2 now says `none`; Phase 1 keeps `none` for `backlight_count` 0.
- **Two existing loader cases change their expected message** (2026-11-11,
  Phase 1): § 1 Non-goals said existing `charlcd_test` cases are edited only
  for the `[0]` arrays, but `check_loader`'s "rows 3" and "panel green"
  cases expect `"rows" must be 1 or 2` and `"panel" must be yellow-green or
  blue-negative` (charlcd_test.c:160, 166), which § 4.2 replaces. Both cases
  stay and now expect the § 4.2 messages; the Non-goals and case 10 say so.
- **`seq` is per controller, not shared** (2026-11-11, Phase 2): § 4.3
  listed `seq` with the shared parts. `seq` is the access count a timer
  carries so `charlcd_timer` can drop timers from an earlier access
  (`(id >> 2) != seq`). Shared, an E2 rise would bump it and silently drop
  controller 1's pending `TIMER_WRITE` / `TIMER_READ_OFF` (a write on E1
  followed within its hold window by an E2 access would be lost). Each
  `CharLcdCtl` keeps its own `seq`; with one controller the timer ids are
  the same as before (the goldens prove it), and Phase 3's top id bit names
  the controller.
- **The `[0]` test edits are 28 lines, not 25** (2026-11-11, Phase 2): the
  § 2 grep (`ddram\[\|\.busy\|cgram\[`) misses `ddram_known` uses and
  `memcmp(st.ddram, …)` arguments. Every `LcdState` RAM/busy use was edited
  (`git diff -U0 backend/tests/charlcd_test.c | grep -c '^+[^+]'` → 28 at
  Phase 2): `st.`/`lcd.`/`before.`/`after.` `ddram`, `ddram_known`, `busy`.

## Summary

The risky part is moving the model's state into per-controller records. It
lands alone in Phase 2, gated on the unchanged tests and goldens, before
any new behavior is added.
