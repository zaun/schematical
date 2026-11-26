# plan-74-J: App: Ethernet details, facts, kind names, drawer, symbols, Part Editor, harness

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-74-I

J makes the app show the new parts:

- Ethernet ICs: their facts, their kind names, the drawer groups and the Part Editor kind.
- RJ45 jacks: their facts and kind name. They draw as plain blocks of their pins, like USB
  connectors.

The ICs already turn Run off, since `"simulated": false` is handled. J proves it on a real
schematic.

References:

- plan-74-E § 3.1 (`PartEthernet`), § 3.2 (`PartConnector.rj45`, `magnetics`, `poe`).
- `docs/backend.md` § Adding a kind, steps 5–6 (the Part Editor's lists, `part-editor-schema.py`,
  `PartKindRoundTripTests`, `PartKindKeysTests`, `KindGuideTests`, `variantBlurbs`).
- `schematical/schematical/Backend/PartCatalog.swift` (`Details`, line 680; the connector chain,
  lines 1601–1615; `symbol(for:)`, line 2028, connector case 2041–2047; `detailLabels`,
  line 1271); `Backend/PartFacts.swift` (`kindName(_ kind:)`, line 377; `kindName(_ part:)`,
  line 478; the connector rows, line 147); `Preferences/ReferencePrefixes.swift:95`;
  `Editor/BundledPartFile.swift:136`; `Editor/KindInfo.swift`;
  `Editor/Wizard/Guides/VariantBlurbs.swift:78`.
- Precedent: `planning/completed/plan-48-H-timer-opamp-app.md`,
  `scripts/harness/terminal-app.swift` (a connector harness).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-I complete | `ls planning/completed/plan-74-I-*.md` → exists, or every I box ticked | NOT MET (2026-11-25: I not started) |
