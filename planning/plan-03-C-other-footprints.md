# plan-03-C: SRAM, oscillator, switch and 7-segment footprints; key required

Last updated: 2026-10-31
Effort: large (3h–1d)
Depends on: plan-03-B

Adds the last 68 footprints — SRAM 20, oscillator 6, DIP switch 15,
7-segment display 27 — and tags the remaining 350 parts. Then the
`"footprint"` key becomes required on every part except supplies (VCC, GND),
which reject it.

Outcome: all 3,815 non-supply parts have a footprint; `footprints.txt` has
126 lines; the loader rejects a non-supply part with no footprint and a
supply part with one; `AGENTS.md` and `docs/parts-format.md` say so.

References:

- plan-03-A §4 (format; `explicit` style), plan-03-B §3 (pad rules R1–R6).
- Datasheets: `datasheets/sram/` (20 PDFs), `datasheets/osc/lmk6c.pdf`,
  `datasheets/osc/SiT1534-datasheet.pdf`,
  `datasheets/switch/ENG_CS_1308111-1_SWITCHES_CORE_PROGRAM_CATALOG_0308.pdf`,
  `datasheets/led-7/` (27 PDFs).

## Prerequisites

See plan-03-A §Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-03-B complete | `ls planning/completed/plan-03-B-logic-footprints.md` → exists | MET (2026-11-01: exists, 6ac023a) |
| Logic parts tagged | `grep -c '"footprint"' schematical/schematical/Parts/{gate,dff,jkff}.json` → 2552, 672, 186 | MET (2026-11-01: 2552, 672, 186) |
| Switch drawings in the repo (added 2026-11-01, see Corrections) | `pdfinfo datasheets/switch/ENG_DS_8-1773450-9_SECTION_A_DIP_SWITCHES.pdf \| grep Pages` → 44 | MET (2026-11-01: `Pages: 44`; added in this plan) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all prerequisites.

## 1. Goal

- `grep -c '"footprint"'` → sram 172, oscillator 93, switch 36, 7seg 49.
- `wc -l < backend/tests/golden/footprints.txt` → 126.
- `library_check` has cases "non-supply part without footprint" and "supply
  part with footprint", both rejected.
- `ctest` passes; `traces.txt` no diff.

### Non-goals

- No pinout or behavior change; no new parts; no drawer change.

## 2. Current State

### Measured populations (`python3 -I` over the part files, 2026-10-31)

| Kind | Parts | Footprints | Grouping |
|---|---|---|---|
| sram | 172 | 20 | §4.1 |
| oscillator | 93 | 6 | `(package, pins)`: 2520 VSON 4 → 28, 2520 VSON 6 → 21, 3225 VSON 4 → 19, 3225 VSON 6 → 23, 2012 → 1, 1508 CSP → 1 |
| switch | 36 | 15 | through-hole (`DIP`) 4/8/12/16/20 pins → 4/5/4/5/4; SMD 2.54 mm (`SMD`, series GDS/GDR/ADE) 4/8/12/16/20 → 1/3/1/3/1; GDH 4/8/16 → 1/1/1; GDHL 8/16 → 1/1 |
| 7seg | 49 | 27 | one per datasheet named in the device `note` (27 distinct); `LTD-6000.pdf` covers 23 parts, every other datasheet 1 |

### Verified properties

- SRAM package facts from the datasheets' text (`pdftotext`):
  AS6C6264/AS6C62256 "28 pin 600 mil P-DIP", "28 pin 330 mil SOP", "28 pin
  8mm x 13.4mm sTSOP"; AS6C1008/AS6C4008 "32 pin 600 mil P-DIP", "32 pin
  450 mil SOP", "8mm x 20mm TSOP-I", "8mm x 13.4mm sTSOP"; AS6C4008 "32-pin
  400 mil TSOP-II"; AS7C256A "28-pin SOJ (300 mil)", "TSOP 1 (8×13.4 mm)";
  AS7C31024B "32-pin SOJ (300 mil)" (names `…TJC…`) and "(400 mil)" (names
  `…JC…`); AS7C4096A "36-pin SOJ (400 mil)"; AS7C31026B/AS7C34098A/AS7C4098A
  "44-pin SOJ (400 mil)"; 44-pin TSOP-II "400 mil" (AS6C8008, AS6C1608B,
  AS6C2016, AS6C4016, AS6C8016, AS7C38098A); AS6C1616/AS7C316098A "48-pin
  12mm x 20mm TSOP-I"; 48-ball 6 × 8 mm TFBGA (AS6C8008, AS6C1608B,
  AS6C2016, AS6C4016, AS6C8016, AS7C316098A, AS7C38098A); AS6C6416 "48-ball
  8mm x 10mm TFBGA"; AS7C351232 "90-ball 8mm x 13mm TFBGA"; AS7C34098A
  "48-pin Mini BGA" (size in its drawing only).
