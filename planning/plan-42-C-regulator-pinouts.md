# plan-42-C: Voltage regulators — pinouts and regulator keys per (series, package)

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-42-B

C reads the pin table of every `add` row's PDF and fills `specs.tsv`'s
pinout columns: the pin numbers, names and roles of each (series, package),
and the regulator keys the `regulator` kind needs (plan-42-G § 3.1): which pin
is each element's input, output, enable (and its polarity), power-good and
adjust pin, the analog pins, Vref and `adjustTo` for adjustable parts, and
the minimum input. The behavioral outcome: `pinout-check.py` reports `0
pinout errors` for every spec row an `add` row uses, and one part per
distinct pinout of each maker is hand-checked against its PDF.

References:

- `planning/plan-42-G-regulator-kind.md` § 3.1 (device keys, roles, part
  keys: the target of every column here).
- `docs/parts-format.md` (pin roles; "Thermal pads are not pins"; pins that
  share a name are one node).
- `planning/plan-40-data/census/pinout-check.py`, `specs-fill.py` (the
  precedents C adapts).

## Prerequisites

plan-42-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-42-B complete | `ls planning/completed/plan-42-B-*.md` → exists | NOT MET (2026-11-15: not started) |
| The final pick exists | `awk -F'\t' '$29=="add"' planning/plan-42-data/candidates.tsv \| wc -l` → B's N | NOT MET (2026-11-15: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-42-data/census/pinout-check.py --maker <m>` →
  `0 pinout errors` for each maker, over every spec row an `add` row uses.
- § Corrections lists one hand check per distinct pinout of each maker (spec
  row, PDF page, result).

### Non-goals (explicit constraints)

- No part file, footprint, backend or app change (H writes the parts).
- No pin data from DigiKey (it has none) and no guessing from a package:
  the same SOT-223 is IN-GND-OUT for one series and ADJ/GND-OUT-IN for
  another.

## 2. Current State

- `specs.tsv` (A) has one row per (maker, series, PDF, Package / Case), its
  `pinout`, `pinout_adj`, `regulators`, `analog`, `vref`, `vin_min`,
  `dropout`, `adjust_to` columns empty.
- Pin tables read at planning (`pdftotext -layout` of the three probe PDFs,
  plan-42-A § 2):
  - `lm1117.pdf` Table 6-1 "Pin Functions": `ADJ/GND` pin 1 (adjust pin of
    the adjustable option, ground of the fixed ones), `VIN` 3 (WSON 2, 3,
    4), `VOUT` 2 and TAB (WSON 5, 6, 7, TAB); SOT-223 `VOUT` 2 and 4.
  - `ADP7118.pdf` Table 5 "Pin Function Descriptions": TSOT `VIN` 1, `GND`
    2, `EN` 3 ("Drive EN low to turn off"), `SENSE/ADJ` 4 (sense of the
    fixed parts, adjust of the adjustable), `VOUT` 5; LFCSP adds `SS`; `EP`
    exposed pad tied to GND.
  - `MCP1700-…-20001826E.pdf` Table 3-1 "Pin Function Table": SOT-23 `GND`
    1, `VOUT` 2, `VIN` 3; SOT-89 `GND` 1, `VIN` 2, `VOUT` 3; TO-92 the same as
    SOT-89; DFN-6 `VIN` 1, `GND` 3, `VOUT` 6, `NC` 2, 4, 5, `EP` 7.
  So one (series, package) can carry two pinouts — fixed and adjustable —
  and a pin name like `ADJ/GND` changes role with the option.

## 3. Rules

1. **Names** as the datasheet's pin table prints them, with an option-
   dependent name split: `ADJ/GND` is `GND` on a fixed part and `ADJ` on an
   adjustable one; `SENSE/ADJ` is `SENSE` and `ADJ`; `FB/VOUT` is `VOUT`
   (fixed, the pin senses the output) and `FB` (adjustable). Several pins of
   one name (VIN 2, 3, 4; VOUT and TAB) keep the one name: they are one node.
2. **Roles.** Each element's input pin(s) `power`; GND, PGND, AGND
   `ground`; each element's output pin(s) `output`; enable `input`;
   power-good `output` (open-drain, only when the datasheet says open-drain
   or open-collector; else it is an analog pin); every other connected pin
   (FB, ADJ, SENSE, SS, NR, BYP, BOOT / BST, COMP, RT, SYNC, MODE, VCC bias,
   SW when not the output) takes its electrical role (`input`, `output`,
   `inout`) and goes in `analog`; NC and DNC `nc`; a numbered position with
   no lead `absent`. An exposed pad is not a pin (the footprint carries it).
3. **Two pinouts per spec row** when the series has fixed and adjustable
   options in that package: `pinout` (fixed) and `pinout_adj`; H makes them
   two devices.
4. **The output pin** (`out`): VOUT, OUT, VO or VOS when the part has one;
   else the switch pin (SW, LX, OUTPUT — LM2596's pin 2 is "Output", the
   switch). The library has no inductor, so the model drives the regulated
   level on that pin (plan-42-G § Non-goals).
5. **The enable** (`enable`, `enableActive`): EN, ENABLE, ON, CE → `high`;
   SHDN, SD, ON/OFF where low turns the part on (LM2596, LM2576: "ON/OFF",
   low = on) → `low`; as the datasheet's truth table says. A part with no
   enable pin has none.
