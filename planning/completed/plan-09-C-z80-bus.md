# plan-09-C: The Z80 bus adapter

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-09-B

This sub-plan adds `"core": "z80"` to the `cpu` kind (plan-09-B). It also
adds the adapter that runs the vendored `z80.h` core (plan-09-A) from the
CLK pin. The adapter drives the Z84C00's machine cycles at the times in the
PS0178 AC table:

- opcode fetch with refresh;
- memory read/write;
- I/O read/write with the automatic wait state;
- interrupt acknowledge in IM 0/1/2;
- NMI, HALT, WAIT, BUSREQ/BUSACK and RESET.

Outcome of C: `cpu_test` passes every Z80 case of §5. A fixture Z84C00 at
1 MHz with a bench memory leaves reset and runs:

```
LD A,12h / LD BC,3456h / LD (8000h),A / OUT (10h),A / HALT
```

- The bench logs the memory write of `12` at `8000` with MREQ#/WR# at their
  datasheet edges.
- It logs an I/O write of `12` to port `1210` (A on A8–A15) with IORQ#/WR#
  and one automatic wait state.
- HALT# goes low.
- `circuit_cpu_state` reports BC `3456`, status Halted, last instruction
  `HALT`.

References:

- plan-09-A (cores; Z80 expected failures §4.4), plan-09-B (§4.1–§4.6: kind,
  state API, the lost/differential rules this sub-plan reuses).
- `datasheets/cpu/ps0178.pdf` (pins PDF p.1–2; AC printed p.35–36),
  `datasheets/cpu/um0080.pdf` (timing chapter from printed p.7: fetch,
  memory, I/O, int-ack, NMI, BUSREQ, HALT, RESET diagrams).
- `backend/src/third_party/chips/z80.h` (step table; `_wait()`, `_fetch`,
  `z80_reset`, `z80_prefetch`).

## Prerequisites

