# ti-c — TI dual parallel UARTs: TL16C2550, TL16C2552, TL16C2752, TL16C752C/D, NS16C2552/NS16C2752

All PDFs are in `datasheets/controller-uart/`. Page numbers are PDF pages (1-based), not printed folios. Where the printed page number differs, the PDF page is still used.

## PDF → devices

| PDF | Devices | Notes |
|---|---|---|
| tl16c2550.pdf (SLWS161E, 49 pp) | **TL16C2550** | PFB48, RHB32, FN44 drawn. No FN orderable in the addendum (pp35-37). |
| tl16c2550-q1.pdf (SLWS232, 40 pp) | **TL16C2550** (automotive grade -Q1) | Same text as tl16c2550.pdf; PFB only. |
| tl16c2552.pdf (SLWS163A, 40 pp) | **TL16C2552** | FN44 (and RHB32, not orderable). |
| tl16c2752.pdf (SLWS188A, 29 pp) | **TL16C2752** | FN44 (RHB32 drawn in the pin table only). Register **bit semantics for the enhanced bank are not described**; see that section. |
| tl16c752c.pdf (SLLS646C, 64 pp) | **TL16C752C** | PFB48, RHB32. |
| tl16c752ci-q1.pdf (57 pp) | **TL16C752C** (TL16C752CI-Q1 grade) | Feature text is the same word-for-word as tl16c752c.pdf (checked by diff). |
| tl16c752d.pdf (57 pp) | **TL16C752C** (TL16C752D grade) | Feature text is the same word-for-word. Differences: 48 MHz at 3.3 V, −40…85 °C, PFB only. |
| tl16c752d-q1.pdf (57 pp) | **TL16C752C** (TL16C752D-Q1 grade) | Feature text is the same word-for-word. −40…105 °C. |
| ns16c2552.pdf (SNLS238D, 56 pp) | **NS16C2552**, **NS16C2752** | Two designs in one sheet (16- vs 64-byte FIFO). |

**Census parts I could not decode from these PDFs:**
- **TL16C752PT** (Obsolete, census "48-LQFP 7x7, 2.7–3.6 V, IC DUAL 3.3V UART"). No PT package, LQFP drawing, or 3.3 V-only grade appears in tl16c752c.pdf, or in any of the other three TL16C752 PDFs. It is presumably the older TL16C752B, which has no datasheet in this group. It is obsolete, so leave it out.
- **TL16C2752IFNR**: obsolete and not listed in the tl16c2752.pdf addendum. It decodes from the part number as FN44, I grade, tape and reel.

---

## TL16C2550

- **Title:** "1.8-V to 5-V DUAL UART WITH 16-BYTE FIFOS". Two TL16C550D-type ACEs that share only the bus interface and the clock.
- **Datasheets:** tl16c2550.pdf: pins pp2-6, ROC p10-11, timing p13-14, waveforms p15-21, registers pp24-33, addendum pp35-37, PFB pp42-43, RHB pp46-47. tl16c2550-q1.pdf: the same content; ordering p8, addendum p33, PFB pp37-38.

### Parts
| Census part | Package (code) | Pins | Pinout | Grade | Supply |
|---|---|---|---|---|---|
| TL16C2550PFB | TQFP PFB (PFB0048A) | 48 | tqfp48 | commercial 0…70 °C (addendum p35) | 1.62–5.5 V in four bands (below) |
| TL16C2550PFBR | TQFP PFB, T&R 1000 | 48 | tqfp48 | 0…70 °C | same |
| TL16C2550IPFB | TQFP PFB | 48 | tqfp48 | I: −40…85 °C | same |
| TL16C2550IPFBR | TQFP PFB, T&R | 48 | tqfp48 | I: −40…85 °C | same |
| TL16C2550IRHB | VQFN RHB (RHB0032E) | 32+EP | qfn32 (tl16c2550-rhb variant, see below) | I: −40…85 °C, Last Time Buy | same |
| TL16C2550IPFBRQ1 | TQFP PFB, T&R | 48 | tqfp48 | automotive Q100, −40…85 °C (q1 p8) | same |

Decoding: "I" = −40…85 °C, none = 0…70 °C (abs-max table p10). "R" = tape and reel, "Q1" = automotive.

### VCC (ROC, p10-11)
The part runs in four separate bands. The table does not cover the gaps between them.
- 1.8 V ±10%: 1.62–1.98 V, clock ≤ 10 MHz
- 2.5 V ±10%: 2.25–2.75 V, clock ≤ 16 MHz
- 3.3 V ±10%: the table prints "3 / 2.5 / 2.75", a typo. The TL16C2552 sheet gives 3.0 / 3.3 / 3.6 V for the same band. Clock ≤ 20 MHz.
- 5 V ±10%: 4.5–5.5 V, clock ≤ 24 MHz
- Census: 1.8–5 V.

VI is 0…VCC in every band, so inputs are **not 5 V-tolerant** (inputMax = VCC). Abs-max VI is −0.5…7 V.

### Pinouts
**tqfp48 (PFB, p2; table pp4-6)**. TL16C752C PFB uses exactly the same pinout.
```
1 D5 inout        13 XTAL1 input     25 NC nc          37 NC nc
2 D6 inout        14 XTAL2 output    26 A2 input       38 CTSA input
3 D7 inout        15 IOW input       27 A1 input       39 DSRA input
4 RXB input       16 CDB input       28 A0 input       40 CDA input
5 RXA input       17 GND ground      29 INTB output*   41 RIA input
6 TXRDYB output   18 RXRDYB output   30 INTA output*   42 VCC power
7 TXA output      19 IOR input       31 RXRDYA output  43 TXRDYA output
8 TXB output      20 DSRB input      32 OPA output     44 D0 inout
9 OPB output      21 RIB input       33 RTSA output    45 D1 inout
10 CSA input      22 RTSB output     34 DTRA output    46 D2 inout
11 CSB input      23 CTSB input      35 DTRB output    47 D3 inout
12 NC nc          24 NC nc           36 RESET input    48 D4 inout
```
\* INTA/INTB are 3-state outputs: Hi-Z when MCR[3] = 0.

**qfn32 (RHB, p3 drawing; table pp4-6).** Exposed thermal pad: the land pattern (p47) numbers it pin 33. Its net is not named; the outline note on p46 only says it "must be soldered to the PCB".
```
1 D6 inout      9 NC nc          17 NC nc            25 CTSA input
2 D7 inout     10 XTAL1 input    18 A2 input         26 VCC power
3 RXB input    11 XTAL2 output   19 A1 input         27 D0 inout
4 RXA input    12 IOW input      20 A0 input         28 D1 inout
5 TXA output   13 GND ground     21 INTB? output     29 D2 inout
6 TXB output   14 IOR input      22 INTA? output     30 D3 inout
7 CSA input    15 RTSB output    23 RTSA output      31 D4 inout
8 CSB input    16 CTSB input     24 RESET input      32 D5 inout
```
**Conflict on pins 21/22.** The p3 drawing shows 21 = INTB and 22 = INTA; I checked the drawing as an image. The pin table on p4 gives INTA = 21, INTB = 22. The RHB part has no DSR, RI, CD, OP, RXRDY, TXRDY or DTR pins (note on p3).

The FN44 drawing on p3 has no census part, so it is omitted.

### Host interface (Intel-style, p4-5, p16 waveforms)
- **Bus:** A2..A0 and D7..D0. Two chip selects, CSA and CSB, both active low; each one selects its own UART channel.
- **Read:** IOR is active low. Data is driven td10 after IOR↓ and floats td11 after IOR↑. CS and address must be valid td7/td8 before IOR↓ and held th6/th7 after IOR↑.
- **Write:** IOW is active low. Data is latched on **IOW↑** (setup tSU3, hold th5). Address and CS must be valid td5/td4 before IOW↓ and held th4/th3 after IOW↑.
- No DDIS, no ALE, no RCLK or BAUDOUT pin. The feature list (p1) claims an "Independent Receiver Clock Input", but no such pin exists. Treat it as a leftover from another datasheet.
- The CTS/RTS pin descriptions mention "EFR bit 7/6", but this part has **no EFR**. Auto flow control is set by MCR[5] (p7, p30). This is a datasheet error.

### Registers (per channel; Table 1 p24, Table 3 p25)
| DLAB | A2..A0 | Read | Write |
|---|---|---|---|
| 0 | 000 | RBR | THR |
| 0 | 001 | IER | IER |
| X | 010 | IIR | FCR |
| X | 011 | LCR | LCR |
| X | 100 | MCR | MCR |
| X | 101 | LSR | (LSR; factory test only, "not recommended") |
| X | 110 | MSR | (—) |
| X | 111 | SCR | SCR |
| 1 | 000 | DLL | DLL |
| 1 | 001 | DLM | DLM |

DLAB = LCR[7].

- **IER:** b0 ERBI (received data available, plus the time-out in FIFO mode). b1 ETBEI (THRE). b2 ELSI (receiver line status). b3 EDSSI (modem status). b7:4 always 0. Reset 0x00.
- **IIR (read):** b0 = 0 when an interrupt is pending. b3:1 = ID (table below). b5:4 = 0. b7:6 = 11 when FCR[0] = 1, else 00. Reset 0x01.
- **FCR (write):**
  - b0 FIFO enable. It must be 1 when writing the other bits, or they are not programmed. Changing b0 clears both FIFOs.
  - b1 RX FIFO reset; b2 TX FIFO reset. Both self-clear and do not clear the shift registers.
  - b3 DMA mode: with FCR0 = 1, it switches the RXRDY/TXRDY mode from 0 to 1.
  - b5:4 reserved.
  - b7:6 RX trigger: 00 → 1, 01 → 4, 10 → 8, 11 → 14 bytes.
  - Reset 0x00.
