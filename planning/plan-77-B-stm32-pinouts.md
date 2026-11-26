# plan-77-B: STM32: `gen-stm32.py` — pinouts, devices and facts from ST's pin data

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-77-A

B turns the XML files A copied (`planning/plan-77-data/opd/`) into the 300 parts' devices and
pinouts, checks every pinout against its datasheet's pin table, takes the facts the XML lacks
from the datasheets, and lists the footprints C and D must draw. Its output is staging data in
`planning/plan-77-data/`; no shipped file changes (F merges it).

References:

- `planning/plan-77-A-stm32-data.md` (§ Prerequisites, the pick); `planning/plan-77-data/README.md`
  (the measured shape of the XML).
- `docs/parts-format.md` § Pinouts and pins (line 150; `side`, line 177; the more-than-40-pins rule,
  line 784), § `mcu` (line 2889).
- Precedent: `planning/completed/plan-36-B-mcu-pinouts.md` (ATDF pinouts, `roles.py`,
  `pinout-lint.py`, the datasheet text check) and its tools in `planning/plan-36-data/census/`.

## Prerequisites

plan-77-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-77-A complete | `ls planning/completed/plan-77-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| A's outputs | `python3 -I planning/plan-77-data/census/pick.py --check` → `add 300`; `ls planning/plan-77-data/opd/LICENSE` → exists | NOT MET (A not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-77-data/devices.json`: one `mcu` device object per device of § 3.1 (no
  `footprint` values yet: F fills them from C and D), with every `add` part under it.
- `pinout-checks.tsv`: every pinout `pass` against its datasheet's text, or `eye` with the page
  and figure read by hand.
- `facts.tsv`: clock, flash, RAM, EEPROM and VDD range per device, each with its source.
- `footprints-needed.txt`: every package the `add` parts use, with its datasheet figure.

### Non-goals

- No change to `Parts/`, `backend/` or the app. The `core` values `devices.json` uses are E's to
  add to the loader; B writes them as E will accept them (§ 3.3).
- No alternate-function data: a pin's name is its primary name (§ 3.2). The XML's `<Signal>` lists
  are not carried.

## 2. Current State

There is no STM32 pinout. The XML (README § STM32_open_pin_data): one file per RefName with its
package, core(s), flash per letter, RAM, and `<Pin Name Position Type>`; 193 files bond several
names to one position; 725 of 2,050 MCU files have no `<Frequency>` / `<Voltage>`. The existing
`mcu` pinouts (AVR, `planning/plan-36-data/pinouts.tsv`) are named `p<N>` (dual rows), `quad<N>`
(four sides, the exposed pad counted), `bga<N>`, `wlcsp<N>`; port pins have `group`
`port-<letter>`; RESET is `input` with `activeLow`.

## 3. Design

### 3.1 Devices

A device is one (Line, Die, RAM) of the `add` parts: `Line` and `Die` from the XML, RAM the
part's (the XML gives one RAM per flash letter; a part takes the one of its letter). Parts of a
device differ only by flash (part key `"flash"`), package and packaging. `id` =
`stm32<line digits/letters lower>-<die lower>[-<ram>k]` (the RAM suffix only when one (Line, Die)
has two RAMs), e.g. `stm32f103-die410`. `function` = the Line (`STM32F103`); `description` =
`"Arm Cortex-M3 MCU, 72 MHz"` (core and clock from § 3.3); `drawer` =
`["Microcontroller", "STM32", "<Family without STM32>"]` (`F1`, `L4+`, …); `note` = the RefNames
and `STM32_open_pin_data <COMMIT>`, plus the datasheet pages of any fact from a datasheet.

### 3.2 Pinouts

One pinout per (device, package):

| XML package | Pinout key |
|---|---|
| `TSSOP<n>`, `SO<n>` (SO8N) | `p<n>` |
| `LQFP<n>`, `UFQFPN<n>`, `VFQFPN<n>`, `UQFN<n>` | `quad<n>`, or `quad<n+1>` when the Mcu element says `HasPowerPad="true"`: pad n+1 is the exposed pad, name `VSS`, role `ground` |
| `UFBGA<n>`, `TFBGA<n>`, `LFBGA<n>`, `VFBGA<n>` | `bga<n>` (ball names = XML positions) |
| `WLCSP<n>` | `wlcsp<n>` (ball names = XML positions) |

When one device has two packages with the same key and different pin lists (LQFP48 and UQFN48),
the second package's key takes its name as a suffix: `quad48-uqfn48`.

Pins, one per XML position:

- **Name**: the XML `Name` with a trailing annotation removed (` [PA11]`, ` (PC14)`), e.g.
  `PC14-OSC32_IN`, `PA0-WKUP`, `NRST`. Several entries on one position: an `NC` entry is dropped
  when another entry is not `NC`; the rest are joined with `/` in XML order (`PA9/PA11`).
