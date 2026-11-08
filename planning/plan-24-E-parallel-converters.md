# plan-24-E: Parallel-bus ADCs and DACs

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-24-D

E adds the `converter` `bus` form `"parallel"`: a microprocessor-style data
bus with chip select, read/write strobes, address/channel pins,
conversion-start and BUSY/EOC, and byte or word access, so the parallel ADCs
(197 in scope + 49 parallel + SPI) and DACs (182 + 1) run.

Outcome of E: `converter_test` passes the §3 cases; `ctest` T0 + 1; goldens unchanged.

References: plan-24-D §3 (schema, engine); `backend/src/models/pio.c`
(a 6502-bus peripheral: CS/RW/data bus handling), `sram.c` (bus read
timing), `docs/parts-format.md` § `pio`.

## Prerequisites

plan-24-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-24-D complete | `ls planning/completed/plan-24-D-*.md` → exists | MET (`planning/completed/plan-24-D-converter-kind.md`, 2026-11-07 at e94a309) |

## 1. Goal

- `converter_test` PASS for every §3 case; `ctest` T0 + 1 of T0 + 1; golden diff empty.

## 2. Design

`"bus": {"parallel": {...}}`:

| Key | Value |
|---|---|
| `"data"` | 8–16 pin names (`PIN_INOUT` for ADCs with registers, outputs for read-only ADCs, inputs for DACs) |
| `"cs"`, `"rd"`, `"wr"` | controls `{"pin","active"}` (any may be absent: ADCs with only CS + RD, DACs with CS + WR) |
| `"address"` | optional pin names: register / channel / byte select (A0 high-byte/low-byte on 8-bit buses) |
| `"convst"` | optional `{"pin","edge"}`: conversion start |
| `"byteOrder"` | for word results on an 8-bit bus: `"high-first"` / `"low-first"` / a `hben` pin |
| `"access"` | `{"tACC": ns, "tDF": ns}`: data valid after RD asserts, float after it releases |

ADC: CONVST edge (or the CS·WR start the datasheet uses) starts the
conversion; BUSY/EOC per `ready`; RD with CS drives the §3.3 code on `data`
after `tACC`, Z `tDF` after release. DAC: CS·WR latches `data` into the input
register selected by `address`; LDAC/`update` as in D.

## 3. Tests (in `converter_test.c`)

| Fixture | Cases |
|---|---|
| ADS7816-style parallel ADC (CONVST, BUSY, 12-bit word) | conversion, BUSY timing, read 0, data Z when not selected |
| 8-bit-bus 12-bit ADC with HBEN | two reads give the high and low byte in the configured order |
| parallel DAC with double buffering (DAC7625-style) | write input registers by address, LDAC copies all |
| bus contention | RD while not CS → data stays Z |

## Phases

### Phase 1 — Parallel bus

- [x] Parallel front end in `converter_par.c` (CMake line landed with D): CS/RD/WR or R/W reads and writes, address pins, ALE latch, byte select, tAcc/tDf, latch-on-change DACs; the loader's parallel object and docs § `converter` (bus) landed with D. §3 cases in `converter_test`: ADS7816-style CONVST/BUSY/read 0x800/float, HBEN high/low bytes, DAC7625-style writes by address + R/W read-back + LDAC copies both, RD without CS stays Z — all `ok`; `library_check`: `"data" needs 4-24 pins`.
- [x] `clocked` bus style (schema.md § 3, plan-24-A Corrections): data pins drive the next `read`-pattern bits on each clock edge while `oe` is active, `clockOut` follows the clock; `configBus` (SPI/I2C configuration port beside a parallel or clocked bus). Cases: a 12-bit clocked ADC drives the 0 V code after a clock edge and Z with OE inactive; a 1-pin modulator alternates 1/0 per clock; registers written over `configBus` read back. — all `ok` (`clocked ADC: a CLK rising edge drives the 0 V code (0x800)`, `OE inactive floats the data`, `DRY follows CLK`, `modulator: ... alternates 1, 0`, `CLKOUT = CLK / 2`, `config bus: a register written over SPI reads back` on 3-wire SDIO); `library_check`: `"clock" needs a pin`.

Acceptance: `ctest --test-dir /tmp/logic-p24` → T0 + 1 of T0 + 1 (`100% tests passed, 0 tests failed out of 25`); golden diff empty (0 lines) (seconds). All 764 ADC/DAC specs still load (`loaded 1911 parts`).
Commit: —

## Validation Plan

E's gate: Phase 1. Landing tier at the end of I.

## Decisions

- Parallel parts run on the same register/conversion engine as D; only the
  front end differs.

## Corrections

- Most census "parallel" ADCs are free-running pipelined converters (data
  out every CLK, no RD/CS) or have an SPI configuration port; schema.md adds
  the `clocked` style and `configBus` for them, and E implements both (task
  added, plan-24-A Corrections).

## Summary

E makes the parallel converters usable straight on a 6502 or Z80 bus.