plan-09-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-09-B complete | `ls planning/plan-09-B-* 2>/dev/null` → no output; `ls planning/completed/plan-09-B-*.md` → exists | MET (2026-11-01 at 71321d2; plan-09-A's rows re-run at 1da038c the same day, all MET) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of *all*
> prerequisites.**

## 1. Goal

- `cpu_test` passes every case of §5.1, alongside B's cases.
- `library_check` passes, with the Z80 rejection cases of §5.2.
- Goldens unchanged (no part ships until D). `ctest` → T0 + 2 tests, all
  passing.

### Non-goals (explicit constraints)

- The vendored core is not edited. Its known differences stay, are listed in
  `docs/parts-format.md` § `cpu`, and are pinned by `cpu_core_test`:
  - **During HALT the bus shows the HALT opcode's own address** in the
    halted M1 cycles (the real chip shows the next address). PC is advanced
    when the interrupt is taken, so execution resumes correctly.
  - The undocumented flag bits 3/5 after SCF/CCF (Q register), and the
    flags of an interrupted repeating block instruction.
- Not modeled:
  - the read-data hold time;
  - the 4 MHz Z84C0004 grade (obsolete, replaced by 6 MHz per PS0178);
  - Z80 peripheral daisy-chain signals (IEI/IEO are not CPU pins).
- No 6502-family behaviour change (`cpu_test` §5.1–5.3 of B must still
  pass).

## 2. Current State

- plan-09-B gives:
  - `CPU_KIND`, with `"core"` limited to `6502`/`65c02`;
  - `CpuCoreOps` dispatch;
  - `CpuState` and `circuit_cpu_state`;
  - the instruction tracker;
  - the lost/differential rules (plan-09-B §4.4), written for a
    one-tick-per-cycle core.
- `z80.h` contract (read in the header and measured by the plan-09-A
  probes):
  - `z80_tick` advances one T-state.
  - A request appears in the returned pins on a fixed tick of its machine
    cycle (§ Verified).
  - The tick after a read request consumes the data bits of its input
    pins.
  - `_wait()` re-runs the same step while WAIT is set.
  - `z80_reset()` / `z80_prefetch(0)` start at PC 0.
  - There is no RES, BUSREQ or BUSACK pin (`Z80_PIN_RES` exists, but the
    header says "currently not emulated").
  - The HALT pin bit is set in returned pins while halted.
  - NMI is edge-detected against the stored pins.

### Measured populations

| What | Count | Command / source |
|---|---|---|
| Z80 bus transactions in the 1604-test subset, by type | fetch 2956, memory read 2028, memory write 583, I/O read 15, I/O write 15 | `/tmp/p08/run/sstz80d.c` (`D=1`), 2026-11-01 |
| Core request tick vs SingleStepTests T-state, by type | fetch −1 (all 2956), mem read 0 (2028), mem write 0 (583), I/O read 0 (15), I/O write −1 (15) | same run, `sort \| uniq -c` |
| Z80 timing parameters used | 37 (§4.2) | PS0178 printed p.35–36 rows 1–53 minus the 4 MHz-only notes |

### Verified properties

- **Where each request sits in its machine cycle.** SingleStepTests flags
  fetch, memory read and memory write on T2, and I/O read/write on TW* (the
  third T-state). Their `cycles` lists for `32`, `D3` and `DB` show this.
  Combined with the offsets above, the core issues its requests on:
  - fetch on **T1**;
  - memory read and write on **T2**;
  - I/O write on **T2**;
  - I/O read on **TW\***.

  This holds for every opcode in the subset, with no exceptions.
- **Where the core checks WAIT** (`z80.h` step table, e.g. lines 1584,
  1590, 1991–1995, 2041–2046, 2796):
  - fetch: on the tick after the request (T2);
  - memory: on the request tick (T2);
  - I/O write: on the tick after the request (TW\*);
  - I/O read: on the request tick (TW\*).

  These are the T-states in which the Z80 samples WAIT (PS0178 row 17:
  WAIT setup to clock fall). Only the edge differs: the core ticks once per
  T-state, while the chip samples at the falling edge.
- **Interrupt acknowledge and NMI** (probe `/tmp/p08/probe/z80int.c`,
  2026-11-01):
  - IM 2 acknowledge: the `_fetch` tick (T1) returns no request. The next
    tick (T2) is idle. The third (TW\*1) returns M1|IORQ. The fourth (TW\*2)
    checks WAIT and consumes the vector. The fifth (T3) refreshes. The sixth
    is T4. Pushes and the vector reads follow as memory cycles.
  - During the acknowledge, the core leaves the last refresh address on the
    bus, not PC.
  - NMI: a normal M1 fetch at PC (data ignored), 5 T-states. Then two
    pushes and a fetch at `0066`.
  - HALT: the HALT pin bit first appears on the T1 tick of the first
    halted M1, one T-state later than the chip, which drives HALT low after
    the falling edge of T4 (PS0178 row 36).

## 3. Design Overview

The Z80 adapter is a **commit scheduler** around the core:

1. **Classify.** At each CLK↑ the adapter knows the committed core state.
   It peeks ahead on a copy of the core (up to 3 ticks, with this
   T-state's samples and WAIT inactive) to find the next bus request and
   its tick offset. The offset and the request type identify this ↑ as
   the T1 of a machine cycle of a given kind, or as an internal T-state
   (§4.3). The address (and
   write data) the chip must drive at T1↑ comes from the peek. This is
   needed because the core issues memory, I/O and int-ack requests one or
   two T-states after T1.
2. **Commit** each core tick at the clock edge where the chip samples that
   tick's inputs (§4.3 table). Ticks with no input commit at their
   T-state's ↑.
3. **Drive** each machine cycle's waveform from the datasheet edges
   (§4.4), independently of where the core put its request.
4. **Extras the core lacks:** RESET pin, BUSREQ/BUSACK, PC on the bus
   during the int-ack, and HALT# at T4↓ (via a one-tick peek).

Correctness risk concentrates in the scheduler: a tick committed at the
wrong edge shifts every later cycle. Each machine-cycle type has a
`cpu_test` case that checks every edge it drives, and the program case
(§5.1-15) checks the whole sequence.

Rejected alternatives:

- **One commit per CLK↑, outputs straight from the core.** This reproduces
  the core's "simplified" bus: requests one or two T-states late and RD for
  one T-state. Memories wired to MREQ#/RD# would see wrong windows. Rejected
  because the datasheet waveform is the point of a pin-level part.
- **Editing `z80.h` to move requests to T1.** It is vendored unmodified
  (plan-09-A Non-goals). The peek gives the same information.
- **Hard-coding per-opcode T-state tables.** The peek reads the core's own
  schedule, so it covers all 1604 opcode forms with five rules.

## 4. Detailed Design

### 4.1 Part file keys (`"core": "z80"`)

As plan-09-B §4.1, with these control keys in place of the 6502 ones (each
names one pin, all required for `z80`, and each rejected on other cores):

- outputs: `"m1"`, `"mreq"`, `"iorq"`, `"rd"`, `"wr"`, `"rfsh"`, `"halt"`,
  `"busack"` (all active low);
- inputs: `"wait"`, `"int"`, `"nmi"`, `"reset"`, `"busreq"` (active low).

`"clock"` is CLK; `"address"` has 16 names and `"data"` has 8. Grades use the
timing names of §4.2, plus `"vcc"`, `"inputMax"` and `"note"`. `tcc`, `twch`
and `twcl` must be > 0.

Registers in `CpuState` (16 bits unless noted), in order:
`AF BC DE HL AF' BC' DE' HL' IX IY SP PC`, `I` (8), `R` (8), `WZ`, `IM` (2),
`IFF1` (1), `IFF2` (1). `PC` is the core's PC (the next byte to fetch).

### 4.2 Timing names (ns, PS0178 printed p.35–36; row number in brackets)

| Name | Meaning | 6 | 8 | 10 | 20 |
|---|---|---|---|---|---|
| `tcc` [1] | clock period min | 162 | 125 | 100 | 50 |
| `twch` [2] / `twcl` [3] | clock high / low min | 65 | 55 | 40 | 20 |
| `tdcra` [6] | CLK↑ → address valid | 90 | 80 | 65 | 57 |
| `tdcfmreqf` [8] | CLK↓ → MREQ↓ | 70 | 60 | 55 | 40 |
| `tdcrmreqr` [9] | CLK↑ → MREQ↑ | 70 | 60 | 55 | 40 |
| `tdcfmreqr` [12] | CLK↓ → MREQ↑ | 70 | 60 | 55 | 40 |
| `tdcfrdf` [13] | CLK↓ → RD↓ | 80 | 70 | 65 | 40 |
| `tdcrrdr` [14] | CLK↑ → RD↑ | 70 | 60 | 55 | 40 |
| `tsdcr` [15] | data setup → CLK↑ (M1) | 30 | 30 | 25 | 12 |
| `tswaitcf` [17] | WAIT setup → CLK↓ | 60 | 50 | 20 | 7.5 |
| `tdcrm1f` [19] / `tdcrm1r` [20] | CLK↑ → M1↓ / M1↑ | 80 | 70 | 65 | 45 |
| `tdcrrfshf` [21] | CLK↑ → RFSH↓ | 110 | 95 | 80 | 60 |
| `tdcrrfshr` [22] | CLK↑ → RFSH↑ | 100 | 85 | 80 | 60 |
| `tdcfrdr` [23] | CLK↓ → RD↑ | 70 | 60 | 55 | 40 |
| `tdcrrdf` [24] | CLK↑ → RD↓ (I/O) | 70 | 60 | 55 | 40 |
| `tsdcf` [25] | data setup → CLK↓ (M2–M5) | 40 | 30 | 25 | 12 |
| `tdcriorqf` [27] | CLK↑ → IORQ↓ | 65 | 55 | 50 | 40 |
| `tdcfiorqr` [28] | CLK↓ → IORQ↑ | 70 | 60 | 55 | 40 |
| `tdcfwrf` [30] | CLK↓ → WR↓ | 70 | 60 | 55 | 40 |
| `tdcfwrr` [32] | CLK↓ → WR↑ | 70 | 60 | 55 | 40 |
| `tdcrwrf` [34] | CLK↑ → WR↓ (I/O) | 60 | 60 | 50 | 40 |
| `tdcfhalt` [36] | CLK↓ → HALT | 260 | 225 | 90 | 70 |
| `twnmi` [37] | NMI pulse min | 60 | 60 | 60 | 60 |
| `tsbusreq` [38] | BUSREQ setup → CLK↑ | 50 | 40 | 30 | 15 |
| `tdcrbusackf` [40] | CLK↑ → BUSACK↓ | 90 | 80 | 75 | 40 |
| `tdcfbusackr` [41] | CLK↓ → BUSACK↑ | 90 | 80 | 75 | 40 |
| `tdcrdz` [42] | CLK↑ → data float | 80 | 70 | 65 | 40 |
| `tdcrctz` [43] | CLK↑ → MREQ/IORQ/RD/WR float | 70 | 60 | 65 | 40 |
| `tdcraz` [44] | CLK↑ → address float | 80 | 70 | 75 | 40 |
| `tsreset` [46] | RESET setup → CLK↑ | 60 | 45 | 40 | 15 |
| `tsint` [48] | INT setup → CLK↑ | 70 | 55 | 50 | 15 |
| `tdcfiorqf` [51] | CLK↓ → IORQ↓ (int-ack) | 70 | 60 | 55 | 45 |
| `tdcfd` [53] | CLK↓ → write data valid | 130 | 115 | 110 | 75 |

Grade keys `"6"`, `"8"`, `"10"`, `"20"`. The holds (rows 16, 18, 39, 47, 49)
are not checked (Non-goals). The table's footnote values for slower clocks
are not used: each grade uses its minimum-period values.

### 4.3 The commit scheduler

Samples:

- INT, NMI, BUSREQ and RESET at every CLK↑;
- WAIT at every CLK↓;
- data at the cycle's data edge (below).

A pin that changed within its setup time before the edge counts as X. An
NMI low shorter than `twnmi` counts as X. Z reads inactive (high).

**MC start (at each CLK↑, when no machine cycle is in progress).** Peek on a
copy, inputs = this ↑'s samples, WAIT inactive. The first request found
decides the MC:

| Request in the peek | Its offset k | This ↑ is T1 of |
|---|---|---|
| M1\|MREQ\|RD | 0 | opcode fetch (also the NMI's fetch) |
| MREQ\|RD, MREQ\|WR | 1 | memory read / write |
| IORQ\|WR | 1 | I/O write |
| IORQ\|RD | 2 | I/O read |
| M1\|IORQ | 2 | interrupt acknowledge |

When the request appears at a larger k than its type's, this ↑ is an
internal T-state: commit one tick, drive nothing.

**Commit edges within an MC.** Each row lists the MC's core ticks in order.
"Hold to X" means the tick is committed at edge X with that edge's samples.
"Retry" means the core re-ran the step because WAIT was set, which makes
that T-state a Tw, so the same tick is committed again at the next such
edge. "+" means two ticks commit at the same instant, in order.

| MC | Commits |
|---|---|
| Opcode fetch (4 T) | T1↑: tick 1 (request). Tick 2 (WAIT, data) held to T3↑, with WAIT from the last ↓ and data sampled at this ↑ (setup `tsdcr`); retry at each next ↑ while WAIT. + tick 3 (refresh). T4↑: tick 4. |
| Memory read (3 T) | T1↑: tick 1. T2↓: tick 2 (WAIT, request); retry at each ↓. T3↓: tick 3 (data, setup `tsdcf`). |
| Memory write (3 T) | T1↑: tick 1. T2↓: tick 2 (WAIT, request); retry at each ↓. T3↑: tick 3. |
| I/O write (4 T) | T1↑: tick 1. T2↑: tick 2 (request). TW\*↓: tick 3 (WAIT); retry. T3↑: tick 4. |
| I/O read (4 T) | T1↑: tick 1. T2↑: tick 2. TW\*↓: tick 3 (WAIT, request); retry. T3↓: tick 4 (data, `tsdcf`). |
| Int acknowledge (6 T) | T1↑: tick 1. T2↑: tick 2. TW\*1↑: tick 3 (M1\|IORQ). Tick 4 (WAIT, vector) held to T3↑ (WAIT from TW\*2↓, data at T3↑, `tsdcr`); retry. + tick 5 (refresh). T4↑: tick 6. |

Every commit with an X input uses plan-09-B's differential rule. The masked
stored-pin bits are D0–D7, INT and WAIT; NMI stays compared, because its
edge state matters.

**HALT#.** At the T4↓ of every fetch, peek one tick. If its pins carry HALT
and HALT# is high, drive HALT# low at + `tdcfhalt`. When a commit returns
pins without HALT while HALT# is low, drive it high at the next ↓ +
`tdcfhalt`.

**BUSREQ.** Sampled at the ↑ of the last T-state of each MC: the ↑ whose
next ↑ is a T1, known from the peek. If it is 0, at the next ↑:
- no MC starts; status BUS_RELEASED;
- BUSACK# low at + `tdcrbusackf`;
- address Z at + `tdcraz`;
- data Z at + `tdcrdz`;
- MREQ#, IORQ#, RD# and WR# Z at + `tdcrctz`;
- no ticks and no refresh.

Each ↑ re-samples BUSREQ. At the first 1, BUSACK# goes high at the next ↓ +
`tdcfbusackr`, and the following ↑ starts the pending MC (outputs driven
again). BUSREQ = X → lost.

**RESET.** Sampled at every ↑.
- 0: status RESET. Count the ↑. Drive the address and data Z, and M1#,
  MREQ#, IORQ#, RD#, WR#, RFSH#, HALT# and BUSACK# high.
- 1 after at least 3 counted: `z80_reset`; status RUNNING; cycles 0. That
  ↑ is T1 of the fetch at `0000`.
- 1 after fewer than 3: lost.
- X: lost.

**Clock.** High < `twch`, low < `twcl`, or ↑-to-↑ < `tcc` → lost.
CLK = X → lost.

**Lost.** Drive the address, data and every control output X (strong).
Only RESET leaves it. Status at start: LOST.

### 4.4 Waveforms

Transport drives. "A ← v" means: X at the edge, then v at + `tdcra`.

- **Opcode fetch:**
  - T1↑: A ← PC; M1# low (+`tdcrm1f`); RFSH# high (+`tdcrrfshr`) if it
    was low.
  - T1↓: MREQ# low (+`tdcfmreqf`); RD# low (+`tdcfrdf`).
  - T3↑ (after ticks 2–3): MREQ# high (+`tdcrmreqr`); RD# high
    (+`tdcrrdr`); M1# high (+`tdcrm1r`); A ← the refresh address (I, R);
    RFSH# low (+`tdcrrfshf`).
  - T3↓: MREQ# low (+`tdcfmreqf`).
  - T4↓: MREQ# high (+`tdcfmreqr`).
- **Memory read:**
  - T1↑: A ← address.
  - T1↓: MREQ# low, RD# low.
  - T3↓: MREQ# high (+`tdcfmreqr`); RD# high (+`tdcfrdr`).
- **Memory write:**
  - T1↑: A ← address.
  - T1↓: MREQ# low; D ← data (X, then data at +`tdcfd`).
  - T2↓: WR# low (+`tdcfwrf`).
  - T3↓: MREQ# high; WR# high (+`tdcfwrr`).
  - The next T1↑: D Z (+`tdcrdz`).
- **I/O read:**
  - T1↑: A ← port (the core's full 16-bit address).
  - T2↑: IORQ# low (+`tdcriorqf`); RD# low (+`tdcrrdf`).
  - T3↓: IORQ# high (+`tdcfiorqr`); RD# high (+`tdcfrdr`).
- **I/O write:**
  - T1↑: A ← port.
  - T1↓: D ← data (+`tdcfd`).
  - T2↑: IORQ# low; WR# low (+`tdcrwrf`).
  - T3↓: IORQ# high; WR# high (+`tdcfwrr`).
  - The next T1↑: D Z.
- **Interrupt acknowledge:**
  - T1↑: A ← PC (the core's `pc`; the adapter's fix, § Verified); M1# low.
  - TW\*1↓: IORQ# low (+`tdcfiorqf`).
  - T3↑: M1# high and IORQ# high (+`tdcrm1r`); then the refresh as in the
    fetch.

### 4.5 Status and tracker

- Status:
  - HALTED while the core's pins carry HALT;
  - BUS_RELEASED and RESET as above;
  - otherwise RUNNING.
- The instruction tracker (plan-09-B §4.6) starts an instruction at each
  fetch MC while the core's `prefix_active` is false. It records the bytes
  of the fetch and memory reads at start + n, and disassembles with
  `z80dasm_op`.

## 5. Tests

### 5.1 `cpu_test` Z80 cases

Fixture: the D device `Z84C00` (PDIP-40 pinout, grades 6/8/10/20, plan-09-D
§4.2), part grade `10`, clock 1 MHz (50 %).

The bench:
- serves memory on MREQ#+RD# (drives from RD#↓ + 100 ns until RD#↑ +
  20 ns);
- stores on MREQ#+WR# at WR#↑;
- serves I/O the same way on IORQ#;
- answers an int-ack (M1#+IORQ#) with a set vector byte;
- logs every MC as (type, address, data, T-states).

Reset is RESET# low for 4 clocks.

1. **Power-up lost.** Outputs X; status LOST.
2. **Reset.** After release, the first MC is a fetch at `0000`; status
   RUNNING.
3. **Short reset.** 2 clocks → LOST.
4. **Fetch waveform.** The first fetch:
   - A valid at T1↑ + `tdcra`; M1# low at T1↑ + `tdcrm1f`;
   - MREQ# and RD# low at T1↓ + their delays;
   - at T3↑: RD#, MREQ# and M1# high; A = `00`:R (I = 0); RFSH# low at
     `tdcrrfshf`;
   - MREQ# low at T3↓ and high at T4↓.
5. **Memory read and write.** `3A 00 80` (LD A,(8000h)): a read MC whose RD#
   falls at T1↓ and rises at T3↓, with the data sampled there. `32 01 80`:
   a write MC whose D is valid at T1↓ + `tdcfd` and whose WR# is low from
   T2↓ to T3↓; the bench stores the value.
6. **I/O.** `D3 10` with A = `12`: a 4-T-state I/O write, port `1210`,
   IORQ#/WR# low from T2↑. `DB 20`: a 4-T-state I/O read; A gets the bench's
   byte.
7. **WAIT.** WAIT# low across T2↓ of a memory read for 2 clocks: the MC
   lasts 5 T-states, and RD# stays low through the Tw states. WAIT# = X at a
   T2↓ that checks it → LOST.
8. **Data setup.** The opcode byte changes `tsdcr` − 1 ns before T3↑ (`00`
   to `76`): LOST. At `tsdcr` + 1 ns: HALT executes.
9. **IM 1.** EI, INT# low: an int-ack MC of 6 T-states with A = PC, M1# low
   from T1↑, IORQ# low from TW\*1↓. Then two writes (PC pushed), then a
   fetch at `0038`.
10. **IM 2.** I = `12`, vector `34`: after the acknowledge, reads at
    `1234`/`1235`, then a fetch at the word read. **IM 0** with `FF` on the
    bus: a fetch at `0038` (RST 38h).
11. **NMI.** NMI# 1→0: a 5-T-state fetch MC, two pushes, a fetch at `0066`.
    A pulse shorter than `twnmi` → LOST, unless the differential rule finds
    no effect (the NMI not yet sampled into an edge).
12. **HALT.** `76`: HALT# low at T4↓ + `tdcfhalt` of that fetch's MC. The
    halted M1 cycles keep refreshing (R advances). INT# with EI exits:
    HALT# high, then an int-ack.
13. **BUSREQ.** BUSREQ# low: after the current MC, BUSACK# low; address,
    data, MREQ#, IORQ#, RD# and WR# Z; no MC for 5 clocks; status
    BUS_RELEASED. Release: BUSACK# high at the next ↓, then the pending MC
    runs.
14. **Clock too fast.** One high phase < `twch` → LOST.
15. **Program** (the outcome at the top of this sub-plan):
    - the logged MC sequence matches the instruction timing of each opcode
      (machine cycles of 4,3 / 4,3,3 / 4,3,3,3 / 4,3,4 / 4 T-states);
    - BC = `3456`; status HALTED; last instruction `HALT`;
    - the instruction before it reads back as `OUT (10h),A`.

### 5.2 `library_check` Z80 rejection cases

- `"sync"` on a `"z80"` device;
- a missing `"busack"`;
- `tcc` 0;
- `"m1"` naming two pins.

## Phases

> **NOTE — keep the checkboxes current as you go. This is not bookkeeping; it is
> the only way anyone can see where the work actually is.**
>
> - Tick `- [x]` **in the same commit as the work it describes** — never in a
>   batch at the end of a phase, never "I'll update the plan after". If it is a
>   separate step it will be skipped, and the list becomes a lie.
> - Use `- [~]` for partially done and say in one line what remains.
> - Mark a task moot with `- [x] ~~text~~ — moot: <evidence>` rather than
>   deleting it. A vanished task is indistinguishable from a forgotten one.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Add any task you discover you needed; an unlisted task that got done is a
>   task the next reader will do twice.
>
> **An unticked box means NOT DONE.** Someone reading this file — including the
> person who has to decide whether to stop — is entitled to trust that.

### Phase 1 — Keys and the fetch/memory path

The loader accepts Z80 devices. The scheduler handles fetch, memory read and
write, RESET, clock checks and lost.

- [x] `models/cpu.c`: `"z80"` core, its keys and timing names (§4.1–§4.2),
      and its registers in `state` (the registers live in `cpuz80.c`'s
      `state`, like the 6502 family's). `library_check` → `ok   valid z80
      cpu file`.
- [x] `models/cpuz80.c`: `CpuCoreOps` for `z80`. §4.3 for the fetch, memory
      read and memory write rows; RESET; clock; lost. §4.4 for those MCs.
      Added to `backend/CMakeLists.txt`. Fresh build → 0 warnings.
- [x] `library_check.c`: §5.2. `cpu_test.c`: the Z80 fixture and cases 1–5,
      8, 14. The 4 §5.2 cases print `ok` (`"sync" is not a key of core
      "z80"`, `missing "busack"`, `"tcc" must be more than 0`, `m1: pin
      name "M1#" is used by 2 pins; one is needed`); Z80 cases 1–5, 8, 14
      PASS.

Acceptance: the Z80 leaves reset, fetches and reads/writes memory with
datasheet edges; its loader rejects bad devices.
  Check: `ctest --test-dir /tmp/logic-p09 -R 'cpu_test|library_check' -V | grep -c ': FAIL'` → 0 (est. 2 min). Measured: 0.
Commit: f14384e

### Phase 2 — I/O, WAIT, interrupts, HALT, BUSREQ

- [x] `cpuz80.c`: the I/O and int-ack rows; WAIT retries; HALT#; BUSREQ;
      the int-ack PC (§4.3–§4.4). Cases 6, 7, 9–13 PASS.
- [x] The tracker for Z80 (§4.5). Case 15 → `HALT` last, `OUT (10h),A`
      before it, at 0008.
- [x] `cpu_test.c`: cases 6, 7, 9–13, 15. `/tmp/logic-p09/cpu_test` →
      `cpu_test: all passed`.
- [x] `docs/parts-format.md` § `cpu`: the Z80 keys, timing names,
      behaviour, and the Non-goals' known differences. `docs/backend.md`:
      `cpuz80.c` in Layout (and the Z80 bench under `cpu_test`).

Acceptance: every case of §5.1 passes, and B's 6502-family cases still pass.
  Check: `ctest --test-dir /tmp/logic-p09 -R cpu_test -V | grep -c ': FAIL'` → 0, and `grep -c ': PASS'` → the number of `expect` calls the run makes (est. 1 min). Measured: 0 and 117 (86 call sites; the 6502-family ones run once per core).
Commit: f14384e (code and cases), 9d4a1d3 (docs)

## Validation Plan

- Tests: `cpu_test` Z80 cases (negatives 3, 7, 8, 11, 14) and
  `library_check` §5.2.
- Coverage check: every row of the §4.3 table is hit:
  - fetch: cases 4, 8;
  - memory: 5;
  - I/O: 6;
  - int-ack: 9, 10;
  - NMI fetch: 11.
  - BUSREQ, RESET and HALT have cases 13, 2/3 and 12.
- Runtime proof: case 15, a program from reset to HALT through the engine.
- Doc sync: `docs/parts-format.md` § `cpu`, `docs/backend.md`.
- Final gate (once, after Phase 2):
  `rm -rf /tmp/logic-land-p09c && cmake -S backend -B /tmp/logic-land-p09c && cmake --build /tmp/logic-land-p09c -j8 && ctest --test-dir /tmp/logic-land-p09c`
  → `100% tests passed`, T0 + 2 tests; `git diff --stat backend/tests/golden`
  → empty (est. 2 min). Plus the `xcodebuild` (est. 2 min).

## Decisions

Owner decisions: plan-09-A § Decisions. Author decisions: § 3 Rejected
alternatives (peek-and-commit scheduler, no core edits, no per-opcode
tables).

## Corrections

- **One commit for both phases' code.** The scheduler is one state machine
  over all six machine-cycle kinds: the MC start classifies all of them,
  and the I/O and int-ack rows share the fetch's phase counter. So
  `cpuz80.c` and the case functions landed whole in the Phase 1 commit, and
  both phases' boxes are ticked there. Phase 2's own commit holds the docs.
- **35 timing names, not 37** (§2 Measured populations). The §4.2 table
  lists 35 names (rows with "/" carry two). `TIMING_Z80` has those 35.
- **Bench log, writes.** WR# falls in T2, so a memory write's T1 is the
  rise before the WR# fall. The first run logged it one rise late. Case 5
  failed on `zlength` (read 0005 measured 4 T, write 2 T); fixed in the
  bench, not the model.
- **INT X while interrupts are disabled.** The core records INT in
  `int_bits` on every tick, but reads it only at a fetch with IFF1 set, and
  every tick rewrites it. The differential compare ignores that bit while
  IFF1 is 0 in both copies. Otherwise any undriven-then-X INT# would make a
  DI program lost.
- **RFSH# high at every MC start**, not only before a fetch (§4.4 lists it
  under the fetch). A fetch's RFSH# low lasts to the next T1 rise whatever
  machine cycle follows (LD A,(nn): fetch, then memory reads).
- **NMI# sampling.** §4.3 gives NMI no setup row. A fall is latched in
  `pin_changed` and passed to the core at the next rise, so a legal pulse
  between two rises is not missed. A pulse shorter than `twnmi` reads X at
  that rise (case 11: LOST).
- **Lengths in case 15.** The expected list is per logged machine cycle,
  with internal T-states counted into the cycle before them (the int-ack's
  extra T before its pushes is case 9's `t1 + 7`).
- **`ctest -V` prefix.** As in plan-09-B, the PASS/FAIL greps need
  `': PASS'` / `': FAIL'` (each output line is prefixed `<n>: `).

## Summary

C carries the plan's largest timing risk: mapping a core that is accurate
per T-state, but simplified per edge, onto the Z84C00's edges. The mapping
rests on request and WAIT positions measured across all 1604 opcode forms.
It is checked per machine-cycle type and by a whole program.
