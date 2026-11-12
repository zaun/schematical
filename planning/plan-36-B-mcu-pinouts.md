# plan-36-B: Microcontrollers — pinouts and device facts

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-36-A

B turns the `add` rows of `candidates.tsv` into pin tables and device
facts. Most AVR pinouts come from Microchip's own machine-readable device
files: the ATDF XML inside the device packs (DFPs), which list every
package's pins by position. The rest are transcribed from the datasheets:
AVR devices whose ATDF has no pinout, the ESP32 modules and the RP2040.
Device facts come from the same sources: core, flash, RAM and EEPROM from
the ATDF, clock and supply range from DigiKey, each checked against the
datasheet.

Outcome: `planning/plan-36-data/pinouts.tsv` holds one row per pin of
every pinout an `add` row uses. `pinout-map.tsv` maps each (device,
package code) to one of those pinouts, and `devices.tsv` holds one row
per device. G generates `mcu.json` from these three files and
`candidates.tsv`.

References:

- `docs/parts-format.md` § Pinouts and pins (numbered and ball pinouts, roles).
- `planning/plan-36-data/packs.tsv` (the 12 packs: URL, SHA-256, bytes);
  `census/atdfmatch.py` (`load_atdf`, `geom`, `match`).
- The pack licence: `LICENSE.txt` in each `.atpack`, Apache-2.0,
  "Copyright (c) 2026 Microchip Technology Inc." (read from
  `Microchip.ATmega_DFP.3.6.299.atpack`).

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-36-A complete | `ls planning/completed/plan-36-A-*.md` → exists | NOT MET (2026-11-12: plan written) |
| The packs are on disk | `ls /tmp/p36/packs/atdf/*.atdf \| wc -l` → 353 (A Phase 1 extracts them) | NOT MET until A Phase 1 |

## 1. Goal

- Every `add` row of `candidates.tsv` has `pinout_src`, one of `atdf`,
  `transcribed` or `module-table`, and a `pinout-map.tsv` line for its
  (device, `package_code`).
- `pinouts.tsv` passes `census/pinout-lint.py`. Each numbered pinout of N
  pins lists the numbers 1…N exactly once, each ball pinout lists each
  ball once, every role is one of the seven, and every name used is in
  the role table (§ 3.2) or matches a port pattern.
- Each ATDF pinout used passes the name check (§ 3.4). The ten popular
  devices of § 3.4 pass a full figure-by-figure check, recorded in
  `pinout-checks.tsv`.
- `devices.tsv` has one row per device of the `add` rows (316 devices at
  planning time, before the verdicts).

### Non-goals (explicit constraints)

- No alternate pin functions (ADC, SPI, UART names) in the pin names: a pin
  is named by its ATDF pad name or the datasheet's primary name (§ 3.1).
