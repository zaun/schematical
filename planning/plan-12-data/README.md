# plan-12 data

Research outputs that plan-12 (serial controllers: UARTs, USARTs, ACIAs,
I2C/SPI-to-UART bridges and the SC18IS604 SPI-to-I2C bridge, plus the
pull-up and pull-down parts) builds from. Collected 2026-11-02 from the
datasheets in `datasheets/controller-uart/` and `datasheets/controller-i2c/`
and from DigiKey's in-stock catalog. The design (kind, models, keys) is in the
plan files; this folder holds the per-device facts they cite.

| File | Contents |
|---|---|
| `parts.tsv` | Every DigiKey in-stock part checked (244 part names: DigiKey's 249 in-stock rows on 2026-11-02, five names listed twice under two makers merged, `node scripts/digikey.js --in-stock`): name, pdf, device, maker, pinout, package (repo package family name), package code, pins, footprint id, `footprint_new`, flags, vcc min/max, input max, DigiKey status, `status` (`add` 219 / `leave-out` 25) and the reason. |
| `devices.tsv` | The 79 devices: model (`ns16550`, `ay1013`, `i8251`, `i82510`, `acia6551`, `hs82c52`, `com78c80x`, `max3100`, `max310x`, `sc18is604`), drawer function, the spec file that describes it, part count. |
| `specs/*.md` | One fact sheet per datasheet group, one `## <device>` section per device: parts decoded, every pinout, host interface, full register tables, behavior, timing, VCC, package drawings, notes. `ti-a` (TL16C450/451/452/550B/550C/550D, PC16550D, 82C50A), `ti-b` (TL16C552/552A/554/554A/750/750E/754B/754C), `ti-c` (TL16C2550/2552/752C, NS16C2552/2752; TL16C2752 is left out), `exar-a` (ST16C450/1450/550/1550/2550/554, XR16C2550, XR16L580), `exar-b` (XR16L2551/2552, XR16M2550/2551/2650/2651/752), `exar-c` (XR16V2550/2551/2552/2650/2651/2652), `exar-d` (XR16V554/564/654/798, XR16C854/864, XR16L784, SC16C650B, SC16C752B), `classic-a` (AY-5-1013A, COM2017, COM1863, HD-6402, CDP1854A; COM8017 and M66230 are left out), `classic-b` (8251A, 82510, CDP65C51, 82C52, HD-6406, COM78C804/802, COM78804, COM92C451), `bridges` (SC16IS740/750/752, PI7C9X760/762, XR20M1170/1280, SC18IS604), `maxim` (MAX3100, MAX3140, MAX3107/3108/3109, MAX14830). |
| `matrix.md` | Feature matrix across all devices: Table A (16550-register devices, one row each), Table B (other register models and the bridges' host protocols), §3 the model grouping and the `ns16550` parameter values seen, §4 every datasheet contradiction the spec authors flagged. |
| `decisions.md` | Every contradiction of `matrix.md` §4 resolved (405 entries) by policies P1–P11 or by the planner (last section). The models follow these decisions. |
| `footprints.md` | The 24 new footprints as ready JSON objects (one line each), with drawings, derivations, outside sources (ADI/Maxim, Renesas/Intersil and TI drawings not in the repo PDFs) and the loader probe (24 load, 0 pad overlaps). `SOP-24-24P2W` is not added (M66230 is left out). |

Policies used by `decisions.md` (the models follow them wherever a datasheet
is silent):

- **P1** a register or pin table beats prose; **P2** undefined → X (reads of
  undefined addresses and simultaneous selects on a read drive X; undefined
  writes are ignored; divisor 0 stops that baud generator); **P3** unstated
  reset values come from the PC16550D table, else 0x00, outputs inactive;
  **P4** unstated loopback behavior follows the PC16550D; **P5** clock and
  baud limits are not checked, timing uses the main supply row; **P6** pin
  drawing beats pin table; **P7** errata not in the repo are not modeled;
  **P8** a feature with no register bit or pin is not modeled; **P9** vendor
  conventions per vendor; **P10** documentation slips have no model effect;
  **P11** otherwise the spec author's recommended reading.

Census conventions (`parts.tsv`):

- Only parts DigiKey had in stock on 2026-11-02 (AGENTS.md). DigiKey's status
  governs the `obsolete` flag (the ECS oscillator precedent, ba9856e); Last
  Time Buy, Not For New Designs and "Discontinued at DigiKey" are not flagged.
- A part is left out when its package cannot be determined (no ordering
  decode, no DigiKey package, no unambiguous maker convention) or its
  datasheet has no pinout for its package; when its device cannot be fully
  modeled from the repo datasheets (TL16C2752: extended registers named but
  not described); when its package has no drawing (M66230); or when every
  in-stock part of its PDF is obsolete (ST16C650A).
- Package names follow `docs/parts-format.md` § Parts: one family name, no pin
  count or material (`DIP` for plastic and ceramic; ceramic parts carry the
  `ceramic` flag).
