# Exar (MaxLinear) XR16V25xx / XR16V26xx DUARTs — group exar-c

All PDFs are in `datasheets/controller-uart/`. Page numbers are the PDF page numbers
(they match the printed page footers in these PDFs).

## PDF → devices

| PDF | Rev / date | Device(s) | Census parts |
|---|---|---|---|
| xr16v2550.pdf (46 p) | 1.0.3, Jul 2012 | XR16V2550 | XR16V2550IL |
| xr16v2551.pdf (49 p) | 1.0.3, Dec 2011 | XR16V2551 | XR16V2551IL, XR16V2551IM |
| xr16v2552.pdf (46 p) | 1.0.3, Mar 2010 | XR16V2552 | XR16V2552IJ |
| xr16v2650.pdf (47 p) | 1.0.2, May 2007 | XR16V2650 | XR16V2650IL, XR16V2650IM |
| xr16v2651.pdf (51 p) | 1.0.2, May 2007 | XR16V2651 | XR16V2651IL, XR16V2651IM |
| xr16v2652.pdf (48 p) | 1.0.2, May 2007 | XR16V2652 | XR16V2652IJ (DigiKey **Obsolete**), XR16V2652IL |

**Every census part decodes.** Exar suffix: `I` = industrial −40 to +85 °C (the only grade
any of these six datasheets sells), then the package letter: `L` = 32-pin QFN 5×5
(the ordering tables print `IL`, `IL32` or `IL-F`), `M` = 48-lead TQFP 7×7, `J` = 44-lead
PLCC. `-F` = green/RoHS, `TR` = tape and reel. The census names have no suffix.
All parts: VCC 2.25–3.6 V, one supply grade only. The AC tables have 2.5 V ±10 % and
3.3 V ±10 % columns.

Watermark: on every page of xr16v2551.pdf and xr16v2651.pdf there is a diagonal note,
"The product(s) mentioned in this data sheet are no longer being manufactured and may not
be ordered (OBS)". But the ordering tables in those PDFs say "Active", and the census shows
both parts Active and in stock. The watermark also breaks up the PDF text layer. I read
those pages from the rendered images where it mattered (p2, p42).

### How the six designs relate

They are **one UART core** (an enhanced 16C550 with EFR, Xon/Xoff, auto RTS/CTS, IrDA,
fractional BRG, 4X/8X/16X sampling, sleep, DVID/DREV), used two per chip. There are three
host-interface wrappers and two FIFO sizes:

| | 16-byte FIFO (DVID 0x02) | 32-byte FIFO (DVID 0x06) |
|---|---|---|
| "2550" wrapper: Intel bus, CSA#/CSB#, TXRDY#/RXRDY# A/B, OP2# A/B, MCR[3]-gated 3-state INT | **XR16V2550** | **XR16V2650** |
| "2551" wrapper: 2550 plus the 16/68# pin (Intel or Motorola bus) and the PwrSave pin (+ CLKSEL on the 2651) | **XR16V2551** | **XR16V2651** |
| "2552" wrapper: PLCC44/QFN32, single CS# plus CHSEL, AFR register, MF# pins (OP2#/BAUDOUT#/RXRDY#), INT always driven active-high | **XR16V2552** | **XR16V2652** |

The 2650/2651/2652 differ from the 2550/2551/2552 **only** in these ways: 32-byte FIFOs,
different TX/RX trigger tables, different auto-RTS and Xon/Xoff threshold tables, and
DVID = 0x06. I checked this with a word-level diff of the full text. Each 26xx part has
the same pinout as its 25xx sibling, except that the 2651 TQFP has CLKSEL on pin 25. Below,
§A gives the core once, §B gives the FIFO-size differences, and the per-device sections
give pinouts, host interface and the differences.

---

## A. Common UART core (applies to all six; per-channel, two identical channels A and B)

Cited from xr16v2550.pdf unless noted. The same text appears at shifted pages in the
others: 2650/2552/2652 are about −1 page, and 2551/2651 are about +2 pages from §2.6 on.

### A.1 Registers (2550 Table 8 p21, Table 9 p22-23, descriptions p23-35)

The address is A2..A0. Each channel has its own set. "Enhanced bits" (IER[7:4], ISR[5:4],
FCR[5:4], MCR[7:5], and all of DLD) can be written only while EFR[4]=1. When EFR[4] is
set back to 0, the values are latched and kept (p34).

| A2-A0 | Access condition | Name | R/W | Reset |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | R | xx |
| 000 | LCR[7]=0 | THR | W | xx |
| 001 | LCR[7]=0 | IER | R/W | 0x00 |
| 010 | LCR≠0xBF (2550/2551/2650/2651); LCR[7]=0 on 2552/2652 | ISR | R | 0x01 |
| 010 | same | FCR | W | 0x00 |
| 011 | (always; see Notes) | LCR | R/W | 0x00 |
| 100 | LCR≠0xBF | MCR | R/W | 0x00 |
| 101 | LCR≠0xBF | LSR | R | 0x60 |
| 110 | LCR≠0xBF | MSR | R | bits3:0=0, bits7:4 = inverted pin levels |
| 111 | LCR≠0xBF | SPR | R/W | 0xFF |
| 000 | LCR[7]=1, LCR≠0xBF | DLL | R/W | 0x01 at **power-up only**; RESET pin does not change it |
| 001 | LCR[7]=1, LCR≠0xBF | DLM | R/W | 0x00 at power-up only |
| 010 | LCR[7]=1, LCR≠0xBF, EFR[4]=1 | DLD | R/W | 0x00 |
| 010 | LCR[7]=1, LCR≠0xBF, EFR[4]=0 — **2552/2652 only** | AFR | R/W | 0x00 |
| 000 | LCR[7]=1, LCR≠0xBF, and DLL=DLM=0x00 | DREV | R | revision (0x01 = rev A) |
| 001 | LCR[7]=1, LCR≠0xBF, and DLL=DLM=0x00 | DVID | R | 0x02 (2550/2551/2552), 0x06 (2650/2651/2652) |
| 010 | LCR=0xBF | EFR | R/W | 0x00 |
| 100 | LCR=0xBF | XON1 | R/W | 0x00 |
| 101 | LCR=0xBF | XON2 | R/W | 0x00 |
| 110 | LCR=0xBF | XOFF1 | R/W | 0x00 |
| 111 | LCR=0xBF | XOFF2 | R/W | 0x00 |