| The app does not know the kind yet | `grep -c 'PART_ETHERNET' schematical/schematical/Backend/PartCatalog.swift schematical/schematical/Backend/PartFacts.swift schematical/schematical/Editor/BundledPartFile.swift` → 0, 0, 0 | MET (2026-11-25: 0, 0, 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

On the app's real catalog (`scripts/harness/ethernet-app.swift`, every line `ok`):

- The drawer has **Interface → Ethernet → PHY** and **→ Controller**, and **Connector → RJ45**.
- `W5500` placed: reference `U1`, kind name "Ethernet controller", a block symbol with its pins.
  Facts rows: `Type` Controller, `Speed` 10/100 Mbit/s, `Host interface` SPI, `TCP/IP` Hardwired,
  `Supply` its range, and `Play mode` `Not simulated`. Run is off
  (`SimulationAvailability.canStart` false, with the existing reason).
- A `LAN8720A` part: kind name "Ethernet PHY", `Host interface` RMII.
- A Bel MagJack: reference `J1`, kind name "RJ45 jack", a block symbol with its pins and `SHIELD`.
  Facts rows: `Type` RJ45, `Speed` 10/100 or 10/100/1000, `Magnetics` Yes, `PoE` (only when 1).
  Detail labels `["RJ45", "10/100", "magnetics"]`.
- File › New Part… offers the Ethernet kind: `PartKindRoundTripTests` round-trips every shipped
  `ethernet` part, and `connector` parts with `rj45`, with 0 mismatched.

### Non-goals (explicit constraints)

- No new symbol: an Ethernet IC is a block, as is a jack.
- No new reference category: ICs are `U`, jacks `J` (`.femaleConnector`, the existing connector case).
- No change to `SimulationAvailability`: `"simulated": false` already turns Run off.
- No LED lighting on a jack.

## 2. Current State

- `CatalogPart.Details` has a case per facts shape. Connectors branch on test point → card →
  terminal/JST (`.wireConnector`) → barrel/USB (`.connector`) (`PartCatalog.swift:1601–1615`).
  A connector of an unknown mode falls into `.connector` with `usb` nil: a "Barrel connector"
  drawn as `.barrelJack`. So an RJ45 part shows as a barrel jack until J lands.
- `kindName(_ kind:)` has no `PART_ETHERNET` case, and returns "" for an unnamed kind.
- `BundledPartFile.name(of:)` names a part file per kind (`PART_OPAMP: "opamp"`, line 136).
- `ReferenceCategory.of` returns `.femaleConnector` for `PART_CONNECTOR` that is not a test
  point (`ReferencePrefixes.swift:95`), and `.integratedCircuit` by default.

## 3. Design

### 3.1 Catalog

```swift
struct EthernetInfo: Hashable, Sendable { let type: String; let speedMbps: UInt32; let host: [String]; let tcpip: Bool }
```

- `Details.ethernet(EthernetInfo)`, with a builder branch on `part.ethernet?.pointee` (the host
  bits → names in `ETHERNET_HOST_*` order) and an accessor `ethernet`.
- `Details.rj45(speed: String, magnetics: Bool, poe: Bool, ratedMV: UInt32, ratedMA: UInt32)`: a
  new branch in the connector chain, before `.connector`, on `connector.rj45 != 0` (`'F'` →
  `"10/100"`, `'G'` → `"10/100/1000"`).
- `symbol(for:)`: `.rj45` → `.block`.
- `detailLabels`: Ethernet: speed text and host names (`["10/100", "SPI"]`). RJ45: `["RJ45",
  speed] + (magnetics ? ["magnetics"] : []) + (poe ? ["PoE"] : [])`.

### 3.2 Facts and names

- `kindName(_ kind:)`: `PART_ETHERNET` → "Ethernet".
- `kindName(_ part:)`: Ethernet → "Ethernet PHY" / "Ethernet controller"; `.rj45` → "RJ45 jack".
- Rows:
  - Ethernet: `Type`, `Speed` (`10 Mbit/s`, `10/100 Mbit/s`, `10/100/1000 Mbit/s`), `Host
    interface` (upper case, comma list), `TCP/IP` (`Hardwired`, only when set), and `Play mode`
    `Not simulated`.
  - RJ45: `Type` RJ45, `Speed`, `Magnetics` Yes / No, `PoE` Yes (only when set), and `Rating` when
    given.

### 3.3 Part Editor

- `KindInfo` entry "Ethernet": the blurb "An Ethernet PHY or controller (facts only, not
  simulated)", examples `W5500`, `LAN8720A-CP`, group `.processors` (the serial kind's, `KindInfo.swift:103`), `simulated:
  false`. `PartKinds.all` and `infoOrder` get `"ethernet"`.
- `BundledPartFile.name(of:)`: `PART_ETHERNET: "ethernet"`.
- `scripts/part-editor-schema.py`: `KIND_FILES["ethernet"] = ["ethernet.c"]`, and the same in
  `PartKindKeysTests.files`. Run `python3 scripts/part-editor-schema.py . ethernet`, which writes
  `Editor/EthernetDraft.swift` (it must print `missing []`). `type` is the `variantKey`, so
  `VariantBlurbs.swift` gets `"ethernet": ["phy": …, "controller": …]`. The schema goes in
  `PartKinds.schemas`.
- Connector: `python3 scripts/part-editor-schema.py . connector` regenerates `ConnectorDraft.swift`
  with `rj45`, `magnetics` and `poe`. The connector has no curated guide (`ls
  Editor/Wizard/Guides/` has none), so its wizard steps are derived.
- `PartKindRoundTripTests`: `testEthernet` and the shipped count; the connector count grows by
  the jacks.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Catalog, facts, names

- [ ] `Backend/PartCatalog.swift` per § 3.1; `Backend/PartFacts.swift` per § 3.2 (each case only if
      absent).
- [ ] `schematicalTests/EthernetTests.swift`:
  - kind names, facts rows and detail labels of `W5500`, the first `LAN8720A` part, and the first
    `rj45-bel-` part;
  - `ReferenceCategory.of` → `.integratedCircuit` / `.femaleConnector`;
  - `symbol(for:)` → `.block` for both;
  - `SimulationAvailability` refuses a document with a placed `W5500`.

Acceptance: the tests pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p74-dd test -only-testing:schematicalTests/EthernetTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min: an app build).
  Result:
Commit:

### Phase 2 — Part Editor

- [ ] § 3.3.

Acceptance: the editor's kind tests pass.
  Check: the same `xcodebuild … test` with `-only-testing:` `PartKindRoundTripTests`, `PartKindKeysTests`, `KindGuideTests`, `WizardStepsTests`, `WizardModelTests`, `KindInfoTests`, `PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 10 min).
  Result:
Commit:

### Phase 3 — Harness

- [ ] `scripts/harness/ethernet-app.swift` (from `terminal-app.swift`): the drawer groups, placing
      `W5500` and a Bel jack, their names, references and facts, and Run off with the `W5500` placed.
- [ ] `DrawerPerformanceTests` stays green.

Acceptance: every harness line `ok`.
  Check: `APP=/tmp/p74-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/ethernet-app.swift /tmp/p74-harness && /tmp/p74-harness/H.app/Contents/MacOS/harness` → `all checks ok` (est. 3 min after the app build).
  Result:
Commit:

## Validation Plan

- The phase checks. Final gate: plan-74-K.

## Decisions

- Author: **RJ45 is its own `Details` case**, not `.connector` with another optional. The USB case
  already overloads `.connector`, and RJ45's facts (speed, magnetics, PoE) share nothing with a
  barrel jack's.
- Author: **the IC kind name says PHY or controller**: those are the two parts a designer picks
  between.

## Corrections

## Summary

The app learns one kind and one connector mode: names, facts, drawer, editor, and a harness run
proving it on the real catalog.
