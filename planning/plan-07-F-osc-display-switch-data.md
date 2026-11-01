# plan-07-F: Oscillator, 7-segment and switch voltage data; the keys become required

Last updated: 2026-11-01
Effort: medium (1h–2h)
Depends on: plan-07-E

This sub-plan fills in the last three kinds:

- Oscillators get a `vcc` range.
- 7-segment displays get a forward-voltage `vf`.
- Switches get a `ratedVolts`.

Then the loader starts requiring the keys. A gate, dff, jkff, SRAM, EEPROM
or oscillator part with no resolved `vcc` is rejected. So is a 7-seg device
with no `vf`, and a switch device with no `ratedVolts`. LEDs and supplies
take no new key.

Outcome:

- `library_check` rejects a gate part with no `vcc` (`missing "vcc"`).
- The whole library loads.
- Spot values:
  - `LTC-4727JR` lists `vf=2000-2600`.
  - `TE-GDS0204` lists `rated=24000`.
  - An LMK6 part of the 1.8 V option lists `vcc=1700-1900`.

References:

- plan-07-A §4.1–4.2 (`vf`, `ratedVolts`, oscillator `vcc`, the error
  style)
- plan-07-C §4 (the method)
- `datasheets/osc/` (10), `led-7/` (27), `switch/` (2)
- `docs/parts-format.md`

## Prerequisites

The same as plan-07-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-07-E complete | `ls planning/plan-07-E-* 2>/dev/null` → no output | MET (2026-11-01: no matches; archived in 1a4e3fb) |

## 1. Goal

- **Oscillators.** All 108 oscillator parts (20 devices) resolve a `vcc`.
- **7-seg.** All 49 7-seg devices have `vf`.
- **Switches.** All 27 switch devices have `ratedVolts`.
- **Loader.** It enforces all three keys and the earlier kinds' `vcc`, with
  `library_check` cases.

### Non-goals (explicit constraints)

- `traces.txt` does not change.
- LEDs get no voltage data. The generic LEDs pass voltage through (plan-07-B).
- No check change. 7-seg VF is shown, not checked.

## 2. Current State

- **Oscillators.**
  - 20 devices, 108 parts, 10 PDFs.
  - The LMK6 devices (`lmk6c-*`, `lmk6d-*`, `lmk6h-*`, `lmk6p-*`) all come
    from `lmk6c.pdf`. Their supply is a part-number option:
    `Device Supply Voltage(1) 1.7 1.8 1.9` and `(2) 2.37 2.5, 3.3 3.5`
    (pdftotext lines 339 and 341).
  - 23 of the 91 LMK6 parts are the 1.8 V option (census, 2026-11-01). The
    letter rule that picks it is on the datasheet's ordering page.
  - The device range goes to the 2.37–3.5 option, and the 1.8 V parts
    override it.
  - The Geyer `12.94761` sheet is "1.8 V ±5%" (1.71–1.89). Its device's
    family range is wider, so that part gets its own `vcc`.
