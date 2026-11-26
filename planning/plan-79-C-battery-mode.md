# plan-79-C: Backend: the connector's `"battery"` mode, tests, docs

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-79-B; plan-74-E (§ Prerequisites)

C adds a seventh `connector` mode, `"battery"`: a PCB-mount battery holder, retainer, clip or
contact. Its pins are passive terminals like a USB connector's; it drives nothing and supplies no
voltage. No part file changes here: D–H add the parts.

References:

- `docs/parts-format.md` § `connector` (the modes, the test point's one-pin rule);
  `docs/backend.md` § tests, § goldens.
- `backend/src/models/connector.c` (278 lines: `DEVICE_KEYS` line 17, the card slot's `FORBIDDEN`
  line 97, the test point's line 123, the mode exclusion lines 147–170, the pin-count rule lines
  220–228); `backend/include/parts.h:184` (`PartConnector`);
  `backend/tests/support/listing.c:453` (the connector facts); `backend/tests/connector_test.c`
  (237 lines; the USB "joins nothing" case lines 74–87).
- `planning/plan-74-E-ethernet-kind-rj45.md` § 3.2: the `"rj45"` mode, which edits the same lines
  (the exclusion message, both `FORBIDDEN` lists, `PartConnector`, the listing's connector facts).

## Prerequisites

plan-79-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (owner, 2026-11-25: backend letters wait for it) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK open, 126 boxes unticked) |
| plan-68's size and coverage checks run | `node scripts/source-size.js --check` → exit 0; `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: neither script exists; plan-68-A adds them) |
| plan-74-E complete: the `"rj45"` mode is in `connector.c`, so C extends its exclusion message and `FORBIDDEN` lists rather than racing it | `ls planning/completed/plan-74-E-*.md` → exists; `grep -c '"rj45"' backend/src/models/connector.c` → above 0 | NOT MET (2026-11-25: 0; plan-74-E not started) |
| plan-79-B complete | `ls planning/completed/plan-79-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25) |
| The mode is absent | `grep -c '"battery"' backend/src/models/connector.c` → 0 | MET (2026-11-25: 0) |
| T and goldens at C's start | `grep -c '^logic_test' backend/CMakeLists.txt`; `wc -l < backend/tests/golden/parts.txt` | 77 and 40832 at planning; plans 69–75 add programs and parts first. C records its own T1 and P1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A connector device with `"battery"` loads: `PartConnector.battery` (`'h'` holder, `'r'` retainer,
  `'c'` clip, `'k'` contact, `'s'` snap), `battery_cells` (the cell sizes), `battery_cell_count`,
  `cells`, `coin_diameter_um`. The other modes' keys are rejected on it, and its keys on them.
- In a circuit, a holder's `+` and `-` on two driven nets join nothing, as for USB.
- `ctest` passes T1 programs (no new program: the cases go into `connector_test`). The goldens are
  unchanged: no shipped part uses the mode yet.

### Non-goals (explicit constraints)

- No voltage source: a holder never drives its pins. A schematic that needs the battery's voltage
  places a `supply` symbol on the `+` net, as today.
- No change to the other connector modes, to `circuit.c`, or to the voltage check.
- No part file and no app change (D–I).

## 2. Current State

See References. `connector_load_device` picks the mode by key presence: test point, card slot, then
`usb` / `terminal` / `jst` (after plan-74-E also `rj45`), which exclude each other, then barrel.
`bind_pinout` refuses a one-pin connector unless it is a test point (`a test point has exactly one
pin` / `a connector has at least two pins`). The listing prints `testpoint=1`, `card=…`, `usb=…`,
`terminal=…`, `jst=…` (and after 74-E `rj45=…`) as the connector's facts.

## 3. Design

### 3.1 Keys

| Key | Value |
|---|---|
| `"battery"` | `"holder"`, `"retainer"` (a coin cell retainer: a spring on the cell's top, the cell's bottom on a PCB pad), `"clip"` (a cylindrical clip whose ends touch the cell's ends), `"contact"` (one contact: one end of one cell, or a link between two cells) or `"snap"` (a PCB-mount 9 V snap). Else `"battery" must be "holder", "retainer", "clip", "contact" or "snap"` |
| `"cell"` | non-empty array of distinct cell names from § 3.2 (a multi-size holder names each), else `unknown battery cell "X"` / `battery cell "X" is named twice` |
| `"coinDiameter"` | mm, above 0 and at most 60; required when `"cell"` holds `"coin"` (`a coin cell holder needs "coinDiameter"`), rejected otherwise (`"coinDiameter" is only for coin cells`) |
| `"cells"` | the number of cells it holds, 1 to 16 (`"cells" must be 1 to 16`); optional, default 1; a `contact` takes only 1 |

No `centerPin`, `contacts`, `pitch`, `ratedVolts` or `ratedAmps` (`a battery holder has no "X"`).
`"battery"` with `usb`, `terminal`, `jst` or `rj45` is `a connector is one of "usb", "terminal",
"jst", "rj45" and "battery"` (plan-74-E's message, extended). A card slot and a test point reject
`battery`, `cell`, `coinDiameter` and `cells` with their existing `FORBIDDEN` lists, and `cell`,
`coinDiameter` and `cells` without `battery` are `"X" is only for battery holders`.

### 3.2 Cell names

`coin`, `AAAA`, `AAA`, `AA`, `A`, `C`, `D`, `N`, `1/2AA`, `2/3A`, `1/3N`, `CR2`, `CR123A`,
`CR-P2`, `A23`, `A27`, `9V`, `14500`, `18350`, `18650`, `20700`, `21700`, `26650`. These are
DigiKey's Battery Cell Size values of category 86's PCB-mount rows (`show.py
planning/plan-76-data/raw/p86.json "Battery Cell Size"`), with "Coin, 20.0mm" written as `coin`
plus `coinDiameter` 20 and "Multiple (A, AA, CR2)" as `["A", "AA", "CR2"]`. A part whose drawing
names a size not in the list is a `skip` (`cell size not modelled`) in D–H; the list does not grow
silently.

### 3.3 Pins

Every pin is `inout` or `nc`. Names:

- a `contact` has exactly one `inout` pin, named `+`, `-` or `LINK` (a dual contact joining one
  cell's `+` to the next cell's `-`): `a battery contact has exactly one pin, "+", "-" or "LINK"`.
  This is the second one-pin connector after the test point;
- every other battery device has `inout` pins named `+` and `-` (one of each), or, for a holder
  whose cells have their own terminals (no internal series link), `1+`, `1-` … `<cells>+`,
  `<cells>-`: `a battery holder's pins are "+" and "-", or "1+", "1-" … per cell`;
- several pads of one terminal (a holder's two solder tabs on `+`) are one pin with several pads in
  the footprint (`docs/parts-format.md` § `explicit`); a mounting or retention tab that touches no
  cell is an unnumbered footprint pad (`"pin": 0`), not a pin.

### 3.4 Facts and listing

`PartConnector` gains `char battery; const char *const *battery_cells; size_t battery_cell_count;
uint8_t cells; uint32_t coin_diameter_um;` (all 0 / NULL on other connectors). Listing facts:
`battery=<word> cell=<names, comma> cells=<n>` and ` coin_um=<n>` when set, e.g.
`battery=holder cell=coin cells=1 coin_um=20000`. `DEVICE_KEYS` gains `battery`, `cell`,
`coinDiameter`, `cells`.

### 3.5 Shared lines

After plan-74-E: grep each line before editing (`grep -n '"rj45"' backend/src/models/connector.c`)
and add `"battery"` after whatever mode is last in the exclusion message, both `FORBIDDEN` lists
and `PartConnector`. No other plan of 69–82 edits `connector.c` (`grep -l 'connector\.c'
planning/plan-*.md` → plan-68-R, plan-74-A, plan-74-E, at planning).

## 4. Docs

- `docs/parts-format.md` § `connector`: a "**Battery holders** (plan-79)" paragraph with the keys
  of § 3.1, the cell names of § 3.2, the pin rule of § 3.3, the drawer
  `["Connector", "Battery", "Coin" | "Cylindrical" | "9 V"]`, the function words
  (`Coin cell holder`, `Coin cell retainer`, `Coin cell contact`, `Cylindrical holder`,
  `Cylindrical clip`, `Cylindrical contact`, `9 V holder`, `9 V snap`, `9 V contact`), the package
  words (`Through hole`, `SMD`), and the footprint prefix `BAT-` (plan-79-D § 3.2).
- The first paragraph of § `connector` lists battery holders among the connector families.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The mode

- [ ] `connector.c` and `PartConnector` per § 3.1–3.4; the shared lines per § 3.5 (record the greps here).
- [ ] `listing.c`'s connector facts per § 3.4.
- [ ] `backend/tests/connector_test.c`: § 5 (a)–(h).

Acceptance: a battery holder loads with its facts and stays a passive terminal block; the other
modes are unchanged.
  Check: `cmake --build /tmp/logic-p79 -j8 && /tmp/logic-p79/connector_test && ctest --test-dir /tmp/logic-p79 -j8` → `100% tests passed`, T1 programs; `git diff --stat backend/tests/golden` → empty (est. 5 min).
  Result:
  Also (plan-68's rules; owner, 2026-11-25): `node scripts/source-size.js --check` → exit 0 and
  `node scripts/coverage.js --check --backend` → exit 0, so `connector.c` stays under 1000 code
  lines and every new line runs (est. 6 min: one instrumented backend build with `ctest`, the only
  tool that shows every new line runs).
Commit:

### Phase 2 — Docs

- [ ] § 4.

Acceptance: the docs name the mode and its keys.
  Check: `grep -c '"battery"' docs/parts-format.md` → above 0; `grep -c 'coinDiameter' docs/parts-format.md` → above 0 (est. < 1 min).
  Result:
Commit:

## 5. Test cases (`connector_test.c`)

Fixtures, inline JSON: `bat-coin` (`"battery": "holder"`, `"cell": ["coin"]`, `"coinDiameter": 20`,
pins `1` `+` inout, `2` `-` inout; part `BAT-TEST-COIN`), `bat-aa2` (`"holder"`, `["AA"]`,
`"cells": 2`, pins `+`, `-`), `bat-link` (`"contact"`, `["AA"]`, one pin `LINK`; part
`BAT-TEST-LINK`).

- (a) `BAT-TEST-COIN`: `battery` `'h'`, `battery_cell_count` 1, cell `coin`, `cells` 1,
  `coin_diameter_um` 20000, `usb` 0, `test_point` 0.
- (b) `bat-aa2`: `cells` 2; a `"cell": ["A", "AA", "CR2"]` device: `battery_cell_count` 3.
- (c) `BAT-TEST-LINK` loads with one pin; a `contact` with two pins → `a battery contact has
  exactly one pin, "+", "-" or "LINK"`; a holder with pins `A`, `B` → `a battery holder's pins are
  "+" and "-", or "1+", "1-" … per cell`; a 2-cell holder with `1+`, `1-`, `2+`, `2-` loads.
- (d) `"battery": "pack"` → the `"battery" must be …` error; `"cell": ["AA", "AA"]` →
  `battery cell "AA" is named twice`; `"cell": ["LR44"]` → `unknown battery cell "LR44"`.
- (e) `["coin"]` without `coinDiameter` → `a coin cell holder needs "coinDiameter"`;
  `coinDiameter` with `["AA"]` → `"coinDiameter" is only for coin cells`; `"cells": 0` and `17` →
  `"cells" must be 1 to 16`; a `contact` with `"cells": 2` → the same error.
- (f) `"battery"` with `"usb": "C"` → `a connector is one of "usb", "terminal", "jst", "rj45" and
  "battery"`; `"pitch"` with `"battery"` → `a battery holder has no "pitch"`; `"cell"` on a USB
  device → `"cell" is only for battery holders`; `"battery"` on a test point → `a test point has
  no "battery"`.
- (g) a circuit with `5V` on `+`'s net and `GND` on `-`'s net of `BAT-TEST-COIN`: `circuit_start`
  returns 0 and after 1 ms `circuit_shorted_net` reports neither net (the USB case's shape).
- (h) the listing line of `BAT-TEST-COIN` contains `battery=holder cell=coin cells=1 coin_um=20000`.

## Validation Plan

- `connector_test`; `listing_check`, `trace_check`, `erc_pins_check`, `footprint_check` with
  unchanged goldens.
- Final gate: plan-79-J.

## Decisions

- Owner, 2026-11-25: battery holders are a `connector` mode, not a kind, and supply no voltage in
  play mode.
- Author: **after plan-74-E, not beside it.** Both letters edit the same exclusion message,
  `FORBIDDEN` lists and struct; landing C after 74-E means C extends a finished mode list instead
  of merging two half-edited ones.
- Author: **`"cell"` is an array.** DigiKey lists multi-size holders (`Multiple (A, AA, CR2)`), and
  the drawer and details need every size such a holder takes.
- Author: **pins named by polarity.** A designer wires `+` and `-`, not pad numbers; the pad
  numbers stay in the footprint, as for the USB connectors' `SHIELD`.
- Author: **one-pin contacts are allowed.** A single battery contact is sold and placed on its own
  (DigiKey's "Contact (Single)": 41 cylindrical + 36 coin + 6 9 V PCB-mount rows); a pair of them
  is a holder built on the board.

## Corrections

## Summary

A passive `"battery"` connector mode with cell, count and polarity rules, after plan-74-E's RJ45
mode, with its rejection and "joins nothing" cases.
