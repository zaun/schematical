# plan-11-C: Character LCD parts, footprints, goldens and a CPU board test

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-11-B

This sub-plan ships the parts:

- `Parts/charlcd.json`: 5 devices, 5 orderable parts, all in stock and in
  production. Each is one module from one datasheet;
- 5 new footprints in `footprints.json`: 4 module footprints from the
  mechanical drawings and the 1×16 header used as every part's
  `connectorFootprint`;
- regenerated goldens (5 added lines in each of `parts.txt` and
  `traces.txt`, 5 in `footprints.txt`);
- real-part tests: every part runs the init sequence and shows text, and
  each device's own exec times hold. The end-to-end test is a whole CPU
  board: an MPS6502 runs a program from an AT28C256 and prints `HELLO`
  on an NHD-0216K1Z-NSW-BBW-L wired straight to its bus. The same board
  with a W65C02S breaks the LCD's hold time, and the model catches it.

Outcome of C: `ctest` 18 of 18; goldens P0 + 5 / P0 + 5 / F0 + 5 lines,
only added lines.

References:

- plan-11-A § Prerequisites (the gate), § Decisions; plan-11-B (the kind,
  §4.2 keys, §6 test helpers).
- `AGENTS.md` § Datasheets (stock rule, footprint rule, goldens).
- `docs/parts-format.md` § Footprints (§ `explicit`, mid-values, the
  `note`), § `charlcd`.
- The five datasheets in `datasheets/module-disp-char/` (pages below).

## Prerequisites

The gate is plan-11-A § Prerequisites. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-11-B complete | `grep -c 'Commit: —' planning/plan-11-B-charlcd-kind.md` → 0 (or the file is in `planning/completed/`) | NOT MET (2026-11-01: B not started) |
| The kind loads | `grep -n 'CHARLCD_KIND' backend/src/library/kinds.c` → one line | NOT MET (lands in B) |
| Still in stock | the plan-11-A stock command → five `1`s | MET (2026-11-01) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `ctest --test-dir /tmp/logic-p11` → `100% tests passed, 0 tests failed out of 18`.
- `wc -l < backend/tests/golden/parts.txt` → P0 + 5 (7162 at 01c8377);
  `traces.txt` → the same; `footprints.txt` → F0 + 5 (200).
