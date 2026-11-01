# plan-07-E: Voltage data for flip-flops, SRAM and EEPROM parts

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-07-D

This sub-plan fills in `vcc` (and `inputMax` where it applies) for every
part of the D and JK flip-flop devices, on their timing entries. It does the
same for every SRAM and EEPROM part, on their grades. The method is
plan-07-C §4. Memory datasheets use their own wording, covered in §4 below.

Outcome: all 1204 dff, jkff, SRAM and EEPROM parts list `|vcc=`. The spot
values match their datasheets:

| Part | Value |
|---|---|
| SN74HC74N | `vcc=2000-6000,in=6000` |
| SN74LS112AN | `vcc=4750-5250,in=5250` |
| AS6C62256-55PCN | `vcc=2700-5500,in=5500` |
| AT28C256-15JU | `vcc=4500-5500,in=5500` |

References:

- plan-07-C §2 (the cross-check table, the 54/74 and CD/SN splits) and §4
  (the method)
- plan-07-A §4.1 (`vcc` on dff/jkff timing entries and on SRAM/EEPROM grades)
- `datasheets/ff-d/` (59), `ff-jk/` (21), `sram/` (20), `eeprom/` (12)

## Prerequisites

The same as plan-07-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-07-D complete | `ls planning/plan-07-D-* 2>/dev/null` → no output | NOT MET (2026-11-01) |

## 1. Goal

- Every dff, jkff, SRAM and EEPROM part gets a `vcc` range. It comes either
  from its entry or grade, or from its own key.

### Non-goals (explicit constraints)

- `traces.txt` does not change.
- `parts.txt` changes only by an added `|vcc=` field on lines of these four
  kinds.
- No timing, grade or pinout values change.

## 2. Current State

| Kind | Devices | Pairs | Parts | PDFs |
|---|---|---|---|---|
| dff | 20 | 47 (device, family) | 672 | 59 |
| jkff | 6 | 27 (device, family) | 186 | 21 |
| sram | 21 | 39 (device, grade) | 172 | 20 |
| eeprom | 12 | 29 (device, grade) | 174 | 12 |

Notes on the sources:

- Every SRAM and EEPROM device `note` names its PDF (21/21 and 12/12).
  Only 6 of the 20 dff notes name one, and none of the 6 jkff notes do, so
  for those use the text search from plan-07-C §4.
- Seven flip-flop PDFs are scans (no supply-voltage text; command in
  plan-07-C §2):
  - ff-d: `cd40175b`, `sn74ls74a`
  - ff-jk: `sn54ls76a`, `sn74ls107a`, `sn74ls109a`, `sn74ls112a`,
    `sn74ls73a`
- How memory sheets state VCC (pdftotext output):
  - AS6C62256: `Supply Voltage VCC 2.7 3.3 5.5`, and `VIH … VCC+0.5`, so
    there is no `inputMax`.
  - AT28C256: `VCC Power Supply 5V ± 10%`, which means 4.5–5.5.
  - AT28BV parts: "2.7V to 3.6V".
  - AT28LV010: "3.3V ± 10%", which means 2.97–3.63.

### Measured populations

| What | Count | Command |
|---|---|---|
| devices / parts / pairs per kind | the table above | `python3 -I` census over the four part files (2026-11-01) |
| PDFs per folder | 59 / 21 / 20 / 12 | `ls datasheets/<folder>/*.pdf \| wc -l` |

## 3. Design Overview

This is data entry, following plan-07-C §4. The memory-specific rules are
in §4.

The risk is a percentage range converted wrong, or a BV or LV part
inheriting a 5 V grade. Each EEPROM device has a single supply option, so
the range goes on its grades. The Phase 2 spot check covers one 5 V device
and one BV device.

This work is behavior-neutral.

## 4. Memory rules

- **"X V ± P %"**: min = X·(1 − P/100) and max = X·(1 + P/100), rounded to
  the millivolt. For example, 5 V ± 10% is 4.5–5.5, and 3.3 V ± 10% is
  2.97–3.63.
- **"A V to B V"** or a MIN/MAX row: use the values as printed.
- **Several supply options in one device** (a part-number letter): set the
  grade's `vcc` to the option most of its parts use, and override the other
  parts. Record the letter rule in the device `note`.
- **`VIH` maximum printed as `VCC + 0.5`**: no `inputMax`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick boxes in the same
> commit as the work, use `- [~]` for partial work, mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`, and fill in `Commit:`.
> **An unticked box means NOT DONE.**

### Phase 1 — Flip-flops

- [ ] Run plan-07-C §4 on the 20 dff and 6 jkff devices (74 entries).
- [ ] Regenerate `parts.txt`.

Acceptance: every dff and jkff part has a range, and nothing else changed.
  Check: a python script, like plan-07-C's, over `dff.json` and `jkff.json`
  counting parts without a resolved `vcc` → `0`.
  `git diff --quiet backend/tests/golden/traces.txt` → exit 0.
  `git diff -U0 backend/tests/golden/parts.txt | grep '^+[^+]' | grep -vc '|vcc='`
  → `0` (est. 1 min).
Commit: —

### Phase 2 — SRAM and EEPROM

- [ ] Run §4 (and plan-07-C §4 steps 2–4) on the 21 SRAM and 12 EEPROM
      devices (68 grades).
- [ ] Regenerate `parts.txt`.

Acceptance: every part of the four kinds has a range, and the spot values
match.
  Check: the Phase 1 script over the four files → `0`. Then
  `grep -E '^(SN74HC74N|SN74LS112AN|AS6C62256-55PCN|AT28C256-15JU)\|' backend/tests/golden/parts.txt | grep -o 'vcc=.*'`
  → the Outcome values. One `AT28BV256` part → `vcc=2700-3600,in=3600`
  (est. 1 min).
Commit: —

## Validation Plan

- Tests: `listing_check`.
- Coverage check: the Phase 2 script counts all 1204 parts.
- Runtime proof: none; this is data only.
- Doc sync: none.
- Final gate (once): `ctest --test-dir /tmp/p07` → 12/12 (est. 1 min).

## Decisions

See plan-07-A § Decisions and plan-07-C § Decisions. Author decision: a
± % range is converted to min/max (§4) and rounded to the millivolt.

## Corrections

## Summary

This is data only. The risk is the ± % conversion and multi-option
devices, and the BV spot check guards them.
