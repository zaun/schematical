# TI group B: TL16C552/552A, TL16C554/554A, TL16C750/750E, TL16C754B/754C

Source PDFs are in `datasheets/controller-uart/`. Page numbers are PDF page numbers; in every one of these PDFs they match the printed page numbers.

## PDF → devices
| PDF | device(s) | census parts |
|---|---|---|
| tl16c552.pdf | TL16C552 | TL16C552FN, TL16C552FNR |
| tl16c552a.pdf | TL16C552A | TL16C552AFN, TL16C552AFNG4 (Obsolete), TL16C552AFNR, TL16C552APN (Obsolete) |
| tl16c554.pdf | TL16C554 (incl. TL16C554I grade) | TL16C554FN, TL16C554FNR, TL16C554IFN, TL16C554IPN, TL16C554PN, TL16C554PNR |
| tl16c554a.pdf | TL16C554A (the PDF also covers TL16CP554A, which is not in the census) | TL16C554AFN, TL16C554AFNR, TL16C554AIFN, TL16C554AIFNR, TL16C554AIPN, TL16C554APN, TL16C554APNR |
| tl16c750.pdf | TL16C750 | TL16C750PM |
| tl16c750e.pdf | TL16C750E | TL16C750EPFBR |
| tl16c754b.pdf | TL16C754B | TL16C754BFN; TL16C754FN (Obsolete; the original TL16C754, see below) |
| tl16c754c.pdf | TL16C754C, sold as TL16CP754C | TL16CP754CPM, TL16CP754CIPM |

## Census parts not fully decodable
- **TL16C754FN** (Obsolete): this is the original TL16C754. Its datasheet (TL16C754.pdf) is not in the repo, and tl16c754b.pdf does not list this part.
  - Package: FN, 68-pin PLCC. The pinout is assumed to match TL16C754B.
  - Temperature grade and any register differences from the B revision: unknown.
- **TL16C552APN** (Obsolete): tl16c552a.pdf prints no temperature range for this part. Details are in the TL16C552A section.
- **Package drawings:** tl16c552.pdf, tl16c552a.pdf and tl16c554.pdf contain no outline drawings. Use FN0068A and PN0080A from tl16c554a.pdf, pages 43–47.
  - tl16c750.pdf has no drawing either. Use PM0064A from tl16c754c.pdf, pages 53–55.

## TL16C552

- Title (as printed): "TL16C552 Dual Asynchronous Communications Element with FIFO" (SLLS102B, Dec 1990, rev. Mar 1996). "Enhanced dual channel version of the popular TL16C550 ACE" + "fully bidirectional parallel data port that fully supports the parallel Centronics-type printer"; "Hardware and software compatible with TL16C452" (p1–2).
- Datasheet: `datasheets/controller-uart/tl16c552.pdf` (35 pp: data sheet pp1–32, package option addendum p33, tube info p34, notice p35). PDF page = printed page number. **No package outline/mechanical drawing is in this PDF.**

### Parts
| census part | package | pins | pinout | grade | supply |
|---|---|---|---|---|---|
| TL16C552FN  | PLCC, TI "FN", tube of 18 | 68 | plcc68 | commercial 0 to 70 °C (addendum p33; rec. op. cond. p5), 8 MHz CLK max | 4.75–5.25 V (5 V ±5 %) |
| TL16C552FNR | PLCC "FN", 250-pc small T&R (R = tape & reel) | 68 | plcc68 | same | same |

Decode: TL16C552 + FN (68-pin PLCC) + R (reel). Only one grade and one package exist for this device (p2: "housed in a 68-pin plastic leaded chip carrier").

### Pinout plcc68 (FN, top view p1; Terminal Functions pp3–5)
Overbars dropped. Active levels in Behavior.
```
1 PEMD input          24 RTS0 output        47 PD6 inout
2 TRI input           25 DTR0 output        48 PD5 inout
3 CS1 input           26 SOUT0 output       49 PD4 inout
4 CLK input           27 GND ground         50 PD3 inout
5 DSR1 input          28 CTS0 input         51 PD2 inout
6 RI1 input           29 DCD0 input         52 PD1 inout
7 GND ground          30 RI0 input          53 PD0 inout
8 DCD1 input          31 DSR0 input         54 GND ground
9 RXRDY0 output       32 CS0 input          55 STB inout (open-drain, ~10k pull-up)
10 SOUT1 output       33 A2 input           56 AFD inout (open-drain, ~10k pull-up)
11 DTR1 output        34 A1 input           57 INIT inout (open-drain, ~10k pull-up)
12 RTS1 output        35 A0 input           58 SLIN inout (open-drain, ~10k pull-up)
13 CTS1 input         36 IOW input          59 INT2 output (3-state)
14 DB0 inout          37 IOR input          60 INT1 output (3-state)
15 DB1 inout          38 CS2 input          61 RXRDY1 output
16 DB2 inout          39 RESET input        62 SIN1 input
17 DB3 inout          40 VDD power          63 ERR input
18 DB4 inout          41 SIN0 input         64 VDD power
19 DB5 inout          42 TXRDY1 output      65 SLCT input
20 DB6 inout          43 ENIRQ input        66 BUSY input
21 DB7 inout          44 BDO output         67 PE input
22 TXRDY0 output      45 INT0 output (3-state) 68 ACK input
23 VDD power          46 PD7 inout
```
(Top-side pin order 9..1,68..61 reconstructed from the Terminal Functions table, which assigns every pin; it agrees with the drawing.) No NC pins, no thermal pad. Power: VDD 23, 40, 64; GND 7, 27, 54 ("all terminals must be tied", p3).
Internal resistors: STB/AFD/INIT/SLIN ~10 kΩ pull-up to VDD (pp3–5, p6 note 2); TRI ~5 kΩ pull-down (p5). RESET has ~400 mV hysteresis (p4).
Mode pins: PEMD (pin 1) selects printer-port compatibility (low) vs extended/bidirectional (high); ENIRQ (pin 43) selects INT2 source (low = AT mode, high = PRINT bit mode). TRI (pin 2, active high, level-sensitive) 3-states every output and I/O pin (board test).

### Host interface (pp3–4, p6–7, Tables 2/13 pp16, 29)
- Intel-style: IOR (active low read), IOW (active low write), A2..A0, three active-low chip selects: CS0 → ACE #1 (serial channel 0 pins *0), CS1 → ACE #2 (channel 1 pins *1), CS2 → parallel printer port. One data bus DB7..DB0 shared.
- No ALE: "The internal address strobe is always active" (p6 note 3) — address/CS are transparent; they must be valid tsu before IOR/IOW falling and held th after rising.
- Read: data driven while IOR low and a CS is low (enable ten ≤60 ns after IOR↓; released tdis ≤60 ns after IOR↑, p7). DB is high-Z otherwise (p3).
- Write: data latched at IOW rising edge (tsu6 = 15 ns before IOW↑, th5 = 15 ns after, p7).
- BDO (pin 44, active high): asserted while either serial channel or the parallel port is being read (IOR low with CS0/CS1/CS2 selected); intended as 74LS245 direction control (p3). tpd1 ≤60 ns IOR↓→BDO↑ / IOR↑→BDO↓ (p7; text on p7 says "IOR high to BDO high or IOR low to BDO low", Figure 4 p10 shows BDO high during IOR low — treat BDO = active-high "read in progress").
- Behaviour when more than one CS is low simultaneously is not specified.

### Registers — each serial channel (TL16C550-compatible; Tables 2, 3 pp16–17)
| DLAB | A2 A1 A0 | name | R/W | reset |
|---|---|---|---|---|
| 0 | 000 | RBR | R | not affected by reset (p26) |
| 0 | 000 | THR | W | not affected |
| 0 | 001 | IER | R/W | 0x00 |
| x | 010 | IIR | R | 0x01 |
| x | 010 | FCR | W | 0x00 |
| x | 011 | LCR | R/W | Table 10 says "all bits cleared" (p26); text p26 says "LCR ... not affected" — conflict, see Notes |
| x | 100 | MCR | R/W | 0x00 |
| x | 101 | LSR | R (write = factory test only, p24) | 0x60 |
| x | 110 | MSR | R | bits 3..0 = 0, bits 7..4 = inverted inputs |
| x | 111 | SCR | R/W | not stated (not in Table 10) |
| 1 | 000 | DLL | R/W | not affected |
| 1 | 001 | DLM | R/W | not affected |

Bits:
- IER: b0 ERBFI received-data-available (+ FIFO timeout) int enable; b1 ETBEI THRE int enable; b2 ERLSI receiver line status int enable; b3 EDSSI modem status int enable; b7..4 always 0 (pp17, 19–20).
- IIR: b0 = 0 if interrupt pending; b3..b1 interrupt ID (table below); b5,b4 = 0; b7,b6 = 1 when FCR0 = 1 (both 0 when FIFOs disabled) (p20).
- FCR: b0 FIFO enable (Tx+Rx); b1 Rx FIFO reset (self-clearing, shift reg not cleared); b2 Tx FIFO reset (same); b3 DMA mode select (RXRDY/TXRDY mode 1 when FCR0=1); b5,b4 reserved; b7,b6 Rx trigger: 00=1, 01=4, 10=8, 11=14 bytes (Table 4 p18). Other FCR bits are only programmed when FCR0 = 1 in the same write ("Programming of other FCR bits is enabled by setting FCR0 = 1"). Changing FCR0 (either direction) clears both FIFOs.
- LCR: b1..0 WLS 00=5,01=6,10=7,11=8 data bits; b2 STB 0 = 1 stop, 1 = 1.5 stop (5-bit) / 2 stop (6–8 bit); b3 PEN; b4 EPS 1 = even, 0 = odd; b5 stick parity (PEN=1: parity bit forced to the opposite of EPS, i.e. EPS=1 → 0, EPS=0 → 1); b6 break (SOUT forced low, transmitter logic unaffected); b7 DLAB (Figure 17 p22, p21).
- MCR: b0 DTR (1 → DTR pin low); b1 RTS (1 → RTS pin low); b2 OUT1 (no pin on this device, internal only — loopback source for RI); b3 OUT2/"Enable external interrupt (INT0 or INT1)" (Table 3 p17; INTx 3-state enable per pin description p4); b4 LOOP; b7..5 always 0 (p27).
- LSR: b0 DR; b1 OE; b2 PE; b3 FE; b4 BI; b5 THRE; b6 TEMT; b7 error in Rx FIFO (always 0 in 450 mode) (Table 9 p24, pp25).
- MSR: b0 ΔCTS; b1 ΔDSR; b2 TERI; b3 ΔDCD; b4 CTS; b5 DSR; b6 RI; b7 DCD (status bits are complements of the pins: bit = 1 when pin low) (Table 11 p28).
- SCR: 8-bit scratch, no effect (p32).

### Registers — parallel port (CS2 low; Tables 12, 13 p29; pp22–24)
A2 is not used for the printer port (Table 13 uses A1,A0 only).
| IOR IOW | A1 A0 | register |
|---|---|---|
| L H | 00 | Read data (LPD) |
| L H | 01 | Read status (LPS) |
| L H | 10 | Read control (LPC) |
| L H | 11 | invalid |
| H L | 00 | Write data |
| H L | 01 | invalid |
| H L | 10 | Write control |
| H L | 11 | invalid |

- Read data: PD7..PD0. Compatibility mode (PEMD low): returns last data written (output latch). Extended mode (PEMD high): returns the output latch when DIR = 0, or the live PD0–PD7 pin levels when DIR = 1 (p22).
- Write data: latched into output register. PEMD low: driven immediately onto PD0–PD7 (always outputs). PEMD high: only driven when DIR = 0; DIR = 1 disables the PD output buffers (p22, Table 6 p23: PEMD L → "PC/AT mode – output"; PEMD H, DIR 0 → "PS/2 mode – output"; PEMD H, DIR 1 → "PS/2 mode – input").
- Read status (LPS, read only, Table 7 p23): b0 = 1, b1 = 1 (reserved, always set); b2 PRINT (active low printer-interrupt status: cleared on the active-to-inactive (rising) edge of ACK — text says "if bit 4 of the control register is set"; set again by a read of the status port; default 1); b3 ERR = level of ERR pin; b4 SLCT = level of SLCT pin; b5 PE = level of PE pin; b6 ACK = level of ACK pin; b7 BSY = complement of BUSY pin ("BSY, active low ... corresponds to BUSY input (active high)"). Defaults of b3..b7 follow the pins.
- Read control / Write control (LPC, R/W, Table 8 p24, Table 12): b0 STB (1 → STB pin asserted low), b1 AFD (1 → AFD pin low), b2 INIT (1 → INIT pin NEGATED/high; 0 → INIT asserted low), b3 SLIN (1 → SLIN pin asserted low; the text on p24 says "SLCT signal is asserted" — typo for SLIN), b4 INT2 EN (1 enables INT2; 0 puts INT2 in 3-state), b5 DIR (used only when PEMD high; 1 = input), b7,b6 reserved, read 0. Read control returns the last value written.
- Open-drain control pins STB/AFD/INIT/SLIN: driven low when asserted, else released (10 kΩ pull-up). They are I/O (p3–5) but no register reads their pin level back (Read control returns the written value).
- Reset value of LPC/LPD not stated in this datasheet.

INT2 / ENIRQ (p3, p4):
- INT2 is an active-high 3-state output, enabled by LPC bit 4 (INT2 EN); high-Z after reset.
- ENIRQ low = "PC/AT mode": INT2 output internally connected to the ACK input.
- ENIRQ high: INT2 internally tied to the PRINT bit of the status register. "INT2 is latched high on rising edge of ACK." (The TL16C552A datasheet clarifies this: INT2 = inverse of PRINT, held until the status register is read — see TL16C552A section.)

### Behaviour
Baud generator (p30): CLK pin (external clock only, no crystal pins; dc to 8 MHz) drives both channels' 16-bit divisors; RCLK = CLK / divisor = 16 × baud; divisor = clock ÷ (baud × 16), 1..65535. Writing either DLL or DLM immediately loads the 16-bit baud counter. No BAUDOUT/RCLK pins (internal). Divisor 0 behaviour not stated. Tables 14–16 (pp30–31) give divisors for 1.8432 MHz, 3.072 MHz and "8.192 MHz" (Table 16 values are actually for 8.000 MHz, e.g. 9600 → 52; 50 baud row printed as 1000, should be 10000 — see Notes).

Receiver (p31): 16× sampling. Idle-high SIN; high-to-low transition starts a counter; at count 7½ of the 16× clock (centre of start bit) start bit is valid if SIN still low, else ignored (noise rejection). Data bit 0 first (LSB first, p3/p16). Receiver checks one stop bit only (p21). Data bits sampled at their centres ("within ±3.125 % of actual centre"; start bit may begin up to one 16× clock before detection). Status: LSR0 set when complete character (incl. parity/stop) transferred to RBR/FIFO; OE if RBR not read before next char transferred (450 mode); PE; FE when stop bit sampled 0. BI when SIN held low longer than a full word time (start + data + parity + stop); only one zero character is loaded into the FIFO on break (p25).
Transmitter: SOUT idle mark (high); held mark when transmitter disabled/RESET/empty/loop (p4). Break (LCR6) forces SOUT low. Recommended break sequence on p21.
LSR set/clear (pp24–25):
- DR: set when char in RBR/FIFO; cleared when RBR/FIFO read empty (FIFO: cleared when FIFO empty).
- OE: cleared by LSR read. FIFO mode: set when FIFO full and the next character is completely received; that character in the shift register is overwritten and not transferred to the FIFO.
- PE/FE/BI: cleared by LSR read; in FIFO mode they are per-character and shown when that character is at the top of the FIFO.
- THRE: set when THR→TSR transfer (450 mode) / when Tx FIFO empty (FIFO mode); cleared by writing THR. Not cleared by LSR read.
- TEMT: set when THR (FIFO) and TSR both empty.
- LSR7: FIFO mode only; set if any PE/FE/BI char is in the FIFO; cleared by LSR read if no subsequent errors in the FIFO.
- Status setting is inhibited during LSR/MSR reads: a condition arising during IOR active is set at the trailing edge of the read; if the bit was set during the read and the same condition recurs, it is cleared at the trailing edge instead of being set again (p29).
FIFOs: 16-byte Tx and 16-byte Rx FIFOs per channel (p1). Rx FIFO entries carry PE/FE/BI.
Receive timeout (pp18–19): with FIFO and IER0 enabled, timeout interrupt (IIR = 0xCC, i.e. bits 3..0 = 1100) when ≥1 char in Rx FIFO, no character received for 4 continuous character times (second stop bit counted if programmed) and no CPU read of the FIFO for 4 character times. Timer reset by a CPU read of the FIFO or by a new received character (when no timeout pending). Timeout cleared + timer reset when CPU reads a character from the Rx FIFO. ("At 300 baud and 12-bit characters, 160 ms max latency".)
Rx trigger interrupt: IIR = 04 when FIFO count ≥ trigger; cleared when count drops below trigger. LSR0 set when char transferred into FIFO, cleared when FIFO empty.
Tx FIFO interrupt (p19): with FCR0 = 1 and IER1 = 1, THRE interrupt (IIR = 02) when Tx FIFO empty; cleared by THR write or IIR read. The Tx FIFO empty indication is delayed by one character time minus the last stop bit time if THRE = 1 and there have not been at least two bytes in the Tx FIFO at the same time since the last THRE = 1. The first transmitter interrupt after changing FCR0 is immediate (if enabled).
FIFO polled mode: FCR0 = 1 with IER bits cleared — no timeout/trigger indications; read LSR for status (p19).
Interrupts (Table 5 p20; priority 1 highest):
| IIR3 IIR2 IIR1 IIR0 | prio | type | source | reset by |
|---|---|---|---|---|
| 0 0 0 1 | – | none | – | – |
| 0 1 1 0 | 1 | Rx line status | OE, PE, FE, BI | LSR read |
| 0 1 0 0 | 2 | Rx data available | data available (450) / trigger level reached (FIFO) | RBR read until FIFO below trigger |
| 1 1 0 0 | 2 | char timeout (FIFO only) | see above | RBR read |
| 0 0 1 0 | 3 | THRE | THRE | IIR read (if THRE is the source) or THR write |
| 0 0 0 0 | 4 | modem status | ΔCTS, ΔDSR, TERI, ΔDCD | MSR read |
- INT0/INT1 pins: active high, 3-state; enabled by MCR3 (OUT2), high-Z when MCR3 = 0 and after reset (p4). Disabling all IER sources inhibits IIR and the INT output; LSR/MSR still update (p19).
- Modem status interrupt whenever any of MSR0–MSR3 is set and IER3 = 1 (p28).
Modem lines (pp27–29): DTR/RTS pins = NOT MCR0/MCR1 (high after reset). MSR4..7 = complements of CTS, DSR, RI, DCD pins. Δ bits set on any change since last MSR read; TERI only on RI low→high transition of the pin ("trailing edge"; RI pin description p4 says "high to low" — text p28 says MSR2 set on RI low-to-high, "High-to-low transitions on RI do not activate TERI"). Reading MSR clears the Δ bits only. DCD has no effect on the receiver (p3). No auto flow control.
Loopback (MCR4 = 1, p27): SOUT forced to mark (high); SIN disconnected; TSR output looped to RSR input; CTS/DSR/DCD/RI pins disconnected; internally CTS←RTS (MCR1), DSR←DTR (MCR0), RI←OUT1 (MCR2), DCD←OUT2 (MCR3) (MSR4..7 reflect MCR1, MCR0, MCR2, MCR3, p28); DTR/RTS output pins forced inactive (high). Modem-status interrupts are generated by writing MCR0–3; external pins are ignored. Interrupt logic fully operational.
DMA pins RXRDY/TXRDY (p4–5; both active low):
- RXRDY mode 0 (FCR0 = 0, or FCR0 = 1 & FCR3 = 0): low when ≥1 char in RBR/Rx FIFO; high when empty.
- RXRDY mode 1 (FCR0 = 1, FCR3 = 1): low when trigger level reached or timeout occurred; high when Rx FIFO/RBR empty.
- TXRDY mode 0: low when no chars in THR/Tx FIFO; goes high after the first character is loaded.
- TXRDY mode 1: low when Tx FIFO has no characters (text) ; high when Tx FIFO completely full (16).
- Only mode 0 in 450 mode (FCR0 = 0).
Reset (RESET active low, ≥1 µs, pp4, 26, Table 10):
- IER = 0; IIR = 0x01; LCR "all bits cleared" (Table 10; but text says LCR not affected); MCR = 0; FCR = 0; LSR = 0x60; MSR bits 0–3 = 0, 4–7 follow inputs; RBR, THR, DLL, DLM not affected (text p26). Clears all pending interrupts; initializes Tx/Rx clock counters.
- Outputs: SOUT high; RTS high; DTR high; INT0/INT1 high-Z (MCR3 = 0); INT2 high-Z (p4); OUT1/OUT2 internal = "high"; RXRDY/TXRDY not stated (follow mode-0 rules: RXRDY high, TXRDY low after FIFOs empty).
- After reset THRE = 1, so enabling IER1 immediately produces a THRE interrupt (p26).
- Printer-port register reset values not stated.
TRI: all outputs and I/O pins (incl. DB, PD, INTx, BDO, SOUTx, ...) high-Z while TRI high (p5).