- `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (only added lines).
- `grep -c '|charlcd|' backend/tests/golden/parts.txt` → 5, each ending
  `|footprint=LCM-…|connector=HDR-1X16-P2.54|vcc=…`.
- `charlcd_test`'s CPU-board case shows `HELLO` (MPS6502) and catches the
  hold violation (W65C02S) (§4.6).

### Non-goals (explicit constraints)

- No existing part, footprint, listing line or trace hash changes.
- No app change (plan-11-D).
- No part beyond the five (owner, plan-11-A § Decisions). The siblings
  DigiKey lists (other BZ/K1Z/AMC variants) have their own datasheets, not
  in the folder.

## 2. Current State

Datasheet facts, read 2026-11-01 for this plan (pages are PDF pages):

| Fact | NHD-0216BZ-FL-YBW | NHD-0216K1Z-FL-YBW | NHD-0216K1Z-NSW-BBW-L | 162H BC BW | AMC1602GR-B-Y6WFDY |
|---|---|---|---|---|---|
| Maker | Newhaven | Newhaven | Newhaven | Displaytech | Orient Display |
| DigiKey stock (2026-11-01) | 4045, Active | 7648, Active | 3688, Active | 114, Active | 118, Active |
| Controller | ST7066U | ST7066U | ST7066U | "ST7066U or Equivalent" (p.8) | "S6A0069 or equivalent" (p.10) |
| Font | A00 (p.9) | A00 (p.10) | A00 (p.10) | A00 (p.12) | A00 (p.14) |
| Panel | STN positive yellow-green, transflective | same | STN blue negative, transmissive | STN positive yellow-green, transflective | STN positive yellow-green, transflective (Y…F) |
| Backlight | yellow-green LED | yellow-green LED | white side LED | yellow-green LED | yellow-green LED |
| VDD min–max | 4.7–5.3 V | 4.5–5.5 V | 4.5–5.5 V | 4.5–5.5 V | 4.5–5.5 V |
| Exec: instr / data / clear-home | 37 / 37 µs / 1.52 ms | same | same | 39 / 43 µs / 1.53 ms (p.13) | 39 / 43 µs / 1.53 ms (p.15) |
| Pins 1–3 / 15–16 names | VSS VDD V0 / LED+ LED- | same | same | VSS VDD VO / A K (p.7) | VSS VDD V0 / LED(+) LED(-) (p.8) |
| Board | 84.0 × 44.0 | 80.0 × 36.0 | 80.0 × 36.0 | 84.0 × 44.0 | 84.0 × 44.0 |
| Drawing | p.3 | p.4 | p.4 | p.8 | p.10 |

- Pins 4–14 are `RS`, `R/W`, `E`, `DB0`…`DB7` on all five.
- Bus timing is the same on all five (plan-11-B §2).
- All five wait 40 ms at power-up.
- Every part is a single-part datasheet: no other variant is covered.

Mechanical, from the drawings. The origin is the PCB's top-left in front
view, y down, mm; the drawings' general tolerance is ±0.3, with the
outline ±0.5. These are the values used as printed (BSC / single values):

- **NHD-0216BZ (p.3)**:
  - holes: 4 × Ø2.5 at (2.5, 4.0), (81.5, 4.0), (2.5, 40.0), (81.5, 40.0);
    the top two are drawn as slots open to the side edges; no pad size;
  - pins: 16 × Ø1.0, one row 2.54 above the bottom edge (y = 41.46);
  - pin order left to right is 14, 13, …, 1, 15, 16: pin 14 at x = 10.2,
    pin 1 at 43.22, 15 at 45.76, 16 at 48.3;
  - pads drawn as rounded slots to the edge, no size given.
- **NHD-0216K1Z (both, p.4)**:
  - holes: 4 × Ø2.5 PTH with Ø5.0 pads at (2.5, 2.5), (77.5, 2.5),
    (2.5, 33.5), (77.5, 33.5);
  - pins: two 1×16 rows, at y = 2.0 and y = 34.0. Pin 1 at x = 8.0 on
    both, pitch 2.54 to pin 16 at 46.1. Ø1.0 PTH, pad width 1.8;
  - separate backlight solder pads: the module's own LED leads, not part
    of the interface.
- **162H (p.8)**:
  - top holes: half-holes "R1.25 PTH / R2.5 PAD" at (2.5, 4.0) and
    (81.5, 4.0);
  - bottom holes: "Ø2.5 PTH / Ø5.0 PAD" at (4.0, 40.0) and (80.0, 40.0);
  - pins: 16 × Ø1.0 PTH, one row 2.0 above the bottom edge (y = 42.0);
  - order left to right 14, …, 1, 16, 15: pin 14 at 10.2, pin 1 at 43.22,
    16 at 45.76, 15 at 48.3; pad 1.8.
- **AMC1602G (p.10)**:
  - holes: top "R1.25" at (2.5, 4.0), (81.5, 4.0); bottom Ø2.5 at
    (4.0, 40.0), (80.0, 40.0); no pad sizes;
  - pins: 16 × Ø1.0, row 2.6 above the bottom edge (y = 41.4); same x and
    order as the 162H; no pad size.

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts added | 5 | § Fact table |
| Footprints added | 5 | §4.3 |
| Golden lines today (P0 / F0) | 7157 / 195 | `wc -l < backend/tests/golden/parts.txt`; `footprints.txt` |
| Existing footprint matching any module | 0 | no footprint in `footprints.json` has a 2.5 mm hole (`grep -c '"drill":2.5' schematical/schematical/Parts/footprints.json` → 0) |

### Verified properties

- **The MPS6502 board meets every LCD bus limit at 1 MHz** (calculated
  from the part files; the times are what the models use):
  - MPS6502 grade 1: tADS 300, tAH 30, tMDS 200, tDHW 30 ns
    (`cpu.json` MCS6502);
  - SN74HC11N rise and fall 10 ns (`gate.json` 74_11 HC);
  - ECS-2100A-010: 1 MHz, so PHI2 is high 500 ns.
  - E = PHI2 · A14 · /A15 rises and falls 10 ns after PHI2:
    - tPW about 500 ≥ 140;
    - RS and R/W valid 300 ns after PHI2 falls, so well before E rises
      (tAS 0);
    - write data valid 200 ns after PHI2 rises, so 310 ns before E falls
      (tDSW 40);
    - data held 30 − 10 = 20 ns past E's fall (tH 10); address held
      20 ns (tAH 10);
    - E rises at least 4 cycles apart (tC 1200).
  - Read: LCD data is valid 110 ns after PHI2 rises, well before the
    CPU's sample at PHI2's fall.
- **The W65C02S breaks it on the same wiring.** W65C02S grade 14: tDHW 10,
  tAH 10. E falls 10 ns after PHI2, so the data and address change 0 ns
  after E falls, which is less than tH = tAH = 10. The first write
  violates the hold window, and §4.5 of plan-11-B makes the controller
  LOST.
- **A floating bus during the 6502 reset is harmless.** The reset reads
  0000, 0100, 01FF and 01FE as dummy reads. With nothing at those
  addresses the data is Z (read as X). `docs/parts-format.md` § `cpu` says
  a tick on X data runs twice and goes on when both runs agree, and a
  dummy read's data is unused, so they agree.

## 3. Design

The data work follows the repo's footprint rules (`docs/parts-format.md`
§ The `"footprint"` part key: mid-values, the `note`, no overlaps):

- **One device per datasheet**, one part each. Panel, backlight color and
  VDD differ between the two K1Z datasheets, so they cannot share a
  device.
- **Four module footprints, one header footprint.** The two K1Z modules
  share one drawing, so they share a footprint. The 162H and AMC1602G
  differ only in pin-row height (2.0 vs 2.6 mm from the bottom edge), and
  that is enough for separate footprints. The header is the owner's
  Connector Only: pins 1–16 in order, left to right.
- **Pads with no size in the drawing**:
  - pins get round Ø1.8 (pin 1 square), the size the K1Z and 162H
    drawings print;
  - mounting holes get Ø5.0 pads, the K1Z and 162H pad size;
  - the half-holes and edge slots are drawn as Ø2.5 holes at their
    centres.
  Each `note` says so.

**Correctness gate:** added-lines-only goldens, plus behavior tests. Every
existing line stays byte-identical; the 15 new lines are expected.

**Rejected alternatives**

- *One shared 84×44 footprint for 162H and AMC1602G.* Rejected: their pin
  rows sit 0.6 mm apart, and a 1.0 mm drill at the wrong height would not
  fit the module.
- *Numbering the K1Z backlight solder pads as pins 15/16.* Rejected: they
  are where the module's LED leads are soldered, not interface holes the
  host board connects to.
- *The W65C02S for the CPU board's positive case.* Rejected: its 10 ns hold
  times break the LCD's on this wiring (§2). That is why it is the
  negative case instead.

## 4. Detailed Design

### 4.1 Devices (`Parts/charlcd.json`)

Common to all five:

- `"function": "16x2"`, `"drawer": ["Displays", "Character"]`,
  `"columns": 16`, `"rows": 2`, `"rom": "A00"`;
- `"bus": {"rs": "RS", "rw": "R/W", "e": "E", "data": ["DB0", …, "DB7"]}`;
- `"timing": {"tc": 1200, "tpw": 140, "tas": 0, "tah": 10, "tdsw": 40, "th": 10, "tddr": 100, "tdhr": 10}`;
- one pinout `"p16"`: pins 1–16 named as the § Fact table says. Roles:
  - 1 `ground`, 2 `power`;
  - 3 (V0), 4–6 and 15–16 `input`;
  - 7–14 `inout`, `"group": "data"`;
- the part: `"package": "Module"`, `"pinout": "p16"`,
  `"connectorFootprint": "HDR-1X16-P2.54"`.

| Device id | Part | `controller` | `panel` | `backlight` (anode, cathode, color) | `vcc` | `exec` (ns: instruction, data, clearHome, powerOn) | `footprint` |
|---|---|---|---|---|---|---|---|
| `NHD-0216BZ-FL` | `NHD-0216BZ-FL-YBW` | `ST7066U` | `yellow-green` | `LED+`, `LED-`, `Yellow-green` | [4.7, 5.3] | 37000, 37000, 1520000, 40000000 | `LCM-NHD-0216BZ` |
| `NHD-0216K1Z-FL` | `NHD-0216K1Z-FL-YBW` | `ST7066U` | `yellow-green` | `LED+`, `LED-`, `Yellow-green` | [4.5, 5.5] | same | `LCM-NHD-0216K1Z` |
| `NHD-0216K1Z-NSW` | `NHD-0216K1Z-NSW-BBW-L` | `ST7066U` | `blue-negative` | `LED+`, `LED-`, `White` | [4.5, 5.5] | same | `LCM-NHD-0216K1Z` |
| `162H` | `162H BC BW` | `ST7066U` | `yellow-green` | `A`, `K`, `Yellow-green` | [4.5, 5.5] | 39000, 43000, 1530000, 40000000 | `LCM-DT-162H` |
| `AMC1602G` | `AMC1602GR-B-Y6WFDY` | `S6A0069` | `yellow-green` | `LED(+)`, `LED(-)`, `Yellow-green` | [4.5, 5.5] | same as 162H | `LCM-AMC1602G` |

`"contrast"` is `"V0"`, or `"VO"` on the 162H. Descriptions follow the
pattern `"16x2 character LCD module, STN yellow-green, transflective,
yellow-green LED backlight"`. Each device's `note` names the PDF and its
pages (pin table, electrical, timing, exec, font, drawing).