DREV/DVID: "set DLL and DLM both to 0x00 (DLD = 0xXX)", then read DVID/DREV (p7, p33).
Writes to 000/001 with LCR[7]=1 still go to DLL/DLM. That is implied: the reads alias
only while DLL=DLM=0.

**IER** (p24-25)
- [0] RX data ready / RX timeout interrupt enable.
- [1] THR-empty / TX-ready interrupt enable. "If the THR is empty when this bit is
  enabled, an interrupt will be generated."
- [2] RX line status interrupt enable. An LSR[1] (overrun) interrupt fires as soon as the
  character is received. LSR[4:2] interrupts fire when the errored character reaches the
  top of the FIFO (RHR).
- [3] Modem status interrupt enable.
- [4] Sleep mode enable (enhanced bit).
- [5] Xoff / special-character interrupt enable (enhanced).
- [6] RTS# interrupt enable: fires on the RTS# pin going low→high (enhanced).
- [7] CTS# interrupt enable: fires on the CTS# pin going low→high (enhanced).

**ISR** (p25-26, Table 10 p26). Reading it returns the highest-priority pending source:

| Prio | ISR[5:0] | Source | Cleared by |
|---|---|---|---|
| 1 | 000110 | LSR (receiver line status) | reading LSR |
| 2 | 001100 | RX data timeout | reading RHR |
| 3 | 000100 | RX data ready (RHR full / FIFO ≥ trigger) | reading until the FIFO drops below the trigger |
| 4 | 000010 | TX ready (THR empty / TX FIFO below trigger) | reading ISR, or writing THR |
| 5 | 000000 | MSR | reading MSR |
| 6 | 010000 | received Xoff or special character | Xoff: reading ISR, or Xon received. Special character: reading ISR, or the next character is received |
| 7 | 100000 | CTS#/RTS# change of state (auto flow control) | reading MSR |
| – | 000001 | none (default) | |

- ISR[7:6] = 11 when the FIFOs are enabled (FCR[0]=1), else 00.
- ISR[0] = 0 means an interrupt is pending.
- RX timeout: "4 word lengths as defined by LCR[1:0] plus 12 bits time … 3.7-4.6
  character times" (p13; Interrupt Generation p25: "4-char plus 12 bits delay timer").
- 2551/2651 add a sleep **wake-up indicator** that reads as ISR low nibble 0x1. See §2551.

**FCR** (write-only; p26-27)
- [0] FIFO enable. Must be 1 when any other FCR bit is written, "or they will not be
  programmed".
- [1] RX FIFO reset. Self-clearing. Clears the pointers only; the RSR is not touched.
  Active only if [0]=1. (The 2652 adds "and FIFO level counter logic".)
- [2] TX FIFO reset. Same rules; the TSR is not touched.
- [3] DMA mode: changes the TXRDY#/RXRDY# behavior (§A.4).
- [5:4] TX trigger select (enhanced; needs EFR[4]=1).
- [7:6] RX trigger select.
- Trigger tables are in §B.

**LCR** (p27-29)
- [1:0] word length: 00=5, 01=6, 10=7, 11=8.
- [2] stop bits: 0 = 1 stop; 1 = 1.5 stops with 5-bit words, 2 stops with 6/7/8-bit words.
- [3] parity enable.
- [4] even parity (0 = odd).
- [5] forced parity: with [3]=1, [5:4]=10 sends parity 1 ("mark"), [5:4]=11 sends parity 0
  ("space"). The receiver checks the same value.
- [6] TX break: forces TX LOW (space) until cleared.
- [7] divisor latch enable.
- Writing 0xBF selects the enhanced bank.

**MCR** (p29-30)
- [0] DTR#: 1 drives the pin LOW.
- [1] RTS#: 1 drives the pin LOW. It must be 1 for auto-RTS to start.
- [2] OP1#. With IrDA on, it sets the RX polarity: 0 = RX input is active-low encoded IrDA
  data (idle low, the default); 1 = active-high (idle high). In loopback it drives MSR[6]
  (RI).
- [3] OP2#. On the 2550/2551/2650/2651: 0 = INT A/B 3-stated and OP2# HIGH (default);
  1 = INT enabled and OP2# LOW. On the 2552/2652 it controls only OP2# (on the MF# pin);
  see those sections. In loopback it drives MSR[7] (CD).
- [4] internal loopback.
- [5] Xon-Any (enhanced): any received character resumes TX. The character goes into the
  FIFO unless it is Xon/Xoff while software flow control is on.
- [6] IrDA enable (enhanced). TX idles LOW.
- [7] prescaler (enhanced): 0 = ÷1, 1 = ÷4. On the 2651 the CLKSEL pin sets it at reset
  (§2651).

**LSR** (p30-31). Reset value 0x60.
- [0] data ready: set when a character goes into RHR/FIFO, clear when it is empty.
- [1] overrun: a character arrived while the FIFO was full. The RSR is overwritten and the
  character is NOT put in the FIFO.
- [2] parity error, [3] framing error (invalid stop bit), [4] break (RX LOW for at least a
  full character frame; in FIFO mode only one break character goes into the FIFO). These
  three are tags for the character now at the top of the FIFO, updated each time RHR is
  read.
- [5] THR empty (FIFO mode: TX FIFO empty).
- [6] THR and TSR both empty.
- [7] at least one PE/FE/BI character anywhere in the RX FIFO. Clears when none remain.
- The datasheet does not say whether reading LSR clears [1]; the interrupt is "cleared by
  a read to the LSR register" (p25). The usual 16550 behavior is that reading LSR clears
  [1] and the [4:2] flags that were read. This is assumed, not stated.

