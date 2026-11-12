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
| plan-36-D complete | `ls planning/completed/plan-36-D-*.md` → exists | NOT MET (2026-11-12: plan written) |

## 1. Goal

- One footprint per distinct land-pattern figure of the in-scope modules
  (13 devices at planning time: ESP32-WROOM-32, -WROOM-DA, -WROVER,
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

- [ ] For each module datasheet: find the land-pattern figure's page and
      write the object into `footprints-E.json` (§ 3). Fill
      `package-codes.tsv`.
- [ ] `census/fp-pads-vs-pins.py`: for each ESP32 pinout, the footprint's
      designators equal the pinout's designators.
- [ ] Load: `fp-load.sh footprints-C.json footprints-D.json footprints-E.json`.

Acceptance: every module has a footprint whose pads match its pins, and the footprints load.
  Check: `python3 -I planning/plan-36-data/census/fp-pads-vs-pins.py planning/plan-36-data/footprints-E.json planning/plan-36-data/pinouts.tsv` → `0 mismatches`; the load → only `extra` / `unused` lines (est. 3 min).
Commit: —

## Validation Plan

- Tests: the loader via `fp-load.sh`; `footprint_check` in G.
- Coverage check: `awk -F'\t' '$2=="module" && $6==""' package-codes.tsv | wc -l` → 0.
- Runtime proof: plan-36-I places an ESP32-WROOM-32E-N4 on the PCB page.
- Doc sync: none.
- Final gate: plan-36-I.

## Decisions

- **One designator per small GND pad** — author: the loader requires a pad
  for every pin and matches by designator, and a footprint may not have
  more pads than pins except for pull parts (`docs/parts-format.md`
  § The `"footprint"` part key). So every copper pad is a pin, and they
  are all `GND`, which the format allows (a name used by several pins).

## Corrections

## Summary

E is 13 literal footprints from clear figures. The risk is pad
numbering, and `fp-pads-vs-pins.py` checks it against B's pinouts.