### Timing (pp6–8; 4.75–5.25 V, 0–70 °C; all min/max as printed, no typicals)
Clock: CLK ≤ 8 MHz; tw1 CLK high ≥55 ns; tw2 CLK low ≥55 ns; tw3 RESET low ≥1000 ns (p6).
Read (p6): tw4 IOR low ≥80 ns; tsu1 CS before IOR↓ ≥15; tsu2 A before IOR↓ ≥15; th1 A after IOR↑ ≥20; th2 CS after IOR↑ ≥20; td1 cycle (tsu2+tw4+td2) ≥175; td2 IOR↑ to next IOR/IOW↓ ≥80 ns; in FIFO mode ≥425 ns between Rx FIFO read and IIR/LSR read (note 4).
Write (p7): tw5 IOW low ≥80; tsu4 CS ≥15; tsu5 A ≥15; tsu6 data before IOW↑ ≥15; th3 A ≥20; th4 CS ≥20; th5 data after IOW↑ ≥15; td3 ≥175; td4 ≥80 ns.
Read switching (p7, CL 100 pF): tpd1 IOR→BDO ≤60 ns; ten IOR↓→data valid ≤60 ns; tdis IOR↑→data released 0..60 ns.
Transmitter (p7): td5 THRE int↓ → SOUT start↓ 8–24 RCLK; td6 SOUT start↓ → THRE int↑ 8–8 RCLK (lengthened by 1 char − last stop bit when the Tx interrupt delay is active, note 6); td7 IOW(THR)↑ → THRE int↑ 16–32 RCLK; td8 SOUT start↓ → TXRDY↓ ≤8 RCLK; tpd2 IOW(THR)↓ → THRE int↓ ≤140 ns; tpd3 IOR(IIR)↑ → THRE int↓ ≤140 ns; tpd4 IOW(THR)↑ → TXRDY↑ ≤195 ns. (RCLK = 16× clock.)
Receiver (p8): td9 stop → INT↑ ≤1 RCLK (FIFO mode: data-available/OE/trigger interrupts and RXRDY delayed 3 RCLK; PE/FE/BI for first byte delayed 3 RCLK, immediate for later bytes; timeout / trigger-change interrupt 8 RCLK — note 7 and Fig 12/13 notes pp13–14); tpd5 RCLK↑ → sample clock ≤100 ns; tpd6 IOR(RBR/LSR)↑ → int↓ ≤150 ns; tpd7 IOR(RBR)↓ → RXRDY↑ ≤150 ns.
Modem (p8): tpd8 IOW(MCR)↑ → RTS/DTR ≤100 ns; tpd9 CTS/DSR change → INT↑ ≤170 ns; tpd10 IOR(MSR)↑ → INT↓ ≤140 ns; tpd11 RI↑ → INT↑ ≤170 ns.
Parallel port (p8; host-software-generated handshake): tsu7 data before STB↓ ≥1 µs; th6 data after STB↑ ≥1 µs; tw6 STB low 1–500 µs; BUSY/ACK timings "defined by printer". The device does not generate STB timing itself — STB follows LPC bit 0.

### VCC
4.75–5.25 V (rec. op. p5). VIH 2 V..VDD, VIL ≤0.8 V (TTL levels). Abs-max VI −0.5..7 V (p5) — inputs tolerate up to 7 V abs max, but rec. VIH max = VDD: model inputMax = VDD (5.25 V).

### Package drawings
None printed in tl16c552.pdf. Addendum (p33) gives only "PLCC (FN) | 68". DigiKey census: 68-PLCC (24.23 × 24.23 mm body). Standard TI FN 68-pin PLCC: 1.27 mm pitch, 17 leads/side, J-lead. A later agent must take the FN68 outline from another TI PDF (e.g. tl16c554.pdf / tl16c754b.pdf in this group, if they contain the FN mechanical page).

### Obsolete
None (both census rows Active).

### Notes
- MCR3 conflict: Table 3 (p17) and pin text (p4) say MCR3 enables the external INT0/INT1 3-state output; Figure 18 (p27) says "OUT2 output low". There is no OUT2 pin; treat MCR3 = internal OUT2 = INTx output enable (as TL16C552A Fig. 20 states).
- LCR reset: Table 10 (p26) "All bits cleared" vs text (p26) "The LCR ... not affected". TL16C552A has the same conflict.
- Table 16 (p31) is titled "8.192-MHz Crystal" but the divisors are for 8 MHz; 50-baud divisor printed "1000" (552A prints 10000).
- RI pin description (p4) says TERI on high-to-low; MSR2 text (p28) says low-to-high. Use MSR text (TERI = RI pin rising edge, i.e. RI deasserting; consistent with 16550).
- LPS bit 2 / ACK edge: "cleared on the active to inactive transition of ACK" = ACK rising edge.
- LSR note "LSR is intended only for factory test" (Table 9 footnote p24) — writes to LSR are factory test.
- tpd6 description (p8) "IOR high to reset interrupt low"; TL16C552A prints IOR↓. 
- td6 max printed 8 RCLK here, 9 on TL16C552A.

## TL16C552A

- Title: "TL16C552A, TL16C552AM Dual Asynchronous Communications Element with FIFO" (SLLS189D, Nov 1994, rev. Jan 1999). "Enhanced dual-channel version of the popular TL16C550B ACE"; "Up to 16-MHz clock rate for up to 1-Mbaud operation" (p1–2).
- Datasheet: `datasheets/controller-uart/tl16c552a.pdf` (39 pp: data sheet pp1–35, addendum pp36–37, tube info p38, notice p39). PDF page = printed page. **No package outline drawings in this PDF.**
- Same register set, FIFO size (16), printer port and pin functions as TL16C552 — one functional design with the TL16C552 for modelling purposes, differing in speed/temperature grade, package options, and the clarified PS/2 INT2 behaviour and LPC readback below. Listed separately because it is a separate datasheet/revision ("A").

### Parts
| census part | package | pins | pinout | grade | supply | status |
|---|---|---|---|---|---|---|
| TL16C552AFN   | PLCC "FN", tube 18 | 68 | plcc68 (same pin map as TL16C552) | −40 to 85 °C (addendum p36: "-40 to 85"; rec. op. p7 lists "I suffix −40…85"), 16 MHz CLK | 4.75–5.25 V | Active |
| TL16C552AFNG4 | PLCC "FN", tube 18 (G4 = RoHS/green NiPdAu finish variant) | 68 | plcc68 | same | same | **Obsolete** (census); TI addendum still lists Active |
| TL16C552AFNR  | PLCC "FN", 250 small T&R | 68 | plcc68 | same | same | Active |
| TL16C552APN   | "80-pin TQFP (PN)" per p2 (DigiKey: 80-LQFP 12 × 12 mm) | 80 | lqfp80 | not in TI addendum (dropped); rec. op. temp for the commercial A part: datasheet only gives I (−40…85) and M (−55…125) ranges | 4.75–5.25 V | **Obsolete** (census) |

Decode: TL16C552A + FN (68 PLCC) / PN (80-pin quad flat pack) + R (reel) / G4 (lead-free finish). The "I" (TL16C552AIFN, not in census) and "M"/HV (ceramic CFP, military, not in census) are other grades. Note the addendum rates the non-I TL16C552AFN at −40 to 85 °C too.

### Pinouts
- plcc68: identical to TL16C552 (p1 drawing, FN column of Terminal Functions pp4–6).
- lqfp80 (PN, top view p2, verified against page image `/tmp/p11/img/ti-b-552-a02-02.png` and PN column pp4–6):
```
1 NC            21 NC            41 NC            61 NC
2 NC            22 NC            42 NC            62 NC
3 RXRDY1 output 23 SOUT1 output  43 GND ground    63 BDO output
4 SIN1 input    24 DTR1 output   44 CTS0 input    64 INT0 output
5 ERR input     25 RTS1 output   45 DCD0 input    65 PD7 inout
6 VDD power     26 CTS1 input    46 RI0 input     66 PD6 inout
7 SLCT input    27 DB0 inout     47 DSR0 input    67 PD5 inout
8 BUSY input    28 DB1 inout     48 CS0 input     68 PD4 inout
9 PE input      29 DB2 inout     49 A2 input      69 PD3 inout
10 ACK input    30 DB3 inout     50 A1 input      70 PD2 inout
11 PEMD input   31 DB4 inout     51 A0 input      71 PD1 inout
12 TRI input    32 DB5 inout     52 IOW input     72 PD0 inout
13 CS1 input    33 DB6 inout     53 IOR input     73 GND ground
14 CLK input    34 DB7 inout     54 CS2 input     74 STB inout (open-drain)
15 DSR1 input   35 TXRDY0 output 55 RESET input   75 AFD inout (open-drain)
16 RI1 input    36 VDD power     56 VDD power     76 INIT inout (open-drain)
17 GND ground   37 RTS0 output   57 SIN0 input    77 SLIN inout (open-drain)
18 DCD1 input   38 DTR0 output   58 TXRDY1 output 78 INT2 output
19 RXRDY0 output 39 SOUT0 output 59 ENIRQ input   79 INT1 output
20 NC           40 NC            60 NC            80 NC
```
12 NC pins (1, 2, 20, 21, 22, 40, 41, 42, 60, 61, 62, 80). No exposed pad.

### Differences from TL16C552 (everything else identical — host interface, register map/bits, FIFO, interrupts, DMA pins, loopback, reset)
- Clock: fclock ≤ 16 MHz (rec. op. p7; Figure 1 p10 "16 MHz MAX"); tw1/tw2 CLK high/low ≥31 ns (p7). Table 17 (p34) adds divisors for 16 MHz, up to 1,000,000 baud (divisor 1). (The BRG text p32 still says "dc to 8 MHz" — leftover; use 16 MHz.)
- Temperature: −40 to 85 °C for FN parts (addendum p36); M suffix −55 to 125 °C (HV, not in census).
- MCR (p29–30, Figure 20): b2 OUT1 "has no effect on operation" (internal; still feeds MSR6 in loopback); b3 OUT2 (internal) "0 = external interrupt disabled, 1 = enabled" — removes the TL16C552 Figure 18 conflict.
- Parallel-port Read control bit 5 reads **PEMD • DIR** (logical AND of PEMD pin and DIR bit) (Table 12 p32); Write control bit 5 = DIR. LPC b7,b6 reserved, always cleared (p26).
- ENIRQ/INT2 (p4, Figures 16–17 p17):
  - ENIRQ low (AT mode): INT2 is internally connected to ACK — INT2 follows the ACK pin level (INT2 falls td13 ≤22 ns after ACK↓, rises td14 ≤20 ns after ACK↑). PRINT (LPS2) still goes low at ACK↑ and returns high after IOR↑ of an LPS read (td(int), no value given).
  - ENIRQ high (PS/2 mode): INT2 = inverse of PRINT: latched high on ACK rising edge (td15 ≤24 ns after ACK↑), held until the status register is read; the read clears PRINT (sets LPS2 = 1) and INT2 falls td16 ≤25 ns after IOR↑ (RD_LPS).
  - In both modes INT2 is only driven when LPC4 (INT2 EN) = 1; otherwise high-Z (p26).
  - td13–td16 measured at 15 pF (p10 note 17).
- Parallel-port timing (p10): tsu7 ≥1 µs, th6 ≥1 µs, tw6 STB low ≥1 µs (no 500 µs max given).
- Transmitter td6 SOUT start → THRE int↑: 8–9 RCLK (p9). Transmitter tpd numbering shifted (tpd2 IOW(THR)↓→THRE↓ ≤140 ns; tpd4 IOR(IIR)↑→THRE↓ ≤140 ns; tpd5 IOW(THR)↑→TXRDY↑ ≤195 ns), receiver tpd7 IOR(RBR/LSR)↓ → int reset ≤150 ns; tpd8 IOR(RBR)↓ → RXRDY↑ ≤150 ns; modem tpd9 IOW(MCR)↑ → RTS/DTR ≤100 ns, tpd10 CTS/DSR → INT↑ ≤170, tpd11 IOR(MSR)↑ → INT↓ ≤140, tpd12 RI↑ → INT↑ ≤170 ns (pp9). Read/write cycle timings identical to TL16C552 (tw4/tw5 80 ns, setups 15 ns, holds 20/15 ns, td1/td3 175 ns, td2/td4 80 ns, 425 ns FIFO note; ten/tdis/tpd1 ≤60 ns) (p8). All "not production tested".
- Reset table (Table 10 p29): interrupts shown "Low" (vs "Cleared"); MCR "all bits cleared (5–7 permanently)". Same LCR text/table conflict as TL16C552 (text p28: LCR not affected; table: cleared).
- Electrical: VIL min 0 V; IOH −4 mA most outputs, PD0–7 ±12 mA, open-drain printer controls IOL 12 mA (p7) — analog, out of scope.
- STB description (p6) omits "open-drain" but note 2 (p7) lists INIT, AFD, STB, SLIN as open-drain with ~10 kΩ pull-ups.
- Table 16 (p34) correctly titled "8-MHz Clock" with 50 baud = 10000.

### VCC
4.75–5.25 V (p7). TTL input levels (VIH 2 V). Abs-max VI −0.5..7 V. inputMax = VDD.

### Package drawings
None in the PDF. FN 68-pin PLCC (24.23 mm body, 1.27 mm pitch per census/JEDEC). PN 80-pin: datasheet calls it "80-pin TQFP (PN)"; DigiKey lists 80-LQFP 12 × 12 mm (TI PN = 12 × 12 mm body, 0.5 mm pitch, 14 mm tip-to-tip — standard TI PN outline, **not printed here**; get from another TI PDF containing the PN/S-PQFP-G80 mechanical page, e.g. tl16c554.pdf if present).

### Obsolete
TL16C552AFNG4 and TL16C552APN (census status Obsolete).

### Notes
- The FN parts without "I" are rated −40…85 °C in the current addendum; the datasheet's rec. op. table only lists I and M suffix ranges.
- 552A PN package is not in the current TI addendum (discontinued).
- PE pin (paper empty, active high) vs LSR "PE" (parity error) — name clash only.

## TL16C554

- Title (p1): "TL16C554, TL16C554I Asynchronous Communications Element" — quad ACE, "Consists of Four Improved TL16C550 ACEs Plus Steering Logic" (p1; description says "enhanced quadruple versions of the TL16C550B", p1). 16-byte TX and RX FIFOs per channel (RX FIFO holds 3 error bits per byte), TL16C450 (non-FIFO) mode, DMA mode 0/1 signalling via shared RXRDY/TXRDY.
- Datasheet: `datasheets/controller-uart/tl16c554.pdf` (SLLS165G, Mar 2006), pp 1–30 data sheet, p31 package option addendum, p33–34 packing. **This PDF contains no package outline / land-pattern drawings** — use the FN0068A and PN0080A drawings in `tl16c554a.pdf` pp 43–47 (same TI package codes FN / PN).

### Parts (census, ordering decode from p31 addendum)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| TL16C554FN | PLCC (FN), tube 18 | 68 | plcc68 | 0 to 70 °C (commercial) | 4.75–5.25 V |
| TL16C554FNR | PLCC (FN), small T&R 250 | 68 | plcc68 | 0 to 70 °C | 4.75–5.25 V |
| TL16C554IFN | PLCC (FN), tube 18 | 68 | plcc68 | −40 to 85 °C (I) | 4.75–5.25 V |
| TL16C554IPN | LQFP (PN), tray 119 | 80 | lqfp80 | −40 to 85 °C (I) | 4.75–5.25 V |
| TL16C554PN | LQFP (PN), tray 119 | 80 | lqfp80 | 0 to 70 °C | 4.75–5.25 V |
| TL16C554PNR | LQFP (PN), large T&R 1000 | 80 | lqfp80 | 0 to 70 °C | 4.75–5.25 V |

Suffix rule: `I` before package = industrial (−40..85 °C), no letter = 0..70 °C (p5, p6, p31); `FN` = 68-PLCC, `PN` = 80-LQFP; trailing `R` = tape & reel only (no electrical difference). One speed grade (16 MHz max clock, p6). All census parts decoded.

### Pinout plcc68 (FN, p2 drawing, cross-checked with Terminal Functions pp4–5)
Overbars in the drawing (active low): DSRx, CTSx, DTRx, RTSx, CSx, DCDx, RIx, IOR, IOW, RXRDY, TXRDY, INTN (INTN is drawn with an overbar, but see Behavior). INTx and RESET are active high (no overbar).
```
1 D3 inout      2 D4 inout      3 D5 inout      4 D6 inout      5 D7 inout
6 GND ground    7 RXA input     8 RIA input     9 DCDA input   10 DSRA input
11 CTSA input  12 DTRA output  13 VCC power    14 RTSA output  15 INTA output
16 CSA input   17 TXA output   18 IOW input    19 TXB output   20 CSB input
21 INTB output 22 RTSB output  23 GND ground   24 DTRB output  25 CTSB input
26 DSRB input  27 DCDB input   28 RIB input    29 RXB input    30 VCC power
31 NC nc       32 A2 input     33 A1 input     34 A0 input     35 XTAL1 input
36 XTAL2 output 37 RESET input 38 RXRDY output 39 TXRDY output 40 GND ground
41 RXC input   42 RIC input    43 DCDC input   44 DSRC input   45 CTSC input
46 DTRC output 47 VCC power    48 RTSC output  49 INTC output  50 CSC input
51 TXC output  52 IOR input    53 TXD output   54 CSD input    55 INTD output
56 RTSD output 57 GND ground   58 DTRD output  59 CTSD input   60 DSRD input
61 DCDD input  62 RID input    63 RXD input    64 VCC power    65 INTN input
66 D0 inout    67 D1 inout     68 D2 inout
```
(D7–D0 = FN 66–68, 1–5, i.e. D0=66 … D7=5, p4.) No thermal pad.

