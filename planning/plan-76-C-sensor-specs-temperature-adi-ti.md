# plan-76-C: The specs rules and `specs-check76.py`; temperature specs, ADI and TI

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-B

C sets the rules every sensor specs letter (C–K) follows (§ 3), writes the checker that
enforces them, and fills `specs-temperature.tsv` for ADI (incl. Maxim) and TI: the pinout of each
(datasheet, package) as a part-file pinout, the facts of plan-76-L § 3.1, and the footprint the
package lands on. ADI (≤ 300) and TI (≤ 258) are about three quarters of the temperature rows
(plan-76-A table).

References:

- plan-76-B § 3.3 (`specs-<family>.tsv` columns); plan-76-L § 3.1 (the device keys and units
  these facts feed).
- `docs/parts-format.md` § Pinouts and pins (roles, `group`, `side`, `activeLow`), § Footprints,
  § The `"footprint"` part key.
- Precedents: `planning/plan-74-B-ethernet-specs-microchip.md` § 3 (pin-role table, footprint
  naming, `specs-check.py`); `planning/completed/plan-48-B-timer-opamp-specs.md` (pinout files per
  (datasheet, package)); `Parts/serial.json` device `XR21B1411` (an unnumbered exposed pad is the
  footprint's centre pad, not a pin).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-B complete | `ls planning/completed/plan-76-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25: B not started) |
| The skeleton has ADI and TI rows | `awk -F'\t' 'NR>1 && ($1=="adi" \|\| $1=="ti")' planning/plan-76-data/specs-temperature.tsv \| wc -l` → above 0 | NOT MET (written by B) |

C needs only the kind's key names and units, which plan-76-L § 3.1 fixes in writing; L's code
lands later (it waits for plan-68), and nothing in C runs it.

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `adi` and `ti` row of `specs-temperature.tsv` has `pinout` = a file
  `planning/plan-76-data/pinouts/<datasheet>-<package>.json` (one JSON array of pins), `measures`,
  `output`, `ranges`, `accuracy` (where printed), `address` (I2C parts), `vcc` and `footprint`
  (an existing `footprints.json` id, or a new id that D adds, listed in `footprints-needed.txt`).
- `census/specs-check76.py` passes on those rows.
- A row whose package has no pin table in its PDF becomes `skip` (`no pinout`) in `verdicts.tsv`
  and `datasheets/skipped-parts.md`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No footprint drawn here (D draws the temperature footprints).
- No pin function beyond the datasheet's default (power-up) function.

## 2. Current State

`specs-temperature.tsv` has its skeleton from B. Parts seen at planning (the census page `b518`'s
first 100 rows; A's listing gives every row): TI 52 rows — LM20, LM335A, LM45, LM50, LM60, LM61,
LM70, LM73, LM75A, LM75B, LM94021, LM94022 (SC-70, SOT-23, TO-92, MSOP, SOIC, DSBGA); ADI 10 rows
— TMP35/36/37 (TO-92, SOT-23, SOIC), MAX31875, MAX6605, MAX6612, MAX6613. The temperature census packages (`show.py
raw/b518.json "Package / Case"`): 8-TSSOP/MSOP 141, 8-SOIC 121, TO-92-3 59, SC-70-5 34, SOT-23-3
34, 10-MSOP 33, 8-WFDFN EP 30, 16-SSOP 29, SOT-23-6 28, 10-VFDFN 23, TO-39 metal can 23, module 19,
TO-46 metal can 19. Existing footprints that may match (`footprints.json`, 2026-11-25):
`SOIC-8` and 16 vendor SOIC-8 variants, 13 `MSOP*`, 31 `SOT23*`, 12 `SC70*`, 5 `TO-92-3-*`,
4 `SON-*`, 8 `TSOT*`, 31 `DFN*`; no `TO-39`, `TO-46` or `LGA-14`
(`python3 -I -c "import json;ids=[f['id'] for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']];print(sum(i.startswith('TO-39') for i in ids), sum(i.startswith('TO-46') for i in ids))"` → `0 0`).

## 3. Design (the rules of every specs letter)

### 3.1 Pin roles

| Datasheet pin | Role |
|---|---|
| supply the board feeds (V+, VDD, VCC, VDDIO, +EXC, VS) | `power` |
| GND, VSS, −EXC, a numbered exposed pad that is ground | `ground` |
| digital input (SCL, CS, SCLK, SDI, ADDx, address straps, enable) | `input` |
| digital output (SDO/MISO, ALERT, INT, DRDY, open-drain ALERT/OS) | `output` |
| bidirectional (SDA, DQ of 1-Wire, SDIO) | `inout` |
| analog output (VOUT, OUT of a Hall switch or linear sensor, bridge +OUT/−OUT) | `output` |
| measured path of a current sensor (IP+, IP−) and remote-diode pins (D+, D−) | `inout` |
| filter / reference / bypass pins (FILTER, VREF out, CAP) | `inout` |
| NC, DNC, reserved, factory test | `nc` |

Pin names are the datasheet's; an overbar becomes `activeLow` (`docs/parts-format.md` § Active-low
pins). Bus pins get `"group"`: `i2c`, `spi`, `1wire`. An exposed pad the datasheet does not
number is the footprint's unnumbered centre pad, not a pin.

### 3.2 Facts

- `measures`: the quantities of plan-76-L § 3.1 that the datasheet's title or feature list names,
  limited to the family's quantities. A part's family is its DigiKey category's (plan-76-A
  § Decisions).