**MSR** (p31-32)
- [0] ΔCTS, [1] ΔDSR, [2] TERI: RI# went **LOW→HIGH** (end of ring), [3] ΔCD.
- [4] CTS, [5] DSR, [6] RI, [7] CD: each is the complement of its pin.
- In loopback: [4]=MCR[1] (RTS), [5]=MCR[0] (DTR), [6]=MCR[2] (OP1), [7]=MCR[3] (OP2).
- DSR#/CD#/RI# "have no effect on the UART" (general-purpose inputs).
- Reading MSR clears [3:0] (implied: "a read to the MSR is required to reset the modem
  inputs", p19), and also clears the ISR priority-7 CTS/RTS interrupt.

**SPR**: scratch register, reset 0xFF, kept during sleep (p32).

**DLL/DLM/DLD** (p33). The divisor is DLM:DLL + DLD[3:0]/16.
- DLD[5:4]: 00 = 16X, 01 = 8X, 1x = 4X sampling (Table 13).
- DLD[7:6] reserved.
- Writing DLD needs EFR[4]=1.

**EFR** (p33-35)
- [3:0] software flow control (Table 14, below).
- [4] enhanced-function write enable / latch.
- [5] special-character detect: compare each received character with XOFF2. On a match
  the character still goes into the FIFO and ISR[4] is set. Interaction: with EFR[1:0]=10
  (compare Xon1/Xoff1), flow control and special character both work normally. With
  EFR[1:0]=01 (compare Xon2/Xoff2), Xoff2 is NOT put in the FIFO and raises both the Xoff
  and the special-character interrupt.
- [6] auto-RTS.
- [7] auto-CTS.
- "Whenever changing the TX or RX flow control bits, always reset all bits back to logic 0
  (disable) before programming a new setting" (p33).

Table 14 (p34), EFR[3:0] = CONT-3..0:

| [3] | [2] | [1] | [0] | Function |
|---|---|---|---|---|
| 0 | 0 | 0 | 0 | no TX and no RX flow control |
| 0 | 0 | X | X | no TX flow control |
| 1 | 0 | X | X | TX sends Xon1/Xoff1 |
| 0 | 1 | X | X | TX sends Xon2/Xoff2 |
| 1 | 1 | X | X | TX sends Xon1+Xon2 / Xoff1+Xoff2 (two-character sequence) |
| X | X | 0 | 0 | no RX flow control |
| X | X | 1 | 0 | RX compares Xon1, Xoff1 |
| X | X | 0 | 1 | RX compares Xon2, Xoff2 |
| 1 | 0 | 1 | 1 | TX Xon1/Xoff1; RX compares Xon1 **or** Xon2, Xoff1 **or** Xoff2 |
| 0 | 1 | 1 | 1 | TX Xon2/Xoff2; RX compares Xon1 or Xon2, Xoff1 or Xoff2 |
| 1 | 1 | 1 | 1 | TX Xon1+Xon2 / Xoff1+Xoff2; RX compares the Xon1+Xon2 and Xoff1+Xoff2 sequences |
| 0 | 0 | 1 | 1 | no TX flow control; RX compares the Xon1+Xon2 and Xoff1+Xoff2 sequences |

Xon/Xoff/special comparisons use only the low LCR[1:0] word-length bits (bit 0 = first
data bit received) (p17).

### A.2 Baud generator (p9-11, Fig 5, Table 5)

- XTAL1/XTAL2 oscillator or external clock → prescaler (÷1, or ÷4 when MCR[7]=1) →
  fractional divisor N = DLM:DLL + DLD[3:0]/16, from 1 to 2^16−0.0625 → sampling clock
  (16X/8X/4X per DLD[5:4]). Baud = fclk / prescaler / (N × sampling factor).
- Divisor value 0: not defined in the datasheet. The range is stated as starting at 1.
- In 8X mode an odd non-zero DLD gives ±1/16 bit-time jitter (p10). The 2551/2651 say
  "8X or 4X".
- There is no BAUDOUT pin, except the 2552/2652 MF# option.
- No clock is needed for bus cycles (p7).
- Example: 24 MHz at 16X with 115200 baud gives DLL=0x0D, DLD=0 (0.16 % error).
- Limits (AC table, p38): crystal max 32 MHz at 2.5 V and 24 MHz at 3.3 V (AC table as
  printed; text p9 says "up to 32 MHz at 2.5V"); external clock max 50 MHz at 2.5 V and
  64 MHz at 3.3 V; TECLK min period 10 ns / 7 ns.
- Max data rate: 16 Mbps at 3.3 V and 12.5 Mbps at 2.5 V, with 4X sampling.

### A.3 Transmitter / receiver (p12-14)

- TX frame: start, 5-8 data bits LSB first, optional parity, 1/1.5/2 stop bits. TX idles
  HIGH, and is HIGH during reset.
- Non-FIFO mode: LSR[5] sets when THR is moved to the TSR. LSR[6] sets when the TSR is
  empty.
- FIFO mode: the TX FIFO is the size in §B; the THR is its input. LSR[5] = FIFO empty.
  The TX interrupt fires when the FIFO level falls below the TX trigger, "or when it gets
  empty in case that the FIFO did not get filled over the trigger level on last re-load"
  (p27).
- RX start-bit check: on the falling edge, count 8 sampling clocks (4 at 8X, 2 at 4X) to
  reach mid-bit. If the line is still 0 the start bit is valid; otherwise it was a false
  start. Data and stop bits are sampled mid-bit the same way (p13).
- The RX FIFO is 11 bits wide: 8 data bits + 3 error tags (PE/FE/BI).
- Break: RX LOW for one whole frame. One break character is loaded.
- Overrun: as LSR[1] above.

### A.4 TXRDY# / RXRDY# (2550 Table 2 p8)

| Pin | FIFO off (FCR[0]=0) | FIFO on, FCR[3]=0 | FIFO on, FCR[3]=1 (DMA mode) |
|---|---|---|---|
| RXRDY# | LOW = 1 byte in RHR; HIGH = none | LOW = ≥1 byte in FIFO; HIGH = empty | goes LOW when the FIFO reaches the RX trigger or on timeout; goes HIGH when the FIFO empties |
| TXRDY# | LOW = THR empty; HIGH = byte in THR | LOW = FIFO empty; HIGH = ≥1 byte | LOW = at least 1 empty location; HIGH = FIFO full |

Reset: RXRDY# HIGH, TXRDY# LOW.

### A.5 INT pin (Tables 3/4 p9), Intel mode and 2552/2652

INT is **active HIGH**.
- TX, FIFO off: HIGH = THR empty. TX, FIFO on: HIGH = FIFO below trigger or empty.
- RX, FIFO off: HIGH = 1 byte. RX, FIFO on: HIGH = FIFO at or above trigger.
- In practice INT = (any enabled ISR source pending), cleared as in the ISR table.
- 2550/2551(16 mode)/2650/2651(16 mode): INTx is 3-stated when MCR[3]=0 and driven when
  MCR[3]=1. Reset state: "Three-State Condition".
- 2552/2652: INTA/INTB are always driven (see those sections).

### A.6 Auto RTS / CTS (p15-16)

- Auto-RTS needs EFR[6]=1 and MCR[1]=1.
  - The RX-data interrupt fires at the trigger level.
  - RTS# goes HIGH when the FIFO reaches the *next higher* trigger level.
  - RTS# returns LOW when the FIFO drops to the *next lower* level.
  - Data keeps being accepted until the FIFO is full.
  - Thresholds are in §B.
- Auto-CTS needs EFR[7]=1. CTS# HIGH stops TX after the current character's stop bit.
  CTS# LOW resumes it.
- ISR[5] fires on RTS# going LOW→HIGH (IER[6]) or CTS# going HIGH (IER[7]). Read MSR to
  clear it.
- Auto RTS/CTS is not supported in internal loopback (p20).

### A.7 Auto Xon/Xoff (p17)

- When received characters match Xoff, TX stops after the current character, and ISR[4]
  is set (if IER[5]). When Xon matches, TX resumes and ISR[4] is cleared.
- Flow-control characters are not put in the RX FIFO.
- TX side: when the RX FIFO crosses the trigger, the chip sends Xoff1/2 after "two-
  character times". It sends Xon1/2 once the RX FIFO falls below the next lower level
  (thresholds in §B).
- Xon-Any: MCR[5].

### A.8 IrDA 1.0 (p18)

- Enabled by MCR[6]=1 (needs EFR[4]=1).
- TX sends a HIGH pulse 3/16 of a bit wide for each 0 bit, and idles LOW.
- RX: each light pulse decodes to a 0 bit (Figure 11 shows the decoded data with a
  "1/16 clock delay"). The input polarity is set by MCR[2] (see MCR).
- The figure shows the pulse centred in the bit cell.

### A.9 Sleep (p19)

Sleep is entered only when **all** of these hold:
- No interrupt pending on either channel.
- IER[4]=1 on **both** channels.
- MSR[3:0]=0.
- RX pins idle HIGH.

In sleep the oscillator stops and XTAL2 has no clock. The chip wakes on any of:
- a start bit on RX (H→L);
- a write to THR/FIFO;
- any change on CTS#/DSR#/CD#/RI#.

It goes back to sleep once all interrupts are serviced (and MSR has been read if the
wake-up came from a modem pin). It stays in sleep mode until IER[4]=0. The first RX
characters after wake-up may be lost while the oscillator starts.

### A.10 Loopback (p20, Fig 12)

MCR[4]=1:
- TSR output → RSR input internally.
- TX pin held HIGH. RTS#, DTR# and OP2# pins held HIGH (the figure shows them tied to
  VCC). The 2552 MF# pin is not stated.