### Pinout lqfp80 (PN, p3 drawing, cross-checked pp4–5)
```
1 NC nc        2 DCDD input    3 RID input     4 RXD input     5 VCC power
6 INTN input   7 D0 inout      8 D1 inout      9 D2 inout     10 NC nc
11 D3 inout   12 D4 inout     13 D5 inout     14 D6 inout     15 D7 inout
16 GND ground 17 RXA input    18 RIA input    19 DCDA input   20 NC nc
21 NC nc      22 DSRA input   23 CTSA input   24 DTRA output  25 VCC power
26 RTSA output 27 INTA output 28 CSA input    29 TXA output   30 NC nc
31 IOW input  32 TXB output   33 CSB input    34 INTB output  35 RTSB output
36 GND ground 37 DTRB output  38 CTSB input   39 DSRB input   40 NC nc
41 NC nc      42 DCDB input   43 RIB input    44 RXB input    45 VCC power
46 A2 input   47 A1 input     48 A0 input     49 NC nc        50 XTAL1 input
51 XTAL2 output 52 NC nc      53 RESET input  54 RXRDY output 55 TXRDY output
56 GND ground 57 RXC input    58 RIC input    59 DCDC input   60 NC nc
61 NC nc      62 DSRC input   63 CTSC input   64 DTRC output  65 VCC power
66 RTSC output 67 INTC output 68 CSC input    69 TXC output   70 IOR input
71 NC nc      72 TXD output   73 CSD input    74 INTD output  75 RTSD output
76 GND ground 77 DTRD output  78 CTSD input   79 DSRD input   80 NC nc
```
No thermal pad. INTx outputs are 3-state (see interrupts).

### Host interface
- Intel-style: shared D7–D0 (3-state), A2–A0, one active-low chip select per channel (CSA..CSD; CSx selects channel x, pp4, 15), common active-low IOR (read) and IOW (write) (p5). No ALE/ADS, no DDIS, no Motorola mode. "The internal address strobe is always active" (p7 note 2): address/CS are transparent, sampled around the strobe.
- Read: data driven while IOR low and CSx low; ten IOR↓→D valid 30 ns max, tdis IOR↑→D released 0–20 ns (p7). Setup CSx→IOR↓ 10 ns, A→IOR↓ 15 ns, hold A/CS after IOR↑ 0 ns, IOR low pulse 75 ns min, IOR↑ to next IOR/IOW↓ 50 ns, read cycle td1 140 ns min; in FIFO mode 425 ns min between reads of RX FIFO and status registers (IIR, LSR) (p7 note 3).
- Write: data latched on IOW↑ (tsu5 D before IOW↑ 10 ns, th5 D after IOW↑ 25 ns); IOW pulse 50 ns min; CSx/A setup before IOW↓ 10/15 ns, hold after IOW↑ 5/5 ns; IOW↑ to next strobe 55 ns; cycle td3 120 ns (p7).
- Footnote p15 "The serial channel is accessed when either CSA or CSD is low" — a typo; each CSx enables its own channel (p4). Behaviour with more than one CSx low simultaneously is not stated.

### Registers (per channel; p15 Table 2, p16 Table 3)
| DLAB | A2A1A0 | Read | Write | Reset |
|---|---|---|---|---|
| 0 | 000 | RBR | THR | not affected (p29) |
| 0 | 001 | IER | IER | 0x00 |
| X | 010 | IIR | FCR | IIR 0x01; FCR 0x00 |
| X | 011 | LCR | LCR | 0x00 per Table 12 (but p29 text says LCR not affected — see Notes) |
| X | 100 | MCR | MCR | 0x00 |
| X | 101 | LSR | (factory test only) | 0x60 |
| X | 110 | MSR | (do not write) | bits 3–0 = 0, bits 7–4 = inputs |
| X | 111 | SCR | SCR | not stated |
| 1 | 000 | DLL | DLL | not affected (p29) |
| 1 | 001 | DLM | DLM | not affected (p29) |

Bits:
- RBR/THR: data bits 7–0; fewer than 8 bits right-justified; bit 0 first on the wire (p15).
- IER (p16, p19): b0 ERBI (RX data available + timeout in FIFO mode), b1 ETBEI (THRE), b2 ERLSI (RX line status), b3 EDSSI (modem status), b7–4 always 0.
- IIR (p16, pp19–20): b0 = 0 when interrupt pending; b3–1 ID (table below); b5–4 = 0; b7–6 = 1,1 when FCR0 = 1 (0 otherwise). b3 always 0 in TL16C450 mode.
- FCR (write only, p17): b0 FIFO enable (TX+RX; changing b0 clears both FIFOs; other bits only programmed while b0 = 1 is being set); b1 RX FIFO reset (clears RX FIFO bytes and counter; shift register not cleared; self-clearing not stated); b2 TX FIFO reset (shift reg not cleared); b3 DMA mode select (RXRDY/TXRDY mode 0→1, only if FCR0 = 1); b5–4 reserved; b7–6 RX trigger: 00 = 1, 01 = 4, 10 = 8, 11 = 14 bytes (Table 4).
- LCR (p20–21 Fig 15): b1–0 WLS 00=5, 01=6, 10=7, 11=8 bits; b2 STB 0 = 1 stop, 1 = 1.5 stop (5-bit) / 2 stop (6–8 bit) — receiver checks only first stop bit; b3 PEN; b4 EPS (1 = even, 0 = odd); b5 stick parity (with PEN: parity bit sent/checked as the opposite of LCR4, i.e. EPS=1→0, EPS=0→1); b6 break (TX forced low; transmitter logic unaffected); b7 DLAB.
- MCR (p23 Fig 16): b0 DTR (1 → DTR pin low); b1 RTS (1 → RTS pin low); b2 OUT1 — no external effect (internal only; used in loopback); b3 OUT2 — 1 = channel's INTx output enabled (see INTN); b4 LOOP; b7–5 always 0.
- LSR (pp21–22, Table 6): b0 DR; b1 OE; b2 PE; b3 FE; b4 BI; b5 THRE; b6 TEMT; b7 RX FIFO error (always 0 in 450 mode). Details under Behavior.
- MSR (p24, Table 7): b0 ΔCTS, b1 ΔDSR, b2 TERI (RI pin low→high), b3 ΔDCD, b4 CTS (= NOT CTS pin), b5 DSR (= NOT DSR pin), b6 RI (= NOT RI pin), b7 DCD (= NOT DCD pin). Deltas cleared on MSR read.
- SCR: 8-bit R/W, no effect (p30).
- DLL/DLM: 16-bit divisor low/high byte (p16).

### Behavior
- **Baud generator** (p25): XTAL1 clock (crystal on XTAL1/XTAL2 or external clock into XTAL1; XTAL2 = oscillator output) ÷ divisor N (1 … 65535) = RCLK = 16 × baud. Writing either DLL or DLM immediately reloads the 16-bit baud counter. 16× sampling only. fclock max 16 MHz (p6; text p25 says "dc to 8 MHz" — table p28 uses 16 MHz, divisor 1 → 1 Mbaud). No BAUDOUT/RCLK pins; RCLK is internal (p8 note 7). Divisor 0 behaviour not stated.
- **Receiver** (p28): hunts for a high→low transition from idle, counts 16× clock to 7½ (centre of start bit); start valid if RX still low there, else ignored (false start). Data bits sampled at bit centres, LSB first; parity (if PEN) then stop. Receiver checks one stop bit only; stop sampled 0 → FE. Start bit may begin up to one 16× cycle before detection. LSR0 set when char transferred to RBR/FIFO.
  - OE (p21): non-FIFO: new char overwrites unread RBR, OE set. FIFO: OE when FIFO full and next char completely received; the char in the shift register is not transferred to the FIFO (it is overwritten in the shift register). OE cleared by LSR read.
  - PE/FE/BI are per-character in FIFO mode and appear in LSR when that char is at the top of the FIFO; cleared on LSR read. BI: RX low longer than a full word (start + data + parity + stop); only one zero character is loaded into the FIFO per break (p22).
  - LSR7: set when any PE/FE/BI char is in the FIFO; cleared on LSR read if no further errors in FIFO (p22).
  - Status set during an IOR is deferred to the trailing edge of the read (p24).
- **Transmitter**: THRE (LSR5) set when THR moves to TSR (non-FIFO) or TX FIFO empty (FIFO), cleared by THR write; TEMT (LSR6) when THR/FIFO and TSR both empty (p22). TX idles high (mark). Break: LCR6 forces TX low; recommended sequence (p20).
- **FIFOs**: 16 bytes each direction; RX FIFO + 3 error bits per byte (p1). FCR0 change clears both FIFOs. Trigger levels 1/4/8/14.
  - RX data available interrupt (IIR=04) when FIFO count ≥ trigger; cleared when it drops below trigger (p17).
  - Character timeout (IIR=0C) when ≥1 char in FIFO, no char received for 4 char times (second stop bit included) and no CPU read for 4 char times (p18). Timer reset on RX FIFO read or new char received; timeout interrupt cleared and timer reset by reading a char from the RX FIFO (p18).
  - TX interrupt in FIFO mode: IIR=02 when TX FIFO empty; cleared by THR write or IIR read (p18). "Transmitter FIFO empty indicators are delayed one character time minus the last stop bit time whenever THRE = 1 and there has not been a minimum of two bytes at the same time in transmit FIFO since the last THRE = 1. The first transmitter interrupt after changing FCR0 is immediate" (p18).
  - FIFO polled mode: FCR0 = 1 and IER bits cleared; no timeout/trigger indication, read LSR (p18).
  - Delays (p8 note 6, p13): RX data available, OE, trigger-level interrupts and RXRDY delayed 3 RCLK cycles in FIFO mode; after first byte PE/FE/BI delayed 3 RCLK; timeout interrupt td9 = 8 RCLK cycles.
- **DMA / RXRDY, TXRDY** (pp29–30): RXRDY mode 0 (FCR3=0): low when RX FIFO (or RBR) not empty, high when empty. Mode 1: low when FIFO reaches trigger level or timeout occurred, high when FIFO empty. TXRDY mode 0: low when TX FIFO empty, high when it holds ≥1 byte. Mode 1: low when TX FIFO not full, high when full. **There is one RXRDY and one TXRDY pin for all four channels: "RXRDY and TXRDY outputs from each of the four internal ACEs of the TL16C554 are ANDed together internally" (p29)** — i.e. the pin is low if any channel's (active-low) signal is low. Pin-table text p5 ("RXRDY goes low when the receive FIFO is full", "TXRDY goes low when the transmit FIFO is full") contradicts pp29–30 — use pp29–30.
- **Interrupts** (p19 Table 5):

| IIR3..0 | Prio | Type | Source | Cleared by |
|---|---|---|---|---|
| 0001 | – | none | – | – |
| 0110 | 1 | RX line status | OE, PE, FE, BI | LSR read |
| 0100 | 2 | RX data available | data available / trigger reached | RBR read (until below trigger) |
| 1100 | 2 | char timeout | see FIFOs | RBR read |
| 0010 | 3 | THRE | THRE | IIR read (if THRE is the source) or THR write |
| 0000 | 4 | modem status | ΔCTS, ΔDSR, TERI, ΔDCD | MSR read |

  - INTx is active high (p5). Output enable per channel: **INTN pin** (p4): INTN low or floating → INTx enabled only when that channel's MCR3 (OUT2) = 1, otherwise INTx is high-impedance; INTN high → INTx always enabled, overriding OUT2. (The pin table says "bit 3 of the modem status register" — means MCR bit 3, OUT2. "Allowed to float" implies an internal pull-down, not stated.) On reset INT goes low (Table 12).
  - THRE interrupt fires after reset once IER1 is set because THRE = 1 (p29).
- **Modem lines**: RTS/DTR pins = NOT MCR1/MCR0, high after reset (p23, p29). CTS/DSR/DCD/RI inputs only status/interrupts; CTS has no effect on TX (p4) — no auto flow control on TL16C554.
- **Loopback** (MCR4, p23–24): TX pin held mark (high), RX pin disconnected, TSR output looped to RSR input; modem inputs disconnected; internally DTR→DSR, RTS→CTS, OUT1→RI, OUT2→DCD (MSR5=DTR, MSR4=RTS, MSR6=OUT1, MSR7=OUT2); modem output pins forced inactive (high). Modem-status interrupts generated by writing MCR3–0. Interrupts fully operational.
- **Reset** (p29, Table 12): RESET active high, ≥1000 ns (tw3, p6). Clears: IER = 0, IIR = 01, LCR = 0 (Table 12), MCR = 0, FCR = 0, LSR = 0x60, MSR deltas = 0; TXx high; RTS, DTR high; all interrupts low; TX/RX clock counters initialised. Not affected: divisor latches, RBR, THR (and LCR per p29 text — conflict). Device idle until programmed.
- No sleep, IrDA, RS-485, 9-bit, GPIO, device ID, software reset.

### Timing (5 V, CL = 100 pF, max unless noted)
- Clock: tw1/tw2 clock high/low ≥ 31 ns, fclock ≤ 16 MHz (p6, p9).
- Bus: above (p7).
- Transmitter (p8): td5 INTx↓ → TX start 8–24 RCLK; td6 TX start → INTx↑ 8 RCLK; td7 WR THR → INTx↑ 16–32 RCLK (both lengthened by one char time − last stop bit if TX interrupt delay active, note 5); td8 TX start → TXRDY↓ 8 RCLK; tpd1 IOW(WR THR)↓ → INTx↓ 35 ns; tpd2 IOR(RD IIR)↑ → INTx↓ 30 ns; tpd3 IOW(WR THR)↑ → TXRDY↑ 50 ns.
- Receiver (p8): td9 stop bit → INTx↑ / RXRDY↓ 1 RCLK (3 RCLK in FIFO mode, 8 for timeout, p13 notes); tpd4 read RBR/LSR → INTx↓ 40 ns; tpd5 IOR↓ → RXRDY↑ 30 ns.
- Modem (p8): tpd6 IOW(WR MCR)↑ → RTS/DTR 50 ns; tpd7 CTS/DSR/DCD edge → INTx↑ 30 ns; tpd8 IOR(RD MSR)↑ → INT↓ 35 ns; tpd9 RI↑ → INTx↑ 30 ns.

### VCC
4.75–5.25 V (nom 5) both grades (p6). VIH 2 V..VCC, VIL 0.8 V; abs-max VI −0.5..7 V (p5). TTL levels; inputMax = VCC (5 V part).

### Package drawings (in tl16c554a.pdf)
- FN 68-PLCC: TI **FN0068A** outline tl16c554a.pdf p46, land p47. Pitch 0.050 in (1.27 mm); body .950–.958 in (24.13–24.33 mm); span over leads .985–.995 in (25.02–25.27 mm); lead width .013–.021 in (0.331–0.533 mm) (upper .026–.032); max height .180 in (4.57 mm); land: 68× pad (.094 × .026 in) [2.39 × 0.65 mm], pad-row centre span (.93 in) [23.6 mm], 17 pads/side. JEDEC MS-018.
- PN 80-LQFP: TI **PN0080A** outline tl16c554a.pdf p43, land p44. Pitch 0.5 mm; body 11.8–12.2 mm; span 13.8–14.2 mm; lead width 0.17–0.27 mm; foot length 0.45–0.75 mm; height 1.6 max; land: 80× pad 1.5 × 0.3 mm, pad centre span (13.4) mm, 20 pads/side. JEDEC MS-026.

### Obsolete
None (all six census rows Active).

### Notes
- p15 footnote "either CSA or CSD" — typo.
- INTN pin table refers to "bit 3 of the modem status register"; means MCR3 (OUT2).
- RXRDY/TXRDY pin descriptions (p5) contradict the operation sections (pp29–30).
- LSR4 text says "LSR2 reflects the BI" — means LSR4 (p22).
- Table 12 says LCR "All bits cleared" on reset; p29 text says LCR not affected. TI TL16C550-family convention is LCR cleared — planner decision.
- MSR text (p24) "IIR3 is set" means IER3. RXRDY/TXRDY behaviour in non-FIFO (450) mode is not described here (refers to the TL16C550B data sheet, p4).
- BRG text "dc to 8 MHz" vs 16 MHz rec. op. and the 16-MHz divisor table.
- p1 calls the PN package "TQFP"; addendum calls it LQFP; DigiKey 80-LQFP 12×12.

## TL16C554A

- Title (p1): "TL16C554A, TL16C554AI Asynchronous-Communications Element" — "Consists of Four Improved TL16C550C ACEs"; adds programmable **auto-RTS / auto-CTS** and 3.3 V operation. Otherwise the same register set and behaviour as **TL16C554** (above).
- Datasheet: `datasheets/controller-uart/tl16c554a.pdf` (SLLS509E, Jun 2010), pp 1–36 data sheet, p37–38 revision history (p38 is an unrelated OPA2832 history, stray), p39 addendum, p43–51 package drawings (PN0080A p43–45, FN0068A p46–48, PM0064A p49–).
- Also covers TL16CP554A (PM 64-LQFP; addendum p39: TL16CP554APM / TL16CP554AIPM **Obsolete**; not in census).

### Parts (census, decode p39)
| Part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| TL16C554AFN | PLCC (FN), tube | 68 | plcc68 | 0–70 °C | 3.0–3.6 V or 4.75–5.25 V |
| TL16C554AFNR | PLCC (FN), T&R 250 | 68 | plcc68 | 0–70 °C | same |
| TL16C554AIFN | PLCC (FN), tube | 68 | plcc68 | −40–85 °C | same |
| TL16C554AIFNR | PLCC (FN), T&R 250 | 68 | plcc68 | −40–85 °C | same |
| TL16C554AIPN | LQFP (PN), tray | 80 | lqfp80 | −40–85 °C | same |
| TL16C554APN | LQFP (PN), tray | 80 | lqfp80 | 0–70 °C | same |
| TL16C554APNR | LQFP (PN), T&R 1000 | 80 | lqfp80 | 0–70 °C | same |
All decoded. Single speed grade (16 MHz, both supply ranges, p8–9).

### Pinouts
- plcc68 (FN, p2) and lqfp80 (PN, p4): **identical to TL16C554** (same drawings, Terminal Functions pp6–7). Pin role change: none; RTSx is now also driven by auto-RTS.
- pm64 (PM, p3; obsolete TL16CP554A only, for reference): 1 DSRA, 2 CTSA, 3 DTRA, 4 VCC, 5 RTSA, 6 INTA, 7 CSA, 8 TXA, 9 IOW, 10 TXB, 11 CSB, 12 INTB, 13 RTSB, 14 GND, 15 DTRB, 16 CTSB, 17 DSRB, 18 DCDB, 19 RIB, 20 RXB, 21 VCC, 22 A2, 23 A1, 24 A0, 25 XTAL1, 26 XTAL2, 27 RESET, 28 GND, 29 RXC, 30 RIC, 31 DCDC, 32 DSRC, 33 CTSC, 34 DTRC, 35 VCC, 36 RTSC, 37 INTC, 38 CSC, 39 TXC, 40 IOR, 41 TXD, 42 CSD, 43 INTD, 44 RTSD, 45 GND, 46 DTRD, 47 CTSD, 48 DSRD, 49 DCDD, 50 RID, 51 RXD, 52 VCC, 53–60 D0–D7, 61 GND, 62 RXA, 63 RIA, 64 DCDA. **No INTN, RXRDY, TXRDY** on PM (Terminal Functions has no PM numbers for them, pp6–7).

