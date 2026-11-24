# plan-45-F: Backend — terminal blocks and JST headers in the `connector` kind

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-45-E

F teaches the `connector` kind two new kinds of device: a **terminal block**
(`"terminal"`: the wire clamp) and a **JST header** (`"jst"`: the series),
both with a `"pitch"`. They are passive terminals like the USB connectors:
no model change, the same simulation component (`CONNECTOR_MODEL`), nothing
driven. F changes the loader, the part facts the listing prints, the tests and
`docs/parts-format.md`; it adds no part (G does). The behavioral outcome: a
connector device with `"terminal"` or `"jst"` and a `"pitch"` loads and
exposes `terminal`, `jst`, `pitch_um` and its optional ratings in
`PartConnector`; every malformed one is rejected with a message that names
the key; the 224 existing connector devices and every golden line are
unchanged.

References:

- `docs/parts-format.md` § `connector`, § Devices, § Values;
  `docs/backend.md` § Tests (`library_check`, `listing_check`).
- `backend/src/models/connector.c` (the loader, the USB precedent
  `usb_load`), `backend/include/parts.h:115` (`PartConnector`),
  `backend/tests/support/listing.c:284` (the connector facts),
  `backend/tests/library_check.c` (`check_usb_connector`),
  `backend/tests/connector_test.c`.

## Prerequisites