- CTS#, DSR#, CD#, RI# inputs ignored. Internally RTS→CTS, DTR→DSR, OP1→RI, OP2→CD.
- The RX pin should be held HIGH during the test (else a false break may appear on exit).

### A.11 Reset (Table 15 p36; RESET pin p5, p7)

- RESET is active HIGH, needs a pulse > 40 ns (TRST min 40 ns). On the 2551/2651 in 68
  mode it is RESET#, active LOW.
- Registers after reset:
  - IER/FCR/LCR/MCR/EFR/DLD/XON1/XON2/XOFF1/XOFF2 = 0, ISR = 0x01, LSR = 0x60, SPR = 0xFF.
  - MSR[3:0] = 0, MSR[7:4] = inverted pins.
  - **DLL=0x01 and DLM=0x00 only at power-up; the RESET pin does not change them.**
  - AFR = 0 (2552/2652).
- Outputs after reset: TX HIGH, OP2# HIGH, RTS# HIGH, DTR# HIGH, RXRDY# HIGH, TXRDY# LOW.
  INT is 3-state on the 2550/2650, and on the 2551/2651 in 16 mode. On the 2551/2651 in
  68 mode IRQ# is HIGH (released). The 2552/2652 table gives no INT entry.
- During reset: "transmitter output held HIGH, receiver input ignored".

### A.12 Bus timing (2550 AC table p38; 70 pF; values the same at 2.5 V and 3.3 V unless split)

| Sym | Parameter | 2.5 V | 3.3 V |
|---|---|---|---|
| TAS / TAH | address setup / hold | 0 / 0 ns min | 0 / 0 |
| TCS | chip-select width | 40 min | 35 min |
| TRD | IOR# width | 40 min | 35 min |
| TDY | read/write cycle delay (gap between strobes) | 40 min | 35 min |
| TRDV | data access after IOR# ↓ | **35 max** | **30 max** |
| TDD | data float after IOR# ↑ | 25 max | 25 max |
| TWR | IOW# width | 40 min | 35 min |
| TDS / TDH | data setup / hold to IOW# ↑ | 10 / 3 min | 10 / 3 |
| TWDO | IOW# to modem output | 50 max | 50 |
| TMOD | modem input → INT set | 50 max | 50 |
| TRSI | IOR# → INT reset (MSR) | 50 max | 50 |
| TSSI | stop bit → INT set | 1 Bclk max | 1 |
| TRRI | IOR# → INT reset (RX) | 45 max | 45 |
| TSI | stop → INT (TX) | 45 ns max | 45 |
| TINT | initial INT reset → TX start | 8–24 Bclk | 8–24 |
| TWRI | IOW# → INT reset | 45 max | 45 |
| TSSR | stop → RXRDY# set | 1 Bclk max | 1 |
| TRR | IOR# → RXRDY# reset | 45 max | 45 |
| TWT | IOW# → TXRDY# set | 45 max | 45 |
| TSRT | centre of start → TXRDY# reset | 8 Bclk max | 8 |
| TRST | reset pulse | 40 min | 40 |

Bclk = the 16X/8X/4X sampling clock.
- Read cycle (Fig 15 p40): address valid TAS before CS#/IOR# ↓, data valid TRDV after
  IOR# ↓, data floats TDD after IOR# ↑.
- Write cycle (Fig 16): data is latched on the IOW# rising edge.

### A.13 Supply / input levels (DC table p37)

- VCC 2.25–3.6 V, TA −40..+85 °C.
- VIH max 5.5 V on all logic inputs, so the inputs are 5 V tolerant: **inputMax 5.5 V**.
- **Except XTAL1**: VIHCK max = VCC. The 2551/2552/2650 text says "XTAL1 is not 5V
  tolerant … maximum at the pin should be VCC".
- VIL max 0.5 V (2.5 V) / 0.7 V (3.3 V). VIH min 1.8 / 2.0 V.
- ISLEEP 15 µA (2.5 V) / 30 µA (3.3 V).

## B. FIFO-size differences

