# plan-12-G: Parts — Exar and NXP 16550-family devices

Last updated: 2026-11-02
Effort: medium (1h–2h)
Depends on: plan-12-F

G adds the 30 remaining parallel-bus 16550-family devices to `serial.json`:
the 28 MaxLinear/Exar devices of E and NXP's SC16C650B and SC16C752B (D):
69 parts, and the 4 new footprints they are the first to use.

Devices: ST16C450, ST16C1450, ST16C550, ST16C1550, ST16C2550, ST16C554,
XR16C2550, XR16L580, XR16L2551, XR16L2552, XR16M2550, XR16M2551, XR16M2650,
XR16M2651, XR16M752, XR16V2550, XR16V2551, XR16V2552, XR16V2650, XR16V2651,
XR16V2652, XR16V554, XR16V564, XR16V654, XR16C854, XR16C864, XR16L784,
XR16V798, SC16C650B, SC16C752B.

Outcome of G: 69 more part lines in `parts.txt` and `traces.txt`, 4 more in
`footprints.txt`, only added lines; `shipped` still `ok`; `ctest` 100%.

References: as plan-12-F, with `footprints.md` § HVQFN-32-5X5, QFN-32-5X5,
QFN-48-7X7, QFP-100-14X20.

## Prerequisites

plan-12-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-12-F landed | `grep -c '\|serial\|' backend/tests/golden/parts.txt` → 81 (or F's recorded count after its stock re-check) | NOT MET (2026-11-02) |

## 1. Goal

- `grep -c '|serial|' backend/tests/golden/parts.txt` → F's count + 69 (minus
  stock-check drops).
- `footprints.txt` → F's count + 4; `git diff backend/tests/golden | grep -c
  '^-[^-]'` → 0; `serial_test` `shipped` → `ok`.

### Non-goals

As plan-12-F.

## 2. Current State

After F: `serial.json` has 22 devices; the 30 G device objects are fixtures
(D: SC16C650B, SC16C752B; E: the 28 Exar devices).

| What | Count | Command |
|---|---|---|
| G parts | 69 | plan-12-A §3.4 grouping |
| … obsolete | 6 | same rows, `obsolete` |
| New footprints first used here | 4 | §References |

## 3. Design

As plan-12-F §3: stock re-check (`ST16C`, `XR16C`, `XR16L`, `XR16M`,
`XR16V`, `XR68M`, `SC16C6`, `SC16C7`: 8 calls), device objects from the
fixtures plus `parts` from `parts.tsv`, the 4 footprints verbatim, goldens
regenerated. D-variant parts (ST16C554D…, XR16C854D…, XR16V564D…,
XR16V654D…) carry `"intPin": "driven"` (plan-12-E §3.1).

## Phases

> **Status discipline:** as plan-12-A § Phases.

### Phase 1 — Footprints and parts

- [x] Stock re-check; drops in Corrections. Observed (`node
  scripts/digikey.js --in-stock --list`, 2026-11-02): ST16C 34, XR16C 10,
  XR16L 10, XR16M 19, XR16V 21, XR68M 11, SC16C6 4, SC16C7 3. All 69 G part
  names in `parts.tsv` (30 devices) are in these lists: no drops.
- [ ] `footprints.json` + 4; `serial.json` + 30 devices.
- [ ] Regenerate the goldens. Check: no removed line; added `parts.txt` 69,
  `traces.txt` 69, `footprints.txt` 4.
- [ ] Hand-check the 4 new `footprints.txt` lines against `footprints.md`.
- [ ] Spot-check: `XR16V798IQ-F` (`channels=8`, `QFP-100-14X20`),
  `ST16C554DCQ64-F` (the `intPin` override reaches the part: `trace_check
  --verbose` shows INTA driven after reset), `SC16C752BIBS,157`
  (`HVQFN-32-5X5`).

Acceptance: `ctest` 100%; the golden checks (about 2 min).
Commit: —

## Validation Plan

As plan-12-F.

## Decisions

(none beyond plan-12-A, -E)

## Corrections

(none yet)