- `output`: the plan-76-L § 3.1 names (`i2c` for SMBus, `analog` for a voltage or current output).
- `ranges`: the specified operating measurement range (temperature: the accuracy-specified range
  if the datasheet gives one, else the operating range), in the unit of plan-76-L § 3.1.
- `accuracy`: the best typical-free (guaranteed, max) accuracy over the datasheet's main range,
  ± in the unit. Absent when the datasheet prints only a typical.
- `address`: every 7-bit I2C address the part can take (address pins and part-number options).
- `vcc`: the recommended operating supply range in volts `[min, max]`.
- `extra`: family keys (`axes`, `pressureType`, `switchType`, `isolated`) as `key=value;…`.

### 3.3 Footprints

`footprint` is the `footprints.json` land pattern matching the datasheet's package drawing (body,
pitch, lead count, exposed pad within 0.05 mm; `docs/parts-format.md` § Footprints). Otherwise a
new id `<STYLE>-<pins>-<maker code>` (the precedent's naming: `MSOP-8-ADI-RM`, `SON-6-TI-DRV`,
`TO-46-3-ADI-H`), listed in `footprints-needed.txt` with its drawing reference (file, page, drawing
number). A TO-92 lead form (straight, formed) is its own footprint, as the existing
`TO-92-3-ONSEMI` / `-FORMED` pair.

### 3.4 `census/specs-check76.py`

Copied from plan-48's `specs-check.py` and adapted. For every filled row of the named family:
- the pinout file exists, and its pin count equals the package's lead count (unnumbered pads
  excluded);
- every role is one of the seven roles; at least one `power` and one `ground` pin unless `output`
  is only `bridge` / `capacitance` / `resistance` (plan-76-L § 3.1);
- `measures` and `output` hold only plan-76-L names and satisfy the family rules; `ranges` and
  `accuracy` name measured quantities; `extra` keys belong to the family;
- `vcc` is present unless passive; `footprint` is in `footprints.json` or `footprints-needed.txt`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The checker

- [ ] `planning/plan-76-data/census/specs-check76.py` per § 3.4, with `--family <f>` and
      `--maker <m>`.

Acceptance: the checker rejects a bad row and passes a good one.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --self-test` → `self-test ok` (a built-in good row and one bad row per rule) (est. < 1 min).
  Result:
Commit:

### Phase 2 — ADI temperature

- [ ] Fill every `adi` row of `specs-temperature.tsv`, its pinout files, and its new footprint
      ids in `footprints-needed.txt`.

Acceptance: every ADI row passes.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family temperature --maker adi` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

### Phase 3 — TI temperature

- [ ] The same for every `ti` row.

Acceptance: every TI row passes.
  Check: `python3 -I planning/plan-76-data/census/specs-check76.py --family temperature --maker ti` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

## Validation Plan

- C's checks are its three acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **accuracy is the guaranteed maximum**, not a typical: it is the number a designer
  can rely on, and a typical-only datasheet leaves the fact out rather than overstate it.
- Author: **remote-diode and current-path pins are `inout`**: they are analog terminals the part
  neither only drives nor only reads.
- Author: **one pinout file per (datasheet, package)**, shared by every part number of that
  package, as plan-48 and plan-74.

## Corrections

## Summary

C fixes the specs rules for all sensor families, writes their checker, and fills the ADI and TI
temperature rows.