| | 2550 / 2551 / 2552 (16 B) | 2650 / 2651 / 2652 (32 B) |
|---|---|---|
| FIFO depth TX/RX | 16 / 16 | 32 / 32 |
| DVID | 0x02 | 0x06 |
| TX trigger FCR[5:4] 00/01/10/11 | 1 (def) / 4 / 8 / 14 | **16 (def) / 8 / 24 / 30** |
| RX trigger FCR[7:6] 00/01/10/11 | 1 (def) / 4 / 8 / 14 | 8 (def) / 16 / 24 / 28 |
| Auto-RTS: RTS# HIGH at / LOW at (RX trig 1,4,8,14) | 4/0, 8/1, 14/4, 14/8 | (RX trig 8,16,24,28): 16/0, 24/8, 28/16, 28/24 |
| Xoff sent at / Xon sent at | 1/0, 4/1, 8/4, 14/8 | 8/0, 16/8, 24/16, 28/24 |

Sources:
- 2550: Table 11 p27, Table 6 p15, Table 7 p17.
- 2650: Table 11 p27, Table 6 p15, Table 7 p17.
- 2551 and 2651 (Table 12 p29) and 2552 and 2652 (Table 11 p27) have the same tables as
  their FIFO-size sibling.

---

## XR16V2550 — High Performance DUART with 16-Byte FIFO

xr16v2550.pdf.

**Parts**
- XR16V2550IL: 32-pin QFN 5×5×0.9 (ordering p2, "32-QFN"), pinout `qfn32`, industrial
  −40..+85 °C, 2.25–3.6 V.

**Pinout qfn32** (p2, p3-5). The exposed centre pad is GND and must be soldered (p5).
No TXRDY#/RXRDY#/DTR#/DSR#/CD#/RI#/OP2# pins in this package.

| # | name | role |
|---|---|---|
| 1 | D6 | inout |
| 2 | D7 | inout |
| 3 | RXB | input |
| 4 | RXA | input |
| 5 | TXA | output |
| 6 | TXB | output |
| 7 | CSA | input |
| 8 | CSB | input |
| 9 | NC | nc |
| 10 | XTAL1 | input |
| 11 | XTAL2 | output |
| 12 | IOW | input |
| 13 | GND | ground |
| 14 | IOR | input |
| 15 | RTSB | output |
| 16 | CTSB | input |
| 17 | NC | nc |
| 18 | A2 | input |
| 19 | A1 | input |
| 20 | A0 | input |
| 21 | INTB | output |
| 22 | INTA | output |
| 23 | RTSA | output |
| 24 | RESET | input |
| 25 | CTSA | input |
| 26 | VCC | power |
| 27 | D0 | inout |
| 28 | D1 | inout |
| 29 | D2 | inout |
| 30 | D3 | inout |
| 31 | D4 | inout |
| 32 | D5 | inout |
| EP | GND | ground |

The `tqfp48` pinout is listed under XR16V2650 (same pins). The 2550 TQFP is not in the
census.

**Host interface** (p3, p7-8)
- Intel bus. CSA# and CSB# are each active low. Table 1: both pins 0 selects both
  channels, which is allowed for writes only. The datasheet says "do not attempt to read
  from both".
- A2..A0 select the register.
- Read: the IOR# falling edge starts the cycle; data is driven while CS# and IOR# are both
  low.
- Write: the IOR#/IOW# falling edge starts the cycle; data is latched on the **rising
  edge** of IOW#.
- D7..D0 are 3-stated unless CSx# and IOR# are both low.
- No ALE and no DDIS.
- INTA/INTB: active-high, 3-stated by MCR[3]=0 (§A.5).
- OP2A#/OP2B# (TQFP only) = NOT MCR[3].

**Package drawing**: "PACKAGE DIMENSIONS (32 PIN QFN - 5 X 5 X 0.9 mm)" p45.
- e 0.50 BSC; b 0.18–0.30 (mid 0.24); L 0.35–0.45 (mid 0.40).
- D=E 4.90–5.10 (5.00); A 0.80–1.00; A1 0–0.05; A3 0.15–0.25.
- Exposed pad D2 3.50–3.80 (mid 3.65; "device-dependent, typical tolerance 0.3 mm");
  k ≥ 0.20.
- No land pattern printed. PCB pad advice (p5): thermal pad about the size of the
  centre pad, solder-mask defined, mask opening ≥ 0.0025" inside the pad edge.

The TQFP drawing (p44) is the same as XR16V2650's.

**Obsolete**: none.

---

## XR16V2650 — High Performance DUART with 32-Byte FIFO

xr16v2650.pdf.

Same pins, bus interface, registers and behavior as the XR16V2550 (pin descriptions p3-5
are word-for-word the same). The differences are in §B: 32-byte FIFOs, its own trigger
and threshold tables, and DVID 0x06.

**Parts**
- XR16V2650IL: 32-pin QFN (ordering p2 "XR16V2650IL32 32-Pin QFN"), pinout `qfn32`
  (identical to the 2550 qfn32 above), −40..+85 °C, 2.25–3.6 V.
- XR16V2650IM: 48-lead TQFP 7×7 (ordering p2), pinout `tqfp48`, −40..+85 °C, 2.25–3.6 V.

**Pinout tqfp48** (p2, p3-5; same as the 2550). No exposed pad.

| # | name | role |
|---|---|---|
| 1 | D5 | inout |
| 2 | D6 | inout |
| 3 | D7 | inout |
| 4 | RXB | input |
| 5 | RXA | input |
| 6 | TXRDYB | output |
| 7 | TXA | output |
| 8 | TXB | output |
| 9 | OP2B | output |
| 10 | CSA | input |
| 11 | CSB | input |
| 12 | NC | nc |
| 13 | XTAL1 | input |
| 14 | XTAL2 | output |
| 15 | IOW | input |
| 16 | CDB | input |
| 17 | GND | ground |
| 18 | RXRDYB | output |
| 19 | IOR | input |
| 20 | DSRB | input |
| 21 | RIB | input |
| 22 | RTSB | output |
| 23 | CTSB | input |
| 24 | NC | nc |
| 25 | NC | nc |
| 26 | A2 | input |
| 27 | A1 | input |
| 28 | A0 | input |
| 29 | INTB | output |
| 30 | INTA | output |
| 31 | RXRDYA | output |
| 32 | OP2A | output |
| 33 | RTSA | output |
| 34 | DTRA | output |
| 35 | DTRB | output |
| 36 | RESET | input |
| 37 | NC | nc |
| 38 | CTSA | input |
| 39 | DSRA | input |
| 40 | CDA | input |
| 41 | RIA | input |
| 42 | VCC | power |
| 43 | TXRDYA | output |
| 44 | D0 | inout |
| 45 | D1 | inout |
| 46 | D2 | inout |
| 47 | D3 | inout |
| 48 | D4 | inout |