- **7-seg.**
  - 49 devices, 27 PDFs. Every device `note` names its PDF.
  - Example: `C4727JR` has `VF 2.0 2.6 V IF=20mA`, typ and max with no min,
    so `vf` = [2.0, 2.6] (the owner's rule: min = typ).
  - 11 PDFs are scans: LSHD-5601, LSHD-7801, LSHD-A101, LTC-2623G,
    LTC-4624JR, LTC-4724JR, LTD-2701JD, LTD-4608JR, LTD-4708JR, LTD-6000
    and LTS-4802BJR-H1. LTD-6000 (12 pages) covers 23 devices.
- **Switches.**
  - 27 devices, 2 PDFs.
  - `ENG_DS_8-1773450-9_SECTION_A_DIP_SWITCHES.pdf` gives "Contact rating,
    switching … 25 mA @ 24 VDC" (line 165), next to a non-switching
    100 mA @ 50 VDC.
  - The catalog `ENG_CS_1308111-1…` lists per-family ratings of 24, 20 and
    50 Vdc (lines 105–457).
  - Each device takes its family's **switching** rating.
- **Loader today (after A).** The keys are optional. `Part.vcc == NULL` and
  `vf_max_mv == 0` / `rated_mv == 0` mean "not given".

### Measured populations

| What | Count | Command |
|---|---|---|
| oscillator devices / parts | 20 / 108 | python over `oscillator.json` → `20 108` |
| 7seg devices / parts | 49 / 49 | census → `7seg 49 49` |
| switch devices / parts | 27 / 36 | census → `switch 27 36` |
| PDFs | 10 / 27 / 2 | `ls datasheets/{osc,led-7,switch}/` |

## 3. Design Overview

Two pieces: the data, then the required-key flip.

**The risk.** The flip turns any part missed in C–E into a load failure.
A failed `load_parts` rejects the whole file, and the app would show no
parts of that kind. So the flip lands last, behind a census that is already
at zero, and `library_check` loads every real file.

This work is not behavior-neutral for the loader: new rejections are the
intent. Part behavior stays neutral, so `traces.txt` does not change.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick boxes in the same
> commit as the work, use `- [~]` for partial work, mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`, and fill in `Commit:`.
> **An unticked box means NOT DONE.**

### Phase 1 — Data

- [x] `oscillator.json`:
  - device `vcc` from each PDF;
  - LMK6 1.8 V parts get `"vcc": [1.7, 1.9]` (the letter rule goes in the
    device note);
  - the Geyer part gets `"vcc": [1.71, 1.89]`.
- [x] `7seg.json`: `vf` on each device, from its PDF's "Forward Voltage Per
      Segment". When no MIN is printed, min = TYP. Scans are read from the
      page image.
- [x] `switch.json`: `ratedVolts` on each device, from its family's
      switching rating.
- [x] Regenerate `parts.txt`.

Acceptance: every oscillator part has `vcc`, every 7-seg line has `vf=`,
and every switch line has `rated=`. The spot values match.
  Check:
  `grep -c '|oscillator|.*|vcc=' backend/tests/golden/parts.txt` → 108;
  `grep -c '|7seg|.*vf=' backend/tests/golden/parts.txt` → 49;
  `grep -c '|switch|.*rated=' backend/tests/golden/parts.txt` → 36;
  `grep -E '^(LTC-4727JR|TE-GDS0204)\|' backend/tests/golden/parts.txt` →
  `vf=2000-2600` and `rated=24000`;
  `git diff --quiet backend/tests/golden/traces.txt` → exit 0
  (est. 1 min).
  Result (2026-11-01): `108`, `49`, `36`; `LTC-4727JR … vf=2000-2600`,
  `TE-GDS0204 … rated=24000`; LMK6 1.8 V part `LMK6CE001000DDLER
  vcc=1700-1900,in=1900`; traces `git diff --quiet` → exit 0 (`trace_check`
  exit 0). `/tmp/p07s/verify_misc.py` (every oscillator, 7-seg and switch
  line against the readings) → `193 parts checked, 0 mismatches`.
Commit: —

### Phase 2 — Required keys

- [ ] gate.c, dff.c, jkff.c, sram.c, eeprom.c, oscillator.c: a part whose
      `kind_voltage` result is NULL fails with `missing "vcc"` (in the
      part's place).
- [ ] display7.c: `"vf"` is required. switch.c: `"ratedVolts"` is required.
- [ ] `library_check.c`:
  - Add the three rejection cases (gate part with no `vcc`, 7-seg with no
    `vf`, switch with no `ratedVolts`).
  - Update every inline fixture of these kinds to carry the key.
- [ ] `docs/parts-format.md`: drop "required from plan-07-F", so the keys
      are listed as required.
- [ ] `parts.h` comments: `Part.vcc` NULL only for supplies, LEDs, 7-seg
      and switches.

Acceptance: the new rejections fire, the full library loads, and the
listing is unchanged.
  Check: `cmake --build /tmp/p07 -j8 && ctest --test-dir /tmp/p07` → 12/12,
  and `git diff --quiet backend/tests/golden` against Phase 1's commit
  → exit 0 (est. 1 min).
Commit: —

## Validation Plan

- Tests:
  - `library_check`: the three rejections, plus the full library load.
  - `listing_check`: every value.
- Coverage check: Phase 1's three counts match the population (108, 49,
  36).
- Runtime proof: none; this is data and loader rules.
- Doc sync: `docs/parts-format.md` (Phase 2).
- Final gate (once): `ctest --test-dir /tmp/p07` → 12/12 (est. 1 min). Also
  build the app, since the loader now rejects more:
  `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p07-dd CODE_SIGNING_ALLOWED=NO build`
  → `** BUILD SUCCEEDED **` (est. 3 min). No Swift changes are made, but the
  app bundles these part files.

## Decisions

See plan-07-A § Decisions. Owner rule used here: 7-seg min = typ when the
datasheet prints no minimum. Author decisions:

- A switch takes its switching rating.
- The required-key flip lands last, behind a zero census.

## Corrections

- **Run the same way as plan-07-C** (see its Corrections): one research
  agent wrote `/tmp/p07/data/misc.json`; `/tmp/p07s/apply.py` applied it
  (device-level `vcc`/`vf`/`ratedVolts` before `"parts"`, provenance
  appended to the device note) and `/tmp/p07s/verify_misc.py` compared
  every listed value with the readings.
- **The LMK6 letter rule** (lmk6c pp.3–4): the voltage letter is the
  character just before the package code `DLE`/`DLF` (5th from the end).
  LMK6C: `C` = 2.5–3.3 V, `D` = 1.8 V; LMK6D/H/P: `A` = 2.5–3.3 V, `B` =
  1.8 V (`lmk6c` lines 141, 198). 23 parts are the 1.8 V option and carry
  `"vcc": [1.7, 1.9]`; the rule is in each LMK6 device note.
- **The Geyer 1.8 V part** is `12.94761` under device `kxo-v99-2`, whose
  series range is [1.2, 3.6]; the part carries [1.71, 1.89].
- **Other oscillator ranges**: SiT1534 [1.2, 3.63] (the sheet's wider
  −10…70 °C row; −55…85 °C is 1.5–3.63); OM-7605/OM-0100/ASZKDV/ASH5KW
  [1.6, 5.5]; OV-7604 [1.2, 5.5]; NJU6366A–D [2.0, 5.5]. No oscillator
  sheet gives a fixed input max above VDD, so no `inputMax`.
- **7-seg VF**: no sheet prints a minimum, so min = typ everywhere (the
  owner's rule). The LTD-6000 scan gives the red column (LTD-67x0R) [1.7,
  2.0] and the others [2.1, 2.8]; LTD-6910HR has its own newer sheet
  ([2.0, 2.6]). LTD-2601WC prints a max only at 5 mA ([1.7, 2.4]).
- **Switch rating: "the lower of the two ratings" is wrong for ADE/ADF.**
  Their switching rating is 100 mA @ 24 VDC and their non-switching
  (carry) rating is 1 A @ 5 VDC, so the switching voltage is the higher
  one there. The Decisions rule "a switch takes its switching rating" is
  what was applied: all 27 devices are 24 V (GDS family 25 mA @ 24 VDC,
  `ENG_DS_8-1773450-9` line 165). A 1 A carry rating is a current limit,
  which the voltage check does not model.

## Summary

The risk is the flip rejecting a whole part file. It lands behind
zero-missing counts and a full-library load test.