- After plan-02-C every SRAM BGA part has a ball pinout whose designators
  are the datasheet ball names (plan-02-C §4): the 48-ball parts use the full
  8 × 6 grid A1…H6; AS6C1008 and AS6C4008 share one 36-ball map (8 × 6, C2
  C5 D2–D5 E2–E5 F2 F5 empty); AS7C351232 uses a 15 × 9 grid (rows A–R
  without I/O/Q, columns 4–6 empty). A grid footprint's `balls` is exactly
  that set, and the loader checks it against each part's designators.
- Oscillators: `lmk6c.pdf` has TI board layouts DLE0004A, DLE0006B, DLF0004A,
  DLF0006B (`grep -A3 'EXAMPLE BOARD LAYOUT'`); DLE = 3.2 × 2.5 mm
  (`3225 VSON`), DLF = 2.5 × 2.0 mm (`2520 VSON`) (`lmk6c.pdf` text lines
  269–270). `SiT1534-datasheet.pdf` has a "Recommended Land Pattern (Unit: mm)"
  for each package (text lines 1170, 1230); the 2012 pattern ("SiTime Only
  SPL") has two large end pads with VDD and GND between them.
- Switch catalog has GDH and GDHL sections, both "Half pitch" (text lines
  236–246).
- 7-seg datasheets have no text land patterns (0 of 27 match a layout
  keyword); their dimensions are in drawings, read by rendering the page.

## 3. Design Overview

Same procedure as plan-03-B: write footprints from drawings, tag parts by a
line-local script (`/tmp/p03c-assign.py`), regenerate goldens, hand-check
each new golden line. Then a small loader change makes the key required.

Pad rules: plan-03-B §3 R1–R6. 7-seg displays and switches are through-hole
(R5) with the row spacing from their drawing. Switch footprints share pads
across series; the body outline is the GDS (through-hole and SMD), GDH or
GDHL series body — the author's choice, since pads are identical across
series and only the plastic differs.

Correctness risk: wrong numbers (hand check), and the required-key switch
breaking any loader path that builds parts without footprints —
`library_check` fixtures build small part files and must name a footprint
from now on.

## 4. Detailed Design

### 4.1 SRAM footprints (20)

| id | Parts | Assignment (device / package) | Rule |
|---|---|---|---|
| `PDIP-28-600` | 3 | AS6C6264, AS6C62256 `DIP` | R5, span 15.24 |
| `PDIP-32-600` | 3 | AS6C1008, AS6C4008 `DIP` | R5, span 15.24 |
| `SOP-28-330` | 4 | AS6C6264, AS6C62256 `SOP` | R2 |
| `SOP-32-450` | 2 | AS6C1008, AS6C4008 `SOP` | R2 |
| `SOJ-28-300` | 16 | AS7C256A `SOJ` | R3 |
| `SOJ-32-300` | 8 | AS7C31024B `SOJ`, name contains `TJ` | R3 |
| `SOJ-32-400` | 8 | AS7C31024B `SOJ`, other names (`…JC…`) | R3 |
| `SOJ-36-400` | 8 | AS7C4096A `SOJ` | R3 |
| `SOJ-44-400` | 24 | AS7C31026B, AS7C34098A, AS7C4098A `SOJ` | R3 |
| `TSOP1-32-8X20` | 10 | `TSOP-I`, 32 pins | R2 |
| `TSOP1-48-12X20` | 2 | `TSOP-I`, 48 pins | R2 |
| `STSOP1-28-8X13.4` | 20 | `sTSOP-I`, 28 pins | R2 |
| `STSOP1-32-8X13.4` | 10 | `sTSOP-I`, 32 pins | R2 |
| `TSOP2-32-400` | 1 | AS6C4008 `TSOP-II` | R2 |
| `TSOP2-44-400` | 38 | `TSOP-II`, 44 pins | R2 |
| `TFBGA-36-6X8` | 2 | AS6C1008, AS6C4008 `TFBGA` | R6, 8 rows × 6 columns, the 36 balls of plan-02-C's map |
| `TFBGA-48-6X8` | 8 | AS6C8008, AS6C1608B (2), AS6C2016, AS6C4016, AS6C8016, AS7C316098A, AS7C38098A `TFBGA` | R6, 8 × 6, all 48 balls |
| `TFBGA-48-8X10` | 2 | AS6C6416 `TFBGA` | R6 |
| `MINIBGA-48` | 1 | AS7C34098A_BGA `TFBGA` | R6, 8 × 6 all 48 balls; pitch, ball and body from its own drawing |
| `TFBGA-90` | 2 | AS7C351232 `TFBGA` | R6, 15 × 9, the 90 balls of plan-02-C's map |

Sum 172. TSOP-I leads leave the short sides; it is still `dual` (the body is
wider than tall: `body.width` 20 or 13.4, `body.height` 8 or 12).

### 4.2 Oscillator footprints (6)

| id | Parts | Source |
|---|---|---|
| `OSC-DLF-4` | 28 (`2520 VSON`, 4 pins) | DLF0004A board layout, R1 |
| `OSC-DLF-6` | 21 (`2520 VSON`, 6 pins) | DLF0006B, R1 |
| `OSC-DLE-4` | 19 (`3225 VSON`, 4 pins) | DLE0004A, R1 |
| `OSC-DLE-6` | 23 (`3225 VSON`, 6 pins) | DLE0006B, R1 |
| `OSC-SIT-2012` | 1 (`2012`) | SiT1534 "SiTime Only SPL", `explicit` |
| `OSC-SIT-1508` | 1 (`1508 CSP`) | SiT1534 1508 CSP land pattern, `explicit` |

### 4.3 Switch footprints (15)

| id | Parts | Assignment | Body |
|---|---|---|---|
| `SW-DIP-4` / `-8` / `-12` / `-16` / `-20` | 4 / 5 / 4 / 5 / 4 | package `DIP` | GDS through-hole |
| `SW-SMD-4` / `-8` / `-12` / `-16` / `-20` | 1 / 3 / 1 / 3 / 1 | package `SMD`, series gds/gdr/ade | GDS SMD |
| `SW-GDH-4` / `-8` / `-16` | 1 / 1 / 1 | device id `te_gdh_*` | GDH |
| `SW-GDHL-8` / `-16` | 1 / 1 | device id `te_gdhl_*` | GDHL |

Through-hole: R5, pitch 2.54, span from the catalog drawing. SMD (including
GDH/GDHL): R2 from the catalog drawing's gull-wing dimensions — the
catalog's text layer has no land pattern (`pdftotext … | grep -i 'land
pattern\|board layout\|solder pad'` → no match), so R2 is used throughout.
Half pitch = 1.27. The `note` records the catalog page.

### 4.4 7-segment footprints (27)

One per datasheet; id = `7SEG-` + the datasheet file stem in upper case
(`C4727JR.pdf` → `7SEG-C4727JR`, `LTD-6000.pdf` → `7SEG-LTD-6000`). Each part
gets the footprint of the datasheet its device `note` names. R5, pitch and
row spacing from the drawing, body = the display's outline. Pinouts with
`absent` pins (C4727JR, LTC-2623G, LTC-4624JR, LTC-4627JR, LTC-4724JR,
LTS-312AHR) keep a slot for those positions (the app does not draw a pad for
an absent pin); the 15-pin ones use `perSide` 8 with `slots` matching the
drawing's empty position.

### 4.5 Required key

`library.c:load_part`: after the kind's `load_part`, a part with
`kind != PART_SUPPLY` and no `"footprint"` is rejected (`missing
"footprint"`); a supply part with one is rejected (`a supply has no
footprint`). `library_check` fixtures that build non-supply parts add a
footprint (they load a one-footprint `footprints` text first).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit;
> `- [~]` for partial; moot as `- [x] ~~text~~ — moot: <evidence>`; fill
> `Commit:` when a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — SRAM and oscillators (26 footprints, 265 parts)

- [ ] `footprints.json`: the 20 SRAM (§4.1) and 6 oscillator (§4.2) footprints.
- [ ] `/tmp/p03c-assign.py` for `sram.json` and `oscillator.json` (0 unmatched;
      172 and 93 tagged).
- [ ] Regenerate goldens; hand-check each new `footprints.txt` line.

Acceptance: SRAM and oscillator parts carry footprints; behavior unchanged.
  Check: `ctest --test-dir /tmp/logic-plan03 --output-on-failure` → all pass (est. 1 min).
  Check: `git diff --exit-code backend/tests/golden/traces.txt` (est. seconds).
Commit: —

### Phase 2 — Switches and 7-segment displays (42 footprints, 85 parts)

- [ ] `footprints.json`: the 15 switch (§4.3) and 27 7-seg (§4.4) footprints.
- [ ] Tag `switch.json` (36) and `7seg.json` (49).
- [ ] Regenerate goldens; hand-check.

Acceptance: every non-supply part has a footprint.
  Check: `ctest --test-dir /tmp/logic-plan03 --output-on-failure` → all pass; `wc -l < backend/tests/golden/footprints.txt` → 126 (est. 1 min).
  Check: `python3 -I` one-off: every part in every kind file except `supply.json` has `"footprint"` → 3815 (est. seconds).
Commit: —

### Phase 3 — Required key, docs

- [ ] `library.c`: §4.5 rules; `library_check`: the two new cases, fixtures
      updated.
- [ ] `docs/parts-format.md`: `footprint` is required (except supplies);
      the footprint list is in `footprints.json`.
- [ ] `AGENTS.md` § Datasheets: a new part also names a footprint from
      `footprints.json`, added from the datasheet's package drawing when no
      existing footprint matches (rules in `docs/parts-format.md`).

Acceptance: the loader enforces the key; docs and agent rules say so.
  Check: `ctest --test-dir /tmp/logic-plan03 -R library_check --output-on-failure` → passes, including the two cases (est. 1 min).
  Check: `grep -n 'footprint' AGENTS.md docs/parts-format.md | wc -l` → ≥ 3 (est. seconds).
Commit: —

## Validation Plan

- Tests: `footprint_check` (126-line golden), `listing_check`, `trace_check`,
  `library_check` (required-key cases).
- Coverage check: the "every non-supply part has a footprint" count (3815) and
  `footprint_check`'s unused-footprint failure.
- Runtime proof: `xcodebuild` (plan-03-A command) succeeds and the drawer
  footer still reads "3,815 parts from backend".
- Doc sync: `docs/parts-format.md`, `AGENTS.md`.
- Final gate (once): `cmake -S backend -B /tmp/logic-final && cmake --build /tmp/logic-final -j8 && ctest --test-dir /tmp/logic-final` (est. 1 min) + `xcodebuild` (est. 3 min).

## Decisions

See plan-03-A §Decisions. Author's choices recorded here: one footprint per
7-seg datasheet (no merging of look-alike drawings); switch bodies come from the GDS/GDH/GDHL series.

## Corrections

- 2026-11-01: **missing prerequisite — switch drawings.** The catalogue in
  the repo (`ENG_CS_1308111-1_SWITCHES_CORE_PROGRAM_CATALOG_0308.pdf`) is a
  short-form catalogue: all 30 pages rendered, no dimension drawings at all
  (only "Dimensions (WxH)" such as GDS 6.2 × 2.7). TE's own document links
  refuse automated download; TE's full DIP-switch catalogue 8-1773450-9
  ("Section A DIP Switches", 44 pages, GDS A4–A5, GDH A10–A11, GDHL
  A12–A13) was fetched from RS Components
  (docs.rs-online.com/6995/A700000015638293.pdf) and added as
  `datasheets/switch/ENG_DS_8-1773450-9_SECTION_A_DIP_SWITCHES.pdf`. Its
  parts are the library's existing, active GDS/GDH/GDHL parts.
- 2026-11-01: §4.3 says the switch catalogue has no land pattern, so SMD uses
  R2. The 8-1773450-9 drawings do give recommended pad layouts for GDS SMD
  (p.5 "S"), GDH (p.11) and GDHL (p.13, "RECOMMENDED PAD LAYOUT"), so all
  ten SMD switch footprints use **R1**. The table's last column ("04 4
  10.02mm 7.62mm") is "Dim M" (first-to-last pin along the row), not the
  row spacing; GDS row spacing is .300 in = 7.62 mm (printed 7.60), GDH
  .299 in (7.60).
- 2026-11-01: §4.1 says TSOP-I `body.width` is 20 or 13.4; those are lead
  tip-to-tip sizes. The plastic bodies are 18.4 (8×20 and 12×20) and 11.8
  (8×13.4), from the drawings' D1.
- 2026-11-01: §2 cites SiT1534 text lines 1170/1230 for the land patterns;
  in a `pdftotext -layout` dump they are at lines 480 and 535 (PDF p.9 and
  p.10).

## Summary

Data entry for the non-TI parts plus a two-line loader rule. Risk is in
reading drawings (hand check per line) and in fixtures that relied on the
optional key.