- No change to the part-file format: `pinouts.tsv` uses only the pin keys
  that exist (`number`/`ball`, `name`, `role`; `group` and `side` are G's).
- No pack, ATDF or extracted XML is committed. Only the derived TSVs and
  the scripts go in the tree; the packs are re-downloaded by URL and
  SHA-256 (`packs.tsv`).

## 2. Current State

### Measured populations (planning listing, 2026-11-12, before A's verdicts)

| What | Count | Command |
|---|---|---|
| ATDF files in the 12 packs | 353 | `ls /tmp/p36/packs/atdf \| wc -l` |
| ATDF pinouts | 455; 20 of them skip numbers (the NC pads are left out) | `atdfmatch.load_atdf()`, numeric positions vs max |
| AVR rows whose package has an ATDF pinout (same geometry and pin count) | **1242** of 1468: 1029 with one match, 213 with several (all 213 have identical pad lists) | `python3 -I census/atdfmatch.py` → `one pinout 1029, several 213`; the several-match check → `identical pad lists 213 different 0` |
| …distinct (ATDF device, pinout) | 336 | same → `distinct (ATDF device, pinout) used 336` |
| AVR rows with no ATDF pinout | **226** on 92 (device, geometry, pin count) units, from 59 datasheet URLs: 186 rows where the ATDF has no pinouts at all (52 devices: the AT90PWM and AT90USB parts, ATmega162/165/325/3250/3290/406/645/6450/6490/8515/8535, ATtiny24/44/84/26/261A/461A/861A/2313/2313A/4313/1634/167/87/43U/828, ATxmega192C3), 18 with no ATDF device (ATtiny102F, ATtiny1634R, ATtiny828R, ATtiny28V, AT90PWM2, AVR32EB14/20/28/32), 22 with no ATDF pinout of that package | the planning-time unit census (§ 3.3), `atdfmatch.py` miss lines |
| Distinct non-port ATDF pad names | 72 (§ 3.2) | every `pad` not matching `^P[A-R][0-9]$` across all ATDF pinouts |
| ESP32 module devices / pin tables | 13 devices; one "Pin Definitions" table each | `candidates.tsv` ESP32 devices |
| RP2040 | 1 device, QFN-56 plus the exposed pad, pin 57 (GND) | `rp2040-datasheet.pdf` § 1.4.1 "Pin Locations" (`pdftotext` line 629) |

### Verified properties

- **ATDF pinouts match the datasheet.** For ATmega328P, ATDF `TQFP32` lists
  `1 PD3, 2 PD4, 3 GND …`. The datasheet's Figure 1-1 (32 TQFP Top View,
  `ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061B.pdf` p.12) prints
  `PD3 1, PD4 2, GND 3, VCC 4, GND 5, VCC 6, PB6 7, PB7 8` on its left
  side (`pdftotext -layout` lines 480–560).
- **Several matches are one pinout.** In each of the 213 rows where two
  ATDF pinouts match (e.g. `DIP28` and `SOIC28`, `TQFP_QFN_44` and
  `DRQFN_44` for ATmega164PA), the pad lists are identical. So the rule
  "same geometry and pin count → same numbering" holds, except for the
  three geometries kept apart in § 3.1.
