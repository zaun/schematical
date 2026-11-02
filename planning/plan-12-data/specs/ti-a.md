# ti-a: TI / National / Intersil 16450/16550-class UARTs

## PDF → devices

| PDF (datasheets/controller-uart/) | Device(s) | Census parts |
|---|---|---|
| tl16c450.pdf | TL16C450 | TL16C450FN |
| tl16c451.pdf | TL16C451 (1 UART + parallel printer port), TL16C452 (2 UARTs + printer port) | TL16C451FN (Obsolete), TL16C451FNR, TL16C452FNR |
| tl16c550b.pdf | TL16C550B | TL16C550BFN (Obsolete), TL16C550BN (Obsolete), TL16C550BPTR |
| tl16c550c.pdf | TL16C550C | TL16C550CFN (Obs), CFNR, CFNRG4, CIFN (Obs), CIFNR, CIPTR, CPFB (Obs), CPFBR, CPT (Obs), CPTG4 (Obs), CPTR |
| tl16c550d.pdf | TL16C550D | TL16C550DIPFB, DIPFBR, DIPT, DIPTR, DIRHB, DIRHBR, DIZQSR (Obs), DPFBR, DPT, DPTR, DRHB, DRHBR, DZQSR (Obs) |
| pc16550d.pdf | PC16550D | PC16550DV (TI row Obsolete, National row Active), PC16550DVX (TI row Obsolete, National row Active) |
| 82c50a.pdf | 82C50A | CS82C50A-5, CS82C50A-596 (Obsolete) |

## Census parts I could not decode

- None. Every census part has a package, pinout and grade. Two caveats:
  - The FN68 package (TL16C451/452) and the ZQS 24-ball BGA (TL16C550D) have no outline drawing in their PDFs. Pinout and ball map are given; footprint dimensions must come from TI's package documents.
  - CS82C50A-5 is not a row in the 2006 Intersil ordering table. It decodes from the naming scheme: C = 0–70 °C, S = 44-PLCC, -5 = 625 kbaud grade, no 96 = not tape and reel.
- Census vcc "3.3V, 5V" for TL16C550B contradicts its datasheet, which says 5 V only (4.75–5.25 V). See the TL16C550B section.

## TL16C450

- **Title / function:** "TL16C450 Asynchronous Communications Element" (ACE): a CMOS 16450-class
  single UART, no FIFO. It is a drop-in, faster replacement for National NS16C450 (p1). Source: `datasheets/controller-uart/tl16c450.pdf`
  (SLLS037C, Mar 1988, rev. Jan 2006, 31 pp). Page numbers below are PDF pages; data pages 1–24
  carry the same printed numbers. Package addendum p25, materials p26, FN0044A outline p27,
  land pattern p28, stencil p29.
- **Nearest sibling:** TL16C550B/C/D and PC16550D add a 16-byte FIFO, the FCR at write address 2,
  RXRDY/TXRDY pins and IIR[3,6,7]. The TL16C450 has none of these: IIR bits 3–7 always read 0 and
  there is no FCR. It has the same 8250/16450 register map as the 82C50A, plus a scratch register.

### Parts
| Census part | Package | Pins | Pinout name | Grade | VCC |
|---|---|---|---|---|---|
| TL16C450FN | PLCC, TI code **FN** (drawing FN0044A) | 44 | plcc44 | Commercial 0–70 °C; one speed grade (XTAL ≤ 9 MHz) | 4.75–5.25 V |

The suffix decodes as FN = 44-pin PLCC tube (p25 lists TL16C450FN with a tube of 26, and TL16C450FNR
on a 500-piece reel). No other grade exists. The N package (40-pin DIP) is shown on p1 but is not in
the census: "will be obsoleted as of 7/30/2006" (p1).

### Pinout: plcc44 (FN, top view, p1; terminal table pp4–5)
Pins marked with an overbar in the datasheet are listed here as active-low. Repo names drop the
overbar. DISTR/DOSTR exist in both polarities, so the complemented pin of each pair gets the `BAR` suffix.

| Pin | Name | Role | Active / notes |
|---|---|---|---|
| 1 | NC | nc | |
| 2 | D0 | inout | 3-state data bus |
| 3 | D1 | inout | |
| 4 | D2 | inout | |
| 5 | D3 | inout | |
| 6 | D4 | inout | |
| 7 | D5 | inout | |
| 8 | D6 | inout | |
| 9 | D7 | inout | |
| 10 | RCLK | input | receiver 16× clock |
| 11 | SIN | input | serial in |
| 12 | NC | nc | |
| 13 | SOUT | output | serial out |
| 14 | CS0 | input | active high |
| 15 | CS1 | input | active high |
| 16 | CS2 | input | active low (overbar) |
| 17 | BAUDOUT | output | overbar name; 16× transmit clock |
| 18 | XTAL1 | input | clock / crystal input |
| 19 | XTAL2 | output | crystal output |
| 20 | DOSTRBAR | input | write strobe, active low |
| 21 | DOSTR | input | write strobe, active high |
| 22 | VSS | ground | |
| 23 | NC | nc | |
| 24 | DISTRBAR | input | read strobe, active low |
| 25 | DISTR | input | read strobe, active high |
| 26 | DDIS | output | high when the CPU is not reading |
| 27 | CSOUT | output | high when selected |
| 28 | ADS | input | address strobe, active low |
| 29 | A2 | input | |
| 30 | A1 | input | |
| 31 | A0 | input | |
| 32 | NC | nc | |
| 33 | INTRPT | output | active high, totem-pole (no 3-state described) |
| 34 | NC | nc | |
| 35 | OUT2 | output | active low |
| 36 | RTS | output | active low |
| 37 | DTR | output | active low |
| 38 | OUT1 | output | active low |
| 39 | MR | input | master reset, active high |
| 40 | CTS | input | active low |
| 41 | DSR | input | active low |
| 42 | DCD | input | active low |
| 43 | RI | input | active low |
| 44 | VCC | power | |

There is no exposed pad, and no pin has a mode-selected function.

For reference only (not in census), the dip40 N package from p1 is: 1–8 D0–D7, 9 RCLK, 10 SIN,
11 SOUT, 12 CS0, 13 CS1, 14 CS2, 15 BAUDOUT, 16 XTAL1, 17 XTAL2, 18 DOSTRBAR, 19 DOSTR, 20 VSS,
21 DISTRBAR, 22 DISTR, 23 DDIS, 24 CSOUT, 25 ADS, 26 A2, 27 A1, 28 A0, 29 NC, 30 INTRPT, 31 OUT2,
32 RTS, 33 DTR, 34 OUT1, 35 MR, 36 CTS, 37 DSR, 38 DCD, 39 RI, 40 VCC.

### Host interface (p4, pp6–7, Figs 2–3 pp9–10)
- **Bus:** 8250/16450 style. Data is D7–D0. The register address is A2–A0. There are three chip selects:
  CS0 is high, CS1 is high and CS2 is low. The chip is selected only when all three are true (p4).
- **Read strobes:** there are two read strobes, DISTR (active high) and DISTRBAR (active low).
  A read happens while the chip is selected and either strobe is active. "Only one of these inputs is
  required…the other input should be tied in its inactive state" (p4).
- **Write strobes:** DOSTR and DOSTRBAR work the same way for writes (p4). The write data is latched
  at the trailing edge of the strobe: tsu3 and th5 are measured to WR1↑ / WR2↓, i.e. the end of the
  strobe (p6, Fig 2).
- **ADS address strobe:** while ADS is low, A2–A0 and CS0/CS1/CS2 pass straight through
  ("drive the internal select logic directly"). When ADS goes high, they are latched on that
  rising edge and held while ADS stays high (p4). ADS may be tied low for non-multiplexed buses.
  The † timing parameters apply only with ADS tied low (pp6, 9–10).
- **CSOUT:** high while selected (after the ADS latch, if used). It follows the select logic
  after td3 (p4, p7).
- **DDIS:** high whenever the CPU is not reading the chip, and low during a selected read. Its
  timing is tdis(R) (p4, p7, Fig 3). DDIS is meant to disable an external bus transceiver
  (Fig 8, p13).
- **Data-bus 3-state:** D7–D0 drive only during a selected read. They float td11 after the
  strobe ends (p7).
- **INTRPT:** active high. There is no OUT2 gating and no 3-state on this part: the datasheet does
  not mention either. INTRPT is "reset…when the interrupt is serviced or as a result of a master
  reset" (p4).

### Registers (Table 1 p15, Table 3 p16, bit descriptions pp17–21)
DLAB is LCR[7]. Bit 0 is the LSB, and it is the first bit sent and received (p16).

| DLAB | A2 A1 A0 | Name | R/W | Reset (MR) |
|---|---|---|---|---|
| 0 | 000 | RBR (receiver buffer) | R | no effect (p15) |
| 0 | 000 | THR (transmit holding) | W | no effect |
| 0 | 001 | IER | R/W | 0x00 |
| X | 010 | IIR | R | 0x01 |
| X | 011 | LCR | R/W | 0x00 |
| X | 100 | MCR | R/W | 0x00 |
| X | 101 | LSR | R (writing is "not recommended outside of a factory testing environment", p19) | 0x60 |
| X | 110 | MSR | R | bits 3–0 = 0; bits 7–4 = complemented inputs |
| X | 111 | SCR (scratch) | R/W | no effect |
| 1 | 000 | DLL (divisor LSB) | R/W | no effect |
| 1 | 001 | DLM (divisor MSB) | R/W | no effect |

Writing to address 010 is not described: there is no FCR on this part. Treat that write as ignored.

**Register bits**

- **IER (p17):**
  - bit 0 ERBF: received-data-available interrupt enable.
  - bit 1 ETBE: THRE interrupt enable.
  - bit 2 ELSI: receiver line status interrupt enable.
  - bit 3 EDSSI: modem status interrupt enable.
  - bits 7–4: always 0.
- **IIR (pp17–18):**
  - bit 0: 0 means an interrupt is pending, 1 means none.
  - bits 2–1: interrupt ID (see Interrupts below).
  - bits 7–3: always 0.
- **LCR (pp18–19):**
  - bits 1–0 WLS: 00 = 5, 01 = 6, 10 = 7, 11 = 8 data bits.
  - bit 2 STB: 0 = 1 stop bit; 1 = 1.5 stop bits when 5-bit words are selected, otherwise 2.
    The receiver checks only the first stop bit.
  - bit 3 PEN: parity enable. The parity bit goes between the last data bit and the first stop bit.
  - bit 4 EPS: 1 = even parity, 0 = odd.
  - bit 5 stick parity: with PEN = 1, EPS = 1 and stick = 1, the parity bit is sent and checked as 0.
    With PEN = 1, EPS = 0 and stick = 1, it is sent and checked as 1.
  - bit 6 Set Break: forces SOUT to the spacing state (0). It "has no affect on the transmitter
    logic, it only affects the serial output."
  - bit 7 DLAB.
- **LSR (pp19–20):**
  - bit 0 DR: set when a complete character has moved into the RBR; cleared by reading the RBR.
  - bit 1 OE: set when the RBR is overwritten by the next character before the CPU read it.
  - bit 2 PE: parity error.
  - bit 3 FE: framing error. The stop bit was not 1.
  - bit 4 BI: break. SIN was held 0 for longer than a full-word time (start + data + parity + stop).
  - Bits 1–4 are cleared by reading the LSR, and they are the receiver line status interrupt sources.
  - bit 5 THRE: set when the THR contents move to the TSR; cleared when the CPU loads the THR.
  - bit 6 TEMT: set when the THR and the TSR are both empty.
  - bit 7: always 0.
- **MCR (p20):**
  - bit 0 DTR, bit 1 RTS, bit 2 OUT1, bit 3 OUT2: writing 1 drives the pin low (active);
    writing 0 drives it high.
  - bit 4 LOOP.
  - bits 7–5: always 0.
- **MSR (p21):**
  - bit 0 DCTS: CTS changed since the last MSR read.
  - bit 1 DDSR: DSR changed since the last MSR read.
  - bit 2 TERI: RI input went low→high. In bit terms, the RI pin went from active (0) to inactive (1),
    so the MSR[6] bit went 1→0.
  - bit 3 DDCD: DCD changed since the last MSR read.
  - bit 4 CTS: complement of the CTS pin.
  - bit 5 DSR: complement of the DSR pin.
  - bit 6 RI: complement of the RI pin.
  - bit 7 DCD: complement of the DCD pin.
  - Bits 3–0 are all cleared by an MSR read.
- **SCR:** an 8-bit scratch register with no other effect (p24).
- **DLL / DLM:** the 16-bit divisor, DLM:DLL. Writing either one also loads the 16-bit baud counter
  (p21).

### Behavior
- **Baud generator (p21, Fig 1 p8):**
  - The clock source is XTAL1, either a crystal on XTAL1/XTAL2 or an external clock into XTAL1
    (Fig 10, p23). It runs from dc to 9 MHz (p21; tw1/tw2 at f = 9 MHz max, p7).
  - divisor = fXTAL1 / (baud × 16), with a divisor range of 1 to 65535. The output is 16× the baud rate.
  - The divisor latches must be loaded during initialization (p21). What a divisor of 0 does is not
    stated.
  - BAUDOUT (active-low name) is the transmitter's 16× clock, fXTAL1 / N. Its waveform
    (Fig 1, p8) depends on N:
    - N = 1: BAUDOUT follows XTAL1.
    - N = 2: low for 1 XTAL1 cycle, high for 1.
    - N = 3: low for 2, high for 1.
    - N > 3: low for 2 XTAL1 cycles, then high for N−2.
    - Transitions follow XTAL1↑ after td1 / td2.
  - The receiver is clocked by RCLK, 16× the baud rate. For the same rate both ways, tie BAUDOUT to
    RCLK (p4). RCLK is good to 9 MHz (Fig 1).
  - The feature list claims "dc to 256 Kbit/s" (p1).
  - Example divisors: 1.8432 MHz gives 50 → 2304 … 9600 → 12, 38400 → 3, 56000 → 2 (Table 7).
    3.072 MHz gives 9600 → 20, 38400 → 5 (Table 8, p22).
- **Transmitter (pp19–20, 24):**
  - Double-buffered: THR → TSR. The frame is start (0), 5–8 data bits LSB first, optional parity,
    then 1 / 1.5 / 2 stop bits (1). SOUT idles at mark (1). MR sets SOUT high.
  - When the TSR is idle, the THR moves into it, THRE sets and TEMT stays 0. TEMT sets when both
    are empty.
  - Timing from Fig 5 (p11):
    - A write to the THR with the TSR idle starts the start bit after td15 = 8–24 BAUDOUT cycles,
      counted from the INTRPT clear.
    - THRE/INTRPT sets td16 = 8 BAUDOUT cycles after the start bit begins (the THR moved to the TSR).
    - td18 = 16–32 BAUDOUT cycles: "initial write to interrupt (THRE)".
  - Break: LCR[6] forces SOUT to 0, and the transmitter keeps running underneath (p19).
- **Receiver (p24, Fig 4 p11):**
  - Runs on the 16× RCLK. The sample clock is RCLK ÷ 16, aligned so data is sampled mid-bit
    (Fig 4 shows "8 CLKs" from the start edge to the first sample).
  - False-start-bit detection is a listed feature (p1): a start bit that is not still low at
    mid-bit is rejected. The datasheet gives no further detail. Treat it as standard 16450
    behavior: the start bit is re-checked at count 8 of 16.
  - Only the first stop bit is checked (p18).
  - When a character is complete it goes into the RBR and DR sets. If DR was already 1 (the RBR was
    unread), the old character is overwritten and OE sets (p19).
  - PE, FE and BI are latched with that character. LSR status sets td13 = 1 RCLK cycle after the
    stop-bit sample (p7).
  - Break: when SIN is 0 for a whole frame, BI sets (FE also, per standard 16450; the datasheet
    only says BI). The datasheet gives no detail on how the break character is loaded or on
    re-arming. The usual behavior is that one zero character is loaded into the RBR and the next
    character waits for SIN to return to mark. **Not stated; flag.**
- **FIFOs:** none. There are no RXRDY/TXRDY pins and no DMA modes.
- **Interrupts (Table 4, p18):**

  | IIR[2:0] | Priority | Type | Source | Cleared by |
  |---|---|---|---|---|
  | 001 | – | none | – | – |
  | 110 | 1 (highest) | Receiver line status | OE, PE, FE or BI | reading the LSR |
  | 100 | 2 | Received data available | data in the RBR | reading the RBR |
  | 010 | 3 | THR empty | THRE | reading the IIR (when THRE is the source) or writing the THR |
  | 000 | 4 (lowest) | Modem status | DCTS, DDSR, TERI, DDCD | reading the MSR |

  - INTRPT is high when any enabled source is pending. It is cleared after the clear action:
    td14 (140 ns) for an LSR/RBR read, td17/td19 (140 ns) for a THR write or IIR read, and
    td22 (140 ns) for an MSR read.
  - A modem input change sets INTRPT after td21 ≤ 170 ns (pp7–8).
  - The p17 priority list mentions "receiver character time out". This TL16C450 has no FIFO and no
    timeout code (Table 4 has no 1100), so it is a copy-paste leftover.
- **Modem lines (pp4–5, 20–21):**
  - The DTR, RTS, OUT1 and OUT2 pins are the complements of MCR[0..3]. They change td20 ≤ 100 ns
    after the MCR write.
  - The inputs CTS, DSR, RI and DCD read back complemented in MSR[4..7], and their delta bits
    are in MSR[0..3].
  - There is no auto flow control.
- **Loopback (MCR[4], p20):**
  - SOUT is forced high (mark) and SIN is disconnected. The TSR output is wired internally to the
    RSR input.
  - CTS, DSR, DCD and RI are disconnected. Inside the chip, RTS feeds CTS (MSR[4] = MCR[1]),
    DTR feeds DSR (MSR[5] = MCR[0]), OUT1 feeds RI (MSR[6] = MCR[2]) and OUT2 feeds DCD
    (MSR[7] = MCR[3]).
  - The four modem output pins go to their inactive state (high).
  - Transmit, receive and modem interrupts all still work. The modem interrupt sources become
    MCR[3:0], and everything is still gated by the IER.
- **Extras:**
  - The scratch register.
  - CSOUT and DDIS.
  - There is no sleep mode, IrDA, device ID register or software reset.
- **Reset (MR high, Table 2 p15):**

  | Register / signal | Value after MR |
  |---|---|
  | IER | 0x00 |
  | IIR | 0x01 |
  | LCR | 0x00 (p15 lists "All bits low" with no reset control named; treat it as master reset) |
  | MCR | 0x00 |
  | LSR | 0x60 (THRE = TEMT = 1) |
  | MSR | bits 3–0 = 0; bits 7–4 follow the inputs |
  | SOUT | high |
  | INTRPT | low |
  | OUT1, OUT2, RTS, DTR | high (inactive) |
  | SCR, DLL, DLM, RBR, THR | unchanged |

  - MR pulse width is ≥ 1000 ns (twMR, p6).
  - During MR, DDIS and CSOUT are not specified separately. The data bus is not driven unless a
    read is in progress.
- **Undefined / reserved:** writing to the LSR (p19). Writing to IIR address 010 is not described.
  A divisor of 0 is not described.

### Timing (VCC = 4.75–5.25 V, TA = 0–70 °C; all figures are min/max limits, none typical)
| Symbol | Parameter | Value | Page |
|---|---|---|---|
| tcR / tcW | read / write cycle time | ≥ 175 ns | p6 |
| tw5 | ADS low pulse width | ≥ 15 ns | p6 |
| tw6 | write strobe pulse width | ≥ 80 ns | p6 |
| tw7 | read strobe pulse width | ≥ 80 ns | p6 |
| twMR | MR pulse width | ≥ 1000 ns | p6 |
| tsu1 / tsu2 | address / CS setup before ADS↑ | ≥ 15 ns | p6 |
| th1 / th2 | address / CS hold after ADS↑ | ≥ 0 ns | p6 |
| tsu3 | data setup before the strobe trailing edge (WR1↑ or WR2↓) | ≥ 15 ns | p6 |
| th5 | data hold after the strobe trailing edge | ≥ 15 ns | p6 |
| th3 / th4 | CS / address hold after the write trailing edge | ≥ 20 ns | p6 |
| th6 / th7 | CS / address hold after the read trailing edge | ≥ 20 ns | p6 |
| td4 / td5 | CS / address valid before the write strobe leading edge (ADS low) | ≥ 15 ns | p6 |
| td7 / td8 | CS / address valid before the read strobe leading edge (ADS low) | ≥ 15 ns | p6 |
| td6 / td9 | strobe end to next ADS↓ | ≥ 80 ns | p6 |
| tw1 / tw2 | XTAL1 / RCLK high / low (f ≤ 9 MHz) | ≥ 50 ns | p7 |
| td3 | select to CSOUT | ≤ 70 ns | p7 |
| td10 | read strobe active to data valid | ≤ 60 ns | p7 |
| td11 | read strobe inactive to data float | 0–60 ns | p7 |
| tdis(R) | read strobe to DDIS | ≤ 60 ns | p7 |
| tw3 / tw4 | BAUDOUT low / high (6.25 MHz, ÷1) | ≥ 80 ns | p7 |
| td1 / td2 | XIN↑ to BAUDOUT↑ / BAUDOUT↓ | ≤ 125 ns | p7 |
| td12 | RCLK to sample clock | ≤ 100 ns | p7 |
| td13 | stop sample to LSI / RDR interrupt | 1 RCLK cycle | p7 |
| td14 | read RBR / LSR to interrupt reset | ≤ 140 ns | p7 |
| td15 | INTRPT (THRE) to transmit start | 8–24 BAUDOUT cycles | p7 |
| td16 | start to THRE interrupt | 8 BAUDOUT cycles | p7 |
| td17 | write THR to interrupt reset | ≤ 140 ns | p7 |
| td18 | initial write to THRE interrupt | 16–32 BAUDOUT cycles | p7 |
| td19 | read IIR to THRE interrupt reset | ≤ 140 ns | p7 |
| td20 | write MCR to output | ≤ 100 ns | p8 |
| td21 | modem input to INTRPT | ≤ 170 ns | p8 |
| td22 | read MSR to interrupt reset | ≤ 140 ns | p8 |

### VCC
- Recommended VCC is 4.75–5.25 V, nominal 5 V, at 0–70 °C (p5).
- Absolute maximum input voltage is −0.5 to 7 V (p5). Since VIH max is listed as VCC, the inputs
  are not specified as tolerant above VCC: **inputMax = VCC**.

### Package drawing
- **TI FN0044A** (PLCC, 4.57 mm max height, JEDEC MS-018, document 4215154/A):
  - Outline on p27; example land pattern on p28; stencil on p29.
  - Body is 16.51–16.66 mm square (.650–.656 in); span across leads is 17.40–17.65 mm (.685–.695 in).
  - Pitch is 1.27 mm (.050 in). Lead width (upper) is 0.66–0.81 mm and the contact width is
    0.33–0.53 mm.
  - The lead row-to-row dimension (seating, J-bend centers) is 14.79–16.20 mm (.582–.638 in).
    Height is ≤ 4.57 mm.
  - Land pattern: 44 pads of 2.35 × 0.75 mm, at 1.27 mm pitch, on a 16.2 mm pad-row center spacing
    in both X and Y.
  - Pin 1 is at the top center; pins run counter-clockwise viewed from the top. Pins 1–6 are on the
    top edge, 7–17 on the left, 18–28 on the bottom, 29–39 on the right and 40–44 on the top.

