# plan-43-C: Reference-board parts — switches, USB-C receptacles, buffers, oscillators

Last updated: 2026-11-15
Effort: large (3h–1d)
Depends on: plan-43-B

C adds the 14 `letter` = `C` rows of `planning/plan-43-data/targets.tsv`: two
tactile switches (through plan-27's generator), one DIP switch, two GCT
USB-C receptacles, two single-gate buffers and seven clock oscillators, with
their footprints and `datasheets.json` entries. The behavioral outcome:
`listing_check` lists 14 more parts (32881 → 32895) and `ctest` passes 36
programs.

References:

- `docs/parts-format.md` § `switch` (and its paragraph on
  `planning/plan-27-data/gen-parts.py`), § `connector` (USB connectors: pin
  order, names, packages, footprint origin), § `buffer`, § `oscillator`,
  § Footprints.
- `planning/plan-27-data/gen-parts.py` (docstring: what a row needs, the
  refusals), `candidates.tsv`, `specs.tsv`, `maker-urls.tsv`,
  `datasheets/switch/tactile/datasheets.tsv`.
- `planning/plan-43-data/targets.tsv`, `facts.tsv`.
- plan-43-B § 4.3 (footprint reuse rule) and § 4.4 (datasheet entries).

## Prerequisites

plan-43-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-43-B complete | `ls planning/completed/plan-43-B-*.md` → exists | NOT MET (2026-11-15: not started) |
| The tactile generator is idle | `python3 -I planning/plan-27-data/gen-parts.py --dry-run` → `no changes` | MET (2026-11-15 at 834ef073: `no changes`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `wc -l < backend/tests/golden/parts.txt` → 32895; no golden line removed
  (`git diff <start of C> -- backend/tests/golden | grep -c '^-[^-]'` → 0).
- The 14 part names are in the library: `python3 -I -c "import json,glob,csv;
  n={p['name'] for f in glob.glob('schematical/schematical/Parts/*.json')
  for d in json.load(open(f)).get('devices',[]) for p in d['parts']};
  print(sum(r['mpn'] in n for r in
  csv.DictReader(open('planning/plan-43-data/targets.tsv'),delimiter='\t')
  if r['letter']=='C'))"` → 14.
- `python3 -I planning/plan-27-data/gen-parts.py --dry-run` → `no changes`
  after the run that adds the two tactile switches.

### Non-goals (explicit constraints)

- No slide switch, micro-USB connector, crystal or auto-direction
  translator (plan-43-A § 5).
- No change to an existing device's keys or timing, and no hand edit inside
  the `tact_` region of `switch.json`.
- No new drawer leaf: every part goes under a leaf that exists (`Switch →
  Tactile`, `Switch → DIP`, `Connector → USB → USB-C Port`, the buffers'
  leaves, `Oscillator → <band>`).

## 2. Current State

- **Tactile switches** are generated: `gen-parts.py` rebuilds `switch.json`
  from its first `tact_` device to the end from `candidates.tsv` rows with
  `decision` = `add`, one `specs.tsv` row per part (maker, series, mounting,
  `mpn_regex`, terminals, `pins` such as `A=1;B=2`, footprint id, note), the
  `MAKERS` table (DigiKey maker prefix → `"manufacturer"`), `maker-urls.tsv`
  (DigiKey URL → maker URL) and `datasheets/switch/tactile/datasheets.tsv`
  (the placed PDF per URL). It refuses a row whose rating has no `<amps>A @
  <volts>V` and an unknown footprint id.
- **DIP switches** are hand-written devices `te_<series>_<NN>` with
  `actuator`, `contacts` `[["1A","1B"], …]`, `ratedVolts`, pinout `p<2N>`;
  `te_gds_08`'s SMD part uses footprint `SW-SMD-16`.
- **USB-C receptacles** with 16 contacts on 12 pads are devices
  `usb-<maker>-<series>` with pins 1–12 `GND`, `VBUS`, `CC1`, `D+`, `D-`,
  `SBU1`, `CC2`, `D+`, `D-`, `SBU2`, `VBUS`, `GND` and 13 `SHIELD`
  (`usb-amp-10155435`); 41 devices have a `Mid mount` part.
- **Buffers**: devices `1G125` and `1G126` have pinout `p5` for SOT-23
  (footprint `SOT23-5`) and an `LVC` timing entry; `SN74LVC1G126DBVT` and
  `SN74LVC1G125DBVT` are there.
- **Oscillators**: one device per (family, frequency band), id
  `<family, lower case>-<band>` with band 0 = under 1 MHz, 1 = 1–10 MHz,
  2 = 10–100 MHz, 3 = 100 MHz and up, and `function` the band's label
  (`"10 MHz to 100 MHz"`), drawer `["Oscillator"]`, package `SMD`.

### Measured populations

| What | Count | Command |
|---|---|---|
| C rows | 14 (oscillator 7, switch 3, buffer 2, connector 2) | `awk -F'\t' '$1=="C"{print $2}' planning/plan-43-data/targets.tsv \| sort \| uniq -c` |
| Datasheet ids C uses / already present | 14 / 1 (`sn74lvc1g126`) | `targets.tsv` columns 12 and 11 |
| Oscillator frequencies | 12, 18.432, 20, 26, 27, 48, 50 MHz: all band 2 (10–100 MHz) | `grep -o 'Frequency=[^;]*' planning/plan-43-data/facts.tsv` |

### Verified properties

- **The generator is stable**: a dry run today changes nothing (the
  prerequisite row).
- **`CS1213AGF260`**: SPST-NO, 0.05 A @ 50 VDC, 3.9 × 2.9 mm, gull wing,
  without boss (`Switch-CS1213.pdf` p.1 ordering key: A, G, F260;
  `facts.tsv`). **`SWT0105-003516JSA`**: SPST-NO, 0.05 A @ 12 VDC, 3.7 ×
  3.7 mm, J lead (`swt0105-spec.pdf` p.1). Both ratings parse with the
  generator's pattern.
- **`219-8MST`**: 8 positions, SPST, gull wing, switching rating 100 mA @
  20 VDC, non-switching 50 VDC (`CTS-Switches-DIP-219-Series-Datasheet.pdf`
  p.1–2, `pdftotext -layout … | grep -n 'VDC'`).
- **`SN74LVC1G125DBVR` is UMW's part**: TI's part of that name shows 0 stock
  (`raw/SN74LVC1G125DBVR.json`, three makers under one number), and the
  library has no part of that name, so the name is free.
- **`ECS-TXO-32CSMV-260-AN-TR`** has a clipped-sine output; the library's
  other ECS TCXO devices (`ecs-txo-3225-2`) are modelled the same way, as a
  toggling output.

## 3. Design Overview

Four independent groups, smallest risk first: buffers (two lines beside
existing siblings), oscillators (new devices of a simple kind), switches
(one hand device, two generator rows), USB-C (two explicit footprints from
drawings — the largest risk: pad positions, slots for the shell legs, the
mid-mount cut-out).

## 4. Detailed design

### 4.1 Tactile switches (generator inputs)

- `planning/plan-27-data/gen-parts.py` `MAKERS`: add `("CIT Relay", "CIT
  Relay & Switch")` and `("GCT", "GCT")`.
- `candidates.tsv`: one row per part with the 22 columns of its header:
  maker (DigiKey's name), mpn, status `Active`, qty and the DigiKey columns
  from `facts.tsv` (`Circuit` → circuit, `Switch Function` → function,
  mounting `Surface Mount`, `Actuator Type` → actuator, `Actuator
  Orientation` → orientation, `Termination Style` → termination, `Outline`
  → outline, `Contact Rating @ Voltage` → rating, `Illumination`, ingress
  `-`, `Actuator Height off PCB, Vertical` → height), datasheet_url = the
  DigiKey URL of `raw/<mpn>.json`, decision `add`, pdf `1`, hit `1`, dead
  `0`.
- `maker-urls.tsv`: SWT0105's DigiKey URL → `https://gct.co/files/specs/swt0105-spec.pdf`.
- `datasheets/switch/tactile/datasheets.tsv`: the two PDFs' lines (A's
  `refile.py` put the files in `tactile/cit/` and `tactile/gct/`; path, the
  DigiKey URL the generator keys on, bytes, sha256).
- `specs.tsv`: one row per part — terminals and `pins` from the datasheet's
  circuit diagram, a new footprint from its PCB layout
  (`SW-TACT-CIT-CS1213-G`, `SW-TACT-GCT-SWT0105`), appended to
  `footprints.json` first so the generator finds the id.
- Run `python3 -I planning/plan-27-data/gen-parts.py --maker "CIT Relay"
  --maker GCT`.

### 4.2 DIP switch

Device `cts_219_08` after the last `te_` device and before the first
`tact_` device: function `219`, description `219 series SMD DIP switch, 8
positions`, drawer `["Switch", "DIP"]`, actuator `Raised slide`, contacts
`1A`/`1B` … `8A`/`8B`, `ratedVolts` 20, pinout `p16` numbered as the
datasheet's land pattern, a `note` with the rating's page. Part `219-8MST`,
manufacturer `CTS`, package `SMD`, footprint `SW-SMD-16` when the 219
gull-wing land pattern matches it by B § 4.3's rule, else `SW-SMD-16-CTS-219`.

### 4.3 USB-C receptacles

Devices `usb-gct-usb4105` and `usb-gct-usb4500` after the last USB-C
device: function `USB-C Port`, drawer `["Connector", "USB"]`, `"usb": "C"`,
`ratedVolts` / `ratedAmps` from each spec's electrical table, the 13-pin
pinout of § 2 (the pin-to-contact mapping in the device `note`), packages
`SMD right angle` (USB4105-GF-A) and `Mid mount` (USB4500-03-0-A),
manufacturer `GCT`. Footprints `USB-GCT-USB4105` and `USB-GCT-USB4500`,
style `explicit`, from each spec's recommended PCB layout by the rules of
`docs/parts-format.md` § `connector` (origin at the centre of the housing's
top-view outline, mating face up, one `SHIELD` pad per leg, the mid-mount
cut-out's size in the footprint `note`).

### 4.4 Buffers

`SN74LVC1G126DBVR` in device `1G126`: every key of `SN74LVC1G126DBVT` but
the name. `SN74LVC1G125DBVR` in device `1G125`: manufacturer `UMW`,
datasheet `2a03638ccb44ac09ba9843d7747386c9`, package `SOT-23`, pinout `p5`
after checking the UMW datasheet's pin order against it, family `LVC`,
footprint by B § 4.3 against `SOT23-5`.

### 4.5 Oscillators

One device per part (ids in `targets.tsv` column `device`): `family` the
series name (`ASEM1`, `ASE`, `ECS-TXO-32CSMV`, `SG-210STF`, `MC2016Z`,
`DSC1001`, `IQXO-791`), `minHz` / `maxHz` and `vcc` from the datasheet,
`outputs` the one output pin, a four-pin pinout named after the body
(`3225`, `2520`, `2016`, `5032`) with the datasheet's pin functions, the
`note` in the style of `ecs-txo-3225-2`. Parts: `defaultHz` the part's
frequency, `enablePin` the enable or standby pin's number where the part has
one (`facts.tsv` `Function=`), `"flags": ["automotive"]` on
`MC2016Z48.0000C19XSH`, `["ceramic"]` on `830208220401`. Manufacturers
`Abracon`, `ECS Inc.`, `Epson`, `Kyocera AVX`, `Microchip Technology`,
`Würth Elektronik`. Footprints by B § 4.3 against column
`footprint_candidates`, else `OSC-<MAKER>-<body>-4`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Buffers and oscillators

- [ ] Add the two buffer parts (§ 4.4) and the seven oscillator devices,
      parts and footprints (§ 4.5), with their `datasheets.json` entries.
- [ ] Regenerate the goldens; hand-check added `footprints.txt` lines.

Acceptance: 9 more lines in `parts.txt`; none removed.
  Check: `cmake --build /tmp/logic-p43 -j8 && ctest --test-dir /tmp/logic-p43 -R 'library_check|listing_check|trace_check|footprint_check|buffer_test'`
  → `100% tests passed` (est. 2 min).
Commit: —

### Phase 2 — Switches

- [ ] Add `cts_219_08` and its part (§ 4.2).
- [ ] Add the two tactile footprints, then the generator inputs, and run
      the generator (§ 4.1).
- [ ] Regenerate the goldens; hand-check added `footprints.txt` lines.

Acceptance: 3 more lines in `parts.txt`; the generator is idle again.
  Check: the Phase 1 command with `switch_test|tactile_test`;
  `python3 -I planning/plan-27-data/gen-parts.py --dry-run` → `no changes`
  (est. 2 min).
Commit: —

### Phase 3 — USB-C receptacles

- [ ] Add the two devices, parts, footprints and datasheet entries (§ 4.3).
- [ ] Regenerate the goldens; hand-check the two `footprints.txt` lines pad
      by pad against the specs' layouts.

Acceptance: `parts.txt` has 32895 lines.
  Check: the Phase 1 command with `connector_test`; `wc -l <
  backend/tests/golden/parts.txt` → 32895 (est. 2 min).
Commit: —

## Validation Plan

- Tests: loader and golden programs as above; no new test program.
- Coverage check: 14 new lines in `parts.txt` and `traces.txt`.
- Doc sync (in E): `docs/parts-format.md` § `connector` (USB footprints:
  add `USB-GCT-…` and GCT), § `switch` (the DIP paragraph gains the 219).
- Final gate: plan-43-E.

## Decisions

- The UMW part keeps DigiKey's part number `SN74LVC1G125DBVR` — author; TI's
  part of that name is out of stock and absent, so there is no clash.
- The tactile switches go through plan-27's generator, as
  `docs/parts-format.md` § `switch` requires — author.

## Corrections

## Summary

The USB-C land patterns carry the risk; they land last, each checked pad by
pad against the golden line.
