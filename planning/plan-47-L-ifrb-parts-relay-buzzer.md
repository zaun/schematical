# plan-47-L: Parts — relays and buzzers; the goldens and editor lists

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-K

L fills `Parts/relay.json` and `Parts/buzzer.json`: one part per relay and
buzzer `add` row of `planning/plan-47-data/candidates.tsv` (1153 at planning
time: 600 + 553), with the footprints G and H drew, the datasheet entries,
the editor's kind lists and the goldens. The behavioral outcome: the library
loads the two new part files; `grep -c '|relay|'
backend/tests/golden/parts.txt` and `grep -c '|buzzer|'` are the relay and
buzzer `add` rows; the goldens grow by exactly the new lines; `ctest`
passes. With L every part of plan-47 is in the library.

References:

- plan-47-K § 3 (the generator, the datasheet entries, the editor lists'
  steps, the golden procedure): L extends `census/gen-parts.py` with two
  types and repeats the steps for two kinds.
- plan-47-C § 3.2, § 3.3 (`specs.tsv` relay and buzzer columns), plan-47-G
  § 3.2 (`renumber`), plan-47-G and H (`footprints-G.json`, `-H.json`,
  `-H-explicit.json`, `footprint-map.tsv`), plan-47-J § 3.1 and § 3.4 (the
  kinds' keys).
- `docs/parts-format.md` § `switch` (the tactile-switch device shape: one
  device per contact arrangement), § Parts (flags).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-K complete | `ls planning/completed/plan-47-K-*.md` → exists | NOT MET (2026-11-16: not started) |
| The kinds exist | `grep -c 'RELAY_KIND\|BUZZER_KIND' backend/src/library/kinds.c` → 4 | NOT MET (2026-11-16: 0) |
| Every G and H line has a footprint that matches its pinout | `python3 -I planning/plan-47-data/census/fp-check.py G H` → `0 without a footprint`; `python3 -I planning/plan-47-data/census/fp-pins.py G` and `… H` → `0 errors` | NOT MET (2026-11-16: no map) |
| Golden sizes re-measured | `wc -l backend/tests/golden/{parts,traces,footprints}.txt` | record P0 / R0 / F0 at L's start (after K: the baseline + K's parts) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `ctest --test-dir /tmp/logic-p47-l -j8` → `100% tests passed`.
- `backend/tests/golden/parts.txt` and `traces.txt` grow by exactly the
  relay and buzzer `add` rows (1153 at planning time); `footprints.txt` by
  the footprints those parts use; no existing line changes.
- `PartKindRoundTripTests.testRelay` and `testBuzzer` pass with the shipped
  counts; `PartKindKeysTests` passes.
- Every part of plan-47 is loaded: the four kinds' golden counts sum to the
  `add` rows (2953 at planning time).

### Non-goals (explicit constraints)

- No change to the backend models or the app's symbols and views.
- No existing part, footprint or datasheet entry changes.
- No generic relay or buzzer.

## 2. Current State

After K: `inductor.json` and `fuse.json` exist, `gen-parts.py` writes them,
the editor knows the two kinds. `relay.json` and `buzzer.json` do not exist;
the G and H footprints are in `planning/plan-47-data/`.

### Measured populations

