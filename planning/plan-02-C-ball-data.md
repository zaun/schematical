# plan-02-C: Real ball names for the BGA parts

Last updated: 2026-10-31
Effort: medium (1h–2h)
Depends on: plan-02-B

Gives every BGA part in the library its datasheet ball names:

- the 12 SRAM TFBGA pinouts (15 parts), converted in place;
- the 25 TI DSBGA devices (43 parts), which get a new `"dsbga"` ball
  pinout instead of sharing the SOT-23/SON numbers.

The SiT1534 1508 CSP stays numbered, because its datasheet numbers the pads
1–4.

Outcome: exactly the 58 BGA lines of `backend/tests/golden/parts.txt` change,
only in designators (plus `NC` → `DNU` on 4 YFP parts). `traces.txt` is
unchanged. The app labels those parts' pins with ball names and saves wires
to them as `"pin": "A1"`.

References:

- `docs/parts-format.md` "Pinouts and pins" (ball pinouts, as written in plan-02-A Phase 3).
- `docs/backend.md` "Tests" (golden regeneration).
- `AGENTS.md` "Datasheets" (run the backend tests; regenerate the golden lines of parts whose facts change on purpose).
- Datasheets: `datasheets/sram/*.pdf`, `datasheets/gate-*/`, `datasheets/ff-d/`
  (sources per device in §4).

## Prerequisites

See plan-02-A for the plan-wide prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-02-B complete | `ls planning/completed/plan-02-B-* && ! ls planning/plan-02-B-* 2>/dev/null` → the completed path only | NOT MET (2026-10-31) |

## 1. Goal

- Every part whose package is `TFBGA` or `DSBGA` has a ball pinout. Its
  designators are the datasheet ball names, and its pins keep the positions
  they had.

### Non-goals (explicit constraints)

- No behavior change: names, roles, groups and **positions** of every pin
  stay the same, so `traces.txt` is byte-identical. The one exception is the
  name of the YFP DNU ball (§4.2). It is role `nc`, so it does not change
  traces.
- No part added or removed, and no non-BGA part touched. The DIP/SOT/SON
  parts of the 25 DSBGA devices keep their numbered pinouts.
- `OSC-SIT1534-1508` (`1508 CSP`) stays numbered.
- Pinouts are edited as text, line by line, never re-serialized with a JSON
  library. The files keep their one-pin-per-line layout.

## 2. Current State

- SRAM BGA pinouts are numbered 1..N **row-major over the populated balls**.
  For each of the 12 devices, a reading of every ball drawing confirmed that
  pin n is the n-th populated ball in row-major order, with every name
  matching. Pinout names are `bga` (10 devices), `p48` (AS6C6416) and `p90`
  (AS7C351232).
- DSBGA parts list `"pinout": "p5"`, `"p6"` or `"p8"`, the device's numbered
  SOT-23/SON pinout. Their ball maps were read from the TI datasheets (§4.2).
  The SN74LVC1G80 source is `datasheets/ff-d/sces221t.pdf` (SCES221T, Figure
  4-1, YZP bottom view).
- Pins are one per line, e.g. `          {"number":1,"name":"A","role":"input"},`
  (`sed -n 3194,3200p schematical/schematical/Parts/gate.json`).

### Measured populations

| What | Count | Command |
|---|---|---|
| TFBGA parts / pinouts | 15 / 12 | `grep -o '"package":"TFBGA"' schematical/schematical/Parts/sram.json \| wc -l` → 15; `python3 -c "import json;print(sum(len({p['pinout'] for p in d['parts'] if p['package']=='TFBGA'}) for d in json.load(open('schematical/schematical/Parts/sram.json'))['devices']))"` → 12 pinouts |
| DSBGA parts / devices | 43 / 25 | `grep -o '"package":"DSBGA"' schematical/schematical/Parts/{gate,dff}.json \| wc -l` → 43 (gate 35, dff 8); 25 devices (gate 19, dff 6) |
| YFP (6-ball, DNU) parts | 4 | SN74AUP1G00YFPR, SN74AUP1G02YFPR, SN74AUP1G08YFPR, SN74AUP1G32YFPR (one per `_SON` device) |
| Golden lines expected to change | 58 | 15 + 43 |

