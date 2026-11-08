# plan-24-A: ADCs, DACs and RTCs — census, datasheets and ADC part data

Last updated: 2026-11-07
Overall Effort: huge (> 3d) — plan-24 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-24 adds the Texas Instruments analog-to-digital converters (ADCs),
digital-to-analog converters (DACs) and real-time clocks (RTCs) that DigiKey
has in stock with status Active or Last Time Buy, on SPI, I2C or a parallel
bus (RTCs: every interface). The simulator carries logic levels only, so the
converters are modeled on their **digital side**: the bus protocol, the
registers and commands, conversion timing and the BUSY/DRDY/EOC signals. An
ADC conversion returns the code of a 0 V input; a DAC stores what it is
written and drives no analog output. RTCs keep time from simulated time.

The single behavioral outcome of plan-24: loading the part files gives the
baseline count + N24 parts (`add` rows of `planning/plan-24-data/parts.tsv`);
a 6502 or bus-master test circuit can write and read back every new part's
registers, start a conversion and see its completion signal and result code;
RTC parts count seconds of simulated time; `ctest` passes T0 + 2 programs
(`converter_test`, `rtc_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | download ADC datasheets; ADC rows of `parts.tsv`, `specs.md`; skipped parts | large |
| B | DAC and RTC datasheets and part data | large |
| C | the 79 new footprints | large |
| D | `converter` kind: SPI/I2C front ends (from plan-12), the declarative frame/register/command engine, `converter_test` | large |
| E | parallel-bus front end; parallel ADC and DAC behavior | large |
| F | `rtc` kind: timekeeping, alarms, battery-backed RAM, I2C and parallel buses; `rtc_test` | large |
| G | ADC parts (`converter.json`) | large |
| H | DAC and RTC parts | medium |
| I | app: the two kinds in the drawer and sidebar, register view; plan-24 landing tier | medium |

Outcome of A: `datasheets/adc/` holds every TI PDF the ADC `add` rows cite;
`parts.tsv` has one row per ADC candidate with `add`/`skip`; `specs.md` has
one section per ADC device in the form plan-24-D §3 defines.

References:

- `AGENTS.md`; `docs/parts-format.md` (§ `serial`, the `"bus"` object of the
  I2C/SPI targets; § Footprints; § Supply voltage); `docs/backend.md`.
- `backend/src/models/serial_target.h` / `serial_target.c` — the SPI target
  and I2C target front ends (plan-12-I §3.1) plan-24 reuses.
- `planning/plan-24-data/candidates.tsv` (census), `census/`,
  `footprints-needed.txt`.
- `planning/completed/plan-12-I-bridges.md` (target front ends),
  `plan-12-K-app.md` (terminal window, the pattern for a part window),
  `plan-10-J-app.md`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p24 && cmake --build /tmp/logic-p24 -j8 && ctest --test-dir /tmp/logic-p24` → `100% tests passed` | MET; **T0 = 24** (`100% tests passed, 0 tests failed out of 24`, 2026-11-07 at 6e40f67) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `footprints.txt` | MET; **P0 = 11126**, **F0 = 423** (2026-11-07 at 6e40f67) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p24-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-07 at 6e40f67) |
| SPI/I2C target front ends exist | `grep -c 'SerialSpi\|SerialI2c' backend/src/models/serial_target.h` → ≥ 2 | MET (32, 2026-11-07 at 6e40f67) |
| No `converter`/`rtc` kind yet | `ls schematical/schematical/Parts/{converter,rtc}.json 2>/dev/null` → no output | MET (no output, 2026-11-07 at 6e40f67) |
| DigiKey script works | `node scripts/digikey.js --in-stock --manufacturer 296 --limit 1 ADS1115IDGSR` → a part line | MET (`ADS1115IDGSR Active 31952`, 2026-11-07; needs the shell outside the sandbox and the `DIGIKEY_*` exports of `~/.zshrc`) |