- **LCR:**
  - b1:0 word length 5/6/7/8.
  - b2 stop bits: 1; or 1.5 for a 5-bit word / 2 for 6–8 bits. The receiver checks only the first stop bit.
  - b3 PEN. b4 EPS (1 = even). b5 stick parity: with b3 = 1, b4 = 1 → parity always 0; b4 = 0 → parity always 1.
  - b6 break: forces TX to 0. b7 DLAB.
  - Reset 0x00.
- **MCR:**
  - b0 DTR: 1 → DTR pin low.
  - b1 RTS: 1 → RTS pin low.
  - b2 OUT1: internal only; feeds loopback.
  - b3 OUT2: 0 → INT is Hi-Z and OP = 1; 1 → INT is driven and OP = 0.
  - b4 LOOP. b5 AFE (autoflow enable). b7:6 always 0.
  - Reset 0x00.
- **LSR:**
  - b0 DR. b1 OE. b2 PE. b3 FE. b4 BI.
  - b5 THRE. b6 TEMT.
  - b7: error in RX FIFO; always 0 in 450 mode.
  - Reset 0x60.
- **MSR:**
  - b0 ΔCTS. b1 ΔDSR. b2 TERI: RI went low→high (pin level).
  - b3 ΔDCD.
  - b4 = NOT CTS pin; b5 = NOT DSR; b6 = NOT RI; b7 = NOT CD.
  - Reset: b3:0 = 0, b7:4 follow the pins. Reading MSR clears b3:0.
- **SCR:** 8-bit scratch. Not reset.
- **DLL/DLM:** 16-bit divisor. Not reset.

Interrupt table (Table 5, p28):
| IIR b3..b0 | Priority | Source | Cleared by |
|---|---|---|---|
| 0001 | — | none | — |
| 0110 | 1 | LSR: OE, PE, FE, BI | reading LSR |
| 0100 | 2 | received data available (450 mode) / RX FIFO at trigger (FIFO mode) | reading RBR; in FIFO mode, when the FIFO drops below the trigger |
| 1100 | 2 | character time-out (FIFO mode only) | reading RBR |
| 0010 | 3 | THRE | reading IIR (when THRE is the source) or writing THR |
| 0000 | 4 | modem status: ΔCTS, ΔDSR, TERI, ΔDCD | reading MSR |

Reset table (Table 2, p24):
- IER = 0; IIR = 0x01; FCR = 0; LCR = 0; MCR = 0; LSR = 0x60; MSR b3:0 = 0.
- TX = high. INT = Hi-Z. OP = high. RTS = high. DTR = high.
- SCR, DLL/DLM, RBR, THR: no effect.
- RX FIFO cleared by MR/FCR1/FCR0 change; TX FIFO cleared by MR/FCR2/FCR0 change.

### Behavior
- **Baud generator (p31-32):**
  - divisor = XIN ÷ (baud × 16), 1…65535.
  - The clock is XTAL1, or a crystal across XTAL1/XTAL2. There is no prescaler.
  - The 16× clock drives both TX and RX; there is no separate RCLK pin.
  - Loading either divisor latch also loads the 16-bit baud counter.
  - Tables 9/10 (p32) give divisors for 1.8432 MHz and 3.072 MHz.
- **Receiver:**
  - Start-bit check: the start bit is sampled at its middle, 8 16×-clocks in (Fig 13 "8 CLKs", p17). A false start is rejected (feature list).
  - Data is sent and received LSB first (D0 first, p5).
  - Framing error: the receiver resyncs by assuming the 0 is the next start bit, and "samples this start bit twice" (p29).
  - Break: BI is set when RX stays low longer than a whole frame (start + data + parity + stop). Only one 0x00 character goes into the FIFO. Reception resumes after RX is marking for at least 2 RCLK samples and then a valid start bit arrives (p29).
  - Overrun in FIFO mode: OE is set only when the FIFO is full and another character has finished in the shift register. That character is overwritten and never enters the FIFO.
  - PE, FE and BI travel with each character and show in LSR when that character reaches the top of the FIFO. Reading LSR clears OE, PE, FE and BI.
  - LSR7 is set while any PE/FE/BI character is in the FIFO. It clears on an LSR read when no further errors remain.
- **FIFO (p25-27):**
  - 16 bytes each way. The RX FIFO stores 3 error bits per byte.
  - The RX data interrupt fires at the trigger level and clears when the level drops below it.
  - Time-out interrupt: at least 1 character is in the FIFO, no character has been received for 4 character times (the 2nd stop bit counts), and the CPU has not read the FIFO for 4 character times. It clears on an RBR read. When no time-out is pending, the timer restarts on each new character or FIFO read.
  - TX: THRE fires when the TX FIFO is empty. If the FIFO never held 2 bytes at once since it was last empty, THRE is delayed by one character time minus the last stop bit. The first THRE after FCR0 changes is immediate.
  - Polled mode: IER = 0 with FIFOs on.
- **DMA pins (Figs 16-20, p19-20; pin text p6):**
  - RXRDY mode 0: low when ≥ 1 character (td13 = 1 RCLK after the stop sample); high again after the RBR read empties it.
  - RXRDY mode 1: low at the trigger level or on time-out; high when the FIFO is empty.
  - TXRDY mode 0: low when THR/FIFO is empty; high after a write (td20).
  - TXRDY mode 1: high when the FIFO is full (16th byte written); low again td21 = 9 baudout cycles after the next start bit.
  - The pin table says TXRDY goes low "when there are at least a trigger level number of spaces". There is no TX trigger on this part.
- **Auto flow (p7-8, Table 8 p30):**
  - MCR[5] = 1 with MCR[1] = 1 → auto-RTS and auto-CTS. MCR[5] = 1 with MCR[1] = 0 → auto-CTS only. MCR[5] = 0 → neither.
  - Auto-CTS: the transmitter checks CTS before each byte. CTS must go inactive before the middle of the last stop bit to stop the next byte (tSU4).
  - Auto-RTS with trigger 1, 4 or 8: RTS goes high when the FIFO reaches the trigger; it goes low again when the FIFO is emptied by RBR reads.
  - Auto-RTS with trigger 14: RTS goes high after the first data bit of the 16th character; it goes low again when there is ≥ 1 free byte.
  - With autoflow enabled, ΔCTS does not raise an interrupt (MSR b0 text, p31).
- **Loopback (MCR[4], p30):**
  - TX is forced high and RX is disconnected; the TSR output feeds the RSR internally.
  - The CTS, DSR, CD and RI inputs are disconnected. Internally RTS → CTS, DTR → DSR, OUT1 → RI, OUT2 → CD (MSR b4..b7 equal MCR b1, b0, b2, b3).
  - The DTR, RTS, OUT1 and OUT2 outputs are held inactive (high).
  - Modem interrupts then come from MCR[3:0].
- **Reset:** RESET is active high, minimum pulse tw8 = 1 µs. Outputs are as in the reset table.
- Not present: sleep, IrDA, RS-485, Xon/Xoff, device ID.

### Timing (p13-14; max values, CL 30 pF; 1.8 / 2.5 / 3.3 / 5 V)
- Clock high tw1 ≥ 40/25/20/18 ns. Clock low tw2 ≥ 115/80/62/57 ns as printed (checked in the p13 image).
- IOW width tw6 ≥ 80/55/45/40 ns. IOR width tw7 ≥ 80/55/45/40 ns. Cycle tcR = tcW ≥ 115/80/62/57 ns.
- Data setup to IOW↑ tSU3 ≥ 25/20/15/15. Data hold th5 ≥ 15/10/5/5. Address hold th4/th7 ≥ 20/15/10/10. Address setup td5/td8 ≥ 15/10/7/7. CS setup and hold 0.
- IOR↓ to data td10 ≤ 55/35/25/20 ns. IOR↑ to float td11 ≤ 40/30/20/20 ns.
- Stop to INT / RXRDY↓ td13 = 1 RCLK cycle. RBR/LSR read to INT reset td14 ≤ 100/90/80/70 ns.
- Initial write to TX start td15 = 8…24 baudout. Start to INT td16 = 8…10. IOW to INT reset td17 ≤ 70/60/50/50 ns. Initial write to THRE INT td18 = 16…34 baudout. IOR↑ to INT reset td19 ≤ 70/50/35/35.
- Write to TXRDY inactive td20 ≤ 60/45/35/35 ns. Start to TXRDY active td21 = 9 baudout.
- CTS setup before mid-stop tSU4 ≥ 30/20/10/10 ns. CTS low to TX↓ td25 ≤ 24 baudout.
- Auto-RTS delays td26–td29 = 2 baudout.
- WR MCR to output td22 ≤ 90/70/60/50 ns. Modem input to INT td23 ≤ 60/50/40/35. MSR read to INT reset td24 ≤ 80/60/50/40.
- Note on p14: in FIFO mode, keep ≥ 1 baud clock between reading RBR and reading IIR/LSR.