### 4.2 Module footprints (`explicit`, body = the module PCB)

Local coordinates: body centre origin, y down, mm.

- Pin pads: Ø1.8, drill 1.0; pin 1 `rect` 1.8 × 1.8, the others `round`.
- Mounting holes: `"pin": 0`, round Ø5.0, drill 2.5.

| id | body | pin row y | pin x | holes (x, y) |
|---|---|---|---|---|
| `LCM-NHD-0216BZ` | 84.0 × 44.0 | 19.46 | pins 1–14: x = 1.22 − (n − 1)·2.54 (pin 14 at −31.8); 15: 3.76; 16: 6.3 | (±39.5, −18), (±39.5, 18) |
| `LCM-NHD-0216K1Z` | 80.0 × 36.0 | −16 **and** 16 (each pin has two pads, top row first) | x = −32 + (n − 1)·2.54 (pin 16 at 6.1) | (±37.5, ±15.5) |
| `LCM-DT-162H` | 84.0 × 44.0 | 20.0 | pins 1–14 as BZ; 16: 3.76; 15: 6.3 | (±39.5, −18), (±38, 18) |
| `LCM-AMC1602G` | 84.0 × 44.0 | 19.4 | as 162H | as 162H |

Descriptions, for example: `"Newhaven NHD-0216K1Z 16x2 LCD module, 80.0 x
36.0 mm, two 1x16 headers (top and bottom), 4 mounting holes"`. Notes
record the drawing page and every size taken from another drawing (§3).
For the BZ, the note also says that the top holes are slots open to the
edge, drawn as round holes.

