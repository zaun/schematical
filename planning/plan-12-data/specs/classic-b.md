# classic-b — 8251A, 82510, CDP65C51, 82C52, HD-6406, SMSC COM78C802/78804/78C804/92C451

Fact sheet for plan-12 (UART part kinds). All PDFs are in `datasheets/controller-uart/`. Page numbers are **PDF page numbers** (1-based), not printed book pages, unless noted. Scanned PDFs were read from page images (`/tmp/p11/img/classic-b-*`).

## PDF → devices

| PDF | pages | text layer | device(s) | census parts |
|---|---|---|---|---|
| 8251a.pdf | 26 | no (Rochester cover + Intel 1986 scan) | **8251A** USART (async + sync) | 8251A/BXA, D8251A, LD8251A, MD8251A/B, MR8251A/B, N8251A, N8251A-G, P8251A, P8251A-G (+ MR82510/B misfiled → 82510) |
| 82510.pdf | 41 | no (Rochester cover + Intel 1989 scan) | **82510** async serial controller | N82510, TN82510 (+ MR82510/B) |
| cdp65c51.pdf | 18 | no | **CDP65C51 / CDP65C51A** ACIA | CDP65C51ASM2-96, CDP65C51M1, CDP65C51M4S2357 |
| 82c52.pdf | 21 | yes (Intersil FN2950 Rev 4, 2015) | **82C52** UART/BRG | CS82C52, CS82C5296, ID82C52, IP82C52 (×2 rows), IS82C52R2490, IS82C52Z |
| hd-6406.pdf | 11 | no | **HD-6406** PACI (82C52 core + DMA/ALE/status pins) | HD3-6406-5 |
| com78c804.pdf | 14 | no | **COM78C804** quad scanner UART (base design) | COM78C804LJP |
| com78c802.pdf | 14 | no | **COM78C802** dual (78C804 design, 2 lines) | COM78C802, COM78C802P |
| com78804.pdf | 15 | no | **COM78804** quad (NMOS 78C804) | COM78804P |
| com92c451.pdf | 16 | no | **COM92C451** 16450 ACE + PC printer port | COM92C451P |

## Census parts not (fully) decodable

- **COM78C802** (no suffix) — package unknown; datasheet has DIP-40 and PLCC-44.
- **IS82C52R2490** — "R2490" suffix not in ordering table; base IS82C52 = industrial PLCC (N28.45).
- **CDP65C51M4S2357** — "S2357" suffix not in PDF (base M4 = SOIC, 4 MHz).
- **CDP65C51ASM2-96** — "SM" package letter and SSOP not in the datasheet (only D/E/M); DigiKey says 28-SSOP 0.154"; pinout assumed same numbering, unverified. -96 = tape & reel (not printed here; Intersil 82C52 sheet uses "96" for T&R).
- **8251A/BXA** — military suffix undecoded (no ordering info in PDF).
- **N8251A / N8251A-G / P8251A** — 8251A PDF has no ordering info; Intel prefix N = PLCC, but DigiKey lists AMD N8251A and P8251A as 28-CDIP — conflict. No PLCC pinout in the PDF.
- **MR8251A/B, MR82510/B** — Intel "R" = LCC; LCC pinout not printed in either PDF.
- **TN82510, LD8251A** — extended-temp prefixes (T, L) from Intel convention only; limits not in PDFs.
- **HD3-6406-5** — "HD3" = plastic DIP by Harris convention only (-5 = 0..70 °C is printed).
- **COM78C804LJP, COM78C802P, COM78804P** — LJP = PLCC / P = DIP decoded from the COM92C451 ordering line (same maker), not printed in their own sheets.
- **MR82510/B** is listed in the census under `8251a.pdf`; it is an 82510.

## 8251A

- Title: "8251A Programmable Communication Interface" (USART), Intel, Nov 1986, order no. 205222-002, HMOS. PDF has a Rochester Electronics cover page (p1, "N8251A-G-ROCA") followed by the Intel datasheet (PDF p2–26 = book pages 2-1..2-25).
- Datasheet: `datasheets/controller-uart/8251a.pdf` (26 pp, scanned). Pinout p2; pin/function descriptions p3–8; programming p9–15; ratings/DC/AC p18–19 with notes p17; waveforms p20–26. **No ordering table, no package drawing; only a 28-pin DIP pinout is printed** ("28-Pin DIP Package", p2).

### Parts (census rows filed under 8251a)
Datasheet gives only: commercial 0..70 °C, VCC 5 V ±10% (p18), "Available in EXPRESS and Military Versions" (p2), and "For Extended Temperature EXPRESS, use MIL 8251A electrical parameters" (p19). Prefix decode below uses Intel's standard package/grade prefixes (P = plastic DIP, D = CERDIP, N = PLCC, R = LCC; L = EXPRESS extended temp + burn-in; M…/B = military, "/B" = MIL-STD-883 class B) — **not printed in this PDF**.
| census part | maker | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|---|
| P8251A | AMD (DigiKey says 28-CDIP) | P = plastic DIP (Intel convention); DigiKey says CDIP — conflict, use dip28 either way | 28 | dip28 (600 mil) | commercial 0..70 °C | 4.5–5.5 V |
| P8251A-G | Rochester | plastic DIP, "-G" = Rochester green/RoHS (not in PDF) | 28 | dip28 | commercial | 4.5–5.5 V |
| D8251A | Rochester | CERDIP | 28 | dip28 | commercial | 4.5–5.5 V |
| LD8251A | Rochester | CERDIP, EXPRESS extended temp (−40..+85 °C) w/ burn-in | 28 | dip28 | extended (uses MIL parameters, p19) | 4.5–5.5 V |
| MD8251A/B | Rochester | CERDIP, military | 28 | dip28 | −55..+125 °C, 883B | 4.5–5.5 V (MIL params not in PDF) |
| MR8251A/B | Rochester | R = leadless chip carrier (LCC), military | 28 | **lcc28 — pinout NOT in this PDF** | military | — |
| 8251A/BXA | Rochester | "/BXA" = SMD/DESC-style military CERDIP code (X = case "X"? A = finish) — **undecoded** | 28 | dip28 (probable) | military | — |
| N8251A | AMD (DigiKey says 28-CDIP) | Intel N = PLCC, but DigiKey says CDIP — **conflict; package unverifiable from PDF** | 28 | dip28 or plcc28 | commercial | 4.5–5.5 V |
| N8251A-G | Rochester | as N8251A, -G green; Rochester cover page (p1) lists "N8251A-G-ROCA" with this datasheet | 28 | ? | commercial | 4.5–5.5 V |
| MR82510/B | Rochester | **misfiled — this is an 82510 (see 82510 section), not an 8251A** | | | | |
- Flags: N8251A / N8251A-G (PLCC vs CDIP conflict; no PLCC pinout printed), MR8251A/B (LCC pinout not printed), 8251A/BXA (suffix undecoded), all military/extended electrical limits not in this PDF.

### Pinout (DIP-28, p2 Figure 2)
```
 1 D2 inout        15 TXRDY out (high)
 2 D3 inout        16 SYNDET inout  (SYNDET/BD = sync detect / break detect)
 3 RXD in          17 CTS in (low)
 4 GND ground      18 TXEMPTY out (high)
 5 D4 inout        19 TXD out
 6 D5 inout        20 CLK in
 7 D6 inout        21 RESET in (high)
 8 D7 inout        22 DSR in (low)
 9 TXC in (TxC, low-active marking: data shifted on falling edge)  23 RTS out (low)
10 WR in (low)     24 DTR out (low)
11 CS in (low)     25 RXC in (RxC, data sampled on rising edge)
12 CD in (C/D: 1 = control/status, 0 = data)   26 VCC power
13 RD in (low)     27 D0 inout
14 RXRDY out (high) 28 D1 inout
```
Pin 16 role depends on mode: async → output BRKDET (break detect); sync internal-sync → output SYNDET; sync external-sync (mode bit ESD=1) → input SYNDET. Reset to output-low on RESET (p8).

### Host interface (p4 table, p5, p18, p22–23)
- Intel RD/WR with CS low and C/D (p4):

| C/D | RD | WR | CS | operation |
|---|---|---|---|---|
| 0 | 0 | 1 | 0 | 8251A data → data bus (read RX data) |
| 0 | 1 | 0 | 0 | data bus → 8251A data (write TX data) |
| 1 | 0 | 1 | 0 | status → data bus |
| 1 | 1 | 0 | 0 | data bus → control (mode/sync/command) |
| X | 1 | 1 | 0 | data bus 3-state |
| X | X | X | 1 | data bus 3-state |
- CS high: data bus float, RD and WR have no effect (p5); "As long as the 8251A is not selected, RD and WR do not affect the internal operation" (p3).
- CS and C/D are "considered addresses" (p17 note 2): stable tAR/tAW before RD/WR falling, held tRA/tWA after rising. Write data latched at WR rising edge (setup tDW, hold tWD). Read data valid tRD after RD falling; floats tDF after RD rising.
- CLK (p4): internal timing only, normally the CPU's phase-2 TTL clock; must be > 30× the RX/TX data bit rates (p4); p17 note 5: 1x mode fTx/fRx ≤ 1/(30 tCY); 16x/64x ≤ 1/(4.5 tCY); note 8: external sync mode requires CLK/bit ratio > 34.
- Recovery time between writes (tRV = 6 tCY, p18) — note 4 (p17): this is for Mode initialization only; "Write Data is allowed only when TxRDY = 1. Recovery Time between Writes for Asynchronous Mode is 8 tCY and for Synchronous Mode is 16 tCY."
- Status update: max delay 28 CLK periods from event (p16, p17 note 7); status update inhibited during status read (p3, p14).

### Control-word sequence (p9 Fig 7, p13)
After reset (RESET pin or internal reset command) the next C/D=1 write is the **Mode Instruction**. If mode is sync: next C/D=1 write is SYNC CHAR 1, then (if double sync) SYNC CHAR 2. All subsequent C/D=1 writes are **Command Instructions** until another reset (Command bit6 IR) returns to Mode format. In async, sync-char writes are skipped. Mode can only be changed after a reset (p9).
Power-up (p13 NOTE): device may come up in Mode, Sync-char or Command state; safe init = write 00h, 00h, 00h with C/D=1 (worst case: sync mode + two sync chars), then 40h (internal reset), then the Mode Instruction.

### Mode Instruction — async (B2B1 ≠ 00; Fig 8 p10)
- D1:D0 B2B1 baud factor: 00 = sync mode; 01 = 1x; 10 = 16x; 11 = 64x.
- D3:D2 L2L1 char length: 00 = 5, 01 = 6, 10 = 7, 11 = 8 bits.
- D4 PEN parity enable (1 = enable).
- D5 EP: 1 = even, 0 = odd.
- D7:D6 S2S1 stop bits: 00 invalid, 01 = 1, 10 = 1½, 11 = 2 (only affects Tx; Rx never requires more than one stop bit).
### Mode Instruction — sync (D1:D0 = 00; Fig 10 p12)
- D3:D2 char length as above; D4 PEN; D5 EP.
- D6 ESD external sync detect: 1 = SYNDET is an input; 0 = SYNDET is an output.
- D7 SCS single character sync: 1 = single sync char; 0 = double sync chars. Note: in external sync mode, programming double sync affects only Tx.
- Parity bit is not counted in char length; parity bit not readable; for <8 bits, unused bits are don't-care on write and read as 0 (p10).

### Command Instruction (Fig 12 p14)
- D0 TxEN: 1 = transmit enable.
- D1 DTR: 1 forces DTR pin low.
- D2 RxE: 1 = receive enable.
- D3 SBRK: 1 forces TxD low (send break); 0 normal.
- D4 ER: 1 = reset error flags PE, OE, FE.
- D5 RTS: 1 forces RTS pin low.
- D6 IR internal reset: 1 returns 8251A to Mode Instruction format.
- D7 EH enter hunt mode (sync only; no effect in async): 1 = enable search for sync characters.
- Note: "Error Reset must be performed whenever RxEnable and Enter Hunt are programmed" (p14).

### Status Read (C/D=1 read; Fig 13 p15)
- D0 TxRDY: status bit = transmit data buffer empty (NOT conditioned by CTS/TxEN). TxRDY **pin** = buffer empty · (CTS = 0) · (TxEN = 1).
- D1 RxRDY, D2 TxEMPTY, D6 SYNDET/BRKDET: same definitions as the pins.
- D3 PE: set on parity error; reset by Command ER; doesn't inhibit operation.
- D4 OE: set when CPU doesn't read a char before the next one becomes available; reset by ER; previous char lost.
- D5 FE (async only): set when valid stop bit not detected at end of a character; reset by ER.
- D7 DSR: 1 when DSR pin is low.

