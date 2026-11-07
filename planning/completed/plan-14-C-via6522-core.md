# plan-14-C: The `via6522` model, part 1 — ports, handshakes, timers, interrupts

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-14-B

C adds the W65C22 versatile interface adapter as a `pio` model `via6522`:
the 16 registers, both ports with input latching, the CA/CB control lines
in all eight PCR modes, Timer 1 (one-shot and free-run, PB7 output), Timer 2
(one-shot and PB6 pulse counting), and IFR/IER/IRQB. The shift register
(ACR bits 2–4) is D; until D lands, every ACR shift mode other than `000`
behaves as `000`.

Outcome of C: `pio_test` cases C1–C14 pass on inline copies of the
`W65C22N` and `W65C22S` devices of §3.1 (D ships them in `pio.json`); no
golden change.

References: `datasheets/pio/w65c22.pdf` — Table 2-1 register map p.7,
§2.1–2.4 (ports, handshake, PCR Table 2-5) p.7–12, Figures 2-1/2-2
(handshake) p.10–11, §2.5–2.10 timers p.13–19 (Table 2-8 ACR p.15, Figure
2-3 one-shot p.16, Figure 2-4 free-run p.17, Figure 2-5 pulse counting
p.19), §2.14 interrupts p.24, Tables 2-11/2-12 IFR/IER p.25–26, §3 pins
p.27–35 (§3.5 IRQB, §3.9 reset), AC tables 4-3a/4-3b p.40–41 and 4-4a/4-4b
p.42–43, timing figures p.44–47, §5 caveats p.48 (bus holding);
plan-14-A §4.3 (`bus65`), plan-14-B §4 (bench helpers).

## Prerequisites

plan-14-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-14-B landed | `grep -c -F 'model=pia6521 ' backend/tests/golden/parts.txt` → 4 | MET (2026-11-07: `4`) |

## 1. Goal

- `pio_test` C1–C14 `ok`, B and A cases still `ok`.
- `ctest` 100%, T0 + 1 programs; goldens unchanged.

### Non-goals

- No shift register behavior (D), no part (D), no app change.
- No NMOS 6522 quirks the W65C22 sheet does not describe.

## 2. Current State

After B: `pio` has the `pia6521` model; `bus65` drives every access; the
bench has `bench_bus65_read`/`write`.

## 3. Design

### 3.1 Device keys

```json
"model": "via6522",
"phi2": "PHI2", "rw": "RWB", "reset": "RESB",
"data": ["D0","…","D7"], "csHigh": ["CS1"], "csLow": ["CS2B"], "rs": ["RS0","RS1","RS2","RS3"],
"pa": ["PA0","…","PA7"], "pb": ["PB0","…","PB7"],
"ca1": "CA1", "ca2": "CA2", "cb1": "CB1", "cb2": "CB2", "irq": "IRQB",
"irqOpenDrain": true,
"busHold": false,
"grades": {"14": { … }}
```

- `"irqOpenDrain"` (bool, required): W65C22N `true` (IRQB pulls down only),
  W65C22S `false` (totem pole) (p.24, §3.5 p.30).
- `"busHold"` (bool, required): W65C22S `true` — every pin but PHI2 has a
  bus-holding device (§5.3 p.48); W65C22N `false`.
- Timing names (ns): `tcyc tpwh tpwl tacr tcar tcdr thr tpcr tacw tcaw tdcw
  thw tcpw` (Table 4-3a/b) and `tca2 trs1 trs2 twhs tds trs3 trs4 t21 til
  tsr1 tsr2 tal tpdh` (Table 4-4a/b). `tipw`, `ticw`, `tips`, `tics` are
  2 × `tcyc` in both tables and `tsr3` is 0 … `tcyc`, so they are computed,
  not keys. `tcyc`, `tpwh`, `tpwl` must be > 0. `tRS1` is printed in µs in
  both tables; it is ns like its neighbors (every other 65xx sheet, and the
  µs value would be longer than a cycle).
