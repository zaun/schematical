# plan-74-C: Ethernet IC specs, TI, ADI, WIZnet, Renesas; the final footprints-needed list

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-74-B

C fills `specs-ic.tsv` for the rows of Texas Instruments (PHY ≤ 57), Analog Devices (PHY ≤ 25),
WIZnet (controller 4) and Renesas (2, ICS1893 PHYs listed under controllers), with B's rules
(plan-74-B § 3). It then closes `footprints-needed.txt`, the hand-off to D.

References:

- plan-74-B § 3 (roles, facts, footprints, `specs-check.py`); plan-74-A § 3.5.
- The families on the planning census' saved rows (`raw/eth710.json`'s first 100 of 267,
  `raw/ethc2.json`'s 74): TI DP83825I, DP83848 (C / H / I / J / K / M / VYB / YB), DP83869; ADI
  ADIN1200, ADIN1300; WIZnet W5100, W5100S, W5300, W5500; Renesas 1893BFILF, 1893CKILF. A's
  listing gives the rest of TI's and ADI's rows.

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-B complete | `ls planning/completed/plan-74-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25: B not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `ti`, `adi`, `wiznet` and `renesas` row of `specs-ic.tsv` passes `specs-check.py`, as B's
  rows do.
- WIZnet rows have `tcpip` 1, `type` `controller`, and `host` `spi` (W5500), `spi,parallel`
  (W5100, W5100S) or `parallel` (W5300), as each datasheet's host-interface section says.
- `footprints-needed.txt` is final. It lists every id that some `specs-ic.tsv` row names and
  `footprints.json` lacks, each with its drawing reference (PDF, page, drawing number).

### Non-goals (explicit constraints)

- As plan-74-B § Non-goals.
- ADI's single-pair parts (ADIN1100, ADIN1110) and TI's DP83TC/TD/TG parts are already `out`
  (plan-74-A § 3.2). C does not re-open them.

## 2. Current State

B has written `specs-check.py` and the Microchip rows; `footprints-needed.txt` holds Microchip's new
ids. TI datasheets name packages by TI code (`RHB` VQFN-32, `RGZ` VQFN-48, `PT` LQFP-48, `RTA`
WQFN-40), and `footprints.json` already names several TI-coded footprints (`VQFN-24-RGE`,
`VQFN-32-RHM`, `LQFP-48-PT`, … from `grep -o '"id": "[A-Z]*QF[NP]-[0-9]*-R[A-Z]*"'
schematical/schematical/Parts/footprints.json`). ADI uses its `CP-40-` / `CP-32-` LFCSP codes.

## 3. Design

B's § 3 unchanged. In addition:

- **WIZnet**: `MODE` / `PMODE` pins are `input`; the PHY LEDs (`LINKLED`, `SPDLED`, `ACTLED`, …)
  are `output` with `"group": "led"`; `TXP`/`TXN`/`RXP`/`RXN` are `inout` `mdi`. The W5100's bus
  and SPI pins share pins: the pin's name is the datasheet's (`/SCS`, `SCLK`, `MOSI`, …) with the
  bus function where the datasheet names both.
- **Renesas 1893**: a PHY (`type` `phy`, `host` `mii,rmii`), though DigiKey lists it under
  Controllers.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — TI and ADI PHYs

- [ ] Pinout files and facts for every `ti` and `adi` row.

Acceptance: those rows pass.
  Check: `python3 -I planning/plan-74-data/census/specs-check.py --maker ti && python3 -I planning/plan-74-data/census/specs-check.py --maker adi` → `<n> rows ok, 0 failed` twice (est. 1 min).
  Result:
Commit:

### Phase 2 — WIZnet, Renesas, the footprint list

- [ ] Pinout files and facts for the `wiznet` and `renesas` rows per § 3.
- [ ] `footprints-needed.txt` final: `census/specs-check.py --needed` rewrites it from the rows and
      `footprints.json`.

Acceptance: every row of `specs-ic.tsv` passes, and the needed list is exactly the missing ids.
  Check: `python3 -I planning/plan-74-data/census/specs-check.py` → `<n> rows ok, 0 failed`; `python3 -I planning/plan-74-data/census/specs-check.py --needed --check` → `footprints-needed.txt up to date: <k> ids` (est. 1 min).
  Result:
Commit:

## Validation Plan

- `specs-check.py` per phase. Final gate: plan-74-K.

## Decisions

- Author: **WIZnet parts are `controller` with `tcpip` 1.** They are MAC+PHY parts with a host bus,
  as the owner scoped them. The stack is a fact, not a separate type.

## Corrections

## Summary

The remaining four makers' pinouts and facts, and the final list of footprints D draws.
