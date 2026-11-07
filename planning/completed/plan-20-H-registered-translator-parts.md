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
| plan-20-G complete | `ls planning/completed/plan-20-G-*.md` → exists | MET (2026-11-07: `planning/completed/plan-20-G-buffer-parts.md`) |

## 1. Goal

- Goldens + Nreg + Ntrans each; `grep -c 'vccb=' backend/tests/golden/parts.txt` → Ntrans; 0 removed; `ctest` T0 + 1.

## 2. Design

Same generator as G; translator devices carry `domains` and per-family
`vccB`/`inputMaxB` from `specs.md`.

## Phases

### Phase 1 — Parts and goldens

- [x] Generate the `registered` (28 specs → the devices with parts; **Nreg = 155**) and `translator` (**Ntrans = 231**) devices: `land.py buffer registered,translator` → `buffer devices added 49 parts 386`, appending the 12 remaining plan-20-B footprints (`VSSOP-24`, `VQFN-24-RHL`, `UQFN-16-RSV`, `UQFN-10`, `UQFN-12`, `X2QFN-12`, `NFBGA-83`, `X1SON-8`, `X2SON-8-DTM`, `X2SON-6-DTQ`, `X2SON-6-DEA`, `SOT23-THIN-8`; `missing []`). Coverage: `grep -c '|buffer|' parts.txt` → `2135` = 1331 + 418 + 155 + 231 = the `add` rows of the four buffer models. Goldens: `386 0 parts.txt`, `386 0 traces.txt`, `12 0 footprints.txt`; `grep -c 'vccb=' parts.txt` → `231` = Ntrans.
- [x] Spot checks (`parts.txt`): `SN74ABT646DW|buffer|SOIC|Logic|Transceiver|ABT||units=8 … |vcc=4500-5500`; `SN74LVC543ADW|…|Transceiver|LVC||units=8 … |vcc=1650-3600,in=5500`; `SN74LVC1T45DBVR|buffer|SOT-23|Logic|Level shifter|LVC||units=1 … |vcc=1650-5500,in=5500|vccb=1650-5500,in=5500`; `SN74LVC8T245PWR|Level shifter|LVC` (SN74LVC8T245PW is not in the census); `TXU0104QPWRQ1|Level shifter|TXU … |vcc=1080-5500|vccb=1080-5500` (TXU0104PWR not in the census; device `TXU0104` uses the tristate form: fixed direction, OE); `CD4504BE|buffer|DIP|Logic|Level shifter|C||units=6 … |vcc=5000-18000,in=18000|vccb=5000-18000,in=18000`.
- [x] Runtime voltage proof: `voltage_test` `check_real_translator` with the real SN74LVC1T45DBVR (VCCA 1.8 V, VCCB 5 V, DIR at 1.8 V) between SN74LVC1G04DBVR (1.8 V) and SN74HC04N (5 V), plus a 3.6 V-max input on the A net (which a single supply range would push to 5 V) → `PASS: SN74LVC1T45DBVR at 1.8 V / 5 V between a 1.8 V LVC1G04 and a 5 V HC04: no problem`. `ctest` → `100% tests passed, 0 tests failed out of 22`.

Acceptance: `ctest` T0 + 1 of T0 + 1; golden counts (seconds).
Commit: fe1583e

## Validation Plan

H's gate: Phase 1. Landing tier at the end of I.

## Decisions

None beyond plan-20-A.

## Corrections

1. **Ntrans = 231**, not the 232 translator `add` rows plan-20-A first counted: TXV0108RGYR is skipped (no RGY0024 drawing, plan-20-B Corrections 3), so the `TXV0108` device has no part and is not generated.
2. **Spot-check names**: SN74LVC8T245PW and TXU0104PWR are not candidates (not in stock); siblings checked.

## Summary

H lands the storage transceivers and the first dual-supply parts.
