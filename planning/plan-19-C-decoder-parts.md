# plan-19-C: Decoder, encoder and 7-segment parts

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-19-B

C writes `schematical/schematical/Parts/decoder.json` from
`planning/plan-19-data/parts.tsv` and `specs.md`, adds CD4511B to the
existing `latch.json` device family, and regenerates the goldens.

Outcome of C: `grep -c '|decoder|' backend/tests/golden/parts.txt` = the
number of `add` rows of `parts.tsv` other than CD4511B (N19 − n4511);
`parts.txt` grows by exactly N19 lines; no existing line changes.

References: plan-19-A §3.3 (data conventions), plan-19-B §4 (model keys),
`docs/parts-format.md` § Parts, § Footprints.

## Prerequisites

plan-19-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-19-B complete | `ls planning/completed/plan-19-B-*.md` → exists | NOT MET (2026-11-07) |
| Part data present | `awk -F'\t' 'NR>1 && $11=="add"' planning/plan-19-data/parts.tsv \| wc -l` → N19 (recorded by A) | NOT MET (A not run) |

## 1. Goal

- `decoder.json` loads; `ctest` T0 + 1 of T0 + 1.
- `wc -l < backend/tests/golden/parts.txt` = P(start of C) + N19; same for
  `traces.txt`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0.
- Every device id in `specs.md` appears once in `decoder.json` or
  `latch.json`.

### Non-goals

- No model change (a part that needs one is a plan-19-B bug: fix it there,
  with a `decoder_test` case, and record it in Corrections).
- No change to existing `latch.json` devices other than adding CD4511B parts
  (§3).

## 2. Current State

- `latch.json` device `74_4511` (CD74HC4511/HCT4511, model `bcd7seg`,
  pinout p16). CD4511B has the same function and pin order (CD4511B
  datasheet pin map = CD74HC4511's; checked in A's `specs.md` § 4511B).
- Part lines are generated, not hand-written (plan-10 precedent: a script
  under `/tmp/p19/` reads `parts.tsv` and writes the `parts` arrays; it is
  not committed; the goldens and the counts below check its output).

## 3. Design

- `decoder.json`: `{"format": "logic-parts", "version": …, "kind":
  "decoder", "devices": [...]}` with the header of the other part files.
  Devices in function order of plan-19-A §3.1; each device's `model`,
  model keys, `pinouts`, `timing` from `specs.md`; `parts` from `parts.tsv`
  (`name`, `package`, `pinout`, `footprint`, `family`, `flags` when not
  empty, `vcc`/`inputMax` for SN54 parts).
- CD4511B: new device `4511B` in `latch.json` (CD4000 device ids are
  `<number>B`), model `bcd7seg`, keys copied from `74_4511` with the
  CD4511B pin names and its `C` family timing from `specs.md` § 4511B.
- `function` / `drawer` per plan-19-A §3.3.

## Phases

> **Status discipline:** as plan-19-A § Phases.

### Phase 1 — Part files

- [ ] Generate `decoder.json` and the `4511B` device; load check: `ctest --test-dir /tmp/logic-p19 -R library_check` → pass.
- [ ] Device coverage: every `device` in `parts.tsv` `add` rows is a device id in the two files (python set difference → empty).

Acceptance: both checks (seconds).
Commit: —

### Phase 2 — Goldens and spot checks

- [ ] Regenerate `parts.txt` and `traces.txt` (docs/backend.md § Tests). Check: removed lines 0; added lines N19 in each.
- [ ] Spot checks against `specs.md`, one listing line each: `SN74HC138N` (`|decoder|`, `HC`, `units=1`, `PDIP-16`, `vcc=2000-6000`), `SN74LS145N` (open-drain outputs), `SN74LS148N` (`Encoder`), `SN7447AN` (`7-segment decoder`), `CD4511BE` (`|latch|`), one `military` and one `automotive` part.
- [ ] `trace_check --verbose SN74HC138N` shows exactly one of Y0…Y7 low whenever G1 = 1 and G2A = G2B = 0.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts as §1 (seconds).
Commit: —

## Validation Plan

C's gate: Phase 2 (`ctest` about 13 s, golden counts). Landing tier in D.

## Decisions

- CD4511B is a `latch` part (owner scope, plan-19-A Decisions; design
  plan-19-A §3.2).

## Corrections

## Summary

C is data entry against a tested model; the risk is a wrong pin or polarity,
caught by the device coverage check, the spot checks and the trace check.
