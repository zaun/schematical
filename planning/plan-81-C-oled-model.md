# plan-81-C: The SSD1306-class model: GDDRAM, commands, profiles, I2C and 4-wire SPI, `circuit_oled_state`

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-81-B

C makes the `oled` kind simulate the monochrome page-addressed controllers (owner decision,
2026-11-25: simulate the SSD1306 class). A device with a `"model"` runs the controller's command
set on its I2C or 4-wire SPI interface, chosen by the module's strap pins at reset, and publishes
its pixels through a new `oled_state` hook and `circuit_oled_state`. D adds 3-wire SPI and the two
parallel interfaces. No part ships here (E).

References:

- Controller datasheets in `datasheets/display-oled/controllers/` (plan-81-A Phase 2):
  `SSD1306.pdf` (rev 1.1: Table 7-1 BS[2:0]; Table 8-1 pin use; § 8.1.5 I2C; § 8.7 "No data read
  is provided in serial mode operation"; Table 9-1 commands and RESET values; § 10.1.3 addressing;
  Table 10-3 COM pins), `SSD1305.pdf`, `SSD1309.pdf`, `SH1106.pdf` (IM0–2 table p. 6;
  "Read is not possible while in serial interface mode", text line 506; I2C "supports both read
  and write access"), `SH1107.pdf`.
- `backend/src/models/serial_target.h` (`SerialSpi`, `SerialI2c`: events, `serial_*_init/start/pin/enable`).
- `backend/include/lcd_state.h`, `backend/src/engine/sim.h:157-159`, `backend/src/circuit.c:779`
  (the `lcd_state` precedent for a display-state hook).
- `backend/src/models/charlcd.c` (X tracking per field, LOST, power-on state).

## Prerequisites

plan-81-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-81-B complete | `ls planning/completed/plan-81-B-*.md` → exists | NOT MET (2026-11-25) |
| The controller datasheets are on disk | `ls datasheets/display-oled/controllers/{SSD1306,SSD1305,SSD1309,SH1106,SH1107}.pdf` → 5 files | NOT MET (A Phase 2 fetches them; planning fetched each once into a scratch folder: `%PDF-`, sizes in `planning/plan-81-data/README.md`) |
| No display-state hook named `oled_state` | `grep -c 'oled_state' backend/src/engine/sim.h backend/include/circuit.h` → 0 0 | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- An `oled` device with `"model"` loads (keys § 3.2) and is simulated: a test that bit-bangs the
  SSD1306 init sequence and a 128 × 32 pattern over I2C to an SSD1305-profile fixture reads back
  that pattern from `circuit_oled_state`, pixel for pixel; the same over 4-wire SPI.
- Every command of § 3.3 has a test; pixels never written read X; RES# low resets the registers
  and keeps GDDRAM; an X on RES# makes everything X.
- `ctest` passes T1 + 1 programs (no new program: cases go into `oled_test`).

### Non-goals (explicit constraints)

- No 3-wire SPI, 8080 or 6800 (D). A strap that selects one of them in C makes the device hold
  every pixel X and record `interface "spi3" is not modeled yet` as its state's `fault` (removed in D).
- No greyscale or colour controller model; they stay facts only (owner decision).
- No analog behaviour: contrast, precharge, VCOMH, clock divide, charge pump and DC-DC are stored
  as facts (contrast is published) and change nothing else.
- Scrolling (`26h`/`27h`/`29h`/`2Ah`/`A3h`/`2Fh`) is accepted and not animated: while scrolling is
  active the published image is all X with `fault` = `scrolling is not modeled`; `2Eh` clears it.
  (Rejected: a frame-timed scroll, which would need the oscillator and frame rate the model does
  not have.)

## 2. Current State

- B's `oled` loads facts devices only; the engine model is passive.
- `SimTime` is in ps (`sim.h:7`); `pin_changed` gives each edge with `sim_now()`.
- `SerialI2c`'s `address` callback returns ACK/NACK per address byte and its `next` callback
  supplies read bytes; `SerialSpi` hands whole bytes in `received` and asks `next` for MISO bytes.

## 3. Design

### 3.1 Profiles (`"model"`)

| `"model"` | Controllers (`"controller"` values that may use it) | GDDRAM | Addressing | Specific commands |
|---|---|---|---|---|
| `"ssd1306"` | SSD1306, SSD1306B, SSD1306BZ, SSD1315 | 128 × 64 (8 pages) | `20h` horizontal / vertical / page; `21h`, `22h` | `8Dh` charge pump |
| `"ssd1309"` | SSD1309 | 128 × 64 | as ssd1306 | `FDh` command lock (`16h` locks every command but `FDh`; `12h` unlocks) |
| `"ssd1305"` | SSD1305 | 132 × 64 | as ssd1306 | `D8h` area colour / low power (fact), `FDh` lock as ssd1309, `82h` brightness (fact) |
| `"sh1106"` | SH1106, SH1106G, CH1116 | 132 × 64 | page only (no `20h`–`22h`) | `ADh`+`8Bh`/`8Ah` DC-DC, `30h`–`33h` pump voltage, `E0h`/`EEh` read-modify-write |
| `"sh1107"` | SH1107 | 128 × 128 (16 pages) | `20h` page / `21h` vertical (one-byte forms) | `DCh` + byte start line (0–127), `D3h` offset 0–127, `A8h` multiplex up to 128, `ADh` DC-DC, `E0h`/`EEh` |

SSD1315 and CH1116 have no controller datasheet on the hosts A uses
(`planning/plan-81-data/README.md`); the module datasheets print their command tables, which match
SSD1306 and SH1106 for every command above. A profile/controller pair outside this table is an
error: `controller "X" cannot use model "ssd1306"`.

### 3.2 Keys of a simulated device

`"model"` replaces `"simulated": false` (`an oled with a "model" is simulated: remove "simulated":
false`); needs `"format": "graphic"` and `"depth": 1`. Plus:

| Key | Value | Error |
|---|---|---|
| `"bus"` | `{"cs", "dc", "res", "rw", "e", "data": [D0 … D7], "straps": {"BS0" \| "IM0": …}}`: pin names, each one pin. `data` lists the module pins wired to D0–D7 (a serial-only module lists D0 SCLK/SCL, D1 SDIN/SDA, and D2 when the module brings SDAOUT out; absent bits as `null`). `cs`, `rw`, `e`, `res` may be `null` (tied on the module). `straps`: each strap of the controller (`BS0`–`BS2` on SSD13xx, `IM0`–`IM2` on SH110x) is a module pin name, or `0` / `1` (tied on the module) | `"bus" needs "dc"`, `strap "BS1" must be a pin name, 0 or 1`, `"data" must have 8 entries` |
| `"glass"` | `{"columnOffset": n, "segRemap": 0\|1, "comScan": 0\|1, "comPins": <DAh value>}`: the reference setting under which glass pixel (x, y) shows RAM column x + columnOffset, row y (the module datasheet's init sequence). `columnOffset` 0–(RAM width − width); `comPins` 0x02, 0x12, 0x22 or 0x32 (ssd13xx) | `"columnOffset" must be 0 to <n>` |
| `"address"` | optional: the 7-bit I2C base address, default `0x3C` (SA0 adds 1) | `"address" must be 0x3C` (the only base these controllers have) |

Pins named by `bus` are roles `input` (CS, D/C#, RES#, R/W#, E, straps) or `inout` (D0–D7).
A part of a simulated device is accepted by `circuit_add_part`.

### 3.3 State and commands (`oled_ctl.c`)

Registers, each a value plus a known flag (charlcd's per-field X): display on (`AEh`/`AFh`),
entire on (`A4h`/`A5h`), inverse (`A6h`/`A7h`), contrast (`81h`), start line (`40h`–`7Fh`;
sh1107 `DCh`), offset (`D3h`), multiplex (`A8h`, 16–64; sh1107 16–128), segment remap
(`A0h`/`A1h`), COM scan (`C0h`/`C8h`), COM pins (`DAh`), addressing mode, column/page pointers
and window (`21h`, `22h`, `00h`–`0Fh`, `10h`–`1Fh`, `B0h`–`B7h`/`BFh`), the facts (clock `D5h`,
precharge `D9h`, VCOMH `DBh`, charge pump `8Dh`, DC-DC `ADh`, pump `30h`–`33h`), lock (`FDh`),
scrolling flag, read-modify-write flag, NOP `E3h`. Multi-byte commands collect their argument
bytes (Table 9-1's lengths per profile); a data byte (D/C# = 1) writes the 8 vertical pixels of the
current column and page and moves the pointer per addressing mode (horizontal: column, wrap to
window start and next page; vertical: page first; page mode: column, no wrap past the window end —
SSD1306 § 10.1.3; SH1106 page mode: column to 131 and stops).

RESET values are Table 9-1's (SSD1306) and each SH datasheet's: display off, start line 0, offset
0, mux 63 (sh1107: 127), remap 0, scan 0, COM pins 0x12, page mode, contrast 0x7F (sh110x 0x80),
pointers 0. At sim start the registers hold their RESET values and every GDDRAM bit is X (the
datasheets give GDDRAM no power-on value). RES# low (≥ 3 µs, the SSD1306 `t1`) resets the
registers and keeps GDDRAM; RES# reading X makes every register and GDDRAM bit X (LOST, as charlcd).
An unknown command byte, a command byte with X bits, or D/C# reading X at a byte's sample makes
every register X (LOST); a data byte with X bits stores X bits.

### 3.4 Image

For glass pixel (x, y), 0 ≤ x < width, 0 ≤ y < height:

1. The physical SEG and COM that the glass wires to (x, y) are fixed by `glass`: SEG
   s = (segRemap ? Wram − 1 − (x + columnOffset) : x + columnOffset); COM c is the COM that,
   under `glass.comScan` and `glass.comPins` with offset 0 and start line 0, scans RAM row y
   (Table 10-3's mapping, inverted).
2. Under the firmware's registers: SEG s shows RAM column (remap ? Wram − 1 − s : s); COM c is
   driven in scan slot k by (scan, COM pins) per Table 10-3; slot k ≥ mux is off; slot k shows
   RAM row (k + start line) mod Hram, after the offset rotates the slots by `offset` (§ 10.1.15).
3. Pixel = off when display off; on when entire on; else the RAM bit, inverted when inverse.
   X when any register the step reads is X, or the RAM bit is X.

`OledState` (`backend/include/oled_state.h`): `width`, `height`, `display_on` (1/0/−1),
`contrast` (0–255, −1 X), `inverse`, `fault` (NULL or a message), and `pixels[128][128]` of
`OLED_PIXEL_OFF` / `OLED_PIXEL_ON` / `OLED_PIXEL_X`. New `SimComponentType.oled_state` hook
(`sim.h`, after `lcd_state`) and `int circuit_oled_state(const Circuit *, size_t part, OledState *)`
(`circuit.h`/`circuit.c`, the `circuit_lcd_state` shape, `circuit.c:779-794`: 0 on success, −1
with the error `part <n> is not an OLED display` for a part without the hook).

### 3.5 Interfaces (`oled.c`: strap decode; serial front ends from `serial_target.h`)

At reset (sim start, and each RES# rise) the model reads the straps: a tied strap is its level, a
pin strap is the pin's level then. Decode: SSD13xx BS[2:0] 010 I2C, 000 4-wire SPI, 001 3-wire SPI,
110 8080, 100 6800 (Table 7-1); SH110x IM[2:0] per its table (IM2 IM1 IM0: 010 I2C, 000 4-wire,
001 3-wire, 110 8080, 100 6800; p. 6). A strap at X or an undefined code: every pixel X, `fault`
`interface straps are X` / `straps select no interface`.

- **I2C** (`SerialI2c` on D0 = SCL, D1 = SDA in, D2 = SDA out when wired, open drain): address =
  base + SA0, SA0 = the D/C# pin's level at the address byte. Write: control byte (Co, D/C#) then
  bytes per Co (Co = 1: one byte, then a control byte again; Co = 0: the rest of the transaction is
  of that D/C#). Read (R/W# = 1): ssd13xx profiles NACK the address (§ 8.7: no read in serial
  mode); sh110x profiles ACK and return the status byte (bit 7 BUSY = 0, bit 6 ON/OFF = display
  off, low bits 0) after a control byte with D/C# = 0, else GDDRAM data with the parallel read's
  dummy-read rule (SH1106 p. 9).
- **4-wire SPI** (`SerialSpi` on CS#, D0 = SCLK, D1 = SDIN, the front end's mode 0: SDIN sampled
  on SCLK's rising edge, SSD1306 Fig. 8-5): D/C# sampled at each byte's 8th bit (SSD1306 Fig. 8-5); MISO
  unused (no reads: § 8.7, SH1106 line 506). CS# `null` (module ties it low) = always selected.

### Rejected alternatives

- **One model per controller.** The five profiles share the pointer, image and LOST logic and
  differ in RAM size and a dozen opcodes; a profile table keeps one engine.
- **Publishing GDDRAM and letting the app render.** The SEG/COM mapping (§ 3.4) is the part most
  likely to be wrong; it belongs in C where tests pin it, not in Swift.

## 4. Phases

### Phase 1 — profiles, keys, GDDRAM and commands

- [ ] `backend/src/models/oled.c`: the `"model"`, `"bus"`, `"glass"`, `"address"` keys and errors (§ 3.2).
- [ ] `backend/src/models/oled_ctl.c` / `oled_ctl.h`: profiles (§ 3.1), registers, commands,
      RESET, LOST (§ 3.3), fed bytes by a test hook `oled_ctl_byte(ctl, dc, data, known)`.
- [ ] `backend/tests/oled_test.c`: per command and profile, bytes fed through the hook.

**Acceptance:** `ctest --test-dir /tmp/logic-p81 -R oled_test --output-on-failure` → passed, with
one case per opcode of § 3.3 per profile that has it (count the `OLED_CASE(` lines: ≥ the opcode
table's rows). (≈ 30 s.)

Commit:

### Phase 2 — image and state

- [ ] `oled_ctl.c`: § 3.4's image.
- [ ] `backend/include/oled_state.h`, `sim.h` hook, `circuit.h`/`circuit.c` `circuit_oled_state`.
- [ ] Tests: each of A0/A1 × C0/C8 × the four COM pin values on a 128 × 64 glass with reference
      `A1 C8 0x12` (the common module init) against hand-computed 8 × 8 corners; offset and start
      line; mux 32 on a 64-row glass; SH1106 column offset 2; inverse, entire on, display off; X.

**Acceptance:** `ctest … -R oled_test` → passed. (≈ 30 s.)

Commit:

### Phase 3 — I2C and 4-wire SPI

- [ ] `oled.c`: strap decode at reset (§ 3.5), `SerialI2c` and `SerialSpi` wiring, RES# handling.
- [ ] Tests, bit-banged on a `Bench` (`backend/tests/support/bench.h`) with `oled_test.c`'s own
      static `i2c_bit/i2c_start/i2c_stop/i2c_byte/i2c_in` and `spi_byte` helpers, the shape of
      `backend/tests/lcd_spi_i2c_test.c:222-290` (each test file keeps its own, as
      `rtc_test.c:273-320` does): the SSD1306 app-note init
      sequence (`AEh D5h 80h A8h 1Fh D3h 00h 40h 8Dh 14h 20h 00h A1h C8h DAh 02h 81h 8Fh D9h F1h
      DBh 40h A4h A6h AFh`) and a 512-byte pattern, over I2C at address 0x3C and 0x3D (SA0), and
      over SPI; SSD13xx I2C read NACKed; SH1106 I2C status read; RES# pulse; straps X.

**Acceptance:** `ctest … -R oled_test` → passed; `ctest --test-dir /tmp/logic-p81 -j8` → 100%. (≈ 1 min.)

Commit:

### Phase 4 — docs

- [ ] `docs/parts-format.md` § `oled`: simulated devices, keys, profiles, behaviour (§ 3.1–3.5).
- [ ] `docs/backend.md` § Layout (`oled_ctl.c`), and the display-state API next to `circuit_lcd_state`.

**Acceptance:** `grep -c 'circuit_oled_state' docs/backend.md` → ≥ 1. (≈ 1 s.)

Commit:

## 5. Validation Plan

End of C: `ctest --test-dir /tmp/logic-p81 -j8` → 100% of T1 + 1; `node scripts/source-size.js
--check` and `node scripts/coverage.js --check --backend` → exit 0 (`oled.c`, `oled_ctl.c` 100%
lines); goldens unchanged (`git diff --stat backend/tests/golden` → empty).
