# plan-20-H: Parts — registered transceivers and translators

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-20-G

H adds the `registered` and `translator` devices of
`planning/plan-20-data/parts.tsv` to `buffer.json` and regenerates the
goldens. The translator parts are the first dual-supply parts in the
catalog.

Outcome of H: `grep -c '|buffer|' parts.txt` = Ntri + Nxcvr + Nreg + Ntrans;
goldens grow by Nreg + Ntrans each; every translator line carries a
`|vccb=` field; none removed.

References: plan-20-E §2, plan-20-F §2 (keys, domains).

## Prerequisites

plan-20-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-20-G complete | `ls planning/completed/plan-20-G-*.md` → exists | NOT MET (2026-11-07) |

## 1. Goal

- Goldens + Nreg + Ntrans each; `grep -c 'vccb=' backend/tests/golden/parts.txt` → Ntrans; 0 removed; `ctest` T0 + 1.

## 2. Design

Same generator as G; translator devices carry `domains` and per-family
`vccB`/`inputMaxB` from `specs.md`.

## Phases

### Phase 1 — Parts and goldens

- [ ] Generate the `registered` and `translator` devices; coverage check; regenerate goldens.
- [ ] Spot checks: `SN74ABT646DW`, `SN74LVC543ADW`, `SN74LVC1T45DBVR` (`vccb=1650-5500`), `SN74LVC8T245PW`, `TXU0104PWR` (fixed direction), `CD4504BE` (`C`).
- [ ] Runtime voltage proof: `circuit_check` (or a `voltage_test` case using the real part) with SN74LVC1T45DBVR at VCCA 1.8 V / VCCB 5 V between a 1.8 V LVC1G04 and a 5 V HC04 → no problem.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts (seconds).
Commit: —

## Validation Plan

H's gate: Phase 1. Landing tier at the end of I.

## Decisions

None beyond plan-20-A.

## Corrections

## Summary

H lands the storage transceivers and the first dual-supply parts.
