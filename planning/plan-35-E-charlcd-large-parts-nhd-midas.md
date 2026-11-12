# plan-35-E: 20x4 / 40x2 / 40x4 LCDs — the 22 Newhaven and 13 Midas parts, goldens, CPU-board tests

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-35-D

E adds the 35 Newhaven and Midas parts to `Parts/charlcd.json`, one device
per datasheet as plan-11-C did. It merges their 17 footprints from
`footprints-C.json` into `footprints.json`, moves the five 16 × 2 parts to
the `16x2` drawer leaf, and regenerates the goldens. `charlcd_test`'s
real-part check stops listing five names and instead runs every `charlcd`
part in the library. Two CPU boards show the new geometry end to end.

Outcome of E: parts.txt P0 + 35 lines (5 lines change only in the drawer
field), traces.txt P0 + 35 (no hash changes), footprints.txt F0 + 17;
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
| plan-35-D complete | `grep -c '^Commit: —' planning/plan-35-D-charlcd-large-kind.md` → 0 (or in `planning/completed/`) | NOT MET (2026-11-11) |
| The kind takes 4 rows | `grep -c '"rows" must be 1, 2 or 4' backend/src/models/charlcd.c` → 1 | NOT MET (D) |
| footprints-C.json loads | `planning/plan-35-data/census/fp-load.sh planning/plan-35-data/footprints-C.json` → only extra/unused for its ids | NOT MET (C) |
| Baseline measured now | `wc -l < backend/tests/golden/parts.txt` (P0), `… footprints.txt` (F0) | record at start (26729 / 1212 at 656b6d1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `grep -c '|charlcd|' backend/tests/golden/parts.txt` → 5 + 35 = 40.
- `wc -l < backend/tests/golden/parts.txt` → P0 + 35; `traces.txt` the
  same; `footprints.txt` → F0 + 17.
- `git diff backend/tests/golden/parts.txt | grep -c '^-[^-]'` → 5, each a
  16 × 2 line whose only change is `Displays/Character` →
  `Displays/Character/16x2`; `git diff backend/tests/golden/traces.txt | grep -c '^-[^-]'` → 0.
- `ctest --test-dir /tmp/logic-p35` → `100% tests passed`.
- `charlcd_test` shows text on every one of the 40 `charlcd` parts (§ 4.3)
  and passes the two CPU boards (§ 4.4).

### Non-goals (explicit constraints)

- No Focus part (F) and no app change (G).
- No existing part's facts, pins, footprint or trace change, apart from
  the drawer field of the five 16 × 2 parts.

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
| Their footprints | 17 | `grep -c '^LCM-MIDAS\|^LCM-NHD' planning/plan-35-data/footprints-needed.txt` |
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
  - `drawer`: `["Displays", "Character", "<function>"]`.
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
- **Drawer:** the five 16 × 2 devices change to
  `["Displays", "Character", "16x2"]`.

**Correctness gate:**

- added-lines-only goldens, except the five drawer lines;
- `charlcd_test`'s every-part check (each part runs the init sequence
  through its own pins and shows text; two-enable parts on both
  controllers);
- the two boards.

## 4. Detailed Design

### 4.1 `charlcd_test` every-part check

`check_real_parts` loops over `library` parts with `kind == PART_CHARLCD`:

- 8-bit init, clear, then `"HELLO"`, through E (or E1);
- two-enable parts do the same through E2 with `"WORLD"`;
- a 4-row part also sets DDRAM 0x14 and writes `"ROW3"`;
- expect cell (0,0) = ROM 'H'; on 4-row parts cell (2,0) = 'R' and,
  with two enables, cell (2,0) = 'W'.

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
   E2 = PHI2 · A14 · /A15 · A13, two more SN74HC11N gates. The program
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

- [ ] Write `planning/plan-35-data/census/gen-parts.py` (§ 3) and
      `planning/plan-35-data/specs.tsv` with its header.
- [ ] Fill one `specs.tsv` row per Newhaven and Midas part from its PDF:
      pin table, electrical table, timing page (render at 300 dpi where it
      is an image), instruction table, init flow; `ST7066U.pdf` for the 16
      devices of § 2. Record each page used.

Check: `python3 -I planning/plan-35-data/census/gen-parts.py --check planning/plan-35-data/specs.tsv planning/plan-35-data/candidates.tsv`
→ 35 rows, every key present, every pin count matching `candidates.tsv`
(est. 1 min).

Commit: —

### Phase 2 — parts, footprints, goldens

- [ ] Paste the generator's 35 devices into `charlcd.json`; change the
      five 16 × 2 drawers to `16x2`.
- [ ] Merge `footprints-C.json` into `footprints.json` with
      `python3 -I planning/plan-35-data/census/fp-merge.py schematical/schematical/Parts/footprints.json planning/plan-35-data/footprints-C.json`
      (it appends the objects, one line each, and refuses duplicates).
- [ ] Regenerate the goldens (`docs/backend.md` § goldens:
      `listing_check --print`, `trace_check --print`, `footprint_check --print`)
      and hand-check five new parts.txt lines, one per pinout shape,
      against their PDFs.

Check: § 1's three line counts and the two `grep -c '^-[^-]'` checks;
`ctest --test-dir /tmp/logic-p35` → `100% tests passed` (est. 5 min).

Commit: —

### Phase 3 — every-part check and the boards

- [ ] § 4.1, § 4.2 and § 4.3 in `charlcd_test.c`.

Check: `/tmp/logic-p35/charlcd_test | grep -c '^FAIL'` → 0, with one
every-part PASS line per `charlcd` part (40) and the two board lines
(est. 2 min).

Commit: —

## Validation Plan

Per-phase checks. The landing tier is H.

## Decisions

- **One device per datasheet** (plan-11-C rule, author).
- **Size leaves under Displays → Character** (author): with 72 character
  LCDs after F, a single leaf would be a long, mixed list. The 16 × 2
  parts move to `16x2` so the leaf has no loose parts beside size folders.
- **Exec and timing from `ST7066U.pdf` where the module prints none**
  (author): the Midas PDFs send the reader to it, and the 3 Newhaven PDFs
  name it as their controller.
- **Manufacturer string "Midas Displays"** (author): DigiKey's name; there
  is no earlier Midas part in the library (`grep -c 'Midas' schematical/schematical/Parts/*.json` → 0
  at 656b6d1).

## Corrections

(none yet)

## Summary

Data entry at volume. The every-part check runs each part through its own
pins, and the boards prove the 4-line map and the two-enable wiring
end to end.