6. **Vref and `adjustTo`** (adjustable rows): Vref is the datasheet's typical
   reference / feedback voltage (`vref`, V). `adjustTo` is `output` when the
   datasheet's output formula is Vout = Vref × (1 + R2 / R1) with R1 between
   OUT and ADJ (LM317, LM1117-ADJ, LM350, the Micrel MIC29xx2 "ADJ" parts),
   and `ground` when Vref is the FB pin's voltage to ground (Vout = Vref × (1
   + R1 / R2), R1 from OUT to FB): every FB part.
7. **Minimum input** (`vin_min`, V): a switching row takes the datasheet's
   recommended minimum input; a linear row takes it only when the datasheet
   gives one (else the kind uses Vout + dropout, plan-42-G § 3.3).
   `dropout` (V): empty when DigiKey gives "Voltage Dropout (Max)" (H takes
   it); for a linear row DigiKey gives none for (51 picked rows at planning:
   `awk -F'\t' '$29=="candidate" && $4 ~ /linear/ && $20==""'
   planning/plan-42-data/candidates.tsv | wc -l` → 51), C writes the
   datasheet's maximum dropout at the highest load current it lists.
8. **Elements** (`regulators`): a JSON list in the kind's form, one object
   per regulator in the datasheet's numbering (`IN1`/`OUT1` first), with
   `type` (`linear`, `buck`, `boost`, `buck-boost`, `charge-pump`, `sepic`:
   the datasheet's primary topology; DigiKey's "Boost, Flyback, SEPIC" parts
   are `boost`), `in`, `out`, and the optional keys.
9. **Pinout format:** `[[number, name, role], …]` per position, every lead
   once, numbers as the datasheet's package drawing (BGA balls as `A1`).

`census/pinout-check.py --maker <m>` checks, for every spec row an `add` row
uses: every lead number of the package (pin count from DigiKey's Package /
Case, e.g. `SC-74A, SOT-753` → 5; `TO-261-4` → 4) appears once; every
`regulators` name names a pin of every pinout; roles follow rule 2 (input
`power`, output `output`, enable `input`); `analog` names exist; an
adjustable element has `vref` and an `adjust` pin, a fixed one neither;
`adjust_to` is set for every adjustable row. It prints `<n> pinout errors`
with one line per error.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Texas Instruments

- [ ] `census/pinout-check.py` (§ 3) and `specs-fill.py`'s pinout columns.
      Smoke run on the empty columns prints its error count.
- [ ] Read each TI spec row's pin table (pin configuration figure and "Pin
      Functions" table); fill the columns by § 3 (`specs-ti-pins.tsv`,
      merged by `specs-fill.py`).

Acceptance: every TI spec row an `add` row uses is complete and consistent.
  Check: `python3 -I planning/plan-42-data/census/pinout-check.py --maker "Texas Instruments"` → `0 pinout errors` (est. 1 min; the reading is the work).
Commit: —

### Phase 2 — Analog Devices

- [ ] As Phase 1 ("Pin Configurations and Function Descriptions";
      Maxim-origin "Pin Description"); `specs-adi-pins.tsv`.

Acceptance: as Phase 1.
  Check: `pinout-check.py --maker "Analog Devices"` → `0 pinout errors` (est. 1 min).
Commit: —

### Phase 3 — Microchip

- [ ] As Phase 1 ("Pin Descriptions" / "Pin Function Table"; Micrel-origin
      "Pin Configuration" / "Pin Description"); `specs-microchip-pins.tsv`.

Acceptance: as Phase 1.
  Check: `pinout-check.py --maker "Microchip Technology"` → `0 pinout errors` (est. 1 min).
Commit: —

### Phase 4 — Hand checks

- [ ] For each maker, the first spec row of each distinct `pinout` /
      `pinout_adj` and each distinct `regulators` object: re-read the PDF's
      pin table and the enable truth table, record (spec row, page, result)
      in § Corrections; a mismatch is fixed and its whole maker re-checked.

Acceptance: every distinct pinout has a recorded hand check.
  Check: `python3 -I planning/plan-42-data/census/pinout-check.py --distinct` → the number of distinct (maker, pinout, regulators) combinations, equal to the number of hand-check lines in § Corrections (est. 1 min).
Commit: —

## Validation Plan

- Tests: none (data); `pinout-check.py` is the check; H's loader run
  (`library_check`, the `regulator` kind's role rules) checks the same data
  again when it becomes part files.
- Coverage check: `pinout-check.py` covers every spec row an `add` row uses
  (it prints that row count with its result).
- Runtime proof: n/a.
- Doc sync: none.
- Final gate: plan-42-J.

## Decisions

- Author: **fixed and adjustable options are separate pinouts** (rule 3):
  the same pin changes role (§ 2: `ADJ/GND`, `SENSE/ADJ`).
- Author: **the switch pin is the output when there is no VOUT pin** (rule
  4): the library has no inductor kind and the owner asked for no new one.
- Author: **unused control pins are analog**, so the voltage check never
  judges a pin the model does not simulate.
- Author: **DigiKey's dropout figure where it has one** (rule 7). It is not
  always the datasheet's maximum: LM1117's DigiKey value is `1.2V @ 800mA`,
  the 25 °C typical, while `lm1117.pdf` § 7.5 gives 1.3 V maximum over
  temperature. The voltage check uses dropout only as a headroom gate
  (plan-42-G § 3.3), where 0.1 V moves the verdict only for an input within
  0.1 V of the edge; reading 600 datasheets' tables for it is not worth that.

## Corrections

## Summary

C is reading work across about 444 (series, package) rows. The risk is a
swapped pin (IN and OUT on a SOT-223), which would make the app wire a part
backwards; `pinout-check.py`'s role rules and one hand check per distinct
pinout guard it.