| What | Count | Command |
|---|---|---|
| `add` rows of L | the final pick's relay and buzzer rows (600 + 553 at planning time) | `awk -F'\t' '$23=="add"{print $3}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c` |
| …relay classes (planning pick) | power 353, signal 154, ssr 44, auto 44, reed 5 | `awk -F'\t' '$23=="add" && $3=="relay"{print $4}' … \| sort \| uniq -c` (`candidate` at planning time) |
| …relay contact forms and coil types | plan-47-J § 2 | there |
| …buzzer classes (planning pick) | transducer 331, indicator 222 | same, `$3=="buzzer"` |
| Military relays / AEC-Q200 rows among them | 19 / 0 | `awk -F'\t' '$23=="add" && ($3=="relay" \|\| $3=="buzzer"){print $14}' … \| sort \| uniq -c` |
| Distinct datasheet URLs (download set) | relay 171, buzzer 569 | plan-47-C § 2 |
| Relay coil voltages (planning pick, electromechanical and reed) | 12 V 174, 24 V 107, 5 V 83, 6 V 36, 3 V 33, 9 V 27, 26.5 V 21, 18 V 19, 4.5 V 15, 48 V 11, 15 V 9, the rest under 7 each | `awk -F'\t' '$23=="candidate" && $3=="relay"{print $8}' … \| sort \| uniq -c \| sort -rn` |

### Verified properties

- **A device's pinouts are shared by its parts**, and a part names one
  (`docs/parts-format.md` § Pinouts and pins): relays of one contact
  arrangement, coil arrangement and pin numbering share a device; the
  generator groups by exactly those.
- **The drawer has up to three levels and groups leaves by `function`**
  (`Backend/PartCatalog.swift` `functionGroups`): Switches & Relays → Relay → Power → DPDT.

## 3. Design

### 3.1 Relay devices and parts

`gen-parts.py inductor fuse ptc relay` writes `Parts/relay.json`:

- one device per (class, `rtype`, coils, `polarized`, `latching`, contacts,
  pinout) of `specs.tsv`: `"kind": "relay"`, `"type"`, `"coils"`,
  `"latching"`, `"contacts"` exactly as plan-47-J § 3.1 from the specs
  columns (`coils` `A1-A2` → `[{"a": "A1", "b": "A2"}]`, `polarized` on the
  entry; `contacts` `COM/NO/NC;…` → entries, `@n` → `"coil": n`);
- `drawer`: `["Switches & Relays", "Relay", "Signal"]`, `["Switches & Relays",
  "Relay", "Power"]`, `["Switches & Relays", "Relay", "Reed"]`, `["Switches & Relays", "Relay",
  "Solid state"]`, `["Switches & Relays", "Relay", "Automotive"]` (the
  class); `function`: the contact arrangement from the device's contacts:
  `SPST-NO`, `SPST-NC`, `SPDT`, `DPST-NO`, `DPST-NC`, `DPDT`, `3PDT`,
  `4PDT`, `4PST-NO`; a mix as `1 NO + 1 NC`, `2 NO + 2 NC`; several
  channels as `2 × SPST-NO`, `2 × SPDT`, `4 × SPST-NO`; a latching device
  appends ` latching` (`DPDT latching`);
- `id`: `relay_<class>_<function slug>_<pinout key>`, a counter suffix on a
  clash; `description`: "Signal relay", "Power relay", "Reed relay",
  "Solid-state relay", "Automotive relay"; `note`: the datasheets and pages,
  the view of the pin numbering and, for a renumbered pinout, the datasheet's
  own terminal numbers (plan-47-G § 3.2);
- one part per row: `name`, `manufacturer` (`TE Connectivity`, `Franjobaim`,
  `Panasonic`), `datasheet`, `package` (the specs `label`), `pinout`,
  `footprint`, and the part keys of plan-47-C § 3.1 (`coilVolts`,
  `coilOhms`, `contactAmps`, `contactVolts`, `operateMs`, `releaseMs`,
  `onOhms`), an optional key omitted when blank; `"flags": ["military"]` for
  a Military-grade row, `["automotive"]` for an AEC-Q200 row.

### 3.2 Buzzer devices and parts

`gen-parts.py … buzzer` writes `Parts/buzzer.json`:

- one device per (`driver`, `technology`, pinout): `"kind": "buzzer"`,
  `"driver"`, `"technology"`, `"positive": "+"`, `"negative": "-"`;
  `drawer` `["Displays & Indicators", "Buzzers"]`; `function` `Indicator (built-in
  driver)` or `Transducer (external drive)`; `id`
  `buzzer_<driver>_<technology>_<pinout key>`; `description` "Piezo buzzer"
  or "Magnetic buzzer";
- one part per row: `name`, `manufacturer` (`Same Sky`, `PUI Audio`,
  `Mallory Sonalert`), `datasheet`, `package` (`THT` or `SMD`), `pinout`,
  `footprint`, `hz`, and the optional `volts`, `voltsMin`, `voltsMax`, `ma`,
  `db`.

### 3.3 The editor's kind lists

As plan-47-K § 3.2, for `relay` and `buzzer` (each only if absent):
`PartKinds.all`, `infoOrder` (`"relay"` after `"switch"`, `"buzzer"` after
`"led"`); `KindInfo(kind: "relay", name: "Relay", blurb: "A coil that moves
contacts: electromechanical, reed and solid-state relays", examples: <the
most-stocked signal and power relay>, group: .connectors)`;
`KindInfo(kind: "buzzer", name: "Buzzer", blurb: "A sounder that plays
while its + pin is high and its − pin low", examples: <the most-stocked
indicator and transducer>, group: .displays)`; `BundledPartFile.name(of:)`;
`KIND_FILES` and `PartKindKeysTests.files` (`"relay": ["relay.c"],
"buzzer": ["buzzer.c"]`); `python3 scripts/part-editor-schema.py . relay`
and `… buzzer` → `missing []` → `Editor/RelayDraft.swift`,
`Editor/BuzzerDraft.swift`; `PartKinds.schemas`; `testRelay`, `testBuzzer`
and their `expected` counts. The relay kind has a `"type"` with three values
and no per-type key set the form must switch on beyond the two
solid-state-only rules, so it gets no `variantKey` and no `FieldCondition`
table.

### 3.4 Docs

`docs/parts-format.md`: the `### relay` and `### buzzer` sections (J wrote
the keys) gain the library paragraph (makers, drawer paths, the pin names of
plan-47-C § 3.2, the renumbering note, a JSON example device with one part
each); § Parts Flags: `military` also names "MIL-qualified relays (DigiKey
grade Military, MIL-PRF-39016 and the like)"; the § Datasheets paragraph of
K gains L's hosts.