plan-24 waits for no other plan (plans 19–22 run in any order, owner
2026-11-07; plan-24 touches none of their kinds).

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all rows.

## 1. Goal

- `awk -F'\t' 'NR>1 && $1=="ADC"' planning/plan-24-data/parts.tsv | wc -l` → 1798 (every ADC candidate: 1718 + 80 the stock re-check found, Corrections), each `add` or `skip` with a reason.
- `ls datasheets/adc/*.pdf | wc -l` = distinct `pdf` values of ADC `add` rows.
- No code change in A.

### Non-goals

- No analog voltages anywhere (owner): no analog input value, no DAC output
  voltage, no analog nets.
- Out of scope (owner): high-speed converters with JESD204, LVDS or CML
  outputs (`interface` = `highspeed` in the census), and converters with no
  SPI/I2C/parallel interface (`other`: UART, PWM-output, …) — `skip` with
  reason "out of scope (interface)".

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| ADC candidates (TI, in stock, category 700) | 1718 | `awk -F'\t' 'NR>1 && $1=="ADC"' planning/plan-24-data/candidates.tsv \| wc -l` |
| … status Active or Last Time Buy | 1365 (Active 1322, LTB 43; Obsolete 317, Discontinued 23, NRND 13 excluded) | `… \| cut -f5 \| sort \| uniq -c` |
| … in scope (SPI, I2C, parallel; addable status) | 1151 in 460 base products | `awk -F'\t' 'NR>1 && $1=="ADC" && ($5=="Active"\|\|$5=="Last Time Buy") && $2~/^(spi\|i2c\|parallel\|parallel\+spi\|i2c\+spi)$/' … \| wc -l` |
| … by interface | SPI 790, parallel 197, I2C 114, parallel + SPI 49, I2C + SPI 1; out of scope: high-speed 169, other 45 | `cut -f2` count of the addable rows |
| DAC candidates (category 701) | 945; addable 798; in scope 737 in 232 base products (SPI 448, parallel 182, I2C 62, I2C + SPI 44, parallel + SPI 1; high-speed 41, other 20) | same commands with `$1=="DAC"` |
| RTC candidates (category 690) | 44; addable 17 (Active 9, LTB 8) in 8 base products: BQ32000 2, BQ32002 2, BQ3285 1, BQ4802 6, BQ4845 1, DP8570 1, DP8573 1, unnamed base 3 | `awk -F'\t' 'NR>1 && $1=="RTC" && ($5=="Active"\|\|$5=="Last Time Buy"){print $4}' … \| sort \| uniq -c` |
| Packages with no footprint (in-scope addable rows) | 79 packages, 1001 rows | `planning/plan-24-data/footprints-needed.txt` |

Known census gap: DigiKey caps a search at 300 results; the ADC search
returned 1718 of the 1808 it counted (2026-11-07). A Phase 1's per-base-name
`--in-stock` check is the authoritative stock check; rows it finds that
are not in `candidates.tsv` are added and counted in Corrections.

### Verified properties

| Claim | How verified |
|---|---|
| The SPI and I2C pin protocols already exist as reusable front ends that hand a model whole bytes | `backend/src/models/serial_target.h` header comment: `SerialSpi` (CS, SCLK, MOSI, MISO; modes 0–3), `SerialI2c` (START/STOP, address ACK, bytes, open-drain SDA), `SerialHost` loaded from a device's `"bus"` object, address straps |
| No candidate is already in the catalog | name set ∩ `Parts/*.json` names → 0 (2026-11-07) |

## 3. Design Overview (all of plan-24)

### 3.1 Kinds

- `converter` (ADCs and DACs; `function` `"ADC"` / `"DAC"`, drawer
  `["Interface", "ADC"]` / `["Interface", "DAC"]`).
- `rtc` (`function` `"RTC"`, drawer `["Interface", "RTC"]`).

### 3.2 One declarative converter model, not 692 hand-written ones

