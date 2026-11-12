# plan-36-G: Microcontrollers — the generator, `Parts/mcu.json`, datasheets, goldens

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-36-F

G writes `census/gen-parts.py`. From `candidates.tsv` (`add` rows),
`pinouts.tsv`, `pinout-map.tsv`, `devices.tsv` and `package-codes.tsv` it
generates `schematical/schematical/Parts/mcu.json`. G merges
`footprints-C/D/E.json` into `footprints.json`, adds the `datasheets.json`
entries, and regenerates the goldens. That is the plan's one golden
change: new lines only.

The single behavioral outcome: loading the part files gives P + one part
per `add` row, where P is the part count when G starts (P0 = 27828 at
7ca1070, plus parts other plans added since). Every new line is
`|mcu|`-kind and ends `|unsimulated`. `git diff backend/tests/golden`
shows only added lines: no existing line changes or disappears.

References:

- plan-36-A § Prerequisites; F § 3.3 (the keys); B § 3 (pinout names,
  roles, device facts); C § 3 (`package` values, ids).
- `docs/parts-format.md` § Devices, § Parts, § `mcu` (F), § Datasheets.
- `docs/backend.md` § Tests (regenerating a golden).
- `planning/plan-35-data/census/gen-parts.py` (the generator precedent).

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-36-F complete | `ls planning/completed/plan-36-F-*.md` → exists | NOT MET (2026-11-12: plan written) |
| `mcu.json` is empty | `python3 -c "import json;print(len(json.load(open('schematical/schematical/Parts/mcu.json'))['devices']))"` → 0 | NOT MET until F |

## 1. Goal