### Obsolete
None. TL16C450FN is Active.

### Notes
- The p17 IIR text mentions "receiver character time out" and the p1 heading says "16× clock", both
  left over from the 16550 text. There is no FIFO or timeout on this part.
- Typo on p6: th1 is described as "Hold time, address low after ADS↑". It means address valid.
- p15 Table 2 gives no reset-control entry for the LCR, but its reset state is "All bits low".
- The INTRPT output has no OUT2 gating or 3-state. This differs from PC-style 16550 use, where
  OUT2 gates the interrupt externally on the board.
- The TERI wording "RI input… changed from a low to a high state" (p21) refers to the pin level. RI
  is active low, so this is the trailing edge of a ring, i.e. the end of the ring pulse.
- The datasheet gives no text on break / false-start handling beyond what is above.

## TL16C451

- Title (as printed): "TL16C451, TL16C452 ASYNCHRONOUS COMMUNICATIONS ELEMENTS", TI SLLS053C (May 1989, rev. Aug 1999). Function: one TL16C450-class ACE (serial channel 0, no FIFO) plus a Centronix/IBM-PC-compatible parallel printer port (p.1, p.21).
- Datasheet: `datasheets/controller-uart/tl16c451.pdf` (26 pages: pp.1–22 data sheet, p.23–24 Package Option Addendum, p.25 Pack Materials, p.26 notice). Pinout p.2; terminal functions pp.5–6; ratings/ROC p.7; timing pp.7–9; waveforms pp.9–12; registers pp.14–22.
- Difference from TL16C452: the 451 has only serial channel 0. The channel-1 pins of the 452 are, on the 451: CS1 pin 3 → VCC; DSR1 5, RI1 6, RLSD1 8, CTS1 13, SIN1 62 → GND; SOUT1 10, DTR1 11, RTS1 12, INT1 60 → NC (p.5–6 "names shown in brackets are for the TL16C451"; p.2 drawing). Everything else (channel 0, parallel port, pins, timing) is identical. Each serial channel is a TL16C450 (no FIFO, no FCR).

### Parts
| Census part | DigiKey status | Package | Pins | Pinout name | Grade | VCC |
|---|---|---|---|---|---|---|
| TL16C451FN  | Obsolete (TI addendum p.23 still says Active) | PLCC, TI "FN", tube of 18 | 68 | plcc68 | commercial 0–70 °C | 4.75–5.25 V |
| TL16C451FNR | Active | PLCC FN, small T&R 250 | 68 | plcc68 | 0–70 °C | 4.75–5.25 V |
Decode (addendum p.23): `TL16C451` + `FN` (68-pin PLCC) + `R` (tape & reel). Only one grade exists (0–70 °C, 5 V). Same die/marking "TL16C451FN" for both.