### Behavior
- **Transmitter** (p5, p10–11): TX buffer (1-deep) + shift register (double-buffered). Starts when TxEN = 1 and CTS = 0. Data shifted out on TxC falling edge, at TxC/1, /16, /64 (async) or TxC (sync). TxD held marking immediately on master reset, or when TxEN or CTS is off, or when transmitter empty (p5). "Tx Enable logic enhancement prevents a Tx Disable command from halting transmission until all data previously written has been transmitted"; if TxEN off or CTS off while transmitting, the Tx sends all data written prior to the disable before shutting down (p3, p5). At end of transmission TxD returns to mark unless SBRK (p3). Async frame: start (0), data LSB first, parity, programmed stop bits; break = continuous low while SBRK=1.
- **TxRDY pin**: high when buffer empty & CTS low & TxEN; reset by leading (falling) edge of WR when a data char is written (p5). TxRDY pin delay from center of last bit ≤ 14 tCY; TxRDY↓ from WR falling ≤ 400 ns (p19).
- **TxEMPTY** (p5): high when 8251A has no characters to send; reset upon receiving a char from CPU "if the transmitter is enabled"; remains high when transmitter disabled. Sync: high = no char loaded and SYNC char(s) are about to be or are being sent as fillers; TxEMPTY does not go low while SYNC is shifted out. Delay from center of last bit ≤ 20 tCY (p19).
- **Sync TX** (p11–12): TxD high until first char written (usually SYNC). When CTS low, first char sent. If CPU doesn't supply a char before buffers empty, SYNC chars (or the single sync char) are automatically inserted; TxEMPTY high during them; TxEMPTY reset by a data char write.
- **Async RX** (p6, p10): RxD normally high. Falling edge triggers start; validated by strobing again at nominal center (16x/64x only) — if low, valid start bit and bit counter locates bit centers. Data and parity sampled on RxC rising edge. Low at the STOP bit → FE. Only one stop bit required. Char loaded into RX buffer, RxRDY high. If previous char not fetched, new char replaces it and OE set. Errors don't affect operation. "RxD initialization": after master reset, a valid "1" must be detected on RxD before searching for a start bit (prevents break/unused line from appearing as data) — async only, once per master reset (p6). "A refined Rx initialization prevents the Receiver from starting when in 'break' state" (p3). Receiver baud counter starts at start bit edge; 16x mode sample at 8 RxC periods, then every 16 (Fig p21).
- **RxRDY** (p7): high when char ready; reset by the leading edge of RD (≤ 400 ns, p19) — "RxRDY is reset automatically upon the CPU data read" (p9). RxEnable off holds RxRDY reset. Async: receiver must be enabled to sense start bit and char complete. Sync: receiver enabled and char finished assembly. RxRDY pin delay from center of last bit ≤ 26 tCY (p19).
- **Break detect (async)** (p8): BRKDET pin/status goes high whenever RxD remains low through two consecutive stop-bit sequences (including start, data and parity bits). Reset only by master chip reset or RxD returning to 1.
- **Sync RX / hunt** (p12–13): ENTER HUNT (EH) in first command (with RxE and ER). Internal sync: data on RxD sampled on RxC rising edge; RX buffer compared at every bit boundary with SYNC1 until match; if double sync, next received char also compared with SYNC2; when both detected (contiguously), hunt ends, char sync achieved, SYNDET pin (output) high. SYNDET goes high in the middle of the last bit of the (second) sync char, or middle of the parity bit if parity enabled (p8, p12). SYNDET reset by status read. EH also sets all used char bits of the RX buffer to ones (prevents false SYNDET from buffer contents). SYNDET F/F reset at each status read regardless of internal/external; that doesn't return to hunt. In sync mode not in hunt, sync detection still works but only at the known word boundaries (so SYNDET can reassert on subsequent sync chars). Parity checked when not in hunt regardless of RxE (p13). Parity/overrun checked as in async.
- **External sync** (ESD=1, p8, p12): SYNDET is an input; a high level (positive-going) forces the 8251A out of hunt and it starts assembling chars on the rising edge of the next RxC; high can be removed after one RxC cycle; internal sync detect disabled; external sync status provided via flip-flop cleared on status read (p3); the SYNDET F/F may be set at any bit boundary. Setup tES after RxC rising: min 16 tCY, max tRPD − tCY (p19).
- **Modem**: DSR, CTS general-purpose inputs (DSR → status D7; CTS gates TX). DTR, RTS general outputs (Command D1, D5; 1 → pin low) (p5). tWC control delay from WR rising to TxEn/DTR/RTS ≤ 8 tCY; tCR control setup before read (DSR, CTS) ≥ 20 tCY (p19).
- **Reset** (p3): RESET high forces "Idle" mode until new control words written; min pulse 6 tCY with CLK running (p17 note 6). Internal reset (Command D6) also → idle and Mode format. SYNDET reset to output low. TxD marking. Reset values of command bits (TxEN/RxE/DTR/RTS) and status bits are not tabulated; implied: transmitter/receiver disabled, DTR/RTS high (inactive), TxRDY low, RxRDY low, TxEMPTY high ("remains high when transmitter disabled", p5).

### Timing (p18–19, 0..70 °C, VCC 5 V ±10%, CL 150 pF)
READ: tAR 0 min; tRA 0 min; tRR RD width 250 min; tRD data delay from RD 200 max; tDF RD to float 10–100.
WRITE: tAW 0; tWA 0; tWW 250 min; tDW 150 min; tWD 20 min; tRV 6 tCY (mode init; 8 tCY async / 16 tCY sync between writes, p17).
CLK: tCY 320–1350 ns; tφ high 120 to tCY−90; low 90 min; rise/fall 20 max.
Serial: tDTx TxD delay from TxC falling ≤ 1 µs; fTx/fRx 1x DC–64 kHz, 16x DC–310 kHz, 64x DC–615 kHz; tTPW/tRPW clock pulse width 1x 12 tCY, 16x/64x 1 tCY; tTPD/tRPD clock pulse delay 1x 15 tCY, 16x/64x 3 tCY; tTxRDY ≤ 14 tCY; tTxRDY CLEAR ≤ 400 ns; tRxRDY ≤ 26 tCY; tRxRDY CLEAR ≤ 400 ns; tIS internal SYNDET delay from RxC rising ≤ 26 tCY; tES 16 tCY..(tRPD − tCY); tTxEMPTY ≤ 20 tCY; tWC ≤ 8 tCY; tCR ≥ 20 tCY.
Async up to 19.2 kbaud, sync DC–64 kbaud (p2).

### VCC
5 V ±10% (4.5–5.5 V), 0..70 °C (p18). Abs max any pin −0.5..+7 V → inputs tolerate 5 V (NMOS, no VCC clamp spec); VIH 2.0 V, VIL 0.8 V, VOH 2.4 V @ −400 µA, VOL 0.45 V @ 2.2 mA; ICC 100 mA max.

### Package drawings
None in PDF. dip28: standard 28-lead 600-mil DIP, 0.100" pitch. PLCC/LCC variants (N, R prefixes) have no pinout or drawing here.

### Obsolete
None (all census 8251A rows Active).

### Notes
- Reset values of command/status are not tabulated in the datasheet — derived from text.
- Waveforms p24–26 (async TX/RX flag timing, sync TX/RX) are rotated diagrams; read but no additional numeric data beyond the AC table.
- Census anomaly: MR82510/B listed under 8251a.pdf; belongs to 82510.

## 82510

- Title: "82510 Asynchronous Serial Controller", Intel CHMOS III, October 1989, order 290116-004 (book pp 2-40..2-79). PDF p1 = Rochester Electronics cover ("REI Datasheet").
- Datasheet: `datasheets/controller-uart/82510.pdf` (41 pp, scanned). Features/block p2; pinouts p3; pin defs p3–4; register architecture p5–6; interrupts p7–8; clocking p8; TX/RX p9–11; BRG/timers p11–12; diagnostics/power-down p12–13; register map p14–17; register details p18–36; DC p37; AC p38–40; logic diagrams p41. **No ordering table, no package drawings** (p2: "28-Lead DIP and PLCC Packages (See Packaging Spec., Order #: 231369)").
- Different device from 16450 parts: 8250A/16450-compatible bank 0 plus three more banks (35 registers), two 4-deep FIFOs, two 16-bit BRG/timers, 9-bit/µLAN mode, control-character recognition.

### Parts
Datasheet only gives commercial 0..70 °C, VCC 5 V ±10% (p37). Prefix decode = Intel convention, **not printed in this PDF**: N = PLCC, P = plastic DIP, T = extended temperature (−40..+85 °C), MR…/B = military LCC 883B (Intel "R" = leadless chip carrier).
| census part | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|
| N82510 (Rochester) | PLCC | 28 | plcc28 | commercial 0..70 °C | 4.5–5.5 V |
| TN82510 (Rochester) | PLCC | 28 | plcc28 | extended (T prefix; limits not in PDF) | 4.5–5.5 V |
| MR82510/B (Rochester; census filed it under 8251a.pdf) | LCC (ceramic leadless), military 883B | 28 | lcc28 — assume same numbering as PLCC (not printed) | −55..+125 °C (not in PDF) | — |
Flags: TN82510 temp range and MR82510/B package/pinout are from Intel convention only.

### Pinout (p3 Figs 2–3; DIP and PLCC have identical pin numbers; PLCC pin 1 = D4 at top centre)
```
 1 D4 inout          15 DTR/TB out (DTR act. low | TB = BRGB out)
 2 D5 inout          16 RTS out (act. low; INPUT during hardware reset = clock strap)
 3 D6 inout          17 RESET in (high)
 4 D7 inout          18 CS in (low)
 5 INT out (high)    19 WR in (low)
 6 TXD out           20 RD in (low)
 7 VSS ground        21 VCC power
 8 OUT2/X2 out (OUT2 GP out, act. low | X2 crystal)   22 A0 in
 9 CLK/X1 in (ext clock | crystal)                    23 A1 in
10 RI/SCLK in (RI act. low | SCLK serial clock in)    24 A2 in
11 DSR/TA/OUT0 inout (DSR in | TA = BRGA out | OUT0 GP out)  25 D0 inout
12 DCD/ICLK/OUT1 inout (DCD in | ICLK sys-clock out | OUT1 GP out)  26 D1 inout
13 RXD in            27 D2 inout
14 CTS in (low)      28 D3 inout
```
Multifunction selection (Table 1 p4; PMD register; * = default): pin8 *OUT2 / X2 (selected by hardware strap at reset, see clocking); pin9 *CLK / X1; pin10 SCLK / *RI; pin11 OUT0 / TA / *DSR; pin12 OUT1 / ICLK / *DCD; pin15 TB / *DTR.

### Host interface (p3, p5, p39)
Demultiplexed Intel bus: CS low + RD low = read; CS low + WR low = write; A2..A0 + bank pointer (GIR/BANK bits 6:5) select register. Address/CS setup to RD/WR falling TAVRL/TAVWL, hold after rising 0. Write data latched on WR rising edge (TDVWH setup, TWHDX hold). Read data valid TRLDV after RD falling, float TRHDZ after RD rising. Command inactive→active ≥ Tcy+15 (TCIAD). D7–D0 3-state when not reading. INT active high.

### Register map (Table 9 p14; DLAB = LCR bit 7; GIR/BANK at address 2 in all banks)
| Bank | A2..0 | Read | Write |
|---|---|---|---|
| 0 (NAS, 8250A/16450) | 0 DLAB=0 | RXD | TXD |
| | 1 DLAB=0 | GER | GER |
| | 0 DLAB=1 | BAL | BAL |
| | 1 DLAB=1 | BAH | BAH |
| | 2 | GIR/BANK | BANK |
| | 3 | LCR | LCR |
| | 4 | MCR | MCR |
| | 5 | LSR | LSR |
| | 6 | MSR | MSR |
| | 7 | ACR0 | ACR0 (= 8250 scratchpad) |
| 1 (WORK) | 0 | RXD | TXD |
| | 1 | RXF | TXF |
| | 2 | GIR/BANK | BANK |
| | 3 | TMST | TMCR |
| | 4 | FLR | MCR |
| | 5 | RST | RCM |
| | 6 | MSR | TCM |
| | 7 | GSR | ICM |
| 2 (GENERAL CONF) | 0 | — | — |
| | 1 | FMD | FMD |
| | 2 | GIR/BANK | BANK |
| | 3 | TMD | TMD |
| | 4 | IMD | IMD |
| | 5 | ACR1 | ACR1 |
| | 6 | RIE | RIE |
| | 7 | RMD | RMD |
| 3 (MODEM CONF) | 0 DLAB=0 | CLCF | CLCF |
| | 1 DLAB=0 | BACF | BACF |
| | 0 DLAB=1 | BBL | BBL |
| | 1 DLAB=1 | BBH | BBH |
| | 2 | GIR/BANK | BANK |
| | 3 | BBCF | BBCF |
| | 4 | PMD | PMD |
| | 5 | MIE | MIE |
| | 6 | TMIE | TMIE |
| | 7 | — | — |

Default wake-up values (hardware or software reset; Table 4 p6): BAL 02H, BAH 00H, GER 00H, GIR/BANK 01H, LCR 00H, MCR 00H, LSR 60H, MSR 00H, ACR0 00H, RST 00H, ACR1 00H, RIE 1EH, RMD 00H, CLCF 00H, BACF 04H, BBCF 84H, PMD FCH, MIE 0FH, TMIE 00H, BBL 05H, BBH 00H, TMST 30H, FLR 00H, GSR 12H, FMD 00H, TMD 00H, IMD 0CH; RXD/TXD/RXF/TXF/TMCR/RCM/TCM/ICM "—". These make it 8250A/16450-compatible with an 18.432 MHz crystal (p6).

### Register bits (summary pp15–17, detail pp18–36)
- **GER** (b0 A1): 7:6 0; 5 TIE timers; 4 TxIE TX machine; 3 MIE modem; 2 RxIE RX machine; 1 TFIE TX FIFO; 0 RFIE RX FIFO. Masks GSR bits from GIR; doesn't change GSR (p26).
- **GIR/BANK** (A=2 all banks; 01H): 7 0; 6 BANK1; 5 BANK0; 4 0; 3:1 BI2..BI0 highest-priority enabled pending block: 101 Timer (highest), 100 TX machine, 011 RX machine, 010 RX FIFO, 001 TX FIFO, 000 Modem (lowest); 0 IPN interrupt pending, **active low** (0 = pending). Writes change only BANK bits. GIR updated ~2 clocks after GSR; continuously updated (p28).
- **LCR** (b0 A3; 00H): 7 DLAB; 6 SBK set break (TxD low regardless of activity until cleared); 5 PM2; 4 PM1; 3 PM0; 2 SBL0; 1 CL1; 0 CL0 (p19).
  - Parity (Table 12; SPF = TMD bit2): PM0=0 → none; PM0=1,SPF=0: PM2 PM1 = 00 odd, 01 even, 10 high (mark), 11 low (space); PM0=1,SPF=1,PM2=0,PM1=0 → software parity (bit from TxF.SP; received parity bit reported in RxF.RPE).
  - Stop bits (Table 13; SBL2:SBL1 = TMD bits1:0, SBL0 = LCR bit2), 16x / 1x: 000 4/4, —; 001 6/4 if 5-bit chars else 8/4, —; 010 3/4, 1; 011 4/4, 1; 100 5/4, 1; 101 6/4, 1; 110 7/4, 1; 111 8/4, 2. Rx identifies ≥3/4 stop bit. (Default all-zero = 1 stop.) In 1x mode only 1 or 2 stops (p9).
  - Char length (Table 14; NBCL = TMD bit5): 0 00 5; 0 01 6; 0 10 7; 0 11 8; 1 00 9 bits.