### Verified properties

- **Every ball name passes plan-02-A's ball rule.** Rows used:
  - the 36- and 48-ball grids use `A`–`H`;
  - AS7C351232 uses `A`–`H, J, K, L, M, N, P, R`, skipping I/O/Q;
  - DSBGA uses `A`–`D`.

  Columns are 1–9, and no name has two row letters.
- **Every ball's pin name exists in the device's numbered pinout**, so all
  kind keys bind. The exception is the YFP ball B2 (datasheet "DNU", ours
  `NC`), which no kind key names.
- **Ball count equals pin count** for every device. The 5-ball YZP grid has
  no ball at B2, and the pinout simply leaves it out.

## 3. Design

- **Position order is preserved.** Each ball pinout lists its balls in the
  order of the numbered pins they replace: ball for pin 1 first, then pin 2,
  and so on. This keeps engine indices (so traces are byte-identical) and
  the version 1 file meaning (plan-02-B: pin n = n-th pin). For SRAM this
  order *is* row-major. For DSBGA it is the SOT/SON order, so the block
  symbol keeps its exact layout and only the labels change.
- **SRAM:** convert the existing pinout in place by replacing `"number":n`
  with `"ball":"<ball(n)>"`. Rename `p48` (AS6C6416) and `p90` (AS7C351232)
  to `bga`, in the pinout key and in their parts' `"pinout"`, to match the
  other ten. Pinout names never reach the listing (`listing.c` does not print
  them).
- **DSBGA:** add a `"dsbga"` pinout to the device, a copy of the numbered
  pinout's pin lines with `"number":n` replaced by `"ball":"<ball(n)>"`.
  Move only the `"package":"DSBGA"` parts to `"pinout":"dsbga"`.
- **Notes:** each SRAM device note's BGA segment names the ball grid. Each
  DSBGA device note gains `DSBGA balls: <datasheet> p.<n>`.
- The text edits are made with a throwaway script in `/tmp` (not
  committed), which works on the files' lines.

**Gate class:** facts change on purpose for exactly 58 parts. The gate is:

- the regenerated `parts.txt` diff is exactly those 58 lines;
- `traces.txt` is byte-identical.

A trace diff means a position moved. Find it by diffing the old and new
listing line of **one** part and fix the order. Never regenerate traces.

**Rejected alternatives**

- *List balls row-major (A1, A2, B1…).* It would reorder engine indices for
  all 25 DSBGA devices, because every L5/L6/L8 layout puts its last pin at A2.
  That breaks byte-identical traces and the version 1 meaning of saved
  files. Row-major stays the doc's guidance for *new* devices.
- *Rename the DNU ball's role to something new.* `nc` already means "nothing
  to wire". Only the name follows the datasheet.

## 4. The ball maps

`ball(n)` is the ball for numbered pin n, in pin order 1, 2, … N.

### 4.1 SRAM (`sram.json`)

| Devices | Grid | `ball(n)` for n = 1…N |
|---|---|---|
| AS6C1008, AS6C4008 (36 balls) | 8 × 6, C2 C5 D2–D5 E2–E5 F2 F5 unpopulated | A1 A2 A3 A4 A5 A6 B1 B2 B3 B4 B5 B6 C1 C3 C4 C6 D1 D6 E1 E6 F1 F3 F4 F6 G1 G2 G3 G4 G5 G6 H1 H2 H3 H4 H5 H6 |
| AS6C8008, AS6C1608B, AS6C2016, AS6C4016, AS6C8016, AS6C6416, AS7C34098A_BGA, AS7C38098A, AS7C316098A (48 balls) | 8 × 6, full | `"ABCDEFGH"[(n-1)/6]` + `((n-1)%6)+1` |
| AS7C351232 (90 balls) | 15 × 9, columns 4–6 unpopulated | `"ABCDEFGHJKLMNPR"[(n-1)/6]` + `[1,2,3,7,8,9][(n-1)%6]` |

Spot checks the implementer verifies after the edit, by
`listing_check --print | grep '^<part>|'`:

- AS6C1008-55BIN pin 14 is `C3:NC:nc`;
- AS6C6416-55BIN pin 1 is `A1:LB#:input`;
- AS7C351232-10BIN pin 49 is `J1:CE2:input`, and pin 90 is `R9:DQ17:inout`.

