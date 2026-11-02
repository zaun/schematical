# exar-d — XR16V554, XR16V564, XR16V654, XR16V798, XR16C854, XR16C864, XR16L784, SC16C650B, SC16C752B

Plan-11 datasheet extraction (group exar-d). All PDFs in `datasheets/controller-uart/`. Page numbers are PDF
page numbers (= printed page numbers for these sheets). Page images used: /tmp/p11/img/exar-d-*.

## PDF → devices

| PDF | Device(s) | Census parts |
|---|---|---|
| xr16v554.pdf | XR16V554 (incl. 554D bonding option) | XR16V554IJ, XR16V554IL, XR16V554IV |
| xr16v564.pdf | XR16V564 (incl. 564D) | XR16V564IJ, XR16V564IL, XR16V564IV, XR16V564DIV |
| xr16v654.pdf | XR16V654 (incl. 654D) | XR16V654IV-F, XR16V654DIV-F, XR16V654IL-F |
| xr16v798.pdf | XR16V798 | XR16V798IQ-F |
| xr16c854.pdf | XR16C854 (incl. 854D) | XR16C854CV-F, XR16C854IV-F, XR16C854DIV-F, XR16C854IQ-F, XR16C854IQ, XR16C854CQ |
| xr16c864.pdf | XR16C864 | XR16C864CQ-F, XR16C864IQ-F |
| xr16l784.pdf | XR16L784 | XR16L784CV-F, XR16L784IV-F |
| sc16c650b.pdf | SC16C650B | SC16C650BIBS,128 |
| sc16c752b.pdf | SC16C752B | SC16C752BIB48,128, SC16C752BIBS,157 (2 census rows) |

Census parts not decoded: **none** (all 25 census rows decoded). Caveats: Sipex-era names without "-F"
(XR16V554Ix, XR16V564Ix, XR16C854CQ/IQ) are decoded from the same ordering codes; XR16C854CQ/IQ are not in
the current XR16C854 ordering table (only -F) and may be pre-"F2" date-code parts without 5 V-tolerant
inputs. NXP ",128"/",157" suffixes are packing codes.

Design families in this group:
- XR16V554 → plain quad 16C550 (16-byte FIFO).
- XR16V564 / XR16V654 → ST16C654-style enhanced quad (EFR bank, DLD fractional divisor, 32 / 64-byte FIFO);
  same pins as V554 except CLKSEL.
- XR16C854 / XR16C864 → ST16C854-style 128-byte quad (FCTR/EMSR/TRG/FC, tables A-D); 864 adds DMA, OP1/OP2,
  BCLK, auto RS-485 on OP1 (QFP100 only).
