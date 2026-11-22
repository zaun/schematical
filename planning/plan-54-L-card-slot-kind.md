# plan-54-L: Backend — card slots in the `connector` kind

Last updated: 2026-11-22
Effort: medium (1h–2h)
Depends on: plan-54-K

L teaches the `connector` kind a third kind of device: a **card slot** (`"card"`:
what the slot takes, `"cardPitch"`: the contact pitch). It is a passive terminal
like the USB connectors: no model change, the same simulation component
(`CONNECTOR_MODEL`), nothing driven. L changes the loader, the part facts the
listing prints, the tests and `docs/parts-format.md` (including the naming
conventions M and N follow); it adds no part (M and N do). The behavioral outcome:
a connector device with `"card"` (and optionally `"cardPitch"`) loads and
exposes `card` and `card_pitch_um` in `PartConnector`; every malformed one is
rejected with a message that names the key; the 224 existing connector devices
and every golden line are unchanged.

References:

- `docs/parts-format.md` § `connector`, § Devices, § Values;
  `docs/backend.md` § Tests (`library_check`, `listing_check`).
- `backend/src/models/connector.c` (the loader; the USB precedent `usb_load`),
  `backend/include/parts.h:132` (`PartConnector`),
  `backend/tests/support/listing.c` (the connector facts, `case PART_CONNECTOR`),
  `backend/tests/library_check.c` (`check_usb_connector`),
  `backend/tests/connector_test.c`.

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-K complete | `ls planning/completed/plan-54-K-*.md` → exists | NOT MET (2026-11-22: not started) |
| The connector kind has no card key | `grep -c '"card"\|"cardPitch"' backend/src/models/connector.c` → 0 | MET (2026-11-22 at 7b449718: 0) |
| `PartConnector` has no `card` member | `grep -c 'const char \*card;' backend/include/parts.h` → 0 | MET (2026-11-22 at 7b449718: 0; the struct is the five-field `{center_diameter_um, rated_mv, rated_ma, contacts, usb}`, `parts.h:132`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- A connector device may name `"card"` (a string of 1 to 40 bytes: `"microSD"`,
  `"DDR4 SODIMM"`, `"PCI Express"`) and optionally `"cardPitch"` (mm, above 0 and
  at most 20). A card slot has no `centerPin`, `contacts`, `usb`, `ratedVolts` or
  `ratedAmps`.
- `PartConnector` has `const char *card` (NULL for barrel and USB devices) and
  `uint32_t card_pitch_um` (0 when `cardPitch` is absent and for barrel and USB).
- `library_check` rejects each malformed device with a message naming the key;
  `connector_test` shows a card slot joining nothing; `listing_check` prints its facts;
  the existing goldens do not change (`git diff --stat backend/tests/golden` → empty).

### Non-goals (explicit constraints)

- No new kind, no new `PART_*`, no new test program (T0 stays), no change to the
  engine model, to `circuit.c` or to the barrel and USB devices.
- No part is added; the parts are M's and N's.
- The app's `PartDetails` is O's: this letter does not touch Swift, and the app still
  builds because it reads only the fields it knew.

## 2. Current State

`connector.c` loads a device that is a USB connector (`"usb"`) or a barrel jack
(`centerPin`, `ratedVolts`, `ratedAmps`, optional `contacts`): `DEVICE_KEYS` is
`{"centerPin", "ratedVolts", "ratedAmps", "contacts", "usb", NULL}`; `usb_load` rejects
`centerPin` and `contacts` and makes the ratings optional; `connector_bind_pinout`
needs two or more pins and binds `contacts` (none for USB). `PartConnector` is
`{center_diameter_um, rated_mv, rated_ma, contacts, usb}`; the listing prints
`usb=A rated=5V/900mA` for USB and `center_um=… rated=… contacts=…` for a barrel
jack. `docs/parts-format.md` § `connector` describes both. The kind rejects keys
it does not know (`DEVICE_KEYS`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Connector devices / parts in the library | 224 / 376 | plan-54-A § 2 |
| Backend test programs | 41 | `grep -c '^logic_test' backend/CMakeLists.txt` |
| Golden lines for connectors | 376 | `grep -c '\|connector\|' backend/tests/golden/parts.txt` |
| Largest pinout of a library part | 196 pins (`ADS5401IZAY`); card slots go to 694 | `awk -F'\|' '{n=gsub(/,/,",",$9)+1; if(n>m){m=n}} END{print m}' backend/tests/golden/parts.txt` → 196 |

### Verified properties

- `usb_load` is the pattern: it reads `kind_string`, validates, calls `kind_volts` and
  `kind_number` for the optional ratings (`connector.c`, `usb_load`).
- A connector with `contacts` 0 has an empty `ConnectorPins` and never calls
  `sim_set_contact` (`connector_bind_pinout` returns `bound` with `count` 0,
  `connector_set` returns 0), so a card slot is inert, as USB is.
- `connector_bind_pinout` needs at least two pins and has no upper limit; a pinout of
  any length loads (the footprint limit is 1000 pads, `footprints.c:17`).
- Strings the loader keeps are copied with `kind_strdup` (`connector.c`, the
  `contacts` pin names), so `card` is arena-owned like them.

## 3. Design

### 3.1 The loader

`connector_load_device` branches first on `"card"`: a device that has it goes to
`card_load`, which rejects the barrel and USB keys; the existing `"usb"` and barrel
branches are unchanged.

| Key | Rule | Message |
|---|---|---|
| `card` | string, 1 – 40 bytes | `"card" must be a string of 1 to 40 characters` |
| `cardPitch` | optional, number, above 0, at most 20 | `"cardPitch" must be above 0 and at most 20 mm` |
| `cardPitch` without `card` | rejected | `"cardPitch" is only for card slots` |
| `usb`, `centerPin`, `contacts`, `ratedVolts`, `ratedAmps` with `card` | rejected | `a card slot has no "usb"` (and `"centerPin"`, `"contacts"`, `"ratedVolts"`, `"ratedAmps"`) |

`DEVICE_KEYS` gains `"card"` and `"cardPitch"`. `PartConnector.card` is NULL for a
barrel jack and a USB connector, so their facts do not change.

`listing.c` prints, for a card slot, `card=<text> pitch_um=<n>` (`pitch_um=0` when
absent) before the barrel line; barrel and USB lines are unchanged.

**Correctness risk** is small and concentrated in the one rule that touches existing
devices: a barrel jack must still require `centerPin`. The existing negative test
(`barrel: still needs its centre pin`) covers it; L adds the new keys' cases beside it.

**Byte-identity.** The goldens (`parts.txt`, `traces.txt`, `footprints.txt`) are the gate for
the existing parts: they do not change. The expected diff is empty.

### 3.2 The naming conventions (M and N write the parts by them)

- **Pins.** Passive `inout` terminals (`nc` for a pad the datasheet marks not
  connected), named as plan-54-B § 3.3 and plan-54-C § 3.3: card sockets by function
  (`CLK`, `CMD`, `DAT0`, `VCC`, `RST`) with one `SHIELD` pin for the shell; module and
  card-edge sockets by the contact designator (`1`, `288`, `A1`, `B12`), the pinout
  numbered 1 … N; a pinout of more than 40 pins has `"side": "in"` on its first half
  (rounded up), so the block symbol has two columns.
- **Drawer.** `["Connector", "Card slot", "<leaf>"]`; leaf `SD`, `microSD`, `SIM` or
  `Smart card` for a memory / SIM socket (DigiKey's card types: Secure Digital – SD and
  SD → `SD`; microSD, Secure Digital – microSD, microSD + UFS → `microSD`; SIM Card, Mini,
  Micro and Nano SIM, SIM / SAM → `SIM`; Smart Card → `Smart card`), `Memory module` for
  a module socket, `Card edge` for a card-edge socket.
- **Function.** The card text (`card`) for a memory / SIM socket (`Nano SIM`, `microSD`)
  and a module socket (`DDR4 SODIMM`); for a card-edge socket `<pitch> mm <card>`
  (`1.00 mm PCI Express`, `0.80 mm Dual edge`).
- **Package** (the part's `package`): from DigiKey's mounting type — `Surface Mount,
  Right Angle` → `SMD right angle`; `Surface Mount` → `SMD vertical`; `Through Hole` →
  `Through hole vertical`; `Through Hole, Right Angle` → `Through hole right angle`;
  `Surface Mount, Through Hole` → `Hybrid vertical`; `Surface Mount, Right Angle; Through
  Hole` → `Hybrid right angle`; `Board Edge, Straddle Mount` → `Board edge straddle`; a part DigiKey gives `-` takes its datasheet's mounting.
- **Device `id`.** `card-<family>-<maker slug>-<variant slug>` in lower case, `<family>` `socket`
  (memory / SIM), `module` or `edge` (`card-socket-gct-sim8055-nano-sim`),
  one device per variant of `candidates.tsv` (the `variant` column), its pinouts `p<NN>` for
  the position counts its parts use.
- **Part** `name` the DigiKey manufacturer part number; `manufacturer` `Same Sky`,
  `Amphenol`, `GCT`, `TE Connectivity`, `Molex` or `Samtec`; `datasheet` the id of the part's
  PDF; `footprint` `<stem>-<positions>` (H to K § 3.1).

Rejected alternative: a new `cardslot` kind (rejected: it would copy `connector.c` and the
app's switches; the USB connectors are the precedent for a passive connector family inside
`connector`). Rejected: sharing a `pitch` key with other connector families (rejected: this plan
is independent of plan-45, which also adds a `pitch` key with other rules).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The loader and its tests

- [ ] Re-read `backend/include/parts.h` (`PartConnector`) and `connector.c` on the state
      `main` has now (`git log --oneline -5 -- backend/src/models/connector.c`); another plan may
      have added members and keys: edit by hunk.
- [ ] `backend/include/parts.h`: `PartConnector` gains `const char *card; uint32_t
      card_pitch_um;` after its last member, and the comment names them (NULL / 0 for barrel
      and USB).
- [ ] `backend/src/models/connector.c`: `DEVICE_KEYS` gains `"card"`, `"cardPitch"`;
      `card_load` and the first branch of § 3.1; the header comment names card slots.
- [ ] `backend/tests/library_check.c`: `check_card_slot_connector` (beside
      `check_usb_connector`): a fixture device with `'card':'microSD'` loads with `card` equal
      `"microSD"` and `card_pitch_um` 0; one with `'card':'PCI Express','cardPitch':1.0` loads
      with 1000; `expect_fail` for: `card` as a number, empty `card`, a 41-byte `card`,
      `cardPitch` 0, `cardPitch` 21, `cardPitch` without `card`, and each of `usb`,
      `centerPin`, `contacts`, `ratedVolts`, `ratedAmps` with `card`.

Acceptance: the loader accepts card slots and rejects the malformed ones.
  Check: `cmake -S backend -B /tmp/logic-p54 && cmake --build /tmp/logic-p54 -j8 --target library_check && /tmp/logic-p54/library_check` → every line `ok`, exit 0, including `ok   card slot connectors load; malformed ones fail` (est. 8 min: a first build).
Commit: —

### Phase 2 — The listing and the circuit test

- [ ] `backend/tests/support/listing.c` (`case PART_CONNECTOR`): the card-slot line of § 3.1
      before the barrel line.
- [ ] `backend/tests/connector_test.c`: a fixture card slot (3 pins) beside the USB case: pins 1
      and 2 on two nets, one driven high from `5V`, the other from `GND`: after `circuit_start`
      and `circuit_advance(circuit, 1)` neither net is shorted; `part->connector->card` and
      `card_pitch_um` as loaded.
- [ ] `/tmp/logic-p54/listing_check --print > /tmp/p54-parts.txt && diff /tmp/p54-parts.txt
      backend/tests/golden/parts.txt` → no output (no shipped part has the new keys yet).

Acceptance: the circuit and listing tests pass and the goldens are unchanged.
  Check: `cmake --build /tmp/logic-p54 -j8 && ctest --test-dir /tmp/logic-p54 -R 'connector_test|library_check|listing_check|trace_check|footprint_check'` → `100% tests passed`; `git diff --stat backend/tests/golden` → empty (est. 8 min).
Commit: —

### Phase 3 — Docs

- [ ] `docs/parts-format.md` § `connector`: a subsection "Card slots": the keys and rules of § 3.1;
      no `centerPin`, `contacts`, `usb`, ratings; every pin a passive `inout` terminal named by
      § 3.2; the drawer, function, package and device-id conventions of § 3.2; footprints
      `SOCK-…`, `DIMM-…`, `EDGE-…` (plan-54-H to K); Active parts only; no generic. The intro
      sentence names card slots beside "Barrel power connectors" and the USB paragraph.

Acceptance: the doc names every key and the drawers.
  Check: `grep -c '"card"\|"cardPitch"' docs/parts-format.md` → at least 3 (est. 10 min).
Commit: —

## Validation Plan

- Tests: `library_check` (the new cases, negative included), `connector_test`,
  `listing_check` / `trace_check` / `footprint_check` (unchanged goldens).
- Coverage check: `connector.c` is in the `logic` library that every test links through
  `logic_test_support` (`add_library(logic STATIC` in `backend/CMakeLists.txt`).
- Runtime proof: `connector_test`'s fixture card slot joins nothing.
- Doc sync: `docs/parts-format.md` § `connector`.
- Final gate: none; the landing tier is P's.

## Decisions

- Author: **Extend `connector`, not a new kind** (§ 3.2 rejected alternative); **own keys
  `card` / `cardPitch`**, so the plan is independent of plan-45 (owner, 2026-11-22: independent).
- Author: **No ratings on a card slot**: the datasheets of these families rate the contact, not
  the slot, and no part would use them.
- Author: **`card` is free text of 1 – 40 bytes**, not a closed list: its values are DigiKey's
  card types and memory types of the pick, shown in the sidebar and grouped by the part's
  `drawer`, not by the loader.

## Corrections

## Summary

L is a small loader change with two device keys, the tests that pin them and the doc with the
naming conventions. Its one hazard is the existing barrel rule, which the existing negative
test guards; the goldens must not move.