plan-45-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-45-E complete | `ls planning/completed/plan-45-E-*.md` → exists | MET (2026-11-24: not archived, every E box ticked: `grep -c '\[ \]' planning/plan-45-E-jst-footprints.md` → 0; E's commit 1dcc28c7) |
| The connector kind has no terminal or JST key | `grep -c '"terminal"\|"jst"\|"pitch"' backend/src/models/connector.c` → 0 | MET (2026-11-24 at 1dcc28c7, before F: 0) |
| `PartConnector` is the five-field struct | `grep -c 'rated_ma; unsigned contacts; char usb; } PartConnector' backend/include/parts.h` → 1 | MET, reworded (2026-11-24: the struct has grown since: plan-54 added `card` / `card_pitch_um`, plan-57 `test_point`; the command's `usb; } PartConnector` ending → 0, the struct is `{center_diameter_um, rated_mv, rated_ma, contacts, usb, card, card_pitch_um, test_point}`: F appends `pitch_um`, `terminal`, `jst` after `test_point`; no code initializes it positionally) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A connector device may name `"terminal"` (`"screw"`, `"spring"`,
  `"push-in"` or `"pluggable"`) or `"jst"` (`"XH"`, `"PH"`, `"EH"`, `"ZH"`,
  `"SH"`, `"GH"` or `"VH"`), and then must name `"pitch"` (mm, above 0 and at
  most 20). `ratedVolts` and `ratedAmps` are optional as for USB.
- `PartConnector` has `pitch_um`, `terminal` (`'s'`, `'c'`, `'p'`, `'h'` for
  screw, spring-cage, push-in, pluggable header; 0 otherwise) and `jst` (the
  series' first letter: `'X'`, `'P'`, `'E'`, `'Z'`, `'S'`, `'G'`, `'V'`; 0
  otherwise).
- `library_check` rejects each malformed device with a message naming the
  key; `connector_test` shows a terminal block and a JST header joining
  nothing; `listing_check` prints their facts; the existing goldens do not
  change (`git diff --stat backend/tests/golden` → empty).

### Non-goals (explicit constraints)

- No new kind, no new `PART_*`, no new test program (T0 stays 36), no change to
  the engine model, to `circuit.c`, or to the barrel and USB devices.
- No part is added; the 88 generics and the picked parts are G's.
- The app's `PartDetails` is H's: this letter does not touch Swift, and the
  app still builds because it reads only the fields it knew.

## 2. Current State

`connector.c` loads a device that is a USB connector (`"usb"`) or a barrel
jack (`centerPin`, `ratedVolts`, `ratedAmps`, optional `contacts`):
`DEVICE_KEYS` is `{"centerPin", "ratedVolts", "ratedAmps", "contacts",
"usb", NULL}`; `usb_load` rejects `centerPin` and `contacts` and makes the
ratings optional; `connector_bind_pinout` needs two or more pins and binds
`contacts` (none for USB). `PartConnector` is
`{center_diameter_um, rated_mv, rated_ma, contacts, usb}`; the listing prints
`usb=A rated=5V/900mA` for USB and `center_um=… rated=… contacts=…` for a
barrel jack. `docs/parts-format.md` § `connector` describes both.

### Measured populations

| What | Count | Command |
|---|---|---|
| Connector devices / parts in the library | 224 / 376 | A § 2 |
| Backend test programs | 36 | `grep -c '^logic_test' backend/CMakeLists.txt` |
| Golden lines for connectors | 376 | `grep -c '|connector|' backend/tests/golden/parts.txt` |

### Verified properties

- `usb_load` is the pattern: it reads `kind_string`, validates, calls
  `kind_volts` and `kind_number` for the optional ratings (`connector.c:20`).
- A connector with `contacts` 0 has an empty `ConnectorPins` and never calls
  `sim_set_contact` (`connector_bind_pinout` returns `bound` with `count` 0,
  `connector_set` returns 0), so a terminal block is inert, as USB is.
- `circuit_add_part` only initializes contacts for `part->connector->contacts`
  positions (`circuit.c:156`); 0 for the new devices.

## 3. Design

The loader branches on the first of `"usb"`, `"terminal"`, `"jst"` it finds
and rejects a device with more than one:

| Key | Rule | Message |
|---|---|---|
| more than one of `usb`, `terminal`, `jst` | rejected | `a connector is one of "usb", "terminal" and "jst"` |
| `terminal` | string, one of `screw`, `spring`, `push-in`, `pluggable` | `"terminal" must be "screw", "spring", "push-in" or "pluggable"` |
| `jst` | string, one of `XH`, `PH`, `EH`, `ZH`, `SH`, `GH`, `VH` | `"jst" must be "XH", "PH", "EH", "ZH", "SH", "GH" or "VH"` |
| `pitch` with `terminal` / `jst` | required, number, above 0, at most 20 | `a terminal block needs "pitch"`, `a JST connector needs "pitch"`, `"pitch" must be above 0 and at most 20 mm` |
| `pitch` without them | rejected | `"pitch" is only for terminal blocks and JST connectors` |
| `centerPin`, `contacts` with them | rejected | `a terminal block has no "centerPin"` (and `"contacts"`; `a JST connector …`) |
| `ratedVolts`, `ratedAmps` | optional, as `usb_load` | `ratings must be above 0`, `"ratedAmps" must be above 0 and at most 100` |

`DEVICE_KEYS` gains `"terminal"`, `"jst"`, `"pitch"`. `terminal_load` is
`usb_load`'s twin (shared ratings code). The three new `PartConnector`
fields are zero for barrel and USB devices, so their facts do not change.

`listing.c` prints, for a terminal block, `terminal=<screw|spring|push-in|pluggable>
pitch_um=<n> rated=<V>V/<mA>mA` and, for a JST header, `jst=<XH…> pitch_um=<n>
rated=…`; barrel and USB lines are unchanged. (`rated` prints 0V/0mA when
the ratings are absent, as USB's does.)

**Correctness risk** is small and concentrated in the one rule that touches
existing devices: a barrel jack must still require `centerPin`. The existing
negative test (`barrel: still needs its centre pin`) covers it; F adds the
new keys' cases beside it.

**Byte-identity.** The goldens (`parts.txt`, `traces.txt`, `footprints.txt`)
are the gate for the existing parts: they do not change. The expected diff
is empty.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The loader and its tests

- [x] `backend/include/parts.h`: `PartConnector` gains `uint32_t pitch_um;
      char terminal; char jst;` and its comment names them (zero for barrel
      and USB).
      Appended after `test_point` (Corrections); the comment names the codes and says both are passive.
- [x] `backend/src/models/connector.c`: `DEVICE_KEYS`, `terminal_load` and the
      branch of § 3; the file's header comment names terminal blocks and JST
      headers.
      `DEVICE_KEYS` + `"terminal"`, `"jst"`, `"pitch"`; `terminal_load` (one function for both keys: the clamp / series table, the pitch, the shared `optional_ratings_load` that `usb_load` now also calls); the dispatch checks "one of usb / terminal / jst" and "pitch only with them" before the USB branch; the card-slot and test-point `FORBIDDEN` lists gain the three keys.
- [x] `backend/tests/library_check.c`: `check_terminal_connector` (beside
      `check_usb_connector`): a fixture device for each clamp (`'terminal':'screw'`,
      `'spring'`, `'push-in'`, `'pluggable'`) loads with `pitch_um` 5080,
      `terminal` `'s'`/`'c'`/`'p'`/`'h'`, `rated_mv`/`rated_ma` from the keys
      and 0 when absent; a JST fixture for each of the seven series loads with
      `jst` the series' first letter and `pitch_um` 2500; `expect_fail` for:
      two of the three keys, an unknown clamp, an unknown series, no pitch
      (both), pitch 0, pitch 21, pitch on a USB device, `centerPin` and
      `contacts` with `terminal`, `ratedAmps` 0.
      All of them (four clamps and seven series by `TERMINAL_CASE`; the ratings absent; 21 `expect_fail` cases: the three two-of-three pairs, unknown clamp and series, no pitch ×2, pitch 0 and 21, pitch on USB and on a barrel jack, `centerPin` and `contacts` with each key, `ratedAmps` 0, `ratedVolts` 0, and the card slot and test point rejecting the new keys) plus the old barrel negative on the new fixture.

Acceptance: the loader accepts the new devices and rejects the malformed ones.
  Check: `cmake --build /tmp/logic-p45 -j8 --target library_check && /tmp/logic-p45/library_check` → every line `ok`, exit 0, including `ok   terminal block and JST header connectors load; malformed ones fail` (est. 5 min). Result: exit 0, `library_check: all passed`, 539 `ok` lines, the line above present.
Commit: this commit (hash in the next one)

### Phase 2 — The listing and the circuit test

- [x] `backend/tests/support/listing.c` (`case PART_CONNECTOR`): the two new
      lines of § 3 before the barrel line.
      After the USB line and before the barrel line: `terminal=<screw|spring|push-in|pluggable> pitch_um=<n> rated=<V>V/<mA>mA` and `jst=<XH…> pitch_um=<n> rated=…`.
- [x] `backend/tests/connector_test.c`: a fixture terminal block (3 pins) and
      a JST header (2 pins) beside the USB case: each part's pins 1 and 2 on
      two nets, one driven high from `5V`, the other from `GND`: after
      `circuit_start` and `circuit_advance(circuit, 1)` neither net is
      shorted; `part->connector->terminal` / `jst` and `pitch_um` as loaded.
      The terminal fixture has 4 pins (the `LED-DIP4` footprint it names has 4 pads; a 3 pin pinout is refused against it) and the JST header 2 (`LED-0402`); `PASS: a terminal block and a JST header load with their pitch and join nothing`.
- [x] `/tmp/logic-p45/listing_check --print > /tmp/p45-parts.txt && diff
      /tmp/p45-parts.txt backend/tests/golden/parts.txt` → no output (no
      shipped part has the new keys yet).
      `diff` → no output (exit 0); 35921 lines.

Acceptance: the circuit and listing tests pass and the goldens are unchanged.
  Check: `cmake --build /tmp/logic-p45 -j8 && ctest --test-dir /tmp/logic-p45 -R 'connector_test|library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff --stat backend/tests/golden` → empty (est. 8 min). Result: `100% tests passed, 0 tests failed out of 5`; `git diff backend/tests/golden | wc -l` → 0; the whole suite `100% tests passed, 0 tests failed out of 57`.
Commit: this commit (hash in the next one)

### Phase 3 — Docs

- [x] `docs/parts-format.md` § `connector`: a subsection "Terminal blocks and
      JST headers": the keys of § 3; no `centerPin`, no `contacts`; every pin
      a passive `inout` terminal named `"1"`…`"n"` (pin 1 the first along the
      row, as the footprint); drawers `["Connector", "Terminal block",
      "<pitch> mm"]` (function `Screw, vertical`, `Spring, horizontal`,
      `Push-in, angled`, `Plug-in header, vertical`, or `Generic`) and
      `["Connector", "JST", "<series>"]` (function `Top entry`, `Side entry`
      or `Generic`); `package` `Through hole vertical`, `Through hole right
      angle`, `Through hole angled`, `SMD vertical`, `SMD right angle`;
      footprints `TB-…` and `JST-…` (plan-45-C … E); the device's ratings are
      the datasheet's, optional. The intro sentence names terminal blocks and
      JST headers beside "Barrel power connectors".
      Added as "**Terminal blocks and JST headers** (plan-45)" after the test-point paragraph, with the keys, every rejection message, the `PartConnector` fields, and a table of drawer, function, package and footprint; the section's first sentence names terminal blocks and JST headers beside the other connectors.
- [x] `docs/parts-format.md` § Devices table of kinds: no change (the kind is
      still `connector`); the intro list of kinds gets no new name.
      No change made there.

Acceptance: the doc names every key and the drawers.
  Check: `grep -c '"terminal"\|"jst"\|"pitch"' docs/parts-format.md` → at least 3 (est. 10 min). Result: `12`.
Commit: this commit (hash in the next one)

## Validation Plan

- Tests: `library_check` (the new cases, negative included), `connector_test`,
  `listing_check` / `trace_check` / `footprint_check` (unchanged goldens).
- Coverage check: `connector.c` is in the `logic` library
  (`backend/CMakeLists.txt:80`, inside `add_library(logic STATIC` at line 8) that
  every test links through `logic_test_support`.
- Runtime proof: `connector_test`'s two new parts join nothing.
- Doc sync: `docs/parts-format.md` § `connector`.
- Final gate: none; the landing tier is I's.

## Decisions

- Author: **Extend `connector`, not a new kind.** A terminal block and a JST
  header are connectors that the library models as passive terminals; a new
  kind would copy `connector.c` and the app's switches. The USB devices are
  the precedent (`"usb"`).
- Author: **Ratings optional**, so the generic parts (no maker rating) need
  none.
- Author: **`jst` is the series (`"SH"` covers the SH and SR parts, which
  share one datasheet and one 1.00 mm pitch)**; `pitch` is explicit so a
  future series needs no loader change.

## Corrections

- **2026-11-24, populations.** The test-program count is **57**, not 36 ("T0 stays 36" is "T0 stays 57": `grep -c '^logic_test' backend/CMakeLists.txt` → 57; no program added). The `connector` kind holds 482 devices / 952 parts (plan-54's card slots and plan-57's test points landed since the plan's 224 / 376; `grep -c '|connector|' backend/tests/golden/parts.txt` → 952); the golden is unchanged (`diff` of `listing_check --print` against it is empty).
- **2026-11-24, the struct and the other modes.** `PartConnector` already had `card`, `card_pitch_um` and `test_point`; F appends `pitch_um`, `terminal`, `jst` (a card slot's `card_pitch_um` stays its own field). A card slot and a test point reject `terminal`, `jst` and `pitch` through their `FORBIDDEN` lists, and a card slot's `cardPitch` still means the contact pitch; the plan's rules covered barrel and USB only. `usb_load`'s ratings code moved into `optional_ratings_load`, shared with `terminal_load` (same messages).
- **2026-11-24, library_check wording.** The `expect_fail` for pitch on a USB device and on a barrel jack both give `"pitch" is only for terminal blocks and JST connectors`; "a connector is one of …" comes first when two of the three keys are present.

## Summary

F is a small loader change with three new device keys, the tests that pin
them and the doc. Its one hazard is the existing barrel rule, which the
existing negative test guards; the goldens must not move.