- XR16V798 (octal) / XR16L784 (quad) → "5G flat register" XR16L788 family (A7..A0 flat map, device config
  block, timer, single INT#).
- SC16C650B (single, 32-byte) / SC16C752B (dual, 64-byte, TCR/TLR) → NXP.
Obsolete census rows: none (statuses Active / Not For New Designs).

## XR16V554

- Title: "XR16V554/554D — 2.25V TO 3.6V QUAD UART WITH 16-BYTE FIFO", Exar Rev 1.0.3 July 2010.
  Datasheet: `datasheets/controller-uart/xr16v554.pdf` (43 pp; pinouts p2-3, ordering p4, pins p4-8,
  function p9-18, registers p19-29, DC p30, AC p31-32, timing figs p33-38, packages p39-42).
- Function: four independent 16C550-compatible UARTs, 16-byte TX FIFO + 16-byte RX FIFO (11 bits wide:
  data + 3 error tags) per channel, shared host bus, one crystal oscillator. **Plain 16C550 register set —
  no EFR, no auto flow control, no sleep, no IrDA, no FIFO-count registers** (despite TX "infrared" mention
  in TX pin text and "LCR≠0xBF" qualifier on DLL/DLM, p4/p6/p19 — there are no LCR=0xBF registers listed).
  Pin-compatible with ST16C554 / TL16C554A / SC16C554B (p1).
- Nearest siblings in this group: XR16V564 (32-byte FIFO + enhanced register set), XR16V654 (64-byte FIFO
  + enhanced set). V554 differs from both by being a bare 16C550 (16-byte FIFO, 4 RX trigger levels only).
- XR16V554 vs XR16V554D: same die; only on the 64-LQFP, where INTSEL is bonded internally: XR16V554IV =
  INTSEL to GND (INT pins 3-state unless MCR[3]=1), XR16V554DIV = INTSEL to VCC (INT always active) (p6, p9).
  Treat as one device with a package-level INTSEL strap.

### Parts (census)
| Part | Package | Pins | Pinout name | Grade | VCC |
|---|---|---|---|---|---|
| XR16V554IJ | 68-lead PLCC | 68 | plcc68 | -40..+85 °C (I) | 2.25-3.6 V |
| XR16V554IL | 48-pin QFN 7x7, exposed pad | 48 | qfn48 | -40..+85 °C | 2.25-3.6 V |
| XR16V554IV | 64-lead LQFP 10x10 | 64 | lqfp64 (INTSEL internally GND) | -40..+85 °C | 2.25-3.6 V |

Ordering table p4: XR16V554IJ 68-PLCC, XR16V554IV 64-LQFP, XR16V554DIV 64-LQFP, XR16V554IL 48-QFN,
XR16V554IV80 80-LQFP, all -40..+85 °C, all "Active". Census names are Sipex-era without "-F"; the
datasheet also lists them without "-F". Suffix decode: J=PLCC, V=LQFP, L=QFN, I=industrial; "D" before
the package letter = INTSEL bonded high. Only one speed/supply grade exists.

### Pinouts (pin table p4-8, drawings p2-3)
Mode-dependent pins: 16/68 (pin present on PLCC and QFN only; internal pull-up; HIGH=Intel "16 mode",
LOW=Motorola "68 mode"). LQFP64/LQFP80 are Intel-only.
In 68 mode: IOR→unused (tie VCC); IOW→R/W (H=read, L=write); CSA→CS (active low); CSB→A3; CSC→A4;
CSD→unused (tie VCC); INTA→IRQ (active-low open-drain, needs pull-up); INTB/C/D→unused, "stay at logic
zero"; RESET→RESET (active low); INTSEL must be LOW (p4-7).

**plcc68** (XR16V554IJ):
```
1 D3 inout        18 IOW input       35 XTAL1 input     52 IOR input
2 D4 inout        19 TXB output      36 XTAL2 output    53 TXD output
3 D5 inout        20 CSB input       37 RESET input     54 CSD input
4 D6 inout        21 INTB output     38 RXRDY output    55 INTD output
5 D7 inout        22 RTSB output     39 TXRDY output    56 RTSD output
6 GND ground      23 GND ground      40 GND ground      57 GND ground
7 RXA input       24 DTRB output     41 RXC input       58 DTRD output
8 RIA input       25 CTSB input      42 RIC input       59 CTSD input
9 CDA input       26 DSRB input      43 CDC input       60 DSRD input
10 DSRA input     27 CDB input       44 DSRC input      61 CDD input
11 CTSA input     28 RIB input       45 CTSC input      62 RID input
12 DTRA output    29 RXB input       46 DTRC output     63 RXD input
13 VCC power      30 VCC power       47 VCC power       64 VCC power
14 RTSA output    31 16/68 input     48 RTSC output     65 INTSEL input
15 INTA output    32 A2 input        49 INTC output     66 D0 inout
16 CSA input      33 A1 input        50 CSC input       67 D1 inout
17 TXA output     34 A0 input        51 TXC output      68 D2 inout
```
(68-mode names: 15 IRQ, 16 CS, 18 R/W, 20 A3, 50 A4, 37 RESET active-low; 52/54 unused; 21/49/55 unused.)

**lqfp64** (XR16V554IV; DIV identical pins): Intel only, no 16/68, no INTSEL, no TXRDY/RXRDY.
```
1 DSRA input      17 DSRB input      33 CTSC input      49 CDD input
2 CTSA input      18 CDB input       34 DTRC output     50 RID input
3 DTRA output     19 RIB input       35 VCC power       51 RXD input
4 VCC power       20 RXB input       36 RTSC output     52 VCC power
5 RTSA output     21 VCC power       37 INTC output     53 D0 inout
6 INTA output     22 A2 input        38 CSC input       54 D1 inout
7 CSA input       23 A1 input        39 TXC output      55 D2 inout
8 TXA output      24 A0 input        40 IOR input       56 D3 inout
9 IOW input       25 XTAL1 input     41 TXD output      57 D4 inout
10 TXB output     26 XTAL2 output    42 CSD input       58 D5 inout
11 CSB input      27 RESET input     43 INTD output     59 D6 inout
12 INTB output    28 GND ground      44 RTSD output     60 D7 inout
13 RTSB output    29 RXC input       45 GND ground      61 GND ground
14 GND ground     30 RIC input       46 DTRD output     62 RXA input
15 DTRB output    31 CDC input       47 CTSD input      63 RIA input
16 CTSB input     32 DSRC input      48 DSRD input      64 CDA input
```

**qfn48** (XR16V554IL): no DTR/DSR/CD/RI, no TXRDY/RXRDY. Exposed center pad = GND (p7).
```
1 CTSA input      13 RXB input      25 RTSC output     37 VCC power
2 VCC power       14 16/68 input    26 INTC output     38 INTSEL input
3 RTSA output     15 A2 input       27 CSC input       39 D0 inout
4 INTA output     16 A1 input       28 TXC output      40 D1 inout
5 CSA input       17 A0 input       29 IOR input       41 D2 inout
6 TXA output      18 XTAL1 input    30 TXD output      42 D3 inout
7 IOW input       19 XTAL2 output   31 CSD input       43 D4 inout
8 TXB output      20 RESET input    32 INTD output     44 D5 inout
9 CSB input       21 GND ground     33 RTSD output     45 D6 inout
10 INTB output    22 RXC input      34 GND ground      46 D7 inout
11 RTSB output    23 CTSC input     35 CTSD input      47 GND ground
12 CTSB input     24 VCC power      36 RXD input       48 RXA input
EP GND (center pad)
```
(68-mode names as in PLCC: 4 IRQ, 5 CS, 7 R/W, 9 A3, 27 A4, 20 RESET active-low.)
Note: pin table p7 lists QFN GND as "21, 47" only; drawing p3 shows pin 34 = GND (only number not
otherwise assigned). Use GND for 34.

80-LQFP (XR16V554IV80, not in census) pinout is on p3 — not transcribed.

### Host interface (p4-5, p10-11, p31)
- Intel (16 mode): A2..A0 select register; CSA/CSB/CSC/CSD active-low select channel A-D (Table 1 p11).
  All four low at once = write same register in all channels ("do not attempt to read from all four");
  other multi-select combos not defined. IOR active low: falling edge starts internal read, data on bus
  until IOR rises (host latches on rising edge). IOW active low: falling edge starts write, **rising edge
  transfers data** into the register. No clock needed for bus cycles. D7..D0 driven only while CS and IOR
  low (3-state otherwise; TDD float time).
- Motorola (68 mode): CS (=CSA pin) active low, R/W (=IOW pin, H read/L write), A4:A3 (=CSC:CSB) select
  channel (Table 2 p11: 00 A, 01 B, 10 C, 11 D), A2..A0 register. Cycle framed by CS low; write data
  latched at CS rising (TWDS/TWDH to CS deassert, fig 17 p35).
- No ALE, no DDIS.
- TXRDY / RXRDY (PLCC/LQFP80 only) = logical AND of the four channels' TXRDY#/RXRDY# (active low: pin low
  if any channel's signal is low) (p5).

### Registers (per channel; Table 7 p19, Table 8 p20, descriptions p20-28)
| A2-A0 | Cond. | Name | R/W | Reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | R | XX |
| 000 | LCR[7]=0 | THR | W | XX |
| 001 | LCR[7]=0 | IER | R/W | 00 |
| 010 | – | ISR | R | 01 |
| 010 | – | FCR | W | 00 |
| 011 | – | LCR | R/W | 00 |
| 100 | – | MCR | R/W | 00 |
| 101 | LCR[7]=0 | LSR | R (writable, not recommended) | 60 |
| 110 | LCR[7]=0 | MSR | R (writable, not recommended) | bits3-0=0, bits7-4 = inverted input levels |
| 111 | – | SPR | R/W | FF |
| 000 | LCR[7]=1 | DLL | R/W | 01 (power-up only, not reset by RESET pin) |
| 001 | LCR[7]=1 | DLM | R/W | 00 (power-up only) |
Reset table p29. Note LSR/MSR are only listed under LCR[7]=0; Table 7 does not say what 101/110 read
when LCR[7]=1 (16C550 practice: still LSR/MSR) — unspecified.

- IER: b0 RX data ready (and timeout) int enable; b1 THR empty int enable (if THR already empty when set →
  interrupt immediately); b2 RX line status int enable (LSR[1] immediately on overrun, LSR[2-4] when the
  errored char is in RHR); b3 modem status int enable; b7-4 reserved (read 0 per table) (p21).
- ISR: b0 0=interrupt pending, 1=none; b3-1 source code (table below); b5-4 reserved 0; b7-6 = 11 when FIFOs
  enabled (FCR[0]=1), 00 otherwise (p22-23).
- FCR (write-only): b0 FIFO enable (TX+RX; must be 1 when writing other bits or they are ignored); b1 RX
  FIFO reset (pointers/counter, RSR untouched, self-clearing; only when b0=1); b2 TX FIFO reset (same);
  b3 DMA mode select (TXRDY/RXRDY behavior); b5-4 reserved; b7-6 RX trigger: 00=1, 01=4, 10=8, 11=14
  (Table 10 p23).
- LCR: b1-0 word length 5/6/7/8; b2 stop bits: 0=1, 1=1.5 (5-bit) or 2 (6-8 bit); b3 parity enable;
  b4 even(1)/odd(0); b5 stick parity: with b3=1, b5=1,b4=0 → parity forced 1 (mark); b5=1,b4=1 → forced 0
  (space) (Table 11 p25); b6 break (TX forced 0 until cleared); b7 divisor latch enable (p24-25).
- MCR: b0 DTR (1 → DTR pin LOW); b1 RTS (1 → RTS pin LOW); b2 reserved, OP1 (no pin) — in loopback drives
  RI; b3 INT output enable (16 mode, see Table 12) and OP2 in loopback (drives CD); b4 loopback; b7-5
  reserved. **MCR[7] prescaler is mentioned in §2.8 ("If the pre-scaler is used (MCR bit-7 = 1), data rate
  4x less") but MCR[7] is listed Reserved on p26 and in Table 8** — datasheet contradiction (see Notes).
- LSR: b0 data ready (set when char moved RSR→RHR/FIFO, cleared when RHR/FIFO empty); b1 overrun (char
  arrives while FIFO full (or RHR full in non-FIFO); RSR char overwritten, not put in FIFO); b2 parity error
  tag of char at top of FIFO; b3 framing error tag (no valid stop bit); b4 break tag (RX low ≥ one full
  frame; only one break char (0x00) loaded into FIFO; RX must return to mark before next char); b5 THR empty
  (non-FIFO: THR empty; FIFO: TX FIFO empty); b6 THR and TSR both empty; b7 RX FIFO global error (any
  PE/FE/BI char in FIFO; clears when none remain). LSR read clears b1..b4 per interrupt clearing rules
  (p26-27, 22).
- MSR: b0 ΔCTS, b1 ΔDSR, b2 TERI (RI changed LOW→HIGH, i.e. pin goes inactive), b3 ΔCD (any change);
  b4 CTS = complement of CTS pin (loopback: = MCR[1]); b5 DSR (= MCR[0] in loopback); b6 RI (= MCR[2] in
  loopback); b7 CD (= MCR[3] in loopback). Delta bits cleared by MSR read (p27-28, 22).
- SPR: scratch, reset FF.
- DLL/DLM: 16-bit divisor (DLM:DLL).

ISR priority (Table 9 p22):
| Prio | ISR[3:0] | Source | Clear |
|---|---|---|---|
| 1 | 0110 | RX line status (LSR 1-4) | read LSR |
| 2 | 1100 | RX data timeout | read RHR |
| 3 | 0100 | RX data ready / trigger | read until FIFO below trigger (non-FIFO: read RHR) |
| 4 | 0010 | THR empty (TX FIFO empty) | read ISR (when this is the source) or write THR |
| 5 | 0000 | Modem status (MSR 0-3) | read MSR |
| – | 0001 | none | |

### Behavior
- **Baud generator** (p13-14): XTAL1 crystal (≤24 MHz) or external clock (≤64 MHz at 3.3 V, ≤50 MHz at
  2.5 V, p31); one oscillator shared by all 4 channels; each channel has own BRG. Baud = clock / (16 ×
  divisor), divisor = DLM:DLL, 1..65535. Divisor 0: not defined in datasheet. Text claims fractional
  increments of 1/16 and an MCR[7] ÷4 prescaler — neither has a register in this part (no DLD, MCR[7]
  reserved); model as integer divisor, no prescaler (flag for planner). No BAUDOUT/RCLK pins. Table 6
  (p14, titled 14.7456 MHz) divisors: 921.6k→1, 460.8k→2, 230.4k→4, 153.6k→6, 76.8k→12, 38.4k→24,
  19.2k→48, 9600→96, 4800→192, 2400→384, 400→2304.
- **Transmitter** (p14-15): start bit, 5-8 data LSB first, optional parity, 1/1.5/2 stop; 16 clocks per
  bit. LSR[5] set when THR (non-FIFO) / FIFO (FIFO mode) empties; LSR[6] when TSR also empty. Idle/reset TX
  = HIGH. Break: LCR[6] forces TX low.
- **Receiver** (p16): falling edge of start starts 16x counter; sampled at count 8 (middle); still low →
  valid start, else false start ignored. Data/parity/stop sampled mid-bit. Errors in LSR[4:2], stored as
  3-bit tag per FIFO entry, presented when that byte is at the top (RHR). RX timeout: no data received for
  4 word lengths (per LCR[1:0]) + 12 bit times (≈3.7-4.6 char times) with data in FIFO → ISR 1100 (p16,22).
  Timeout interrupt only when IER[0]=1.
- **FIFOs**: 16 TX, 16 RX (+3-bit error tag). FCR[0] enables both. RX data ready interrupt asserts when FIFO
  level reaches trigger, deasserts when below (p21). DMA/TXRDY/RXRDY (Table 5 p13):
  - FIFO disabled: RXRDY low = 1 byte in RHR, high = empty; TXRDY low = THR empty, high = byte in THR.
  - FIFO enabled, FCR[3]=0 (mode 0): RXRDY low = ≥1 byte in RX FIFO, high = empty; TXRDY low = TX FIFO
    empty, high = ≥1 byte in FIFO.
  - FIFO enabled, FCR[3]=1 (mode 1): RXRDY goes low when RX FIFO reaches trigger or timeout, high when
    FIFO empties; TXRDY low = ≥1 empty location, high = TX FIFO full.
  (Tables 3/4 p12 headed "INT pin" for TX/RX actually repeat TXRDY/RXRDY-like levels with inverted sense;
  they contradict the ISR text — treat INT as an active-high OR of enabled pending interrupts.)
- **Interrupts** (p22, 26): INTx pin active HIGH in 16 mode (pending interrupt → HIGH). Output enable:
  INTSEL=0 & MCR[3]=0 → 3-state; INTSEL=0 & MCR[3]=1 → active; INTSEL=1 → always active (Table 12 p26).
  In 68 mode a single IRQ (on INTA pin) is active-low open-drain wired-OR of all 4 channels; INTSEL must be
  low. Pin type of INTA marked "(OD)" — open drain applies to 68 mode IRQ.
- **Modem**: RTS/DTR outputs = complement of MCR[1]/MCR[0]. CTS/DSR/CD/RI inputs only reported in MSR —
  "This input has no effect on the UART" (DSR/CD/RI, p6-7). CTS text p28 says "A HIGH on CTS# will stop the
  transmitter as soon as the current character has finished" — but no auto-CTS enable exists in this part;
  conflicts with 16C550 behavior (flag; most likely boilerplate).
- **Loopback** (MCR[4], p18 fig 11): TSR output → RSR input internally; TX pin held HIGH; RTS and DTR pins
  de-asserted (HIGH); CTS, DSR, CD, RI pins ignored; MSR[4]←MCR[1] (RTS), MSR[5]←MCR[0] (DTR),
  MSR[6]←MCR[2] (OP1), MSR[7]←MCR[3] (OP2). Interrupts work normally. RX pin should be held high.
- **Reset** (p7, p11, Table 13 p29): RESET pin active HIGH in 16 mode, active LOW in 68 mode; ≥40 ns.
  Resets IER=00, FCR=00, ISR=01, LCR=00, MCR=00, LSR=60, MSR[3:0]=0, SPR=FF (sic), FIFOs. DLL/DLM NOT
  reset by pin (power-up only: 01/00). Outputs during/after reset: TX HIGH, RTS HIGH, DTR HIGH, RXRDY HIGH,
  TXRDY LOW, INT (16 mode) 3-state if INTSEL=LOW (XR16V554), LOW if INTSEL=HIGH or XR16V554D; IRQ (68 mode)
  HIGH (released).
- No sleep, no IrDA, no auto flow control, no special characters, no device ID.

### Timing (p31-32; limits min/max, no typicals; 2.5 V±10% / 3.3 V±10%, 70 pF)
| Sym | Parameter | 2.5 V | 3.3 V |
|---|---|---|---|
| XTAL1 | crystal freq max | 24 MHz | 24 MHz |
| ECLK | ext clock max | 50 MHz | 64 MHz |
| TECLK | ext clock period min | 10 ns | 7 ns |
| TAS/TAH | addr setup/hold (16) min | 0/0 | 0/0 |
| TCS | CS width min | 50 | 40 ns |
| TRD | IOR width min | 50 | 40 |
| TDY | read/write cycle delay min | 50 | 40 |
| TRDV | data access from IOR max | 45 | 35 |
| TDD | data disable after IOR max | 10 | 10 |
| TWR | IOW width min | 50 | 40 |
| TDS/TDH | data setup/hold to IOW rise min | 10/5 | 10/5 |
| TADS/TADH | addr setup/hold (68) min | 0/0 | 0/0 |
| TRWS | R/W setup to CS min | 0 | 0 |
| TRDA | data access (68) max | 45 | 35 |
| TRDH | data disable (68) max | 10 | 10 |
| TWDS/TWDH | write data setup/hold (68) min | 10/5 | 10/5 |
| TRWH | CS deassert to R/W deassert min | 10 | 5 |
| TCSL / TCSD | CS width / cycle delay (68) min | 50/50 | 40/40 |
| TWDO | IOW to modem output max | 50 | 50 |
| TMOD | modem input to INT max | 50 | 50 |
| TRSI | IOR to reset interrupt max | 50 | 50 |
| TSSI | stop to set interrupt max | 1 Bclk | 1 Bclk |
| TRRI | IOR to reset interrupt max | 45 | 45 |
| TSI | start to interrupt max | 45 ns | 45 ns |
| TINT | initial INT reset to TX start | 8-24 Bclk | 8-24 Bclk |
| TWRI | IOW to reset interrupt max | 45 | 45 |
| TSSR | stop to set RXRDY max | 1 Bclk | 1 Bclk |
| TRR | IOR to reset RXRDY max | 45 | 45 |
| TWT | IOW to set TXRDY max | 45 | 45 |
| TSRT | center of start to reset TXRDY max | 8 Bclk | 8 Bclk |
| TRST | reset pulse min | 40 ns | 40 ns |
Bclk = 16x baud clock (p32). Max data rate 4 Mbps @3.3 V, 3.125 Mbps @2.5 V (p1).

### VCC
2.25-3.6 V (all parts, -40..+85 °C), p1/p30. Abs max supply 4 V; any pin -0.3..5.5 V. All inputs 5 V
tolerant except XTAL1 (p7) → inputMax 5.5 V; XTAL1 limited to VCC. VIH 1.8 V (2.5 V) / 2.0 V (3.3 V);
VIL 0.5/0.7 V (p30).

### Package drawings
- 48 QFN 7x7x0.9, 0.50 pitch (p39): D 6.85-7.15 (7.00); D2 (pad) 5.10-5.30 (5.20); b 0.18-0.30 (0.24);
  L 0.30-0.50 (0.40); e 0.50; A 0.80-1.00; k ≥0.20. No land pattern printed.
- 64 LQFP 10x10x1.4 (p40): D 11.80-12.20 (12.00 span); D1 9.90-10.10 (body 10); e 0.50; B (lead w)
  0.17-0.27 (0.22); L 0.45-0.75 (0.60); A max 1.60. No land pattern.
- 68 PLCC (p41, inch-controlled): D 0.985-0.995" (25.02-25.27 mm) span; D1 0.950-0.958" body; D2
  0.890-0.930" (22.61-23.62) contact-row; D3 0.800" typ; e 1.27; B 0.33-0.53 (0.43), B1 0.66-0.81;
  A 4.19-5.08. No land pattern.
- (80 LQFP 12x12, p42: D 13.80-14.20, D1 11.80-12.20, e 0.50 — not a census package.)

### Obsolete
None (census status Active for all three).

### Notes
- §2.8 claims fractional divisor (1/16 steps) and MCR[7] ÷4 prescaler; register map has neither (MCR[7]
  reserved, no DLD). Text was copied from XR16V654-class sheet. Plan: integer divisor only.
- CTS# description p28 implies hardware CTS stop; no enable bit exists. Treat as boilerplate.
- Tables 3/4 (p12) "INT pin" levels inconsistent with ISR-driven INT; ignore.
- SPR resets to FF (Table 13), not 00.
- QFN GND pin 34 missing from pin table (p7), present in drawing (p3).

## XR16V564

- Title: "XR16V564/564D — 2.25V TO 3.6V QUAD UART WITH 32-BYTE FIFO", Exar Rev 1.0.4 July 2010; every page
  carries an "NRND — Not Recommended for New Designs" watermark. Datasheet:
  `datasheets/controller-uart/xr16v564.pdf` (54 pp; pinouts p2-3, ordering p4, pins p4-8, function p9-23,
  registers p24-39, DC p40, AC p41-42, timing figs p43-48, packages p50-53).
- Function: four 16C550-compatible UARTs with 32-byte TX/RX FIFOs, 4 selectable TX and RX trigger levels,
  enhanced (ST16C654-style) register bank: EFR, Xon1/2, Xoff1/2, auto RTS/CTS with hysteresis, auto
  Xon/Xoff, special char detect, Xon-any, IrDA 1.0, sleep with auto-wake, fractional divisor (DLD) with
  16x/8x/4x sampling, ÷1/÷4 prescaler (MCR[7] + CLKSEL pin). Pin compatible with ST16C554/TL16C754B/
  SC16C754B (p1).
- Differs from XR16V554 (nearest sibling): 32-byte FIFOs (vs 16), RX triggers 8/16/24/30 (vs 1/4/8/14),
  TX triggers, enhanced registers, DLD, sleep, IrDA; CLKSEL pin replaces one VCC pin (LQFP64 pin 21,
  PLCC68 pin 30). Bus interface, pin map otherwise identical to XR16V554. Differs from XR16V654 by FIFO
  depth (32 vs 64) and trigger tables.
- XR16V564 vs XR16V564D: same die; on the 64-LQFP only, INTSEL bonded to GND (IV) or VCC (DIV) (p6, p9).
  One device.

### Parts (census)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| XR16V564IJ | 68-lead PLCC | 68 | plcc68 | -40..+85 °C | 2.25-3.6 V |
| XR16V564IL | 48-pin QFN 7x7, exposed pad | 48 | qfn48 | -40..+85 °C | 2.25-3.6 V |
| XR16V564IV | 64-lead LQFP 10x10 | 64 | lqfp64 (INTSEL=GND internally) | -40..+85 °C | 2.25-3.6 V |
| XR16V564DIV | 64-lead LQFP 10x10 | 64 | lqfp64 (INTSEL=VCC internally) | -40..+85 °C | 2.25-3.6 V |
Ordering p4: IJ PLCC68, IV LQFP64, DIV LQFP64, IL QFN48, IV80 LQFP80; all -40..+85 °C, "Active" (but NRND
watermark). Census names are Sipex-era, no "-F"; same strings as the ordering table. All decoded.

### Pinouts (pin table p4-8, drawings p2-3)
Identical to XR16V554 (see that section: plcc68, lqfp64, qfn48 tables) **except**:
- plcc68: pin 30 = CLKSEL input (XR16V554: VCC). VCC = 13, 47, 64. GND = 6, 23, 40, 57.
- lqfp64: pin 21 = CLKSEL input (XR16V554: VCC). VCC = 4, 35, 52. GND = 14, 28, 45, 61.
- qfn48: no CLKSEL; identical to XR16V554 qfn48 (VCC 2, 24, 37; GND 21, 47 per table + pin 34 GND per
  drawing; EP = GND).
CLKSEL: "Baud-Rate-Generator Input Clock Prescaler Select for channels A-D. Only sampled during power up or
a reset. VCC = ÷1 (default), GND = ÷4. MCR[7] can override the state of this pin following reset" (p7).
(No pull-up/pull-down stated for CLKSEL.) 16/68 (pull-up), INTSEL (pull-down) as in V554.
TX pins: in IrDA mode (MCR[6]=1) the TX idle level is LOW; RX IrDA idle LOW (p6).

### Host interface
Identical to XR16V554 (Intel IOR/IOW + CSA-D; Motorola CS + R/W + A4:A3 on PLCC/QFN only; write data
latched on IOW rising edge; Table 1/2 p11).

### Registers (Table 9 p24, Table 10 p25-26, descriptions p26-38, reset Table 17 p39)
| A2-A0 | Condition | Name | R/W | Reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR / THR | R / W | XX |
| 001 | LCR[7]=0 | IER | R/W | 00 |
| 010 | LCR[7]=0 | ISR / FCR | R / W | 01 / 00 |
| 011 | any | LCR | R/W | 00 |
| 100 | LCR≠BF | MCR | R/W | 00 |
| 101 | LCR[7]=0 | LSR | R/W (write not recommended) | 60 |
| 110 | LCR[7]=0 | MSR | R/W (write not recommended) | [3:0]=0, [7:4]=inverted pins |
| 111 | LCR≠BF | SPR | R/W | FF |
| 000 | LCR[7]=1, LCR≠BF | DLL | R/W | 01 (power-up only; RESET pin does not change) |
| 001 | LCR[7]=1, LCR≠BF | DLM | R/W | 00 (power-up only) |
| 010 | LCR[7]=1, LCR≠BF, EFR[4]=1 | DLD | R/W | 00 |
| 010 | LCR=0xBF | EFR | R/W | 00 |
| 100 | LCR=0xBF | XON1 | R/W | 00 |
| 101 | LCR=0xBF | XON2 | R/W | 00 |
| 110 | LCR=0xBF | XOFF1 | R/W | 00 |
| 111 | LCR=0xBF | XOFF2 | R/W | 00 |
Notes on access: Table 9 lists MCR/LSR/MSR/SPR only for LCR[7]=0 (MCR/SPR show no condition); with
LCR=0xBF the 100-111 addresses are Xon/Xoff, 010 is EFR, 000/001 not listed (LCR=0xBF has LCR[7]=1 but
DLL/DLM are specified "LCR≠0xBF" → reads at 000/001 with LCR=BF are unspecified). With LCR[7]=1 & ≠BF,
addresses 011 LCR; 100-111 unspecified in table (16C550 practice: MCR/LSR/MSR/SPR).

Bits ("enhanced" bits writable only while EFR[4]=1; otherwise they keep their latched value, and are 0
after reset):
- IER: b0 RX data/timeout; b1 TX ready (non-FIFO: THR empty; FIFO: TX FIFO below TX trigger / empty; if
  already empty when set → immediate int); b2 RX line status; b3 modem status; **b4 sleep mode enable**
  (EFR[4]); **b5 Xoff / special char int enable** (EFR[4]); **b6 RTS int enable** (RTS pin LOW→HIGH under
  auto-RTS) (EFR[4]); **b7 CTS int enable** (CTS pin LOW→HIGH under auto-CTS) (EFR[4]) (p27-28).
- ISR: b0 0=pending; b3-1 code; b4 Xoff/special char detected (EFR[4]); b5 CTS/RTS change LOW→HIGH
  (EFR[4]); b7-6 = 11 if FIFO enabled.
- FCR (W): b0 FIFO enable (must be 1 for other bits to take); b1 RX FIFO reset (self-clear); b2 TX FIFO reset;
  b3 DMA mode; **b5-4 TX trigger (EFR[4]=1)**; b7-6 RX trigger (Table 12 p30, verified on page image):
  | bits | RX trigger (FCR[7:6]) | TX trigger (FCR[5:4]) |
  |---|---|---|
  | 00 | 8 (default) | 16 (default) |
  | 01 | 16 | 8 |
  | 10 | 24 | 24 |
  | 11 | 30 | 30 |
  TX trigger semantics: TX interrupt when FIFO count falls below the TX trigger level, or when it empties
  if it was never filled above the trigger on the last reload (p30).
- LCR: as 16C550 (same as XR16V554). Writing 0xBF selects the enhanced bank. Note p32 text misprints the
  force-parity line (both say LCR[4]=LOW); Table 13: b5=1,b4=0 → parity 1; b5=1,b4=1 → parity 0.
- MCR: b0 DTR (1→pin LOW); b1 RTS (1→pin LOW; must be 1 to start auto-RTS); b2 OP1 (internal only; RI in
  loopback); b3 INT output enable / OP2 (CD in loopback) — Table 14 as in V554; b4 loopback; **b5 Xon-any**
  (EFR[4]): any RX char resumes a suspended transmitter; the char goes into the FIFO unless it is an
  Xon/Xoff char under active software flow control; **b6 IrDA enable** (EFR[4]); **b7 clock prescaler**
  (EFR[4]): 0=÷1, 1=÷4 (p33). CLKSEL pin presets MCR[7] at power-up/reset (Table 17 lists MCR=00 — conflict
  with CLKSEL=GND giving ÷4; see Notes).
- LSR: identical to V554 (b0 DR, b1 OE, b2 PE, b3 FE, b4 BI, b5 THRE, b6 TEMT, b7 RX FIFO error).
- MSR: identical to V554 (b0 ΔCTS, b1 ΔDSR, b2 TERI, b3 ΔCD, b4-7 CTS/DSR/RI/CD complement or loopback
  MCR[1]/[0]/[2]/[3]).
- SPR scratch (reset FF).
- DLL/DLM: integer divisor. DLD: b3-0 fractional divisor (n/16); **b5-4 sampling: 00=16x, 01=8x, 1X=4x**
  (Table 15 p36); b7-6 reserved. DLD requires EFR[4]=1 to access.
- EFR (LCR=0xBF): b3-0 software flow control select (Table 16 p37):
  - b3b2 (TX): 00 none, 10 send Xon1/Xoff1, 01 send Xon2/Xoff2, 11 send Xon1+Xon2 / Xoff1+Xoff2 (two
    sequential chars).
  - b1b0 (RX): 00 none, 10 compare Xon1/Xoff1, 01 compare Xon2/Xoff2, 11 → combined with b3b2:
    1011 = TX Xon1/Xoff1, RX compares Xon1 **or** Xon2, Xoff1 or Xoff2; 0111 = TX Xon2/Xoff2, RX compares
    Xon1 or Xon2, Xoff1 or Xoff2; 1111 = TX both, RX compares Xon1 **and** Xon2 (sequence), Xoff1 and Xoff2;
    0011 = no TX flow control, RX compares Xon1 and Xon2, Xoff1 and Xoff2. ("Always reset all bits to 0
    before programming a new setting.")
  - b4 enhanced function enable (gate for IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5], DLD); clearing it latches
    the enhanced values.
  - b5 special character detect: compare each RX char with XOFF2; on match char goes into FIFO and ISR[4]
    set. If EFR[1:0]=01 (compare Xon2/Xoff2), Xoff2 is a flow-control char: not stored, generates Xoff and
    special char interrupt (if IER[5]) (p37).
  - b6 auto-RTS enable; b7 auto-CTS enable (p38).
- XON1, XON2, XOFF1, XOFF2: flow-control characters; compare uses only the LCR[1:0] word length bits
  (bit 0 = first received bit) (p20).

ISR priority (Table 11 p29) — ISR[5:0]:
| Prio | ISR[5:0] | Source | Clear (p28) |
|---|---|---|---|
| 1 | 000110 | RX line status | read LSR |
| 2 | 001100 | RX timeout | read RHR |
| 3 | 000100 | RX data ready (trigger) | read below trigger |
| 4 | 000010 | TX ready (below TX trigger / empty) | read ISR or write THR |
| 5 | 000000 | modem status | read MSR |
| 6 | 010000 | RX Xoff or special char | read ISR, or Xon received (Xoff) / next char received (special) |
| 7 | 100000 | CTS or RTS LOW→HIGH | read MSR |
| – | 000001 | none | |

### Behavior
- **Baud generator** (p13-15): XTAL1 → prescaler (÷1 or ÷4, MCR[7]) → fractional BRG divisor
  N = DLM:DLL + DLD[3:0]/16 (1 .. 65535.9375) → sampling clock of 16x/8x/4x (DLD[5:4]).
  Baud = XTAL1 / prescaler / (sampling × N). In 8x mode with odd non-zero DLD the bit time jitters ±1/16
  (p14). Table 6 (p15, 24 MHz, 16x): e.g. 9600 → DLM 00 DLL 9C DLD 4; 115200 → 0D/0; 921600 → 1/A.
  Divisor 0 behavior not stated. Max 16 Mbps (64 MHz ext clock, 4x) (p1, p9). No BAUDOUT pin.
- **Transmitter / receiver**: as XR16V554 but bit time = 16/8/4 sampling clocks; start bit checked after 8
  (16x), 4 (8x), 2 (4x) clocks (p17). RX timeout 4 word lengths + 12 bit times (p17). Overrun, error tags,
  break (one 0x00 char loaded), LSR[7] as V554.
- **FIFOs**: 32 TX / 32 RX (11 bits). TXRDY/RXRDY rules (Table 5 p13) identical to V554 (mode 1 TXRDY low =
  ≥1 empty location, high = full; mode 1 RXRDY low at RX trigger or timeout, high when empty).
- **Auto RTS** (EFR[6], p18 Table 7): RTS pin must first be asserted via MCR[1]=1. RX data interrupt at
  trigger; RTS forced HIGH when FIFO reaches the next-higher trigger table level; RTS returns LOW when FIFO
  falls to the next-lower level:
  | RX trigger | INT at | RTS HIGH at | RTS LOW at |
  |---|---|---|---|
  | 8 | 8 | 16 | 0 |
  | 16 | 16 | 24 | 8 |
  | 24 | 24 | 30 | 16 |
  | 30 | 30 | 30 | 24 |
  RTS interrupt (IER[6]) when RTS pin goes LOW→HIGH; ISR[5].
- **Auto CTS** (EFR[7], p19): when CTS pin HIGH, transmitter stops after current char's stop bit; resumes
  when CTS LOW. CTS interrupt (IER[7]) on CTS HIGH (deassert); ISR[5].
- **Auto Xon/Xoff** (p20 Table 8): RX side compares received chars (1 or 2 sequential per EFR) to Xoff →
  TX halts after current char, ISR[4] Xoff int if IER[5]; Xon match resumes and clears ISR[4]. Flow
  chars are not stored in the FIFO. TX side: when RX FIFO reaches trigger, sends Xoff char(s) **2 character
  times after** the trigger is crossed; sends Xon char(s) when FIFO drops to one trigger level below:
  trigger 8 → Xon at 0; 16 → 8; 24 → 16; 30 → 24.
- **Special character** (EFR[5], p20, p37) and **Xon-any** (MCR[5]) as above.
- **IrDA 1.0** (MCR[6], p21): TX emits a HIGH pulse 3/16 bit wide for each 0 bit (data 1 → no pulse); TX idle
  LOW. RX decoder: a HIGH pulse on RX = 0 bit ("each time it senses a light pulse, it returns HIGH to the
  data bit stream" — i.e. pulse → logic 0 data, figure 12 shows 1/16-clock delay); RX idles LOW.
- **Sleep** (p22): enter when ALL four channels have IER[4]=1, no interrupt pending in any channel
  (ISR[0]=1), MSR[3:0]=0, RX pins HIGH → crystal oscillator stops (XTAL2 no output). Wake on RX start bit
  (H→L), write to THR/FIFO, or change on CTS/DSR/CD/RI. Re-sleeps automatically once all interrupts are
  cleared; stays enabled until IER[4]=0. After modem-input wake, MSR read needed. First RX chars may be lost
  while the oscillator restarts.
- **Interrupts / INT pins**: as V554 (Table 14 p33): INTSEL/MCR[3] → 3-state or active-HIGH in 16 mode;
  68 mode single active-low open-drain IRQ on INTA.
- **Loopback** (MCR[4], p22-23): as V554 (TX pin HIGH, RTS/DTR HIGH, modem inputs ignored; MSR[4..7] ←
  MCR[1],[0],[2],[3]).
- **Reset** (Table 17 p39; pin p7): RESET active-high (16 mode) / active-low (68 mode), ≥40 ns. Registers:
  DLD=00, IER=00, FCR=00, ISR=01, LCR=00, MCR=00, LSR=60, MSR[3:0]=0, SPR=FF, EFR/XON1/XON2/XOFF1/XOFF2=00;
  DLL/DLM reset only at power-up. Outputs: TX HIGH, RTS HIGH, DTR HIGH, RXRDY HIGH, TXRDY LOW, INT 3-state
  (XR16V564 with INTSEL low) or LOW (INTSEL high / XR16V564D); IRQ (68 mode) 3-state (V554 sheet says HIGH).

### Timing (p41-42; min/max only; 2.5 V±10% / 3.3 V±10%, 70 pF)
Identical to XR16V554 table except **TDS (data setup, 16 mode) = 15 ns min** (both supplies) (V554: 10).
Values: XTAL ≤24 MHz; ECLK ≤50 (2.5 V)/64 (3.3 V) MHz; TCS/TRD/TDY/TWR 50/40 ns; TRDV 45/35; TDD 10;
TDH 5; TAS/TAH 0; 68 mode TRDA 45/35, TRDH 10, TWDS 10, TWDH 5, TRWH 10/5, TCSL/TCSD 50/40; TWDO 50; TMOD 50;
TRSI 50; TSSI 1 Bclk; TRRI 45; TSI 45; TINT 8-24 Bclk; TWRI 45; TSSR 1 Bclk; TRR 45; TWT 45; TSRT 8 Bclk;
TRST ≥40 ns. Bclk = 16x/8x/4x sampling clock.

### VCC
2.25-3.6 V, -40..+85 °C (p1, p40). Inputs 5 V tolerant except XTAL1 (p7; VIH max 5.5 V p40) → inputMax 5.5 V.
(Abs-max table p40 says "Voltage at any pin GND-0.3 V to 4 V" — contradicts 5 V tolerance; V554 sheet says
5.5 V.) ISLEEP max 350 µA (2.5 V) / 450 µA (3.3 V).

### Package drawings (p50-52) — identical numbers to XR16V554
- QFN48 7x7x0.9 (p50): e 0.50, b 0.18-0.30, L 0.30-0.50, D 6.85-7.15, D2 5.10-5.30.
- LQFP64 10x10 (p51): D 11.80-12.20, D1 9.90-10.10, e 0.50, B 0.17-0.27, L 0.45-0.75.
- PLCC68 (p52, inch control): D 25.02-25.27, D1 24.13-24.33, D2 22.61-23.62, e 1.27, B 0.33-0.53.
No land patterns printed.

### Obsolete
None in census (all Active). Datasheet watermarked NRND.

### Notes
- Reset table says MCR=0x00, but CLKSEL=GND is said to select ÷4 at reset — so MCR[7] (or the internal
  prescaler) reflects CLKSEL after reset. Recommend: after reset, prescaler = ÷4 if CLKSEL low; MCR[7]
  readback unspecified.
- MCR[7] overriding CLKSEL needs EFR[4]=1.
- Abs-max pin voltage 4 V vs 5 V tolerant inputs (see VCC).

## XR16V654

- Title: "XR16V654/654D — 2.25V TO 3.6V QUAD UART WITH 64-BYTE FIFO", Exar Rev 1.0.2 Sept 2007.
  Datasheet: `datasheets/controller-uart/xr16v654.pdf` (58 pp; pinouts p2-4, ordering p5, pins p5-10,
  function p11-25, registers p26-41, DC p42, AC p43-44, timing figs p45-50, packages p51-55).
- Function: identical architecture to XR16V564 (enhanced ST16C654-class quad UART: EFR/Xon/Xoff, auto
  RTS/CTS with hysteresis, auto Xon/Xoff, special char, Xon-any, IrDA 1.0, sleep, DLD fractional divisor with
  16x/8x/4x sampling, MCR[7]/CLKSEL prescaler) but with **64-byte TX/RX FIFOs** and 64-byte trigger tables.
  The 100-QFP package (XR16V654IQ, not in census) adds per-channel TXRDYA-D/RXRDYA-D, IRTXA-D, CHCCLK and
  FSRS/FSTAT. Pin compatible with ST16C654/654D, TL16C754B, SC16C654B (p1).
- Diff vs XR16V564 (verified by text diff of register chapters): FIFO 64; RX trigger 8/16/56/60; TX trigger
  8/16/32/56 (default 8 not 16); auto-RTS/Xoff hysteresis tables re-scaled; LSR and MSR are **read-only**
  (V564: "writeable, not recommended"); FSTAT register (100-QFP only); TWDS(68 mode, 3.3 V) 15 ns; TDS 10 ns.
  Everything else (bit definitions, interrupts, sleep, IrDA, loopback, reset) is word-for-word the V564 text.
- XR16V654 vs XR16V654D: same die; 64-LQFP only: INTSEL bonded GND (IV) / VCC (DIV) (p7, p11). One device.

### Parts (census)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| XR16V654IV-F | 64-lead LQFP 10x10 | 64 | lqfp64 (INTSEL=GND internally) | -40..+85 °C | 2.25-3.6 V |
| XR16V654DIV-F | 64-lead LQFP 10x10 | 64 | lqfp64 (INTSEL=VCC internally) | -40..+85 °C | 2.25-3.6 V |
| XR16V654IL-F | 48-pin QFN 7x7, exposed pad | 48 | qfn48 | -40..+85 °C | 2.25-3.6 V |
Ordering p5 lists XR16V654IJ (PLCC68), IV, DIV, IQ (100-QFP), IL, IV80 (all -40..+85 °C, Active) without
"-F"; "-F" is MaxLinear's RoHS/lead-free suffix (not explained in this sheet). All census parts decoded.
DigiKey status for all three: "Not For New Designs".

### Pinouts (pin table p5-10)
- **lqfp64** and **qfn48**: pin numbers identical to XR16V564 (and to XR16V554 except LQFP64 pin 21 =
  CLKSEL). Full list: see XR16V554 lqfp64/qfn48 tables with LQFP pin 21 = CLKSEL input.
  LQFP64: VCC 4, 35, 52; GND 14, 28, 45, 61. QFN48: VCC 2, 24, 37; GND 21, 47 (+ pin 34 GND per drawing p4 —
  same omission as V554/V564 tables); EP = GND (p10). QFN48 has no CLKSEL (prescaler defaults ÷1 — inferred,
  not stated), no DTR/DSR/CD/RI, no TXRDY/RXRDY.
- 16/68 (QFN pin 14, pull-up), INTSEL (QFN pin 38, pull-down), 68-mode pin meanings identical to V554.
- 100-QFP (not in census) extras: TXRDYA-D 5/25/56/81, RXRDYA-D 100/31/50/82, IRTXA-D 6/24/57/75 (always
  IrDA-encoded regardless of MCR[6], idle LOW), CHCCLK 42 (channel C clock; tie to XTAL2 or ext 16x clock),
  FSRS 76 (active-low, pull-up: drives FSTAT onto D7..D0 with no address needed) (p7-9).

### Host interface
Identical to XR16V554/V564 (Intel 16 mode: CSA-D active low, IOR/IOW, write latched on IOW rising edge;
Motorola 68 mode on QFN48/PLCC68/QFP100: CS, R/W, A4:A3 channel select) (p12-13).

### Registers (Table 9 p26, Table 10 p27-28, reset Table 17 p41)
Same map, bits, access conditions and reset values as XR16V564 (RHR/THR, IER, ISR/FCR, LCR, MCR, LSR, MSR,
SPR; DLL/DLM/DLD with LCR[7]=1 & ≠BF (DLD also EFR[4]=1); EFR/XON1/XON2/XOFF1/XOFF2 at LCR=0xBF), plus:
| A2-A0 | Condition | Name | R/W | Reset |
|---|---|---|---|---|
| XXX | FSRS pin LOW (100-QFP only) | FSTAT | R | FF |
FSTAT: b3-0 = inverted TXRDYD..A pins (b0=A); b7-4 = RXRDYD..A pin levels (b4=A) (p28, p40).
LSR (101) and MSR (110) are Read-only here.
FCR trigger table (Table 12 p32; layout same as the V564 page whose image was checked — first block is
FCR[5:4] TX, second FCR[7:6] RX; consistent with the auto-RTS table below):
| bits | RX trigger FCR[7:6] | TX trigger FCR[5:4] |
|---|---|---|
| 00 | 8 (default) | 8 (default) |
| 01 | 16 | 16 |
| 10 | 56 | 32 |
| 11 | 60 | 56 |
ISR codes/priority (Table 11 p31) identical to V564 (0x06, 0x0C, 0x04, 0x02, 0x00, 0x10 Xoff/special,
0x20 CTS/RTS, 0x01 none).

### Behavior
All as XR16V564 (baud generator with prescaler + DLD + 16/8/4x, TX/RX, RX timeout 4 words + 12 bits,
TXRDY/RXRDY Table 5 p15, auto CTS, Xon/Xoff (Xoff sent 2 char times after trigger), special char,
Xon-any, IrDA 3/16 pulse, sleep (all 4 channels IER[4]=1, oscillator stops), loopback, INT/INTSEL/MCR[3]
Table 14 p35) with these 64-byte tables:
- Auto RTS hysteresis (Table 7 p20):
  | RX trigger | INT at | RTS HIGH at | RTS LOW at |
  |---|---|---|---|
  | 8 | 8 | 16 | 0 |
  | 16 | 16 | 56 | 8 |
  | 56 | 56 | 60 | 16 |
  | 60 | 60 | 60 | 56 |
- Auto Xoff/Xon (Table 8 p22): trigger 8: Xoff at 8 (+2 char times), Xon at 0; 16: 16 / 8; 56: 56 / 16;
  60: 60 / 56.
- Reset (Table 17 p41): as V564 plus FSTAT=FF, IRTX pins LOW; TX HIGH, RTS/DTR HIGH, RXRDY HIGH, TXRDY LOW,
  INT 3-state (INTSEL low) / LOW (INTSEL high, or 654D); IRQ (68 mode) 3-state. DLL/DLM power-up only.
- CHCCLK (100-QFP): channel C's BRG input clock comes from this pin, not the oscillator (p9).

### Timing (p43-44; min/max; 2.5 V±10% / 3.3 V±10%)
Same as XR16V554 table (TDS 10 ns, TDH 5) except TWDS (68 mode write data setup) = 10 ns (2.5 V) / **15 ns
(3.3 V)**. XTAL ≤24 MHz; ECLK ≤50/64 MHz; TRDV 45/35 ns; TCS/TRD/TWR/TDY 50/40; TDD 10; TRST ≥40 ns;
TWDO/TMOD/TRSI 50; TRRI/TSI/TWRI/TRR/TWT 45; TSSI/TSSR 1 Bclk; TSRT 8 Bclk; TINT 8-24 Bclk.

### VCC
2.25-3.6 V, -40..+85 °C. Inputs 5 V tolerant except XTAL1 (VIH max 5.5 V, p42) → inputMax 5.5 V; abs-max
table again says pins -0.3..4 V (p42, contradiction). DC levels p42 = same as V554 (VIH 1.8/2.0 V).
ISLEEP ≤350/450 µA.

### Package drawings
- QFN48 7x7x0.9 (p51): e 0.50, b 0.18-0.30, L 0.30-0.50, D 6.85-7.15, D2 (pad) 5.10-5.30, A 0.80-1.00.
- LQFP64 10x10x1.4 (p52): D 11.80-12.20, D1 9.90-10.10, e 0.50, B 0.17-0.27, L 0.45-0.75, A ≤1.60.
- (PLCC68 p53, LQFP80 p54, QFP100 14x20 p55: D 23.65-24.15, E 17.65-18.15, e 0.65 — not census.)
No land patterns printed.

### Obsolete
None (DigiKey "Not For New Designs", not obsolete).

### Notes
- QFN48 GND pin 34 missing from pin table (same as V554).
- TX trigger default text says 8 (V564 says 16) — consistent with the table here.
- CLKSEL/MCR[7] reset conflict as in V564.

## XR16V798

- Title: "XR16V798 — HIGH PERFORMANCE 2.25V TO 3.6V OCTAL UART WITH FRACTIONAL BAUD RATE", Exar Rev 1.0.1
  July 2008; pages watermarked NRND. Datasheet: `datasheets/controller-uart/xr16v798.pdf` (56 pp; pinout +
  ordering p2, pins p3-6, function p7-20, register map p20-30, register bits p30-45, DC p46, AC p47-48,
  timing figs p49-52, package p53).
- Function: 8 enhanced 16550 UARTs in one flat 256-byte address space (A7..A0), single open-drain INT#,
  global interrupt source registers INT0-INT3, 16-bit general-purpose timer/counter (TMRCK or oscillator),
  per-channel 8X/16X select (8XMODE), soft reset per channel, SLEEP per channel, REGB simultaneous-write,
  DVID=0x48 / DREV. Per channel: 64-byte TX/RX FIFOs, TX/RX FIFO level counters (TXCNT/RXCNT), programmable
  trigger levels (TXTRG/RXTRG, Table-D) or tables A/B/C, auto RTS/CTS **or** DTR/DSR flow control with
  programmable hysteresis, auto Xon/Xoff, special char, Xon-any, send-char-immediate, TX/RX disable, auto
  RS-485 direction control with 0-15 bit turnaround delay, IrDA 1.0 (ENIR pin preset, RX invert option),
  fractional divisor DLD, ÷1/÷4 prescaler. Pin compatible with XR16L788 (p1). No TXRDY/RXRDY pins, no
  per-channel INT pins, no INTSEL/CLKSEL.
- This is a different design from the XR16V554/564/654 quads (flat address map, device config block,
  16-register channel map); closest relative in this group: XR16L784 (quad, same "5G flat" architecture).

### Parts (census)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| XR16V798IQ-F | 100-lead QFP 14x20x3 mm (1.95 mm form) | 100 | qfp100 | -40..+85 °C | 2.25-3.6 V |
Ordering p2: only XR16V798IQ, 100-Lead QFP, -40..+85 °C, Active (no "-F" in sheet; -F = RoHS suffix).
Decoded. DigiKey: Not For New Designs.

### Pinout — qfp100 (pin table p3-6; drawing p2 verified for bus bit order)
```
1 RST input       26 A1 input       51 DSR5 input     76 CD2 input
2 16/68 input     27 A0 input       52 CD5 input      77 RI2 input
3 VCC power       28 VCC power      53 RI5 input      78 RTS2 output
4 GND ground      29 GND ground     54 RTS5 output    79 DTR2 output
5 D7 inout        30 CS input       55 DTR5 output    80 TX2 output
6 D6 inout        31 RX7 input      56 TX5 output     81 XTAL2 output
7 D5 inout        32 CTS7 input     57 RX4 input      82 XTAL1 input
8 D4 inout        33 DSR7 input     58 CTS4 input     83 GND ground
9 D3 inout        34 CD7 input      59 DSR4 input     84 VCC power
10 D2 inout       35 RI7 input      60 CD4 input      85 TX1 output
11 D1 inout       36 RTS7 output    61 RI4 input      86 DTR1 output
12 D0 inout       37 DTR7 output    62 RTS4 output    87 RTS1 output
13 IOW input      38 TX7 output     63 DTR4 output    88 RI1 input
14 TMRCK input    39 RX6 input      64 TX4 output     89 CD1 input
15 ENIR input     40 CTS6 input     65 RX3 input      90 DSR1 input
16 INT output(OD) 41 DSR6 input     66 CTS3 input     91 CTS1 input
17 VCC power      42 CD6 input      67 DSR3 input     92 RX1 input
18 GND ground     43 RI6 input      68 CD3 input      93 TX0 output
19 IOR input      44 RTS6 output    69 RI3 input      94 DTR0 output
20 A7 input       45 DTR6 output    70 RTS3 output    95 RTS0 output
21 A6 input       46 TX6 output     71 DTR3 output    96 RI0 input
22 A5 input       47 GND ground     72 TX3 output     97 CD0 input
23 A4 input       48 VCC power      73 RX2 input      98 DSR0 input
24 A3 input       49 RX5 input      74 CTS2 input     99 CTS0 input
25 A2 input       50 CTS5 input     75 DSR2 input     100 RX0 input
```
All modem pins active-low (RTSn = RTS#, etc.). INT# open-drain active-low, needs 47k-100k pull-up (p3).
Mode-dependent (16/68 pin, p3, p6): HIGH = Intel: IOR read strobe, IOW write strobe, CS chip select (all
active low). LOW = Motorola: IOW pin = R/W (H read, L write), CS = read/write strobe (active low), IOR
unused (tie VCC). No pull-ups specified on 16/68.

### Host interface (p3, p7, p47-50)
- 8-bit bus, A7..A0 flat: A7=0 → UART channel space; A6:A4 = channel 0-7, A3:A0 = channel register.
  A7=1 → device configuration registers 0x80-0x8F (A6:A4 = 000; 0x90-0xFF not described). Table 1 p7.
- Intel: CS low + IOR low → read; data driven while both low; CS low + IOW low → write; "the leading edge
  transfers the data byte" (p3; timing fig 16 p49 shows TWDS/TWDH around IOW rising edge → latch on IOW
  rising). Motorola: CS is the strobe; R/W sampled with TRWS setup before CS falls; write latched at CS
  rising (fig 17).
- REGB[0]=1: any channel-register write goes to all 8 channels (A6:A4 ignored); must be cleared before reads
  (p7).
- No ALE/DDIS/TXRDY/RXRDY.

### Device configuration registers (Table 8 p22, descriptions p22-27)
| Addr | Name | R/W | Reset | Bits |
|---|---|---|---|---|
| 0x80 | INT0 | R | 00 | bit n = channel n has an interrupt pending (1 = pending) |
| 0x81 | INT1 | R | 00 | 24-bit field {INT3,INT2,INT1}: channel n's 3-bit source code at bits [3n+2:3n]. INT1: b2-0 ch0, b5-3 ch1, b7-6 ch2[1:0] |
| 0x82 | INT2 | R | 00 | b0 ch2[2], b3-1 ch3, b6-4 ch4, b7 ch5[0] |
| 0x83 | INT3 | R | 00 | b1-0 ch5[2:1], b4-2 ch6, b7-5 ch7 |
| 0x84 | TIMERCNTL | R/W | 00 | write: b3-0 command (Table 10); read: 01 if timer interrupt pending, else 00 (read clears timer int) |
| 0x85 | TIMER | R | 00 | reserved, reads 00 |
| 0x86 | TIMERLSB | R/W | 00 | timer reload value bits 7-0 |
| 0x87 | TIMERMSB | R/W | 00 | timer reload value bits 15-8 (reads do not return the running count) |
| 0x88 | 8XMODE | R/W | 00 | bit n: 1 = channel n uses 8x sampling, 0 = 16x |
| 0x89 | REGA | R | 00 | reserved |
| 0x8A | RESET | W | 00 | bit n = 1: soft-reset channel n (self-clearing); channel registers → Table 19 defaults |
| 0x8B | SLEEP | R/W | 00 | bit n = 1: sleep enable for channel n |
| 0x8C | DREV | R | 01 | revision (01 = rev A) |
| 0x8D | DVID | R | 48 | device ID 0x48 (XR16L788 = 0x28) |
| 0x8E | REGB | R/W | 00 | b0 simultaneous write to all UARTs; b7-1 reserved |
| 0x8F | – | – | – | not listed |

3-bit per-channel interrupt code (Table 9 p23), priority 1 highest:
| code | prio | source | clear |
|---|---|---|---|
| 000 | – | none, or wake-up indicator | (wake-up: read ISR) |
| 001 | 1 | RXRDY and RX line status (OR of LSR[4:1]) | read RX below trigger; LSR read |
| 010 | 2 | RX timeout | as RXRDY |
| 011 | 3 | TXRDY (THR/TX FIFO, or TSR in auto-RS485) | read ISR |
| 100 | 4 | MSR, RTS/CTS or DTR/DSR delta, Xon/Xoff/special char | MSR read (first two); ISR read (Xoff etc.) |
| 101, 110 | – | reserved | |
| 111 | 7 | timer time-out (channel 0 only; reserved for others) | read TIMERCNTL |

Timer commands (TIMERCNTL[3:0] write, Table 10 p24; self-clearing): 0001 enable timer int; 0010 disable
timer int; 0011 one-shot; 0100 re-triggerable; 0101 clock = oscillator; 0110 clock = TMRCK pin; 0111,1000
reserved; 1001 start; 1010 stop; 1011 reset timer; 1100-1111 reserved (text also says "1011 to 1111 reserved"
— contradicts 1011 = reset). N = TIMERMSB:LSB, valid 0x0002-0xFFFF. One-shot: interrupt N clocks after
start. Re-triggerable: interrupt every N clocks (p25). Table 8 on p22 instead labels TIMERCNTL bits
b3 "clock source", b2 "function select", b1 "start timer", b0 "enable timer INT" — conflicting legacy
(XR16L788-style) description; the command table on p24 is the detailed one.

### UART channel registers (Table 11 p28, Table 12 p29-30; A3..A0 within channel)
| A3-A0 | Condition | Name | R/W | Reset |
|---|---|---|---|---|
| 0000 | LCR[7]=0 | RHR / THR | R / W | XX |
| 0000 | LCR[7]=1 | DLL | R/W | 01 |
| 0001 | LCR[7]=1 | DLM | R/W | 00 |
| 0010 | LCR[7]=1 | DLD | R/W | 00 (b3-0 fraction, b7-4 read 0) |
| 0001 | LCR[7]=0 | IER | R/W | 00 |
| 0010 | LCR[7]=0 | ISR / FCR | R / W | 01 / 00 |
| 0011 | – | LCR | R/W | 00 |
| 0100 | – | MCR | R/W | 00 (b6 = ENIR sample) |
| 0101 | – | LSR | R (table says R/W; text "Read only") | 60 |
| 0110 | – | MSR (read) / RS485 delay + TX/RX disable (write) | R / W | [3:0]=0, [7:4]=inputs |
| 0111 | – | SPR | R/W | FF (preserved through sleep) |
| 1000 | – | FCTR | R/W | 00 |
| 1001 | – | EFR | R/W | 00 |
| 1010 | – | TXCNT (R) / TXTRG (W) | | 00 / 00 |
| 1011 | – | RXCNT (R) / RXTRG (W) | | 00 / 00 |
| 1100 | – | XCHAR (R) / XOFF1 (W) | | 00 / 00 |
| 1101 | – | reserved (R) / XOFF2 (W) | | 00 |
| 1110 | – | reserved (R) / XON1 (W) | | 00 |
| 1111 | – | reserved (R) / XON2 (W) | | 00 |
No LCR=0xBF bank: enhanced registers live at 8-F directly. "Shaded" enhanced bits need EFR[4]=1 to be
written: IER[7:5], ISR[5:4], FCR[5:4], MCR[7:5] and [3:2], MSR-write[7:2] (p42).

Bits:
- IER: b0 RX data/timeout; b1 TX ready (FIFO mode: interrupt twice — when TX FIFO falls below trigger and
  again when empty; auto-RS485: second one delayed until TSR empty); b2 RX line status — fires immediately
  when a char with error is received (only LSR[7] shows if it isn't at top) and again when that char reaches
  the top (p31 example); b3 modem status; **b4 reserved** (sleep is in SLEEP reg); b5 Xon/Xoff/special
  char int; b6 RTS/DTR int (pin L→H); b7 CTS/DSR int (pin L→H).
- ISR: b0 0=pending; b5-1 code (Table 13 p32); b7-6 = 11 when FIFOs enabled.
- FCR: b0 FIFO enable (required for other bits); b1 RX FIFO reset; b2 TX FIFO reset; b3 DMA mode — no
  effect (no TXRDY/RXRDY pins); b5-4 TX trigger; b7-6 RX trigger (tables selected by FCTR[7:6]); "the
  receiver and transmitter cannot use different trigger tables; whichever selection is made last applies to
  both" (p33).
- LCR: 16C550 (b1-0 word 5-8, b2 stop 1/1.5/2, b3 PEN, b4 EPS, b5 stick (b4=0→1, b4=1→0), b6 break,
  b7 DLAB).
- MCR: b0 DTR (1 → pin LOW); b1 RTS (1 → pin LOW); b2 auto-flow pin select: 0 = RTS/CTS, 1 = DTR/DSR (OP1
  in loopback → MSR[6] RI); b3 **send char immediate** (self-clearing; the next access must be the THR write;
  that byte is sent ahead of the FIFO right after the char in TSR; Xon/Xoff have higher priority; needs
  MSR-write[3]=0) — in loopback acts as OP2 → MSR[7] CD, must be 0 when leaving loopback; b4 loopback;
  b5 Xon-any; b6 IrDA (preset from ENIR pin at power-up / RST# rising edge / soft reset); b7 prescaler
  ÷1/÷4 (p36-37).
- LSR: b0 DR; b1 OE; b2 PE (top of FIFO); b3 FE; b4 BI; b5 TX FIFO empty (THR empty in non-FIFO); b6 TX FIFO
  and TSR empty; b7 RX FIFO contains an error (p37-38).
- MSR read: b0 ΔCTS, b1 ΔDSR, b2 TERI (RI L→H at pin... "changed from logic 0 to logic 1"), b3 ΔCD; b4 CTS,
  b5 DSR, b6 RI, b7 CD (complement of pins; loopback: RTS(MCR1), DTR(MCR0), OP1(MCR2), OP2(MCR3)).
- MSR write (p39-40): b7-4 auto-RS485 turnaround delay 0-15 bit times (value = delay; Table 16 row "9 1 0 1"
  is a typo for 0101=5); b3 TX disable (TSR halts; FIFO keeps data); b2 RX disable (finishes current char —
  or receives one more full char if idle — then stops; re-enabling while RX is toggling may fill FIFO with
  unknown data); b1-0 reserved. Enabled by EFR[4].
- SPR: scratch.
- FCTR: b7-6 trigger table A/B/C/D; b5 auto-RS485 enable; b4 IrDA RX invert (1 = active-low IR pulses);
  b3-0 RTS/DTR hysteresis for table D (Table 17 p41): 0000 0 (= "next trigger level"? see Notes), 0001 ±4,
  0010 ±6, 0011 ±8, 0100 ±8, 0101 ±16, 0110 ±24, 0111 ±32, 1100 ±12, 1101 ±20, 1110 ±28, 1111 ±36,
  1000 ±40, 1001 ±44, 1010 ±48, 1011 ±52.
- EFR: b3-0 software flow control (Table 18 p43 — identical encoding to XR16V564 EFR); b4 enhanced-bit write
  enable/latch; b5 special char detect (XOFF2; same rules as V564); b6 auto RTS/DTR; b7 auto CTS/DSR.
- TXCNT (R): TX FIFO count 0-64. TXTRG (W): TX trigger 0-64 (table D; interrupt when level falls below).
  RXCNT (R): RX FIFO count 0-64. RXTRG (W): RX trigger 0-64 (interrupt when level rises to it).
- XCHAR (R, each bit self-clears on read; whole reg cleared when software flow control is disabled):
  b3 last sent control char was Xon; b2 last sent was Xoff; b1 last received was Xon; b0 last received was
  Xoff; b7-4 reserved (p44).
- XOFF1/XOFF2/XON1/XON2: write-only.

Trigger tables (Table 14 p34), FCTR[7:6] selects:
| Table | FCTR[7:6] | RX trigger FCR[7:6]=00/01/10/11 | TX trigger FCR[5:4]=00/01/10/11 |
|---|---|---|---|
| A (16C550) | 00 | 1 / 4 / 8 / 14 | 1 (only value; FCR[5:4] ignored) |
| B (16C650A) | 01 | 8 / 16 / 24 / 28 | 16 / 8 / 24 / 30 |
| C (16C654) | 10 | 8 / 16 / 56 / 60 | 8 / 16 / 32 / 56 |
| D | 11 | RXTRG register | TXTRG register |
(Column assignment for B/C inferred from the Xon table (Table 6: RX 8/16/24/28) and the V564/V654 sheets;
the table text interleaves TX and RX columns. Default after reset = table A, RX trigger 1, TX trigger 1.)

ISR priority (Table 13 p32) — note RX data is priority 2 and timeout 3 here:
| Prio | ISR[5:0] | Source |
|---|---|---|
| 1 | 000110 | LSR |
| 2 | 000100 | RX data ready |
| 3 | 001100 | RX timeout |
| 4 | 000010 | TX ready |
| 5 | 000000 | MSR |
| 6 | 010000 | Xon/Xoff/special char |
| 7 | 100000 | CTS/DSR or RTS/DTR change |
| – | 000001 | none or wake-up indicator |
Clearing (p32): LSR int by LSR read; RX data by reading below trigger; timeout by RHR read; TX by ISR read or
THR write; MSR by MSR read; Xon/Xoff by ISR read; special char by ISR read or next char received; RTS/CTS
change by MSR read.

### Behavior
- **Clock/baud** (p8-10): one oscillator (XTAL1/XTAL2, crystal ≤24 MHz, ext clock ≤50 MHz @2.5 V / 64 MHz
  @3.3 V) feeds all 8 BRGs and the timer. Baud = XTAL1 / prescaler(1|4, MCR[7]) / (16 or 8 per 8XMODE bit) /
  (DLM:DLL + DLD[3:0]/16). 8x mode with odd DLD → ±1/16 bit jitter. Max 8 Mbps (8x) / 4 Mbps (16x) (p7).
- **TX/RX** as 16550: LSB first, start/5-8 data/parity/1-2 stop; start validated at mid-bit (8 of 16 or 4 of
  8 clocks); RX timeout 4 char times + 12 bits; overrun keeps FIFO intact; break loads one 0x00 char.
- **Auto RTS/DTR** (p15, p40): MCR[2] selects RTS/CTS or DTR/DSR. Must assert pin first (MCR[1] or [0]=1).
  Tables A-C: pin deasserts (HIGH) when RX FIFO reaches the next trigger level above the selected one and
  reasserts (LOW) when it falls to the next level below (example p40: table C, trigger 56 → HIGH at 60, LOW
  at 16). Table D: HIGH at RXTRG + hysteresis, LOW below RXTRG − hysteresis (FCTR[3:0]).
- **Auto CTS/DSR**: TX stops after the current char's stop bit while pin HIGH; resumes when LOW.
- **Xon/Xoff** (p16-17): Xoff sent 2 char times after RX FIFO crosses trigger; Xon sent when below one level
  under trigger (tables A-C) or below trigger − hysteresis (table D). Table 6 (table B): trigger 8 → Xon at 0;
  16 → 8; 24 → 16; 28 → 24. Flow chars not stored in FIFO.
- **Auto RS-485** (FCTR[5], p17, p40): the selected pin (RTS, or DTR — "RTS# or DTR#") goes HIGH when a byte
  is loaded into THR/FIFO, before the start bit; goes LOW after the last stop bit of the last char plus the
  MSR[7:4] delay (0-15 bit times). TX-empty interrupt becomes TSR-empty.
- **IrDA** (p17-18): MCR[6] (preset by ENIR). TX: 3/16-bit HIGH pulse per 0 bit, idle LOW. RX: a pulse = 0
  bit; FCTR[4]=1 accepts active-low pulses. While IrDA is on, the receiver is disabled while the
  transmitter is sending (echo suppression).
- **Sleep** (p19, p26): channel sleeps when its SLEEP bit = 1 and no interrupt pending. When all 8 asleep
  (SLEEP=0xFF, no interrupts, MSR[3:0]=0, all RX high) the oscillator stops. Wake on RX start bit, THR
  write, or modem input change; device ready after 32 crystal clocks; generates an interrupt — if the waking
  event's interrupt isn't enabled, a "wake-up" interrupt appears whose ISR reads "no interrupt" (01) and is
  cleared by that ISR read. Returns to sleep after interrupts are cleared; stays in sleep mode until
  SLEEP=00.
- **Loopback** (p20): TX pin HIGH, RTS/DTR pins HIGH, modem inputs ignored; TSR→RSR internal.
- **Reset** (p6-7, Table 19 p45): RST# active LOW ≥40 ns; **no power-on reset** — RST# must be pulsed at
  power-up. Soft reset per channel via RESET reg. Values: DLL=01, DLM=00, DLD=00, IER=00, FCR=00, ISR=01,
  LCR=00, MCR=00 (b6 from ENIR), LSR=60, MSR[3:0]=0 / [7:4]=input levels, SPR=FF, FCTR, EFR, TXCNT, TXTRG,
  RXCNT, RXTRG, XCHAR, XON1/2, XOFF1/2 = 00. Outputs: TX HIGH, RTS HIGH, DTR HIGH (table also lists
  "IRTX LOW" — no such pins on this part). INT# released (no pending interrupts). Device config reset values
  as in register table (all 00 except DREV 01, DVID 48).

### Timing (p47-48; min/max; columns 2.5 V and 3.3 V; header says 0-70 °C or -40..85 for industrial)
| Sym | Parameter | 2.5 V | 3.3 V |
|---|---|---|---|
| TC1,TC2 | clock pulse period min | 10 ns | 7 ns |
| TOSC | crystal max | 24 MHz | 24 MHz |
| TECK | ext clock max | 50 MHz | 64 MHz |
| TAS / TAH | address setup/hold (16) min | 3 / 3 | 3 / 3 |
| TCS | CS width min | 25 | 25 |
| TDY | delay between CS active cycles min | 25 | 25 |
| TRD | read strobe width min | 25 | 25 |
| TWR | write strobe width min | 20 | 20 |
| TRDV | read data valid max | 20 | 20 |
| TWDS / TWDH | write data setup/hold (16) min | 10 / 7 | 10 / 7 |
| TRDH | read data hold/float (16) max | 15 | 15 |
| TADS / TADH | addr setup/hold (68) min | 5 / 5 | 5 / 5 |
| TRWS | R/W setup to CS (68) min | 5 | 5 |
| TRDA | read data access (68) max | 20 | 20 |
| TRDH | read data hold (68) max | 15 | 15 |
| TWDS / TWDH (68) | min | 10 / 7 | 10 / 7 |
| TRWH | CS deassert to R/W deassert min | 1 | 1 |
| TCSL / TCSD | CS width / cycle delay (68) min | 25 / 25 | 25 / 25 |
| TWDO | IOW to modem output max | 50 | 50 |
| TMOD | modem input to INT max | 50 | 50 |
| TRSI | IOR to reset interrupt max | 50 | 50 |
| TSSI | stop to set interrupt max | 1 Bclk | 1 Bclk |
| TRRI | IOR to reset interrupt max | 45 | 45 |
| TSI | stop to interrupt max | 45 ns | 45 ns |
| TWRI | IOW to reset interrupt max | 45 | 45 |
| TRST | reset pulse min | 40 ns | 40 ns |
Bclk = 16x or 8x clock (p48).

### VCC
2.25-3.6 V (-40..+85 °C for IQ). Abs max supply 4 V, any pin -0.5..4 V (p46). p1: "5V tolerant serial
(modem) inputs"; p7: "5 volt tolerant inputs (except XTAL1)"; DC table gives no VIH max. Conservative:
inputMax 5.5 V only for RX/CTS/DSR/CD/RI (serial/modem) inputs, bus inputs VCC (flag). VIH 1.8 V (2.5 V) /
2.0 V (3.3 V); VIHCK max 4 V. ISLEEP 0.6 mA (all 8 asleep).

### Package drawing
100-lead QFP 14x20 mm, 1.95 mm form, Exar drawing "Rev. 2.00" (p53): D 23.65-24.15 (23.90), D1 19.90-20.10,
E 17.65-18.15 (17.90), E1 13.90-14.10; e 0.65; B (lead w) 0.22-0.38 (0.30); L 0.65-0.95 (0.80); A 2.60-3.40;
A1 0.05-0.35. Drawing p2/p53: pins 1-30 along one 20 mm (long) edge, 31-50 next 14 mm edge, 51-80 opposite
long edge, 81-100 remaining short edge (counter-clockwise, top view).
No land pattern.

### Obsolete
No (DigiKey "Not For New Designs").

### Notes
- TIMERCNTL: Table 8 bit labels vs Table 10 command codes conflict; and "1011 = reset timer" vs "1011-1111
  reserved". Use command codes 0001-1010 (+1011 reset).
- FCTR[3:0]=0000 → Table 17 says hysteresis 0, text says "next FIFO trigger level" after reset (only matters
  for table D).
- Reset table lists IRTX[7:0] LOW — no IRTX pins exist.
- RS-485 control uses "RTS# or DTR#" (MCR[2] presumably selects, not explicitly stated for RS-485).
- ISR priority order RXRDY (2) above RX timeout (3) differs from the other Exar sheets.

## XR16C854

- Title: "XR16C854/854D — 2.97V TO 5.5V QUAD UART WITH 128-BYTE FIFO", Exar Rev 3.1.0 April 2013; NRND
  watermark. Datasheet: `datasheets/controller-uart/xr16c854.pdf` (53 pp; pinouts p2-3, ordering p4, pins
  p4-8, function p9-22, registers p23-40, DC p41, AC p42-43, timing figs p44-49, packages p50-52 (drawing
  images), revision p53).
- Function: four enhanced 16C550 UARTs, 128-byte TX/RX FIFOs, trigger tables A/B/C/D (D = programmable via
  TRG), FIFO level counters (FC, FLVL), auto RTS (with selectable hysteresis) / auto CTS, auto Xon/Xoff,
  special char, Xon-any, IrDA 1.0 (RX inversion option), sleep, ÷1/÷4 prescaler (CLKSEL + MCR[7]), device
  ID (DVID=0x14) / DREV, scratchpad swap (FLVL/EMSR), FSTAT on QFP100. **Integer divisor only, 16x only**
  (no DLD, no 8x/4x). Up to 2 Mbps @5 V (p1, p9). 16 mode (Intel) and 68 mode (Motorola, PLCC/QFP only).
- Siblings: XR16C864 = same register set + RS-485 auto direction (FCTR[3]) and OP1/OP2 related pins (see
  that section). Differs from XR16V654 by 128-byte FIFOs, A-D trigger tables, FCTR/EMSR/TRG/FC/FLVL/DVID,
  no fractional divisor, 2.97-5.5 V supply.
- XR16C854 vs XR16C854D: same die; 64-LQFP has no INTSEL pin: XR16C854 (CV/IV) INTSEL bonded to GND, 854D
  (DCV/DIV) bonded to VCC → INT pins always active (p6, p9). One device.

### Parts (census)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| XR16C854CV-F | 64-LQFP 10x10 | 64 | lqfp64 (INTSEL=GND) | 0..+70 °C (C) | 2.97-5.5 V |
| XR16C854IV-F | 64-LQFP 10x10 | 64 | lqfp64 (INTSEL=GND) | -40..+85 °C (I) | 2.97-5.5 V |
| XR16C854DIV-F | 64-LQFP 10x10 | 64 | lqfp64 (INTSEL=VCC) | -40..+85 °C | 2.97-5.5 V |
| XR16C854IQ-F | 100-QFP 14x20 | 100 | qfp100 | -40..+85 °C | 2.97-5.5 V |
| XR16C854IQ | 100-QFP 14x20 | 100 | qfp100 | -40..+85 °C | 2.97-5.5 V |
| XR16C854CQ | 100-QFP 14x20 | 100 | qfp100 | 0..+70 °C | 2.97-5.5 V |
Ordering p4 lists only "-F" (Green/RoHS) parts and "TR" (tape & reel) variants: CJ/IJ (PLCC68), CV/IV/DCV/DIV
(LQFP64), CQ/IQ (QFP100). Decode: C = 0..70 °C, I = -40..85 °C, J = PLCC, V = LQFP, Q = QFP, D prefix =
INTSEL internally high. Sipex-era census names XR16C854CQ / XR16C854IQ (no -F) are the leaded versions of
CQ-F / IQ-F — same package and grade; they are not in the current ordering table. Old-date-code parts
("DC YYWW" top mark and older) lack 5 V-tolerant inputs (p1, p11) — likely applies to the non-F stock.

### Pinouts (table p4-8)
**lqfp64** (CV-F, IV-F, DIV-F): pin numbering identical to XR16V564/XR16V654 lqfp64:
```
1 DSRA input      17 DSRB input      33 CTSC input      49 CDD input
2 CTSA input      18 CDB input       34 DTRC output     50 RID input
3 DTRA output     19 RIB input       35 VCC power       51 RXD input
4 VCC power       20 RXB input       36 RTSC output     52 VCC power
5 RTSA output     21 CLKSEL input    37 INTC output     53 D0 inout
6 INTA output     22 A2 input        38 CSC input       54 D1 inout
7 CSA input       23 A1 input        39 TXC output      55 D2 inout
8 TXA output      24 A0 input        40 IOR input       56 D3 inout
9 IOW input       25 XTAL1 input     41 TXD output      57 D4 inout
10 TXB output     26 XTAL2 output    42 CSD input       58 D5 inout
11 CSB input      27 RESET input     43 INTD output     59 D6 inout
12 INTB output    28 GND ground      44 RTSD output     60 D7 inout
13 RTSB output    29 RXC input       45 GND ground      61 GND ground
14 GND ground     30 RIC input       46 DTRD output     62 RXA input
15 DTRB output    31 CDC input       47 CTSD input      63 RIA input
16 CTSB input     32 DSRC input      48 DSRD input      64 CDA input
```
Intel mode only (no 16/68 pin); RESET active high.

**qfp100** (IQ-F, IQ, CQ):
```
1 NC              26 NC              51 NC              76 FSRS input
2 NC              27 NC              52 NC              77 NC
3 NC              28 NC              53 NC              78 NC
4 NC              29 NC              54 NC              79 NC
5 TXRDYA output   30 NC              55 NC              80 NC
6 IRTXA output    31 RXRDYB output   56 TXRDYC output   81 TXRDYD output
7 DSRA input      32 CDB input       57 IRTXC output    82 RXRDYD output
8 CTSA input      33 RIB input       58 DSRC input      83 CDD input
9 DTRA output     34 RXB input       59 CTSC input      84 RID input
10 VCC power      35 CLKSEL input    60 DTRC output     85 RXD input
11 RTSA output    36 16/68 input     61 VCC power       86 VCC power
12 INTA output    37 A2 input        62 RTSC output     87 INTSEL input
13 CSA input      38 A1 input        63 INTC output     88 D0 inout
14 TXA output     39 A0 input        64 CSC input       89 D1 inout
15 IOW input      40 XTAL1 input     65 TXC output      90 D2 inout
16 TXB output     41 XTAL2 output    66 IOR input       91 D3 inout
17 CSB input      42 CHCCLK input    67 TXD output      92 D4 inout
18 INTB output    43 RESET input     68 CSD input       93 D5 inout
19 RTSB output    44 RXRDY output    69 INTD output     94 D6 inout
20 GND ground     45 TXRDY output    70 RTSD output     95 D7 inout
21 DTRB output    46 GND ground      71 GND ground      96 GND ground
22 CTSB input     47 RXC input       72 DTRD output     97 RXA input
23 DSRB input     48 RIC input       73 CTSD input      98 RIA input
24 IRTXB output   49 CDC input       74 DSRD input      99 CDA input
25 TXRDYB output  50 RXRDYC output   75 IRTXD output    100 RXRDYA output
```
Mode pins: 16/68 (pin 36, pull-up): HIGH Intel, LOW Motorola. In 68 mode: 15 IOW → R/W; 13 CSA → CS;
17 CSB → A3; 64 CSC → A4; 66 IOR and 68 CSD unused ("N.C."); 12 INTA → IRQ (active-low open-drain);
18/63/69 INTB-D unused (stay LOW); 43 RESET → active-low; INTSEL must be LOW. INTSEL (87) pull-down;
FSRS (76) pull-up. CHCCLK (42) = channel C BRG clock input (tie to XTAL2 normally; or external 16x/MIDI
clock). IRTXA-D = always-IrDA-encoded TX copies (idle LOW). TXRDY/RXRDY (44/45) = "wire-ORed" (i.e. AND of
active-low) status of the four channels' TXRDY/RXRDY. TXRDYA-D/RXRDYA-D = per channel.
Factory test mode: IOR, IOW and CS all asserted low simultaneously → device enters factory test mode, not
operational until power cycle (p8). (Model: illegal state.)

### Host interface
As XR16V554: Intel CSA-D active-low select (Table 1 p11; all four low = broadcast write), IOR/IOW strobes,
write data latched on IOW rising edge, read data driven while IOR low. Motorola: CS + R/W + A4:A3 (Table 2).
Asynchronous (no clock needed).

### Registers (Table 8 p23, Table 9 p24-25, descriptions p26-39, reset Table 19 p40)
| A2-A0 | Condition | Name | R/W | Reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR / THR | R / W | XX |
| 000 | LCR[7]=1, LCR≠BF | DLL | R/W | XX (Table 19) |
| 001 | LCR[7]=1, LCR≠BF | DLM | R/W | XX |
| 000 | LCR[7]=1, ≠BF, DLL=DLM=00 | DREV (read) | R | rev (01 = rev A) |
| 001 | LCR[7]=1, ≠BF, DLL=DLM=00 | DVID (read) | R | 0x14 |
| 001 | LCR[7]=0 | IER | R/W | 00 |
| 010 | LCR[7]=0 | ISR / FCR | R / W | 01 / 00 |
| 011 | – | LCR | R/W | 00 |
| 100 | LCR[7]=0 | MCR | R/W | 00 (b7 from CLKSEL) |
| 101 | LCR[7]=0 | LSR (R) / reserved (W) | | 60 |
| 110 | LCR[7]=0 | MSR (R) / reserved (W) | | [3:0]=0, [7:4]=inverted pins |
| 111 | LCR[7]=0, FCTR[6]=0 | SPR | R/W | FF (preserved in sleep) |
| 111 | LCR[7]=0, FCTR[6]=1 | FLVL (R) / EMSR (W) | | 00 / 00 |
| 000 | LCR=0xBF | FC (R) / TRG (W) | | 00 / 00 |
| 001 | LCR=0xBF | FCTR | R/W | 00 |
| 010 | LCR=0xBF | EFR | R/W | 00 |
| 100 | LCR=0xBF | XON1 | R/W | 00 |
| 101 | LCR=0xBF | XON2 | R/W | 00 |
| 110 | LCR=0xBF | XOFF1 | R/W | 00 |
| 111 | LCR=0xBF | XOFF2 | R/W | 00 |
| XXX | FSRS pin LOW (QFP100) | FSTAT | R | FF |
DVID/DREV: reading DLM/DLL returns DVID/DREV whenever the divisor latch currently holds 0x0000 (p11, p36).

Bit definitions:
- IER: b0 RX data; b1 TX ready (non-FIFO THR empty; FIFO below TX trigger; immediate if already empty);
  b2 RX line status (LSR[4:1] — "generate an interrupt immediately when the character has been received");
  b3 modem status; b4 sleep enable; b5 Xoff/special char int; b6 RTS int (pin L→H); b7 CTS int (pin L→H);
  b7-4 need EFR[4]=1 (p26-27).
- ISR: b0 0=pending; b3-1 code; b4 Xoff/special detected ("once set stays 1 until Xon received"); b5
  CTS/RTS changed; b7-6 FIFOs enabled. b5-4 need EFR[4] (p28).
- FCR: b0 FIFO enable; b1 RX reset; b2 TX reset; b3 DMA mode (TXRDY/RXRDY); b5-4 TX trigger (EFR[4]=1);
  b7-6 RX trigger. Table chosen by FCTR[5:4]; last selection applies to both TX and RX.
- LCR: standard 16C550 (as XR16V554); LCR=0xBF selects enhanced bank.
- MCR: b0 DTR (1 → LOW); b1 RTS (1 → LOW); b2 reserved/OP1 (loopback RI) — "If OP1# output is required for
  RS485 operation, use the XR16C864"; b3 INT output enable (Table 13: INTSEL=0,MCR3=0 3-state; =1 active;
  INTSEL=1 active) / OP2 in loopback (CD); b4 loopback; b5 Xon-any (EFR[4]); b6 IrDA (EFR[4]); b7
  prescaler ÷1/÷4 (EFR[4]; preset from CLKSEL at power-up/reset: CLKSEL=VCC → ÷1, GND → ÷4) (p32-33).
- LSR, MSR: identical to XR16V554 definitions (LSR b0 DR, b1 OE, b2 PE, b3 FE, b4 BI, b5 THRE, b6 TEMT,
  b7 FIFO error; MSR b0-3 deltas (b2 TERI), b4-7 CTS/DSR/RI/CD).
- EMSR (write, LCR[7]=0 & FCTR[6]=1) (p35-36): b1-0 FIFO counter mode for FLVL reads: 00 RX count, 01 TX
  count, 10 RX count, 11 alternate RX/TX (first read after write = RX, then TX, RX, ...); b3-2 reserved;
  b5-4 extended RTS hysteresis (with FCTR[1:0]); b7-6 reserved.
- FLVL (read, FCTR[6]=1): FIFO count per EMSR[1:0].
- FC (read, LCR=BF): RX count (FCTR[7]=0) or TX count (FCTR[7]=1). TRG (write, LCR=BF): table-D trigger,
  RX (FCTR[7]=0) or TX (FCTR[7]=1). Range not stated (FIFO = 128).
- FCTR (LCR=BF) (p37-38): b1-0 RTS hysteresis (with EMSR[5:4]); b2 IrDA RX invert (0 = idle LOW pulses
  HIGH; 1 = inverted, idle HIGH); b3 "Auto RS-485 direction control — not available in XR16C854", but this
  bit changes TX-ready interrupt to transmitter (TSR) empty (Table 3 p12); b5-4 trigger table A/B/C/D;
  b6 scratchpad swap (SPR → FLVL/EMSR); b7 TRG/FC select (0 RX, 1 TX).
- EFR (LCR=BF): b3-0 software flow control (Table 18 p38 — same encoding as XR16V564); b4 enhanced enable
  (IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5]); b5 special char (XOFF2); b6 auto RTS; b7 auto CTS (p38-39).
- XON1/2, XOFF1/2 (R/W).
- FSTAT (QFP100, FSRS low, any address): b3-0 = inverted TXRDYD..A, b7-4 = RXRDYD..A (b0/b4 = channel A)
  (p25, p39).

Trigger tables (Table 11 p30; FCTR[5:4]):
| Table | FCTR[5:4] | RX FCR[7:6]=00/01/10/11 | TX FCR[5:4]=00/01/10/11 |
|---|---|---|---|
| A | 00 | 1 / 4 / 8 / 14 | 1 (FCR[5:4]=00 only listed) |
| B | 01 | 8 / 16 / 24 / 28 | 16 / 8 / 24 / 30 |
| C | 10 | 8 / 16 / 56 / 60 | 8 / 16 / 32 / 56 |
| D | 11 | TRG (FCTR[7]=0) | TRG (FCTR[7]=1) |
(B/C column assignment: printed table interleaves; RX = 8/16/24/28 confirmed by Xoff Table 7 p19 and
matches the V564 page image layout.) Default after reset: table A, RX 1, TX 1.

RTS hysteresis (Table 15 p36), EMSR[5:4]:FCTR[1:0] → ±chars: 0000 0, 0001 4, 0010 6, 0011 8, 0100 8,
0101 16, 0110 24, 0111 32, 1000 40, 1001 44, 1010 48, 1011 52, 1100 12, 1101 20, 1110 28, 1111 36.
"This table is for programmable trigger levels only (Table D). The hysteresis for tables A-C is the next
higher and next lower trigger levels" (p17). After reset FCTR[1:0]=0 → "next trigger level".

ISR priority (Table 10 p28): 1 LSR 0x06; 2 RX timeout 0x0C; 3 RX data 0x04; 4 TX ready 0x02; 5 MSR 0x00;
6 Xoff/special 0x10; 7 CTS/RTS 0x20; none 0x01. Clearing (p28): LSR read; read below trigger; RHR read;
ISR read or THR write; MSR read; Xoff: ISR read or Xon received; special: ISR read or next char; CTS/RTS:
MSR read.

### Behavior
- **Baud** (p13-14, p36): XTAL1 (crystal 8? see timing, ≤24 MHz; ext clock ≤24 MHz @3.3 V / 32 MHz @5 V) →
  prescaler ÷1/÷4 (MCR[7], preset by CLKSEL; QFP100 channel C uses CHCCLK pin instead of oscillator) →
  divisor N = DLM:DLL (1..65535) → 16x clock. Baud = clk/prescaler/(16·N). N=0 not allowed (AC table: N
  min 1). Table 6 p14 (14.7456 MHz): 921.6k N=1 … 400 N=2304 (÷1); ÷4 gives 230.4k…100.
- **TX/RX**: as XR16V554 (16x only, mid-bit sampling at count 8, RX timeout 4 chars + 12 bits, error tags
  per FIFO byte, overrun keeps FIFO, one 0x00 per break).
- **FIFO / DMA**: 128 deep. TXRDY/RXRDY (Table 5 p13) identical rules to V554 (mode 0 / mode 1).
- **INT pins** (Tables 3/4 p12, Table 13 p32): active HIGH when enabled per INTSEL/MCR[3]; with FCTR[3]=1 the
  TX interrupt fires on "transmitter empty" (TSR) instead of THR/FIFO empty.
- **Auto RTS**: EFR[6], start by MCR[1]=1. Tables A-C: RTS HIGH at next higher trigger level, LOW at next
  lower (as V564 Table 7 logic). Table D: HIGH at trigger+hyst, LOW at trigger−hyst. Interrupt at trigger.
- **Auto CTS**: EFR[7]; TX stops after current stop bit while CTS HIGH.
- **Xon/Xoff** (p19): Xoff sent 2 char times after RX FIFO crosses trigger; Xon when below one level under
  trigger (A-C) or trigger−hysteresis (D). Table 7 (table B): 8→Xon at 0; 16→8; 24→16; 28→24.
- **Special char, Xon-any**: as V564.
- **IrDA** (p19-20): MCR[6]; 3/16-bit HIGH pulse per 0 bit, idle LOW; RX pulse → 0 bit; FCTR[2] RX invert.
  IRTX pins (QFP100) always encoded.
- **Sleep** (p20-21): all four channels IER[4]=1, no interrupts, MSR[3:0]=0, RX high → oscillator stops;
  wake on RX start, THR load, modem input change; auto re-sleep; ISLEEP ≤100 µA (3.3 V) / 200 µA (5 V).
- **Loopback**: as V554 (TX HIGH, RTS/DTR HIGH, inputs ignored, MSR ← MCR[1],[0],[2],[3]).
- **Reset** (p8, p11, Table 19 p40): RESET active-high (16 mode) / active-low (68 mode), ≥40 ns. IER/FCR/LCR/
  MCR=00, ISR=01, LSR=60, SPR=FF, EMSR/FLVL/TRG/FC/FCTR/EFR/XON/XOFF=00, FSTAT=FF; DLL/DLM = XX (not reset).
  Outputs: TX 1, IRTX 0, RTS 1, DTR 1, RXRDY 1, TXRDY 0, INT 3-state (XR16C854) / 0 (854D), IRQ 1 (68 mode).

### Timing (p42-43; min/max; columns 3.3 V and 5.0 V; 0-70 °C or -40..85 for I)
| Sym | Parameter | 3.3 V | 5.0 V |
|---|---|---|---|
| CLK | clock pulse duration min | 20 ns | 15 ns |
| OSC | crystal frequency max | 8 MHz (sic) | 24 MHz |
| OSC | external clock max | 24 MHz | 32 MHz |
| TAS / TAH | addr setup/hold (16) min | 10 / 10 | 5 / 5 |
| TCS | CS width min | 70 | 50 |
| TRD | IOR width min | 70 | 50 |
| TDY | read/write cycle delay min | 70 | 50 |
| TRDV | data access max | 70 | 50 |
| TDD | data disable | 0-25 | 0-15 |
| TWR | IOW width min | 70 | 50 |
| TDS / TDH | data setup/hold min | 10 / 10 | 5 / 5 |
| TADS / TADH | addr setup/hold (68) min | 10 / 15 | 10 / 15 |
| TRWS | R/W setup to CS min | 10 | 5 |
| TRDA | read access (68) max | 70 | 50 |
| TRDH | read hold (68) max | 15 | 15 |
| TWDS / TWDH (68) | min | 20 / 10 | 15 / 10 |
| TRWH | CS deassert to R/W deassert min | 10 | 10 |
| TCSL / TCSD | CS width / cycle delay (68) min | 70 / 70 | 50 / 70 |
| TWDO | IOW to modem output max (100 pF) | 50 | 40 |
| TMOD | modem input to INT max | 40 | 35 |
| TRSI | IOR to reset interrupt max | 40 | 35 |
| TSSI | stop to set interrupt max | 1 Bclk | 1 Bclk |
| TRRI | IOR to reset interrupt max | 40 | 40 |
| TSI | stop to interrupt max | 45 | 40 |
| TINT | INT reset to TX start | 8-24 Bclk | 8-24 Bclk |
| TWRI | IOW to reset interrupt max | 45 | 40 |
| TSSR | stop to set RXRDY max | 1 Bclk | 1 Bclk |
| TRR | IOR to reset RXRDY max | 45 | 40 |
| TWT | IOW to set TXRDY max | 45 | 40 |
| TSRT | center of start to reset TXRDY max | 8 Bclk | 8 Bclk |
| TRST | reset pulse min | 40 ns | 40 ns |
| N | divisor | 1..65535 | 1..65535 |
(OSC "8" at 3.3 V is as printed — likely a typo for 24; p9 text says 24 MHz crystal.)

### VCC
2.97-5.5 V; C grade 0..70 °C, I grade -40..85 °C (p41). Abs max 7 V supply / pins. Inputs 5 V tolerant
(VIH max 5.5 V) except XTAL1 — **only for top-mark "F2 YYWW" and newer**; older "DC YYWW" parts VIH max = VCC
(p1, p11, p41). inputMax: 5.5 V (current -F parts). VIH 2.0 V (3.3 V) / 2.2 V (5 V); VIL 0.8 V.

### Package drawings
- 64 LQFP 10x10x1.4, JEDEC MS-026 variation BCD, Exar drawing "64 PIN LQFP PACKAGE OUTLINE" rev A
  02/11/09 (p50, image): D 12.00 BSC, D1 10.00 BSC, e 0.50, b 0.17/0.22/0.27, L 0.45/0.60/0.75, A ≤1.60.
  (Small-print values read from page image — same as the V-series LQFP64 drawing.)
- 100 QFP 14x20, 1.95 mm form (Option 2), JEDEC MS-022, rev B 01/24/13 (p52, image): body 14.00 x 20.00
  BSC, span 17.90 x 23.90 BSC, e 0.65, b 0.22-0.40, c 0.11-0.23, L 0.73/0.88/1.03, A ≤3.40, A1 ≥0.25,
  A2 2.50/2.70/2.90; ND 20, NE 30.
- (68 PLCC p51 — not a census package.) No land patterns.

### Obsolete
None in census (MaxLinear rows "Not For New Designs"; Sipex rows "Active").

### Notes
- DLL/DLM reset "XX" (Table 19) — unlike V-series (01/00).
- OSC max 8 MHz at 3.3 V in AC table vs 24 MHz in text.
- FCTR[3] described as RS-485 but "not available in XR16C854" except for TX interrupt behavior.
- Block diagram mentions "OP2A#" output but no OP2 pin appears in any pin table.

## XR16C864

- Title: "XR16C864 — 2.97V TO 5.5V QUAD UART WITH 128-BYTE FIFO", Exar Rev 2.2.0 April 2013; NRND watermark.
  Datasheet: `datasheets/controller-uart/xr16c864.pdf` (51 pp; pinout p2, ordering p3, pins p3-7, function
  p8-23, registers p24-41, DC p42, AC p43-44, timing figs p45-49, package p50).
- Function: the XR16C854 die feature set in a 100-QFP only, plus: per-channel **OP1# (general output or
  auto RS-485 direction), OP2#, BCLK (16x baud clock output)**, and a **host DMA interface** (DACKA-D, AEN,
  TC inputs; TXDRQA-D/RXDRQA-D outputs sharing the TXRDYA-D/RXRDYA-D pins, enabled by EMSR[2]/EMSR[3]).
  Pins that are NC on the XR16C854 QFP100 are used for these (pins 1-4, 26-30, 51-55, 77-80).
- Register-chapter text diff vs XR16C854 (both sheets compared programmatically): identical except
  MCR[2] (OP1#/RS-485 output control), MCR[3] label "OP2#/INT Output Enable", EMSR[3:2] (DMA request
  enables), FCTR[3] (auto RS-485 on OP1#, real function), §2.8 Direct Memory Access, §2.19 Auto RS-485, and
  no XR16C864D variant (INTSEL is a real pin). AC and DC tables identical to XR16C854. DVID = 0x14 (same as
  XR16C854 per p36 text "0x14 for XR16C864") — software cannot tell them apart by DVID.

### Parts (census)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| XR16C864CQ-F | 100-QFP 14x20 | 100 | qfp100-864 | 0..+70 °C | 2.97-5.5 V |
| XR16C864IQ-F | 100-QFP 14x20 | 100 | qfp100-864 | -40..+85 °C | 2.97-5.5 V |
Ordering p3: CQ-F, CQTR-F, IQ-F, IQTR-F (TR = tape & reel, -F = Green/RoHS). All decoded.

### Pinout — qfp100 (pin table p3-7)
Same as XR16C854 qfp100 except these (854: NC) pins:
```
1 BCLKA output    26 DACKB input     51 BCLKC output    77 DACKD input
2 OP2A output     27 AEN input       52 OP2C output     78 OP1D output
3 OP1A output     28 OP1B output     53 OP1C output     79 OP2D output
4 DACKA input     29 OP2B output     54 TC input        80 BCLKD output
                  30 BCLKB output    55 DACKC input
```
and dual-function outputs: 5/25/56/81 = TXRDYA-D (active low) or TXDRQA-D (active high when EMSR[2]=1);
100/31/50/82 = RXRDYA-D or RXDRQA-D (EMSR[3]=1). Full list for convenience:
```
1 BCLKA o   21 DTRB o   41 XTAL2 o  61 VCC      81 TXRDYD/TXDRQD o
2 OP2A o    22 CTSB i   42 CHCCLK i 62 RTSC o   82 RXRDYD/RXDRQD o
3 OP1A o    23 DSRB i   43 RESET i  63 INTC o   83 CDD i
4 DACKA i   24 IRTXB o  44 RXRDY o  64 CSC i    84 RID i
5 TXRDYA o  25 TXRDYB o 45 TXRDY o  65 TXC o    85 RXD i
6 IRTXA o   26 DACKB i  46 GND      66 IOR i    86 VCC
7 DSRA i    27 AEN i    47 RXC i    67 TXD o    87 INTSEL i
8 CTSA i    28 OP1B o   48 RIC i    68 CSD i    88 D0 io
9 DTRA o    29 OP2B o   49 CDC i    69 INTD o   89 D1 io
10 VCC      30 BCLKB o  50 RXRDYC o 70 RTSD o   90 D2 io
11 RTSA o   31 RXRDYB o 51 BCLKC o  71 GND      91 D3 io
12 INTA o   32 CDB i    52 OP2C o   72 DTRD o   92 D4 io
13 CSA i    33 RIB i    53 OP1C o   73 CTSD i   93 D5 io
14 TXA o    34 RXB i    54 TC i     74 DSRD i   94 D6 io
15 IOW i    35 CLKSEL i 55 DACKC i  75 IRTXD o  95 D7 io
16 TXB o    36 16/68 i  56 TXRDYC o 76 FSRS i   96 GND
17 CSB i    37 A2 i     57 IRTXC o  77 DACKD i  97 RXA i
18 INTB o   38 A1 i     58 DSRC i   78 OP1D o   98 RIA i
19 RTSB o   39 A0 i     59 CTSC i   79 OP2D o   99 CDA i
20 GND      40 XTAL1 i  60 DTRC o   80 BCLKD o  100 RXRDYA o
```
(5/25/56/81 TXRDYA-D; 100/31/50/82 RXRDYA-D; 6/24/57/75 IRTXA-D.) VCC 10, 61, 86; GND 20, 46, 71, 96.
Motorola-mode pin meanings as XR16C854 (16/68 pin 36).

### Host interface
As XR16C854 (Intel / Motorola, CSA-D or CS+A4:A3, A2..A0) plus the DMA interface (p5, p9):
- DACKx (active low, connect VCC if unused), AEN (active high, GND if unused), TC (active-high pulse,
  GND if unused). "Direct Memory Access cycle will start processing when CPU/Host sets DACK low and AEN
  high. All writes will be to the TX FIFO and all reads will be from the RX FIFO. A0-A2 and CS# A-D will be
  ignored." (Read/write strobes are still IOR/IOW — implied, not stated.)
- TXDRQx: HIGH = transmit request; held HIGH until TC pulses or the TX FIFO is filled above its trigger level.
  Enabled by EMSR[2]=1 (pin then no longer TXRDY).
- RXDRQx: HIGH = receive request; held HIGH until TC pulses or the RX FIFO is emptied. Enabled by EMSR[3]=1.
- When/what asserts DRQ (assumed: TX FIFO below TX trigger / RX FIFO at RX trigger or timeout) and how TC is
  qualified per channel (TC is shared; presumably with DACKx) are **not specified**; no DMA timing given.

### Registers
Identical to XR16C854 (see that section: map incl. DREV/DVID via DLL/DLM=0, SPR/FLVL/EMSR swap, LCR=0xBF
bank TRG/FC/FCTR/EFR/XON/XOFF, FSTAT via FSRS; trigger tables A-D; hysteresis table; ISR priorities;
reset values) with these bit changes (Table 9 p25-26, p33-38):
- MCR[2]: OP1# output control (general purpose output; pin OP1x). In loopback it drives MSR[6] (RI). "If
  Auto RS-485 is enabled via FCTR[3], MCR[2] should not be written to." Polarity not stated; by the "#"
  convention MCR[2]=1 → OP1 pin LOW (as DTR/RTS).
- MCR[3]: "OP2#/INT Output Enable" — INT enable per INTSEL table (as 854) and OP2 (loopback → MSR[7] CD);
  the text does not say that the OP2x pins follow MCR[3] outside loopback, but the table label implies
  OP2 pin = NOT MCR[3] (flag).
- EMSR[2]: TX DMA request enable (TXRDYx pin becomes TXDRQx). EMSR[3]: RX DMA request enable.
- FCTR[3]: 0 = OP1# general purpose via MCR[2]; 1 = OP1# is auto RS-485 direction output, and the TX-ready
  interrupt changes to "transmitter (TSR) empty" (Table 3 p12).
- No XR16C864D; INT reset state 3-state (INTSEL low) (Table 19 p41).

### Behavior (diffs only; everything else = XR16C854)
- **Auto RS-485** (FCTR[3], p20): OP1# de-asserted (logic 1) after the last stop bit of the last char
  transmitted; re-asserted (logic 0) when data is loaded and before the start bit. No turnaround delay
  option. (So OP1 LOW = transmit, HIGH = receive.)
- **BCLKx**: 16x clock of the selected baud rate from channel x's BRG ("internally connected to the RCLK
  input") (p6).
- **OP2x**: general-purpose output (see MCR[3] flag).
- **Reset** (Table 19 p41): outputs as XR16C854 (TX 1, IRTX 0, RTS 1, DTR 1, RXRDY 1, TXRDY 0, INT 3-state,
  IRQ 1); OP1/OP2/BCLK/DRQ reset states not listed (by MCR=00: OP1/OP2 HIGH; DRQ disabled → pins are
  TXRDY 0 / RXRDY 1).
- Factory test mode on IOR=IOW=CS=0 simultaneously (p7).

### Timing / VCC
AC (p43-44) and DC (p42) tables identical to XR16C854 (3.3 V / 5 V columns; see that section). No DMA
(DACK/TC/DRQ) timing given. VCC 2.97-5.5 V; inputs 5 V tolerant except XTAL1 for "F2 YYWW" and newer.

### Package drawing
100 QFP 14x20 1.95 mm form, JEDEC MS-022 (p50, image, same Exar outline as XR16C854 p52): e 0.65, body
14x20, span 17.90 x 23.90, b 0.22-0.40, L 0.73-1.03.

### Obsolete
None (DigiKey "Not For New Designs").

### Notes
- DMA protocol under-specified (DRQ assert conditions, TC qualification, timing).
- OP2 pin control and OP1 polarity inferred.
- DVID identical to XR16C854 (0x14).

## XR16L784

- Title: "XR16L784 — HIGH PERFORMANCE 2.97V TO 5.5V QUAD UART", Exar Rev 1.2.3 July 2008. Datasheet:
  `datasheets/controller-uart/xr16l784.pdf` (51 pp; pinout + ordering p2, pins p3-5, function p6-18,
  registers p19-40, DC p41-42, AC p43, timing figs p44-47, package p48, revision p49).
- Function: quad version of the XR16L788 "5G flat register" architecture — the same design family as
  XR16V798 in this group (flat A7..A0 map, device configuration block at 0x80-0x8F with INT0-INT2, 16-bit
  timer, 8XMODE, RESET, SLEEP, DVID/DREV, REGB; channel registers at 0x00-0x3F with FCTR/EFR/TXCNT/TXTRG/
  RXCNT/RXTRG/XCHAR/Xon/Xoff at A3=1; 64-byte FIFOs; trigger tables A-D; auto RTS/CTS or DTR/DSR with
  hysteresis; auto RS-485 with 0-15 bit delay; IrDA with ENIR preset; single open-drain INT#).
- Differences vs XR16V798 (text diff of p7-40 vs V798 p8-45):
  - 4 channels (A5:A4), 0x40-0x7F reserved; INT3 unused (reads 00); DVID = 0x24.
  - **Integer divisor only** (no DLD); DLL/DLM reset = XX (random at power-up); crystal ≤24 MHz, ext clock up
    to 50 MHz (5 V) with 2 kΩ pull-up on XTAL2 (p8).
  - MCR[3] reserved (no send-char-immediate); MCR[2] still RTS/DTR flow select (OP1 in loopback).
  - MSR write: only b7-4 (RS-485 delay); b3-0 = 0 (no TX/RX disable).
  - XCHAR: only b1 (Xon detected) and b0 (Xoff detected); b7-2 = 0.
  - Timer TIMERCNTL is a **bit-mapped** register (not command codes): b0 INT enable, b1 start/stop, b2
    one-shot(1)/re-trigger(0), b3 ext(1)/int(0) clock (Table 10 p22).
  - Wake-up interrupt cleared by reading INT0 (V798: by reading ISR).
  - RX timeout code "cleared when the FIFO becomes empty" (Table 9 p22) vs RHR read.
  - IER[2] LSR interrupt fires "immediately when the character has been received"; IER[1] TX int "whenever
    THR empty or TX FIFO below trigger" (no "twice" wording).
  - Supply 2.97-5.5 V; 5 V tolerant inputs (except XTAL1).

### Parts (census)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| XR16L784CV-F | 64-LQFP 10x10x1.4 | 64 | lqfp64-784 | 0..+70 °C | 2.97-5.5 V |
| XR16L784IV-F | 64-LQFP 10x10x1.4 | 64 | lqfp64-784 | -40..+85 °C | 2.97-5.5 V |
Ordering p2: XR16L784CV (0..70 °C) and XR16L784IV (-40..85 °C), 64-Lead LQFP, Active. "-F" = RoHS suffix
(not in this sheet). Decoded.

### Pinout — lqfp64-784 (table p3-5; drawing p2 verified)
```
1 A2 input        17 D6 inout       33 RX2 input      49 XTAL2 output
2 A3 input        18 D7 inout       34 CTS2 input     50 XTAL1 input
3 A4 input        19 16/68 input    35 DSR2 input     51 GND ground
4 A5 input        20 RST input      36 CD2 input      52 VCC power
5 A6 input        21 RX3 input      37 RI2 input      53 TX0 output
6 A7 input        22 CTS3 input     38 RTS2 output    54 DTR0 output
7 IOR input       23 DSR3 input     39 DTR2 output    55 RTS0 output
8 IOW input       24 CD3 input      40 TX2 output     56 RI0 input
9 VCC power       25 RI3 input      41 RX1 input      57 CD0 input
10 GND ground     26 RTS3 output    42 CTS1 input     58 DSR0 input
11 D0 inout       27 DTR3 output    43 DSR1 input     59 CTS0 input
12 D1 inout       28 TX3 output     44 CD1 input      60 RX0 input
13 D2 inout       29 GND ground     45 RI1 input      61 INT output (open drain)
14 D3 inout       30 VCC power      46 RTS1 output    62 CS input
15 D4 inout       31 TMRCK input    47 DTR1 output    63 A0 input
16 D5 inout       32 ENIR input     48 TX1 output     64 A1 input
```
Modem pins active low. INT# open-drain active-low (47k-100k pull-up). RST# active low. ENIR active high
(sampled at power-up, RST# rising edge, or soft reset → MCR[6] of every channel). 16/68: HIGH Intel (IOR,
IOW, CS active low); LOW Motorola (IOW = R/W, CS = strobe, IOR tie VCC). No internal pull-ups stated.

### Host interface (p3, p6)
Flat 8-bit map: A7=0, A6=0: A5:A4 = channel 0-3, A3:A0 = channel register; A7=0, A6=1: reserved;
A7=1: device configuration 0x80-0x8F. REGB[0] = write to all 4 channels (clear before reading).
Strobing as XR16V798 (Intel write latched at IOW rising; Motorola at CS rising).

### Device configuration registers (Table 8 p20, p21-24)
| Addr | Name | R/W | Reset | Bits |
|---|---|---|---|---|
| 0x80 | INT0 | R | 00 | b3-0 channel 3-0 interrupt pending; b7-4 reserved (0). Reading INT0 clears the sleep wake-up interrupt |
| 0x81 | INT1 | R | 00 | b2-0 ch0 code, b5-3 ch1 code, b7-6 ch2 code[1:0] |
| 0x82 | INT2 | R | 00 | b0 ch2 code[2], b3-1 ch3 code, b7-4 reserved |
| 0x83 | INT3 | R | 00 | reserved (XR16L788 only) |
| 0x84 | TIMERCNTL | R/W | 00 | b0 timer INT enable (reading TIMERCNTL clears the timer interrupt); b1 1=start/0=stop(pause); b2 0=re-triggerable, 1=one-shot; b3 0=oscillator clock, 1=TMRCK pin; b7-4 reserved |
| 0x85 | TIMER | R | 00 | reserved |
| 0x86 | TIMERLSB | R/W | 00 | reload value low |
| 0x87 | TIMERMSB | R/W | 00 | reload value high (16-bit down counter) |
| 0x88 | 8XMODE | R/W | 00 | b3-0: 1 = channel uses 8x sampling |
| 0x89 | REGA ("REG 1") | R | 00 | reserved |
| 0x8A | RESET | W | 00 | b3-0: write 1 = soft reset channel (self-clearing) |
| 0x8B | SLEEP | R/W | 00 | b3-0: sleep enable per channel |
| 0x8C | DREV | R | 01 = rev A | revision |
| 0x8D | DVID | R | 0x24 | upper nibble 2 = XR16L78x, lower = channel count |
| 0x8E | REGB ("REG 2") | R/W | 00 | b0 simultaneous write to all UARTs |
Per-channel 3-bit interrupt codes (Table 9 p22): 000 none; 001 (prio 1) RXRDY & RX line status; 010 (2)
RX timeout (cleared when FIFO empties); 011 (3) TXRDY (THR, or TSR in auto-RS485); 100 (4) MSR / RTS-CTS /
DTR-DSR delta / Xon/Xoff / special char; 101, 110 reserved; 111 (7) timer time-out (channel 0 only).
Timer: interrupt N clocks after start (one-shot) or every N clocks (re-trigger), N = TIMERMSB:LSB (the V798
sheet states N valid 0x0002-0xFFFF; not stated here).

### Channel registers (Table 11 p25, Table 12 p26-27, p27-40)
| A3-A0 | Cond. | Name | R/W | Reset |
|---|---|---|---|---|
| 0000 | LCR[7]=0 | RHR / THR | R / W | XX |
| 0000 | LCR[7]=1 | DLL | R/W | XX |
| 0001 | LCR[7]=1 | DLM | R/W | XX |
| 0001 | LCR[7]=0 | IER | R/W | 00 |
| 0010 | – | ISR / FCR | R / W | 01 / 00 |
| 0011 | – | LCR | R/W | 00 |
| 0100 | – | MCR | R/W | 00 (b6 from ENIR) |
| 0101 | – | LSR | R | 60 |
| 0110 | – | MSR (R) / RS-485 turnaround delay (W) | | [3:0]=0, [7:4]=inputs |
| 0111 | – | SPR | R/W | FF (kept in sleep) |
| 1000 | – | FCTR | R/W | 00 |
| 1001 | – | EFR | R/W | 00 |
| 1010 | – | TXCNT (R) / TXTRG (W) | | 00 |
| 1011 | – | RXCNT (R) / RXTRG (W) | | 00 |
| 1100 | – | XCHAR (R) / XOFF1 (W) | | 00 |
| 1101 | – | XOFF2 (W) | | 00 |
| 1110 | – | XON1 (W) | | 00 |
| 1111 | – | XON2 (W) | | 00 |
Bits:
- IER: b0 RX data/timeout; b1 THR empty / TX FIFO below trigger; b2 RX line status; b3 modem status; b4
  reserved; b5 Xoff/special char (EFR[4]); b6 RTS/DTR int (pin transition) (EFR[4]); b7 CTS/DSR int (EFR[4]).
- ISR: b0 0=pending; b5-1 code (table below); b7-6 FIFOs enabled.
- FCR: b0 FIFO enable; b1 RX reset; b2 TX reset; b3 DMA mode (no effect — no TXRDY/RXRDY pins); b5-4 TX
  trigger; b7-6 RX trigger; table per FCTR[7:6]; last selection applies to both.
- LCR: standard 16C550.
- MCR: b0 DTR (1 → LOW); b1 RTS (1 → LOW); b2 0 = RTS/CTS, 1 = DTR/DSR for auto flow (OP1 → MSR[6] in
  loopback); b3 reserved (OP2 → MSR[7] in loopback per loopback figure); b4 loopback; b5 Xon-any; b6 IrDA
  (ENIR preset); b7 prescaler ÷1/÷4. b7-5 need EFR[4] (p33; Table 12 shading).
- LSR: b0 DR, b1 OE, b2 PE, b3 FE, b4 BI, b5 THR (FIFO) empty, b6 THR+TSR empty, b7 FIFO error.
- MSR read: as 16C550 (b0 ΔCTS, b1 ΔDSR, b2 TERI, b3 ΔCD, b4 CTS, b5 DSR, b6 RI, b7 CD).
- MSR write: b7-4 auto RS-485 turnaround delay 0-15 bit times (Table 16; "9 1 0 1" row is a typo for
  0101 = 5); b3-0 = 0. Requires EFR[4].
- FCTR: b3-0 RTS/DTR hysteresis for table D (Table 17 p37 — same 16 values as XR16V798: 0000 0, 0001 ±4,
  0010 ±6, 0011 ±8, 0100 ±8, 0101 ±16, 0110 ±24, 0111 ±32, 1100 ±12, 1101 ±20, 1110 ±28, 1111 ±36, 1000 ±40,
  1001 ±44, 1010 ±48, 1011 ±52); b4 IrDA RX invert; b5 auto RS-485 enable; b7-6 trigger table A/B/C/D.
- EFR: b3-0 software flow control (same Table 18 encoding as XR16V564/V798); b4 enhanced-bit enable (IER[7:4],
  ISR[5:4], FCR[5:4], MCR[7:5], MSR[7:4]); b5 special char (XOFF2); b6 auto RTS/DTR; b7 auto CTS/DSR.
- TXCNT/RXCNT: 0-64 FIFO counts. TXTRG/RXTRG: table-D triggers 0-64.
- XCHAR (R): b1 Xon received, b0 Xoff received; self-clear on read; cleared when software flow control is
  disabled.
Trigger tables (Table 14 p31): identical to XR16V798 — A: RX 1/4/8/14, TX 1; B: RX 8/16/24/28, TX 16/8/24/30;
C: RX 8/16/56/60, TX 8/16/32/56; D: RXTRG/TXTRG.
ISR (Table 13 p29): prio 1 LSR 0x06; 2 RX data 0x04; 3 RX timeout 0x0C; 4 TX 0x02; 5 MSR 0x00; 6 Xon/Xoff/
special 0x10; 7 CTS/DSR/RTS/DTR change 0x20; none 0x01.

### Behavior
All as XR16V798 section except:
- Baud = clk / prescaler / (16 or 8 per 8XMODE) / (DLM:DLL), divisor 1..65535 (p8). Max 3.125 Mbps (16x) /
  6.25 Mbps (8x) with 50 MHz ext clock @5 V. Table 4 p9 (14.7456 MHz): 921.6k = divisor 1 … 400 = 2304.
- Sleep (p17, p24): SLEEP=0x0F + no interrupts + MSR[3:0]=0 + all RX idle HIGH → oscillator stops; wake on RX
  start, THR load, modem input change; ready after 32 crystal clocks; a special interrupt with "no pending"
  indication is generated, cleared by reading INT0.
- Auto RS-485 (FCTR[5]): RTS (or DTR? text says RTS#) goes LOW after last stop bit + MSR[7:4] delay, HIGH when
  data loaded (before start bit); TX interrupt becomes TSR empty.
- IrDA: per pin text (p3), "if FCTR[4]=0 a LOW on RXD is a mark" (i.e. idle LOW, HIGH pulse = 0 bit); IrDA
  1.0 in body text, "IrDA 1.1" on p1.
- Reset (Table 19 p40): RST# ≥40 ns low; registers as table (DLL/DLM XX); outputs TX HIGH, RTS HIGH, DTR HIGH
  (IRTX LOW listed — no such pins).

### Timing (p43; min/max; 3.3 V and 5 V columns; 70 pF)
| Sym | Parameter | 3.3 V | 5 V |
|---|---|---|---|
| TC1,TC2 | clock pulse period min | 10 ns | 8 ns |
| TOSC | crystal max | 16 MHz | 24 MHz |
| TECK | ext clock max | 33 MHz | 50 MHz |
| TAS / TAH | addr setup/hold (16) min | 5 / 10 | 5 / 10 |
| TCS | CS width min | 50 | 50 |
| TDY | between CS cycles min | 50 | 50 |
| TRD | read strobe min | 50 | 50 |
| TWR | write strobe min | 40 | 30 |
| TRDV | read data valid max | 35 | 25 |
| TWDS / TWDH | write data setup/hold (16) min | 15 / 15 | 10 / 10 |
| TRDH | read data hold (16) max | 15 | 10 |
| TADS / TADH | (68) min | 10 / 10 | 5 / 10 |
| TRWS | R/W setup to CS min | 10 | 10 |
| TRDA | read access (68) max | 35 | 25 |
| TRDH (68) | max | 15 | 10 |
| TWDS / TWDH (68) | min | 10 / 10 | 10 / 10 |
| TRWH | min | 10 | 10 |
| TCSL / TCSD | (68) min | 50 / 50 | 40 / 40 |
| TWDO | IOW to modem output max | 50 | 50 |
| TMOD | modem input to INT max | 50 | 35 |
| TRSI | IOR to reset INT max | 50 | 35 |
| TSSI | stop to set INT max | 1 Bclk | 1 Bclk |
| TRRI | IOR to reset INT max | 45 | 40 |
| TSI | stop to INT max | 45 | 40 |
| TWRI | IOW to reset INT max | 45 | 40 |
| TRST | reset pulse min | 40 | 40 |

### VCC
2.97-5.5 V (3.3 V/5 V ±10% in DC table); C = 0..70 °C, I = -40..85 °C. Abs max 7 V supply/pins. Inputs 5 V
tolerant except XTAL1; VIH max 6.0 V (p41) → inputMax 5.5 V (use VCC-independent). VIH 2.0 V; VIL 0.7/0.8 V.
ISLEEP 0.6 mA (3.3 V) / 1.5 mA (5 V).

### Package drawing
64-LQFP 10x10x1.4 (p48): D 11.80-12.20 (12.00), D1 9.90-10.10, e 0.50, B 0.17-0.27, L 0.45-0.75, A ≤1.60.
No land pattern.

### Obsolete
None (Active).

### Notes
- Text says "start up all 8 UARTs" / "8 UARTs" in places (copied from XR16L788).
- §2.7 mentions "see EMSR bit-7" — no EMSR exists in this part (stray reference).
- XCHAR row in Table 12 vs V798: only receive indicators.

## SC16C650B

- Title: "SC16C650B — 5 V, 3.3 V and 2.5 V UART with 32-byte FIFOs and infrared (IrDA) encoder/decoder",
  NXP product data sheet Rev. 04, 14 Sept 2009. Datasheet: `datasheets/controller-uart/sc16c650b.pdf` (48 pp;
  ordering p2, pinouts p3-5, pin table p5-8, function p8-15, registers p16-28, limits p29, static p29-30,
  dynamic p30-31, timing figs p32-38, package outlines p39-41, soldering p42+).
- Function: single-channel 16C550-compatible UART (powers up as 16C450), 32-byte TX/RX FIFOs, independent TX
  and RX trigger levels, auto RTS/CTS, auto Xon/Xoff (1 or 2 chars), special-char detect, IrDA encoder/
  decoder, sleep, ÷1/÷4 clock prescaler (MCR[7]), selectable INT output type (push-pull / "open source"),
  ST16C650A-compatible register set. Up to 3 Mbit/s with 48 MHz clock at 5 V (p8). Integer divisor, 16x only.
- Closest relatives here: XR16V564 (same ST16C650A-style EFR bank and 32-byte FIFO trigger tables, but 4
  channels / fractional divisor). SC16C752B = dual 64-byte version with different TCR/TLR registers.

### Parts (census)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| SC16C650BIBS,128 | HVQFN32 5x5x0.85, SOT617-1, exposed pad | 32 | hvqfn32 | industrial -40..+85 °C | 2.5 / 3.3 / 5 V ±10 % |
Ordering Table 1 p2: SC16C650BIA44 (PLCC44, SOT187-2), SC16C650BIB48 (LQFP48 7x7, SOT313-2),
SC16C650BIBS (HVQFN32, SOT617-1); single industrial grade, VCC = 2.5 V, 3.3 V or 5 V ±10 %. ",128" is NXP's
12NC packing suffix (reel), not in the datasheet. Decoded.

### Pinout — hvqfn32 (Fig 4 p5, Table 2 p5-8; drawing checked for active-low marks)
```
1 D5 inout        9 XTAL1 input     17 A1 input       25 CTS input
2 D6 inout        10 XTAL2 output   18 A0 input       26 DSR input
3 D7 inout        11 IOW input      19 RXRDY output   27 VCC power
4 RCLK input      12 n.c.           20 INT output     28 D0 inout
5 RX input        13 GND ground     21 RTS output     29 D1 inout
6 TX output       14 IOR input      22 DTR output     30 D2 inout
7 CS input        15 TXRDY output   23 OUT output     31 D3 inout
8 BAUDOUT output  16 A2 input       24 RESET input    32 D4 inout
EP: exposed pad = GND (die ground on both pin 13 and pad; pin 13 must still be connected) (p8 note 1)
```
Active levels (HVQFN32): CS, IOR, IOW, BAUDOUT, CTS, DSR, RTS, DTR, OUT, RXRDY, TXRDY active LOW; INT active
HIGH (push-pull, or "open source" when MCR[5]=1); RESET active HIGH. HVQFN32 omits AS, CS0/CS1/CS2 (single CS
instead), active-high IOR/IOW, DDIS, DCD, RI, OUT1/OUT2 (single OUT = MCR[2]). In the HVQFN32 the absent
modem inputs: DCD/RI are not bonded — MSR[7]/MSR[6] states for this package are not stated (flag).
Other packages (not in census): PLCC44 / LQFP48 have AS, CS0, CS1, CS2 (selected when CS0=1, CS1=1, CS2=0),
IOR + IOR (active-high read on PLCC 25 / LQFP 20, active-low on PLCC 24 / LQFP 19), IOW + IOW (active-high
PLCC 21 / LQFP 17, active-low PLCC 20 / LQFP 16), DDIS, DCD, RI, OUT1, OUT2 — not transcribed.

### Host interface (p5-8, p30-33)
- HVQFN32: CS active-low; A2..A0; IOR (low) read strobe; IOW (low) write strobe. Data D7..D0 3-state, driven
  only during a selected read. Write data latched with t16s/t16h around the end (rising edge) of IOW (Fig 11).
- (Other packages: AS address strobe — while AS LOW, A2..A0/CS0/CS1/CS2 pass through; on AS rising edge they
  are latched and held while AS HIGH; AS tied low = transparent. DDIS goes LOW while the CPU reads.)
- RESET must be pulsed while CS/IOR/IOW are inactive (p31 note 4).

### Registers (Table 3 p9, Table 8 p16-17, bit tables p17-28, reset Tables 24-25 p28)
| A2-A0 | Condition | Read | Write | Reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | THR | XX |
| 001 | LCR[7]=0 | IER | IER | 00 |
| 010 | LCR[7]=0 | ISR | FCR | ISR 01 / FCR 00 |
| 011 | any | LCR | LCR | 00 |
| 100 | LCR[7]=0 | MCR | MCR | 00 |
| 101 | LCR[7]=0 | LSR | n/a | 60 |
| 110 | LCR[7]=0 | MSR | n/a | X0 ([7:4] = inputs) |
| 111 | LCR[7]=0 | SPR | SPR | FF (Table 8) |
| 000 | LCR[7]=1 (≠BF) | DLL | DLL | XX |
| 001 | LCR[7]=1 (≠BF) | DLM | DLM | XX |
| 010 | LCR=0xBF | EFR | EFR | 00 |
| 100 | LCR=0xBF | Xon1 | Xon1 | 00 |
| 101 | LCR=0xBF | Xon2 | Xon2 | 00 |
| 110 | LCR=0xBF | Xoff1 | Xoff1 | 00 |
| 111 | LCR=0xBF | Xoff2 | Xoff2 | 00 |
Footnotes: general set "accessible only when LCR[7]=0" (so with LCR[7]=1, 010-111 are unspecified except
LCR at 011); "special" set only when LCR[7]=1; enhanced only when LCR=0xBF.

Bits (enhanced bits marked E need EFR[4]=1 to modify; EFR[4]=0 latches them):
- IER: b0 RX data (trigger / timeout); b1 THR empty; b2 RX line status ("issued whenever a fully assembled
  character is transferred to RHR/FIFO" — p18 wording; treat as 16C550 LSR[4:1] error interrupt); b3 modem
  status; b4 sleep (E); b5 Xoff int (E); b6 RTS int — RTS pin 0→1 (E); b7 CTS int — CTS pin 0→1 (E).
- FCR (W): b0 FIFO enable (must be 1 for others); b1 RX FIFO reset (self-clear); b2 TX FIFO reset; b3 DMA mode
  ("not supported in HVQFN32 package, should not be written", Table 8 note 4); b5-4 TX trigger (E):
  00=16, 01=8, 10=24, 11=30; b7-6 RX trigger: 00=8, 01=16, 10=24, 11=28 (Tables 11-12 p20). TX trigger
  default 16 for compatibility; "user can write TX trigger but activation will not take place until EFR[4]=1"
  (p9).
- ISR (R): b0 0=pending; b3-1 code; b4 Xoff/special char detected (E; "stays 1 until Xon received"); b5
  CTS/RTS change (E); b7-6 = 11 when FIFO enabled. Reading ISR clears the current pending interrupt only.
- LCR: b1-0 word length 5-8; b2 stop (1; 1.5 for 5-bit; 2); b3 parity enable; b4 even; b5 forced parity
  (b4=0 → 1, b4=1 → 0); b6 break; b7 divisor latch (and with 0xBF the enhanced set).
- MCR: b0 DTR (1 → pin LOW); b1 RTS (1 → pin LOW); b2 OUT (HVQFN; OUT1 elsewhere): 1 → pin LOW, loopback →
  RI; b3 OUT2 (no pin on HVQFN32, "should not be written"; loopback → DCD); b4 loopback; b5 INT type (E):
  0 = normal push-pull, 1 = "open source" wire-OR (needs 200-500 Ω pull-down); b6 IrDA enable (E); b7 clock
  select ÷1/÷4 (E). Note: unlike 16C550 PC usage, OUT2 does not gate INT.
- LSR: b0 DR; b1 OE (char arrives with FIFO full; shift reg overwritten); b2 PE, b3 FE (top of FIFO); b4 BI
  (one break char loaded); b5 THR empty / TX FIFO empty; b6 THR+TSR empty; b7 FIFO data error — "cleared when
  LSR is read" (p25).
- MSR: b0 ΔCTS, b1 ΔDSR, b2 TERI (RI 0→1), b3 ΔDCD (cleared on MSR read); b4 CTS, b5 DSR, b6 RI, b7 DCD
  (complement of pins; loopback: RTS, DTR, OUT1, OUT2).
- SPR: scratch.
- EFR (LCR=BF): b3-0 Cont-3..0 software flow control (Table 23 p28): b3b2 TX: 00 none, 10 Xon1/Xoff1, 01
  Xon2/Xoff2, 11 both (2 chars); b1b0 RX: 00 none, 10 compare Xon1/Xoff1, 01 compare Xon2/Xoff2;
  1011 → TX Xon1/Xoff1, RX compares Xon1 **and** Xon2, Xoff1 **and** Xoff2; 0111 → TX Xon2/Xoff2, RX
  compares both pairs; 1111 → TX both, RX both pairs (NXP prints "and" for 1011/0111 where Exar prints "or");
  b4 enhanced-function enable/latch (IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5]); b5 special char detect (XOFF2;
  requires EFR[3:0]=0000); b6 auto RTS; b7 auto CTS.
- Xon1/Xon2/Xoff1/Xoff2: flow chars; comparison uses LCR[1:0] word length bits.

ISR priorities (Table 13 p21):
| Prio | ISR[5:0] | Source |
|---|---|---|
| 1 | 000110 | RX line status |
| 2 | 000100 | RX data ready (trigger) |
| 2 | 001100 | RX timeout |
| 3 | 000010 | THR empty / TX below trigger |
| 4 | 000000 | modem status |
| 5 | 010000 | Xoff received / special char |
| 6 | 100000 | CTS/RTS change |
| – | 000001 | none |
Clear: LSR read; RX below trigger / RHR read; ISR read or THR write (TX); MSR read (modem, CTS/RTS change
per 16C650 practice — not explicitly stated); Xoff: until Xon received (p21).

### Behavior
- **Baud / clock** (p12-13): XTAL1 crystal (≤24 MHz) or external clock (≤16 MHz @2.5 V, 32 MHz @3.3 V, 48 MHz
  @5 V, Table 28 note 1). Prescaler ÷1 or ÷4 (MCR[7]). Baud = XTAL1 / prescaler / (16 · divisor),
  divisor = DLM:DLL, 1..65535. BAUDOUT pin = 16x clock (XTAL1/prescaler/divisor). RCLK pin = receiver 16x clock
  input (tie to BAUDOUT for normal use) — on HVQFN32 both pins present (4 RCLK, 8 BAUDOUT).
- **Transmitter**: start, 5-8 data LSB first, parity, 1/1.5/2 stop; TX HIGH idle/reset. Break via LCR[6].
- **Receiver**: start bit validated 7½ 16x clocks after falling edge (p17); RX timeout = 4 character times
  with data in FIFO, counter reset at the center of each received stop bit and on each RHR read; no timeout if
  FIFO empty (p11). Overrun, error tags at FIFO top, one 0x00 char per break.
- **FIFO / DMA** (p14, p19-20): RXRDY mode 0: LOW when ≥1 char, HIGH when empty; mode 1: LOW at trigger or
  timeout, HIGH when empty. TXRDY mode 0: LOW when THR/FIFO empty, HIGH after first char loaded; mode 1: LOW
  when ≥1 empty space, HIGH when FIFO full. "DMA operation is not supported in the HVQFN32 package" (p14) —
  pins exist (15, 19); FCR[3] must stay 0, so treat as mode 0 only.
- **Auto RTS/CTS** (p10, p27): CTS HIGH stops TX after current char's stop bit; ISR[5] if IER[7]. Auto RTS:
  interrupt at trigger; RTS goes HIGH "at the next trigger level" and returns LOW below "the next lower
  trigger level". Table 4 p10 instead gives: trigger 8 → negate RTS/send Xoff at 8, assert RTS/send Xon at 0;
  16 → 16 / 7; 24 → 24 / 15; 28 → 28 / 23 (i.e. RTS off at the trigger, back on 1 below the previous level) —
  conflict with text (flag; ST16C650A behaves per Table 4).
- **Xon/Xoff** (p10-11): received Xoff (1 or 2 chars per EFR) halts TX after current char; Xon resumes; flow
  chars not stored. TX side: Xoff sent "as soon as received data passes the programmed trigger level"; Xon
  sent when data drops below the next lower level (Table 4 values above).
- **Special char** (EFR[5]): match with Xoff2 → char stored in FIFO, ISR[4] set.
- **IrDA** (MCR[6], Fig 20 p38): TX 3/16-bit HIGH pulse for each 0 bit, idle LOW; RX: pulse → 0, 0-1 16x clock
  delay.
- **Sleep** (p14): EFR[4]=1 and IER[4]=1 → sleeps when no interrupt pending; wake on start bit, modem input
  change (RI, CTS, DSR, DCD), RX, or THR write; auto re-sleep after last char transmitted/read; XTAL2 floats in
  sleep (p8 note 2). ICC(sleep) ≤50 µA.
- **Loopback** (p14-15, MCR[4]): TX→RX internal, TX pin HIGH?; modem inputs disconnected; CTS←RTS, DSR←DTR,
  RI←OUT1, DCD←OUT2; RTS/DTR/OUT pins go inactive (HIGH) per pin descriptions; interrupts work.
- **Reset** (Tables 24-25 p28): RESET active HIGH; tRESET ≥100 ns @2.5 V, 40 ns @3.3/5 V. IER=00, ISR=01,
  LCR=00, MCR=00, LSR=60, MSR[3:0]=0, FCR=00, EFR=00 (SPR=FF, Xon/Xoff=00, DLL/DLM=XX per Table 8). Outputs:
  TX HIGH, RTS HIGH, DTR HIGH, RXRDY HIGH, TXRDY LOW, INT LOW; OUT HIGH (pin text).
- Internal pull-up resistance ≥500 kΩ (Table 27; which pins not stated).

### Timing (Table 28 p30-31; min/max; 2.5 V / 3.3 V / 5.0 V ±10 %, 25 pF where noted)
| Sym | Parameter | 2.5 V | 3.3 V | 5.0 V |
|---|---|---|---|---|
| tWL / tWH | ext clock pulse width min | 15 / 15 | 13 / 13 | 10 / 10 ns |
| fXTAL1 | ext clock max (crystal ≤24 MHz) | 16 | 32 | 48 MHz |
| t4w | AS width min | 45 | 35 | 25 |
| t5s / t5h | address setup/hold to AS min | 5 / 5 | 5 / 5 | 1 / 5 |
| t6s | CS setup to AS min | 10 | 5 | 0 |
| t6s' / t7h' | address setup/hold (AS tied low) min | 10 / 5 | 10 / 5 | 5 / 5 |
| t6h | CS / address hold min | 0 | 0 | 0 |
| t7d | IOR delay from CS min | 10 | 10 | 10 |
| t7w | IOR width min | 77 | 26 | 23 |
| t7h | CS hold from IOR min | 0 | 0 | 0 |
| t8d | IOR delay from address min | 10 | 10 | 10 |
| t9d | read cycle delay min | 20 | 20 | 20 |
| t11d | IOR to DDIS max | 100 | 35 | 30 |
| t12d | IOR to data valid max | 77 | 26 | 23 |
| t12h | data disable max | 15 | 15 | 15 |
| t13d | IOW delay from CS min | 10 | 10 | 10 |
| t13w | IOW width min | 20 | 20 | 15 |
| t13h | CS hold from IOW min | 0 | 0 | 0 |
| t14d | IOW delay from address min | 10 | 10 | 10 |
| t15d | write cycle delay min | 25 | 25 | 20 |
| t16s / t16h | data setup/hold min | 20 / 15 | 20 / 5 | 15 / 5 |
| t17d | IOW to modem output max | 100 | 33 | 29 |
| t18d | modem input to INT max | 100 | 24 | 23 |
| t19d | IOR to reset INT (modem) max | 100 | 24 | 23 |
| t20d | stop to set INT max | 1 RCLK | 1 RCLK | 1 RCLK |
| t21d | IOR to reset INT max | 100 | 29 | 28 |
| t22d | start to set INT max | 100 | 45 | 40 |
| t23d | IOW to TX start | 8-24 RCLK | 8-24 RCLK | 8-24 RCLK |
| t24d | IOW to reset INT max | 100 | 45 | 40 |
| t25d | stop to set RXRDY max | 1 RCLK | 1 RCLK | 1 RCLK |
| t26d | IOR to reset RXRDY max | 100 | 45 | 40 |
| t27d | IOW to set TXRDY max | 100 | 45 | 40 |
| t28d | start to reset TXRDY max | 8 RCLK | 8 RCLK | 8 RCLK |
| tRESET | RESET pulse min | 100 | 40 | 40 |
| N | divisor | 1..65535 | | |

### VCC
2.5 V, 3.3 V or 5 V, each ±10 % (2.25-5.5 V overall), -40..+85 °C. Limits (p29): VCC ≤7 V; data bus pins
-0.3..VCC+0.3; **input-only pins -0.3..5.3 V** ("5 V tolerant on input only pins"). inputMax: 5.3 V for
input-only pins, VCC for D7..D0. VIH 1.6 / 2.0 / 2.2 V; VIL 0.65 / 0.8 / 0.8 V.

### Package drawing
HVQFN32 SOT617-1 (JEDEC MO-220), body 5x5x0.85 (Fig 24 p41): D/E 4.9-5.1, Dh/Eh (pad) 2.95-3.25 (3.10),
e 0.5, e1/e2 3.5, b 0.18-0.30 (0.24), L 0.3-0.5 (0.4), A ≤1.0, A1 0-0.05. Soldering/footprint guidance
p42+ is generic (no land pattern dims for SOT617-1 found in text).

### Obsolete
No (Active).

### Notes
- Auto-RTS hysteresis: text (next trigger level) vs Table 4 (RTS off at trigger, on at previous-level − 1).
- DMA mode unsupported on HVQFN32 yet TXRDY/RXRDY pins exist.
- HVQFN32 lacks DCD/RI pins; MSR[7:6] value unspecified (likely reads as inactive).
- LSR[7] clears on LSR read (16C550 style), differs from Exar "clears when no errors remain".
- SPR/DLL/DLM missing from Table 24 reset list (Table 8 default column: SPR FF, DLL/DLM XX).

## SC16C752B

- Title: "SC16C752B — 5 V, 3.3 V and 2.5 V dual UART, 5 Mbit/s (max.), with 64-byte FIFOs" (title text prints
  "5 V, 2.2 V and 2.5 V" — typo), NXP product data sheet Rev. 6.1, 5 Feb 2025. Datasheet:
  `datasheets/controller-uart/sc16c752b.pdf` (45 pp; ordering p2, pinouts p4, pin table p5-7, function p7-17,
  registers p18-28, limits p30, static p30-31, dynamic p31-32, timing figs p32-35, packages p37-38).
- Function: dual UART (channels A, B), each 64-byte TX/RX FIFOs (RX with 3 error bits per byte), TI
  TL16C752-style register set: EFR at LCR=0xBF, Xon1/2, Xoff1/2, **TCR** (auto-RTS / software flow halt and
  resume levels) and **TLR** (programmable TX/RX trigger levels, granularity 4), **FIFO Rdy** register,
  auto-RTS/CTS, Xon/Xoff (1 or 2 chars), Xon-any, special char, sleep, ÷1/÷4 prescaler. No IrDA, no fractional
  divisor, 16x only. Pin/software compatible with SC16C752 / TL16C752, pin compatible with SC16C2550 (p1).
  Up to 5 Mbit/s at 3.3/5 V (80 MHz clock), 3 Mbit/s at 2.5 V.
- Differs from SC16C650B: two channels with separate CSA/CSB and INTA/INTB, 64-byte FIFOs, TCR/TLR instead of
  fixed hysteresis tables, MCR[3] gates INT (3-state) and drives OP pins, MCR[2] = FIFO Rdy enable, LCR reset =
  0x1D, no IrDA.

### Parts (census)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| SC16C752BIB48,128 | LQFP48 7x7x1.4, SOT313-2 | 48 | lqfp48 | -40..+85 °C | 2.5 / 3.3 / 5 V ±10 % |
| SC16C752BIBS,157 (two census rows, two DigiKey maker names) | HVQFN32 5x5x0.85, SOT617-1 | 32 | hvqfn32 | -40..+85 °C | 2.5 / 3.3 / 5 V ±10 % |
Ordering Table 1 p2: SC16C752BIB48 (LQFP48, SOT313-2), SC16C752BIBS (HVQFN32, SOT617-1). ",128"/",157" are
NXP 12NC packing variants (not in the datasheet). Both decoded; single industrial grade.

### Pinouts (Fig 2/3 p4, Table 2 p5-7)
**lqfp48**:
```
1 D5 inout        13 XTAL1 input    25 n.c.           37 n.c.
2 D6 inout        14 XTAL2 output   26 A2 input       38 CTSA input
3 D7 inout        15 IOW input      27 A1 input       39 DSRA input
4 RXB input       16 CDB input      28 A0 input       40 CDA input
5 RXA input       17 GND ground     29 INTB output    41 RIA input
6 TXRDYB output   18 RXRDYB output  30 INTA output    42 VCC power
7 TXA output      19 IOR input      31 RXRDYA output  43 TXRDYA output
8 TXB output      20 DSRB input     32 OPA output     44 D0 inout
9 OPB output      21 RIB input      33 RTSA output    45 D1 inout
10 CSA input      22 RTSB output    34 DTRA output    46 D2 inout
11 CSB input      23 CTSB input     35 DTRB output    47 D3 inout
12 n.c.           24 n.c.           36 RESET input    48 D4 inout
```
**hvqfn32** (no DSR/DTR/CD/RI, no TXRDY/RXRDY):
```
1 D6 inout        9 CSB input       17 A2 input       25 CTSA input
2 D7 inout        10 XTAL1 input    18 A1 input       26 VCC power
3 RXB input       11 XTAL2 output   19 A0 input       27 D0 inout
4 RXA input       12 IOW input      20 INTB output    28 D1 inout
5 TXA output      13 GND ground     21 INTA output    29 D2 inout
6 TXB output      14 IOR input      22 OPA output     30 D3 inout
7 OPB output      15 RTSB output    23 RTSA output    31 D4 inout
8 CSA input       16 CTSB input     24 RESET input    32 D5 inout
EP: exposed thermal pad (Dh/Eh 3.1 mm per SOT617-1); this sheet gives no connection note (the SC16C650B
sheet for the same outline says die ground is on the pad — connect to GND).
```
Active levels: CSA, CSB, IOR, IOW, CTS, DSR, CD, RI, RTS, DTR, TXRDY, RXRDY active LOW; INTA/INTB active
HIGH, 3-state unless MCR[3]=1; OPA/OPB = NOT MCR[3] (HIGH after reset); RESET active HIGH.
HVQFN32: absent DSR/CD/RI inputs → MSR[5]/[6]/[7] unspecified (flag).

### Host interface (p5-6, p32-33)
Intel-style: CSA (channel A) / CSB (channel B) active-low; A2..A0 register select; IOR low → read
("HIGH-to-LOW transition on IOR loads register onto D bus"); IOW: "LOW-to-HIGH transition on IOW transfers the
data bus into the register" (write latched at IOW rising edge). D7..D0 3-state. Both CS low at once: not
defined except FIFO Rdy read (either CS low). Dynamic table also specifies tsu3 (IOW/IOR assertion to XTAL1
rising, 20 ns) and th5 (XTAL1 falling to IOW/IOR release, 20 ns) — implies the bus strobes are sampled by
XTAL1 internally (flag; requires a running clock for register access).

### Registers (Table 9 p18, Table 10 p18-19, bits p20-27, reset Table 4 p11)
| A2-A0 | Access condition | Read | Write | Reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | THR | pointers cleared |
| 001 | LCR[7]=0 | IER | IER | 00 |
| 010 | LCR[7]=0 | IIR | FCR | IIR 01 / FCR 00 |
| 011 | any | LCR | LCR | **1D** |
| 100 | LCR[7]=0 | MCR | MCR | 00 |
| 101 | LCR[7]=0 | LSR | – | 60 |
| 110 | LCR[7]=0 (and not TCR mode) | MSR | – | [3:0]=0, [7:4]=inputs |
| 111 | LCR[7]=0 (not TLR / FIFO Rdy mode) | SPR | SPR | not reset |
| 000 | LCR[7]=1 | DLL | DLL | not reset |
| 001 | LCR[7]=1 | DLM | DLM | not reset |
| 010 | LCR=0xBF | EFR | EFR | 00 |
| 100 | LCR=0xBF | Xon1 | Xon1 | not reset |
| 101 | LCR=0xBF | Xon2 | Xon2 | not reset |
| 110 | LCR=0xBF | Xoff1 | Xoff1 | not reset |
| 111 | LCR=0xBF | Xoff2 | Xoff2 | not reset |
| 110 | EFR[4]=1 and MCR[6]=1 | TCR | TCR | 00 |
| 111 | EFR[4]=1 and MCR[6]=1 | TLR | TLR | 00 |
| 111 | CSA or CSB low, MCR[2]=1, MCR[4]=0 | FIFO Rdy | – | – |
(Table 9 notes [3]-[6]. The TCR/TLR condition presumably also requires LCR[7]=0 / LCR≠BF — not stated.
"DLL, DLM, SPR, XON1, XON2, XOFF1, XOFF2 are not reset by RESET" (Table 4 note).)

Bits (bits marked E writable only when EFR[4]=1):
- IER: b0 RHR (RX data / timeout); b1 THR; b2 RX line status; b3 modem status; b4 sleep (E); b5 Xoff (E);
  b6 RTS int (E); b7 CTS int (E). "Re-enabling IER[1] will not cause a new interrupt if the THR is below the
  threshold" (p24).
- IIR: b0 0 = pending; b3-1 code; b4 Xoff/special char detected; b5 RTS/CTS LOW→HIGH change; b7-6 mirror
  FCR[0].
- FCR (W): b0 FIFO enable; b1 RX FIFO reset; b2 TX FIFO reset (self-clearing); b3 DMA mode 0/1; b5-4 TX trigger
  (E): 00 = 8 spaces, 01 = 16, 10 = 32, 11 = 56; b7-6 RX trigger: 00 = 8 chars, 01 = 16, 10 = 56, 11 = 60.
  Overridden by TLR nibbles when non-zero.
- LCR: b1-0 word 5-8; b2 stop 1 / 1.5 / 2; b3 parity enable; b4 even; b5 forced parity (b4=0 → 1, b4=1 → 0);
  b6 break; b7 DLAB. Reset value printed as 0x1D = 0001 1101 → 6-bit word, 2 stop bits, parity enabled,
  even parity (Table 4 p11; as printed).
- MCR: b0 DTR (1 → LOW; loopback → MSR[5]); b1 RTS (1 → LOW; loopback → MSR[4]); b2 FIFO Rdy register
  enable (loopback → MSR[6]); b3 IRQ enable / OP: 0 → INTA/INTB 3-state and OP pin HIGH; 1 → INT active and OP
  LOW (loopback → MSR[7]); b4 loopback; b5 Xon-any (E); b6 TCR/TLR access enable (E); b7 clock ÷1/÷4 (E).
- LSR: b0 data in RX FIFO; b1 overrun; b2 PE, b3 FE, b4 BI (top-of-FIFO char; LSR[4:2] read straight from the
  FIFO word, 0 if FIFO empty); b5 THR (TX FIFO) empty; b6 THR+TSR empty; b7 error anywhere in RX FIFO
  (cleared when no errors remain). Reading LSR does not advance the FIFO.
- MSR: b0 ΔCTS, b1 ΔDSR, b2 TERI (RI LOW→HIGH), b3 ΔCD (cleared on read); b4 CTS, b5 DSR, b6 RI, b7 CD
  (complement of pins; loopback: MCR[1], MCR[0], MCR[2], MCR[3]).
- SPR: scratch.
- EFR (LCR=BF): b3-0 software flow control (Table 3 p9: TX b3b2 = 00 none, 10 Xon1/Xoff1, 01 Xon2/Xoff2,
  11 both; RX b1b0 = 00 none, 10 compare Xon1/Xoff1, 01 compare Xon2/Xoff2; 1011/0111/1111 → RX compares Xon1
  **and** Xon2, Xoff1 and Xoff2 (sequential)); b4 enhanced functions / write enable for IER[7:4], FCR[5:4],
  MCR[7:5]; b5 special char detect (XOFF2); b6 auto RTS; b7 auto CTS.
- TCR: b3-0 RX FIFO halt level, b7-4 resume level, each nibble × 4 (0-60 chars). Must program halt > resume
  before enabling auto-RTS/software flow; no hardware check (p26).
- TLR: b3-0 TX trigger (spaces) = nibble × 4, b7-4 RX trigger (chars) = nibble × 4 (4-60); nibble 0 → use FCR
  value. With TLR RX in use, leave FCR[7:6]=00.
- FIFO Rdy (R): b0 TX FIFO A status, b1 TX FIFO B status, b4 RX FIFO A status, b5 RX FIFO B status (DMA
  status — presumably the TXRDY/RXRDY signal states; polarity not stated), others 0.
- DLL/DLM: divisor; DLL=DLM=0 → no baud clock ("UART effectively disabled"). Must not be written while sleep
  enabled.

Interrupt priority (Table 6 p12, Table 18 p25):
| Prio | IIR[5:0] | Type | Source | Reset |
|---|---|---|---|---|
| 1 | 000110 | RX line status | OE, FE, PE, BI in RX FIFO (via LSR[7] for FE/PE/BI) | FE/PE/BI: all erroneous chars read out; OE: read LSR |
| 2 | 001100 | RX timeout | stale data in RX FIFO | read RHR |
| 2 | 000100 | RHR | data ready (FIFO off) / RX FIFO above trigger | read RHR (below trigger) |
| 3 | 000010 | THR | THR empty / TX FIFO passes above trigger (spaces) | read IIR or write THR |
| 4 | 000000 | modem | MSR[3:0] set | read MSR |
| 5 | 010000 | Xoff / special char | Xoff received / special char | Xon received / read IIR (p9: special char cleared by IIR read; p12: by LSR read — conflict) |
| 6 | 100000 | CTS/RTS | pin change active→inactive (LOW→HIGH) | read IIR |
| – | 000001 | none | | |

### Behavior
- **Baud** (p15-17): XTAL1 crystal (≤24 MHz) or external clock (≤48 MHz @2.5 V, ≤80 MHz @3.3/5 V). Baud =
  XTAL1 / prescaler(1 or 4, MCR[7], default 1) / (16 · divisor); divisor 1..65535; 0 = no clock. No BAUDOUT.
- **TX/RX**: 5-8 bits LSB first, parity, 1/1.5/2 stop; RX sampled mid-bit; RX timeout when RX idle (HIGH) for 4
  character times with data in FIFO (p15); overrun: with FIFO disabled the single location is overwritten.
- **FIFO / DMA** (p13-15, pin text): mode 0 / FIFO off: TXRDY LOW when TX FIFO/THR empty, HIGH after one char
  loaded; RXRDY LOW when ≥1 char, HIGH when empty. Mode 1: TXRDY LOW when ≥ trigger-level spaces available,
  HIGH when FIFO full; RXRDY LOW at trigger or timeout, HIGH when FIFO empty **or an error is flagged by
  LSR[7]**.
- **Auto RTS** (p8): RTS active (LOW) while RX FIFO level < TCR halt level; deasserted (HIGH) when halt level
  reached; reasserted when FIFO drops to the TCR resume level. **Auto CTS**: transmitter checks CTS before each
  byte; CTS must go HIGH before the middle of the last stop bit to stop the next byte. With auto-CTS on, CTS
  changes don't raise modem interrupts (p8).
- **Software flow** (p9-10): RX Xoff (1 or 2 sequential chars) halts TX after current char, sets IIR[4] (if
  IER[5]); Xon resumes and clears IIR[4]. TX: Xoff sent when RX FIFO passes TCR halt level; Xon sent when RX
  FIFO reaches TCR resume level; flow chars sent with current word length (LS bits). Xon-any (MCR[5]): any char
  resumes ("an Xon1 may be taken as Xon-any, causing Xon2 to be written to the FIFO"). Special char: match Xoff2
  → sets IIR[4], char stored, TX not halted. Software and hardware flow are assumed never both enabled.
- **Sleep** (p15): EFR[4]=1 and IER[4]=1; entered when RX idle, TX FIFO and TSR empty, no interrupts pending
  except THR and timeout, RX FIFO empty. UART clock and baud clock stop (per channel). Wake on RX change, modem
  input change, or TX FIFO write. Don't write DLL/DLM during sleep. ICC(sleep) ≤50 µA.
- **Loopback** (MCR[4]): TX internally → RX, TX pin disabled (HIGH presumed), MCR[3:0] → MSR[7:4]
  (DTR→DSR, RTS→CTS, MCR[2]→RI, MCR[3]→CD).
- **Reset** (Tables 4-5 p11): RESET active HIGH (≥100 ns @2.5 V, ≥40 ns @3.3/5 V; only while CSA, CSB, IOR, IOW
  inactive). IER 00, IIR 01, FCR 00, LCR 1D, MCR 00, LSR 60, MSR[3:0]=0, EFR 00, TCR 00, TLR 00, FIFO pointers
  cleared; DLL/DLM/SPR/Xon/Xoff kept. Outputs: TX HIGH, RTS HIGH, DTR HIGH, RXRDY HIGH, TXRDY LOW, INTA/INTB
  3-state (high-Z), OPA/OPB HIGH.

### Timing (Table 26 p31-32; min/max; 2.5 V / 3.3 V or 5 V ±10 %, 25 pF)
| Sym | Parameter | 2.5 V | 3.3/5 V |
|---|---|---|---|
| td1 | IOR delay from CS min | 10 | 0 ns |
| td2 | read cycle delay min | 20 | 20 |
| td3 | IOR to data valid max | 77 | 26 |
| td4 | data disable max | 15 | 15 |
| td5 | IOW delay from CS min | 10 | 10 |
| td6 | write cycle delay min | 25 | 25 |
| td7 | IOW to modem output max | 100 | 33 |
| td8 | modem input to INT max | 100 | 24 |
| td9 | IOR to reset INT (modem) max | 100 | 24 |
| td10 | stop to set INT max | 1 RCLK | 1 RCLK |
| td11 | IOR to reset INT max | 100 | 29 |
| td12 | start to set INT max | 100 | 100 |
| td13 | IOW to TX start | 8-24 RCLK | 8-24 RCLK |
| td14 | IOW to reset INT max | 100 | 70 |
| td15 | stop to set RXRDY max | 1 RCLK | 1 RCLK |
| td16 | IOR to reset RXRDY max | 100 | 75 |
| td17 | IOW to set TXRDY max | 100 | 70 |
| td18 | start to reset TXRDY max | 16 RCLK | 16 RCLK |
| td19 | between successive IOW and IOR min | 20 | 20 |
| th1/th2 | CS hold from IOR/IOW min | 0 | 0 |
| th3 | data hold min | 15 | 15 |
| th4 | address hold min | 0 | 0 |
| th5 | XTAL1 HIGH→LOW to IOW/IOR release min | 20 | 20 |
| tp1/tp2 | clock period min | 10 | 6 |
| fXTAL1 | ext clock max (crystal ≤24) | 48 MHz | 80 MHz |
| tw(RESET) | min | 100 | 40 |
| tsu1 | address setup min | 0 | 0 |
| tsu2 | data setup min | 16 | 16 |
| tsu3 | IOW/IOR assertion to XTAL1 LOW→HIGH min | 20 | 20 |
| tw1 | IOR width min | 77 | 30 |
| tw2 | IOW width min | 30 | 30 |
(RCLK = internal 16x baud clock.)

### VCC
2.5 V, 3.3 V or 5 V, each ±10 %, -40..+85 °C (p30). Limits: VCC ≤7 V; D7..D0 -0.3..VCC+0.3; input-only pins
-0.3..5.3 V ("5 V tolerant on input only pins") → inputMax 5.3 V for inputs, VCC for data bus. VIH 1.6 V
(2.5 V) / 2.0 V; VIL 0.65 / 0.8 V. Ci 18 pF.

### Package drawings
- LQFP48 SOT313-2 (JEDEC MS-026) (Fig 23 p37): D/E 6.9-7.1 body, HD/HE 8.85-9.15 span, e 0.5, bp 0.17-0.27,
  Lp 0.45-0.75 (L 1.0), A ≤1.6, A1 0.05-0.20, A2 1.35-1.45, c 0.12-0.18, ZD/ZE 0.55-0.95.
- HVQFN32 SOT617-1 (JEDEC MO-220) (Fig 24 p38): D/E 4.9-5.1, Dh/Eh 2.95-3.25, e 0.5, e1/e2 3.5, b 0.18-0.30,
  L 0.3-0.5, A ≤1.0, A1 0-0.05.
No land patterns in this sheet.

### Obsolete
No (Active).

### Notes
- Title voltage typo ("2.2 V"). Description mentions "status of TXRDYn/RXRDYn for all four ports" (copied from
  SC16C754B); FIFO Rdy has 2 channels.
- LCR reset value 0x1D is as printed (unusual vs 16C550 0x00).
- Special-char interrupt clear: IIR read (p9) vs LSR read (p12).
- tsu3/th5 relate host strobes to XTAL1 edges (synchronous bus sampling).
- HVQFN32 exposed-pad connection not stated in this sheet.