### Pinout — plcc68 (FN, top view, p.2; roles from p.5–6)
| Pin | Name | Role |
|---|---|---|
| 1 | LPTOE | input (active low) |
| 2 | GND | ground |
| 3 | VCC | power (CS1 on the 452) |
| 4 | CLK | input (terminal table says I/O; it is the only clock input, no XTAL2) |
| 5 | GND | ground |
| 6 | GND | ground |
| 7 | GND | ground |
| 8 | GND | ground |
| 9 | GND | ground |
| 10 | NC | nc |
| 11 | NC | nc |
| 12 | NC | nc |
| 13 | GND | ground |
| 14–21 | DB0–DB7 | inout (DB0 = pin 14 … DB7 = pin 21) |
| 22 | GND | ground |
| 23 | VCC | power |
| 24 | RTS0 | output (active low) |
| 25 | DTR0 | output (active low) |
| 26 | SOUT0 | output (terminal table p.6 wrongly says "I") |
| 27 | GND | ground |
| 28 | CTS0 | input (active low) |
| 29 | RLSD0 | input (active low) |
| 30 | RI0 | input (active low) |
| 31 | DSR0 | input (active low) |
| 32 | CS0 | input (active low) |
| 33 | A2 | input |
| 34 | A1 | input |
| 35 | A0 | input |
| 36 | IOW | input (active low) |
| 37 | IOR | input (active low) |
| 38 | CS2 | input (active low) |
| 39 | RESET | input (active low) |
| 40 | VCC | power |
| 41 | SIN0 | input |
| 42 | GND | ground |
| 43 | GND | ground |
| 44 | BDO | output |
| 45 | INT0 | output (active high, 3-state) |
| 46 | PD7 | inout |
| 47 | PD6 | inout |
| 48 | PD5 | inout |
| 49 | PD4 | inout |
| 50 | PD3 | inout |
| 51 | PD2 | inout |
| 52 | PD1 | inout |
| 53 | PD0 | inout |
| 54 | GND | ground |
| 55 | STB | inout (open drain + internal pull-up; active low) |
| 56 | AFD | inout (open drain + pull-up; active low) |
| 57 | INIT | inout (open drain + pull-up; active low) |
| 58 | SLIN | inout (open drain + pull-up; active low) |
| 59 | INT2 | output (active high, 3-state) |
| 60 | NC | nc (INT1 on the 452) |
| 61 | GND | ground |
| 62 | GND | ground (SIN1 on the 452) |
| 63 | ERROR | input (active low) |
| 64 | VCC | power |
| 65 | SLCT | input (active high) |
| 66 | BUSY | input (active high) |
| 67 | PE | input (active high) |
| 68 | ACK | input (active low) |
No exposed pad. VCC pins 3*, 23, 40, 64 (*451 only); GND 2,5,6,7,8,9,13,22,27,42,43,54,61,62 on the 451 (p.6 lists the 452's GND set 2,7,9,22,27,42,43,54,61). No pin is mode-selected. No BAUDOUT, RCLK, XTAL2, OUT1, OUT2, DDIS, TXRDY/RXRDY, CS-combination or ADS pins.

### Host interface (p.5–6, p.10, p.14, p.21)
- Intel-style: IOR (low = read), IOW (low = write), A0–A2, DB0–DB7 3-state. No ALE/ADS (addresses not latched; must be stable through the strobe: tsu1/th1).
- Chip selects, each active low and each selecting one block: CS0 → serial channel 0 (8 registers, A2..A0); CS2 → parallel port (registers selected by A1,A0 only; A2 not listed in Table 8 p.21 — treat as don't care). CS1 does not exist on the 451 (pin tied to VCC). Behaviour with CS0 and CS2 both low is not specified.
- Write: data latched on IOW rising edge (tsu3 data setup before IOW↑, th3 hold after IOW↑; address/CS setup to IOW↓, hold after IOW↑; Fig 2 p.10).
- Read: DB driven while IOR low and CS low; data valid td5 after IOR↓ (text says "before read IOR↑" but Fig 3 measures from IOR↓), floats td6 after IOR↑ (Fig 3 p.10).
- BDO ("bus buffer output", pin 44): terminal table p.5 says "active (high) when the CPU is reading data… can disable an external transceiver", but Fig 3 (p.10) draws BDO high at idle, going LOW tdis(R) after IOR↓ and back high tdis(R) after IOR↑, and the parameter is named "IOR↓ to BD0↓" (p.8). Conflict — the waveform/parameter say BDO is low during a read of the selected chip. Whether BDO responds to reads of an unselected device is not stated (presumably only when a CSx is low).
- Data bus is 3-state when not selected or in write mode (Ioz conditions p.7).

### Registers — serial channel 0 (CS0 low), TL16C450 set (Table 1 p.14, Table 3 p.15, bit text pp.16–22)
DLAB = LCR[7].
| DLAB | A2 A1 A0 | Name | R/W | Reset |
|---|---|---|---|---|
| 0 | 000 | RBR receiver buffer | R | unaffected by reset (Table 2) |
| 0 | 000 | THR transmitter holding | W | unaffected |
| 0 | 001 | IER interrupt enable | R/W | 0x00 |
| x | 010 | IIR interrupt identification | R only (writes ignored — no FCR) | 0x01 |
| x | 011 | LCR line control | R/W | 0x00 |
| x | 100 | MCR modem control | R/W | 0x00 |
| x | 101 | LSR line status | R (writing "not recommended outside factory test", p.19) | 0x60 |
| x | 110 | MSR modem status | R | bits 3–0 = 0, bits 7–4 = complement of CTS/DSR/RI/RLSD inputs |
| x | 111 | SCR scratch | R/W | unaffected |
| 1 | 000 | DLL divisor LSB | R/W | unaffected |
| 1 | 001 | DLM divisor MSB | R/W | unaffected |
Note Table 1 marks addresses 010–111 as DLAB = X, so they are reachable with DLAB = 1.

Bits:
- RBR/THR: data bits 0–7; bit 0 = LSB = first bit sent/received. For 5–7-bit words the unused upper RBR bits are not specified (p.15 doesn't say; 16450 convention is 0).
- IER: b0 ERBFI received-data-available int enable; b1 ETBEI THRE int enable; b2 ELSI receiver line status int enable; b3 EDSSI modem status int enable; b7–4 always 0 (p.16).
- IIR: b0 0 = interrupt pending, 1 = none; b2–1 interrupt ID (Table 4 below); b7–3 always 0 (p.17). Reset: b0=1, b1,b2 = 0.
- LCR: b1–0 WLS word length 00=5, 01=6, 10=7, 11=8; b2 STB stop bits: 0 → 1 stop; 1 → 1.5 stop if 5-bit, 2 stop for 6/7/8-bit (Table 6); b3 PEN parity enable (parity bit between last data bit and first stop; checked on receive); b4 EPS 1 = even, 0 = odd; b5 stick parity: PEN=1,EPS=1,stick=1 → parity bit sent and checked as 0; PEN=1,EPS=0,stick=1 → sent and checked as 1; b6 set break: forces SOUT to spacing (0); does not affect transmitter logic, only the output; b7 DLAB (p.18).
- LSR: b0 DR — set when a complete character transferred into RBR, cleared by reading RBR; b1 OE — RBR overwritten by next character before being read, cleared by reading LSR; b2 PE — parity mismatch, cleared by reading LSR; b3 FE — stop bit not 1 (invalid), cleared by reading LSR; b4 BI — SIN held 0 for longer than a full-word time (start + data + parity + stop), cleared by reading LSR; b5 THRE — set when THR contents transferred to the TSR, cleared when the CPU loads THR; b6 TEMT — set when THR and TSR both empty, cleared when either holds a character; b7 always 0. Bits 1–4 are the receiver-line-status interrupt sources (p.19).
- MCR: b0 DTR (1 → DTR pin low); b1 RTS (1 → RTS pin low); b2 OUT1 — "reserved location used only in the loopback mode" (no pin); b3 OUT2 — interrupt output enable: 1 enables INTx, 0 disables (pin text p.5: INTx is a 3-state output enabled by MCR bit 3, i.e. Hi-Z when OUT2 = 0); b4 LOOP; b7–5 always 0 (pp.19–20).
- MSR: b0 DCTS (CTS changed since last MSR read); b1 DDSR; b2 TERI (RI input went low→high, i.e. ring ended — pin text p.6 says "transitioned from a low to a high state"); b3 DRLSD; b4 CTS = complement of CTS pin (loop: = MCR b1 RTS); b5 DSR = complement of DSR pin (loop: = MCR b0 DTR); b6 RI = complement of RI pin (loop: = MCR b2 OUT1); b7 RLSD = complement of RLSD pin (loop: = MCR b3 OUT2). b3–0 all cleared by reading MSR (p.20).
- SCR: 8-bit scratchpad, no effect on operation (p.22).
- DLL/DLM: divisor bits 0–7 / 8–15 (p.15).

### Registers — parallel port (CS2 low; Tables 7 and 8, p.21)
| IOR | IOW | A1 A0 | Register |
|---|---|---|---|
| L | H | 00 | Read data: b7..b0 = PD7..PD0 |
| L | H | 01 | Read status: b7 BUSY, b6 ACK, b5 PE, b4 SLCT, b3 ERROR, b2–0 = 1 |
| L | H | 10 | Read control: b7–5 = 1, b4 IRQ ENB, b3 SLIN, b2 INIT, b1 AFD, b0 STB |
| L | H | 11 | Invalid |
| H | L | 00 | Write data: PD7..PD0 latch |
| H | L | 01 | Invalid |
| H | L | 10 | Write control: b4 IRQ ENB, b3 SLIN, b2 INIT, b1 AFD, b0 STB (b7–5 shown as "1") |
| H | L | 11 | Invalid |
- Read data "allows the microprocessor to read the information on the parallel bus" (the PD pins). Read status reads printer status "in the five most significant bits". Read control "allows the state of the control lines to be read". Write control sets the control lines; "These signals are cleared when a reset occurs" (p.21) — i.e. write-control register = 0 after reset (so IRQ ENB = 0, INT2 3-stated). Write-data register reset state not stated.
- Polarity of each status/control bit relative to its pin is NOT stated anywhere in this datasheet. The datasheet only says the port is "completely compatible with the parallel port implementation used in the IBM serial/parallel adaptor" (p.21). (IBM adapter convention, for the planner's reference, not from this PDF: status b7 = inverted BUSY pin, b6–b3 = ACK, PE, SLCT, ERROR pin levels; control b0 STB, b1 AFD, b3 SLIN are inverted to the pin (1 → pin low), b2 INIT is not inverted (0 → pin low), read-control returns the pin levels through the same inversion.) Flag: needs a decision.
- Direction: PD0–PD7 are driven from the write data register when LPTOE pin is low, Hi-Z when LPTOE is high (so they can be inputs) (p.5, p.6). LPTOE is a pin, not a register bit. td20: PD valid ≤60 ns after LPTOE↓.
- STB/AFD/INIT/SLIN: open-drain with internal pull-up (≈2 mA internal IOL, sink ≥10 mA, Note 2 p.7). Model as pull-low / release (weak-high).
- INT2: "active-high 3-state output generated by the positive transition of ACK. It is enabled by bit 4 of the write control register" (p.5); ACK "generates a printer port interrupt during its positive transition" (p.5). But Fig 7 (p.12) shows INT2 simply following ACK: ACK↓ → INT2↓ (td21), ACK↑ → INT2↑ (td21), i.e. INT2 = ACK level (high while ACK idle high). No latched interrupt flag or clear mechanism is described. Treat INT2 = ACK pin level when IRQ ENB = 1, Hi-Z when IRQ ENB = 0 (consistent with Fig 7 and with the IBM adapter); flag.
- Handshake: software-driven only — the CPU writes data, pulses STB via the write-control register; the printer answers with ACK low pulse; BUSY high = not ready. There is no automatic strobe generation (none described).

### Behavior — serial channel
- Baud generator (p.22, Fig 1 p.9): single clock input CLK, dc to 9 MHz (tw1/tw2 ≥ 50 ns each, p.8). Divisor 1 … 65535 from DLM:DLL; output = 16× baud; `divisor = fCLK / (baud × 16)`. Loading either divisor latch also loads the 16-bit baud counter ("to prevent long counts on initial load"). Divisor 0 behaviour not specified. Internal BAUDOUT (active-low drawing, Fig 1): ÷1 = CLK; ÷N (N>3) low for 2 CLK cycles then high N−2 cycles. RCLK is internal and equals BAUDOUT (Fig 4 note p.11); no external RCLK/BAUDOUT pins. Max data rate quoted "dc to 256 kbit/s" (p.1).
- Receiver (p.22, Fig 4 p.11): 16× clock; sample clock occurs 8 RCLK cycles into each bit (mid-bit). Start-bit validation/false-start rejection is not described. Frame: start, 5–8 data LSB first, optional parity, stop. Only the first stop bit is checked (FE = stop bit not 1). Character transferred to RBR at the stop-bit sample; DR (and the RDA interrupt) set td8 = 1 RCLK cycle after the stop sample. Overrun: new character overwrites RBR, OE set. Break: BI set when SIN is 0 for a full word time; the datasheet doesn't say whether a 0 character is loaded or how the receiver re-syncs (16450 convention: one 0 character loaded, waits for SIN to return to 1 — not stated here).
- Transmitter (p.22, Fig 5 p.11): THR → TSR when TSR idle; THRE set on transfer. td10: SOUT start bit begins 8–24 baudout cycles after the initial THR write (IOW↑). THRE interrupt re-asserts td13 = 16–32 baudout cycles after an initial write (i.e. after the THR→TSR transfer), and td11 = 8 baudout cycles after the start of the stop bit ("stop (SOUT) low to interrupt" — Fig 5 shows it in the stop bit before the next start). 1/1.5/2 stop bits per LCR. Break forces SOUT = 0 regardless of the shifter.
- Interrupts (Table 4 p.17, Fig 10 p.16):
  | IIR b2 b1 b0 | Prio | Type | Source | Cleared by |
  |---|---|---|---|---|
  | 0 0 1 | – | none | – | – |
  | 1 1 0 | 1 (highest) | receiver line status | OE, PE, FE, BI | reading LSR |
  | 1 0 0 | 2 | received data available | DR | reading RBR |
  | 0 1 0 | 3 | THR empty | THRE | reading IIR (if THRE is the source) or writing THR |
  | 0 0 0 | 4 | modem status | DCTS, DDSR, TERI, DRLSD | reading MSR |
  (p.17 text lists priority 2 as "receiver data ready or receiver character time out" — leftover 16550 wording; this part has no FIFO/timeout.) INTx = OR of enabled sources (each gated by its IER bit) gated by MCR[3] (OUT2); INTx active high; Hi-Z when OUT2 = 0 (p.5). Reset → INTx low (p.5, Table 2).
- Modem lines: CTS, DSR, RLSD, RI inputs active low; MSR b4–7 = complements. RTS/DTR outputs low when MCR bit = 1; high after reset.
- Loopback (MCR[4], p.20): SOUT forced high (marking); SIN disconnected; TSR output looped to RSR input; CTS/DSR/RLSD/RI pins disconnected; MCR DTR→DSR, RTS→CTS, OUT1→RI, OUT2→RLSD; RTS/DTR pins forced inactive (high). Rx/Tx interrupts operational; modem-status interrupts come from MCR bits 0–3; all interrupts still controlled by IER. Effect of loop on INTx enable (OUT2) not stated.
- Reset (RESET pin low, ≥1000 ns twRST p.8; Table 2 p.14): IER = 0; IIR = 0x01; LCR = 0; MCR = 0; LSR = 0x60; MSR b0–3 = 0 (b4–7 follow inputs); SOUT high; all INTRPT sources low; OUT2, RTS, DTR, OUT1 internal signals high (i.e. inactive); SCR, DLL/DLM, RBR, THR unaffected. Parallel-port control signals cleared (p.21). States of PD (depend on LPTOE), INT2, BDO during reset not stated.
- Not present: FIFOs, FCR, DMA/TXRDY/RXRDY, timeout, auto flow, sleep, IrDA.

### Timing (VCC 4.75–5.25 V, 0–70 °C; all min/max as printed — no typicals)
System (p.7–8): tcR read cycle ≥175 ns; tcW write cycle ≥175 ns; tw1/tw2 CLK high/low ≥50 ns (9 MHz max, p.9); tw5 IOW pulse ≥80 ns; tw6 IOR pulse ≥80 ns; twRST ≥1000 ns; tsu1 A0–A2 before IOW↓/IOR↓ ≥15 ns; tsu2 CSx before IOW↓/IOR↓ ≥15 ns; tsu3 data before IOW↑ ≥15 ns; th1 address after IOW↑/IOR↑ ≥20 ns; th2 CS after strobe↑ ≥20 ns; th3 data after IOW↑ ≥15 ns (printed "before", typo); td3 IOW↑ to next IOW↓ ≥80 ns; td4 IOR↑ to next IOR↓ ≥80 ns.
Switching (p.8, CL = 100 pF): td5 IOR↓ to data valid ≤60 ns; td6 IOR↑ to data float 0–60 ns; tdis(R) IOR↓ to BDO↓ ≤60 ns; td7 RCLK↑ to sample clock ≤100 ns; td8 stop sample to INT↑ = 1 RCLK cycle; td9 read RBR/LSR IOR↑ to INT↓ ≤140 ns; td10 THR write to SOUT start 8–24 baudout cycles; td11 stop to THRE INT↑ 8 baudout cycles; td12 IOW↓ (write THR) to INT↓ ≤140 ns; td13 initial write to THRE INT↑ 16–32 baudout cycles; td14 IIR read IOR↑ to INT↓ ≤140 ns; td15 write MCR IOW↑ to RTS/DTR ≤100 ns (p.8). p.9: td16 modem input change to INT↑ ≤170 ns; td17 read MSR IOR↑ to INT↓ ≤140 ns; td18 IOW↑ to SLIN/AFD/STB/INIT change ≤60 ns; td19 IOW↑ to PD0–PD7 change ≤60 ns; td20 LPTOE↓ to PD valid ≤60 ns; td21 ACK↓/↑ to INT2↓/↑ ≤100 ns.

### VCC
Recommended 4.75–5.25 V (nom 5), 0–70 °C (p.7). VIH 2 V – VCC, VIL −0.5 – 0.8 V (TTL levels). Inputs not specified above VCC (VIH max = VCC; abs max VI 7 V) → inputMax = VCC.

### Package drawing
The PDF contains no mechanical outline or land pattern (pp.23–26 are addendum, tube data, notice). Only: "PLCC (FN) | 68" (addendum p.23); tube SPQ 18, L 506.98 W 25.91 mm (p.25). DigiKey census: 68-PLCC 24.23×24.23 mm body. TI's standard 68-pin FN outline (from TI general knowledge, NOT in this PDF — verify against TI FN0068A/MPLC004 drawing): 1.27 mm pitch, 17 leads per side, body ≈24.13–24.33 mm square, J-lead span ≈25.02–25.27 mm. Footprint agent should fetch the TI FN (68) drawing.

### Obsolete
TL16C451FN (DigiKey Obsolete). TL16C451FNR Active.

### Notes / datasheet errors
- SOUT0 role printed "I" (p.6) — it is an output.
- BDO polarity conflict (text high-active vs waveform low during read) — see Host interface.
- INT2 described as edge-generated (p.5) but drawn as following ACK level (Fig 7 p.12).
- Parallel-port bit/pin polarities unspecified (IBM-compatible claim only).
- ICC test condition mentions "XTAL1 at 4 MHz" (p.7) — there is no XTAL1; it means CLK.
- td15 "(RTS, DTS)" = RTS, DTR. tw5 printed "IOW↑" — it's the IOW low pulse width (Fig 2).
- IIR priority-2 text mentions "character time out" though no FIFO exists.
- CLK listed as I/O; treat as input.

## TL16C452

- Title: same document as TL16C451 (SLLS053C), "TL16C452 Consists of Two TL16C450s Plus a Centronix-Type Printer Interface" (p.1). Datasheet `datasheets/controller-uart/tl16c451.pdf`, pinout p.3, block diagram p.4, all other pages shared.
- Difference from TL16C451: adds serial channel 1 (a second, independent TL16C450 ACE) selected by CS1 (pin 3), with its own SIN1, SOUT1, RTS1, DTR1, CTS1, DSR1, RLSD1, RI1, INT1. Both channels share CLK, A0–A2, DB0–DB7, IOR, IOW, RESET, BDO. Each channel has its own full register set (including its own divisor latches and scratch register) and its own MCR OUT2 gating of its INT pin. Parallel port identical.

### Parts
| Census part | DigiKey status | Package | Pins | Pinout name | Grade | VCC |
|---|---|---|---|---|---|---|
| TL16C452FNR | Active | PLCC TI "FN", small T&R 250 | 68 | plcc68 (452 assignment) | 0–70 °C | 4.75–5.25 V |
Decode (addendum p.23): `TL16C452` + `FN` (68 PLCC) + `R` (reel). (TL16C452FN tube and FNRG4 exist in the addendum but are not in the census.)

### Pinout — plcc68 (FN, top view, p.3)
Same as the TL16C451 table above except:
| Pin | Name | Role |
|---|---|---|
| 3 | CS1 | input (active low) |
| 5 | DSR1 | input (active low) |
| 6 | RI1 | input (active low) |
| 8 | RLSD1 | input (active low) |
| 10 | SOUT1 | output |
| 11 | DTR1 | output (active low) |
| 12 | RTS1 | output (active low) |
| 13 | CTS1 | input (active low) |
| 60 | INT1 | output (active high, 3-state) |
| 62 | SIN1 | input |
VCC = 23, 40, 64; GND = 2, 7, 9, 22, 27, 42, 43, 54, 61 (p.6, p.3). All other pins as TL16C451 (pins 1, 4, 14–59, 63, 65–68 identical). No exposed pad.

### Host interface, registers, behavior, timing, VCC
Identical to TL16C451 (sections above), with CS1 low selecting channel 1's register set (Table 1 p.14 applies per channel). Timing tables (pp.7–9) cover CS0, CS1, CS2 identically (tsu2/th2 "CSx"). Reset (Table 2) applies to both channels. Simultaneous selection of more than one CS not specified.

### Package drawing
Same FN 68-pin PLCC as TL16C451 (no outline in the PDF; see above).

### Obsolete
None (TL16C452FNR Active).

### Notes
- Terminal table p.5 lists INT1 with INT0 as "active-high 3-state output enabled by bit 3 of the MCR" — each channel's own MCR[3].
- BDO is shared: asserted on a read of any selected block (presumably CS0, CS1 or CS2); not explicitly stated.

## TL16C550B

- Title (p1): "TL16C550B, TL16C550BI ASYNCHRONOUS COMMUNICATIONS ELEMENT", SLLS136B, Jan 1994, rev. Aug 1996. It is a single-channel 16550A-class UART (ACE). It powers up in TL16C450-compatible "character mode"; FCR0=1 switches it to FIFO mode with a 16-byte TX FIFO and a 16-byte RX FIFO (plus 3 error bits per RX byte). RXRDY and TXRDY give DMA signalling. p1 says it is a "faster plug-in replacement for National Semiconductor NS16550A".
- Datasheet: `datasheets/controller-uart/tl16c550b.pdf`, 35 pages. PDF page numbers equal the printed page numbers. Pages used: pinouts p3, pin functions p5–6, ratings/DC p7, AC p8–9, waveforms p10–17, registers p20–31, mechanical p32–34.
- Variants: TL16C550B is the 0–70 °C grade. TL16C550BI is the −40–85 °C grade and comes only in FN (p2, p7). Both have the same function and supply range.
- How it differs from TL16C550C/D (inferred from the sibling PDFs' titles, "…with Autoflow Control"): TL16C550B has no auto-RTS/CTS. MCR bits 5–7 are permanently 0 (p28). It has no 3.3 V operation (VCC is 4.75–5.25 V only, p7). It has no sleep mode. Its register set is otherwise the classic 16550A set.

### Parts
This datasheet has no ordering-information table. I decoded the part numbers from the package letters on p2: N = 40-pin PDIP, FN = 44-pin PLCC, PT = 48-pin TQFP. The trailing R is TI's tape-and-reel suffix; that is TI convention and is not stated in this PDF. None of the census parts has the "I" suffix, so all three are the commercial 0–70 °C grade.

| Census part | Package (code) | Pins | Pinout name | Grade | VCC |
|---|---|---|---|---|---|
| TL16C550BFN (Obsolete) | 44-pin PLCC, TI FN (S-PQCC-J44) | 44 | plcc44 | commercial 0–70 °C | 4.75–5.25 V |
| TL16C550BN (Obsolete) | 40-pin PDIP, 600 mil, TI N (R-PDIP-T40) | 40 | dip40 | commercial 0–70 °C | 4.75–5.25 V |
| TL16C550BPTR (Active) | 48-pin TQFP/LQFP 7×7, TI PT (S-PQFP-G48), tape & reel | 48 | tqfp48 | commercial 0–70 °C | 4.75–5.25 V |

- The census lists VCC "3.3V, 5V" for the FN and N parts, and has no package or VCC for PTR. This PDF supports **5 V only** (p7), so the census VCC field is wrong for the B part.

### Pinouts (p3, cross-checked against the pin-function table p5–6)
Active levels come from the overbars on p3 and from the pin text. The pin list is identical across the three packages except for the pin numbers. The NC pins have no internal connection.

| Signal | N (dip40) | FN (plcc44) | PT (tqfp48) | Role | Active level |
|---|---|---|---|---|---|
| D0 | 1 | 2 | 43 | inout | — |
| D1 | 2 | 3 | 44 | inout | |
| D2 | 3 | 4 | 45 | inout | |
| D3 | 4 | 5 | 46 | inout | |
| D4 | 5 | 6 | 47 | inout | |
| D5 | 6 | 7 | 2 | inout | |
| D6 | 7 | 8 | 3 | inout | |
| D7 | 8 | 9 | 4 | inout | |
| RCLK | 9 | 10 | 5 | input | 16× receive clock |
| SIN | 10 | 11 | 7 | input | |
| SOUT | 11 | 13 | 8 | output | |
| CS0 | 12 | 14 | 9 | input | high |
| CS1 | 13 | 15 | 10 | input | high (see Notes: drawing error) |
| CS2 | 14 | 16 | 11 | input | low |
| BAUDOUT | 15 | 17 | 12 | output | drawn with overbar |
| XIN | 16 | 18 | 14 | input (crystal/clock) | |
| XOUT | 17 | 19 | 15 | output (crystal) | |
| WR1 | 18 | 20 | 16 | input | low |
| WR2 | 19 | 21 | 17 | input | high |
| VSS | 20 | 22 | 18 | ground | |
| RD1 | 21 | 24 | 19 | input | low |
| RD2 | 22 | 25 | 20 | input | high |
| DDIS | 23 | 26 | 22 | output | high = "not reading" |
| TXRDY | 24 | 27 | 23 | output | low |
| ADS | 25 | 28 | 24 | input | low |
| A2 | 26 | 29 | 26 | input | |
| A1 | 27 | 30 | 27 | input | |
| A0 | 28 | 31 | 28 | input | |
| RXRDY | 29 | 32 | 29 | output | low |
| INTRPT | 30 | 33 | 30 | output | high |
| OUT2 | 31 | 35 | 31 | output | low |
| RTS | 32 | 36 | 32 | output | low |
| DTR | 33 | 37 | 33 | output | low |
| OUT1 | 34 | 38 | 34 | output | low |
| MR | 35 | 39 | 35 | input | high |
| CTS | 36 | 40 | 38 | input | low |
| DSR | 37 | 41 | 39 | input | low |
| DCD | 38 | 42 | 40 | input | low |
| RI | 39 | 43 | 41 | input | low |
| VCC | 40 | 44 | 42 | power | |
| NC | — | 1, 12, 23, 34 | 1, 6, 13, 21, 25, 36, 37, 48 | nc | |

- None of these packages has an exposed pad. Note D of the PT drawing (p34) says it "may also be a thermally-enhanced plastic package with leads connected to the die pads".
- No pin has a mode-selected function.

### Host interface (p5–6, p8, Figs 2–3 on p11–12)
- **Bus type:** Intel-style, with separate read and write strobes. Each strobe has two inputs of opposite polarity. Only one input of each pair is used; the other is tied inactive (RD2 low or RD1 high; WR2 low or WR1 high).
  - The chip is reading when (RD1=0 OR RD2=1) and it is selected.
  - The chip is writing when (WR1=0 OR WR2=1) and it is selected.
- **Chip select:** the chip is selected when CS0=1 AND CS1=1 AND CS2=0.
- **Address:** A2..A0.
- **ADS address strobe:**
  - While ADS is low, A0–A2 and CS0–CS2 pass straight through to the select logic.
  - On ADS low→high, A0–A2 and CS0–CS2 are latched and held while ADS is high.
  - For non-multiplexed buses, tie ADS low (Fig 13, p18).
- **Write:** the data is latched on the trailing edge of the strobe (WR1↑ / WR2↓). Fig 2 shows tsu3 (data setup) ending at the trailing edge and th5 (data hold) after it. The p8 text says "before WR1↓ or WR2↑"; that is a datasheet typo (see Notes).
- **Read:**
  - D7–D0 drive while the chip is selected and read is active. Data is valid td10 after RD active.
  - The bus floats td11 after RD goes inactive. At all other times D7–D0 are 3-state.
- **DDIS:** high whenever the CPU is not reading the chip. It goes low during a selected read, after tdis(R). It is meant to disable an external bus transceiver (p5, Fig 3, Fig 14).
- **Read side effects:**
  - Reading RBR pops the RX FIFO.
  - Reading LSR clears OE/PE/FE/BI.
  - Reading MSR clears the delta bits.
  - Reading IIR clears a THRE interrupt if THRE was the source.
- **Write side effect:** writing THR clears the THRE interrupt.
- Note 3 (p9): in FIFO mode, keep at least 425 ns between reads of the RX FIFO and reads of the status registers (IIR or LSR).

### Registers (Table 1 p20, Table 3 p21, bit text p22–31)
DLAB is LCR[7].

| A2A1A0 | DLAB | Name | R/W | Reset (MR) |
|---|---|---|---|---|
| 000 | 0 | RBR receiver buffer | R | not affected |
| 000 | 0 | THR transmitter holding | W | not affected |
| 001 | 0 | IER | R/W | 0x00 |
| 010 | x | IIR | R | 0x01 |
| 010 | x | FCR | W | 0x00 |
| 011 | x | LCR | R/W | 0x00 |
| 100 | x | MCR | R/W | 0x00 |
| 101 | x | LSR | R (writing is "factory test only", p26 footnote) | 0x60 |
| 110 | x | MSR | R | bits 3:0 = 0; bits 7:4 = complemented inputs |
| 111 | x | SCR scratch | R/W | not affected |
| 000 | 1 | DLL divisor LSB | R/W | not affected |
| 001 | 1 | DLM divisor MSB | R/W | not affected |

Reset table (Table 2, p20):
- Both FIFOs are also cleared by FCR1 (RX) or FCR2 (TX), and by any change of FCR0 ("ΔFCR0").
- MR leaves SCR, DLL, DLM, RBR and THR unchanged.

**IER** (p24):

| Bit | Meaning when set |
|---|---|
| 0 | ERBI: enable received-data-available interrupt; in FIFO mode this also enables the timeout interrupt |
| 1 | ETBEI: enable THRE interrupt |
| 2 | ELSI: enable receiver-line-status interrupt |
| 3 | EDSSI: enable modem-status interrupt |
| 7:4 | always 0 |

**IIR** (p24–25, Table 5):

| Bit | Meaning |
|---|---|
| 0 | 0 = interrupt pending, 1 = none pending |
| 2:1 | interrupt ID |
| 3 | 1 together with bit 2 means a character timeout; always 0 in 450 mode |
| 5:4 | 0 |
| 7:6 | both set when FCR0=1 ("FIFOs enabled"); 0 in 450 mode |

IIR[3:0] codes, highest priority first:

| IIR[3:0] | Priority | Source | Cleared by |
|---|---|---|---|
| 0001 | — | none | — |
| 0110 | 1 | line status: OE, PE, FE or BI | reading LSR |
| 0100 | 2 | received data available (450 mode), or RX FIFO at/above trigger (FIFO mode) | reading RBR; in FIFO mode, when the FIFO drops below the trigger |
| 1100 | 2 | character timeout | reading RBR |
| 0010 | 3 | THR empty (FIFO mode: TX FIFO empty) | reading IIR when THRE is the source, or writing THR |
| 0000 | 4 | modem status: ΔCTS, ΔDSR, TERI, ΔDCD | reading MSR |

**FCR** (write-only; p22):

| Bit | Meaning |
|---|---|
| 0 | Enables both FIFOs. It must be 1 when any other FCR bit is written, or those bits are not programmed. Any change of this bit clears both FIFOs. |
| 1 | Clears the RX FIFO and its counter. The shift register is not cleared. Self-clearing. |
| 2 | Clears the TX FIFO and its counter. The shift register is not cleared. Self-clearing. |
| 3 | When FCR0=1, switches RXRDY/TXRDY from DMA mode 0 to mode 1 |
| 5:4 | reserved |
| 7:6 | RX trigger level: 00 = 1, 01 = 4, 10 = 8, 11 = 14 bytes (Table 4) |

**LCR** (p25–26):

| Bit | Meaning |
|---|---|
| 1:0 | word length: 00 = 5, 01 = 6, 10 = 7, 11 = 8 bits |
| 2 | STB. 0 = 1 stop bit. 1 = 1.5 stop bits for a 5-bit word, 2 stop bits for 6/7/8-bit words. The receiver checks only the first stop bit. |
| 3 | PEN: parity generated and checked |
| 4 | EPS: 1 = even, 0 = odd |
| 5 | Stick parity: bits 3,4,5 = 1,1,1 → parity bit sent and checked as 0; bits 3,4,5 = 1,0,1 → parity bit sent and checked as 1 |
| 6 | Break: forces SOUT to space (0). It affects only the serial output, not the transmitter logic. |
| 7 | DLAB |

**LSR** (p26–27):

| Bit | Name | Behavior |
|---|---|---|
| 0 | DR | Set when a complete character is moved into RBR or the FIFO. Cleared when all data in RBR/FIFO has been read. |
| 1 | OE | Set when a character overwrites an unread RBR. In FIFO mode it sets only when the FIFO is full and the next character has been completely received in the shift register. OE is reported at once; the shift-register character is overwritten and never goes into the FIFO. Cleared by reading LSR. |
| 2 | PE | Cleared by reading LSR. In FIFO mode it belongs to its character and shows when that character reaches the top of the FIFO. |
| 3 | FE | No valid (1) stop bit. Same FIFO handling as PE. After a framing error the receiver assumes the error was caused by the next start bit: it "samples this start bit twice and then accepts the input data". |
| 4 | BI | SIN held at 0 for longer than one full word time (start + data + parity + stop). Same FIFO handling. One 0 character is loaded into the FIFO. The next character is accepted only after SIN goes to mark and a valid start bit follows. Cleared by reading LSR. |
| 5 | THRE | Set when THR moves into TSR (FIFO mode: when the TX FIFO is empty). Cleared when THR is written (FIFO mode: when at least one byte is written). |
| 6 | TEMT | Set when THR (FIFO mode: TX FIFO) and TSR are both empty. |
| 7 | Error in RX FIFO | 0 in 450 mode. In FIFO mode, set when at least one PE/FE/BI error is in the FIFO. Cleared when LSR is read and no further errors remain in the FIFO. |

LSR bits 1–4 are the sources of the line-status interrupt.

**MCR** (p27–28):

| Bit | Meaning |
|---|---|
| 0 | DTR: 1 drives the DTR pin low |
| 1 | RTS: same scheme as DTR |
| 2 | OUT1: same scheme |
| 3 | OUT2: same scheme |
| 4 | LOOP |
| 7:5 | permanently 0 |

**MSR** (p28–29):

| Bit | Name | Meaning |
|---|---|---|
| 0 | ΔCTS | CTS changed since the last MSR read |
| 1 | ΔDSR | DSR changed since the last MSR read |
| 2 | TERI | RI pin went low→high |
| 3 | ΔDCD | DCD changed since the last MSR read |
| 4 | CTS | complement of the CTS pin; in loop = MCR1 (RTS) |
| 5 | DSR | complement of the DSR pin; in loop = MCR0 (DTR). The datasheet says "MCR bit 1 (DTR)"; that is a typo. |
| 6 | RI | complement of the RI pin; in loop = MCR2 (OUT1) |
| 7 | DCD | complement of the DCD pin; in loop = MCR3 (OUT2) |

Reading MSR clears bits 3:0.

**SCR:** 8-bit scratch register with no effect on the chip (p31).

**DLL/DLM:** the 16-bit divisor (p29).

### Behavior
- **Baud generator** (p29, Fig 1 p10):
  - The clock source is the XIN/XOUT crystal oscillator or an external clock on XIN. p29 gives the range as "dc and 8 MHz"; p8 gives f = 9 MHz max (tw1/tw2 ≥ 40 ns); p1 gives "DC to 562 Kbit/s" (= 9 MHz / 16).
  - The divisor ranges from 1 to 65535. Divisor = fXIN / (16 × baud).
  - BAUDOUT is the 16× clock (fXIN / divisor). It is active-low drawn, and its waveform is:
    - divisor 1: BAUDOUT follows XIN;
    - divisor 2: square wave;
    - divisor 3: low for 2 XIN cycles, high for 1;
    - divisor N > 3: low for 2 XIN cycles, then high for N−2 cycles.
  - Writing either divisor latch also loads the 16-bit baud counter, so the first count is not long.
  - The transmitter is always clocked by the internal 16× clock (BAUDOUT). The receiver is clocked by the RCLK pin; tie BAUDOUT to RCLK for the same rate.
  - Divisor 0 behavior is not specified.
  - Tables 8 and 9 (p29–30) give example divisors for 1.8432 MHz and 3.072 MHz.
- **Receiver** (Fig 4 p13, p26–27):
  - 16× sampling. Each bit is sampled 8 RCLKs into the bit, i.e. mid-bit. td12, RCLK to sample, is 10 ns max.
  - The p1 feature list includes "False start bit detection"; the validation rule is not detailed.
  - Bit order is LSB first. Frame: start bit, 5–8 data bits, optional parity, then stop.
  - DR / the RX interrupt / RXRDY set td13 = 1 RCLK cycle after the stop-bit sample. For the timeout interrupt, td13 = 8 RCLKs.
- **Transmitter** (Fig 9 p16, p9):
  - First write to an idle transmitter → start bit: td15 = 8 to 24 BAUDOUT cycles.
  - Stop bit → THRE interrupt: td16 = 8 to 9 BAUDOUT cycles.
  - Initial THR write → THRE interrupt: td18 = 16 to 32 BAUDOUT cycles.
  - SOUT is mark (1) when idle and after MR.
- **FIFO mode** (p22–23):
  - Both FIFOs are 16 deep.
  - The RX data interrupt (IIR = 0100) and its IIR indication are asserted at or above the trigger level and clear when the FIFO drops below it.
  - DR is set when a character enters the FIFO and cleared when the FIFO is empty.
  - **RX timeout:** asserted when all of these hold:
    - at least one character is in the FIFO;
    - the last character was received more than 4 character times ago (counting the second stop bit if two are programmed);
    - the last CPU read of the FIFO was more than 4 character times ago.
  - Character time is derived from RCLK. A timeout that has fired is cleared, and its timer reset, by reading one character. If no timeout has fired, the timer resets on each received character or CPU read. Maximum timeout example: 160 ms at 300 baud with a 12-bit character.
  - **TX interrupt:** set when the TX FIFO is empty. Cleared by a THR write or an IIR read. The TX-FIFO-empty indication is delayed by one character time minus the last stop bit when THRE=1 and the FIFO has not held at least two bytes at once since the last THRE=1. The first TX interrupt after changing FCR0 is immediate if it is enabled.
  - Timeout and trigger interrupts share priority 2; TX FIFO empty has priority 3.
  - **Polled FIFO mode** (FCR0=1 with the IER bits clear): the FIFOs work normally, LSR reports status, and no trigger or timeout indications appear.
- **DMA pins** (p6, Figs 7, 8, 10, 11 p15–16):
  - **RXRDY, mode 0** (450 mode, or FCR0=1 with FCR3=0): low while at least one character is in RBR/FIFO; goes high when it is empty.
  - **RXRDY, mode 1** (FCR0=1, FCR3=1): goes low when the trigger level or the timeout is reached; goes high when the FIFO/holding register is empty.
  - **TXRDY, mode 0:** goes high td20 (35 ns max) after a THR write (Fig 10, byte #1). It goes low again td21 (8 BAUDOUT cycles max) after SOUT's start bit, i.e. when THR/FIFO is empty again.
  - **TXRDY, mode 1:** goes high when the TX FIFO becomes full (Fig 11, byte #16). It goes low td21 after the next start bit, i.e. when space is available again. The exact re-assert threshold is not stated beyond "FIFO full" in the figure.
  - Only mode 0 is available in 450 mode.
- **Interrupt pin:** INTRPT is an active-high output, always driven. The datasheet says nothing about 3-state or OUT2 gating. INTRPT = any enabled source pending.
- **Modem outputs:** DTR, RTS, OUT1 and OUT2 are the inverse of MCR bits 0–3. td22 = 50 ns max from the MCR write. A modem input change sets INTRPT within td23 = 35 ns.
- **Loopback** (MCR4=1, p28):
  - SOUT is forced high (mark), SIN is disconnected, and the TSR output feeds the RSR input internally.
  - CTS, DSR, DCD and RI are disconnected. Internally RTS→CTS, DTR→DSR, OUT1→RI, OUT2→DCD.
  - The DTR/RTS/OUT1/OUT2 pins are forced inactive (high).
  - All interrupts still work; the modem interrupts come from MCR bits 3:0.
- **Reset** (MR high, at least 1 µs; Table 2, p20):
  - Registers reset: IER = 0, IIR = 0x01, FCR = 0, LCR = 0, MCR = 0, LSR = 0x60, MSR[3:0] = 0.
  - SOUT goes high, and INTRPT goes low (all sources cleared).
  - OUT1, OUT2, RTS and DTR all go high.
  - Both FIFOs are cleared.
  - RXRDY, TXRDY and DDIS are not listed in Table 2. By inference RXRDY is high (empty), TXRDY is low (THR empty) and DDIS is high (not reading); this is inferred, not stated.
- **Reserved/undefined:** FCR bits 5:4 are reserved. Writing LSR is factory test only.

### Timing (VCC 4.75–5.25 V, all temperature grades; one table for both grades)
Unless marked MIN, values are maxima; CL = 75 pF where stated.

**Clock and bus timing requirements (all MIN), p8**

| Symbol | Parameter | Value |
|---|---|---|
| tcR | read cycle | 87 ns |
| tcW | write cycle | 87 ns |
| tw1, tw2 | XIN high / low (f = 9 MHz max) | 40 ns |
| tw5 | ADS low | 9 ns |
| tw6 | WR pulse width | 40 ns |
| tw7 | RD pulse width | 40 ns |
| tw8 | MR pulse | 1 µs |
| tsu1 | address before ADS↑ | 8 ns |
| tsu2 | CS before ADS↑ | 8 ns |
| tsu3 | data setup before the WR trailing edge | 15 ns |
| th1, th2 | address / CS hold after ADS↑ | 0 ns |
| th3 | CS hold after WR end | 10 ns |
| th4 | address hold after WR end | 10 ns |
| th5 | data hold after WR end | 5 ns |
| th6 | CS hold after RD end | 10 ns |
| th7 | address hold after RD end | 20 ns |
| td4 | CS before WR | 7 ns |
| td5 | address before WR | 7 ns |
| td6 | WR end to ADS↓ | 40 ns |
| td7 | CS before RD | 7 ns |
| td8 | address before RD | 7 ns |
| td9 | RD end to ADS↓ | 40 ns |

**Bus switching (max), p8**

| Symbol | Parameter | Value |
|---|---|---|
| td10 | RD active → data valid | 45 ns |
| td11 | RD end → data float | 20 ns |
| tdis(R) | RD → DDIS | 20 ns |

**Baud generator, p8**

| Symbol | Parameter | Value |
|---|---|---|
| tw3, tw4 | BAUDOUT low / high at 9 MHz ÷ 2 | 80 ns min |
| td1 | XIN↑ → BAUDOUT↑ | 75 ns max |
| td2 | XIN → BAUDOUT↓ | 65 ns max |

**Receiver, p9**

| Symbol | Parameter | Value |
|---|---|---|
| td12 | RCLK → sample | 10 ns |
| td13 | stop → interrupt / RXRDY↓ | 1 RCLK cycle |
| td14 | RBR/LSR read → interrupt cleared | 40 ns |

**Transmitter, p9**

| Symbol | Parameter | Value |
|---|---|---|
| td15 | initial write → start bit | 8 to 24 BAUDOUT cycles |
| td16 | stop → INTRPT | 8 to 9 BAUDOUT cycles |
| td17 | THR write → interrupt cleared | 50 ns |
| td18 | initial THR write → THRE interrupt | 16 to 32 BAUDOUT cycles |
| td19 | IIR read → THRE interrupt cleared | 35 ns |
| td20 | THR write → TXRDY high | 35 ns |
| td21 | start bit → TXRDY low | 8 BAUDOUT cycles |

**Modem, p9**

| Symbol | Parameter | Value |
|---|---|---|
| td22 | MCR write → outputs | 50 ns |
| td23 | modem input → INTRPT | 35 ns |
| td24 | MSR read → interrupt cleared | 40 ns |

### VCC (p7)
- Recommended operating range: 4.75–5.25 V for both grades. TL16C550B: 0–70 °C. TL16C550BI: −40–85 °C.
- Input levels: VIH 2 V to VCC, VIL −0.5 to 0.8 V (TTL).
- Absolute maximum: VI −0.5 to 7 V.
- inputMax = VCC (the recommended VIH maximum). The chip is not 5 V-tolerant above VCC; at 5 V supply this does not matter.

### Package drawings
None of these drawings has a land pattern printed.

| Package | Drawing | Page | Key dimensions |
|---|---|---|---|
| FN, 44-pin PLCC (S-PQCC-J**) | TI 4040005/B 03/95, JEDEC MS-018 | p32 | Pitch 1.27 mm. Span D/E 17.40–17.65 (mid 17.53). Body D1/E1 16.51–16.66 (mid 16.59). D2/E2 7.39–8.10. Lead width 0.33–0.53 (mid 0.43), upper shoulder 0.66–0.81. Height 4.57 max; 0.51 min standoff. |
| N, 40-pin PDIP (R-PDIP-T**) | TI 4040053/B 04/95, JEDEC MS-011 | p33 | Pitch 2.54. Row spacing 15.24 (0.600 in) between lead centres; spread 14.99–15.49 across. Body width 13.21–14.22. Length A 51.82–53.09. Lead width 0.38–0.53 (mid 0.46), shoulder 1.52 typ. Seating to tip 3.18 min. Height 5.08 max. |
| PT, 48-pin LQFP/TQFP (S-PQFP-G48) | TI 4040052/B 03/95, JEDEC MO-136 | p34 | Pitch 0.50. Lead width 0.17–0.27 (mid 0.22). Body 6.80–7.20 (7.00) square. Tip-to-tip span 8.80–9.20 (9.00). Foot length 0.45–0.75 (mid 0.60). Height 1.60 max; standoff 0.05 min; body thickness 1.35–1.45. Pin-row extent 5.50 typ. |

### Obsolete
TL16C550BFN and TL16C550BN are Obsolete at DigiKey. TL16C550BPTR is Active.

### Notes / datasheet errors
1. **CS1 overbar:** the N and FN pinout drawings (p3) show CS1 with an overbar. The pin-function text (p5: CS0=H, CS1=H, CS2=L), the PT drawing, Fig 2 (p11, "CS0, CS1, CS2-bar") and Fig 13 (CS1 tied H) all say CS1 is active high. Use active high.
2. **tsu3 edge:** p8 defines tsu3 as data valid "before WR1↓ or WR2↑". Fig 2 (p11) measures it to the trailing edge (WR1↑/WR2↓), where data is latched. Use the trailing edge.
3. **MSR[5] in loopback:** the text says it equals "MCR bit 1 (DTR)". It should be MCR bit 0, since DTR is MCR0.
4. **LSR[7] wording:** p27 says "In the TL16C550B … mode, this bit is always cleared" and then describes its FIFO-mode function. The intended meaning is 0 in 450 mode and active in FIFO mode.
5. **Fig 8 caption:** it reads "FCR = 1 or FCR3 = 1". It should be FCR0 = 1 and FCR3 = 1.
6. **XIN maximum:** 8 MHz (p29) conflicts with 9 MHz (p8) and with 562 kbit/s (p1). The AC tables are specified at 9 MHz.
7. **Supply in the census:** the census says "3.3V, 5V" for this part; this datasheet allows only 5 V ±5%.
8. **Not in the datasheet:** no ordering table, no land patterns, and no description of the TXRDY mode-1 re-assert threshold.

## TL16C550C

- Title (p1): "TL16C550C Asynchronous Communications Element with Autoflow Control" (SLLS177I, Mar 1994, rev. Mar 2021). Covers TL16C550C (0–70 °C) and TL16C550CI (−40–85 °C).
- Datasheet: `datasheets/controller-uart/tl16c550c.pdf` (51 pp). Pins p3–5; ratings/ROC p6–7; DC p8; timing p9–11; waveforms p12–18; autoflow p19–20; registers p21–31; clocks p31–32; addendum p37–38; drawings p41–50.
- How it differs from its siblings: it is a TL16C550B (16550 with 16-byte FIFOs, 16450 mode at power-up, TXRDY/RXRDY DMA pins) plus **autoflow control** (MCR[5] AFE: auto-RTS and auto-CTS) (p1, p19). Same 3-bit register map, no EFR and no extended registers, no sleep, no IrDA, no prescaler. It is not a TL16C550D (no 32/24-pin packages, no sleep/low-voltage features). Register bits that 16550B leaves reserved: MCR[5] = AFE.

### Parts (census)

Suffix decode, from the p37 addendum and p6 temp ranges: `FN` = 44-pin PLCC (FN0044A). `PT` = 48-pin LQFP (PT0048A). `PFB` = 48-pin TQFP (PFB0048A). `I` = industrial (−40..85 °C); no `I` = commercial (0..70 °C). `R` = tape and reel. `G4` = green/RoHS packing variant (same die and package). Each part's supply is either 3.0–3.6 V or 4.75–5.25 V. The datasheet gives one die with two ROC tables, not two grades (p6–7).

| Part | Package | Pins | Pinout | Temp | Supply | DigiKey status |
|---|---|---|---|---|---|---|
| TL16C550CFN | PLCC FN0044A | 44 | plcc44 | 0–70 | 3.0–3.6 V / 4.75–5.25 V | Obsolete |
| TL16C550CFNR | PLCC FN0044A (T&R) | 44 | plcc44 | 0–70 | same | Active |
| TL16C550CFNRG4 | PLCC FN0044A (T&R, G4) | 44 | plcc44 | 0–70 | same | Discontinued at DigiKey |
| TL16C550CIFN | PLCC FN0044A | 44 | plcc44 | −40–85 | same | Obsolete |
| TL16C550CIFNR | PLCC FN0044A (T&R) | 44 | plcc44 | −40–85 | same | Active |
| TL16C550CIPTR | LQFP PT0048A (T&R) | 48 | lqfp48 | −40–85 | same | Active |
| TL16C550CPFB | TQFP PFB0048A | 48 | lqfp48 (same pinout as PT) | 0–70 | same | Obsolete |
| TL16C550CPFBR | TQFP PFB0048A (T&R) | 48 | lqfp48 | 0–70 | same | Active |
| TL16C550CPT | LQFP PT0048A | 48 | lqfp48 | 0–70 | same | Obsolete |
| TL16C550CPTG4 | LQFP PT0048A (G4) | 48 | lqfp48 | 0–70 | same | Obsolete |
| TL16C550CPTR | LQFP PT0048A (T&R) | 48 | lqfp48 | 0–70 | same | Active |

Every census part decodes. The non-reel parts (CFN, CIFN, CPFB, CPT, CPTG4) are not in the 9-Nov-2025 addendum (p37). That fits their Obsolete status. The addendum lists TL16C550CPFBRG4 at −40..85 °C even though it has no "I". That looks like a TI addendum error, and the part is not in the census anyway. The PT and PFB packages have identical pin numbering (Fig 4-3, p3) and differ only in body thickness: PFB is 1.2 mm max, PT is 1.6 mm max. Footprints are almost the same (see Package drawings). The N package (40-pin DIP) is in the pin table but is "Not Recommended for New Designs" and no census part uses it. Its pinout is still given below.

### Pinouts (p3–5, Fig 4-1/4-2/4-3 and Table 4-1)

Signals and roles are the same in every package:
- Inputs: A0 A1 A2 ADS CS0 CS1 CS2 CTS DCD DSR RI MR RCLK RD1 RD2 WR1 WR2 SIN.
- Outputs: BAUDOUT DDIS DTR INTRPT OUT1 OUT2 RTS RXRDY SOUT TXRDY.
- XIN is the oscillator/clock input and XOUT the oscillator output; Table 4-1 lists both as "I/O". Model XIN as an input and XOUT as an output that drives the inverted XIN.
- D0–D7 are inout. VCC is power, VSS is ground.
- None of the packages has an exposed pad.

| Signal | Role | N dip40 | FN plcc44 | PT/PFB 48 |
|---|---|---|---|---|
| D0 | inout | 1 | 2 | 43 |
| D1 | inout | 2 | 3 | 44 |
| D2 | inout | 3 | 4 | 45 |
| D3 | inout | 4 | 5 | 46 |
| D4 | inout | 5 | 6 | 47 |
| D5 | inout | 6 | 7 | 2 |
| D6 | inout | 7 | 8 | 3 |
| D7 | inout | 8 | 9 | 4 |
| RCLK | input | 9 | 10 | 5 |
| SIN | input | 10 | 11 | 7 |
| SOUT | output | 11 | 13 | 8 |
| CS0 | input | 12 | 14 | 9 |
| CS1 | input | 13 | 15 | 10 |
| CS2 | input | 14 | 16 | 11 |
| BAUDOUT | output | 15 | 17 | 12 |
| XIN | input | 16 | 18 | 14 |
| XOUT | output | 17 | 19 | 15 |
| WR1 | input | 18 | 20 | 16 |
| WR2 | input | 19 | 21 | 17 |
| VSS | ground | 20 | 22 | 18 |
| RD1 | input | 21 | 24 | 19 |
| RD2 | input | 22 | 25 | 20 |
| DDIS | output | 23 | 26 | 22 |
| TXRDY | output | 24 | 27 | 23 |
| ADS | input | 25 | 28 | 24 |
| A2 | input | 26 | 29 | 26 |
| A1 | input | 27 | 30 | 27 |
| A0 | input | 28 | 31 | 28 |
| RXRDY | output | 29 | 32 | 29 |
| INTRPT | output | 30 | 33 | 30 |
| OUT2 | output | 31 | 35 | 31 |
| RTS | output | 32 | 36 | 32 |
| DTR | output | 33 | 37 | 33 |
| OUT1 | output | 34 | 38 | 34 |
| MR | input | 35 | 39 | 35 |
| CTS | input | 36 | 40 | 38 |
| DSR | input | 37 | 41 | 39 |
| DCD | input | 38 | 42 | 40 |
| RI | input | 39 | 43 | 41 |
| VCC | power | 40 | 44 | 42 |

NC pins (read from the drawings, p3):
- FN plcc44: 1, 12, 23, 34.
- PT/PFB 48: 1, 6, 13, 21, 25, 36, 37, 48.

Active levels (overbars in the drawings, p3):
- Active low: CS2, BAUDOUT, WR1, RD1, ADS, TXRDY, RXRDY, OUT1, OUT2, RTS, DTR, CTS, DSR, DCD, RI.
- Active high: CS0, CS1, WR2, RD2, MR, INTRPT, DDIS.

No pin's function is selected by a mode pin. Each strobe has a low-active and a high-active input instead (RD1/RD2, WR1/WR2).

### Host interface (p3–5, p9, Fig 6-2/6-3 p13–14)

- Intel-style bus. Read: RD1 (active low) or RD2 (active high). Write: WR1 (active low) or WR2 (active high). Only one of each pair is needed; tie the other to its inactive level (RD2 low or RD1 high; WR2 low or WR1 high) (p5).
- The chip is selected when CS0 = 1, CS1 = 1 and CS2 = 0 (p4).
- Registers are addressed by A2..A0 plus DLAB = LCR[7].
- ADS is an address strobe and is transparent while low. While ADS is low, A0–A2, CS0, CS1 and CS2 feed the select logic directly. On ADS↑ those signals are latched, and they are held while ADS is high (p3). If no latching is needed, tie ADS low.
- Write:
  - The data are latched on the trailing edge of the write strobe: WR1↑ or WR2↓.
  - Timing (Fig 6-2, p13): tsu3 is data setup before the trailing edge and th5 is data hold after it.
  - Address and CS must be valid td5/td4 before the leading edge of WR. They must then hold th4/th3 after the trailing edge; the † note says these apply only when ADS is low.
- Read:
  - D0–D7 drive data td10 after the leading edge of RD (RD1↓ or RD2↑).
  - They float (high-Z) td11 after the trailing edge.
  - Otherwise D0–D7 are high-Z (3-state, p4).
- DDIS (p4) is high whenever the CPU is not reading the selected chip. It goes low during a selected read, tdis(R) after the RD leading edge, and returns high tdis(R) after the RD trailing edge (Fig 6-3). Use it to disable an external bus transceiver.
- Register reads with side effects:
  - RBR read pops the FIFO.
  - LSR read clears OE/PE/FE/BI.
  - MSR read clears the delta bits.
  - IIR read clears the THRE interrupt if THRE is the source.
- FIFO-mode bus note (5.10 footnote, p10): allow a read cycle of at least 425 ns between reads of the receive FIFO and reads of the status registers (IIR or LSR).

### Registers (Table 7-1 p21–22, Table 7-3 p23, §7.7.2–7.7.14 p24–31)

| DLAB | A2A1A0 | Name | R/W | Reset (MR) |
|---|---|---|---|---|
| 0 | 000 | RBR receiver buffer (FIFO top) | R | not affected |
| 0 | 000 | THR transmitter holding (FIFO) | W | not affected |
| 0 | 001 | IER | R/W | 0x00 |
| X | 010 | IIR | R | 0x01 |
| X | 010 | FCR | W | 0x00 |
| X | 011 | LCR | R/W | 0x00 |
| X | 100 | MCR | R/W | 0x00 |
| X | 101 | LSR | R (writing it is "not recommended outside factory test", p28) | 0x60 |
| X | 110 | MSR | R | bits 3:0 = 0; bits 7:4 = complement of the CTS/DSR/RI/DCD inputs |
| X | 111 | SCR scratch | R/W | not affected |
| 1 | 000 | DLL divisor LSB | R/W | not affected |
| 1 | 001 | DLM divisor MSB | R/W | not affected |

There are no other register banks. In a model, give unaffected registers an unknown (X) power-up value.

**IER** (p25):
- b0 ERBI: enables the received-data-available interrupt (and, in FIFO mode, the timeout interrupt).
- b1 ETBEI: enables the THRE interrupt.
- b2 ELSI: enables the receiver line status interrupt.
- b3 EDSSI: enables the modem status interrupt.
- b7:4: always 0.

**IIR** (read; p25–26):
- b0: 0 = interrupt pending, 1 = none pending.
- b2:1: interrupt ID (see the interrupt table below).
- b3: 1 together with b2 means a character timeout. It is always 0 in 16450 mode.
- b5:4: always 0.
- b7:6: both 1 when FCR[0] = 1 (FIFOs enabled), else 0.

**FCR** (write only; p24):
- b0 FIFO enable: enables both TX and RX FIFOs. FCR[0] must be 1 when any other FCR bit is written, or the other bits are not programmed. Changing FCR[0] clears both FIFOs.
- b1: RX FIFO reset. Clears every byte and the counter; the shift register is not cleared. Self-clearing.
- b2: TX FIFO reset. Same as b1, for the TX FIFO. Self-clearing.
- b3: DMA mode select. With FCR0 = 1, setting FCR3 switches RXRDY/TXRDY from mode 0 to mode 1. The datasheet text literally says "from level 0 to level 1"; it means mode.
- b5:4: reserved.
- b7:6: RX trigger level: 00 = 1 byte, 01 = 4, 10 = 8, 11 = 14 (Table 7-4).

**LCR** (p27):
- b1:0 WLS: 00 = 5, 01 = 6, 10 = 7, 11 = 8 data bits.
- b2 STB: 0 = 1 stop bit. 1 = 1.5 stop bits with 5-bit words, 2 stop bits with 6/7/8-bit words. The receiver checks only the first stop bit.
- b3 PEN: parity bit is generated after the last data bit and checked on receive.
- b4 EPS: 1 = even parity, 0 = odd.
- b5 stick parity:
  - PEN = 1, EPS = 1, stick = 1: parity bit transmitted and checked as 0.
  - PEN = 1, EPS = 0, stick = 1: parity bit transmitted and checked as 1.
- b6 break control: forces SOUT to space (0). It affects only SOUT, not the transmitter logic.
- b7 DLAB: divisor latch access.

**MCR** (p29):
- b0 DTR, b1 RTS, b2 OUT1, b3 OUT2: setting a bit drives its pin low; clearing it drives the pin high.
- b4 LOOP: loopback (see Behavior).
- b5 AFE: autoflow enable.
- b7:6: always 0 ("6–7 permanent", p22).

**LSR** (p28):
- b0 DR: set when a character is moved into the RBR or FIFO. Cleared when the RBR/FIFO has been read empty.
- b1 OE overrun: cleared by reading the LSR.
- b2 PE: per-character in FIFO mode; shows when that character is at the top of the FIFO. Cleared by reading the LSR.
- b3 FE: per-character, same rules as PE.
- b4 BI break: per-character, same rules as PE.
- b5 THRE:
  - 16450 mode: set when the THR is transferred to the TSR; cleared when the CPU writes the THR.
  - FIFO mode: set when the TX FIFO is empty; cleared when at least one byte is written.
- b6 TEMT: THR (or TX FIFO) and TSR both empty.
- b7: error in RX FIFO (FIFO mode only; 0 in 16450 mode). Set when at least one PE/FE/BI character is in the FIFO. Cleared when the LSR is read and no further errored character is in the FIFO.
- b1–b4 are the receiver line status interrupt sources.

**MSR** (p30):
- b0 ΔCTS: changed since the last MSR read. When AFE is on, it causes no interrupt (see Notes).
- b1 ΔDSR: changed since the last MSR read.
- b2 TERI: RI went low→high at the pin. The pin is active low, so this is the trailing edge of a ring: internal ring indication 1→0.
- b3 ΔDCD: changed since the last MSR read.
- b4 CTS, b5 DSR, b6 RI, b7 DCD: the complements of the pins.
- In loop mode the status bits are internal: CTS = MCR[1] (RTS), DSR = MCR[0] (DTR), RI = MCR[2] (OUT1), DCD = MCR[3] (OUT2).
- b3:0 are cleared by an MSR read.

**SCR**: 8-bit scratch register with no other effect (p31).

**DLL/DLM**: the 16-bit divisor. Writing either latch also loads the 16-bit baud counter (p31).

### Behavior

**Baud generator** (p31)
- Reference clock:
  - The clock is on XIN: a crystal between XIN and XOUT, or an external clock into XIN. Range is dc to 16 MHz.
  - ROC maximum oscillator/clock is 16 MHz at 5 V (p7) and 14.9 MHz at 3.3 V (p6).
- Divisor = fXIN / (baud × 16), range 1 to 65535. Divisor 0 is not defined by the datasheet.
- BAUDOUT is the 16× clock, active low, = fXIN / divisor. It feeds the transmitter internally.
- The receiver is clocked by the RCLK pin (16× rate). Wire BAUDOUT to RCLK for equal RX/TX rates (p4).
- No prescaler. Sampling is always 16×.
- BAUDOUT pulse widths: tw3/tw4 ≥ 50 ns at 16 MHz, CLK÷2 (p10).
- Typical divisors (Tables 7-9/7-10, p31–32): 1.8432 MHz → 9600 baud = 12. 3.072 MHz → 9600 baud = 20.

**Transmitter** (p31, Fig 6-9 p17)
- Frame: start bit (0), 5–8 data bits LSB first (p23, note 1), optional parity, then 1, 1.5 or 2 stop bits (1). The idle line is mark (1).
- THR/FIFO data move to the TSR when the TSR is idle.
- The first write to an idle transmitter starts the start bit after td15 = 8 to 24 BAUDOUT cycles (p11).
- Break: LCR[6] forces SOUT = 0 while set.
- After MR, SOUT is high.

**Receiver** (p27–28, p31, Fig 6-4 p15)
- Samples SIN with 16× RCLK, at mid-bit: the sample clock is 8 RCLKs after the start-bit edge (Fig 6-4). This is "false start bit detection" (p1): a start bit that is no longer low at its middle is rejected. The datasheet does not state the exact start-bit qualification; the usual 16550 rule is a re-check at mid-start.
- The stop bit is sampled once (only the first stop bit is checked).
- Framing error: the stop bit is 0. To resynchronize, the ACE assumes the bad stop bit was the next start bit, "samples this start bit twice and then accepts the input data" (p28).
- Break: SIN is held low longer than a full word time (start + data + parity + stop).
  - Only one 0 character is loaded into the FIFO, with BI (and normally FE) set.
  - The next character is accepted only after SIN has been mark for at least 2 RCLK samples and a valid start bit follows (p28).
- Overrun:
  - 16450 mode: a new character overwrites the RBR before the old one is read, and OE is set.
  - FIFO mode: OE is set only when the FIFO is full and another character has been completely received in the shift register. That character is overwritten in the shift register and is not transferred to the FIFO. OE is shown immediately, not when the character reaches the FIFO top.

**FIFOs** (p24–25)
- Depth: TX 16 × 8 bits; RX 16 × (8 + 3 error bits: PE, FE, BI).
- At power-up/MR the chip is in 16450 mode (FCR = 0): 1-byte RBR/THR. FCR[0] = 1 selects FIFO mode.
- RX interrupt in FIFO mode (FCR0 = 1, IER0 = 1):
  - The received-data-available interrupt and IIR = 0x?4 assert when the FIFO count reaches the trigger level.
  - They clear when the count drops below the trigger level.
  - LSR[0] = 1 while at least one byte is in the FIFO.
- Character timeout (IIR = 0x?C) asserts when all three hold:
  - at least 1 character is in the RX FIFO;
  - no character has been received for more than 4 continuous character times (including the 2nd stop bit if programmed);
  - the CPU has not read the FIFO for more than 4 character times.
- Character times are counted with RCLK. At 300 baud with 12-bit characters the maximum delay is 160 ms.
- The timeout interrupt clears, and its timer resets, when the CPU reads one character. If no timeout has occurred yet, the timer is reset by a new received character or by a CPU read of the FIFO.
- Fig 6-5/6-6 note A: the timeout interrupt asserts td13 = 9 RCLKs after the stop sample.
- TX interrupt in FIFO mode:
  - THRE (IIR = 0x?2) asserts when the TX FIFO is empty.
  - It clears when the THR is written (1–16 bytes may be written while servicing) or when the IIR is read.
  - Delay rule: if at least 2 bytes have not been in the TX FIFO at the same time since it was last empty, the THRE interrupt is delayed by one character time minus the last stop bit.
  - The first THRE interrupt after changing FCR0 is immediate if enabled (p25).
- Polled mode is FCR0 = 1 with the IER bits clear. There is no trigger or timeout indication, but LSR works as normal (p25).

**DMA pins** (p5, Fig 6-7/6-8 p16, Fig 6-10/6-11 p17)
- RXRDY (active low):
  - Mode 0 (FCR0 = 0, or FCR0 = 1 with FCR3 = 0): low while at least 1 character is in the RBR/FIFO. Goes high when it is empty.
  - Mode 1 (FCR0 = 1, FCR3 = 1): goes low when the trigger level or the timeout is reached. Goes high when the FIFO is empty.
  - Assert delay: td13 after the stop-bit sample (1 RCLK; 9 RCLKs for a timeout).
  - Deassert delay: td14 after the RD of the last byte.
- TXRDY (active low):
  - Mode 0: low when the THR/TX FIFO is empty, i.e. no characters. Goes high after the first write (td20 after WR).
  - Mode 1: low while the TX FIFO is not full. Goes high when the FIFO is full: td20 after the WR of byte #16 (Fig 6-11). Goes low again td21 (9 baudout cycles max) after the next start bit, once a slot is free.
  - 16450 mode allows DMA mode 0 only.
- The datasheet does not list RXRDY/TXRDY reset states in Table 7-2. From the definitions after MR (empty FIFOs, mode 0), RXRDY = 1 and TXRDY = 0. This is derived, not printed.

**Interrupts** (Table 7-5 p26; p25)

| IIR[3:0] | Priority | Type | Source | Cleared by |
|---|---|---|---|---|
| 0001 | – | none | – | – |
| 0110 | 1 (highest) | receiver line status | OE, PE, FE or BI | reading LSR |
| 0100 | 2 | received data available | data in RBR (16450 mode), or trigger level reached (FIFO mode) | reading RBR (FIFO: count drops below trigger) |
| 1100 | 2 | character timeout | see FIFOs | reading RBR |
| 0010 | 3 | THR empty | THRE | reading IIR (if THRE is the source) or writing THR |
| 0000 | 4 (lowest) | modem status | ΔCTS, ΔDSR, TERI, ΔDCD | reading MSR |

- IIR[7:6] = 11 in FIFO mode.
- INTRPT is active high and is a normal push-pull output, not 3-state. It is high while any enabled interrupt is pending.
- There is no OUT2 gating of INTRPT on this part: OUT2 is only a user output pin (p4).
- MR drives INTRPT low.
- LSR-error timing in FIFO mode: the error shows (and the LSI interrupt asserts) when the errored character reaches the top of the FIFO (Fig 6-6).

**Modem lines and loopback** (p29)
- DTR, RTS, OUT1 and OUT2 follow the inverse of MCR[0..3].
- Inputs CTS, DSR, RI and DCD appear inverted in MSR[4..7].
- Loop (MCR[4] = 1):
  - SOUT is forced high (mark).
  - SIN is disconnected, and the TSR output feeds the RSR input internally.
  - CTS/DSR/DCD/RI are disconnected.
  - The MCR bits feed the MSR: RTS→CTS, DTR→DSR, OUT1→RI, OUT2→DCD.
  - The four modem output pins are forced inactive (high).
  - RX and TX interrupts work normally. Modem interrupts are sourced from MCR[3:0].
  - INTRPT still works.

**Autoflow control** (p19–20, Table 7-8 p29, Fig 6-13/6-14/6-15 p18, Fig 7-2/7-3/7-4 p20)
- AFE (MCR[5]) = 1 with MCR[1] (RTS) = 1 enables auto-RTS and auto-CTS.
- AFE = 1 with MCR[1] = 0 enables auto-CTS only. AFE = 0 disables both.
- Autoflow works in FIFO mode (p1: "In the FIFO mode, there is a selectable autoflow control feature").
- Auto-CTS:
  - The transmitter checks CTS before it starts each byte. If CTS is low (active), it sends.
  - To stop the next byte, CTS must go high before the middle of the last stop bit of the current byte (tsu4 ≥ 10 ns before mid-stop).
  - The current byte always completes. When CTS goes low again, transmission restarts: SOUT start bit td25 ≤ 24 baudout cycles after CTS↓.
  - With autoflow on, CTS changes do not raise a modem interrupt.
- Auto-RTS, trigger level 1, 4 or 8:
  - RTS goes high (deasserted) when the RX FIFO reaches the trigger level: td26 ≤ 2 baudout cycles after the midpoint of the stop bit of the threshold byte.
  - The sender may send one more byte.
  - RTS goes low again once the RX FIFO is emptied: td27 ≤ 2 baudout cycles after the RBR read of the last byte.
- Auto-RTS, trigger level 14:
  - RTS goes high at the first data bit of the 16th character: td28 ≤ 2 baudout cycles after the midpoint of data bit 0.
  - It goes low again when the RX FIFO has at least one free byte, "and no incoming byte is in processing or there is more than one byte of space available".
  - When the FIFO is full, the first RBR read reasserts RTS: td29 ≤ 2 baudout cycles after RBR RD.

**Extras**: there is no sleep, IrDA, RS-485, 9-bit mode, GPIO, device ID or software reset. SCR is the only extra register.

**Reset** (MR is active high, minimum pulse tw8; Table 7-2 p22)
- IER = 0.
- IIR = 0x01 (b0 set; b1, 2, 3, 6, 7 clear).
- FCR, LCR and MCR = 0.
- LSR = 0x60.
- MSR[3:0] = 0.
- RX and TX FIFOs cleared.
- SOUT, OUT1, OUT2, RTS and DTR = high. INTRPT = low.
- SCR, DLL/DLM, RBR and THR are not affected.
- FIFOs are also cleared by FCR1 (RX) and FCR2 (TX), and by any change of FCR0 (Table 7-2).
- Not listed in Table 7-2: BAUDOUT keeps running from the divisor latches, which MR does not touch. DDIS is high (not reading).

### Timing (p9–11)

The tables have a single column, labelled "over recommended ranges" of supply. Some rows are conditioned "f = 16 MHz, VCC = 5 V". There are no separate 3.3 V values.

| Param | Meaning | Value |
|---|---|---|
| tcR / tcW | read and write cycle time | ≥ 87 ns |
| tw1 / tw2 | XIN high / low | ≥ 25 ns at 16 MHz, VCC = 5 V |
| tw5 | ADS low | ≥ 9 ns |
| tw6 | WR pulse | ≥ 40 ns |
| tw7 | RD pulse | ≥ 40 ns |
| tw8 | MR pulse | ≥ "1 ns" as printed; probably a typo for 1 µs (see Notes) |
| tsu1 / tsu2 | address / CS setup before ADS↑ | ≥ 8 ns |
| th1 / th2 | address / CS hold after ADS↑ | ≥ 0 ns |
| tsu3 | data setup before WR1↑ / WR2↓ | ≥ 15 ns |
| th5 | data hold after WR trailing edge | ≥ 5 ns |
| th3 / th4 | CS / address hold after WR trailing edge | ≥ 10 ns |
| td4 / td5 | CS / address valid before WR leading edge (ADS low only) | ≥ 7 ns |
| td6 | WR trailing edge to ADS↓ | ≥ 40 ns |
| th6 | CS hold after RD trailing edge | ≥ 10 ns |
| th7 | address hold after RD trailing edge | ≥ 20 ns |
| td7 / td8 | CS / address before RD leading edge (ADS low only) | ≥ 7 ns |
| td9 | RD trailing edge to ADS↓ | ≥ 40 ns |
| td10 | RD leading edge to data valid | ≤ 45 ns, CL 75 pF |
| td11 | RD trailing edge to data float | ≤ 20 ns |
| tdis(R) | RD edge to DDIS edge | ≤ 20 ns (p10) |
| td1 | XIN↑ to BAUDOUT↑ | ≤ 45 ns (p10) |
| td2 | XIN to BAUDOUT↓ | ≤ 45 ns (p10) |
| tw3 / tw4 | BAUDOUT low / high | ≥ 50 ns |
| td12 | RCLK to sample | ≤ 10 ns (p10) |
| td13 | stop sample to INTRPT, or to RXRDY↓ | 1 RCLK cycle (9 RCLKs for timeout) |
| td14 | read RBR/LSR to INTRPT reset (or to RXRDY↑) | ≤ 70 ns |
| td15 | initial THR write to start of transmit | 8–24 baudout cycles |
| td16 | start bit to INTRPT (THRE) | 8–10 baudout cycles |
| td17 | WR THR to INTRPT reset | ≤ 50 ns |
| td18 | initial write to INTRPT (THRE) | 16–34 baudout cycles |
| td19 | read IIR to INTRPT reset | ≤ 35 ns |
| td20 | write to TXRDY inactive | ≤ 35 ns |
| td21 | start to TXRDY active | ≤ 9 baudout cycles |
| td22 | WR MCR to output | ≤ 50 ns |
| td23 | modem input change to INTRPT | ≤ 35 ns |
| td24 | RD MSR to INTRPT reset | ≤ 40 ns |
| td25 | CTS↓ to SOUT↓ | ≤ 24 baudout cycles |
| td26–td29 | auto-RTS delays (see Autoflow) | ≤ 2 baudout cycles each |
| tsu4 | CTS↑ setup before mid-stop bit | ≥ 10 ns |

All values are MIN except td10, td11, tdis, td1/td2, td12, td14, td17, td19–td29, which are MAX.

### VCC

ROC (p6–7):
- 3.3 V nominal: 3.0–3.6 V. Clock ≤ 14.9 MHz. Ambient 0–70 °C as printed.
- 5 V nominal: 4.75–5.25 V. Clock ≤ 16 MHz. Ambient 0–70 °C as printed.
- The "I" parts are rated −40–85 °C in Abs Max (p6). There is no separate I ROC table.

Input voltage:
- Abs max: VI −0.5 to 7 V on any input, at any VCC (p6).
- ROC: VI is 0..VCC, so inputs are not specified as 5 V-tolerant at 3.3 V. inputMax = VCC by ROC, 7 V abs.
- 5 V input levels: TTL, VIH 2 V and VIL 0.8 V; XIN uses 0.7/0.3 VCC.
- 3.3 V input levels: 0.7/0.3 VCC (TTL-compatible per note).

### Package drawings

- **FN0044A**, 44-pin PLCC (JEDEC MS-018).
  - Outline p48 (4215154/A); land pattern p49; stencil p50.
  - Pitch 1.27 mm (.050 in). Body 16.51–16.66 mm (.650–.656 in), mid 16.59 mm.
  - Lead tip span 17.40–17.65 mm (.685–.695 in), mid 17.53 mm. Height ≤ 4.57 mm.
  - Lead width 0.33–0.53 mm (mid 0.43 mm). Shoulder width 0.66–0.81 mm.
  - Row spacing at the contact .582–.638 in (14.79–16.20 mm).
  - Land pattern: 44 pads 2.35 × 0.75 mm, pad-row centres 16.2 mm apart (both axes), pitch 1.27 mm.
  - Pin 1 is at the top centre; pins run counter-clockwise (6..1 on top, 44..40).
- **PT0048A**, 48-pin LQFP, 1.6 mm max (JEDEC MS-026).
  - Outline p44 (4215159/B); land pattern p45; stencil p46.
  - Body 6.8–7.2 mm (7.0). Tip span 8.8–9.2 mm (9.0). Pitch 0.5 mm.
  - Lead width 0.17–0.27 mm (0.22). Foot length 0.45–0.75 mm (0.60).
  - Land pattern: 48 pads 1.6 × 0.3 mm, row centres 8.2 mm apart.
- **PFB0048A**, 48-pin TQFP, 1.2 mm max (JEDEC MS-026).
  - Outline p41 (4215157/A); land pattern p42; stencil p43.
  - Body 6.8–7.2 mm. Tip span 8.8–9.2 mm. Pitch 0.5 mm.
  - Lead width 0.17–0.27 mm. Foot length 0.45–0.75 mm.
  - Land pattern: 48 pads 1.35 × 0.25 mm, row centres 8.5 mm apart.
- PT and PFB: pin 1 is at the top-left corner of the left side; pins run counter-clockwise (1–12 left, 13–24 bottom, 25–36 right, 37–48 top).
- p47 is blank. The datasheet has no N (DIP-40) drawing, and no census part uses N.

### Obsolete

These census rows have DigiKey status Obsolete: TL16C550CFN, TL16C550CIFN, TL16C550CPFB, TL16C550CPT, TL16C550CPTG4. TL16C550CFNRG4 is "Discontinued at DigiKey" but Active in the TI addendum.

### Notes

- Typos and unclear text in the datasheet:
  - tw8 (MR pulse) is printed "1 ns" (p9). Older TI 16550 sheets give 1 µs; treat it as 1 µs or flag it.
  - FCR3 text says "level 0 to level 1"; it means DMA mode 0 to mode 1.
  - LSR "Bit 12" is a typo for bit 1.
  - §7.2 refers to "Figure 4"; it means Fig 7-4.
  - LSR b7 text says "In the TL16C550C mode, this bit is always cleared"; this contradicts the FIFO-mode definition. Use: 0 in 16450 mode, error-in-FIFO in FIFO mode.
- MSR[0] wording (p30): "When autoflow control is enabled (ΔCTS is cleared), no interrupt is generated." It is not clear whether ΔCTS still sets with AFE on. The CTS pin description (p4) says a modem interrupt is generated "when CTS changes levels and the auto-CTS mode is not enabled". Safest model: with AFE = 1, CTS changes do not set ΔCTS and give no interrupt.
- RX timeout counting uses RCLK (p24).
- p23 Table 7-3 shows FCR bits 4–5 as "Reserved", IIR bits 4–5 as 0, and IER bits 4–7 as 0.
- 3.3 V: the same timing table applies; it has no 3.3 V-specific numbers.

## TL16C550D

- Title: "TL16C550D, TL16C550DI Asynchronous Communications Element with Autoflow Control", TI SLLS597E (Apr 2004, rev Dec 2008). Datasheet: `datasheets/controller-uart/tl16c550d.pdf`, 60 pages (pp1–42 datasheet, pp43–49 package addendum/materials, pp50–59 drawings).
- What it is: a speed/voltage upgrade of the TL16C550C, functionally the same (p1). 16-byte TX and RX FIFOs (the RX FIFO keeps 3 error bits per byte), auto-RTS/auto-CTS, and TL16C450 register compatibility after reset (p1). **Compared with its nearest sibling TL16C550C:** the register set and behavior are the same. The differences are the supply range (2.25–5.5 V in three bands), higher clock limits, and new packages (RHB QFN32 and ZQS BGA24). Those packages leave out pins, and RCLK is tied to BAUDOUT internally (pp2–3).

### Parts (census) — decode (ordering/addendum p43–44, temps p13)
Part-number pattern: `TL16C550D` + `I` (−40…85 °C; with no `I` it is 0…70 °C, p13) + package (`PT` = LQFP48 PT0048A, `PFB` = TQFP48 PFB0048A, `RHB` = VQFN32 RHB0032E, `ZQS` = 24-ball MicroStar Junior BGA). A trailing `R` means tape and reel; no suffix means tray (PT/PFB) or tube (RHB) (p43–44, p48–49). Every part covers VCC 2.25–5.5 V (in three bands, see VCC).

| Census part | Package (code) | Pins | Pinout name | Temp grade | Supply | DigiKey status |
|---|---|---|---|---|---|---|
| TL16C550DIPFB | TQFP PFB0048A | 48 | tqfp48 | I: −40…85 °C | 2.25–5.5 V | Active |
| TL16C550DIPFBR | TQFP PFB0048A | 48 | tqfp48 | I | 2.25–5.5 V | Active |
| TL16C550DPFBR | TQFP PFB0048A | 48 | tqfp48 | C: 0…70 °C (p43) | 2.25–5.5 V | Active |
| TL16C550DIPT | LQFP PT0048A | 48 | lqfp48 (same pinout as tqfp48) | I | 2.25–5.5 V | Active |
| TL16C550DIPTR | LQFP PT0048A | 48 | lqfp48 | I | 2.25–5.5 V | Active |
| TL16C550DPT | LQFP PT0048A | 48 | lqfp48 | C | 2.25–5.5 V | Active |
| TL16C550DPTR | LQFP PT0048A | 48 | lqfp48 | C | 2.25–5.5 V | Active |
| TL16C550DIRHB | VQFN RHB0032E + exposed pad | 32 | qfn32 | I | 2.25–5.5 V | Active |
| TL16C550DIRHBR | VQFN RHB0032E | 32 | qfn32 | I | 2.25–5.5 V | Active |
| TL16C550DRHB | VQFN RHB0032E | 32 | qfn32 | C | 2.25–5.5 V | Active |
| TL16C550DRHBR | VQFN RHB0032E | 32 | qfn32 | C | 2.25–5.5 V | Active |
| TL16C550DIZQSR | BGA ZQS (MicroStar Jr 3x3) | 24 | bga24 | I | 2.25–5.5 V | **Obsolete** |
| TL16C550DZQSR | BGA ZQS | 24 | bga24 | C | 2.25–5.5 V | **Obsolete** |

Every census part decodes. One caveat: ZQS does not appear in this PDF's package addendum (p43–44), and the PDF has no ZQS drawing. The ZQS grade comes from the part-number rule (I / no I), and the ZQS pinout is on p2 and p12.

### Pinouts
**PT/PFB 48 (tqfp48 / lqfp48)** (p2 drawing, p8–9 table). Active-low signals have the overbar dropped from their names.
```
1 NC nc        2 D5 inout     3 D6 inout     4 D7 inout     5 RCLK input   6 NC nc
7 SIN input    8 SOUT output  9 CS0 input    10 CS1 input   11 CS2 input   12 BAUDOUT output
13 NC nc       14 XIN input   15 XOUT output 16 WR1 input   17 WR2 input   18 VSS ground
19 RD1 input   20 RD2 input   21 NC nc       22 DDIS output 23 TXRDY output 24 ADS input
25 NC nc       26 A2 input    27 A1 input    28 A0 input    29 RXRDY output 30 INTRPT output
31 OUT2 output 32 RTS output  33 DTR output  34 OUT1 output 35 MR input    36 NC nc
37 NC nc       38 CTS input   39 DSR input   40 DCD input   41 RI input    42 VCC power
43 D0 inout    44 D1 inout    45 D2 inout    46 D3 inout    47 D4 inout    48 NC nc
```
(XIN/XOUT are listed as "I/O" for the crystal; model XIN as the clock input and XOUT as an output. No pin's function is selected by another pin.)

**RHB 32 (qfn32)** (p2 drawing; matches the RHB block diagram on p6). The p10 table prints the PT pin numbers for DSR (39), DTR (33), INTRPT (30) and MR (35). That is a datasheet error; the drawing values below are correct.
```
1 D4 inout     2 NC nc        3 D5 inout     4 D6 inout     5 D7 inout     6 SIN input
7 SOUT output  8 CS2 input    9 NC nc        10 XIN input   11 XOUT output 12 WR1 input
13 VSS ground  14 RD1 input   15 NC nc       16 NC nc       17 A2 input    18 A1 input
19 A0 input    20 INTRPT output 21 RTS output 22 DTR output 23 MR input    24 CTS input
25 DSR input   26 DCD input   27 RI input    28 VCC power   29 D0 inout    30 D1 inout
31 D2 inout    32 D3 inout    33 (exposed thermal pad, no signal; must be soldered, p51 note 3)
```
RHB leaves out CS0, CS1, ADS, RD2, WR2 and RCLK (inputs) and DDIS, TXRDY, RXRDY, OUT1, OUT2 and BAUDOUT (outputs). BAUDOUT is connected to RCLK internally (p2). The datasheet does not say how the missing inputs are tied internally. Because "all of the functionality ... is maintained", the implied ties are CS0 = CS1 = 1, ADS = 0 (transparent), RD2 = 0 and WR2 = 1. That is inference, not printed.

**ZQS 24-ball BGA (bga24)**, top view (p2 table; p12 terminal functions). Ball B2 is unpopulated.
```
A1 D5 inout   A2 D4 inout   A3 D2 inout   A4 D0 inout   A5 VCC power
B1 D7 inout   B2 (no ball)  B3 D3 inout   B4 D1 inout   B5 MR input
C1 SIN input  C2 SOUT output C3 D6 inout  C4 CTS input  C5 RTS output
D1 CS2 input  D2 WR1 input  D3 RD1 input  D4 INTRPT output D5 A0 input
E1 XIN input  E2 XOUT output E3 VSS ground E4 A2 input  E5 A1 input
```
ZQS also leaves out DSR, RI, DCD (inputs) and DTR (output), on top of the RHB omissions; BAUDOUT is tied to RCLK internally (p3). The datasheet does not say what MSR bits 5–7 / 1–3 read when those inputs are missing.

### Host interface (p8–9, p16, Figs 6–7 p19–20)
- Intel-style, with two strobes of opposite polarity for each direction. A read is active when RD1 = 0 **or** RD2 = 1 while the chip is selected; a write is active when WR1 = 0 **or** WR2 = 1. Only one strobe of each pair is used; the other must be tied inactive (RD2 low / RD1 high, WR2 low / WR1 high) (p8–9). RHB and ZQS have only RD1 and WR1 (active low).
- Chip select (PT/PFB): selected when CS0 = 1, CS1 = 1 and CS2 = 0 (p8). RHB/ZQS: selected when CS2 = 0 (p12).
- Address A2..A0 selects the register; DLAB = LCR[7] (Table 1, p32).
- ADS (PT/PFB): while ADS = 0, A0–A2 and CS0–CS2 pass straight to the internal select logic. On ADS↑ they are latched, and they stay latched while ADS = 1 (p8). Tie ADS low for non-multiplexed buses.
- Writes latch data on the trailing edge of the write strobe: setup tsu3 before WR1↑/WR2↓, hold th5 after (p16).
- D7–D0 are 3-state. They drive only during an active read cycle: valid td10 after RD active, floating td11 after RD inactive (p16).
- DDIS (PT/PFB only) is high whenever the CPU is not reading. It goes low during a selected read, delay tdis(R) (p8, p17, Fig 7). It is used to disable an external transceiver.

### Registers (Table 1 p32, Table 3 p33, bit text p33–39)
| DLAB | A2A1A0 | Name | R/W | Reset (MR, Table 2 p32) |
|---|---|---|---|---|
| 0 | 000 | RBR receiver buffer (16-byte FIFO in FIFO mode) | R | not affected |
| 0 | 000 | THR transmitter holding (16-byte FIFO) | W | not affected |
| 0 | 001 | IER | R/W | 0x00 |
| x | 010 | IIR | R | 0x01 |
| x | 010 | FCR | W | 0x00 |
| x | 011 | LCR | R/W | 0x00 |
| x | 100 | MCR | R/W | 0x00 |
| x | 101 | LSR | R (writes "not recommended outside factory testing", p37 note 1) | 0x60 |
| x | 110 | MSR | R | bits 3–0 = 0; bits 7–4 follow the inputs |
| x | 111 | SCR scratch | R/W | not affected |
| 1 | 000 | DLL divisor LSB | R/W | not affected |
| 1 | 001 | DLM divisor MSB | R/W | not affected |

Table 1 marks DLAB as "X" for addresses 2–7. With DLAB = 1, addresses 0 and 1 reach DLL and DLM. There are no enhanced or extended register banks (no EFR, no LCR = 0xBF bank).

Bits (bit 0 is the LSB and is sent and received first, p33 note 1):
- **RBR/THR**: data bits 0–7.
- **IER**: b0 ERBI, received-data-available interrupt (in FIFO mode this also enables the timeout interrupt). b1 ETBEI, THRE interrupt. b2 ELSI, receiver line status interrupt. b3 EDSSI, modem status interrupt. b4–7 are not used and always read 0 (p35). Clearing b0–b3 turns off the interrupt system.
- **IIR**: b0 = 0 when an interrupt is pending, 1 when none. b1–b2 give the interrupt ID. b3 = 1 (together with b2) for a timeout; it is always 0 in 450 mode. b4–5 always read 0. b6–7 are both 1 when FCR0 = 1, and 0 in 450 mode (p35). The codes are in the interrupt table below.
- **FCR** (write only, p33):
  - b0: enables both FIFOs. It must be 1 when any other FCR bit is written, or those bits are not programmed. Changing b0 clears both FIFOs.
  - b1: clears the RX FIFO and its counter, not the shift register. Self-clearing.
  - b2: clears the TX FIFO and its counter, not the shift register. Self-clearing.
  - b3: DMA mode select. With FCR0 = 1, setting it switches RXRDY/TXRDY from mode 0 to mode 1.
  - b4–5: reserved.
  - b6–7: RX trigger level: 00 = 1, 01 = 4, 10 = 8, 11 = 14 bytes (Table 4 p34).
- **LCR** (p36–37):
  - b1:0 word length: 00 = 5, 01 = 6, 10 = 7, 11 = 8 bits.
  - b2 STB: 0 = 1 stop bit; 1 = 1.5 stop bits for 5-bit words, 2 stop bits for 6/7/8-bit words (Table 7). The receiver checks only the first stop bit.
  - b3 PEN: parity is generated between the last data bit and the first stop bit, and checked on receive.
  - b4 EPS: 1 = even, 0 = odd parity.
  - b5 stick parity: with b3, b4, b5 = 1, the parity bit is sent and checked as 0. With b3 = 1, b4 = 0, b5 = 1, it is sent and checked as 1.
  - b6 break: forces SOUT to space (0). It affects SOUT only, not the transmitter logic.
  - b7 DLAB.
- **MCR** (p37–38):
  - b0 DTR, b1 RTS, b2 OUT1, b3 OUT2: a 1 drives the pin low, a 0 drives it high.
  - b4 LOOP.
  - b5 AFE, autoflow enable.
  - b6–7: always 0 ("6−7 permanent", Table 2).
  - Flow configuration (Table 8): AFE = 1 and RTS = 1 → auto-RTS + auto-CTS; AFE = 1 and RTS = 0 → auto-CTS only; AFE = 0 → both off.
- **LSR** (p37):
  - b0 DR: set when a complete character moves into the RBR or FIFO; cleared when all of the data in the RBR/FIFO has been read.
  - b1 OE: in 450 mode, the RBR was overwritten before it was read. In FIFO mode, it sets only when the FIFO is full and another character has been fully received into the shift register. That character is overwritten in the shift register and is not moved to the FIFO. OE is shown at once. Read LSR to clear.
  - b2 PE, b3 FE, b4 BI: in FIFO mode each is tied to its character and shows when that character is at the top of the FIFO. Read LSR to clear.
  - b5 THRE: set when the THR moves to the TSR (FIFO mode: TX FIFO empty); cleared when the CPU writes THR.
  - b6 TEMT: THR (or TX FIFO) and TSR both empty.
  - b7: errors in the RX FIFO. In FIFO mode it is 1 when at least one PE/FE/BI byte is in the FIFO, and it clears on an LSR read if no further errors remain in the FIFO. Always 0 in 450 mode.
  - Bits 1–4 are the sources of the line-status interrupt.
- **MSR** (p38–39):
  - b0 ΔCTS: CTS changed since the last MSR read. It does not interrupt when autoflow is enabled: the text says "When autoflow control is enabled (ΔCTS is cleared), no interrupt is generated."
  - b1 ΔDSR: DSR changed since the last MSR read.
  - b2 TERI: RI went low→high, i.e. the pin went from active to inactive.
  - b3 ΔDCD: DCD changed since the last MSR read.
  - b4 = NOT CTS pin (loopback: = MCR1 RTS); b5 = NOT DSR (loopback: MCR0 DTR); b6 = NOT RI (loopback: MCR2 OUT1); b7 = NOT DCD (loopback: MCR3 OUT2).
  - Reading MSR clears b0–b3.
- **SCR**: 8-bit scratch register with no effect on the device (p41).
- **DLL/DLM**: 16-bit divisor. Writing either latch also loads the 16-bit baud counter (p39).

### Behavior
- **Baud generator** (p39, Fig 5 p18):
  - Clock source: XIN, from a crystal on XIN/XOUT or an external clock.
  - Divisor = fXIN / (baud × 16), range 1…65535. The output is the 16× clock, available on BAUDOUT (PT/PFB; active-low name).
  - BAUDOUT waveform for divisor N:
    - N = 1: BAUDOUT follows XIN.
    - N = 2: low 1 XIN cycle, high 1.
    - N = 3: low 2, high 1.
    - N > 3: low for 2 XIN cycles, then high for N−2 XIN cycles.
  - The transmitter is clocked by BAUDOUT. The receiver is clocked by RCLK (PT/PFB; tie it to BAUDOUT for the same rate). On RHB and ZQS, RCLK is tied to BAUDOUT internally.
  - Clock limits (ROC p13–14): 16 MHz at 2.5 V; 20 MHz at 3.3 V (ZQS only: 48 MHz, ≤ 3 Mbaud at divisor 1, and 40 MHz for divisor ≥ 2, p1); 24 MHz at 5 V.
  - The p39 text says the "clock input range [is] dc and 16 MHz". That is stale and conflicts with the ROC limits.
  - Divisor 0 is not described.
- **Receiver**:
  - The 16× RCLK samples SIN. The data sample is taken 8 RCLK cycles into the bit (Fig 8 p21, "8 CLKs"), i.e. at mid-bit.
  - "False-start bit detection" (p1): the start bit is re-validated at mid-bit. The exact algorithm is not printed beyond that phrase.
  - Bits arrive LSB first.
  - The receiver checks only the first stop bit (p36).
  - FE: stop bit = 0. After a framing error the receiver resynchronizes. It assumes the error came from the next start bit, "samples this start bit twice and then accepts the input data" (p37).
  - Break: SIN low for longer than a full word time (start + data + parity + stop) sets BI. Only one 0 character goes into the FIFO. The next character is accepted only after SIN is marking for at least 2 RCLK samples and then a valid start bit arrives (p37).
- **Transmitter**:
  - The THR (or 16-byte FIFO) moves to the TSR when the TSR is idle. Bits go out on SOUT LSB first: start, data, parity, then stop(s).
  - SOUT is high (marking) after reset (p9).
  - Timing (p17): the first transmission starts 8–24 BAUDOUT cycles after the initial THR write (td15). The THRE interrupt comes 8–10 baudout cycles after the start bit (td16), and 16–34 baudout cycles after the initial write (td18).
- **FIFOs** (p33–35):
  - 16 bytes each. RX entries are 11 bits wide (data + PE/FE/BI).
  - FCR0 enables the FIFOs (FIFO mode). Toggling FCR0 clears both FIFOs (Table 2: RX/TX FIFO reset by MR, FCR1/FCR2 and ΔFCR0).
  - **RX data interrupt** (FIFO mode): asserted when the FIFO count is at or above the trigger level, and cleared when the count drops below it. IIR shows code 0x4 under the same condition.
  - **Timeout** (IIR 0xC): fires when all of these hold:
    - the FIFO holds at least 1 character;
    - the last character was received more than 4 character times ago (a second stop bit counts when programmed);
    - the last CPU read of the FIFO was more than 4 character times ago.
  - Timeout timing:
    - Character time is measured with RCLK, so it scales with the baud rate.
    - Once a timeout has fired, reading one character clears both the interrupt and the timer.
    - While no timeout is pending, a new character or a CPU read resets the timer.
    - Example: 160 ms at 300 baud with 12-bit characters (p34).
    - Fig 9/10 note: "for a time-out interrupt td13 = 9 RCLKs".
  - **TX interrupt** in FIFO mode: THRE (IIR 0x2) when the TX FIFO is empty. It clears on a THR write or an IIR read.
    - Delay rule: the interrupt is delayed by one character time minus the last stop bit time if at least 2 bytes have not been in the TX FIFO at the same time since it last went empty.
    - "The first transmitter interrupt after changing FCR0 is immediate if enabled" (p34).
  - **FIFO-polled mode**: FCR0 = 1 with the IER bits 0–3 cleared. LSR works as usual (LSR0 set while ≥1 byte is in the FIFO, LSR7 errors-in-FIFO). No trigger or timeout indication (p34–35).
  - **DMA pins** (PT/PFB only, p9, Figs 11–12, 14–15):
    - Mode 0 = FCR0 = 0, or FCR0 = 1 with FCR3 = 0. Mode 1 = FCR0 = 1 with FCR3 = 1. 450 mode allows only mode 0.
    - RXRDY mode 0: low while ≥1 character is in the RBR/FIFO; high when none remain.
    - RXRDY mode 1: goes low when the trigger level or timeout is reached; goes high when the FIFO is empty.
    - TXRDY mode 0 (Fig 14): goes high td20 after a THR write (byte 1); goes low td21 (≤ 9 baudout cycles) after the start bit, once the THR/FIFO is empty again.
    - TXRDY mode 1 (Fig 15): goes high td20 after the write that fills the FIFO (byte 16); goes low td21 after the next start bit, when a FIFO location frees up.
- **Interrupts** (Table 5 p35–36). INTRPT is active high and push-pull; no 3-state or OUT2 gating is described in this datasheet. INTRPT is low after MR.

  | IIR[3:0] | Priority | Type | Source | Reset by |
  |---|---|---|---|---|
  | 0001 | – | none | – | – |
  | 0110 | 1 (highest) | receiver line status | OE, PE, FE, BI | reading LSR |
  | 0100 | 2 | received data available | data in RBR (450 mode) or trigger level reached (FIFO) | reading RBR (FIFO: drop below trigger) |
  | 1100 | 2 | character timeout | FIFO mode only | reading RBR |
  | 0010 | 3 | THRE | THR/FIFO empty | reading IIR (if THRE is the source) or writing THR |
  | 0000 | 4 | modem status | CTS, DSR, RI, DCD (ΔCTS not when AFE) | reading MSR |
- **Modem lines and loopback** (p38):
  - MCR bits 0–3 drive DTR, RTS, OUT1, OUT2 low when set.
  - LOOP = 1 does all of the following:
    - SOUT is forced high.
    - SIN is disconnected.
    - The TSR output loops into the RSR.
    - The CTS/DSR/DCD/RI pins are disconnected; internally RTS→CTS, DTR→DSR, OUT1→RI, OUT2→DCD (per MSR bits 4–7).
    - The DTR/RTS/OUT1/OUT2 pins are forced inactive (high).
    - Modem interrupts come from MCR[3:0], and all interrupts still work under IER control.
- **Autoflow** (p3–4, p38, Figs 2–4, 17–19):
  - Enable: MCR5 = 1 with MCR1 = 1 gives auto-RTS + auto-CTS; MCR5 = 1 with MCR1 = 0 gives auto-CTS only.
  - Auto-CTS: the transmitter checks CTS before sending each byte. If CTS goes high (inactive) more than tsu4 = 10 ns before the middle of the last stop bit, the current byte finishes and the next is held. When CTS goes low again, SOUT's start bit follows within td25 ≤ 24 baudout cycles. CTS changes do not raise modem interrupts while autoflow is on.
  - Auto-RTS with trigger 1, 4 or 8:
    - RTS goes high (inactive) when the RX FIFO reaches the trigger level, within td26 ≤ 2 baudout cycles after the stop-bit midpoint of the threshold byte.
    - The sender may still send one more byte.
    - RTS goes low again once the RX FIFO is emptied by RBR reads, td27 ≤ 2 baudout cycles after the read of the last byte.
  - Auto-RTS with trigger 14:
    - RTS goes high after the first data bit of the 16th character (td28 ≤ 2 baudout cycles from the mid of data bit 0).
    - It goes low again when at least one byte of space is free and no byte is incoming, or when more than one byte of space is free.
    - When the FIFO is full, the first RBR read re-asserts RTS (td29 ≤ 2 baudout cycles after RBRRD low).
- **Extras**: no sleep, IrDA, RS-485, GPIO or device ID. OUT1/OUT2 are user outputs (PT/PFB only). SCR is a scratch register.
- **Reset** (MR is active high, minimum width 1 µs; Table 2 p32):
  - Cleared: IER, FCR, LCR, MCR.
  - IIR = 0x01. LSR = 0x60. MSR[3:0] = 0.
  - SCR, DLL/DLM, RBR and THR are not affected. Both FIFOs are cleared.
  - Pins: SOUT, OUT1, OUT2, RTS and DTR go high; INTRPT goes low.
  - After reset the chip is in 450 (character) mode.
  - The datasheet gives no reset state for DDIS, TXRDY or RXRDY. By the pin rules: DDIS is high (not reading); RXRDY is high (empty); TXRDY is low (empty, mode 0). That is inference.
- **Undefined / reserved**: FCR4–5 are reserved. Writes to LSR are factory-test only. IER4–7 are always 0. MSR inputs on ZQS (missing pins) are not specified.

### Timing (p16–18)
The tables are min/max "over recommended ranges of supply voltage". There is **one** table for every VCC band; only the clock pulse widths are split by band. No typicals are printed.

| Symbol | Parameter | Value | Page |
|---|---|---|---|
| tw1 / tw2 | XIN high / low | ≥ 25 ns (2.5 V, 16 MHz); ≥ 20 ns (3.3 V, 20 MHz); ≥ 18 ns (5 V, 24 MHz); ≥ 8 ns (3.3 V ZQS, 48 MHz) | p16 |
| tcR / tcW | read / write cycle | ≥ 87 ns | p16 |
| — | FIFO mode: read cycle between RX FIFO and status (IIR/LSR) reads | ≥ 425 ns | p17 note |
| tw5 | ADS low | ≥ 9 ns | p16 |
| tw6 | WR pulse | ≥ 40 ns | p16 |
| tw7 | RD pulse | ≥ 40 ns | p16 |
| tw8 | MR pulse | ≥ 1 µs | p16 |
| tsu1 / tsu2 | address / CS setup to ADS↑ | ≥ 8 ns | p16 |
| th1 / th2 | address / CS hold after ADS↑ | ≥ 0 ns | p16 |
| tsu3 | data setup before WR1↑/WR2↓ | ≥ 15 ns | p16 |
| th5 | data hold after WR↑ | ≥ 5 ns | p16 |
| th3 / th4 | CS / address hold after WR end | ≥ 10 ns | p16 |
| th6 | CS hold after RD end | ≥ 10 ns | p16 |
| th7 | address hold after RD end | ≥ 20 ns | p16 |
| td4 / td5 | CS / address valid before WR start (ADS low) | ≥ 7 ns | p16 |
| td7 / td8 | CS / address valid before RD start | ≥ 7 ns | p16 |
| td6 / td9 | WR / RD end to next ADS↓ | ≥ 40 ns | p16 |
| td10 | RD active to data valid | max 45 ns (CL = 75 pF) | p16 |
| td11 | RD inactive to data float | max 20 ns | p16 |
| tdis(R) | RD to DDIS | max 20 ns | p17 |
| tw3 / tw4 | BAUDOUT low / high (24 MHz, ÷2, 5 V) | ≥ 35 ns | p17 |
| td1 / td2 | XIN to BAUDOUT ↑ / ↓ | max 45 ns | p17 |
| td12 | RCLK to sample | max 10 ns | p17 |
| td13 | stop bit to INTRPT, or read RBR to LSI interrupt, or stop bit to RXRDY↓ | max 1 RCLK cycle (9 RCLKs for timeout) | p17, p22 |
| td14 | read RBR/LSR to INTRPT reset | max 70 ns | p17 |
| td15 | initial write to transmit start | 8–24 baudout cycles | p17 |
| td16 | start to INTRPT (THRE) | 8–10 baudout cycles | p17 |
| td17 | WR THR to INTRPT reset | max 50 ns | p17 |
| td18 | initial write to INTRPT (THRE) | 16–34 baudout cycles | p17 |
| td19 | read IIR to INTRPT reset (THRE) | max 35 ns | p17 |
| td20 | write to TXRDY inactive | max 35 ns | p17 |
| td21 | start to TXRDY active | max 9 baudout cycles | p17 |
| td22 | write MCR to output | max 50 ns | p18 |
| td23 | modem input change to INTRPT | max 35 ns | p18 |
| td24 | read MSR to INTRPT reset | max 40 ns | p18 |
| tsu4 | CTS↑ before stop-bit midpoint | ≥ 10 ns | p16 |
| td25 | CTS low to SOUT↓ | max 24 baudout cycles | p18 |
| td26 / td27 / td28 / td29 | auto-RTS delays (above) | max 2 baudout cycles each | p18 |

### VCC (p13–14)
- The ROC table has three bands:
  - 2.25–2.75 V: f ≤ 16 MHz, VIH 1.8 V, VIL 0.6 V.
  - 3.0–3.6 V: f ≤ 20 MHz (ZQS 48 MHz), VIH 0.7 VCC, VIL 0.3 VCC.
  - 4.5–5.5 V: f ≤ 24 MHz, VIH 2 V (XIN 0.7 VCC), VIL 0.8 V (XIN 0.3 VCC).
- The pin tables say "2.25-V to 5.5-V". The gaps between bands (2.75–3.0 V and 3.6–4.5 V) are not characterized.
- Input voltage in ROC is 0…VCC, so the inputs are **not** specified as 5 V-tolerant above VCC: inputMax = VCC. Absolute max for VI is 7 V.
- Temperature: TL16C550D 0–70 °C; TL16C550DI −40–85 °C.

### Package drawings
- **PT0048A** LQFP-48 (outline p57, land pattern p58, stencil p59):
  - Pitch 0.5. Body 6.8–7.2 (7.0). Span tip-to-tip 8.8–9.2 (9.0).
  - Lead width 0.17–0.27 (0.22). Foot length 0.45–0.75 (0.60). Height 1.6 max (body 1.35–1.45).
  - Land pattern: 48× pads 1.6 × 0.3, pad-row center span (8.2) both axes.
- **PFB0048A** TQFP-48 (outline p54, land pattern p55, stencil p56):
  - Pitch 0.5. Body 6.8–7.2 (7.0). Span 8.8–9.2 (9.0).
  - Lead width 0.17–0.27 (0.22). Foot 0.45–0.75 (0.60). Height 1.2 max.
  - Land pattern: 48× pads 1.35 × 0.25, pad-row center span (8.5).
- **RHB0032E** VQFN-32 (generic view p50, outline p51, land pattern p52, stencil p53):
  - Body 4.9–5.1 (5.0). Pitch 0.5 (corner pin centers at 2× 3.5).
  - Terminal width 0.2–0.3 (0.25). Terminal length 0.3–0.5 (0.4). Height 1.0 max.
  - Exposed pad (pin 33) 3.45 ± 0.1 square, must be soldered.
  - Land pattern: 32× pads 0.6 × 0.25, pad-row center span (4.8), thermal land 3.45, optional 0.2 vias.
  - Stencil: 4 × 1.49 windows (75 % coverage).
- **ZQS** (24-ball MicroStar Junior, 3 × 3 mm per DigiKey): **no outline in this PDF.** Only the ball map on p2 is given (5 × 5 grid with B2 empty). The pitch and dimensions must come from TI's MicroStar Junior ZQS drawing elsewhere. Both ZQS census parts are obsolete.

### Obsolete
TL16C550DIZQSR and TL16C550DZQSR (DigiKey status Obsolete; also missing from TI's package addendum).

### Notes / datasheet errors
- **RHB pin table (p10)**: DSR 39, DTR 33, INTRPT 30 and MR 35 are copied from the PT table. The real RHB pins are 25, 22, 20 and 23 (p2 drawing, p6 block diagram).
- **RHB NC list**: it is truncated as "2, 9, 15, 16".
- **RHB CS2 text**: the RHB CS2 row still describes CS0/CS1.
- **LSR7 (p37)**: the text says "In the TL16C550D mode, this bit is always cleared". That is a typo: in FIFO mode it is the error-in-FIFO flag.
- **Pin label typo**: the baud table spells BAUDOUT as "BADOUT".
- **Baud clock range (p39)**: the text says "dc to 16 MHz", which conflicts with the 24 MHz limit at 5 V.
- **Electrical test conditions (p14–15)**: the 2.5 V and 5 V columns quote VCC = 3.6 V. That is a copy error; it does not matter for the logic.
- **Bus protocol**: identical to the TL16C550C, so the TL16C550C model can be shared, using a supply-band-dependent clock limit and the new packages.

## PC16550D

- **Title:** "PC16550D Universal Asynchronous Receiver/Transmitter with FIFOs" (National Semiconductor, June 1995; the PDF has a TI cover page).
- **Datasheet:** `datasheets/controller-uart/pc16550d.pdf`, 27 pages. PDF page = printed page + 1 (PDF p1 is the TI cover). Below, "p" means the printed page and "PDF" the PDF page.
  - PDF p24–27 is a 2001 National product-folder web page. It holds the package/ordering table.
- **Function:** a 16450-compatible UART with a 16-byte TX FIFO and a 16-byte RX FIFO. The RX FIFO also stores 3 error bits per byte. Powers up in 16450 ("character") mode.
- **Pins vs. 16450:** pin-compatible with the 16450 except DIP pin 24, which was CSOUT and is now TXRDY, and DIP pin 29, which was NC and is now RXRDY (p1).
- **Nearest sibling:** TL16C550B/C, which has the same 16550 register set.
  - PC16550D has no autoflow (MCR[5] is permanently 0) and no MCR[5]-based AFE.
  - It has the original 16550 "transmitter interrupt delay" rule (see FIFO section).
  - It is functionally the same 16550 register model as TL16C550B.

### Parts

| Census part | Status | Package | Pins | Pinout | Grade | Supply |
|---|---|---|---|---|---|---|
| PC16550DV (TI row) | **Obsolete** | V = 44-lead plastic chip carrier, NS V44A | 44 | plcc44 | 0…70 °C | 4.5–5.5 V |
| PC16550DV (National row) | Active | V44A | 44 | plcc44 | 0…70 °C | 4.5–5.5 V |
| PC16550DVX (National row) | Active | V44A on reel ("reel of 500", PDF p26 package table) | 44 | plcc44 | 0…70 °C | 4.5–5.5 V |
| PC16550DVX (TI row) | **Obsolete** | V44A on reel | 44 | plcc44 | 0…70 °C | 4.5–5.5 V |

- Package-letter decode comes from the order numbers on the connection diagrams (p12–13): N = PC16550DN 40-DIP (N40A), V = PC16550DV 44-PLCC (V44A), VEF = PC16550DVEF 44-TQFP (VEF44A).
- The X suffix means reel: in the PDF p26 table, PC16550DV ships in "tube of 25" and PC16550DVX in "reel of 500", both with marking "PC16550DV".
- There is only one grade: TA 0…+70 °C, VDD = +5 V ±10% (p3 DC and p4 AC headers).
- Every census part is decoded.

### Pinouts

**dip40 (N40A, p12; for reference only, no census part)**
| Pin | Name | Role |
|---|---|---|
| 1–8 | D0–D7 | inout |
| 9 | RCLK | input |
| 10 | SIN | input |
| 11 | SOUT | output |
| 12 | CS0 | input |
| 13 | CS1 | input |
| 14 | CS2 | input |
| 15 | BAUDOUT | output |
| 16 | XIN | input |
| 17 | XOUT | output |
| 18 | WRBAR | input |
| 19 | WR | input |
| 20 | VSS | ground |
| 21 | RDBAR | input |
| 22 | RD | input |
| 23 | DDIS | output |
| 24 | TXRDY | output |
| 25 | ADS | input |
| 26 | A2 | input |
| 27 | A1 | input |
| 28 | A0 | input |
| 29 | RXRDY | output |
| 30 | INTR | output |
| 31 | OUT2 | output |
| 32 | RTS | output |
| 33 | DTR | output |
| 34 | OUT1 | output |
| 35 | MR | input |
| 36 | CTS | input |
| 37 | DSR | input |
| 38 | DCD | input |
| 39 | RI | input |
| 40 | VDD | power |

**plcc44 (V44A, p13 "Chip Carrier Package", top view): every census part**

`number name role`:
```
1 NC nc
2 D0 inout
3 D1 inout
4 D2 inout
5 D3 inout
6 D4 inout
7 D5 inout
8 D6 inout
9 D7 inout
10 RCLK input
11 SIN input
12 NC nc
13 SOUT output
14 CS0 input
15 CS1 input
16 CS2 input
17 BAUDOUT output
18 XIN input
19 XOUT output
20 WRBAR input
21 WR input
22 VSS ground
23 NC nc
24 RDBAR input
25 RD input
26 DDIS output
27 TXRDY output
28 ADS input
29 A2 input
30 A1 input
31 A0 input
32 RXRDY output
33 INTR output
34 NC nc
35 OUT2 output
36 RTS output
37 DTR output
38 OUT1 output
39 MR input
40 CTS input
41 DSR input
42 DCD input
43 RI input
44 VDD power
```

- **How the pins map:** PLCC = DIP + 1 for DIP pins 1–10, +2 for 11–20, +3 for 21–30, +4 for 31–40. The NCs sit at 1, 12, 23 and 34.
- **Overbar pairs:**
  - WRBAR/RDBAR are the active-low strobes (DIP 18/21, PLCC 20/24); WR/RD are the active-high ones (DIP 19/22, PLCC 21/25), per p12 "RD, RD, Read, Pins 22 and 21: When RD is high or RD is low…".
  - Overbars on the drawings: CS2, BAUDOUT, ADS, TXRDY, RXRDY, OUT1, OUT2, DTR, RTS, CTS, DSR, DCD, RI. MR and INTR have none.
- **No thermal pad.**
- **No mode-select pins.** The FIFO/16450 mode is software-selected (FCR0).
- **VEF44A TQFP pinout (p13):** printed but not a census part, so not listed.

### Host interface

- **Bus type:** Intel-style 8-bit, with dual-polarity read and write strobes and an optional address latch.
- **Chip select:** selected when CS0 = 1, CS1 = 1 and CS2 = 0 (p11).
- **Address:** A2..A0 select the register, together with DLAB = LCR[7].
- **Read:** happens while (RD = 1 or RDBAR = 0) and the chip is selected.
  - "Only an active RD or RD input is required… tie either the RD input permanently low or the RD input permanently high, when it is not used" (p12).
  - The effective read strobe is therefore RD OR NOT RDBAR (my reading); the unused one is tied inactive.
- **Write:** same rule with WR/WRBAR.
  - Data is latched at the trailing (inactive-going) edge of the write strobe: tDS/tDH are referenced to the end of the WR pulse (Write Cycle waveform, p6).
- **ADS (active low):**
  - "The positive edge of an active Address Strobe (ADS) signal latches the Register Select (A0, A1, A2) and Chip Select (CS0, CS1, CS2) signals" (p11).
  - While ADS is low the address and select paths are transparent. Tie ADS low when the address and selects are stable for the whole cycle.
  - Waveform (p6): tAS/tCS are setup and tAH/tCH are hold around the rising edge of ADS.
- **Data bus:** 3-state. It drives only during a selected read (tRVD after the read strobe; floats tHZ after the read strobe ends).
- **DDIS:** "goes low whenever the CPU is reading data from the UART" (p11). It is high otherwise and is meant to disable or turn around an external transceiver.
  - Waveform: DDIS falls tRDD after the read strobe goes active and rises tRDD after it ends.
- **CSOUT:** not present (that pin became TXRDY).

### Registers (p10 "Register Addresses", p14 Table II, p13 Table I reset)

| DLAB | A2A1A0 | Name | R/W | Reset |
|---|---|---|---|---|
| 0 | 000 | RBR, receiver buffer | R | not reset |
| 0 | 000 | THR, transmitter holding | W | not reset |
| 0 | 001 | IER | R/W | 0x00 |
| x | 010 | IIR | R | 0x01 |
| x | 010 | FCR | W | 0x00 |
| x | 011 | LCR | R/W | 0x00 |
| x | 100 | MCR | R/W | 0x00 |
| x | 101 | LSR | R (writes for factory test only) | 0x60 |
| x | 110 | MSR | R | bits 3..0 = 0, bits 7..4 = inputs |
| x | 111 | SCR, scratch | R/W | not stated |
| 1 | 000 | DLL, divisor LSB | R/W | not reset |
| 1 | 001 | DLM, divisor MSB | R/W | not reset |

MR "clears all the registers (except the Receiver Buffer, Transmitter Holding, and Divisor Latches)" (p10). Table I does not list the SCR.

**Bits:**

**RBR/THR:** data bits 0..7. Bit 0 is sent and received first.

**IER:**
- b0 ERBFI: received-data-available interrupt, plus the timeout interrupt in FIFO mode.
- b1 ETBEI: THRE interrupt.
- b2 ELSI: receiver line status interrupt.
- b3 EDSSI: modem status interrupt.
- b7..4 are always 0. Table I shows them as permanently-low bits.

**IIR:**
- b0 = 0 when an interrupt is pending.
- b2..1 give the interrupt ID (see the interrupt table).
- b3 is 0 in 16450 mode. In FIFO mode it is set together with b2 when a timeout interrupt is pending.
- b5..4 = 0.
- b7..6 = 1 when FCR0 = 1 ("FIFOs Enabled"), otherwise 0.

**FCR (write-only):**
- b0 FIFO enable.
  - Writing 1 enables both FIFOs. Resetting FCR0 clears all bytes in both FIFOs.
  - Changing between FIFO mode and 16450 mode clears the FIFOs.
  - FCR0 "must be a 1 when other FCR bits are written to or they will not be programmed."
- b1 RCVR FIFO reset: clears the RX FIFO and its counter. Self-clearing; the shift register is not cleared.
- b2 XMIT FIFO reset: the same for the TX FIFO.
- b3 DMA mode select: RXRDY/TXRDY switch from mode 0 to mode 1 when FCR0 = 1.
- b5..4 reserved.
- b7..6 RX trigger level: 00 → 1, 01 → 4, 10 → 8, 11 → 14 bytes (p16).

**LCR:**
- b1..0 WLS: 00 = 5, 01 = 6, 10 = 7, 11 = 8 bits.
- b2 STB:
  - 0 = 1 stop bit.
  - 1 = 1.5 stop bits when the word is 5 bits, 2 stop bits when it is 6–8 bits.
  - "The Receiver checks the first Stop-bit only."
- b3 PEN: parity bit between the last data bit and the stop bit.
- b4 EPS: 0 = odd, 1 = even (when PEN = 1).
- b5 stick parity: with PEN = 1 and EPS = 1 the parity bit is sent and checked as 0; with PEN = 1 and EPS = 0 it is sent and checked as 1. Disabled when 0.
- b6 Set Break: forces SOUT to 0. Acts only on SOUT; the transmitter logic keeps running.
- b7 DLAB.

**MCR:**
- b0 DTR: 1 drives the DTR pin low.
- b1 RTS: 1 drives the RTS pin low.
- b2 OUT1: 1 drives the OUT1 pin low.
- b3 OUT2: 1 drives the OUT2 pin low.
- b4 LOOP.
- b7..5 permanently 0. Table I shows MCR bits 7..4 in bold as permanently low, which conflicts with b4 being LOOP. The text p17 says "Bits 5 through 7: These bits are permanently set to logic 0", so only 7..5 are fixed.

**LSR:**
- b0 DR: set when a complete character is transferred into the RBR or the FIFO. Cleared when all data in the RBR or FIFO has been read.
- b1 OE: see Behavior. Cleared by reading LSR.
- b2 PE: cleared by reading LSR. In FIFO mode it belongs to its character and is shown when that character is at the top of the FIFO.
- b3 FE: stop bit sampled as 0. Same clear rule and FIFO association as PE.
- b4 BI: SIN held at 0 for longer than a full word time (start + data + parity + stop). Same clear rule and FIFO association.
- b5 THRE:
  - Set when THR moves to TSR. Cleared when the CPU loads THR.
  - In FIFO mode it is set when the XMIT FIFO is empty and cleared when at least one byte is written.
- b6 TEMT: set when THR and TSR are both empty (FIFO mode: the XMIT FIFO and TSR). Cleared when either holds a character.
- b7: 0 in 16450 mode.
  - In FIFO mode it is set when at least one PE, FE or BI is in the FIFO.
  - Reading LSR clears it if there are no subsequent errors in the FIFO.
- Writing LSR: "not recommended as this operation is only used for factory testing. In the FIFO mode the software must load a data byte in the Rx FIFO via Loopback Mode in order to write to LSR2–LSR4. LSR0 and LSR7 can't be written to in FIFO mode" (p16).

**MSR:**
- b0 DCTS: CTS changed since the last MSR read.
- b1 DDSR: DSR changed.
- b2 TERI: RI went from low to high (the trailing edge of the ring).
- b3 DDCD: DCD changed.
- b4 CTS = NOT CTS pin (= MCR RTS in loop).
- b5 DSR = NOT DSR pin (= MCR DTR in loop).
- b6 RI = NOT RI pin (= MCR OUT1 in loop).
- b7 DCD = NOT DCD pin (= MCR OUT2 in loop).
- Reading MSR clears b3..0.
- The modem interrupt fires whenever any of b3..0 is 1 and IER3 = 1 ("Whenever bit 0, 1, 2, or 3 is set to logic 1, a MODEM Status Interrupt is generated", p17).

**SCR:** 8-bit read/write; it has no effect on the UART.

**DLL/DLM:** the 16-bit divisor.

### Behavior

**Baud generator (p15)**
- **Clock source:** XIN (crystal between XIN and XOUT, or an external clock into XIN with XOUT unused).
  - Input clock range: DC to 24 MHz (p15; the waveform title "External Clock Input (24.0 MHz Max.)", p4).
- **Formula:** divisor = fIN / (baud × 16), with N = 1…2^16−1 (p4 table).
  - The text p15 says "any divisor from 2 to 2^16−1" while the table and features say 1. "Using a divisor of zero is not recommended."
- **Loading:** writing either divisor latch immediately loads the 16-bit baud counter.
- **BAUDOUT:**
  - It is the 16× clock (fIN/N) and drives the transmitter internally.
  - The receiver uses RCLK; tie BAUDOUT to RCLK to use the same rate.
  - BAUDOUT is active-low on the drawing. Waveform p4: tHW/tLW are specified for fX = 8 MHz, ÷2.
- **Sampling:** 16×.
- **Baud tables:** Table III (p15) gives divisors for 1.8432, 3.072 and 18.432 MHz. The 18.432 MHz column for 1200 baud prints "920", which should be 960.

**Transmitter and receiver**
- **Frame:** start (0), 5–8 data bits LSB first, optional parity, then 1, 1.5 or 2 stops. Idle and mark are 1.
- **Receiver stop check:** only the first stop bit is checked.
- **Start-bit validation:** "False start bit detection" (p1). The sample point is not stated in this PDF; the classic 16450 mid-bit sample (count 7½ of 16) is described in the 82C50A sheet. tSCD ≤ 2000 ns from RCLK to the sample time.
- **Framing-error resync:** after an FE the UART "assumes that the framing error was due to the next start bit, so it samples this 'start' bit twice and then takes in the 'data'" (p16).
- **Break:**
  - When a break occurs, only one zero character is loaded into the FIFO.
  - The next transfer is enabled after SIN goes to mark and a valid start bit is received.
- **Overrun:**
  - 16450 mode: RBR overwritten.
  - FIFO mode: OE happens only when the FIFO is full and the next character is complete in the shift register. OE is indicated immediately; the shift-register character is overwritten and not transferred.
- **Break-send procedure (p15):** load a 0x00 pad on THRE, set break after the next THRE, wait for TEMT, then clear break.

**FIFOs (p16–18)**
- **Depth:** 16 TX × 8 and 16 RX × (8 + 3 error bits).
- **Trigger levels:** 1, 4, 8 or 14.
- **Enable and reset:** FCR0 enables. Toggling FCR0 clears both FIFOs. FCR1 and FCR2 clear their FIFOs; these bits are self-clearing.
- **RX interrupt mode** (FCR0 = 1, IER0 = 1):
  - The RDA interrupt and IIR indication come on when the FIFO reaches the trigger level and clear as soon as it drops below.
  - The RLS interrupt (IIR = 06) has higher priority.
  - DR is set as soon as a character enters the FIFO and cleared when the FIFO is empty.
- **Timeout:**
  - Fires when at least one character is in the FIFO, the last character was received more than 4 continuous character times ago (the second stop bit counts if 2 stops are programmed), and the last CPU read of the FIFO was more than 4 character times ago.
  - Character times are computed from RCLK.
  - It is cleared, and the timer reset, by reading one character.
  - With no timeout pending, the timer resets on each new character received or each CPU read.
  - "Maximum… 160 ms at 300 baud with a 12-bit character."
- **TX interrupt (FIFO mode):**
  - The THRE interrupt (IIR 02) occurs when the XMIT FIFO is empty. It clears on a THR write (1–16 bytes may be written while servicing) or an IIR read.
  - Transmitter interrupt delay: "The transmitter FIFO empty indications will be delayed 1 character time minus the last stop bit time whenever the following occurs: THRE = 1 and there have not been at least two bytes at the same time in the transmit FIFO since the last THRE = 1."
  - "The first transmitter interrupt after changing FCR0 will be immediate, if it is enabled" (p18).
- **Priorities:** timeout and trigger have the same priority as RDA; XMIT FIFO empty has the same priority as THRE.
- **Polled mode:** FCR0 = 1 with IER bits 0. The RX and TX sides are polled separately. There is no trigger or timeout indication, but the FIFOs still hold characters.
- **RX status delays (p4 Note 2):** in FIFO mode, trigger-level interrupts, RDA, active RXRDY and OE are delayed 3 RCLKs.
  - PE/FE/BI are delayed 3 RCLKs after the first byte; for later bytes they update immediately after the RBR read goes inactive.
  - The timeout interrupt is delayed 8 RCLKs.

**DMA pins (p12, waveforms p8–9)**
- **RXRDY (active low):**
  - Mode 0 (16450 mode, or FCR0 = 1 with FCR3 = 0): low when at least 1 character is in the RBR or FIFO; high when none remain.
  - Mode 1 (FCR0 = 1, FCR3 = 1): low when the trigger level or timeout is reached; high when the FIFO is empty.
  - tRXI ≤ 290 ns from the RBR read to RXRDY inactive.
- **TXRDY (active low):**
  - Mode 0: low when there is no character in the THR or XMIT FIFO; high after the first character is loaded.
  - Mode 1: low when the XMIT FIFO is empty; high when the XMIT FIFO is completely full.
  - tWXI ≤ 195 ns from a write to TXRDY inactive. tSXA ≤ 8 BAUDOUT cycles from start to TXRDY active.
- In 16450 mode only mode 0 is allowed.

**Interrupts (Table IV, p16)**

| IIR[3:0] | Priority | Type | Source | Reset |
|---|---|---|---|---|
| 0001 | – | none | – | – |
| 0110 | highest | Receiver line status | OE, PE, FE or BI | read LSR |
| 0100 | 2nd | Received data available | data available or trigger level reached | read RBR, or FIFO drops below trigger |
| 1100 | 2nd | Character timeout (FIFO only) | no characters in or out for 4 character times with ≥1 character in the FIFO | read RBR |
| 0010 | 3rd | THRE | THR empty | read IIR (if it is the source) or write THR |
| 0000 | 4th | Modem status | CTS, DSR, RI or DCD | read MSR |

- **IIR freeze:** while the CPU accesses IIR, the UART freezes interrupts and does not change the indication until the access completes; new interrupts are recorded (p17).
- **INTR pin:** active high and push-pull. It goes high when any enabled condition is active and is reset low by service or MR (p10).
- **OUT2 gating:** the PDF does not describe OUT2 gating INTR. INTR is always driven.
- **Interrupt timing:**
  - THRE: tSI from the initial write to the interrupt is 16–24 BAUDOUT cycles. tSTI from stop to the THRE interrupt is 8 cycles. Both are lengthened by 1 character time minus the last stop bit when the transmitter-interrupt delay is active (p5 Note 1).

**Modem and loopback (p17–18)**
- **Outputs:** the MCR bits drive the pins inverted.
- **Inputs:** the MSR status bits are the complements of the pins.
- **Effects:** CTS has no effect on the transmitter and DCD has no effect on the receiver (p10–11).
- **Loopback (MCR4 = 1):**
  - SOUT is forced to mark (1) and SIN is disconnected. The TSR output feeds the RSR input.
  - The CTS/DSR/RI/DCD pins are disconnected. Internally RTS → CTS, DTR → DSR, OUT1 → RI, OUT2 → DCD.
  - The DTR/RTS/OUT1/OUT2 pins are forced high (inactive).
  - The RX and TX interrupts are fully operational. The modem interrupts are sourced from MCR[3:0] and still gated by IER.

**Reset (MR, active high, Schmitt; Table I p13)**
- **Registers:** IER = 0, IIR = 0x01, FCR = 0, LCR = 0, MCR = 0, LSR = 0x60, MSR = 0000 in the low nibble with the high nibble following the inputs. The RX and TX FIFOs are cleared.
- **Outputs:** SOUT high; INTR low; OUT1, OUT2, RTS and DTR high.
- **Not stated:** DDIS (high, since no read is in progress), RXRDY (high, since no data), and TXRDY (low, since the THR is empty, per the mode-0 rule) are inferred.
- **Pulse width:** tMR ≥ 5000 ns.

**Undefined / reserved**
- FCR[5:4] are reserved.
- Writing LSR is for factory test only.
- A divisor of 0 is not recommended.
- The SCR reset value is not stated.

### Timing (p4–5, TA 0–70 °C, VDD 5 V ±10%; all min/max, no typicals)

**Bus timing**

| Parameter | Value | Note |
|---|---|---|
| tADS, ADS width | min 60 ns | |
| tAS, address setup | min 60 ns | |
| tAH, address hold | min 0 ns | |
| tCS, CS setup | min 60 ns | |
| tCH, CS hold | min 0 ns | |
| tAR / tAW, address to RD/WR | min 30 ns | ADS tied low |
| tCSR / tCSW, CS to RD/WR | min 30 ns | ADS tied low |
| tRA / tWA, address hold from RD/WR | min 20 ns | |
| tRCS / tWCS, CS hold from RD/WR | min 20 ns | |
| tRD, read strobe width | min 125 ns | |
| tRVD, read to data | max 60 ns | 100 pF |
| tHZ, read end to float | 0–100 ns | |
| tRDD, read to DDIS | max 60 ns | |
| tRC, read recovery | min 125 ns | |
| RC | min 280 ns | |
| tWR, write strobe width | min 100 ns | |
| tDS, data setup | min 30 ns | |
| tDH, data hold | min 30 ns | |
| tWC, write recovery | min 150 ns | |
| WC | min 280 ns | |
| tMR | min 5000 ns | |
| tXH / tXL, clock high/low | min 55 ns | condition printed "External Clock (8, Max.)", i.e. at 8 MHz |

**Baud generator**
- tBHD / tBLD: max 175 ns.
- tHW: min 75 ns. tLW: min 100 ns.

**Receiver**
- tRINT (read RBR or LSR to interrupt reset): max 1000 ns.
- tRXI: max 290 ns.
- tSCD: max 2000 ns.
- tSINT (stop to set interrupt): 1 RCLK cycle. In FIFO mode it is 3 RCLKs, or 8 for a timeout (p7 Note 2).
- tRAI is printed as "—".

**Transmitter (p5)**
- tHR (write THR to interrupt reset): max 175 ns.
- tIR (read IIR to THRE reset): max 250 ns.
- tIRS (initial INTR reset to transmit start): 8–24 BAUDOUT cycles.
- tSI: 16–24 cycles.
- tSTI: 8 cycles (min = max = 8).
- tSXA: max 8 cycles.
- tWXI: max 195 ns.

**Modem**
- tMDO (write MCR to output): max 200 ns.
- tRIM (read MSR to interrupt reset): max 250 ns.
- tSIM (modem input to interrupt): max 250 ns.

p4 Note 4 says "These specifications are preliminary."

### VCC

- **Supply:** VDD = 5 V ±10%, i.e. 4.5–5.5 V, at 0–70 °C (p3).
- **Inputs:**
  - VIH max = VDD. Absolute maximum for any pin is −0.5 to +7.0 V (p3).
  - They are not 5 V-tolerant above VDD in any special sense; the part is a 5 V part.
  - Inputs are TTL-level (VIH 2.0 V, VIL 0.8 V). MR is a Schmitt trigger with 2.0 V and 0.8 V thresholds.

### Package drawing

- **Drawing:** NS V44A, "44-Lead Plastic Chip Carrier (V)" (p21 = PDF p22, rev K). No land pattern is printed.
- **Pitch:** 0.050 in (1.27 mm).
- **Body:** 0.650 +0.006/−0.000 in (16.51 mm).
- **Span over leads:** 0.690 −0.005 in (17.53 mm) typ.
- **Lead row-to-row at the contact (seating) line:** 0.610 ±0.020 in (15.49 mm) typ.
- **Lead width:** lower 0.017 ±0.004 in (0.43 mm); upper/shoulder 0.029 ±0.003 in (0.74 mm).
- **Height:** 0.165–0.180 in (4.19–4.57 mm). Standoff 0.105 ±0.015 in (2.67 mm); 0.020 in (0.51 mm) min to the seating plane.
- **Pin 1:** at the top centre; pins 1–6 run counter-clockwise to 7 at the left (top view, p13 and p21).
- **N40A DIP (reference only):** 0.100 in pitch, 0.600 in row spacing.

### Obsolete

- Obsolete: PC16550DV (TI row) and PC16550DVX (TI row).
- The National-manufacturer rows of the same part numbers are Active.

### Notes

- **Contradictions in the datasheet:**
  - Divisor minimum: 1 (p1, p4) vs 2 (p15).
  - MCR bit 4 shown as permanently low in Table I, although it is the LOOP bit.
  - Table III 18.432 MHz / 1200 baud prints 920, which should be 960.
- **INTR and OUT2:** INTR is not tri-stated by OUT2 on this chip. PC boards do the gating externally.
- **Effective strobes:** RD and RDBAR are ORed as active conditions (and the same for WR and WRBAR). Driving both active, or the unused one floating, is undefined. The datasheet only says to tie the unused one inactive.
- **Clock speeds:** 1.5 Mbaud requires a 24 MHz clock with a divisor of 1 (p15 note), even though tXH/tXL ≥ 55 ns only guarantees about 9 MHz. The 24 MHz figure comes from the waveform title and §8.3.

## 82C50A

- **Title:** "82C50A CMOS Asynchronous Communications Element" (Intersil data sheet FN2958.5, August 24, 2006).
- **Datasheet:** `datasheets/controller-uart/82c50a.pdf`, 25 pages. Printed page = PDF page.
- **Function:** a single-chip CMOS UART with a baud rate generator (the 8250/16450-class "ACE"): DC to 625 kbaud with a DC to 10 MHz clock.
  - No FIFOs, no DMA pins.
  - Has a scratch register and CSOUT.
- **Nearest sibling:** TL16C450. The register set is the same: RBR/THR/IER/IIR/LCR/MCR/LSR/MSR/SCR/DLL/DLM, with IIR bits 7..3 = 0.
  - Pinout is the same as the 16450 (DIP pin 24 = CSOUT, pin 29 = NC), but the pins have the Intel 8250 names: DISTR/DOSTR for RD/WR, XTAL1/XTAL2, INTRPT.
  - Differences from PC16550D: no FIFO/FCR, no TXRDY/RXRDY, and CSOUT is present.

### Parts

| Census part | Status | Package | Pins | Pinout | Grade | Supply |
|---|---|---|---|---|---|---|
| CS82C50A-5 (Harris) | Active | S = 44-lead PLCC, Intersil N44.65 | 44 | plcc44 | C = 0…+70 °C; -5 = 625 kbaud / 10 MHz | 4.5–5.5 V |
| CS82C50A-596 (Intersil) | **Obsolete** | N44.65 PLCC, tape and reel ("96") | 44 | plcc44 | 0…+70 °C; -5 | 4.5–5.5 V |

**How the part numbers decode** (Ordering Information p1; operating temperature p17):
- The prefix letter C/I is the temperature range: C = 0…+70 °C, I = −40…+85 °C. The datasheet writes this as "CX82C50A-5 0 °C to +70 °C; IX82C50A-5 −40 °C to +85 °C".
- The second letter is the package: P = 40 Ld PDIP (E40.6), S = 44 Ld PLCC (N44.65).
- "-5" is the only speed grade, under the "625K BAUD" column.
- A "Z" suffix means Pb-free. "96" means tape and reel.

**CS82C50A-5 itself:**
- It is not a row in the ordering table. The table lists CS82C50A-596, whose marking is "CS82C50A-5", plus CS82C50A-5Z and CS82C50A-5Z96.
- Using the scheme above it decodes as the PLCC, commercial-temperature, -5 grade part in non-reel (tube) packing, the older Harris order code.
- That decode is certain from the naming scheme but not printed as an orderable row in this revision.

### Pinouts (p3)

**dip40 (E40.6; reference only, no census part)**
```
1 D0 inout
2 D1 inout
3 D2 inout
4 D3 inout
5 D4 inout
6 D5 inout
7 D6 inout
8 D7 inout
9 RCLK input
10 SIN input
11 SOUT output
12 CS0 input
13 CS1 input
14 CS2 input
15 BAUDOUT output
16 XTAL1 input
17 XTAL2 output
18 DOSTRBAR input
19 DOSTR input
20 GND ground
21 DISTRBAR input
22 DISTR input
23 DDIS output
24 CSOUT output
25 ADS input
26 A2 input
27 A1 input
28 A0 input
29 NC nc
30 INTRPT output
31 OUT2 output
32 RTS output
33 DTR output
34 OUT1 output
35 MR input
36 CTS input
37 DSR input
38 DCD input
39 RI input
40 VCC power
```

**plcc44 (N44.65): every census part**
```
1 NC nc
2 D0 inout
3 D1 inout
4 D2 inout
5 D3 inout
6 D4 inout
7 D5 inout
8 D6 inout
9 D7 inout
10 RCLK input
11 SIN input
12 NC nc
13 SOUT output
14 CS0 input
15 CS1 input
16 CS2 input
17 BAUDOUT output
18 XTAL1 input
19 XTAL2 output
20 DOSTRBAR input
21 DOSTR input
22 GND ground
23 NC nc
24 DISTRBAR input
25 DISTR input
26 DDIS output
27 CSOUT output
28 ADS input
29 A2 input
30 A1 input
31 A0 input
32 NC nc
33 INTRPT output
34 NC nc
35 OUT2 output
36 RTS output
37 DTR output
38 OUT1 output
39 MR input
40 CTS input
41 DSR input
42 DCD input
43 RI input
44 VCC power
```

- **Pin names:** the PLCC drawing labels pin 33 "INTRP"; the same signal is INTRPT on the DIP.
- **Overbar pairs:** DOSTRBAR/DISTRBAR are the active-low strobes (DIP 18/21, PLCC 20/24). DOSTR/DISTR are active high (DIP 19/22, PLCC 21/25), per the Pin Description p4.
- **Overbars on the drawing:** CS2, BAUDOUT, ADS, OUT1, OUT2, DTR, RTS, CTS, DSR, DCD, RI. The PLCC drawing also puts an overbar on CS1 (pin 15) — see Notes.
- **Pin 29 (DIP):** the pin description says "NC 29 Do Not Connect".
- **No thermal pad.** No mode-select pins.

### Host interface (p4–5)

- **Bus type:** Intel 8-bit with dual-polarity strobes. Read is DISTR (active high) or DISTRBAR (active low); write is DOSTR (active high) or DOSTRBAR (active low).
  - "Only an active DISTR or DISTR, not both, is used… If DISTR is used as the read input, DISTR should be tied high" (inactive). The same applies to DOSTR.
- **Chip select:** selected when CS0 = 1, CS1 = 1 and CS2 = 0. The chip selects enable the strobes.
- **CSOUT (active high):** high when the chip is selected by CS0/CS1/CS2. "No data transfer can be initiated until CSOUT is a logic 1."
  - tCSC ≤ 125 ns from select to CSOUT (p18; the waveform also shows it in the ADS-latched case).
- **ADS (active low):**
  - "When ACTIVE (low), ADS latches the Register Select (A0, A1, A2) and Chip Select (CS0, CS1, CS2) inputs."
  - It is required when the address is not stable for the whole cycle ("multiplexed mode"). Otherwise tie it low ("non-multiplexed mode").
  - The waveforms (p21) show tAS/tCS setup and tAH/tCH hold referenced to the rising (trailing) edge of ADS, so the latch is transparent while ADS is low and holds on its rising edge.
  - p18 Note 1: in multiplexed mode the part works in 80C86/88 systems up to 3 MHz.
- **Address:** A2..A0 plus DLAB select the register (Table 1, p7).
- **Write:** data is latched at the trailing edge of the write strobe. tDS and tDH are around the end of the DOSTR pulse (Figure 6, p21).
- **Read:** D0–D7 drive tDDD after the read strobe goes active and float tHZ after it ends.
  - "These lines are normally in a high impedance state except during read operations."
  - For characters under 8 bits, the unused high RBR bits read 0.
- **DDIS:** low (INACTIVE) when the CPU is reading the chip, high otherwise. An active-high DDIS disables an external transceiver while the CPU reads. tDD ≤ 75 ns.
- **Status-bit timing during reads (p11):**
  - Setting LSR/MSR status bits is inhibited during a status-register read. A condition that arises during the read sets its bit at the trailing edge of the read.
  - If a bit was already set and the same condition recurs during the read, the bit is cleared at the trailing edge instead of being set again.
  - For the MSR delta bits: if a delta bit is already true and a change occurs during the read, that change is not shown. If it is false, the change is shown after the read.

### Registers (Table 1 p7, Table 3 p13, Table 7 p15)

| DLAB | A2A1A0 | Name | R/W | Reset (MR) |
|---|---|---|---|---|
| 0 | 000 | RBR | R | not affected |
| 0 | 000 | THR | W | not affected |
| 0 | 001 | IER | R/W | 0x00 |
| x | 010 | IIR | R | 0x01 |
| x | 011 | LCR | R/W | 0x00 |
| x | 100 | MCR | R/W | 0x00 |
| x | 101 | LSR | R | 0x60 |
| x | 110 | MSR | R | bits 3..0 = 0, bits 7..4 = inputs |
| x | 111 | SCR | R/W | not stated |
| 1 | 000 | DLL | R/W | not affected |
| 1 | 001 | DLM | R/W | not affected |

- The IER text calls it "a Write register", but Table 1 lists it as a normal register with no read-only or write-only qualifier. A 16450-class IER is readable.
- Address 010 has no write function (there is no FCR). Writing to it is undefined and not described.

**Bits:**

**IER (p13):**
- b0 ERBFI, received data available.
- b1 ETBEI, THRE.
- b2 ELSI, receiver line status.
- b3 EDSSI, modem status.
- b7..4 = 0.
- Disabling an interrupt inhibits its IIR indication and INTRPT. LSR and MSR still update.

**IIR (Table 2 p12):**
- b0 = 0 when an interrupt is pending.
- b2..1 give the ID.
- b7..3 = 0.

**LCR (p7–8):**
- b1..0 WLS: 5/6/7/8 data bits.
- b2 STB: 0 = 1 stop bit; 1 = 1.5 stop bits (5-bit words) or 2 stop bits (6–8-bit words). "The receiver checks for two stop bits if programmed" (p8). This differs from the 16550 wording.
- b3 PEN.
- b4 EPS: 1 = even.
- b5 stick parity: with PEN = 1 the parity bit is forced to 1 if EPS = 0 and to 0 if EPS = 1.
- b6 Set Break: SOUT = 0. Acts only on SOUT.
- b7 DLAB.

**MCR (p9–10):**
- b0 DTR, b1 RTS, b2 OUT1, b3 OUT2. Writing 1 drives the pin low.
- b4 LOOP.
- b7..5 = 0.

**LSR (p8–9):**
- b0 DR: set when a character is moved into the RBR (including a break). Cleared by reading RBR.
- b1 OE: RBR overwritten before it was read.
- b2 PE.
- b3 FE: stop bit low.
- b4 BI: SIN at space for longer than a full word time (start + data + parity + stop).
- Reading LSR clears b1–b4. Errors are set during reception of the stop bits. "The error flags are not reset by the absence of an error condition in the next received character. The flags reflect the last character only if no overrun occurred."
- b5 THRE: set when THR moves to TSR. Cleared by a THR write. Not cleared by an LSR read.
- b6 TEMT: set when THR and TSR are both empty. Cleared when a character is loaded into THR, and stays low until it has shifted out.
- b7 = 0.

**MSR (p10–11):**
- b0 DCTS, b1 DDSR.
- b2 TERI: RI went from low to high. High-to-low does not set it.
- b3 DDCD.
- b4 CTS, b5 DSR, b6 RI, b7 DCD: "inverted versions of the actual input pins".
- In loop mode: b4 = RTS, b5 = DTR, b6 = OUT1, b7 = OUT2 (taken from the MCR).
- Reading MSR clears b3..0.

**SCR:** read/write, no effect on the chip.

**DLL/DLM:** divisor bits 7..0 and 15..8.

### Behavior

**Baud generator (p11, p14, p16)**
- **Clock source:** a crystal on XTAL1/XTAL2 (fundamental, parallel resonant, 1–10 MHz; Table 8 p16), or an external clock into XTAL1 with XTAL2 left open.
- **Formula:** divisor = fIN / (16 × baud), N = 1…65535.
  - Writing either latch immediately loads the 16-bit counter ("prevents long counts on initial load").
  - Divide-by-1 is allowed: DLL = 1, DLM = 0.
- **Limits:** maximum clock 10 MHz, giving a maximum BAUDOUT of 10 MHz and 625 kbps (p16).
- **BAUDOUT:** the 16× clock, fIN / N (p4). It is active-low on the drawing.
  - The front-page feature list says BAUDOUT "provides either a buffered oscillator or 16X… baud rate clock". With N = 1, BAUDOUT is the buffered oscillator.
- **BAUDOUT waveform (Figure 5, p20):**
  - For N > 3: low (tLW) for 2 XTAL1 cycles and high (tHW) for N − 2 XTAL1 cycles.
  - ÷1, ÷2 and ÷3 are drawn separately.
  - tBLD for ÷1 is measured from the XTAL1 falling edge; every other tBLD/tBHD is from the XTAL1 rising edge.
- **Receiver clock:** RCLK is the receiver's 16× clock. Tie it to BAUDOUT or drive it externally.

**Receiver (p14)**
- **Start-bit detection:**
  - The start-bit detector looks for a high-to-low transition.
  - On detection it resets a counter and counts the 16× clock to 7½, the centre of the start bit.
  - The start bit is valid if SIN is still low at that sample, which rejects noise spikes.
  - The data bits are then sampled at mid-bit, every 16 clocks.
- **Timing margins:** "the center of the data cells will occur within ±3.125% of the actual center, providing an error margin of 46.875%. The start bit can begin as much as one 16X clock cycle prior to being detected."
- **Frame:** data is LSB first. Parity precedes the first stop bit. FE is set if the stop bit is not high.
- **Overrun:** OE is set if the RBR has not been read before the next RSR→RBR transfer. The new character replaces the old.
- **Break:** a break character loads the RBR (DR is set, "including Break"). It is an all-zero character, including parity and stop.
- **Interrupt timing:** tSINT (stop to the data-ready/error interrupt) is 1 BAUDOUT cycle (p19). Figure 8 shows a sample clock every 16 RCLKs ("8 CLKS" marked to the first sample).

**Transmitter (p14)**
- Writing THR clears THRE. THR moves to TSR at the start bit, setting THRE again.
- TEMT goes high when the last word has shifted out. "THRE is set high one THR to TSR transfer time later" — the datasheet's wording, which is ambiguous.
- SOUT is held at mark while MR is active, when the TSR is empty, or in loop mode. CTS does not affect SOUT (p4).
- **Break-send sequence (p8):** load a 0x00 pad character on THRE, set break on the next THRE, wait for TEMT, then clear break.

**Interrupts (Table 2 p12, Figure 1 p13)**

| IIR[2:0] | Priority | Type | Source | Reset |
|---|---|---|---|---|
| xx1 | – | none | – | – |
| 110 | 1 | Receiver line status | OE, PE, FE or BI | LSR read |
| 100 | 2 | Received data available | DR | RBR read |
| 010 | 3 | THRE | THRE | IIR read (if THRE is the source) or THR write |
| 000 | 4 | Modem status | DCTS, DDSR, TERI or DDCD | MSR read |

- "No other interrupts are acknowledged until the interrupt is serviced by the CPU." The IIR shows the highest pending interrupt.
- INTRPT is an active-high, push-pull output: the OR of (DR & ERBFI), (THRE & ETBEI), ((OE|PE|FE|BI) & ELSI) and ((DCTS|DDSR|TERI|DDCD) & EDSSI) (Figure 1).
- OUT2 does not gate INTRPT.
- After MR, THRE and TEMT are set, so enabling ETBEI raises a THRE interrupt immediately (p15).

**Modem lines and loopback (p10)**
- **Outputs:** MCR bit = 1 drives the pin low.
- **Loopback (MCR4 = 1):**
  - SOUT is held at mark and SIN is disconnected. The TSR output feeds the RSR.
  - CTS/DSR/DCD/RI are disconnected. Internally DTR, RTS, OUT1 and OUT2 connect to the modem inputs: RTS → CTS, DTR → DSR, OUT1 → RI, OUT2 → DCD, per the MSR description.
  - The DTR/RTS/OUT1/OUT2 pins are forced high.
  - The TX and RX interrupts work. The modem interrupts come from MCR[3:0] and are still gated by IER.
- The front-page feature list says "Loopback and Echo Modes", but no echo mode is described anywhere in the datasheet.

**Reset (MR, active high, Schmitt; p15 Table 7)**
- **Pulse width:** TMRW ≥ 500 ns.
- **What it initialises:** the TX and RX clock counters. It clears the LSR except THRE and TEMT, which are set, and clears the MCR along with its outputs.
- **Registers:** IER = 0; IIR = 01 (bits 7..3 permanently low); LCR = 0; MCR = 0; LSR = 0x60; MSR bits 0..3 = 0 with bits 4..7 following the inputs.
- **Outputs:** SOUT high; INTRPT low; OUT1, OUT2, RTS and DTR high.
- **Not affected:** the divisor latches, RBR and THR.
- **Not stated:** BAUDOUT behaviour during MR. The divisor is not cleared, so it presumably keeps running.
- After MR is removed the chip stays idle until it is programmed.

**Software reset (p16):** the datasheet means a software procedure — rewrite LCR, the divisor latches and MCR, then read LSR and RBR. There is no reset register.

### Timing (p18–19; VCC 5.0 V ±10%; 0–70 °C for C, −40–85 °C for I; one -5 column; all min/max)

**Bus timing**

| # | Parameter | Value |
|---|---|---|
| 1 | TAW, ADS width | min 50 ns |
| 2 | TAS, address setup | min 60 ns |
| 3 | TAH, address hold | min 0 ns |
| 4 | TCS, CS setup | min 60 ns |
| 5 | TCH, CS hold | min 0 ns |
| 6 | TDIW, read strobe width | min 150 ns |
| 7 | TRC, read recovery | min 270 ns |
| 8 | RC = TAR + TDIW + TRC | min 500 ns |
| 9 | TDD, read to DDIS | max 75 ns |
| 10 | TDDD, read to data | max 120 ns |
| 11 | THZ, read end to float | 10–75 ns |
| 12 | TDOW, write strobe width | min 150 ns |
| 13 | TWC, write recovery | min 270 ns |
| 14 | WC | min 500 ns |
| 15 | TDS, data setup | min 90 ns |
| 16 | TDH, data hold | min 60 ns |

**Demultiplexed operation (ADS tied low)**
- TCSC (select to CSOUT): max 125 ns.
- TRA / TRCS (address and CS hold from read): min 20 ns.
- TAR / TCSR (read delay from address and CS): min 80 ns.
- TWA / TWCS (address and CS hold from write): min 20 ns.
- TAW / TCSW (write delay from address and CS): min 80 ns.
- Symbol clash: "TAW" is used both for the ADS width (#1) and for the write delay from address (#24).

**Reset and clock**
- TMRW: min 500 ns.
- TXH / TXL: min 40 ns.

**Baud generator**
- N: 1 to 2^16 − 1.
- TBLD / TBHD: max 250 ns.
- TLW / THW: min 40 ns, at TXL = TXH = 50 ns.

**Receiver**
- TSCD (RCLK to sample): max 250 ns.
- TSINT: 1 BAUDOUT cycle.
- TRINT (read RBR to interrupt reset): max 250 ns.

**Transmitter**
- THR (write to interrupt reset): max 250 ns.
- TIRS (initial INTR reset to transmit start): 8–24 BAUDOUT cycles.
- TSI (initial write to interrupt): 16–32 cycles.
- TSTI (stop to THRE interrupt): 8–24 cycles.
- TIR (read IIR to THRE reset): max 250 ns.

**Modem**
- TMDO (write to output): max 500 ns.
- TSIM (modem input to interrupt): max 500 ns.
- TRIM (read MSR to interrupt reset): max 500 ns.

**AC load:** 100 pF, with 520 Ω to 1.7 V (p20).

### VCC

- **Supply:** 4.5–5.5 V ("Operating Voltage Range", p17).
- **Inputs:** range GND − 0.5 V to VCC + 0.5 V, so they are not tolerant above VCC (inputMax = VCC).
- **Levels:** TTL-compatible: VIH 2.0 V, VIL 0.8 V. External clock VIH(CLK) = VCC − 0.8 V. MR is a Schmitt trigger with 2.0/0.8 V thresholds.
- **Standby current:** ICCSB ≤ 100 µA, with no defined sleep mode.

### Package drawing

- **Drawing:** Intersil N44.65, "44 Lead Plastic Leaded Chip Carrier Package" (JEDEC MS-018AC), p25 (Rev. 2 11/97). No land pattern is printed.
- **Pitch:** 0.050 in (1.27 mm) TP.
- **D/E over the leads:** 0.685–0.695 in (17.40–17.65 mm).
- **D1/E1 body:** 0.650–0.656 in (16.51–16.66 mm).
- **D2/E2:** 0.291–0.319 in (7.40–8.10 mm). Measured at the seating-plane contact point from the centreline, so the row-to-row contact span is about 0.61 in (≈15.5 mm).
- **Lead width:** lower 0.013–0.021 in (0.33–0.53 mm); upper 0.026–0.032 in (0.66–0.81 mm).
- **Height:** A 0.165–0.180 in (4.20–4.57 mm); A1 0.090–0.120 in (2.29–3.04 mm).
- **E40.6 PDIP (reference only):** p24, MS-011-AC, e 0.100 in, eA 0.600 in.

### Obsolete

- CS82C50A-596 is Obsolete at DigiKey.

### Notes

- **Datasheet errors:**
  - The PLCC drawing (p3) puts an overbar on CS1, but the DIP drawing and the pin table (p5: "CS0, CS1, CS2 … H, H, L") show CS1 active high. Use active high.
  - MSR(3) DDCD cites "Pin-36"; DCD is pin 38 (p11).
  - MSR(7) text says "MSR(4) is equivalent to OUT2"; it means MSR(7) (p11).
  - The OUT2 pin text says "setting MCR(3) (OUT1)"; it means OUT2 (p5).
- **"Echo mode"** is advertised on p1 but never described.
- **Two-stop-bit check:** the 82C50A checks two stop bits ("The receiver checks for two stop bits if programmed"), unlike the 16550-family text, which checks only the first. Whether a second stop bit at 0 sets FE is not stated further.
- **CS82C50A-5:** the census part (listed under Harris) decodes by naming scheme but is not a row in this revision's ordering table (see Parts).

