# plan-35-E: 20x4 / 40x2 / 40x4 LCDs — the 22 Newhaven and 13 Midas parts, goldens, CPU-board tests

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-35-D

E adds the 35 Newhaven and Midas parts to `Parts/charlcd.json`, one device
per datasheet as plan-11-C did. It merges their 18 footprints from
`footprints-C.json` into `footprints.json` and regenerates the goldens.
(The five 16 × 2 parts are already in the `16x2` drawer leaf: the leaf is
`function`, Corrections 9.) `charlcd_test`'s
real-part check stops listing five names and instead runs every `charlcd`
part in the library. Two CPU boards show the new geometry end to end.

Outcome of E: parts.txt P0 + 35 lines (no line changes), traces.txt P0 + 35 (no hash changes), footprints.txt F0 + 18;
`ctest` passes.

References:

- plan-35-A § Prerequisites and § 2 (`ST7066U.pdf`, inclusion rule).
- plan-35-D § 4 (the keys this uses).
- `docs/parts-format.md` § `charlcd`, § Supply voltage, § Footprints.
- `planning/completed/plan-11-C-charlcd-parts.md` (device layout, `note`
  style, the HELLO board of §4.6).
- `planning/plan-35-data/candidates.tsv`: columns `panel`, `backlight`,
  `pinout`, `pins`, `footprint`, `connector`, `pdf`.

## Prerequisites

The gate is plan-35-A § Prerequisites. E adds:

| Must be true | Command | Status |
|---|---|---|
| plan-35-D complete | `grep -c '^Commit: —' planning/plan-35-D-charlcd-large-kind.md` → 0 (or in `planning/completed/`) | MET (2026-11-11 at fcbda57: 0) |
| The kind takes 4 rows | `grep -c -F 'rows\" must be 1, 2 or 4' backend/src/models/charlcd.c` → 1 (corrected, plan-35-D Corrections) | MET (2026-11-11 at fcbda57: 1) |
| footprints-C.json loads | `planning/plan-35-data/census/fp-load.sh planning/plan-35-data/footprints-C.json` → only extra/unused for its ids | MET (2026-11-11: B + C on the live backend → "34 new footprints: load ok", plan-35-C Phase 2) |
| Baseline measured now | `wc -l < backend/tests/golden/parts.txt` (P0), `… footprints.txt` (F0) | recorded at start: P0 = 26729, F0 = 1212 (2026-11-11 at fcbda57; unchanged since 656b6d1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `grep -c '|charlcd|' backend/tests/golden/parts.txt` → 5 + 35 = 40.
- `wc -l < backend/tests/golden/parts.txt` → P0 + 35; `traces.txt` the
  same; `footprints.txt` → F0 + 18 (17 at planning; plan-35-C Corrections 1).
- `git diff backend/tests/golden/parts.txt | grep -c '^-[^-]'` → 0 (was 5
  drawer lines; moot, Corrections 9); `git diff backend/tests/golden/traces.txt | grep -c '^-[^-]'` → 0.
- `ctest --test-dir /tmp/logic-p35` → `100% tests passed`.
- `charlcd_test` shows text on every one of the 40 `charlcd` parts (§ 4.1)
  and passes the two CPU boards (§ 4.3; corrected, Corrections 10).

### Non-goals (explicit constraints)

- No Focus part (F) and no app change (G).
- No existing part's facts, pins, footprint or trace change.

## 2. Current State

- `charlcd.json` has 5 devices, each `"drawer": ["Displays", "Character"]`
  (`grep -c '"drawer": \["Displays", "Character"\]' schematical/schematical/Parts/charlcd.json`
  → 5).
- `charlcd_test.c:1144` `check_real_parts` lists the five names.
- The 35 parts (from `candidates.tsv`):

| Group | Parts | Size | Pinout | Panel / backlight |
|---|---|---|---|---|
| NHD-0420AZ (3.3 V) | FL-GBW-33V3, FL-YBW-33V3, FSW-GBW-33V3 | 20x4 | p16 | gray, yellow-green, gray / Yellow-green, Yellow-green, White |
| NHD-0420DZ | FL-GBW, FL-YBW, FL-YBW-33V3, FSW-FBW, NSW-BBW | 20x4 | p16 | gray, yellow-green ×2, white, blue-negative |
| NHD-0420E2Z | FL-GBW, FL-YBW, FSW-GBW, NSW-BBW | 20x4 | p16 | gray, yellow-green, gray, blue-negative |
| NHD-0420H1Z | FL-GBW-33V3, FSW-GBW, FSW-GBW-33V3 | 20x4 | p16 | gray |
| NHD-0240AZ | FL-GBW, FL-YBW, FSW-FBW, NSW-BBW | 40x2 | p16 | gray, yellow-green, white, blue-negative |
| NHD-0440AZ-RN-FBW | — | 40x4 | p18-dual-nc | white / none |
| NHD-0440WH | ATFH-JT#, ATMI-JT# | 40x4 | p18-dual | white, blue-negative / White |
| Midas MC42004/5/8 | MC42004A6W-FPTLW-V2, MC42004A6W-SPR-V2, MC42005A6W-FPTLW-V2, MC42005A6W-SPTLY-V2, MC42008A6W-FPTLW, MC42008A6W-SPTLY | 20x4 | p16 (SPR: p16-nc) | FPTLW white / White; SPTLY, SPR yellow-green / Yellow-green, none |
| Midas RGB 20x4 | MD42004A6W-FPTLRGB, MD42005C6W-FPTLRGB | 20x4 | p18-rgb | white / Red, Green, Blue |
| Midas MD24005A6W-FPTLRGB | — | 40x2 | p18-rgb-pads | white / Red, Green, Blue |
| Midas MC44005 | BNMLW-V2, FPTLW-V2, SPTLY-V2 | 40x4 | p18-dual | blue-negative, white, yellow-green |
| Midas MD44005A6W-FPTLRGB | — | 40x4 | p20-rgb-dual | white / Red, Green, Blue |

### Measured populations

| What | Count | Command |
|---|---|---|
| Newhaven parts | 22 | `awk -F'\t' '$1=="Newhaven Display Intl" && $4=="add"' planning/plan-35-data/candidates.tsv \| wc -l` |
| Midas parts | 13 | `… $1=="Midas Displays" …` |
| Their footprints | 18 (17 at planning; plan-35-C Corrections 1) | `grep -c '^LCM-MIDAS\|^LCM-NHD' planning/plan-35-data/footprints-needed.txt` |
| Devices whose timing and exec come from `ST7066U.pdf` | 13 Midas (timing and exec), 3 Newhaven (exec only: NHD-0240AZ-FSW-FBW, NHD-0240AZ-NSW-BBW, NHD-0440AZ-RN-FBW) | plan-35-A § 2 |

### Verified properties

- **Newhaven timing tables are images.** `pdftotext` on
  NHD-0420DZ-FL-GBW finds the "Timing Characteristics" heading but no
  timing number (`grep -c -E '1200|tAS|tPW'` → 0). They are read from a
  300 dpi render of that page, as plan-11-C read the 16 × 2 pages.
- **Every Newhaven and Midas PDF names its controller.** Newhaven: ST7066U
  (DZ, AZ, H1Z, 0240AZ, 0440*; "SPLC780D OR ST7066U" on DigiKey) or
  KS0066U (E2Z, per DigiKey). Midas: ST7066U / ST7063C+ST7066U. The device's
  `controller` is the PDF's own wording, read in Phase 1.
- **Pin names** come from each PDF's pin table (`census/pins.py` found 16,
  18 or 20 pins in all 35; MD24005's backlight pins are named only). They
  are typed as printed (`LED+`, `A`, `K(R)`, `Vss`).

## 3. Design

- **One device per datasheet** (plan-11-C § 3): 35 devices with 35 parts.
  - Device `id`: the part name without a trailing `#` (one part per
    device, so the part name is unique); none clashes with the five
    existing ids.
  - `function`: `20x4`, `40x2` or `40x4`.
  - `drawer`: `["Displays", "Character"]`; the `function` is the leaf
    (corrected, Corrections 9).
- **Facts per device:**
  - `columns` / `rows` from the size;
  - `controller` as printed; `rom` `A00`;
  - `panel` and `backlight` from `candidates.tsv`; the color as the
    datasheet prints it (`White`, `Yellow-green`; RGB: `Red`, `Green`,
    `Blue`);
  - `bus` from the pin table: `e` is `["E1", "E2"]` on the 40 × 4 parts;
  - `contrast` is the V0/VO/Vo pin name;
  - `timing` and `exec` from the PDF (or `ST7066U.pdf` p.36 at 5 V and
    p.20, for the 16 devices above);
  - `vcc` = the min and max of the PDF's VDD row in its electrical
    characteristics (e.g. NHD-0420AZ-FL-GBW-33V3 "Supply Voltage VDD 3.1
    3.3 3.5 V"); no `inputMax`, as on the 16 × 2 devices;
  - `powerOn` = the PDF's initialization wait (40 ms on every flow seen;
    ST7066U.pdf: 40 ms).
- **`note`:** the PDF and the pages of pins, electrical, timing,
  instructions, font and drawing, and `ST7066U.pdf` pages where used.
- **Parts:**
  - `name` = the orderable part number (with its `#`);
  - `manufacturer` "Newhaven Display" (as the existing parts) or "Midas
    Displays";
  - `datasheet` = A's id; `package` "Module";
  - `footprint` and `connectorFootprint` from `candidates.tsv`.
- **Generator:** `planning/plan-35-data/census/gen-parts.py specs.tsv
  candidates.tsv` prints the device objects. `specs.tsv` (one row per part,
  filled from the PDFs in Phase 1) holds everything `candidates.tsv`
  does not: device id, controller, vcc, timing ×8, exec ×4, pin names
  1…N, note pages. The script is written in Phase 1; its output is pasted
  into `charlcd.json` in id order.
- **MD24005 pin numbers:** the interface is 1–14, and the named backlight
  pads are numbered 15 A, 16 K(R), 17 K(G), 18 K(B) in the datasheet's
  listing order (as C's footprint).
- **Drawer:** ~~the five 16 × 2 devices change to
  `["Displays", "Character", "16x2"]`~~ — moot: they already list under
  Displays → Character → 16x2 (Corrections 9).

**Correctness gate:**

- added-lines-only goldens;
- `charlcd_test`'s every-part check (each part runs the init sequence
  through its own pins and shows text; two-enable parts on both
  controllers);
- the two boards.

## 4. Detailed Design

### 4.1 `charlcd_test` every-part check

`check_real_parts` loops over `library` parts with `kind == PART_CHARLCD`:

- through the part's own bus pins (`bus_pins`) and its own timing and
  exec times;
- 8-bit init, clear, then `"HELLO"`, through E (or E1);
- a 4-row part then sets DDRAM 0x14 and writes `"ROW3"`, through E (or E1);
- two-enable parts then init, clear and write `"WORLD"` through E2;
- expect cells (0,0…4) = ROM `HELLO`; on one-enable 4-row parts cell
  (2,0) = 'R'; with two enables, 0x14 is controller 1's line 0, column 20,
  so cell (0,20) = 'R' and cell (2,0) = 'W' (corrected, Corrections 10).

The line names the part, so a failure says which one.

### 4.2 Exec checks

The existing "second data write 40 µs later" case keeps its two parts, and
adds `NHD-0420DZ-FL-YBW` (37 µs exec) and `MC44005A6W-FPTLW-V2` (ST7066U
37 µs).

### 4.3 The boards

1. **20 × 4: MPS6502 + AT28C256 + NHD-0420DZ-FL-YBW**, wired as plan-11-C
   §4.6's HELLO board (E = PHI2 · A14 · /A15). The program writes
   `LINE1`, then sets DDRAM 0x40 / 0x14 / 0x54 and writes `LINE2`,
   `LINE3`, `LINE4`. Expect rows 0–3 to show them, and BF 0.
2. **40 × 4: the same board with NHD-0440WH-ATFH-JT#.** A13 selects E1
   (A13 = 0) or E2 (A13 = 1): E1 = PHI2 · A14 · /A15 · /A13,
   E2 = PHI2 · A14 · /A15 · A13, two more SN74HC11N gates (gate 1 becomes
   SEL = A14 · /A15, gates 2 and 3 are PHI2 · SEL · /A13 and
   PHI2 · SEL · A13; /A13 is SN74HC00N gate 3; corrected, Corrections 11).
   The program
   initialises both controllers and writes `TOP` through E1 and `BOTTOM`
   through E2. Expect row 0 `TOP` and row 2 `BOTTOM`.
   - The timing margins are those of plan-11-C § 2's MPS6502 analysis
     (same CPU, clock and gate family; NHD-0440WH's AC table is read in
     Phase 1).
   - Any limit the analysis does not meet is a bug in the board wiring,
     to fix before the case is written.

## Phases

> Tick `- [x]` in the same commit as the work; `- [~]` for partial with one
> line of what remains. Fill each `Commit:` line when the phase lands. **An
> unticked box means NOT DONE.**

### Phase 1 — specs from the PDFs

- [x] Write `planning/plan-35-data/census/gen-parts.py` (§ 3) and
      `planning/plan-35-data/specs.tsv` with its header. — the column
      format is the script's docstring; pin roles come from the names
      (VSS/GND/FGND ground, VDD power, NC nc, the 8 data pins inout);
      `-` in `candidates.tsv` `backlight` means none.
- [x] Fill one `specs.tsv` row per Newhaven and Midas part from its PDF:
      pin table, electrical table, timing page (render at 300 dpi where it
      is an image), instruction table, init flow; `ST7066U.pdf` for the 16
      devices of § 2. Record each page used. — 35 rows, pages in each
      `note`. Timing: 1200/140/0/10/40/10/100/10 on 32 (19 Newhaven own
      tables, 13 Midas from ST7066U.pdf p.36), 1200/480/0/10/80/10/320/10
      on the three Newhaven parts with a 3 V table (tpw: 460 write / 480
      read printed, 480 used). Exec 37/37/1520 µs + 40 ms on 33,
      39/43/1530 µs on the two NHD-0420H1Z 5 V-table parts. Every
      controller string is `ST7066U` (see Corrections).

Check: `python3 -I planning/plan-35-data/census/gen-parts.py --check planning/plan-35-data/specs.tsv planning/plan-35-data/candidates.tsv`
→ 35 rows, every key present, every pin count matching `candidates.tsv`
(est. 1 min). — 2026-11-11: "35 rows ok".

Commit: 0420112

### Phase 2 — parts, footprints, goldens

- [x] Paste the generator's 35 devices into `charlcd.json`; ~~change the
      five 16 × 2 drawers to `16x2`~~ (moot, Corrections 9). — appended
      after the five, in id order (`json.load` → 40 devices); every
      device's drawer is `["Displays", "Character"]`.
- [x] Merge `footprints-C.json` into `footprints.json` with
      `python3 -I planning/plan-35-data/census/fp-merge.py schematical/schematical/Parts/footprints.json planning/plan-35-data/footprints-C.json`
      (it appends the objects, one line each, and refuses duplicates). —
      "18 footprints appended".
- [x] Regenerate the goldens (`docs/backend.md` § goldens:
      `listing_check --print`, `trace_check --print`, `footprint_check --print`)
      and hand-check five new parts.txt lines, one per pinout shape,
      against their PDFs. — NHD-0420DZ-FL-YBW (p16: pins p.3, VDD
      4.5–5.5 p.4), MC42004A6W-SPR-V2 (p16-nc: 15/16 NC, p.2 table),
      MD24005A6W-FPTLRGB (p18-rgb-pads: 15 A, 16–18 K(R/G/B)),
      NHD-0440AZ-RN-FBW (p18-dual-nc: 9 E1, 15 E2, 16–18 NC, no
      backlight, `bus=11,10,9,15,…`), MD44005A6W-FPTLRGB (p20-rgb-dual:
      17 LED+, 18 K(B), 19 K(G), 20 K(R), `enables=2`) — each matches its
      pin table.

Check: § 1's three line counts and the two `grep -c '^-[^-]'` checks;
`ctest --test-dir /tmp/logic-p35` → `100% tests passed` (est. 5 min). —
2026-11-11: parts.txt 26764 (P0 26729 + 35), traces.txt 26764,
footprints.txt 1230 (F0 1212 + 18), `grep -c '|charlcd|'` → 40,
`git diff backend/tests/golden | grep -c '^-[^-]'` → 0; ctest "100% tests
passed, 0 tests failed out of 30".

Commit: 0c364c6

### Phase 3 — every-part check and the boards

- [x] § 4.1, § 4.2 and § 4.3 in `charlcd_test.c`. — `check_every_part`
      loops over the library's `PART_CHARLCD` parts; pin names come from
      `bus_pins`, the access shape (setup tAS, E high max(tPW, tDSW, tDDR),
      hold max(tH, tAH, tDHR), cycle ≥ tC, each + 20 ns) and the waits
      (exec + 1 µs, power-on + 1 ms) from the part's device in
      `charlcd.json`. The boards' programs are assembled in the test (one
      BF-polled list loop per controller, no indexed store to the LCD).