460 ADC and 232 DAC base products differ in protocol details but share a
small set of mechanisms. The `converter` device describes its digital side
in data (plan-24-D §3): the bus, the frame layout (simple ADCs: CS falling
starts a conversion, N bits shift out), a register map (address, width,
reset value, read/write/read-only bits), a command set (opcodes that start,
stop, reset, read data, write/read registers), the conversion time per
setting, the ready signals (BUSY, DRDY, EOC, ALERT), and for DACs the input
register / DAC register pair with LDAC/CLR. The engine runs it. A datasheet
feature the schema cannot express is recorded in `specs.md` under
"not modeled" and in Corrections; nothing outside the digital side is
modeled by design (§1 non-goals).

Rejected alternatives:

- **One C model per chip family** (ADS1x1x, ADS8xxx, DAC8x6x, …): dozens of
  models with the same register/command plumbing; the data form is
  checkable against the datasheet's register map line by line.
- **Analog values on nets**: the owner chose digital side only.

### 3.3 Conversion result

An ADC conversion returns the code of a 0 V input on every channel: 0 for
unipolar straight binary, 0 for bipolar two's complement, mid-scale
(`1 << (bits-1)`) for bipolar offset binary (the datasheet's output-code
table names the format; `specs.md` records it per device).

### 3.4 Conventions

- Inclusion: in stock, status Active or Last Time Buy (owner, 2026-11-07),
  in the datasheet's ordering table, interface in scope; others `skip` with
  the reason, recorded in `datasheets/skipped-parts.md` § TI ADCs / DACs /
  RTCs. A PDF with no addable part is deleted and recorded in
  `datasheets/not-included.md`.
- No logic family: converters and RTCs carry `vcc` (the digital supply
  range, DVDD/IOVDD where separate; `inputMax` from the recommended digital
  input range) like the serial kind. Analog supply pins (AVDD, REF) are
  `PIN_POWER` pins the voltage check ignores for logic levels.
- Device ids: the TI base product (`ADS1115`, `DAC8562`, `BQ32000`);
  variants with one pinout and one register map share a device.

## 4. Detailed Design (this sub-plan)

- Datasheets: `datasheets/adc/`, one PDF per base product,
  `https://www.ti.com/lit/ds/symlink/<lowercase base>.pdf` (`curl -sfL -o`);
  fallback the DigiKey `datasheet_url` when `file` says PDF.
- `planning/plan-24-data/parts.tsv`: `group name device package code pins
  footprint flags status pdf decision reason`. `specs.md`: per ADC device —
  PDF pages; pinout per package; `bus` (SPI mode, I2C address and straps, or
  parallel pins); frame or register map and commands in the plan-24-D §3
  schema; conversion times; ready signals; output code format; digital VCC
  and input max; a "not modeled" list.
- Work order: SPI simple-frame ADCs first (no registers), then SPI register
  ADCs, I2C, parallel, so D/E can test against finished sections.

## Phases

> **Status discipline:** tick with command and output; `Commit:` when landed;
> Corrections for differences; current branch; only this plan's files.

### Phase 1 — ADC stock and datasheets

- [x] Stock re-check (Corrections: category listings instead of 460 per-base calls): `census/catlist.sh` → `list-700-active.tsv` 1323 of 1412 (`Number of Bits=12, series=-: 389 parts and no way to split it`), `list-700-ltb.tsv` 43, `list-701-*` 792 + 6, `list-690-*` 9 + 8 (`stock/`); every in-scope Active/LTB census row is in a listing (`an2.py`: 0 missing); 1 status refresh (`ADC0848CCVX/NOPB` Obsolete → Active); `census/kw.sh 700 ADS8 THS TLC TLV …` recovered 80 of the 89 truncated 12-bit parts, appended to `candidates.tsv` by `census/addrows.py` (`added 80`; 2787 rows).
- [x] Download the PDFs (`census/dl.sh`, 496 files, 440 distinct documents by MD5, `census/canon.py` names each by the key most rows cite). Check: `ls datasheets/adc/*.pdf | wc -l` → 440 = distinct `pdf` of ADC `add` rows (440). 7 bases have no TI datasheet (`ti.com/product/<P>` → 404: THS1041, TLV5535-Q1, ADC10664, ADC76, ADC700, ADS1210, ADS1212 — the last two are in `ads1211.pdf` / `ads1213.pdf`).

