# plan-10-A: Counters, dividers, latches, shift registers — groundwork

Last updated: 2026-11-01
Overall Effort: huge (> 3d) — plan-10 is A → B → C → D → E → F → G → H → I → J
Effort: medium (1h–2h)
Depends on: nothing (§ Prerequisites)

plan-10 adds every TI counter, divider, latch and shift register the owner
downloaded into `datasheets/{counters,divider,latch,shifters}/`: 2097
orderable parts in 124 devices. Three of those devices are edge-triggered
D flip-flops and join the existing `dff` kind; the other 121 need three new
part kinds — `latch`, `counter` and `shift` — whose devices pick a
behavioral model (§3.2). Each part simulates the datasheet's function table,
names a footprint, carries its supply range, and shows up in the app's
drawer.

The single behavioral outcome of plan-10: loading the part files gives 6108
parts (4011 today + 2097), every new part runs its datasheet behavior in a
model test or a golden trace, and `ctest` passes 15 of 15.

This sub-plan does the groundwork no new kind depends on yet:

- the datasheet folders: drop the 10 PDFs the owner left out (on-chip
  oscillators, the JTAG scan port, a part with no orderable numbers) and
  record them; commit the remaining 234;
- five new logic family codes (ABT, BCT, LVT, AVC, TPIC);
- a per-pin open-drain query for the voltage check, replacing the gate-only
  one;
- an `"output": "open-drain"` option on `dff`;
- the 34 flip-flop parts: SN74LS374/SN74S374 into the existing `74_374`
  device, and the TPIC6273 and TPIC6B273 power-logic octal flip-flops.

Outcome of A: `ctest` 12 of 12; the goldens differ from today's only by 34
added part lines (each golden), and `footprints.txt` is unchanged.

References:

- `AGENTS.md` (datasheet rules, part files, goldens).
- `docs/parts-format.md` (whole file; § `dff`, § Values, § Supply voltage,
  § Footprints), `docs/backend.md` (adding a kind, tests).
- `planning/plan-10-data/README.md` and the files it lists: every device's
  pinouts, behavior, timing and VCC, and `parts.tsv`, one row per part.
- `planning/completed/plan-04-A-eeprom-kind.md` (the last kind added; the
  pattern this plan follows), `planning/completed/plan-03-B-logic-footprints.md`
  §3 (pad rules R1–R6).

## Prerequisites