### 4.3 The header footprint

`HDR-1X16-P2.54`, `explicit`:

- `"1x16 pin header, 2.54 mm pitch, pins 1-16 left to right"`;
- body 40.64 × 2.54;
- pin n at x = (n − 8.5)·2.54, y = 0;
- pads Ø1.8 with drill 1.0, pin 1 `rect`;
- note: "Connector Only footprint of the character LCD modules (plan-11);
  hole and pad from the modules' Ø1.0 PTH / 1.8 pads".

`footprint_check` sees it used through `connector_footprint` (plan-11-A).

### 4.4 Goldens

```
/tmp/logic-p11/listing_check --print > backend/tests/golden/parts.txt
/tmp/logic-p11/trace_check --print > backend/tests/golden/traces.txt
/tmp/logic-p11/footprint_check --print > backend/tests/golden/footprints.txt
```

Review: `git diff --stat backend/tests/golden` shows 5 + 5 + 5 insertions
and 0 deletions. Hand-check the 5 footprint lines against §4.2–4.3:
pad count, each pin's x and y, hole positions. `LCM-NHD-0216K1Z` has
32 designated pads plus 4 holes; the others have 16 plus 4.

### 4.5 Real-part tests (`charlcd_test`)

- **Every part.** For each of the five, by name from the library: the
  8-bit init and `HELLO` of plan-11-B case 2 give `ddram[0..4]` =
  `HELLO` and the dots of cell 0 = ROM 48.
