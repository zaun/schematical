# plan-71-H: Parts — the four generics, `transceiver.json`, datasheets, footprints, goldens, the Part Editor's kind lists

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-71-G

H writes `Parts/transceiver.json` from the final pick. It adds the
datasheet entries and the footprints the parts use, regenerates the
goldens, and registers the kind in the Part Editor's lists and schemas.

References:

- plan-71-A / -D (`candidates.tsv` `add` rows, `specs.tsv`, the PDFs and
  `datasheets.tsv` files), plan-71-E / -F (`footprints-E.json`,
  `footprints-F.json`, `footprint-map.tsv`), plan-71-G § 3.1 (the format).
- `docs/parts-format.md` § Devices, § Parts, § Datasheets;
  `docs/backend.md` § Adding a kind step 6 (the Part Editor), § Tests (the
  goldens).
- `planning/completed/plan-48-G-timer-opamp-parts.md` § 3.1–3.4 (the
  precedent: devices per spec group, ids, generics, datasheets, footprints,
  the editor), `planning/plan-48-data/census/gen-parts.py`, `fp-merge.py`
  (the scripts H adapts); `scripts/part-editor-schema.py`.

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-71-G complete | `ls planning/completed/plan-71-G-*.md` → exists, or every G box ticked | NOT MET (2026-11-25: G not started) |
| Every `add` row has a footprint and a pinout | `python3 -I planning/plan-71-data/census/fp-check.py E` and `… F` → `0 without a footprint`; `python3 -I planning/plan-71-data/census/final.py --check` → `0 without a pinout` | NOT MET (D–F) |
| Golden sizes at H's start | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt backend/tests/golden/erc_pins.txt` | recorded here at H's start as **P1, R1, F1, E1** (P0 40832, R0 40832, F0 3993, E0 40832 at planning; other plans may have landed parts) |
| The part file is absent | `ls schematical/schematical/Parts/transceiver.json` → no such file | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/transceiver.json` holds the four generics and one part per `add`
  row (at planning up to 1,506 picked rows, A § 2).
- `backend/tests/golden/parts.txt`, `traces.txt` and `erc_pins.txt` each
  grow by exactly those parts (P1 / R1 / E1 + 4 + the `add` rows).
  `footprints.txt` grows by exactly the objects of `footprints-E.json` and
  `footprints-F.json`. No golden line is removed or changed.
- `Parts/datasheets.json` has an entry for every PDF an `add` row cites.
- The Part Editor knows the kind: `PartKindRoundTripTests` passes with
  `testTransceiver`.

### Non-goals (explicit constraints)

- No change to an existing part, footprint or datasheet entry.
- No hand-written part: every part comes from `gen-parts.py`. A wrong part
  is fixed in `specs.tsv`, `footprint-map.tsv` or the script.
- No app view changes beyond the editor's lists (I).

## 2. Current State

- `Parts/` has 51 JSON files (`ls schematical/schematical/Parts/*.json | wc -l`
  → 51 at 63e2f7d2) and no `transceiver.json`.
- `datasheets.json` has 10,962 entries (`python3 -c "import json;print(len(json.load(open('schematical/schematical/Parts/datasheets.json'))['datasheets']))"`
  → 10962).
- `KIND_FILES` in `scripts/part-editor-schema.py` names 45 kinds (the
  plan-48-G count command → 45).
- The Interface drawer category holds `Serial` (90 devices in
  `serial.json`), Parallel I/O and the display Drivers
  (`docs/parts-format.md` § The drawer's categories). H adds the group
  `Transceiver` with the sub-groups `CAN`, `LIN`, `RS-232`, `RS-485`.

## 3. Design

### 3.1 Devices and parts (`census/gen-parts.py`, from plan-48's)

- **One device per spec group** (protocol, company, PDF, signal map). A
  PDF with two signal maps (a half- and a full-duplex part, or standby vs
  silent) gives two devices. `pinouts` are the group's spec rows, keyed
  `<label lower case>-<pins>` (`soic-8`, `vson-8`).
- **Ids**: `<datasheet id>` (`tcan1042h`, `max3232e`), with `-b`, `-c` for
  a second and third device of one PDF.
- **Drawer and function**: `["Interface", "Transceiver", "CAN" | "LIN" | "RS-232" | "RS-485"]`.
  The function is `"CAN"` / `"CAN FD"` (the datasheet's title names FD),
  `"LIN"`, `"RS-232"`, or `"RS-485"` / `"RS-422"` (drivers or receivers
  only), plus `" isolated"` for an isolated part. `description` is the
  datasheet's title line.
- **Device keys** from the spec row's `facts`. `dataRate` comes from the
  device's most-stocked row's DigiKey Data Rate parameter, and goes on each
  part when the device's parts differ.
- **Parts**: `name` (the part number), `manufacturer` (`Texas
  Instruments`, `NXP`, `Microchip Technology`, `Analog Devices`,
  `MaxLinear`, `Renesas Electronics`, `onsemi`), `datasheet` (the id),
  `package`, `pinout`, `footprint` (`footprint-map.tsv`), and `"flags"`
  `["automotive"]` for an AEC-Q100 row and `["obsolete"]` per plan-71-A
  § 4.
- **Order**: devices by protocol, company, datasheet id; parts by name; the
  generics last.

### 3.2 The generics

`"manufacturer": "Generic"`, no datasheet, `footprintChoices` `DIP` → the
`PDIP-*` and `SOIC` → the `SOIC-*` of the same pin count.

| Part | Device / function | Pinout | Keys | Footprint |
|---|---|---|---|---|
| `GENERIC-CAN` | `can-generic`, `"Generic"` | 1 TXD, 2 GND, 3 VCC, 4 RXD, 5 NC, 6 CANL, 7 CANH, 8 S (the TJA1051 order) | G § 5's `test-can`, `dataRate` 1000000, `vcc` [4.5, 5.5] | `SOIC-8` |
| `GENERIC-RS485` | `rs485-generic` | 1 RO, 2 RE#, 3 DE, 4 DI, 5 GND, 6 A, 7 B, 8 VCC (the MAX485 order) | `test-485`, `failSafe` true, `dataRate` 2500000, `vcc` [4.75, 5.25] | `PDIP-8` |
| `GENERIC-RS232` | `rs232-generic` | the 16-pin MAX232 / MAX3232 order | `test-232`, `dataRate` 250000, `vcc` [3, 5.5] | `PDIP-16` |
| `GENERIC-LIN` | `lin-generic` | the TJA1021 order (G § 5 `test-lin`) | `test-lin`, `dataRate` 20000, `vcc` [5, 27] | `SOIC-8` |

### 3.3 Datasheets and footprints

The plan-48-G § 3.3 rules:

- one `datasheets.json` entry per cited PDF, `id` = the file name in lower
  case without `.pdf`, with the folder's `datasheets.tsv` URL;
- an existing id is not added again;
- `fp-merge.py --only <ids>` appends the E / F objects the parts name, and
  skips an id another plan already added (recorded in § Corrections).

### 3.4 The editor (`docs/backend.md` § Adding a kind step 6)

Each is added only if absent (`grep -c '"transceiver"'` first in each
file):

- `KindInfo` in `Editor/KindInfo.swift` (name "Line transceiver", blurb
  "CAN, RS-485, RS-232 or LIN driver and receiver", examples
  `GENERIC-CAN`, `GENERIC-RS485`, picker group Interface), and the kind in
  `PartKinds.all` and `infoOrder`;
- `PART_TRANSCEIVER` in `BundledPartFile.name(of:)` and the kind in
  `PartFacts`;
- `"transceiver": ["transceiver.c"]` in `scripts/part-editor-schema.py`
  `KIND_FILES` and in `PartKindKeysTests.files`;
- `python3 scripts/part-editor-schema.py . transceiver` →
  `Editor/TransceiverDraft.swift`, printing `missing []`. `protocol` is the
  schema's `variantKey`, with one `variantBlurbs` sentence per protocol in
  `Guides/VariantBlurbs.swift`;
- the schema in `PartKinds.schemas` (`Editor/KindSchema.swift`);
- `testTransceiver` with the shipped part count in
  `PartKindRoundTripTests`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The generics and the CAN and LIN parts

- [ ] `census/gen-parts.py` per § 3.1–3.2; `transceiver.json` with the
      generics and the `can` and `lin` rows.
- [ ] The datasheet entries and footprints those parts name (§ 3.3).
- [ ] Regenerate the goldens (`docs/backend.md` § Tests).

Acceptance: the parts load, and the goldens grow by exactly them.
  Check: `ctest --test-dir /tmp/logic-p71-h -j8` → `100% tests passed`; `git diff backend/tests/golden/parts.txt | grep -c '^-[^-]'` → 0; `grep -c '|transceiver|' backend/tests/golden/parts.txt` → 4 + the `can` and `lin` `add` rows (est. 5 min: build, regenerate, suite).
  Result:
Commit:

### Phase 2 — The RS-485 and RS-232 parts

- [ ] The `rs485` and `rs232` rows, their datasheets and footprints; goldens.

Acceptance: every `add` row is a part, and nothing else changed.
  Check: `grep -c '|transceiver|' backend/tests/golden/parts.txt` → 4 + `awk -F'\t' 'NR>1 && $NF=="add"' planning/plan-71-data/candidates.tsv | wc -l`; `git diff backend/tests/golden/ | grep -c '^-[^-]'` → 0; `wc -l backend/tests/golden/footprints.txt` → F1 + the E and F object count (est. 5 min).
  Result:
Commit:

### Phase 3 — The Part Editor

- [ ] § 3.4, every item.

Acceptance: the editor round-trips every shipped transceiver part.
  Check: `xcodebuild test -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p71-dd CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/KindGuideTests -only-testing:schematicalTests/WizardStepsTests -only-testing:schematicalTests/KindInfoTests` → `** TEST SUCCEEDED **` (est. 8 min: an app build; these five classes are the ones a new kind touches, `docs/backend.md` § Adding a kind).
  Result:
Commit:

## Corrections

(none yet)