- **Role** (`census/roles.py`): `I/O`, `MonoIO` → `inout`, `group` `port-<letter>` when the
  first name matches `^P[A-Z][0-9]+`; `Reset` → `input`, `activeLow`, group `program`; `Boot` →
  `input`, group `program`; `NC` → `nc`; `Power` → `ground` when the name starts `VSS` or `GND`,
  else `power` (VDD, VDDA, VBAT, VREF+, VDDUSB, VCAP, VDDSMPS, VLXSMPS: the regulator pins are
  `power`, as the RP2040's DVDD is, docs/parts-format.md § `mcu` Voltage).
- **Side**: pinouts of more than 40 pins: the first half (rounded up) `"side": "in"`
  (docs/parts-format.md line 784's rule); smaller: none.

`census/pinout-lint.py`: every position used once, every role one of the seven, every power pin
set has a `power` and a `ground` pin (`mcu.c`: `an mcu pinout needs a power pin`).

### 3.3 Facts (`facts.tsv`)

- `core`: the first `<Core>`: `Arm Cortex-M0` → `cortex-m0`, `M0+` → `cortex-m0plus`, `M3` →
  `cortex-m3`, `M4` → `cortex-m4`, `M7` → `cortex-m7`, `M33` → `cortex-m33`, `M55` →
  `cortex-m55`. A second `<Core>` (STM32H745/H747/H755/H757: M7 + M4) → `core2` (E's key). Any
  other core string stops the generator with the file name.
- `clock`: `<Frequency>` MHz × 10^6; without one, the datasheet's maximum CPU frequency
  (`f_HCLK` in "General operating conditions"), page recorded.
- `flash`: per part, the XML `<Flash>` of the part's flash letter × 1024; the device's `flash` is
  the smallest of its parts (the ESP32 precedent: a part overrides it).
- `ram`: § 3.1, × 1024.
- `eeprom`: STM32L0 and STM32L1 only (the lines with data EEPROM): the datasheet's "Data EEPROM"
  size, page recorded; every other line 0 (absent).
- `vcc`: `<Voltage Min Max>`; without one, the datasheet's VDD operating range (General operating
  conditions table), page recorded. On the device.

### 3.4 The datasheet check (`census/pinout-check.py`)

plan-36-B's method: for each pinout, the datasheet's pinout chapter (pages from its "Pinouts and
pin description" heading to the next chapter, `pdftotext -layout`) must contain every pin name's
first token (`PC14` of `PC14-OSC32_IN`, both of `PA9/PA11`) at least once, and the package's
figure caption. `pass` → recorded. A miss → the figure is read by eye, recorded `eye` with page
and figure, and a name the datasheet prints differently is corrected in `pinout-overrides.tsv`
(position, datasheet name); the datasheet wins over the XML.

### 3.5 `footprints-needed.txt`

One line per (XML package, datasheet recommended-footprint figure) the `add` parts use:
`package  footprint_id  datasheet  page  figure  drawing_code  parts`. Ids follow § 3.6; the
reuse column names `SOIC-8-ST-SO8N` for SO8N and `TSSOP-14-ST` for TSSOP14 and is `new`
otherwise.

### 3.6 Footprint ids (for C and D)

Measured at planning on `stm32f103c8.pdf` (DS5319 Rev 20): ST's LQFP48 footprint example
(Figure 56, `5B_LQFP48_FP_V1`) has 1.20 × 0.30 pads, 9.70 outer and 7.30 inner extent (pad-centre
span 8.50); the library's `LQFP-48-PT` (TI PT0048A) has 1.6 × 0.3 pads at 8.2. ST's LQFP64
(Figure 52, `5W_LQFP64_FP_V2`): 1.20 × 0.30, 12.70 outer (span 11.50); `LQFP-64-10X10` (TI):
1.5 × 0.3 at 11.4. So the TI-drawn LQFP / QFN / TSSOP-20 footprints are not reused. The only
footprints drawn from ST drawings are `SOIC-8-ST-SO8N` and `TSSOP-14-ST`; those are reused.
Every other package gets `<PACKAGE>-ST` (`LQFP-48-ST`, `UFQFPN-48-ST`, `UFBGA-100-ST`); a second
drawing code for the same package with other dimensions gets `<PACKAGE>-ST-<code>`. A ball package
whose populated balls differ by die gets `<PACKAGE>-ST-<die>` (`WLCSP-49-ST-DIE460`).

### Rejected alternatives

- **One device per XML file**: a line would show as dozens of devices differing only by package;
  the library's devices hold one die with a pinout per package (plan-36).
- **Port-only pin names** (`PC14` instead of `PC14-OSC32_IN`): the datasheet's pin tables use
  the full name; the name is what a user matches against the datasheet.
- **Trusting the XML without a datasheet check**: plan-36-B found two wrong ATDF pinouts in 323;
  the check costs one text scan per pinout.

## 4. Phases

### Phase 1 — Roles, lint, generator

- [ ] `census/roles.py`, `census/pinout-lint.py` (§ 3.2) in `planning/plan-77-data/census/`.
- [ ] `planning/plan-77-data/gen-stm32.py` (§ 3.1–3.3, XML facts only) → `devices.json`.

Acceptance: `python3 -I planning/plan-77-data/gen-stm32.py --check` → `parts 300` and a device
count; `python3 -I planning/plan-77-data/census/pinout-lint.py planning/plan-77-data/devices.json`
→ `0 problems`.

Commit:

### Phase 2 — Datasheet facts and the pin check

- [ ] `facts.tsv`: the datasheet values of § 3.3 for every device the XML leaves without them.
- [ ] `census/pinout-check.py` → `pinout-checks.tsv`; `pinout-overrides.tsv` for each correction.
- [ ] Re-run `gen-stm32.py` with the facts and overrides.

Acceptance: `python3 -I planning/plan-77-data/census/pinout-check.py --summary` → every pinout
`pass` or `eye`, 0 `miss`; `gen-stm32.py --check` → no device without `clock`, `ram`, `vcc`.

Commit:

### Phase 3 — Footprints needed

- [ ] `footprints-needed.txt` (§ 3.5, § 3.6): each package's figure located in a datasheet that
      covers it.

Acceptance: `python3 -I planning/plan-77-data/gen-stm32.py --footprints` → every (device,
package) of `devices.json` maps to a line of `footprints-needed.txt`.

Commit:

## 5. Validation Plan

B's checks are its three acceptance commands (each under a minute). No backend or app check: B
writes staging files only.