- **MCR** (b0 A4 / b1 A4 write; 00H): 7:6 reserved; 5 OUT0; 4 LC loopback control (type per IMD.LEM); 3 OUT2; 2 OUT1; 1 RTS; 0 DTR. Output pins = complement of bit (1 → pin low). Affect multifunction pins only when configured as GP outputs (p35).
- **LSR** (b0 A5; 60H): 7 0; 6 TxST = GSR.TxIR (TX machine idle: empty or disabled); 5 TFST = GSR.TFIR (TX FIFO ≤ threshold); 4 BkD; 3 FE; 2 PE; 1 OE; 0 RFIR (RX FIFO above threshold; forced low during any read from RX FIFO; writing 0 acknowledges RX FIFO interrupt). Reading LSR clears RST bits 7:1 and LSR bits 4:1 (p29).
- **MSR** (b0/b1 A6; 00H): 7 DCDC complement of DCD pin (if input per PMD); 6 DRIC complement of RI (if input); 5 DSRC complement of DSR (if input); 4 CTSC complement of CTS; 3 DDCD changed; 2 DRI high-to-low transition on RI; 1 DDSR; 0 DCTS. Reading MSR clears the delta bits; delta bits are the MIR source (p32). Table 5 service also allows writing 0 into MSR 3:0.
- **ACR0** (b0 A7) / **ACR1** (b2 A5): 8-bit compare character. Normal mode: programmed control char (right-justified, leading zeros if <8 bits); match → RST.PCRF. µLAN: station address; only address chars compared (p21).
- **RXD** (read A0, b0/b1): oldest char in RX FIFO, right-justified, leading zeros; reading loads next FIFO entry into RXD/RXF (p36).
- **TXD** (write A0): pushes TXD + current TXF into TX FIFO (p35).
- **RXF** (b1 A1 read, per-char flags loaded with RXD): 7 —; 6 ROK char OK (no parity/framing error; parity not included in S/W parity mode); 5 RxN noisy (16x only: non-identical samples on ≥1 bit); 4 RPE parity error (S/W parity mode: received parity bit as-is); 3 ACR address/control char marker (control char normal mode; address char µLAN); 2 BKF char is part of a break sequence; 1 RFE no stop bit (FE also generated for the first char of a break sequence); 0 RND 9th data bit (0 for <9-bit chars) (p31).
- **TXF** (b1 A1 write): 7 µLAN address marker bit; 6 SP software parity bit; 5 D8 ninth data bit; 4:0 reserved (p36).
- **TMST** (b1 A3 read; 30H): 7:6 —; 5 GBS gate B state; 4 GAS gate A state (TMCR gates; no interrupt); 3:2 —; 1 TBEx timer B expired; 0 TAEx timer A expired (p32).
- **TMCR** (b1 A3 write): 7:6 0; 5 TGB gate B (1 enable counting); 4 TGA gate A; 3:2 0; 1 STB start (trigger) timer B; 0 STA start timer A. No effect when BRG in BRG mode. TGA/TGB are levels (not auto-reset) (p34).
- **FLR** (b1 A4 read; 00H): 7 —; 6:4 RX FIFO occupancy 0–4; 3 —; 2:0 TX FIFO occupancy 0–4 (p31).
- **RST** (b1 A5 read; 00H): 7 CRF control/address char received (µLAN: address char; normal: standard ASCII/EBCDIC control char); 6 PCRF programmed char matched (ACR0/ACR1); 5 BkT break terminated; 4 BkD break detected (RxD low for one char frame plus a stop bit); 3 FE; 2 PE; 1 OE (char lost because RX FIFO full); 0 RFIR (= LSR.0). Cleared on read except bit0 (p30).
- **RCM** (b1 A5 write, command, self-clearing): 7 RxE receive enable; 6 RxDI receive disable; 5 FRM flush RX machine (reset RX logic, enables reception, unlocks RX FIFO); 4 FRF flush RX FIFO; 3 LRF lock RX FIFO (received chars discarded, status still reported); 2 ORF open/unlock RX FIFO; 1:0 0 (p34).
- **TCM** (b1 A6 write): 7:4 0; 3 FTM flush TX machine (reset TX logic, not regs/FIFO, and enable transmission); 2 FTF flush TX FIFO; 1 TxEN; 0 TxDI disable (current char completes) (p33).
- **GSR** (b1 A7 read; 12H): 7:6 —; 5 TIR; 4 TxIR (TX machine empty or disabled/idle); 3 MIR; 2 RxIR; 1 TFIR (TX FIFO ≤ threshold); 0 RFIR (RX FIFO > threshold) (p28).
- **ICM** (b1 A7 write): 7:5 0; 4 SRST software reset (identical to hardware reset except strapping options; lasts 4 clocks; → default wake-up mode); 3 INTA manual interrupt acknowledge (INT forced inactive for 2 clocks, then reasserts if other enabled interrupts pending); 2 STC status clear (clears RST, MSR, TMST); 1 PDM power down; 0 0 (p33).
- **FMD** (b2 A1; 00H): 7:6 0; 5:4 RFT1:0 RX FIFO threshold (interrupt when occupancy > value); 3:2 0; 1:0 TFT1:0 TX FIFO threshold (interrupt when occupancy ≤ value) (p21).
- **TMD** (b2 A3; 00H): 7 EED error echo disable (echo mode: don't echo chars with errors); 6 CED control/address char echo disable; 5 NBCL 9-bit chars; 4:3 TM1:TM0 transmit mode: 00 manual, 01 reserved, 10 semi-automatic, 11 automatic; 2 SPF software parity; 1 SBL2; 0 SBL1 (p20, bit1/bit0 labels as printed).
- **IMD** (b2 A4; 0CH): 7:4 reserved 0; 3 IAM: 0 manual ack, 1 automatic ack; 2 RFD RX FIFO depth: 0 = four bytes, 1 = one byte (8250A emulation); 1 ULM: 0 normal, 1 µLAN; 0 LEM: with MCR.LC=1: 0 local loopback, 1 remote loopback; with LC=0: 0 no echo, 1 auto echo (p18).
- **RIE** (b2 A6; 1EH): 7 CRE (RST.CRF); 6 PCRE (RST.PCRF); 5 BkTE; 4 BkDE; 3 FEE; 2 PEE; 1 OEE; 0 reserved (p26).
- **RMD** (b2 A7; 00H): 7 µCM0, 6 µCM1 (figure p22 labels bit7 = µCM0, bit6 = µCM1) — µLAN mode: 00 manual (address chars reported via RST.CRF and written to FIFO), 01 semi-automatic (also OPENs RX FIFO on any address char; user locks), 10 automatic (OPEN on address match, LOCK on mismatch), 11 reserved; normal mode: 00 no standard control chars recognized (programmed always are), 01 ASCII 00H–1FH + 7FH, 10 reserved, 11 EBCDIC 00H–3FH. 5 DPD disable DPLL; 4 SWM sampling window: 0 = 3/16, 1 = 7/16; 3 SSM start-bit sampling: 0 majority vote, 1 all samples must be 0; 2:0 reserved (p22).
- **CLCF** (b3 A0 DLAB=0; 00H): 7 RxCM 0=16x, 1=1x (RX clock = SCLK pin); 6 RxCS (16x): 0 BRGB, 1 BRGA; 5 TxCM 0=16x, 1=1x (SCLK); 4 TxCS: 0 BRGB, 1 BRGA; 3:0 0 (p23).
- **BACF** (b3 A1 DLAB=0; 04H): 6 BACS BRGA source: 0 system clock, 1 SCLK pin (ignored in timer mode); 2 BAM: 0 timer mode (source always system clock), 1 BRG mode; others 0 (p23).
- **BBCF** (b3 A3; 84H): 7:6 BBCS1:0 BRGB source: 00 system clock, 01 SCLK pin, 10 BRGA output, 11 reserved (ignored in timer mode); 2 BBM: 0 timer, 1 BRG; others 0 (p24).
- **BAL/BAH** (b0 A0/A1 DLAB=1) BRGA divisor/count LSB/MSB; **BBL/BBH** (b3 A0/A1 DLAB=1) BRGB.
- **PMD** (b3 A4; FCH): 7 DIOD DCD/ICLK/OUT1 direction; 6 DIOF its function; 5 DTAD DSR/TA/OUT0 direction; 4 DTAF its function; 3 RRF: 0 SCLK, 1 RI; 2 DTF: 0 TB, 1 DTR; 1:0 0. Direction 0 = output, 1 = input (DCD/DSR). Function (output only) 0 = special (ICLK / TA), 1 = GP (OUT1 / OUT0 via MCR) (p25).
- **MIE** (b3 A5; 0FH): 3 DCDE; 2 RIE (ΔRI); 1 DSRE; 0 CTSE; 7:4 0 (p27).
- **TMIE** (b3 A6; 00H): 1 TBIE; 0 TAIE; others 0 (p27).

### Behavior
- **System clock** (p8, p38): crystal on X1/X2 (parallel resonant, 4–18.432 MHz) or external TTL clock on CLK. Hardware strap: during RESET, RTS is an input with weak pull-up; if driven low (e.g. open collector from RESET) → crystal oscillator mode; else external clock (pin 8 becomes OUT2). External-clock default: system clock = CLK/2; strapping DTR low during reset disables the ÷2. Strapping both DTR and RTS is forbidden. Crystal mode: system clock = crystal/2 (Table 7 note). Strap timing: TTLRSL RTS/DTR low ≥ 6 Tcy before RESET falls; hold 0..Tcy−20 after (p38); RTS/DTR driven high internally during RESET. System clock (after ÷2) must be ≥ 16× TX or RX baud.
- **BRGs** (p11–12): two 16-bit counters; fo = fin/divisor; counts down by 2, reload & toggle at 2 → 50% duty; odd divisors count down by 1 first time (duty deviation 1/(2·divisor)). Divisor 1..65535; 3 → 33%, 2 → 50%, 1 = source, 0 forbidden. Sources: BRGA ← system clock or SCLK; BRGB ← system clock, SCLK or BRGA output (cascade). TX and RX 16x clocks each from BRGA or BRGB; 1x clock from SCLK only. Default wake-up wiring: BACF 04H → BRGA in BRG mode from system clock, divisor BAL/BAH (default 2); BBCF 84H → BRGB in BRG mode clocked by BRGA output, BBL=05H; CLCF 00H → TX and RX 16x clocks from BRGB. So 16x clock = sysclk / (BAL:BAH × 5); with 18.432 MHz crystal (sysclk 9.216 MHz) that equals 1.8432 MHz / divisor — exact 8250 divisor compatibility (derived from the default values, p6 + p23–24). Table 7 (p12) standard 16x divisors from 9.216 MHz: 110 5236 (.007%), 300 1920, 1200 480, 2400 240, 9600 60, 19200 30, 38400 15, 56000 10 (2.8%), 288000 2 — these are single-BRG divisors (not the cascaded 8250 path).
- Writing a BRG configuration register resets that BRG; mode/source changes only after hardware/software reset; divisor may be updated during operation unless the BRG clocks an active serial machine (p12).
- **Timers** (p12): BRG in timer mode counts system clock from divisor down to 1; at terminal count sets TAEx/TBEx (maskable interrupt) and can pulse TA/TB pins. Delay = Count × system clock period. Started by TMCR STA/STB; restart by re-trigger; gated by TGA/TGB. Count 1 forbidden in timer mode.
- **TX** (p9): TX FIFO 4 × 11-bit (9 data, parity, address flag). TX FIFO interrupt when level ≤ threshold. One-char delay between FIFO empty and TX idle (threshold 0 is safe). Writing >4 chars → extra char dropped, error not reported. TX machine enable/disable via TCM or CTS; disable mid-char completes the char. Handshake modes (TMD.TM): manual (CTS/RTS ignored by TX; CPU via MCR/MSR); semi-automatic (RTS asserted while transmitter enabled; TX only when CTS active; CTS inactive mid-char → finish char then idle); automatic (as semi-auto but RTS asserted only while enabled AND data to send; RTS deasserts when shift register and FIFO empty). CPU can force RTS active via MCR regardless. 1x TX clock: synchronous to SCLK; TXD valid ≤ 170 ns after SCLK low (p40).
- **RX** (p10–11): RX FIFO 4-deep (or 1-deep, IMD.RFD — default 1 = one byte); interrupt when level > threshold. Overrun: FIFO full + new char → **oldest char discarded**, new char loaded, OE set in RST and LSR. Lock/unlock via RCM (µLAN filtering per RMD modes). Sampling: digital filter + voting counter; majority of samples in window 3/16 or 7/16 (Fig 11: 4-bit DPLL counter, windows centred on count ~8); non-unanimous → RxN noisy flag. DPLL resyncs on edges (disable via RMD.DPD). Start bit: falling edge resets DPLL counter; verified by majority or absolute (all samples) voting; false start → back to search. Break: input low for entire frame incl. stop → BkD + FE in RST and LSR, NULL char with FE and break flag pushed into RX FIFO, RX goes idle; first mark → BkT set; resume. 1x RX: RXD setup/hold 250 ns to SCLK rising (p40).
- **Control char recognition** (p11 Table 6): standard ASCII set (00–1FH + 7FH) or EBCDIC (00–3FH) per RMD, plus ACR0/ACR1 user chars → CRF/PCRF interrupts.
- **µLAN / 9-bit** (p10, p22): MCS-51 9-bit protocol; address chars = address marker bit set (TxF bit7 on transmit); RX compares address chars with ACR0/ACR1.
- **Diagnostics** (p12–13): Local loopback (MCR.LC=1, LEM=0): TX output → RX input internally, TXD pin held mark; OUT1, OUT2, DTR, RTS internally → RI, DCD, DSR, CTS respectively; OUT0 pin held mark. Remote loopback (LC=1, LEM=1): RXD pin routed straight to TXD pin (≤170 ns, p40), data not passed to RX machine. Auto echo (LC=0, LEM=1): chars read out of RX FIFO automatically pushed (with flags) into TX FIFO; RX baud must be ≤ TX baud; TMD.EED/CED suppress echo of errored/control chars.
- **Interrupts** (p7–8, p18, p26–29): two-level masking (block enables in GER + per-source RIE/MIE/TMIE). Priority Timer > TX machine > RX machine > RX FIFO > TX FIFO > Modem. INT asserted when an interrupt is coded in GIR (IPN=0). Ack modes (IMD.IAM): automatic — any service operation acts as ack; INT forced low two clocks then updated; manual — CPU must write ICM.INTA; INT low ≥2 clocks then reasserts if pending. Service table (Table 5 p8): Timers — read TMST; TX machine — write char to TX FIFO; RX machine — read RST or LSR, or write 0 to bit in RST/LSR; RX FIFO — write 0 to LSR/RST bit0 or read char; TX FIFO — write to FIFO, or read GIR when TX FIFO is the pending interrupt (enters **TX FIFO Hold State**: no TX FIFO interrupts until TFIR drops by writing a char or TFIE cleared — emulates 8250 THRE clear-on-IIR-read, p29); Modem — read MSR or write 0 into MSR bits 3:0. Status Clear (ICM.STC) clears RST, MSR, TMST.
- **Power down** (p13): ICM.PDM. Sleep (crystal: automatic; ext clock: user also stops clock): registers not preserved; exit by hardware reset or by reading FLR then software reset; must reprogram (allow oscillator start-up ~1 ms). Idle (ext clock still running): registers preserved; reading FLR exits. FLR data read on exit is invalid.
- **Reset**: RESET pin high (≥ 8 Tcy with CLK; 1 ms for crystal start-up) or ICM.SRST → default wake-up values above; bank 0 selected; RTS/DTR pins driven high during reset (input-strap sampling); 8250A/16450-compatible mode.

### Timing (p38–40; 0..70 °C, VCC 5 V ±10%; ns; Tcy = system clock period)
- System clock, ÷2 active: input period (Tcy/2) 54–250, low/high 25 min, rise/fall 10 max, crystal 4.0–18.432 MHz. ÷2 inactive: Tcy ≥ 108, low ≥ 54, high 44–250, rise/fall 15.
- Reset: TRSHL reset width 8 Tcy min (CLK mode); TTLRSL 6 Tcy; TRSLTX 0..Tcy−20.
- Read: TRLRH RD width ≥ 2Tcy+65; TAVRL addr/CS setup 7; TRHAX hold 0; TRLDV data valid ≤ 2Tcy+65; TCIAD command inactive→active ≥ Tcy+15; TRHDZ float ≤ 40.
- Write: TWLWH WR width ≥ 2Tcy+15; TAVWL 7; TWHAX 0; TDVWH data setup 90; TWHDX hold 12.
- SCLK 16x mode: period ≥ 216, low/high ≥ 93, rise/fall 15. SCLK 1x mode: period ≥ 3500, low/high ≥ 1650. RXD 1x setup/hold to SCLK high 250/250. TXD 1x valid after SCLK low ≤ 170. Remote loopback RXD→TXD ≤ 170.
- Max baud 288k (p2) = 9.216 MHz/32.

### VCC
4.5–5.5 V, 0..70 °C (p37). Inputs abs max VCC+0.5 V (not tolerant above VCC). VIH 2.0 V, VIL 0.8 V. ICC 3.8 mA/MHz; power-down 2 mA (clock running) / 500 µA standby.

### Package drawings
None in PDF (refers to Intel packaging spec 231369). plcc28: JEDEC 28-lead PLCC 0.450" square, 0.050" pitch. dip28: 600 mil, 0.100" pitch.

### Obsolete
None (all Active).

### Notes
- Receive/transmit logic diagrams (p41) show worked RST/FLR sequences (RST 04H after programmed CC, 44H/5CH during break, etc.).
- Default RX FIFO depth is 1 (IMD=0CH) and auto-ack enabled — i.e. 8250 behaviour at reset.
- MSR bit 7 is named "DCD Input Inverted" (p15) and "DCD Complement" (p16) — same meaning.

## CDP65C51 (and CDP65C51A)

- Title: "CDP65C51, CDP65C51A CMOS Asynchronous Communications Interface Adapter (ACIA)", Harris, January 1991, File Number 2747 (book pages 5-12..5-29).
- Datasheet: `datasheets/controller-uart/cdp65c51.pdf` (18 pp, scanned, no text layer; read from page images). PDF p1 = features/pinout, p2 ratings/DC, p3–4 pin descriptions, p5–6 registers, p7–13 operation waveforms, p14 status/program-reset/misc notes, p15 divisor table, p16 loop-back, p17 bus timing, p18 TX/RX timing. **No ordering-information table and no package outline drawings in this PDF.**
- CDP65C51 vs CDP65C51A: identical except CTS handling (p1, p8–9): see Behavior/CTS. Treat as ONE device with a variant flag `ctsCompletesChar` (A=true).

### Parts
Datasheet package letters (p1): "D" = 28-lead hermetic dual-in-line sidebrazed ceramic, "E" = 28-lead plastic DIP, "M" = 28-lead dual-in-line small-outline (SOIC). Speed types: "-1, -2, -4" = 1 MHz, 2 MHz, 4 MHz φ2 (cycle time 1 µs/0.5 µs/0.25 µs, p1, p17). Temp: D −55..+125 °C, E and M −40..+85 °C (p2).
| census part | variant | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|---|
| CDP65C51M1 (Harris, Obsolete) | CDP65C51 (non-A) | M = 28-SOIC (DigiKey 0.295"/7.50 mm) | 28 | so28 (same pin numbering as DIP) | -1: 1 MHz, −40..+85 °C | 3–6 V (char. at 5 V±5%) |
| CDP65C51ASM2-96 (Harris, Obsolete) | CDP65C51A | **"SM" not defined in this PDF**; DigiKey says 28-SSOP/QSOP 0.154" | 28 | ssop28 (assume DIP numbering — unverified) | -2: 2 MHz assumed; -96 = tape & reel (Harris convention, not printed here) | 3–6 V |
| CDP65C51M4S2357 (Intersil, Active) | CDP65C51 (non-A) | M = 28-SOIC | 28 | so28 | -4: 4 MHz, −40..+85 °C; "S2357" suffix undecoded (custom/screening code) | 3–6 V |
- Flags: **CDP65C51ASM2-96** — SSOP package and "SM" letter code not in this datasheet; pinout for SSOP not printed (only one "Package types D, E and M" pinout, p1). **CDP65C51M4S2357** — suffix S2357 undecoded.

### Pinout (p1, "Package types D, E and M", top view — same for all three)
```
 1 VSS ground          15 VDD power
 2 CS0 in (active high) 16 DCD in (active low)
 3 CS1 in (active low)  17 DSR in (active low)
 4 RES in (active low)  18 D0 inout
 5 RXC inout            19 D1 inout
 6 XTLI in              20 D2 inout
 7 XTLO out             21 D3 inout
 8 RTS out (act. low)   22 D4 inout
 9 CTS in (act. low)    23 D5 inout
10 TXD out              24 D6 inout
11 DTR out (act. low)   25 D7 inout
12 RXD in               26 IRQ out (active low, open drain)
13 RS0 in               27 PHI2 in (φ2 clock)
14 RS1 in               28 RW in (R/W: 1=read, 0=write)
```
RXC (pin 5) is bidirectional: input = receiver 16x clock when Control bit4 (RCS) = 0; output = receiver 16x clock (baud-rate generator output) when RCS = 1 (p4, p7 Fig 6). No NC pins.

### Host interface (p3, p5, p17)
- 6500/6800-style: φ2 clock + R/W. Selected when CS0 = 1 AND CS1 = 0 (p3). RS1:RS0 select register. Data transfers happen during φ2 high (p5 "Timing and Control"). Write: data latched on φ2 falling edge (Fig 22, tDCW setup / tHW hold to φ2 fall). Read: data valid tCDR after φ2 rise, held tHR after φ2 fall; bus high-Z otherwise.
- Address/CS/RW set up tAC/tWC before φ2 rising, hold tCAH/tCWH after φ2 falling.
- Register map (Table I, p3):

| RS1 | RS0 | Write | Read |
|---|---|---|---|
| 0 | 0 | Transmit Data Register | Receiver Data Register |
| 0 | 1 | Programmed Reset (data "don't care") | Status Register |
| 1 | 0 | Command Register | Command Register |
| 1 | 1 | Control Register | Control Register |

### Registers
**Status register** (read RS=01, Fig 3 p5). Hardware reset: `0 - - 1 0 0 0 0` (b7=0, b6/b5 = DSR/DCD pin levels, b4=1, b3..0=0). Program reset: only b2 (overrun) cleared, others unchanged.
- b0 Parity error*; b1 Framing error*; b2 Overrun*; (*no interrupt for these conditions)
- b3 RDRF Receiver Data Register Full: 1 when char moved from RX shift register to RDR; 0 when processor reads RDR.
- b4 TDRE Transmitter Data Register Empty: 1 when TDR moved to TX shift register; 0 when processor writes TDR.
- b5 DCD: 0 = DCD pin low (detect), 1 = high (not detected).
- b6 DSR: 0 = DSR pin low (ready), 1 = high (not ready).
- b7 IRQ: 1 = interrupt has occurred (IRQ pin low); cleared to 0 by reading the Status Register.
- PE/FE/OE are checked when RDR read (p5): they reflect the last data word; FE set at the time the RX interrupt occurs and re-evaluated for every word (p11 Fig 15). Parity bit is stripped and not stored in RDR (p5).
- DCD/DSR (p5): bits reflect pin levels. On a change of either input, an interrupt occurs immediately (unless processor interrupts disabled: Command bit0 = 0). When the interrupt occurs, bits show the levels immediately after the change; subsequent level changes do not alter the bits until the Status Register is read; at that time, another interrupt immediately occurs if levels changed and bits reflect new levels (p5, p12 Fig 16).

**Control register** (RS=11 R/W, Fig 4 p6). Hardware reset 00h; program reset: unchanged.
- b3..b0 SBR baud select: 0000 = 1/16x external clock (on XTLI); 0001 50; 0010 75; 0011 109.92; 0100 134.58; 0101 150; 0110 300; 0111 600; 1000 1200; 1001 1800; 1010 2400; 1011 3600; 1100 4800; 1101 7200; 1110 9600; 1111 19200 (with 1.8432 MHz).
- b4 RCS receiver clock source: 0 = external receiver clock (RXC input, 16x); 1 = baud rate (internal BRG; RXC becomes 16x output).
- b6:b5 WL word length: 00 = 8, 01 = 7, 10 = 6, 11 = 5 bits.
- b7 SBN: 0 = 1 stop bit; 1 = 2 stop bits, except 1½ stop bits for WL=5 with no parity, and 1 stop bit for WL=8 with parity.

**Command register** (RS=10 R/W, Fig 5 p6). Hardware reset 00h; program reset: b4..b0 ← 0, b7..b5 unchanged.
- b0 DTR: 0 = not ready (DTR pin high), and "the receiver and transmitter are both disabled"; also disables all selected interrupts. 1 = ready (DTR pin low), enables.
- b1 IRD receiver interrupt control: 0 = RX interrupt (RDRF) enabled; 1 = disabled.
- b3:b2 TIC: 00 RTS high, TX interrupt disabled; 01 RTS low, TX interrupt enabled; 10 RTS low, TX interrupt disabled; 11 RTS low, TX interrupt disabled, transmit BREAK on TXD. Note (p6): changing from 01 to 10 may generate a break; always go 01→00→10.
- b4 REM receiver echo mode: 0 normal; 1 echo. Bits 3 and 2 must be zero for echo mode; RTS will be low.
- b5 PME parity mode enable: 0 = no parity bit generated / no check; 1 = enabled.
- b7:b6 PMC parity mode: 00 odd TX/RX; 01 even TX/RX; 10 mark parity bit transmitted, check disabled; 11 space parity transmitted, check disabled.

**TDR/RDR** (RS=00): bit0 first transmitted/received (LSB first). Unused TDR bits don't-care; unused RDR high bits read 0; parity not in RDR (p5).

### Behavior
- **Baud generator** (p7 Fig 6, p15 Table II): XTLI/XTLO crystal (1.8432 MHz nominal) or external clock on XTLI with XTLO floating (p3, p14). Baud = f(XTLI)/divisor; divisors: 0001 36864, 0010 24576, 0011 16768, 0100 13696, 0101 12288, 0110 6144, 0111 3072, 1000 1536, 1001 1024, 1010 768, 1011 512, 1100 384, 1101 256, 1110 192, 1111 96; 0000 = no divisor, TX clock = XTLI/16 (external 16x clock on XTLI). (16x clock = f/(divisor/16).) Transmitter always uses BRG/XTLI; receiver uses BRG (RCS=1, RXC outputs the 16x clock) or RXC input /16 (RCS=0).
- **Transmitter**: start, 5–8 data LSB first, optional parity, stop bit(s). TDRE set when TDR transfers to shift register; interrupt (if TIC=01) "at the beginning of the Start Bit" (p7 Fig 7). If TDR not loaded by end of stop bit, TXD stays mark; when finally loaded, start bit begins immediately (p8 Fig 9) — note Fig 9 shows TDRE interrupts continuing at character rate while idle. Break (TIC=11): continuous break sent after both TDR and shift register empty; on return to normal, a stop bit is generated then normal transmission (p13 Fig 18).
- **CTS** (p4, p8–9): TX automatically disabled while CTS high. **CDP65C51 (non-A)**: TXD goes to mark *immediately* when CTS goes high (current character truncated); TDRE interrupts continue at the same rate; status register doesn't indicate TDR empty (Fig 10, p8). **CDP65C51A**: completes the character currently being shifted out, then mark; IRQ not asserted again until CTS goes low; normal transmission resumes when CTS low (Fig 10A, p9). No status bit for CTS. CTS has no effect on receiver.
- **Receiver**: RDRF + IRQ (if IRD=0) "at about the 8/16 point through the Stop Bit" (p7 Fig 8); with 1½ stop bits, halfway through the trailing half-stop bit (p12 Fig 17). PE/OE/FE updated at that time. Overrun: if RDR not read before next char's interrupt, new char not transferred; RDR keeps last valid char; OE set; all following data lost until read (p9 Fig 11). Start-bit validation: if RXD goes low then high during first 9 receiver clocks after a stop bit, a false start bit → not received (p14 item 6). Receive continuous break: interrupt with framing error (parity/overrun normal); then no more interrupts; receiver disabled until first stop bit (mark) encountered (p13 Fig 19). Framing error does not inhibit receiver; cleared if next word OK (p11).
- **Echo mode** (REM=1, TIC=00; p10 Fig 12–14): TXD retransmits RXD delayed by ½ bit time. CTS affects echoing same as transmitter. On overrun in echo, TXD goes to mark until first start bit after RDR read.
- **Command bit0 = 0** (p14 item 2): all interrupts disabled incl. DCD/DSR; receiver disabled but a char currently being received completes first; transmitter disabled after both TDR and shift register empty.
- **DCD** (p12 Fig 16): change causes interrupt and status; Fig 16 text: after DCD goes low, "no interrupt will occur here since receiver is not enabled until first start bit detected". While DCD high, receiver data still received? Fig 16 shows RX interrupts not occurring while DCD high ("As long as DCD is high no further interrupts for receiver will occur") → DCD high disables receiver interrupts. DCD/DSR transitions have no effect on transmitter (p14 item 8). DCD/DSR inputs must not float.
- **Interrupts**: IRQ open-drain active low (p3). Sources: TDRE (TIC=01), RDRF (IRD=0), DCD change, DSR change — all gated by Command b0 = 1. PE/FE/OE/no own interrupt. Status read clears b7 and releases IRQ (p14 "Status register operation": read clears bit 7; subsequent DSR/DCD transitions cause another interrupt). Suggested service order p14.
- **Programmed reset** (write RS=01, p14): registers not completely cleared (see above); DTR goes high immediately; RX and TX interrupts disabled immediately — if IRQ low at reset it stays low until serviced unless caused by DCD/DSR; DCD/DSR interrupts disabled immediately (if IRQ low due to them it goes high; status bits continue to follow pins, no interrupt); overrun cleared.
- **Hardware reset** RES low (p3): Command and Control cleared (→ DTR high, RTS high, all interrupts disabled, 1 stop/8 bits/ext clock); Status ← DSR, DCD per pins, TDRE=1, else 0. Required after power-up. Min pulse tRES (p18).
- **Loop-back**: no internal loop-back mode; external circuit (Fig 21, p16) only. Remote loop-back via echo mode with RCS=1, REM=1, TIC=10, IRD=0 per p16 list (note p16 says Command bit1 must be 0 "to disable receiver interrupts" — contradicts Fig 5 where 0 = enabled; datasheet error).
- **Misc** (p14): echo mode → RTS low. Odd parity = total 1s (data+parity) odd. Received parity bit not stored. Full duplex.

### Timing
Bus (p17, VDD 5 V±5%, −40..+85 °C, CL 75 pF), min unless noted, values for -1 / -2 / -4:
- tCYC 1 / 0.5 / 0.25 µs; φ2 pulse width tC 400 / 200 / 100 ns
- tAC address (CS0, CS1, RS0, RS1) setup 120 / 60 / 30 ns; tCAH hold 0 / 0 / 0
- tWC R/W setup 120 / 60 / 30; tCWH hold 0 / 0 / 0
- tDCW data setup (write) 120 / 60 / 35; tHW data hold 20 / 10 / 5
- tCDR read access **max** 200 / 150 / 50 ns; tHR read hold 20 / 10 / 10; tCDA bus active time (invalid data) 40 / 20 / 10 (min)
Serial (p18): tCCY TX/RX clock period (XTLI or RXC) min 400* / 325 / 250 ns (*baud = 1/(16·tCCY)); tCH, tCL 175 / 145 / 110 ns min; tDD XTLI→TXD max 500 / 410 / 315 ns; tDLY φ2→DTR/RTS max 500 / 410 / 315; tIRQ φ2→IRQ clear max 500 / 410 / 315; tRES min 400 / 300 / 200 ns. tr/tf 10–30 ns.

### VCC
Recommended operating 3–6 V (p2, −40..+85 °C). Electrical tables at 5 V±5%. Input range VSS..VDD (abs max VDD+0.5) → not tolerant above VDD. VIH 2.0 V (XTLI/XTLO 3.0 V), VIL 0.8 V. IRQ output leakage spec'd at 5 V (open drain).

### Package drawings
Not in PDF. Known from DigiKey census: M1 = 28-SOIC 7.50 mm body (JEDEC MS-013 style, 1.27 mm pitch); ASM2-96 = 28-SSOP/QSOP 3.90 mm body (0.635 mm pitch QSOP) — a package not described in this datasheet.

### Obsolete
CDP65C51ASM2-96, CDP65C51M1.

### Notes
- Datasheet inconsistency p16 vs Fig 5 on Command bit1 polarity (Fig 5: 0 = receiver interrupt enabled).
- Fig 9 shows TDRE interrupts recurring each character time even with nothing to send — modelers must decide (65C51 real silicon quirk).

## 82C52

- Title: "82C52 CMOS Serial Controller Interface" — single-chip UART + Baud Rate Generator, DC–16 MHz (1 Mbaud with 16x clock). Intersil FN2950 Rev 4.00, Nov 25 2015.
- Datasheet: `datasheets/controller-uart/82c52.pdf` (21 pp, text layer). Pinout p2, pin description p3–4, registers p5–9, interrupt structure p9, DC p11, AC p12–13, UART timing characterization p14–17. **No package outline drawings in this PDF** (only drawing numbers in the ordering table, p1).

### Parts (ordering information p1; "C/I/M" prefix letter = temp; "P/S/D/R" = package; "96" suffix = tape & reel; "Z" = Pb-free)
| census part | maker | package (drawing) | pins | pinout | grade | VCC |
|---|---|---|---|---|---|---|
| CS82C52 | Harris | PLCC, N28.45 | 28 | plcc28 | C: 0..+70 °C | 4.5–5.5 V |
| CS82C5296 | Intersil (Obsolete) | PLCC tape & reel, N28.45 (p1 row "CS82C5296 … PLCC (Tape & Reel)") | 28 | plcc28 | C: 0..+70 °C | 4.5–5.5 V |
| ID82C52 | Renesas | CERDIP, F28.6 | 28 | dip28 (600 mil) | I: −40..+85 °C | 4.5–5.5 V |
| IP82C52 (Intersil, Obsolete) | | PDIP, E28.6 | 28 | dip28 (600 mil) | I: −40..+85 °C | 4.5–5.5 V |
| IP82C52 (Harris, Active) | | PDIP, E28.6 | 28 | dip28 | I: −40..+85 °C | 4.5–5.5 V |
| IS82C52R2490 | Harris | PLCC (base IS82C52, N28.45) | 28 | plcc28 | I: −40..+85 °C | 4.5–5.5 V |
| IS82C52Z | Renesas | PLCC Pb-free, N28.45 | 28 | plcc28 | I: −40..+85 °C | 4.5–5.5 V |

- **IS82C52R2490**: suffix "R2490" is not in the ordering table (p1). Base IS82C52 = industrial PLCC; the R2490 suffix is undecoded (likely a custom/reel code). Flag.
- All grades: speed is the same (16 MHz max clock, "1M BAUD" column heading p1). Military M82C52 (MD/MR, CLCC J28.A) not in census.
- Only one speed grade; all parts electrically identical except temp (VIH 2.0 V C/I, 2.2 V M; p11).

### Pinouts (p2) — PDIP/CERDIP and PLCC/CLCC use the **same pin numbers** (PLCC pin 1 = RD at top centre)
```
1 RD in        8 D5 inout     15 SDO out     22 TBRE out
2 WR in        9 D6 inout     16 GND ground  23 RST in (Schmitt)
3 D0 inout    10 D7 inout     17 CTS in      24 INTR out
4 D1 inout    11 A0 in        18 DSR in      25 SDI in
5 D2 inout    12 A1 in        19 DTR out     26 DR out
6 D3 inout    13 IX in (xtal/ext clock)  20 RTS out  27 VCC power
7 D4 inout    14 OX out (xtal; leave open w/ ext clk)  21 CO out  28 CS0 in
```
(Chip select is printed "CSO"/"CS0" — pin named CS0 with overbar; repo name `CS0` or `CS`.) No NC pins; no exposed pad.
Active levels (p3–4): RD, WR, CS0 active low; CTS, DSR, DTR, RTS active low; TBRE, DR, INTR, RST active high; D0–D7 true-high, 3-state.

### Host interface (p3, p5 Table 1, p12–13)
- Intel-style RD/WR strobes, gated by CS0 (low). A1:A0 select register. CS0, A0, A1 must be valid TSVCTL before RD/WR leading (falling) edge and held TCTHSX after trailing (rising) edge. Write data latched at WR rising edge (setup TDVWH, hold TWHDX). Data bus driven only while CS0=0 and RD=0; otherwise high-Z.
- Unused data bits for <8-bit words: ignored on write to TBR; read as 0 from RBR (p3, p8).
- Register map (Table 1, p5), CS0=0:

| A1 | A0 | WR (write) | RD (read) |
|---|---|---|---|
| 0 | 0 | TBR Transmitter Buffer Register | RBR Receiver Buffer Register |
| 0 | 1 | UCR UART Control Register (write-only) | USR UART Status Register |
| 1 | 0 | MCR Modem Control Register | MCR (read back) |
| 1 | 1 | BRSR Bit Rate Select Register (write-only) | MSR Modem Status Register |

### Registers
**UCR** (A=01 write, Fig 1 p5). Not affected by reset.
- D0 Stop bits: 0 = 1 stop bit; 1 = 1.5 stop bits (Tx) and 1 stop (Rx) if 5 data bits, else 2 stop bits for 6/7/8 data bits.
- D3:D1 Parity: 000 Tx&Rx even; 001 Tx&Rx odd; 010 Tx even, Rx odd; 011 Tx odd, Rx even; 100 Tx even, Rx check disabled; 101 Tx odd, Rx check disabled; 11X generation and check disabled (no parity bit).
- D5:D4 Word length: 00=5, 01=6, 10=7, 11=8 bits.
- D7:D6 reserved, write 00.
- Note p5: "During the Echo Mode, the transmitter always repeats the received word and parity, even when the UCR is programmed with different or no parity."

**BRSR** (A=11 write, Fig 2 p6). Not affected by reset except D7 → 0.
- D1:D0 Prescaler: 00 ÷1, 01 ÷3, 10 ÷4, 11 ÷5.
- D6:D2 Divisor select: 00000 ÷2, 00001 ÷4, 00010 ÷16/3, 00011 ÷8, 00100 ÷32/3, 00101 ÷16, 00110 ÷58/3, 00111 ÷22, 01000 ÷32, 01001 ÷64, 01010 ÷128, 01011 ÷192, 01100 ÷256, 01101 ÷288, 01110 ÷352, 01111 ÷512, 10000 ÷768, 11111 = External (÷1). Codes 10001–11110 not listed (undefined).
- D7 CO select: 0 = CO outputs buffered IX; 1 = CO outputs BRG output (16x clock). Reset → 0.
- 16x clock = f(IX) / prescaler / divisor. With 1.8432/2.4576/3.072 MHz and ÷3/÷4/÷5 → 614.4 kHz; Table 2 (p6): 38.4K=External, 19.2K=2, 9600=4, 7200=16/3, 4800=8, 3600=32/3, 2400=16, 2000=58/3 (1986.2, 0.69%), 1800=22 (1745.45, 3.03%), 1200=32, 600=64, 300=128, 200=192, 150=256, 134.5=288 (133.33), 110=352 (109.09), 75=512, 50=768. Fractional divisors (16/3, 32/3, 58/3): BRG output is 50% duty cycle "except when 'external' is selected and the Prescaler is set to ÷3 or ÷5" (p6). How fractional division distributes edges is not specified.

**MCR** (A=10 R/W, Fig 3 p7). Cleared by reset (=00h).
- D0 RTS: 1 → RTS pin low (asserted); 0 → high.
- D1 DTR: 1 → DTR pin low; 0 → high.
- D2 INTEN: 1 interrupts enabled; 0 disabled — INTR held low.
- D4:D3 Mode: 00 Normal; 01 Transmit Break (transmitter sends only break characters: start, data, stop all 0); 10 Echo; 11 Loop Test.
- D5 REN receiver enable: 0 gates off receiver input.
- D6 MIEN modem interrupt enable.
- D7 must be written 0.

**USR** (A=01 read, Fig 5 p8). Reset: all 0 except TC and TBRE = 1 (→ 60h).
- D0 PE parity error; D1 FE framing error; D2 OE overrun; D3 RBRK received break; D4 MS modem status change (any transition on CTS or DSR); D5 TC transmission complete (TBR and TR both empty); D6 TBRE TBR empty; D7 DR data ready.
- OE/FE/PE/RBRK updated with each received character "during reception of the stop bits" (p7).
- Read clears: "Reading the USR register not only clears the edge detect circuit but also clears (sets to 0) all of the status bits. The output pins associated with these status bits are not affected" (p9). (Ambiguity: TBRE/DR bits are also stated to be "updated each time TBRE/DR changes state" — p14 notes 3, 7; most likely the DR/TBRE bits mirror the pins and the clear-on-read applies to the latched flags PE/FE/OE/RBRK/MS/TC. Planner decide.)
- Status-set during read (p9 NOTE): setting of USR/MSR status bits is inhibited during a status-register read; if a condition occurs during the read, the bit is set at the trailing edge of RD. If the bit was already set at read time and the same condition reoccurs, it is cleared at the RD trailing edge instead of set again.

**MSR** (A=11 read, Fig 6 p8): D0 CTS (1 = pin low/true), D1 DSR (1 = pin low/true), D2 = 0, D3 = 0, D7:D4 undefined. Live pin images, inverted.

**RBR** (A=00 read): received data right-justified, unused high bits read 0; D0 = first bit received. **TBR** (A=00 write): D0 = first bit transmitted; unused high bits ignored.

### Behavior
- **Clocking**: IX/OX crystal oscillator (parallel-resonant fundamental 1–16 MHz, p9 Table 3) or external clock into IX (OX open). BRG = prescaler × divisor-select counters → 16x clock to TX and RX (p6). CO pin = IX buffer (BRSR7=0) or BRG 16x (BRSR7=1).
- **Transmitter**: TBR → TR (shift register); LSB first; start, 5–8 data, optional parity (Tx parity may differ from Rx), 1/1.5/2 stops. TX and RX share word length and stop count (p9). SDO held mark (high) when CTS false, when RST true, when TR empty, or in Loop mode (p3). TBRE pin high when TBR transferred to TR; cleared by write to TBR; set by reset. TC set when TBR and TR both empty and last char's transmission done.
- **CTS**: high (false) inhibits starting a transmission; if it goes false mid-character, the current char completes (p3, p8). CTS doesn't affect Loop mode. TCTHX: CTS must go high ≥ 4·TCY+10 ns before end of last stop bit to block the next char in TBR (p14).
- **TX start timing** (p14 notes 2, 4): TR empty: TBRE goes high at 4th falling CO(BRG) edge after WR high; SDO start bit at 5th falling edge. TR full: TBRE high at trailing edge of 15th CO(BRG) in last stop bit, start bit at trailing edge of 16th, provided WR rose by 12th. CTS-held: 4th/5th falling edge after CTS goes low.
- **Receiver**: start bit detected, RX baud counter starts at start edge, sample 8 CO(BRG) periods later (mid-bit), then every 16 (Fig 17 p15). Data shifted with 1x clock synced to start bit; whole char → RBR, sets DR pin and DR bit. Double-buffered; if RBR not read before next char completes → OE (data in receiver register lost, p8). Break = all bits incl. parity and stop zero → RBRK; break char loads RBR and sets DR (p7). FE = improper stop bit(s). DR, OE/FE/PE/RBRK set at trailing edge of 11th CO(BRG) in the last stop bit; to avoid OE, RD(RBR) must go low by trailing edge of 8th CO(BRG) in last stop bit (p14 notes 5B, 7). DR pin cleared by RBR read or RST. SDI disabled in loop mode or RST.
- **Modes** (Fig 4 p7): Echo — SDI is buffered (not resynchronized) straight to SDO; normal TX disabled; receiver still receives. Loop — TX serial output routed to receiver input; SDO held mark; SDI ignored. Transmit Break — sends all-zero characters.
- **Interrupts** (Fig 9 p9): INTR = INTEN AND ( posedge-detect(RBRK|TC|OE|FE|PE in USR) OR (MIEN AND any-edge-detect(DSR,CTS)) ). USR posedge detector cleared by RD(USR); modem edge detector cleared by RD(MSR) (MSR read does not change MSR bits). TBRE and DR do NOT drive INTR (separate pins). TC set at reset → INTR fires when INTEN later enabled unless USR read first (p9). MS-USR bit is set on modem change regardless of masks (p14 note 5C). INTR is a push-pull active-high output, held low when INTEN=0.
- **Reset** (RST high, Schmitt, ≥2 IX clocks; p3, p5): resets BRG counters and bit counters; BRSR unchanged except D7←0 (CO=IX); USR ← TC=1, TBRE=1, others 0; MCR ← 0 (RTS, DTR high; INTR low; mode Normal; REN=0; MIEN=0); UCR unaffected. TBRE pin high, DR pin low, SDO mark. After RST low, device idle until programmed. No power-on default of UCR/BRSR given (undefined at power-up). Software "reset" = rewrite UCR, BRSR, MCR and read USR & RBR (p9).
- Register update rule (p5): control registers may be written any time the device is not immediately transmitting/receiving; MCR should be written last.

### Timing (p12, VCC 5 V ±10%, all grades same; max/min as printed)
- TSVCTL select (CS0,A) setup to RD/WR falling: 30 ns min. TCTHSX select hold after control rising: 50 ns min.
- TCTLCTH RD/WR pulse width 150 ns min. TCTHCTL control disable→next enable 190 ns min.
- TRLDV RD low → data valid 120 ns max (100 pF). TRHDZ RD high → data float 0–60 ns.
- TDVWH data setup to WR rising 50 ns min; TWHDX data hold 20 ns min.
- IX clock 0–16 MHz; high/low 25 ns min each, TCHCL+TCLCH ≥ 62.5 ns. CO rise/fall 15 ns max.
- Characterized (p14, bench, not guaranteed): CO(IX) from IX 30 ns; CO(BRG) from IX 80 ns; SDO from CO(BRG) low 30 ns; WR low → TBRE low 50 ns; CO low → TBRE high 50 ns; INTR high on flag 50 ns; INTR high on MS 50 ns; RD low → INTR low 60 ns; CO low → DR high 40 ns; RD low → DR low 50 ns; WR high → RTS/DTR active 50 ns.

### VCC
Operating 4.5–5.5 V (p11). Inputs: abs max VCC+0.5 V → not 5 V-tolerant beyond VCC (inputMax = VCC+0.5). VIH 2.0 V (C/I), VIL 0.8 V (TTL); RST Schmitt VCC−0.5/GND+0.5; external IX clock CMOS levels VCC−0.5/GND+0.5.

### Package drawings
Not in this PDF. Referenced codes (p1): PDIP **E28.6** (28-lead 600 mil DIP, 0.100" pitch), PLCC **N28.45** (28-lead PLCC 0.450" body, 0.050" pitch; DigiKey: 11.51×11.51 mm), CERDIP **F28.6**, CLCC J28.A. Dims must come from Intersil package drawings (not in repo PDF).

### Obsolete
CS82C5296 (Intersil), IP82C52 (Intersil row). (IP82C52 also has an Active Harris row.)

### Notes
- Pin 28 label printed "CSO" (letter O) in places; it is "CS0".
- USR clear-on-read vs DR/TBRE mirroring ambiguity (above).
- BRSR divisor codes 10001–11110 undefined.

## HD-6406

- Title: "HD-6406 CMOS Programmable Asynchronous Communication Interface" (PACI) — single-chip UART + BRG, DC–16 MHz (0–1 Mbaud), DMA or vectored interrupt mode, Harris (book pages 5-13..5-23).
- Datasheet: `datasheets/controller-uart/hd-6406.pdf` (11 pp, scanned). p1 features/pinout/block diagram, p2–3 pin description, p4–8 functional description & registers, p8 register bit summary, p9 ratings/DC, p10 AC, p11 timing diagrams. **No ordering table, no package drawings, only the 40-pin DIP pinout is printed.**
- Nearest sibling: **82C52** (same Harris SAJI IV UART/BRG core: identical UCR, BRSR, MCR, USR bit layouts and register addressing). HD-6406 adds: 40-pin package with ALE address latch, second chip select CS1 (active high), DMA acks TXDACK/RXDACK, RI and RLSD modem inputs (MSR bits 2,3), TC/OE/FE/PE status output pins, SIE/SFD interrupt-mask pins, and DR/TBRE are interrupt sources (gated by SIE). Model as a separate device (different pins), sharing the 82C52 core.

### Parts
| census part | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|
| HD3-6406-5 (Harris, Active) | "HD3" = plastic DIP per Harris prefix convention (HD1 = CERDIP, HD3 = PDIP, HD4 = LCC) — **the "3" code is not printed in this PDF**; only DIP pinout exists | 40 | dip40 (600 mil) | -5: 0..+70 °C (p9) | 4.5–5.5 V |
Grades (p9): -5 = 0..+70 °C, -9 = −40..+85 °C, -2/-8 = −55..+125 °C. Single speed (16 MHz). Flag: package digit decoded from Harris convention only.

### Pinout (DIP-40, p1; pin roles p2–3)
```
 1 CS0 in (act. low)      21 TC out (high)
 2 RD in (low)            22 RLSD in (low)
 3 WR in (low)            23 CTS in (low)
 4 D0 inout               24 DSR in (low)
 5 D1 inout               25 RI in (low)
 6 D2 inout               26 DTR out (low)
 7 D3 inout               27 RTS out (low)
 8 D4 inout               28 CO out
 9 D5 inout               29 TBRE out (high)
10 D6 inout               30 RST in (high, Schmitt)
11 D7 inout               31 SIE in (high)
12 A0 in                  32 SFD in (high)
13 A1 in                  33 INTR out (high)
14 ALE in (high)          34 SDI in
15 TXDACK in (low)        35 OE out (high)
16 RXDACK in (low)        36 FE out (high)
17 IX in                  37 PE out (high)
18 OX out                 38 DR out (high)
19 SDO out                39 CS1 in (act. high)
20 GND ground             40 VCC power
```

### Host interface (p2, p4 Table 1, p8, p10–11)
- Intel RD/WR, enabled when CS0 = 0 AND CS1 = 1 (non-DMA cycles).
- ALE (p2, p4): "ALE true enables the internal transparent address latches for the A0, A1 inputs. The address is latched when ALE goes false (low)." For demultiplexed bus tie ALE high (then A0/A1 must be held for the whole cycle). ALE pulse width 50 ns min, A setup to ALE fall 20 ns, hold 20 ns; control inactive to ALE high 20 ns (p10).
- Register map (Table 1, p4; strobe = RD/WR, data latched on WR rising edge per waveform p11):

| A1 | A0 | WR | RD |
|---|---|---|---|
| 0 | 0 | TBR | RBR |
| 0 | 1 | UCR | USR |
| 1 | 0 | MCR | MCR |
| 1 | 1 | BRSR | MSR |
- DMA (p2, p7–8): RXDACK low acts as a chip select + address override: RD while RXDACK low reads RBR "when the DR bit in the USR is true"; TXDACK low + WR writes TBR "when the TBRE bit in the USR is true". A0, A1, ALE ignored; CS0/CS1 normally inactive during DMA. Behavior if DR/TBRE false during a DACK cycle is not specified. (Pin text p2 says for RD "contents of RBR output providing the DR bit in the Modem Status Register (MSR) is true" — typo for USR.)
- D0–D7 3-state except during reads. Unused bits of <8-bit words: ignored on write, 0 on read.

### Registers (bit summary p8; figs p4–6) — identical to 82C52 except MSR
- **UCR** (write-only): D0 stop select (0 = 1 stop; 1 = 1.5 stop Tx/1 stop Rx for 5-bit, 2 stops for 6/7/8); D3:D1 parity control (000 Tx&Rx even; 001 Tx&Rx odd; 010 Tx even/Rx odd; 011 Tx odd/Rx even; 100 Tx even/Rx check disabled; 101 Tx odd/Rx check disabled; 11X generation & check disabled); D5:D4 word length 00=5..11=8; D7:D6 reserved (write 0). Echo mode repeats received word and parity regardless of UCR.
- **BRSR** (write-only): D1:D0 prescaler 00 ÷1, 01 ÷3, 10 ÷4, 11 ÷5; D6:D2 divisor 00000 ÷2, 00001 ÷4, 00010 ÷16/3, 00011 ÷8, 00100 ÷32/3, 00101 ÷16, 00110 ÷58/3, 00111 ÷22, 01000 ÷32, 01001 ÷64, 01010 ÷128, 01011 ÷192, 01100 ÷256, 01101 ÷298 (as printed; 288 in Table 2 and 82C52), 01110 ÷352, 01111 ÷512, 10000 ÷768, 11111 external (÷1); D7 CO select (0 = buffered IX, 1 = BRG 16x).
  - Table 2 (p5): 38.4K ext, 19.2K 2, 9600 4, 7200 16/3, 4800 8, 3600 32/3, 2400 16, 2000 58/3, **1800 21**, 1200 32, 600 64, 300 128, 200 192, 150 256, 134.5 288, 110 352, 75 512, 50 768; error table: 2000→1986.2 (0.69%), 134.5→133.33 (0.87%), 110→109.71 (0.26%), 1800→1828.57 (1.56%).
  - **Datasheet inconsistencies**: 00111 = ÷22 in Fig 2 but 1800 = 21 in Table 2 (1828.57 = 614400/16/21); 01101 = ÷298 in Fig 2 vs 288 in Table 2; 110 baud error 109.71 vs 82C52's 109.09 (=÷352). The 82C52 datasheet (newer) gives 22 and 288 consistently — recommend using 22 and 288.
- **MCR** (R/W): D0 RTS (1 → pin low), D1 DTR (1 → pin low), D2 INTEN, D4:D3 mode (00 normal, 01 transmit break, 10 echo, 11 loop test), D5 REN, D6 MIEN, D7 must be 0.
- **USR** (read): D0 PE, D1 FE, D2 OE, D3 RBRK, D4 MS, D5 TC, D6 TBRE, D7 DR. "Reading the USR clears all of the status bits in the USR but does not affect associated output pins" (p6, bold).
- **MSR** (read, Fig 6 p6): D0 CTS, D1 DSR, D2 RLSD, D3 RI (1 = pin low = true), D7:D4 undefined ("Not used").
- **RBR/TBR**: D0 = first bit; RBR right-justified, unused bits 0.

### Behavior (as 82C52 unless noted)
- BRG, transmitter, receiver, break, loop/echo modes (Fig 4 p5), CTS rule (finish current char), double buffering, overrun: identical text to 82C52 (p4–7).
- **Status output pins** (p3): TC high when complete char incl. stop bits transmitted and TBRE true; reset by a data write to TBR; RST sets TC. OE pin: true when RBR full when a character received (transfer does not occur); updated each time a char is transferred to RBR; RST clears. FE pin: invalid stop bits in last char, updated each transfer; RST clears. PE pin: parity mismatch, updated each char transferred to RBR; reset by RST or when no parity check programmed. DR pin: high when char in RBR; reset by RBR read or RST. TBRE pin high when TBR transferred to TR; set by RST; cleared by TBR write. These pins are NOT affected by USR reads.
- **Interrupts** (Fig 9 p7, p3): INTR = INTEN · [ SIE · posedge(DR | TBRE) + SFD̄ · posedge(RBRK | TC | OE | FE | PE) + MIEN · anyedge(RLSD, RI, DSR, CTS) ]. Edge detectors on USR cleared by RD(USR) (which also clears USR bits); modem detector cleared by RD(MSR). SIE high enables DR/TBRE as interrupt sources (single interrupt line use); SFD high prevents USR bits PE, OE, FE and TC from causing an interrupt (p3; figure also includes RBRK in that group). INTR active-high push-pull, low when INTEN = 0.
- **SDI**: disabled in loop mode, during RST, or when REN = 0 (p3).
- **SDO**: held mark when CTS false, RST true, TR empty, or loop mode (p2).
- **Reset** (RST high ≥ 2 IX clocks, p4): as 82C52 — BRG/bit counters reset, BRSR unaffected (pin text p3 says BRSR bit 7 set to 0 on reset for CO), USR cleared except TC and TBRE set, MCR cleared (RTS/DTR high, INTR low), UCR unaffected.
- Crystal (p8 Table 3): 1–16 MHz parallel-resonant fundamental; external clock on IX, OX open.

### Timing (p10, VCC 5 V ±10%, all grades)
TLHLL ALE width 50 min; TAVLL addr setup to ALE↓ 20; TLLAX addr hold 20; TSVCTL select setup to control leading edge 30; TCTHSX select hold 50; TCTLCTH RD/WR width 150; TCTHCTL control disable→enable 190; TRLDV RD↓→data valid 120 max (100 pF); TRHDZ read disable 0–60; TCTHLH control inactive → ALE high 20; TDVWH data setup 50; TWHDX data hold 20; FC 0–16 MHz (TCHCL+TCLCH ≥ 62.5 ns); TCHCL/TCLCH 25 min; IX rise/fall ≤ 1/(6FC) or 50 ns; CO rise/fall 15 max (50 pF). Select = CS0, CS1, A0, A1 or TXDACK/RXDACK (p11 diagrams). No serial/interrupt timing table in this PDF (use 82C52 characterization as guide).

### VCC
4.5–5.5 V (p9). Inputs abs max VCC+0.5 V (not tolerant). VIH 2.0 V (-5/-9), 2.2 V (-2/-8); VIL 0.8; RST Schmitt VCC−0.5/GND+0.5; IX ext clock VCC−0.5/GND+0.5.

### Package drawings
None in PDF. dip40 = standard 40-lead 600-mil plastic DIP (0.100" pitch). (Thermal data p9 mentions CERDIP and LCC packages; no LCC pinout given.)

### Obsolete
None (HD3-6406-5 Active).

### Notes
- BRSR divisor typos (22 vs 21, 298 vs 288) — see above.
- Pin 2 description references "MSR" for DR bit — should be USR.

## COM78C804 (SMC quad "scanner" UART — base design for COM78C802 and COM78804)

- Title: "COM78C804 Four-channel Universal Asynchronous Receiver/Transmitter Quad UART", Standard Microsystems Corp., PRELIMINARY (book pp 121–134). **Not a 16450/16550 register set** — SMC's own per-line MR1/MR2/Command/Status design with an on-chip interrupt scanner (same family as COM78808 octal).
- Datasheet: `datasheets/controller-uart/com78c804.pdf` (14 pp, scanned). Pinouts p1; block diagram + pin table p2; address map + bus p3; signals/architecture p4–5; status/mode/command p5–7; modes/summary regs p8; scanner p9; DC p10; bus timing p11–12; misc timing p13; waveforms p14. **No ordering table, no package drawings.**

### Parts
Ordering suffixes are not printed in the 78C80x datasheets. SMC's convention is printed in the companion COM92C451 sheet (p1: "COM92C451 LJP for PLCC, COM92C451 P for DIP"); applied here by analogy.
| census part | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|
| COM78C804LJP | PLCC (LJP, by analogy) | 44 | plcc44 | 0..70 °C (only range given, p10) | 5 V (no tolerance printed in DC table) |

### Pinouts (p1, Table 1 p2)
DIP-48:
```
 1 DCD2 in        13 WR in          25 DCD1 in       37 ADD2 in
 2 DSR2 in        14 DS in (*)      26 DSR1 in       38 DS in (*)
 3 TXD2 out       15 DL3 inout      27 TXD1 out      39 ADD3 in
 4 VDD power      16 DL2 inout      28 VSS2 ground   40 ADD4 in
 5 DL7 inout      17 DL1 inout      29 NC            41 MRST in (tie to GND)
 6 DL6 inout      18 DL0 inout      30 VDD power     42 CLK in
 7 DL5 inout      19 VDD power      31 IRQLN0 out    43 VSS1 ground
 8 DL4 inout      20 TXD0 out       32 IRQLN1 out    44 TXD3 out
 9 RDY out (OD)   21 DSR0 in        33 IRQTXRX out   45 DSR3 in
10 RESET in       22 DCD0 in        34 IRQ out (OD)  46 DCD3 in
11 VSS0 ground    23 RXD0 in        35 ADD0 in       47 RXD3 in
12 CS in          24 RXD1 in        36 ADD1 in       48 RXD2 in
```
(*) DIP pins 14 and 38 are both DS and "must be connected together" (p1).
PLCC-44:
```
 1 RXD2   2 DCD2   3 DSR2   4 TXD2   5 DL7   6 DL6   7 DL5   8 DL4   9 RDY  10 RESET  11 VSS0
12 CS    13 WR    14 DS    15 DL3   16 DL2  17 DL1  18 DL0  19 TXD0 20 DSR0 21 DCD0  22 RXD0
23 RXD1  24 DCD1  25 DSR1  26 TXD1  27 VSS2 28 VDD  29 IRQLN0 30 IRQLN1 31 IRQTXRX 32 IRQ 33 ADD0
34 ADD1  35 ADD2  36 ADD3  37 ADD4  38 MRST 39 CLK  40 VSS1 41 TXD3 42 DSR3 43 DCD3  44 RXD3
```
Roles: DL inout (3-state); ADD, CS, DS, WR, RESET, MRST, CLK, RXDn, DSRn, DCDn input; TXDn, IRQLN, IRQTXRX output (push-pull); IRQ, RDY open-drain outputs. Active-low: CS, DS, WR, RESET, RDY, IRQ, DSRn, DCDn. No exposed pad.

### Host interface (p3, p11–12)
- Data-strobe bus: CS (select), DS (strobe), WR (direction: low = write into UART, high = UART drives DL). Register selected by ADD<4:0> while DS and CS asserted. Write: CPU asserts DS when data valid and removes DS before data removed — data latched at DS rising edge (setup tDSU, hold tDHO). Read: DL driven only while CS low, DS low and WR high; drivers go high-Z within 50 ns when CS high, DS high or WR low (p3).
- RDY (open drain): asserted (low) after CS falling edge (tRDL ≤ 90 ns), released after CS rising (tRDH ≤ 210 ns = 75 + RC) (p4, p12). Purely a ready/acknowledge; not data-dependent.
- Register address map (Table 2 p3): ADD<4:3> = line number (0–3), ADD<2:0>:

| ADD2..0 | access | register |
|---|---|---|
| 000 | read | Line n Receiver Buffer |
| 000 | write | Line n Transmitter Holding |
| 001 | read | Line n Status |
| 010 | read/write | Line n Mode Registers 1,2 (alternating pointer) |
| 011 | read/write | Line n Command |
| 100 (ADD4:3 = X) | read | Interrupt Summary |
| 101 (ADD4:3 = X) | read (write-to-clear, see below) | Data Set Change Summary |
| 110, 111 | — | not listed |

### Registers (per line unless noted; all cleared by RESET unless noted)
- **Status** (read-only, Fig 3 / Table 4 p5): 7 DSR (= inverted DSR pin); 6 DCD (= inverted DCD pin); 5 FER framing error (char currently in RX buffer register lacked stop bit; only first stop bit checked; all-zero char incl. parity with FER = Break; cleared by clearing RxEN, RESET, or setting RERR); 4 ORR overrun (char in RX buffer not read before another char received; cleared by RxEN clear, RESET or RERR); 3 PER parity error (cleared by RxEN clear, RESET, RERR, **or by reading the current character in the receiver buffer register**); 2 TxEMT (serializer finished a char and no new char in THR; cleared by loading THR, clearing TxEN, or RESET); 1 RxRDY (char loaded into FIFO; cleared by reading RX buffer, clearing RxEN, RESET); 0 TxRDY (THR empty; cleared by loading THR, clearing TxEN, RESET; set when TxEN set and THR empty; NOT set in automatic-echo or remote-loopback mode; data can be overwritten if a consecutive write is performed while TxRDY is clear).
- **Mode Register 1** (Fig 4 / Table 5 p5–6): 7:6 STOP: 00 invalid, 01 1.0, 10 1.5, 11 2.0. 5:4 PAR CTRL: 11 even, 01 odd, X0 disabled. 3:2 CHAR LENGTH: 00 5, 01 6, 10 7, 11 8 (RX chars right-aligned, high bits 0; parity not shown in RX buffer). 1 RSRV. 0 MCIE modem-control interrupt enable (effective when Command.RxIE = 1).
- **Mode Register 2** (Fig 5 / Table 6 p6–7): 7:4 XMIT RATE, 3:0 RECV RATE, same code: 0000 50, 0001 75, 0010 110 (109.09), 0011 134.5 (133.33), 0100 150, 0101 300, 0110 600, 0111 1200, 1000 1800 (1745.45), 1001 2000 (2021.05), 1010 2400, 1011 3600 (3490.91), 1100 4800, 1101 7200 (6981.81), 1110 9600, 1111 19200. CLK = 4.9152 MHz. Internal divisor not printed; derive as CLK/(16·actual) — e.g. 19200 → 16, 9600 → 32, 110 → 2816, 1800 → 176, 2000 → 152, 3600 → 88, 7200 → 44, 134.5 → 2304 (computed, not printed).
- **MR1/MR2 access** (p5): same address; each access (read or write, any mix) alternates MR1 → MR2 → MR1 via an internal pointer. Pointer reset to MR1 by RESET or by a **read of that line's Command register**. Not RMW-safe.
- **Command** (R/W, Fig 6 / Table 7 p6–7): 7:6 OPER MODE: 00 normal, 01 automatic echo, 10 local loopback, 11 remote loopback. 5 RxIE: RxRDY generates an interrupt (also gates MCIE). 4 RERR: when set, clears FER/ORR/PER — **not self-clearing** (stays set until written 0 or RESET). 3 TxBRK: TxD forced to spacing at the conclusion of the char presently being transmitted; when cleared, normal operation resumes and any char pending in THR is moved to the serializer and sent; minimum break = twice the character length plus 1 bit time (78804 p7 wording), integral number of bit times. 2 RxEN: enable receiver; clearing stops assembly, clears receiver error bits and RxRDY, clears receiver interrupt conditions, initializes receiver logic (FIFO data lost). 1 TxIE: TxRDY made visible to the scanner. 0 TxEN: enable transmitter; clearing inhibits chars that follow (current char completes), clears TxRDY and TX interrupt conditions, initializes TX logic **except the THR, whose char is retained** (XON/XOFF use).
- **Receiver Buffer** (read): two-entry FIFO + assembly register ("triple buffered"); reading pops. **THR** (write).
- **Interrupt Summary** (read-only, Fig 7 / Table 8 p8): 7 IRQ (scanner found an interrupting condition; IRQ pin asserted); 6:4 read 0; 3:1 INT LINE NO — bit 2 = IRQLN1, bit 1 = IRQLN0 (bit 3 not described, presumably 0 for octal-part compatibility); 0 Tx/Rx (1 = transmitter, 0 = receiver; = IRQTxRx). Bits 3–0 are a free-running counter and only valid when bit 7 = 1.
- **Data Set Change Summary** (Fig 8 p8): bit 0 = line 0, bit 1 = line 1, bit 6 = line 2, bit 7 = line 3 (DSCHNG3..0), bits 5:2 = 0. Bit set when that line's DSR or DCD input changes state. Cleared by **writing 1** into the bit position: software reads, saves, writes the copy back (with interrupts disabled, immediately after the read). Changes twice within ~1 µs may be missed. RESET disables and initializes the change logic; after RESET deasserts, future changes are reported.

### Behavior
- **Clock/baud** (p4): CLK 4.9152 MHz ±0.1 %, 50 ± 5 % duty; all baud rates and internal clocks derive from it. Per-line independent TX and RX rates (MR2). No external baud clock option.
- **Serial**: TXD high when idle, low while TxBRK. RXD must be high ≥ ½ bit before a high-to-low transition is recognized as a start bit (p4). Frame: start, 5–8 data LSB first, optional parity, 1/1.5/2 stops. Only first stop bit checked on RX.
- **Receiver** (p4): with RxEN, deserialized chars go to the 2-entry FIFO; RxRDY set while FIFO non-empty; if FIFO full when next char completes → ORR (char in RX buffer not read before another char received). RESET or RxEN=0 initializes receiver; RxRDY cleared and buffer contents undefined.
- **Transmitter** (p4–5): with TxEN, THR → serializer whenever serializer idle. RESET clears TxRDY and loses THR; software TxEN clear keeps THR. TxEMT may assert up to two char times after the last load (p9).
- **Operating modes** (p7–8): Normal. Automatic echo — RX assembled normally (RxEN set) and RXD routed to TXD; TxEN ignored, transmitter disabled; TxRDY/TxEMT cleared; no TX interrupts. Local loopback — RXD ignored; receiver input = transmitter output; RxEN and TxEN must be set; TXD held high; rates per MR2. Remote loopback — RXD returned to TXD; nothing received/transmitted; RxRDY/TxRDY/TxEMT disabled; TxEN and RxEN held cleared.
- **Interrupt scanner** (p9): sequentially checks each line for a receiver interrupt, then each line for a transmitter interrupt. Receiver interrupt: (RxRDY & RxIE) or (DSCHNG & RxIE & MCIE). Transmitter interrupt: TxRDY & TxIE. On detection the scanner stops, asserts IRQ (open drain, low), drives IRQLN<1:0> = line and IRQTXRX = 1 for TX / 0 for RX, and updates the Interrupt Summary register. IRQ is deasserted and the scanner restarted by: reading the RX buffer or clearing RxIE (RxRDY type); clearing MCIE, RxIE, or the DSCHNG bit (modem type); loading the THR or clearing TxIE (TX type). After an RX interrupt the scan resumes from where it stopped; after a TX interrupt it restarts at position 0 (line 0 receiver) → receivers have priority. IRQLN/IRQTXRX valid only while IRQ asserted (setup tISU/hold tIHO 100 ns around IRQ edges). If modem interrupts are used, IRQ must be level-triggered (p9).
- **Modem inputs**: DSR, DCD per line → status bits 7/6 (inverted pins) and DSCHNG. No modem outputs (no RTS/DTR pins).
- **Reset**: RESET (active low, ≥ 1 µs) clears all internal status bits, MR1, MR2, Command (TxEN/RxEN/TxIE/RxIE/RERR/TxBRK/mode = 0); FIFOs/THR lost; DSCHNG logic initialized; MR pointer → MR1. p4 text says "TxD<3:0> lines are asserted" during RESET — ambiguous (TXD is active-high data; most likely driven high/mark). DS must be high tDRSU before RESET rises and tDRHO after (p13).
- MRST: manufacturing reset — tie to ground.

### Timing (p12–13, 0..70 °C; ns)
tASU ADD setup to DS↓ 30; tAHO ADD hold after DS↑ 10; tCSU CS setup to DS↓ 30; tCHO CS hold 10; tWSU WR setup to DS↓ 30; tWHO WR hold 10; tDD DS↓ → data valid 165 max (150 pF); tDDZL/ZH DS↓ → DL driven 0–165; tDDLZ/HZ DS↑ → float 50 (50 pF)/60 (100 pF)/65 (150 pF) max; tDF data hold after DS↑ 0 min; tDSU data setup to DS↑ 50; tDHO data hold 30; tDPWH DS high 450 min; tDPWLR DS low read 180–10,000; tDPWLW DS low write 130–10,000; tID DS↓ → IRQ released 635 max (= 500 + RC); tRDL CS↓ → RDY↓ 90 max; tRDH CS↑ → RDY↑ 210 max (75 + RC). Misc: tCP 203.45 (4.9152 MHz), CLK high/low 95; tRES 1000; tDRSU 900; tDRHO 1000; tDSPW DCD/DSR pulse ≥ 1000; tISU/tIHO 100; tTXSK TxD bit-width skew 250.
Note: DS low max 10 µs → dynamic logic; model can ignore.

### VCC
Single +5 V (tolerance not printed in DC table; ratings p10: 0..70 °C, max VCC +7 V, I/O pins VCC+0.3 V → inputs not tolerant above VCC). VIH 2.0, VIL 0.8 V (TTL). IDD 20 mA typ.

### Package drawings
None in this PDF. plcc44 standard JEDEC 0.050" pitch 44-PLCC (0.650" body). dip48: see COM78804 p15 outline (48-lead plastic DIP, 0.600" row).

### Obsolete
None (Active).

### Notes
- Status PER description says "setting reset error RERR (bit 2 of the command register)" — RERR is bit 4 (datasheet typo).
- Interrupt Summary bit 3 not described.

## COM78C802

- Title: "COM78C802 Two-channel Universal Asynchronous Receiver/Transmitter Dual UART", SMC, PRELIMINARY (book pp 107–120). Datasheet `datasheets/controller-uart/com78c802.pdf` (14 pp, scanned), same page layout as COM78C804.
- **Same design as COM78C804 restricted to two lines** (Table 2 p3): ADD3 selects line (0/1), ADD2..0 as COM78C804, summary registers at ADD = X100/X101. Register formats, modes, scanner, timing (Table 10 p12, Table 11 p13) identical in value to COM78C804. Differences:
  - Interrupt Summary (Table 8 p8): 7 IRQ; 6:2 read 0; 1 INT LINE NO (= IRQLN0); 0 Tx/Rx. Bits 1–0 free-running, valid when bit 7 set.
  - Data Set Change Summary (Fig 8 p8): bits 1:0 = DSCHNG1:0, bits 7:2 = 0.
  - Only one line-number output IRQLN0 (Table 3 p4: 0 → line 0, 1 → line 1).
  - IDD 15 mA typ (p10).

### Parts
| census part | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|
| COM78C802P | DIP (P, by SMC convention from COM92C451 p1) | 40 | dip40 | 0..70 °C | +5 V |
| COM78C802 | **no suffix — package undecodable** (datasheet offers DIP-40 and PLCC-44) | ? | ? | 0..70 °C | +5 V |

### Pinouts (p1, Table 1 p2)
DIP-40:
```
 1 DCD1 in     11 CS in        21 DCD0 in     31 ADD1 in
 2 RXD1 in     12 WR in        22 RXD0 in     32 ADD2 in
 3 NC          13 DS in        23 NC          33 ADD3 in
 4 DL7 inout   14 DL3 inout    24 VSS2 ground 34 MRST in
 5 DL6 inout   15 DL2 inout    25 VDD power   35 CLK in
 6 DL5 inout   16 DL1 inout    26 IRQLN0 out  36 VSS1 ground
 7 DL4 inout   17 DL0 inout    27 NC          37 NC
 8 RDY out OD  18 NC           28 IRQTXRX out 38 NC
 9 RESET in    19 TXD0 out     29 IRQ out OD  39 TXD1 out
10 VSS0 ground 20 DSR0 in      30 ADD0 in     40 DSR1 in
```
PLCC-44 (same footprint positions as COM78C804, unused channels NC):
```
 1 NC   2 NC   3 NC   4 NC   5 DL7  6 DL6  7 DL5  8 DL4  9 RDY 10 RESET 11 VSS0
12 CS  13 WR  14 DS  15 DL3 16 DL2 17 DL1 18 DL0 19 TXD0 20 DSR0 21 DCD0 22 RXD0
23 NC  24 NC  25 NC  26 NC  27 VSS2 28 VDD 29 IRQLN0 30 NC 31 IRQTXRX 32 IRQ 33 ADD0
34 ADD1 35 ADD2 36 ADD3 (*) 37 ADD3 (*) 38 MRST 39 CLK 40 VSS1 41 TXD1 42 DSR1 43 DCD1 44 RXD1
```
(*) PLCC pins 36 and 37 are both ADD3 and must be connected together (p1, Table 1). Table 1 lists DCD<1:0> at DIP 1,21 and VSS at DIP 10,24,36 / PLCC 11,27,40.

### Package drawings: none in PDF. ### Obsolete: none.

## COM78804

- Title: "COM78804 Four-channel Universal Asynchronous Receiver/Transmitter Quad UART", SMC, PRELIMINARY (pages numbered 1–15). Datasheet `datasheets/controller-uart/com78804.pdf` (15 pp, scanned). NMOS predecessor of COM78C804.
- **Register set, address map (Table 2 p3), modes, scanner, interrupt-summary and data-set-change formats identical to COM78C804** (verified p3, p6–8). DIP-48 pinout identical to COM78C804 DIP (p1; DS on pins 14 and 38 tied). "Also available in PLCC" (p1) but no PLCC pinout printed. Differences:
  - DC (p10): IDD 100 mA typ; abs max positive voltage on any I/O pin +0.7 V (as printed — likely VCC+0.7). No operating VCC tolerance printed.
  - Bus timing (Table 10 p12): **tDSU data setup to DS↑ = 0 ns min** (COM78C804: 50); all other values identical.
  - Table 1 (p2) misprints RxD<3:0> as "outputs" (they are inputs).
  - Only document in this group with a package outline: **48 LEAD PLASTIC DIP PACKAGE OUTLINE** (p15, inches): A length 2.437–2.457; B body width 0.520–0.550; C body thickness 0.145–0.155; D lead width 0.016–0.021; E 0.520–0.630; F shoulder width 0.048–0.060; G pitch 0.090–0.110 (0.100 nominal); H end lead offset 0.065–0.090; J lead thickness 0.007–0.014; K seating-to-lead-tip 0.120–0.140; L row span (outside leads) 0.610–0.675; M 0.210–0.250 (overall height). Row spacing 0.600" nominal.

### Parts
| census part | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|
| COM78804P | DIP (P, SMC convention; p15 outline) | 48 | dip48 | 0..70 °C | +5 V |
### Obsolete: none.

## COM92C451

- Title: "COM92C451 Combination UART/Printer Interface", SMC, ©1990, Rev E 2/90 (16 pp). CMOS. Datasheet `datasheets/controller-uart/com92c451.pdf` (scanned). Pinouts + ordering p1; TOC p2; pin functions p3–4; system p5; register access table p6; PIA p7; BRG/LCR p7–8; LSR p9; MCR/MSR p10; IIR/IER/SCR p11; DC p12; timing p13–16. No package drawings.
- Two independent blocks: **ACE** = 16450/82C50A-compatible UART (no FIFO, no FCR), selected by CE1; **PIA** = IBM-PC-compatible Centronics printer port (data/status/control), selected by CE0.
- Nearest siblings: any 16C450 (ACE register set identical in layout); differs by having no BAUDOUT/RCLK/OUT1/DDIS/CSOUT pins, a single CLK input (no crystal oscillator), and an ACE interrupt output IRQ1 not gated by OUT2.

### Parts (ordering p1: "COM92C451 LJP for PLCC, COM92C451 P for DIP")
| census part | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|
| COM92C451P | DIP | 48 | dip48 | 0..70 °C | 4.75–5.25 V (5.0 V +5%, p12) |

### Pinouts (p1, p3–4)
DIP-48:
```
 1 RESET in (high)   13 A1 in          25 ERROR in (low)   37 SOUT out
 2 IOW in (low)      14 A2 in          26 GND ground       38 CE1 in (low)
 3 IOR in (low)      15 IRQ0 out (high, 3-state) 27 PD7 inout 39 CLK in
 4 D0 inout          16 CE0 in (low)   28 PD6 inout        40 IRQ1 out (high)
 5 D1 inout          17 SLC out (low, open drain) 29 PD5 inout 41 OUT2 out (low)
 6 D2 inout          18 INIT out (low, open drain) 30 PD4 inout 42 RTS out (low)
 7 D3 inout          19 AUTOFD out (low, OD) 31 PD3 inout  43 DTR out (low)
 8 D4 inout          20 STROBE out (low, OD) 32 PD2 inout  44 CTS in (low)
 9 D5 inout          21 BUSY in (high) 33 PD1 inout        45 DSR in (low)
10 D6 inout          22 ACK in (low)   34 PD0 inout        46 DCD in (low)
11 D7 inout          23 PE in (high)   35 GND ground       47 RI in (low)
12 A0 in             24 SLCT in (high) 36 SIN in           48 VCC power
```
PLCC-68 (for reference; no census part uses it):
```
 1 NC  2 RESET 3 IOW 4 IOR 5 D0 6 D1 7 D2 8-11 NC 12 D3 13 D4 14 D5 15 D6 16 D7 17 A0 18 NC 19 A1 20 A2
21 IRQ0 22 CE0 23 SLC 24 INIT 25-28 NC 29 AUTOFD 30 STROBE 31 BUSY 32 ACK 33 PE 34 SLCT 35 NC 36 ERROR
37 GND 38 PD7 39 PD6 40 PD5 41 PD4 42-45 NC 46 PD3 47 PD2 48 PD1 49 PD0 50 GND 51 SIN 52 NC 53 SOUT
54 CE1 55 CLK 56 IRQ1 57 OUT2 58 RTS 59-62 NC 63 DTR 64 CTS 65 DSR 66 DCD 67 RI 68 VCC
```

### Host interface (p3, p5–6, p14)
- Intel IOR/IOW. CE0 low selects PIA, CE1 low selects ACE; chip enable must be low and stable for the whole access. Write data latched on IOW rising edge. D0–D7 high-Z when not reading.
- **CE0 = CE1 = 0 with IOW = 0 → ACE Baud Rate Test Mode** (not further described). CE0 = CE1 = 0 without IOW low is illegal (p3).
- Register map (Table 1 p6). PIA (CE0=0, CE1=1; A2 = X): A1A0 = 00 Data (R/W), 10 Control (R/W), 01 Status (R). ACE (CE0=1, CE1=0): A2A1A0 = 000 RBR (R, DLAB 0) / THR (W, DLAB 0) / DLL (R/W, DLAB 1); 001 IER (R/W, DLAB 0) / DLM (R/W, DLAB 1); 010 IIR (R); 011 LCR (R/W); 100 MCR (R/W); 101 LSR (R); 110 MSR (R); 111 SCR (R/W). "Combinations other than the ones shown are illegal" (no FCR; no writes to LSR/MSR/IIR; PIA A1A0=11 undefined).

### PIA registers (p7)
- **Data** (R/W): write latched at IOW rising and driven on PD0–PD7 (non-inverting); read returns PD0–PD7 pin levels. Cleared by RESET. No direction-control bit (port always driving; PC/XT-style).
- **Control** (R/W; bits 0–4 latched on IOW rising; bits 0–4 cleared by RESET; bits 5–7 read 1, not writable): bit0 → STROBE pin inverted (read: STROBE pin level inverted); bit1 → AUTOFD inverted (read: pin inverted); bit2 → INIT not inverted (read: INIT pin level); bit3 → SLC inverted (read: pin inverted); bit4 = PIA interrupt enable (IRQ0 enabled; interrupt occurs when ACK received; otherwise IRQ0 high-Z). Outputs SLC/INIT/AUTOFD/STROBE are open-drain, active low, high-Z when not active.
- **Status** (R; not cleared by RESET): bits 2:0 read 0; bit3 = ERROR pin; bit4 = SLCT pin; bit5 = PE pin; bit6 = ACK pin; bit7 = complement of BUSY pin.
- IRQ0 timing (Fig 9 p16): IRQ0 rises ≤ 35 ns after ACK falls (when enabled); clear condition not stated (waveform shows it remaining high after ACK returns high). RESET floats IRQ0 (≤ 55 ns) and drives STROBE/AUTOFD/SLC inactive-high and INIT low (≤ 65 ns) — consistent with control = 0.

### ACE registers (p7–11) — 16450 layout
- **Baud generator**: CLK input DC–10 MHz, divisor 1–65,535 (DLM:DLL) → 16x clock; max 625 kbaud. Loading either latch immediately loads the 16-bit counter. Divisor 0 → ÷3; 1 → baud clock is the inverse of CLK; 2 → ÷2, 50 % duty; ≥3 → ÷N, low for two counts, high for the rest. Tables 4/5 (p8): 1.8432 MHz: 50 2304, 75 1536, 110 1047, 134.5 857, 150 768, 300 384, 600 192, 1200 96, 1800 64, 2000 58, 2400 48, 3600 32, 4800 24, 7200 16, 9600 12, 19200 6, 38400 3, 56000 2; 3.072 MHz: 50 3840, 75 2560, 110 1745, 134.5 1428, 150 1280, 300 640, 600 320, 1200 160, 1800 107, 2000 96, 2400 80, 3600 53, 4800 40, 7200 27, 9600 20, 19200 10, 38400 5, 56000 3. RX and TX share the BRG (no RCLK pin, despite the feature bullet "Independent Receiver Clock Input").
- **LCR** (R/W, cleared by RESET): 1:0 word length 5/6/7/8; 2 stop: 0 = 1, 1 = 1.5 (5-bit) or 2 (6–8 bit); 3 PEN; 4 EPS (1 even); 5 stick parity (parity bit transmitted/checked as the opposite of bit 4, i.e. EPS=1 → 0, EPS=0 → 1); 6 set break (SOUT spacing until cleared); 7 DLAB.
- **LSR** (R; "cleared at power up by RESET" as printed): 0 DR (reset on RBR read); 1 OE (previous char destroyed; reset on LSR read); 2 PE; 3 FE (resync: assumes error was the next start bit, samples it twice and accepts the data); 4 BI (SIN spacing longer than a full word; after break SIN must be 1 for ≥ ½ bit before restart); 2–4 reset on LSR read; 5 THRE (reset when THR loaded); 6 TEMT (THR and TSR empty); 7 always 0. Reset value of THRE/TEMT after RESET: datasheet says LSR cleared; real 16450 gives 60H — flag.
- **MCR** (R/W, cleared by RESET): 0 DTR (1 → pin low); 1 RTS; 2 BIT2 (loopback-only, no pin — the "OUT1" position); 3 OUT2 (1 → pin low); 4 LOOPBACK; 7:5 read 0. Loopback: SOUT forced mark; SIN disconnected; TSR → RSR internally; modem inputs disconnected; RTS→CTS, DTR→DSR, BIT2→RI, OUT2→DCD internally; RTS, DTR, OUT2 pins forced inactive (high); interrupts fully operational with modem sources from MCR bits.
- **MSR** (R, cleared by RESET): 0 DCTS; 1 DDSR; 2 TERI (RI changed low→high logic, i.e. "from a logic '0' to logic '1'" of the RI input); 3 DDCD; 4 CTS; 5 DSR; 6 RI; 7 DCD (complements of pins; loopback: = MCR RTS, DTR, BIT2, OUT2). Deltas reset on MSR read. CTS has no effect on the transmitter; DCD has no effect on the receiver (p4).
- **IIR** (R, cleared at reset → as printed; bit0 "1" = no interrupt): 0 = 0 when interrupt pending; 2:1 ID; 7:3 = 0. Priority (Table 11 p11): 11 receiver line status (OE, PE, FE, BI; reset by LSR read); 10 received data available (reset by RBR read); 01 THR empty (reset by reading IIR when it is the source, or writing THR); 00 modem status (CTS, DSR, RI, DCD; reset by MSR read). IIR frozen during the read access; updated after.
- **IER** (R/W, cleared by RESET): 0 RX data available; 1 THRE; 2 RX line status; 3 modem status; 7:4 read 0. Disabling all inhibits IIR and IRQ1.
- **SCR** (R/W) scratchpad, cleared by RESET.
- **IRQ1**: active high; asserted when an enabled interrupt is pending. No OUT2 gating (OUT2 is just a pin). Whether IRQ1 is 3-stated when no interrupt is not stated explicitly (p3 says "enabled by setting the appropriate bits in the IER").

### Reset (p3)
RESET high: clears PIA Data and Control registers and all ACE registers except RBR, THR and divisor latches. Outputs: SOUT mark (LCR break bit 0), RTS/DTR/OUT2 high, IRQ1 low/inactive, IRQ0 high-Z, STROBE/AUTOFD/SLC high (released), INIT low, PD = 00h.

### Timing (p13–16; 0..70 °C, VCC 5 V +5 %)
- Read (Fig 5): address setup 30, CE setup 20, IOR width ≥ 100, IOR→data valid ≤ 70, IOR↑→float 0–70, address hold 20, CE hold 20, read cycle ≥ 140.
- Write (Fig 6): address setup 30, CE setup 20, IOW width ≥ 100, data setup 50, data hold 20, address hold 20, CE hold 20, write cycle ≥ 140.
- CLK (Fig 7): period ≥ 100 (10 MHz), low ≥ 40, high ≥ 60, rise/fall ≤ 5.
- TX (Fig 3): stop bit → THRE interrupt 8 baud clocks (16x clocks); read IIR → reset interrupt ≤ 150; write THR → reset interrupt ≤ 150; initial write → interrupt 16–32 baud clocks; initial interrupt reset → TX start 8–24 baud clocks.
- RX (Fig 4): stop bit → interrupt ≤ 1 baud clock; read LSR/RBR → reset interrupt ≤ 150.
- Modem (Fig 8): write MCR → output ≤ 100; modem input → IRQ1 ≤ 100; read MSR → reset interrupt ≤ 100.
- Printer (Fig 9): RESET → STROBE/AUTOFD/INIT/SLC ≤ 65; RESET → IRQ0 float ≤ 55; ACK → IRQ0 ≤ 35.

### VCC
DC table (p12): 0..70 °C, VCC 5.0 V +5 % (printed "5.0V+5%"). Pins abs max VCC+0.3 V (not 5 V tolerant above VCC). VIH 2.0, VIL 0.8. Printer data bus 24 mA sink / 3 mA source; printer control open-drain 24 mA sink. ICC 20 mA max.

### Package drawings: none in PDF (dip48 600 mil; can reuse COM78804 p15 48-DIP outline dims).
### Obsolete: none.
### Notes
- "Independent Receiver Clock Input" feature bullet has no corresponding pin.
- LSR/IIR reset values printed as "cleared" — conflicts with 16450 convention (LSR 60H, IIR 01H); IIR text itself defines bit0 = 1 as no interrupt.