### Differences from TL16C554 (everything else identical, same page structure +4)
1. **MCR5 = AFE** (autoflow enable) (p20 Table 3, p27–28). MCR7–6 = 0. Flow configuration (Table 7, p27): AFE=1,RTS(MCR1)=1 → auto-RTS + auto-CTS; AFE=1,MCR1=0 → auto-CTS only; AFE=0 → both disabled.
2. **Auto-RTS** (p33–34, Figs 22–23): tied to RX FIFO trigger level. Trigger 1/4/8: RTS deasserted (high) when the RX FIFO level reaches the trigger; reasserted (low) once the RX FIFO is emptied by reading RBR. Trigger 14: RTS deasserted after the first data bit of the 16th character is present on RX; reasserted when the RX FIFO has at least one free byte space ("asserted again when there is at least one byte of space available and no incoming byte is in processing or there is more than one byte of space available"; "when the receive FIFO is full, the first RBR read reasserts RTS", Fig 23 notes). Timing (p11): tpd11 RCV threshold byte (midpoint of stop bit) → RTS↑ 2 baudout cycles; tpd12 RD RBR (last byte) → RTS↓ 2; tpd13 first data bit of 16th char → RTS↑ 2; tpd14 RD RBR → RTS↓ 2 baudout cycles. ("baudout" = internal 16× clock, p11 note 7.)
3. **Auto-CTS** (p33–34, Fig 21): transmitter checks CTS before each next byte; CTS low → sends; if CTS goes high before the middle of the last stop bit of the current byte, the current byte finishes but the next is not sent; resumes when CTS goes low. tpd10 CTS low → SOUT↓ (start) 24 baudout cycles max; tsu6 CTS high setup to midpoint of TX stop bit 2 baudout cycles (p11). With auto-CTS enabled, ΔCTS does **not** generate a modem-status interrupt (p6, p28).
4. **RTS pin** (p7): inactive high on reset, in loop mode, or MCR1 = 0; in auto-RTS mode set inactive by receiver threshold logic.
5. **Supply**: adds low-voltage grade VCC 3.0–3.6 V (nom 3.3) with same VIH 2 V / VIL 0.8 V, ICC 40 mA (p9); 5 V grade 4.75–5.25 V (p8). Timing tables (pp9–11) are not split by supply — same numbers as TL16C554. Abs-max VI −0.5..7 V (p7); recommended VIH max = VCC — 5 V tolerance at 3.3 V is not stated (inputMax = VCC to be safe).
6. Character-timeout wording (p22): ≥1 char in FIFO, no new char received for ≥4 char times, FIFO not read for ≥4 char times. IIR3 described correctly as set with bit 2 for character time-out (p24).
7. MSR text lists RI among modem inputs (p28). Loopback mapping stated explicitly (p27): DTR→DSR, RTS→CTS, OUT1→RI, OUT2→DCD.
8. Receiver text (p32) no longer states the 7½ count but describes the same centre sampling.
9. BRG text lists 1.8432, 3.072, 8 and 16 MHz (p29).

### Timing
Same as TL16C554 (pp9–11 identical values: tw4 75, tsu1 10, tsu2 15, th1/th2 0, td1 140 (425 FIFO), td2 50, tw5 50, tsu3 10, tsu4 15, tsu5 10, th3/th4 5, th5 25, td3 120, td4 55, ten 30, tdis 0–20 ns; serial/modem same), plus the autoflow numbers above.

### Package drawings
FN0068A (p46 outline, p47 land) and PN0080A (p43 outline, p44 land) — dims as listed under TL16C554. PM0064A (p49, 10×10 body, 12 mm span, 0.5 mm pitch, 0.17–0.27 lead) is for the obsolete TL16CP554A only.

### Obsolete
No census rows obsolete. (TL16CP554APM/AIPM obsolete per p39, not in census.)

### Notes
- MCR text p28 "Bit 6 − Bit 7: MCR5, MCR6, and MCR7 are permanently cleared" and Table 13 (p35) "MCR … all bits cleared (5−7 permanent)" contradict MCR5 = AFE (p20, p27); AFE is real (Table 7, Fig 19). Treat MCR7–6 as 0, MCR5 R/W, cleared on reset.
- Table 7 header says "MSR BIT 5 / MSR BIT 1" — means MCR.
- Terminal Functions (p7, verified on page image): FN column blank for RIx and RTSx rows (use p2 drawing: RIA 8, RIB 28, RIC 42, RID 62; RTSA 14, RTSB 22, RTSC 48, RTSD 56). PM numbers for RIx listed sorted (19,30,50,63) rather than A–D order (RIA 63, RIB 19, RIC 30, RID 50).
- p1 calls PM "plastic quad flatpack (PQFP)" and the revision history says "48-pin PM package" — PM is 64-pin LQFP.
- p34 says trigger levels "described in Figures 3 and 4" — should be Figures 22 and 23.
- Same INTN / RXRDY-TXRDY / LCR-reset / CSA-or-CSD ambiguities as TL16C554.

## TL16C750

- Title (p1): "TL16C750 Asynchronous Communications Element with 64-Byte FIFOs and Autoflow Control", SLLS191C, Jan 1995, revised Dec 1997.
- Function: a single 16C550C-type ACE. Differences from the 550C: a 16- or 64-byte FIFO (selected by FCR5), auto-RTS/auto-CTS (MCR5 = AFE), sleep mode (IER4) and low-power mode (IER5).
- Datasheet: `datasheets/controller-uart/tl16c750.pdf`. 33 data pages, then the package addendum (pp34–37) and the notice (p38). Printed page numbers equal PDF page numbers.
- **There are no package outline drawings in this PDF.** Use PM0064A from `tl16c754c.pdf` pp53–55 (see Package drawings below).

### Parts
| Census part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| TL16C750PM (Active) | TI PM, 64-pin LQFP 10×10 (the datasheet says "SQFP") | 64 | lqfp64 | Commercial, 0–70 °C (addendum p34: "Op temp 0 to 70"; p1: the I grade exists only for the 64-pin package, but TL16C750PM has no I) | 3.0–3.6 V or 4.75–5.25 V (p8) |

- TL16C750FN/FNR (44-pin PLCC) appear in the addendum but are not in the census.
- I could decode every census part.

### Pinouts
Both pinouts are on p2; the image was checked at /tmp/p11/img/ti-b-750-p-02.png. The terminal table is on pp4–5.

**PM, 64-pin LQFP (lqfp64)**

| Pin | Name | Role | Pin | Name | Role |
|---|---|---|---|---|---|
| 1 | XIN | input | 33 | CTS | input |
| 2 | XOUT | output | 34 | NC | nc |
| 3 | NC | nc | 35 | DSR | input |
| 4 | WR1 | input | 36 | DCD | input |
| 5 | NC | nc | 37 | NC | nc |
| 6 | WR2 | input | 38 | RI | input |
| 7 | NC | nc | 39 | NC | nc |
| 8 | VSS | ground | 40 | VCC | power |
| 9 | RD1 | input | 41 | NC | nc |
| 10 | RD2 | input | 42 | D0 | inout |
| 11 | NC | nc | 43 | D1 | inout |
| 12 | DDIS | output | 44 | NC | nc |
| 13 | TXRDY | output | 45 | D2 | inout |
| 14 | NC | nc | 46 | D3 | inout |
| 15 | ADS | input | 47 | NC | nc |
| 16 | NC | nc | 48 | D4 | inout |
| 17 | A2 | input | 49 | NC | nc |
| 18 | A1 | input | 50 | D5 | inout |
| 19 | NC | nc | 51 | D6 | inout |
| 20 | A0 | input | 52 | D7 | inout |
| 21 | RXRDY | output | 53 | NC | nc |
| 22 | NC | nc | 54 | RCLK | input |
| 23 | INTRPT | output | 55 | SIN | input |
| 24 | NC | nc | 56 | NC | nc |
| 25 | OUT2 | output | 57 | NC | nc |
| 26 | RTS | output | 58 | SOUT | output |
| 27 | NC | nc | 59 | CS0 | input |
| 28 | DTR | output | 60 | NC | nc |
| 29 | NC | nc | 61 | CS1 | input |
| 30 | OUT1 | output | 62 | CS2 | input |
| 31 | NC | nc | 63 | NC | nc |
| 32 | MR | input | 64 | BAUDOUT | output |

- There is no exposed pad.
- Active-low pins (overbar in the drawing): WR1, RD1, TXRDY, ADS, RXRDY, OUT2, RTS, DTR, OUT1, CTS, DSR, DCD, RI, CS2, BAUDOUT.
- Active-high pins: WR2, RD2, CS0, CS1, MR, INTRPT, DDIS.
- XIN can take a crystal (with XOUT) or an external clock (Fig 21, p33).

**FN, 44-pin PLCC (plcc44)**, for reference only; it is not in the census.

| Pins | Signals |
|---|---|
| 1–11 | NC, D0, D1, D2, D3, D4, D5, D6, D7, RCLK, SIN |
| 12–22 | NC, SOUT, CS0, CS1, CS2, BAUDOUT, XIN, XOUT, WR1, WR2, VSS |
| 23–33 | NC, RD1, RD2, DDIS, TXRDY, ADS, A2, A1, A0, RXRDY, INTRPT |
| 34–44 | NC, OUT2, RTS, DTR, OUT1, MR, CTS, DSR, DCD, RI, VCC |

### Host interface (pp4–5, p22)
- **Bus:** Intel-style, with dual-polarity strobes.
  - A read happens when RD1 is low or RD2 is high while the chip is selected. Tie the unused strobe inactive (RD2 low or RD1 high).
  - A write happens when WR1 is low or WR2 is high while selected. Tie the unused strobe inactive.
- **Chip select:** the chip is selected when CS0 = 1, CS1 = 1 and CS2 = 0 (p4).
- **ADS address strobe (p4):**
  - While ADS is low, A0–A2 and CS0–CS2 pass straight through.
  - On the rising edge of ADS they are latched and held while ADS stays high.
  - With no external latch, tie ADS low (Fig 18, p20).
- **Write timing:** data is captured by the trailing edge of the strobe (WR1↑ or WR2↓). Setup tsu3 and hold th5 are measured to that edge (p10, Fig 5).
- **Data bus:** D7–D0 are 3-state and are driven only during a selected read.
- **DDIS** is high whenever the CPU is not reading the chip and low during a selected read. It is meant to disable an external bus transceiver (p4, Fig 6).
- **Register select, Table 1 (p22):**

| DLAB | A2 A1 A0 | Read | Write |
|---|---|---|---|
| 0 | 000 | RBR | THR |
| 0 | 001 | IER | IER |
| X | 010 | IIR | FCR |
| X | 011 | LCR | LCR |
| X | 100 | MCR | MCR |
| X | 101 | LSR | LSR (factory test only) |
| X | 110 | MSR | MSR |
| X | 111 | SCR | SCR |
| 1 | 000 | DLL | DLL |
| 1 | 001 | DLM | DLM |

### Registers (Table 3, p23; bit text pp24–31; reset values from Table 2, p22)

- **RBR** (read, DLAB=0, address 0): receive data. Bit 0 is the first bit on the line. Not changed by MR.
- **THR** (write, DLAB=0, address 0): transmit data. Not changed by MR.
- **IER** (address 1, DLAB=0, read/write). Reset 0x00.

| Bit | Name | Meaning |
|---|---|---|
| 0 | ERBI | received-data-available interrupt enable (also the timeout interrupt) |
| 1 | ETBEI | THRE interrupt enable |
| 2 | ELSI | receiver line status interrupt enable |
| 3 | EDSSI | modem status interrupt enable |
| 4 | — | sleep mode enable |
| 5 | — | low-power mode enable |
| 6–7 | — | always 0 |

- **IIR** (address 2, read). Reset 0x01.

| Bit | Meaning |
|---|---|
| 0 | 0 = interrupt pending |
| 1–2 | interrupt ID |
| 3 | timeout (FIFO mode only; always 0 in 450 mode) |
| 4 | always 0 |
| 5 | 1 = 64-byte FIFO enabled |
| 6–7 | 1 when FIFOs are enabled |

  - IIR[7:5]: 000 = 450 mode, 110 = 16-byte mode, 111 = 64-byte mode (p26).
- **FCR** (address 2, write). Reset 0x00. Description p24.

| Bit | Meaning |
|---|---|
| 0 | FIFO enable. Must be 1 when writing the other FCR bits or they are not programmed. Changing this bit clears both FIFOs. |
| 1 | RX FIFO reset. Self-clearing; the receive shift register (RSR) is not cleared. |
| 2 | TX FIFO reset. Self-clearing; the transmit shift register (TSR) is not cleared. |
| 3 | DMA mode select (0 = mode 0, 1 = mode 1); takes effect only when FCR0 = 1 |
| 4 | reserved |
| 5 | 1 = 64-byte mode, 0 = 16-byte mode. Writable only while LCR7 = 1 ("‡ Access to … FCR bit 5 require LCR bit 7 = 1"). |
| 7:6 | RX trigger level, Table 4 below |

  - Table 4 (p24), RX trigger level:

| FCR7:6 | 16-byte mode | 64-byte mode |
|---|---|---|
| 00 | 1 | 1 |
| 01 | 4 | 16 |
| 10 | 8 | 32 |
| 11 | 14 | 56 |

- **LCR** (address 3, read/write). Reset 0x00 (Table 2).

| Bit | Name | Meaning |
|---|---|---|
| 1:0 | WLS | word length: 00 = 5, 01 = 6, 10 = 7, 11 = 8 bits |
| 2 | STB | 0 = 1 stop bit; 1 = 1.5 stop bits at 5-bit words, 2 at 6/7/8-bit words. The receiver checks only the first stop bit (p27, Table 7 p28). |
| 3 | PEN | parity enable |
| 4 | EPS | 1 = even parity, 0 = odd |
| 5 | stick parity | PEN=1, EPS=1, bit5=1 → parity bit sent and checked as 0. PEN=1, EPS=0, bit5=1 → sent and checked as 1. |
| 6 | break | forces SOUT low; affects only the serial output |
| 7 | DLAB | divisor latch access; also needed to write FCR5 |

- **MCR** (address 4, read/write). Reset 0x00; bits 6–7 always 0.

| Bit | Name | Meaning |
|---|---|---|
| 0 | DTR | 1 drives the DTR pin low |
| 1 | RTS | 1 drives the RTS pin low |
| 2 | OUT1 | 1 drives the OUT1 pin low |
| 3 | OUT2 | 1 drives the OUT2 pin low |
| 4 | LOOP | loopback |
| 5 | AFE | autoflow enable |

- **LSR** (address 5, read). Reset 0x60.

| Bit | Name |
|---|---|
| 0 | DR |
| 1 | OE |
| 2 | PE |
| 3 | FE |
| 4 | BI |
| 5 | THRE |
| 6 | TEMT |
| 7 | error in RX FIFO |

  - Writing the LSR is "not recommended outside of a factory testing environment" (p28).
- **MSR** (address 6, read). Reset: bits 0–3 cleared; bits 4–7 follow the inputs.

| Bit | Name | Meaning |
|---|---|---|
| 0 | ΔCTS | CTS changed |
| 1 | ΔDSR | DSR changed |
| 2 | TERI | trailing edge of RI |
| 3 | ΔDCD | DCD changed |
| 4 | CTS | complement of the CTS pin; MCR1 (RTS) in loopback |
| 5 | DSR | complement of the DSR pin; MCR0 (DTR) in loopback |
| 6 | RI | complement of the RI pin; MCR2 (OUT1) in loopback |
| 7 | DCD | complement of the DCD pin; MCR3 (OUT2) in loopback |

  - Bits 0–3 are cleared when the MSR is read (pp30–31).
- **SCR** (address 7): scratch register, no effect on the chip. Not changed by MR.
- **DLL / DLM** (addresses 0 and 1 with DLAB=1): the 16-bit divisor. Not changed by MR.
- The FCR, LCR and MCR reset to 0. The RX and TX FIFOs are cleared by MR, by FCR1 or FCR2, or when FCR0 changes (Table 2).

### Behavior

**Baud generator (p31, Fig 4 p12)**
- The source is XIN (crystal on XIN/XOUT, or an external clock), up to 16 MHz at 5 V and 14 MHz at 3.3 V (p8).
- divisor = fXIN / (baud × 16), with divisor 1 to 65535. There is no prescaler.
- Loading either divisor latch also loads the 16-bit counter.
- **BAUDOUT** is the 16× clock, driven out low-active. Its waveform (Fig 4):
  - ÷1: BAUDOUT follows XIN.
  - ÷2 and ÷3: shown separately.
  - N > 3: low for 2 XIN cycles, high for N−2 XIN cycles.
- The transmitter always uses the internal 16× clock.
- The receiver uses the RCLK pin as its 16× clock. Tie BAUDOUT to RCLK for the same rate.
- Divisor tables are on p32 (Table 9 for 1.8432 MHz, Table 10 for 3.072 MHz).

**Receiver**
- 16× sampling. The sample point is 8 RCLK clocks into the bit (Fig 7: "8 Clocks").
- False start bits are rejected (p1); the method is not described beyond that.
- After a framing error the ACE assumes the error was caused by the next start bit, samples that start bit twice, and then accepts data (p29).
- **Break:** BI is set when SIN stays low for longer than one full word (start + data + parity + stop).
  - Only one 0x00 character is loaded into the FIFO.
  - The next character is accepted after SIN is high for at least 2 RCLK samples and a valid start bit follows (p29).
- **Overrun:** in FIFO mode, OE is set only when the FIFO is full and another character has been fully received. The character in the shift register is overwritten and not transferred (p28).
- **Error flags:** PE, FE and BI travel with each character in the FIFO and show in the LSR when that character reaches the top. OE, PE, FE and BI are cleared by reading the LSR.
- **DR** is set when a character reaches the RBR or FIFO, and cleared when the RBR or FIFO is empty.

**Transmitter**
- **THRE (LSR5):** set when the THR (or, in FIFO mode, the TX FIFO) is empty; cleared when the THR is written.
- **TEMT (LSR6):** set when the THR (or FIFO) and the TSR are both empty.
- **THRE delay in FIFO mode:** THRE is delayed by "one character time minus the last stop bit time when there have not been at least two bytes in the transmit FIFO at the same time since the last time that THRE = 1". The first transmit interrupt after FCR0 changes is immediate (p25).

**FIFOs**
- Both FIFOs are 16 bytes (FCR5 = 0) or 64 bytes (FCR5 = 1). The RX FIFO stores 3 error bits per byte.
- Reset leaves the chip in 450 mode (FIFOs off).
- **Receive timeout (p25):** the timeout interrupt fires when all of these hold:
  - at least 1 character is in the FIFO;
  - the last character was received more than 4 continuous character times ago (the second stop bit is counted if programmed);
  - the last CPU read of the FIFO was more than 4 character times ago.
- Character times are measured from RCLK. The timer resets when a character is received or the CPU reads the FIFO.
- **LSR7:** set when any PE, FE or BI is in the FIFO. Cleared when the LSR is read and no further errors remain in the FIFO (p29). The text says "In TL16C750 mode and in TL16C450 mode, this bit is always cleared", which is unclear; probably it means "in 450 (non-FIFO) mode".
- **Polled mode:** FCR0 = 1 with IER0–3 = 0 (p25).
- **DMA (pp4–5, Figs 10, 11, 13, 14):**
  - RXRDY, mode 0 (FCR0 = 0, or FCR0 = 1 with FCR3 = 0): low while at least 1 character is in the RBR or FIFO; high when it is empty.
  - RXRDY, mode 1 (FCR0 = 1, FCR3 = 1): goes low when the trigger level or the timeout is reached; goes high when the FIFO is empty.
  - TXRDY, mode 0: low when the THR or FIFO is empty; high after the first byte is written (Fig 13, td20).
  - TXRDY, mode 1: low while the FIFO has room; high when the FIFO is full (Fig 14, "Byte #16 … FIFO Full", drawn for 16-byte mode; the 64-byte mode equivalent is assumed to be "full"). TXRDY goes low again td21 = 9 baud cycles after the start bit.
  - 450 mode allows only DMA mode 0.

**Interrupts (Table 5, p27)**

| IIR[3:0] | Priority | Source | Cleared by |
|---|---|---|---|
| 0001 | — | none | — |
| 0110 | 1 (highest) | line status: OE, PE, FE, BI | reading the LSR |
| 0100 | 2 | received data available: RBR full in 450 mode, or trigger level reached | reading the RBR (FIFO mode: when the level drops below the trigger) |
| 1100 | 2 | character timeout | reading the RBR |
| 0010 | 3 | THRE | reading the IIR (if THRE is the source) or writing the THR |
| 0000 | 4 (lowest) | modem status: ΔCTS, ΔDSR, TERI, ΔDCD | reading the MSR |