### Package drawings
- **PFB0048A** (outline p42, land pattern p43; q1 pp37-38).
  - Body 7.0 (6.8–7.2) mm; span 9.0 (8.8–9.2).
  - Pitch 0.5. Lead width 0.22 (0.17–0.27). Foot length 0.6 (0.45–0.75). Height 1.2 max.
  - Land pattern: 48 pads 1.35 × 0.25; pad-row centres (8.5) mm.
- **RHB0032E** (p46 / p47).
  - Body 5.0 × 5.0 (4.9–5.1). Pitch 0.5.
  - Terminal length 0.4 (0.3–0.5); width 0.25 (0.2–0.3). Exposed pad 3.45 ± 0.1. Height 1 max.
  - Land pattern: pads 0.6 × 0.25, pad centres (4.8), EP 3.45.

### Obsolete
None. TL16C2550IRHB is Last Time Buy.

### Notes
- EFR references in the pin text are wrong (no EFR on this part).
- The "Independent receiver clock input" claim is wrong.
- The thermal table calls RHB "32-Pin TQFP" (p33).
- In the IIR bit-1/2 text, the 4th priority is printed as a sub-bullet. It is only formatting.

---

## TL16C2552

- **Title:** "1.8-V to 5-V DUAL UART WITH 16-BYTE FIFOS". It is the TL16C2550 core with one CS plus a CHSEL pin, multifunction MF pins, and an AFR register.
- **Datasheet:** tl16c2552.pdf: pins pp2-5, ROC pp8-9, timing pp11-12, registers pp22-31, addendum p32, FN pp36-37.

### Parts
| Census part | Package | Pins | Pinout | Grade | Supply |
|---|---|---|---|---|---|
| TL16C2552FN | PLCC FN (FN0044A), tube | 44 | plcc44 | 0…70 °C (addendum p32) | 1.62–5.5 V, four bands as TL16C2550 |
| TL16C2552FNR | PLCC FN, T&R 500 | 44 | plcc44 | 0…70 °C | same |

ROC (pp8-9) has the same bands as the TL16C2550, and the 3.3 V band is printed correctly here (3.0–3.6 V). Clock limits: 10 / 16 / 20 / 24 MHz. VI 0…VCC, so inputs are not 5 V-tolerant.

### Pinout plcc44 (FN, p2; table pp3-5)
```
1 TXRDYA output   12 GND ground     23 RTSB output    34 INTA output*
2 D0 inout        13 XTAL2 output   24 IOR input      35 MFA output
3 D1 inout        14 A1 input       25 RXB input      36 RTSA output
4 D2 inout        15 A2 input       26 TXB output     37 DTRA output
5 D3 inout        16 CHSEL input    27 DTRB output    38 TXA output
6 D4 inout        17 INTB output*   28 CTSB input     39 RXA input
7 D5 inout        18 CS input       29 DSRB input     40 CTSA input
8 D6 inout        19 MFB output     30 CDB input      41 DSRA input
9 D7 inout        20 IOW input      31 RIB input      42 CDA input
10 A0 input       21 RESET input    32 TXRDYB output  43 RIA input
11 XTAL1 input    22 GND ground     33 VCC power      44 VCC power
```
\* INTA/INTB are 3-state, gated by MCR[3].

MFA/MFB is selected by AFR[2:1] (per channel): OP (active low), BAUDOUT (16× clock), or RXRDY (active low). There are no separate RXRDY pins.

### Host interface
- CS (active low) together with CHSEL selects the channel: CHSEL = 1 → channel A, 0 → channel B. A2..A0 select the register.
- IOR and IOW are as on the TL16C2550. The timing table allows "IOW or CS" / "IOR or CS" as the strobe (p11), so the access is the overlap of CS and the strobe, and writes latch on the trailing edge.
- Consecutive cycles must be spaced: td12 (write→write) / td13 (read→read) ≥ 100/75/60/50 ns.
- With AFR[0] = 1 (concurrent write), a write goes to both channels. Reads still follow CHSEL.

### Registers
Same as TL16C2550, with these changes (Table 1 p22, Table 3 p23):
- **AFR** at DLAB = 1, A = 010. When DLAB = 1, IIR/FCR are therefore not reachable at 010 (Table 1 lists IIR/FCR with DLAB = 0).
- **AFR bits (p31):**
  - b0 concurrent write. It can be set or cleared from either channel's register set. The user must make LCR[7] equal in both channels before concurrent writes to addresses 0–2.
  - b2:1 MF select: 00 = OP (default), 01 = BAUDOUT, 10 = RXRDY, 11 = reserved.
  - b7:3 reserved, 0.
  - Reset 0x00.
- **MCR b3** also drives OP on MF when AFR[2:1] = 00.

**Reset table (Table 2, p22) differs from the TL16C2550 table:**
- "MCR: all bits except bit 3 cleared, MCR3 set".
- "INT: Output buffer enabled".
- "OP: Low".
- The MF pin text (p4) agrees: MCR bit 3 defaults to 1 after reset or power-up.

But the INTA/INTB pin text (p4) says "INTA-B are in the high-impedance state after reset". This is a datasheet conflict; see Notes.

### Behavior
Identical to the TL16C2550 section: FIFO (16 bytes), trigger levels 1/4/8/14, time-out, IIR table (pp25-26), LSR, MSR, loopback, autoflow through MCR[5]+MCR[1], and the baud formula. Additions:
- **BAUDOUT on MF:** it is the 16× clock. Fig 10 (p13) shows divisor 1 → MF = XTAL1, divisor 2 → XIN/2, divisor 3 → 1/3, and N > 3 → low for 2 XIN cycles and high for N−2 cycles. The figure is labelled "When AFR2:1 = 01".
- **RXRDY on MF:** behaves like the TL16C2550 RXRDY (mode 0 and mode 1 waveforms, Figs 16-17, p15-16).
- **TXRDYA/B:** dedicated pins, behaving as on the TL16C2550.

### Timing (p11-12; 1.8 / 2.5 / 3.3 / 5 V)
- tw1 = tw2 (clock high/low) ≥ 40/25/20/18 ns; one merged row.
- tw6/tw7 strobe ≥ 80/55/45/40. tSU3 ≥ 25/20/15/15. th5 ≥ 15/10/5/5. th4/th7 ≥ 20/15/10/10. td5/td8 ≥ 15/10/7/7.
- td10 ≤ 55/35/25/20 ns. td11 ≤ 40/30/20/20 ns (float after IOR↑ or CS↑).
- td12/td13 cycle-to-cycle ≥ 100/75/60/50 ns.
- BAUDOUT (FN only, p11): low/high tw3/tw4 ≥ 80/50/42/35 ns at CLK÷2; XIN↑ to BAUDOUT↑/↓ td1/td2 ≤ 55/40/30/25 ns.
- Receiver, transmitter and modem rows are the same as the TL16C2550 (p11-12).

### Package drawing
**FN0044A** (outline p36, land pattern p37; inches controlling).
- Body 16.51–16.66 mm (0.650–0.656 in); lead tip span 17.40–17.65 mm (0.685–0.695 in).
- Pitch 1.27. Lead width 0.33–0.53 (mid 0.43). Height 4.57 max.
- Land pattern: 44 pads 2.35 × 0.75; pad-row centres (16.2) mm.

### Obsolete
None in the census. TL16C2552IFN/IFNR are listed Active in the addendum but are not in the census.

### Notes
- Reset-state conflict on MCR[3] and INT, as above. The reset table plus the MF text say MCR3 = 1 after reset, so INT is driven and OP is low; the INT pin text says Hi-Z. A model has to pick one. The reset table is the more specific source.
- Pin text cites "EFR bit 7/6", but there is no EFR.
- The IOW pin text mentions "CSA and CSB", which do not exist on this part.

---

## TL16C2752

- **Title:** "1.8-V to 5-V DUAL UART WITH 64-BYTE FIFOS". It is pin- and software-compatible with the TL16C2552 (p2) and adds an enhanced register set:
  - 64-byte FIFOs and TX FIFO thresholds;
  - IrDA, RS-485 control and Xon/Xoff;
  - an 8× sampling option;
  - device ID registers.
- **Datasheet:** tl16c2752.pdf: description p2, pins pp4-5, ROC pp7-8, timing pp10-12, registers p20 (Table 1) and p21 (Table 2), addendum p22, FN pp25-26.
- **Errata:** "TL16C752C/TL16C754C/TL16C2752 Short STOP Bit Errata, SLLZ058" is cited in tl16c752c.pdf p51. It is not in the repo.

### Parts
| Census part | Package | Pins | Pinout | Grade | Supply |
|---|---|---|---|---|---|
| TL16C2752FN | PLCC FN (FN0044A), tube | 44 | plcc44 (same as TL16C2552) | 0…70 °C | 1.62–5.5 V bands |
| TL16C2752IFN | PLCC FN, tube | 44 | plcc44 | I: −40…85 °C | same |
| TL16C2752IFNR | PLCC FN, T&R | 44 | plcc44 | I | **Obsolete**; not in the addendum |

- ROC (pp7-8) has the same four bands. Clock limits: 10 / 16 / 20 / 24 MHz.
- The **feature list (p1) instead claims 16 / 24 / 32 / 48 MHz**: 1 / 1.5 / 2 / 3 Mbps at 16×, double that at 8×. The BAUDOUT timing table (p11) has tighter numbers than the TL16C2552, which is consistent with the faster clocks. **This is a conflict; flag it.**
- Inputs are not 5 V-tolerant (VI 0…VCC).

