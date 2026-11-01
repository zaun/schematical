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
| plan-07-C complete | `ls planning/plan-07-C-* 2>/dev/null` → no output | MET (2026-11-01: `ls planning/plan-07-C-*` → no matches; archived in e745682) |

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

- [x] Run plan-07-C §4 on the 12 OR and 14 NOR devices (80 entries).
- [x] Regenerate `parts.txt`.

Acceptance: every OR and NOR part has a range, and no other field changed.
  Check: the plan-07-C Phase 1 script with `in ('OR','NOR')` → `0`;
  `git diff --quiet backend/tests/golden/traces.txt` → exit 0;
  `git diff -U0 backend/tests/golden/parts.txt | grep '^+[^+]' | grep -vc '|vcc='`
  → `0` (est. 1 min).
  Result (2026-11-01): script → `0`; traces `git diff --quiet` → exit 0
  (`trace_check` exit 0); `… | grep -vc '|vcc='` → `0`, and the 855 changed
  lines differ only by `|vcc=` (`855 855 True`). `verify.py` → `434 parts
  checked, 0 mismatches` (OR), `421 parts checked, 0 mismatches` (NOR).
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

- **Run the same way as plan-07-C** (see its Corrections): research agents
  per batch (OR; NOR; XOR + XNOR together with the JK flip-flops) wrote
  `/tmp/p07/data/{or,nor,xor-jk}.json`; `/tmp/p07s/apply.py` applied them;
  `/tmp/p07s/verify.py` compared every listed value with the readings.
- **The OR batch wrote `inputMax` 5.5 on the LVC 1G/2G entries**, where VI
  max equals VCC max. The apply step drops an `inputMax` that is not above
  the `vcc` max (plan-07-C Decisions), so those entries carry no
  `inputMax`, like the other batches. The listed value is the same
  (`in=` = the `vcc` max either way), e.g. SN74LVC1G32DBVR
  `vcc=1650-5500,in=5500`.
- **74_02 LVC is a 13–13 tie** (SN74LVC02A 1.65–3.6 vs SN54/5962/-Q1/-EP
  2–3.6): the entry takes the SN74 range (plan-07-C tie rule).
- **CD4070B/4077B** (`cd4070b` p.4) print only "Supply Voltage Range
  (Typical) 3V to 18V", used as [3, 18].

## Summary

This sub-plan is data only, with the same risk and the same guard as C. No
behavior changes.
