# plan-36-E: Microcontrollers — the Espressif module footprints

Last updated: 2026-11-12
Effort: medium (1h–2h)
Depends on: plan-36-D

E draws the land pattern of each in-scope Espressif module from its
datasheet's "Recommended PCB Land Pattern" figure. That is 13 module
devices at planning time. A module and its U variant (external antenna)
share a figure only if their datasheet draws one figure for both;
otherwise each gets its own. The objects go in
`planning/plan-36-data/footprints-E.json` (style `explicit`). G merges
them.

Outcome of E: every ESP32 `pinout-map.tsv` line names a footprint whose
pad designators equal the module pinout's designators (B, `module<N>`),
and `fp-load.sh footprints-C.json footprints-D.json footprints-E.json`
reports only `extra` / `unused` lines for exactly the new ids.

References:

- plan-36-A § Prerequisites; plan-36-C § 3 (ids, reuse rule).
- `docs/parts-format.md` § `explicit` — literal pads.
- The module PDFs in `datasheets/mcu/espressif/` (A). For example,
  `esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf` has its land patterns
  in § 11.1 "PCB Land Pattern" (Figures 13, 14: "ESP32-WROOM-32E / -32UE
  Recommended PCB Land Pattern"; `pdftotext` lines 266, 345–346).

## Prerequisites

plan-36-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-36-D complete | `ls planning/completed/plan-36-D-*.md` → exists | MET 2026-11-13 (3dadcee) |

## 1. Goal

- One footprint per distinct land-pattern figure of the in-scope modules
  (11 devices after A and E, see Corrections; 13 at planning time: ESP32-WROOM-32, -WROOM-DA, -WROVER,
  -MINI-1, S2-SOLO, S2-SOLO-2, S2-MINI-2, S3-WROOM-1, S3-MINI-1,
  C3-WROOM-02, C3-MINI-1, C6-WROOM-1, C6-MINI-1). The column `footprint`
  of each ESP32 row of `package-codes.tsv` is filled (code = the module
  device; geometry `module`).
- Loads clean with C's and D's objects.

### Non-goals (explicit constraints)

- No keep-out or antenna-clearance zones: the footprint format has pads
  and a body only. The body outline is the module outline, and the `note`
  repeats the datasheet's antenna keep-out size.
- No change to `Parts/`, goldens or the format.

## 2. Current State

- No module footprint exists: `grep -c '"id": "ESP' schematical/schematical/Parts/footprints.json` → 0.
- `explicit` footprints are literal pad lists, each with a designator,
  centre, size and shape (`docs/parts-format.md` § `explicit`).
  plan-11-C's `LCM-*` and plan-35's module footprints are the precedent.

## 3. Design

- Pads as the figure gives them: castellated edge pads at the printed
  pitch and size, numbered as the pin layout figure numbers them. The
  exposed GND pad and any GND grid get the designators B gave them in the
  module pinout. Where the datasheet draws a grid of small GND pads that
  are all one pin (e.g. the MINI modules' GND grid), each small pad gets
  its own designator, the next numbers after the edge pads, and B's
  pinout has one `GND` ground pin per small pad.
- Body = the module outline (W × L, mid-values). The origin is the body
  centre. The antenna end is up: the figure's orientation, pin 1 at the
  top of the left side as the format requires. Where the figure puts
  pin 1 elsewhere, the pads are rotated, and the `note` says so.
- Ids: `ESP32-<module>`, e.g. `ESP32-WROOM-32E`, `ESP32-S3-WROOM-1`,
  `ESP32-C3-MINI-1`. A figure that covers a module and its U variant gets
  one id. `package` value (G): `Module`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Module footprints

- [x] For each module datasheet: find the land-pattern figure's page and
      write the object into `footprints-E.json` (§ 3). Fill
      `package-codes.tsv`.
      `census/fp-specs-E.py` (PyMuPDF): 21 footprints from 21 figures, the 11 modules and 10 U variants. Pads come from each figure's vector outlines. Pin numbers come from the module's "Pin Layout (Top View)" figure: ring pads are matched in order around the centre, and the thermal grid takes its one label. Runs are fitted to their pitch (no pad moves > 0.03 mm). Every footprint was compared with its printed dimensions (pad sizes, the 1.27 / 0.8 / 0.85 / 1.5 pitches, the 17.5 / 11.8 column spans); C3-MINI-1 and WROOM-DA (since dropped) were also drawn and compared with their figures. All 11 `module` rows of `package-codes.tsv` are filled (package `Module`). The 23 U-variant MPNs are in `footprint-mpn.tsv`.
- [x] `census/fp-pads-vs-pins.py`: for each ESP32 pinout, the footprint's
      designators equal the pinout's designators.
      → `21 (pinout, footprint) pairs, 0 mismatches` (designator sets equal for ESP32); `--all-rows` over C+D+E → `631 (pinout, footprint) pairs, 0 mismatches`.
- [x] Load: `fp-load.sh footprints-C.json footprints-D.json footprints-E.json`.
      → `63 footprints appended` / `63 new footprints: load ok` (13 + 29 + 21).

Acceptance: every module has a footprint whose pads match its pins, and the footprints load.
  Check: `python3 -I planning/plan-36-data/census/fp-pads-vs-pins.py planning/plan-36-data/footprints-E.json planning/plan-36-data/pinouts.tsv` → `0 mismatches`; the load → only `extra` / `unused` lines (est. 3 min).
  Result 2026-11-13: `21 (pinout, footprint) pairs, 0 mismatches`; `63 new footprints: load ok`. Coverage: `awk -F'\t' '$2=="module" && $6==""' package-codes.tsv | wc -l` → 0.
Commit: 88fa398

## Validation Plan

- Tests: the loader via `fp-load.sh`; `footprint_check` in G.
- Coverage check: `awk -F'\t' '$2=="module" && $6==""' package-codes.tsv | wc -l` → 0.
- Runtime proof: plan-36-I places an ESP32-WROOM-32E-N4 on the PCB page.
- Doc sync: none.
- Final gate: plan-36-I.

## Decisions

- ~~**One designator per small GND pad**~~ (superseded, see Corrections: the figures number a grid as one pin) — author: the loader requires a pad
  for every pin and matches by designator, and a footprint may not have
  more pads than pins except for pull parts (`docs/parts-format.md`
  § The `"footprint"` part key). So every copper pad is a pin, and they
  are all `GND`, which the format allows (a name used by several pins).

## Corrections

- **2026-11-13: 11 modules, not 13.** ESP32-S2-SOLO was out in A (its datasheet marks -N4 "End of life"). ESP32-WROOM-DA is out here: its cover is watermarked "END OF LIFE (EOL)" and 30 of its 31 pages carry the banner, which A's ordering-table scan did not read. `dead-verdicts.tsv` gained the line, so A's rule 4 makes -N8 and -N4 `out`. The PDF is deleted, `datasheets/obsolete-not-included.md` has its line and `skipped-parts.md` loses -N4. The add rows are 1424 and the devices 303 (`devices.py` / `facts-check.py` → `303 devices, 0 mismatches`). The other Espressif PDFs carry EOL / NRND marks on 1–6 pages only, on other codes, which A read.
- **2026-11-13: U variants have their own figures.** Each of the 10 two-variant datasheets draws the U (external antenna, shorter body: 18 x 19.2, 13.2 x 12.5, 15.4 x 15.4, 18 x 14.3, 13.2 x 13.5) as a separate figure. So there are 21 footprints, and the U MPNs are mapped in a new `footprint-mpn.tsv` (G reads it first, plan-36-G § 3.3). WROVER-E / -IE share one figure and one footprint.
- **2026-11-13: a thermal grid is one pin.** § 3 had each small GND pad get its own designator. But every Pin Layout figure numbers the grid as one pin (C3-MINI-1 "Pin 49", WROOM-32E "39", C6-WROOM-1 "29"), and B's pinouts follow the tables (C3-MINI-1: 53 pins). So the grid's nine pads are all that pin; the format allows several pads per pin (`docs/parts-format.md` § `explicit`). The Decision below is superseded.
- **2026-11-13: ESP32-MINI-1 is 13.2 x 19.0 mm** (ordering table, and the drawn outline), not the 16.6 assumed.
- **2026-11-13: ESP32-MINI-1U's figure leaves one thermal pad undrawn**, where dimension lines cross (8 of 9). MINI-1's figure and the shared pin layout show 9, so the 3 x 3 grid is completed; the footprint `note` says so.
- **2026-11-13: drawing quirks handled.** Pads can be drawn as outline + fill strips (merged), as four loose blue lines (C3-WROOM-02's thermal pads) or as a chamfered polygon (C3-MINI-1's pin-1 thermal pad, drawn as a plain rect). Below the antenna area the WROOM outlines are not drawn, so their length is checked against the L dimension's extension line.

## Summary

E is 13 literal footprints from clear figures. The risk is pad
numbering, and `fp-pads-vs-pins.py` checks it against B's pinouts.