This table is the gate for all of plan-10. B–J point here and add their own
rows.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p10 && cmake --build /tmp/logic-p10 -j8 && ctest --test-dir /tmp/logic-p10` → `100% tests passed, 0 tests failed out of 12` | MET (2026-11-01 re-run at 1db7b45 in worktree P-10: 12 of 12, 3.6 s) |
| Goldens at their measured size | `wc -l < backend/tests/golden/parts.txt` → 4011; `traces.txt` → 4011; `footprints.txt` → 140 | MET (2026-11-01 re-run: 4011/4011/140) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p10-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01 re-run: BUILD SUCCEEDED) |
| No new kind yet | `ls schematical/schematical/Parts/{latch,counter,shift}.json backend/src/models/{latch,counter,shift,seq}.c 2>/dev/null` → no output | MET (2026-11-01 re-run: no output) |
| The datasheets are present | `ls datasheets/counters/*.pdf datasheets/divider/*.pdf datasheets/latch/*.pdf datasheets/shifters/*.pdf \| wc -l` → 244 | MET (2026-11-01 re-run: 244) |
| plan-09 not started (plan-10 lands first; owner decision) | `ls planning/completed/plan-09-* 2>/dev/null` → no output; `grep -c 'Commit: [0-9a-f]' planning/plan-09-*.md` → every file 0 | MET (2026-11-01 re-run: none archived; 5 files, each 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `ls datasheets/{counters,divider,latch,shifters}/*.pdf | wc -l` → 234, and
  `datasheets/not-included.md` lists the 10 PDFs of §4.1.
- `ctest --test-dir /tmp/logic-p10` → `100% tests passed, 0 tests failed out of 12`.
- `wc -l < backend/tests/golden/parts.txt` → 4045 (4011 + 34), `traces.txt` →
  4045, `footprints.txt` → 140; `git diff backend/tests/golden` shows only
  added lines.
- `grep -c '^SN74LS374\|^SN74S374\|^SN54LS374\|^SNJ54LS374\|^SN54S374\|^SNJ54S374\|^7801102\|^JM38510/32503\|^M38510/32503' backend/tests/golden/parts.txt` → 24;
  `grep -c '^TPIC6273\|^TPIC6B273' backend/tests/golden/parts.txt` → 10.

### Non-goals (explicit constraints)

- No existing part's listing line or trace hash changes. Every existing
  `parts.txt` and `traces.txt` line stays byte-identical (the gate for this
  sub-plan: only added lines).
- The `dff` model's behavior for every existing part is unchanged; the new
  `"output"` key defaults to `push-pull`, which is today's behavior.
- The voltage check's results for every existing circuit are unchanged: the
  per-pin query returns exactly what `gate_open_drain` returned for gates and
  0 for every other existing part.
- No new kind, no app change, no footprint (all three flip-flop devices use
  existing footprints, §4.3).

## 2. Current State

- Logic families: `kind_logic_family` (`backend/src/library/kind_util.c:435-450`)
  accepts the 20 names in `FAMILIES` (`HC LS ACT HCT F AC AHCT ALS AS AHC S C H
  HCS LV LVC AUP AUC ALVC TTL`); `docs/parts-format.md` § Values lists the same
  20.
- Open drain: `voltage.c:263` treats a part's `PIN_OUTPUT` pins as not driving
  a voltage when `gate_open_drain(part)` (`models/gate.c:297`, declared in
  `models/model_pins.h:20`) is true. That is the only caller.
- `dff` (`backend/src/models/dff.c`): device keys `units outputEnable
  presetActiveHigh clearActiveHigh clockEdge timing`; outputs always driven
  strong (`schedule_pin` → `sim_schedule_inertial_drive(..., DRIVE_STRONG, ...)`).
  `74_374` in `dff.json` has timing entries HC, HCT, AHCT, … but no LS or S.
- Datasheets: the four folders are untracked (`git status --short` →
  `?? datasheets/counters/` … `?? datasheets/shifters/`), 244 PDFs.
- The two flip-flop sources: `latch/sn74ls373.pdf` (LS374/S374 sections;
  `plan-10-data/latch-a.md` § 74_374) and `latch/tpic6273.pdf`,
  `latch/tpic6b273.pdf` (`latch-b.md` § TPIC6273, § TPIC6B273).

### Measured populations

| What | Count | Command |
|---|---|---|
| PDFs downloaded | 244 | prerequisite command above |
| PDFs left out (§4.1) | 10 | §4.1 table |
| Orderable parts plan-10 adds | 2097 | `tail -n +2 planning/plan-10-data/parts.tsv \| wc -l` |
| Devices | 124 | `cut -f2 planning/plan-10-data/parts.tsv \| tail -n +2 \| sort -u \| wc -l` |
| … of them `dff` (this sub-plan) | 3 devices, 34 parts | `awk -F'\t' '$2=="74_374"\|\|$2~/^TPIC6B?273$/' planning/plan-10-data/parts.tsv \| wc -l` → 34 |
| … `latch` (C) | 28 devices, 750 parts | `planning/plan-10-C-latch-parts.md` §2 |
| … `counter` (F) | 57 devices, 730 parts | `planning/plan-10-F-counter-parts.md` §2 |
| … `shift` (I) | 36 devices, 583 parts | `planning/plan-10-I-shift-parts.md` §2 |
| Parts flagged obsolete | 275 | `awk -F'\t' '$9=="Obsolete"' planning/plan-10-data/parts.tsv \| wc -l` |
| New footprints | 18 | `planning/plan-10-data/footprints.md` (17 sections + VQFN-20-RGY confirmed; PDIP-20-NE not added) |
| Current tests | 12 | prerequisite command |

### Verified properties

| Claim | How verified |
|---|---|
| No new part name collides with an existing one | Census names vs every `name` in `schematical/schematical/Parts/*.json` (python set intersection, 2026-11-01) → 0 |
| No new datasheet covers only obsolete parts | Each PDF's ordering table (census): every PDF has at least one Active, NRND or Last-Time-Buy row → AGENTS.md's delete rule applies to none |
| Every part maps to an existing or a new footprint | `parts.tsv` `footprint` column: 1921 rows existing ids, 176 rows the 18 new ids, 0 unmapped |
| The 18 new footprints load and do not overlap | Throwaway probe `/tmp/p10/probe` (merged `footprints.json` → `load_footprints` → 158 footprints, pad counts as designed; pairwise pad overlap test → 0) |
| A new `.c` under `backend/src/models/` needs one line in `backend/CMakeLists.txt` (`add_library(logic …)` lists every source) and no app project edit; a new `Parts/*.json` needs neither | `backend/CMakeLists.txt:8-29` lists the sources explicitly; `project.pbxproj`: `backend/src` and `schematical` are `PBXFileSystemSynchronizedRootGroup`s; tests load `Parts/*.json` by directory (`tests/support/read_file.c:39`) and the app by bundle URLs (`PartCatalog.swift:404`) |

## 3. Design Overview (all of plan-10)

### 3.1 Where each part goes

| Kind | Devices | Parts | Sub-plans |
|---|---|---|---|
| `dff` (existing) | `74_374` (+LS, S), `TPIC6273`, `TPIC6B273` | 34 | A |
| `latch` (new) | 28 | 750 | B (kind, models, tests), C (parts) |
| `counter` (new; dividers too) | 57 | 730 | D, E (kind, models, tests), F (parts) |
| `shift` (new) | 36 | 583 | G, H (kind, models, tests), I (parts) |
| app | — | — | J |

### 3.2 Kinds dispatch to models

Each new kind is one `PartKindOps` (`kind.h`) and one `SimComponentType`.
A device names its behavior with a `"model"` key; the kind looks the model
up in its own table and the model supplies the device keys, binding and
engine callbacks behind the kind's single `SimComponentType` (the precedent:
plan-09-B's `cpu` kind, "one `SimComponentType` per kind; a kind with several
behaviours dispatches inside its model").

| Kind | Models (sub-plan) |
|---|---|
| `latch` | `transparent`, `addressable`, `sr`, `bcd7seg`, `bus-mux` (B) |
| `counter` | `sync`, `ripple` (D); `johnson`, `seg7`, `divn`, `rate` (E) |
| `shift` | `shift` (G); `ls673`, `tap64`, `bus-register`, `fifo`, `serial-in` (H) |

Rejected alternatives:

- **One kind per datasheet function** (a `74_161` model, a `74_595` model, …):
  121 models for 2063 parts; most are the same machine with different
  controls. The 17 parametric models cover them; the few one-off machines
  (FIFO, 4034, LV8153, 4517, LS673) get their own models.
- **A register-transfer description language in JSON** (registers, next-state
  expressions, output equations): one engine for everything, but the FIFO's
  2 µs fall-through, the 40110's self-timed carry pulses and the LV8153's
  auto-baud sampling need timers and state machines the language would have
  to grow into a programming language, and expression-heavy part files are
  hard to check against a datasheet.
- **Dividers as a fourth kind**: they are counters (decoded Johnson, programmable
  down counters, rate multipliers); a drawer leaf (`"function": "Divider"`)
  separates them for people without another `PartKind`, listing case and app
  branch.
- **Latches inside `dff`**: level-sensitive transparency, addressable and S-R
  latches, BCD decoding and bidirectional bus latches share nothing with the
  edge-triggered model's code beyond pin parsing.

### 3.3 Shared pieces

- `backend/src/models/seq.h` + `seq.c` (B): the helpers every new model uses —
  family timing entries, control objects, level reading with the Z/X rules of
  §3.5, edge detection, change-suppressed inertial output scheduling with
  3-state, open-drain and inversion, a bit-vector with a known mask, model-key
  checking. B writes it with the latch models; D, E, G and H only add to it.
- Every new kind sets `Part.logic` (`PartLogic`: family, units), so the
  listing prints the family and `units=`, and the app's drawer gets the
  family filter (J).
- `model_pins.h`: `part_pin_open_drain(part, pin)` (A) — the voltage check asks
  per pin; each kind answers for its own parts.

### 3.4 Data conventions (every data sub-plan: A, C, F, I)

- **Parts** come from `planning/plan-10-data/parts.tsv`: one JSON part per row,
  `name`, `package` (the `package` column), `pinout` (§ below), `footprint`
  (the `footprint` column), `family` (the `family` column) and `flags` (the
  `flags` column; omit the key when empty). The `parts` arrays are generated
  from the TSV by a script under `/tmp/p10/`; the script is not committed
  (the goldens and the name/device check of each data sub-plan verify its
  output).
- **Flags** (`parts.tsv` already applies them, per `docs/parts-format.md`
  § Parts): `obsolete` for status Obsolete only (NRND and Last-Time-Buy parts
  are not flagged); `military` for SN54/SNJ54/SNV54/CD54 names, 5962-, JM38510/
  and M38510/ numbers, 7-digit SMD numbers and CD4000 `F3A` parts;
  `automotive` for names ending `Q1`.
- **Device ids**: `74_<function>` for 74-series designs (`74_161`, `74_16373`,
  `74_1G373`), `<number>B`/`<number>A` for CD4000 designs (`4017B`, `4059A`),
  the part number for TPIC parts (`TPIC6B595`). CMOS 4000 functions made in a
  74 family (CD74HC4017) are a separate device `74_4017` because their pin
  names differ. Two devices are split by family because their behavior
  differs: `74_867_AS` and `74_869_AS` (AS gates RCO with ENP too; `counter-a.md`
  § 74_867).
- **Pinouts** are named by pin count and style: `p14`, `p16`, `p20`, `p24`,
  `p48`, `p56` for numbered packages whose pin n is the DIP's pin n (FK, RGY,
  RKS, BQA and BQB included, when the device's spec section says so), `fk20`
  / `fk28` for LCCC maps with NC terminals, `yzp` / `nmj` for ball maps. Pin
  names are the spec section's, which follow the repo convention (no overbar
  marks, `BAR` suffix only for a true/complement pair). Where families of one
  device print different names at the same pin (LS `OC`/`C` vs `OE`/`LE`;
  CD74 `D0..D7` vs SN `1D..8D`), the device uses the spec section's chosen
  names for all its parts. An exposed thermal pad is not a pin: the footprint's
  `centre` pad stays unnumbered (the `VQFN-14` precedent).
- **Family codes** (§4.2): bus-hold variants take the base family — LVCH →
  LVC, ALVCH → ALVC, LVTH → LVT — and the standard TTL 54/74 parts (SN54163,
  SN5497, SN54192, SN54166) are `TTL`.
- **Timing**: one `{rise, fall}` per family, the device's main path, typical at
  the datasheet's reference condition (4.5–5 V, CL = 50 pF; LV/LVC/ALVC/AVC/
  LVT at 3.3 V; CD4000 at VDD = 5 V; AUC at 1.8 V), else the printed maximum
  at that condition, else the printed typical at the lower load (LS/S at
  15–45 pF), with a `note` naming the path, the basis and the page, e.g.
  `"note": "CLK to Q typ, 4.5 V 50 pF; VCC 2-6 V (sn74hc161 p.6)"`. The main
  path per model is fixed in that model's section (clock to Q for counters and
  shift registers, D to Q for transparent latches); models with self-timed
  behavior add the named extra keys their section lists.
- **Supply**: each timing entry's `vcc` is the family's Recommended Operating
  Conditions range from the spec section; `inputMax` where the section gives
  one. Where SN54 and SN74 rows differ, the entry carries the SN74 range and
  each SN54/SNJ54/SNV54/5962-/JM38510/M38510 part carries its own `"vcc"`
  (the `docs/parts-format.md` "SN54 vs SN74" rule).
- **Drawer**: `"drawer": ["Logic"]`, `"function"` one of `"Latch"`, `"Counter"`,
  `"Divider"`, `"Shift register"`; each device's function is set in its
  sub-plan's device table (dividers are the devices from `datasheets/divider/`).
  `"description"` is the datasheet title in sentence case, e.g.
  `"Octal transparent D-type latch, 3-state"`.
- **`note`** on each device: the datasheet(s) and the spec section, e.g.
  `"note": "sn74hc373 p.3, p.9; plan-10-data/latch-a.md § 74_373"`.

### 3.5 Signal rules (every new model)

Mirroring `dff`:

- **Power-on state**: every stored bit 0 (latches, counters, registers,
  shift stages; a FIFO empty), then asynchronous controls apply at start.
  Datasheets say "unknown"; the library's precedent (`dff`: Q = 0 at power-on)
  is kept for every kind so circuits start deterministic.
- **Edges**: Verilog posedge/negedge — any move into 1 (rising) or into 0
  (falling) from another value, including from X or Z.
- **Asynchronous controls** (clear, preset, reset, set, asynchronous load and
  preset enable): Z reads inactive, X makes every bit they can change X.
- **Output enables**: active → driven; inactive → Z; X or Z → X (the `dff`
  `outputEnable` rule).
- **Every other input** (data, clocks, enables, mode and select pins): Z reads
  as X. An X on a synchronous enable or mode pin at a clock edge makes every
  bit that edge could change X; an X data bit stores X.
- **Outputs**: driven strong, inertially (`sim_schedule_inertial_drive`), with
  the entry's fall delay for 0, rise for 1 and Z, the longer for X, and
  rescheduled only when their value changes (the `dff` `scheduled`/`driven`
  pattern). Open-drain outputs drive 0 strong and release to Z for 1.
- **Bus hold** (ALVCH, LVCH, LVTH) is not modeled: a floating data input reads
  X, as on every other part.

### 3.6 Where correctness risk concentrates

- The parametric `sync` counter (D) and `shift` register (G): most parts
  (`74_161` … `74_595`) run through them, and their option combinations
  (sync/async controls, gated clocks, carry equations, storage registers)
  are where a wrong priority silently mislabels hundreds of parts. Each gets a
  model test with one case per function-table row of every device that uses
  it, before any part data lands.
- The data sub-plans (C, F, I) change the goldens by ~2000 lines; they come
  after their kind's model tests so a golden regenerated over a model bug
  cannot hide it.
- Byte-identity is the gate for existing parts only (every existing golden
  line unchanged, in every sub-plan). New parts are proven by the model tests,
  not by goldens: their golden lines are recorded output, reviewed by count
  and by spot checks against the spec sections.

## 4. Detailed Design (this sub-plan)

### 4.1 Datasheets

Delete these 10 PDFs and add `datasheets/not-included.md` (title "Datasheets
not included", the same columns as `obsolete-not-included.md`: PDF, family,
category / group, notes) with one row each:

| PDF | Why (owner decision, 2026-11-01) |
|---|---|
| `counters/cd4060b.pdf` | on-chip RC/crystal oscillator; left out |
| `counters/cd74hc4060.pdf` | on-chip oscillator; left out |
| `counters/sn74hc4060.pdf` | on-chip oscillator; left out |
| `counters/cd4045b.pdf` | on-chip oscillator (21-stage counter); left out |
| `divider/cd4521b.pdf` | on-chip oscillator (24-stage divider); left out |
| `divider/cd4536b.pdf` | on-chip oscillator (programmable timer); left out |
| `divider/cd4541b.pdf` | on-chip oscillator (programmable timer); left out |
| `divider/sn74ls292.pdf` | on-chip oscillator (LS292/LS294 programmable dividers); left out |
| `shifters/sn74abt8996.pdf` | IEEE 1149.1 addressable scan port; no use in the simulator |
| `latch/sn74lvc1g373b-q1.pdf` | no orderable part numbers yet (product preview, SLVSN16, Aug 2026) |

Each notes cell also gives the TI product page URL
(`https://www.ti.com/product/<NAME>`). The 234 remaining PDFs are committed as
they are.

### 4.2 Families

`FAMILIES` in `kind_util.c` gains `ABT`, `BCT`, `LVT`, `AVC`, `TPIC` (appended,
in that order); `docs/parts-format.md` § Values lists them with one line each:
ABT and BCT (BiCMOS bus interface), LVT (3.3 V ABT; LVTH parts use it), AVC,
TPIC (TI power-logic parts with DMOS drain outputs). `library_check` gets one
case: a gate part with `"family": "TPIC"` and a `TPIC` timing entry loads.

### 4.3 Flip-flop parts

- `dff` device key `"output"`: `"push-pull"` (default) or `"open-drain"`.
  Open-drain: a 0 drives 0 strong after the fall delay; a 1 (and Z) releases
  the pin to Z after the rise delay; X drives X. It applies to Q and QN.
  `docs/parts-format.md` § `dff` gets the row.
- `part_pin_open_drain(const Part *part, size_t pin)` in `model_pins.h`
  replaces `gate_open_drain`: 1 when `pin` is a `PIN_OUTPUT` the part never
  drives high. Gate: every output pin when the device is `open-drain`. dff:
  every Q/QN pin when `open-drain`. Every other kind: 0 (B, D and G add
  theirs). `voltage.c:263` calls it with the connection's pin.
- `74_374` gains timing entries `LS` and `S` from `latch-a.md` § 74_374
  (LS CLK→Q typ 15/19 ns at 45 pF; S 8/11 ns at 15 pF; VCC SN74 4.75–5.25 V,
  SN54 parts `"vcc": [4.5, 5.5]`) and the 24 parts of `parts.tsv` with
  device `74_374`. Their pinout is the device's existing 20-pin one: the LS
  sheet's pin 1 `OC` and pin 11 `CLK` are the device's `OE` and `CLK` at the
  same positions (FK pin n = DIP pin n), so the parts use the existing pinout
  and names.
- New devices `TPIC6273` and `TPIC6B273` (`latch-b.md` §§): `units` 8 with
  `d` 1D…8D, a common `clk` CLK and `clr` CLR, `q` absent, `qn` DRAIN1…DRAIN8
  (DRAIN is on, pulled low, when the stored bit is 1, so the pin is the open-
  drain complement), `clearActiveHigh` false, `presetActiveHigh` false,
  `clockEdge` rising, `"output": "open-drain"`; timing `TPIC` from the spec
  section (CLK→DRAIN typ; VCC 4.5–5.5 V); `function` "D flip-flop",
  `drawer` ["Logic"]; pinout `p20` from the spec section (the 20-pin DW and N
  packages share it); the 5 parts each from `parts.tsv` (footprints
  `SOIC-20W`, `PDIP-20`).

## Compatibility / Format Impact

- Part-file format: five more accepted family names; one optional `dff`
  device key. Existing files load unchanged.
- C API: `gate_open_drain` (internal header `model_pins.h`, not public) is
  replaced; no public header changes in A.

## Phases

> **Status discipline (applies to every phase).**
> - A task checkbox is ticked only when its result is observed, with the
>   command and output recorded on the task line.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Anything that differs from the plan goes in Corrections, not silently.
> - Commits are made on the current branch (AGENTS.md: never branch unless the
>   owner names the branch). Itemized commits; touch only this plan's files.

### Phase 1 — Datasheets

- [x] Delete the 10 PDFs of §4.1; write `datasheets/not-included.md`.
  Check: `ls datasheets/{counters,divider,latch,shifters}/*.pdf | wc -l` → 234;
  `grep -c '\.pdf' datasheets/not-included.md` → 10. (Ran: 234; 10.)
- [x] Commit `datasheets/{counters,divider,latch,shifters}/` and
  `datasheets/not-included.md`. Check: `git status --short datasheets` → no
  output. (The 244 PDFs were already on main, c0d447a/68b2c71/94a8526; this
  commit has the 10 deletions and the new file. Ran after commit: no output.)

Acceptance: the two checks above (seconds).
Commit: —

### Phase 2 — Families, per-pin open drain, `dff` `output`

- [x] `kind_util.c` `FAMILIES` + docs § Values (§4.2); `library_check` case.
  (`library_check` → `ok   gate part with family TPIC`.)
- [x] `model_pins.h`/`gate.c`/`dff.c`/`voltage.c`: `part_pin_open_drain`
  (§4.3); `gate_open_drain` removed. Check: `grep -rn gate_open_drain backend` →
  no output. (Ran: no output. Each kind answers through a new optional
  `PartKindOps.pin_open_drain`; the dispatcher is in `kinds.c`, see
  Corrections.)
- [x] `dff.c`: `"output"` key, open-drain scheduling; docs § `dff` row;
  `library_check` cases: `"output": "open-drain"` loads, `"output": "x"` is
  rejected with `"output" must be "push-pull" or "open-drain"`. (Ran:
  `ok   dff with "output": "open-drain"`; `ok   dff output x: test.json:
  devices[0] (d): "output" must be "push-pull" or "open-drain"`.)
- [x] `voltage_test`: (ran: `PASS: an open-drain dff's Q carries no voltage`,
  `PASS: a push-pull dff's Q at 5 V into a 3.6 V input is INPUT_OVER`) one case — a 5 V-powered dff fixture with
  `"output": "open-drain"` whose Q drives input A of `VFX-AUC1G00` (input max
  3.6 V) reports no problem; the same circuit with a push-pull dff fixture
  reports the input-max problem. The two fixtures go in
  `tests/support/voltage_fixture.h` as a third file string
  (`VOLTAGE_FIXTURE_DFFS`, kind `dff`, SOT23-5: 1 D, 2 CLK, 3 GND, 4 Q,
  5 VCC; parts `VFX-OD74` and `VFX-PP74`, `vcc` [2, 6]).

Acceptance: `ctest --test-dir /tmp/logic-p10` → 12 of 12;
`git diff --stat backend/tests/golden` → no change (seconds). (Ran: `100%
tests passed, 0 tests failed out of 12`; golden diff empty.)
Commit: —

### Phase 3 — The 34 flip-flop parts

- [ ] `dff.json`: `74_374` LS and S entries and 24 parts; devices `TPIC6273`,
  `TPIC6B273` with 5 parts each (§4.3).
- [ ] Regenerate `parts.txt` and `traces.txt` (docs/backend.md § Tests).
  Check: `git diff backend/tests/golden | grep -c '^-[^-]'` → 0;
  `git diff backend/tests/golden/parts.txt | grep -c '^+[^+]'` → 34, same for
  `traces.txt`.
- [ ] Spot check three new listing lines against the spec sections:
  `SN74LS374N` (`|LS|`, `units=8`, `vcc=4750-5250`), `SNJ54LS374J`
  (`military`, `vcc=4500-5500`), `TPIC6B273DW` (pins `DRAIN1`… `output`,
  footprint `SOIC-20W`, `vcc=4500-5500`).
- [ ] `trace_check --verbose TPIC6273N` shows no drain pin ever driven `1`
  (only `0` and `Z`).

Acceptance: `ctest` 12 of 12; goldens 4045 / 4045 / 140 lines, added lines
only (seconds).
Commit: —

## Validation Plan

The landing tier for plan-10 runs once, at the end of J (plan-10-J
§ Validation Plan). For A: the Phase 3 acceptance (`ctest` 12 of 12, about
4 s; golden line counts and the added-only diff) is the gate, since A changes
the loader, the dff model and the voltage check — exactly what those 12
tests cover.

## Decisions

- **Scope** (owner, 2026-11-01): all four folders, except the 8 datasheets
  with on-chip oscillators (owner: "leave these chips out" — the PDFs are
  deleted and recorded) and the SN74ABT8996 (owner: model every unusual chip
  but the ABT8996). SN74LVC1G373B-Q1 is dropped because its datasheet has no
  orderable parts.
- **App work is part of plan-10** (owner): sub-plan J.
- **plan-10 lands before plan-09** (owner). plan-09's text is updated in the
  same commit as this plan: plan-09-A's prerequisites require plan-10
  complete, and plan-09-B §2 counts 13 kinds.
- **The NE (power PDIP) package uses `PDIP-20`.** No NE drawing exists in the
  datasheets; TI lists it as a 20-pin 300-mil PDIP whose lead pattern is the
  N package's, so a separate footprint would only change the drawn body.
- **VQFN-20-RGY** is taken from TI drawing 4225320/A (RGY0020A), which the
  repo datasheets do not include (`plan-10-data/footprints.md`).
- **Military numbers** the datasheets do not map are resolved as in
  `plan-10-data/README.md`.
- **Power-on state 0, bus hold not modeled, one timing pair per family**:
  §3.4–§3.5 (the library's existing conventions).
- **New families are separate codes** (ABT, BCT, LVT, AVC, TPIC), not folded
  into near neighbors, so the drawer's family filter tells them apart.

## Corrections

- 2026-11-01, Phase 1: §2 said the four datasheet folders were untracked.
  False when A ran: main's log has c0d447a, 68b2c71, 94a8526 committing them.
  Phase 1's commit therefore only deletes the 10 PDFs and adds
  `datasheets/not-included.md`.
- 2026-11-01, Prerequisites: `/tmp/logic-p10` held a CMake cache generated
  from the main checkout (`CMake Error: The source … does not match`); it was
  deleted and regenerated from the worktree. Command and result otherwise as
  written.
- 2026-11-01, Phase 2: §4.3 said "each kind answers for its own parts" but not
  how. Implemented as an optional ninth `PartKindOps` member
  `pin_open_drain(part, pin)` (NULL = never, so existing kinds' initializers
  are unchanged) and `part_pin_open_drain` in `library/kinds.c`, which checks
  the pin is `PIN_OUTPUT` and dispatches. B, D and G add theirs by setting
  that member. `logic_test_support` gains `src/models` on its include path so
  `library_check` can call `part_pin_open_drain`. `docs/backend.md` § Adding a
  kind names the member.

## Summary

A clears the ground: the datasheet folders committed with 10 left out and
recorded, five family codes, a per-pin open-drain query, an open-drain
`dff`, and the 34 flip-flop parts. No new kind yet; the goldens grow by 34
lines and nothing existing changes.
