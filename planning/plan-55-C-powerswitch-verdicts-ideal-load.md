# plan-55-C: Verdicts — ideal diodes and load switches: pinouts, facts, skips

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-55-B

C reads the datasheets B placed for the **ideal-diode** and **load-switch** candidate rows (at most
90 and 241, plan-55-A § 2) and gives every row a verdict, `add` or `skip`, with its pinout, its
switches and its facts in the form plan-55-A § 3.1 defines. It writes the rules D and E follow
(§ 3). The behavioral outcome: every candidate row of the two families has a row in
`planning/plan-55-data/verdicts.tsv`; every `add` row names a `specs.tsv` row whose pinout and
elements pass `pinout-check.py`; every `skip` row is listed in `datasheets/skipped-parts.md` with
its reason. No part file, footprint, backend or app file changes.

References:

- `planning/completed/plan-55-A-powerswitch-kind.md` § 3.1 (the kind's keys, roles, enable and flag
  modes), § 3.2 (naming conventions); plan-55-B § 3.2 (`candidates.tsv`).
- `planning/completed/plan-42-B-regulator-verdicts.md` § 3 and `plan-42-C-regulator-pinouts.md` § 3
  (the verdict and pinout rules this letter adapts), `planning/plan-42-data/census/` (`verdicts.py`,
  `pinout-check.py`, `scheme-check.py`: the scripts C adapts).
- `datasheets/skipped-parts.md` (the format), `AGENTS.md` (§ Datasheets).

## Prerequisites