- [x] Timing margins (§ 4.3): NHD-0440WH-ATFH-JT.pdf pp.7–8 (images, read
      at 300 dpi): tC 1200, tPW 140, tR/tF ≤ 25, tAS 0, tAH 10, tDSW 40,
      tH 10, tDDR ≤ 100 ns — the same as NHD-0420DZ-FL-YBW and the 16 × 2
      NHD parts. Both boards keep PHI2 → E (E1, E2) at one SN74HC11N gate
      (10 ns), so plan-11-C § 2 holds unchanged: tPW ≈ 500 ≥ 140 (+360);
      tAS 210 ≥ 0 (RS, R/W valid 300 ns after PHI2 falls, E rises 510 ns
      after); tDSW 310 ≥ 40 (+270); tH = tAH = 30 − 10 = 20 ≥ 10 (+10);
      tR/tF 10 ≤ 25; LCD accesses ≥ 4 cycles apart ≥ tC 1200; read data
      valid 110 ns after PHI2 rises, sampled at its fall (+390). A second
      gate level in the PHI2 path would leave tH = 30 − 20 = 10, zero
      margin, hence the SEL arrangement (Corrections 11).

Check: `/tmp/logic-p35/charlcd_test | grep -c '^FAIL'` → 0, with one
every-part PASS line per `charlcd` part (40) and the two board lines
(est. 2 min). — 2026-11-11: FAIL count 0 (`all passed`); 40 every-part
PASS lines (`E 4.1 every part: <name> (…)`) plus "40 charlcd parts run";
"PASS: E 4.3 MPS6502 + NHD-0420DZ-FL-YBW board: DDRAM 00/40/14/54 =
LINE1-4 on rows 0-3 (ROM cells), BF 0, the CPU RUNNING at its JMP"
(`JMP $8022`) and "PASS: E 4.3 MPS6502 + NHD-0440WH-ATFH-JT# board: TOP
through E1 on row 0, BOTTOM through E2 on row 2, rows 1 and 3 blank, both
BF 0, the CPU RUNNING at its JMP" (`JMP $8042`); the four § 4.2 exec lines
pass; `ctest --test-dir /tmp/logic-p35` → "100% tests passed, 0 tests
failed out of 30"; `git diff --exit-code backend/tests/golden` → exit 0.
Sanity: E2's init and WORLD sent through E1, ROW3's 0x94 changed to 0xD4
on board 1, and board 2's E2 gate fed /A13 → 9 FAILs (the 7 two-enable
parts and both boards); restored → 0.