**Package drawings**

"PACKAGE DIMENSIONS (48 PIN TQFP - 7 X 7 X 1 mm)" p43:
- e 0.50 BSC; B (lead width) 0.17–0.27 (mid 0.22); L 0.45–0.75 (mid 0.60).
- D (tip span) 8.80–9.20 (9.00); D1 (body) 6.90–7.10 (7.00).
- A 1.00–1.20; A1 0.05–0.15; A2 0.95–1.05; c 0.09–0.20.
- No land pattern.

QFN32: p44, the same dimensions as the 2550.

**Timing**: the AC table (p37) is identical to the 2550's (the header says
VCC = 2.25–3.6 V).

**Obsolete**: none.

---

## XR16V2551 — High Performance DUART with 16-Byte FIFO and PowerSave Feature

xr16v2551.pdf. Watermarked "no longer manufactured (OBS)"; see top.

This is the 2550 core and registers (identical, DVID 0x02) plus:
1. The **16/68#** pin, which selects the Intel or the Motorola bus.
2. The **PwrSave** pin.
3. A sleep **wake-up interrupt indicator**.

**Parts**
- XR16V2551IL: 32-pin QFN (ordering p2), pinout `qfn32-2551`, −40..+85 °C, 2.25–3.6 V.
- XR16V2551IM: 48-lead TQFP (p2), pinout `tqfp48-2551`, −40..+85 °C, 2.25–3.6 V.

**Pinouts** (p2 image, p3-6). Same as the 2550 qfn32 / tqfp48, except:
- qfn32: pin 9 = **PWRSAVE** (input), pin 17 = **16/68#** (input). The pad is GND.
- The printed name is `16/68#` (HIGH = Intel "16" mode, LOW = Motorola "68" mode). If
  `/` is not legal in a pin name, the planner picks one, e.g. `SEL16`.
- tqfp48: pin 12 = **PWRSAVE** (input), pin 24 = **16/68#** (input). Pins 25 and 37 = NC.
  (Rev 1.0.3 history: "Corrected 'CLKSEL' pin to NC". Block diagram p1 still shows
  CLKSEL.)

Pins whose function depends on 16/68# (p3-6; Fig 2 p2):

| pin (qfn/tqfp) | 16/68#=1 (Intel) | 16/68#=0 (Motorola) |
|---|---|---|
| 14/19 | IOR (read strobe, active low) | unused; tie to VCC |
| 12/15 | IOW (write strobe, active low) | R/W (HIGH = read, LOW = write) |
| 7/10 | CSA (active low) | CS (active low, the single chip select) |
| 8/11 | CSB (active low) | A3: channel select, 0 = A, 1 = B |
| 22/30 | INTA (active high, 3-state via MCR[3]) | IRQ: **device** interrupt, active LOW, **open drain**, needs an external pull-up |
| 21/29 | INTB | not used: Fig 2 labels it "NC (INTB)" and Fig 3 shows "(no connect)" |
| 24/36 | RESET (active high) | RESET (active LOW) |

