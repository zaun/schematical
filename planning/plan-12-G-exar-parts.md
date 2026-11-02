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
- [x] `footprints.json` + 4; `serial.json` + 30 devices. The 4 objects
  verbatim from `footprints.md`; `python3 -I backend/tests/fixtures/serial/
  merge.py --ship F,G` → `serial.json: 52 devices, 150 parts`.
- [x] Regenerate the goldens. Check: no removed line; added `parts.txt` 69,
  `traces.txt` 69, `footprints.txt` 4. Observed: removed 0 / added 69, 69, 4.
- [x] Hand-check the 4 new `footprints.txt` lines against `footprints.md`.
  QFN-32-5X5: pads at ±2.400 (span 4.8), first −1.750 (pitch 0.5),
  0.625×0.275, centre 3.8; QFN-48-7X7: ±3.450 (6.9), first −2.750,
  0.700×0.290, centre 5.2; QFP-100-14X20: x ±8.610 (17.22), y ±11.610
  (23.22), sides 30/20 (first −9.425 and −6.175 at pitch 0.65),
  1.680×0.360, body 14×20; HVQFN-32-5X5: ±2.450 (4.9), 0.700×0.290,
  centre 3.1. All as the JSON.
- [x] Spot-check: `XR16V798IQ-F` (`channels=8`, `QFP-100-14X20`),
  `ST16C554DCQ64-F` (the `intPin` override reaches the part: `trace_check
  --verbose` shows INTA driven after reset), `SC16C752BIBS,157`
  (`HVQFN-32-5X5`). Observed: `XR16V798IQ-F|serial|QFP|…|model=ns16550
  channels=8 grade=I|…|footprint=QFP-100-14X20`; `SC16C752BIBS,157|…
  |channels=2 grade=I|…|footprint=HVQFN-32-5X5`; INTA (pin 6):
  ST16C554DCQ64-F `0 pin 6 -> 0`, `2333203 pin 6 -> 1`, …, while
  ST16C554CQ64-F never drives it (3-state, MCR[3] = 0).

Acceptance: `ctest` 100%; the golden checks (about 2 min). Observed:
`100% tests passed, 0 tests failed out of 19`; `serial_test` → `PASS`
(`compared: 52 ns16550`).
Commit: c74f63d

## Validation Plan

As plan-12-F.

## Decisions

(none beyond plan-12-A, -E)

## Corrections

- **SC16C650B and SC16C752B `inputMax`.** The data had a grade `inputMax` of
  5.3 V, below the 5.5 V supply maximum; the loader refuses an input limit
  under the supply range (`docs/parts-format.md` § Supply voltage), so the
  key was removed and the input limit is the supply's, the loader's default
  (15723fb).
- **Shipped after E, H and I merged** (all four groups' models were on the
  branch); the golden order is still F → G → H → I → J.