- **Per-device exec.** A second data write 40 µs after the first is
  accepted on `NHD-0216K1Z-NSW-BBW-L` (data 37 µs) and not accepted on
  `162H BC BW` (data 43 µs).
- **Facts.** `NHD-0216K1Z-NSW-BBW-L`:
  - `part->charlcd->panel` = `blue-negative`;
  - `backlight_color` = `White`;
  - `footprint->id` = `LCM-NHD-0216K1Z` and `connector_footprint->id` =
    `HDR-1X16-P2.54`;
  - its footprint has 2 pads with designator `"1"`.

### 4.6 The CPU board test (`charlcd_test`, circuit API)

Parts:

- `5V`, `GND`;
- `ECS-2100A-010` (1 MHz);
- `MPS6502`;
- `AT28C256-15PU`;
- `SN74HC00N`, `SN74HC11N`;
- `TE-GDS0204`;
- `NHD-0216K1Z-NSW-BBW-L`.

Nets:

- **Address and data.** CPU A0–A14 go to EEPROM A0–A14, and A0 also to LCD
  RS. CPU D0–D7, EEPROM I/O0–7 and LCD DB0–7 share one bus.
- **Decode.**
  - HC00 gate 1 (1A = 1B = A15): 1Y = /A15, which is the EEPROM CE#.
  - HC00 gate 2 (2A = 2B = R/W): 2Y is the EEPROM OE#.
  - EEPROM WE# is tied to 5V.
  - HC11 gate 1 makes the LCD's E: 1A = PHI2OUT, 1B = A14, 1C = HC00 1Y.
  - LCD R/W comes from CPU R/W.
- **Clock and reset.**
  - The oscillator output drives PHI0IN.
  - Switch position 1 (pins 1A, 1B) connects RES# to GND. It is closed at
    start and opened at 10 µs (Z reads inactive).
  - RDY, IRQ#, NMI# and SO# are unconnected (inactive).
- **Power.** LCD VSS = GND, VDD = 5V, V0 = GND, LED+ = 5V, LED- = GND.
  Every chip's power pins go to 5V and GND.

The memory map gives the EEPROM 8000–FFFF, LCD instructions at 4000 and
LCD data at 4001. The program is loaded with `circuit_memory_load` at
EEPROM offset 0 (address 8000):

```
8000 A2 00     LDX #0
8002 BD 27 80  LDA $8027,X   ; command list
8005 F0 0B     BEQ $8012
8007 2C 00 40  BIT $4000     ; busy flag -> N
800A 30 FB     BMI $8007
800C 8D 00 40  STA $4000
800F E8        INX
8010 D0 F0     BNE $8002
8012 A2 00     LDX #0
8014 BD 2C 80  LDA $802C,X   ; text
8017 F0 0B     BEQ $8024
8019 2C 00 40  BIT $4000
801C 30 FB     BMI $8019
801E 8D 01 40  STA $4001
8021 E8        INX
8022 D0 F0     BNE $8014
8024 4C 24 80  JMP $8024
8027 38 0C 01 06 00        ; function set, display on, clear, entry mode
802C 48 45 4C 4C 4F 00     ; "HELLO"
```

The reset vector at offset 7FFC/7FFD is `00 80`, and NMI and IRQ point to
8024. The test runs 50 ms with `circuit_advance`. Expected:

1. `circuit_lcd_state`: `ddram[0..4]` = `HELLO`, `ddram[5]` = 20, and the
   dots of cells 0–4 equal ROM 48 45 4C 4C 4F. `busy` = 0.