### 4.2 DSBGA (`gate.json`, `dff.json`)

| Layout | `ball(n)`, n = 1…N | Devices |
|---|---|---|
| L5 (YZP, 5 balls) | A1 B1 C1 C2 A2 | gate 1G00, 1G02, 1G08, 1G32, 1G86; dff 1G79, 1G80 |
| L6Y (YFP, 6 balls) | A1 B1 C1 C2 B2 A2, and pin 5 is renamed `"DNU"` (role `nc`) | gate 1G00_SON, 1G02_SON, 1G08_SON, 1G32_SON |
| L6 (YZP, 6 balls) | A1 B1 C1 C2 B2 A2 | gate 1G10, 1G11, 1G27, 1G332, 1G386; dff 1G175, 1G374 |
| L8 (YZP/YFP, 8 balls) | A1 B1 C1 D1 D2 C2 B2 A2 | gate 2G00, 2G02, 2G08, 2G32, 2G86; dff 1G74, 2G74 |

Resulting maps, which the spot checks confirm:

- 1G00: A1=A, B1=B, C1=GND, C2=Y, A2=VCC.
- 1G80: A1=D, B1=CLK, C1=GND, C2=QBAR, A2=VCC.
- 1G10: A1=A, B1=GND, C1=B, C2=Y, B2=VCC, A2=C.
- 1G374: A1=CLK, B1=GND, C1=D, C2=Q, B2=VCC, A2=OE.
- 1G74: A1=CLK, B1=D, C1=QBAR, D1=GND, D2=Q, C2=CLR, B2=PRE, A2=VCC.

Sources (for the note text):

| Device(s) | Datasheet, page |
|---|---|
| 1G00, 1G00_SON | `gate-nand/sn74aup1g00.pdf` p.5 |
| 1G02, 1G02_SON | `gate-nor/sn74lvc1g02.pdf` p.3; `gate-nor/sn74aup1g02.pdf` p.3 |
| 1G08, 1G08_SON | `gate-and/sn74aup1g08.pdf` p.4 |
| 1G32, 1G32_SON | `gate-or/sn74aup1g32.pdf` p.3 |
| 1G86 | `gate-xor/sn74lvc1g86.pdf` p.3 |
| 2G00 / 2G02 / 2G08 / 2G32 / 2G86 | `sn74aup2g00.pdf` p.1 / `sn74aup2g02.pdf` p.1 / `sn74aup2g08.pdf` p.3 / `sn74aup2g32.pdf` p.1 / `sn74lvc2g86.pdf` p.1 |
| 1G10 / 1G11 / 1G27 / 1G332 / 1G386 | `sn74lvc1g10.pdf` p.1 / `sn74lvc1g11.pdf` p.3 / `sn74lvc1g27.pdf` p.1 / `sn74lvc1g332.pdf` p.1 / `sn74lvc1g386.pdf` p.1 |
| 1G74 / 2G74 | `ff-d/sn74aup1g74.pdf` p.3 / `ff-d/sn74lvc2g74.pdf` p.3 |
| 1G79 / 1G80 | `ff-d/sn74lvc1g79.pdf` p.3 / `ff-d/sces221t.pdf` p.3 |
| 1G175 / 1G374 | `ff-d/sn74lvc1g175.pdf` p.3 / `ff-d/sn74lvc1g374.pdf` p.1 |

## Compatibility / Format Impact

- 58 parts' pin designators change. Version 1 schematics that wire these
  parts still open on the same pins, because positions are kept (plan-02-B).
- Version 2 files save their balls by name.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. Use `- [~]` for partial work, plus one line on what
> remains. Mark moot tasks `- [x] ~~text~~ — moot: <evidence>`. Fill
> `Commit:` the moment a phase lands. An unticked box means NOT DONE.

### Phase 1 — SRAM TFBGA balls

- [ ] `sram.json`: the 12 pinouts get `"ball"` designators per §4.1. Rename `p48`/`p90` to `bga` in the pinouts and in their parts.
- [ ] `sram.json`: BGA note segments name the grid (AS6C4016, AS7C34098A_BGA and AS7C351232 gain one).
- [ ] `backend/tests/golden/parts.txt`: regenerate with `/tmp/logic-bga/listing_check --print > backend/tests/golden/parts.txt`.