- Grade `14` values, 5 V columns:
  - W65C22N (Tables 4-3a, 4-4a): `tcyc 70 tpwh 35 tpwl 35 tacr 10 tcar 10
    tcdr 20 thr 10 tpcr 10 tacw 10 tcaw 10 tdcw 10 thw 10 tcpw 250 tca2 75
    trs1 75 trs2 100 twhs 75 tds 100 trs3 150 trs4 88 t21 88 til 62 tsr1 35
    tsr2 105 tal 88 tpdh 10`, `vcc [4.75, 5.25]` (Table 4-2a).
  - W65C22S (Tables 4-3b, 4-4b): the same except `tcpw 30`; `vcc [1.71, 5.5]`
    (Table 4-2b spans 1.71–5.5 V; timing is the 5 V column only, as the
    W65C02S's).

### 3.2 Registers (Table 2-1)

| RS | Write | Read |
|---|---|---|
| 0 | ORB | IRB (§3.3) |
| 1 | ORA, with handshake | IRA, with handshake |
| 2, 3 | DDRB, DDRA | DDRB, DDRA |
| 4 | T1 low latch | T1 counter low; clears IFR6 |
| 5 | T1 high latch; latch → counter; clears IFR6; starts T1 | T1 counter high |
| 6 | T1 low latch | T1 low latch |
| 7 | T1 high latch; clears IFR6 | T1 high latch |
| 8 | T2 low latch | T2 counter low; clears IFR5 |
| 9 | T2 counter high; low latch → counter low; clears IFR5; starts T2 | T2 counter high |
| A | SR (D) | SR (D) |
| B | ACR | ACR |
| C | PCR | PCR |
| D | IFR: a 1 bit clears that flag; bit 7 ignored | IFR, bit 7 = any (IFR & IER & 0x7F) |
| E | IER: bit 7 = 1 sets, 0 clears the bits written as 1 | IER, bit 7 reads 1 |
| F | ORA, no handshake | IRA, no handshake |

Writes take effect at the PHI2 fall (`bus65`). Port outputs change at fall +
`tcpw`. An unknown access makes the addressed register X (an unknown read of
a register with a read side effect makes the flag it clears X).

### 3.3 Ports (§2.1–2.3, §3.6–3.7)

- DDR 1: the pin is driven from OR. DDR 0: input, not driven.
- **Read IRA** (RS 1 or F): ACR bit 0 = 0 → the PA pins (every bit, output
  bits included) sampled at the PHI2 rise, X where a pin changed less than
  `tpcr` before it; ACR bit 0 = 1 → the byte latched at the last active CA1
  edge (pins sampled at the edge; a pin that changed less than `til` before
  the edge or within `tpdh` after it latches X).
- **Read IRB** (RS 0): output bits read ORB; input bits read the pins (ACR
  bit 1 = 0) or the CB1-edge latch (ACR bit 1 = 1), with the same rules.
  When ACR bit 7 = 1, bit 7 reads the T1 output level.
- **Bus hold** (`"busHold": true`): an input that reads Z reads the last
  known level it had (X before the first). With `false`, Z reads X. This
  applies to every input the model reads (port inputs, CA1, CA2, CB1, CB2,
  RESB, the bus pins through `bus65`, which gets the same flag).

### 3.4 Control lines (PCR, §2.4 Table 2-5)

- PCR bit 0 / bit 4: CA1 / CB1 active edge (1 = rising). An active edge sets
  IFR1 / IFR4 (and latches the port when ACR bit 0 / 1 is set).
- PCR bits 3–1 (CA2), 7–5 (CB2):
  - `000` input, falling edge sets IFR0 / IFR3; `010` rising edge. Reading or
    writing ORA (RS 1) clears IFR0 and IFR1; ORB clears IFR3 and IFR4.
  - `001` / `011` independent interrupt input (falling / rising): as above,
    but an ORA/ORB access does not clear IFR0 / IFR3 (Table 2-5 note).
  - `100` handshake output.
    - CA2, read handshake (Figure 2-1): low `tca2` after the PHI2 fall of a
      read of RS 1; high `trs2` after the next active CA1 edge.
    - CA2 / CB2, write handshake (Figure 2-2): low `twhs` after the PHI2 rise
      that follows a write of RS 1 / RS 0 (the next cycle's rise); high
      `trs4` after the next active CA1 / CB1 edge.
  - `101` pulse output: CA2 low `tca2` after the PHI2 fall of a read of RS 1,
    high `trs1` after the next PHI2 fall; CA2 / CB2 after a write of RS 1 /
    RS 0: low `twhs` after the next rise, high `trs3` after the rise after
    that.
  - `110` / `111`: CA2 / CB2 driven low / high from the PCR write's fall.
  - In input modes CA2/CB2 are not driven.
- A CA1/CB1/CA2/CB2 input that is X at an edge it watches sets the flag X.

### 3.5 Timer 1 (§2.5–2.7, Figures 2-3, 2-4)

Cycle rules (j = PHI2 cycles after the cycle whose fall wrote RS 5; the
counter decrements once per PHI2 fall, `bus65` `cycle()`):

- A read of the counter in cycle j (j ≥ 1) returns N − (j − 1) while that is
  ≥ 0, so it reads N in cycle 1 and 0 in cycle N + 1; it reads 0xFFFF in
  cycle N + 2.
- **Time-out:** at the PHI2 rise of cycle N + 2 — **N + 1.5 cycles** after
  the write's fall (Figure 2-4) — IFR6 is set and, with ACR bit 7, PB7
  toggles.
- **One-shot** (ACR bit 6 = 0): the counter keeps decrementing (0xFFFE in
  cycle N + 3, …); IFR6 is not set again until RS 5 is written. ACR bit 7 =
  1: PB7 goes low at the write's fall + `tcpw` and high at the time-out.
- **Free-run** (ACR bit 6 = 1): in cycle N + 3 the counter reads the latch
  value L (reloaded), and the next time-out is at the rise of cycle
  (N + 2) + (L + 2): every **L + 2** cycles (Figure 2-4 "N+2"). ACR bit 7 = 1:
  PB7 low at the write, toggles at each time-out (a square wave).
- A write of RS 7 (latch only) changes L for the next reload, clears IFR6,
  and does not restart the count.
- With ACR bit 7 = 1, PB7 is driven by T1 whatever DDRB bit 7 is (Table 2-8:
  "PB7 output"); ACR bit 7 = 0 returns PB7 to DDRB/ORB.

### 3.6 Timer 2 (§2.8–2.10)

- **One-shot** (ACR bit 5 = 0): the cycle rules of T1 one-shot, from a write
  of RS 9 (counter high ← data, counter low ← low latch). IFR5 at the rise
  of cycle N + 2; the counter keeps rolling (0xFFFF, 0xFFFE, …) with no
  further IFR5 until RS 9 is written again.
- **Pulse counting** (ACR bit 5 = 1): the counter decrements once for each
  falling edge on PB6 that reads 0 at a PHI2 rise (§2.10: "Logic 0 during the
  leading edge of PHI2"); PB6 pulses closer than `tips` or narrower than
  `tipw` (2 × `tcyc`) count X (the counter goes X). IFR5 is set when the
  count reaches 0, once per RS 9 write; the counter keeps decrementing.
- D adds T2 as the shift-register clock (its low byte), which also changes
  T2's reload in those modes.

### 3.7 Interrupts (§2.14)

- IFR bits: 0 CA2, 1 CA1, 2 SR (D), 3 CB2, 4 CB1, 5 T2, 6 T1.
- IRQB asserted while (IFR & IER & 0x7F) ≠ 0, at the same edge that sets the
  flag; released at the PHI2 fall of the access that clears the last
  enabled flag. No delay is added: the sheet gives no IRQB delay figure.
  `"irqOpenDrain": true` → asserted = 0, released = Z (`pin_open_drain`
  answers 1); `false` → asserted 0, released 1.

### 3.8 Reset and clock

- RESB low: every register cleared **except** T1, T2 (counters and latches)
  and SR, which keep their value (X at power-up) (§3.9); all port and control
  pins inputs; timers and SR stop; IRQB released. Leaving reset, the timers
  stay stopped until their start write.
- RESB X: every register X, IRQB X. RESB Z: X, or the held level with
  `busHold`.
- PHI2 widths and period as `bus65` rule 6 with `tpwh`, `tpwl`, `tcyc`.

## 4. Tests (added to `pio_test`)

Inline device copies `W65C22N` and `W65C22S` (§3.1) in the test file; PHI2
1 MHz.

| Case | Device | Checks |
|---|---|---|
| C1 reset | N | registers cleared, T1/T2 latches kept; ports inputs; IRQB Z |
| C2 ports | S | DDRA/DDRB, ORA/ORB to pins at fall + 30 ns; IRB mixes ORB and pins |
| C3 latching | N | ACR = 0x03, CA1/CB1 edge latches PA/PB; pin change 20 ns before the edge latches X |
| C4 RS 1 vs RS F | N | RS F read/write leaves IFR1/IFR0 and CA2 alone; RS 1 clears them |
| C5 CA2 read handshake | N | PCR = 0x08: read RS 1 → CA2 low at fall + 75 ns; CA1 edge → high 100 ns later |
| C6 CB2 write handshake | S | PCR = 0x80: write RS 0 → CB2 low at next rise + 75 ns; CB1 edge → high 88 ns later |
| C7 pulse outputs | N | PCR = 0x0A (CA2 pulse), 0xA0 (CB2 pulse) |
| C8 independent input | S | PCR = 0x02: CA2 falling sets IFR0; RS 1 read does not clear it; IFR write does |
| C9 T1 one-shot | N | write T1 = 0x0005: reads 5,4,3,2,1,0,FFFF in cycles 1–7; IFR6 at the rise of cycle 7 (6.5 µs after the write's fall; Corrections); no second IFR6 |
| C10 T1 free-run PB7 | S | ACR = 0xC0, T1 = 3: PB7 toggles every 5 cycles; RS 7 write changes the next period only |
| C11 T2 one-shot | N | IFR5 at N + 1.5; counter rolls to FFFF and on |
| C12 T2 pulse count | S | ACR = 0x20, T2 = 3, three PB6 pulses → IFR5 after the third; a 1-cycle-wide pulse → counter X |
| C13 IER/IFR/IRQB | N, S | IER set/clear by bit 7; IFR bit 7; IRQB Z vs 1 released per `irqOpenDrain` |
| C14 bus hold | S vs N | a PA input driven 1 then released reads 1 on S, X on N |

Also `library_check`: a `via6522` device missing `"irqOpenDrain"`, and one
with 3 `"rs"` names, are refused.

## Phases

> **Status discipline:** as plan-14-A § Phases.

### Phase 1 — Registers, ports, control lines, interrupts

- [x] `backend/src/models/pio_via6522.c` (§3.1–§3.4, §3.7, §3.8) registered
      in `pio.c`; `bus65` gains the `busHold` flag (§3.3; a Correction in A
      if A's rule text changes); `backend/CMakeLists.txt`. (`bus65` already
      had the flag from A; the model sets it from `"busHold"`.) Build: no
      errors or warnings.
- [x] `pio_test` C1–C8, C13, C14; `library_check` refusals. → `ok C1 reset` …
      `ok C8 independent input`, `ok C13 IER/IFR/IRQB (N)`, `(S)`, `ok C14 bus hold`;
      `ok   via6522 missing irqOpenDrain: … missing "irqOpenDrain"`,
      `ok   via6522 rs of 3: … "rs" needs exactly 4 pin names`. A mutation
      (read-handshake release delay trs2 → trs4) makes C5 fail, then reverted.

Acceptance: those cases `ok`; earlier cases still `ok`.
  Check: `/tmp/logic-p14/pio_test` → every case `ok` (est. 1 min with the build).
Commit: 77adb41

### Phase 2 — Timers

- [x] T1 and T2 (§3.5, §3.6) in `pio_via6522.c`.
- [x] `pio_test` C9–C12. → `ok C9 T1 one-shot`, `ok C10 T1 free-run PB7`,
      `ok C11 T2 one-shot`, `ok C12 T2 pulse count`.
- [x] `docs/parts-format.md` § `pio` → Model `via6522` (keys, timing names,
      §3.2–§3.8; "Shift register: D").
- [x] Acceptance: `/tmp/logic-p14/pio_test` → 33 `ok` (A1–A8, B1–B10, C1–C14
      with C13 on both dies); `ctest` → `100% tests passed, 0 tests failed out of 21`;
      the golden diff stat → empty.

Acceptance: C1–C14 `ok`; `ctest` 100%, goldens unchanged.
  Check: `ctest --test-dir /tmp/logic-p14` → `100% tests passed`; `git diff --stat backend/tests/golden` → empty (est. 2 min).
Commit: 77adb41

## Validation Plan

`pio_test` C1–C14. The landing tier is plan-14-J's.

## Decisions

- Author: IRQB follows the flags with no added delay (§3.7: the sheet gives
  none).
- Author: with ACR bit 7 = 1, T1 drives PB7 regardless of DDRB bit 7 (§3.5).
- Author: bus hold is modeled as "Z reads the last known level" inside the
  model; the net itself stays Z (§3.3).
- Author: `tRS1` is ns, not µs (§3.1).

## Corrections

- Phases 1 and 2 landed in one commit: the timers were written with the
  rest of the model, and C9–C12 ran in the same build.
- §4 C9: "5.5 µs after the write's fall" contradicts §3.5 (time-out at the
  rise of cycle N + 2, N + 1.5 cycles after the fall) and C9's own read
  sequence (FFFF in cycle 7). For N = 5 the rise of cycle 7 is 6.5 µs after
  the fall; the case checks IRQB low at that rise.
- §3.6 / §4 C12: `tipw`/`tips` = 2 × tCYC must be the **actual** PHI2
  period for C12's "1-cycle-wide pulse → X" to hold at 1 MHz (2 × the
  grade's 70 ns is 140 ns, far under a 1 µs pulse). The model measures the
  PHI2 period between rises and uses two of it. Spacing is measured from
  the end of one pulse to the start of the next.
- §3.3: the datasheet (p.8) adds that a latched IRA, once read, is
  "transparent, reflecting the current state of the PA bus pins until the
  next CA1 latching transition"; IRB likewise. Modeled, and checked in C3
  (the second IRA read shows the pins). The plan's text said only "the byte
  latched at the last active CA1 edge".
- §4 C10: the plan's "RS 7 write changes the next period only" is tested
  with RS 7 = 1 in cycle 7 (L = 0x0103): time-outs at cycles 5, 10, then
  10 + 0x105 = 271.
- §3.4 (not stated): entering handshake (`100`) or pulse (`101`) mode sets
  CA2/CB2 high, as the PIA (plan-14-B); C5–C7 check it.
- §3.4: CA2's handshake can be strobed by a read (`tca2`, released `trs2`
  after the CA1 edge) or a write (`twhs`, released `trs4`); the model keeps
  which one drove it low to pick the release delay.
- `via6522` keeps a 64-pin hold table: a device with more pins is refused
  (`a via6522 has at most 64 pins`).