plan-55-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-55-B complete | `ls planning/completed/plan-55-B-*.md` → exists | MET (2026-11-23: archived at bb8897b7) |
| Every candidate row has a PDF | `awk -F'\t' '$12=="candidate" && $14==""' planning/plan-55-data/candidates.tsv \| wc -l` → 0 (`state` is column 12, `pdf` column 14 of plan-55-B § 3.2) | MET (2026-11-23: 0; 919 candidate rows, every one with a PDF) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-55-data/verdicts.tsv` has one row per candidate row of the families `ideal` and
  `load`: `mpn`, `verdict` (`add` or `skip`), `reason` (a skip reason of § 3.2), `spec` (the
  `specs.tsv` row key), `flags` (`automotive`, `military`, `enhanced`, or empty).
- `specs.tsv` rows of those families have their pinout, elements and facts filled.
- `python3 -I planning/plan-55-data/census/pinout-check.py --family ideal --family load` → `0 pinout errors`.
- `datasheets/skipped-parts.md` has a `## Power switches (plan-55)` section with one row per
  `skip` row of these families (grouped by reason and PDF).

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app; the part files are H's.
- No footprint decision: C records DigiKey's `Package / Case` and `Supplier Device Package`
  per spec row; F and G draw the footprints (`footprints-needed.txt` is E's).
- No part is added from a datasheet that does not print its part number or its ordering-code
  scheme (`not in the datasheet's ordering table`).

## 2. Current State

B's output: `candidates.tsv`, `specs.tsv` (one row per (company, series, PDF), pinout columns empty),
`datasheets/powerswitch/<company>/`. The four datasheets read at planning (plan-55-A § 2: LM66100,
LM66200, TPS22918, TPS2051B) are the worked examples of § 3.3.

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidates at most, families `ideal` and `load` | 90 + 241 = 331 | plan-55-A § 2 (`pick` lines of `families.out`) |
| Candidate rows after B | the sum of `state` `candidate` rows of the two families | `awk -F'\t' '($1=="ideal"\|\|$1=="load") && $12=="candidate"' planning/plan-55-data/candidates.tsv \| wc -l` (B's output; recorded here when C starts) |

### Verified properties

- **The kind covers the four worked datasheets.** LM66100 is one `ideal-diode` switch with `CE`
  (low, `"input"`), `ST` (`"disabled"`), `NC` pin 4 `nc`; LM66200 is two `ideal-diode` switches
  sharing `VOUT` and `ON` (low, `"logic"`) with `ST` in `"analog"`; TPS22918 is one `load` switch
  (`ON` high), `CT` and `QOD` in `"analog"`; TPS2051B (letter D's family) is one `limited` switch
  (`EN` high, `OC` `"fault"`). Read from the pin tables of the PDFs in plan-55-A § 2.

## 3. Rules (D and E follow them)

### 3.1 `specs.tsv` columns

`company`, `series`, `pdf`, `ordering` (a regex that fully matches the orderable part numbers of the
document's ordering table), `package`, `supplier_package`, `pinout` (JSON `[[number, name, role],
…]`, every lead once, numbers as the package drawing, BGA balls as `A1`), `type` (`load`, `limited`,
`ideal-diode`), `switches` (a JSON list of the kind's element objects), `analog` (a JSON list),
`vin_min`, `vin_max`, `iout`, `ilimit`, `ron`, `iq` (volts, amps, ohms, as the kind's keys), `flags`,
`note`. One row per (series, package pin assignment); a series with two enable polarities by part
number is two rows (two devices, plan-55-A § 3.2).

### 3.2 Verdicts

A row is `add` only when all of these hold; any other outcome is `skip` with exactly one of these
reasons:

1. `no datasheet on the maker's site` — B's `dead` 1 rows.
2. `not in the datasheet's ordering table` — the part number is not printed verbatim in the PDF and
   no `ordering` regex fully matches it.
3. `control` — the part is switched by anything but an enable pin (I²C, SPI, PWM, an analog level)
   or its enable is a multi-level setting.
4. `select logic` — two or more inputs with a select, priority or mode pin that decides which one
   powers the output (a power mux); two ideal diodes sharing one output with at most one shared
   enable and no select pin (the LM66200) are `add`.
5. `external pass element` — the package has a gate pin for a pass transistor outside it.
6. `more than 4 switches` or `negative supply` — outside the kind's 1 to 4 elements, or a supply
   below ground.
7. `no pin table` — the PDF has no pin table the text scan reads and no figure the pinout can be
   read from.

### 3.3 Pin, enable, flag and fact rules

1. **Names** as the datasheet's pin table prints them; several pins of one name are one node.
2. **Roles.** Each switch's input pin(s) `power`; GND `ground`; each output `output`; enable `input`;
   a flag `output`; NC / DNC `nc`; a numbered position with no lead `absent`; an exposed pad is not
   a pin. Every other connected pin (slew CT, soft-start SS, discharge QOD, current-limit set ILIM,
   VBIAS, MODE, a push-pull or comparator status such as the LM66200's ST, a power-good pin) takes
   its electrical role (`input`, `output`, `inout`, `power`) and goes in `analog`.
3. **The enable.** EN, ON, CE, ENABLE, SHDN, `ON/OFF`: `enableActive` as the datasheet's truth table
   says; `enableRef` is `"input"` only when the datasheet says the pin is compared with the input
   voltage (the LM66100's CE) and its active level is low; a part with no enable pin has none.
4. **The flag.** An open-drain, active-low fault, overcurrent or thermal output (FLG, OC, FAULT,
   `nFAULT`) is `flag` with `"fault"`; an open-drain output the datasheet says is low when the switch
   is disabled (the LM66100's ST) is `flag` with `"disabled"`; every other output is `analog`.
5. **Elements** (`switches`): one object per channel in the datasheet's numbering (`IN1`/`OUT1`
   first); channels that share an input or an output pin share its name.
6. **Facts.** `vin_max` is the recommended operating maximum input, the absolute maximum when the
   datasheet gives no recommended one; `vin_min` the undervoltage lockout or the recommended minimum;
   `iout` the continuous current per switch; `ilimit` (a `limited` part with one fixed value) the
   typical current limit; `ron` the typical on-resistance at the highest input; `iq` the typical
   quiescent current. A fact the datasheet does not give is empty. A part with DigiKey
   `Current - Output (Max)` of one value and a datasheet that gives another takes the datasheet's.
7. **Flags** (`flags`): `automotive` for AEC-Q100 qualified parts (DigiKey `Grade` Automotive, `-Q1`),
   `military` for 5962- and SMD-drawing numbers, `enhanced` for `-EP`.

`census/pinout-check.py --family <f>` checks, for every spec row an `add` row uses: every lead of the
package (the pin count from `Package / Case`) appears once; every name an element uses names a pin
of the pinout; roles follow rule 2; `analog` names exist; an `"input"` enable has `enableActive`
`low`; `ilimit` only on a `limited` row. It prints `<n> pinout errors`, one line per error.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the
> moment a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The scripts and the worked examples

- [x] `census/verdicts.py` (adapted from plan-42's: reads `candidates.tsv` and `specs.tsv`, applies
      § 3.2, writes `verdicts.tsv`, prints the counts per family, company and reason; `--verify-skipped`
      checks `datasheets/skipped-parts.md`), `census/pinout-check.py` (§ 3.3). Beside them `census/merge-specs.py` (merges a reader's packet into `specs.tsv`) and `census/texts.py` (the `pdftotext` texts the readers use): Correction 1.
- [x] The Texas Instruments rows of the three worked datasheets of these families (LM66100,
      LM66200, TPS22918) in `specs.tsv`, checked by `pinout-check.py`. Observed: `pinout-check.py --family ideal --family load` → `4 rows checked; 0 pinout errors` (lm66100-q1, lm66200, tps22918, tps22918-q1).

Acceptance: the scripts run on the worked rows.
  Check: `python3 -I planning/plan-55-data/census/pinout-check.py --family ideal --family load` → `0 pinout errors` (est. 1 min).
Commit: (filled by the next commit)

### Phase 2 — Texas Instruments (ideal diodes and load switches)

- [ ] Read every TI PDF of the two families (`pdftotext -layout`): ordering tables, pin tables,
      electrical characteristics; fill `specs.tsv` (§ 3.1) and the facts (§ 3.3).

Acceptance: every TI row of the two families has a spec row or a skip reason.
  Check: `python3 -I planning/plan-55-data/census/verdicts.py --family ideal --family load --company ti --report` → every TI row `add` or `skip` with a reason of § 3.2 (est. 1 min); `pinout-check.py` → `0 pinout errors`.
Commit: —

### Phase 3 — Analog Devices, Monolithic Power Systems, onsemi, Microchip

- [ ] The same for Analog Devices and Monolithic Power Systems (ideal diodes) and onsemi and Microchip
      Technology (load switches).

Acceptance: every row of the two families has a verdict.
  Check: `python3 -I planning/plan-55-data/census/verdicts.py --family ideal --family load --report` → no row without `add` or `skip`; `pinout-check.py` → `0 pinout errors` (est. 2 min).
Commit: —

### Phase 4 — Skips and the hand check

- [ ] `datasheets/skipped-parts.md`: a `## Power switches (plan-55)` section, one row per `skip` row of
      the two families (part numbers grouped by reason and PDF); the section's header names plan-55.
- [ ] Hand-check five `add` rows (two ideal diodes, three load switches, from different companies)
      against their PDFs: name, pin order, enable polarity, `vin_max`; record in § Corrections.

Acceptance: every skip is listed and the hand checks agree.
  Check: `python3 -I planning/plan-55-data/census/verdicts.py --family ideal --family load --verify-skipped` → `0 skip rows missing from skipped-parts.md` (est. 1 min).
Commit: —

## Validation Plan

- Tests: the two scripts' output is the check; no backend or app test is touched.
- Coverage check: `verdicts.py --report` counts every candidate row of the two families once.
- Runtime proof: none (data only).
- Doc sync: `datasheets/skipped-parts.md`; `planning/plan-55-data/README.md` lists the scripts.
- Final gate: plan-55-K.

## Decisions

- Owner, 2026-11-22 (plan-55-A § Decisions): scope, catalog rule, naming.
- Author: **the skip reasons are a closed list** (§ 3.2), so a part the kind cannot express is
  never forced into it and never silently dropped.
- Author: **a status output that is not an open-drain fault or disabled flag is `analog`** (§ 3.3
  rule 4): the model never drives it.

## Corrections

1. **The `specs.tsv` rows are keyed (company, series, pdf, package, supplier_package)** (plan-55-B Correction 8) and carry a `skip` column (one of § 3.2's reasons 3 to 7: `control`, `select logic`, `external pass element`, `more than 4 switches`, `negative supply`, `no pin table`) beside § 3.1's columns: a spec row that the kind cannot express is a `skip` row and every candidate row of its key is a skip with that reason (reasons 1 and 2 stay per part: `no datasheet` is B's `out` rows, `not in the datasheet's ordering table` is `hit` 0 with no `ordering` match). A key may have several rows told apart by `ordering`.
2. **The readers work in packets.** The 372 spec rows of the three families are filled by readers in parallel (one packet of whole PDFs each, output a TSV of the `specs.tsv` columns, checked by `pinout-check.py --specs <packet> --all` and merged by `merge-specs.py`); the Texas Instruments, Microchip and Diodes rows of `limited` (plan-55-D and E's reading) are in the same packets, because a PDF serves several families and is read once. C's Phases 2 and 3, D's Phase 1 and E's Phase 1 are ticked as their rows are merged and checked; each letter's own checks, hand checks and verdicts are run in its turn.
3. **`pinout-check.py` checks filled rows** (non-skip rows with a pinout, type or switches) of the requested families and companies; `--complete` also reports a used row that is not read; `verdicts.py` refuses a candidate whose spec row is neither read nor a skip.

## Summary

C is reading, not code: the risk is a pin name or enable polarity transcribed wrongly, which the
checker (`pinout-check.py`), the loader's role rules in H and the five hand checks catch before a
part reaches the library.