### Pinout
Same 44 pins as the TL16C2552 (table pp4-5; I checked every number). CD, DSR, RI and CTS "should be pulled high if unused".

### Host interface
Same as the TL16C2552: CS + CHSEL, IOR, IOW, and AFR concurrent write.

### Registers (Table 1 p20 and Table 2 p21, checked as images)
Address map:
| A2..A0 | Condition | Read | Write | Reset |
|---|---|---|---|---|
| 000 | LCR[7] = 0 | RHR | THR | XX |
| 001 | LCR[7] = 0 | IER | IER | 00 |
| 010 | LCR[7] = 0 | ISR (IIR) | FCR | 01 / 00 |
| 011 | any | LCR | LCR | 00 |
| 100 | LCR ≠ 0xBF | MCR | MCR | 00 |
| 101 | LCR ≠ 0xBF | LSR | reserved | 60 |
| 110 | LCR ≠ 0xBF | MSR | reserved | X0 |
| 111 | LCR ≠ 0xBF, FCTR[6] = 0 | SPR | SPR | FF |
| 111 | LCR ≠ 0xBF, FCTR[6] = 1 | FLVL (RX/TX FIFO level count) | EMSR | 00 / 80 |
| 000 / 001 | LCR[7] = 1, LCR ≠ 0xBF | DLL / DLM | DLL / DLM | XX |
| 010 | LCR[7] = 1, LCR ≠ 0xBF | AFR | AFR | 00 |
| 000 | LCR[7] = 1, LCR ≠ 0xBF, DLL = DLM = 0x00 | DREV (revision) | — | 00 |
| 001 | same | DVID | — | 0x0A |
| 000 | LCR = 0xBF | FC (FIFO level count) | TRG (trigger level) | 00 |
| 001 | LCR = 0xBF | FCTR | FCTR | 00 |
| 010 | LCR = 0xBF | EFR | EFR | 00 |
| 100–111 | LCR = 0xBF | XON1, XON2, XOFF1, XOFF2 | same | 00 |

Bit names (Table 2). Bits shaded in the PDF are writable only when EFR[4] = 1: IER[7:4], ISR[5:4], FCR[5:4] and MCR[7:5]. EFR b4 is labelled "Enable IER[7:4], ISR[5:4], FCR[5:4], MCR[7:5]".
- **IER:** b7 CTS int enable, b6 RTS int enable, b5 Xoff int enable, b4 sleep mode enable, b3 modem status, b2 RX line status, b1 TX empty, b0 RX data.
- **ISR:** b7:6 FIFOs enabled. b5:0 = INT source bits 5…0.
- **FCR:** b7:6 RX FIFO trigger. b5:4 TX FIFO trigger. b3 DMA mode. b2 TX FIFO reset. b1 RX FIFO reset. b0 FIFO enable.
- **LCR:** b7 divisor enable, b6 set TX break, b5 set (stick) parity, b4 even parity, b3 parity enable, b2 stop bits, b1:0 word length.
- **MCR:** b7 BRG prescaler, b6 IR mode enable, b5 XonAny, b4 internal loopback, b3 OP2# output control, b2 reserved (OP1#), b1 RTS#, b0 DTR#.
- **LSR:** b7 RX FIFO global error, b6 THR & TSR empty, b5 THR empty, b4 RX break, b3 framing, b2 parity, b1 overrun, b0 data ready.
- **MSR:** b7 CD# input, b6 RI#, b5 DSR#, b4 CTS#, b3 ΔCD#, b2 ΔRI#, b1 ΔDSR#, b0 ΔCTS#.
- **EMSR** (write): b7 "16X Sampling Rate Mode", b6 "LSR Error Interrupt Imd/Dly#", b5 "Auto RTS Hyst. Bit 3", b4 "Auto RTS Hyst Bit 2", b3 "Auto RS485 Output Inversion", b2 reserved, b1:0 "Rx/Tx FIFO Count".
- **AFR:** b7:3 reserved, b2 RXRDY# select, b1 Baudout# select, b0 concurrent write.
- **DVID** = 0000 1010.
- **FCTR:** b7 "RX/TX Mode", b6 "SCPAD Swap", b5:4 "Trig Table Bit 1:0", b3 "Auto RS485 Direction Control", b2 "RX IR Input Inv.", b1:0 "Auto RTS Hyst Bit 1:0".
- **EFR:** b7 auto CTS, b6 auto RTS, b5 special char select, b4 enable enhanced bits, b3:0 software flow control bits 3:0.

**NOT DECODABLE from this PDF.** The datasheet gives no further description of these registers. Missing:
- FCR[7:6] and FCR[5:4] trigger tables, and the "trigger table" A–D selection that FCTR[5:4] picks;
- TRG / FC semantics, and what "Rx/Tx Mode" FCTR[7] and EMSR[1:0] select (FLVL/FC reporting RX vs TX count);
- the hysteresis encoding (FCTR[1:0] + EMSR[5:4]), and what "SCPAD swap" does;
- the 8×/16× sampling details;
- IrDA pulse format, and RS-485 direction timing and polarity;
- the ISR priority table with sources 5:4, sleep conditions, Xon/Xoff rules, and the reset values of output pins.

The text only adds the following:
- p2: "embedded IrDA encoders and decoders, RS-485 transceiver controls, software flow control (Xon/Xoff) modes, programmable transmit FIFO thresholds, extended receive and transmit threshold levels for interrupts, and extended receive threshold levels for flow control halt/resume".
- p6: "Auto-RTS … When the receiver FIFO level reaches the defined halt trigger level 8 … RTS is automatically reasserted once the defined resume trigger level is reached".

The register map and names match MaxLinear/Exar's **XR16L2752** exactly; NS16C2752 p1 calls itself "compatible to EXAR XR16L2752". **The XR16L2752 datasheet is not in datasheets/controller-uart/.** The planner needs it, or must accept that this part cannot be modelled beyond the 16C550 core plus the NS16C2752-style EFR/Xon features. Do NOT treat NS16C2752 as a substitute: its register set differs (no FCTR/EMSR/TRG/FC/FLVL/DVID).

### Behavior that is printed
- 16C450 mode after reset.
- MF pins as on the TL16C2552.
- TXRDY pins: Figs 14-15 (p17) show mode 0 and mode 1. In mode 1 TXRDY goes high on FIFO full.
- Baud: 16× (or 8×) clock; divisor 1…65535 (p2).
- The block diagram has IR ENC/DEC per channel (p3).
- Autoflow waveform: Fig 2/3, p6.
- RESET is active high, tw8 ≥ 1 µs.
- INT is Hi-Z after reset (pin text p4). The MF/OP text again says MCR bit 3 defaults to 1 after reset (p4), which is the same conflict as on the TL16C2552.

### Timing (pp10-12; 1.8 / 2.5 / 3.3 / 5 V)
- tw1/tw2 ≥ 25/16/12/8 ns (merged row).
- Bus timing is identical to the TL16C2552: tw6/tw7 80/55/45/40; td10 55/35/25/20; td11 40/30/20/20; tSU3 25/20/15/15; th5 15/10/5/5; td12/td13 100/75/60/50.
- BAUDOUT tw3/tw4 ≥ 50/35/27/16 ns; td1/td2 ≤ 35/25/20/15 ns.
- Receiver, transmitter and modem rows are the same as the TL16C2552.

### Package drawing
FN0044A (p25 outline, p26 land pattern), as in the TL16C2552 section.

### Obsolete
TL16C2752IFNR.

---

## TL16C752C  (covers TL16C752C, TL16C752CI-Q1, TL16C752D, TL16C752D-Q1)

- **Title:** "TL16C752C Dual UART With 64-Byte FIFO". It is pin-compatible with SC16C752B / XR16M752 and with the TL16C2550 PFB.
- The enhanced set is the TL16C750/SC16C752B style: EFR, TCR/TLR, FIFORdy, AFR (IrDA / RS-485 / concurrent write), sleep, Xon/Xoff, and a ÷4 prescaler. All four PDFs have the same Feature Description, Register Maps and functional modes (diffed word by word). They differ only in grade.
- **Datasheets** (tl16c752c.pdf pages):
  - pins: PFB pp3-5, RHB pp6-7; ROC p8; timing pp11-14;
  - flow control pp19-22; reset p23; interrupts p24; time-out p25; baud pp26-27; DMA and sleep pp28-29;
  - register map p30, Table 7 pp31-32 (checked as images); FCR p33, LCR p34, LSR p35, MCR p36, MSR p36, IER p37, IIR/EFR p38, TCR/TLR p39, FIFORdy/AFR pp40-41, RS-485 p41, IrDA pp42-45;
  - addendum pp52-53; RHB pp58-59; PFB pp61-62.
  - The other PDFs have the same sections about 2 pages earlier (tl16c752d.pdf: register map p28, Table 7 pp29-30, ROC p6, timing p9, PFB pp54-55).