- The INTRPT pin is active high and always driven; it is not 3-state and is not gated by OUT2 (pp4, 22). It is low after MR.
- ΔCTS does not raise an interrupt while AFE = 1 (p30).

**Modem lines and autoflow (pp5–6, 30)**
- MCR bits drive the pins inverted: bit = 1 gives a low pin.
- Table 8 (p30):
  - AFE = 1, RTS = 1: auto-RTS and auto-CTS.
  - AFE = 1, RTS = 0: auto-CTS only.
  - AFE = 0: no autoflow.
- **Auto-RTS:** RTS goes high when the RX FIFO reaches the trigger level (1/4/8/14, or 1/16/32/56 in 64-byte mode). It goes low again once the FIFO has been emptied by RBR reads.
  - td26: at most 2 baud cycles from the threshold byte to RTS↑.
  - td27: at most 3 baud cycles from the read of the last byte to RTS↓ (p11).
- **Auto-CTS:** the transmitter checks CTS before each byte. CTS must go high before the middle of the last stop bit to stop the next byte (tsu4 = 10 ns max setup before the mid-stop point). td25 = 24 baud cycles max from CTS low to SOUT↓ (start bit).
- Autoflow is available only in FIFO mode (p1: "In the FIFO mode, there is a selectable autoflow control").
- **Loopback (MCR4) (p30):**
  - SOUT is forced high, SIN is disconnected, and the TSR output feeds the RSR.
  - CTS, DSR, DCD and RI are disconnected.
  - Internally RTS→CTS, DTR→DSR, OUT1→RI and OUT2→DCD.
  - The four modem outputs are forced high (inactive).
  - Modem interrupts then come from MCR0–3.

**Sleep and low power (p26)**
- IER4 = 1 enables sleep. The ACE stays awake when:
  - there is a byte in the transmitter,
  - SIN has activity,
  - it is in loopback,
  - or any of ΔCTS, ΔDSR, ΔDCD or TERI is 1.
- IER5 = 1 enables low-power mode, which stops the ACE clock but keeps the oscillator running. Otherwise it behaves like sleep.
- The datasheet does not say which clock sleep stops beyond this.

**Reset (MR, active high, minimum 1 µs) (Table 2, p22)**
- Registers: IER = 0, IIR = 0x01, FCR = 0, LCR = 0, MCR = 0, LSR = 0x60, MSR[3:0] = 0.
- SCR, DLL, DLM, RBR and THR are unchanged.
- Outputs: SOUT high; INTRPT low; OUT1, OUT2, RTS and DTR high.
- RXRDY and TXRDY reset states are not listed.

**Not covered by this datasheet:** no IrDA, RS-485, 9-bit mode or software flow control.

### Timing (5 V and 3.3 V share one table; limits as printed, pp10–11)

**Bus timing**

| Parameter | Symbol | Limit |
|---|---|---|
| Read cycle | tcR | 87 ns min |
| Write cycle | tcW | 87 ns min |
| XIN high / low | tw1 / tw2 | 25 ns min each (at 16 MHz) |
| ADS low | tw5 | 9 ns min |
| WR pulse | tw6 | 40 ns min |
| RD pulse | tw7 | 40 ns min |
| MR pulse | tw8 | 1 µs min |
| Address setup to ADS↑ | tsu1 | 8 ns |
| CS setup to ADS↑ | tsu2 | 8 ns |
| Data setup to WR1↓ / WR2↑ | tsu3 | 15 ns (as printed; the figure shows the setup to the trailing edge) |
| Address hold after ADS↑ | th1 | 0 ns |
| CS hold after ADS↑ | th2 | 0 ns |
| CS hold after WR↑ | th3 | 10 ns |
| Address hold after WR↑ (ADS low) | th4 | 10 ns |
| Data hold after WR | th5 | 5 ns |
| CS hold after RD | th6 | 10 ns |
| Address hold after RD | th7 | 20 ns |
| CS / address valid to WR↓ | td4 / td5 | 7 ns |
| WR↑ to next ADS↓ | td6 | 40 ns |
| CS / address to RD↓ | td7 / td8 | 7 ns |
| RD↑ to next ADS↓ | td9 | 40 ns |
| RD↓ to data valid | td10 | 45 ns max (CL = 75 pF) |
| RD↑ to data float | td11 | 20 ns max |
| RD to DDIS | tdis(R) | 20 ns max |

- Minimum read cycle between FIFO reads and IIR/LSR reads in FIFO mode: 425 ns (Note 10, p11).

**Baud generator**

| Parameter | Symbol | Limit |
|---|---|---|
| BAUDOUT low / high (at ÷2) | tw3 / tw4 | 50 ns min |
| XIN↑ to BAUDOUT↑ | td1 | 45 ns max |
| XIN to BAUDOUT↓ | td2 | 45 ns max |

**Receiver**

| Parameter | Symbol | Limit |
|---|---|---|
| RCLK to sample clock | td12 | 10 ns max |
| Stop bit to INT, LSI or RXRDY↓ | td13 | 2 RCLK cycles max (9 RCLKs for a timeout) |
| Read RBR/LSR to INT↓ | td14 | 120 ns max |

**Transmitter**

| Parameter | Symbol | Limit |
|---|---|---|
| INTRPT to transmit start | td15 | 8–24 baud cycles |
| Start to interrupt | td16 | 8–10 baud cycles |
| WR THR to interrupt reset | td17 | 50 ns max |
| Initial write to THRE interrupt | td18 | 16–34 baud cycles |
| Read IIR to interrupt reset | td19 | 70 ns max |
| Write to TXRDY inactive | td20 | 75 ns max |
| Start to TXRDY active | td21 | 9 baud cycles |

**Modem (p12)**

| Parameter | Symbol | Limit |
|---|---|---|
| WR MCR to output | td22 | 60 ns max |
| Modem input change to INT | td23 | 35 ns max |
| RD MSR to interrupt reset | td24 | 45 ns max |

### VCC (p8)
- 3.3 V grade: 3.0–3.6 V, clock up to 14 MHz. VIH = 0.7 VCC, VIL = 0.3 VCC.
- 5 V grade: 4.75–5.25 V, clock up to 16 MHz. VIH = 0.7 VCC, VIL = 0.2 VCC.
- Inputs are rated to VI ≤ VCC in the operating conditions, but "fail-safe" terminals are rated to an absolute maximum of 6.5 V (p7). The data bus uses "fail safe CMOS compatible input buffers" (p4). The datasheet does not list which other terminals are fail-safe, so treat inputMax as VCC except D7–D0.

### Package drawings
- PM (64-pin LQFP) is not drawn in tl16c750.pdf. The same TI package code, PM0064A, is drawn in `datasheets/controller-uart/tl16c754c.pdf`: outline p53, land pattern p54, stencil p55.
  - Body 10.0 mm (9.8–10.2).
  - Lead span 12.0 mm (11.8–12.2).
  - Pitch 0.5 mm, 16 leads per side.
  - Lead width 0.17–0.27 mm (0.22 mid).
  - Foot length 0.45–0.75 mm (0.60 mid).
  - Height 1.6 mm max.
  - Land pattern: 64 × (1.5 × 0.3) mm pads at 0.5 mm pitch, pad row centers (11.4) mm apart.
- The FN (PLCC-44) drawing is not in this PDF; FN is not in the census.

### Obsolete
- None. TL16C750PM is Active.

### Notes
- The p2 FN drawing may show an overbar on CS1; the terminal table (p4) says CS1 is active high, and the PM drawing has no bar on CS1. Use active high.
- Table 2 lists the IER reset as "0–5 forced and 6–7 permanent" and the MCR as "6–7 permanent", i.e. those bits always read 0.
- The p25 text says "The transmit FIFO is cleared [IIR (3–0) = 1] when the THR is written". This means the THRE interrupt is cleared, not the FIFO.
- p24 calls the line-status IIR value "06 or 0110h"; this means 0110 binary.
- Interrupt priority 2 covers both the trigger level and the timeout.
- The RXRDY/TXRDY states during MR are not specified.

---

## TL16C750E

- Title (p1): "TL16C750E UART with 128-Byte FIFO", SLLSF10, Dec 2019 (initial release).
- Datasheet: `datasheets/controller-uart/tl16c750e.pdf`, 62 pages.
  - pp1–55: data.
  - p56: orderable addendum.
  - pp57–58: packing.
  - p59: PFB0048A outline; p60: land pattern; p61: stencil.
  - Printed page numbers equal PDF page numbers.
- A different design from the TL16C750:
  - 128-byte FIFOs and a 16C750-family "enhanced" register set (EFR, Xon/Xoff, TCR, TLR, FIFO Ready, AFR, DLF fractional divisor).
  - Software flow control, IrDA SIR and RS-485 direction on DTR.
  - Selectable 8×/16× sampling and a ÷4 prescaler.
  - Single 8-bit bus with a MODE pin (IOR/IOW or IOW-only).
  - No DDIS, ADS, BAUDOUT, RCLK or OUT1/OUT2. It is pin-compatible with the PFB TL16C550D (p19).

### Parts
| Census part | Package | Pins | Pinout | Grade | VCC |
|---|---|---|---|---|---|
| TL16C750EPFBR (Active) | TI PFB, 48-pin TQFP 7×7, PFB0048A, tape and reel | 48 | tqfp48 | −40 to 105 °C (addendum p56; only one grade exists) | 1.62–1.98, 2.25–2.75, 3.0–3.6 or 4.5–5.5 V (p5–6) |

- I could decode every census part.

### Pinout (PFB, 48-pin TQFP, tqfp48)
The drawing is on p3 (image /tmp/p11/img/ti-b-750-e-03.png); the pin table is on pp4–5.

| Pin | Name | Role | Pin | Name | Role |
|---|---|---|---|---|---|
| 1 | NC | nc | 25 | NC | nc |
| 2 | D5 | inout | 26 | A2 | input |
| 3 | D6 | inout | 27 | A1 | input |
| 4 | D7 | inout | 28 | A0 | input |
| 5 | NC | nc | 29 | RXRDY | output |
| 6 | NC | nc | 30 | INT | output |
| 7 | RX | input | 31 | OP | output |
| 8 | TX | output | 32 | RTS | output |
| 9 | NC | nc | 33 | DTR | output |
| 10 | MODE | input | 34 | NC | nc |
| 11 | CS | input | 35 | RESET | input |
| 12 | NC | nc | 36 | NC | nc |
| 13 | NC | nc | 37 | NC | nc |
| 14 | XTAL1 | input | 38 | CTS | input |
| 15 | XTAL2 | output | 39 | DSR | input |
| 16 | IOW | input | 40 | CD | input |
| 17 | NC | nc | 41 | RI | input |
| 18 | VSS | ground | 42 | VCC | power |
| 19 | IOR | input | 43 | D0 | inout |
| 20 | NC | nc | 44 | D1 | inout |
| 21 | NC | nc | 45 | D2 | inout |
| 22 | NC | nc | 46 | D3 | inout |
| 23 | TXRDY | output | 47 | D4 | inout |
| 24 | NC | nc | 48 | NC | nc |

- There is no exposed pad.
- Active-low pins (overbar on p3): CS, IOW, IOR, TXRDY, RXRDY, OP, RTS, DTR, CTS, DSR, CD, RI.
- RESET has no bar and is active high. INT is active high (Fig 6, p12).
- INT is 3-state: it is driven only while MCR3 = 1 (pin table p4).
- MODE (pin 10) is strapped to VCC or GND and selects the bus protocol:
  - VCC: IOR is used.
  - GND: IOR is unused and must be tied to VCC.

### Host interface (pp4, 32; Figs 2–5, pp10–12)
- **Address:** A2–A0. CS is active low.
- **MODE = VCC (IOR used):** while CS is low, IOR low does a read and IOW low does a write.
- **MODE = GND (IOR unused):**
  - When CS goes low, IOW is sampled: low means write, high means read (p32).
  - The read data is enabled by the CS low pulse (t12d is from CS).
  - The write is the CS low pulse with IOW held low. IOW setup/hold to CS are t29s/t29h (Fig 3).
  - In this mode CS acts as the strobe.
- **Write latch edge:** data setup t16s and hold t16h are measured to the rising edge of IOW (Fig 2) or of CS (Fig 3, IOW-only mode). Data is captured on the trailing edge.
- **Data bus:** D7–D0 are 3-state and driven only during a read.
- There are no DDIS or ADS pins.

### Register map (Figure 30 p34; Table 8 pp35–36)
Access conditions:
- **DL:** LCR[7] = 1.
- **AFR:** LCR[7:5] = 100.
- **EF (enhanced bank):** LCR = 0xBF.
- **TC (TCR/TLR):** EFR[4] = 1 and MCR[6] = 1.
- **DLF:** LCR[7] = 1, LCR ≠ 0xBF, EFR[4] = 1, and MCR ≠ 0bx1x0x1xx.
- **FR (FIFO Ready):** CS = 0, MCR[2] = 1, MCR[4] = 0.
- **Normal:** LCR[7] = 0 (and LCR ≠ 0xBF), with the TC and FR conditions false.
- MCR[7:5], FCR[5:4] and IER[7:4] can be modified only while EFR[4] = 1 (Fig 30 note).

| A2:A0 | Normal read | Normal write | DL | AFR (LCR[7:5]=100) | EF (LCR=0xBF) | TC | Other |
|---|---|---|---|---|---|---|---|
| 000 | RHR | THR | DLL | (DLL; LCR7 = 1) | (DLL) | — | — |
| 001 | IER | IER | DLH | (DLH) | (DLH) | — | — |
| 010 | IIR | FCR | (IIR/FCR) | AFR | EFR | — | — |
| 011 | LCR | LCR | LCR | LCR | LCR | LCR | — |
| 100 | MCR | MCR | MCR | MCR | Xon1 | MCR | — |
| 101 | LSR | — | LSR | LSR | Xon2 | LSR | — |
| 110 | MSR | — | MSR | MSR | Xoff1 | TCR | — |
| 111 | SPR | SPR | (SPR) | (SPR) | Xoff2 | TLR | DLF (DLF condition), FIFO Ready (read only, FR condition) |

Reading the table above:
- Fig 30 shows DLL/DLH at 000/001 when LCR[7] = 1, which includes LCR = 0xBF and LCR[7:5] = 100.
- With LCR[7] = 1 and LCR ≠ 0xBF, address 010 is AFR only when LCR[7:5] = 100. For other LCR[7] = 1 values Fig 30 does not say what 010 is.
- With LCR = 0xBF, address 010 is EFR. The read of 010 is also EFR.

Two priority cases are not stated:
- At 111, DLF and TLR can both be enabled.
- MSR at 110 is shown only for "none of the below conditions" (Table 8: LCR ≠ 0xBF and none of TCR/TLR/FIFORdy).

**RHR / THR (000, LCR[7] = 0)**
- Each is a 128-byte FIFO. With FIFOs disabled, FIFO location 0 is used (p37).

**IER (001, LCR[7] = 0)**. Reset 0x00; bits 7–4 writable only with EFR[4] = 1 (Table 14, p41).

| Bit | Meaning |
|---|---|
| 0 | RHR (RX data) interrupt enable (also the timeout interrupt) |
| 1 | THR interrupt enable. Re-enabling it causes a new interrupt if the THR is below the threshold. |
| 2 | RX line status interrupt enable |
| 3 | modem status interrupt enable |
| 4 | sleep mode enable |
| 5 | Xoff / special character interrupt enable |
| 6 | RTS interrupt enable |
| 7 | CTS interrupt enable |

**IIR (010, read, LCR[7] = 0)**. Reset 0x01 (Table 15, p42).

| Bit | Meaning |
|---|---|
| 0 | 0 = interrupt pending |
| 3:1 | encoded interrupt |
| 4 | Xoff or special character detected |
| 5 | CTS/RTS changed from low to high |
| 7:6 | both mirror FCR[0] |

**FCR (010, write, LCR[7] = 0)**. Reset 0x00 (Table 9, p37).

| Bit | Meaning |
|---|---|
| 0 | enable the TX and RX FIFOs |
| 1 | clear the RX FIFO and reset its counter; self-clearing |
| 2 | clear the TX FIFO and reset its counter; self-clearing |
| 3 | DMA mode 0 / 1 |
| 5:4 | TX trigger (spaces available): 00 = 16, 01 = 32, 10 = 64, 11 = 120. Needs EFR[4] = 1 to write. |
| 7:6 | RX trigger (characters): 00 = 1, 01 = 4, 10 = 120, 11 = 124 |

- The RX trigger values are as printed; see Notes.
- FCR can be written only while LCR[7] = 0 (Table 8 access column).

**AFR (010, LCR[7:5] = 100, read/write)**. Reset 0x10 (Table 22, p44; Table 2).

| Bit | Name | Meaning |
|---|---|---|
| 0 | RES | reserved, no function |
| 1 | IREN | IrDA SIR mode |
| 2 | 485EN | RS-485 half-duplex: DTR is driven high while data is pending (see RS-485 below) |
| 3 | 485LG | large-block mode |
| 4 | RCVEN | receiver enable while 485EN or IREN is set. Default 1. 0 also disables the character-timeout interrupt "even in RS-232 mode". |
| 7:5 | DLY2:0 | RS-485 turn-off delay in bit times |

**EFR (010, LCR = 0xBF, read/write)**. Reset 0x00 (Table 17, p42).

| Bit | Meaning |
|---|---|
| 3:0 | software flow control select (Table 1 below) |
| 4 | enhanced function enable; write-enables IER[7:4], FCR[5:4] and MCR[7:5]. Also needed for TCR, TLR, DLF and sleep. |
| 5 | special character detect: compare RX data with Xoff2 |
| 6 | auto-RTS |
| 7 | auto-CTS |

**LCR (011, always accessible)**. Reset 0x1D = 0b0001_1101: 6-bit words, 2 stop bits, parity enabled, even (Table 2 p25; Table 8 p35).

| Bit | Meaning |
|---|---|
| 1:0 | word length: 00 = 5, 01 = 6, 10 = 7, 11 = 8 |
| 2 | 0 = 1 stop bit; 1 = 1.5 stop bits at 5-bit words, 2 at 6–8-bit words |
| 3 | parity enable |
| 4 | 1 = even, 0 = odd |
| 5 | forced parity: LCR5 = 1 and LCR4 = 0 → parity bit 1; LCR5 = 1 and LCR4 = 1 → parity bit 0 |
| 6 | break: TX forced low |
| 7 | divisor latch enable (Table 8 calls it "DLAB & EFR enable") |

**MCR (100, LCR ≠ 0xBF)**. Reset 0x00; bits 7–5 need EFR[4] = 1 (Table 12, p40).

| Bit | Meaning | In loopback |
|---|---|---|
| 0 | 1 = DTR pin low | drives MSR[5] |
| 1 | 1 = RTS pin low; auto-RTS overrides | drives MSR[4] |
| 2 | FIFO Ready register enable | drives MSR[6] |
| 3 | 1 = INT pin active (driven) and OP pin low; 0 = INT 3-state and OP high | drives MSR[7] |
| 4 | internal loopback: TX→RX, MCR[3:0] → MSR | — |
| 5 | Xon Any | — |
| 6 | TCR/TLR access enable | — |
| 7 | clock prescaler: 0 = ÷1, 1 = ÷4 | — |

**Xon1, Xon2, Xoff1, Xoff2 (100–111, LCR = 0xBF, read/write)**
- Not reset by RESET (Table 2 note). Table 8 shows default bits of 0xFF, except Xon2 printed as 0xEF; these are probably the power-on values.