Acceptance: both checks (the call quota sets the pace).
Commit: a1c3238

### Phase 2 — ADC part data

- [x] ADC rows of `parts.tsv` (1798, Corrections), ADC sections of `specs.md` (516, generated from `specs/*.json`), `README.md`. Checks: `census/checks.py ADC` → `ADC: 1798 rows (candidates 1798); undecided 0; add without footprint 0; highspeed/other added 0`; `census/validate.py` → `516 spec files, 0 with problems`; `census/coverage.py ADC adc` → `1798 rows, 1192 add, 516 devices, 432 PDFs, 432 cited; 0 problems`.
- [x] `datasheets/skipped-parts.md` § TI ADCs (`census/skipped_md.py ADC` → `606 skipped rows in 18 reasons`); 8 PDFs with no addable part deleted and recorded in `datasheets/not-included.md` (`ls datasheets/adc/*.pdf | wc -l` → 432).
- [x] Spec writing: 55 batches of about 10 devices by subagents against `schema.md` and `/tmp/p24/brief.md` (4 pilots first; the brief and schema were extended after each wave), plus 3 fix-up batches (revision splits, per-part I2C addresses, 2-wire parts, multi-frame register sequences). Every spec validates (above).

Acceptance: the checks above (minutes).
Commit: —

## Validation Plan

Landing tier at the end of I. A has no code; its gate is Phase 2's checks.

## Decisions

- **Analog side not modeled** (owner, 2026-11-07): conversions return a fixed
  value (the 0 V code, §3.3, author's choice of which value); DAC writes are
  stored and produce nothing visible beyond read-back.
- **Interfaces: SPI, I2C, parallel** (owner, 2026-11-07); high-speed
  JESD204/LVDS converters out.
- **RTCs: all of them** (owner, 2026-11-07) — 17 addable parts in 8 base products.
- **Only Active and Last Time Buy parts** (owner, 2026-11-07).
- **Plan number 24** (owner, 2026-11-07; plan-23 is the pin-header plan).

## Corrections

- **Stock re-check by category listing, not per base name** (2026-11-07). The
  plan's 460 per-base calls (+240 for B) exceed the shared quota (473 left at
  the start, other sessions drawing on it). `--category 700/701/690
  --manufacturer 296 --in-stock --status 0` and `--status "Last Time Buy"`
  list DigiKey's whole in-stock Active/LTB set in 71 calls and check the same
  thing (stock > 0, status) for every row at once, including rows the census
  missed. Its one gap is DigiKey's 300-result cap on the unsplittable
  "12-bit, series -" bucket (389 Active ADCs): prefix keyword searches inside
  the category (`ADS8`, `THS`, `TLC`, `TLV`, …) recovered 80 of the 89 parts
  not listed; **9 remain unidentified** (DigiKey shows no names past 300; no
  prefix tried — ADS7, ADC1, ADS1, ADS5, ADS6, ADS4, ADC3, ADS2, ADC0, AMC,
  5962, ADC7, LM, SM7, MSP — found them). An `ADCS` keyword matched the whole
  category (43 calls).
- **Census grows 1718 → 1798 ADC rows** (`added 80`), all Active, in scope
  (SPI/parallel), bases ADS8634/61/64/65/68, THS1206/07/082/09/40, TLC1225,
  TLC2543/51/52/54/55/58/74/78, TLV2541–2556. In scope after the status and
  interface filters: 1225 rows in 486 base products (was 1151 / 460).