Acceptance: only the 15 TFBGA lines change, and traces are unchanged.
  Check: `cmake --build /tmp/logic-bga -j8 && ctest --test-dir /tmp/logic-bga` → `100% tests passed` (after the regeneration); `git diff --numstat backend/tests/golden` → `15	15	backend/tests/golden/parts.txt` only; `git diff -U0 backend/tests/golden/parts.txt | grep '^+[A-Z]' | grep -vc '|TFBGA|'` → `0`; the §4.1 spot checks (est. 1 min).
Commit: —

### Phase 2 — DSBGA balls

- [ ] `gate.json`: 19 devices get a `"dsbga"` pinout per §4.2, and their 35 DSBGA parts switch to it.
- [ ] `dff.json`: 6 devices get a `"dsbga"` pinout per §4.2, and their 8 DSBGA parts switch to it.
- [ ] Device notes gain the `DSBGA balls:` source (§4.2 table).
- [ ] `backend/tests/golden/parts.txt`: regenerate as in Phase 1.

Acceptance: only the 43 DSBGA lines change, and traces are unchanged.
  Check: `ctest --test-dir /tmp/logic-bga` (after rebuild and regeneration) → `100% tests passed`; `git diff --numstat backend/tests/golden` → `43	43	backend/tests/golden/parts.txt` only (relative to Phase 1's commit); `git diff -U0 backend/tests/golden/parts.txt | grep '^+[A-Z0-9]' | grep -vc '|DSBGA|'` → `0`; `git diff -U0 backend/tests/golden/parts.txt | grep -c ':DNU:nc:'` → `4`; the §4.2 spot checks (est. 1 min).
Commit: —

## Validation Plan

- Tests: `listing_check` (the regenerated lines), `trace_check` (unchanged
  `traces.txt`, the proof that no position moved), `library_check` (all part
  files load).
- Coverage check: `/tmp/logic-bga/listing_check | tail -1` reports `3817 parts compared`. Every converted part is in the denominator.
- Runtime proof (with the `run` skill):
  1. Place `AS6C1008-55BIN` and `SN74LVC1G00YZPR`. The pin labels read A1 …
     H6 and A1/B1/C1/C2/A2.
  2. Wire VCC to the gate's `A2` (VCC), save, and run
     `jq '.wires[].to.pin'` on the file. It shows `"A2"`.
  3. Open a version 1 file in `/tmp` that wires `SN74AUC1G00YZPR` pin 4 to an
     LED (written by hand from `test.schematical`'s shape). The wire lands
     on the `C2`/`Y` pin.
- Doc sync: none beyond the part files (format documented in plan-02-A).
- Final gate (run once, after Phase 2, plan-02 overall):
  - `cmake -S backend -B /tmp/logic-bga-final && cmake --build /tmp/logic-bga-final -j8 && ctest --test-dir /tmp/logic-bga-final`
    (est. 30 s);
  - `git diff 6861e27 -- backend/tests/golden/traces.txt` → empty. 6861e27
    is `main` before plan-02-A, so this covers all of plan-02;
  - the `xcodebuild` (est. 30 s) and the runtime proof (est. 10 min).

## Decisions

- All 12 SRAM TFBGA and 25 DSBGA devices get datasheet ball names. Decided by the owner, 2026-10-31. (§4)
- The SiT1534 1508 CSP stays numbered, because its datasheet (Table 3, Figure 2) names pads 1–4. This is a datasheet fact, not a choice. (§1)
- The YFP B2 ball is named `DNU` (datasheet), role `nc`. Author's call: the name follows the datasheet, and the role keeps it unwireable-by-intent and trace-neutral. (§4.2)
- Converted pinouts keep the position order of the numbered pinout they replace. Author's call: this keeps traces and version 1 files exact. (§3)

## Corrections

## Summary

Data-only and mechanical. The maps are fully specified above from the
datasheets. The risk is a mis-ordered line, and the byte-identical
`traces.txt` plus the 58-line `parts.txt` diff catch it. Nothing outside the
three part files and the golden changes.