**LSR (101, read, LCR ≠ 0xBF)**. Reset 0x60 (Table 11, p39).

| Bit | Meaning |
|---|---|
| 0 | at least 1 character in the RX FIFO |
| 1 | overrun |
| 2 | parity error (top-of-FIFO character) |
| 3 | framing error (top-of-FIFO character) |
| 4 | break (top character is 00) |
| 5 | THR empty |
| 6 | THR and TSR empty |
| 7 | at least one PE, FE or BI is in the RX FIFO |

- LSR[4:2] come straight from the top-of-FIFO entry: reading the LSR does not pop the FIFO, and if the FIFO is empty they read 0 (p26, p39).

**MSR (110, read)**. Reset: [3:0] = 0, [7:4] follow the pins (Table 13, p40).

| Bit | Meaning |
|---|---|
| 0 | ΔCTS |
| 1 | ΔDSR |
| 2 | TERI (RI went low→high) |
| 3 | ΔCD |
| 4 | complement of CTS, or MCR1 in loopback |
| 5 | complement of DSR, or MCR0 in loopback |
| 6 | complement of RI, or MCR2 in loopback |
| 7 | complement of CD, or MCR3 in loopback |

- Bits 0–3 are cleared when the MSR is read.

**TCR (110, TC condition, read/write)**. Reset 0x00 (Table 19, p43).

| Bits | Meaning |
|---|---|
| 3:0 | RX FIFO level that halts the remote sender (RTS high, or send Xoff) |
| 7:4 | RX FIFO level that restores it (RTS low, or send Xon) |

- Level = field × 8, giving 0–120 (p43; the example on p24 uses 7 → 56 and 4 → 32).
- Software must ensure TCR[3:0] > TCR[7:4]; there is no hardware check.

**SPR (111, normal)**: scratch register. Not reset.

**TLR (111, TC condition, read/write)**. Reset 0x00 (Table 20, p43).

| Bits | Meaning |
|---|---|
| 3:0 | TX trigger in spaces, = field × 8 |
| 7:4 | RX trigger in characters, = field × 8 |

- A field of 0 means the FCR-selected level is used instead.

**DLF (111, DLF condition, read/write)**. Reset 0x00 per Table 8; Table 2 omits it (Table 18, p43).

| Bit | Meaning |
|---|---|
| 7 | 0 = 16× sampling, 1 = 8× |
| 6 | reserved, reads 0 |
| 5:0 | fraction x/64 |

**FIFO Ready (111, read, MCR[2] = 1 and MCR[4] = 0)** (Table 21, p44)

| Bit | Meaning |
|---|---|
| 0 | 1 = at least the TX trigger level of spaces is free |
| 3:1 | 0 |
| 4 | 1 = RX FIFO above the RX trigger level, or a timeout occurred |
| 7:5 | 0 |

- This register mirrors the TXRDY and RXRDY pins in software.

**DLL / DLH (000 / 001, LCR[7] = 1)**: divisor LSB and MSB. Not reset. Write them only while sleep (IER[4]) is off (p33, p43).

### Behavior

**Baud generator (pp28–31)**
- The input is XTAL1: a crystal from 1 to 24 MHz, or an external clock up to 48 MHz (Fig 27 notes p30).
- Maximum clock by supply: 16 MHz at 1.8 V, 24 MHz at 2.5 V, 48 MHz at 3.3 V and 5 V.
- The prescaler is ÷1 or ÷4 (MCR[7]).
- divisor = (fXTAL / prescaler) / (baud × 8 or 16), with the 8 or 16 set by DLF[7].
- The divisor is DLH:DLL + DLF[5:0]/64, where the integer part is 1 to 65535.
- DLL = DLH = 0 stops the baud clock, which disables the UART.
- Example (p51): 24 MHz at 9600 baud gives 156.25, so DLH = 0x00, DLL = 0x9C, DLF = 0x10. Write sequence: LCR = 0xBF, EFR = 0x10, LCR = 0x93, DLH, DLL, DLF, then LCR = 0x13.
- The fractional divider takes effect only when EFR[4] = 1; it is assumed that otherwise DLF is ignored (p30 conditions).
- There is no BAUDOUT or RCLK pin; TX and RX share the clock.

**Receiver**
- The RX line is sampled 3 times per bit and a majority vote decides the value (p20 note under Fig 19).
- 16× sampling (or 8× with DLF7).
- **Timeout:** fires when RX has been high (idle) for (4 × programmed word length) + 12 bits and at least 1 byte is in the RX FIFO (p27).
- **Break:** RX low for at least one character time sets BI and stores one 00 byte (p39).
- **Overrun:** overflow loses characters (p37). The datasheet does not say how OE clears; it is assumed to clear on an LSR read, by the interrupt table "OE: Read LSR" (p26).

**FIFOs and DMA (pp20, 32)**
- RX and TX FIFOs are 128 bytes each. Both are disabled after reset (trigger 1).
- Trigger levels come from FCR, or from TLR when TLR is non-zero.
- **DMA mode 0 or FIFOs off:**
  - TXRDY is low when the TX side is empty and goes high after 1 character is loaded.
  - RXRDY is low while at least 1 character is present and high when empty.
- **DMA mode 1:**
  - TXRDY is low when at least the trigger level of spaces is free and high when the FIFO is full.
  - RXRDY goes low at the trigger level or on a timeout. It goes high when the FIFO is empty or LSR7 flags an error.
- Writing the first byte starts transmission at once, so TXRDY can pulse (p21 note).

**Interrupts (Table 4, p26; Table 16, p42)**

| IIR[5:0] | Priority | Source | Cleared by |
|---|---|---|---|
| 000001 | — | none | — |
| 000110 | 1 | OE, FE, PE or BI in the RX FIFO | FE/PE/BI: all erroneous characters read out of the FIFO; OE: reading the LSR |
| 001100 | 2 | RX timeout | reading the RHR |
| 000100 | 2 | RHR: data ready (FIFO off) or RX at/above trigger | reading the RHR |
| 000010 | 3 | THR: empty (FIFO off) or TX FIFO passes the trigger level | reading the IIR or writing the THR |
| 001000 | 4 | modem: MSR[3:0] ≠ 0 | reading the MSR |
| 010000 | 5 | Xoff received or special character | receiving Xon, or reading the IIR (special character) |
| 100000 | 6 | RTS or CTS changed from active (low) to inactive (high) | reading the IIR |

- The INT pin is active high and 3-state unless MCR[3] = 1 (p4, p40). OP is the complement of MCR[3].
- **RS-485 interrupt hold-off:** with 485EN set and 485LG clear, transmit interrupts are held off until the TSR is empty.

**Hardware flow control (p21)**
- **Auto-RTS (EFR6):** RTS goes high when the RX FIFO reaches the TCR[3:0] HALT level and low again at the TCR[7:4] RESTORE level.
- **Auto-CTS (EFR7):** CTS must go high before the middle of the last stop bit to stop the next byte; transmission resumes when CTS goes low.

**Software flow control (pp22–24), Table 1, EFR[3:0]**

| EFR[3:2] | Transmit |
|---|---|
| 00 | no transmit flow control |
| 10 | send Xon1/Xoff1 |
| 01 | send Xon2/Xoff2 |
| 11 | send Xon1+Xon2 / Xoff1+Xoff2 as pairs |

| EFR[1:0] | Receive |
|---|---|
| 00 | no receive flow control |
| 10 | compare Xon1/Xoff1 |
| 01 | compare Xon2/Xoff2 |
| 11 | compare Xon1 or Xon2 / Xoff1 or Xoff2 when EFR[3:2] = 10 or 01; compare the sequential pairs Xon1 and Xon2 / Xoff1 and Xoff2 when EFR[3:2] = 11 or 00 |

- **On receiving Xoff:** transmission halts after the current character, IIR[4] is set, and INT fires if IER[5] = 1. Transmission resumes on the matching Xon, which clears IIR[4].
- **Sending Xoff and Xon:** Xoff is sent when the RX FIFO passes TCR[3:0]; Xon is sent when it falls to TCR[7:4].
- **Errored characters:** a flow-control character received with a parity, framing or break error is stored as data.
- **Unmatched pairs:** with paired matching, an Xon1 or Xoff1 not followed by Xon2 or Xoff2 is written to the FIFO.
- **Mode changes after an Xoff:** if software flow control is disabled or changed after an Xoff was sent, the originally programmed Xon is sent automatically. If the FIFO is still above the trigger, the new Xoff is sent.
- **Word length:** Xon and Xoff are sent with the current word length (5–8 LSBs).
- **Xon Any (MCR5):** after an Xoff, any received character resumes transmission. An Xon1 can be taken as an Xon Any, so an Xon2 may land in the FIFO (p22).
- **Special character (EFR5):** an RX character equal to Xoff2 sets IIR[4]. The character is stored in the FIFO and transmission does not halt. Reading the IIR clears it.
- Hardware and software flow control are assumed never to be enabled together.

**RS-485 (pp44–45)**
- With 485EN set, DTR goes high "one bit time before the first stop bit of the first data byte" and stays high while the THR or TSR holds data.
- After both are empty, DTR stays high for DLY[2:0] more bit times. The text says "0 to 15 bit times", but DLY is a 3-bit field (0–7); see Notes. Then DTR goes low.
- The delay starts only when the TSR and THR are both empty; any THR write cancels it.
- Fig 31 shows DTR going high at the first transmission, about 1 baud time after WR THR.
- RCVEN = 0 with 485EN set disables the receiver while transmitting. Table 23 (p45) lists the effect of each LOOP/RCVEN/AFR combination.

**IrDA SIR (pp46–49)**
- Supported only up to 115.2 kbps.
- **Encoder:** when the internal TX bit is 1, the TX pin is low. When the bit is 0, TX goes high on the falling edge of the 7th 16× clock and low on the falling edge of the 10th, a 3-clock pulse in each 16-clock bit.
- **Decoder:**
  - A falling edge on RX makes the internal RX low on the next rising edge of the 16× clock, for 16 cycles.
  - Data is strobed on the 8th 16× clock after the internal RX falling edge.
  - Negative or positive pulses are accepted; a positive pulse is decoded from its trailing (falling) edge.
- RCVEN gates the receiver.

**Sleep (p33)**
- Needs EFR[4] = 1 and IER[4] = 1.
- The chip enters sleep when RX is idle, the TX FIFO and TSR are empty, no interrupt is pending except THR or timeout, and the RX FIFO is empty.
- The UART clock and baud clock stop.
- It wakes on any RX change, any modem input change, or a write to the TX FIFO.
- Do not write DLL or DLH while asleep.

**Loopback (MCR4)**
- TX loops to RX internally. MCR[3:0] feed MSR[7:4] as listed under MCR.
- The TX and RX pins are disabled (pin table p4). Output pin levels in loopback are not stated.

**Reset (RESET pin, active high, minimum 200 ns) (Tables 2–3, p25)**
- Registers: IER = 0x00, IIR = 0x01, FCR = 0x00, LCR = 0x1D, MCR = 0x00, LSR = 0x60, MSR[3:0] = 0, EFR = 0x00, TCR = 0x00, TLR = 0x00, AFR = 0x10. The RHR and THR pointers are cleared.
- Not reset: DLL, DLH, SPR, Xon1, Xon2, Xoff1, Xoff2.
- Pins: TX, RTS, DTR and RXRDY high; TXRDY low.
- INT is 3-state because MCR3 = 0, and OP is high.
- During reset the transmitter output and receiver input are disabled (p4).

### Timing (Table 7.6, pp7–9; TA −40 to 105 °C). Values are 1.8 V / 2.5 V / 3.3 V / 5 V.

**MODE = VCC (IOR used)**

| Parameter | Symbol | Value |
|---|---|---|
| Reset pulse width | tRESET | 200 ns min (all supplies) |
| Clock period | CP | 63 / 42 / 20 / 20 ns min |
| Clock frequency | — | 16 / 24 / 48 / 48 MHz max |
| Address setup | t6s | 65 / 45 / 30 / 20 ns min |
| Address hold | t6h | 15 / 10 / 7 / 5 ns min |
| IOR pulse width | t7w | 85 / 70 / 50 / 40 ns min |
| Read cycle delay | t9w | 85 / 70 / 60 / 50 ns min |
| IOR to data valid | t12d | 90 / 55 / 35 / 25 ns max |
| Data disable | t12h | 45 / 30 / 20 / 15 ns max |
| IOW pulse width | t13w | 85 / 70 / 50 / 40 ns min |
| Write cycle delay | t15w | 85 / 70 / 60 / 50 ns min |
| Data setup | t16s | 70 / 50 / 30 / 20 ns min |
| Data hold | t16h | 35 / 25 / 15 / 10 ns min |
| IOW to modem output | t17d | 80 / 50 / 35 / 25 ns max |
| Modem input to INT | t18d | 120 / 80 / 50 / 35 ns max |
| IOR to INT reset | t19d | 100 / 65 / 40 / 30 ns max |
| Stop to INT | t20d | 1 baud |
| IOR to INT reset | t21d | 100 / 65 / 40 / 30 ns max |
| Stop to TX interrupt | t22d | 1 baud |
| Initial IOW to transmit start | t23d | 8–24 baud |
| IOW to INT reset | t24d | 90 / 60 / 35 / 25 ns max |
| Stop to RXRDY | t25d | 1 baud |
| IOR to RXRDY reset | t26d | 100 / 65 / 40 / 30 ns max |
| IOW to TXRDY | t27d | 80 / 50 / 35 / 25 ns max |
| Start to TXRDY reset | t28d | 16 baud |

**MODE = GND (IOW only), where it differs (pp8–9)**

| Parameter | Symbol | Value |
|---|---|---|
| Address setup | t6s | 70 / 45 / 30 / 20 ns |
| CS to data valid | t12d | 95 / 65 / 40 / 25 ns |
| Data setup | t16s | 75 / 50 / 30 / 25 ns |
| Data hold | t16h | 80 / 50 / 35 / 25 ns |
| CS to modem output | t17d | 80 / 50 / 35 / 25 ns |
| Modem input to INT | t18d | 120 / 75 / 45 / 35 ns |
| CS to INT reset | t19d | 95 / 65 / 40 / 30 ns |
| IOR to INT reset (as printed) | t21d | 85 / 55 / 40 / 30 ns |
| IOW to INT reset | t24d | 90 / 60 / 40 / 25 ns |
| CS to RXRDY reset | t26d | 95 / 60 / 35 / 25 ns |
| CS to TXRDY | t27d | 80 / 50 / 35 / 25 ns |
| IOW hold to CS | t29h | 15 / 10 / 7 / 5 ns min |
| IOW setup to CS | t29s | 70 / 50 / 30 / 20 ns min |

### VCC (Recommended Operating Conditions, pp5–6)

| Grade | VCC | VIH min | VIL max | Clock max |
|---|---|---|---|---|
| 1.8 V | 1.62–1.98 V | 1.4 V | 0.4 V | 16 MHz |
| 2.5 V | 2.25–2.75 V | 1.8 V | 0.6 V | 24 MHz |
| 3.3 V | 3.0–3.6 V | 0.7 × VCC | 0.8 V | 48 MHz |
| 5 V | 4.5–5.5 V | 2 V (XTAL1: 0.7 × VCC) | 0.8 V (XTAL1: 0.3 × VCC) | 48 MHz |

- Input voltage limit: 0.9 × VCC at 1.8 V and 2.5 V; VCC at 3.3 V and 5 V.
- The absolute maximum for VI is VCC + 0.5 V (p5). Inputs are not 5 V tolerant, so inputMax = VCC.

### Package drawing
- PFB0048A, 48-pin TQFP, 1.2 mm max height (outline p59, land pattern p60, stencil p61; drawing 4215157/A).
  - Body 7.0 mm (6.8–7.2).
  - Lead span 9.0 mm (8.8–9.2).
  - Pitch 0.5 mm, 12 leads per side.
  - Lead width 0.17–0.27 mm (0.22 mid).
  - Foot length 0.45–0.75 mm (0.60 mid).
  - Land pattern: 48 × (1.35 × 0.25) mm pads, pad row centers (8.5) mm apart.
  - There is no thermal pad.

### Obsolete
- None.

### Notes and datasheet errors
- **Copy-paste from multi-channel parts:** p3 says "two such devices are incorporated", p1 mentions "DTRx per channel", and the FIFO Ready footnote says "any CS A-B = 0". The chip has one channel; the FIFO Ready bits are labeled "RX FIFO A / TX FIFO A".
- **DLF formula conflict:** p30 gives DLF = ROUND(frac × 128), but Table 7 (p31), Table 18 and the p51 example all use ×64 (x/64). Use 64.
- **TCR range:** Table 19 says "(0 to 60)", but p43 says 0–120 with granularity 8 and the p24 example confirms ×8. Use field × 8.
- **RS-485 delay:** p45 says DLY gives "0 to 15 bit times", but DLY[2:0] is 3 bits (0–7).
- **RX trigger levels:** FCR[7:6] reads 1/4/120/124 (p37). That looks odd (a TL16C752-like set would be 8/16/56/60), but it is printed this way, with no other source in the PDF.
- **LCR reset 0x1D** is printed in both Table 2 and the Table 8 defaults; it is unusual but consistent. It means 6-bit words, 2 stop bits, parity enabled, even.
- **Timeout definition:** (4 × word length) + 12 bits (p27), not "4 character times".
- **Blank figures:** Figs 7, 9, 11, 13, 15 and 17 (the IOW-only mode versions) are empty apart from their captions; Fig 17 shows only a t27d label.
- **Table 8 IIR bits 7:6** are labeled "FCR(0)", i.e. both equal FCR0.
- **MSR footnote (p40):** the modem pins are active low; the MSR status bits are active high.
- **Unspecified details:** how OE clears, when the CTS/RTS interrupt sets in auto mode, the register at 010 for LCR[7] = 1 values other than 100xxxxx and 0xBF, and what DLF does with EFR4 = 0.

## TL16C754B

- Title: "TL16C754B QUAD UART WITH 64-BYTE FIFO" (SLLS397A, Nov 1999, rev. June 2004). Datasheet: `datasheets/controller-uart/tl16c754b.pdf`. PDF page = printed page. Pages 1–2 pinouts; 3–4 terminal functions; 5–16 functional description; 17–18 recommended operating conditions; 19–24 timing; 25–34 registers; 35 FN drawing; 36 PN drawing; 37–38 package addendum.
- Function: four independent ST16C654-compatible UARTs (pin compatible with TL16C554 and ST16C654, p5). Each has 64-byte TX and RX FIFOs, auto-RTS/CTS, Xon/Xoff, TCR/TLR programmable trigger levels, sleep mode, and a shared FIFO-ready register. The four channels share D0–D7, A0–A2, IOR, IOW, RESET, XTAL1/2 and CLKSEL. Each channel has its own CS, INT, TX, RX and modem lines. RXRDY and TXRDY are single shared pins.
- Siblings: TL16C554 has 16-byte FIFOs and no EFR, TCR, TLR, FIFORdy, sleep or flow control. TL16C754C (below) is this design plus the AFR register (IrDA, RS-485, concurrent write), with different FCR RX trigger levels and lower supply grades.

### Parts
| census part | status | package | pins | pinout | grade | supply |
|---|---|---|---|---|---|---|
| TL16C754BFN | Active | TI FN (S-PQCC-J68) PLCC, tube | 68 | plcc68 | −40…85 °C (only grade; p1, addendum p37) | 2.7–3.6 V or 4.5–5.5 V (p17, p18) |
| TL16C754FN | **Obsolete** (DigiKey) | FN = 68-pin PLCC (TI package letter) | 68 | plcc68 (assumed same as TL16C754B) | **not decodable from repo PDFs** | DigiKey says 2.7–5.5 V |