### Parts
| Census part | Package | Pins | Pinout | Grade | Supply / clock |
|---|---|---|---|---|---|
| TL16C752CPFB | TQFP PFB (PFB0048A) | 48 | tqfp48 (same as TL16C2550) | 0…70 °C (addendum p52), Last Time Buy | 1.62–5.5 V bands; 16/24/32/48 MHz |
| TL16C752CIPFB | TQFP PFB | 48 | tqfp48 | I −40…85 °C, LTB | same |
| TL16C752CIPFBR | TQFP PFB, T&R | 48 | tqfp48 | I, LTB | same |
| TL16C752CRHB | VQFN RHB (RHB0032E) | 32+EP | qfn32 (752C variant) | 0…70 °C; decoded from the part number (not in the addendum; census package blank) | same |
| TL16C752CRHBR | VQFN RHB, T&R | 32+EP | qfn32 | 0…70 °C, LTB | same |
| TL16C752CIRHB | VQFN RHB | 32+EP | qfn32 | I; decoded from the part number (not in the addendum) | same |
| TL16C752CIRHBR | VQFN RHB, T&R | 32+EP | qfn32 | I, LTB | same |
| TL16C752CIPFBRQ1 | TQFP PFB, T&R | 48 | tqfp48 | Q100, −40…105 °C (ci-q1 abs-max) | 1.62–5.5 V; 16/24/32/48 MHz |
| TL16C752DPFBR | TQFP PFB, T&R | 48 | tqfp48 | −40…85 °C (d abs-max) | 1.62–5.5 V; **16/24/48/48 MHz** (3.3 V raised to 48 MHz, d p6/p9) |
| TL16C752DPFBRQ1 | TQFP PFB, T&R | 48 | tqfp48 | Q100, −40…105 °C | 1.62–5.5 V; 16/24/32/48 MHz |
| TL16C752PT | — | — | — | **cannot decode**: obsolete, not in these PDFs | — |

ROC (c p8):
- 1.8 V band: 1.62–1.98 V, VI −0.3…0.9×VCC.
- 2.5 V band: 2.25–2.75 V, VI −0.3…0.9×VCC.
- 3.3 V band: 3.0–3.6 V, VI to VCC.
- 5 V band: 4.5–5.5 V, VI to VCC.
- Abs-max VI is VCC + 0.5, so inputs are **not 5 V-tolerant**.
- The TA = 0…70 note on the timing table is printed on every variant.

### Pinouts
- **tqfp48:** identical, pin by pin, to the TL16C2550 tqfp48 above (c pp3-5). The D variant's pin table lists NC as 12, 24 and 37; the drawing also shows 25 as NC.
  - DTRA/DTRB double as the RS-485 driver enable.
  - RXRDYA (31), RXRDYB (18), TXRDYA (43), TXRDYB (6).
- **qfn32 (RHB, c p6, checked as an image).** This pinout **differs from the TL16C2550 RHB**: DTRA/DTRB are present and there is no NC pin.
```
1 D6 inout      9 XTAL1 input    17 A1 input       25 CTSA input
2 D7 inout     10 XTAL2 output   18 A0 input       26 VCC power
3 RXB input    11 IOW input      19 INTB output*   27 D0 inout
4 RXA input    12 GND ground**   20 INTA output*   28 D1 inout
5 TXA output   13 IOR input      21 RTSA output    29 D2 inout
6 TXB output   14 RTSB output    22 DTRA output    30 D3 inout
7 CSA input    15 CTSB input     23 DTRB output    31 D4 inout
8 CSB input    16 A2 input       24 RESET input    32 D5 inout
EP (33) thermal pad, net not stated
```
\* INTA/INTB are 3-state. \*\* Pin 12 is drawn "GND,NC"; the pin table lists both "GND 12" and "NC 12".

There are no DSR, RI, CD, OP, RXRDY or TXRDY pins on RHB. The note on p6 omits TXRDYB, but it is not bonded either.

### Host interface
- Intel bus: A2..A0, D7..D0. CSA and CSB are active low; each selects its channel. If both are low, both channels are addressed (the AFR CONC text).
- IOR and IOW are active-low "valid level" strobes (p4). Per Figs 1/2 the cycle is the overlap of CS and the strobe.
- Timing is at TA 0…70 °C (p11):
  - Address setup t6s ≥ 20/15/10/5 ns, hold t6h ≥ 15/10/7/5.
  - Strobe width t7w (IOR) and t13w (IOW) ≥ 85/70/50/40.
  - Cycle delay t9d/t15d ≥ 85/70/60/50.
  - IOR to data t12d ≤ 65/50/35/25. Data disable t12h ≤ 35/25/20/15.
  - Write data setup t16s ≥ 40/30/20/15, hold t16h ≥ 35/25/15/10, relative to the IOW trailing edge.
- No DDIS and no ALE.

### Registers (Figure 26 p30; Table 7 pp31-32)
Access conditions:
- **LCR[7] = 1** (and LCR ≠ 0xBF for AFR only): DLL at 000, DLH at 001.
- **LCR[7:5] = 100:** AFR at 010.
- **LCR = 0xBF:** EFR at 010, Xon1 at 100, Xon2 at 101, Xoff1 at 110, Xoff2 at 111. 000/001 still give DLL/DLH, since LCR[7] = 1.
- **EFR[4] = 1 and MCR[6] = 1:** TCR at 110, TLR at 111.
- **Any CS low, MCR[2] = 1 and MCR[4] = 0:** FIFORdy at 111 (read).
- Otherwise: RHR/THR 000, IER 001, IIR/FCR 010, LCR 011, MCR 100, LSR 101, MSR 110, SPR 111.
- **Only EFR[4] = 1 allows writing IER[7:4], FCR[5:4] and MCR[7:5].** Per EFR b4 (p38), clearing EFR[4] latches those bits.

| Reg | Bits (b7…b0) | Reset (Table 7 / Table 2) |
|---|---|---|
| IER | b7 CTS int (CTS low→high), b6 RTS int, b5 Xoff int, b4 sleep, b3 modem status, b2 RX line status, b1 THR, b0 RX data/time-out | 00 |
| IIR | b7:6 = FCR[0] mirrored, b5 CTS/RTS, b4 Xoff/special char, b3:1 priority code, b0 0 = pending | 01 |
| FCR | b7:6 RX trigger 00 = 1, 01 = 4, 10 = 56, 11 = 60 chars; b5:4 TX trigger 00 = 8, 01 = 16, 10 = 32, 11 = 56 spaces (EFR[4] gated); b3 DMA mode; b2 TX FIFO reset; b1 RX FIFO reset (both self-clearing); b0 FIFO enable | 00 |
| LCR | as 16550: b1:0 word length; b2 stop (1, or 1.5 for 5-bit / 2); b3 PEN; b4 1 = even; b5 force parity: b5 = 1 with b4 = 0 → 1, b4 = 1 → 0; b6 break; b7 DLAB | **0x1D** (Table 2 p23 and Table 7: 8 bits, 2 stop, odd parity enabled) |
| MCR | b7 clock ÷1/÷4 (1 = ÷4); b6 TCR/TLR access; b5 Xon-any; b4 loopback; b3 IRQ enable (0 = INT Hi-Z, OP = 1); b2 FIFORdy enable (loopback → MSR[6]); b1 RTS; b0 DTR | 00 |
| LSR | b7 error in RX FIFO, b6 TEMT, b5 THRE, b4 BI, b3 FE, b2 PE, b1 OE, b0 DR | 0x60 |
| MSR | b7 CD#, b6 RI#, b5 DSR#, b4 CTS# (complements of the pins; MCR[3,2,0,1] in loopback); b3 ΔCD, b2 TERI, b1 ΔDSR, b0 ΔCTS (cleared on read) | b3:0 = 0 |
| SPR | scratch | not reset (Table 7 shows FF) |
| DLL/DLH | divisor | not reset |
| EFR | b7 auto-CTS, b6 auto-RTS, b5 special char detect (vs Xoff2), b4 enhanced function enable, b3:0 software flow mode (table below) | 00 |
| Xon1/Xon2/Xoff1/Xoff2 | 8-bit characters | not reset (Table 2 note) |
| TCR | b3:0 HALT level, b7:4 RESUME level, each ×4 (0…60) | 00 |
| TLR | b3:0 TX trigger (spaces) ×4, b7:4 RX trigger ×4; 0 → use FCR levels | 00 |
| FIFORdy (read) | b5 RX FIFO B ready, b4 RX FIFO A, b1 TX FIFO B, b0 TX FIFO A; b7:6 and b3:2 = 0 | — |
| AFR | b7:5 DLY2:0 (RS-485 turn-off delay, bit times), b4 RCVEN, b3 485LG, b2 485EN (labelled "485RN" in Table 7), b1 IREN, b0 CONC | **0x10** (AFR4 set, Table 2) |

Software flow modes, EFR[3:0] (Table 1, p21):
- TX side, bits 3:2: 00 none; 10 send Xon1/Xoff1; 01 send Xon2/Xoff2; 11 send Xon1+Xon2 / Xoff1+Xoff2.
- RX side, bits 1:0: 00 none; 10 compare Xon1/Xoff1; 01 compare Xon2/Xoff2.
- Bits 1:0 = 11: the receiver compares "Xon1 or Xon2, Xoff1 or Xoff2" when bits 3:2 = 10 or 01. When bits 3:2 = 11 or 00, it compares "Xon1 and Xon2, Xoff1 and Xoff2" (sequential pair).

