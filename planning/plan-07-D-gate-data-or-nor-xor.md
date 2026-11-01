# plan-07-D: Voltage data for OR, NOR, XOR and XNOR gate parts

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-07-C

This sub-plan fills in `vcc` (and `inputMax` where it applies) for every
part of the 35 OR, NOR, XOR and XNOR devices in `gate.json`. It uses
plan-07-C §4's method. When it is done, every gate part has a range.

Outcome: all 2552 gate parts list `|vcc=`, and the spot values match
their datasheets:

- SN74HC02N `vcc=2000-6000,in=6000`
- SN74LS86AN `vcc=4750-5250,in=5250`
- SN74LVC1G32DBVR `vcc=1650-5500,in=5500`

References:

- plan-07-C §2 (the measured splits and the per-family cross-check table)
  and §4 (the method)
- `datasheets/gate-or/` (52), `gate-nor/` (54), `gate-xor/` (39),
  `gate-xnor/` (4)

## Prerequisites

The same as plan-07-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-07-C complete | `ls planning/plan-07-C-* 2>/dev/null` → no output | NOT MET (2026-11-01) |

## 1. Goal

- Every gate part with `"function"` `OR`, `NOR`, `XOR` or `XNOR` gets a
  `vcc` range, either from its timing entry or from its own key.
- After this sub-plan, every one of the 2552 gate parts has a range.

### Non-goals (explicit constraints)

- `traces.txt` must not change.
- The only change in `parts.txt` is an added `|vcc=` field on lines for
  these four functions.
- No other kind is touched.

## 2. Current State

- The four functions have 35 devices and 1148 parts in total.
- (device, family) pairs by function: OR 37, NOR 43, XOR 24, XNOR 4.
  That is 108 timing entries in total.
- The 4 split timing entries (`otherLow`/`otherHigh`) are all in XOR
  devices (74x86). Their `vcc` goes at the top level of the entry
  (plan-07-A §4.1).
- 11 PDFs are scanned, with no supply-voltage text (command in plan-07-C §2):
  - gate-or: `cd4071b`, `sn74ls32`
  - gate-nor: `cd4001b`, `cd4078b`, `sn54hc4002`, `sn74ls02`, `sn74ls27`
  - gate-xor: `cd4030b`, `sn74ls86`, `sn74ls86a`
  - gate-xnor: `sn74ls226`

### Measured populations

| What | Count | Command |
|---|---|---|
| devices / parts | 35 / 1148 | python over `gate.json`, functions OR/NOR/XOR/XNOR → `(1148, 35)` |
| pairs per function | OR 37, NOR 43, XOR 24, XNOR 4 | python census → `{'NOR': 43, 'OR': 37, 'XOR': 24, 'XNOR': 4}` |
| split entries | 4 | python: entries with `otherLow` → `4` |
| PDFs | 149 | `ls datasheets/gate-{or,nor,xor,xnor}/*.pdf \| wc -l` → 149 |

## 3. Design Overview

This sub-plan is data entry that follows plan-07-C §4 exactly. The risk is
the same as in C: a part inheriting the wrong entry value. The step that
lists each part against its datasheet row guards against it. This work is
behavior-neutral (see Non-goals).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick boxes in the same
> commit as the work, use `- [~]` for partial work, mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`, and fill in `Commit:`.
> **An unticked box means NOT DONE.**

### Phase 1 — OR and NOR devices

- [ ] Run plan-07-C §4 on the 12 OR and 14 NOR devices (80 entries).
- [ ] Regenerate `parts.txt`.

Acceptance: every OR and NOR part has a range, and no other field changed.
  Check: the plan-07-C Phase 1 script with `in ('OR','NOR')` → `0`;
  `git diff --quiet backend/tests/golden/traces.txt` → exit 0;
  `git diff -U0 backend/tests/golden/parts.txt | grep '^+[^+]' | grep -vc '|vcc='`
  → `0` (est. 1 min).
Commit: —

### Phase 2 — XOR and XNOR devices

- [ ] Run plan-07-C §4 on the 7 XOR and 2 XNOR devices (28 entries,
      including the 4 split entries).
- [ ] Regenerate `parts.txt`.

Acceptance: every gate part has a range, and the spot values match.
  Check: the script over all gate devices → `0`; then
  `grep -E '^(SN74HC02N|SN74LS86AN|SN74LVC1G32DBVR)\|' backend/tests/golden/parts.txt | grep -o 'vcc=.*'`
  → the Outcome values (est. 1 min).
Commit: —

## Validation Plan

- Tests: `listing_check` checks every value.
- Coverage check: the Phase 2 script covers all 2552 gate parts.
- Runtime proof: none. This is data only.
- Doc sync: none.
- Final gate (once): `ctest --test-dir /tmp/p07` → 12/12 (est. 1 min).

## Decisions

See plan-07-A § Decisions and plan-07-C § Decisions.

## Corrections

## Summary

This sub-plan is data only, with the same risk and the same guard as C. No
behavior changes.
