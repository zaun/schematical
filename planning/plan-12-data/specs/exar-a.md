# Exar group A — ST16C450 / 1450 / 550 / 1550 / 2550 / 650A / 554, XR16C2550, XR16L580

Fact sheets for plan-12 (UART models), extracted from the PDFs below. Pages are PDF pages unless a section
says otherwise (ST16C554 uses printed page = PDF page − 6; ST16C1450 uses PDF page = printed page + 2).
Pins named without overbar marks; `(#)` in pin lists means active low. Where both polarities of a strobe exist
(IOR and IOR#), the active-low one is `IORBAR` / `IOWBAR`.

## PDF → devices

| PDF (datasheets/controller-uart/) | devices covered | census parts |
|---|---|---|
| st16c450.pdf | ST16C450 | ST16C450CQ48, ST16C450IQ48 |
| st16c1450.pdf | ST16C1450 (ST16C1451 is a 28-pin variant with no census part) | ST16C1450CQ48 |
| st16c550.pdf | ST16C550 | ST16C550CJ44-F, ST16C550IJ44, ST16C550IJ44-F, ST16C550CQ48-F, ST16C550IQ48-F, ST16C550IQ48TR-F |
| st16c1550.pdf | ST16C1550 (ST16C1551 has the same TQFP48; it differs only in the 28-PLCC, which has no census part) | ST16C1550IQ48-F |
| st16c2550.pdf | ST16C2550 | ST16C2550CQ48-F, ST16C2550IQ48-F, ST16C2550IQ48TR-F |
| st16c650a.pdf | ST16C650A (PDF watermarked OBS) | ST16C650ACJ44; also ST16C650CJ44 and ST16C650IJ44 (non-A; see below) |
| st16c554.pdf | ST16C554 / ST16C554D (one die; the LQFP64 parts differ only in how INTSEL is bonded); ST68C554 has no census part | ST16C554CQ64-F, ST16C554DCQ64-F, ST16C554DIQ64-F |
| xr16c2550.pdf | XR16C2550 (same as ST16C2550) | XR16C2550IM-F |
| xr16l580.pdf | XR16L580 | XR16L580IL-F, XR16L580ILTR-F, XR16L580IM-F |

Device sections, in order: ST16C450, ST16C1450, ST16C550 (the reference 16550 core), ST16C2550, ST16C1550,
ST16C650A, ST16C554, XR16C2550, XR16L580.

### Census parts that could not be fully decoded or that need a correction
- **ST16C650CJ44, ST16C650IJ44** (the non-A ST16C650): st16c650a.pdf covers only the ST16C650A. Its suffixes decode
  the usual way (CJ44 = commercial PLCC44, IJ44 = industrial PLCC44), but this PDF does not document the
  non-A part's pinout, registers or electrical specs. DigiKey's description for ST16C650IJ44 is the 650A headline
  text, so it may be the A die sold under the old number. Treat both as ST16C650A (marked as an assumption), or
  leave them out.
- **XR16L580ILTR-F**: the census case column says 24-VFQFN 4x4, but the ordering table (p3) says ILTR-F is the
  **32-QFN 5x5**; the 24-pin parts are IL24-F and IL24TR-F. The census description ("32QFN") agrees with the
  ordering table.
- **ST16C450CQ48, ST16C450IQ48**: these decode cleanly, but every page of the PDF carries an obsolete (OBS)
  watermark, while the census lists both as Active.
- Wrong DigiKey descriptions, harmless: ST16C550IJ44 ("PIPELINE REGISTER") and ST16C650ACJ44 ("VIDEO AMP").

### Obsolete census rows
ST16C550CJ44-F, ST16C550IJ44, ST16C550IJ44-F, ST16C650ACJ44, ST16C650CJ44, XR16L580IL-F, XR16L580ILTR-F.

### Package/pinout names used
| name | parts |
|---|---|
| tqfp48 | 48-TQFP 7x7, e = 0.5. The pin maps differ per device: ST16C450, ST16C1450, ST16C550, ST16C1550, ST16C2550 (= XR16C2550), XR16L580 |
| plcc44 | 44-PLCC, e = 1.27. ST16C550 and ST16C650A (the 650A adds SEL on pin 34) |
| lqfp64 | 64-LQFP 10x10, e = 0.5. ST16C554 / ST16C554D |
| qfn32 | 32-QFN 5x5 with exposed GND pad. XR16L580 |

## ST16C450

- Title (as printed): "ST16C450 UNIVERSAL ASYNCHRONOUS RECEIVER/TRANSMITTER (UART)", Rev. 4.20, September 2003.
  Function: NS16450-compatible single UART, no FIFO ("Pin to pin and functionally compatible to the
  Industry Standard 16450", p1). Datasheet: `datasheets/controller-uart/st16c450.pdf`, 28 pages
  (PDF page = printed page). Every page carries a red watermark "The product (or products) mentioned
  in this data sheet are no longer being manufactured and may not be ordered (OBS)" — see Obsolete.
- Nearest sibling: ST16C1450 (below) is NOT register-identical (different INT gating, THR-interrupt
  clearing, IER[5] special mode) and has a reduced pin set.

### Parts
| census part | package | pins | pinout name | grade | supply |
|---|---|---|---|---|---|
| ST16C450CQ48 | 48-Lead TQFP (7x7, 0.5 mm) | 48 | tqfp48 | C = commercial 0..+70 °C | 3.3 V ±10% or 5.0 V ±10% (tables p18-19); front page "2.97 to 5.5 volt operation" (p1) |
| ST16C450IQ48 | 48-Lead TQFP | 48 | tqfp48 | I = industrial -40..+85 °C | same |
Ordering table p1: CP40 = 40-PDIP, CJ44 = 44-PLCC, CQ48 = 48-TQFP; C = 0..70 °C, I = -40..+85 °C.
Census rows have maker Sipex and blank package/status columns; both decoded from the part number.

### Pinout tqfp48 (Figure 1 p2; pin table p4-7)
Overbar convention: the chip has both IOR and -IOR, IOW and -IOW, so the active-low ones get `BAR`.
```
1  NC       nc
2  D5       inout
3  D6       inout
4  D7       inout
5  RCLK     input   receiver 16x clock input
6  NC       nc
7  RX       input
8  TX       output
9  CS0      input   active high
10 CS1      input   active high
11 CS2      input   active low (-CS2)
12 BAUDOUT  output  active low name (-BAUDOUT): 16x baud clock out
13 NC       nc
14 XTAL1    input
15 XTAL2    output
16 IOWBAR   input   -IOW, active-low write strobe
17 IOW      input   active-high write strobe
18 GND      ground
19 IORBAR   input   -IOR, active-low read strobe
20 IOR      input   active-high read strobe
21 NC       nc
22 DDIS     output  -DDIS, active low
23 CSOUT    output  active high
24 AS       input   -AS, address strobe, active low
25 NC       nc
26 A2       input
27 A1       input
28 A0       input
29 NC       nc
30 INT      output  active high, always driven
31 OP2      output  -OP2, active low
32 RTS      output  -RTS, active low
33 DTR      output  -DTR, active low
34 OP1      output  -OP1, active low
35 RESET    input   active high
36 NC       nc
37 NC       nc
38 CTS      input   -CTS, active low
39 DSR      input   -DSR, active low
40 CD       input   -CD, active low
41 RI       input   -RI, active low
42 VCC      power
43 D0       inout
44 D1       inout
45 D2       inout
46 D3       inout
47 D4       inout
48 NC       nc
```
No exposed pad. (DIP40 / PLCC44 pin numbers are in the p4-7 table but no census part uses them.)

### Host interface (p4-7, p18, timing figures p21-22)
- Intel-style I/O bus with dual-polarity strobes: read when (IORBAR=0 OR IOR=1), write when
  (IOWBAR=0 OR IOW=1) — "Either an active -IOR or IOR is required" (p4), same for IOW (p4). The unused
  strobe of each pair must be tied inactive.
- Chip select = CS0=1 AND CS1=1 AND CS2=0 (p4).
- AS (-AS): address/chip-select latch. Pin text (p4): "A logic 0 transition on -AS latches the state
  of the chip selects and the register select bits A0-A2 ... If not required, the -AS input can be
  permanently tied to a logic 0 (it is edge triggered)." Read/write timing figures (p21/p22): T4w is
  the -AS low pulse; T5s (address setup) and T6s (CS setup) end at the **rising** edge of -AS, T5h/T6h
  (hold) start there. So: transparent while AS=0, latched on AS rising edge (16450 ADS behavior); the
  text's "logic 0 transition" conflicts — see Notes. With AS tied low, address/CS are live
  (T7d/T8d/T13d/T14d apply "only when -AS is tied low", p18 note 1).
- CSOUT: high when the chip is selected (p5); T10d CSOUT delay from chip select. Reset state 0 (p17).
- DDIS (-DDIS): goes low while the CPU is reading the chip (p5) — i.e. selected AND read strobe
  active; T11d delay from -IOR both edges (p21 figure). Intended to disable external bus transceivers.
- Read: data driven onto D7..D0 T12d after read strobe active; 3-state T12h after the strobe ends
  (p18, p21). Write: data latched at the end (rising -IOW / falling IOW) of the write strobe:
  T16s setup and T16h hold are drawn around the trailing edge (p22).
- D0 is the LSB and the first serial data bit (p4).

### Registers (Table 2 p8, Table 4 p12, bit text p13-17)
| A2A1A0 | cond | name | R/W | reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | R | XX |
| 000 | LCR[7]=0 | THR | W | XX |
| 001 | LCR[7]=0 | IER | R/W (Table 2 lists write only; Table 4 shows it as a register with default 00) | 00 |
| 010 | — | ISR (IIR) | R | 01 |
| 011 | — | LCR | R/W (Table 2 lists write only) | 00 |
| 100 | — | MCR | R/W (Table 2 lists write only) | 00 |
| 101 | — | LSR | R (Table 2) | 60 |
| 110 | — | MSR | R | X0 (bits 7-4 = inverted input pins) |
| 111 | — | SPR | R/W | FF (Table 4) |
| 000 | LCR[7]=1 | DLL | R/W | XX |
| 001 | LCR[7]=1 | DLM | R/W | XX |
Write to 010 (ISR) and to 101/110: no register listed (Table 2 write column blank).

Bits:
- IER: [0] RX data ready int enable; [1] THR empty int enable; [2] receiver line status int enable;
  [3] modem status int enable; [7:4] "Not used and set to 0" (p13).
- ISR: [0] 0 = interrupt pending, 1 = none (default); [3:1] source code; [7:4] = 0 (p14).
  Table 5 (p14): priority 1 `0110` LSR (receiver line status); 2 `0100` RXRDY (received data ready);
  3 `0010` TXRDY (THR empty); 4 `0000` MSR; `0001` none.
- LCR (p14-15): [1:0] word length 00=5, 01=6, 10=7, 11=8; [2] stop bits 0 = 1, 1 = 1.5 (5-bit) or
  2 (6/7/8-bit); [3] parity enable; [4] even parity (0 = odd); [5] forced ("stick") parity:
  [5:3]=1x1: [4]=0 → parity forced to 1, [4]=1 → forced to 0, both TX and RX check; [6] set break —
  TX forced to 0 until cleared; [7] divisor latch access (text also says "and enhanced feature
  register enabled" — copy error, the 450 has no such register).
- MCR (p15): [0] 1 → DTR pin 0; [1] 1 → RTS pin 0; [2] 1 → OP1 pin 0; [3] 1 → OP2 pin 0;
  [4] loopback; [7:5] not used, 0.
- LSR (p15-16): [0] data ready; [1] overrun (new char arrived while RHR full; the new char in the
  shift register is lost, RHR intact); [2] parity error; [3] framing error (no valid stop bit);
  [4] break received; [5] THR empty; [6] THR and TSR both empty; [7] = 0.
- MSR (p16): [0] ΔCTS, [1] ΔDSR, [2] trailing-edge RI (RI pin 0→1, i.e. RI bit 1→0), [3] ΔCD;
  [4] CTS = NOT CTS pin, [5] DSR, [6] RI, [7] CD (each complement of its pin). Delta bits "are set to
  a logic 0 whenever the CPU reads this register" (p16). Loopback: CTS=MCR[1] (RTS), DSR=MCR[0] (DTR),
  RI=MCR[2] (OP1), CD=MCR[3] (OP2) (p16).
- SPR: 8-bit scratch, no effect (p17).
- DLL/DLM: 16-bit divisor, DLM = bits 15-8.

### Behavior
- Baud generator (p9): clock = crystal on XTAL1/XTAL2 (parallel resonant, 22-33 pF, external 1 MΩ
  across, p9-10) or external clock into XTAL1 (XTAL2 = buffered clock output, p6). Divisor N = 1 ..
  2^16-1 (p18). BAUDOUT = f(XTAL1)/N = 16 × baud; baud = f(XTAL1)/(16·N) (p9: "-BAUDOUT = 16 x Baud
  Rate"; Table 3 at 1.8432 MHz: divisor 12 → 9600). Transmitter uses the internal 16x clock; the
  receiver uses the **RCLK pin** as its 16x clock — tie RCLK to BAUDOUT for equal rates (p5, p7).
  Divisor 0: not specified. Max clock 24 MHz at 5 V, 8 MHz at 3.3 V (T3w p18); clock high/low ≥17 ns.
- Transmitter: THR → TSR; LSR[5] set when THR moves to TSR; LSR[6] when both empty (p13, p16).
  Frame: start (0), 5-8 data LSB first, optional parity, 1/1.5/2 stop (1), idle 1. In the TX
  timing figure (p25) the THR-empty INT ("Tx Ready") is drawn rising T22d after the stop bit of the
  character, and it is reset T24d after the next THR write; the first transmission starts 8-24 RCLK after the INT is reset by the
  write (T23d, p18/p25).
- Receiver (p13): falling edge of start starts a 16x counter; after 7½ clocks the line is sampled at
  the start-bit centre; if still 0 the start bit is valid, else a false start is ignored. Bits sampled
  at centre. RX data-ready interrupt set 1 RCLK after the stop-bit sample (T20d, p18/p24). Break: RX
  low for a full character (standard 16450; the 450 text only says "received a break signal").
  Overrun as LSR[1].
- No FIFO, no TXRDY/RXRDY pins, no DMA.
- Interrupts: INT active high, totem-pole, never 3-stated (OP2 has no gating role on this part; pin
  text p5). INT = OR of enabled pending sources. ISR read returns the highest priority; "Whenever the
  interrupt status register is read, the interrupt status is cleared ... only the current pending
  interrupt is cleared by the read" (p13). Per-source clearing per 16450 compatibility (consistent
  with timing T19d/T21d/T24d p18): LSR int cleared by reading LSR (which clears LSR[4:1]); RX data int
  cleared by reading RHR; THR-empty int cleared by reading ISR when it is the reported source or by
  writing THR; modem int cleared by reading MSR. (The 450 sheet has no explicit per-source table —
  see Notes.)
- Modem lines: outputs = NOT MCR bits; inputs reflected inverted in MSR[7:4]; CTS/DSR have no effect
  on TX/RX (p6). Delta bits set on change, RI only on RI pin rising (end of ring).
- Loopback (MCR[4]=1, p10-11, Figure 4): TX and RX disconnected from pins, TSR output → RSR input
  internally; CTS/DSR/CD/RI pins disconnected; internally RTS→CTS, DTR→DSR, OP1→RI, OP2→CD (per MSR
  bit text p16). Interrupts fully operational, modem interrupts driven from MCR[3:0]. Pin states of TX
  and the 4 modem outputs during loopback are not stated (16450 convention: TX = 1, outputs inactive
  high) — see Notes.
- Reset (RESET pin, active high, min 40 ns; p5, p17): IER=00, ISR=01, LCR=00, MCR=00, LSR=60,
  MSR[3:0]=0, MSR[7:4]=input levels; outputs TX=1, OP1=1, OP2=1, RTS=1, DTR=1, CSOUT=0, INT=0.
  RHR/THR/DLL/DLM unaffected (XX); SPR shown as FF in Table 4 but not in the reset list (p17).
  "The UART transmitter output and the receiver input will be disabled during reset time" (p5).
- No sleep/power-down, no IrDA, no device ID.

### Timing (AC table p18; TA 0-70 °C, VCC 3.3 V or 5.0 V ±10%; values listed 3.3 V / 5.0 V)
| sym | parameter | 3.3 V | 5.0 V |
|---|---|---|---|
| T1w,T2w | clock pulse duration | ≥17 ns | ≥17 ns |
| T3w | oscillator/clock frequency | ≤8 MHz | ≤24 MHz |
| T4w | -AS width | ≥35 | ≥25 ns |
| T5s / T5h | address setup / hold (to -AS) | 5 / 5 | 0 / 5 ns |
| T6s / T6h | CS setup / hold (to -AS) | 5 / 0 | 0 / 0 ns |
| T7d | -IOR delay from CS | ≥10 | ≥10 ns |
| T7w | -IOR width | ≥35 | ≥25 ns |
| T7h | CS hold from -IOR | ≥0 | ≥0 |
| T8d | -IOR delay from address | ≥10 | ≥10 ns |
| T9d | read cycle delay (recovery) | ≥40 | ≥30 ns |
| T10d | CSOUT delay from CS | ≤15 | ≤10 ns |
| T11d | -IOR to -DDIS | ≤15 | ≤10 ns |
| T12d | -IOR to data valid | ≤35 | ≤25 ns |
| T12h | data disable (float) | ≤25 | ≤15 ns |
| T13d | -IOW delay from CS | ≥10 | ≥10 ns |
| T13w | -IOW width | ≥40 | ≥25 ns |
| T13h | CS hold from -IOW | ≥0 | ≥0 |
| T14d | -IOW delay from address | ≥10 | ≥10 ns |
| T15d | write cycle delay | ≥40 | ≥30 ns |
| T16s / T16h | data setup / hold | 20 / 5 | 15 / 5 ns |
| T17d | -IOW to modem output | ≤50 | ≤40 ns |
| T18d | modem input to INT set | ≤40 | ≤35 ns |
| T19d | -IOR (MSR read) to INT reset | ≤40 | ≤35 ns |
| T20d | stop bit to RX INT set | ≤1 RCLK | ≤1 RCLK |
| T21d | -IOR (RHR read) to INT reset | ≤45 | ≤40 ns |
| T22d | stop to THR-empty INT | ≤45 | ≤40 ns |
| T23d | initial INT reset (THR write) to TX start | 8..24 RCLK | 8..24 RCLK |
| T24d | -IOW to INT reset | ≤45 | ≤40 ns |
| TR | reset pulse width | ≥40 | ≥40 ns |
| N | divisor | 1..2^16-1 | |
All max values are max (no typicals printed). Figures: clock p20, read p21, write p22, modem p23,
RX p24, TX p25.

### VCC
Recommended: 3.3 V ±10% or 5.0 V ±10% (table headers p18-19); feature list says 2.97-5.5 V (p1).
Abs max: supply 7 V, any pin GND-0.3 to VCC+0.3 V (p19) → inputs NOT 5 V tolerant at 3.3 V
(inputMax = VCC). VIH 2.0 V (3.3 V) / 2.2 V (5 V); clock VIH 2.4 / 3.0 V (p19). ICC 1.3 / 3 mA.

### Package drawing
48-TQFP: "PACKAGE OUTLINE DRAWING 48 LEAD THIN QUAD FLAT PACK (TQFP)", p27, no drawing code, no
land pattern. mm: A 1.00-1.20, A1 0.05-0.15, A2 0.95-1.05, B (lead width) 0.17-0.27 (mid 0.22),
C 0.09-0.20, D (span) 8.80-9.20 (9.00), D1 (body) 6.90-7.10 (7.00), e 0.50 BSC, L 0.45-0.75 (mid
0.60), α 0-7°. 12 leads per side, pin 1 top of left side, CCW.

### Obsolete
Census: both rows "Active" (Sipex, qty 1606 / 535). The datasheet itself is watermarked OBS on every
page, though its ordering table (Rev 4.20) lists all six as Active. Treat as active per census; the
PDF watermark is worth a flag-review.

### Notes
- Loopback text p10 is garbled: "-OP1 and -OP2 in the MCR register (bits 0-1) control the modem -RI
  and -CD inputs" and "-DTR and -RTS (bits 0-1) are used to control the modem -CTS and -DSR inputs
  respectively" (which would be DTR→CTS). The MSR bit text p16 is the consistent 16450 mapping and
  is used above (RTS→CTS, DTR→DSR, OP1→RI, OP2→CD).
- AS edge: pin text says latched on "logic 0 transition", timing figure shows setup/hold to the
  rising edge (16450 ADS). Model: latch transparent while AS=0, hold on AS rising.
- Table 2 marks IER/LCR/MCR as write-mode only, but Table 4 and 16450 compatibility make them R/W.
- LSR read-clearing of bits 1-4 and MSR delta-clear on read: MSR stated (p16); LSR not stated in
  this sheet (16450 standard: reading LSR clears LSR[4:1]).
- p27 inch column prints e as "0.20 BSC" (should be 0.020); mm 0.50 BSC is the control column.
- LCR[7] text mentions an "enhanced feature register" — none exists on this device.
- 3.3 V clock limit 8 MHz caps 3.3 V operation at 500 kbps.

## ST16C1450

- Title (as printed): "ST16C1450 2.97V TO 5.5V UART", Rev. 4.2.1, August 2005.
  Function: single UART with "1 byte Transmit FIFO (THR)" / "1 byte Receive FIFO with error tags
  (RHR)", footprint compatible with SSI 73M1550/73M2550; explicitly **not** compatible with the
  standard 16450 / MS Windows driver (THR interrupt behavior) (PDF p3).
  Datasheet: `datasheets/controller-uart/st16c1450.pdf`, 30 PDF pages. PDF p1-2 are a 2009 Exar web
  page printout; the datasheet proper starts on PDF p3 = printed page 1, so **PDF page = printed + 2**.
  Pages below are PDF pages (printed in parentheses where useful).
- Differs from ST16C450: single active-low CS#, IOR#/IOW# only (no active-high strobes), no AS,
  CSOUT, DDIS, RCLK, BAUDOUT, OP1, OP2 pins; INT is 3-state, enabled by MCR[3]; IER[5] "special mode"
  adds soft reset to an RST output pin, power-down (MCR[7]) and ISR[5:4] status bits; THR-empty
  interrupt is level-type, not cleared by an ISR read alone. Receive start-bit sample after 8 clocks
  (vs 7½ printed for the 450).

### Parts
| census part | package | pins | pinout name | grade | supply |
|---|---|---|---|---|---|
| ST16C1450CQ48 | 48-Lead TQFP 7x7x1 mm | 48 | tqfp48 | C = 0..+70 °C | 2.97-5.5 V |
Ordering info p4 (printed 2): CJ28 = 28-PLCC, CQ48 = 48-TQFP, C = 0..+70 °C, I = -40..+85 °C.
Web page p2 lists the same with "-F" (RoHS) suffix.

### Pinout tqfp48 (Figure 2 p4; pin table p5-6)
```
1  NC     nc
2  NC     nc
3  D4     inout
4  D5     inout
5  D6     inout
6  D7     inout
7  RX     input
8  TX     output
9  CS     input   CS#, active low
10 NC     nc
11 NC     nc
12 NC     nc
13 NC     nc
14 NC     nc
15 XTAL1  input
16 XTAL2  output
17 IOW    input   IOW#, active low
18 NC     nc
19 GND    ground
20 IOR    input   IOR#, active low
21 RI     input   RI#, active low
22 RST    output  buffered reset out, active high
23 INT    output  3-state, active high
24 NC     nc
25 NC     nc
26 NC     nc
27 A2     input
28 A1     input
29 NC     nc
30 A0     input
31 RTS    output  RTS#, active low
32 DTR    output  DTR#, active low
33 RESET  input   active high
34 CTS    input   CTS#, active low
35 NC     nc
36 NC     nc
37 NC     nc
38 NC     nc
39 DSR    input   DSR#, active low
40 CD     input   CD#, active low
41 VCC    power
42 NC     nc
43 D0     inout
44 NC     nc
45 D1     inout
46 D2     inout
47 D3     inout
48 NC     nc
```
N.C. list p6: 1, 2, 10-14, 18, 24-26, 29, 35-38, 42, 44, 48 (matches figure). No exposed pad.
(PLCC28 has no RST pin; not in census.)

### Host interface (p5, AC table p22, Figures 9-10 p24)
- Intel bus: CS#=0 selects. IOR#: "The falling edge instigates an internal read cycle ... places it
  on the data bus" (p5); data valid TRDV after IOR# falling, floats TDD after IOR# rising.
  IOW#: falling edge starts the write cycle, **rising edge** latches D7..D0 into the register
  addressed by A2..A0 (p5); TDS/TDH around IOW# rising.
- Address A2..A0 setup TAS / hold TAH relative to the strobe (Fig 9/10); no address latch.
- No DDIS, CSOUT, AS.

### Registers (Table 2 p12, Table 3 p13, bit text p14-19)
| A2A1A0 | cond | name | R/W | reset (Table 6 p20) |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | R | XX |
| 000 | LCR[7]=0 | THR | W | XX |
| 001 | LCR[7]=0 | IER | R/W | 00 |
| 010 | — | ISR | R (write: reserved) | 01 |
| 011 | — | LCR | R/W | 00 |
| 100 | — | MCR | R/W | 00 |
| 101 | LCR[7]=0 | LSR | R (write: reserved) | 60 |
| 110 | LCR[7]=0 | MSR | R (write: reserved) | [3:0]=0, [7:4]=inverted inputs |
| 111 | LCR[7]=0 | SPR | R/W | FF |
| 000 | LCR[7]=1 | DLL | R/W | XX |
| 001 | LCR[7]=1 | DLM | R/W | XX |
(Table 2 groups 101-111 under LCR[7]=0; with LCR[7]=1 at 010-111 the table does not say otherwise,
so model 010-111 as unaffected by LCR[7], like the 16450.)

Bits:
- IER (p14): [0] RHR data int enable; [1] THR empty int enable ("If the THR is empty when this bit is
  enabled, an interrupt will be generated"); [2] RX line status int enable (LSR bits 1-4); [3] modem
  status int enable; [4] reserved; [5] Special Mode Enable — 1 enables ISR[5:4] (RXRDY/TXRDY),
  MCR[2] soft reset and MCR[7] power-down; [7:6] reserved.
- ISR (p14-15): [0] 0 = pending, 1 = none; [3:1] source per Table 4 (p15): 1 `0110` LSR, 2 `0100`
  RXRDY, 3 `0010` TXRDY, 4 `0000` MSR, `0001` none. [7:4] "Reserved" (p15) and 0 in Table 3, but
  with IER[5]=1 ISR[4] = TXRDY (1 while THR empty) and ISR[5] = RXRDY (1 while RHR holds data) —
  from IER[5] text p14 and Figures 11/12 p25 (labels "RXRDY (ISR bit-5)", "TXRDY (ISR bit-4)").
- LCR (p15-17): identical to ST16C450 ([1:0] length, [2] stop, [3] parity en, [4] even, [5] stick,
  [6] break, [7] divisor latch access). Table 5 p16 parity: X X 0 none; 0 0 1 odd; 0 1 1 even;
  1 0 1 forced 1 (mark); 1 1 1 forced 0 (space).
- MCR (p17-18):
  [0] 1 → DTR# pin 0; [1] 1 → RTS# pin 0;
  [2] OP1#/soft reset: no OP1 pin; in loopback drives internal RI (1 → RI# low). In normal mode with
      IER[5]=1, MCR[2]=1 drives RST high (ORed with RESET pin). Does not reset the 1450 itself.
  [3] OP2#/INT enable: normal mode 0 = INT 3-state (default), 1 = INT driven (active high).
      In loopback drives internal CD (1 → CD# low).
  [4] loopback; [6:5] reserved;
  [7] power-down enable, "can only be accessed when IER bit-5 = 1" (p18): 1 = power-down mode.
- LSR (p18): [0] data ready; [1] overrun (as 450); [2] parity error tag, [3] framing error tag,
  [4] break tag (RX low for at least one character frame) — tags belong to the char in RHR and are
  updated when RHR is unloaded (p9); [5] THR empty (set when THR → TSR, cleared on host write);
  [6] THR and TSR empty; [7] reserved (0).
- MSR (p19): [0] ΔCTS, [1] ΔDSR, [2] RI# 0→1 (end of ring), [3] ΔCD; [4] CTS = NOT CTS#, [5] DSR,
  [6] RI, [7] CD. Loopback: [4]=MCR[1], [5]=MCR[0], [6]=MCR[2], [7]=MCR[3].
- SPR (p19): scratch; preserved during sleep, FF after reset or power cycle.
- DLL/DLM: 16-bit divisor; "defaults to a random value upon power up or a reset" (p7).

### Behavior
- Baud generator (p7-8): crystal (parallel resonant, 10-22 pF load, 1.8432-24 MHz, 500K-1M feedback
  resistor, Figure 3) or external clock on XTAL1. divisor = f(XTAL1) / (baud × 16), 1..2^16-1. No
  prescaler, 16x sampling only, no BAUDOUT/RCLK pins (both TX and RX use the internal 16x clock).
  Table 1 p8: 14.7456 MHz, divisor 1 → 921.6 kbps. Max clock 24 MHz at 5 V (1.5 Mbps), 8 MHz at 3.3 V.
- Transmitter (p8): THR → TSR; 16 clocks per bit; start, data LSB first, parity, stop(s).
  LSR[5] set when THR moves to TSR; LSR[6] when both empty.
- Receiver (p9): start detect on falling edge; after **8** 16x clocks sample the start-bit centre,
  valid if still 0; data and stop bits sampled at their centres. Errors in LSR[4:2], RHR data-ready
  interrupt (IER[0]).
- "1-byte FIFO" = plain THR/RHR; no FCR, no trigger levels, no DMA pins, no timeout.
- Interrupts (p15): generation — LSR: any of LSR[4:1]; RXRDY: data in RHR; TXRDY: THR empty; MSR: any
  of MSR[3:0]. Clearing — LSR int: read LSR (flags/tags themselves not cleared until the char is
  removed from RHR); RXRDY: read RHR; **TXRDY: read ISR AND set IER[1]=0, or write THR** (Fig 12 note
  p25: "INT is cleared when the ISR is read and IER[1] is disabled") — i.e. effectively level:
  pending while IER[1]=1 and THR empty; MSR int: read MSR.
  INT pin: active high when MCR[3]=1; high-Z when MCR[3]=0 (default; reset state "Three-State",
  p20). Not open-drain.
- Modem lines: outputs = NOT MCR[1:0]; inputs inverted into MSR; no auto flow control (Rev 4.2.0
  removed a bogus auto-RTS mention, p28).
- Loopback (MCR[4]=1, p10-11, Figure 6): TSR output → RSR input; TX pin held 1 (mark); RTS# and DTR#
  pins de-asserted (high); CTS#, DSR#, CD#, RI#, RX inputs ignored; internal RTS→CTS, DTR→DSR,
  OP1(MCR[2])→RI, OP2(MCR[3])→CD. All UART functions operate normally. Caution: hold RX at 1 during
  loopback or a false break may be reported on exit (p10).
- Soft reset / RST pin (p6, p10, p17): with IER[5]=0, RST = RESET pin state. With IER[5]=1,
  RST = RESET OR MCR[2]. Reset state of RST = 1 (because RESET is high).
- Power-down (p10, p18): MCR[7]=1 (needs IER[5]=1) — "conserve power ... without destroying internal
  register configuration data"; IPWRDN 50 µA (3.3 V) / 200 µA (5 V) with inputs static and RX idle 1
  (p21). Wake-up: clear MCR[7]. What stops (oscillator / BRG) is not stated; model as clocks stopped,
  registers retained, bus access still works (needed to clear MCR[7]).
- Reset (RESET active high, ≥40 ns; Table 6 p20): IER=00, ISR=01, LCR=00, MCR=00, LSR=60,
  MSR[3:0]=0, MSR[7:4]=inverted inputs, SPR=FF, DLL/DLM/RHR/THR=XX; TX=1, RTS#=1, DTR#=1, RST=1,
  INT=3-state.

### Timing (AC table p22 / printed 20; TA 0-70 °C; VCC 2.97-5.5 V; columns 3.3 V / 5.0 V)
| sym | parameter | 3.3 V | 5.0 V |
|---|---|---|---|
| CLK | clock pulse duration | ≥63 ns | ≥21 ns |
| OSC | oscillator / ext clock frequency | ≤8 MHz | ≤24 MHz |
| TAS | address setup | ≥5 | ≥0 ns |
| TAH | address hold | ≥10 | ≥5 ns |
| TCS | chip select width | ≥50 | ≥40 ns |
| TRD | IOR# width | ≥35 | ≥25 ns |
| TDY | read/write cycle delay | ≥40 | ≥30 ns |
| TRDV | data access time | ≤35 | ≤25 ns |
| TDD | data disable time | 0..25 | 0..15 ns |
| TWR | IOW# width | ≥40 | ≥25 ns |
| TDS / TDH | data setup / hold | 20 / 5 | 15 / 5 ns |
| TWDO | IOW# to modem output | ≤50 | ≤40 ns |
| TMOD | modem input to INT set | ≤40 | ≤35 ns |
| TRSI | IOR# to INT reset (MSR) | ≤40 | ≤35 ns |
| TSSI | stop to set interrupt | ≤1 Bclk | ≤1 Bclk |
| TRRI | IOR# to reset interrupt | ≤45 | ≤40 ns |
| TSI | stop to interrupt | ≤45 | ≤40 ns |
| TINT | initial INT reset to TX start | 8..24 Bclk | 8..24 Bclk |
| TRST | reset pulse width | ≥40 | ≥40 ns |
| N | divisor | 1..2^16-1 | |
Bclk = 16× data rate. Figures 11/12 (p25) use TSSR, TRR, TSRT, TWT which are not in the table.

### VCC
Recommended 2.97-5.5 V (p3, p21-22), both grades. Abs max: supply 7 V, any pin GND-0.3 V to **7 V**
(p21) — inputs survive 5 V at 3.3 V supply per abs-max, but VIH max is listed as VCC (p21); the sheet
doesn't claim 5 V tolerance. VIH 2.0 V (3.3 V) / 2.2 V (5 V); clock VIH 2.4 / 3.0 V; ICC 1.3 / 3 mA.

### Package drawing
"PACKAGE DIMENSIONS (48 PIN TQFP - 7 X 7 X 1 mm)" p26 (printed 24), no code, no land pattern.
mm: A 1.00-1.20, A1 0.05-0.15, A2 0.95-1.05, B 0.17-0.27 (mid 0.22), C 0.09-0.20, D 8.80-9.20 (span
9.00), D1 6.90-7.10 (body 7.00), e 0.50 BSC, L 0.45-0.75 (mid 0.60), α 0-7°. Same outline as the
ST16C450 48-TQFP. θJA 59 °C/W (p21).

### Obsolete
None: census status Active (qty 819).

### Notes
- RST pin text (p6): "This output is only available on the ST16C1451", yet Figure 2 (p4) shows RST on
  pin 22 of ST16C1450CQ48 and the front page says "The 48-pin TQFP package provides a buffered reset
  output". The 1451 was dropped in Rev 4.2.1; the sentence is a leftover. Treat pin 22 = RST on the
  CQ48.
- Table 3 heading "SHADED BITS ARE ENABLED WHEN EFR BIT-4=1" is a copy error — the 1450 has no EFR;
  the gate is IER[5].
- ISR[7:4] are "Reserved" (p15) yet IER[5] enables ISR[5:4] (p14, p25). Polarity from Fig 11/12:
  RXRDY high = data ready, TXRDY high = THR empty.
- Receiver start-bit sampling: 8 clocks here vs 7½ on the 450 sheet.
- Power-down internals not described (see Behavior).
- MCR[2] in loopback with IER[5]=1: whether it also drives RST is not stated.
## ST16C550

- Title (as printed): "ST16C550 UART WITH 16-BYTE FIFO's", Rev. 5.01, April 2005 (p1). Function: single
  NS16C550-compatible UART, 16-byte TX and RX FIFOs, 2.97–5.5 V, "24MHz clock operation at 5V, 16MHz clock
  operation at 3.3V" (p1). Datasheet: `datasheets/controller-uart/st16c550.pdf`, 35 pages (PDF page = printed page).
- Nearest sibling: ST16C450 (section above) — same register map without FCR/FIFOs; the 450's TQFP pin 23 is CSOUT
  where the 550 has -TXRDY, and 450 pin 29 is NC where the 550 has -RXRDY. This 550 is the **reference 16550 core**
  that the ST16C2550, ST16C554, ST16C1550 and XR16C2550 sections below are described against.

### Parts
| census part | package | pins | pinout name | grade | supply |
|---|---|---|---|---|---|
| ST16C550CJ44-F | 44-Lead PLCC | 44 | plcc44 | C: 0..+70 °C | 2.97–5.5 V (3.3 V ±10 % or 5 V ±10 % tables) |
| ST16C550IJ44 | 44-Lead PLCC (no -F: non-RoHS, Sipex-branded census row) | 44 | plcc44 | I: -40..+85 °C | same |
| ST16C550IJ44-F | 44-Lead PLCC | 44 | plcc44 | I: -40..+85 °C | same |
| ST16C550CQ48-F | 48-Lead TQFP 7x7 | 48 | tqfp48 | C: 0..+70 °C | same |
| ST16C550IQ48-F | 48-Lead TQFP 7x7 | 48 | tqfp48 | I: -40..+85 °C | same |
| ST16C550IQ48TR-F | 48-Lead TQFP 7x7, tape & reel | 48 | tqfp48 | I: -40..+85 °C | same |
Ordering table p1: CP40 = 40-PDIP, CJ44 = 44-PLCC, CQ48 = 48-TQFP; C = 0..70 °C, I = -40..+85 °C. The -F (RoHS/
lead-free) and TR (tape & reel) suffixes are not in this 2005 PDF's table; they are MaxLinear's standard suffixes
(packing only, same die). Census description "SIPEX PIPELINE REGISTER 8-BIT" on ST16C550IJ44 is a DigiKey
description error; the part number decodes cleanly.

### Pinout plcc44 (p1 drawing; pin table p4-7, column "44")
```
1  NC        nc
2  D0        inout
3  D1        inout
4  D2        inout
5  D3        inout
6  D4        inout
7  D5        inout
8  D6        inout
9  D7        inout
10 RCLK      input   receiver 16x clock
11 RX        input
12 NC        nc
13 TX        output
14 CS0       input   active high
15 CS1       input   active high
16 CS2       input   active low (-CS2)
17 BAUDOUT   output  -BAUDOUT (16x baud clock)
18 XTAL1     input
19 XTAL2     output
20 IOWBAR    input   -IOW active-low write strobe
21 IOW       input   active-high write strobe
22 GND       ground
23 NC        nc
24 IORBAR    input   -IOR active-low read strobe
25 IOR       input   active-high read strobe
26 DDIS      output  -DDIS active low
27 TXRDY     output  -TXRDY active low
28 AS        input   -AS address strobe
29 A2        input
30 A1        input
31 A0        input
32 RXRDY     output  -RXRDY active low
33 INT       output  active high, always driven (no OUT2 gating, no 3-state)
34 NC        nc
35 OP2       output  -OP2
36 RTS       output  -RTS
37 DTR       output  -DTR
38 OP1       output  -OP1
39 RESET     input   active high
40 CTS       input   -CTS
41 DSR       input   -DSR
42 CD        input   -CD
43 RI        input   -RI
44 VCC       power
```

### Pinout tqfp48 (Figure 1 p2; pin table p4-7, column "48")
```
1  NC       nc          25 NC       nc
2  D5       inout       26 A2       input
3  D6       inout       27 A1       input
4  D7       inout       28 A0       input
5  RCLK     input       29 RXRDY    output (-RXRDY)
6  NC       nc          30 INT      output (active high)
7  RX       input       31 OP2      output (-OP2)
8  TX       output      32 RTS      output (-RTS)
9  CS0      input       33 DTR      output (-DTR)
10 CS1      input       34 OP1      output (-OP1)
11 CS2      input (-CS2)35 RESET    input (active high)
12 BAUDOUT  output      36 NC       nc
13 NC       nc          37 NC       nc
14 XTAL1    input       38 CTS      input (-CTS)
15 XTAL2    output      39 DSR      input (-DSR)
16 IOWBAR   input (-IOW)40 CD       input (-CD)
17 IOW      input       41 RI       input (-RI)
18 GND      ground      42 VCC      power
19 IORBAR   input (-IOR)43 D0       inout
20 IOR      input       44 D1       inout
21 NC       nc          45 D2       inout
22 DDIS     output      46 D3       inout
23 TXRDY    output      47 D4       inout
24 AS       input (-AS) 48 NC       nc
```
No exposed pad. PDIP40 exists in the ordering table but has no census part.

### Host interface (p4-7, AC table p21, figures p24-25)
- Intel I/O bus with both strobe polarities: read active when IORBAR=0 or IOR=1 ("Either an active -IOR or IOR is
  required", p4); write when IOWBAR=0 or IOW=1 (p4). Unused strobe tied inactive (IOR to 0 when using -IOR, -IOR
  to 1 when using IOR, p4-5).
- Select = CS0=1 AND CS1=1 AND CS2=0 (p4).
- AS (-AS): p4 text "A logic 1 transition on -AS latches the state of the chip selects and the register select
  bits, A0-A2 ... If not required, the -AS input can be permanently tied to a logic 0." Read/write figures p24:
  T5s/T6s setup and T5h/T6h hold drawn around the rising edge of -AS; T4w = -AS low width. → address & CS
  transparent while AS=0, latched on AS 0→1, held while AS=1. Note 1 (p21): T6s'/T7h' apply only with -AS tied low.
- DDIS: "goes to a logic 0 when the external CPU is reading data from the ST16C550" (p6) = selected AND read strobe
  active; T11d after -IOR edges (p24). The 550 has **no CSOUT pin** (the 450's CSOUT position carries -TXRDY).
- Read: D7..D0 driven T12d after read strobe active, 3-stated T12h after strobe ends (p21, p24-25). Data bus is
  3-state at all other times.
- Write: data latched at the trailing edge of the write strobe (T16s setup / T16h hold drawn around -IOW rising,
  p24-25). D0 = LSB = first serial bit (p4).

### Registers (Table 2 p9, Table 4 p13, bit text p14-19)
| A2A1A0 | cond | read | write | reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | THR | XX |
| 001 | LCR[7]=0 | IER | IER | 00 |
| 010 | — | ISR | FCR | ISR 01 / FCR 00 |
| 011 | — | LCR | LCR | 00 |
| 100 | — | MCR | MCR | 00 |
| 101 | — | LSR | "Reserved" | 60 |
| 110 | — | MSR | "Reserved" | X0 |
| 111 | — | SPR | SPR | FF (Table 4) |
| 000 | LCR[7]=1 | DLL | DLL | XX |
| 001 | LCR[7]=1 | DLM | DLM | XX |
Note: Table 2 says DLL/DLM replace RHR/THR/IER only at 000/001; all other addresses are unaffected by LCR[7].

Bits:
- IER (p14-15): [0] RX data ready (and RX timeout) int enable; [1] THR empty int enable; [2] RX line status int
  enable; [3] modem status int enable; [7:4] "Not used and set to 0".
- FCR (write-only, p15): [0] FIFO enable (TX and RX) — "This bit must be a 1 when other FCR bits are written to or
  they will not be programmed"; [1] RX FIFO reset: clears RX FIFO and counter logic (RSR untouched), self-clears;
  [2] TX FIFO reset: clears TX FIFO and counter (TSR untouched), self-clears; [3] DMA mode select 0/1; [5:4] not
  used; [7:6] RX trigger: 00=1, 01=4, 10=8, 11=14 bytes. No TX trigger.
- ISR (read, p16-17): [0] 0 = interrupt pending, 1 = none; [3:1] source; [5:4] = 0; [7:6] = 11 when FIFOs enabled
  (FCR[0]=1), 00 otherwise. Table 5 (p16):
  | priority | ISR[3:0] | source |
  |---|---|---|
  | — | 0001 | none |
  | 1 | 0110 | LSR receiver line status (overrun, parity, framing, break) |
  | 2 | 0100 | RXRDY received data ready / trigger reached |
  | 2 | 1100 | RXRDY receive data time-out |
  | 3 | 0010 | TXRDY THR empty |
  | 4 | 0000 | MSR modem status |
- LCR (p17-18): [1:0] word length 5/6/7/8; [2] stop: 0 = 1, 1 = 1.5 (5-bit) / 2 (6-8 bit); [3] parity enable;
  [4] even (1)/odd (0); [5] forced parity: with [3]=1, [5:4]=10 → parity always 1, 11 → always 0, for TX and RX
  check; [6] set break: TX forced 0 until cleared; [7] divisor latch access ("and enhanced feature register
  enabled" — leftover text, the 550 has no EFR).
- MCR (p18): [0] 1 → -DTR=0; [1] 1 → -RTS=0; [2] 1 → -OP1=0; [3] 1 → -OP2=0; [4] loopback enable; [7:5] not used,
  0. Note: on this part -OP2 does NOT gate INT (INT always driven, p5).
- LSR (read-only, p18-19): [0] data ready (≥1 char in RHR/FIFO; cleared when FIFO empty); [1] overrun (char arrived
  with FIFO/RHR full; the shift-register char is overwritten, FIFO data intact); [2] parity error, [3] framing error
  (no valid stop bit), [4] break (RX low for one full frame time; only one break char (0x00) loaded into FIFO) —
  bits 2-4 are associated with the char at the top of the FIFO (i.e. the next char RHR read returns); [5] THRE: THR
  (450 mode) / TX FIFO (FIFO mode) empty, cleared when ≥1 byte written; [6] TEMT: THR/FIFO and TSR both empty;
  [7] FIFO data error: ≥1 PE/FE/BI in the RX FIFO, cleared when none remain (0 in 450 mode). LSR[1:4] cleared by
  reading LSR (standard 16550; the PDF states reset value only, see Notes).
- MSR (p19): [0] ΔCTS, [1] ΔDSR, [2] TERI (-RI changed from 0 to 1, i.e. trailing edge of ring), [3] ΔCD; delta
  bits set when the input changes, cleared when MSR is read; [4] CTS = NOT -CTS, [5] DSR = NOT -DSR, [6] RI = NOT
  -RI, [7] CD = NOT -CD. In loopback: [4]=MCR[1] (RTS), [5]=MCR[0] (DTR), [6]=MCR[2] (OP1), [7]=MCR[3] (OP2).
- SPR (p19): 8-bit scratch, R/W, no effect.
- DLL/DLM: 16-bit divisor.

### Behavior
- Baud generator (p10): clock = XTAL1 (crystal with 1 MΩ across XTAL1/XTAL2, 22–47 pF caps, 1.8432–24 MHz, Fig 3
  p11; or external clock into XTAL1, XTAL2 = buffered clock out). Divisor = XTAL1 / (baud × 16), N = 1..2^16-1
  (p21 "N Baud rate divisor 1 216-1"). No prescaler. -BAUDOUT = 16× baud clock output (frequency XTAL1/N; waveform
  sketches p23 for N=1 (=clock), 2, 3, >3). Receiver is clocked by **RCLK** pin (16×), which must be wired externally
  to -BAUDOUT to use the internal generator (p5-6). Transmitter uses the internal BRG directly. Max clock 24 MHz at
  5 V, 16 MHz at 3.3 V (T3w p21); clock pulse width min 17 ns (T1w/T2w). Divisor 0 not specified.
- Receiver (p14): on falling edge of start (or false start) a 16× counter starts; after 7½ clocks the start bit is
  sampled at its centre; if still 0 it is validated, else ignored. Data bits sampled at bit centres (every 16
  clocks), LSB first, then optional parity, then stop bit(s) (only first stop bit checked — standard 16550; not
  stated). Character moved to RHR/FIFO at the stop bit; LSR[0] set; interrupt T20d = 1 RCLK after stop (p21).
- Transmitter: start(0), 5-8 data LSB first, optional parity, 1/1.5/2 stop. TX idles 1. THR→TSR transfer sets
  LSR[5]. T23d: delay from initial INT reset (THR write) to transmit start = 8..24 RCLK (p21).
- Break: LCR[6]=1 forces TX=0 (p18). Break detect: RX=0 for a whole frame (start+data+parity+stop) → LSR[4], a single
  0x00 char loaded (p18).
- FIFO (p9, p14-15): 16 bytes each. FCR[0]=0 → 450 mode (1-byte RHR/THR). RX FIFO stores the 3 error bits per char.
  RX data-available interrupt fires when count reaches trigger; ISR and interrupt clear when count drops below
  trigger (p14). FIFO keeps loading after trigger until full (p15).
- RX timeout (p9-10): timeout T = 4 × P + 12 bit times where P = programmed word length (5-8); worked examples
  (7 bits → 40 bit times). Counter reset at the centre of each stop bit received and on each RHR read; no timeout if
  RX FIFO empty. Timeout interrupt (ISR 1100) is priority 2 alongside RX data. Cleared by reading RHR (standard).
  Note: this differs from the NS16550 "4 character times".
- Polled FIFO mode (p14): FCR[0]=1 with IER[3:0]=0.
- DMA / pin behavior (p11 tables, p15):
  * -RXRDY: mode 0 (FCR[0]=0, or FCR[0]=1 & FCR[3]=0): 0 when ≥1 char in RHR/FIFO, 1 when empty. Mode 1 (FCR[0]=1,
    FCR[3]=1): goes 0 when trigger level reached or timeout occurs; goes 1 when FIFO empties.
  * -TXRDY: mode 0: 0 when THR/TX FIFO empty, 1 after the first char is loaded. Mode 1: 1 when TX FIFO completely
    full, 0 when ≥1 location empty.
  * Timing: T25d stop→-RXRDY 1 RCLK; T26d -IOR→-RXRDY reset 45/40 ns (3.3/5 V); T27d -IOW→-TXRDY set 45/40;
    T28d start→-TXRDY reset 8 RCLK (p21).
- Interrupts: INT active high, totem-pole, = OR of enabled pending sources (no OUT2 gating; MCR[3] only drives
  -OP2). Priorities/codes in ISR table. Set/clear (standard 16550, consistent with p14-19):
  LSR int: set on OE/PE/FE/BI (PE/FE/BI when the char reaches top of FIFO), cleared by reading LSR. RX data: set at
  trigger (FIFO) or on char ready (450 mode); cleared when below trigger / RHR read. Timeout: cleared on RHR read.
  THR empty: set when THR/TX FIFO becomes empty (and on enabling IER[1] while empty — standard), cleared by reading
  ISR when it is the source or by writing THR (T24d -IOW→reset interrupt 45/40 ns p21). MSR: set on any delta bit,
  cleared on MSR read (T19d -IOR→reset 40/35 ns). "Whenever the interrupt status register is read, the interrupt
  status is cleared. However ... only the current pending interrupt is cleared by the read" (p16) — only the THR
  interrupt is actually cleared by ISR read in a 16550.
- Modem: outputs -DTR/-RTS/-OP1/-OP2 = NOT MCR[0..3]. -CTS/-DSR have no effect on TX/RX (no auto flow control,
  p7). -RI: interrupt on 0→1 transition of the pin (p7). T17d -IOW→modem output 50/40 ns; T18d modem input→INT
  40/35 ns.
- Loopback (MCR[4]=1, p11 + Fig 4 p12): TX pin forced 1 (p7), RX pin ignored, TSR output looped to RSR. Modem
  inputs disconnected; internally CTS←RTS(MCR[1]), DSR←DTR(MCR[0]), RI←OP1(MCR[2]), CD←OP2(MCR[3]) (per MSR bit text
  p19; the p11 prose is garbled). Fig 4 shows -RTS, -DTR, -OP1, -OP2 outputs held at VCC (inactive, 1) in loopback.
  Interrupts fully operational (p11).
- Reset (RESET active high, pulse ≥ 40 ns TR p21; reset table p20): IER=00; ISR bit0=1 others 0; LCR=MCR=00; LSR
  bits 0-4=0, 5-6=1, 7=0 (=0x60); MSR bits 0-3=0, 4-7 = input levels; FCR=00. Outputs: TX=1, -OP1=1, -OP2=1, -RTS=1,
  -DTR=1, -RXRDY=1, -TXRDY=0, INT=0. RHR/THR/DLL/DLM/SPR not listed as reset (SPR default FF in Table 4). FIFOs
  are not stated as cleared; the TX/RX are "disabled during reset time" (p6).
- No sleep, no IrDA, no auto flow, no device ID.

### Timing (AC table p21; TA per grade, Vcc 3.3 / 5.0 V ±10 %; all min unless stated max)
| sym | parameter | 3.3 V | 5.0 V |
|---|---|---|---|
| T1w,T2w | clock pulse duration | 17 min | 17 min |
| T3w | osc/clock frequency (max) | 16 MHz | 24 MHz |
| T4w | -AS width | 35 | 25 |
| T5s/T5h | address setup/hold (to -AS rise) | 5/5 | 0/5 |
| T6s/T6h | CS setup/hold | 5/0 | 0/0 |
| T6s' | address setup (AS tied low) | 10 | 5 |
| T7d | -IOR delay from CS | 10 | 10 |
| T7w | -IOR width | 77 | 38 |
| T7w' | CS width | 77 | 38 |
| T7h | CS hold from -IOR | 0 | 0 |
| T7h' | address hold (AS low) | 5 | 5 |
| T8d | -IOR delay from address | 10 | 10 |
| T9d | read cycle delay | 77 | 38 |
| T11d | -IOR to -DDIS (max) | 15 | 10 |
| T12d | -IOR to data valid (max) | 35 | 25 |
| T12h | data disable (max) | 25 | 15 |
| T13d | -IOW delay from CS | 10 | 10 |
| T13w | -IOW width | 27 | 15 |
| T13w' | CS width | 77 | 38 |
| T13h | CS hold from -IOW | 0 | 0 |
| T14d | -IOW delay from address | 10 | 10 |
| T15d | write cycle delay | 77 | 38 |
| T16s/T16h | data setup/hold | 20/5 | 15/5 |
| T17d | -IOW to modem output (max) | 50 | 40 |
| T18d | modem input to INT set (max) | 40 | 35 |
| T19d | -IOR to INT reset (max) | 40 | 35 |
| T20d | stop to INT set (max) | 1 RCLK | 1 RCLK |
| T21d | -IOR to reset interrupt (max) | 45 | 40 |
| T22d | stop to interrupt (max) | 45 | 40 |
| T23d | initial INT reset to TX start | 8..24 RCLK | 8..24 RCLK |
| T24d | -IOW to reset interrupt (max) | 45 | 40 |
| T25d | stop to set -RXRDY | 1 RCLK | 1 RCLK |
| T26d | -IOR to reset -RXRDY (max) | 45 | 40 |
| T27d | -IOW to set -TXRDY (max) | 45 | 40 |
| T28d | start to reset -TXRDY | 8 RCLK | 8 RCLK |
| TR | reset pulse width | 40 | 40 |
(ns unless noted. Min/max columns in the PDF are not always populated; I list the single printed number. Rev history
p35: AC values changed for parts with top mark date code "B2 YYWW" and newer.)

### VCC
Recommended: 3.3 V ±10 % or 5.0 V ±10 % (table headers p21-22); front page "2.97 to 5.5 volt" (p1). Abs max: supply
7 V, any pin GND-0.3 to VCC+0.3 (p22) → inputs NOT 5 V tolerant above VCC (inputMax = VCC). VIH 2.0 (3.3 V) / 2.2
(5 V); clock VIH 2.4/3.0 (p22).

### Package drawings
- 44-lead PLCC (p33, inch controlling): pitch e 1.27 mm (0.050"; PDF misprints "0.50 BSC" in the inch column);
  D (outer, over leads) 17.40–17.65 mm; D1 (body) 16.51–16.66; D2 (lead centre/contact span) 14.99–16.00;
  D3 12.70 typ (lead row length); A 4.19–4.57; A1 2.29–3.05; B (lead width) 0.33–0.53; B1 0.66–0.81. No land
  pattern printed.
- 48-lead TQFP (p34, mm controlling): body D1 6.90–7.10; span D 8.80–9.20; pitch e 0.50; lead width B 0.17–0.27;
  foot L 0.45–0.75; A 1.00–1.20; A1 0.05–0.15. No land pattern printed.

### Obsolete
ST16C550CJ44-F, ST16C550IJ44, ST16C550IJ44-F (DigiKey status Obsolete). The PDF (2005) lists all as Active.

### Notes
- Loopback prose p11 is garbled ("-OP1 and -OP2 in the MCR register (bits 0-1) control the modem -RI and -CD");
  the MSR bit descriptions p19 are taken as authoritative (RTS→CTS, DTR→DSR, OP1→RI, OP2→CD).
- LCR[7] text mentions "enhanced feature register" — not present on this part.
- LSR read-clear of bits 1-4 and THR-interrupt clear-on-ISR-read are not spelled out; standard 16550 behavior
  assumed (also how XR16C2550 / ST16C2550 datasheets describe it explicitly).
- Timeout formula 4×P+12 bits (not "4 char times").
## ST16C2550

- Title (as printed): "ST16C2550 2.97V TO 5.5V DUART WITH 16-BYTE FIFO", Rev. 4.4.1, January 2011 (p1).
  Function: two independent 16C550-compatible UART channels (A, B) sharing one bus, one XTAL. Datasheet:
  `datasheets/controller-uart/st16c2550.pdf`, 37 pages (PDF page = printed page).
- Differs from ST16C550 (single): two channels selected by CSA#/CSB#; no CS0/CS1/AS/DDIS/BAUDOUT/RCLK/OP1/IOR/IOW
  (active-high) pins — receiver uses the internal BRG directly; INTA/INTB are **3-stated unless MCR[3]=1**
  (OP2# shares MCR[3]); 5 V-tolerant inputs (parts dated "A2 YYWW" and newer, p1/p6); external clock up to 64 MHz
  (4 Mbps); timeout described as RHR-read-until-below-trigger etc. Register map identical to ST16C550 per channel.

### Parts
| census part | package | pins | pinout name | grade | supply |
|---|---|---|---|---|---|
| ST16C2550CQ48-F | 48-Lead TQFP 7x7x1 mm | 48 | tqfp48 | C: 0..+70 °C | 2.97–5.5 V (3.3 V±10 % / 5 V±10 % tables) |
| ST16C2550IQ48-F | 48-Lead TQFP | 48 | tqfp48 | I: -40..+85 °C | same |
| ST16C2550IQ48TR-F | 48-Lead TQFP, tape & reel | 48 | tqfp48 | I: -40..+85 °C | same |
Ordering table p3: CP40 PDIP, CJ44 PLCC, CQ48 TQFP; C/I temperature letter. -F = RoHS, TR = reel (not in the PDF
table; packing only).

### Pinout tqfp48 (Figure 2 p2; pin table p4-6, "48-TQFP" column)
```
1  D5      inout        25 NC      nc
2  D6      inout        26 A2      input
3  D7      inout        27 A1      input
4  RXB     input        28 A0      input
5  RXA     input        29 INTB    output  active high, 3-state when MCR[3]=0 (ch B)
6  TXRDYB  output (#)   30 INTA    output  same, ch A
7  TXA     output       31 RXRDYA  output (#)
8  TXB     output       32 OP2A    output (#)
9  OP2B    output (#)   33 RTSA    output (#)
10 CSA     input (#)    34 DTRA    output (#)
11 CSB     input (#)    35 DTRB    output (#)
12 NC      nc           36 RESET   input  active high
13 XTAL1   input        37 NC      nc
14 XTAL2   output       38 CTSA    input (#)
15 IOW     input (#)    39 DSRA    input (#)
16 CDB     input (#)    40 CDA     input (#)
17 GND     ground       41 RIA     input (#)
18 RXRDYB  output (#)   42 VCC     power
19 IOR     input (#)    43 TXRDYA  output (#)
20 DSRB    input (#)    44 D0      inout
21 RIB     input (#)    45 D1      inout
22 RTSB    output (#)   46 D2      inout
23 CTSB    input (#)    47 D3      inout
24 NC      nc           48 D4      inout
```
(#) = active low in the datasheet (name ends in #). No exposed pad. NC pins 12, 24, 25, 37 ("typically, should be
connected to GND", p6). PLCC44/PDIP40 have no census part.

### Host interface (p4, p8 Table 1, AC p27-28, Fig 15/16 p29-30)
- Intel bus: IOR# (falling edge starts internal read; data on bus for host to latch on rising edge, p4), IOW#
  (falling edge starts, **rising edge transfers data** into the register, p4). No active-high strobes, no AS
  latch: A2..A0 and CS must be valid TAS before and TAH after the strobe (Fig 15/16).
- CSA#/CSB# (Table 1 p8): both 1 → deselected; CSA#=0,CSB#=1 → channel A; CSA#=1,CSB#=0 → channel B;
  both 0 → both selected: writes go to both channels ("useful during power up initialization"); "do not attempt
  to read from both uarts simultaneously" (bus contention — model as X).
- D7..D0 3-state except during a selected read. No DDIS. No clock needed for bus cycles (p8).

### Registers (per channel; Tables 6-7 p16-17, text p17-25)
Identical to ST16C550 (address map, bits, reset values) with these wording differences:
- IER[1] (p18): "If the Transmit FIFO is empty when this bit is enabled, an interrupt will be generated."
- IER[2] (p18): LSR bit-1 (overrun) interrupts immediately when the char is received; LSR bits 2-4 interrupt
  when the char with the error reaches RHR ("when the character with errors is read out of the FIFO" / "when the
  character is in the RHR", p23).
- IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5]: "Reserved" (read 0).
- FCR[1]/[2] "only active when FCR bit-0 is a 1" (p20).
- MCR[2]: "Reserved. OP1# is not available as an output pin ... In the Loopback Mode, this bit is used to write
  the state of the modem RI# interface signal" (p22).
- **MCR[3]** (p23): 0 = INTA/INTB 3-state and OP2# = 1 (default); 1 = INT outputs enabled (active mode) and OP2# = 0.
  (Each channel's MCR[3] controls its own INTx and OP2x#.)
- LSR[4] (p23): "The break indication remains until the RX input returns to the idle condition".
- SPR (p25): "preserved during sleep mode" (copy-paste; no sleep on this part) — reset FF.
ISR priority table (Table 8 p19) is printed as 5 levels: 1 LSR 0110; 2 RX timeout 1100; 3 RX data 0100; 4 TX ready
0010; 5 MSR 0000; none 0001 — i.e. timeout is listed above RX data (same codes as ST16C550).

### Behavior (deltas vs ST16C550; otherwise identical)
- Clock (p10-12): XTAL1/XTAL2 crystal 1.8432–24 MHz (500 kΩ–1 MΩ feedback R), or external clock on XTAL1; with
  2 kΩ pull-up on XTAL2 external clock up to 64 MHz at 5 V room temp (Fig 5/6). Each channel: divisor = XTAL1 /
  (baud × 16), 1..2^16-1. No prescaler (Table 5 header "MCR Bit-7=0" is a leftover). Both TX and RX of a channel
  use its BRG; no BAUDOUT/RCLK pins.
- Receiver start bit (p13): validated **after 8 clocks** (vs 7½ in ST16C550 text); "rest of the data bits and stop
  bits are sampled and validated in this same manner" (mid-bit).
- RX timeout (p13): "not received for 4 word lengths as defined by LCR[1:0] plus 12 bits time" = 3.7–4.6 char
  times (same formula). Interrupt generation/clearing list (p19): LSR int ← any LSR[4:1], cleared by LSR read; RX
  data ← trigger, cleared when FIFO below trigger; timeout cleared by reading RHR; TXRDY ← TX FIFO empty, cleared by
  ISR read or THR write; MSR ← MSR[3:0], cleared by MSR read.
- Pin tables (p9 Tables 2-4): -RXRDY/-TXRDY as ST16C550. Table 3/4 also show "INT pin" behavior: TX: 1 when THR/FIFO
  empty; RX: 1 when byte present (non-FIFO) / FIFO at-or-above trigger.
- Loopback (p15, Fig 11): TX held 1, RTS#/DTR# outputs de-asserted (1), modem inputs ignored; internally
  RTS→CTS, DTR→DSR, OP1 (MCR[2])→RI, OP2 (MCR[3])→CD. Fig 11 shows OP2A#/OP2B# pin held at VCC in loopback.
  Caution: RX input must be 1 during loopback or a false break may be reported on exit (p15).
- Reset (p8, Table 11 p25): RESET active high ≥40 ns. Registers as ST16C550 (SPR=FF). Outputs: TX=1, OP2#=1,
  RTS#=1, DTR#=1, RXRDY#=1, TXRDY#=0, **INT = three-state**.

### Timing (AC table p27-28; 70 pF load; Vcc 3.3 V±10 % / 5 V±10 %; ns)
| sym | parameter | 3.3 V | 5.0 V |
|---|---|---|---|
| — | crystal frequency (max) | 20 MHz | 24 MHz (unit misprinted "ns") |
| CLK | clock pulse duration (min) | 17 | 8 |
| OSC | external clock frequency (max) | 30 MHz | 64 MHz |
| TAS / TAH | address setup / hold (min) | 5 / 10 | 0 / 5 |
| TCS | chip select width (min) | 70* | 40 |
| TRD | IOR# strobe width (min) | 70* | 40 |
| TDY | read or write cycle delay (min) | 70* | 40 |
| TRDV | data access time (max) | 35 | 25 |
| TDD | data disable time (min/max) | 0/25 | 0/15 |
| TWR | IOW# strobe width (min) | 40 | 25 |
| TDS / TDH | data setup / hold (min) | 20 / 5 | 15 / 5 |
| TWDO | IOW# to modem output (max) | 50 | 40 |
| TMOD | modem input to INT (max) | 40 | 35 |
| TRSI | IOR# to reset INT (max) | 40 | 35 |
| TSSI | stop to set INT | 1 Bclk | 1 Bclk |
| TRRI | IOR# to reset INT (max) | 45 | 40 |
| TSI | start to interrupt (max) | 45 | 40 |
| TINT | initial INT reset to TX start | 8..24 Bclk | 8..24 Bclk |
| TWRI | IOW# to reset INT (max) | 45 | 40 |
| TSSR | stop to set RXRDY# | 1 Bclk | 1 Bclk |
| TRR | IOR# to reset RXRDY# (max) | 45 | 40 |
| TWT | IOW# to set TXRDY# (max) | 45 | 40 |
| TSRT | centre of start to reset TXRDY# | 8 Bclk | 8 Bclk |
| TRST | reset pulse width (min) | 40 | 40 |
*55 ns if VCC = 3.3 V +10 %/-5 % and TA 0..70 °C (Fig 12 p28 gives temperature curves). Bclk = 16× data rate.

### VCC
2.97–5.5 V (p1, p6); tables at 3.3 V ±10 % and 5 V ±10 % (p26). Abs max "Voltage at any pin GND-0.3 V +5.5 V" and
VIH max 5.5 V at both supplies (p26): **inputs 5 V tolerant** (inputMax 5.5 V) for "A2 YYWW" and newer date codes.

### Package drawing
48-pin TQFP 7x7x1 mm (p34, mm controlling): D 8.80–9.20, D1 6.90–7.10, e 0.50 BSC, B (lead width) 0.17–0.27,
L 0.45–0.75, A 1.00–1.20, A1 0.05–0.15, A2 0.95–1.05, C 0.09–0.20. No land pattern. (44-PLCC p35, 40-PDIP p36 — not
needed.) Thermal θja 59 °C/W TQFP.

### Obsolete
None (all three census rows Active).

### Notes
- Table 5 (p12) header says "MCR Bit-7=0" though the part has no prescaler; ignore.
- SPR text mentions sleep mode — not a feature of this part.
- AC table "Crystal Frequency 20 24 ns" unit error (MHz).

## ST16C1550 (and ST16C1551)

- Title (as printed): "ST16C1550/51 2.97V TO 5.5V UART WITH 16-BYTE FIFO", Rev. 4.2.1, August 2005 (p1). Function:
  single 16C550-class UART, 16-byte FIFOs, pin/function compatible to SSI 73M1550/2550, with an IER[5] "special
  mode" (TX FIFO trigger, power-down, soft-reset output, TXRDY/RXRDY status bits in ISR). Datasheet:
  `datasheets/controller-uart/st16c1550.pdf`, 36 pages (PDF page = printed page).
- "The ST16C155X is not compatible with the industry standard 16550 and will not work with the standard serial
  port driver in MS Windows" (p1) — because the THR-empty interrupt is not cleared by an ISR read alone (p17).
- Differs from ST16C550: no TXRDY/RXRDY/BAUDOUT/RCLK/OP1/OP2/DDIS/AS pins, single CS#, IOR#/IOW# active-low only;
  INT 3-state controlled by MCR[3]; RST buffered-reset output; IER[5] special mode; TX trigger levels; TX interrupt
  semantics differ (see Behavior). It is the FIFO version of ST16C1450 (frag above); same special-mode concept.
- ST16C1550 vs ST16C1551: identical in the 48-TQFP (same pinout drawing, Figures 2/3 p2-3, both have RST on pin 22).
  They differ only in the 28-PLCC: 1550CJ28 has XTAL1/XTAL2 and no RST; 1551CJ28 has CLK (external clock only) and
  RST (p2-3, p6). No census part for ST16C1551.

### Parts
| census part | package | pins | pinout name | grade | supply |
|---|---|---|---|---|---|
| ST16C1550IQ48-F | 48-Lead TQFP 7x7x1 mm | 48 | tqfp48 | I: -40..+85 °C | 2.97–5.5 V |
Ordering table p4: CJ28 = 28-PLCC, CQ48 = 48-TQFP; C 0..70 °C, I -40..+85 °C. -F = RoHS (not in PDF).
Census vcc column blank; DigiKey status Active.

### Pinout tqfp48 (Figure 2 p2, verified on the page image; pin table p5-6)
```
1  NC      nc           25 NC      nc
2  NC      nc           26 NC      nc
3  D4      inout        27 A2      input
4  D5      inout        28 A1      input
5  D6      inout        29 NC      nc
6  D7      inout        30 A0      input
7  RX      input        31 RTS     output (#)
8  TX      output       32 DTR     output (#)
9  CS      input (#)    33 RESET   input  active high
10 NC      nc           34 CTS     input (#)
11 NC      nc           35 NC      nc
12 NC      nc           36 NC      nc
13 NC      nc           37 NC      nc
14 NC      nc           38 NC      nc
15 XTAL1   input        39 DSR     input (#)
16 XTAL2   output       40 CD      input (#)
17 IOW     input (#)    41 VCC     power
18 NC      nc           42 NC      nc
19 GND     ground       43 D0      inout
20 IOR     input (#)    44 NC      nc
21 RI      input (#)    45 D1      inout
22 RST     output  active high buffered reset
23 INT     output  active high, 3-state unless MCR[3]=1
24 NC      nc           46 D2 inout / 47 D3 inout / 48 NC nc
```
No exposed pad. Note D0 is at pin 43 with NC at 44 (D1 = 45) — as printed in both table and drawing.

### Host interface (p5-6, AC p26, Fig 10/11 p28)
- Intel: CS# active low (single select). IOR# falling edge starts read, data valid TRDV later, removed TDD after
  IOR# rises. IOW# falling edge starts, rising edge transfers data (p5). A2..A0 valid TAS before / TAH after the
  strobe (no address latch). D7..D0 3-state when not reading.

### Registers (Tables 3-4 p14-15, text p16-24)
Address map identical to ST16C550 (LCR[7] selects DLL/DLM at 000/001). Bit differences:
- IER: [0]-[3] as ST16C550; [4] reserved; **[5] Special Mode Enable**: enables ISR[5:4], FCR[5:4] (TX trigger),
  MCR[2] soft reset and MCR[7] power-down (p17, Table 4); [7:6] reserved.
- ISR: [0] pending (0)/none (1); [3:1] source; **[4] = NOT TXRDY**, **[5] = NOT RXRDY** when IER[5]=1 (0 otherwise)
  (p18; TXRDY/RXRDY per Table 1 p8); [7:6] = 11 when FIFOs enabled.
  Priority (Table 5 p18): 1 LSR 0110; 2 RX timeout 1100; 3 RX data 0100; 4 TX ready 0010; 5 MSR 0000; none 0001.
- FCR: [0] FIFO enable (must be 1 to write other bits); [1] RX FIFO reset, [2] TX FIFO reset (only when FCR[0]=1,
  self-clearing); [3] DMA mode (affects ISR TXRDY/RXRDY status); **[5:4] TX trigger 00=1, 01=4, 10=8, 11=14** (only
  when IER[5]=1); [7:6] RX trigger 00=1, 01=4, 10=8, 11=14 (Table 6 p19).
- LCR: as ST16C550.
- MCR: [0] DTR# (1 → pin 0); [1] RTS# (1 → pin 0); **[2] OP1/soft reset**: in loopback drives internal RI; in normal
  operation: when IER[5]=1, RST pin = RESET pin OR MCR[2]; when IER[5]=0 RST follows RESET (p21, pin text p6). "Note
  that asserting MCR bit-2 does not reset the 155X." **[3] INT output enable**: 0 = INT 3-state, 1 = INT active
  (not in loopback); in loopback writes the internal CD; [4] loopback; [6:5] reserved; **[7] power-down** (only
  accessible when IER[5]=1): 1 = power down mode (p22). See Notes for the MCR[7] prescaler conflict.
- LSR, MSR, SPR, DLL, DLM: as ST16C550 (MSR loopback: [4]=MCR[1], [5]=MCR[0], [6]=MCR[2], [7]=MCR[3], p23).
- Reset (Table 8 p24): DLL/DLM/RHR/THR XX; IER 00; FCR 00; ISR 01; LCR 00; MCR 00; LSR 60; MSR[3:0]=0, [7:4]= inverted
  input levels; SPR FF. Outputs: TX 1, RTS# 1, DTR# 1, **RST 1** (follows RESET), INT 3-state.

### Behavior
- Clock & BRG (p8-10): crystal 1.8432–24 MHz on XTAL1/XTAL2 (500 k–1 MΩ feedback), or external clock on XTAL1.
  p9 text: "MCR register bit-7 sets the prescaler to divide the input crystal or external clock by 1 or 4";
  divisor = (XTAL1 / prescaler) / (baud × 16), divisor 1..2^16-1; Table 2 (p10) gives rates for MCR[7]=1 (÷4) and
  MCR[7]=0 (÷1). Conflicts with MCR[7] = power-down (p12, p22) — see Notes. 16× sampling; TX and RX share the BRG.
  Max OSC 8 MHz at 3.3 V, 24 MHz at 5 V (p26).
- Receiver/transmitter/break/overrun/error tags: as ST16C2550 text (start validated after 8 16× clocks, p11).
- TX FIFO / TXRDY interrupt (p11, p16-19): "The THR empty flag can generate a transmit empty interrupt (ISR bit-1)
  when the amount of data in the FIFO falls below its programmed trigger level" (FIFO mode) — trigger from FCR[5:4]
  when IER[5]=1 (default 1 → effectively "empty"). FCR[5:4] text: interrupt "when the number of characters in the
  FIFO falls below the selected trigger level, or when it gets empty in case that the FIFO did not get filled over
  the trigger level on last re-load." IER[1]: interrupt generated if THR empty when enabled.
- Interrupt clearing (p17): LSR int cleared by reading LSR "(but flags and tags not cleared until character(s) that
  generated the interrupt(s) has been emptied or cleared from FIFO)"; RX data cleared when FIFO below trigger;
  RX timeout cleared by "reading RHR until empty"; **TXRDY cleared by a read of ISR AND clearing IER[1], or by
  loading data into the TX FIFO** (not by ISR read alone — the non-16550 behavior); MSR cleared by reading MSR.
- RX timeout: 4 × word length (LCR[1:0]) + 12 bit times (p11).
- TXRDY/RXRDY (no pins; visible inverted in ISR[4]/[5] when IER[5]=1; Table 1 p8, positive-true):
  RXRDY: FIFO off: 1 = byte present; FIFO on DMA 0: 1 = ≥1 byte; DMA 1: 1 when trigger reached or timeout, 0 when
  FIFO empty. TXRDY: FIFO off: 1 = THR empty; DMA 0: 1 = FIFO empty; DMA 1: 1 = ≥1 empty location, 0 = full.
- INT: active high, totem-pole when MCR[3]=1, 3-state when MCR[3]=0 (default) (p5, p21).
- Power down (p12, p22, p25): MCR[7]=1 with IER[5]=1. "conserve power ... without destroying internal register
  configuration data". IPWRDN 50 µA (3.3 V) / 200 µA (5 V). What stops is not stated beyond that; Test 1 note:
  RX must idle at 1 while powered down. Wake = write MCR[7]=0 (implied). Model: oscillator/BRG stopped, registers
  kept, bus still works.
- Soft reset output: RST = RESET OR (IER[5] AND MCR[2]) (p6, p12, p21).
- Loopback (p13 Fig 7): TX held 1, RTS#/DTR# pins de-asserted (1), modem inputs ignored; RTS→CTS, DTR→DSR, OP1
  (MCR[2])→RI, OP2 (MCR[3])→CD. RX must stay 1 to avoid false break on exit.
- No auto flow control (removed in rev 4.2.0, p34), no IrDA, no device ID.

### Timing (AC table p26; Vcc 2.97–5.5 V; ns; columns 3.3 V / 5.0 V)
| sym | parameter | 3.3 V | 5.0 V |
|---|---|---|---|
| CLK | clock pulse duration (min) | 63 | 21 |
| OSC | osc / ext clock frequency (max) | 8 MHz | 24 MHz |
| TAS / TAH | address setup / hold | 5 / 10 | 0 / 5 |
| TCS | chip select width | 50 | 40 |
| TRD | IOR# strobe width | 35 | 25 |
| TDY | read/write cycle delay | 40 | 30 |
| TRDV | data access (max) | 35 | 25 |
| TDD | data disable (min/max) | 0/25 | 0/15 |
| TWR | IOW# strobe width | 40 | 25 |
| TDS / TDH | data setup / hold | 20 / 5 | 15 / 5 |
| TWDO | IOW# to output (max) | 50 | 40 |
| TMOD | modem input to INT (max) | 40 | 35 |
| TRSI | IOR# to reset INT (max) | 40 | 35 |
| TSSI | stop to set INT | 1 Bclk | 1 Bclk |
| TRRI | IOR# to reset INT (max) | 45 | 40 |
| TSI | stop to interrupt (max) | 45 | 40 |
| TINT | initial INT reset to TX start | 8..24 Bclk | 8..24 Bclk |
| TSSR/TRR/TWT/TSRT | (RXRDY/TXRDY internal status) | 1 Bclk / 45 / 45 / 8 Bclk | 1 Bclk / 40 / 40 / 8 Bclk |
| TRST | reset pulse width | 40 | 40 |

### VCC
2.97–5.5 V (p1; DC/AC headers p25-26 "VCC IS 2.97V TO 5.5V"). Abs max "Voltage at Any Pin GND-0.3 V to 7 V" but
VIH max = VCC (p25) → treat inputs as **not** 5 V tolerant (inputMax = VCC); see Notes.

### Package drawing
48-pin TQFP 7x7x1 mm (p32, mm): D 8.80–9.20, D1 6.90–7.10, e 0.50 BSC, B 0.17–0.27, L 0.45–0.75, A 1.00–1.20,
A1 0.05–0.15. No land pattern. (28-PLCC p33: D 12.32–12.57, e 1.27 — no census part.)

### Obsolete
None (census row Active).

### Notes
- **MCR[7] conflict**: p9 + Table 2 describe MCR[7] as a ÷1/÷4 clock prescaler; p12, Table 4 p15 and p22 define
  MCR[7] as Power Down Enable (only when IER[5]=1). Table 4 (the register map) shows only "0/Power Down Mode". The
  prescaler text appears copied from XR16L580-family datasheets. Recommend: MCR[7] = power-down; no prescaler.
- Table 4 title "SHADED BITS ARE ENABLED WHEN EFR BIT-4=1" — no EFR on this part; shaded bits are IER[5]-gated.
- Pin table says RST "only available on the ST16C1551", but Figure 2 (ST16C1550CQ48) shows RST on pin 22 and the
  general description says every package except the 28-pin ST16C1550 has it. Taken: ST16C1550 TQFP has RST.
- Abs max 7 V on any pin vs VIH max VCC: inconsistent; conservative inputMax = VCC.
- SPR "preserved during sleep mode" = power-down.
## ST16C650A

- Title (as printed): "ST16C650A 2.90V TO 5.5V UART WITH 32-BYTE FIFO", Rev. 5.0.3, June 2010 (p1). Datasheet:
  `datasheets/controller-uart/st16c650a.pdf`, 50 pages (PDF page = printed page). **Every page carries the red
  watermark "The product(s) mentioned in this data sheet are no longer being manufactured and may not be ordered
  (OBS)"**; the ordering table (p2) still says Active.
- Function: single UART, 32-byte TX/RX FIFOs, 16550 register set + Exar enhanced page (EFR, Xon1/2, Xoff1/2 at
  LCR=0xBF) + XFR/IRPW (EFR[4]-gated) + DVID/DREV; Intel bus (SEL=1) or PC-ISA mode with on-chip COM1-4/LPT decoder
  (SEL=0); auto RTS/CTS with hysteresis; auto Xon/Xoff + special char; IrDA 1.0 with programmable pulse width;
  auto RS-485 direction on RTS#; sleep with wake-up interrupt; ÷1/÷4 prescaler. Up to 3.125 Mbps at 5 V (p1).
- Features are date-code dependent (p1, p8, p50): "HC YYWW" and newer: 2.90–5.5 V, 5 V-tolerant inputs, auto RS485,
  programmable IR pulse width, sleep wake-up indicator, device ID/rev, 3.125 Mbps; "I2 YYWW" and newer: 0 ns address
  hold. Model the newest behavior.
- Nearest sibling in this group: ST16C550 (16-byte FIFO, same Intel pin set minus SEL; no enhanced registers). The
  PLCC44 Intel pinout is the ST16C550 PLCC44 pinout plus SEL on pin 34 (550: NC).

### Parts
| census part | package | pins | pinout name | grade | supply | status |
|---|---|---|---|---|---|---|
| ST16C650ACJ44 | 44-Lead PLCC | 44 | plcc44 | C: 0..+70 °C | 2.90–5.5 V | Obsolete |
| ST16C650CJ44 | 44-Lead PLCC (decoded by pattern: CJ44 = commercial PLCC44) | 44 | plcc44 | C: 0..+70 °C | **not covered by this PDF** | Obsolete |
| ST16C650IJ44 | 44-Lead PLCC (IJ44 = industrial PLCC44) | 44 | plcc44 | I: -40..+85 °C | **not covered by this PDF** | Active (Sipex row; census vcc blank; description "2.90 V TO 5.5 V UART WITH 32-BYT…") |
Ordering table p2 lists only ST16C650ACJ44 / ACQ48 / AIJ44 / AIQ48 (C 0..70 °C, I -40..+85 °C).
**ST16C650 (non-A)**: the PDF never mentions the non-A part (grep for "ST16C650 " / predecessor finds nothing). The
package/grade letters decode the same way (CJ44/IJ44), but its pinout, register set and electrical specs cannot be
confirmed from this PDF. Census ST16C650IJ44's DigiKey description ("2.90 V to 5.5 V UART with 32-byte…") is the
ST16C650A headline, suggesting DigiKey/Sipex sold the A die under the old number; ST16C650ACJ44's description
("SIPEX VIDEO AMP 1 CHANNEL") is a DigiKey error. Recommendation: treat ST16C650CJ44/IJ44 as ST16C650A (flag as
assumption) or drop them.

### Pinout plcc44 (Figure 2 p2; pin table p3-6). Function depends on SEL (pin 34): SEL=1 Intel / SEL=0 PC mode
```
pin  Intel (SEL=1)   role        | PC mode (SEL=0)  role
1    NC              nc          | A9      input  (internal 100 kΩ pull-up)
2    D0              inout       | D0
3    D1              inout       | D1
4    D2              inout       | D2
5    D3              inout       | D3
6    D4              inout       | D4
7    D5              inout       | D5
8    D6              inout       | D6
9    D7              inout       | D7
10   RCLK            input       | S2      input
11   RX              input       | RX
12   NC              nc          | A4      input  (internal 100 kΩ pull-up)
13   TX              output      | TX
14   CS0             input  (H)  | A5      input
15   CS1             input  (H)  | A6      input
16   CS2             input  (#)  | A7      input
17   BAUDOUT         output (#)  | LPT1    output (#)  (BAUDOUT internally tied to RCLK in PC mode)
18   XTAL1           input  (NOT 5 V tolerant)
19   XTAL2           output
20   IOWBAR          input  (#)  | IOWBAR  input (#)
21   IOW             input  (H)  | A8      input
22   GND             ground
23   NC              nc          | S1      input  (internal 100 kΩ pull-up)
24   IORBAR          input  (#)  | IORBAR  input (#)
25   IOR             input  (H)  | A3      input
26   DDIS            output (#)  | LPT2    output (#)
27   TXRDY           output (#)  | IRQC    output (active high, 3-state)
28   AS              input  (#)  | AEN     input (#)
29   A2              input       | A2
30   A1              input       | A1
31   A0              input       | A0
32   RXRDY           output (#)  | IRQB    output (active high, 3-state)
33   INT             output (H)  | IRQA    output (active high; 3-state / open-source per Table 3)
34   SEL             input  (internal 100 kΩ pull-up; 1 = Intel, 0 = PC)
35   OP2             output (#)  | S3      input
36   RTS             output (#)
37   DTR             output (#)
38   OP1             output (#)
39   RESET           input  (active high)
40   CTS             input  (#)
41   DSR             input  (#)
42   CD              input  (#)
43   RI              input  (#)
44   VCC             power
```
(#) active low, (H) active high. IOR and IOR# both exist → `IORBAR`/`IOWBAR` for the active-low ones. Pins whose
direction changes with SEL (10, 12, 17, 21, 23, 25, 26, 27, 32, 35, 1) must be modelled as mode-dependent
(input vs output). No exposed pad. (48-TQFP pinout p2/p3-6 exists but no census part uses it.)

### Host interface (p3-6, p9-10, AC p39-40, Fig 17-22 p42-44)
- **Intel mode (SEL=1)**: select = CS0=1 AND CS1=1 AND CS2#=0 (p3: "selected when all three chip selects are
  active"). Read when IOR#=0 or IOR=1; write when IOW#=0 or IOW=1; "Only one chipselect and one read strobe should be
  used" (Fig 19 note). IOR# falling edge starts read, data valid TRDV later, released TDD after strobe end; IOW#
  falling edge starts write, **trailing edge transfers the data** (p3). DDIS# = 0 while the host reads the chip
  (p6), TDIS after IOR#.
- **AS#** (p4): "the leading-edge transition of AS# latches the chip selects (CS0, CS1, CS2#) and the address lines
  A0, A1 and A2"; tie to GND if unused. Fig 19/20 (p43, image read): TAS1 address setup before AS# falling, TAH1
  address hold after AS# **rising**; TCS1 CS before AS# fall, TCSH CS hold after AS# rise; TRD1/TWR1 AS# to strobe.
  Model: latch A2..A0 and CS on AS# falling edge (address also required stable through AS# low, so a transparent-
  while-low/latch-on-rise model is equally consistent). "I2 YYWW" and newer: address latched internally even with
  AS# tied low → 0 ns TAH (p4, p39).
- **PC mode (SEL=0)** (p4-5, p10-11, Table 1 p11): on-chip decoder on A9..A3 + AEN# with S3..S1:
  | S3 S2 S1 | COM decode (A9..A3) | port | IRQ pin |
  |---|---|---|---|
  | 0 0 0 | 0x3F8-0x3FF | COM1 | IRQB (PC IRQ4) |
  | 0 0 1 | 0x2F8-0x2FF | COM2 | IRQC (PC IRQ3) |
  | 0 1 0 | 0x3E8-0x3EF | COM3 | IRQB |
  | 0 1 1 | 0x2E8-0x2EF | COM4 | IRQC |
  | 1 0 0 | 0x3F8-0x3FF | COM1 | IRQA (IRQn) |
  | 1 0 1 | 0x2F8-0x2FF | COM2 | IRQA |
  | 1 1 0 | 0x3E8-0x3EF | COM3 | IRQA |
  | 1 1 1 | 0x2E8-0x2EF | COM4 | IRQA |
  | — | 0x278-0x27F | LPT2 → LPT2# output low | — |
  | — | 0x378-0x37F | LPT1 → LPT1# output low | — |
  UART selected when AEN#=0 and A9..A3 match; "When AEN# transition to logic 0, it decodes and validates COM 1-4 ports
  address" (p4). IOR#/IOW# strobes; IOR/IOW pins become A3/A8. Bus timing TAS3/TRD3/TRD4/TWR2/TWR3/TDS3/TDH3 (p40).
  In PC mode BAUDOUT is internally connected to RCLK (p5).
- D7..D0 3-state when not being read.

### Registers (Table 7 p22, Table 8 p23-24, text p24-37)
Address map (A2A1A0):
| addr | condition | read | write |
|---|---|---|---|
| 000 | LCR[7]=0 | RHR | THR |
| 001 | LCR[7]=0 | IER | IER |
| 010 | LCR≠0xBF | ISR | FCR |
| 011 | any | LCR | LCR |
| 100 | LCR[7]=0 | MCR | MCR |
| 101 | LCR[7]=0 | LSR | XFR (write-only, needs EFR[4]=1) |
| 110 | LCR[7]=0 | MSR | IRPW (write-only, needs EFR[4]=1) |
| 111 | LCR[7]=0 | SPR | SPR |
| 000 | LCR[7]=1, LCR≠0xBF | DLL (DREV if DLL=DLM=0x00) | DLL |
| 001 | LCR[7]=1, LCR≠0xBF | DLM (DVID if DLL=DLM=0x00) | DLM |
| 010 | LCR=0xBF | EFR | EFR |
| 100 | LCR=0xBF | XON1 | XON1 |
| 101 | LCR=0xBF | XON2 | XON2 |
| 110 | LCR=0xBF | XOFF1 | XOFF1 |
| 111 | LCR=0xBF | XOFF2 | XOFF2 |
Accesses to 100-111 with LCR[7]=1 but LCR≠0xBF are not defined by Table 7 (only "LCR[7]=0" is listed for MCR/LSR/
MSR/SPR); 000/001 at LCR=0xBF not defined either. DVID reads 0x04; DREV e.g. 0x01 = rev A (p10, p34-35).

Bits ("shaded" = writable only while EFR[4]=1; retain values when EFR[4] is cleared, p35):
- IER: [0] RX data/timeout; [1] TX ready (THR empty non-FIFO / TX FIFO below TX trigger in FIFO mode; fires if empty
  when enabled); [2] RX line status; [3] modem status; [4]* sleep mode enable; [5]* Xoff/special-char interrupt
  enable; [6]* RTS# interrupt enable (RTS# pin LOW→HIGH under auto RTS); [7]* CTS# interrupt enable (CTS# LOW→HIGH
  under auto CTS) (p25-26).
- ISR (Table 9 p27): bits [5:0] code; [7:6] = 11 when FIFOs enabled.
  | pri | ISR[5:0] | source | cleared by |
  |---|---|---|---|
  | 1 | 000110 | LSR (LSR[4:1]) | LSR read |
  | 2 | 001100 | RX timeout | RHR read |
  | 3 | 000100 | RX data ≥ trigger | FIFO below trigger |
  | 4 | 000010 | TX ready (TX trigger / FIFO empty / transmitter empty in auto-RS485) | ISR read or THR write |
  | 5 | 000000 | MSR delta | MSR read |
  | 6 | 010000 | received Xoff or special char | ISR read, or Xon received (Xoff) / next char received (special) |
  | 7 | 100000 | CTS# or RTS# change (auto flow) | MSR read |
  | — | 000001 | none, or wake-up indicator | (wake-up cleared by ISR read) |
  (p26-27.) ISR[4] "once set ... will stay a logic 1 until a Xon character is received" (p27). ISR[5:4] only
  when EFR[4]=1.
- FCR (write-only, p27-28): [0] FIFO enable (must be 1 for other bits); [1] RX FIFO reset, [2] TX FIFO reset (only
  when FCR[0]=1, self-clear); [3] DMA mode; [5:4]* TX trigger; [7:6] RX trigger. Table 10 (p28):
  | FCR[7:6] | RX trigger | auto-RTS de-assert (RTS#→1) at | re-assert (RTS#→0) at |
  |---|---|---|---|
  | 00 | 8 | 16 | 0 |
  | 01 | 16 | 24 | 8 |
  | 10 | 24 | 28 | 16 |
  | 11 | 28 | 28 | 24 |
  | FCR[5:4] | TX trigger |
  |---|---|
  | 00 | 16 |
  | 01 | 8 |
  | 10 | 24 |
  | 11 | 30 |
  TX interrupt: when FIFO count falls below TX trigger, or when it becomes empty if it was not filled above the
  trigger on the last reload (p28). Text says default RX trigger "= 1" (p28) but Table 10 gives 8 for 00 — see Notes.
- LCR (p28-30): standard 16550 ([1:0] length, [2] stop, [3] parity en, [4] even, [5] stick, [6] break, [7] DLAB).
  Writing 0xBF selects the enhanced page; LCR[7]=1 with LCR≠0xBF selects DLL/DLM.
- MCR (p30-31): [0] DTR# (1→pin 0); [1] RTS# (1→pin 0; must be 1 before auto RTS acts); [2] OP1# (1→pin 0);
  [3] Intel: OP2# (1→pin 0); PC mode: IRQn enable; [4] loopback; [5]* INT type: 0 = active (push-pull) / 3-state,
  1 = open-source (Table 3); [6]* IrDA enable; [7]* prescaler: 0 = ÷1, 1 = ÷4.
- LSR (read-only, p31-32): [0] DR; [1] OE (interrupt immediately); [2] PE, [3] FE, [4] BI — tags for char in RHR;
  interrupt when char is in RHR (XFR[3]=0) or immediately on receipt (XFR[3]=1); "The break indication is cleared
  when LSR is read, but the RX input may still be a logic 0" (p31); [5] THRE (THR / TX FIFO empty); [6] TEMT;
  [7] FIFO error.
- XFR (write-only, needs EFR[4]=1, p32-33): [0] IrDA half-duplex (RX disabled while transmitting); [1] invert IR RX
  input (0: idles 0; 1: idles 1, pulses low); [2] auto RS-485 enable; [3] LSR bad-data interrupt timing (0 = when
  char reaches RHR, 16550-compatible; 1 = immediately on receipt, reset on LSR read, re-fires when char reaches
  RHR if not read); [4] Xon-Any (any received char acts as Xon); [5] invert RS-485 control output (0: RTS#=1 for
  transmit, 0 for receive; 1: RTS#=0 transmit, 1 receive — "User must assert RTS# for operation to take effect");
  [7:6] reserved.
- MSR (p33-34): [3:0] ΔCTS, ΔDSR, TERI (RI# 0→1), ΔCD; [7:4] CTS, DSR, RI, CD = complement of pins; loopback:
  [4]=MCR[1], [5]=MCR[0], [6]=MCR[2], [7]=MCR[3].
- IRPW (write-only, needs EFR[4]=1, p34): 0x00 = standard 3/16-bit pulse; N = 1..255 → IR TX pulse width = N ×
  crystal clock period (XTAL1 period, before prescaler). "Never allow PW to exceed the operating data rate bit
  period, else the encoder stops."
- SPR: R/W, preserved in sleep, reset FF.
- DLL/DLM: baud = (clock / prescaler / 16) / divisor (p34).
- EFR (LCR=0xBF, p35-36): [3:0] software flow control select (Table 12):
  | EFR[3:2] (TX) | meaning | EFR[1:0] (RX) | meaning |
  |---|---|---|---|
  | 00 | no TX flow control | 00 | no RX flow control |
  | 10 | transmit Xon1/Xoff1 | 10 | receiver compares Xon1/Xoff1 |
  | 01 | transmit Xon2/Xoff2 | 01 | receiver compares Xon2/Xoff2 |
  | 11 | transmit Xon1+Xon2 / Xoff1+Xoff2 (2-char sequences) | 11 | see combos |
  Special combos with EFR[1:0]=11: EFR[3:0]=1011 → TX Xon1/Xoff1, RX compares Xon1 **or** Xon2, Xoff1 **or** Xoff2;
  0111 → TX Xon2/Xoff2, RX compares Xon1 or Xon2 / Xoff1 or Xoff2; 1111 → TX Xon1&Xon2/Xoff1&Xoff2, RX compares the
  2-char sequences; 0011 → no TX flow control, RX compares Xon1&Xon2/Xoff1&Xoff2. "Whenever changing the TX or RX
  flow control bits, always reset all bits back to logic 0 before programming a new setting" (p35).
  [4] enhanced-function enable (unlocks IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5], XFR, IRPW; clearing it latches
  them; reset clears them to 16550-compatible 0); [5] special character detect (compare each RX char with XOFF2;
  match → char still stored in FIFO and ISR[4] set; with EFR[1:0]=01 (Xon2/Xoff2 compare) Xoff2 is NOT stored and
  gives both Xoff and special interrupts); [6] auto RTS; [7] auto CTS.
- XON1/XON2/XOFF1/XOFF2: R/W at LCR=0xBF; only LCR[1:0] word-length bits compared (bit 0 = first/LSB bit) (p18).
- Reset (Table 13 p37): DLL/DLM/RHR/THR XX; IER 00; FCR 00; ISR 01; LCR 00; MCR 00; LSR 60; XFR 00; MSR[3:0]=0,
  [7:4] = "logic levels of the inputs" (see Notes); IRPW 00; SPR FF; EFR 00; XON1/2, XOFF1/2 = 00. I/O: TX = 1
  (normal) / 0 (infrared — moot since MCR[6]=0 after reset); RTS# 1; DTR# 1; OP1# 1; OP2# 1; TXRDY# 0; RXRDY# 1; INT
  (Intel) 0; IRQA/B/C (PC) three-state. Pins LPT1#/LPT2#/DDIS# not listed (combinational: high unless decoded/read).

### Behavior
- Clock/BRG (p12-13): XTAL1/XTAL2 crystal 1.8432–24 MHz (500 kΩ–1 MΩ feedback) or external clock on XTAL1 (not 5 V
  tolerant). Prescaler ÷1 (MCR[7]=0) / ÷4 (MCR[7]=1) → BRG divisor 1..2^16-1 → 16× clock. Divisor = (XTAL1 /
  prescaler) / (baud × 16). TX uses the BRG; RX uses RCLK pin in Intel mode (BAUDOUT# must be wired to RCLK, p5) and
  the internal BAUDOUT in PC mode. BAUDOUT# = 16× clock output (Intel mode). Ext clock max 33 MHz @3.3 V, 50 MHz
  @5 V; crystal max 20 / 24 MHz (p39).
- Receiver (p15): start validated after 8 16× clocks; bits sampled mid-bit; error tags per char (32 × 11-bit FIFO).
  Timeout: no char for 4 word lengths (LCR[1:0]) + 12 bit times (p15).
- Transmitter: start, 5-8 data LSB first, parity, stop; LSR[5]/[6] as 16550. TX interrupt per TX trigger (above).
- DMA pins (Table 2 p10): RXRDY#/TXRDY# exactly as ST16C550 mode 0/1.
- INT (Table 3 p11): Intel: MCR[5]=0 → INT push-pull, 0 inactive / 1 active; MCR[5]=1 → INT 3-state when inactive,
  drives 1 when active (open source, external 1–10 kΩ pull-down). PC mode: S3=0 → IRQA 3-state (IRQB or IRQC used per
  Table 1); S3=1 & MCR[3]=0 → IRQA 3-state; S3=1 & MCR[3]=1 & MCR[5]=0 → IRQA push-pull active high; S3=1 & MCR[3]=1 &
  MCR[5]=1 → IRQA open-source. IRQB/IRQC: "active high, tri-state", enabled by MCR[3]=1 (p4); only the one selected by
  Table 1 is used (other stays 3-state).
- Auto RTS (p16-17): EFR[6]=1 and MCR[1]=1; RTS# goes 1 when RX FIFO reaches the de-assert level (next trigger above
  the programmed one, Table 10) and back to 0 when the FIFO drains to the re-assert level; receiver keeps accepting
  until full. IER[6] → interrupt (ISR[5]) when RTS# goes 1. Not supported in internal loopback (p21).
- Auto CTS (p16): EFR[7]=1; TX suspends after the current character's stop bit when CTS#=1, resumes on CTS#=0;
  IER[7] → interrupt (ISR[5]) on CTS# 0→1.
- Auto Xon/Xoff (p17-18, Table 5): RX compare per EFR[1:0]; on Xoff match TX halts after current char, ISR[4]/Xoff
  interrupt (IER[5]); resumes on Xon match (or any char if XFR[4]). Flow chars are not stored in the RX FIFO. TX side
  (EFR[3:2]): Xoff sent 2 character-times after RX FIFO reaches the trigger; Xon sent when FIFO drops to one trigger
  level below:
  | RX trigger | INT at | Xoff sent at (chars in FIFO) | Xon sent at |
  |---|---|---|---|
  | 8 | 8 | 8* | 0 |
  | 16 | 16 | 16* | 8 |
  | 24 | 24 | 24* | 16 |
  | 28 | 28 | 28* | 24 |
  (*after 2 char times). Flow-control register reset = 0x00 (p18).
- Special char (p18): EFR[5]; compare each RX char with XOFF2 (word-length bits); match → char stored + ISR[4].
- Auto RS-485 (p18, XFR p33): XFR[2]=1: RTS# asserted for transmit before the first bit and returned after the
  last stop bit of the last character when TX FIFO and TSR are empty; polarity per XFR[5] (Table 6: XFR[5]=0 →
  RTS#=1 during TX, 0 during RX; =1 → inverted). TX interrupt then means "transmitter empty" (p26).
- IrDA (p19, Fig 13): MCR[6]=1; TX idles 0, each 0 data bit → high pulse 3/16 bit wide (or IRPW×Tclk) — pulse
  starts mid-bit per Fig 13 ("1/2 Bit Time" mark); RX: each light pulse (high on RX, or low if XFR[1]=1) → 0 bit;
  idle 0; "1/16 Clock Delay" on decode (Fig 13). XFR[0] half-duplex.
- Sleep (p20): enters when IER[4]=1 AND no interrupt pending (ISR[0]=1) AND MSR[3:0]=0 AND RX idle 1; oscillator
  stopped (XTAL2 no clock). Wakes on RX 1→0, THR/FIFO write, or change on CTS#/DSR#/CD#/RI#; then issues a wake-up
  interrupt with ISR = 0x01 (indistinguishable from "none"), cleared by ISR read; re-enters sleep automatically
  when conditions return; leave by IER[4]=0. Isleep 30 µA (3.3 V) / 100 µA (5 V) (p38).
- Loopback (p21, Fig 14): TX = 1, RTS#/DTR#/OP1#/OP2# outputs de-asserted (VCC), modem inputs ignored;
  RTS→CTS, DTR→DSR, OP1→RI, OP2→CD; RX must be 1; auto RTS/CTS not supported in loopback.
- Reset: RESET active high ≥ 40 ns (p6, p10).

### Timing (AC table p39-40; 2.90–5.5 V; 70 pF; ns; columns 3.3 V / 5.0 V)
| sym | parameter | 3.3 V | 5.0 V |
|---|---|---|---|
| CLK | clock pulse duration | 30 | 20 |
| OSC | crystal frequency (max) | 20 MHz | 24 MHz |
| OSC | external clock (max) | 33 MHz | 50 MHz |
| TAS / TAH | address setup / hold (AS# low) | 5 / 10 ("HC" and older), 0 ("I2" newer) | 5 / 10, 0 |
| TCS | chip select width | 50 | 40 |
| TRD | IOR# strobe width | 50 | 40 |
| TDY | read/write cycle delay | 40 | 30 |
| TRDV | data access (max) | 40 | 30 |
| TDD | data disable (max) | 15 | 10 |
| TWR | IOW# strobe width | 50 | 40 |
| TDS1 / TDH1 | data setup / hold (AS# low) | 20 / 5 | 10 / 5 |
| TASW | AS# width | 35 | 25 |
| TAS1/TAH1, TAS2/TAH2 | address setup/hold (AS# used) | 5/10 | 5/10 |
| TCS1 / TCSH / TCS2 | CS to AS# / AS# to CS / AS# to CS | 5 / 0 / 5 | 5 / 0 / 5 |
| TRD1 / TRD2 | AS# to read / CS to IOR# | 10 / 10 | 10 / 10 |
| TDIS | IOR# to DDIS# (max) | 15 | 10 |
| TWR1 | AS# to IOW# | 10 | 10 |
| TDS2 / TDH2 | data setup / hold (AS# used) | 20 / 5 | 10 / 5 |
| TAS3 / TRD3 / TRD4 / TWR2 / TWR3 | PC mode address setup, AEN#↔IOR#/IOW# | 5/10/10/10/10 | same |
| TDS3 / TDH3 | data setup / hold (PC) | 20 / 5 | 10 / 5 |
| TWDO / TMOD / TRSI | IOW#→modem out / modem in→INT / IOR#→INT reset (max) | 50 / 40 / 40 | 40 / 35 / 35 |
| TSSI / TSSR | stop to INT / to RXRDY# | 1 Bclk | 1 Bclk |
| TRRI / TSI / TWRI / TRR / TWT | (max) | 45 each | 40 each |
| TINT | initial INT reset to TX start | 8..24 Bclk | 8..24 Bclk |
| TSRT | centre of start to reset TXRDY# | 8 Bclk | 8 Bclk |
| TRST | reset pulse width | 40 | 40 |
Max data rate 2 Mbps @3.3 V, 3.125 Mbps @5 V (p50).

### VCC
2.90–5.5 V (p1, p38). Inputs 5 V tolerant except XTAL1 for "HC YYWW" and newer (p6, p8): VIH max 5.5 V (p38); older
"GC" parts VIH max VCC. Abs max any pin -0.5 to 7 V (p38). inputMax 5.5 V (XTAL1: VCC).

### Package drawing
44-pin PLCC (p49, inch controlling): e 1.27 (0.050 BSC); D 17.40–17.65 (span over leads); D1 16.51–16.66 (body);
D2 14.99–16.00; D3 12.70 typ; B 0.33–0.53; B1 0.66–0.81; A 4.19–4.57; A2 0.51 min. No land pattern. (48-TQFP p48:
D 8.80–9.20, D1 6.90–7.10, e 0.50 — no census part.)

### Obsolete
ST16C650ACJ44 and ST16C650CJ44 (DigiKey Obsolete); whole PDF watermarked OBS. ST16C650IJ44 census Active.

### Notes
- RS-485 enable bit: p18 text says "enabled by XFR bit-3"; Table 6 (p18) and XFR description (p33) say XFR[2].
  Use XFR[2] (XFR[3] is LSR bad-data interrupt). p18 also says it "re-asserts RTS# (logic 1)" — polarity per Table 6.
- RX trigger default: FCR[7:6] text "(logic 0 = default, RX trigger level =1)" vs Table 10 00 → 8. The 650A family
  table governs: 00 = 8.
- MSR reset: Table 13 says bits 7-4 = "logic levels of the inputs" (others say "inverted"); MSR bits are the
  complement of the pins per p33-34.
- Table 8 EFR row lists "MSR[7:4]" among EFR[4]-gated items; text (p35) does not — ignore.
- Sleep wake-up interrupt shares ISR code 0x01 with "no interrupt".
- ST16C650 (non-A) is not documented in this PDF (see Parts).
## ST16C554 (and ST16C554D)

- Title (as printed): "ST16C554 / ST16C554D / ST68C554 2.97V to 5.5V Quad UARTs with 16-Byte FIFO", MaxLinear
  Rev 4.0.2, 9/4/19. Function: four independent 16C550-compatible UARTs (A-D), shared bus, one XTAL. Datasheet:
  `datasheets/controller-uart/st16c554.pdf`, 39 PDF pages; printed page numbers = PDF page − 6 (6 roman front pages).
  Cites below are **printed** page numbers.
- Register set per channel = ST16C550 (16550) exactly; no EFR/auto flow/IrDA despite stray text (see Notes).
- **ST16C554 vs ST16C554D**: same die. The D adds the PLCC68 package with 16/68# (Intel/Motorola) and INTSEL pins
  plus wire-ORed TXRDY#/RXRDY# pins. In the LQFP64 neither 16/68#, INTSEL, TXRDY# nor RXRDY# is bonded out; the
  only difference between the two LQFP64 parts is how INTSEL is bonded internally (p2, p5):
  * ST16C554CQ64: INTSEL bonded to **GND** → INT A-D are 3-state until that channel's MCR[3]=1.
  * ST16C554DCQ64 / DIQ64: INTSEL bonded to **VCC** → INT A-D always driven ("continuous interrupt mode"),
    MCR[3] has no effect on INT.
  Intel mode only in LQFP64 (16/68# effectively high). So model one device with a per-part constant `intsel`.

### Parts
| census part | package | pins | pinout name | grade | supply |
|---|---|---|---|---|---|
| ST16C554CQ64-F | LQFP64 10x10x1.4 mm | 64 | lqfp64 | C: 0..70 °C; INTSEL=GND (3-state INT) | 2.97–5.5 V |
| ST16C554DCQ64-F | LQFP64 | 64 | lqfp64 | C: 0..70 °C; INTSEL=VCC (continuous INT) | 2.97–5.5 V |
| ST16C554DIQ64-F | LQFP64 | 64 | lqfp64 | I: -40..+85 °C; INTSEL=VCC | 2.97–5.5 V |
Ordering table p32 (Table 21): CQ64-F tray, DCQ64-F tray, DIQ64-F tray, all lead-free.
Speed grade: 1.5 Mbps / 24 MHz at 5 V, 500 kbps / 8 MHz at 3.3 V (p i, AC table p23).

### Pinout lqfp64 (Figure 4 p1 "LQFP64 Pinout, Intel Mode Only"; Table 1 p2-4 LQFP64 column)
```
1  DSRA   input (#)     17 DSRB   input (#)     33 CTSC   input (#)     49 CDD    input (#)
2  CTSA   input (#)     18 CDB    input (#)     34 DTRC   output (#)    50 RID    input (#)
3  DTRA   output (#)    19 RIB    input (#)     35 VCC    power         51 RXD    input
4  VCC    power         20 RXB    input         36 RTSC   output (#)    52 VCC    power
5  RTSA   output (#)    21 VCC    power         37 INTC   output        53 D0     inout
6  INTA   output        22 A2     input         38 CSC    input (#)     54 D1     inout
7  CSA    input (#)     23 A1     input         39 TXC    output        55 D2     inout
8  TXA    output        24 A0     input         40 IOR    input (#)     56 D3     inout
9  IOW    input (#)     25 XTAL1  input         41 TXD    output        57 D4     inout
10 TXB    output        26 XTAL2  output        42 CSD    input (#)     58 D5     inout
11 CSB    input (#)     27 RESET  input (high)  43 INTD   output        59 D6     inout
12 INTB   output        28 GND    ground        44 RTSD   output (#)    60 D7     inout
13 RTSB   output (#)    29 RXC    input         45 GND    ground        61 GND    ground
14 GND    ground        30 RIC    input (#)     46 DTRD   output (#)    62 RXA    input
15 DTRB   output (#)    31 CDC    input (#)     47 CTSD   input (#)     63 RIA    input (#)
16 CTSB   input (#)     32 DSRC   input (#)     48 DSRD   input (#)     64 CDA    input (#)
```
(#) active low. INTA-D active high; 3-state per INTSEL/MCR[3] (below). No NC pins; no exposed pad. VCC 4,21,35,52;
GND 14,28,45,61. (PLCC68 has 16/68# pin 31, INTSEL 65, TXRDY# 39, RXRDY# 38, and Motorola alternates:
IOW#→R/W#, CSA#→CS#, CSB#→A3, CSC#→A4, CSD#/IOR#→VCC, INTA→IRQ# open drain, RESET→RESET# — not in any census
part.)

### Host interface (p2-7, AC p23, Fig 16/17 p25)
- Intel (16 mode) only on LQFP64: IOR# falling edge starts read, data presented for the host to latch on IOR#
  rising (p2); IOW# falling edge starts write, **rising edge transfers data** (p2). No address latch: A2..A0 valid
  TAS before/TAH after strobe.
- Channel select (Table 2 p7): CSA#..CSD# active low, one per channel; all high = deselected; several low = write
  goes to all selected channels ("do not attempt to read from all four UARTs simultaneously" → contention, X).
- D7..D0 3-state except during a selected read. No DDIS, no clock needed for bus cycles (p6).

### Registers (per channel; Tables 8-9 p13-14, text p15-21)
Same addresses, bits and defaults as ST16C550 (see that section). Specific statements:
- IER[7:4] reserved (0). IER[1]: "If the THR is empty when this bit is enabled, an interrupt will be generated" (p15).
  IER[2]: LSR[1] interrupts immediately on overrun; LSR[4:2] interrupt when the char with the error is in RHR (p15).
- ISR[5:4] reserved 0; ISR[7:6] = 11 when FIFOs enabled. Priority (Table 10 p16): 1 LSR 0110; 2 RX timeout 1100;
  3 RX data 0100; 4 TX empty 0010; 5 MSR 0000; none 0001.
- FCR[5:4] reserved; RX trigger 1/4/8/14 (Table 11 p17); FCR[1]/[2] only act when FCR[0]=1.
- MCR[2]: reserved (OP1#, internal RI in loopback only). **MCR[3] INT output enable** (p18-19, Table 15):
  | INTSEL | MCR[3] | INT A-D (16 mode) |
  |---|---|---|
  | 0 | 0 | three-state |
  | 0 | 1 | active |
  | 1 | X | active |
  MCR[3] also drives internal OP2 (→CD) in loopback. No OP2# pin. MCR[7:5] reserved.
- LSR and MSR are described as "Read and Write ... writeable, but it is not recommended" (p19, p20) — model writes
  as storing into the register (16550 behavior: write overwrites status bits) or ignore; see Notes.
- SPR R/W, reset FF.
- Reset (Table 16 p21): **DLL/DLM unknown at power-up and NOT reset by the RESET pin**; RHR/THR XX; IER 00; FCR 00;
  ISR 01; LCR 00; MCR 00; LSR 60; MSR[3:0]=0, [7:4] = inverted input levels; SPR FF. I/O: TX HIGH, RTS# HIGH,
  DTR# HIGH, RXRDY# HIGH, TXRDY# LOW, INT (16 mode) LOW, IRQ# (68 mode) HIGH (INTSEL=LOW). (Table also lists "IRTX
  LOW" — no such pin.)

### Behavior
- Clock/BRG (p8-9): crystal (fundamental, parallel resonant) with 300–400 kΩ feedback R (Fig 7) or external clock on
  XTAL1; per channel divisor = XTAL1/(16 × baud), 1..2^16-1; 16× sampling; TX and RX of a channel share its BRG.
  No prescaler (Table 7 shows an "MCR Bit-7=1" column with rates /4 — leftover; MCR[7] reserved). OSC max 8 MHz @
  3.3 V, 24 MHz @ 5 V (p23).
- RX start validated after 8 16× clocks at start-bit centre, all bits sampled mid-bit (p10). Timeout: 4 word
  lengths (LCR[1:0]) + 12 bit times = 3.7–4.6 char times (p10).
- TX FIFO/THR, LSR[5]/[6], break, overrun, error tags: as ST16C550/ST16C2550.
- Interrupt generation/clearing (p16): LSR ← LSR[4:1], cleared by LSR read; RXRDY ← trigger, cleared below trigger;
  timeout ← timer, cleared by reading RHR; TXRDY ← THR empty / TX FIFO empty, cleared by ISR read or THR write;
  MSR ← MSR[3:0], cleared by MSR read.
- INT pin tables (Tables 4/5 p7-8) are descriptive of an enabled interrupt (TX: high when THR/FIFO empty or "below
  trigger level"; RX: high when ≥1 byte / FIFO above trigger).
- TXRDY#/RXRDY# (per channel, internal only on LQFP64; Table 6 p8): as ST16C550 DMA mode 0/1. (PLCC68 pins are the
  AND of the four channels' active-low signals.)
- Loopback (p12 Fig 13): TX held HIGH, RTS#/DTR# de-asserted, modem inputs ignored; RTS→CTS, DTR→DSR, OP1
  (MCR[2])→RI, OP2 (MCR[3])→CD. RX must be held HIGH during loopback or a false break may be seen on exit.
- RESET active high, ≥40 ns (p4, p7).
- No sleep, no IrDA, no auto RTS/CTS, no device ID.

### Timing (Table 20 p23-24; 3.3 V ±10 % / 5.0 V ±10 %; 70 pF; ns)
| sym | parameter | 3.3 V | 5.0 V |
|---|---|---|---|
| CLK | external clock low/high time (min) | 63 | 21 |
| OSC | crystal / external clock frequency (max) | 8 MHz | 24 MHz |
| TAS / TAH | address setup / hold (16 mode) | 5 / 5 | 0 / 5 |
| TCS | chip select width | 80 | 50 |
| TRD | IOR# strobe width | 80 | 50 |
| TDY | read cycle delay | 40 | 30 |
| TRDV | data access (max) | 40 | 25 |
| TDD | data disable (max) | 25 | 15 |
| TWR | IOW# strobe width | 35 | 25 |
| TDY | write cycle delay | 40 | 30 |
| TDS / TDH | data setup / hold | 20 / 5 | 15 / 5 |
| TADS/TADH/TRWS/TRDA/TRDH/TWDS/TWDH/TRWH/TCSL/TCSD | 68-mode only (PLCC68) | 10/15/10/40/25/20/10/10/80/40 | 10/15/10/25/15/15/10/10/50/30 |
| TWDO | IOW# to modem output (max) | 50 | 40 |
| TMOD | modem input to INT (max) | 40 | 35 |
| TRSI | IOR# to reset INT (max) | 40 | 35 |
| TSSI | stop to set INT | 1 Bclk | 1 Bclk |
| TRRI | IOR# to reset INT (max) | 45 | 40 |
| TSI | start to interrupt (max) | 45 | 40 |
| TINT | initial INT reset to TX start | 8..24 Bclk | 8..24 Bclk |
| TWRI | IOW# to reset INT (max) | 45 | 40 |
| TSSR | stop to set RXRDY# | 1 Bclk | 1 Bclk |
| TRR | IOR# to reset RXRDY# (max) | 45 | 40 |
| TWT | IOW# to set TXRDY# (max) | 45 | 40 |
| TSRT | centre of start to reset TXRDY# | 8 Bclk | 8 Bclk |
| TRST | reset pulse width (min) | 40 | 40 |

### VCC
2.97–5.5 V (p i, DC table p22). Abs max any pin GND-0.3 to VCC+0.3, VIH max = VCC (p22) → **not 5 V tolerant**
(inputMax = VCC). ICC 3 mA (3.3 V) / 6 mA (5 V).

### Package drawing
LQFP64 10x10x1.4 mm, MaxLinear drawing POD-00000092 rev A, JEDEC MS-026 (BCD) (Figure 26, printed p30 = PDF p36; image
read): D/E 12.00 BSC (span), D1/E1 10.00 BSC (body), e 0.50 BSC, b 0.17/0.20/0.23 (min/nom/max), L 0.45/0.60/0.75,
L1 1.00 REF, A 1.60 max, A1 0.05–0.15, A2 1.35/1.40/1.45, c 0.09–0.16. **Typical recommended land pattern printed
on the same page**: pads 0.3 mm wide × 1.5 mm long, pitch 0.5, pad-centre to pad-centre across the package 11.4 mm
both directions.

### Obsolete
None (all three census rows Active).

### Notes
- Copy-paste artifacts to ignore: features "infrared encoder and decoder interface" (p5) and block diagram "IR ENDEC",
  "IRTXA"; reset table "IRTX LOW"; MSR[4] "A HIGH on the CTS# pin will stop the UART transmitter" (p20) — the pin
  table says modem inputs have "no effect on the UART" (p3) and there is no EFR, so no auto-CTS; Table 9 "LCR0xBF"
  comment; Table 7 "MCR Bit-7=1" column; Fig 13 caption "Channels A and B".
- Reset table says INT (16 mode) = LOW, but with INTSEL=0 and MCR[3]=0 (reset) INT is three-state (Table 15). For
  CQ64 model INT = Z after reset; for DCQ64/DIQ64 INT = 0 (driven, no pending interrupt).
- LSR/MSR "Read and Write" (writable, not recommended) — other Exar 16550 datasheets list them read-only.
- DLL/DLM explicitly not reset by RESET.

## XR16C2550

- Title (as printed): "XR16C2550 2.97V TO 5.5V DUART WITH 16-BYTE FIFO", Rev 1.0.2, February 2007 (p1). Datasheet:
  `datasheets/controller-uart/xr16c2550.pdf`, 37 pages (PDF = printed page for body pages).
- **Same device as ST16C2550** (see that section): pin-for-pin identical 48-TQFP pinout (Figure 2 p2, pin table
  p3-5), identical register descriptions (word-level comparison of sections 3.0–4.11 found no differences other than
  page cross-references), identical reset table (Table 11 p24) and identical AC table (p26). Differences, all
  minor: datasheet temperature is industrial only (-40..+85 °C, DC/AC headers p25-26); "All inputs are 5V tolerant"
  unconditionally (p1, p5) instead of only for "A2 YYWW" date codes; rev history (p35) says it applies to top mark
  date code "B2 YYWW" and newer. Model: reuse the ST16C2550 model.

### Parts
| census part | package | pins | pinout name | grade | supply |
|---|---|---|---|---|---|
| XR16C2550IM-F | 48-Lead TQFP 7x7x1 mm | 48 | tqfp48 (same pin map as ST16C2550 tqfp48) | I: -40..+85 °C | 2.97–5.5 V |
Ordering (p2): XR16C2550IJ = 44-PLCC, XR16C2550IM = 48-TQFP, both -40..+85 °C. -F = RoHS.

### Pinout tqfp48
Identical to ST16C2550 tqfp48 (pin table p3-5): 1 D5, 2 D6, 3 D7, 4 RXB, 5 RXA, 6 TXRDYB, 7 TXA, 8 TXB, 9 OP2B,
10 CSA, 11 CSB, 12 NC, 13 XTAL1, 14 XTAL2, 15 IOW, 16 CDB, 17 GND, 18 RXRDYB, 19 IOR, 20 DSRB, 21 RIB, 22 RTSB,
23 CTSB, 24 NC, 25 NC, 26 A2, 27 A1, 28 A0, 29 INTB, 30 INTA, 31 RXRDYA, 32 OP2A, 33 RTSA, 34 DTRA, 35 DTRB,
36 RESET, 37 NC, 38 CTSA, 39 DSRA, 40 CDA, 41 RIA, 42 VCC, 43 TXRDYA, 44 D0, 45 D1, 46 D2, 47 D3, 48 D4.
Roles as ST16C2550 (all modem/strobe/select pins active low; INTA/INTB active high, 3-state when MCR[3]=0).

### Host interface, registers, behavior
As ST16C2550 (pages here: CPU interface p7-8, Table 1 p8, DMA Table 2 p8, INT Tables 3/4 p8, oscillator p9,
BRG p9-11 incl. 64 MHz external clock with 2 kΩ XTAL2 pull-up, transmitter p11-12, receiver p12, loopback p14,
registers Tables 6/7 p15-16, IER..DLM p16-24, reset Table 11 p24).

### Timing (AC table p26; TA -40..+85 °C; Vcc 2.97–5.5 V; 70 pF)
Identical numbers to ST16C2550: crystal 20/24 MHz (3.3/5 V), CLK 17/8 ns, OSC 30/64 MHz, TAS 5/0, TAH 10/5, TCS/
TRD/TDY 70*/40 (*55 ns at 3.3 V +10/-5 % and 0..70 °C), TRDV 35/25, TDD 0-25/0-15, TWR 40/25, TDS 20/15, TDH 5/5,
TWDO 50/40, TMOD 40/35, TRSI 40/35, TSSI 1 Bclk, TRRI 45/40, TSI 45/40, TINT 8..24 Bclk, TWRI 45/40, TSSR 1 Bclk,
TRR 45/40, TWT 45/40, TSRT 8 Bclk, TRST 40.

### VCC
2.97–5.5 V. VIH max 5.5 V at both supplies (p25) and "All inputs are 5V tolerant" (p5) → inputMax 5.5 V. (Abs max
line says "GND-0.3 V to VCC+0.3 V" (p25) — contradicts; see Notes.)

### Package drawing
48-pin TQFP 7x7x1 mm (p33): D 8.80–9.20, D1 6.90–7.10, e 0.50, B 0.17–0.27, L 0.45–0.75, A 1.00–1.20, A1 0.05–0.15,
A2 0.95–1.05. No land pattern.

### Obsolete
None (Active).

### Notes
- Abs max "Voltage at any pin VCC+0.3 V" vs "5V tolerant inputs" / VIH max 5.5 V: the 5 V-tolerant statements are
  explicit and repeated; take inputMax = 5.5 V.
- SPR text "preserved during sleep mode" — no sleep mode.
## XR16L580

- Title (as printed): "XR16L580 SMALLEST 2.25V TO 5.5V UART WITH 16-BYTE FIFO AND POWERSAVE", Rev. 1.4.2, August
  2011 (p1). Datasheet: `datasheets/controller-uart/xr16l580.pdf`, 49 pages (PDF page = printed page).
- Function: single UART, 16-byte TX/RX FIFOs, ST16C580-compatible enhanced registers (EFR, Xon1/2, Xoff1/2 at
  LCR=0xBF), selectable TX/RX triggers, auto RTS/CTS with hysteresis, auto Xon/Xoff + special char + Xon-any, IrDA
  1.0, sleep with wake-up interrupt, **PwrSave** pin (bus isolation in sleep), Intel/Motorola bus select (16/68#),
  ÷1/÷4 prescaler, DVID/DREV. No TXRDY#/RXRDY#/RCLK/BAUDOUT#/OP1#/OP2#/IOR/IOW/CS0/CS1/AS#/DDIS# pins (p1).
- Nearest sibling in this group: ST16C650A — same enhanced-register concept, but L580 has 16-byte FIFOs with
  16550 trigger tables (1/4/8/14), no XFR/IRPW (their functions moved to MCR: MCR[2] IR invert, MCR[5] Xon-any), no
  RS-485, no PC mode, adds Motorola mode and PwrSave; DVID = 0x01 (650A: 0x04).

### Parts
| census part | package | pins | pinout name | grade | supply | status |
|---|---|---|---|---|---|---|
| XR16L580IL-F | 32-Lead QFN 5x5x0.9 mm, exposed pad | 32 (+EP) | qfn32 | I: -40..+85 °C | 2.25–5.5 V | Obsolete |
| XR16L580ILTR-F | 32-Lead QFN 5x5x0.9 mm, tape & reel | 32 (+EP) | qfn32 | I: -40..+85 °C | 2.25–5.5 V | Obsolete |
| XR16L580IM-F | 48-Lead TQFP 7x7x1 mm | 48 | tqfp48 | I: -40..+85 °C | 2.25–5.5 V | Active |
Ordering table p3: IL24-F / IL24TR-F = 24-QFN; **IL-F / ILTR-F = 32-QFN**; IM-F / IMTR-F = 48-TQFP; "TR = Tape and
Reel, F = Green / RoHS". So the census case "24-VFQFN Exposed Pad, 24-QFN (4x4)" for XR16L580ILTR-F is a DigiKey
error — the part number (no "24") and the census description ("…32QFN") both say 32-QFN 5x5.

### Pinout qfn32 (Figure 2 p2 — page image checked; pin table p4-6). Pin function depends on 16/68# (pin 2)
```
pin  16/68#=1 (Intel)   role         | 16/68#=0 (Motorola)
1    D4                 inout
2    16/68              input  (no internal pull-up on QFN; 1 = Intel, 0 = Motorola)
3    D5                 inout
4    D6                 inout
5    D7                 inout
6    RX                 input
7    TX                 output
8    CS                 input (#)
9    PWRSAVE            input  active high (no internal pull-down on QFN)
10   XTAL1              input  (NOT 5 V tolerant)
11   XTAL2              output
12   IOW                input (#)    | RW   input (1 = read, 0 = write)
13   GND                ground
14   IOR                input (#)    | unused (NC; tie off)
15   NC                 nc
16   NC                 nc
17   A2                 input
18   A1                 input
19   A0                 input
20   INT                output, active high, 3-state unless MCR[3]=1 | IRQ  output, active low, open drain
21   RTS                output (#)
22   DTR                output (#)
23   RESET              input, active high | RESET (RESET#) active low
24   CTS                input (#)
25   DSR                input (#)
26   CD                 input (#)
27   RI                 input (#)
28   VCC                power
29   D0                 inout
30   D1                 inout
31   D2                 inout
32   D3                 inout
EP   GND                ground (exposed thermal pad, "at GND potential", p6)
```

### Pinout tqfp48 (Figure 2 p2; pin table p4-6)
```
1  16/68  input (internal pull-up)    25 NC
2  D5     inout                       26 A2      input
3  D6     inout                       27 A1      input
4  D7     inout                       28 A0      input
5  NC                                 29 NC
6  NC                                 30 INT     output (Intel: active high, 3-state unless MCR[3]=1;
7  RX     input                                  Motorola: IRQ#, active low open drain)
8  TX     output                      31 NC
9  NC                                 32 RTS     output (#)
10 NC                                 33 DTR     output (#)
11 CS     input (#)                   34 NC
12 NC                                 35 RESET   input (Intel: active high; Motorola: RESET# active low)
13 PWRSAVE input (internal pull-down) 36 NC
14 XTAL1  input (not 5 V tolerant)    37 NC
15 XTAL2  output                      38 CTS     input (#)
16 IOW    input (#) / Motorola: RW    39 DSR     input (#)
17 NC                                 40 CD      input (#)
18 GND    ground                      41 RI      input (#)
19 IOR    input (#) / Motorola: NC    42 VCC     power
20 NC                                 43 D0      inout
21 NC                                 44 D1      inout
22 NC                                 45 D2      inout
23 NC                                 46 D3      inout
24 NC                                 47 D4      inout
                                      48 NC
```
No exposed pad on the TQFP. Pin-compatible with ST16C550/580/650A/XR16C850 48-TQFP minus the listed signals (p1).
(#) = active low. Name collision none (single CS; IOR/IOW are active-low only) → `IOR`, `IOW`, `CS`.

### Host interface (p4-5, p7-8, AC p39-40, Fig 16-19 p41-43)
- **Intel (16/68#=1)**: CS# low selects. IOR# falling edge starts read, data driven TRDV later, released TDD after
  IOR# rises (p4). IOW# falling edge starts write, **rising edge transfers data** (p4). No address latch; TAH = 0 ns
  ("'0 ns' Address Hold Time (TAH and TADH)", p1) — address sampled at the start of the strobe/CS.
- **Motorola (16/68#=0)**: CS# is the bus strobe; R/W# (pin IOW#) 1 = read, 0 = write; IOR# unused; INT becomes
  IRQ# (active low, open drain, external pull-up); RESET becomes RESET# (active low) (p4-5). Fig 18/19 (p42-43): R/W#
  set up TRWS before CS# falls and held TRWH after CS# rises; write data TWDS/TWDH around CS# rising; read data valid
  TRDA after CS# low, released TRDH after CS# high; address TADS before CS#, TADH = 0.
- D7..D0 3-state except during a selected read. No DMA pins: "The DMA Enable bit (FCR bit-3) does not have any
  function in this device" (p9).

### Registers (Table 5 p22, Table 6 p23-24, text p24-36)
| A2A1A0 | condition | read | write |
|---|---|---|---|
| 000 | LCR[7]=0 | RHR | THR |
| 001 | LCR[7]=0 | IER | IER |
| 010 | LCR≠0xBF | ISR | FCR |
| 011 | any | LCR | LCR |
| 100 | LCR≠0xBF | MCR | MCR |
| 101 | LCR≠0xBF | LSR | reserved |
| 110 | LCR≠0xBF | MSR | reserved |
| 111 | LCR≠0xBF | SPR | SPR |
| 000 | LCR[7]=1 (≠0xBF) | DLL (DREV when DLL=DLM=0x00) | DLL |
| 001 | LCR[7]=1 (≠0xBF) | DLM (DVID = 0x01 when DLL=DLM=0x00) | DLM |
| 010 | LCR=0xBF | EFR | EFR |
| 100 | LCR=0xBF | — | XON1 (Table 6 & §4.16: write-only) |
| 101 | LCR=0xBF | — | XON2 |
| 110 | LCR=0xBF | — | XOFF1 |
| 111 | LCR=0xBF | — | XOFF2 |
(Table 5 marks Xon/Xoff "Read/Write"; Table 6 and §4.16 p35 say write-only — see Notes.) 000/001 at LCR=0xBF: DLL/DLM
per "LCR[7]=1" in Table 6 (unclear).

Bits (* = EFR[4]-gated, p33):
- IER (p24-25): [0] RX data/timeout; [1] TX ready (THR empty / TX FIFO below TX trigger; fires if empty when
  enabled); [2] RX line status (OE immediate, PE/FE/BI when char reaches RHR); [3] modem status; [4]* sleep enable;
  [5]* Xoff/special char interrupt; [6]* RTS# interrupt (RTS# low→high); [7]* CTS# interrupt (CTS# low→high).
- ISR (Table 7 p27): same codes and priorities as ST16C650A: 1 LSR 000110; 2 RX timeout 001100; 3 RX data 000100;
  4 TX ready 000010; 5 MSR 000000; 6 Xoff/special 010000; 7 CTS#/RTS# change 100000; none/wake-up 000001. [7:6] FIFOs
  enabled. ISR[5:4] only when EFR[4]=1; ISR[4] stays 1 until Xon received. Clearing (p26): LSR int by LSR read
  ("but FIFO error bit does not clear until the character(s) … read from the FIFO"); RX data below trigger; timeout
  by RHR read; TX ready by ISR read or THR write; MSR by MSR read; Xoff by ISR read or Xon received; special char by
  ISR read or next char received; RTS#/CTS# by MSR read; wake-up by ISR read.
- FCR (write-only, p27-28): [0] FIFO enable; [1]/[2] RX/TX FIFO reset (FCR[0]=1, self-clear); [3] no function;
  [5:4]* TX trigger 00=1, 01=4, 10=8, 11=14; [7:6] RX trigger 00=1, 01=4, 10=8, 11=14 (Table 8 p28). TX interrupt when
  count falls below trigger, or on empty if not filled above trigger at last reload.
- LCR (p28-30): standard; 0xBF selects enhanced registers.
- MCR (p30-31): [0] DTR# (1→0); [1] RTS# (1→0; must be 1 for auto RTS); [2]* IR RX invert when MCR[6]=1 and EFR[4]=1
  (0 = RX input active-low encoded IrDA data (default), 1 = active-high; "In this mode, this bit is write-only"),
  else OP1 (loopback → RI); [3] INT output enable (Intel: 0 = 3-state, 1 = active; no function in Motorola mode;
  loopback → CD); [4] loopback; [5]* Xon-any (any RX char resumes TX; the char is stored unless it is an Xon/Xoff
  char under software flow control); [6]* IrDA enable (TX idles 0); [7]* prescaler 0 = ÷1, 1 = ÷4.
- LSR (read-only, p31-32): as 16550; LSR[4] "break indication remains until the RX input returns to the idle
  condition"; [7] clears when no errors remain in the FIFO.
- MSR (p32-33): as 16550; loopback [4]=MCR[1], [5]=MCR[0], [6]=MCR[2], [7]=MCR[3].
- SPR: R/W, preserved in sleep, reset FF (p33).
- DLL/DLM: baud = (clock/prescaler/16)/divisor; **power-up value 0x0001; not reset by the RESET pin** (Table 11 p36;
  p11 "defaults to the maximum baud rate (DLL = 0x01 and DLM = 0x00) upon power up").
- DVID 0x01, DREV e.g. 0x01 = rev A (p8, p33).
- EFR (LCR=0xBF, p33-35): [3:0] software flow control select — Table 10 p34 identical to ST16C650A Table 12 (TX:
  EFR[3:2] 00 none/10 Xon1-Xoff1/01 Xon2-Xoff2/11 both as 2-char sequences; RX: EFR[1:0] 00 none/10 Xon1-Xoff1/01
  Xon2-Xoff2; combos 1011, 0111 = RX compares Xon1 or Xon2 / Xoff1 or Xoff2; 1111 = TX & RX double; 0011 = no TX, RX
  double). Reset to 0 before changing flow-control bits. [4] enable enhanced bits (IER[7:4], ISR[5:4], FCR[5:4],
  MCR[2], MCR[7:5]); clearing latches them; reset clears them. [5] special char detect (vs XOFF2; char stored + ISR[4];
  with EFR[1:0]=01 Xoff2 not stored, gives Xoff + special interrupt if IER[5]). [6] auto RTS. [7] auto CTS.
- Reset (Table 11 p36): DLL/DLM 0x0001 power-up only; RHR/THR XX; IER 00; FCR 00; ISR 01; LCR 00; MCR 00; LSR 60;
  MSR[3:0]=0, [7:4] = inverted input levels; SPR FF; EFR 00; XON1/2, XOFF1/2 00. Outputs: TX 1; RTS# 1; DTR# 1;
  INT three-state (Intel). (Motorola IRQ# open drain → released/high-Z.)

### Behavior
- Clock/BRG (p10-12): crystal ≤24 MHz at 5 V (500 k–1 MΩ feedback) or external clock on XTAL1 up to 50 MHz (2 kΩ
  pull-up on XTAL2) — XTAL2 is a buffered clock output usable by other devices. Prescaler ÷1/÷4 (MCR[7]) → divisor
  1..2^16-1 → 16× clock shared by TX and RX. Divisor = (XTAL1/prescaler)/(baud×16).
- Receiver (p14): start validated after 8 16× clocks; data/parity/stop sampled mid-bit. Timeout 4 word lengths +
  12 bits.
- INT/IRQ# (Tables 1/2 p9-10): Intel INT active high (when MCR[3]=1, else Z); Motorola IRQ# active low, open drain;
  INT pin TX state: high when THR empty / FIFO below TX trigger or empty; RX: high when data / FIFO at-above trigger.
- Auto RTS (p16): EFR[6]=1 and MCR[1]=1. RTS# de-asserts (→1) when RX FIFO reaches **one trigger level above** the
  programmed RX trigger (Table 8 ladder 1/4/8/14); re-asserts (→0) when FIFO falls to **one level below**. Example
  Fig 10 (p15): trigger 8 → RTS# off at 14, on at 4. (For trigger 14, "next level above" is undefined — assume FIFO
  full/16; see Notes.) IER[6] interrupt on RTS# low→high.
- Auto CTS (p16): EFR[7]=1; TX stops after current char's stop bit when CTS#=1, resumes at 0; IER[7] interrupt on
  CTS# low→high.
- Auto Xon/Xoff (p18, Table 4): RX: match per EFR[1:0] → TX halts after current char, ISR[4]; Xon (or any char if
  MCR[5]) resumes. Flow chars not stored. TX: Xoff sent 2 char-times after RX FIFO crosses trigger; Xon sent when FIFO
  falls one trigger level below: trigger 1 → Xoff at 1, Xon at 0; 4 → 4/1; 8 → 8/4; 14 → 14/8.
- Special char (p18): EFR[5], compares LCR word-length bits with XOFF2.
- IrDA (p19 Fig 12): MCR[6]=1; TX idles 0; each 0 bit → 3/16-bit high pulse (fixed, no IRPW); RX idles 0; pulse →
  0 bit (Fig 12) ; RX polarity invert via MCR[2] (when EFR[4]=1). "1/16 clock delay" on decode. No half-duplex option.
- Sleep (p20): enters when ISR[0]=1 (none pending) AND divisor ≠ 0 AND IER[4]=1 AND MSR[3:0]=0 AND RX idle 1; stops
  oscillator (no XTAL2 clock). Wakes on RX 1→0, THR/FIFO load, or change on CTS#/DSR#/CD#/RI#; issues wake-up
  interrupt (ISR=0x01, "will show up in the ISR register only if no other interrupts are enabled"), cleared by ISR
  read; returns to sleep automatically when all interrupts serviced; exit by IER[4]=0.
- **PwrSave** (p20): PwrSave pin = 1 AND device asleep → address, data and control inputs (A2-A0, D7-D0, IOR#, IOW#,
  CS#) internally isolated (ignored); data bus not driven. Wake only by RX start bit or modem input change (not by
  bus write). Returns to Power-Save after MSR read and all interrupts serviced. Leave by IER[4]=0 (which needs a bus
  write — so in practice PwrSave pin to 0 first) "and/or the Power-Save pin is connected to GND". Modem inputs,
  16/68# and Reset are not isolated (p38 Test 2).
- Loopback (p21, Fig 13): TX=1, RTS#/DTR# de-asserted, modem inputs ignored; RTS→CTS, DTR→DSR, OP1(MCR[2])→RI,
  OP2(MCR[3])→CD; auto hardware/software flow control operate in loopback (p21). RX must be 1.
- Reset: RESET (Intel, active high) / RESET# (Motorola, active low), ≥40 ns (p5, p8).

### Timing (AC table p39-40; VCC 2.25–5.5 V; 50 pF; ns; columns 2.5 V / 3.3 V / 5.0 V)
| sym | parameter | 2.5 V | 3.3 V | 5.0 V |
|---|---|---|---|---|
| — | crystal frequency (max) | 18 MHz | 20 MHz | 24 MHz |
| OSC | external clock (max) | 24 MHz | 33 MHz | 50 MHz |
| CLK | ext clock low/high time (min) | 20 | 15 | 10 |
| TAS / TAH | address setup / hold (16) | 5 / 0 | 5 / 0 | 10 / 0 |
| TCS | CS# width (16) | 100 | 50 | 30 |
| TRD | IOR# width | 100 | 50 | 30 |
| TDY | read cycle delay | 100 | 50 | 30 |
| TRDV | data access (max) | 75 | 50 | 25 |
| TDD | data disable (max) | 30 | 20 | 20 |
| TWR | IOW# width | 100 | 50 | 30 |
| TDY | write cycle delay | 100 | 50 | 30 |
| TDS / TDH | data setup / hold (16) | 25 / 3 | 15 / 3 | 12 / 5 |
| TADS / TADH | address setup / hold (68) | 5 / 0 | 5 / 0 | 10 / 0 |
| TRWS | R/W# setup to CS# (68) | 10 | 10 | 10 |
| TRDA | read data access (68, max) | 75 | 50 | 25 |
| TRDH | read data disable (68, max) | 30 | 20 | 20 |
| TWDS / TWDH | write data setup / hold (68) | 25 / 3 | 15 / 3 | 12 / 5 |
| TRWH | CS# de-assert to R/W# de-assert (68) | 10 | 10 | 10 |
| TCSL / TCSD | CS# width / cycle delay (68) | 100 / 100 | 50 / 50 | 30 / 30 |
| TWDO | IOW# to modem output (max) | 150 | 75 | 50 |
| TMOD | modem input to INT (max) | 150 | 75 | 50 |
| TRSI | IOR# to INT reset (max) | 150 | 75 | 50 |
| TSSI | stop to set INT | 1 Bclk | 1 Bclk | 1 Bclk |
| TRRI / TSI / TWRI | (max) | 150 | 75 | 50 |
| TINT | initial INT reset to TX start | 8..24 Bclk | 8..24 Bclk | 8..24 Bclk |
| TRST | reset pulse width (min) | 40 | 40 | 40 |
Max data rate 1 Mbps @2.25–2.5 V, 2 Mbps @3.3 V, 3.125 Mbps @5 V (p1, p7).

### VCC
2.25–5.5 V, industrial only (p1, p38). All inputs 5 V tolerant except XTAL1 (p5, p8): VIH max 5.5 V (p38); abs max any
pin -0.3 to 7 V (p37). inputMax 5.5 V (XTAL1 = VCC). Isleep/Ipwrsv: QFN 6/15/30 µA, TQFP 16-mode 65/90/150 µA (2.5/3.3/5 V).

### Package drawings
- 32-pin QFN 5x5x0.9 mm (p47, mm controlling): D/E 4.90–5.10; exposed pad D2 3.50–3.80 (note: "the actual center
  pad is metallic and the size (D2) is device-dependent with a typical tolerance of 0.3mm"); e 0.50 BSC; b 0.18–0.30;
  L 0.35–0.45; k ≥ 0.20; A 0.80–1.00; A1 0.00–0.05; A3 0.15–0.25. Pin 1 at the corner next to pin 32 (Fig 2). No land
  pattern printed (only the center-pad recommendation on p6: solder-mask-defined opening ≤ the exposed pad, thermal
  vias to GND).
- 48-pin TQFP 7x7x1 mm (p46): D 8.80–9.20, D1 6.90–7.10, e 0.50, B 0.17–0.27, L 0.45–0.75, A 1.00–1.20, A1 0.05–0.15,
  A2 0.95–1.05, C 0.09–0.20. No land pattern.
- (24-QFN 4x4 p48: D 3.90–4.10, D2 2.50–2.80 — no census part after correcting ILTR-F.)

### Obsolete
XR16L580IL-F, XR16L580ILTR-F (DigiKey Obsolete; PDF 2011 lists Active).

### Notes
- Census row XR16L580ILTR-F case "24-VFQFN 4x4" contradicts ordering table (ILTR-F = 32-QFN) and its own description.
- Xon/Xoff registers: Table 5 says Read/Write, Table 6 + §4.16 say write-only. Model as write-only (reads undefined);
  or R/W if the model prefers — flag.
- IrDA decode text (p19) says a light pulse "returns a logic 1 to the data bit stream", while Fig 12 and the 650A
  datasheet show pulse → 0 bit. Use pulse → 0 (IrDA).
- MCR[2] default 0 = "RX input as active-low encoded IrDA data" vs IR RX idles at 0 — polarity naming is confusing;
  default (MCR[2]=0) = standard IrDA (high pulses on RX = 0 bits, per Fig 12).
- Auto-RTS de-assert level for RX trigger 14 not defined ("one trigger level above").
- 16/68# pull-up and PwrSave pull-down exist only on the 48-TQFP (p5); QFN pins must be driven.
- Reset table title "FOR CHANNEL A AND B" and Test 2 "RXA and RXB" are copy-paste (single channel).