Interrupt priority (Table 4 p24; Table 15 p38):
| IIR[5:0] | Prio | Source | Clear |
|---|---|---|---|
| 000001 | — | none | — |
| 000110 | 1 | RX line status: OE, or FE/PE/BI in the FIFO (via LSR[7]) | FE/PE/BI: when every erroneous char has been read from the FIFO; OE: reading LSR |
| 001100 | 2 | RX time-out | reading RHR |
| 000100 | 2 | RHR: data ready (FIFO off) / RX FIFO at or above trigger | reading RHR |
| 000010 | 3 | THR: THR empty (FIFO off) / TX FIFO spaces ≥ trigger | reading IIR, or writing THR |
| 000000 | 4 | modem status (MSR[3:0]) | reading MSR |
| 010000 | 5 | Xoff received / special character | an Xon is received (for Xoff), or reading IIR (special char) |
| 100000 | 6 | CTS or RTS changed from active (low) to inactive (high) | reading IIR |

Table 15 (p38) prints the modem row as "0 0 1 0 0 0". This is a datasheet error; Table 4 has 000000.

### Behavior
- **Clock and baud (pp26-27):**
  - Divisor = (XTAL1 ÷ prescaler) ÷ (baud × 16), 1…65535. Prescaler = 4 if MCR[7] = 1, else 1.
  - The text mentions a "CLKSEL pin" that sets MCR[7] at reset. **No CLKSEL pin exists on PFB or RHB**, so MCR[7] resets to 0 (÷1).
  - DLL = DLH = 0 stops the baud clock, and the UART is effectively disabled.
  - A crystal up to 24 MHz may be used; above that, an external oscillator is required (Fig 23 note).
  - TX and RX share the one baud clock.
- **Receiver:** 16× sampling with a 3-sample majority vote on every bit (Fig 15 note, p18). Data and stop/parity behave as on the 16550. Break: TX is driven low by LCR[6]. On receive, BI is set and a 00 byte is stored (LSR b4 text).
- **RX time-out (p25):** RX has been high for 4 × (programmed word length) + 12 bits while the RX FIFO holds ≥ 1 byte.
- **LSR[4:2]** show the error flags of the character at the top of the FIFO. Reading LSR does not pop the FIFO. LSR[7] is set while any error is anywhere in the FIFO.
- **Trigger levels:** set by FCR, or by TLR when its field is non-zero. After reset the FIFOs are disabled, so the effective trigger is 1 byte. Re-enabling IER[1] gives a new THR interrupt if the TX level is below threshold.
- **DMA (pp28-29):**
  - Mode 0, or FIFO disabled: TXRDY is active (low) when THR/FIFO is empty and goes inactive after 1 character. RXRDY is active when ≥ 1 character and inactive when empty.
  - Mode 1: TXRDY is active when ≥ trigger spaces and inactive when the FIFO is full. RXRDY is active at the trigger or on time-out; inactive when the FIFO is empty or LSR[7] flags an error.
  - Timing: stop to RXRDY t25d = 1 baud; IOR to RXRDY reset t26d = 1 µs (as printed); IOW to TXRDY t27d ≤ 70/60/50/40 ns; start to TXRDY reset t28d = 16 baud.
- **Hardware flow (p19-20):**
  - Auto-RTS: RTS is active while the RX FIFO level is below TCR[3:0] (HALT), goes inactive at HALT, and becomes active again at TCR[7:4] (RESUME). The program must set TCR[3:0] > TCR[7:4]; there is no hardware check.
  - Auto-CTS: the transmitter checks CTS before each byte. Releasing CTS before the middle of the last stop bit stops the next byte.
- **Software flow (pp19-22):**
  - Xoff is sent when the RX FIFO passes TCR[3:0]; Xon is sent when it reaches TCR[7:4].
  - A received Xoff halts TX after the current character, sets IIR[4] and raises INT if IER[5] = 1. The correct Xon(s) resume TX and clear IIR[4].
  - Xon-any (MCR[5]): any received character resumes TX.
  - Special character (EFR[5]): data matching Xoff2 sets IIR[4] without halting, and the character goes into the FIFO.
  - Flow characters are not stored in the FIFO. Exception: with sequential pairs, Xon1/Xoff1 are stored when the next character is not Xon2/Xoff2.
  - A flow character received with a parity, framing or break error is stored as data.
  - Disabling software flow after an Xoff was sent makes the UART send Xon. Changing EFR[3:0] after an Xoff sends the old Xon, then the new Xoff if the FIFO is still above the trigger.
  - Flow characters are sent with the current word length (5–8 LSBs).
  - Hardware and software flow must not be enabled together.
- **Sleep (p29):**
  - Enabled by EFR[4] = 1 plus IER[4] = 1.
  - Entered when RX is idle, the TX FIFO and TSR are empty, no interrupts are pending except THR/time-out, and the RX FIFO is empty.
  - In sleep the UART clock and baud clock stop.
  - Wakes on any RX change, any modem input change, or a write to the TX FIFO.
  - Do not write DLL/DLH while asleep.
- **IrDA (AFR[1] IREN; pp42-45):** SIR, specified only up to 115.2 kbps.
  - TX: while internal TX is 1, the TX pin stays low. For each 0 bit, TX rises on the falling edge of the 7th 16× clock and falls on the falling edge of the 10th, giving a 3-clock pulse (bit cells of 16 clocks).
  - RX: a falling edge on RX pulls internal RX low on the next 16× clock rising edge, for 16 clocks. Positive pulses also decode, because the trailing (falling) edge is used. Sampling is at the 8th clock.
  - RCVEN = 0 disables the receiver while transmitting in IrDA/RS-485 mode. With RCVEN = 0 the character time-out interrupt is unavailable, even in RS-232 mode (Table 21, p41).
- **RS-485 (AFR[2], pp40-42):**
  - The DTRx pin is set **high** while THR or TSR holds data. It rises one bit time before the first data (Fig 27 shows 1 baud time before TX starts; the text says "one bit time before the first stop bit"; ambiguous).
  - DTRx stays high for DLY[2:0] bit times after the last stop bit, then goes low. The text says "0 to 15 bit times", but DLY is 3 bits (0–7). Ambiguous; flag it.
  - Any THR write cancels the delay.
  - With 485EN set, TX interrupts wait until the TSR is empty unless 485LG = 1; with 485LG, interrupts are normal (threshold or empty).
  - RCVEN = 1 lets the receiver snoop its own transmission.
- **CONC (AFR[0]):** writes go to both channels (Table 20).
- **Loopback (MCR[4]):** MCR[3:0] loop to MSR[7:4] (DTR → DSR, RTS → CTS, MCR2 → RI, MCR3 → CD); TX loops to RX internally; the TX pin is inactive.
- **INT:** active high. 3-state when MCR[3] = 0 (Hi-Z after reset). The OP pins are the complement of MCR[3] and are high after reset.
- **Reset (Tables 2-3, p23):**
  - RESET is active high, tRESET ≥ 200 ns.
  - IER = 0, IIR = 01, FCR = 0, LCR = 0x1D, MCR = 0, LSR = 0x60, EFR = 0, TCR = 0, TLR = 0, AFR = 0x10. RHR/THR pointers cleared.
  - Not reset: DLL, DLH, SPR, Xon/Xoff.
  - Outputs: TX = high, RTS = high, DTR = high, RXRDY = high, **TXRDY = low**, INT = Hi-Z, OP = high.

### Timing (tl16c752c p11; 1.8 / 2.5 / 3.3 / 5 V)
- Bus timing as above. CP clock period ≥ 63/42/32/20 ns (D variant: 63/42/20/20).
- IOW to modem output t17d ≤ 60/40/30/20 ns (50 pF).
- Modem input to INT t18d ≤ 70/55/45/35. IOR to INT reset t19d ≤ 80/55/40/30.
- Stop to INT t20d = 1 baud (16× clock). IOR to RX INT reset t21d ≤ 55/45/35/25. Stop to TX INT t22d = 1 baud.
- Initial IOW to TX start t23d = 8…24 baud. IOW to TX INT reset t24d ≤ 75/45/35/25.
- "baudrate" units in this table mean 16× clock cycles.

### Package drawings
- **PFB0048A:** c pp61-62 (ci-q1, d, d-q1: pp54-55). Same as the TL16C2550 entry.
- **RHB0032E:** c pp58-59. Same as the TL16C2550 entry, EP 3.45 mm.

### Obsolete
TL16C752PT (undecodable).

### Notes
- Table 7 for D, D-Q1 and CI-Q1 shows FIFORdy bits labelled "RX FIFO D/C, TX FIFO D/C status". This is a copy-over from the TL16C754; C rev C says those bits were deleted. Treat them as 0.
- FIFORdy, SPR and TLR all sit at 111. The priority is FIFORdy when MCR[2] = 1 and loop is off, else TLR when EFR[4] = 1 and MCR[6] = 1 (also under LCR ≠ 0xBF), else SPR. The datasheet does not state this priority explicitly.
- Short-STOP-bit errata SLLZ058 is not in the repo.

---

## NS16C2552

- **Title:** "NS16C2552/NS16C2752 Dual UART with 16-byte/64-byte FIFO's and up to 5 Mbit/s Data Rate". It is footprint- and default-register compatible with the PC16552D, and adds an EFR-based enhanced set: Xon/Xoff, auto-RTS/CTS, sleep, IrDA, ÷4 prescaler and an ID register.
- **Datasheet:** ns16c2552.pdf: pinout drawings p2 (image checked), pin tables pp3-5, register map pp6-8, register details pp8-21, operation pp21-31, differences p37, DC/AC specs pp38-39, addendum pp44-45, PFB pp49-50, FN pp53-54.
- **Differs from NS16C2752 in:** 16-byte FIFOs; RX triggers 1/4/8/14; no TX trigger (FCR[5:4] reserved); TXRDY mode 1 asserts only when the TX FIFO is empty; DREV ID nibble = 0x3.

