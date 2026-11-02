# Exar group B — XR16L2551, XR16L2552, XR16M2550/2551/2650/2651, XR16M752/XR68M752

## PDF → devices
| PDF (datasheets/controller-uart/) | device(s) | census parts |
|---|---|---|
| xr16l2551.pdf | XR16L2551 | XR16L2551IM-F |
| xr16l2552.pdf | XR16L2552 | XR16L2552IJ, XR16L2552IJTR, XR16L2552IM-F |
| xr16m2550.pdf | XR16M2550 | XR16M2550IL32, XR16M2550IM48 |
| xr16m2551.pdf | XR16M2551 | XR16M2551IL32, XR16M2551IM48 |
| xr16m2650.pdf | XR16M2650 | XR16M2650IL32, XR16M2650IM48 |
| xr16m2651.pdf | XR16M2651 | XR16M2651IL32, XR16M2651IM48 |
| xr16m752.pdf | XR16M752 (XR16M752 + XR68M752 as one device; XR68M752 adds the 16/68# pin on TQFP pin 24, which is NC on the XR16M752; 64-byte FIFOs — the census "34B" is wrong) | XR16M752IM48-F, XR16M752IM48TR-F, XR68M752IM48TR-F |

Census parts not decoded: none. Suffix notes: "-F" (lead-free/RoHS) and "TR" (tape & reel) are not in the older Exar ordering tables but do not change package/grade. IL32 = 32-QFN 5x5, IM48 / IM = 48-TQFP 7x7, IJ = 44-PLCC, I = -40..+85 °C.

Group-wide flags:
- xr16m2650.pdf, xr16m2551.pdf and xr16m2651.pdf carry an "OBS — no longer being manufactured and may not be ordered" watermark on every page, but DigiKey lists the census rows as Active (Sipex). The planner should decide whether to flag them obsolete.
- XR16M2550 and XR16M2551 both return DVID 0x02; XR16M2650 and XR16M2651 both return 0x06.
- Family relationships: the M2550 has the base 2550 register set (16-byte FIFO, fractional BRG, 4X/8X/16X sampling). The M2650 is the same with 32-byte FIFOs and different trigger tables. The M2551 and M2651 add the 16/68# Intel/Motorola pin, a PWRSAVE pin, a CLKSEL pin (TQFP only) and a wake-up interrupt. The L-series (L2551, L2552) runs at 2.25–5.5 V with integer divisors (see their own sections).

## XR16L2551

- **Title / function (p1):** "XR16L2551 — LOW VOLTAGE DUART WITH POWERSAVE", Exar REV. 1.1.3, May 2007. Dual 16C550-compatible UART, 16-byte TX and RX FIFOs per channel, 2.25–5.5 V, 5 V-tolerant inputs, Intel/Motorola bus select (16/68# pin), auto RTS/CTS, auto Xon/Xoff, special-character detect, IrDA 1.0 encoder/decoder, BRG prescaler ÷1/÷4, sleep with auto wake-up, plus a PowerSave pin that isolates the host bus while asleep.
- **Datasheet:** `datasheets/controller-uart/xr16l2551.pdf` (49 pp; content pp1–47, TOC pp48–49).
- **How it differs from its siblings (as the datasheet says):**
  - p1: "enhanced register set is compatible to the ST16C2550 and XR16L2550". p7: "Its feature set is compatible to the XR16L2550 device but offers Intel or Motorola data bus interface and PowerSave to isolate the data bus interface during Sleep mode." So the register set, FIFO size and features match XR16L2550. The extras are the **16/68# pin** (Motorola mode: CS#, R/W#, A3 = channel select, one open-drain IRQ#) and the **PWRSAVE pin**. The 48-TQFP pinout is different from the L2550 (pins 12 = PWRSAVE, 24 = 16/68#). p1: "pin-to-pin compatible to XR16L2751CM".
  - Checked in xr16l2550.pdf (not stated in this PDF): the L2550 also reports DVID = 0x02 (xr16l2550.pdf "DLM will provide 0x02 to indicate L2550"). So **DVID can't tell L2550 from L2551**.
  - XR16L2552 is not mentioned in this PDF. Its own PDF shows a 2552-style part (A3 channel select, MF# pin/AFR, 44-PLCC + 48-TQFP), so it is a different device.
  - No ADS/ALE pin, no BAUDOUT pin, no OP1# pin, no MF pin, no RS-485 control, no FCTR/EMSR/TX trigger levels. TX trigger is fixed (FCR[5:4] reserved).

### Parts

| Census part | Package | Pins | Pinout name | Grade | Supply |
|---|---|---|---|---|---|
| XR16L2551IM-F | 48-Lead TQFP 7x7x1 mm ("IM" = 48-TQFP per ordering table p2; "-F" = lead-free suffix, not in this 2007 ordering table) | 48 | tqfp48 | Industrial −40…+85 °C (only grade, p2) | 2.25–5.5 V (p1, p6, p36) |

Other package in the datasheet: XR16L2551IL = 32-Lead QFN 5x5x0.9 mm (p2). It has no census part; its pinout is listed briefly below.

Undecodable: none. The "-F" suffix is not in this datasheet's ordering table (p2 lists only XR16L2551IM / IL). It is Exar's standard RoHS/lead-free suffix and does not change the part's function.

### Pinout — tqfp48 (48-TQFP, p2 Figure 2 + pin table pp3–6)

The function of some pins depends on 16/68#. The first name is Intel mode (16/68# = 1), the name in brackets is Motorola mode (16/68# = 0).

| Pin | Name (Intel) | Name (Motorola) | Role | Notes |
|---|---|---|---|---|
| 1 | D5 | D5 | inout | |
| 2 | D6 | D6 | inout | |
| 3 | D7 | D7 | inout | |
| 4 | RXB | RXB | input | idles high |
| 5 | RXA | RXA | input | idles high |
| 6 | TXRDYB | TXRDYB | output | active low |
| 7 | TXA | TXA | output | |
| 8 | TXB | TXB | output | |
| 9 | OP2B | OP2B | output | active low; MCR[3] of ch B |
| 10 | CSA | CS | input | active low. Intel: chip select A. Motorola: the single chip select |
| 11 | CSB | A3 | input | Intel: chip select B, active low. Motorola: A3 address/channel select (0 = A, 1 = B) |
| 12 | PWRSAVE | PWRSAVE | input | active high |
| 13 | XTAL1 | XTAL1 | input | crystal/ext clock; NOT 5 V tolerant (p10) |
| 14 | XTAL2 | XTAL2 | output | crystal / buffered clock out; stops in sleep |
| 15 | IOW | RW | input | Intel: write strobe, active low. Motorola: R/W# (1 = read, 0 = write) |
| 16 | CDB | CDB | input | active low |
| 17 | GND | GND | ground | |
| 18 | RXRDYB | RXRDYB | output | active low |
| 19 | IOR | (VCC) | input | Intel: read strobe, active low. Motorola: unused, tie to VCC |
| 20 | DSRB | DSRB | input | active low |
| 21 | RIB | RIB | input | active low |
| 22 | RTSB | RTSB | output | active low |
| 23 | CTSB | CTSB | input | active low |
| 24 | 16/68 (name it `MODE16` or `I68`; printed `16/68#`) | same | input | 1 = Intel, 0 = Motorola |
| 25 | NC | NC | nc | (L2751 migration pin, p6) |
| 26 | A2 | A2 | input | |
| 27 | A1 | A1 | input | |
| 28 | A0 | A0 | input | |
| 29 | INTB | (NC) | output | Intel: ch B interrupt, active high, 3-state unless MCR[3]=1. Motorola: unused, "will stay at logic zero level" (p4) |
| 30 | INTA | IRQ | output | Intel: ch A interrupt, active high, 3-state unless MCR[3]=1. Motorola: IRQ#, device interrupt (both channels), active low, open drain, needs pull-up (p3) |
| 31 | RXRDYA | RXRDYA | output | active low |
| 32 | OP2A | OP2A | output | active low |
| 33 | RTSA | RTSA | output | active low |
| 34 | DTRA | DTRA | output | active low |
| 35 | DTRB | DTRB | output | active low |
| 36 | RESET | RESET (RESETBAR) | input | Intel: active-high RESET. Motorola: active-low RESET# |
| 37 | NC | NC | nc | |
| 38 | CTSA | CTSA | input | active low |
| 39 | DSRA | DSRA | input | active low |
| 40 | CDA | CDA | input | active low |
| 41 | RIA | RIA | input | active low |
| 42 | VCC | VCC | power | |
| 43 | TXRDYA | TXRDYA | output | active low |
| 44 | D0 | D0 | inout | |
| 45 | D1 | D1 | inout | |
| 46 | D2 | D2 | inout | |
| 47 | D3 | D3 | inout | |
| 48 | D4 | D4 | inout | |

There is no exposed pad on the TQFP. The pin table (pp3–6) and Figure 2 (p2) agree for every pin; I checked Figure 2 against the page image.

### Pinout — qfn32 (32-QFN, XR16L2551IL; no census part, brief)

1 D6, 2 D7, 3 RXB, 4 RXA, 5 TXA, 6 TXB, 7 CSA (CS), 8 CSB (A3), 9 PWRSAVE, 10 XTAL1, 11 XTAL2, 12 IOW (R/W#), 13 GND, 14 IOR (tie VCC in 68 mode), 15 RTSB, 16 CTSB, 17 16/68#, 18 A2, 19 A1, 20 A0, 21 INTB (NC in 68 mode), 22 INTA (IRQ#), 23 RTSA, 24 RESET (RESET#), 25 CTSA, 26 VCC, 27 D0, 28 D1, 29 D2, 30 D3, 31 D4, 32 D5. The exposed center pad is GND (p6). The QFN has no DTR/DSR/CD/RI, OP2, TXRDY or RXRDY pins (pin table: "-").

### Host interface (pp3, 8–9, 37–41)

- 8-bit data bus D7..D0 and address A2..A0. Bus cycles are asynchronous; no clock is needed for register access (p8).
- **Intel (16) mode, 16/68# = 1:**
  - CSA# selects channel A and CSB# selects channel B, both active low. Table 1 (p9): 11 = deselected, 01 (CSA# = 0) = A, 10 = B, 00 = **both selected**. Writing with both selected writes the same register in both channels. "do not attempt to read from both uarts simultaneously" (reading with both selected is undefined).
  - Read: the falling edge of IOR# starts an internal read and the data is driven onto D7..D0; the host latches it on the rising edge (p3).
  - Write: the falling edge of IOW# starts the cycle; "the rising edge transfers the data byte on the data bus to an internal register" (p3). So the latch point is the rising edge of IOW# (data setup TDS / hold TDH are referenced to it, Fig 17 p40).
  - Address must be valid TAS before CS# falls and held TAH after CS# rises (Fig 16/17).
  - D bus is driven only while CS# (selected channel) and IOR# are both low. It floats TDD after IOR# rises.
- **Motorola (68) mode, 16/68# = 0:**
  - One CS#, active low (pin CSA#). R/W# is on the IOW# pin; IOR# is unused (tie to VCC). A3 is on the CSB# pin and picks the channel. Table 2 (p9): CS# = 1 → deselected; CS# = 0, A3 = 0 → A; CS# = 0, A3 = 1 → B.
  - Fig 18/19 (pp40–41): A0–A3 setup TADS before CS# falls and hold TADH after CS# rises. R/W# setup TRWS before CS# falls and hold TRWH after CS# rises.
  - Read: data valid TRDA after CS# falls, floats TRDH after CS# rises.
  - Write: data setup TWDS / hold TWDH around the rising edge of CS#, so the write latches on CS# rising.
  - Interrupt: the single IRQ# (pin INTA), open drain, active low. It is the OR of both channels (p3 "device interrupt output"). MCR[3] does not affect IRQ# (p30). The INTB pin is driven to logic 0 (p4).
  - RESET pin polarity is active low in 68 mode.
- No DDIS, ADS/ALE, BAUDOUT, I2C or SPI.
- **PowerSave:** with PWRSAVE = 1 and the device in sleep mode, the address, data and control inputs (A2–A0, D7–D0, IOR#, IOW#, CSA#, CSB#) are internally isolated (p20; block diagram p1). While isolated, a host write to THR cannot wake the part; only an RX start bit or a modem-input change can (p20).

### Registers (per channel; Table 8 p22, Table 9 pp23–24, descriptions pp24–35)

Register access conditions:
- **LCR[7] = 0:** normal 16550 bank.
- **LCR[7] = 1 and LCR ≠ 0xBF:** DLL/DLM at addresses 0/1.
- **DLL = DLM = 0x00 with LCR[7] = 1 (LCR ≠ 0xBF):** reads of 0/1 return DREV/DVID.
- **LCR = 0xBF:** enhanced bank (EFR, Xon/Xoff).

At 0xBF, addresses 0, 1 and 3 still map to the normal entries. Table 8 lists LCR at 011 with no condition. For 000/001 with LCR = 0xBF, Table 8 shows nothing other than "LCR[7]=1, LCR≠0xBF" for DLL/DLM. The datasheet does not say what reading 000/001 returns when LCR = 0xBF. Note this as unspecified. The usual Exar behavior is that DLL/DLM stay accessible, but this PDF does not say so.

| A2A1A0 | Condition | Name | R/W | Reset (Table 14 p35) |
|---|---|---|---|---|
| 000 | LCR[7]=0 | RHR | R | XX |
| 000 | LCR[7]=0 | THR | W | XX |
| 001 | LCR[7]=0 | IER | R/W | 0x00 |
| 010 | LCR≠0xBF | ISR | R | 0x01 |
| 010 | LCR≠0xBF | FCR | W | 0x00 |
| 011 | any | LCR | R/W | 0x00 |
| 100 | LCR≠0xBF | MCR | R/W | 0x00 |
| 101 | LCR≠0xBF | LSR | R (write = reserved) | 0x60 |
| 110 | LCR≠0xBF | MSR | R (write = reserved) | [3:0] = 0, [7:4] = inverted input levels |
| 111 | LCR≠0xBF | SPR | R/W | **0xFF** |
| 000 | LCR[7]=1, LCR≠0xBF | DLL | R/W | XX |
| 001 | LCR[7]=1, LCR≠0xBF | DLM | R/W | XX |
| 000 | LCR[7]=1, LCR≠0xBF, DLL=DLM=0 | DREV | R (Table 8 says Read/Write; Table 9 and §4.13 say read only) | revision, 0x01 = rev A |
| 001 | same | DVID | R | **0x02** |
| 010 | LCR=0xBF | EFR | R/W | 0x00 |
| 100 | LCR=0xBF | XON1 | R/W | 0x00 |
| 101 | LCR=0xBF | XON2 | R/W | 0x00 |
| 110 | LCR=0xBF | XOFF1 | R/W | 0x00 |
| 111 | LCR=0xBF | XOFF2 | R/W | 0x00 |

**DREV/DVID:** the datasheet says "set the baud rate generator registers DLL and DLM both to 0x00. Now reading the content of the DLM will provide 0x02 … DLL … revision" (p9). It does not say how to write DLL again after reading the ID. Presumably the read path is a mux and writes still go to DLL/DLM, since DLL/DLM are listed R/W at the same addresses.

**IER (p25)**

| Bit | Name | Meaning |
|---|---|---|
| 0 | RX data ready interrupt enable | Non-FIFO: a character is in RHR. FIFO: RX trigger level reached. Also enables the RX timeout interrupt (IIR priority 2) |
| 1 | THR empty interrupt enable | Interrupts whenever the TX FIFO/THR becomes empty. If it is already empty when the bit is set, an interrupt is generated at once |
| 2 | RX line status interrupt enable | LSR[1..4]. LSR[1] interrupts as soon as the character is received; LSR[2..4] interrupt when the errored character reaches the top of the FIFO (RHR) |
| 3 | Modem status interrupt enable | |
| 4 | Sleep mode enable | Needs EFR[4] = 1 to modify |
| 5 | Xoff interrupt enable | Receive Xoff / special character. Needs EFR[4] = 1 |
| 6 | RTS# interrupt enable | Interrupts when RTS# goes low→high. Needs EFR[4] = 1 |
| 7 | CTS# interrupt enable | Interrupts when CTS# goes low→high. Needs EFR[4] = 1 |

**ISR (p26–27; Table 10)**
- Bit 0 = 0: an interrupt is pending. Bit 0 = 1: none pending.
- Bits 3:1 = source code.
- Bit 4 = Xoff / special character (only when EFR[4] = 1).
- Bit 5 = CTS#/RTS# low→high (only when EFR[4] = 1).
- Bits 7:6 = 11 when the FIFOs are enabled, 00 when disabled.

Priority codes (ISR[5:0]):

| Priority | ISR[5:0] | Source | Set by | Cleared by |
|---|---|---|---|---|
| 1 | 000110 | Receiver line status | LSR[1..4] | Reading LSR. The flags/tags stay until the errored character(s) leave the FIFO |
| 2 | 001100 | RX data timeout | Timeout timer | Reading RHR |
| 3 | 000100 | RX data ready | RX trigger level | Reading until the FIFO is below the trigger |
| 4 | 000010 | TX ready | TX FIFO/THR empty | Reading ISR or writing THR |
| 5 | 000000 | Modem status | MSR[3:0] | Reading MSR |
| 6 | 010000 | Received Xoff or special character | Match | Xoff: reading ISR or receiving Xon. Special character: reading ISR or receiving the next character |
| 7 | 100000 | CTS#/RTS# change of state | low→high transition during auto CTS/RTS | Reading MSR |
| – | 000001 | none | | |

Clearing rules are from §4.4.2 p26. Reading ISR returns the highest-priority pending source; the others queue.

**FCR (write only, p27–28)**
- Bit 0: FIFO enable. It "must be set to logic 1 when other FCR bits are written or they will not be programmed."
- Bit 1: RX FIFO reset. Clears the pointers and count, not the RSR. Self-clearing. Only works when bit 0 = 1.
- Bit 2: TX FIFO reset. Same rules; does not touch the TSR.
- Bit 3: DMA mode select (changes TXRDY#/RXRDY# behavior).
- Bits 5:4: reserved. Table 9 shows them as 0. No TX trigger levels on this part.
- Bits 7:6: RX trigger: 00 = 1 (default), 01 = 4, 10 = 8, 11 = 14 (Table 11 p28).

Table 9 says shaded bits need EFR[4], and §4.14 lists "FCR[5:4]" among the EFR-protected bits (p24 EFR cell), but FCR[5:4] is described as reserved (p27). Treat them as reserved.

**LCR (p28–29)**
- Bits 1:0: word length 5/6/7/8 (00/01/10/11).
- Bit 2: stop bits. 0 = 1 stop. 1 = 1.5 stops at 5-bit words, 2 stops at 6/7/8-bit words.
- Bit 3: parity enable.
- Bit 4: 0 = odd, 1 = even.
- Bit 5: forced parity. With bit 3 = 1: bit 5 = 1, bit 4 = 0 forces parity to **1 (mark)**; bit 5 = 1, bit 4 = 1 forces it to **0 (space)** (Table 12).
- Bit 6: TX break. Forces TX to 0 until cleared.
- Bit 7: divisor latch access.
- The value 0xBF selects the enhanced bank.

**MCR (p29–30)**

| Bit | Meaning |
|---|---|
| 0 | 1 = DTR# output low; 0 = high (default) |
| 1 | 1 = RTS# output low; 0 = high (default). Must be 1 to start auto-RTS |
| 2 | Reserved (OP1#). No pin. In loopback it drives the internal RI# (MSR[6] = MCR[2]) |
| 3 | OP2#/INT enable. 0 = INTx 3-state and OP2x# pin = 1 (default). 1 = INTx active and OP2x# = 0. No effect on IRQ# in Motorola mode. In loopback it drives the internal CD# (MSR[7] = MCR[3]) |
| 4 | Internal loopback |
| 5 | Xon-Any enable (needs EFR[4] = 1) |
| 6 | IrDA encoder/decoder enable (needs EFR[4] = 1) |
| 7 | Clock prescaler. 0 = ÷1 (default), 1 = ÷4 (needs EFR[4] = 1) |

**LSR (read only, p30–31)**
- Bit 0: data ready. Set when a character moves into RHR/FIFO, cleared when the FIFO is empty.
- Bit 1: overrun. Data arrived while the FIFO was full. The RSR character is overwritten and not put in the FIFO. Interrupts immediately.
- Bit 2: parity error.
- Bit 3: framing error (invalid stop bit).
- Bit 4: break. RX was 0 for at least one character frame time. Only one break character is loaded into the FIFO; the break indication remains until RX returns to mark.
- Bits 2–4 belong to the character at the top of the FIFO (RHR) and are updated as each byte is read.
- Bit 5: THR empty. Non-FIFO: set when the THR moves to the TSR, cleared when THR is loaded. FIFO: set when the TX FIFO is empty, cleared when it holds at least 1 byte.
- Bit 6: THR and TSR both empty (transmitter idle).
- Bit 7: RX FIFO global error. At least one PE/FE/BI anywhere in the RX FIFO; clears when no errored characters remain.

Clearing of bit 1: the datasheet says only that the LSR interrupt clears on an LSR read and that "flags and tags not cleared until character(s) that generated the interrupt(s) has been emptied" (p26). It does not say that reading LSR clears OE. Standard 16550 behavior (cleared by reading LSR) is implied but not stated.

**MSR (read only, p32)**
- Bit 0: ΔCTS. Bit 1: ΔDSR. Bit 2: trailing-edge RI (RI# went 0→1, end of ring). Bit 3: ΔCD.
- Bit 4 = NOT CTS#, bit 5 = NOT DSR#, bit 6 = NOT RI#, bit 7 = NOT CD#.
- In loopback: bit 4 = MCR[1] (RTS), bit 5 = MCR[0] (DTR), bit 6 = MCR[2], bit 7 = MCR[3].
- Delta bits clear when MSR is read (§4.4.2 clears the interrupt on an MSR read; the delta bits are "since the last time it was monitored").

**SPR (p32):** scratch. Kept through sleep. Reset/power-up value is 0xFF.

**DLL/DLM (p32):** 16-bit divisor, 1 to 2^16 − 1.

**EFR (LCR = 0xBF, p33–34)**
- Bits 3:0: software flow control select (Table 13 p33, below).
- Bit 4: enhanced-function write enable. Bits IER[7:4], ISR[5:4], FCR[5:4] and MCR[7:5] can be modified only while EFR[4] = 1. Clearing EFR[4] latches their current values. Reset value 0, so they are 0 after reset.
- Bit 5: special character detect. Compares each RX character with XOFF2.
- Bit 6: auto RTS enable.
- Bit 7: auto CTS enable.
- "whenever changing the TX or RX flow control bits, always reset all bits back to logic 0 (disable) before programming a new setting" (p33).

Table 13 (EFR[3:0] = Cont-3..0):

| EFR[3:2] | TX side | | EFR[1:0] | RX side |
|---|---|---|---|---|
| 00 | no TX flow control | | 00 | no RX flow control |
| 10 | transmit Xon1/Xoff1 | | 10 | compare Xon1/Xoff1 |
| 01 | transmit Xon2/Xoff2 | | 01 | compare Xon2/Xoff2 |
| 11 | transmit Xon1+Xon2 / Xoff1+Xoff2 (two-character sequences) | | 11 | (depends on bits 3:2, below) |

Combinations with EFR[1:0] = 11:
- 1011: TX Xon1/Xoff1; RX compares Xon1 **or** Xon2, Xoff1 **or** Xoff2.
- 0111: TX Xon2/Xoff2; RX compares Xon1 or Xon2, Xoff1 or Xoff2.
- 1111: TX both; RX compares the sequences Xon1+Xon2 and Xoff1+Xoff2.
- 0011: no TX; RX compares the sequences Xon1+Xon2 and Xoff1+Xoff2.

**XON1/XON2/XOFF1/XOFF2 (p34):** 8-bit characters, reset 0x00. Only the low LCR[1:0] word-length bits are compared (p18). Bit 0 is the first (LSB) received bit.

### Behavior

**Clock / BRG (pp10–13, p30, p37)**
- Clock comes from a crystal on XTAL1/XTAL2 (1.8432–24 MHz in Fig 4) or an external clock on XTAL1. XTAL2 is the crystal output / buffered clock. XTAL1 is not 5 V tolerant (p10).
- Prescaler ÷1 or ÷4 (MCR[7]), then a 16-bit divisor, then a fixed 16× sampling clock.
- Data rate = clock / (prescaler × divisor × 16). The p12 formula `divisor = XTAL1 / (rate × 16)` is the MCR[7] = 0 case.
- One BRG per channel, shared by TX and RX.
- Maximum clock (AC table p37) by supply column:
  - 2.5 V: crystal 16 MHz, external 16 MHz.
  - 3.3 V: crystal 20 MHz, external 30 MHz.
  - 5 V: crystal 24 MHz, external 50 MHz (Fig 5: needs a 2 kΩ pull-up on XTAL2).
  - External clock low/high time minimum: 31 / 17 / 10 ns.
- p1 data rates: up to 3.125 Mbps at 5 V, 2 Mbps at 3.3 V, 1 Mbps at 2.5 V with an external clock (1.5 / 1.25 / 1 Mbps with a crystal).
- Table 6 (p13): 14.7456 MHz, MCR[7] = 0. Divisor 1 → 921.6k, 2 → 460.8k … 96 → 9600, 2304 → 400.
- Divisor = 0: not covered (N minimum 1, p38). Sleep requires a non-zero divisor (p20).
- No BAUDOUT/RCLK pins.

**Transmitter (p13–14)**
- Order on the line: start bit, 5–8 data bits LSB first, optional parity, stop bit(s).
- Each bit is 16 BRG clocks.
- THR feeds the TSR.
- Non-FIFO: LSR[5] sets when the THR transfers to the TSR; LSR[6] sets when the TSR is empty.
- FIFO: LSR[5] sets when the FIFO is empty; LSR[6] sets when the FIFO and TSR are both empty.
- Break: LCR[6] forces TX to 0.
- The first start bit follows THR loading by TINT = 8–24 Bclk (p38, "Delay From Initial INT Reset To Transmit Start").

**Receiver (p14–15)**
- Start-bit check: on a falling edge the receiver counts 8 16× clocks, then samples. Still 0 → start bit validated. Back to 1 → false start, ignored.
- Data, parity and stop bits are each sampled at mid-bit (every 16 clocks).
- The RX FIFO is 16 × 11 bits (8 data + PE, FE, BI tags).
- RX timeout: no data received for "4 word lengths as defined by LCR[1:0] plus 12 bits time", described as 3.7–4.6 character times (p15). Interrupt-generation text: "4-char plus 12 bits delay timer" (p26). Cleared by reading RHR.
- RX data-ready interrupt and timeout are enabled by IER[0].
- Overrun: see LSR[1]. Break: see LSR[4].

**FIFOs / DMA mode / TXRDY#-RXRDY# (Table 3 p10; Figs 20–25 pp41–44)**
- 16-byte TX and 16-byte RX FIFO. Enabled by FCR[0].

| Pin | FIFO off | FIFO on, DMA off (FCR[3]=0) | FIFO on, DMA on (FCR[3]=1) |
|---|---|---|---|
| RXRDY# | 0 when 1 byte in RHR; 1 when no data | 0 when ≥1 byte in FIFO; 1 when FIFO empty | 1→0 when FIFO reaches the trigger level or a timeout occurs; 0→1 when FIFO empties |
| TXRDY# | 0 when THR empty; 1 when byte in THR | 0 when FIFO empty; 1 when ≥1 byte in FIFO | 0 when FIFO has ≥1 empty location; 1 when FIFO full |

- INT pin as transmitter (Table 4 p10): INT = 1 when THR/FIFO empty, 0 when it holds data (FIFO on: ≥1 byte).
- INT pin as receiver (Table 5 p10): FIFO off: INT = 1 when 1 byte present, 0 when no data. FIFO on: 1 above the trigger level, 0 below it.
- Both tables assume INT is enabled and show the logical output (active high in Intel mode).

**Interrupt pins**
- Intel mode: INTA/INTB are active high and 3-state while MCR[3] = 0. With MCR[3] = 1 they drive the logical interrupt (1 = pending).
- OP2x# = NOT MCR[3]. p5: "should not be used as a general output else it will disturb the INTA output functionality".
- Motorola mode: IRQ# is open drain, active low, shared by both channels, and independent of MCR[3]. INTB = 0.

**Auto RTS / CTS (pp16–17, p34)**
- Auto RTS (EFR[6]):
  - The host must first set MCR[1] = 1 to assert RTS#.
  - p34: "an interrupt will be generated when the receive FIFO is filled to the programmed trigger level and RTS de-asserts to a logic 1 at the next upper trigger level. RTS# will return to a logic 0 when FIFO data falls below the next lower trigger level."
  - The trigger ladder is 1, 4, 8, 14. Fig 10 example (p15): trigger 8 → RTS# de-asserts when data fills to 14 and re-asserts when it falls to 4.
  - The datasheet does not state the "next upper" for trigger 14, or the "next lower" for trigger 1. Unspecified. Likely 16 (FIFO full) and 0 (empty).
  - The RTS interrupt (ISR[5]) fires on the RTS# low→high transition when IER[6] = 1.
- Auto CTS (EFR[7]):
  - When CTS# goes high, transmission stops after the stop bit of the current character. It resumes when CTS# goes low.
  - The CTS interrupt (IER[7]) fires on the CTS# low→high transition.
  - p16 says the interrupt sets ISR bit-5.
- Auto RTS/CTS does not work in loopback (p21).

**Auto Xon/Xoff (p18, Table 7)**
- On RX: when the RX character(s) match Xoff, TX stops after the current character. ISR[4] is set if IER[5] = 1. TX resumes when Xon is matched, and the flag clears.
- Xon/Xoff characters that are matched for flow control are **not** put in the RX FIFO.
- Xon-Any (MCR[5]): any RX character resumes TX. The character is stored in the FIFO unless it is an Xon/Xoff character and flow control is active.
- TX side, Xoff: the Xoff character(s) are sent "two-character-times" after the RX FIFO crosses the trigger level (Table 7 footnote).
- TX side, Xon: the Xon character(s) are sent once the RX FIFO is "less than one trigger level below the programmed trigger level".
- Table 7:

| Trigger | INT at | Xoff sent at (after 2-char delay) | Xon sent at |
|---|---|---|---|
| 1 | 1 | 1 | 0 |
| 4 | 4 | 4 | 1 |
| 8 | 8 | 8 | 4 |
| 14 | 14 | 14 | 8 |

**Special character detect (p18, p34)**
- With EFR[5] = 1, each RX character is compared with XOFF2. On a match the character goes into the FIFO and ISR[4] is set. The flag auto-clears after the next received character.
- With EFR[1:0] = 10 (compare Xon1/Xoff1), flow control and special character detect work independently.
- With EFR[1:0] = 01 (compare Xon2/Xoff2), XOFF2 is **not** put in the FIFO. It generates both the Xoff interrupt and the special character interrupt (if IER[5] = 1).

**IrDA (p19, p30)**
- Enabled by MCR[6]. IrDA 1.0.
- TX: each 0 data bit (start bit included) becomes a high pulse 3/16 of a bit wide. The IR output idles at 0 and stays 0 for 1 bits.
- Pulse position in the cell: Fig 12 draws each pulse at the start of its 0-bit cell. It also has "1/2 Bit Time" and "3/16 Bit Time" dimension marks near the last pulse, but the drawing is not precise enough to place the pulse exactly. Standard IrDA SIR centres the pulse in the bit cell; that is the planner's choice.
- RX: the idle level is 0. Fig 12 shows each RX high pulse decoding to a **0** data bit (start bit and zeros), with no pulse meaning 1. The decoded data starts a "1/16 Clock Delay" after the pulse.
- The p19 text says "Each time it senses a light pulse, it returns a logic 1 to the data bit stream". This contradicts the figure; follow the figure (pulse = 0).

**Sleep / PowerSave (p20)**
- Sleep is entered when all of these are true:
  - both channels have ISR[0] = 1 (nothing pending);
  - the divisor is non-zero;
  - both channels have IER[4] = 1;
  - MSR[3:0] = 0 (no pending modem deltas);
  - RXA and RXB are idle high.
- In sleep the oscillator stops; XTAL2 shows no clock.
- Wake-up sources (PowerSave not active):
  - RX start bit (1→0);
  - a byte written to THR/FIFO;
  - a change on CTS#, DSR#, CD# or RI#.
- After waking, the part re-sleeps automatically once all interrupts are serviced. A wake from a modem input needs an MSR read.
- It stays out of sleep only when IER[4] = 0.
- The first RX characters may be lost while the oscillator starts (not modeled).
- PowerSave = PWRSAVE pin high **and** in sleep. The bus inputs are isolated (writes and reads are ignored). Only an RX start bit or a modem-input change wakes it. It returns to PowerSave after the MSR read and interrupt servicing. Leaving PowerSave needs IER[4] = 0 and/or PWRSAVE = 0.
- The datasheet does not say how the host reaches IER while isolated (it presumably must wake the part first, by a modem/RX event).
- Read-data behavior while isolated is not stated.

**Loopback (p21, Fig 13)**
- Enabled by MCR[4].
- The TSR output feeds the RSR input internally. The TX pin is held at 1. The RX pin is ignored.
- The RTSx#, DTRx# and OP2x# pins are held at 1 (Fig 13: switched to VCC). CTS#, DSR#, RI# and CD# inputs are ignored.
- Internal wiring: RTS → CTS, DTR → DSR, OP1 (MCR[2]) → RI, OP2 (MCR[3]) → CD.
- p21 says "All regular UART functions operate normally". Interrupts still work in loopback (typical 16550).
- Whether INTx stays enabled by MCR[3] in loopback is not stated.

**Reset (p6, p9, Table 14 p35)**
- RESET pin is active high in Intel mode and active low (RESET#) in Motorola mode. Minimum pulse 40 ns (TRST p38).
- p9 says "active high pulse" without the Motorola qualifier.
- Resets both channels.
- Register values: see the register table. RHR, THR, DLL and DLM are undefined (XX).
- Outputs during and after reset: TX = 1, OP2# = 1, RTS# = 1, DTR# = 1, RXRDY# = 1, TXRDY# = 0, INT = 3-state. In Motorola mode IRQ# would be inactive (released), since no interrupt is pending; not stated explicitly.
- The receiver is ignored during reset (p6).

**Undefined / reserved**
- FCR[5:4] reserved.
- MCR[2] reserved (no pin).
- LSR/MSR writes reserved.
- Reading with both CSA# and CSB# low: forbidden.
- Access to 000/001 when LCR = 0xBF: not specified.
- DVID/DREV reads require DLL = DLM = 0.

### Timing (AC table pp37–38; TA −40…+85 °C, 70 pF; columns 2.5 V / 3.3 V / 5.0 V)

| Symbol | Parameter | 2.5 V | 3.3 V | 5.0 V | min/max | Page |
|---|---|---|---|---|---|---|
| – | Crystal frequency | 16 | 20 | 24 MHz | max | 37 |
| CLK | External clock low/high time | 31 | 17 | 10 ns | min | 37 |
| OSC | External clock frequency | 16 | 30 | 50 MHz | max | 37 |
| TAS / TAH | Address setup / hold (16) | 10 / 10 | 10 / 10 | 10 / 10 ns | min | 37 |
| TCS | Chip select width (16) | 150 | 75 | 50 ns | min | 37 |
| TRD | IOR# strobe width | 150 | 75 | 50 ns | min | 37 |
| TDY | Read cycle delay | 150 | 75 | 50 ns | min | 37 |
| TRDV | Data access time (from IOR# fall) | 125 | 70 | 45 ns | max | 37 |
| TDD | Data disable time (from IOR# rise) | 0–45 | 0–30 | 0–30 ns | min–max | 37 |
| TWR | IOW# strobe width | 150 | 75 | 50 ns | min | 37 |
| TDY | Write cycle delay | 150 | 75 | 50 ns | min | 37 |
| TDS / TDH | Data setup / hold (to IOW# rise) | 25 / 15 | 20 / 10 | 15 / 10 ns | min | 37 |
| TADS / TADH | Address setup / hold (68) | 10 / 10 | 10 / 10 | 10 / 10 ns | min | 37 |
| TRWS | R/W# setup to CS# | 10 | 10 | 10 ns | min | 37 |
| TRDA | Read data access (from CS# fall) | 125 | 70 | 45 ns | max | 37 |
| TRDH | Read data disable (from CS# rise) | 45 | 30 | 30 ns | max | 37 |
| TWDS / TWDH | Write data setup / hold (68) | 25 / 15 | 20 / 10 | 15 / 10 ns | min | 37 |
| TRWH | R/W# hold | 15 | 10 | 10 ns | min | 37 |
| TCSL | CS# width (68) | 150 | 75 | 50 ns | min | 37 |
| TCSD | CS# cycle delay (68) | 150 | 75 | 50 ns | min | 37 |
| TWDO | IOW# to modem output (RTS#/DTR#) | 150 | 75 | 50 ns | max | 37 |
| TMOD | Modem input to set INT | 150 | 75 | 50 ns | max | 37 |
| TRSI | IOR# (MSR read) to reset INT | 150 | 75 | 50 ns | max | 37 |
| TSSI | Stop bit to set interrupt | 1 Bclk | 1 | 1 | max | 38 |
| TRRI | IOR# to reset interrupt | 150 | 75 | 50 ns | max | 38 |
| TSI | Stop to interrupt (TX empty) | 150 | 75 | 50 ns | max | 38 |
| TINT | Initial INT reset to transmit start | 8–24 Bclk | 8–24 | 8–24 | min–max | 38 |
| TWRI | IOW# to reset interrupt | 150 | 75 | 50 ns | max | 38 |
| TSSR | Stop to set RXRDY# | 1 Bclk | 1 | 1 | max | 38 |
| TRR | IOR# to reset RXRDY# | 150 | 75 | 50 ns | max | 38 |
| TWT | IOW# to set TXRDY# | 150 | 75 | 50 ns | max | 38 |
| TSRT | Center of start bit to reset TXRDY# | 8 Bclk | 8 | 8 | max | 38 |
| TRST | Reset pulse width | 40 | 40 | 40 ns | min | 38 |
| N | Baud divisor | 1 … 2^16−1 | | | | 38 |
| Bclk | Baud clock = 16× data rate | | | | | 38 |

Diagrams:
- Intel read/write: Figs 16/17, pp39–40.
- Motorola read/write: Figs 18/19, pp40–41.
- Modem I/O: Fig 15, p39.
- RX/TX ready and interrupt: Figs 20–25, pp41–44.
- Fig 21/24 footnote: "INT is cleared when the ISR is read or when data is loaded into the THR" / "…at least one character in the FIFO".
- Fig 22/23: INT and RXRDY# are set TSSI/TSSR after the stop bit of the character that reaches the trigger level.

### VCC
- Recommended range 2.25–5.5 V. Only one grade: industrial −40…+85 °C (pp1, 6, 36).
- DC table columns are 2.5 V / 3.3 V / 5.0 V.
- All inputs are 5 V tolerant **except XTAL1** (p6, p10): VIH maximum is 5.5 V for every column (p36), abs-max "Voltage at any pin" is GND−0.3 to +5.5 V (p36), but XTAL1 should not exceed VCC (p10).
  - Suggested inputMax 5.5 V for logic inputs, VCC for XTAL1.
- VOL is 0.4 V at 6 / 4 / 2 mA. VOH is 2.4 V at −6 mA, 2.0 V at −1 mA, 1.8 V at −400 µA (column assignment as printed: 5 V / 3.3 V / 2.5 V respectively).
- ICC max 1 / 1.3 / 3 mA. Sleep/PowerSave current 6 / 15 / 30 µA.

### Package drawings
- **48-TQFP (census part), p45**, "PACKAGE DIMENSIONS (48 PIN TQFP - 7 X 7 X 1 mm)". No drawing code printed; it is a JEDEC MS-026-style 7×7 LQFP. Millimetres are the controlling unit.

  | Symbol | Min | Max | Note |
  |---|---|---|---|
  | A | 1.00 | 1.20 | |
  | A1 | 0.05 | 0.15 | |
  | A2 | 0.95 | 1.05 | |
  | B (lead width) | 0.17 | 0.27 | mid 0.22 |
  | C (lead thickness) | 0.09 | 0.20 | |
  | D (tip-to-tip span) | 8.80 | 9.20 | mid 9.00 |
  | D1 (body) | 6.90 | 7.10 | mid 7.00 |
  | e (pitch) | 0.50 BSC | | |
  | L (foot length) | 0.45 | 0.75 | mid 0.60 |
  | α | 0° | 7° | |

  Square package (D and D1 are used for both axes). Pin 1 is at the upper end of the left side, as drawn on p45 (pins 1–12 left, 13–24 bottom, 25–36 right, 37–48 top, counter-clockwise). No land pattern is printed.
- **32-QFN (no census part), p46**: 5×5×0.9 mm.
  - A 0.80–1.00, A1 0–0.05, A3 0.15–0.25.
  - D (body) 4.90–5.10.
  - D2 (exposed pad) 3.50–3.80, "device-dependent, typical tolerance 0.3 mm".
  - b 0.18–0.30, e 0.50 BSC, L 0.35–0.45, k ≥ 0.20.
  - No land pattern is printed. PCB pad guidance is on p6.

### Obsolete
- None. The census row is Active (qty 249). Ordering table p2 lists both IM and IL as Active.

### Notes
- Several active-level and polarity details:
  - INTA/INTB are active high and 3-state in Intel mode; IRQ# is active-low open drain in Motorola mode.
  - In Motorola mode the INTB pin drives 0. It is a driven output, not 3-state.
  - The RESET polarity depends on 16/68#.
  - Model 16/68# as a static mode pin; the datasheet does not describe changing it on the fly.
- p16 (§2.12) says the RTS interrupt is enabled "through IER bit-6 … ISR bit-5 will be set". §4.4.1 says the CTS/RTS interrupts are only for low→high transitions during auto flow control.
- p8: the Figure 3 title says "XR16L2751" (copy error); the figure shows the L2551 interconnections.
- Pin text OP2A# (p5): with MCR[3] = 1, INTA is active and OP2A# = 0.
- §2.12 p16 also says to enable the RTS interrupt "after setting EFR bit-4".
- The TOC lists Figure 14 on p38 and Figures 15/16 on p39; the page numbers match.
- The Xon character for trigger 1 is sent at 0 characters (Table 7), i.e. when the FIFO is empty.
- Nothing on this page set was unreadable. Pin drawings and AC tables were checked against page images (pp2, 21, 37, 38, 40, 45).
## XR16L2552

- **Title / function:** "XR16L2552 — 2.25V TO 5.5V DUART WITH 16-BYTE FIFO" (Exar, Rev. 1.1.2, Sept 2010). Dual 16C550-compatible UART with 16-byte TX/RX FIFOs, 4 RX trigger levels, auto RTS/CTS, auto Xon/Xoff + special character detect, IrDA 1.0 encoder/decoder, sleep with auto-wake, BRG prescaler ÷1/÷4, Alternate Function Register (concurrent write to both channels; multi-function MF# output = OP2#/BAUDOUT#/RXRDY#), device ID/revision registers. "Improved version of the ST16C2552 with lower operating voltages and 5 V tolerant inputs"; pin/function compatible with National PC16552; pin-compatible with ST16C2552, XR16L2752, XR16C2852 in PLCC-44 (p1).
- **Datasheet:** `datasheets/controller-uart/xr16l2552.pdf` (45 pp). Pinouts p2; ordering p2; pin descriptions p3–6; functional p7–20; registers p21–35; DC p36; AC p37–38; timing diagrams p38–42; TQFP-48 outline p43; PLCC-44 outline p44.
- **Nearest siblings:** ST16C2552 (same pinout/register map, 5 V only, not 5 V tolerant); differs from XR16L2752 (64-byte FIFO) and XR16C2852 (128-byte FIFO). Versus a plain 16C550 channel: adds EFR/Xon/Xoff bank at LCR=0xBF, AFR/DREV/DVID in the divisor bank, IER[7:4], ISR[5:4], MCR[7:5].

### Parts

| Census part | Package | Pins | Pinout name | Grade | VCC |
|---|---|---|---|---|---|
| XR16L2552IJ | 44-Lead PLCC ("J") | 44 | plcc44 | I = −40 to +85 °C (p2) | 2.25–5.5 V |
| XR16L2552IJTR | 44-Lead PLCC, tape & reel (TR suffix — not in the PDF's ordering table; TR = tape and reel per census note) | 44 | plcc44 | −40 to +85 °C | 2.25–5.5 V |
| XR16L2552IM-F | 48-Lead TQFP 7x7x1.0 mm ("M") | 48 | tqfp48 | −40 to +85 °C | 2.25–5.5 V |

Ordering table (p2) lists only XR16L2552IM (48-TQFP) and XR16L2552IJ (44-PLCC), both −40..+85 °C, Active. The `-F` suffix (RoHS/green) and `TR` suffix are not in this PDF; they don't change package or grade. Only one speed/voltage grade exists (speed scales with VCC, see Timing). All census rows Active; none obsolete.

### Pinouts

Overbars (#) dropped per repo convention; all `#` signals are active low. From Fig. 2 (p2, verified on page image) and pin table (p3–6).

**tqfp48** (48-TQFP, pin 1 top-left of left side, counter-clockwise):

| Pin | Name | Role |
|---|---|---|
| 1 | D5 | inout |
| 2 | D6 | inout |
| 3 | D7 | inout |
| 4 | A0 | input |
| 5 | XTAL1 | input |
| 6 | GND | ground |
| 7 | XTAL2 | output |
| 8 | RXRDYB | output |
| 9 | A1 | input |
| 10 | A2 | input |
| 11 | CHSEL | input |
| 12 | INTB | output |
| 13 | CS | input |
| 14 | MFB | output |
| 15 | IOW | input |
| 16 | RESET | input |
| 17 | GND | ground |
| 18 | RTSB | output |
| 19 | NC | nc |
| 20 | IOR | input |
| 21 | RXB | input |
| 22 | TXB | output |
| 23 | DTRB | output |
| 24 | CTSB | input |
| 25 | DSRB | input |
| 26 | CDB | input |
| 27 | RIB | input |
| 28 | TXRDYB | output |
| 29 | VCC | power |
| 30 | INTA | output |
| 31 | RXRDYA | output |
| 32 | MFA | output |
| 33 | RTSA | output |
| 34 | DTRA | output |
| 35 | TXA | output |
| 36 | RXA | input |
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

No exposed pad.

**plcc44** (44-PLCC, pin 1 top-centre, counter-clockwise):

| Pin | Name | Role |
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

The PLCC has no separate RXRDYA/RXRDYB pins. RXRDY is available only via MFx with AFR[2:1]=10 (p3–5).

**Mode-selected pins:** MFA/MFB (TQFP 32/14, PLCC 35/19) output OP2x#, BAUDOUTx# or RXRDYx#, chosen by that channel's AFR[2:1] (p5, p32). There is no Intel/Motorola mode pin; the bus is Intel only.

**Pin notes (p3–5):** INTA/INTB are active-high, always-driven outputs. The datasheet has no 3-state or OUT2 gating and no open-drain option (pin type O). Unused RX: tie high or use a 100 kΩ pull-up. Unused CTS/DSR/CD/RI: tie to VCC. DSR, CD and RI "have no effect on the UART" (they only feed MSR). RESET is active high.

### Host interface

- Intel bus, asynchronous. No clock is needed for bus cycles (p7). Signals are CS (active low), IOR (active low), IOW (active low), A2..A0, D7..D0 and CHSEL.
- **Channel select (Table 1, p8):** CS=1 deselects. CS=0, CHSEL=1 selects channel A. CS=0, CHSEL=0 selects channel B. With AFR[0]=1, a write goes to both channels whatever CHSEL is. Reads still follow CHSEL (p8, p32).
- **Read (p3):** the falling edge of IOR starts an internal read of the register at A2..A0. Data goes on D7..D0 and the host latches it on the rising edge of IOR. D drives while CS=0 and IOR=0, then floats within TDD after IOR rises (Fig 16, p39).
- **Write (p3):** the falling edge of IOW starts the cycle and the rising edge transfers D7..D0 into the register. Data setup TDS and hold TDH are measured to the IOW rising edge (Fig 17, p39).
- Address setup TAS and hold TAH are measured around the strobe, with CS framing it (Fig 16/17). There is no ALE and no DDIS output.

### Registers (per channel; Table 7 p21, Table 8 p22–23, descriptions p23–34)

**Access map**

| A2..A0 | Condition | Read | Write |
|---|---|---|---|
| 000 | LCR[7]=0 | RHR | THR |
| 001 | LCR[7]=0 | IER | IER |
| 010 | LCR[7]=0 | ISR | FCR |
| 011 | any | LCR | LCR |
| 100 | LCR≠0xBF | MCR | MCR |
| 101 | LCR≠0xBF | LSR | reserved |
| 110 | LCR≠0xBF | MSR | reserved |
| 111 | LCR≠0xBF | SPR | SPR |
| 000 | LCR[7]=1, LCR≠0xBF | DLL | DLL |
| 001 | LCR[7]=1, LCR≠0xBF | DLM | DLM |
| 010 | LCR[7]=1, LCR≠0xBF | AFR | AFR |
| 000 | LCR[7]=1, LCR≠0xBF, DLL=0x00 and DLM=0x00 | DREV | (Table 7 says R/W; Table 8 says RD) |
| 001 | same | DVID (=0x02) | (same note) |
| 010 | LCR=0xBF | EFR | EFR |
| 100 | LCR=0xBF | XON1 | XON1 |
| 101 | LCR=0xBF | XON2 | XON2 |
| 110 | LCR=0xBF | XOFF1 | XOFF1 |
| 111 | LCR=0xBF | XOFF2 | XOFF2 |

Undocumented: addresses 000 and 001 while LCR=0xBF. Table 7 restricts DLL/DLM to LCR≠0xBF and lists nothing at 0xBF. Sibling Exar parts (ST16C2552 family) map DLL/DLM there; this datasheet doesn't say.

DREV/DVID mechanism (p7, p32): with LCR[7]=1, write DLL=0 and DLM=0. A read of address 001 then returns DVID 0x02, and a read of 000 returns DREV (0x01 = revision A). It is unclear whether this also overrides the DLL/DLM write paths, or whether reading only the zero divisor returns ID/REV.

**Reset values (Table 13, p35):** DLL XX, DLM XX, AFR 00, RHR XX, THR XX, IER 00, FCR 00, ISR 01, LCR 00, MCR 00, LSR 60, MSR[3:0]=0 with MSR[7:4] = inverted input levels, SPR FF, EFR 00, XON1/XON2/XOFF1/XOFF2 00.

**Bit definitions**

- **RHR (R)** — receive data, first-received FIFO byte. The FIFO is 16 × 11 bits, the 3 extra bits being the PE/FE/BI tags (p13).
- **THR (W)** — transmit data. Bit 0 is sent first (p12).
- **IER (R/W):**
  - [0] RX data ready / time-out interrupt enable.
  - [1] THR-empty interrupt enable ("whenever TX FIFO becomes empty").
  - [2] RX line status interrupt enable (LSR[1..4]).
  - [3] modem status interrupt enable.
  - [4] Sleep mode enable — needs EFR[4]=1.
  - [5] Xoff / special-char interrupt enable — needs EFR[4]=1.
  - [6] RTS# interrupt enable (RTS# pin goes low→high) — needs EFR[4]=1.
  - [7] CTS# interrupt enable (CTS# pin goes low→high) — needs EFR[4]=1.
  - Shaded bits [7:4] read 0 / can't be changed unless EFR[4]=1 (Table 8 shows "0/"). (p24–25)
- **ISR (R):**
  - [0] 0 = interrupt pending, 1 = none.
  - [3:1] source code (Table 9).
  - [4] Xoff/special-char (needs EFR[4]).
  - [5] CTS/RTS low→high (needs EFR[4]).
  - [7:6] = 11 when FIFOs enabled, 00 when disabled. (p26)
- **FCR (W):**
  - [0] FIFO enable for both TX and RX. It must be 1 in the same write for the other bits to take effect.
  - [1] RX FIFO reset: clears pointers and count, not the RSR. Self-clearing. Active only if FCR[0]=1.
  - [2] TX FIFO reset: same, but the TSR isn't touched.
  - [3] DMA mode select (TXRDY/RXRDY behaviour).
  - [5:4] reserved (Table 8 shows 0, 0).
  - [7:6] RX trigger: 00=1, 01=4, 10=8, 11=14 (Table 10, p27).
  - There is no TX trigger. Table 8's shading text says EFR[4] enables FCR[5:4], but the FCR section calls them reserved.
- **LCR (R/W):**
  - [1:0] word length: 00=5, 01=6, 10=7, 11=8.
  - [2] stop bits: 0 = 1 stop; 1 = 1.5 stops for 5-bit words, 2 stops for 6–8 bits.
  - [3] parity enable.
  - [4] even parity (0 = odd).
  - [5] forced parity: with [3]=1, [5]=1,[4]=0 forces parity 1 and [5]=1,[4]=1 forces parity 0.
  - [6] TX break: TX forced to 0 until cleared.
  - [7] divisor latch access. (p27–28; Table 11)
- **MCR (R/W):**
  - [0] DTR: 1 → DTR# pin = 0.
  - [1] RTS: 1 → RTS# pin = 0.
  - [2] OP1: no pin; in loopback it drives internal RI. Table 8 labels it "Rsvd (OP1#)".
  - [3] OP2: 1 → OP2# = 0 on MF# when AFR[2:1]=00; in loopback it drives internal CD.
  - [4] internal loopback.
  - [5] Xon-Any enable (needs EFR[4]).
  - [6] IrDA enable (needs EFR[4]).
  - [7] clock prescaler: 0 = ÷1, 1 = ÷4 (needs EFR[4]).
  - (p29)
- **LSR (R):**
  - [0] RX data ready: set when a char moves to RHR/FIFO, cleared when the FIFO is empty.
  - [1] overrun: a char arrived while the FIFO was full; the RSR byte is overwritten and not transferred, and the FIFO is intact.
  - [2] parity error tag, for the char now in RHR.
  - [3] framing error tag (invalid stop), for the char in RHR.
  - [4] break tag: RX=0 for a full character frame. Only one break char is loaded into the FIFO. "Indication remains until RX returns to mark".
  - [5] THR empty: in FIFO mode, TX FIFO empty; cleared when ≥1 byte.
  - [6] THR and TSR both empty.
  - [7] RX FIFO global error: ≥1 PE/FE/BI anywhere in the FIFO, cleared when none remain.
  - (p29–30)
  - Clearing LSR[1:4] — the interrupt clears on an LSR read; the "flags and tags not cleared until character(s) that generated the interrupt(s) has been emptied or cleared from FIFO" (p25). The datasheet doesn't explicitly say that reading LSR clears LSR[1] (standard 16550 behaviour is that it does).
- **MSR (R):**
  - [0] ΔCTS.
  - [1] ΔDSR.
  - [2] trailing-edge RI: the RI# pin goes 0→1.
  - [3] ΔCD.
  - [4] CTS = NOT CTS# pin (loopback: = MCR[1]).
  - [5] DSR = NOT DSR# (loopback: = MCR[0]).
  - [6] RI = NOT RI# (loopback: = MCR[2]).
  - [7] CD = NOT CD# (loopback: = MCR[3]).
  - The deltas are set on change "since last monitored" and cleared by an MSR read (p25, p30–31).
- **SPR (R/W):** scratch. Reset 0xFF; kept through sleep (p31).
- **DLL/DLM (R/W):** 16-bit divisor DLM:DLL. Reset value undefined (XX).
- **AFR (R/W):**
  - [0] concurrent write. "The external CPU can set or clear this bit by accessing either register set" (p32). Ambiguous whether AFR[0] is one shared bit or per-channel.
  - [2:1] MF# function: 00 = OP2# (default), 01 = BAUDOUT#, 10 = RXRDY#, 11 = reserved.
  - [7:3] reserved, reset 0.
  - The user must make sure LCR[7] matches on both channels before a concurrent write to addresses 0, 1 or 2 (p32).
- **DREV (R):** revision, 0x01 = rev A. **DVID (R):** 0x02 (bits 00000010).
- **EFR (R/W, LCR=0xBF):**
  - [3:0] software flow-control select (Table 12, below).
  - [4] enhanced-function enable/latch: 1 = IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5] may be modified; 0 = those values are latched/saved. After reset they are 0.
  - [5] special-character detect (compare against XOFF2).
  - [6] auto-RTS.
  - [7] auto-CTS.
  - "Whenever changing the TX or RX flow control bits, always reset all bits back to logic 0 (disable) before programming a new setting" (p32–34).
- **XON1/XON2/XOFF1/XOFF2 (R/W):** 8-bit flow-control characters. Only the low (word-length) bits are compared; bit 0 = the first received data bit (p17, p34).

**Table 12 — EFR[3:0] (p33)**

| EFR[3:2] | Transmit side |
|---|---|
| 00 | no TX flow control |
| 10 | send Xon1/Xoff1 |
| 01 | send Xon2/Xoff2 |
| 11 | send Xon1+Xon2 / Xoff1+Xoff2 (two-char sequences) |

| EFR[1:0] | Receive side |
|---|---|
| 00 | no RX flow control |
| 10 | compare Xon1/Xoff1 |
| 01 | compare Xon2/Xoff2 |

EFR[1:0]=11 depends on [3:2]:
- With [3:2]=10 or 01, the receiver compares "Xon1 **or** Xon2, Xoff1 **or** Xoff2".
- With [3:2]=11 or 00, the receiver compares "Xon1 **and** Xon2, Xoff1 **and** Xoff2" (sequential pair).

### Behavior

**Baud generator (p10–11, p29, p31)**
- Clock: crystal on XTAL1/XTAL2 (1.8432–24 MHz, Fig 4; parallel-resonant, 500 kΩ–1 MΩ feedback, 22–47 pF caps), or an external clock on XTAL1.
- XTAL2 is the oscillator/buffered-clock output. It stops in sleep (p19).
- Prescaler: MCR[7] (÷1 or ÷4).
- Baud = (clock / prescaler / 16) / divisor, with divisor 1..65535 (2^16−1).
- One BRG per channel serves both TX and RX of that channel. There are no separate RCLK/BAUDOUT pins.
- BAUDOUT# (16× clock) appears on MFx when AFR[2:1]=01.
- Sampling is 16× only.
- Divisor 0 is not specified for baud. A 0 divisor blocks sleep entry and gates the DVID/DREV readback.

**Transmitter (p12)**
- Start bit, data (LSB first), optional parity, then 1/1.5/2 stops. A bit time is 16 BRG clocks.
- LSR[5]: set when the THR byte moves to the TSR (non-FIFO), or when the FIFO is empty (FIFO mode).
- LSR[6]: set when the THR/FIFO and the TSR are all empty.
- Break: LCR[6] forces TX=0.
- TX idles at 1, or at 0 in IrDA mode.
- Auto-CTS: when CTS# goes high, TX stops after the stop bit of the character in progress, and resumes when CTS# goes low (p15, p34).
- Xoff received: TX halts after the current character (p17).
- Xon-Any (MCR[5]=1): any received char resumes TX. That char goes into the RX FIFO unless it is an Xon/Xoff char under Xon/Xoff flow control (p29).

**Receiver (p13)**
- The falling edge starts a 16× counter. At 8 clocks the start bit is re-sampled and must still be 0, else it is a false start and ignored.
- Data, parity and stop bits are each sampled at mid-bit.
- Errors go in LSR[4:2] as per-byte tags that move with the FIFO. They update immediately when the RHR is read.
- Break: RX=0 for a full frame. One zero char with BI is loaded into the FIFO. Re-arms once RX returns to 1 (p30).
- Overrun: a new char completes while the FIFO (or RHR in non-FIFO mode) is full. The RSR char is discarded and LSR[1] is set; the interrupt fires immediately (p24, p30).

**RX timeout (p13, p25)**
- Fires when no data is received for "4 word lengths as defined by LCR[1:0] plus 12 bits time" (≈3.7–4.6 char times) while the FIFO holds data (timeout ISR code 0x0C).
- Cleared by reading RHR.

**FIFOs (p6, p8–9, p26–27)**
- 16 TX + 16 RX bytes. FCR[0] enables both.
- FCR[1]/FCR[2] reset the pointers.
- RX trigger is 1/4/8/14. There is no TX trigger level, and the TX interrupt means the FIFO is empty.
- Table 2 (p9):

| Pin | FIFO disabled (FCR[0]=0) | FIFO, DMA off (FCR[3]=0) | FIFO, DMA on (FCR[3]=1) |
|---|---|---|---|
| RXRDY# | 0 = 1 byte, 1 = no data | 0 = ≥1 byte in FIFO, 1 = empty | goes 1→0 when FIFO reaches trigger or timeout; goes 0→1 when FIFO empties |
| TXRDY# | 0 = THR empty, 1 = byte in THR | 0 = FIFO empty, 1 = ≥1 byte | 0 = ≥1 empty location, 1 = FIFO full |

- "DMA mode" is a legacy term; the device has no DMA request/ack (p8).

**INT pins (Table 3/4, p9)**
- TX: FIFO disabled → INT=1 when THR empty. FIFO enabled → INT=1 when FIFO empty.
- RX: FIFO disabled → INT=1 when 1 byte present. FIFO enabled → INT=1 when at or above trigger.
- These tables describe the INT level for each enabled source; the pin is the OR of the enabled pending interrupts. There is no OUT2 gate. INT=0 at reset.

**Interrupt priority (Table 9, p26). ISR[5:0] values:**

| Priority | ISR[5:0] | Source | Set (p25) | Clear (p25) |
|---|---|---|---|---|
| 1 | 000110 (0x06) | LSR: receiver line status | any of LSR[1..4]; LSR[1] at reception, LSR[2–4] when the erroneous char reaches RHR (p24) | read LSR |
| 2 | 001100 (0x0C) | RXRDY data time-out | RX timeout | read RHR |
| 3 | 000100 (0x04) | RXRDY received data ready | RX trigger reached (or 1 byte in non-FIFO mode) | FIFO falls below trigger |
| 4 | 000010 (0x02) | TXRDY transmit ready | TX FIFO/THR empty | read ISR or write THR |
| 5 | 000000 (0x00) | MSR: modem status | MSR[3:0] | read MSR |
| 6 | 010000 (0x10) | Received Xoff or special character | Xoff/special char detected | Xoff: read ISR or Xon received; special: read ISR or the next char received |
| 7 | 100000 (0x20) | CTS#/RTS# change of state (low→high) | CTS# goes high with auto-CTS on, or RTS# goes high due to auto-RTS | read MSR |
| – | 000001 (0x01) | none | | |

ISR[7:6] adds FIFO-enabled bits (0xC0).

**Auto-RTS (p15, p34; Fig 10 p14)**
- EFR[6]=1, and the host must first set MCR[1]=1 (RTS# low).
- RTS# goes high when the RX FIFO reaches "the next upper trigger level" and goes low again when it falls below "the next lower trigger level".
- Fig 10 example: trigger 8 → RTS# goes high at 14, low again at 4.
- The datasheet doesn't define the upper level for trigger 14 or the lower level for trigger 1 (see Notes).
- IER[6] raises the priority-7 interrupt on RTS# low→high.
- Not supported in loopback (p20).

**Auto-CTS:** EFR[7]=1 (behaviour above). IER[7] raises the priority-7 interrupt on CTS# low→high.

**Auto Xon/Xoff TX of flow chars (Table 6, p17):**
- Xoff is sent 2 character times after the RX FIFO reaches the trigger.
- Xon is sent when the FIFO falls to the level below.
- Flow-control chars received are not put in the FIFO.
- Two-char mode compares two consecutive received chars.

| RX trigger | INT at | Xoff sent at | Xon sent at |
|---|---|---|---|
| 1 | 1 | 1* | 0 |
| 4 | 4 | 4* | 1 |
| 8 | 8 | 8* | 4 |
| 14 | 14 | 14* | 8 |

\* plus 2 character times.

**Special character (EFR[5], p17, p33)**
- Each received char is compared to XOFF2 (word-length bits only). On a match the char goes into the FIFO and ISR[4] is set.
- If EFR[1:0]=10 (Xon1/Xoff1 compare), flow control and special-char both work normally.
- If EFR[1:0]=01 (Xon2/Xoff2), Xoff2 does **not** go into the FIFO and raises both the Xoff interrupt and the special-char interrupt (if IER[5]).

**IrDA (MCR[6], p18, Fig 12)**
- TX: each 0 bit becomes a high pulse 3/16 of a bit wide. Fig 12 shows it centred in the bit (begins at 7/16 of the bit). A 1 bit stays low, and TX idles low.
- RX: idles low. A high pulse in a bit cell decodes as data 0 (Fig 12: "1/16 clock delay" from pulse to decoded edge), and no pulse decodes as 1.
- The prose (p18) says "each time it senses a light pulse, it returns a logic 1", which contradicts Fig 12; the figure follows the IrDA convention (pulse = 0).

**Sleep (IER[4], p19, p36)**

Enters sleep only when all of these hold:
- Both channels have no interrupt pending (ISR[0]=1).
- The divisor is non-zero.
- IER[4]=1 on **both** channels.
- MSR[3:0]=0.
- RX pins idle high.

In sleep the oscillator stops (no XTAL2 output) and registers are kept (SPR is preserved).

Wakes on any of:
- an RX start bit (1→0);
- a write to THR/FIFO;
- a change on CTS#, DSR#, CD# or RI#.

It goes back to sleep once all interrupts are cleared. If woken by a modem input, the host must read MSR. It stays in sleep mode until IER[4]=0. Oscillator start-up may lose the first RX chars.

**Loopback (MCR[4], p20, Fig 13)**
- The TSR output is internally routed to the RSR input.
- The TX pin is held at 1.
- RTS#, DTR# and OP2# (MF) pins go inactive (high). The datasheet says "RTS# and DTR# are de-asserted"; Fig 13 shows the TX, RTS, DTR and OP2 pin outputs tied to VCC.
- Modem inputs are disconnected. Internally, CTS = MCR[1], DSR = MCR[0], RI = MCR[2] (OP1) and CD = MCR[3] (OP2).
- The RX pin is ignored but must be held high to avoid a false break on exit.
- Auto RTS/CTS is not supported.

**MF# OP2#:** MFx = NOT MCR[3] when AFR[2:1]=00, so at reset MF = 1.

**Concurrent write:** AFR[0]=1 → IOW writes go to both channels (p8, p32).

**Reset (p5, p7, p35)**
- RESET is active high and needs a pulse longer than 40 ns.
- It resets registers per Table 13 in both channels.
- Outputs during and after reset: TX = 1, MF# = 1, RTS# = 1, DTR# = 1, TXRDY# = 0, INT = 0.
- The receiver input is ignored during reset.
- RXRDY# (TQFP pins 8/31) is not listed in Table 13. Implied value: 1 (no data), per Table 2.

**Not present:** no 9-bit/RS-485 auto-direction, no GPIO beyond the modem pins, no software reset register, no TX trigger levels, no FCTR/EMSR. FCR[5:4], AFR[7:3] and AFR[2:1]=11 are reserved; LSR/MSR addresses are reserved for writes.

### Timing (AC table p37–38; TA −40..+85 °C, 70 pF; all min/max as printed, no typicals)

| Symbol | Parameter | 2.5 V | 3.3 V | 5.0 V | Unit |
|---|---|---|---|---|---|
| — | Crystal freq max | 16 | 20 | 24 | MHz |
| CLK | Ext clock low/high time min | 31 | 17 | 10 | ns |
| OSC | Ext clock freq max | 16 | 30 | 50 | MHz |
| TAS | Address setup min | 10 | 10 | 10 | ns |
| TAH | Address hold min | 10 | 10 | 10 | ns |
| TCS | Chip select width min | 150 | 75 | 50 | ns |
| TRD | IOR strobe width min | 150 | 75 | 50 | ns |
| TDY | Read cycle delay min | 150 | 75 | 50 | ns |
| TRDV | Data access time max | 125 | 70 | 45 | ns |
| TDD | Data disable time (min/max) | 0/45 | 0/30 | 0/30 | ns |
| TWR | IOW strobe width min | 150 | 75 | 50 | ns |
| TDY | Write cycle delay min | 150 | 75 | 50 | ns |
| TDS | Data setup min | 25 | 20 | 15 | ns |
| TDH | Data hold min | 15 | 10 | 10 | ns |
| TWDO | IOW to modem output max | 150 | 75 | 50 | ns |
| TMOD | Modem input to INT set max | 150 | 75 | 50 | ns |
| TRSI | IOR to INT reset (MSR read) max | 150 | 75 | 50 | ns |
| TSSI | Stop to set interrupt max | 1 | 1 | 1 | Bclk |
| TRRI | IOR to reset interrupt max | 150 | 75 | 50 | ns |
| TSI | Stop to interrupt max | 150 | 75 | 50 | ns |
| TINT | Initial INT reset to transmit start (min/max) | 8/24 | 8/24 | 8/24 | Bclk |
| TWRI | IOW to reset interrupt max | 150 | 75 | 50 | ns |
| TSSR | Stop to set RXRDY# max | 1 | 1 | 1 | Bclk |
| TRR | IOR to reset RXRDY# max | 150 | 75 | 50 | ns |
| TWT | IOW to set TXRDY# max | 150 | 75 | 50 | ns |
| TSRT | Centre of start to reset TXRDY# max | 8 | 8 | 8 | Bclk |
| TRST | Reset pulse width min | 40 | 40 | 40 | ns |
| N | Divisor | 1..2^16−1 | | | |

- Bclk = 16× data-rate clock.
- Figures: clock Fig 14 (p38), modem Fig 15 (p38), read Fig 16 / write Fig 17 (p39), RX/TX ready Figs 18–23 (p40–42).
- p10: up to 64 MHz external clock (4 Mbps) at room temperature and 5 V with a 2 kΩ pull-up on XTAL2 (Fig 5/6). That is beyond the 50 MHz table maximum; the p1 headline is 3.125 Mbps at 50 MHz.

### VCC

- Recommended range: 2.25–5.5 V, single grade (p1, p5, p36).
- All inputs are 5 V tolerant at any VCC. Absolute max "voltage at any pin" is GND−0.3 to +5.5 V (p36), so inputMax = 5.5 V.
- Absolute max supply is 7 V.
- DC limits at 2.5 / 3.3 / 5.0 V (p36):

| Parameter | 2.5 V | 3.3 V | 5.0 V |
|---|---|---|---|
| VIL max (V) | 0.6 | 0.8 | 0.8 |
| VIH min (V) | 2.0 | 2.0 | 2.2 |
| VIHCK min, clock (V) | 2.0 | 2.4 | 3.0 |
| VILCK max, clock (V) | 0.2 | 0.6 | 0.6 |
| ICC max (mA) | 1 | 1.3 | 3 |
| ISLEEP max (µA) | 6 | 15 | 30 |

### Package drawings

- **48-TQFP 7x7x1.0 mm (p43)**, titled "PACKAGE DIMENSIONS (48 PIN TQFP - 7 X 7 X 1 mm)". No drawing code and no land pattern printed. Values in mm (control units):

| Dim | Range (mm) | Mid |
|---|---|---|
| A | 1.00–1.20 | — |
| A1 | 0.05–0.15 | — |
| A2 | 0.95–1.05 | — |
| B (lead width) | 0.17–0.27 | 0.22 |
| C (lead thickness) | 0.09–0.20 | — |
| D (tip-to-tip span) | 8.80–9.20 | 9.00 |
| D1 (body) | 6.90–7.10 | 7.00 |
| e (pitch) | 0.50 BSC | — |
| L (foot length) | 0.45–0.75 | 0.60 |
| α | 0–7° | — |

  12 leads per side. Pin 1 is at the left end of the bottom row in the drawing (orientation per drawing; the pinout in Fig 2 starts pin 1 at the top of the left side).
- **44-PLCC (p44)**, titled "44 LEAD PLASTIC LEADED CHIP CARRIER (PLCC) Rev. 1.00". No other code and no land pattern. Values in mm:

| Dim | Range (mm) | Mid |
|---|---|---|
| A | 4.19–4.57 | — |
| A1 | 2.29–3.05 | — |
| A2 | 0.51 min | — |
| B (lead width, lower) | 0.33–0.53 | 0.43 |
| B1 (upper lead width) | 0.66–0.81 | — |
| C | 0.19–0.32 | — |
| D (span over leads) | 17.40–17.65 | 17.53 |
| D1 (body) | 16.51–16.66 | 16.59 |
| D2 (footprint row centre to centre) | 14.99–16.00 | 15.50 |
| D3 (lead-row length) | 12.70 typ (= 10 × 1.27) | — |
| e (pitch) | 1.27 BSC | — |
| H1 | 1.07–1.42 | — |
| H2 | 1.07–1.22 | — |
| R | 0.64–1.14 | — |

  11 leads per side, pin 1 at top centre. J-lead.

### Obsolete

None. All three census rows are Active.

### Notes / datasheet errors

1. Fig 3 footnote (p7) says MF# becomes BAUDOUT# when "AFR[1:0]='01'". That's a typo for AFR[2:1]=01 (p32 table).
2. MCR[3] reset value conflicts. The MF# pin text (p5) says "MCR bit-3 defaults to a logic 1 condition after a reset". Table 13 (MCR=0x00) and the MCR[3] text say default 0 → OP2# = 1. Follow Table 13 (MF# = 1 at reset, consistent with the I/O reset table).
3. DREV/DVID are R/W in Table 7 but RD in Table 8 / §4.14–4.15. Treat as read-only.
4. Addresses 000/001 with LCR=0xBF are not defined (see Registers).
5. Auto-RTS hysteresis ("next upper/lower trigger level", p34) is only fully defined for trigger 4 and 8 (Fig 10: trigger 8 → 14/4). For trigger 1 and trigger 14 it isn't stated. The sibling XR16L2752/ST16C2552 datasheets may give the table, but this PDF doesn't.
6. The IrDA prose contradicts Fig 12 on RX decode polarity (see Behavior).
7. The sleep section (p19) and Figs 16/17 mention CSA#/CSB#, which this device doesn't have (it uses CS + CHSEL); this is copied text.
8. The headline speed (p1) is 3.125 Mbps at 50 MHz, while p6 says "4 Mbps at 5V" and p10 says 64 MHz with a 2 kΩ pull-up. The AC table maximum is 50 MHz.
9. ISR[5:4] is listed as "enabled" by EFR[4] — these are status bits that read 0 unless EFR[4]=1.
10. Census note: IJ/IJTR rows have blank package and VCC fields. They are decoded as PLCC-44 (J), 2.25–5.5 V, industrial.
## XR16M2550

- Title: "HIGH PERFORMANCE LOW VOLTAGE DUART WITH 16-BYTE FIFO" (Exar, May 2007, rev 1.0.2). Two independent 16550-compatible UART channels with 16-byte TX/RX FIFOs, fractional BRG, 16X/8X/4X sampling, auto RTS/CTS, auto Xon/Xoff, special char detect, IrDA 1.0, sleep mode, DVID/DREV. Intel bus only.
- Datasheet: `datasheets/controller-uart/xr16m2550.pdf` (47 pp; all pages used; key pages: pinouts p2, pin table p3-5, functional p7-20, registers p21-35, electrical p36-37, timing figs p38-42, packages p43-44). No "obsolete" watermark in this PDF.

### Parts
| census part | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|
| XR16M2550IL32 | 32-pin QFN 5x5x0.9 mm, exposed pad ("IL32") | 32 (+EP) | `qfn32` | industrial -40..+85 °C | 1.62–3.63 V |
| XR16M2550IM48 | 48-lead TQFP 7x7x1 mm ("IM48") | 48 | `tqfp48` | industrial -40..+85 °C | 1.62–3.63 V |

Ordering table p2: XR16M2550IL32 = 32-Pin QFN, XR16M2550IM48 = 48-Lead TQFP, both -40..+85 °C, Active. Only one speed/supply grade exists.

### Pinouts (p2 drawing, verified against image; p3-5 table)
`tqfp48` (Intel bus only):
```
1 D5 inout        13 XTAL1 input     25 NC nc          37 NC nc
2 D6 inout        14 XTAL2 output    26 A2 input       38 CTSA input
3 D7 inout        15 IOW input       27 A1 input       39 DSRA input
4 RXB input       16 CDB input       28 A0 input       40 CDA input
5 RXA input       17 GND ground      29 INTB output    41 RIA input
6 TXRDYB output   18 RXRDYB output   30 INTA output    42 VCC power
7 TXA output      19 IOR input       31 RXRDYA output  43 TXRDYA output
8 TXB output      20 DSRB input      32 OP2A output    44 D0 inout
9 OP2B output     21 RIB input       33 RTSA output    45 D1 inout
10 CSA input      22 RTSB output     34 DTRA output    46 D2 inout
11 CSB input      23 CTSB input      35 DTRB output    47 D3 inout
12 NC nc          24 NC nc           36 RESET input    48 D4 inout
```
`qfn32` (reduced pin set: no TXRDY/RXRDY, DTR, DSR, CD, RI, OP2 pins):
```
1 D6 inout     9 NC nc         17 NC nc        25 CTSA input
2 D7 inout     10 XTAL1 input  18 A2 input     26 VCC power
3 RXB input    11 XTAL2 output 19 A1 input     27 D0 inout
4 RXA input    12 IOW input    20 A0 input     28 D1 inout
5 TXA output   13 GND ground   21 INTB output  29 D2 inout
6 TXB output   14 IOR input    22 INTA output  30 D3 inout
7 CSA input    15 RTSB output  23 RTSA output  31 D4 inout
8 CSB input    16 CTSB input   24 RESET input  32 D5 inout
EP (center pad) = GND (p5: metallic, connect to GND)
```
Active levels: IOR, IOW, CSA, CSB, TXRDYx, RXRDYx, RTSx, CTSx, DTRx, DSRx, CDx, RIx, OP2x all active LOW; RESET active HIGH; INTA/INTB active HIGH and 3-state (see interrupts). On QFN32 the missing modem inputs behave as if inactive (not stated — the planner should treat internal DSR/CD/RI as deasserted, i.e. pin=HIGH; datasheet silent).

### Host interface (p3, p7-8, p37, p39 figs 15/16)
- Intel bus only: A2..A0, D7..D0, IOR (active low), IOW (active low), CSA, CSB (active low). Asynchronous, no clock needed for bus cycles (p7).
- Read: "The falling edge [of IOR#] instigates an internal read cycle and retrieves the data byte ... The data byte is placed on the data bus to allow the host processor to read it on the rising edge." D7..D0 driven while CSx low and IOR low; floats TDD after IOR rises.
- Write: "The falling edge [of IOW#] instigates an internal write cycle and the rising edge transfers the data byte on the data bus to an internal register" — data latched on IOW rising edge (TDS before / TDH after rising edge).
- Channel select (Table 1, p8): CSA=1,CSB=1 none; 0,1 = A; 1,0 = B; 0,0 = both selected — "useful ... to write to the same internal registers, but do not attempt to read from both uarts simultaneously" (simultaneous read = bus contention; model as X).
- No DDIS, no ALE, no address latch; address must be valid TAS before strobe and TAH after.

### Registers (Table 8 p21, Table 9 p22-23, §4 p23-35). Per channel, selected by CSA/CSB.
Access conditions:
| A2..A0 | condition | read | write |
|---|---|---|---|
| 000 | LCR[7]=0 | RHR | THR |
| 001 | LCR[7]=0 | IER | IER |
| 010 | LCR≠0xBF (any LCR[7]) | ISR | FCR |
| 011 | any | LCR | LCR |
| 100 | LCR≠0xBF | MCR | MCR |
| 101 | LCR≠0xBF | LSR | (write: not defined) |
| 110 | LCR≠0xBF | MSR | (not defined) |
| 111 | LCR≠0xBF | SPR | SPR |
| 000 | LCR[7]=1, LCR≠0xBF | DLL | DLL |
| 001 | LCR[7]=1, LCR≠0xBF | DLM | DLM |
| 010 | LCR[7]=1, LCR≠0xBF, EFR[4]=1 | DLD | DLD |
| 000 | LCR[7]=1, LCR≠0xBF, DLL=DLM=0x00 | DREV (read-only) | — |
| 001 | LCR[7]=1, LCR≠0xBF, DLL=DLM=0x00 | DVID (read-only) = 0x02 | — |
| 010 | LCR=0xBF | EFR | EFR |
| 100 | LCR=0xBF | XON1 | XON1 |
| 101 | LCR=0xBF | XON2 | XON2 |
| 110 | LCR=0xBF | XOFF1 | XOFF1 |
| 111 | LCR=0xBF | XOFF2 | XOFF2 |

Notes: Table 8 places ISR/FCR at 010 under "LCR≠0xBF" (so with LCR[7]=1 and EFR[4]=0, address 010 is ISR/FCR; with EFR[4]=1 it is DLD). When DLL=DLM=0 reads of 000/001 return DREV/DVID instead of DLL/DLM (writes still go to DLL/DLM). Address 011 (LCR) is always accessible including LCR=0xBF. Addresses 000/001 with LCR=0xBF: Table 8 conditions for DLL/DLM say "LCR≠0xBF"; what is accessed at 000/001/011 when LCR=0xBF is not listed except LCR — treat 000/001 as undefined when LCR=0xBF.

Bits ("shaded"/enhanced bits writable only while EFR[4]=1 — IER[7:4], ISR[5:4] (enable), FCR[5:4], MCR[7:5], DLD):
- RHR (R): received data. THR (W): data to send; bit0 sent first.
- IER (R/W, reset 0x00): b0 RX data ready (RHR / RX trigger / timeout) int enable; b1 THR empty / TX trigger int enable ("If the THR is empty when this bit is enabled, an interrupt will be generated"); b2 receive line status int enable (LSR[4:1]); b3 modem status int enable; b4 sleep mode enable*; b5 Xoff (received Xoff / special char) int enable*; b6 RTS# int enable* (RTS pin low→high under auto RTS); b7 CTS# int enable* (CTS pin low→high under auto CTS). (*requires EFR[4]=1.)
- ISR (R, reset 0x01): b0 0=interrupt pending, 1=none; b3..b1 source code; b4 Xoff/special char (enabled only when EFR[4]=1); b5 RTS/CTS change (EFR[4]=1); b7,b6 = 11 when FIFOs enabled (FCR[0]=1) else 00.
- FCR (W, reset 0x00): b0 FIFO enable (TX+RX; "must be set to logic 1 when other FCR bits are written or they will not be programmed"); b1 RX FIFO reset (self-clearing; RSR not touched; only when b0=1); b2 TX FIFO reset (self-clearing; TSR not touched; only when b0=1); b3 DMA mode select (TXRDY/RXRDY pin behavior); b5:4 TX trigger select* (EFR[4]=1); b7:6 RX trigger select.
  Trigger table (Table 11, p27): RX FCR[7:6] 00=1, 01=4, 10=8, 11=14. TX FCR[5:4] 00=1, 01=4, 10=8, 11=14. (TX interrupt fires when TX FIFO count falls below the TX trigger level, "or when it gets empty in case that the FIFO did not get filled over the trigger level on last re-load", p27.)
- LCR (R/W, reset 0x00): b1:0 word length 00=5 01=6 10=7 11=8; b2 stop bits: 0=1 stop; 1 = 1.5 stop if 5-bit, 2 stop if 6/7/8-bit; b3 parity enable; b4 even parity (0=odd, 1=even); b5 force parity (with b3=1: b5=1,b4=0 → parity forced 1 (mark); b5=1,b4=1 → forced 0 (space)); b6 TX break (TX forced LOW until cleared); b7 divisor latch enable. LCR=0xBF selects the enhanced bank.
- MCR (R/W, reset 0x00): b0 DTR (1 → DTR pin LOW); b1 RTS (1 → RTS pin LOW; must be 1 to start auto RTS); b2 OP1 / IrDA RX polarity: when IrDA enabled (MCR[6]=1, EFR[4]=1) 0 = RX input active-low encoded IrDA (idle LOW), 1 = active-high (idle HIGH); in loopback drives MSR[6] (RI); b3 OP2/INT enable: 0 = INTx 3-state and OP2x pin HIGH; 1 = INTx active and OP2x pin LOW; b4 internal loopback; b5 Xon-Any* (any received char resumes TX; char goes into RX FIFO unless it is an Xon/Xoff char with sw flow control on); b6 IrDA enable* (TX idles LOW); b7 clock prescaler*: 0 = ÷1, 1 = ÷4.
- LSR (R, reset 0x60): b0 data ready (≥1 char in RHR/FIFO); b1 overrun (char arrived while FIFO full; the RSR char is overwritten, not put in FIFO; interrupt immediately on reception); b2 parity error (of char at top of FIFO); b3 framing error (top char); b4 break (top char; "only one break character is loaded into the FIFO"); b5 THR empty (non-FIFO: THR empty; FIFO: TX FIFO empty); b6 THR & TSR empty; b7 RX FIFO global error (≥1 PE/FE/BI in FIFO; clears when no errored bytes remain). Read of LSR clears the line-status interrupt (b1..b4 are "tags" of the top char; b1 clears on LSR read — standard 16550).
- MSR (R): b0 ΔCTS, b1 ΔDSR, b2 TERI (RI changed LOW→HIGH, i.e. end of ring), b3 ΔCD; b4 CTS = NOT CTS pin (loopback: = MCR[1]); b5 DSR = NOT DSR pin (loopback: MCR[0]); b6 RI = NOT RI pin (loopback: MCR[2]); b7 CD = NOT CD pin (loopback: MCR[3]). Δ bits cleared by MSR read. Reset: b3:0 = 0, b7:4 = inverted input levels.
- SPR (R/W): scratch; reset 0xFF (preserved in sleep).
- DLL/DLM (R/W): 16-bit integer divisor; "DLM = 0x00 and DLL = 0x01. Only resets to these values during a power up. They do not reset when the Reset Pin is asserted." (Table 15 p35).
- DLD (R/W, reset 0x00; access needs EFR[4]=1): b3:0 fractional divisor (n/16); b5:4 sampling: 00=16X, 01=8X, 1x=4X (Table 13 p32); b7:6 = 0 (read as 0 per Table 9).
- DVID (R) = 0x02; DREV (R) revision (0x01 = rev A).
- EFR (R/W at LCR=0xBF, reset 0x00): b3:0 software flow control select (Table 14 below); b4 enhanced function enable (enables writing IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5], DLD; clearing it latches those bits); b5 special character detect enable; b6 auto RTS enable; b7 auto CTS enable.
- XON1, XON2, XOFF1, XOFF2 (R/W at LCR=0xBF, reset 0x00).

Table 14 (p33) EFR[3:0] software flow control:
| b3 b2 b1 b0 | meaning |
|---|---|
| 0 0 0 0 | no TX and RX flow control |
| 0 0 X X | no TX flow control |
| 1 0 X X | TX Xon1, Xoff1 |
| 0 1 X X | TX Xon2, Xoff2 |
| 1 1 X X | TX Xon1+Xon2, Xoff1+Xoff2 (two-char sequences) |
| X X 0 0 | no RX flow control |
| X X 1 0 | RX compares Xon1, Xoff1 |
| X X 0 1 | RX compares Xon2, Xoff2 |
| 1 0 1 1 | TX Xon1/Xoff1; RX compares Xon1 or Xon2, Xoff1 or Xoff2 |
| 0 1 1 1 | TX Xon2/Xoff2; RX compares Xon1 or Xon2, Xoff1 or Xoff2 |
| 1 1 1 1 | TX Xon1+Xon2/Xoff1+Xoff2; RX compares Xon1 and Xon2, Xoff1 and Xoff2 (sequences) |
| 0 0 1 1 | no TX flow control; RX compares Xon1 and Xon2, Xoff1 and Xoff2 |
"Whenever changing the TX or RX flow control bits, always reset all bits back to logic 0 before programming a new setting" (p33).

### Behavior
- **Baud generator** (§2.8-2.9 p9-11, Fig 5): clock = XTAL1 (crystal XTAL1/XTAL2 up to 24 MHz, or external clock on XTAL1 up to 32/50/64 MHz at 1.8/2.5/3.3 V). Shared by both channels; each channel has its own prescaler (MCR[7]: ÷1 or ÷4) and BRG. Divisor = DLM:DLL + DLD[3:0]/16, range 1 to 65535.9375. Sampling clock = prescaled clock / divisor; baud = sampling clock / (16, 8 or 4 per DLD[5:4]). "when using 8X sampling mode ... the bit-time will have a jitter (+/- 1/16) whenever the DLD is non-zero and is an odd number" (p10). Divisor reset = 1 (power-up only). Divisor 0 behavior not stated (and DLL=DLM=0 is used for DVID access). No BAUDOUT/RCLK pins. XTAL2 = oscillator/buffered clock output (stops in sleep).
- **Transmitter** (§2.10 p12): start bit, 5-8 data LSB first, optional parity, 1/1.5/2 stop. THR→TSR; LSR[5] set when THR (or FIFO) empties; LSR[6] when TSR also empty. Bit time = 16/8/4 sampling clocks. Break: LCR[6] forces TX LOW. TX idles HIGH (LOW in IrDA mode).
- **Receiver** (§2.11 p13): on falling edge of start, counter at sampling rate; after 8 clocks (4 at 8X, 2 at 4X) start bit sampled at center, validated if still 0 (else false start ignored). Data/parity/stop sampled mid-bit. Errors PE/FE/BI stored as 3 tag bits per FIFO entry (FIFO is 16 x 11 bits) and appear in LSR[4:2] when the char reaches RHR. Break: RX LOW for ≥ one full character frame; one break char (0x00) loaded into FIFO. Overrun: new char completes while FIFO full → LSR[1], RSR char lost (FIFO intact).
  RX timeout: "when data is not received for 4 word lengths as defined by LCR[1:0] plus 12 bits time. This is equivalent to 3.7-4.6 character times" (p13). Cleared by reading RHR.
- **FIFOs**: 16 bytes each. FCR[0] enables both. Trigger table above. FIFO-mode RX interrupt: issued when count reaches RX trigger; cleared when count drops below trigger (§4.3.1 p23).
  TXRDY/RXRDY pins (Table 2 p8), TQFP48 only:
  | pin | FIFO disabled | FIFO en, DMA mode 0 (FCR[3]=0) | FIFO en, DMA mode 1 (FCR[3]=1) |
  |---|---|---|---|
  | RXRDY | LOW = 1 byte in RHR; HIGH = none | LOW = ≥1 byte in FIFO; HIGH = empty | HIGH→LOW when FIFO reaches RX trigger or timeout; LOW→HIGH when FIFO empties |
  | TXRDY | LOW = THR empty; HIGH = byte in THR | LOW = FIFO empty; HIGH = ≥1 byte | LOW = ≥1 empty location; HIGH = FIFO full |
- **Interrupts** (Table 10 p26, §4.4 p25):
  | prio | ISR[5:0] | source | clear |
  |---|---|---|---|
  | 1 | 000110 | LSR (LSR[4:1]) | read LSR |
  | 2 | 001100 | RX timeout | read RHR |
  | 3 | 000100 | RX data ready / RX trigger | read until below trigger (non-FIFO: read RHR) |
  | 4 | 000010 | TX ready (THR empty / below TX trigger) | read ISR or write THR |
  | 5 | 000000 | MSR (MSR[3:0]) | read MSR |
  | 6 | 010000 | received Xoff or special char | read ISR, or Xon received (Xoff) / next char received (special char) |
  | 7 | 100000 | CTS/RTS change (pin went LOW→HIGH under auto flow control) | read MSR |
  | – | 000001 | none | |
  LSR[1] (overrun) interrupts immediately; LSR[4:2] interrupt "when the character with errors is read out of the FIFO" i.e. reaches the top (p24).
  INTA/INTB pins: active HIGH, 3-stated when MCR[3]=0, driven when MCR[3]=1 (same bit drives OP2 pin LOW). Pin = OR of enabled pending interrupts of that channel. Tables 3/4 (p9) restate: TX: non-FIFO INT HIGH = THR empty; FIFO: HIGH = FIFO below trigger or empty. RX: non-FIFO HIGH = 1 byte; FIFO HIGH = above trigger.
- **Auto RTS** (§2.12-2.13 p15, Table 6): enable EFR[6]; start by MCR[1]=1. Hysteresis: RTS pin goes HIGH when RX FIFO reaches the next trigger level above the selected one; returns LOW when FIFO drops to the next level below:
  | RX trigger | INT at | RTS HIGH at | RTS LOW at |
  |---|---|---|---|
  | 1 | 1 | 4 | 0 |
  | 4 | 4 | 8 | 1 |
  | 8 | 8 | 14 | 4 |
  | 14 | 14 | 14 | 8 |
  Optional interrupt IER[6] on RTS LOW→HIGH (ISR[5]). Not supported in loopback.
- **Auto CTS** (§2.14 p15): EFR[7]; CTS pin HIGH stops TX after the current character's stop bit; LOW resumes. Optional interrupt IER[7] on CTS LOW→HIGH (ISR[5]).
- **Auto Xon/Xoff** (§2.15 p17, Table 7): RX compares 1 or 2 sequential chars to Xon/Xoff per EFR[3:0]; on Xoff match TX halts after current char, ISR[4] set (if IER[5]); on Xon match resumes. Flow chars are not stored in RX FIFO. TX side: Xoff sent "two-character times ... after the receive FIFO crosses the selected trigger level"; Xon sent when RX FIFO falls to one trigger level below:
  | RX trigger | INT | Xoff sent at | Xon sent at |
  |---|---|---|---|
  | 1 | 1 | 1* | 0 |
  | 4 | 4 | 4* | 1 |
  | 8 | 8 | 8* | 4 |
  | 14 | 14 | 14* | 8 |
  (* plus 2 character times delay). Xon/Xoff registers reset 0x00. Comparison uses only the LCR word-length bits (bit0 = LSB) (§2.16 p17).
- **Special character** (§2.16 p17, EFR[5] p34): each RX char compared with XOFF2; on match char goes into FIFO and ISR[4] set. If EFR[1:0]=10 both work normally; if EFR[1:0]=01, Xoff2 is not put in FIFO and generates Xoff + special char interrupt.
- **IrDA** (§2.17 p18, Fig 11): MCR[6]=1 (EFR[4]=1). TX: each 0 bit → HIGH pulse 3/16 bit wide (starts at... figure shows pulse centered/at bit start—"3/16 Bit Time" within each 0 bit; "1/2 Bit Time" label shown); 1 bits → LOW; idles LOW. RX: each pulse → 0 bit (with "1/16 clock delay"); polarity inversion via MCR[2]. Exact pulse position within bit: per Fig 11 the pulse is in the middle of the bit (label "1/2 Bit Time"); IrDA 1.0 standard places it centered.
- **Sleep** (§2.18 p19): entered when both channels: no pending interrupt (ISR[0]=1), IER[4]=1, MSR[3:0]=0, RX idle HIGH. Oscillator stops (XTAL2 no clock). Wake on RX start bit (HIGH→LOW), write to THR/FIFO, or change on CTS/DSR/CD/RI. Returns to sleep automatically when interrupts serviced. "first few receive characters may be lost" (crystal startup). SPR preserved.
- **Loopback** (§2.19 p20, Fig 12): MCR[4]=1: TSR output → RSR input internally; TX pin held HIGH (Fig 12 shows TX tied to VCC), RTS, DTR, OP2 pins held HIGH (inactive); RX, CTS, DSR, CD, RI pins ignored; internally CTS←RTS(MCR[1]), DSR←DTR(MCR[0]), RI←OP1(MCR[2]), CD←OP2(MCR[3]). Auto RTS/CTS not supported in loopback. With MCR[4]=1, whether INT pin is still gated by MCR[3] is not stated.
- **Reset** (§2.2 p7, Table 15 p35): RESET active HIGH, >40 ns. Resets: DLD=0, IER=0, FCR=0, ISR=0x01, LCR=0, MCR=0, LSR=0x60, MSR[3:0]=0, SPR=0xFF, EFR=0, XON1/2=XOFF1/2=0; DLL/DLM NOT reset by pin (power-up only: DLL=1, DLM=0). Outputs: TX HIGH, OP2 HIGH, RTS HIGH, DTR HIGH, RXRDY HIGH, TXRDY LOW, INT 3-state. During reset TX held HIGH, RX ignored.
- Undefined / not described: writes to LSR/MSR; reading both channels at once; divisor 0.

### Timing (AC table p37; VCC=1.62–3.63 V, 70 pF; columns 1.8V±10% / 2.5V±10% / 3.3V±10%)
| sym | parameter | 1.8V | 2.5V | 3.3V |
|---|---|---|---|---|
| XTAL1 | crystal freq max | 24 MHz | 24 | 24 |
| ECLK | external clock max | 32 MHz | 50 | 64 |
| TECLK | ext clock period min | 15 ns | 10 | 7 |
| TAS | address setup min | 0 | 0 | 0 |
| TAH | address hold min | 0 | 0 | 0 |
| TCS | chip select width min | 65 | 40 | 35 |
| TRD | IOR width min | 65 | 40 | 35 |
| TDY | read cycle delay min | 65 | 40 | 35 |
| TRDV | data access time max | 60 | 35 | 30 |
| TDD | data disable time max | 25 | 25 | 25 |
| TWR | IOW width min | 65 | 40 | 35 |
| TDY | write cycle delay min | 65 | 40 | 35 |
| TDS | data setup min | 20 | 10 | 10 |
| TDH | data hold min | 3 | 3 | 3 |
| TWDO | IOW to output (RTS/DTR) max | 50 | 50 | 50 |
| TMOD | modem input to INT max | 50 | 50 | 50 |
| TRSI | IOR to reset (MSR) INT max | 50 | 50 | 50 |
| TSSI | stop to set INT max | 1 Bclk | 1 | 1 |
| TRRI | IOR to reset INT max | 45 | 45 | 45 |
| TSI | stop to INT max | 45 ns | 45 | 45 |
| TINT | initial INT reset to TX start | 8 min / 24 max Bclk | same | same |
| TWRI | IOW to reset INT max | 45 | 45 | 45 |
| TSSR | stop to set RXRDY max | 1 Bclk | 1 | 1 |
| TRR | IOR to reset RXRDY max | 45 | 45 | 45 |
| TWT | IOW to set TXRDY max | 45 | 45 | 45 |
| TSRT | center of start to reset TXRDY max | 8 Bclk | 8 | 8 |
| TRST | reset pulse width min | 40 | 40 | 40 |
Bclk = 16X/8X/4X sampling clock. Columns: min vs max as listed in the table (setup/hold/widths are MIN; access/delays MAX). Figures: read p39 Fig 15, write p39 Fig 16, modem p38 Fig 14, RX/TX/INT/RXRDY/TXRDY p40-42 Figs 17-22. Max data rates: 16 Mbps@3.3 V, 12.5@2.5, 8@1.8 (4X sampling).

### VCC
Recommended 1.62–3.63 V (p1, DC table p36). Pin table p5 says "VCC ... 2.25V to 3.6V" — datasheet error (copied from XR16L2550); use 1.62–3.63. Abs max: supply 4 V; "Voltage at Any Pin GND-0.3V to 4V" (p36) → inputs are NOT 5 V tolerant; VIH max = VCC (DC table). inputMax ≈ 4 V abs max; treat as not 5 V tolerant.

### Package drawings
- 48-TQFP 7x7x1 mm (p43, no drawing code, no land pattern): pitch e=0.50 BSC; lead width B 0.17–0.27 (mid 0.22); L (foot) 0.45–0.75 (mid 0.60); body D1 6.90–7.10 (7.0); tip-to-tip D 8.80–9.20 (9.0); A 1.00–1.20, A1 0.05–0.15, A2 0.95–1.05, C 0.09–0.20; 12 pins/side, pin 1 top-left of left side (counter-clockwise).
- 32-QFN 5x5x0.9 mm (p44, no code, no land pattern): D (=E) 4.90–5.10 (5.0); pitch 0.50 BSC; b 0.18–0.30 (mid 0.24); L 0.35–0.45 (0.40); exposed pad D2 (=E2) 3.50–3.80 (mid 3.65; "device-dependent with a typical tolerance of 0.3mm"); A 0.80–1.00, A1 0–0.05, A3 0.15–0.25, k ≥ 0.20. 8 pins/side.

### Obsolete
None in census (both Active). PDF itself has no obsolete watermark.

### Notes
- VCC pin-table error (2.25–3.6 V) noted above.
- Table 3 (p9) has two INTA/B rows for transmitter (row 2: "HIGH = transmitter empty"/"FIFO below trigger level or transmitter empty"), with no row labels distinguishing them — ambiguous leftover; use LSR[5]-based THR-empty semantics per §4.3 IER[1].
- Block diagram (p1) typo "DTSA#" = CTSA#.
- FCR[5:4]/FCR[7:6] text says "logic 0 = default, TX/RX trigger level = 1".

## XR16M2650

- Title: "HIGH PERFORMANCE LOW VOLTAGE DUART WITH 32-BYTE FIFO" (Exar, May 2007, rev 1.0.2). Every page of this PDF carries the red watermark "The product (or products) mentioned in this data sheet are no longer being manufactured and may not be ordered (OBS)" (seen on p1, p27, p37 images) — yet census says Active (Sipex).
- Datasheet: `datasheets/controller-uart/xr16m2650.pdf` (47 pp; same layout/page numbers as XR16M2550: pinouts p2, pins p3-5, registers p21-35, AC p37, packages p43-44).
- **Same as XR16M2550 in every respect (pins, both pinouts, bus, register map and bit definitions, BRG, sleep, IrDA, reset table, DC, packages) except:**
  1. FIFOs are 32 bytes (TX and RX; RX FIFO 32 x 11 bits).
  2. DVID = 0x06 (Table 9 p22: 0000 0110; §2.3 p7, §4.12 p32).
  3. FIFO trigger table (Table 11, p27, read from page image): RX FCR[7:6] 00=8 (default), 01=16, 10=24, 11=28. TX FCR[5:4] 00=16 (default), 01=8, 10=24, 11=30. (Text above table still says "logic 0 = default, TX trigger level = 1 / RX trigger level = 1" — copy error; table wins.)
  4. Auto RTS hysteresis (Table 6 p15): RX trigger 8 → INT 8, RTS HIGH at 16, LOW at 0; 16 → 16, 24, 8; 24 → 24, 28, 16; 28 → 28, 28, 24.
  5. Auto Xon/Xoff (Table 7 p17): trigger 8 → Xoff at 8*, Xon at 0; 16 → 16*, 8; 24 → 24*, 16; 28 → 28*, 24 (* + 2 char times).
  6. Described as "2 enhanced 16C650A" UARTs (p6) but register set is the same as M2550 (no FCTR/EMSR etc.).
  7. VCC pin row (p5) correctly says 1.62–3.63 V.
- Parts:
  | census part | package | pins | pinout | grade | VCC |
  |---|---|---|---|---|---|
  | XR16M2650IL32 | 32-pin QFN 5x5 (EP) | 32 | `qfn32` (identical to XR16M2550 qfn32) | -40..+85 °C | 1.62–3.63 V |
  | XR16M2650IM48 | 48-lead TQFP 7x7 | 48 | `tqfp48` (identical to XR16M2550 tqfp48) | -40..+85 °C | 1.62–3.63 V |
- Timing: AC table p37 (page image checked) is visibly corrupted: rows from TWDO down carry values that look shifted from other rows (TWDO/TMOD/TRSI = 0 min; TSSI 60/35/30 "Bclk" max; TRRI 25 max; TSI 20/10/10 min; TINT 5 min Bclk; TWRI 5 min ns; TSSR 65/40/35 min Bclk; TRR 65/40/35 min ns; TWT 45 max; TSRT 8 Bclk max; TRST 40 min). Rows XTAL1…TDH are identical to XR16M2550. Recommend using the XR16M2550/XR16M2651 values (same die family, same rev date) for the interrupt/RXRDY rows; flag as datasheet error.
- Package drawings: identical dims to XR16M2550: TQFP p43, QFN p44.
- Obsolete: census rows are Active; PDF watermark says OBS (manufacturer-discontinued at the time of the PDF). Flag for planner (AGENTS.md: add census-in-stock parts; consider `"flags": ["obsolete"]`).

## XR16M2551

- Title: "HIGH PERFORMANCE LOW VOLTAGE DUART WITH 16-BYTE FIFO AND POWERSAVE FEATURE" (Exar, May 2007, rev 1.0.2). Watermarked "...no longer being manufactured and may not be ordered (OBS)" on every page.
- Datasheet: `datasheets/controller-uart/xr16m2551.pdf` (51 pp; pinouts p2, pins p3-6, functional p7-22, registers p23-37, DC p38, AC p39-40, timing figs p40-46, packages QFN p47, TQFP p48).
- Differs from XR16M2550: same register set and DVID (0x02!), same 16-byte FIFOs; adds pins 16/68# (Intel/Motorola bus select), PWRSAVE, CLKSEL (TQFP only), and a sleep wake-up indicator interrupt.

### Parts
| census part | package | pins | pinout | grade | VCC |
|---|---|---|---|---|---|
| XR16M2551IL32 | 32-pin QFN 5x5x0.9 (EP) | 32 | `qfn32-2551` | -40..+85 °C | 1.62–3.63 V |
| XR16M2551IM48 | 48-lead TQFP 7x7x1 | 48 | `tqfp48-2551` | -40..+85 °C | 1.62–3.63 V |
(Ordering p2.)

### Pinouts (p2 image, p3-6 table). Mode-dependent names: Intel (16/68#=HIGH) / Motorola (16/68#=LOW).
`tqfp48-2551`:
```
1 D5 inout     13 XTAL1 input        25 CLKSEL input            37 NC nc
2 D6 inout     14 XTAL2 output       26 A2 input                38 CTSA input
3 D7 inout     15 IOW / R/W input    27 A1 input                39 DSRA input
4 RXB input    16 CDB input          28 A0 input                40 CDA input
5 RXA input    17 GND ground         29 INTB / NC(68) output    41 RIA input
6 TXRDYB out   18 RXRDYB output      30 INTA / IRQ output       42 VCC power
7 TXA output   19 IOR / (tie VCC) in 31 RXRDYA output           43 TXRDYA output
8 TXB output   20 DSRB input         32 OP2A output             44 D0 inout
9 OP2B output  21 RIB input          33 RTSA output             45 D1 inout
10 CSA / CS in 22 RTSB output        34 DTRA output             46 D2 inout
11 CSB / A3 in 23 CTSB input         35 DTRB output             47 D3 inout
12 PWRSAVE in  24 16/68 input        36 RESET / RESET# input    48 D4 inout
```
`qfn32-2551`:
```
1 D6 inout     9 PWRSAVE input       17 16/68 input             25 CTSA input
2 D7 inout     10 XTAL1 input        18 A2 input                26 VCC power
3 RXB input    11 XTAL2 output       19 A1 input                27 D0 inout
4 RXA input    12 IOW / R/W input    20 A0 input                28 D1 inout
5 TXA output   13 GND ground         21 INTB / NC(68) output    29 D2 inout
6 TXB output   14 IOR / (tie VCC) in 22 INTA / IRQ output       30 D3 inout
7 CSA / CS in  15 RTSB output        23 RTSA output             31 D4 inout
8 CSB / A3 in  16 CTSB input         24 RESET / RESET# input    32 D5 inout
EP = GND
```
(QFN32 has no CLKSEL, no TXRDY/RXRDY/DTR/DSR/CD/RI/OP2.) Pin name suggestion for 16/68#: repo needs a legal name, e.g. `SEL16` or `M16_68`; HIGH = Intel.
Mode-selected functions (p3-6):
- IOR (Intel, active-low read strobe) / Motorola: unused, "should be connected to VCC".
- IOW (Intel, active-low write strobe) / R/W (Motorola: HIGH = read, LOW = write).
- CSA (Intel, ch A select, low) / CS (Motorola, active-low chip select).
- CSB (Intel, ch B select, low) / A3 (Motorola: 0 = channel A, 1 = channel B).
- INTA (Intel: ch A interrupt, active HIGH, 3-state per MCR[3]) / IRQ (Motorola: device interrupt for both channels, active LOW, open drain, needs external pull-up).
- INTB (Intel: ch B interrupt) / Motorola: "NC (INTB)" in pinout; block diagram p1 labels it "INTB (logic 0)" — i.e. driven LOW / unused in 68 mode (not further specified).
- RESET (Intel: active HIGH) / RESET# (Motorola: active LOW). 40 ns minimum pulse.
- PWRSAVE: active HIGH input (see PowerSave).
- CLKSEL (TQFP only): BRG prescaler default for both channels, "only sampled during power up or a reset. Connect to VCC for divide by 1 and GND for divide by 4. MCR[7] can override the state of this pin following a reset" (p6). Implication: after reset MCR[7] reflects CLKSEL? — Table 16 still lists MCR reset = 0x00; not reconciled in datasheet. QFN32 (no CLKSEL) default not stated — assume ÷1.

### Host interface
- Intel mode (16/68#=HIGH): exactly as XR16M2550 (Table 1 p9, same strobes/edges).
- Motorola mode (16/68#=LOW) (Table 2 p9, Fig 17/18 p42-43): CS active LOW is the strobe; R/W level selects direction; A3 selects channel (A3=0 → A, 1 → B; CS=1 → deselected). No E/clock input. Read: R/W HIGH set up TRWS before CS falls; data valid TRDA after CS falls, floats TRDH after CS rises. Write: R/W LOW; data must be valid TWDS before CS rising edge and held TWDH after (data latched on CS rising edge); R/W held TRWH after CS rises. Address A0-A3 setup TADS before CS falling, hold TADH after CS rising. Both channels share one IRQ (open drain, active low).

### Registers
Identical to XR16M2550 (Table 9/10 p23-25), including DVID = 0x02 (p9, p25, p35). Trigger table (Table 12 p29) identical: RX 1/4/8/14, TX 1/4/8/14 — but text adds "Whichever selection is made last applies to both the RX and TX side" (p29), apparently copied from a different part; contradicts separate FCR[7:6]/FCR[5:4] fields — flag. Auto RTS / Xon-Xoff tables (Tables 7/8) identical to XR16M2550.
ISR addition (Table 11 p28): code 000001 = "None (default) or Wake-up Indicator"; ISR[0]=1 also means "the device has come out of sleep mode".

### Behavior differences vs XR16M2550
- **Sleep wake-up indicator** (§2.18.1 p21): if sleep enabled and device wakes by RX start bit, THR write, or modem input change, "an interrupt is issued ... to signal to the CPU that it is awake. The lower nibble of the ISR will read a value of 0x1 for this interrupt and reading the ISR clears this interrupt." (i.e. INT pin asserted but ISR reads like no interrupt).
- **PowerSave** (§2.18.2 p21): when PWRSAVE=HIGH and the device is in sleep, address, data and control inputs (A2-A0, D7-D0, IOR, IOW, CSA, CSB) are internally isolated (bus ignored, data bus not driven). Wake-up then only by RX start bit or modem input change (CTS/DSR/CD/RI) — NOT by host writes. Returns to PowerSave after MSR read and all interrupts serviced. Exit: IER[4]=0 (only reachable when awake) and/or PWRSAVE=LOW.
- **Reset**: Table 16 p37 identical to M2550's except INT row: "Three-State Condition (16 mode)", IRQ# "HIGH (68 mode)" (open-drain released). §2.2 p8 still says "active high pulse" (Intel only).
- Data rates text (p7): up to 4 Mbps @16X, 8 Mbps @8X, 16 Mbps @4X at 3.3 V.

### Timing (AC p39-40, three columns 1.8/2.5/3.3 V ±10%)
Intel rows identical to XR16M2550 (TAS 0, TAH 0, TCS 65/40/35, TRD 65/40/35, TDY 65/40/35, TRDV 60/35/30 max, TDD 25 max, TWR 65/40/35, TDS 20/10/10, TDH 3; ECLK 32/50/64 MHz, TECLK 15/10/7, XTAL 24 MHz). Motorola rows (p39):
| sym | parameter | 1.8V | 2.5V | 3.3V |
|---|---|---|---|---|
| TADS | address setup min | 0 | 0 | 0 |
| TADH | address hold min | 0 | 0 | 0 |
| TRWS | R/W setup to CS min | 0 | 0 | 0 |
| TRDA | read data access max | 60 | 35 | 30 |
| TRDH | read data disable max | 25 | 25 | 25 |
| TWDS | write data setup min | 20 | 10 | 10 |
| TWDH | write data hold min | 3 | 3 | 3 |
| TRWH | CS deasserted to R/W deasserted min | 5 | 5 | 5 |
| TCSL | CS width min | 65 | 40 | 35 |
| TCSD | CS cycle delay min | 65 | 40 | 35 |
Remaining rows (p39-40) identical to XR16M2550: TWDO 50, TMOD 50, TRSI 50, TSSI 1 Bclk, TRRI 45, TSI 45, TINT 8–24 Bclk, TWRI 45, TSSR 1 Bclk, TRR 45, TWT 45, TSRT 8 Bclk, TRST 40.

### VCC
1.62–3.63 V (p6, p38). Abs max any pin -0.3..4 V (p38) — not 5 V tolerant. ISLEEP 5/8/15 µA.

### Package drawings
QFN32 p47: same as XR16M2550 except L printed 0.35–0.45 mm with inch 0.014–0.018 (mm identical). TQFP48 p48: identical to XR16M2550.

### Obsolete
Census rows Active; PDF watermarked OBS — flag.

## XR16M2651

- Title: "HIGH PERFORMANCE DUART WITH 32-BYTE FIFO AND POWERSAVE FEATURE" (Exar, May 2007, rev 1.0.2). Watermarked OBS on every page (same text as XR16M2551).
- Datasheet: `datasheets/controller-uart/xr16m2651.pdf` (51 pp; same page layout as XR16M2551: pins p2-6, registers p23-37, AC p39-40, QFN p47, TQFP p48).
- = XR16M2551 (pins, both pinouts `qfn32-2551`/`tqfp48-2551` — pin tables verified identical, Intel/Motorola bus, PowerSave, CLKSEL, wake-up indicator, reset table, DC, AC tables — AC p39-40 identical to XR16M2551, all Intel+Motorola rows) **with the XR16M2650 FIFO differences**:
  1. 32-byte TX and RX FIFOs.
  2. DVID = 0x06 (p9, Table 10 p25 "0000 0110", §4.12 p34).
  3. Trigger table (Table 12 p29): RX FCR[7:6] 00=8, 01=16, 10=24, 11=28; TX FCR[5:4] 00=16, 01=8, 10=24, 11=30. Same stray "Whichever selection is made last applies to both the RX and TX side" sentence (p29).
  4. Auto RTS (Table 7 p17): trigger 8/16/24/28 → RTS HIGH at 16/24/28/28, LOW at 0/8/16/24. Xon/Xoff (Table 8 p19): Xoff at 8/16/24/28 (+2 char times), Xon at 0/8/16/24.
- Parts:
  | census part | package | pins | pinout | grade | VCC |
  |---|---|---|---|---|---|
  | XR16M2651IL32 | 32-pin QFN 5x5 (EP) | 32 | `qfn32-2551` | -40..+85 °C | 1.62–3.63 V |
  | XR16M2651IM48 | 48-lead TQFP 7x7 | 48 | `tqfp48-2551` | -40..+85 °C | 1.62–3.63 V |
- Package drawings: QFN32 p47 (L 0.35–0.45 mm / 0.012–0.020 in), TQFP48 p48; dims identical to XR16M2550.
- Obsolete: census rows Active; PDF watermark OBS — flag.
## XR16M752

(One device design covering **XR16M752** (Intel bus only) and **XR68M752** (adds the 16/68#
bus-select pin). Same register set, same 64-byte FIFOs, same datasheet, same die function;
the only difference is that the XR68M752 brings out 16/68# on TQFP pin 24 (QFN pin 17), where
the XR16M752 has NC. 16/68# has an internal pull-up (p.7), so an XR68M752 with pin 24 open or
HIGH behaves exactly like an XR16M752. Rule applied: same registers + same FIFO = one device;
the bus mode is pin-selected. Two TQFP pinouts are given below because pin 24 differs.)

- Title (as printed): "XR16M752/XR68M752 HIGH PERFORMANCE DUART WITH 64-BYTE FIFO", Rev. 1.1.1,
  June 2009, Exar. Datasheet: `datasheets/controller-uart/xr16m752.pdf` (54 PDF pages;
  printed page N = PDF page N for pp.1–52; pp.53–54 are the table of contents).
- Function: dual 16C550-compatible UART, 64-byte TX and RX FIFO per channel, fractional baud
  generator (DLD), 16X/8X/4X sampling, auto RTS/CTS, auto Xon/Xoff, special char, IrDA 1.0,
  auto RS-485 direction via RTS#, sleep with auto wake-up. Pin/software compatible with TI
  TL16C752B and NXP SC16C752B in 48-TQFP (p.1). Register model = TL16C752B style (TCR/TLR/
  FIFO Rdy, EFR, Xon/Xoff 1/2) plus Exar DLD.
- **FIFO depth: 64 bytes TX and 64 bytes RX per channel** (p.1, p.9, p.15, p.17). RX FIFO is
  64 × 11 bits (8 data + 3 error tags) (p.17). The census description "FIFO 34B" for
  XR16M752IM48-F / XR16M752IM48TR-F is wrong; the "64B" on XR68M752IM48TR-F is right.
- **No** FCTR, TXCNT/RXCNT, TXTRG/RXTRG, EMSR, FC, SFR, DREV/DVID, software-reset or 8XMODE/
  4XMODE registers exist on this part (those are on XR16V/M2750/2752/2754-class Exar parts).
  The datasheet's MCR[1] text mentions "FCTR bit-3" (p.32) — a copy error; the RS-485 enable
  is DLD[6] (p.37).

### Parts

| Census part | Base | Package | Pins | Pinout | Temp | VCC | Bus |
|---|---|---|---|---|---|---|---|
| XR16M752IM48-F | XR16M752 | 48-Lead TQFP 7x7x1 mm ("IM48") | 48 | tqfp48 (XR16M752) | -40..+85 °C (I) | 1.62–3.63 V | Intel only |
| XR16M752IM48TR-F | XR16M752 | same, tape & reel | 48 | tqfp48 (XR16M752) | -40..+85 °C | 1.62–3.63 V | Intel only |
| XR68M752IM48TR-F | XR68M752 | same, tape & reel | 48 | tqfp48-68 (XR68M752) | -40..+85 °C | 1.62–3.63 V | Intel or Motorola (pin 24) |

Ordering table (p.3): XR16M752IL32 (32 QFN), XR16M752IM48 (48 TQFP), XR68M752IL32,
XR68M752IM48, XR68M752IB49 (49 STBGA); all -40 to +85 °C, Active. Decode: I = industrial
-40..+85, M48 = 48 TQFP, L32 = 32 QFN, B49 = 49 STBGA. "-F" (lead-free/RoHS) and "TR" (tape &
reel) are not in the 2009 ordering table; they are packing/RoHS suffixes only (standard
MaxLinear convention), no electrical difference. Only one speed/voltage grade exists (one
part runs 1.62–3.63 V; AC limits are tabulated at 1.8/2.5/3.3 V ±10 %). All three census
parts decoded. None Obsolete.

### Pinouts (p.2 Figure 2, pin table pp.4–8)

**tqfp48 — XR16M752 48-TQFP ("Intel Mode Only")**

| Pin | Name | Role |
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
| 29 | INTB | output (3-state) |
| 30 | INTA | output (3-state) |
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

No exposed pad on the TQFP. Active-low pins (datasheet "#"): CSA, CSB, IOR, IOW, TXRDYA/B,
RXRDYA/B, RTSA/B, CTSA/B, DTRA/B, DSRA/B, CDA/B, RIA/B, OP2A/B. RESET active HIGH. INTA/INTB
active HIGH, 3-stated when MCR[3]=0.

**tqfp48-68 — XR68M752 48-TQFP** (identical to the above except pin 24, plus mode-dependent
meanings; p.2 shows both "Intel Mode" and "Motorola Mode" drawings)

| Pin | 16/68# = 1 (Intel) | 16/68# = 0 (Motorola) | Role |
|---|---|---|---|
| 10 | CSA (channel A select, active low) | CS (chip select, active low) | input |
| 11 | CSB (channel B select, active low) | A3 (channel select: 0 = A, 1 = B) | input |
| 15 | IOW (write strobe, active low) | R/W (HIGH = read, LOW = write) | input |
| 19 | IOR (read strobe, active low) | not used (drawn "NC") | input |
| 24 | 16/68# tied to VCC | 16/68# tied to GND | input (internal pull-up) |
| 29 | INTB (ch B interrupt, active high, 3-state) | not used (drawn "NC") | output |
| 30 | INTA (ch A interrupt, active high, 3-state) | IRQ (device interrupt, active low, open drain, needs external pull-up) | output |
| 36 | RESET (active high) | RESET (active low, "RESET#") | input |

All other pins as in tqfp48. Suggested names for the 68 pinout: `CSA` (10), `CSB` (11), `IOW`
(15), `IOR` (19), `MODE16` or `S16_68` (24), `INTB` (29), `INTA` (30), `RESET` (36), with the
Motorola meanings in Behavior. The pin table says 16/68# is "not available for the XR16M752"
(p.7); XR16M752 pin 24 is NC (p.2, p.8 NC list "12, 25, 37" is for the XR68M752 view; the
XR16M752 drawing additionally shows 24 = NC).

(Other packages not in census: 32-QFN 5x5 with exposed GND pad — no DTR/DSR/CD/RI/OP2/TXRDY/
RXRDY pins; 49-STBGA with extra PwrSave (F1), EN485# (A6), ENIR# (F7) pins, p.3, p.7–8.)

### Host interface (pp.4–5, 10–11, 43–45)

- 8-bit data bus D7..D0, 3-state; register address A2..A0. No clock needed for bus cycles;
  cycles are asynchronous (p.10).
- **Intel (16) mode** (XR16M752 always; XR68M752 with 16/68# HIGH):
  - Channel select by CSA#/CSB# (Table 1, p.11): 11 = deselected; 01 = channel A (CSA#=0);
    10 = channel B; 00 = both selected (useful for simultaneous writes; "do not attempt to read
    from both UARTs simultaneously" — reading with both is undefined).
  - Read: IOR# falling starts the internal read, data driven while CS# and IOR# low; host
    latches on IOR# rising. Data valid TRDV after IOR# falling; bus floats TDD after IOR# rising
    (Fig 16, p.43).
  - Write: IOW# falling starts the cycle; data is transferred to the register on the **rising
    edge of IOW#** (p.4). TDS/TDH are referenced to IOW# rising (Fig 17, p.44).
  - Address setup TAS before the strobe/CS leading edge, hold TAH after the strobe trailing edge.
  - INTA, INTB: per channel, active HIGH, enabled (driven) only when that channel's MCR[3]=1,
    otherwise high-impedance; OP2x# = LOW when MCR[3]=1, HIGH when 0 (p.5, p.33).
- **Motorola (68) mode** (XR68M752, 16/68# LOW):
  - CS# (pin 10) selects the device; A3 (pin 11) selects channel: 0 = A, 1 = B (Table 2, p.11).
  - R/W# (pin 15): HIGH = read, LOW = write. IOR# unused.
  - Read: data valid TRDA after CS# falling (with R/W# HIGH, set up TRWS before CS# falling);
    floats TRDH after CS# rising (Fig 18, p.44).
  - Write: data latched at CS# rising; TWDS setup / TWDH hold referenced to CS# rising; R/W#
    held TRWH after CS# rising (Fig 19, p.45). Address setup TADS before CS# falling, hold TADH
    after CS# rising.
  - IRQ# (pin 30): single device interrupt, active LOW, open-drain, external pull-up required
    (p.5). The datasheet does not say how the two channels combine onto IRQ# (presumably wired-OR
    of channel A and B interrupts) nor whether MCR[3] gates it — not stated.
  - RESET# active LOW.
- No DDIS, ALE, or DMA request pins; TXRDYA#/B#, RXRDYA#/B# status outputs (Table 3).

### Registers (per channel; Table 7 p.24, Table 8 pp.25–26, §4 pp.26–39)

Access banks:
- LCR[7]=0: 0 RHR(R)/THR(W), 1 IER, 2 ISR(R)/FCR(W), 3 LCR, 4 MCR, 5 LSR, 6 MSR or TCR,
  7 SPR or TLR or FIFO Rdy.
- LCR[7]=1 and LCR ≠ 0xBF: 0 DLL, 1 DLM, 2 DLD (DLD only when EFR[4]=1; Table 7 lists
  "LCR[7]=1, LCR≠0xBF, EFR[4]=1"). 3 = LCR. 4–7: Table 7 shows MCR/LSR/MSR/SPR as "LCR ≠ 0xBF"
  (i.e. still MCR, LSR, MSR/TCR, SPR/TLR/FIFO Rdy when LCR[7]=1 and LCR≠0xBF).
- LCR = 0xBF: 2 EFR, 4 Xon1, 5 Xon2, 6 Xoff1, 7 Xoff2; 3 = LCR. Addresses 0, 1 with LCR=0xBF:
  not specified in Table 7 (DLL/DLM are "LCR≠0xBF"); treat as undefined.
  (Text p.33: "LCR = 0xBF is required to access EFR.")
- Offset 6 (Table 13, p.33): EFR[4]=0 → MSR; EFR[4]=1 & MCR[6]=0 → MSR; EFR[4]=1 & MCR[6]=1 → TCR.
- Offset 7 (Table 12, p.33), LCR≠0xBF: if MCR[4]=0 and MCR[2]=1 ("MCR[4,2]='01'") → FIFO Rdy
  (read-only) regardless of EFR[4]/MCR[6]; else EFR[4]=0 → SPR; EFR[4]=1 & MCR[6]=0 → SPR;
  EFR[4]=1 & MCR[6]=1 → TLR.
- "Shaded bits" (enhanced, writable only while EFR[4]=1): IER[7:4], ISR[5:4] (enabled), FCR[5:4],
  MCR[7:5], and the whole TCR, TLR and DLD (p.25, p.38).

| Addr | Name | R/W | Reset (Table 16 p.39) | Bits |
|---|---|---|---|---|
| 0 | RHR | R | XX | received byte (top of RX FIFO) |
| 0 | THR | W | XX | byte to transmit; bit 0 sent first |
| 0 (DLAB) | DLL | R/W | 0x01 **power-up only**, not reset by RESET pin | divisor low byte |
| 1 (DLAB) | DLM | R/W | 0x00 power-up only | divisor high byte |
| 2 (DLAB, EFR[4]=1) | DLD | R/W | 0x00 | [3:0] fraction /16; [5:4] sampling: 00=16X, 01=8X, 1X=4X (Table 14); [6] auto RS-485 direction control enable; [7] IrDA encoder/decoder enable |
| 1 | IER | R/W | 0x00 | [0] RX data ready (and timeout) int; [1] THR/TX ready int; [2] RX line status int; [3] modem status int; [4]* sleep mode enable; [5]* Xoff/special-char int; [6]* RTS# int (RTS# LOW→HIGH); [7]* CTS# int (CTS# LOW→HIGH). (*EFR[4]=1 to write) |
| 2 | ISR | R | 0x01 | [0] 0 = interrupt pending; [3:1] source; [4] Xoff/special char (EFR[4]=1); [5] CTS#/RTS# (EFR[4]=1); [7:6] = 11 when FIFOs enabled, 00 when disabled |
| 2 | FCR | W | 0x00 | [0] FIFO enable (must be 1 when writing other FCR bits or they are ignored); [1] RX FIFO reset (self-clearing; RSR untouched); [2] TX FIFO reset (self-clearing; TSR untouched); [3] DMA mode (TXRDY#/RXRDY# behavior); [5:4]* TX trigger; [7:6] RX trigger (Table 10). [1],[2] only act when [0]=1 |
| 3 | LCR | R/W | **0x1D** | [1:0] word length 00=5, 01=6, 10=7, 11=8; [2] stop: 0=1, 1=1.5 (5-bit) / 2 (6–8 bit); [3] parity enable; [4] even parity (0=odd); [5] forced parity (with [4]=0 → parity forced 1, [4]=1 → forced 0); [6] TX break (TX forced LOW); [7] divisor latch enable |
| 4 | MCR | R/W | 0x00 | [0] DTR#: 1 → pin LOW; [1] RTS#: 1 → pin LOW; [2] OP1# (internal only; drives RI in loopback) / FIFO Rdy select at offset 7; [3] OP2#/INT enable: 1 → INTx driven and OP2x# LOW, 0 → INTx 3-state and OP2x# HIGH; [4] internal loopback; [5]* Xon-Any; [6]* TCR/TLR enable; [7]* clock prescaler 0 = ÷1, 1 = ÷4 |
| 5 | LSR | R | 0x60 | [0] data ready; [1] overrun; [2] parity error (for byte at RHR); [3] framing error; [4] break; [5] THR empty (FIFO mode: TX FIFO empty); [6] THR & TSR empty; [7] RX FIFO global error (any PE/FE/BI in FIFO) |
| 6 | MSR | R | [3:0]=0, [7:4]=inverted input levels | [0] ΔCTS; [1] ΔDSR; [2] TERI (RI# LOW→HIGH); [3] ΔCD; [4] CTS (=¬CTS#; loopback = MCR[1]); [5] DSR (=¬DSR#; loopback = MCR[0]); [6] RI (=¬RI#; loopback = MCR[2]); [7] CD (=¬CD#; loopback = MCR[3]) |
| 6 (EFR[4]=1, MCR[6]=1) | TCR | R/W | **0x0F** | [3:0] halt level = value × 4 (0–60); [7:4] resume level = value × 4 (0–60) |
| 7 | SPR | R/W | 0xFF **power-up only** (not reset by pin; preserved in sleep) | scratch |
| 7 (EFR[4]=1, MCR[6]=1) | TLR | R/W | 0x00 | [3:0] TX trigger = value × 4 spaces (4–60), 0 → use FCR[5:4]; [7:4] RX trigger = value × 4 chars (4–60), 0 → use FCR[7:6] |
| 7 (MCR[4]=0, MCR[2]=1) | FIFO Rdy | R | 0x03 | [0] TX FIFO A status; [1] TX FIFO B status; [4] RX FIFO A status; [5] RX FIFO B status; [7:6],[3:2] = 0. Readable from either channel (shows both). Bit meaning not defined in text; reset 0x03 with TXRDY# LOW/RXRDY# HIGH at reset suggests bit = complement of the corresponding TXRDYx#/RXRDYx# pin (inference) |
| 2 (LCR=0xBF) | EFR | R/W | 0x00 | [3:0] software flow control (Table 15); [4] enhanced function enable/latch; [5] special character detect (Xoff2); [6] auto RTS; [7] auto CTS |
| 4 (LCR=0xBF) | Xon1 | R/W | 0x00 power-up only | |
| 5 (LCR=0xBF) | Xon2 | R/W | 0x00 power-up only | |
| 6 (LCR=0xBF) | Xoff1 | R/W | 0x00 power-up only | |
| 7 (LCR=0xBF) | Xoff2 | R/W | 0x00 power-up only | also the special character |

EFR[4] (p.38): when 0, IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5] and DLD keep (latch) their last
values and cannot be modified; after reset they are 0 (ST16C550 compatible). TCR/TLR also
listed as requiring EFR[4]=1.

Table 15 (EFR[3:0], pp.37–38): bits[3:2] TX: 00 none, 10 send Xon1/Xoff1, 01 send Xon2/Xoff2,
11 send Xon1+Xon2 / Xoff1+Xoff2 (two sequential chars). Bits[1:0] RX: 00 none, 10 compare
Xon1/Xoff1, 01 compare Xon2/Xoff2, 11 compare Xon1+Xon2 / Xoff1+Xoff2 (two sequential
chars). Listed combos with [1:0]=11: 1011, 0111, 1111, 0011 as described. Caution: always
clear all EFR[3:0] bits to 0 before programming a new setting (p.37).

### Behavior

**Baud generator** (pp.13–15, 36). Clock: XTAL1/XTAL2 crystal (up to 24 MHz) or external clock
on XTAL1 (32/50/64 MHz max at 1.8/2.5/3.3 V, p.41). XTAL1 not 5 V tolerant. Prescaler ÷1 or ÷4
(MCR[7]). Divisor = DLM:DLL + DLD[3:0]/16, range 1 to 65535.9375. Sampling clock (16X/8X/4X
per DLD[5:4]) = (XTAL1 / prescaler) / divisor; baud = sampling clock / 16 (or 8, 4). Bit time
= 16/8/4 sampling clocks. Fractional implementation adds jitter: 8X mode ±1/16 bit when DLD
odd; 4X mode ±1/8 bit when DLD odd and not a multiple of 4 (p.13–14). Reset divisor 1
(power-up). Divisor 0 not described. Max data rate 16 Mbps (3.3 V, 4X, 64 MHz), 12.5 Mbps
(2.5 V), 8 Mbps (1.8 V) (p.1). No BAUDOUT/RCLK pins. Both channels share the oscillator.
Table 6 (p.15) gives 24 MHz example divisors (e.g. 115200 → DLL 0x0D, DLD 0, 0.16 %).

**Transmitter** (pp.15–16): start bit, 5–8 data LSB first, optional parity, 1/1.5/2 stops.
LSR[5] sets when THR byte moves to TSR (non-FIFO) / when TX FIFO empty (FIFO); LSR[6] when THR
(FIFO) and TSR both empty. TX idles HIGH; LCR[6] forces TX LOW. TX FIFO 64 bytes.

**Receiver** (p.17): start detected on falling edge; sampled at mid-bit after 8 (16X) / 4 (8X)
/ 2 (4X) sampling clocks; if still 0 the start is valid, otherwise false start ignored. Data/
parity/stop sampled mid-bit. Errors stored per byte (PE, FE, BI) and appear in LSR[4:2] when
the byte reaches RHR. Break: RX LOW for ≥ one character frame; in FIFO mode only one break
character loaded (p.34). Overrun: new character arrives while FIFO (or RHR) full → RSR data
overwritten, not put in FIFO; LSR[1] set, interrupt immediately (p.27, p.34). LSR[7] set while
any byte in RX FIFO has PE/FE/BI; clears when none left.
RX timeout: no data for 4 word lengths (per LCR[1:0]) plus 12 bit times (≈3.7–4.6 character
times) with ≥1 byte in FIFO (p.17, p.28, p.36); cleared by reading RHR.

**FIFO triggers** (Table 10 p.30, revised in Rev 1.1.1):
- FCR[5:4] TX trigger (spaces): 00 = 8, 01 = 16, 10 = 32, 11 = 56.
- FCR[7:6] RX trigger (characters): 00 = 8, 01 = 16, 10 = 56, 11 = 60.
- TLR overrides when non-zero (4..60 step 4). RX interrupt when RX FIFO count ≥ trigger
  (p.36; p.30 says "greater than"); cleared when it drops below. TX interrupt when available
  spaces ≥ TX trigger (p.36), or when the FIFO empties if it was not filled past the trigger on
  the last reload (p.30); p.30 text is self-contradictory on direction — use TLR text.
- FCR[5:4] defaults to 0 (=8) because they are enhanced bits.

**TXRDY#/RXRDY# pins** (Table 3, p.12):
- FIFO disabled: RXRDY# LOW = 1 byte in RHR, HIGH = none. TXRDY# LOW = THR empty, HIGH = byte in THR.
- FIFO enabled, FCR[3]=0: RXRDY# LOW = ≥1 byte in FIFO, HIGH = empty. TXRDY# LOW = FIFO empty,
  HIGH = ≥1 byte in FIFO.
- FIFO enabled, FCR[3]=1 (DMA mode): RXRDY# goes LOW when RX FIFO reaches trigger or timeout;
  returns HIGH when FIFO empties or LSR[7]=1. TXRDY# LOW = TX FIFO below trigger level
  (has ≥ trigger spaces), HIGH = FIFO full.
- Reset: TXRDY# LOW, RXRDY# HIGH.

**Interrupts** (Tables 4, 5, 9; pp.12, 28–29). ISR codes, priority 1 highest:

| Pri | ISR[5:0] | Source | Clear |
|---|---|---|---|
| 1 | 000110 (0x06) | Receiver line status: LSR[1], [2], [3], [4] or [7] | read all error characters out of RX FIFO (LSR read for overrun — implied) |
| 2 | 001100 (0x0C) | RX data timeout | read RHR |
| 3 | 000100 (0x04) | RX data ready / trigger reached | RX FIFO falls below trigger (non-FIFO: read RHR) |
| 4 | 000010 (0x02) | TX ready (THR empty / TX FIFO below trigger / transmitter empty in RS-485 mode) | read ISR or write THR |
| 5 | 000000 (0x00) | Modem status MSR[3:0] | read MSR |
| 6 | 010000 (0x10) | Received Xoff or special character | Xoff: when Xon received; special char: read ISR |
| 7 | 100000 (0x20) | CTS# or RTS# LOW→HIGH (de-assert) during auto flow control | read MSR |
| – | 000001 | none | |

IER[1] enable with THR already empty generates an interrupt immediately (p.27). IER[2]: LSR
interrupt stays while LSR[7]=1 (p.27). INT pin polarity (Intel): active HIGH, 3-state unless
MCR[3]=1. Table 4: with auto RS-485 enabled the TX interrupt waits for the transmitter (TSR)
empty, not just THR/FIFO.

**Auto RTS** (pp.18, 38): EFR[6]=1 and MCR[1]=1 (RTS# asserted LOW first). RTS# goes HIGH when
RX FIFO ≥ halt level TCR[3:0]×4; returns LOW when RX FIFO ≤ resume level TCR[7:4]×4. Receiver
keeps accepting data. IER[6] → ISR 0x20 on RTS# LOW→HIGH. Not supported in loopback (p.23).
TCR reset 0x0F → halt 60, resume 0.
**Auto CTS** (p.19): EFR[7]=1: CTS# HIGH stops TX after the current character's stop bit;
CTS# LOW resumes. IER[7] → ISR 0x20 on CTS# LOW→HIGH.
**Auto Xon/Xoff** (p.20): RX match of Xoff char(s) halts TX after current char; Xoff flag
(ISR[4], IER[5]) set; match of Xon resumes and clears. Flow-control characters are not stored
in the RX FIFO. When RX FIFO reaches halt level (TCR[3:0]) the UART sends Xoff1/2; when it falls
to the resume level it sends Xon1/2. Xon-Any (MCR[5]): any received character resumes TX; it
is stored unless it is an Xon/Xoff character under active flow control (p.33). Comparison
uses only LCR[1:0] word-length bits (p.20).
**Special character** (p.20, p.38): EFR[5]=1: each received char compared to Xoff2; on match
the char is stored in FIFO and ISR[4] set (if IER[5]). If EFR[1:0]=01 (Xon2/Xoff2 compare),
Xoff2 is not stored and generates both Xoff and special-char interrupt.
**Auto RS-485** (DLD[6], pp.9, 18, 37): RTS# is the direction output, auto-switched when a byte
is loaded into TX FIFO and released after the last stop bit of the last character (DLD[6]
text: "one bit time after the last stop bit"). TX interrupt delayed to TSR empty. Polarity:
**contradictory** in the datasheet. p.18 (idle LOW = receive, HIGH while transmitting) and
DLD[6] text p.37 (goes HIGH→LOW after the last stop bit, back HIGH when a byte is loaded)
agree: **RTS# HIGH during transmit, LOW when idle/receive**. p.9 says the opposite ("HIGH for
receive state and LOW for transmit state"). Majority reading: HIGH = transmit.
**IrDA** (DLD[7], pp.20–21): TX sends a 3/16-bit-wide HIGH pulse for each 0 bit, idles LOW;
RX idles LOW, each received light pulse decodes as a logic 0 bit — the text says "returns a
logic 1" (p.20) but Fig 12 shows pulses where data bits are 0; follow the figure (pulse = 0).
Fig 12 shows a 1/16-clock decode delay. The pin table (p.5–6) refers to "MCR[6]" for IR —
copy error; IR is DLD[7].
**Loopback** (MCR[4], p.23 Fig 13): TSR output → RSR input internally; TX pin held HIGH; RTS#,
DTR#, OP2# pins held HIGH (Fig 13 shows them tied to VCC); CTS#, DSR#, CD#, RI# inputs ignored;
internally RTS→CTS, DTR→DSR, OP1→RI, OP2→CD (MSR[4..7] = MCR[1],[0],[2],[3]). INT still
works. RX pin must be held HIGH.
**Sleep** (p.22): entered when, for **both** channels, IER[4]=1, no interrupt pending (ISR[0]=1),
MSR[3:0]=0 and RX idle HIGH. Oscillator stops (XTAL2 no output). Wakes on RX start bit (HIGH→
LOW), write to THR/FIFO, or change on CTS#/DSR#/CD#/RI#. Returns to sleep automatically after
interrupts serviced. Disabled by IER[4]=0. First characters after wake may be lost (oscillator
start-up). SPR preserved in sleep. (PowerSave pin only on STBGA.)
**Reset** (p.8, p.11, p.39): RESET pin active HIGH (Intel) / RESET# active LOW (Motorola),
min 40 ns. Resets registers per Table 16 above (DLL/DLM, SPR, Xon/Xoff only on power-up).
Outputs during/after reset: TX HIGH, OP2# HIGH, RTS# HIGH, DTR# HIGH, RXRDY# HIGH, TXRDY# LOW,
INT 3-state (Intel). Receiver input ignored during reset.
**Undefined/reserved**: MCR[6]=0 called "Reserved (default)"; reading with both CSA#/CSB# low;
LCR=0xBF at offsets 0, 1; FIFO Rdy bit polarity not specified; Motorola IRQ# channel combining
and MCR[3] effect not specified.

### Timing (AC table pp.41–42; TA -40..+85 °C, 70 pF; columns 1.8 V ±10 % / 2.5 V ±10 % / 3.3 V ±10 %)

| Sym | Parameter | 1.8 V | 2.5 V | 3.3 V |
|---|---|---|---|---|
| XTAL1 | crystal freq max | 24 MHz | 24 | 24 |
| ECLK | external clock max | 32 MHz | 50 | 64 |
| TECLK | ext clock period min | 15 ns | 10 | 7 |
| TAS | address setup (16) min | 5 | 5 | 5 |
| TAH | address hold (16) min | 0 | 0 | 0 |
| TCS | chip select width (16) min | 60 | 30 | 30 |
| TRD | IOR# width min | 60 | 30 | 30 |
| TDY | read/write cycle delay min | 60 | 30 | 30 |
| TRDV | data access from IOR# max | 55 | 25 | 25 |
| TDD | data disable after IOR# | 0–20 | 0–20 | 0–10 |
| TWR | IOW# width min | 60 | 30 | 30 |
| TDS | data setup to IOW#↑ min | 15 | 10 | 10 |
| TDH | data hold after IOW#↑ min | 5 | 5 | 5 |
| TADS | address setup (68) min | 5 | 5 | 5 |
| TADH | address hold (68) min | 0 | 0 | 0 |
| TRWS | R/W# setup to CS# min | 5 | 5 | 5 |
| TRDA | read data access (68) max | 55 | 25 | 25 |
| TRDH | read data disable (68) max | 20 | 20 | 10 |
| TWDS | write data setup (68) min | 15 | 10 | 10 |
| TWDH | write data hold (68) min | 5 | 5 | 5 |
| TRWH | CS#↑ to R/W# change min | 5 | 5 | 5 |
| TCSL | CS# width (68) min | 60 | 30 | 30 |
| TCSD | CS# cycle delay (68) min | 60 | 30 | 30 |
| TWDO | IOW# to modem output max | 50 | 50 | 50 |
| TMOD | modem input to INT max | 50 | 50 | 50 |
| TRSI | IOR# (MSR read) to INT reset max | 50 | 50 | 50 |
| TSSI | stop bit to set INT max | 1 Bclk | 1 | 1 |
| TRRI | IOR# to reset INT max (p.42) | 45 | 45 | 45 |
| TSI | stop to interrupt max | 45 ns | 45 | 45 |
| TINT | initial INT reset to TX start | 8–24 Bclk | 8–24 | 8–24 |
| TWRI | IOW# to reset INT max | 45 | 45 | 45 |
| TSSR | stop to set RXRDY# max | 1 Bclk | 1 | 1 |
| TRR | IOR# to reset RXRDY# max | 45 | 45 | 45 |
| TWT | IOW# to set TXRDY# max | 45 | 45 | 45 |
| TSRT | center of start to reset TXRDY# max | 8 Bclk | 8 | 8 |
| TRST | reset pulse width min | 40 ns | 40 | 40 |

(ns unless noted; Bclk = 16X/8X/4X sampling clock. Typical values not printed; these are
min/max.) Waveforms: Figs 15–25, pp.43–48.

### VCC
1.62–3.63 V single grade (p.1, p.40). Abs max 4 V, any pin -0.3..4 V (p.40) → **inputs not
5 V tolerant** (inputMax ≈ VCC; abs max 4 V; XTAL1 explicitly ≤ VCC, p.13). VIH min 1.4/1.8/2.0
V and VIL max 0.2/0.5/0.8 V at 1.8/2.5/3.3 V (p.40). Sleep current 3/10/15 µA (16 mode), 50/75/
100 µA (68 mode) max (p.40).

### Package drawing (census parts)
"PACKAGE DIMENSIONS (48 PIN TQFP - 7 X 7 X 1 mm)", p.49 (no drawing code, no land pattern
printed). mm: A 1.00–1.20, A1 0.05–0.15, A2 0.95–1.05, lead width B 0.17–0.27 (mid 0.22),
C 0.09–0.20, D (span) 8.80–9.20 (9.00), D1 (body) 6.90–7.10 (7.00), e 0.50 BSC,
L 0.45–0.75 (0.60), α 0–7°. Square, 12 leads/side, pin 1 corner marked; pins counter-clockwise
(1–12, 13–24, 25–36, 37–48). θJA 59 °C/W, θJC 16 °C/W (p.40).

### Obsolete
None (all three census rows Active).

### Notes
- Census "34B" FIFO is wrong — datasheet says 64 bytes everywhere.
- Datasheet copy errors: MCR[1] references "FCTR bit-3" (p.32; really DLD[6]); TLR text says
  "TCR[3:0]/TCR[7:4]" where it means TLR (p.36); pin table refers IR enable to "MCR[6]" (p.5–6;
  really DLD[7]); INTB description says "OP2A#"/"INTA" (p.5; means OP2B#/INTB); §2.18 says
  "M2751"/"2751" (p.22); Fig 4 Motorola diagram says "2.25 to 3.6 V" (p.10).
- RS-485 RTS# polarity conflict (p.9 vs pp.18/37), IrDA decode polarity text vs figure — see
  Behavior.
- FCR[5:4]/[7:6] default trigger levels were corrected in Rev 1.1.1 (p.52); the values above
  are the corrected ones.
- XR68M752 in Motorola mode: INTB and IOR# are unused (drawn NC); 68-mode sleep current is
  higher (internal pull-up on 16/68# presumably).
