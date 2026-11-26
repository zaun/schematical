# plan-74-B: Ethernet IC specs, Microchip: pinouts and facts per (datasheet, package)

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-74-A

B fills `planning/plan-74-data/specs-ic.tsv` for every Microchip row (PHYs and controllers): the
pinout of each (datasheet, package) as a part-file pinout, the facts of § 3.2, and the footprint
the package lands on. Microchip is about two thirds of the IC rows: PHY ≤ 151, controllers 53 at
planning (plan-74-A § 2). C does the other makers with the same rules.

References:

- plan-74-A § 3.5 (`specs-ic.tsv` columns), § Decisions.
- `docs/parts-format.md` § Pinouts and pins (roles, `group`, `side`, `activeLow`), § Footprints,
  § The `"footprint"` part key.
- plan-74-E § 3.1: the `ethernet` device keys these facts feed.
- Precedents: `planning/completed/plan-48-B-timer-opamp-specs.md` (pinout files per
  (datasheet, package), `specs-check.py`, footprint reuse); `Parts/serial.json` device `XR21B1411`
  (an exposed centre pad that is GND is the footprint's unnumbered pad, not a pin).

## Prerequisites

plan-74-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-74-A complete | `ls planning/completed/plan-74-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| The skeleton has Microchip rows | `awk -F'\t' 'NR>1 && $1=="microchip"' planning/plan-74-data/specs-ic.tsv \| wc -l` → above 0 | NOT MET (written by A) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `microchip` row of `specs-ic.tsv` has `pinout` = a file
  `planning/plan-74-data/pinouts/<datasheet>-<package>.json` (one JSON array of pins, the part-file
  pinout format), `type`, `speed`, `host`, `tcpip` (0 for every Microchip part), `vcc` (the I/O
  supply range) and `footprint` (an existing `footprints.json` id, or a new id that D adds).
- Every row the datasheet contradicts DigiKey on (speed, host interface, package) carries the
  datasheet's value, and the difference is in the row's `note`.
- A row whose package has no pin table in its PDF becomes `skip` (`no pinout`) in `verdicts.tsv` and
  `datasheets/skipped-parts.md`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No new footprint drawn here: B names it, and D draws it.
- No pin function beyond the datasheet's default (power-up) function. Strap pins that are also
  outputs take the role of their run-time function.

## 2. Current State

`specs-ic.tsv` has its skeleton from A. Microchip's Ethernet families on the planning census'
saved rows (`raw/eth710.json`'s first 100 of 267, `raw/ethc2.json`'s 74; A's listing gives the
rest): PHYs KSZ8041, KSZ8061, KSZ8081, KSZ8091,
KSZ8721, KSZ9031, KSZ9131, LAN8700, LAN8710A, LAN8720A, LAN8740A, LAN8742A, LAN8830, LAN8840,
VSC8221; controllers ENC28J60 (SSOP / SOIC / SPDIP / QFN), ENC424J600, ENC624J600, KSZ8851 (-16 /
-32 bus, SNL SPI), LAN91C96, LAN91C111, LAN91C113, LAN9115–LAN9118, LAN9210, LAN9211, LAN9215,
LAN9217, LAN9218, LAN9220, LAN9221, LAN9250, and LAN83C185 (a PHY that DigiKey lists under
controllers; its `type` is `phy`).

## 3. Design

### 3.1 Pin roles (from the datasheet's pin-type column)

| Datasheet type | Role |
|---|---|
| I, IS, IPU, IPD, VIS (an input, with or without pull) | `input` |
| O, O8, O12, OD, OD12 (output, open drain) | `output` |
| I/O, IOPU, IOPD, VO8, AIO, A (analog), the MDI pairs (TX±, RX±, TRD±, TXP/TXN, RXP/RXN) | `inout` |
| strap pin (configuration at reset, then an output: LED, PHYAD, MODE pins) | the run-time role: `output` for an LED / data output, `inout` for an I/O |
| P supply the board feeds (VDDIO, VDD33, VDDA3.3, VDD1A2 where an external 1.2 V feeds it) | `power` |
| P supply the chip's own regulator drives (VDDCR, VDD12 with the internal regulator on, VCAP) | `output` |
| GND, VSS, exposed pad with a pin number | `ground` |
| NC, DNC, reserved | `nc` |

Pin names are the datasheet's, overbar as `activeLow` (`docs/parts-format.md` § Active-low pins).
Each signal of a host interface gets `"group"` `mii`, `rmii`, `rgmii`, `spi` or `bus`; MDI pins
`"group": "mdi"` with `"side": "out"`; LED pins `"group": "led"`. An exposed pad the datasheet
does not number is the footprint's unnumbered centre pad, not a pin (the `XR21B1411` precedent).

### 3.2 Facts

- `type`: `phy` or `controller`.
- `speed`: the highest twisted-pair rate in Mbit/s: 10 (ENC28J60), 100, or 1000.
- `host`: the comma list from `mii`, `rmii`, `rgmii`, `gmii`, `sgmii` (PHY), `spi`, `parallel`
  (controller), in the datasheet's order.
- `tcpip`: 1 only for a hardwired TCP/IP stack (WIZnet, in C).
- `vcc`: the I/O supply's recommended operating range in volts (`[min, max]`). Where the datasheet
  gives several I/O voltages (VDDIO 1.8 / 2.5 / 3.3 V), the range spans their extremes.

### 3.3 Footprints

`footprint` is the id of the `footprints.json` land pattern matching the datasheet's package drawing
(body, pitch, lead count, exposed pad size within 0.05 mm; the rules of `docs/parts-format.md`
§ Footprints). Otherwise it is a new id `<STYLE>-<pins>-<maker code>`, the precedent's naming
(`VQFN-24-RGE`, `TQFP-64-MC-64A`): Microchip's package code, `QFN-32-MC-5X5-LJ`. Every new id goes
to `footprints-needed.txt` with its drawing reference (file, page, drawing number).

`census/specs-check.py` (copied from plan-48's `specs-check.py` and adapted) checks every row:
- the pinout file exists, and its pin count equals the package's lead count (unnumbered pads
  excluded);
- every role is one of the six;
- `type`, `speed`, `host` and `vcc` are present;
- `footprint` is in `footprints.json` or `footprints-needed.txt`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Microchip PHYs

- [ ] Pinout files and facts for every Microchip `phy` row (KSZ80xx, KSZ90xx / KSZ91xx, LAN87xx,
      LAN88xx, VSC8221, and the LAN83C185 controller row).
- [ ] `census/specs-check.py`.

Acceptance: every Microchip PHY row passes `specs-check.py`.
  Check: `python3 -I planning/plan-74-data/census/specs-check.py --maker microchip --type phy` → `<n> rows ok, 0 failed` (est. 1 min).
  Result:
Commit:

### Phase 2 — Microchip controllers

- [ ] Pinout files and facts for every Microchip `controller` row (ENC28J60, ENC424J600,
      ENC624J600, KSZ8851, LAN91Cxx, LAN911x, LAN92xx).
- [ ] `footprints-needed.txt`: the Microchip ids not in `footprints.json`.

Acceptance: every Microchip row passes, and every footprint is present or listed.
  Check: `python3 -I planning/plan-74-data/census/specs-check.py --maker microchip` → `<n> rows ok, 0 failed` (est. 1 min).
  Result:
Commit:

## Validation Plan

- `specs-check.py` per phase. Nothing in `backend/` or the app changes.
- Final gate: plan-74-K.

## Decisions

- Author: **strap pins take their run-time role.** The app draws a pin by its role, and the
  datasheet's reset-time sampling is a board fact, not a pin direction.
- Author: **regulator outputs are `output`, not `power`.** The ERC's missing-power-pin rule would
  otherwise demand a supply on a pin the chip drives itself.
- Author: **`vcc` is the I/O supply.** A part has one range; the core supply is either internal or a
  fixed 1.2 V the datasheet names in the note.

## Corrections

## Summary

Microchip's PHY and controller pinouts, facts and footprints, checked by one script. C does the same
for the other four makers.