### Parts
| Census part | Package | Pins | Pinout | Grade | Supply |
|---|---|---|---|---|---|
| NS16C2552TVSX/NOPB | TQFP PFB (PFB0048A), T&R 1000 | 48 | tqfp48-ns (below) | −40…85 °C | 2.97–5.5 V |

Decoding (addendum p44): TVA = PLCC FN44; TVS = TQFP PFB48; X = tape and reel; /NOPB = lead-free.

### VCC
- 2.97–5.5 V (DC spec p38: two columns, "3.3 V 10%" and "5.0 V 10%").
- **Inputs are 5 V-tolerant** across the whole range (p1). VIH max is 5.5 V in both columns, so inputMax = 5.5 V.
- Clock: crystal ≤ 24 MHz; external clock ≤ 80 MHz (AC spec p38).

### Pinouts (p2, p3-5)
**tqfp48-ns.** This is different from the TL16C2550 tqfp48.
```
1 D5 inout        13 CS input        25 DSR2 input      37 NC nc
2 D6 inout        14 MF2 output      26 DCD2 input      38 CTS1 input
3 D7 inout        15 WR input        27 RI2 input       39 DSR1 input
4 A0 input        16 MR input        28 TXRDY2 output   40 DCD1 input
5 XIN input       17 GND ground      29 VCC power       41 RI1 input
6 GND ground      18 RTS2 output     30 INTR1 output    42 VCC power
7 XOUT output     19 NC nc           31 RXRDY1 output   43 TXRDY1 output
8 RXRDY2 output   20 RD input        32 MF1 output      44 D0 inout
9 A1 input        21 SIN2 input      33 RTS1 output     45 D1 inout
10 A2 input       22 SOUT2 output    34 DTR1 output     46 D2 inout
11 CHSL input     23 DTR2 output     35 SOUT1 output    47 D3 inout
12 INTR2 output   24 CTS2 input      36 SIN1 input      48 D4 inout
```
**plcc44.** Same positions as the TL16C2552 FN (channel 1 = A, 2 = B). Only NS16C2752TVA (obsolete) is in the census.
```
1 TXRDY1, 2-9 D0-D7, 10 A0, 11 XIN, 12 GND, 13 XOUT, 14 A1, 15 A2, 16 CHSL, 17 INTR2, 18 CS,
19 MF2, 20 WR, 21 MR, 22 GND, 23 RTS2, 24 RD, 25 SIN2, 26 SOUT2, 27 DTR2, 28 CTS2, 29 DSR2,
30 DCD2, 31 RI2, 32 TXRDY2, 33 VCC, 34 INTR1, 35 MF1, 36 RTS1, 37 DTR1, 38 SOUT1, 39 SIN1,
40 CTS1, 41 DSR1, 42 DCD1, 43 RI1, 44 VCC
```
- Roles: D = inout (3-state). INTR = output (push-pull; not gated by OUT2). MF and RXRDY on TQFP: RXRDY is a separate pin there; on PLCC it is only available via MF.
- The NC pins are typed "I" in the table.
- MF selection uses AFR[2:1]: 00 = OUT2 (default), 01 = BAUDOUT, 10 = RXRDY, 11 = forced 1. BAUDOUT is also the internal receiver clock (p5).

### Host interface (p3, AC p39)
- CS active low. CHSL = 1 → channel 1, 0 → channel 2.
- RD: data driven on the falling edge, CPU samples on the rising edge.
- WR: data latched on the **rising edge** of WR.
- A2..A0. No ALE or DDIS.
- AFR concurrent write is as on the TL16C2552.

### Registers (Table 1 p6, Table 2 pp7-8, details pp8-21)
Address map, per channel:
- **DLAB = 0:** RBR/THR at 0x0, IER at 0x1, IIR (read) / FCR (write) at 0x2.
- **Any DLAB:** LCR 0x3, MCR 0x4, LSR 0x5, MSR 0x6, SCR 0x7.
- **LCR[7] = 1, LCR ≠ 0xBF:** DLL 0x0, DLM 0x1, AFR 0x2. DREV is at 0x0 (read) when DLL = DLM = 0x00.
- **LCR = 0xBF:** EFR 0x2, XON1 0x4, XON2 0x5, XOFF1 0x6, XOFF2 0x7.
- The summary (p7) marks MCR, LSR and MSR "LCR ≠ 0xBF", and SCR as well (Table 2).
- IER[7:4], FCR[5:4] and MCR[7:5] are writable only when EFR[4] = 1. On reset they clear, and so does IIR[5:4] (Table 20).

| Reg | Bits | Reset |
|---|---|---|
| IER | b7 CTS int (CTS low→high, needs EFR4), b6 RTS int (RTS low→high), b5 Xoff int, b4 sleep, b3 modem status, b2 RX line status, b1 THR empty, b0 RX data + time-out | 00 |
| IIR | b7:6 = 11 if FIFOs on; b5 RTS/CTS low→high; b4 Xoff/special char; b3:1 source; b0 1 = none | 01 |
| FCR | b7:6 RX trigger 00 = 1, 01 = 4, 10 = 8, 11 = 14; b5:4 reserved (no effect); b3 DMA mode 1; b2 TX FIFO reset; b1 RX FIFO reset (both only when FCR0 = 1, self-clearing); b0 FIFO enable (must be set before the other bits are written) | 00 |
| LCR | b7 DLAB; b6 break (SOUT = 0); b5:3 parity: 111 force 0, 101 force 1, 011 even, 001 odd, xx0 none; b2 stop (1; 1.5 if 5-bit, 2 if 6–8 bits); b1:0 word length 5–8 | 00 |
| MCR | b7 clock divider: 1 = XIN ÷ 4; b6 IrDA mode; b5 Xon-any; b4 loopback; b3 OUT2: 1 → OUT2 pin low; b2 OUT1: no pin, feeds MSR6 in loop; b1 RTS: 1 → pin low; b0 DTR: 1 → pin low | 00 |
| LSR | b7 RX FIFO error (0 in 450 mode), b6 TEMT, b5 THRE, b4 BI, b3 FE, b2 PE, b1 OE, b0 DR | 0x60 |
| MSR | b7 ~DCD, b6 ~RI, b5 ~DSR, b4 ~CTS (OUT2, OUT1, DTR, RTS in loop); b3 ΔDCD; b2 TERI (RI 0→1); b1 ΔDSR; b0 ΔCTS; deltas cleared on read | b3:0 = 0 |
| SCR | scratch | **0xFF** (Table 14) |
| DLL/DLM | divisor; not changed by MR | X |
| AFR | b7:3 reserved 0, b2:1 MF select, b0 concurrent write | 00 |
| DREV | b7:4 device ID = 0x3 (NS16C2552) / 0x2 (NS16C2752), b3:0 revision = 0x1 | — |
| EFR | b7 auto-CTS, b6 auto-RTS, b5 special char detect (vs XOFF2), b4 enhanced bits write enable, b3:0 SW flow (table below) | 00 |
| XON1/2, XOFF1/2 | characters | 00 |

Software flow modes, EFR[3:0] (Table 20, p20):
- RX side:
  - 1111 → compares Xon1 & Xon2, Xoff1 & Xoff2.
  - 1011 or 0111 → compares Xon1 or Xon2, Xoff1 or Xoff2.
  - 0011 → compares Xon1 & Xon2, Xoff1 & Xoff2.
  - xx10 → compares Xon1/Xoff1. xx01 → compares Xon2/Xoff2. xx00 → none.
- TX side, bits 3:2: 11 sends Xon1 + Xon2 / Xoff1 + Xoff2; 10 sends Xon1/Xoff1; 01 sends Xon2/Xoff2; 00 none.

Interrupts (Tables 7-8, p10-11):
| IIR[5:0] | Prio | Source | Clear |
|---|---|---|---|
| 000110 | 1 | LSR[4:1] | reading LSR |
| 001100 | 2 | RX time-out: 4 words + 12 bit times | reading RBR |
| 000100 | 3 | RX trigger reached | read until below trigger |
| 000010 | 4 | THR empty / TX threshold | reading IIR or writing THR |
| 000000 | 5 | MSR[3:0] | reading MSR |
| 010000 | 6 | Xoff or special character | reading IIR, or receiving Xon (special char: the next character) |
| 100000 | 7 | CTS input 0→1 (auto-CTS) / RTS output 0→1 (auto-RTS) | reading IIR or MSR |
| 000001 | — | none | — |

While the CPU reads IIR, the channel freezes the indication (p10).

### Behavior
- **Baud (p18, p21):**
  - divisor = fin ÷ (baud × 16), where fin = XIN ÷ (MCR[7] ? 4 : 1). Divisor is 1…65535; "a divisor of zero is not recommended".
  - Each channel has its own prescaler and generator.
  - Loading either latch reloads the baud counter.
  - The 16× clock (BAUDOUT) is the RX clock.