Where the risk concentrates: the relay device grouping (a part attached to
a device whose contacts are not its own) and the specs-to-JSON translation
of `coils` and `contacts`. The loader checks every name against the pinout,
`gen-parts.py --check` re-reads the file against the rows, and
`relay_check` (Phase 1) counts poles per part against DigiKey's contact form.

Rejected alternatives: (1) one device per series (rejected: the drawer leaf
is the contact arrangement, and 157 series would make 157 leaves' worth of
devices with one pinout each); (2) coil voltage as the drawer leaf
(rejected: the arrangement is what a schematic needs first; the coil voltage
is on the drawer row, plan-47-M).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The relays

- [ ] `gen-parts.py`: the `relay` type (§ 3.1) and `--check`'s pole count
      (`relay_check`: each part's device poles and channels against the
      row's DigiKey contact form).
- [ ] `gen-parts.py inductor fuse ptc relay` → `Parts/relay.json` and the
      datasheet entries; merge the G footprints the parts name; regenerate
      the goldens.
- [ ] The editor lists for `relay` (§ 3.3).

Acceptance: the relays load; goldens grow by exactly the new lines.
  Check: `cmake -S backend -B /tmp/logic-p47-l && cmake --build /tmp/logic-p47-l -j8 && ctest --test-dir /tmp/logic-p47-l -j8` → `100% tests passed`; `grep -c '|relay|' backend/tests/golden/parts.txt` = the relay `add` rows; `git diff backend/tests/golden \| grep -c '^-[^-]'` → 0; `python3 -I planning/plan-47-data/census/gen-parts.py relay --check` → `0 differences` (est. 8 min).
Commit: —

### Phase 2 — The buzzers

- [ ] `gen-parts.py`: the `buzzer` type (§ 3.2); `gen-parts.py inductor fuse
      ptc relay buzzer` → `Parts/buzzer.json`; merge the H footprints;
      regenerate the goldens.
- [ ] The editor lists for `buzzer` (§ 3.3).

Acceptance: the buzzers load; every part of plan-47 is in the library; the editor knows all four kinds.
  Check: `cmake --build /tmp/logic-p47-l -j8 && ctest --test-dir /tmp/logic-p47-l -j8` → `100% tests passed`; `grep -c '|inductor|\||fuse|\||relay|\||buzzer|' backend/tests/golden/parts.txt` = `awk -F'\t' '$23=="add"' planning/plan-47-data/candidates.tsv \| wc -l`; `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/PartEditorWindowTests` → `** TEST SUCCEEDED **` (est. 12 min).
Commit: —

### Phase 3 — Docs, the datasheet URL check

- [ ] `docs/parts-format.md` per § 3.4.
- [ ] `node scripts/check-datasheets.js --host <host>` for L's hosts
      (`www.te.com`, `assets.franjobaim.com`, `industry.panasonic.com`,
      `mediap.industry.panasonic.eu`, `www.sameskydevices.com`,
      `puiaudio.com`, `api.puiaudio.com`, `mspindy.com`); record each
      result in § Corrections and in the § Datasheets paragraph.

Acceptance: the docs describe the library's relays and buzzers.
  Check: `grep -c 'Relay → \|Displays & Indicators → Buzzers' docs/parts-format.md` → at least 2; `grep -c 'www.te.com' docs/parts-format.md` → at least 1 (est. 10 min).
Commit: —

## Validation Plan

- Tests: `ctest`; `PartKindRoundTripTests`, `PartKindKeysTests`,
  `PartEditorWindowTests`.
- Coverage check: the golden counts equal the `add` rows, per kind and in
  total; `gen-parts.py --check` with the pole count.
- Runtime proof: n/a in L (N runs relays and buzzers in the app; J's tests
  run the models).
- Doc sync: § 3.4.
- Final gate: plan-47-O.

## Decisions

- **Drawer paths**: Switches & Relays → Relay → Signal / Power / Reed / Solid state /
  Automotive → the contact arrangement; Displays & Indicators → Buzzers → Indicator /
  Transducer (§ 3.1, § 3.2). Relay is a group of the existing Switches & Relays category
  (the drawer regrouping renamed Switch and merged Indicators into Displays & Indicators).
- **A latching relay is its own leaf** (`DPDT latching`): it is wired
  differently.
- **`manufacturer` is `TE Connectivity`** for all four of TE's DigiKey maker
  ids (Aerospace Defense and Marine, Potter & Brumfield, AMP, TE).
- **The editor schemas are generated**; no curated guide.

## Corrections

## Summary

L's risk is the relay device grouping and the translation of `specs.tsv`'s
coil and contact columns into device keys; the loader, `--check` with its
pole count and the golden diff bound it. Untouched: the backend models, the
app's views, every existing part, footprint and datasheet entry.
