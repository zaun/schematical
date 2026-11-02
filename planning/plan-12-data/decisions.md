# plan-12 datasheet contradiction decisions

Source: `plan-12-data/matrix.md` §4, with context from `plan-12-data/specs/*.md`. Dropped from scope (devices no longer in the plan): TL16C2752, ST16C650A, M66230, COM8017/COM8502, CDP6402, SC16C652B, ST16C2450. Entries naming them were removed.

Each line reads `- issue → decision (policy)`. "NPD" marks an entry the policies did not settle; the planner decided each one in the last section.

**Counts** (405 entries; each entry is counted under its first-cited policy): P1 57 · P2 31 · P3 7 · P4 7 · P5 22 · P6 6 · P7 1 · P8 20 · P9 1 · P10 90 · P11 147 · **Planner decisions 16** (last section).

Notes on how the policies were applied:
- P2 is applied before P11. In a few places it overrides the spec author's reading:
  - the TL16C754B and TL16C752C read priority at 110/111;
  - Exar 000/001 at LCR=0xBF (the XR16L580 author's reading was "probably DLL/DLM");
  - SC16IS750 0x02–0x0F with LCR[7]=1;
  - MSR bits of modem inputs that a package does not bond out. This covers SC16C650B and XR16M2550, where the authors' reading was "inactive".
- Those MSR bits are handled the same way on every package. Bits whose input pin is absent read X. Their delta bits (ΔDSR, TERI, ΔDCD) stay 0, because an absent pin never has an edge. In loopback they follow MCR as usual.
- In loopback the INT pin stays gated by the MCR[3] *register bit*, as in normal mode. On the PC16550D the interrupts stay fully operational in loopback. The OP2/OUT2 *pin* is still forced inactive.
- CLKSEL vs reset MCR=0x00 (P1): the reset table wins for the register value, so MCR[7] reads 0 after reset. The prescaler state after reset still comes from the CLKSEL pin (GND → ÷4). A later MCR[7] write overrides it.

---

## ti-a.md

### TL16C450
- p17 IIR mentions a character timeout (a 16550 leftover) → no timeout interrupt; IIR[3] always reads 0 (P8)
- p6 th1 "address low after ADS↑" → th1 = address hold after ADS↑ (address valid, either level) (P10)
- LCR has no reset entry → LCR = 0x00 after MR (P3)
- break and false-start handling unstated → 16450 convention: a start bit still low at the mid-bit sample (count 7½ of 16) is valid, otherwise ignored; RX low for a full character time sets BI and loads one 0x00 character (P11)
- divisor 0 undescribed → divisor 0 stops the baud generator; BAUDOUT drives X while DLL=DLM=0 (P2)

### TL16C451 / TL16C452
- BDO polarity: pin text "active high" vs Fig 3 low during a read → BDO idles high and is low while a selected read is in progress; the Fig 3 waveform and the "IOR↓ to BDO↓" parameter (p8) agree (P1)
- INT2: text says ACK↑ edge, Fig 7 says it follows ACK → INT2 = ACK pin level while IRQ ENB = 1; the Fig 7 waveform and td21 "ACK↓/↑ to INT2↓/↑" agree (P1)
- LPT status/control bit polarity vs pins unstated → IBM PC LPT convention: control b0 STB, b1 AFD and b3 SLIN drive their pins inverted (1 → pin low); b2 INIT drives its pin true; b4 = IRQ ENB. Status b7 = NOT BUSY pin; b6..b3 = ACK, PE, SLCT, ERR pin levels (P11)
- SOUT0 typed "I" → output (P10)
- ICC condition says "XTAL1" → CLK (P10)
- typos td15 "DTS" → DTR; tw5 "IOW↑" → low pulse width; th3 "before" → after (P10)
- read data valid "before IOR↑" vs the figure → td5 measured from IOR↓ (≤ 60 ns); the td5 table row and the figure agree (P1)
- several CS low at once unspecified → a read with more than one of CS0/CS1/CS2 active drives X on D7–D0; a write with several active is taken by every selected block (P2)
- whether BDO responds to unselected reads → BDO is low only while IOR is low and at least one CSx is active (P11)

### TL16C550B
- CS1 overbar on the N/FN drawings → active high, per the pin-function table (CS0=H, CS1=H, CS2=L), the PT drawing, Fig 2 and Fig 13 (P1)
- tsu3 edge → measured to the trailing edge of the write strobe (WR1↑ / WR2↓), where data is latched; Fig 2 and the latch text agree (P1)
- MSR[5] in loopback "MCR bit 1" → MSR[5] = MCR[0] (DTR), per the MCR/MSR bit tables (P1)
- LSR[7] "always cleared" → reads 0 in 450 mode (FCR0=0); in FIFO mode it is the error-in-FIFO flag (P11)
- Fig 8 caption "FCR=1 or FCR3=1" → FCR0=1 and FCR3=1 (P10)
- XIN max 8 MHz (p29) / 9 MHz (p8) / 562 kbit/s (p1) → clock limit not checked (P5)
- census says 3.3/5 V → catalog supply 5 V only (P11)
- TXRDY mode-1 re-assert threshold unstated → per Fig 11: TXRDY goes high when the 16th byte fills the TX FIFO, and goes low again td21 after the next start bit, i.e. as soon as one slot is free (P11)

### TL16C550C
- tw8 "1 ns" → 1 µs (P10)
- FCR3 "level 0 to 1" → DMA mode 0 to mode 1 (P10)
- "LSR bit 12" → LSR bit 1 (P10)
- LSR[7] "always cleared" → as TL16C550B: 0 in 450 mode, error flag in FIFO mode (P11)
- ΔCTS with AFE on → MSR[0] is not set and no modem interrupt is raised by CTS changes while auto-CTS is enabled (P11)
- addendum CPFBRG4 temperature → TI addendum error, catalog uses the datasheet grade (P10)
- no 3.3 V timing → use the 5 V timing rows at every supply (P5)

### TL16C550D
- RHB pin table copies the PT numbers → use the RHB drawing: DSR = 25, DTR = 22, INTRPT = 20, MR = 23 (P6)
- RHB CS2 row describes CS0/CS1 → typo; the RHB has only CS2, active low (P10)
- "dc to 16 MHz" vs 24 MHz at 5 V → clock limit not checked (P5)
- 2.5/5 V columns quote 3.6 V → copy error, no model effect (P10)
- ties for missing RHB/ZQS bus pins → internal ties at their inactive levels: CS0 = CS1 = 1, ADS = 0, RD2 = 0, WR2 = 0 (P11)
- MSR values for the missing ZQS modem inputs → MSR[5] (DSR), MSR[6] (RI) and MSR[7] (DCD) read X outside loopback; MSR[1]/[2]/[3] stay 0 (P2)

### PC16550D
- minimum divisor 1 (p1, p4 table) vs 2 (p15 text) → divisor 1 is valid (P1)
- Table I MCR[7:4] fixed → only MCR[7:5] are fixed at 0; MCR[4] = LOOP. Table II and the p17 text agree (P1)
- 18.432 MHz / 1200 baud prints 920 → 960 (P10)
- tXH/tXL at 8 MHz vs 24 MHz max → clock limit not checked (P5)
- both strobes of a pair active → a read with RD and RDBAR both active drives X on D7–D0; with WR and WRBAR both active the chip performs a single write (P2)
- RX sample point unstated → mid-bit sample at count 7½ of 16 (P11)
- SCR reset unstated → SCR = 0x00 after MR (P3)

### 82C50A
- CS1 overbar → active high (P11)
- MSR(3) "Pin-36" → pin 38 (P10)
- MSR(7) text "MSR(4)" → MSR(7) (P10)
- OUT2 text "MCR(3) (OUT1)" → OUT2 (P10)
- IER "write register" → read/write (P11)
- write to 010 undefined (no FCR) → write ignored (P2)
- does a 0 second stop bit set FE? → yes. When 2 stop bits are programmed, both are checked ("The receiver checks for two stop bits if programmed"; matrix stopCheck = both), and FE is set if either is 0 (P11)
- THRE timing ambiguous → THRE sets at the THR→TSR transfer (start of the start bit); the THRE interrupt follows the timing table: TSTI 8–24 cycles after stop, TIR ≤ 250 ns to reset (P1)
- TAW symbol used for two parameters → documentation slip: #1 TAW = ADS width ≥ 50 ns, #24 TAW = write delay from address ≥ 80 ns, kept as two parameters (P10)
- "echo mode" never described → not modeled (P8)

## ti-b.md

### TL16C552 / TL16C552A / TL16C554 / TL16C554A
- LCR reset: table says cleared, text says unaffected → LCR = 0x00 after reset (P1)

### TL16C552
- Fig 18 "OUT2 output low" with no OUT2 pin → MCR[3] is only the INT enable (P10)
- RI TERI edge (pin text high→low, MSR text low→high) → TERI (MSR[2]) sets when the RI pin goes low→high (P11)
- Table 16 titled 8.192 MHz but values for 8.000 MHz; 50 baud "1000" → 10000 (P10)
- LPC b3 "SLCT" → SLIN (P10)
- BDO edge text vs figure → BDO active high during a selected read (P11)
- TL16C552 vs 552A tpd6 edge and td6 max (8 vs 9) → per device, each from its own sheet (P11)

### TL16C552A
- "dc to 8 MHz" leftover vs 16 MHz → clock limit not checked (P5)

### TL16C554
- footnote "CSA or CSD" → each CSx selects only its own channel (P10)
- INTN text "modem status register bit 3" → MCR[3] (P10)
- RXRDY/TXRDY pin text (p5) vs pp29–30 → pp29–30 behavior (P11)
- LSR4 text "LSR2" → LSR4 (P10)
- MSR text "IIR3" → IER3 (P10)
- "dc to 8 MHz" vs 16 MHz → clock limit not checked (P5)
- PN package TQFP vs LQFP → LQFP80; package naming only (P10)
- several CS low unspecified → a read with more than one CSx low drives X on D7–D0; a write is taken by every selected channel (P2)

### TL16C554A
- MCR5–7 "permanently cleared" vs AFE → MCR[5] = AFE; MCR[7:6] read 0 (P11)
- Table 7 "MSR" header → MCR (P10)
- missing FN pin numbers for RI/RTS → from the p2 drawing: RIA 8, RIB 28, RIC 42, RID 62; RTSA 14, RTSB 22, RTSC 48, RTSD 56 (P6)
- PM package mis-described; figure references wrong → no model effect (P10)

### TL16C750
- CS1 overbar → active high (P11)
- THR write "clears the FIFO" → a THR write clears the THRE interrupt only (P11)
- "06 or 0110h" → binary 0110 (P10)
- LSR7 "in 750 mode always cleared" → reads 0 in 450 mode (P10)
- RXRDY/TXRDY reset states unspecified → RXRDY and TXRDY both drive high (inactive) while MR is active. After MR they follow the mode-0 rules, so TXRDY goes low because the TX side is empty (P3)
- which clocks sleep stops → while asleep (IER4) or in low-power mode (IER5), BAUDOUT drives X. No TX/RX activity happens in either state; the wake conditions are as printed (P2)

### TL16C750E
- multi-channel copy text ("two devices", "CS A-B") → single channel (P10)
- DLF ×128 (p30) vs ×64 (Table 7, Table 18, p51 example) → fraction = DLF[5:0]/64 (P1)
- TCR range 0–60 vs 0–120 (×8) → TCR nibble × 8 (P1)
- RS-485 DLY 0–15 vs a 3-bit field → AFR[7:5] DLY = 0–7 bit times (P1)
- RX triggers 1/4/120/124 → as printed in the FCR table (P1)
- IOW-only mode figures blank → no model effect; the MODE=GND timing table (pp8–9) supplies the values (P10)
- OE clear → OE (LSR[1]) clears on an LSR read (Table 4 "OE: read LSR") (P1)
- auto CTS/RTS interrupt set point → IIR 100000 sets on the CTS pin's or the RTS output's low→high transition while auto-CTS (EFR7) or auto-RTS (EFR6) is enabled; cleared by an IIR read (Table 4) (P1)
- address 010 for LCR[7]=1 values other than 100xxxxx and 0xBF → reads drive X; writes ignored (P2)
- DLF with EFR4=0 → DLF ignored; the divisor is the integer DLH:DLL (P11)
- DLF vs TLR priority at 111 when both conditions hold → reads drive X; writes ignored (P2)
- outputs in loopback → TX held high (mark); DTR, RTS, OUT1 and OUT2 pins forced high (inactive); MSR[7:4] from MCR[3:0]; INT gating as normal mode (P4)

### TL16C754B
- "754A" leftovers → no model effect (P10)
- RXRDY pin text vs DMA section → each channel computes its RXRDY/TXRDY by the per-channel DMA rules; the shared pins then combine them as the pin text says (P11)
- EFR7 reset "NOT CLKSEL" → EFR = 0x00 after reset (P11)
- LCR reset 0x1D → as printed (6 bits, 2 stop bits, parity enabled, even) (P1)
- read decode priority at 110/111 unstated → reads drive X when more than one access condition holds. The conditions are LCR=0xBF (Xon/Xoff), EFR4&MCR6 (TCR/TLR) and MCR2&!MCR4 (FIFORdy, 111 only). Writes go to the single writable target; FIFORdy is read-only and takes no part in write decode. A write where both Xoff and TCR/TLR qualify is ignored (P2; overrides the author's priority order)
- FIFORdy decodes with the selected channel's MCR → as stated (P11)
- T20d/T23d/T28d units → 16× clocks (RCLK) (P11)
- p18 notes refer to nonexistent pins; programmer's guide garbled → no model effect (P10)
- loopback pin states → TX held high; DTR/RTS/OP pins forced high (inactive); MSR[7:4] from MCR[3:0]; INT gating as normal mode (P4)
- census TL16C754FN (original 754) has no datasheet → use the TL16C754B FN68 pinout (P11)

### TL16C754C
- AFR DLY 0–15 vs 3 bits → 0–7 (P1)
- IIR modem code 001000 vs 000000 → 000000 (P11)
- "48SEN" → 485EN (P10)
- IrDA decoder text copied from 550C → IrDA via AFR[1] (IREN) as described; the copied sentence has no model effect (P10)
- INTSEL/RXRDY/TXRDY listed without pins → absent; INT enable is MCR[3] only (P8)
- Xon2 default prints 0xEF → power-up Xon1 = Xoff1 = Xoff2 = SPR = 0xFF and Xon2 = 0xEF, as printed; MR does not change them (P1)

## ti-c.md

### TL16C2550
- pin text cites "EFR bit 7/6" → no EFR; autoflow is MCR[5] (P8)
- "independent receiver clock" claim → not modeled; RX and TX share BAUDOUT (P8)
- 3.3 V band misprinted → catalog band 3.0–3.6 V; not checked (P5)
- RHB pins 21/22: drawing (21 = INTB, 22 = INTA) vs pin table (INTA 21, INTB 22) → drawing: pin 21 = INTB, pin 22 = INTA (P6)
- RHB called TQFP in the thermal table → no model effect (P10)
- TXRDY pin text mentions a trigger → no TX trigger; TXRDY uses the PC16550-style rules (P8)

### TL16C2552
- MCR3/INT reset: reset table MCR=0x08 vs INT pin text high-Z → MCR = 0x08 after reset: INT driven, OP low (P1)
- pin text cites EFR; IOW text cites CSA/CSB → no model effect (P10)

### TL16C752C
- modem IIR 001000 → 000000 (P11)
- CLKSEL mentioned but absent → no CLKSEL; MCR[7] resets 0 (÷1) (P8)
- RS-485: "1 bit before first stop" text vs Fig 27 → DTR rises one bit time before the start bit (Fig 27; the text is a slip) (P10)
- RS-485 DLY 0–15 vs 3 bits → AFR[7:5] = 0–7 bit times (P1)
- D/D-Q1/CI-Q1 Table 7 labels FIFORdy C/D bits → FIFORdy[3:2] and [7:6] read 0 (P11)
- priority at 111 (FIFORdy vs TLR vs SPR) → reads drive X when both the FIFORdy and TLR conditions hold. Writes go to TLR (FIFORdy is read-only) (P2; overrides the author's order; consistent with TL16C754B)
- AFR b2 "485RN" → 485EN (P10)
- errata SLLZ058 not in repo → not modeled; datasheet text followed (P7)
- spec-author error: LCR 0x1D decoded as "8 bits, odd parity" → LCR reset 0x1D = 6 bits, 2 stop bits, parity enabled, even parity (P11)

### NS16C2552
- LCR4 text "1 = odd" vs table → LCR[4] = 1 selects even parity (P1)
- DREV ID "0x03" vs table → DREV = 0x31 (P1)
- Table 2 IIR[5:4] for the 2552 → both devices implement the full IIR set, including [5:4] (P11)

## exar-a.md

### ST16C450
- loopback prose garbled → MSR bit text mapping: RTS→CTS, DTR→DSR, OP1→RI, OP2→CD (P1)
- AS# "logic 0 transition" vs figure → latch transparent while AS# = 0; address and CS held from AS# rising (P11)
- IER/LCR/MCR marked write-only (Table 2) → read/write (Table 4) (P11)
- LSR clear-on-read and interrupt clears unstated → 16450 standard: an LSR read clears LSR[4:1]; RX line-status interrupt cleared by the LSR read (P11)
- loopback pin states → TX = 1; RTS#, DTR#, OP1#, OP2# forced high (P4)
- LCR[7] text mentions EFR → no EFR (P8)
- PLCC e "0.20" → 0.020 in (P10)
- OBS watermark vs census Active → no model effect; no obsolete flag while DigiKey shows the part Active (P10)

### ST16C1450
- RST "only on 1451" vs Fig 2 (pin 22) → RST present on pin 22 of the 48-TQFP (P6)
- "EFR bit-4" header → the gate is IER[5] (P10)
- ISR[7:4] "reserved" vs IER5-enabled ISR[5:4] → with IER[5]=1, ISR[4] = RXRDY and ISR[5] = TXRDY, active high (P11)
- start sample 8 vs 7½ → 8 clocks on this part (P11)
- power-down internals → oscillator/baud clocks stopped, register bus stays alive (P11)
- MCR2→RST in loopback unstated → in loopback MCR[2] only feeds MSR[6]; RST = RESET pin state (MCR[2] is cut from RST like the other forced-inactive outputs) (P4)
- Table 2 grouping of 101–111 → unaffected by LCR[7] (P11)
- abs-max 7 V vs VIH max VCC → not checked; catalog: no 5 V tolerance (P5)
- AC table omits TSSR, TRR, TSRT, TWT (Figs 11/12) → NPD

### ST16C550
- loopback prose garbled → MSR bit mapping RTS→CTS, DTR→DSR, OP1→RI, OP2→CD (P1)
- LCR[7] EFR text → no EFR (P8)
- LSR/ISR clear rules → 16550 standard (P11)
- timeout 4W+12, not 4 characters → as printed (P11)
- PLCC pitch misprint → 0.050 in / 1.27 mm (P10)
- FIFO clear at reset unstated → RX and TX FIFOs cleared by RESET (P3)

### ST16C2550 / XR16C2550
- Table 5 "MCR bit-7" → no prescaler (P8)
- SPR text mentions sleep → no sleep (P8)
- crystal unit "ns" → MHz (P10)
- XR16C2550 abs-max VCC+0.3 vs 5 V-tolerant → not checked; catalog inputMax 5.5 V (P5)

### ST16C1550 (/1551)
- MCR[7] prescaler (p9, Table 2) vs power-down (map) → MCR[7] = power-down; no prescaler (P11)
- "EFR bit-4" title → IER[5] (P10)
- RST "only 1551" → RST present (P11)
- abs-max vs VIH → not checked; catalog inputMax = VCC (P5)
- SPR "sleep" → power-down (P10)

### ST16C554 (/554D)
- copy artifacts (IR encoder, IRTX, CTS stop, LCR=0xBF comment, MCR[7] column, "Channels A and B") → not modeled (P8)
- reset table "INT LOW" → INT high-Z on the CQ64 (3-state INT) and driven 0 on the D parts (P11)
- LSR/MSR R/W vs RO → the register table says R/W, so a write stores into LSR/MSR (factory-test style, as PC16550D) (P1)
- DLL/DLM not reset → unchanged by RESET (P11)

### XR16L580
- Xon/Xoff R/W (Table 5) vs write-only (Table 6) → write-only; reads at those addresses follow the normal-bank decode (P11)
- IrDA pulse "returns 1" vs Fig 12 → a 3/16-bit pulse encodes a data 0 (P11)
- MCR[2] polarity naming → MCR[2]=0 default = standard IrDA RX polarity (P11)
- trigger-14 RTS-off level undefined → RTS deasserts at 16 bytes (P11)
- pull-up/pull-down on TQFP only → as stated (P11)
- "channel A and B" copy text → single channel (P10)
- census ILTR-F "24-QFN" → 32-QFN (P10)
- 000/001 at LCR=0xBF unspecified → reads drive X; writes ignored (P2; author's "probably DLL/DLM" not used)

## exar-b.md

### XR16L2551
- 000/001 at LCR=0xBF unspecified → reads drive X; writes ignored (P2)
- DREV R/W vs RO → DREV/DVID read-only; writes at 000/001 still reach DLL/DLM (P11)
- DLL rewrite after ID read → reads are muxed (ID when DLL=DLM=0); writes always reach DLL/DLM (P11)
- FCR[5:4] EFR-protected but reserved → reserved, writes have no effect (P11)
- OE clear on LSR read unstated → 16550: LSR read clears OE (P11)
- RTS levels at trigger 1/14 ends → RTS off/on at 0/16 (as stated in A2) (P11)
- IrDA prose vs Fig 12 → a pulse encodes data 0 (P11)
- TX pulse position → 3/16-bit pulse centred in the bit cell (starts at 7/16) (P11)
- IER access and read data while bus-isolated (PowerSave) → the bus inputs (A2–A0, D7–D0, IOR#, IOW#, CSA#, CSB#) are ignored. Writes, including IER, are dropped. Reads are not seen, so D7–D0 stay high-Z. The host must wake the part with an RX or modem event first (P11)
- MCR[3] INT gating in loopback unstated → INTx gated by the MCR[3] register bit as in normal mode; OP2# pin forced high (P4)
- "RESET active high" without the Motorola qualifier → RESET active high in Intel mode, active low (RESET#) in Motorola mode (P11)
- Fig 3 titled XR16L2751 → copy error (P10)

### XR16L2552
- Fig 3 "AFR[1:0]=01" → MF# select is AFR[2:1] (P1)
- MF# text "MCR[3] defaults 1" → Table 13: MCR = 0x00 after reset, MF# = 1 (P1)
- DREV/DVID R/W vs RD → read-only (P11)
- 000/001 at 0xBF undefined → reads drive X; writes ignored (P2)
- RTS trigger 1/14 ends → 0/16 (P11)
- IrDA → a pulse encodes data 0 (P11)
- sleep text cites CSA#/CSB# → copy (P10)
- 3.125 Mbps / 50 MHz vs "4 Mbps" / 64 MHz → clock and baud limits not checked (P5)
- ISR[5:4] → read 0 unless EFR[4]=1 (P11)
- AFR[0] shared or per channel → NPD
- DREV read vs DLL write path → as the +ID convention: reads muxed, writes reach DLL/DLM (P11)
- OE clear → LSR read (16550) (P11)

### XR16M2550
- pin-table VCC 2.25–3.6 → catalog 1.62–3.63 V; not checked (P5)
- duplicate TX INT row → IER[1]/LSR[5] THR-empty semantics (P11)
- "DTSA#" → CTSA# (P10)
- 000/001 at 0xBF undefined → reads drive X; writes ignored (P2)
- divisor 0 undefined → baud generator stopped (no TX/RX clock) (P2)
- INT gating in loopback → INTx gated by the MCR[3] register bit as in normal mode; OP2 pin forced high (P4)
- IrDA pulse position → centred (P11)
- OE clear → LSR read (P11)
- QFN32 missing DSR/CD/RI → MSR[5]/[6]/[7] read X outside loopback; their delta bits stay 0 (P2; author's "deasserted" not used)

### XR16M2650
- FCR default trigger 1 (text) → table: RX 8, TX 16 for 00 (P1)
- AC table corrupted from TWDO down → use the XR16M2550/XR16M2651 values for those rows (P11)

### XR16M2650 / XR16M2551 / XR16M2651
- OBS watermark vs DigiKey Active → no model effect; no obsolete flag (P10)

### XR16M2551 / XR16M2651
- "whichever selection made last applies to both RX and TX" → independent RX and TX triggers (table columns) (P1)
- CLKSEL vs reset MCR=0x00 → MCR reads 0x00 after reset (table), but the prescaler after reset follows CLKSEL (GND → ÷4, VCC → ÷1; QFN32 without CLKSEL → ÷1); an MCR[7] write overrides (P1)
- INTB in Motorola mode → INTB driven to logic 0 (P11)

### XR16M752
- RS-485 polarity (p9 HIGH = RX vs pp18/37 HIGH = TX) → HIGH = transmit (P11)
- IrDA "returns 1" → a pulse encodes data 0 (P11)
- MCR[1] text "FCTR bit-3" → DLD[6] (P10)
- TLR text "TCR[3:0]" → TLR (P10)
- pin table IR enable "MCR[6]" → DLD[7] per the register table (P1)
- INTB description cites OP2A#/INTA → OP2B#/INTB (P10)
- "M2751" and 2.25–3.6 V in the Motorola figure → copy errors (P10)
- p30 TX trigger direction → TX trigger when spaces ≥ TLR level (P11)
- RX ">" vs "≥" → RX trigger when level ≥ trigger (P11)
- 000/001 at 0xBF undefined → reads drive X; writes ignored (P2)
- FIFO Rdy polarity → each bit = NOT the matching TXRDYx#/RXRDYx# pin (1 = ready) (P11)
- Motorola IRQ# combining → IRQ# low when either channel has a pending interrupt (wired OR) (P11)
- Motorola IRQ# vs MCR[3] gating → NPD
- census "34B" FIFO → 64 bytes (P10)

## exar-c.md (XR16V2550 / V2650 / V2551 / V2651 / V2552 / V2652)

- Table 8 brackets LCR under "LCR≠0xBF" → LCR (011) always accessible (P11)
- 000/001 at 0xBF undefined → reads drive X; writes ignored (P2)
- 2550/2551/2650/2651: 010 with LCR[7]=1, ≠0xBF, EFR4=0 → ISR (read) / FCR (write), per the Table 8 "LCR≠0xBF" bracket (P1)
- INT tables: second "RS-485" TX row → not modeled (P8)
- 2650/2651/2652: FCR default-trigger text → table values for 00 (P1)
- 2551/2651: "applies to both RX and TX" → independent RX and TX triggers (P1)
- abs-max errors (2551 3.3 V; others 7 V) → not checked; catalog inputMax 5.5 V, XTAL1 ≤ VCC (P5)
- crystal max 32 MHz at 2.5 V (AC) vs 24 MHz (2551/2651 text) → not checked (P5)
- 2552: Fig 3 AFR[1:0] → AFR[2:1] (P1)
- 2552: INT table "AUTO RS485" and MCR[1] "FCTR bit-3" → no RS-485, not modeled (P8)
- 2552: IrDA pin text → the MCR[2] text sets RX polarity (P11)
- MSR/LSR clear-on-read unstated → 16550 standard (P11)
- 2551/2651: does the wake-up indicator assert INT? → yes. On wake from sleep the INT pin asserts (subject to MCR[3] gating in Intel mode; IRQ# in Motorola mode) and ISR reads xxxx0001; an ISR read clears it. The text ("raises an interrupt") and the ISR table code agree (P1)
- 2551/2651: Motorola INTB NC, IRQ# = OR of both channels, write latched on CS# rising → as inferred (P11)
- 2651: CLKSEL vs reset MCR=0x00 → MCR reads 0x00 after reset; prescaler after reset follows CLKSEL (GND → ÷4); an MCR[7] write overrides (P1)
- 2551: block diagram still shows CLKSEL (pin 25 NC per rev 1.0.3) → no CLKSEL; prescaler resets to ÷1 (P10)
- 2551/2651: OBS watermark vs ordering "Active" → no model effect (P10)
- 2652: DC header 0–70 °C → industrial grade only (P10)
- 2652: IJ obsolete at DigiKey but in stock → `"flags": ["obsolete"]` (P11)
- DLL/DLM set at power-up only; sleep needs IER[4] on both channels → as stated (P11)

## exar-d.md

### XR16V554
- fractional divisor and MCR[7] in text but not in map → integer divisor, no prescaler (P8)
- CTS auto-stop text → no auto-CTS (P8)
- INT tables (p12) inconsistent → INTx = OR of the enabled pending sources (P11)
- QFN48 pin 34 missing → GND (P11)
- divisor 0 undefined → baud generator stopped (P2)
- LSR/MSR with LCR[7]=1 unspecified → reads of 101/110 with LCR[7]=1 drive X (P2)

### XR16V564 / XR16V654
- MCR resets 00 but CLKSEL=GND gives ÷4 → prescaler after reset follows CLKSEL (GND → ÷4); MCR[7] reads 0 after reset (table); an MCR[7] write overrides (P1)
- abs-max 4 V vs 5 V-tolerant → not checked (P5)

### XR16V564
- decode of 100–111 at LCR[7]=1 (≠0xBF) and 000/001 at 0xBF unspecified → reads drive X; writes ignored (P2)
- force-parity text misprinted → Table 13 (P1)
- 68-mode IRQ reset 3-state (V564) vs HIGH (V554) → per sheet: released (high-Z) on V564 (P11)

### XR16V654
- interleaved trigger table → first block TX, second block RX (P11)

### XR16V798 / XR16L784
- TIMERCNTL Table 8 bit labels vs Table 10 command codes → Table 10 command codes 0001–1010 (V798) (P11)
- "1011 reset" vs "1011–1111 reserved" → 1011 = reset timer; 1100–1111 reserved (ignored) (P11)
- FCTR[3:0]=0000: Table 17 hysteresis 0 vs text "next trigger level" → hysteresis 0 (table D: RTS/DTR high at RXTRG, low below RXTRG) (P1)
- reset table lists IRTX pins that don't exist → no model effect (P10)
- V798 RS-485 pin "RTS# or DTR#" → MCR[2] selects RTS# (0) or DTR# (1), as for auto flow (P11)
- B/C trigger columns inferred → as inferred (P11)
- Table 16 "9 1 0 1" → 0101 (P10)
- V798 LSR R/W vs RO → read-only (P11)
- V798 5 V tolerance only on serial pins → not checked; catalog: bus inputs VCC-limited (P5)

### XR16L784
- "8 UARTs" text → 4 (P10)
- "EMSR bit-7" → no EMSR, stray reference (P10)
- timer N range unstated → N ≥ 2 counts as on the V798; N = 0 or 1 makes the timer time-out bit and its interrupt X (P2)
- RS-485 pin unclear (RTS# per text vs V798's "RTS# or DTR#") → NPD
- IrDA 1.1 (p1) vs 1.0 (body) → IrDA 1.0 SIR (3/16 pulse) only, as the body describes (P10)

### XR16C854
- crystal max 8 MHz at 3.3 V (AC) vs 24 MHz → not checked (P5)
- DLL/DLM XX vs V-series 01/00 → per device: X at power-up on the 854 (P11)
- FCTR[3] "RS-485" only changes the TX interrupt → as stated: TX interrupt becomes TSR-empty, no pin control (P11)
- OP2A# in block diagram only → no pin (P6)
- interleaved trigger table → RX table B 8/16/24/28 (P11)
- TRG range unstated → TRG 1–128 sets the table-D trigger; with TRG = 0 or > 128, the trigger-reached condition (and its interrupt/RTS effect) is X (P2)
- non-F CQ/IQ may be old non-5 V-tolerant stock → catalog note only; not checked (P5)

### XR16C864
- DMA DRQ/TC protocol under-specified → TXDRQx high while the TX FIFO is below its trigger, until TC pulses or the FIFO fills above trigger; RXDRQx high at RX trigger or timeout, until TC pulses or the FIFO empties; TC acts on the channel whose DACKx is low (P11)
- OP1 polarity and OP2 pin control → OPx pin = NOT the MCR bit (MCR[2] → OP1, MCR[3] → OP2) (P11)
- DVID equals 854 → as printed (P1)
- reset states of OP1/OP2/BCLK/DRQ → OP1/OP2 high, DRQ disabled (pins act as TXRDY/RXRDY), BCLK inactive (P3)

### SC16C650B
- auto-RTS text "next trigger level" vs Table 4 → Table 4: RTS off/on 8→0, 16→7, 24→15, 28→23 (P1)
- DMA "not supported" yet pins exist → mode 0 only; FCR[3] ignored (P11)
- MSR[7:6] on HVQFN32 (no DCD/RI) → read X outside loopback; TERI/ΔDCD stay 0 (P2; author's "inactive" not used)
- SPR/DLL/DLM missing from reset table → Table 8 defaults: SPR = 0xFF at power-up, DLL/DLM = X; RESET leaves them unchanged (P1)
- ISR[5] clear method → MSR read (P11)
- TX pin in loopback → HIGH (P4)

### SC16C752B
- special-character interrupt cleared by IIR read (p9 text) vs LSR read (p12 Table 6) → cleared by an LSR read (P1)
- title "2.2 V" → typo (P10)
- "all four ports" FIFO Rdy → 2 channels (P10)
- LCR 0x1D → as printed (P1)
- TCR/TLR need LCR[7]=0? → yes: TCR/TLR are reached only with LCR[7]=0 and EFR[4]=MCR[6]=1. With LCR[7]=1 (≠0xBF), 110/111 reads drive X and writes are ignored (P11, then P2)
- MSR[7:5] on HVQFN32 → MSR[5]/[6]/[7] read X outside loopback; their delta bits stay 0 (P2)
- exposed pad → footprint EP tied to GND (die ground, per the SC16C650B sheet for the same SOT617-1 outline) (P11)
- FIFO Rdy polarity → NPD
- file-wide: EFR 1011/0111 "or" (Exar) vs "and" (NXP) → SC16C650B and SC16C752B (NXP) need Xon1 then Xon2 / Xoff1 then Xoff2 in sequence; Exar parts (XR16V554/564/654/798, XR16L784, XR16C854/864) accept either character (P9)

## bridges.md

### SC16IS750 (and SC16IS760)
- break text "LSR[6]" → LCR[6] (P10)
- MSR RI "IOState bit 6" → bit 7 (P10)
- "THR passes above trigger" → THR interrupt when TX FIFO spaces ≥ TX trigger (P11)
- addresses 0x02–0x0F with LCR[7]=1 (≠0xBF) → LCR (0x03) stays accessible. Every other address there: reads drive X, writes ignored (P2; the author's "general registers reachable" is not used, except for LCR)
- LSR interrupt clear → LSR read; CTS/RTS interrupt → IIR read (P11)
- THR interrupt cleared by IIR read? → NPD
- SCL/SDA address-strap decode → sampled at each START (P11)
- clock stretching → none (P11)
- I2C td delays in the "Min" column → maxima (P10)
- XTAL note [2] empty → no model effect (P10)

### SC16IS740
- model as a no-GPIO variant or a separate kind → NPD

### SC16IS752 (and SC16IS762)
- only a 15 MHz SPI table → use the SC16IS750 4 MHz SPI timing for the 752 (P11)
- which channel's IIR reports the GPIO interrupt → NPD
- Rev D errata (IrDA wake, overflow clear, IIR edge race, IIR 0xCC with LSR[0]=0) → model Rev E: none of the Rev D faults (P11)

### PI7C9X760 / PI7C9X762
- "eight addresses" → the 16-row address table (P1)
- SFR[4] burst R/W → auto-increment subaddress on burst; flagged (P11)
- TIDLE units → NPD
- external clock "250 MHz @2.5 V" → not checked (P5)
- IrDA "returns 1" → a pulse encodes data 0 (P11)
- ENIR# pin text → no such pin (P8)
- IRQ pull-up "1 Ω / 1.5 Ω" → kΩ (P10)
- GPIO5–7 called "DSR" → DTR, CD, RI (P10)
- DLL "WO" → read/write (P11)
- census C/Q2 revisions not in PDF → no model effect; same die and package (P10)

### XR20M1170
- only 8 unique I2C addresses → as printed (P1)
- tSP 0.5 µs → 50 ns (P10)
- tVD;DAT unit "ns" with 0.6 → 0.6 µs (P10)
- GPIO text "IOControl[0]" → IOControl[1] per the register table (P1)
- IrDA → a pulse encodes data 0 (P11)
- IOControl[3] reset scope → same scope as RESET# per the reset table: everything except DLL/DLM, SPR, Xon/Xoff (P1)
- LCR 0x1D → as printed (P1)

### XR20M1280
- abs-max 4 V vs 5 V-tolerant → not checked; catalog inputMax 5.5 V except XTAL1 (P5)
- TDO ≤ 100 ns at 26 MHz impossible → not checked; TDO used as printed (P5)
- DREV value not printed → DREV reads drive X (P2)
- SLEEP pin active level → HIGH = sleep (P11)
- tSP 0.5 µs → 50 ns (P10)
- reset table truncated → IRQ# released (high-Z/inactive) after reset (P3)

### SC18IS604
- I2CTO header 0x04 → register 0x03 (P10)
- I2CStat reset 0x00 vs 0xF0 → 0xF0 (P11)
- 0xF2 title copied → 0xF2 = data NACK (P10)
- 0xF9 truncated → 0xF9 when a command's byte count is 0 or larger than the bytes actually sent (P11)
- repeated START vs STOP+START in 0x02/0x03 → NPD
- SCL typed "O" → open-drain inout (P10)
- weak pull-ups unstated (which pins) → NPD
- I2CClock < 5 → not checked; SCL = 15 MHz / (8 × I2CClock) for any value ≥ 1; I2CClock = 0 stops the SCL generator (P5, P2)

## maxim.md

### MAX3100 / MAX3140
- test-mode TX: digital loopback (MAX3100) vs TX held low while CS low (MAX3140) → NPD
- R-bit clear → also on the falling edge of SCLK 16 when no new data is available (MAX3140 rule, both parts) (P11)
- T-bit clear at SCLK 16 (MAX3140, Read or Write Data) vs CS rise (MAX3100, Read Data) → NPD
- RA/FE "overrun or framing" (Table 11) vs text (framing) → RA/FE set by a framing error or by an RX FIFO overflow; same clear rules (next good frame, Write Config) (P1)
- config write "clears T" → T = 1 after a config write with no data pending (P11)
- EP "ground or unconnected" → footprint EP tied to GND (p1 drawing) (P6)

### MAX3107
- LCR reset 0x05 (not 8N1) → as printed (P1)
- FlowLvl Resume "transmit FIFO" → receive FIFO (P10)
- Halt/Resume comparisons → MAX3108 wording (P11)
- W/R bit polarity not printed → SPI first byte bit 7 = 1 for write (P11)
- I2C "stays at 0x00" rule only in 3108/3109 → apply to MAX3107 (P11)
- "internal oscillator" in title → none (removed in Rev 3) (P8)
- 2× mode single vs 3-sample → per device (P11)

### MAX3108
- MODE1[7] "—" in map vs IRQSel in text → MODE1[7] = IRQSel; the MODE1 bit text and the p22 power-up/IRQ procedure agree (P1)

### MAX3109
- RevID 0xC2 (bits) vs 0xC1 (footnote) vs "from 0xC0" → RevID reads 0xC2 (bit table) (P1)

### MAX14830
- auto-disable at TxSynch[4] (text) vs bit 6 (register page) → bit 6 (P1)
- "internal oscillator" → none (P8)
- census VCC 1.71–3.6 V → catalog VA 2.35–3.6 V (P11)

## classic-a.md

### AY-5-1013A (family)
- init DAV = 1 (p4) vs flowchart DAV = 0 → DAV = 0 after reset (P11)
- "AY-6-1016" → AY-6-1013 (P10)
- TCP edge for EOC/TBMT/SO differs between drawings → TCP falling edge (1013A); timing approximate (P11)
- start sample 8½/7½ → centre at clock 8 (P11)
- unused RD bits → 0 (P11)
- AY-5-1013 (no A) census part → accept as AY-5-1013A (P11)

### HD-6402
- pin 26 "TRB1" → TBR1 (P10)
- census C grade → R (2 MHz) (P11)
- TBRE after MR → 1 (P11)

### COM2017
- pin 2 "NC / −12 V" → −12 V supply pin (P11)

### COM1863 / COM8018
- HIACC on both (p7) vs COM8018 only (p1) → COM8018 only; COM1863 pin 2 NC (P11)
- stop bits with TSB=1 and 5-bit characters (2 vs 1.5) unstated → NPD

### CDP1854A
- THRE clear at TPB leading vs PSI at trailing → as printed (P11)
- mode-1 FE/PE-OE pins 3-state? → always driven (P11)

## classic-b.md

### 8251A
- reset values not tabulated → derived from the text (P11)
- N8251A PLCC vs DigiKey CDIP → no model effect; catalog uses dip28 (no PLCC pinout printed) (P10)
- MR82510/B misfiled under 8251a → it is an 82510 (P10)

### 82510
- MSR7 naming → same meaning (P10)
- RMD/TMD bit labels → as printed (P11)
- default RX FIFO 1-deep → yes (P11)
- Table 7 divisors single-BRG → as stated (P11)

### CDP65C51 / CDP65C51A
- Command b1 p16 vs Fig 5 → Fig 5: 0 = RX interrupt enabled (P11)
- Fig 9 recurring TDRE interrupt while idle → NPD
- DCD high → disables RX interrupts (P11)
- SSOP pinout assumed → DIP numbering (P11)

### 82C52
- USR clear-on-read vs DR/TBRE live → DR and TBRE mirror the live pins; the other USR bits are latched and clear on read (P11)
- BRSR 10001–11110 → undefined codes: baud generator stopped; CO drives X when BRSR7 = 1 (P2)
- "CSO" → CS0 (P10)
- fractional-divisor edge distribution → 50 % duty except ÷3/÷5 with external clock (P11)

### HD-6406
- 00111 ÷22 vs 1800-baud ÷21 → ÷22 (P11)
- 01101 ÷298 → ÷288 (P11)
- 110-baud error → follow 82C52 (P11)
- DR "in MSR" → USR (P10)
- SFD masks RBRK? → yes (figure) (P11)
- BRSR reset → D7 ← 0 (P11)
- DACK when DR/TBRE false → RXDACK read with DR = 0 drives X on the data bus; TXDACK write with TBRE = 0 is ignored (P2)

### COM78C804 / COM78804
- PER "RERR bit 2" → bit 4 (P10)
- Interrupt Summary bit 3 → reads 0 (P11)
- TxD during RESET → mark (high) (P11)
- MR2 divisors computed, not printed → internal divisor = CLK / (16 × printed actual rate), e.g. 19200 → 16, 110 → 2816 (P11)
- COM78804: RxD typed as output → input (P10)
- COM78804: abs-max "+0.7 V" → VCC+0.7 (P10)

### COM92C451
- "independent receiver clock" → shared BRG; no RCLK (P8)
- LSR/IIR "cleared" at reset → LSR = 0x60, IIR = 0x01 (P3)
- IRQ1 3-state? → the pin table types IRQ1 "out (high)" and only IRQ0 "3-state". IRQ1 is push-pull: high while an enabled interrupt is pending, low otherwise (P1)
- IRQ0 clear condition → NPD
- Baud Rate Test Mode (CE0 = CE1 = 0 with IOW low) → test mode not modeled (P8). Both blocks take the write. A read with CE0 = CE1 = 0 drives X (P2)

---

## Planner decisions (the 16 entries no policy settled)

- ST16C1450: missing TSSR/TRR/TWT/TSRT timings → the ST16C1550 values (same family and process; exar-a.md § ST16C1550). (planner)
- XR16L2552: AFR[0] concurrent write → one bit per channel; a write is broadcast to both channels while the AFR[0] of the channel addressed by the write is 1. (planner)
- XR16M752: MCR[3] and the Motorola-mode IRQ# → not gated; in Motorola mode IRQ# reflects the interrupt regardless of MCR[3] (the XR68M752 has no OUT2 function). (planner)
- XR16L784: auto RS-485 direction → drives RTS# only (as the XR16V798 sibling). (planner)
- SC16C752B: FIFO Rdy register bits → 1 = ready (register meaning), independent of the active-low TXRDY/RXRDY pin level. (planner)
- SC16IS740/750/752/760/762, PI7C9X760/762: an IIR read that reports the THR interrupt clears it (16550 rule). (planner)
- SC16IS740: its own device `SC16IS740` on the `ns16550` model with no GPIO pins; the IODir/IOState/IOIntEna/IOControl addresses are undefined on it and read X (P2). (planner)
- SC16IS752/762: the GPIO input-change interrupt is reported in channel A's IIR only. (planner)
- PI7C9X760/762: TIDLE register → storage only: it reads back what was written and changes no behavior. (planner)
- SC18IS604: read-after-write (0x02) and write-after-write (0x03) → a repeated START between the two transfers, no STOP. (planner)
- SC18IS604: weak pull-ups → none; an undriven input reads X like every other model input. (planner)
- MAX3100 / MAX3140 test mode → each follows its own datasheet: MAX3100 digital loopback (max3100 p8); MAX3140 TX held low while CS is low (max3140 p21). (planner)
- MAX3100 / MAX3140: the T bit clears on CS rising (end of the transaction that read it). (planner)
- COM1863/COM8018: TSB = 1 with 5 data bits → 1.5 stop bits (the AY-5-1013 rule). (planner)
- CDP65C51: TDRE interrupt → level: IRQ stays asserted while TDRE = 1 and the transmit interrupt is enabled, so it recurs while idle (Fig 9). (planner)
- COM92C451: IRQ0 clears as the 16450's INTRPT: THRE by an IIR read that reports it or a THR write, receive data by an RBR read, line status by an LSR read, modem status by an MSR read. (planner)
