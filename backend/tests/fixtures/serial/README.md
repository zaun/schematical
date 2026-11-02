# Serial fixtures: ns16550 parallel-bus UART devices (plan-12-F and -G)

`devices/<id>.json` holds one device object for each of the 52 `ns16550` UART devices with a parallel host bus (`planning/plan-12-data/devices.tsv`: model `ns16550`, function `UART`). Each object is the device exactly as `schematical/schematical/Parts/serial.json` will ship it, minus `"model"` and the model keys (`bus`, `clock`, `channels`, `fifo`, …), which plan-12-D/E add. It carries the datasheet data only: `id`, `function`, `description`, `drawer`, `note`, every pinout its parts use (pin names consistent across a device's pinouts, no overbar marks, Intel-mode names for mode-selected pins, exposed pads not listed), `grades` (timing in ns, `vcc`, `inputMax`, `note`) and its `parts` (the `add` rows of `planning/plan-12-data/parts.tsv`, all re-checked in stock at DigiKey on 2026-11-01). `footprints.json` holds the 11 new footprints these parts use, copied verbatim from `planning/plan-12-data/footprints.md` (F: TQFP-48-7X7, PLCC-68, LQFP-64-10X10, VQFN-32-5X5, LQFP-80-12X12, PDIP-48-600, BGA-24-MICROSTAR; G: QFN-32-5X5, QFN-48-7X7, QFP-100-14X20, HVQFN-32-5X5); tests load it next to the real `footprints.json`, and F/G later move them into the real file. Group F is 22 devices / 81 parts, group G 30 devices / 69 parts.

| Group | Device | Spec section | Parts | Pinouts (pins) | Grades |
|---|---|---|---|---|---|
| F | 82C50A | `ti-a.md` § 82C50A | 2 | plcc44 (44) | -5 |
| F | COM92C451 | `classic-b.md` § COM92C451 | 1 | dip48 (48) | C |
| F | NS16C2552 | `ti-c.md` § NS16C2552 | 1 | tqfp48-ns (48) | I |
| F | NS16C2752 | `ti-c.md` § NS16C2752 | 2 | tqfp48-ns (48), plcc44 (44) | I |
| F | PC16550D | `ti-a.md` § PC16550D | 2 | plcc44 (44) | C |
| F | TL16C2550 | `ti-c.md` § TL16C2550 | 6 | tqfp48 (48), qfn32 (32) | C, I, Q1 |
| F | TL16C2552 | `ti-c.md` § TL16C2552 | 2 | plcc44 (44) | C |
| F | TL16C450 | `ti-a.md` § TL16C450 | 1 | plcc44 (44) | C |
| F | TL16C451 | `ti-a.md` § TL16C451 | 2 | plcc68 (68) | C |
| F | TL16C452 | `ti-a.md` § TL16C452 | 1 | plcc68 (68) | C |
| F | TL16C550B | `ti-a.md` § TL16C550B | 3 | plcc44 (44), dip40 (40), tqfp48 (48) | C |
| F | TL16C550C | `ti-a.md` § TL16C550C | 11 | plcc44 (44), lqfp48 (48) | C, I |
| F | TL16C550D | `ti-a.md` § TL16C550D | 13 | tqfp48 (48), qfn32 (32), bga24 (24) | C, I |
| F | TL16C552 | `ti-b.md` § TL16C552 | 2 | plcc68 (68) | C |
| F | TL16C552A | `ti-b.md` § TL16C552A | 4 | plcc68 (68), lqfp80 (80) | C |
| F | TL16C554 | `ti-b.md` § TL16C554 | 6 | plcc68 (68), lqfp80 (80) | C, I |
| F | TL16C554A | `ti-b.md` § TL16C554A | 7 | plcc68 (68), lqfp80 (80) | C, I |
| F | TL16C750 | `ti-b.md` § TL16C750 | 1 | lqfp64 (64) | C |
| F | TL16C750E | `ti-b.md` § TL16C750E | 1 | tqfp48 (48) | I |
| F | TL16C752C | `ti-c.md` § TL16C752C | 10 | tqfp48 (48), qfn32 (32) | C, I, Q1 |
| F | TL16C754B | `ti-b.md` § TL16C754B | 1 | plcc68 (68) | I |
| F | TL16C754C | `ti-b.md` § TL16C754C | 2 | lqfp64 (64) | C, I |
| G | SC16C650B | `exar-d.md` § SC16C650B | 1 | hvqfn32 (32) | I |
| G | SC16C752B | `exar-d.md` § SC16C752B | 2 | lqfp48 (48), hvqfn32 (32) | I |
| G | ST16C1450 | `exar-a.md` § ST16C1450 | 1 | tqfp48 (48) | C |
| G | ST16C1550 | `exar-a.md` § ST16C1550 | 1 | tqfp48 (48) | I |
| G | ST16C2550 | `exar-a.md` § ST16C2550 | 3 | tqfp48 (48) | C, I |
| G | ST16C450 | `exar-a.md` § ST16C450 | 2 | tqfp48 (48) | C, I |
| G | ST16C550 | `exar-a.md` § ST16C550 | 6 | plcc44 (44), tqfp48 (48) | C, I |
| G | ST16C554 | `exar-a.md` § ST16C554 | 3 | lqfp64 (64) | C, I |
| G | XR16C2550 | `exar-a.md` § XR16C2550 | 1 | tqfp48 (48) | I |
| G | XR16C854 | `exar-d.md` § XR16C854 | 6 | lqfp64 (64), qfp100 (100) | C, I |
| G | XR16C864 | `exar-d.md` § XR16C864 | 2 | qfp100-864 (100) | C, I |
| G | XR16L2551 | `exar-b.md` § XR16L2551 | 1 | tqfp48 (48) | I |
| G | XR16L2552 | `exar-b.md` § XR16L2552 | 3 | plcc44 (44), tqfp48 (48) | I |
| G | XR16L580 | `exar-a.md` § XR16L580 | 3 | qfn32 (32), tqfp48 (48) | I |
| G | XR16L784 | `exar-d.md` § XR16L784 | 2 | lqfp64-784 (64) | C, I |
| G | XR16M2550 | `exar-b.md` § XR16M2550 | 2 | qfn32 (32), tqfp48 (48) | I |
| G | XR16M2551 | `exar-b.md` § XR16M2551 | 2 | qfn32-2551 (32), tqfp48-2551 (48) | I |
| G | XR16M2650 | `exar-b.md` § XR16M2650 | 2 | qfn32 (32), tqfp48 (48) | I |
| G | XR16M2651 | `exar-b.md` § XR16M2651 | 2 | qfn32-2551 (32), tqfp48-2551 (48) | I |
| G | XR16M752 | `exar-b.md` § XR16M752 | 3 | tqfp48 (48), tqfp48-68 (48) | I |
| G | XR16V2550 | `exar-c.md` § XR16V2550 | 1 | qfn32 (32) | I |
| G | XR16V2551 | `exar-c.md` § XR16V2551 | 2 | qfn32-2551 (32), tqfp48-2551 (48) | I |
| G | XR16V2552 | `exar-c.md` § XR16V2552 | 1 | plcc44 (44) | I |
| G | XR16V2650 | `exar-c.md` § XR16V2650 | 2 | qfn32 (32), tqfp48 (48) | I |
| G | XR16V2651 | `exar-c.md` § XR16V2651 | 2 | qfn32-2551 (32), tqfp48-2651 (48) | I |
| G | XR16V2652 | `exar-c.md` § XR16V2652 | 2 | plcc44 (44), qfn32-2552 (32) | I |
| G | XR16V554 | `exar-d.md` § XR16V554 | 3 | plcc68 (68), qfn48 (48), lqfp64 (64) | I |
| G | XR16V564 | `exar-d.md` § XR16V564 | 4 | lqfp64 (64), plcc68 (68), qfn48 (48) | I |
| G | XR16V654 | `exar-d.md` § XR16V654 | 3 | lqfp64 (64), qfn48 (48) | I |
| G | XR16V798 | `exar-d.md` § XR16V798 | 1 | qfp100 (100) | I |