2. `circuit_cpu_state`: status RUNNING, last instruction `JMP $8024`.
3. **The W65C02S board.** Same nets with `W65C02S6TPG-14`:
   - PHI2 is driven by the oscillator, and the oscillator output also feeds
     HC11 1A. The W65C02S's bus times are measured from its PHI2 input, so
     E is 10 ns behind the clock edge the hold times count from;
   - RESB, RWB and the VDD and VSS names replace the MPS6502's;
   - BE is unconnected (Z reads inactive, which keeps the bus enabled).
   After 50 ms:
   - every dot of `circuit_lcd_state` is `LCD_DOT_X`, and `busy` is −1 (the
     first `STA $4000` broke the hold time, so the controller is LOST);
   - `circuit_cpu_state` shows status LOST: its next `BIT $4000` read an X
     busy flag, and the branch depended on it.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the
> same commit as the work it describes; `- [~]` for partial with one line of
> what remains; mark moot tasks `- [x] ~~text~~ — moot: <evidence>`; fill
> each `Commit:` line when the phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Parts, footprints, goldens

Footprints and parts land together: `footprint_check` fails on a footprint
no part uses.

- [ ] `schematical/schematical/Parts/footprints.json`: the 5 footprints of §4.2–4.3.
- [ ] `schematical/schematical/Parts/charlcd.json`: the 5 devices of §4.1.
- [ ] Regenerate the three goldens (§4.4) and review the diff.
- [ ] `docs/parts-format.md` § `charlcd`: the example device is
      `NHD-0216K1Z-NSW` as in §4.1.
- [ ] Nothing to register in Xcode: `Parts/` is a synchronized folder, and
      `BackendCatalog.load` reads every bundled `.json`. `cpu.json` arrived
      the same way (c9da192 touched no `project.pbxproj`).

Acceptance: five parts load with their footprints, and the goldens grow by
added lines only.
  Check: `cmake --build /tmp/logic-p11 -j8 && ctest --test-dir /tmp/logic-p11` → 18 of 18;
  `git diff --stat backend/tests/golden` → 15 insertions, 0 deletions;
  `grep -c '|charlcd|' backend/tests/golden/parts.txt` → 5 (est. 2 min).
Commit: —

### Phase 2 — Real-part and CPU board tests

- [ ] `backend/tests/charlcd_test.c`: §4.5 cases.
- [ ] `backend/tests/charlcd_test.c`: §4.6, the MPS6502 board and the
      W65C02S board.

Acceptance: every part shows `HELLO`, the per-device exec times hold, the
MPS6502 program prints `HELLO`, and the W65C02S board is caught.
  Check: `ctest --test-dir /tmp/logic-p11 -R charlcd_test --output-on-failure` → passes (est. 1 min; the board runs 50 ms of simulated time because the LCD's 40 ms power-up wait comes first).
Commit: —

## Validation Plan

- Tests: §4.5 and §4.6 in `charlcd_test`; the 5 trace lines and 5 listing
  lines; the 5 footprint lines. The negative case is the W65C02S board.
- Coverage check: `listing_check`, `trace_check` and `footprint_check`
  each see the five parts (`grep -c` above). The CPU board exercises the
  real parts through the circuit API, as the app does.
- Doc sync: `docs/parts-format.md` § `charlcd` example.
- Final gate (once, after Phase 2): `ctest --test-dir /tmp/logic-p11` →
  18 of 18; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; the app
  build command of plan-11-A Phase 3 → `** BUILD SUCCEEDED **` (est. 5 min).

## Decisions

- One device and one part per datasheet; K1Z modules share a footprint
  (author, from the drawings). (§3)
- Pad sizes the drawings omit come from the sibling drawings (author; each
  `note` says so). (§3)
- Connector Only is a plain 1×16 header, pins 1–16 left to right, for every
  part (owner, 2026-11-01). (§4.3)
- The K1Z footprint numbers both header rows (owner, 2026-11-01). (§4.2)

## Corrections

## Summary

Data work with exact numbers from the drawings, gated by
added-lines-only goldens. The CPU board ties the controller model to real
parts and real CPU timing. The W65C02S case shows that the timing check
does real work.
