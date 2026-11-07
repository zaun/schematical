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
| Backend tests green | `cmake -S backend -B /tmp/logic-p24 && cmake --build /tmp/logic-p24 -j8 && ctest --test-dir /tmp/logic-p24` → `100% tests passed` | MET; **T0 = 21** (2026-11-07 at 4ae312f) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `footprints.txt` | **P0 = 8022**, **F0 = 237** (2026-11-07 at 7b6b604); re-measure at start |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p24-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | re-run at start |
| SPI/I2C target front ends exist | `grep -c 'SerialSpi\|SerialI2c' backend/src/models/serial_target.h` → ≥ 2 | MET (2026-11-07) |
| No `converter`/`rtc` kind yet | `ls schematical/schematical/Parts/{converter,rtc}.json 2>/dev/null` → no output | MET (2026-11-07) |
| DigiKey script works | `node scripts/digikey.js --in-stock --manufacturer 296 --limit 1 ADS1115IDGSR` → a part line | MET (2026-11-07) |

plan-24 waits for no other plan (plans 19–22 run in any order, owner
2026-11-07; plan-24 touches none of their kinds).

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all rows.

## 1. Goal

- `awk -F'\t' 'NR>1 && $1=="ADC"' planning/plan-24-data/parts.tsv | wc -l` → 1718 (every ADC candidate), each `add` or `skip` with a reason.
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

- [ ] Stock re-check per ADC base name (`node scripts/digikey.js --in-stock --manufacturer 296 --list "<base>"`; 460 base names, about 460 calls — split over two days of the ~1000/day quota); refresh `status`; rows not Active/LTB or out of stock `skip`.
- [ ] Download the PDFs. Check: `ls datasheets/adc/*.pdf | wc -l` = distinct ADC `pdf` values.

Acceptance: both checks (the call quota sets the pace).
Commit: —

### Phase 2 — ADC part data

- [ ] ADC rows of `parts.tsv` (1718), ADC sections of `specs.md`, `README.md`. Checks: 0 undecided ADC rows; 0 `add` rows without a footprint id (existing or a `footprints-needed.txt` package's id); every `highspeed`/`other` row `skip`.
- [ ] `datasheets/skipped-parts.md` § TI ADCs.

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

## Summary

A turns the ADC half of the largest census in the repo into datasheets and
a reviewed part table written in the schema D will execute.
