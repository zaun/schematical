# plan-70-D: Specs: logic-output optocouplers and digital isolators; the final pick

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-70-C

D fills `specs.tsv` for the `logic` and `digital` rows. That means each
line's pinout, its channels (which pin feeds which), its supply sides,
enables and output style, and its part facts, including each digital
isolator part's default output. D decides the `pending` rows of the two
types. It then closes the pick: the final `skip` list, the obsolete PDFs,
and `footprints-needed.txt`, the package list E and F map.

References:

- plan-70-C (the spec tools, § 3.1's naming), plan-70-G § 3.2–3.5 (pin
  names, channels, sides, part keys).
- `planning/plan-48-data/census/final.py` and `footprint-lines.py` (the
  final-pick and package-line scripts D adapts).

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-C complete | `ls planning/completed/plan-70-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-25: C not started) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `logic` and `digital` row of `specs.tsv` has `pinout`, `channels`,
  `sides` and `facts`. `specs-check.py logic digital` → `0 errors`.
- No row of `candidates.tsv` is `pending`. `final.py --report` prints per
  (type, maker) the `add` count, at most 200.
- `datasheets/skipped-parts.md`'s plan-70 section lists every `skip` row
  (`final.py --verify-skipped` → `0 missing`).
- `footprints-needed.txt` has one line per (letter, maker, package,
  supplier package) of the `add` rows, letter `E` (optocouplers) or `F`
  (digital isolators), with the `add` count and the existing footprint of
  the same outline and pin count when there is one.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No footprint drawn here (E, F).

## 2. Current State

Logic-output optocouplers (Broadcom HCPL / ACPL, Toshiba TLP23xx / TLP27xx,
Renesas PS9xxx) have an LED or a logic input on side 1. On side 2 they have
`VCC`, `GND`, an output per channel, and on some an enable (`VE`, active
high, pulled up inside: 6N137). Their outputs are open collector or
push-pull, and inverting or not, per the datasheet's truth table. Digital
isolators (ADuM, ISO77xx, Si86xx) have `VDD1`/`GND1` and `VDD2`/`GND2`, a
channel per input/output pair in either direction (the "Inputs – Side 1/Side
2" facet, A § 2), and on some an enable per side. Their default output when
the input side is not powered is an ordering option (B § 2: Si86xx Table 3,
ISO772x suffix F).

## 3. Design

### 3.1 Spec columns for these types

- `pinout`: as C § 3.1. Names are the datasheet's (`VDD1`, `GND1`, `VIA`,
  `VOA`, `EN1`, `VCC`, `VO`, `VE`). LED pins are renamed `A`/`K` (`A1`/`K1`,
  …).
- `channels`: `in>out;…` per channel, the input pin first (`A1/K1>VO1`
  for an LED input, `VIA>VOA` for a logic input), with `!` after `out` when
  the channel inverts (`A/K>VO!`: 6N137, output low while the LED is on).
- `sides` (`logic`: one side; `digital`: two): `power,ground[,enable+|enable-][,pullup|pulldown]`
  per side. For example `VDD1,GND1;VDD2,GND2,EN2+,pullup` means EN2 is
  active high and reads 1 when left open.
- `facts`: `output` (`push-pull`, `open-drain`, `tri-state`), `viso`,
  `rate` (Mbit/s, the datasheet's maximum data rate), `vcc` (side 1
  min-max V for `digital`, the output side for `logic`), `vccB` (`digital`
  side 2), and `failsafe` (`high`/`low`, `digital` only). `failsafe` is set
  per part by override (C § 3.2's `key@regex=value`) when the ordering key
  makes it a suffix.

### 3.2 The final pick (`census/final.py`)

- `--apply` writes the final `decision` per row (`add`, `skip`, `out`) and
  ticks the refill until every (type, maker) is at min(200, kept rows).
- `--skipped` / `--verify-skipped`: the plan-70 section of
  `datasheets/skipped-parts.md` (B's section, rewritten from the final
  verdicts; read before write).
- `--obsolete`: B § 3.2 rule 5 on the final rows.

### 3.3 `footprints-needed.txt` (`census/footprint-lines.py`)

Columns: `letter company package supplier_package pins pitch body pdf
add_rows example existing`. `existing` is a `footprints.json` id with the
same pin count, pitch, and body within 0.1 mm, or empty. A line is listed
only while it carries an `add` row.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: Logic-output optocouplers

- [ ] `specs-check.py` gains the `channels` and `sides` checks: every input
      and output pin is in exactly one channel, every power pin is in a
      side, and enable pins exist. `--selftest` covers both.
- [ ] Fill every `logic` row (Broadcom, Toshiba, Renesas); merge;
      `scheme-check.py logic`.

Acceptance: the logic rows are complete and decided.
  Check: `specs-check.py logic` → `0 errors`; no `logic` row `pending`
  (`awk -F'\t' 'NR>1 && $1=="logic" && $23=="pending"' planning/plan-70-data/candidates.tsv | wc -l`
  → 0). Takes about 2 min.
Commit:

### Phase 2: Digital isolators

- [ ] Fill every `digital` row (ADI, TI, Skyworks), with `failsafe` for
      every part; merge; `scheme-check.py digital`.

Acceptance: the digital rows are complete, and every `add` digital part
resolves a `failsafe`.
  Check: `specs-check.py digital` → `0 errors, 0 parts without failsafe`.
  Takes about 2 min.
Commit:

### Phase 3: The final pick and the package lines

- [ ] `census/final.py --apply`, `--obsolete`, `--skipped`; the refill for
      rows C and D made `skip`, through B's loop and C or D's spec steps.
- [ ] `census/footprint-lines.py` → `footprints-needed.txt`.
- [ ] Record the final per-(type, maker) `add` counts and the line counts
      (E, F; with and without an `existing` id) in § Corrections.

Acceptance: the pick is final and every package line is listed.
  Check: `python3 -I planning/plan-70-data/census/final.py --verify-skipped`
  → `<n> skip rows, 0 missing`;
  `awk -F'\t' 'NR>1 && $23=="pending"' planning/plan-70-data/candidates.tsv | wc -l`
  → 0; `awk -F'\t' '!/^#/{s+=$9} END{print s}' planning/plan-70-data/footprints-needed.txt`
  equals the `add` count. Takes about 2 min.
Commit:

## Validation Plan

- Tests: `specs-check.py --selftest`; the acceptance checks.
- Coverage check: `git status --short` lists only `planning/plan-70-data/`,
  `datasheets/`.
- Doc sync: `datasheets/skipped-parts.md`, `planning/plan-70-data/README.md`.
- Final gate: plan-70-J.

## Decisions

- Author: **the default output is a part key** (`failsafe`), since the
  ordering option, not the die, sets it (B § 2).
- Author: **an enable's open level is a spec fact** (`pullup` /
  `pulldown`). The datasheets give it (6N137 `VE`: internal pull-up), and G
  reads an open enable by it.
- Author: **a logic-output optocoupler's side 1 has no supply unless its
  input is a logic input.** Then its `sides` has two entries, as a digital
  isolator's.

## Corrections

## Summary

The care is in each line's truth table: inverting or not, the output
style, and the default level. These are what G's model follows.