Commit: —

## Validation Plan

Per-phase checks. The landing tier is H.

## Decisions

- **One device per datasheet** (plan-11-C rule, author).
- **Size leaves under Displays → Character** (author): with 72 character
  LCDs after F, a single leaf would be a long, mixed list. The leaf is the
  device's `function` (`16x2`, `20x4`, `40x2`, `40x4`), so the 16 × 2
  parts were already in theirs (Corrections 9).
- **Exec and timing from `ST7066U.pdf` where the module prints none**
  (author): the Midas PDFs send the reader to it, and the 3 Newhaven PDFs
  name it as their controller.
- **Manufacturer string "Midas Displays"** (author): DigiKey's name; there
  is no earlier Midas part in the library (`grep -c 'Midas' schematical/schematical/Parts/*.json` → 0
  at 656b6d1).

## Corrections

Phase 1 (2026-11-11, evidence = the PDFs' own pages, in each device note):

1. **NHD-0420E2Z's controller is ST7066U**, not KS0066U: all four
   NHD-0420E2Z PDFs print "Built-in ST7066U" (p.2, p.5) and "Driver IC:
   ST7066U" (p.3). The KS0066U came from DigiKey. § 2's "Every Newhaven and
   Midas PDF names its controller" stands; every row's controller is
   ST7066U (six Midas PDFs print "ST7066U+ST7063C"; the notes quote it).
2. **Only the 13 Midas devices take exec from `ST7066U.pdf`, not 16.**
   NHD-0240AZ-FSW-FBW, NHD-0240AZ-NSW-BBW and NHD-0440AZ-RN-FBW print the
   instruction table as an image on p.7 (37 µs, 1.52 ms at 270 kHz). § 2's
   measured-population row and plan-35-A § 2's sentence about them
   overstated it; their exec comes from their own PDF.
3. **Three 3.3 V Newhaven modules print only the 5 V timing table**
   (NHD-0420AZ-FL-YBW-33V3, NHD-0420DZ-FL-YBW-33V3,
   NHD-0420H1Z-FSW-GBW-33V3: tpw 140, tdsw 40, tddr 100). Their devices use
   the table as printed (§ 3 "timing from the PDF"); the notes say so. The
   other three 3.3 V modules print a 2.7 V table and use it.
4. **"powerOn 40 ms on every flow seen" is not quite true:** NHD-0440WH
   (p.10) and NHD-0440AZ-RN-FBW (pp.11–12) print example code with
   `delay(15)` and no unit. They take 40 ms from `ST7066U.pdf` p.25 ("busy
   state lasts for 40 ms after VCC rises to 4.5 V"), as the Midas devices.
5. **10 of the 13 Midas PDFs give VDD as a typical 5.0 V only.** Their
   `vcc` is 4.5–5.5 from `ST7066U.pdf` p.38 (the controller's DC table),
   noted per device. MC42005A6W-FPTLW-V2 (4.5–5.5, p.5) and
   MD24005A6W-FPTLRGB (4.7–5.5, p.2) print a range and use it. No Midas
   module is 3.3 V.
6. **The PDFs' VDD ranges differ from DigiKey's (`candidates.tsv`
   `dk_vcc`)**; the devices use the PDFs (e.g. NHD-0240AZ-FL-GBW
   4.8–5.2, NHD-0420AZ-FSW-GBW-33V3 3.0–3.5, NHD-0420DZ-FSW-FBW 4.7–5.3).
7. **MC44005A6W-BNMLW-V2 contradicts itself**: p.2 says "PIN15: LED+,
   PIN16: LED-", its pin table p.4 says 17 LED+, 18 LED-. The device
   follows the pin table (the drawing agrees, plan-35-C).
8. **RGB cathode order differs per module**: MD42004 and MD44005 number
   K(B), K(G), K(R); MD42005C numbers K(R), K(G), K(B). Pins are as
   printed; every `backlight` array is Red, Green, Blue by name.

Phase 2 (2026-11-11):

9. **The drawer leaf is `function`, not a third `drawer` level.**
   `docs/parts-format.md` § device keys: `"drawer"` is "the parts-drawer
   path above the function: 0–2 strings"; `library.c:685` rejects a third
   level (`"drawer" has at most 2 levels`, measured: the first paste with
   `["Displays", "Character", "16x2"]` failed to load). The five 16 × 2
   devices already have `"function": "16x2"`, so they already list under
   Displays → Character → 16x2: the drawer move is moot, the new devices
   use `["Displays", "Character"]` with `function` 20x4 / 40x2 / 40x4
   (gen-parts.py corrected), and parts.txt changes no existing line (§ 1
   corrected from 5 to 0).

Phase 3 (2026-11-11):

10. **§ 4.1's expectations contradicted each other on two-enable parts**
    ("on 4-row parts cell (2,0) = 'R' and, with two enables, cell (2,0) =
    'W'"). With two enables each controller is a 2-line controller of 40
    columns (plan-35-D § 4.2, § 4.4), so DDRAM 0x14 written through E1 is
    row 0, column 20, and row 2 is E2's line 0. § 4.1 now writes ROW3
    through E (E1) and expects (2,0) = 'R' on one-enable 4-row parts, and
    (0,20) = 'R' and (2,0) = 'W' with two enables. § 1 also pointed at
    § 4.3 / § 4.4 for the check and boards; they are § 4.1 / § 4.3.
11. **Board 2's gates: SEL first, not a 4-input AND after the old E.**
    Chaining /A13 after the board's PHI2 · A14 · /A15 gate puts two gate
    levels (20 ns) between PHI2 and E1/E2, leaving tH = tAH = 30 − 20 =
    10 ns, exactly the limit. SN74HC11N gate 1 makes SEL = A14 · /A15 (1C
    tied to 5V), gates 2 and 3 make E1 = PHI2 · SEL · /A13 and
    E2 = PHI2 · SEL · A13, and SN74HC00N gate 3 makes /A13: still "two
    more SN74HC11N gates", plus one NAND as an inverter, and PHI2 → E stays
    10 ns as in plan-11-C § 2. Board 1 keeps the HELLO board's single gate.

## Summary

Data entry at volume. The every-part check runs each part through its own
pins, and the boards prove the 4-line map and the two-enable wiring
end to end.