- `mcu.json` holds one device per (device) of the `add` rows, and one part
  per `add` row (1521 in-scope rows and 316 devices before A's verdicts).
- `footprints.json` holds every object of `footprints-C/D/E.json`, and
  `footprint_check` passes (each is used).
- `parts.txt` and `traces.txt` grow by exactly the `add` count, and
  `footprints.txt` by the merged footprint count. `git diff
  backend/tests/golden | grep -c '^-[^-]'` → 0.
- Every new part has `manufacturer`, `datasheet` (a `datasheets.json` id),
  `footprint` and `package`.

### Non-goals (explicit constraints)

- No existing part, footprint or datasheet entry changes.
- No app change (H).
- No new `PartKind`, key or loader rule (F did them).

## 2. Current State

- `Parts/mcu.json` exists with `"devices": []` (F Phase 2).
- The six spike PDFs in `datasheets/mcu/` have no `datasheets.json`
  entries (A § 2). Rows that use one get an entry here, named after the
  file.
- The drawer sorts top-level categories alphabetically
  (`PartCatalogStore.load()`, `Backend/PartCatalog.swift:1376–1392`,
  `localizedStandardCompare`). Leaves group by `function`.

## 3. Design

### 3.1 Devices

- `id`: the device name in lower case with `/` and spaces → `-`
  (`atmega328p`, `esp32-s3-wroom-1`, `rp2040`). It must be unique in the
  kind.
- `function`: the device's display name from `devices.tsv`
  (`ATmega328P`, `ESP32-S3-WROOM-1`, `RP2040`), which is the drawer leaf.
- `description`: `"8-bit AVR microcontroller, 32 KB flash"`; `"Wi-Fi +
  Bluetooth LE module (ESP32-S3)"`; `"Dual Cortex-M0+ microcontroller"`.
  Each comes from a template per family plus the device's flash.
- `drawer`: `["Microcontroller", <family>]`, where `<family>` is
  `candidates.tsv`'s `family`: `AVR ATmega`, `AVR ATtiny`, `AVR tinyAVR
  0/1/2`, `AVR megaAVR 0`, `AVR Dx`, `AVR Ex`, `AVR Lx/Sx`, `AVR XMEGA`,
  `AVR AT90`, `ESP32`, `RP2040`.
- `"simulated": false`; `core`, `clock`, `flash`, `ram`, `eeprom` (omitted
  when 0), and `vcc` from `devices.tsv`. `note`: the ATDF file and pinout
  name (`ATmega328P.atdf TQFP32`) or the transcription `src`.
- `pinouts`: each pinout B mapped for the device's `add` rows. Pins in
  number order (ball pinouts row-major).

### 3.2 Pin layout keys (`group`, `side`)

The block puts power and inputs on the left, outputs and inouts on the
right (`ChipLayout.init(pins:)`, `Canvas/SchematicModel.swift:500–555`).
Without help, an MCU's ports would all stack on the right. The generator
sets:

- `group`: `port-<letter>` for `P<letter><n>` pins; `gpio` for ESP32
  `IO*` / RP2040 `GPIO*` pins; `usb` for `D+ D- DM DP USB_DP USB_DM VBUS
  UCAP`; `program` for `UPDI PDI PDI_DATA RESET RUN SWCLK SWDIO EN`;
  `clock` for `XTAL* TOSC* XIN XOUT`; `qspi` for RP2040 `QSPI_*`; `lcd`
  for `SEG* COM* BIAS* CAP* LCDCAP VLCD`; the rest stay `default`.
- `side`: ports are split so the two columns are about equal. In letter
  order, port groups go left (`"side": "in"`) while the left column's
  inout count stays at or below half of all inout port pins. The rest go
  right (`"side": "out"`). For `gpio`, pins with numbers below the median
  GPIO number go left and the others right. `program`, `clock` and `usb`
  pins keep their role's default side.

### 3.3 Parts

`{"name": <mpn>, "manufacturer": "Microchip Technology" | "Espressif
Systems" | "Raspberry Pi", "datasheet": <id>, "package": <C/D/E package
value>, "pinout": <B's pinout>, "footprint": <package-codes.tsv>}`. Extra
keys:

- `flags`: `["automotive"]` for `grade` = `automotive` (6 rows at planning).
- `vcc` / `clock` when the part's DigiKey values differ from the device's
  (B § 3.5: the V and L grades).
- ESP32 modules: `flash` and `psram` decoded from the ordering code
  (`N4` → 4194304, `N8`, `N16`, `H4` (high-temperature 4 MB) → flash;
  `R2` / `R8` → PSRAM). The decoding is checked against the module
  datasheet's ordering table, which lists the flash and PSRAM of each code.

### 3.4 Datasheets

One `datasheets.json` entry per PDF used by an `add` row: `id` = the file
name in lower case without `.pdf`, `url` = the maker URL A downloaded it
from (`datasheets/mcu/datasheets.tsv`). An entry whose id already exists
is reused, not duplicated. The entries are inserted in `id` order, the
file's existing order.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Generator and footprints

- [ ] `planning/plan-36-data/census/gen-parts.py` (§ 3.1–3.3) →
      `schematical/schematical/Parts/mcu.json` (one line per pin and per
      part, the `cpu.json` layout).
- [ ] Merge `footprints-C.json`, `footprints-D.json` and `footprints-E.json`
      into `Parts/footprints.json` (`census/fp-merge.py --write`).
- [ ] `datasheets.json` entries (§ 3.4).

Acceptance: the library loads with every new part.
  Check: `cmake --build /tmp/logic-p36 -j8 && ctest --test-dir /tmp/logic-p36 -R 'library_check|part_file_test'` → passed, and `/tmp/logic-p36/listing_check --print | grep -c '|mcu|'` equals `awk -F'\t' '$24=="add"' planning/plan-36-data/candidates.tsv | wc -l` (est. 4 min).
Commit: —

### Phase 2 — Goldens

- [ ] Regenerate the goldens as `docs/backend.md` § Tests says
      (`/tmp/logic-p36/listing_check --print > backend/tests/golden/parts.txt`,
      the same with `trace_check` → `traces.txt` and `footprint_check` →
      `footprints.txt`).
- [ ] Check that the diff only adds: `git diff backend/tests/golden | grep
      -c '^-[^-]'` → 0. Every `+` line in `parts.txt` is `|mcu|` and ends
      `|unsimulated`.

Acceptance: every backend test passes, and only lines were added.
  Check: `ctest --test-dir /tmp/logic-p36` → `100% tests passed`; `git diff --numstat backend/tests/golden` → each file `<added> 0` (est. 4 min).
Commit: —

## Validation Plan

- Tests: `listing_check`, `trace_check`, `footprint_check`, `library_check`
  over the full library.
- Coverage check: Phase 1's count equality.
- Runtime proof: H places parts in the app.
- Doc sync: `planning/plan-36-data/README.md` (the generator's inputs and
  how to re-run it).
- Final gate: plan-36-I.

## Decisions

- **Drawer `Microcontroller → <family> → <device>`** — author: matches the
  two-level drawer of every kind (`["Memory", "SRAM"]`), and splits 316
  devices into eleven folders of at most 88 leaves (ATmega).
- **One device per die, V/L grades as parts** — author: the V and L parts
  share the die, the pins and the datasheet. Only the supply range and
  clock differ, and those are part keys (F § 3.3).
- **Ports split across both sides** — author: § 3.2. A 100-pin part in one
  column would be a 1700-unit-tall block. The split halves it, and it
  keeps each port together, which matters more than strict pin order on a
  schematic symbol.

## Corrections

## Summary

G is data generation, gated by the real loader and an only-adds golden
diff. Its one judgement call is the symbol layout (§ 3.2). The app shows it
in H, where it can still change without touching the backend.