- **Receiver (p23):**
  - The start bit is sampled at the 8th 16× clock and must be 0 to be valid.
  - Error flags travel with each character. BI loads one 0 character; reception restarts after SIN marks and a valid start bit arrives.
  - The error bits also reset "when the next valid character is loaded into RBR" (LSR text).
- **Time-out (p23):** 4 words (LCR[1:0] length) + 12 bit times ≈ 3.6–4.7 frames. It clears, and the timer restarts, on an RBR read. When inactive, the timer restarts on each new character or RBR read. It uses BAUDOUT as its clock.
- **Auto-RTS** (EFR[6] = 1 and MCR[1] = 1). Levels from Table 27 (p25):
  | RX trigger | INTR at | RTS deasserts at | RTS reasserts at |
  |---|---|---|---|
  | 1 | 1 | 2 | 0 |
  | 4 | 4 | 8 | 1 |
  | 8 | 8 | 14 | 4 |
  | 14 | 14 | 14 | 8 |

  The RTS interrupt (IER[6]) fires on RTS 0→1.
- **Auto-CTS** (EFR[7]): transmission stops after the stop bit of the current character once CTS goes high, and resumes on CTS low. The CTS interrupt (IER[7]) fires on CTS high.
- **Xon/Xoff (pp27-28), per Table 29:**
  | RX trigger | Xoff sent at | Xon sent at |
  |---|---|---|
  | 1 | 1 | 0 |
  | 4 | 4 | 1 |
  | 8 | 8 | 4 |
  | 14 | 14 | 8 |
  - Received Xoff: TX stops after the current frame; IIR[4] is set (if IER[5]). Matching Xon resumes TX and clears IIR[4].
  - Flow characters are not stored.
  - Xon-any (MCR[5]): any received character resumes TX and is stored, unless it is Xon or Xoff.
  - Special character (EFR[5]): data matching XOFF2 is stored and flags IIR[4]. In mode EFR[1:0] = 01, XOFF2 is not stored and raises both the Xoff and special interrupts (Table 20).
  - Comparison uses the LCR word length.
- **TX (p26):**
  - THRE/interrupt when the FIFO is empty.
  - DMA mode 0: TXRDY is low when THR/FIFO is empty and high once not empty.
  - DMA mode 1: TXRDY is low when the TX FIFO is empty and high when full.
  - Writing THR or reading IIR deasserts the interrupt.
- **RXRDY:**
  - Mode 0: low with ≥ 1 character, high when empty.
  - Mode 1: low at the trigger or on time-out, high when empty.
  - In 16450 mode, only mode 0 is available.
- **Sleep (IER[4], pp29-30):**
  - Entered when there is no pending interrupt, the divisor is ≠ 0, MSR[3:0] = 0 and SIN idles at 1.
  - Wakes on a SIN falling edge, a THR write, or any modem input change.
  - Re-sleeps after all interrupts have been serviced.
- **IrDA (MCR[6], p30):**
  - SOUT idles at 0; each 0 bit becomes a 3/16-bit-wide high pulse.
  - On receive, each light pulse decodes as 0, and SIN idles at 0.
  - The p4 text says SOUT idles at 1 in UART mode and at 0 in IrDA mode, and that reset cannot select IrDA.
- **Loopback (p30):**
  - SOUT is held at 1, and RTS/DTR at 1. Modem outputs loop to MSR as in the MCR text.
  - SIN must be held at 1 during the test to avoid a false start bit on exit.
- **INTR (p4):** active high and not 3-stated. Low after reset.
- **Reset (p22):**
  - MR is active high, tRST ≥ 70 ns.
  - An internal power-on reset holds the device for 2^23 XIN cycles (printed "223": 24 MHz → 349 ms).
  - Output states (Table 26): SOUT = 1, OUT2 = 1, RTS = 1, DTR = 1, INTR = 0, TXRDY = 0. RXRDY is not listed.
  - MR clears all registers, including the shift registers, except RBR, THR and the divisor latches. It also clears IER[7:4], IIR[5:4], FCR[5:4] and MCR[7:5].

### Timing (AC spec pp38-39; max; 3.3 V / 5 V)
- **Clock:** crystal ≤ 24 MHz. External clock ≤ 80 MHz, high/low ≥ 6 ns.
- **Bus:**
  - Address setup tAR ≥ 10 ns, hold tRA ≥ 1 ns.
  - RD width tRD ≥ 35/24 ns. Read cycle delay tDY ≥ 35/24. Data access tRDV ≤ 35/24. Data disable tHZ 0…18.
  - WR width tWR ≥ 35/24. Write cycle delay ≥ 35/24. Data setup tDS ≥ 12, hold tDH ≥ 4.
- **Modem:** WR to output tMDO ≤ 20/15 ns. Modem input to INTR tSIM ≤ 20/15. RD to INTR reset tRIM ≤ 23/17.
- **Line:**
  - Stop to INTR tSINT = 4 Bclk. RD to INTR reset tRINT ≤ 45/30 ns.
  - Centre of start to INTR tSTI "16 / 10 ns" (units as printed). WR to TX start tWST 0…16 Bclk. WR to INTR clear tHR ≤ 34/22 ns.
- **DMA:** WR to TXRDY↑ tWXI ≤ 27/18 ns. Start centre to TXRDY↓ tSXA = 8 Bclk. Stop to RXRDY↓ tSSR = 4 Bclk. RD to RXRDY↑ tRXI ≤ 27/18 ns.

### Package drawings
- **PFB0048A:** pp49-50. Same as the TL16C2550 entry.
- **FN0044A:** pp53-54. Same as the TL16C2552 entry.

### Obsolete
None for this device in the census. NS16C2552TVA is obsolete but not in the census.

### Notes
- LCR[4] text: "1 = Odd parity … 0 = Even (default)". This contradicts the parity table directly above it (LCR[5:3] = 011 → even) and the 16550 convention. Use the table.
- The DREV text says "device ID for NS16C2552 is 0x03", while the table puts ID in b7:4 and revision 0x1 in b3:0, which reads as 0x31 / 0x21.
- The Table 2 summary lists IIR bits 5:4 for the 2552 as well; both devices have the full enhanced set.
- p31 Table 31 says the maximum input clock is 80 MHz for both devices.

---

## NS16C2752

- **Same datasheet as NS16C2552** (ns16c2552.pdf, same pages). Pins, bus, registers and behavior are identical except as listed below.
- **FIFO:** 64 bytes.
- **FCR[7:6] RX trigger:** 00 = 8, 01 = 16, 10 = 56, 11 = 60 (p11).
- **FCR[5:4] TX trigger** (written with EFR[4] = 1): 00 = 8, 01 = 16, 10 = 32, 11 = 56 empty spaces (p12).
  - THRE/TX interrupt fires when the empty spaces exceed the threshold.
  - Two-character hysteresis: the host must write ≥ 2 characters before the next interrupt.
  - DMA mode 1: TXRDY is low when spaces exceed the threshold, high when the FIFO is full.
  - Notes on the TX FIFO (p37): reading IIR without filling the FIFO deasserts INTR, and no new THRE fires when the FIFO later empties. Writing ≥ 1 character services the interrupt. A TX FIFO reset causes a THRE interrupt.
- **Auto-RTS** (Table 28):
  | RX trigger | INTR at | RTS deasserts at | RTS reasserts at |
  |---|---|---|---|
  | 8 | 8 | 16 | 0 |
  | 16 | 16 | 56 | 8 |
  | 56 | 56 | 60 | 16 |
  | 60 | 60 | 60 | 56 |
- **Xon/Xoff** (Table 30):
  | RX trigger | Xoff sent at | Xon sent at |
  |---|---|---|
  | 8 | 8 | 0 |
  | 16 | 16 | 8 |
  | 56 | 56 | 16 |
  | 60 | 60 | 56 |
- **DREV** b7:4 = 0x2.

### Parts
| Census part | Package | Pins | Pinout | Grade | Supply |
|---|---|---|---|---|---|
| NS16C2752TVSX/NOPB | TQFP PFB, T&R | 48 | tqfp48-ns | −40…85 °C | 2.97–5.5 V, 5 V-tolerant inputs |
| NS16C2752TVA/NOPB | PLCC FN (FN0044A) | 44 | plcc44 (NS) | −40…85 °C | same; **Obsolete** (addendum p44 and census) |

### Obsolete
NS16C2752TVA/NOPB.

### Notes
NS16C2752 is **not** register-compatible with TL16C2752. The NS part has no FCTR, EMSR, TRG, FC, FLVL or DVID. Model them separately.

---

## Cross-device summary

Three register families:
- **16550 + MCR[5] autoflow:** TL16C2550, TL16C2552 (+ AFR/MF).
- **EFR + Xon/Xoff + MCR[7:5] enhanced bits:** NS16C2552/2752, TL16C752C (+ TCR/TLR/FIFORdy/AFR-IrDA/RS-485).
- **XR16L2752-style FCTR/EMSR/TRG:** TL16C2752. Bit semantics are not available in the repo.

Shared pinouts:
- **tqfp48 (TI):** TL16C2550 = TL16C752C/D.
- **qfn32:** TL16C2550 RHB ≠ TL16C752C RHB.
- **plcc44:** TL16C2552 = TL16C2752 = NS16C2x52 FN (NS uses names SIN/SOUT/RD/WR/MR/XIN/XOUT/CHSL/INTR/DCD and channel suffixes 1/2).
- **NS tqfp48** is its own pinout.