- **TL16C754FN** is the original TL16C754, not the B revision. DigiKey links it to a separate TL16C754.pdf, which is not in the repo. `tl16c754b.pdf` never names it; its ordering addendum lists only TL16C754BFN and TL16C754BPN(R)(G4), all −40…85 °C (p37).
  - The 754C datasheet (p39) calls AFR "functionality beyond the capabilities of the original TL16C754", and the B is described as ST16C654 pin compatible. That makes the FN pinout almost certainly identical, but its temperature grade and any register differences from the B cannot be confirmed from our PDFs.
- No census part uses PN (80-pin LQFP). The PN pinout is still listed below for completeness.

### Pinouts
Repo names have no overbars. Active-low pins: CSx, IOR, IOW, CTSx, DSRx, CDx, RIx, DTRx, RTSx, RXRDY, TXRDY. Active-high pins: INTx, RESET, INTSEL. The FN drawing (p2) and the terminal table (p3–4) agree; checked against the page images.

**plcc68 (FN, p2, p3–4)**
```
1 D3 inout      18 IOW input     35 XTAL1 input   52 IOR input
2 D4 inout      19 TXB output    36 XTAL2 output  53 TXD output
3 D5 inout      20 CSB input     37 RESET input   54 CSD input
4 D6 inout      21 INTB output   38 RXRDY output  55 INTD output
5 D7 inout      22 RTSB output   39 TXRDY output  56 RTSD output
6 GND ground    23 GND ground    40 GND ground    57 GND ground
7 RXA input     24 DTRB output   41 RXC input     58 DTRD output
8 RIA input     25 CTSB input    42 RIC input     59 CTSD input
9 CDA input     26 DSRB input    43 CDC input     60 DSRD input
10 DSRA input   27 CDB input     44 DSRC input    61 CDD input
11 CTSA input   28 RIB input     45 CTSC input    62 RID input
12 DTRA output  29 RXB input     46 DTRC output   63 RXD input
13 VCC power    30 CLKSEL input  47 VCC power     64 VCC power
14 RTSA output  31 NC nc         48 RTSC output   65 INTSEL input
15 INTA output  32 A2 input      49 INTC output   66 D0 inout
16 CSA input    33 A1 input      50 CSC input     67 D1 inout
17 TXA output   34 A0 input      51 TXC output    68 D2 inout
```
INTx are 3-state outputs (see Interrupts). INTSEL has an internal pulldown.

**lqfp80 (PN, p1, p3–4)**: no census part uses it.
```
1 NC  2 NC  3 DSRA  4 CTSA  5 DTRA  6 VCC  7 RTSA  8 INTA  9 CSA  10 TXA
11 IOW  12 TXB  13 CSB  14 INTB  15 RTSB  16 GND  17 DTRB  18 CTSB  19 DSRB  20 NC
21 NC  22 NC  23 CDB  24 RIB  25 RXB  26 CLKSEL  27 NC  28 A2  29 A1  30 A0
31 XTAL1  32 XTAL2  33 RESET  34 RXRDY  35 TXRDY  36 GND  37 RXC  38 RIC  39 CDC  40 NC
41 NC  42 NC  43 DSRC  44 CTSC  45 DTRC  46 VCC  47 RTSC  48 INTC  49 CSC  50 TXC
51 IOR  52 TXD  53 CSD  54 INTD  55 RTSD  56 GND  57 DTRD  58 CTSD  59 DSRD  60 NC
61 NC  62 NC  63 CDD  64 RID  65 RXD  66 VCC  67 INTSEL  68 D0  69 D1  70 D2
71 D3  72 D4  73 D5  74 D6  75 D7  76 GND  77 RXA  78 RIA  79 CDA  80 NC
```
Roles are the same as on plcc68.

### Host interface
- **Bus:** Intel-style. A2–A0 select the register; CSA–CSD (active low) select a channel; IOR and IOW are active-low strobes (p3).
  - A read drives D7–D0 while CSx and IOR are both low. A write takes D7–D0 while CSx and IOW are both low.
  - Fig. 11/12 (p20) time the address and data against the strobe: T6s/T6h for the address, T16s/T16h for write data relative to the end of IOW.
  - Model: latch write data at the trailing (rising) edge of IOW (or of CS, whichever ends the overlap), and drive the bus only during the read overlap.
- **One channel per access:** only one CSx should be low per access, except for the FIFORdy read (any CS low).
- **XTAL1-synchronous strobes (Fig. 13, p20):** T8s/T8h give IOR/IOW setup and hold to XTAL1 edges. IOR/IOW must be at least 2 clock periods wide (T7w, T13w = 2P), and successive IOW→IOR must be 4P apart (T8d). Strobes are effectively sampled by the input clock.
- **Bus outputs:** there is no DDIS and no ALE. D7–D0 are 3-state.

### Registers (per channel; Table 7 p25, Table 8 p26, bit tables p27–32)
Access conditions (notes to Table 7, p25):
- DLL and DLH appear at 000 and 001 when LCR[7] = 1.
- EFR (010) and Xon1, Xon2, Xoff1, Xoff2 (100–111) appear only when LCR = 0xBF.
- TCR (110) and TLR (111) appear only when EFR[4] = 1 and MCR[6] = 1.
- FIFORdy (111, read) appears when any CS is low, MCR[2] = 1 and MCR[4] = 0.
- Write-enable gating: MCR[7:5], IER[7:4] and FCR[5:4] can only be changed when EFR[4] = 1 (p26, p29, p30).

| A2..A0 | condition | read | write | reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | THR | FIFO pointers cleared |
| 001 | LCR[7]=0 | IER | IER | 00 |
| 010 | LCR≠BF | IIR | FCR | IIR=01, FCR=00 |
| 011 | any | LCR | LCR | 0x1D (Table 2, p10) |
| 100 | LCR≠BF | MCR | MCR | bits 6–0 = 0, bit 7 = NOT CLKSEL |
| 101 | LCR≠BF | LSR | — | 0x60 |
| 110 | LCR≠BF, not TCR mode | MSR | — | bits 3–0 = 0, bits 7–4 = inputs |
| 111 | LCR≠BF, not TLR/FIFORdy | SPR | SPR | not reset |
| 000 | LCR[7]=1 | DLL | DLL | not reset |
| 001 | LCR[7]=1 | DLH | DLH | not reset |
| 010 | LCR=BF | EFR | EFR | Table 2: bits 6–0 = 0, bit 7 = NOT CLKSEL (see Notes) |
| 100 | LCR=BF | Xon1 | Xon1 | not reset |
| 101 | LCR=BF | Xon2 | Xon2 | not reset |
| 110 | LCR=BF | Xoff1 | Xoff1 | not reset |
| 111 | LCR=BF | Xoff2 | Xoff2 | not reset |
| 110 | EFR[4]=1 & MCR[6]=1 | TCR | TCR | 00 |
| 111 | EFR[4]=1 & MCR[6]=1 | TLR | TLR | 00 |
| 111 | MCR[2]=1 & MCR[4]=0 | FIFORdy | — | — |

Bits:
- **IER** (Table 14, p30): bit 0 RHR (RX data/timeout) interrupt; 1 THR interrupt; 2 RX line status; 3 modem status; 4 sleep mode*; 5 Xoff interrupt*; 6 RTS interrupt*; 7 CTS interrupt*. Starred bits need EFR[4] to write. "Re-enabling IER[1] will cause a new interrupt, if the THR is below the threshold."
- **FCR** (write-only, Table 9, p27):
  - bit 0: FIFO enable (TX and RX).
  - bit 1: clear RX FIFO and reset its counter (self-clearing).
  - bit 2: clear TX FIFO (self-clearing).
  - bit 3: DMA mode 0/1.
  - bits 5:4 TX trigger (EFR[4] needed): 00 = 8, 01 = 16, 10 = 32, 11 = 56 spaces.
  - bits 7:6 RX trigger: 00 = 8, 01 = 16, 10 = 56, 11 = 60 characters.
- **IIR** (read, Table 15/16, p31):
  - bit 0: 0 = interrupt pending.
  - bits 3:1: code.
  - bit 4: Xoff/special character detected.
  - bit 5: CTS/RTS change from low to high.
  - bits 7:6: both mirror FCR[0].
- **LCR** (Table 10, p28):
  - bits 1:0: word length 5/6/7/8.
  - bit 2: stop bits. 0 = 1; 1 = 1.5 when 5-bit, 2 when 6/7/8-bit.
  - bit 3: parity enable.
  - bit 4: 1 = even, 0 = odd.
  - bit 5: stick parity. With bit 3 = 1: bit 5 = 1 and bit 4 = 0 forces parity 1; bit 5 = 1 and bit 4 = 1 forces parity 0.
  - bit 6: break (TX forced low).
  - bit 7: divisor latch enable. Together with the value 0xBF it also gives EFR access.
- **MCR** (Table 12, p29):
  - bit 0: DTR (1 = pin low). In loopback it drives MSR[5].
  - bit 1: RTS (1 = pin low). In loopback it drives MSR[4]. Overridden by auto-RTS.
  - bit 2: FIFORdy enable. In loopback it drives MSR[6].
  - bit 3: IRQ enable. 0 = INTx high-Z, 1 = active. In loopback it drives MSR[7].
  - bit 4: local loopback.
  - bit 5: Xon Any*.
  - bit 6: TCR/TLR access enable*.
  - bit 7: prescaler. 0 = ÷1, 1 = ÷4. Loaded with NOT CLKSEL at the trailing edge of RESET.
  - Starred bits need EFR[4] to write.
- **LSR** (Table 11, p28):
  - bit 0: data in RX FIFO.
  - bit 1: overrun.
  - bit 2: PE of the top-of-FIFO character.
  - bit 3: FE of the top-of-FIFO character.
  - bit 4: BI of the top-of-FIFO character (associated byte is 00).
  - bit 5: THR empty. In FIFO mode: TX FIFO empty, "can now load up to 64 bytes".
  - bit 6: THR and TSR empty.
  - bit 7: at least one PE/FE/BI anywhere in the RX FIFO; clears when none remain.
  - LSR[4:2] are not stored. They are the error bits of the character at the head of the RX FIFO, and are 0 if the FIFO is empty (p11, p29). Reading LSR does not pop the FIFO.
- **MSR** (Table 13, p30):
  - bit 0: ΔCTS. bit 1: ΔDSR. bit 2: TERI, set on a low→high change of the RI *input*, i.e. MSR[6] going 1→0 (see Notes). bit 3: ΔCD. Bits 0–3 clear on read.
  - bit 4: CTS (complement of the pin; MCR[1] in loopback). bit 5: DSR (complement; MCR[0]). bit 6: RI (complement; MCR[2]). bit 7: CD (complement; MCR[3]).
- **SPR**: scratch register, read/write.
- **DLL/DLH**: 16-bit divisor (DLH is the MSB). "Can only be written to before sleep mode is enabled" (p32).
- **EFR** (Table 17, p31):
  - bits 3:0: software flow-control mode (Table 1 below).
  - bit 4: enhanced-functions enable (write enable for IER[7:4], FCR[5:4], MCR[7:5]).
  - bit 5: special character detect (compare RX data with Xoff2).
  - bit 6: auto-RTS.
  - bit 7: auto-CTS.
- **Xon1, Xon2, Xoff1, Xoff2**: 8-bit flow-control characters.
- **TCR** (Table 18, p32):
  - bits 3:0: HALT level. bits 7:4: RESTORE (resume) level.
  - Each is ×4, giving 0–60. The programmer must keep HALT > RESTORE; there is no hardware check.
- **TLR** (Table 19, p32):
  - bits 3:0: TX trigger, ×4 (4–60 spaces). bits 7:4: RX trigger, ×4 (4–60 characters).
  - A field of 0 means use the FCR-selected level.
- **FIFORdy** (Table 20, p32):
  - bits 3:0: TX status of channels A, B, C, D (bit 0 = A). 1 = at least the TX-trigger number of spaces is free.
  - bits 7:4: RX status of channels A, B, C, D (bit 4 = A). 1 = RX level ≥ trigger, or timeout.
  - Real-time; shared by all four channels.

**Table 1, EFR[3:0] software flow control (p8):**
- Bits 3:2, transmit side:
  - 00: none.
  - 10: send Xon1/Xoff1.
  - 01: send Xon2/Xoff2.
  - 11: send Xon1+Xon2 / Xoff1+Xoff2 as two-character sequences.
- Bits 1:0, receive side:
  - 00: none.
  - 10: compare Xon1/Xoff1.
  - 01: compare Xon2/Xoff2.
  - 11: depends on bits 3:2 (both sides listed):
    - 1011: transmit Xon1/Xoff1; receive compares Xon1 **or** Xon2, Xoff1 **or** Xoff2.
    - 0111: transmit Xon2/Xoff2; receive compares Xon1 or Xon2, Xoff1 or Xoff2.
    - 1111: transmit both pairs; receive compares Xon1 **and** Xon2 (sequential) and Xoff1 **and** Xoff2.
    - 0011: no transmit flow control; receive compares Xon1 and Xon2, Xoff1 and Xoff2 (sequential).

### Behavior
- **Clock and baud rate** (p14, Fig. 9):
  - XTAL1 (crystal 1–24 MHz, or an external clock) feeds a prescaler (÷1 or ÷4, set by MCR[7]), then the 16-bit divisor, which gives a 16× baud clock.
  - baud = fXTAL / prescaler / (16 × divisor), with divisor 1 to 65535. A divisor of 0 stops the baud clock ("UART is effectively disabled").
  - XTAL2 is the oscillator output (it buffers the clock). There is no BAUDOUT and no RCLK pin.
  - Crystals above 24 MHz are not allowed; use an oscillator on XTAL1 (p16). Clock max: 50 MHz at 4.5 V and 45 MHz at 3 V (T3w, p19). The recommended conditions say 35 MHz at 3.3 V and 50 MHz at 5 V (p17–18); the features list says 48 MHz/32 MHz (p1).
- **Receiver:**
  - 16× sampling. "Vote logic" takes three samples per bit and uses the majority (note, p5).
  - Start-bit false detection is present (p1). The datasheet does not say how it is validated; the standard 16550 behavior is a midpoint re-check.
  - LSB first.
  - Each FIFO entry is 8 data bits plus 3 error bits (PE, FE, BI).
  - Overrun: the FIFO is full and a new character completes. The new character is lost (p27: "If overflow occurs, characters are lost") and LSR[1] sets. It is cleared by reading LSR (Table 4).
  - Break: RX low for one character time. Stores 00 with BI set.
  - In loopback, RX is disconnected and TX is fed back internally (p4, p29).
  - Software flow-control characters received with PE, FE or BI are treated as data (p8).
- **Transmitter:** 5–8 bits, parity, 1/1.5/2 stop bits, idles high. Break is driven by LCR[6].
- **FIFOs and triggers:**
  - FCR[0] enables both FIFOs. With FIFOs disabled, FIFO location 0 is used (one-byte holding register).
  - After reset the FIFOs are disabled, so the effective trigger is 1 (p6).
  - The active trigger comes from TLR when its field is non-zero, otherwise from FCR.
- **RX timeout:** fires when RX has been high for (4 × programmed word length) + 12 bits and the RX FIFO holds at least one byte (p13). "Word length" is not defined more precisely; it is probably the full character time. Cleared by reading RHR.
- **DMA and the RXRDY/TXRDY pins** (p12–13):
  - Mode 0, or FIFOs disabled: TXRDY is active (low) when the THR/FIFO is empty and goes inactive after one character is loaded. RXRDY is active when there is at least one character and inactive when empty.
  - Mode 1: TXRDY is active when the trigger-level number of spaces is free and inactive when the FIFO is full. RXRDY is active when the trigger is reached or a timeout occurs, and inactive when the FIFO is empty or LSR[7] flags an error.
  - Pin combining (p4): RXRDY "contains the wire-ORed status of all four receive channel FIFOs". It goes low when any channel's trigger is reached or that channel times out, and high when all RX FIFOs are empty. TXRDY goes low when any channel has the trigger number of spaces, and high when all four TX buffers are full.
  - Reset state: RXRDY high, TXRDY low (Table 3, p10).
- **Interrupts** (Table 4, p11; Table 16, p31). Priority, highest first:

| IIR[5:0] | priority | source | cleared by |
|---|---|---|---|
| 000110 | 1 | RX line status: OE, or PE/FE/BI in the FIFO (LSR[7]) | FE/PE/BI: all erroneous characters read out of the FIFO. OE: reading LSR |
| 001100 | 2 | RX timeout | reading RHR |
| 000100 | 2 | RHR: data ready (no FIFO) or RX ≥ trigger | reading RHR (level below trigger) |
| 000010 | 3 | THR: THR empty (no FIFO), or TX FIFO spaces ≥ trigger | reading IIR, or writing THR |
| 000000 | 4 | modem status: any of MSR[3:0] | reading MSR |
| 010000 | 5 | Xoff received / special character | Xon received (Xoff case), or reading IIR (special-character case) |
| 100000 | 6 | CTS or RTS pin changed from active (low) to inactive (high) | reading IIR |
| 000001 | — | none | — |

  - **INTx pins:** active high. Output enable = INTSEL OR MCR[3]. With INTSEL = 1 the INT outputs are always enabled; with INTSEL = 0 (internal pulldown) MCR[3] = 0 makes them high-Z. INTx are high-Z after reset (p3).
- **Auto-RTS** (EFR[6], p6, p31):
  - RTS is active (low) while the RX FIFO level is below the HALT level (TCR[3:0] × 4). It is deasserted when the level reaches HALT.
  - It is reasserted when the level falls to RESTORE (TCR[7:4] × 4).
- **Auto-CTS** (EFR[7], p7): before each character, TX checks CTS. If CTS goes high before the middle of the last stop bit of the current character, the current character finishes and the next one is not sent. Sending resumes when CTS goes low.
- **Software flow control** (p7–9):
  - Receiving the Xoff character(s) halts TX after the current character, sets IIR[4], and raises INT if IER[5] = 1. Receiving the matching Xon clears it.
  - Transmit side: Xoff1/2 is sent when the RX FIFO passes TCR[3:0]; Xon1/2 is sent when it falls to TCR[7:4].
  - If software flow control is disabled after an Xoff was sent, Xon is sent automatically. If EFR[3:0] changes after an Xoff was sent, the original Xon is sent, then the new Xoff if the level is still above the trigger.
  - Xon/Xoff characters are sent with the current word length (low 5/6/7 bits).
  - Sequential pairs: a received Xon1/Xoff1 is written to the RX FIFO if the following character is not Xon2/Xoff2.
  - Xon Any (MCR[5]): after an Xoff, any received character resumes TX. Note: Xon1 may be taken as "any", which could put Xon2 into the FIFO.
  - Special character (EFR[5]): received data matching Xoff2 sets IIR[4] (Xoff interrupt) without halting TX. The character is stored in the FIFO. Cleared by reading IIR.
  - Hardware and software flow control are "assumed never enabled simultaneously" (p9).
- **Sleep** (p13):
  - Enabled when EFR[4] = 1 and IER[4] = 1.
  - Entered when RX is idle, the TX FIFO and TSR are empty, no interrupt is pending except THR or timeout, and the RX FIFO is empty.
  - While asleep, the UART clock and baud clock stop.
  - Wakes on any RX change, any modem input change, or a write to the TX FIFO.
  - Do not write DLL/DLH while asleep.
- **Loopback** (MCR[4]):
  - TX is looped to RX internally. The TX pin's state is not stated (TL16C550 parts hold it high).
  - MCR[3:0] feed MSR[7:4]: DTR→DSR, RTS→CTS, MCR[2]→RI, MCR[3]→CD.
  - Modem output pins: the datasheet does not say whether they are forced inactive.