In 68 mode **MCR[3] must be 0** (MCR[3] description p31: "if 16/68# pin selects Motorola
bus interface mode, this bit must be set to logic 0"). The datasheet does not say
explicitly that IRQ# is the OR of both channels; it calls it the "device interrupt
output".

**Host interface**
- 16 mode: as the 2550 (Table 1 p9).
- 68 mode (Table 2 p9): CS#=1 → deselected; CS#=0 with A3=0 → channel A; CS#=0 with
  A3=1 → channel B.
- 68 read (Fig 17 p42): address A0–A3 valid TADS before CS# ↓, held TADH after CS# ↑.
  R/W# set up TRWS before CS# ↓, held TRWH after CS# ↑. Data valid TRDA after CS# ↓,
  floats TRDH after CS# ↑.
- 68 write (Fig 18 p43): data set up TWDS / held TWDH around the **CS# rising edge**,
  which latches the data. This is inferred from the figure; the text does not say which
  edge.

**68-mode timing** (AC p39):

| Sym | Parameter | 2.5 V | 3.3 V |
|---|---|---|---|
| TADS / TADH | address setup / hold | 0 / 0 | 0 / 0 |
| TRWS | R/W# setup to CS# | 0 | 0 |
| TRDA | read data access | 35 max | 30 max |
| TRDH | read data disable | 25 max | 25 max |
| TWDS / TWDH | write data setup / hold | 10 / 3 | 10 / 3 |
| TRWH | CS# de-asserted to R/W# de-asserted | 5 | 5 |
| TCSL | CS# width | 40 min | 35 min |
| TCSD | CS# cycle delay | 40 min | 35 min |

16-mode timing and all serial timing are the same as the 2550 (p39-40).

**Wake-up indicator** (p20, p27-28):
- If sleep is enabled and the chip wakes, it raises an interrupt. The "lower nibble of the
  ISR will read a value of 0x1".
- Reading ISR clears it.
- The ISR table lists "0 0 0 0 0 1 — None (default) or Wake-up Indicator".
- The datasheet does not say whether the INT pin is asserted for this. It says "an
  interrupt is issued by the V2551 to signal to the CPU that it is awake". Ambiguous.

**PowerSave** (p21):
- PwrSave HIGH **and** the chip asleep → the address, data and control inputs are
  internally isolated (ignored).
- In this state the chip wakes only on a RX start bit or a modem-input change. A THR
  write cannot wake it, because the bus is isolated.
- It returns to PowerSave after MSR is read and all interrupts are cleared.
- It stays in this mode until IER[4]=0 "and/or" PwrSave goes LOW.

**Reset table** (p37): INT is "Three-State (16 mode)"; IRQ# is HIGH (68 mode).

**VCC / inputs** (DC p38): as the 2550. Note that the absolute-max table (p38) prints
"Voltage at Any Pin GND-0.3V to 3.3V", which contradicts "5V tolerant" and VIH max 5.5 V.
This is a datasheet error; use inputMax 5.5 V except XTAL1 (= VCC).

**Package drawings**
- QFN32: p47, same as the 2550.
- TQFP48: p48, same as the 2650.

**Obsolete**: none (DigiKey Active; ignore the watermark, see top).

---

## XR16V2651 — High Performance DUART with 32-Byte FIFO and PowerSave Feature

xr16v2651.pdf (rev 1.0.2, May 2007). Same watermark as the 2551.

This is the 2551 wrapper with the 32-byte core: §B tables, DVID 0x06. There is **one more
pin, CLKSEL**, on the TQFP.

**Parts**
- XR16V2651IL: 32-pin QFN (ordering p2 "XR16V2651IL32"), pinout `qfn32-2551` (identical:
  9 PwrSave, 17 16/68#), −40..+85 °C, 2.25–3.6 V.
- XR16V2651IM: 48-lead TQFP (p2), pinout `tqfp48-2651`, −40..+85 °C, 2.25–3.6 V.

**tqfp48-2651**: as tqfp48-2551, except **pin 25 = CLKSEL** (input). Pin 37 stays NC.
Fig 2 (p2) shows CLKSEL at pin 25 in both modes.

CLKSEL (p6):
- "Baud-Rate-Generator Input Clock Prescaler Select for channel A and B. This input is only
  sampled during power up or a reset. Connect to VCC for divide by 1 and GND for divide
  by 4. MCR[7] can override the state of this pin following a reset."
- So after reset, MCR[7] = NOT CLKSEL. The reset table (p37) still prints MCR = 0x00,
  which is inconsistent when CLKSEL=GND.
- The QFN has no CLKSEL, so it presumably defaults to ÷1, as in the reset table.

All else is as the 2551:
- 68-mode timing p39.
- Sleep/PowerSave/wake-up indicator p21.
- Reset p37 (INT 3-state in 16 mode / IRQ# HIGH in 68 mode).
- Packages: QFN p47, TQFP p48.

The abs-max table (p38) prints 7 V supply and 7 V at any pin (copied from 5 V parts);
use DC VIH max 5.5 V.

**Obsolete**: none.

---

## XR16V2552 — High Performance DUART with 16-Byte FIFO (ST16C2552 / XR16L2552 compatible)

xr16v2552.pdf.

This is the 2550 core (DVID 0x02, 16-byte tables), with these differences:
1. One chip select **CS#** plus **CHSEL**.
2. An extra **AFR** register.
3. Multi-function **MFA#/MFB#** output pins.
4. INTA/INTB are always driven, active HIGH.
5. Only TXRDY# is a dedicated pin; RXRDY# is available only through MF#.

**Parts**
- XR16V2552IJ: 44-lead PLCC (ordering p2), pinout `plcc44`, −40..+85 °C, 2.25–3.6 V.

The 32-pin QFN (XR16V2552IL32) exists in the datasheet but is not in the census. Its
pinout is the same as XR16V2652IL below.

**Pinout plcc44** (p2 image; p3-5):

| # | name | role |
|---|---|---|
| 1 | TXRDYA | output |
| 2 | D0 | inout |
| 3 | D1 | inout |
| 4 | D2 | inout |
| 5 | D3 | inout |
| 6 | D4 | inout |
| 7 | D5 | inout |
| 8 | D6 | inout |
| 9 | D7 | inout |
| 10 | A0 | input |
| 11 | XTAL1 | input |
| 12 | GND | ground |
| 13 | XTAL2 | output |
| 14 | A1 | input |
| 15 | A2 | input |
| 16 | CHSEL | input |
| 17 | INTB | output |
| 18 | CS | input |
| 19 | MFB | output |
| 20 | IOW | input |
| 21 | RESET | input |
| 22 | GND | ground |
| 23 | RTSB | output |
| 24 | IOR | input |
| 25 | RXB | input |
| 26 | TXB | output |
| 27 | DTRB | output |
| 28 | CTSB | input |
| 29 | DSRB | input |
| 30 | CDB | input |
| 31 | RIB | input |
| 32 | TXRDYB | output |
| 33 | VCC | power |
| 34 | INTA | output |
| 35 | MFA | output |
| 36 | RTSA | output |
| 37 | DTRA | output |
| 38 | TXA | output |
| 39 | RXA | input |
| 40 | CTSA | input |
| 41 | DSRA | input |
| 42 | CDA | input |
| 43 | RIA | input |
| 44 | VCC | power |

**Host interface** (p3, p7 Table 1):
- Intel IOR#/IOW#, as on the 2550.
- CS# is active low. When CS#=0: **CHSEL=1 selects channel A, CHSEL=0 selects channel B**.
  When CS#=1 the device is deselected.
- Both channels can be written at once only through AFR[0] (concurrent write).
- Bus timing is the same as the 2550 (AC p38), with CS# in place of CSA#/CSB#.
- INTA/INTB (p3): "Interrupt output (active high). A HIGH indicates channel A is
  requesting for service". MCR[3] does **not** gate INT here. MCR[3] description (p30):
  "If OP2# is selected as the MF# output, then this bit controls the state of this general
  purpose output. 0 = OP2# HIGH, 1 = OP2# LOW".
- INT reset state is not in Table 15 (p36). Presumably LOW, because ISR = 0x01 (nothing
  pending).

**Register differences** (Table 8 p21, Table 9 p22-23, §4.12 p33):
- IER, ISR and FCR are listed as accessible with LCR[7]=0.
- **AFR** is at A2..A0=010 when LCR[7]=1, LCR≠0xBF and **EFR[4]=0**. DLD takes that
  address when EFR[4]=1. AFR resets to 0x00.
- AFR[0] Concurrent Write:
  - 1 = a single CPU write goes to the same register in **both** channels.
  - Reads still follow CHSEL.
  - It can be set or cleared from either channel's register set.
  - "Ensure LCR bit-7 of both channels are in the same state before executing a
    concurrent write to the registers at address 0, 1, or 2."
- AFR[2:1] select the MF# pin function:
  - 00 = OP2# (default)
  - 01 = BAUDOUT# (the datasheet only says "the Baud rate clock output". It is presumably
    the 16X/8X/4X sampling clock, as on the ST16C2552; not stated.)
  - 10 = RXRDY# (Table 2 behavior)
  - 11 = reserved
- AFR[7:3] reserved, 0.

**Reset/outputs** (Table 15 p36):
- AFR=0x00, so MF# = OP2# = HIGH.
- TXRDY# LOW; TX/RTS#/DTR# HIGH.

**VCC**:
- DC p37 is the same as the 2550: 5 V-tolerant inputs (VIH max 5.5 V); XTAL1 max VCC
  (text p9).
- Abs max (p37) prints 7 V supply and 7 V on any pin (a copy error).

**Package drawing**: "PACKAGE DIMENSIONS (44 PIN PLCC)", "44 LEAD PLASTIC LEADED CHIP
CARRIER (PLCC) Rev. 1.00", p44:
- e 1.27 (0.050 in).
- B (lead width at seating) 0.33–0.53 (mid 0.43); B1 (upper lead) 0.66–0.81.
- D (span over leads) 17.40–17.65 (mid 17.53); D1 (body) 16.51–16.66.
- D2 (footprint across contact centres) 14.99–16.00 (mid 15.50); D3 = 12.70 typ (10 × e).
- A 4.19–4.57; A1 2.29–3.05; A2 ≥ 0.51; C 0.19–0.32; H1 1.07–1.42; H2 1.07–1.22;
  R 0.64–1.14.
- No land pattern.

The QFN32 drawing is p45.

**Obsolete**: none.

---

## XR16V2652 — High Performance DUART with 32-Byte FIFO (2552 pin-out)

xr16v2652.pdf (rev 1.0.2, May 2007).

This is the 2552 wrapper (CS#/CHSEL, AFR, MF#, always-driven active-high INT) with the
32-byte core: §B tables (Table 11 p27; Tables 6/7 p15/p17) and DVID 0x06 (p7, p33).

**Parts**
- XR16V2652IJ: 44-lead PLCC (ordering p2), pinout `plcc44` (identical to the 2552),
  −40..+85 °C, 2.25–3.6 V. **DigiKey status Obsolete** (census row; still listed as in
  stock).
- XR16V2652IL: 32-pin QFN 5×5 (ordering p2 "XR16V2652IL32 32-QFN"), pinout `qfn32-2552`,
  −40..+85 °C, 2.25–3.6 V.

**Pinout qfn32-2552** (p2 of xr16v2552.pdf as an image; pin table p3-5 of either PDF).
The exposed pad is GND.

| # | name | role |
|---|---|---|
| 1 | D6 | inout |
| 2 | D7 | inout |
| 3 | A0 | input |
| 4 | XTAL1 | input |
| 5 | XTAL2 | output |
| 6 | A1 | input |
| 7 | A2 | input |
| 8 | CHSEL | input |
| 9 | INTB | output |
| 10 | CS | input |
| 11 | IOW | input |
| 12 | RESET | input |
| 13 | RTSB | output |
| 14 | IOR | input |
| 15 | RXB | input |
| 16 | TXB | output |
| 17 | CTSB | input |
| 18 | NC | nc |
| 19 | NC | nc |
| 20 | GND | ground |
| 21 | INTA | output |
| 22 | RTSA | output |
| 23 | TXA | output |
| 24 | RXA | input |
| 25 | CTSA | input |
| 26 | VCC | power |
| 27 | D0 | inout |
| 28 | D1 | inout |
| 29 | D2 | inout |
| 30 | D3 | inout |
| 31 | D4 | inout |
| 32 | D5 | inout |
| EP | GND | ground |

This package has no TXRDY#, MF#, DTR#, DSR#, CD# or RI# pins. AFR[2:1] still exists,
but there is no MF# pin to drive.

**Host interface / registers / behavior**: as the 2552. Table 1 (p7) prints the corrected
CHSEL table: CHSEL=1 → A, CHSEL=0 → B.

**Packages**: PLCC44 p44 and QFN32 p45, with the same dimensions as the 2552 and 2550
drawings above.

**VCC**: DC p37 header says "TA=0 to 70 °C (−40 to +85 °C for industrial grade)". Only
industrial parts are ordered. Otherwise as the 2552 (abs max prints 7 V; use 5.5 V
inputMax, XTAL1 ≤ VCC).

---

## Notes, ambiguities, datasheet errors

1. **LCR=0xBF bank, addresses 000/001/011**:
   - Table 8 lists only EFR (010) and XON1..XOFF2 (100-111) for LCR=0xBF.
   - LCR (011) must stay readable and writable to leave the bank. Table 8 puts it under
     the "LCR ≠ 0xBF" bracket, but that cannot be literal.
   - 000/001 with LCR=0xBF are not defined.
   - On the 2550/2551/2650/2651, address 010 with LCR[7]=1, LCR≠0xBF and EFR[4]=0 is not
     stated. Presumably ISR/FCR; the table bracket is "LCR ≠ 0xBF".
2. **INT tables** (2550 Table 3, 2552/2652 Table 3, 2650 Table 3) have a second
   transmitter row. The 2552 labels it "AUTO RS485 MODE = YES". These parts have no RS-485
   mode and no FCTR register. The 2552 MCR[1] text also says "Auto RS485 … enabled by
   FCTR bit-3" (p29). This is copy-over from the XR16V2752/2852-type datasheets — ignore
   it.
3. **2650/2651 FCR text** says "(logic 0 = default, TX trigger level = 1)" and "RX trigger
   level = 1", but their tables give 16 (TX) and 8 (RX) for 00. Trust the tables.
4. **2551/2651 FCR text** adds "Whichever selection is made last applies to both the RX
   and TX side". The tables give independent TX and RX columns, and the 2550/2650 lack
   this sentence. Most likely a stray line copied from another family's datasheet (where
   one trigger table is shared). Recommend independent TX/RX triggers.
5. **2551 abs max** says any pin −0.3..3.3 V. The 2552/2650/2651/2652 abs max say 7 V
   supply and 7 V on pins. All of these contradict the DC table (VIH max 5.5 V) and the
   "5 V tolerant" feature. Use inputMax 5.5 V; XTAL1 ≤ VCC.
6. **XTAL1 crystal max**: the AC table gives 32 MHz at 2.5 V and 24 MHz at 3.3 V. The
   2551/2651 text says "external 24 MHz crystal".
7. **2552 Fig 3 note** says "BAUDOUT# when AFR[1:0]='01'". It should be AFR[2:1]
   (§4.12 p33 is the authority).
8. **MSR clearing / LSR clearing on read**: not stated explicitly beyond "interrupt
   cleared by reading LSR/MSR". Use standard 16550 semantics.
9. **2551/2651 wake-up indicator**: it is unclear whether the INT pin is asserted while
   ISR reads xxxx0001.
10. **2551/2651 Motorola mode**: INTB is unused/NC; IRQ# is open-drain. Motorola write
    latching edge (CS# rising) is inferred from Fig 18.
11. **2651 CLKSEL** vs the reset table: see the 2651 section.
12. **IrDA RX polarity**: the pin descriptions mention "see MCR[6]" (2551/2651/2652) or
    "MCR[6] and MCR[2]" (2552). The MCR[2] text is the authority.
13. **Sleep**: needs IER[4]=1 on **both** channels.
14. **DLL/DLM reset**: they are not reset by the RESET pin; only power-up sets
    0x01/0x00.