- **Datasheet names** come from DigiKey's `lit/gpn`/`symlink` link (doubly
  URL-encoded for some -Q1 parts), not from the base name: -Q1, -EP and
  National-era parts have their own documents (`ads1113-q1.pdf`,
  `adc0832-n.pdf`). Several names are one document (`ads1113/1114/1115.pdf`
  are byte-identical): 496 downloads, 440 documents.
- **The ordering table decides `add`** (§3.4 "in the datasheet's ordering
  table"): `census/poa.py` parses each datasheet's Package Option Addendum
  (part, TI status, package type, designator, pins); `census/rows.py` matches
  every in-scope row (1197 in their own datasheet, 6 in a sibling's), 22 rows
  are in none (skip). The TI designator + pin count gives the footprint
  (`footprint-ids.tsv`); 86 ADC packages, 57 of them new.
- **Device schema made exact** (`planning/plan-24-data/schema.md`): plan-24-D
  §3 was a sketch, too loose to transcribe 700 datasheets against. The exact
  form adds a bit-pattern language for frames and commands, `stream`/`fixed`
  framing, `reply`/`next` read-back, the `clocked` bus style (free-running
  pipelined ADCs and delta-sigma modulators, which most "parallel" census
  rows are), `configBus` (parallel + SPI parts) and the `analog` pin list.
  **The voltage check needs a change**: it judges every `power` pin against
  the part's `vcc`, so analog supplies (AVDD at 5 V on a 3.3 V DVDD part)
  would be flagged; §3.4 assumed they are ignored. plan-24-D gets the task.
  `census/validate.py` checks spec files against the schema.
- **Spec-writing findings** (55 batches): the schema grew, wave by wave,
  2-wire SPI with no CS, multi-field conversion-time tables, `onRead`,
  `startOnEnter`, `pins.levels`, result fields and split result registers,
  write-1-to-clear bits, clears-on-either-event ready pins, ADC
  `channelMap`, per-package and per-part (`partKeys`) I2C addresses,
  pin/field-selected code formats, DDR and multiple/divided clock outputs,
  end-aligned command decoding, SPI without MISO / 3-wire SDIO, `next` for
  I2C reads, set/clear-bit actions, padded register words, command `when`
  conditions (multi-frame programming sequences, unlock keys) and
  field-selected output frames (`outs`), pointer auto-increment. The engine
  (D, E) implements all of them; `schema.md` is the list.
- **Census rows the datasheets moved**: `ADS1014QNKSRQ1`/`ADS1015QNKSRQ1`
  are ADS1014-Q1/ADS1015-Q1 (DigiKey filed them under ADS1013);
  `TLC3545IDR` is TLC3545 (not "TLC354"); four base products are two chip
  revisions each and became two devices (ADS114S06/ADS114S06B,
  ADS114S08/ADS114S08B, ADS8684/ADS8684A, ADS8688/ADS8688A,
  ADS8588S/ADS8588H, ADS8598S/ADS8598H) — `census/devices.py` OVERRIDE.
- **Device-level skips found in the datasheets**: ADS114S18/ADS124S18 have
  only a 7-page short-form datasheet (no pinout or registers); ADS5403,
  ADS5440, ADS5444, ADS5463, ADS54RF63, ADS5474 have LVDS data outputs
  (out of scope) although DigiKey lists them as Parallel. 11 rows; their 8
  PDFs deleted (`datasheets/not-included.md`). ADC add rows: 1192 in 516
  devices.
- **`specs.md` becomes `specs/<DEVICE>.json` + a generated `specs.md` index**:
  one JSON file per device holds the D §3 form; `specs.md` lists every device
  with its datasheet, bus, counts and not-modeled list (generated by
  `census/specs_md.py`).

## Summary

A turns the ADC half of the largest census in the repo into datasheets and
a reviewed part table written in the schema D will execute.