- **Reset** (RESET pin, active high, at least 200 ns; Tables 2–3, p10):
  - Registers: IER = 0. IIR = 01. FCR = 0. LCR = 0x1D. MCR = {NOT CLKSEL, 0000000}. LSR = 0x60. MSR[3:0] = 0. EFR = 0 except bit 7 = NOT CLKSEL (as printed). TCR = TLR = 0. RHR and THR pointers cleared.
  - Not reset: DLL, DLH, SPR, Xon1/2, Xoff1/2.
  - Pins: TX, RTS and DTR high; RXRDY high; TXRDY low; INTx high-Z.
  - "The UART transmitter output and the receiver input will be disabled during reset time" (p4).
- **Not specified:** no 9-bit mode, IrDA, RS-485, GPIO or device-ID register. Reserved encodings are not described.

### Timing (p19; TA −40…85 °C, VCC 3.3 V–5 V ±10%; one column, MIN/MAX as printed; P = input clock period)
| sym | parameter | value |
|---|---|---|
| CP | clock period | min 20 ns |
| TRESET | reset pulse width | min 200 ns |
| T3w | clock speed | max 50 MHz (4.5 V), 45 MHz (3 V) |
| T6s / T6h | address setup / hold | min 0 / 0 ns |
| T7d | IOR delay from CS | min 10 ns |
| T7w | IOR strobe width | min 2P |
| T7h | CS hold from IOR | min 0 ns |
| T8d | IOW-to-IOR successive assertion | min 4P |
| T8s / T8h | IOR/IOW setup to XTAL1↑ / hold from XTAL1↓ | min 20 / 20 ns |
| T9d | read cycle delay | min 2P |
| T12d | IOR to data valid | max 30 ns (4.5 V), 47 ns (3 V) |
| T12h | data disable (float) | max 15 ns |
| T13d | IOW delay from CS | min 10 ns |
| T13w | IOW strobe width | min 2P |
| T13h | CS hold from IOW | min 0 ns |
| T15d | write cycle delay | min 2P |
| T16s / T16h | data setup / hold | min 16 / 15 ns |
| T17d | IOW to modem output | max 50 ns |
| T18d | modem input to INT | max 70 ns |
| T19d | IOR (MSR) to INT reset | max 70 ns |
| T20d | stop bit to INT set | max 1 Rclk (baud) |
| T21d | IOR to INT reset | max 70 ns |
| T22d | stop to (TX) interrupt | max 100 ns (as printed) |
| T23d | initial IOW to TX start | min 8, max 24 baud (16× clocks, per Fig. 18) |
| T24d | IOW to INT reset | max 70 ns |
| T25d | stop to RXRDY set | max 1 Clk |
| T26d | IOR to RXRDY reset | max 1 µs |
| T27d | IOW to TXRDY set | max 70 ns |
| T28d | start to TXRDY reset | max 16 baud |
| T30s | address setup | min 10 ns (purpose not shown in any figure) |

### VCC
- Low-voltage grade: 2.7–3.6 V, nominal 3.3 V (p17).
- Standard grade: 4.5–5.5 V (p18).
- VI is 0 to VCC and the absolute maximum is VCC + 0.5 V (p17), so inputs are **not** 5 V-tolerant at 3.3 V.

### Package drawings
- **FN (S-PQCC-J68)**, drawing 4040005/B (p35), JEDEC MS-018.
  - Pitch 1.27 mm. Lead width 0.33–0.53 mm.
  - 68-pin: D/E (over leads) 25.02–25.27 mm; D1/E1 (body) 24.13–24.33 mm; D2/E2 (lead row to centre) 11.20–11.91 mm.
  - Height max 4.57 mm.
  - No land pattern printed.
- **PN (S-PQFP-G80)**, drawing 4040135/B (p36), JEDEC MS-026. Not a census package.
  - Pitch 0.50 mm. Lead width 0.17–0.27 mm.
  - Body 11.80–12.20 mm square; tip-to-tip 13.80–14.20 mm square.
  - Foot length 0.45–0.75 mm. Height max 1.60 mm.

### Obsolete
- TL16C754FN (DigiKey "Obsolete").

### Notes
- "754A" appears in several pin descriptions (p3–4). These are copy leftovers; read them as TL16C754B.
- **RXRDY pin text vs DMA sections:** the pin text "goes high when all RX FIFOs are empty and there is an error in RX FIFO" (p4) contradicts the DMA sections. Use the DMA rules per channel, then combine. "Wire-ORed" for active-low pins means the pin is low if any channel is ready.
- **EFR[7] reset:** Table 2 says "Bit 7 reflects the inverse of the CLKSEL pin value" for both MCR and EFR. For EFR this looks like a copy error from MCR, since EFR[7] is auto-CTS. Recommend EFR = 0x00 at reset.
- **LCR reset 0x1D** (6 bits, 2 stop bits, even parity) is as printed (Table 2).
- **MSR[2] (TERI):** "changed state from low to high" refers to the RI input, i.e. a ringing → not-ringing edge (the RI pin description says the same, p4). In loopback the datasheet text maps it to MCR[2].
- **Overlapping read decodes at 110/111:** the precedence between MSR/SPR, TCR/TLR and FIFORdy is not stated.
  - Reasonable model: with LCR = 0xBF, Xon/Xoff win. Otherwise, with EFR[4] & MCR[6], TCR/TLR win. Otherwise, a read of 111 with MCR[2] = 1 and MCR[4] = 0 gives FIFORdy. Otherwise, MSR/SPR.
  - FIFORdy uses the MCR of the selected channel.
  - The programmer's guide step "Set temp2 = temp1 * EF" (p34) is garbled.
- **T23d, T20d and T28d units:** the footnote "† Baudrate" means bit times or 16× clocks; the datasheet does not say which. T23d 8–24 matches 16550's "8–24 RCLK" convention (16× clock cycles).
- **Notes 7–12 on p18** reference pins that do not exist (IRQ3–15, GPIO, XOUT). Copy error; ignore.

## TL16C754C (orderable as TL16CP754C)

- Title: "TL16CP754C and TL16C754C – Quad UARTs With 64-Byte FIFO" (SLLS644H, Dec 2007, rev. Jan 2014). Datasheet: `datasheets/controller-uart/tl16c754c.pdf`. PDF page = printed page.
  - Pages: 1–5 features, pinout and terminals; 6–16 functional description; 17–20 ratings and electrical; 22–27 timing; 28–38 registers; 39–44 AFR, RS-485 and IrDA; 45–46 programmer's guide; 48–49 addendum; 53 PM0064A outline; 54 land pattern; 55 stencil.
- **Same as TL16C754B except** (all cited):
  1. **AFR register added.** Alternate Function Register at 010 when LCR[7:5] = 100 (p29, p39, Fig. 22 p28). It adds concurrent write, IrDA SIR, RS-485 direction control, and the RCVEN receiver gating.
  2. **FCR[7:6] RX trigger levels:** 00 = **1**, 01 = **4**, 10 = 56, 11 = 60 characters (Table 11, p32). The TL16C754B has 8/16/56/60.
  3. **Supply grades** 1.8 V, 2.5 V, 3.3 V and 5 V, with a 4-column timing table (p17–18, p22).
  4. **Two temperature grades:** C = 0…70 °C and CI = −40…85 °C (p17, addendum p48).
  5. **One package only:** 64-pin PM LQFP. It has **no RXRDY, TXRDY or INTSEL pins** ("RXRDY and TXRDY functionality is not supported in the TL16C754CPM device", p2; terminal numbers "–", p3–4).
     - INTSEL is absent. It has an internal pulldown on the 754B, so presumably INTx output enable = MCR[3] only. This is inferred; the datasheet does not say it.
     - FIFORdy is still documented (p37–38), so the DMA status is software-only.
  6. **TX idle in IrDA mode:** TXn is low when no data is being sent (p4).
- Register and behavior details not listed above are identical, word for word, to TL16C754B (compare p6–16 and p31–38 with the 754B p6–32): IER, IIR, LCR, LSR, MCR, MSR, EFR, TCR, TLR, FIFORdy, Table 2 software flow control, auto-RTS/CTS, sleep, timeout, reset tables, baud formula and CLKSEL/MCR[7].

### Parts
| census part | status | package | pins | pinout | grade | supply |
|---|---|---|---|---|---|---|
| TL16CP754CPM | Active | TI PM (PM0064A) LQFP 10×10, tray | 64 | lqfp64 | 0…70 °C (addendum p48, "TL16C754C" row p17) | 1.62–5.5 V in four bands (below) |
| TL16CP754CIPM | Active | PM LQFP 64 | 64 | lqfp64 | −40…85 °C (addendum p48) | same |

Decoding: TL16CP754C = device, I = industrial, PM = 64-pin LQFP, R = reel (none in census).

### Pinout (PM, top view p2; terminal table p2–4). Image checked.
```
1 DSRA input    17 DSRB input    33 CTSC input    49 CDD input
2 CTSA input    18 CDB input     34 DTRC output   50 RID input
3 DTRA output   19 RIB input     35 VCC power     51 RXD input
4 VCC power     20 RXB input     36 RTSC output   52 VCC power
5 RTSA output   21 CLKSEL input  37 INTC output   53 D0 inout
6 INTA output   22 A2 input      38 CSC input     54 D1 inout
7 CSA input     23 A1 input      39 TXC output    55 D2 inout
8 TXA output    24 A0 input      40 IOR input     56 D3 inout
9 IOW input     25 XTAL1 input   41 TXD output    57 D4 inout
10 TXB output   26 XTAL2 output  42 CSD input     58 D5 inout
11 CSB input    27 RESET input   43 INTD output   59 D6 inout
12 INTB output  28 GND ground    44 RTSD output   60 D7 inout
13 RTSB output  29 RXC input     45 GND ground    61 GND ground
14 GND ground   30 RIC input     46 DTRD output   62 RXA input
15 DTRB output  31 CDC input     47 CTSD input    63 RIA input
16 CTSB input   32 DSRC input    48 DSRD input    64 CDA input
```
- No NC pins and no exposed pad.
- INTx are active-high 3-state outputs, gated by MCR[3].
- The DTRx pins double as RS-485 driver enables when AFR[2] = 1 (p3).

### Host interface
Same Intel bus as TL16C754B. Fig. 13 and 14 (p23, image checked) show both cases: IOW (or IOR) pulsing while CS is held low, and CS pulsing while IOW (or IOR) is held low. The access is the overlap of CSx low and IOW/IOR low.
- A write latches D[7:0] at the end of the overlap: t16s before, t16h after.
- A read drives D t12d after the overlap starts and floats t12h after it ends.
- The address is set up t6s before the overlap starts and held t6h after it ends.

### Registers (Fig. 22 p28; Table 10 p29–30)
- **Register map:** the 754B map plus **AFR**, read/write at 010 when LCR[7:5] = 100 (that is, LCR = 0x80–0x9F).
  - With LCR = 0xBF, 010 is EFR.
  - With LCR[7] = 1 but neither of those, 010 is IIR/FCR (by elimination, Fig. 22). With LCR[7] = 1, 000/001 are always DLL/DLH, including at 0xBF.
- **AFR bits** (Table 10 p29 header vs Table 23 p39):

| bit | Table 10 name | Table 23 meaning |
|---|---|---|
| 0 | CONC | concurrent write: a write to one channel goes to all four ("Ensure that any indirect addressing modes have been enabled before using") |
| 1 | IREN | IrDA SIR enable (specified to 115.2 kbps) |
| 2 | 485RN (sic) | **485EN**: half-duplex RS-485 mode; DTRx driven high while the THR or TSR holds data, plus a delay |
| 3 | 485LG | large block: TX interrupts as in normal mode (threshold or empty) instead of being held off until the TSR is empty |
| 4 | RCVEN | receiver enable while 485EN or IREN is set. 0 = receiver disabled while the channel transmits, and **the RX timeout interrupt is unavailable even in RS-232 mode** |
| 7:5 | DLY2..DLY0 | RS-485 turn-off delay in bit times. The text says "DLY3–DLY0 … 0 to 15 bit times" but only 3 bits exist; see Notes |

  - AFR reset = **0x10** (all bits except AFR4 cleared; RCVEN = 1) (Table 3 p11; Table 10 p29).
- **Table 24 (p40), LOOP × RCVEN × mode:**
  - Loop off, RCVEN = 1, AFR = 10/14/12 (RS-232/RS-485/IrDA): data stored; threshold, timeout and error interrupts all available.
  - Loop off, RCVEN = 0:
    - AFR = 00 (RS-232): data stored; threshold and error interrupts only (no timeout).
    - AFR = 04 (RS-485) or AFR = 02 (IrDA): no data stored, so no interrupts.
  - Loop on: RCVEN = 1 gives all interrupts. RCVEN = 0 stores data with threshold and error interrupts only, for every mode.
- **Reset and default values in Table 10** (power-up and reset column, p29–30):
  - RHR/THR 00. IER 00. IIR 01. FCR 00. AFR 10. EFR 00. LCR 1D. MCR 00 (bit 7 per CLKSEL). LSR 60. MSR 1110 0000 (inputs). TCR 00. TLR 00. FIFORdy 00.
  - Xon1 FF, Xon2 **EF**, Xoff1 FF, Xoff2 FF, SPR FF. These registers are not reset (Table 3 note); the values are as printed, likely power-up values.
- **IIR modem code conflict:** Table 18 (p36) prints the modem interrupt as 001000, but Table 5 (p12) prints 000000. Use 000000, as on the 754B; 001000 conflicts with the 3-bit code space.

### Behavior additions
- **Concurrent write** (AFR[0]): a write goes to all four channels at once. Reads are unaffected (implied; not stated).
- **RS-485** (AFR[2], p39–40, Fig. 23):
  - DTRx goes high one bit time before the first start bit and stays high while the THR or TSR holds data.
  - After both are empty (after the last stop bit), DTRx stays high for DLY bit times, then goes low.
  - The delay starts only when the TSR and THR are both empty, and a THR write cancels it.
  - With 485LG = 0, TX interrupts are held off until the TSR is empty.
  - With RCVEN = 0, the receiver ignores RX while the channel transmits (Table 23 text). Table 24 says no data is stored at all in RS-485 mode with RCVEN = 0.
- **IrDA SIR** (AFR[1], p41–44):
  - The baud clock (16×) drives the encoder.
  - **Encoder:** while the internal TX bit (Int_Tx) is 1, the TX pin is low and the pulse counter is held clear. While Int_Tx is 0, TX rises on the falling edge of the 7th 16× clock of the bit and falls on the falling edge of the 10th, giving a 3/16-bit high pulse in every 0 bit.
  - **Decoder:** after reset, Int_Rx = 1 and the 4-bit counter is cleared. A falling edge on RX makes Int_Rx go low on the next rising 16× clock, stay low 16 clocks, then return high.
    - Either negative or positive RX pulses are accepted. A positive pulse is recognized at its trailing (falling) edge.
    - The UART samples Int_Rx on the 8th 16× clock after its falling edge, then every 16 clocks.
  - IrDA is only specified to 115.2 kbps (maximum 16× clock 1.843 MHz). The datasheet notes a minimum pulse width of 1.6 µs or 3 clocks.
- **TX/RX idle:** in normal mode TX is high when idle. In IrDA mode TX is low when idle (p4).

### Timing (Timing Requirements p22, TA 0–70 °C; MIN/MAX by supply)
| sym | parameter | 1.8 V | 2.5 V | 3.3 V | 5 V |
|---|---|---|---|---|---|
| tRESET | reset pulse width min | 200 | 200 | 200 | 200 ns |
| CP | clock period min | 63 | 42 | 32 | 20 ns |
| t3w | clock max | 16 | 24 | 32 | 48 MHz |
| t6s | address setup min | 20 | 15 | 10 | 5 ns |
| t6h | address hold min | 15 | 10 | 7 | 5 ns |
| t7w | IOR width min | 85 | 70 | 50 | 40 ns |
| t9d | read cycle delay min | 85 | 70 | 60 | 50 ns |
| t12d | IOR to data max | 65 | 50 | 35 | 25 ns |
| t12h | data disable max | 35 | 25 | 20 | 15 ns |
| t13w | IOW width min | 85 | 70 | 50 | 40 ns |
| t15d | write cycle delay min | 85 | 70 | 60 | 50 ns |
| t16s | data setup min | 40 | 30 | 20 | 15 ns |
| t16h | data hold min | 35 | 25 | 15 | 10 ns |
| t17d | IOW to output max | 60 | 40 | 30 | 20 ns |
| t18d | modem input to INT max | 70 | 55 | 45 | 35 ns |
| t19d | IOR to INT reset max | 80 | 55 | 40 | 30 ns |
| t20d | stop to INT set max | 1 | 1 | 1 | 1 baud |
| t21d | IOR to INT reset max | 55 | 45 | 35 | 25 ns |
| t22d | stop to (TX) INT max | 1 | 1 | 1 | 1 baud |
| t23d | IOW to TX start | 8–24 | 8–24 | 8–24 | 8–24 baud |
| t24d | IOW to INT reset max | 75 | 45 | 35 | 25 ns |
| t25d | stop to RXRDY set max | 1 | 1 | 1 | 1 baud |
| t26d | IOR to RXRDY reset max | 1 | 1 | 1 | 1 µs |
| t27d | IOW to TXRDY set max | 70 | 60 | 50 | 40 ns |
| t28d | start to TXRDY reset max | 16 | 16 | 16 | 16 baud |

The table heading says 0–70 °C. No separate CI-grade timing is printed.

### VCC (Recommended Operating Conditions p17–18)
| band | VCC range | clock max |
|---|---|---|
| 1.8 V | 1.62–1.98 V | 16 MHz |
| 2.5 V | 2.25–2.75 V | 24 MHz |
| 3.3 V | 3.0–3.6 V | 32 MHz |
| 5 V | max 5.5 V (min blank; ±10%, so 4.5 V) | 48 MHz |

- Inputs: VI is 0 to VCC; absolute maximum VCC + 0.5 V. **Not 5 V-tolerant.**
- Model supply range: 1.62–5.5 V, with gaps between the bands. DigiKey states 1.62–5.5 V.

### Package drawing
- **PM0064A**, LQFP 64, 1.6 mm max height (outline p53; land pattern p54; stencil p55), drawing 4215162/A, JEDEC MS-026.
  - Body 9.8–10.2 mm square; tip-to-tip 11.8–12.2 mm.
  - Pitch 0.5 mm. Lead width 0.17–0.27 mm. Foot 0.45–0.75 mm.
  - Land pattern: 64 × (1.5 × 0.3) mm pads, row-to-row span 11.4 mm, pitch 0.5 mm.
  - Pin 1 is at top left; pins run counter-clockwise (1–16 left, 17–32 bottom, 33–48 right, 49–64 top).

### Obsolete
None.

### Notes
- **AFR field widths:** Table 10 bit names (DLY2..0 = bits 7:5, RCVEN = 4, 485LG = 3, 485RN = 2, IREN = 1, CONC = 0) conflict with the Table 23 text ("DLY3–DLY0", "0 to 15 bit times"). Three bits give 0–7. Recommend modelling DLY = AFR[7:5] (0–7 bit times) and flagging it.
- **"RS-485 Application Example" (p41)** labels the signal "48SEN"; this is 485EN.
- **The IrDA decoder text** (p42) says "The TL16C550C UART already strobes incoming serial data at the proper time". This is copy from the 550C datasheet.
- **Missing INTSEL pin:** INTSEL appears in the terminal table with no pin number. Same for RXRDY and TXRDY.
- **FCR RX trigger values:** 1/4/56/60 differ from the 754B's 8/16/56/60. The 754C datasheet is internally consistent on this (only Table 11 gives them), so take them as printed for the 754C.
- **Electrical characteristics (p19–20):** inputs leak at most 10 µA, and IOZ is ±20 µA "chip selected in write mode or chip deselect". Not needed for behavior.
