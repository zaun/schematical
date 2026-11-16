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
| plan-45-E complete | `ls planning/completed/plan-45-E-*.md` → exists | NOT MET (2026-11-16: not started) |
| The connector kind has no terminal or JST key | `grep -c '"terminal"\|"jst"\|"pitch"' backend/src/models/connector.c` → 0 | MET (2026-11-16 at 5bccd0d7: 0) |
| `PartConnector` is the five-field struct | `grep -c 'rated_ma; unsigned contacts; char usb; } PartConnector' backend/include/parts.h` → 1 | MET (2026-11-16: 1) |

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

- [ ] `backend/include/parts.h`: `PartConnector` gains `uint32_t pitch_um;
      char terminal; char jst;` and its comment names them (zero for barrel
      and USB).
- [ ] `backend/src/models/connector.c`: `DEVICE_KEYS`, `terminal_load` and the
      branch of § 3; the file's header comment names terminal blocks and JST
      headers.
- [ ] `backend/tests/library_check.c`: `check_terminal_connector` (beside
      `check_usb_connector`): a fixture device for each clamp (`'terminal':'screw'`,
      `'spring'`, `'push-in'`, `'pluggable'`) loads with `pitch_um` 5080,
      `terminal` `'s'`/`'c'`/`'p'`/`'h'`, `rated_mv`/`rated_ma` from the keys
      and 0 when absent; a JST fixture for each of the seven series loads with
      `jst` the series' first letter and `pitch_um` 2500; `expect_fail` for:
      two of the three keys, an unknown clamp, an unknown series, no pitch
      (both), pitch 0, pitch 21, pitch on a USB device, `centerPin` and
      `contacts` with `terminal`, `ratedAmps` 0.

Acceptance: the loader accepts the new devices and rejects the malformed ones.
  Check: `cmake --build /tmp/logic-p45 -j8 --target library_check && /tmp/logic-p45/library_check` → every line `ok`, exit 0, including `ok   terminal block and JST header connectors load; malformed ones fail` (est. 5 min).
Commit: —

### Phase 2 — The listing and the circuit test

- [ ] `backend/tests/support/listing.c` (`case PART_CONNECTOR`): the two new
      lines of § 3 before the barrel line.
- [ ] `backend/tests/connector_test.c`: a fixture terminal block (3 pins) and
      a JST header (2 pins) beside the USB case: each part's pins 1 and 2 on
      two nets, one driven high from `5V`, the other from `GND`: after
      `circuit_start` and `circuit_advance(circuit, 1)` neither net is
      shorted; `part->connector->terminal` / `jst` and `pitch_um` as loaded.
- [ ] `/tmp/logic-p45/listing_check --print > /tmp/p45-parts.txt && diff
      /tmp/p45-parts.txt backend/tests/golden/parts.txt` → no output (no
      shipped part has the new keys yet).

Acceptance: the circuit and listing tests pass and the goldens are unchanged.
  Check: `cmake --build /tmp/logic-p45 -j8 && ctest --test-dir /tmp/logic-p45 -R 'connector_test|library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff --stat backend/tests/golden` → empty (est. 8 min).
Commit: —

### Phase 3 — Docs

- [ ] `docs/parts-format.md` § `connector`: a subsection "Terminal blocks and
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
- [ ] `docs/parts-format.md` § Devices table of kinds: no change (the kind is
      still `connector`); the intro list of kinds gets no new name.

Acceptance: the doc names every key and the drawers.
  Check: `grep -c '"terminal"\|"jst"\|"pitch"' docs/parts-format.md` → at least 3 (est. 10 min).
Commit: —

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

## Summary

F is a small loader change with three new device keys, the tests that pin
them and the doc. Its one hazard is the existing barrel rule, which the
existing negative test guards; the goldens must not move.