- **The ATDF names the reset and UPDI pads only for the newer parts**:
  ATtiny1614's ATDF has `<signal group="RESET" pad="PA0"/>` and `<signal
  group="UPDI" pad="PA0"/>`. ATmega328P's ATDF has no signal on `PC6`; it
  has only a `RESET` interrupt. So pin names stay the bare pad names
  (§ 3.1).
- **ATDF memory segments**: ATmega328P `FLASH size="0x8000"`, `IRAM
  size="0x0800"`, and an `eeprom` segment. The `device` element carries
  `architecture` (`AVR8`, `AVR8L`, `AVR8X`, `AVR8_XMEGA`) and `family`.
  Their counts over the 353 files: AVR8 175, AVR8X 125, AVR8_XMEGA 45,
  AVR8L 8.

## 3. Design

### 3.1 Pinout names and pin names

- **Pinout name**: `p<N>` for dual-row packages (DIP, SOIC, SSOP, TSSOP,
  DFN/UDFN, SOT-23), the `cpu.json` `p40` precedent. `quad<N>` for QFP,
  QFN and MLF; `drqfn<N>` for dual-row QFNs; `plcc<N>`; `bga<N>` and
  `wlcsp<N>` (ball pinouts); `module<N>` for ESP32 modules, where N counts
  the numbered pads including the exposed GND pad. Packages with the same
  pin numbering share one pinout: DIP, SOIC and SSOP of one device are one
  `p28`, and its TQFP and VQFN are one `quad32`. Two pinouts of one
  device with the same name but different pin lists are a bug in the map
  (lint fails); none was found at planning time (§ 2).
- **Geometry classes are never merged across**: dual, quad, drqfn, plcc,
  grid. PLCC numbering starts mid-side, and DRQFN uses A/B row numbers. A
  TQFP-44 and a PLCC-44 are different pinouts.
- **Pin name** = the ATDF pad name with surrounding space trimmed (`"VCC "`
  occurs 4 times), or the datasheet's primary name for transcribed
  pinouts: the name outside the parentheses, e.g. `PC6` for `PC6
  (RESET/PCINT14)`. Positions missing from an ATDF pinout (20 pinouts) are
  `NC`, role `nc`, the datasheet's DNC/NC pads.
- **The package code picks the pinout**: `package-codes.tsv` is written
  here from A's `package_code` values. Its columns are `code geometry pins
  name drawing footprint`, e.g. `32A	quad	32	32-lead TQFP		`. B
  fills the first four. C–E fill `drawing` (PDF:page) and `footprint`
  (column 6). The ESP32 rows use the module device as the code, with
  geometry `module`. DigiKey's Package / Case is not used
  after A: it is wrong for some rows (`AVR32DD20` listed as `28-SOIC`,
  `AVR16EB28` as `14-TSSOP`).

### 3.2 Roles (`census/roles.py`)

| Role | Names |
|---|---|
| `inout` | port pins `P[A-R][0-9]`; `D+`, `D-`, `DM`, `DP` (USB); `PDI`, `PDI_DATA`, `UPDI`; ESP32 `I/O`; RP2040 `GPIO*`, `QSPI_*`, `SWDIO`, `USB_DP`, `USB_DM` |
| `input` | `RESET`, `PEN`, `AREF`, `REF`, `XTAL1`, `TOSC1`, `ADC6`, `ADC7`, `VBUS` (sense); ESP32 `I` (including `EN`); RP2040 `XIN`, `RUN`, `SWCLK`, `TESTEN` |
| `output` | `XTAL2`, `TOSC2`, `UCAP`, `CAPH`, `CAPL`, `LCDCAP`, `BIAS1`, `BIAS2`, `COM0`–`COM3`, `SEG0`–`SEG23`; ESP32 `O`; RP2040 `XOUT`, `VREG_VOUT` |
| `power` | `VCC`, `AVCC`, `AVDD`, `VDD`, `VDD0`–`VDD2`, `VDDIO2`, `UVCC`, `VUSB`, `VBAT`, `VLCD`; ESP32 `P` except `GND`; RP2040 `IOVDD`, `DVDD`, `USB_VDD`, `ADC_AVDD`, `VREG_VIN` |
| `ground` | `GND`, `GND0`–`GND2`, `AGND`, `UGND`; ESP32 `GND`; RP2040 the exposed pad 57 `GND` |
| `nc` | `NC`, `DNC`; ESP32 `-` |

A name outside the table and the port pattern fails `pinout-lint.py`; add
it to the table with the datasheet's description in the commit.

`VREG_VOUT` is an `output` and `DVDD` a `power` pin. The voltage check
then sees the core rail at the part's I/O level (F § 3.4 explains why
that is safe).

### 3.3 Transcribed pinouts (AVR without an ATDF pinout, ESP32, RP2040)

- `census/units.py candidates.tsv` lists the (device, package code) pairs
  the ATDF cannot fill (92 (device, geometry, pins) units at planning time,
  counted before A replaced DigiKey's packages with the ordering codes).
  One datasheet figure usually covers several devices ("Pinout
  ATtiny24/44/84"). The transcription unit is (PDF, figure), and its rows
  are copied to every device the figure's caption names.
- Procedure per unit: `pdftoppm -r 150 -png -f <p> -l <p>` the figure page;
  type the pins into `transcribed.tsv` (`pdf page figure device-list
  pinout number name`); roles come from § 3.2. The page and figure go in
  `src`, e.g. `transcribed:attiny24_44_84.pdf:p2:Figure 1-1`.
- **ESP32 modules** (`module-table`): the "Pin Definitions" table of each
  module datasheet, with columns Name, No., Type. Pad numbers follow the
  datasheet's pin layout figure, and the exposed GND pad is the next
  number (WROOM-32E: pin 39). Strip trailing footnote digits from names
  using the table's own footnote list (`IO163` → `IO16`).
- **RP2040**: § 1.4.2 "Pin Descriptions" and the QFN-56 pinout (§ 1.4.1),
  pins 1–56 plus 57 `GND`.

### 3.4 Checks

- `census/pinout-lint.py pinouts.tsv`: § 1's structural rules.
- **Name check** (every ATDF pinout used): every pin name of the pinout,
  NC aside, occurs as a token in the text of its datasheet's
  pin-configuration pages. Those are the pages whose text has `Pin
  Configuration`, `Pinout` or `Pin-out` and the device name.
  `census/pinout-check.py` writes `pinout-checks.tsv` (`device pinout pdf
  pages result`).
- **Full check** (by eye, against the figure): ATmega328P (`p28`,
  `quad32`), ATmega2560 (`quad100`), ATmega32U4 (`quad44`), ATmega1284P
  (`p40`, `quad44`), ATmega4809 (`quad48`), ATtiny85 (`p8`), ATtiny13A
  (`p8`), ATtiny84A (`p14`), ATtiny1614 (`p14`), AVR128DA48 (`quad48`).
  Each pin's number and name is compared with the datasheet figure, and the
  result goes in `pinout-checks.tsv` as `full ok` or the differences.

### 3.5 Device facts (`devices.tsv`)

Columns: `device` (the ATDF `name`, e.g. `ATmega328P`; ESP32 module names
as Espressif prints them; `RP2040`), `family` (the drawer folder,
`candidates.tsv`), `core`, `clock_hz`, `flash`, `ram`, `eeprom`,
`vcc_min`, `vcc_max`, `src`.

- `core`: ATDF `architecture` → `avr` (AVR8), `avrrc` (AVR8L, the reduced
  core of ATtiny4/5/9/10/20/40/102/104), `avrxt` (AVR8X), `avrxm`
  (AVR8_XMEGA). Devices with no ATDF take the value of their datasheet
  family. ESP32 / S2 / S3 → `xtensa-lx6` / `xtensa-lx7` / `xtensa-lx7`;
  C3 / C6 → `riscv`; RP2040 → `cortex-m0plus`.
- `flash`, `ram`, `eeprom` (bytes): the ATDF `FLASH` segment, the internal
  SRAM segment (`IRAM` or `INTERNAL_SRAM`), and the `eeprom`-type segment
  (absent → empty). For ESP32 modules: the SoC's SRAM from the module
  datasheet. A module's flash and PSRAM are per part (`N4` = 4 MB flash,
  `R8` = 8 MB PSRAM; G reads them from the part number via the
  datasheet's ordering table). RP2040: flash 0 (external QSPI), ram
  270336 (264 KB).
- `clock_hz`, `vcc_min`, `vcc_max`: the DigiKey `Speed` and `Voltage -
  Supply` of the device's non-V parts. A part whose values differ, such as
  a V or L grade (`ATTINY85V-10PU`: 1.8–5.5 V, 10 MHz), carries its own;
  G writes them as part keys. `census/facts-check.py` confirms each value
  is printed in the datasheet's first three pages (`Operating Voltage`,
  `Speed Grade`). A mismatch is resolved from the datasheet's Electrical
  Characteristics and noted in `src`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Package codes, roles, the ATDF extractor

- [ ] `planning/plan-36-data/package-codes.tsv` from the distinct
      `package_code` values of the `add` rows (§ 3.1). Each code's geometry
      and pin count come from the datasheet's package section.
- [ ] `census/roles.py` (§ 3.2) and `census/pinout-lint.py` (§ 3.4).
- [ ] `census/pinouts.py`: for each `add` AVR row with an ATDF device,
      pick the ATDF pinout whose geometry and pin count equal the row's
      package code (`atdfmatch.geom`/`count`, plus the `plcc` class). Write
      its pins, gaps filled with `NC`, to `pinouts.tsv` (`device pinout
      designator name role src`), and the (device, code) → pinout line to
      `pinout-map.tsv`. Set `pinout_src` = `atdf` in `candidates.tsv`.

Acceptance: the ATDF-covered rows have lint-clean pinouts.
  Check: `python3 -I planning/plan-36-data/census/pinout-lint.py planning/plan-36-data/pinouts.tsv` → `0 problems`, and `awk -F'\t' '$25=="atdf"' planning/plan-36-data/candidates.tsv | wc -l` equals the `atdf` count `pinouts.py` prints (1242 before A's verdicts; the new figure and the reason for any change go in § Corrections) (est. 2 min).
Commit: —

### Phase 2 — Transcriptions

- [ ] `census/units.py` → the (PDF, figure) list for rows without an ATDF
      pinout, written to `transcribe-units.tsv`.
- [ ] Transcribe each unit (§ 3.3) into `transcribed.tsv`. `pinouts.py
      --transcribed transcribed.tsv` merges them; `pinout_src` =
      `transcribed`.
- [ ] The 13 ESP32 module tables and the RP2040 table (§ 3.3) go in
      `transcribed.tsv` too, with `pinout_src` = `module-table` for the
      modules and `transcribed` for the RP2040.

Acceptance: every `add` row has a pinout.
  Check: `awk -F'\t' 'NR>1 && $24=="add" && $25==""' planning/plan-36-data/candidates.tsv | wc -l` → 0, and the lint → `0 problems` (est. 2 min after the transcriptions).
Commit: —

### Phase 3 — Checks and device facts

- [ ] `census/pinout-check.py` (§ 3.4 name check) over every ATDF pinout
      used. A failure is read on the figure and either fixed (ATDF wrong:
      transcribe instead, `src` notes the ATDF difference) or recorded as
      a datasheet typo in `pinout-checks.tsv`.
- [ ] The ten full checks (§ 3.4), recorded in `pinout-checks.tsv`.
- [ ] `census/devices.py` → `devices.tsv` (§ 3.5); `census/facts-check.py`.
- [ ] `planning/plan-36-data/README.md`: the B files.

Acceptance: the checks pass and every device has facts.
  Check: `awk -F'\t' '$5!~/^(ok|full ok|typo:)/' planning/plan-36-data/pinout-checks.tsv | wc -l` → 1 (the header), and `python3 -I planning/plan-36-data/census/facts-check.py planning/plan-36-data/devices.tsv` → `0 mismatches` (est. 5 min).
Commit: —

## Validation Plan

- Tests: none (data). `pinout-lint.py`, `pinout-check.py` and
  `facts-check.py` are the checks; the backend loader re-checks every pinout
  in G.
- Coverage check: Phase 2's `0` empty `pinout_src` for `add` rows.
- Runtime proof: G loads the generated file.
- Doc sync: `planning/plan-36-data/README.md`.
- Final gate: plan-36-I.

## Decisions

- **ATDF as the pin source** — author: the device packs are Microchip's own
  machine data for its tools (Apache-2.0), and they cover 1242 of the 1468
  AVR rows at planning time. Transcribing 455 figures by hand would cost
  days and would be more error-prone. The datasheet stays the part's
  `datasheet` and the reference every pinout is checked against (§ 3.4).
- **Bare pad names, no `/RESET` suffix** — author: the ATDF marks the reset
  pad only on the newer parts (§ 2). One rule for every AVR is clearer than
  half the parts showing it. Rejected: an `alt` pin key for alternate
  functions, which would change the part-file format for every kind and has
  no app UI yet.
- **Exposed pads**: an AVR QFN's exposed pad is not a numbered pin in its
  datasheet or ATDF, so it stays an unnumbered `centre` pad in the
  footprint (C/D). The RP2040 and the ESP32 modules number theirs (57, 39,
  …), so it is a `ground` pin there — author, following each datasheet.

## Corrections

## Summary

B's risk is pin correctness: a wrong pin is a wrong board. The ATDF
covers most parts with the maker's own data. The rest are transcribed by
hand and checked against the figures. The name check runs over every ATDF
pinout used, and ten popular devices get a full check.
