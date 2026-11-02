# plan-12 UART feature matrix

Built from all 11 fact sheets in `plan-12-data/specs/*.md`. Every `## <device>` section was read; family sections are split where their members differ.

- **Page references** cite the spec sheet's own citation. They are given only where a value is surprising.
- **"="** means the same as the row named.
- **"–"** means none / not applicable.

## Legend (used in every Table A cell)

**Bus**
- **I**: Intel IOR/IOW.
- **I-dp**: Intel with dual-polarity strobes (RD1/RD2, WR1/WR2 or DISTR/DOSTR), usually with an ADS/AS address latch.
- **I/M(pin)**: Intel or Motorola, chosen by a pin. In Motorola mode CS is the strobe and IOW is R/W̄.
- **I2C/SPI**: register target behind a serial front end.

**Chip select**
- **3CS**: CS0·CS1·/CS2.
- **perCS**: one /CS per channel; "both low" means a broadcast write.
- **CS+CHSEL**: one /CS plus a channel-select pin.
- **CS+A3 / CS+A4:A3**: the Motorola-mode channel bits.

**DMA mode-1 class**
- **P**: PC16550D. Mode-1 TXRDY goes low only when the TX FIFO is empty and high when it is full. Mode-1 RXRDY goes low at the trigger or on timeout and high when the FIFO is empty.
- **N**: mode-1 TXRDY is low while at least 1 slot is free and high when full. RXRDY = P. Most parts are N.
- **T**: mode-1 TXRDY is low while free spaces ≥ the TX trigger. RXRDY also goes high when LSR[7] (error in FIFO) = 1. This is the TI/NXP 752/754/750E style.
- **X**: XR16C864 DRQ/DACK/TC.
- **Mode 0** is the same everywhere: RXRDY is low with ≥1 character; TXRDY is low when empty.

**INT**
- **drv**: always driven.
- **3S/MCR3**: 3-state unless MCR[3] (OUT2) = 1.
- **OD-L**: open-drain, active low.

**Extended register styles**
- **AFE**: TI MCR[5] autoflow only, no bank.
- **650**: LCR=0xBF bank with EFR@2, XON1@4, XON2@5, XOFF1@6, XOFF2@7. EFR[4] write-enables IER[7:4], ISR[5:4], FCR[5:4] and MCR[7:5], and latches them when it is cleared.
- **+DLD**: DLD at address 2 when LCR[7]=1, LCR≠0xBF and EFR[4]=1. DLD[3:0] is a fraction n/16; DLD[5:4] selects 16×, 8× or 4× sampling.
- **+ID**: DREV@0 and DVID@1 read when LCR[7]=1, LCR≠0xBF and DLL=DLM=0. Writes still reach DLL/DLM.
- **+AFR(Exar)**: AFR@2 when LCR[7]=1 and LCR≠0xBF. b0 is concurrent write; b2:1 selects the MF pin: 00 OP2, 01 BAUDOUT, 10 RXRDY.
- **752**: 650 plus three gates:
  - TCR@6 and TLR@7 when EFR[4]=1 and MCR[6]=1.
  - FIFORdy@7 (read-only) when MCR[2]=1 and MCR[4]=0.
  - TI AFR@2 when LCR[7:5]=100: b0 CONC, b1 IREN, b2 485EN, b3 485LG, b4 RCVEN, b7:5 DLY; reset 0x10.
- **854**: LCR=0xBF bank: 000 FC(R)/TRG(W), 001 FCTR, 010 EFR, 100–111 Xon/Xoff. When FCTR[6]=1 and LCR[7]=0, 111 becomes FLVL(R)/EMSR(W) instead of SPR. Includes +ID.
- **5G**: Exar flat map; see the XR16V798 row and Table B.
- **SC16IS**: 16 registers A[3:0], which are the subaddress bits.
  - 0–7 are 16550.
  - 8 TXLVL, 9 RXLVL, A IODir, B IOState, C IOIntEna, D reserved, E IOControl, F EFCR.
  - 650 bank at LCR=0xBF.
  - TCR@6 and TLR@7 when MCR[2]=1 and EFR[4]=1. MCR[2] itself is writable only with EFR[4]=1.

**Timeout styles**
- **4ch**: 4 character times with no RX and no CPU read.
- **4W+12**: 4 × (programmed word length) + 12 bit times. This is the Exar/TI-EFR family.

**Hysteresis tables** are written `trig→RTS-off/RTS-on`, in RX FIFO bytes.

---

## 1. Table A: 16450/16550-register-compatible devices

There are 63 rows in four sub-tables, all in the same order. Bridges and the two Exar "5G" flat-map parts are included for their UART core. Their host side is in Table B.

### A1: structure, bus, FIFO, triggers, DMA

| # | device | spec | ch | host bus | chip select | FIFO | RX trigger | TX trigger | DMA pins (class) |
|---|---|---|---|---|---|---|---|---|---|
| 1 | TL16C450 | ti-a | 1 | I-dp (DISTR/DOSTR, ADS latch) | 3CS, plus CSOUT out | 0 | – | – | – |
| 2 | TL16C451 | ti-a | 1 + LPT | I (IOR/IOW, no ADS) | /CS0 UART, /CS2 LPT (no CS1) | 0 | – | – | – |
| 3 | TL16C452 | ti-a | 2 + LPT | I | /CS0, /CS1 per channel; /CS2 LPT | 0 | – | – | – |
| 4 | TL16C550B | ti-a | 1 | I-dp + ADS | 3CS | 16 | FCR[7:6] 1/4/8/14 | – | TXRDY/RXRDY, one pair. N-like, but mode-1 TXRDY re-assert threshold is unstated |
| 5 | TL16C550C | ti-a | 1 | I-dp + ADS | 3CS | 16 | 1/4/8/14 | – | 1 pair, N. TXRDY re-asserts td21 ≤9 baudout after the next start bit |
| 6 | TL16C550D | ti-a | 1 | PT/PFB: I-dp + ADS. RHB/ZQS: RD1/WR1 only, no ADS | PT/PFB 3CS; RHB/ZQS /CS2 only | 16 | 1/4/8/14 | – | PT/PFB only, N. Absent on RHB/ZQS |
| 7 | PC16550D | ti-a | 1 | I-dp + ADS | 3CS (no CSOUT) | 16 | 1/4/8/14 | – | 1 pair, **P** (the reference) |
| 8 | 82C50A | ti-a | 1 | I-dp (DISTR/DOSTR) + ADS | 3CS + CSOUT | 0 | – | – | – |
| 9 | TL16C552 | ti-b | 2 + LPT | I (no ALE, transparent); BDO pin | /CS0, /CS1, /CS2 (LPT) | 16 | 1/4/8/14 | – | RXRDY0/1, TXRDY0/1, N |
| 10 | TL16C552A | ti-b | 2 + LPT | = 552 | = 552 | 16 | 1/4/8/14 | – | = 552 |
| 11 | TL16C554 | ti-b | 4 | I (transparent) | perCS A–D | 16 | 1/4/8/14 | – | One shared RXRDY and one TXRDY, the AND of the channels' active-low signals. N (pp29–30; the p5 pin text disagrees) |
| 12 | TL16C554A | ti-b | 4 | = 554 | = 554 | 16 | 1/4/8/14 | – | = 554 |
| 13 | TL16C750 | ti-b | 1 | I-dp + ADS | 3CS | 16, or 64 with FCR[5] | 16-byte: 1/4/8/14. 64-byte: 1/16/32/56 | – | 1 pair, N (full = 16 or 64) |
| 14 | TL16C750E | ti-b | 1 | I. MODE pin: IOR/IOW, or "IOW-only" (CS is the strobe, IOW sampled at CS↓ = R/W̄) | single /CS | **128** | FCR 1/4/**120/124** (as printed); TLR[7:4]×8 overrides | FCR[5:4] 16/32/64/120 spaces; TLR[3:0]×8 | 1 pair, T. TXRDY may pulse on the first write |
| 15 | TL16C754B | ti-b | 4 | I, strobes synchronous to XTAL1 (≥2 clocks) | perCS A–D | 64 | 8/16/56/60; TLR×4 | 8/16/32/56; TLR×4 | Shared wire-OR RXRDY/TXRDY, T |
| 16 | TL16C754C | ti-b | 4 | = 754B | perCS | 64 | **1/4/56/60**; TLR×4 | 8/16/32/56; TLR×4 | **none** (PM64). FIFORdy register only |
| 17 | TL16C2550 | ti-c | 2 | I | perCS A/B | 16 | 1/4/8/14 | – | Per channel, PFB only, N |
| 18 | TL16C2552 | ti-c | 2 | I | CS+CHSEL (1 = A) | 16 | 1/4/8/14 | – | TXRDYA/B dedicated; RXRDY only via MF. N |
| 19 | TL16C2752 | ti-c | 2 | I | CS+CHSEL | 64 | FCR + tables A–D (FCTR[5:4]) + TRG. **Values not in the repo PDF** | FCR[5:4] + TRG; values unknown | TXRDYA/B (mode 1 high when full); RXRDY via MF |
| 20 | TL16C752C/D (+CI-Q1, D-Q1) | ti-c | 2 | I | perCS A/B (both low = both) | 64 | 1/4/56/60; TLR×4 | 8/16/32/56; TLR×4 | Per channel, PFB only, T |
| 21 | NS16C2552 | ti-c | 2 | I (RD/WR) | CS+CHSL (1 = ch1) | 16 | 1/4/8/14 | – (FCR[5:4] reserved) | TQFP per channel; PLCC RXRDY via MF. **P** (TXRDY mode 1 low only when empty) |
| 22 | NS16C2752 | ti-c | 2 | I | CS+CHSL | 64 | 8/16/56/60 | 8/16/32/56 | TXRDY mode 1 low when spaces > threshold (T-like) |
| 23 | ST16C450 | exar-a | 1 | I-dp + AS# (transparent low, latch ↑) | 3CS + CSOUT, DDIS# | 0 | – | – | – |
| 24 | ST16C1450 | exar-a | 1 | I | single /CS, no latch | 0 | – | – | – (TXRDY/RXRDY as ISR[4]/[5] status bits when IER[5]=1) |
| 25 | ST16C550 | exar-a | 1 | I-dp + AS# | 3CS, DDIS# | 16 | 1/4/8/14 | – | 1 pair, N |
| 26 | ST16C2550 | exar-a | 2 | I (no AS#) | perCS A/B; both low = write both, read = contention | 16 | 1/4/8/14 | – | Per channel, N |
| 27 | XR16C2550 | exar-a | 2 | = ST16C2550 | = | 16 | = | – | = |
| 28 | ST16C1550 / 1551 | exar-a | 1 | I | single /CS | 16 | 1/4/8/14 | FCR[5:4] 1/4/8/14 (needs IER[5]) | – (inverted status in ISR[5:4] when IER[5]=1) |
| 29 | ST16C650A | exar-a | 1 | SEL pin: I-dp + AS#, or **PC-ISA decode mode** | Intel: 3CS. PC mode: AEN#=0 and A9..A3 = COM1–4 by S3..S1; IOR/IOW pins become A3/A8 | 32 | 8/16/24/28 | FCR[5:4] (EFR4) 16/8/24/30 | 1 pair, Intel mode only, N. In PC mode the pins become IRQC/IRQB |
| 30 | ST16C554 / 554D | exar-a | 4 | I (LQFP64; 16/68# only on PLCC68) | perCS A–D | 16 | 1/4/8/14 | – | None on LQFP64. PLCC68 has the shared AND |
| 31 | XR16L580 | exar-a | 1 | I/M (16/68#; pull-up on TQFP only) | single /CS | 16 | 1/4/8/14 | FCR[5:4] (EFR4) 1/4/8/14 | – (FCR[3] no function) |
| 32 | XR16L2551 | exar-b | 2 | I/M (16/68#) | I: perCS A/B. M: /CS on CSA pin + A3 on CSB pin | 16 | 1/4/8/14 | – | Per channel, TQFP only, N |
| 33 | XR16L2552 | exar-b | 2 | I | CS+CHSEL (1 = A) | 16 | 1/4/8/14 | – | TXRDYA/B; RXRDY dedicated on TQFP, via MF on PLCC. N |
| 34 | XR16M2550 | exar-b | 2 | I | perCS A/B | 16 | 1/4/8/14 | 1/4/8/14 | Per channel, TQFP only, N |
| 35 | XR16M2650 | exar-b | 2 | I | perCS | 32 | 8/16/24/28 | 16/8/24/30 | = M2550 |
| 36 | XR16M2551 | exar-b | 2 | I/M | perCS / CS+A3 | 16 | 1/4/8/14 | 1/4/8/14 | = M2550 |
| 37 | XR16M2651 | exar-b | 2 | I/M | perCS / CS+A3 | 32 | 8/16/24/28 | 16/8/24/30 | = M2550 |
| 38 | XR16M752 / XR68M752 | exar-b | 2 | M752: I. 68M752: I/M (pin 24 pull-up) | perCS / CS+A3 | 64 (census "34B" is wrong) | 8/16/56/60; TLR×4 (count ≥ level) | 8/16/32/56; TLR×4 | Per channel, T |
| 39 | XR16V2550 | exar-c | 2 | I | perCS A/B | 16 | 1/4/8/14 | 1/4/8/14 | Per channel on TQFP only; census QFN32 has none. N |
| 40 | XR16V2650 | exar-c | 2 | I | perCS | 32 | 8/16/24/28 | 16/8/24/30 | = V2550 |
| 41 | XR16V2551 | exar-c | 2 | I/M | perCS / CS+A3 | 16 | 1/4/8/14 | 1/4/8/14 | = V2550 |
| 42 | XR16V2651 | exar-c | 2 | I/M | perCS / CS+A3 | 32 | 8/16/24/28 | 16/8/24/30 | = V2550 |
| 43 | XR16V2552 | exar-c | 2 | I | CS+CHSEL | 16 | 1/4/8/14 | 1/4/8/14 | TXRDYA/B; RXRDY via MF only |
| 44 | XR16V2652 | exar-c | 2 | I | CS+CHSEL | 32 | 8/16/24/28 | 16/8/24/30 | = V2552. QFN32 has no TXRDY or MF |
| 45 | XR16V554 / 554D | exar-d | 4 | I/M (16/68 on PLCC68/QFN48; LQFP Intel only) | perCS A–D (all low = broadcast write) / CS+A4:A3 | 16 | 1/4/8/14 | – | Shared AND TXRDY/RXRDY (PLCC68, LQFP80), N |
| 46 | XR16V564 / 564D | exar-d | 4 | = V554 | = V554 | 32 | 8/16/24/**30** | 16/8/24/30 | = V554 |
| 47 | XR16V654 / 654D | exar-d | 4 | = V554 | = V554 | 64 | 8/16/56/60 | 8/16/32/56 | Shared. QFP100 (not in census) adds per-channel pins and FSTAT |
| 48 | XR16C854 / 854D | exar-d | 4 | I/M (PLCC68/QFP100); LQFP64 Intel only | perCS / CS+A4:A3 | **128** | FCTR[5:4] table: A 1/4/8/14, B 8/16/24/28, C 8/16/56/60, D TRG. FCR[7:6] picks the level | A 1; B 16/8/24/30; C 8/16/32/56; D TRG. **The last table selection applies to both TX and RX** | LQFP64 none. QFP100: shared + per channel + FSTAT, N |
| 49 | XR16C864 | exar-d | 4 | I/M + **host DMA** (DACKA–D, AEN, TC) | perCS / CS+A4:A3 | 128 | = 854 | = 854 | Per channel TXRDY/RXRDY, or **TXDRQ/RXDRQ active-high** via EMSR[2]/[3]. **X** |
| 50 | XR16V798 | exar-d | 8 | I/M (16/68) | /CS + A7..A0: A6:A4 channel, A3:A0 register; A7=1 global block | 64 | FCTR[7:6] tables A–D (D = RXTRG 0–64) | A 1; B 16/8/24/30; C 8/16/32/56; D TXTRG | – (FCR[3] no effect) |
| 51 | XR16L784 | exar-d | 4 | I/M | /CS + A7..A0: A5:A4 channel | 64 | = V798 | = V798 | – |
| 52 | SC16C650B | exar-d | 1 | I. HVQFN32 (census): /CS, /IOR, /IOW. Other packages: I-dp + AS + DDIS | /CS (HVQFN32); 3CS on others | 32 (powers up in 450 mode) | 8/16/24/28 | 16/8/24/30 (EFR4) | Pins exist, but "DMA not supported" on HVQFN32 → mode 0 only |
| 53 | SC16C752B | exar-d | 2 | I, strobes synchronous to XTAL1 | perCS A/B (both low undefined except FIFO Rdy) | 64 | 8/16/56/60; TLR×4 | 8/16/32/56; TLR×4 | Per channel, LQFP48 only, T |
| 54 | SC16IS740 | bridges | 1 | I2C/SPI (SPI 4 MHz) | I2C A1/A0 straps (16 addresses 0x48–0x57) or SPI /CS | 64 | 8/16/56/60; TLR×4 | 8/16/32/56; TLR×4 | – |
| 55 | SC16IS750 | bridges | 1 | I2C/SPI (4 MHz) | = 740 | 64 | = | = | – |
| 56 | SC16IS760 | bridges | 1 | I2C/SPI (15 MHz) | = 740 | 64 | = | = | – |
| 57 | SC16IS752 | bridges | 2 | I2C/SPI (4 MHz) | = 740; channel = subaddress bits 2:1 | 64 | = | = | – |
| 58 | SC16IS762 | bridges | 2 | I2C/SPI (15 MHz) | = 752 | 64 | = | = | – |
| 59 | PI7C9X760 | bridges | 1 | I2C/SPI (SPI 33 MHz) | = SC16IS750 (16-address table) | 64 | = SC16IS (FCR[0]=0 → trigger 1) | = | – |
| 60 | PI7C9X762 | bridges | 2 | I2C/SPI | = 752 | 64 | = | = | – |
| 61 | XR20M1170 | bridges | 1 | I2C/SPI (SPI 18 MHz) | I2C **8 addresses 0x30–0x37** / SPI /CS | 64 | 8/16/56/60; TLR×4 | 8/16/32/56; TLR×4 | – |
| 62 | XR20M1280 | bridges | 1 | I2C/SPI (SPI 26 MHz); **A2:A0 map** | 0x30–0x37 | **128** | FCTR tables A–D (= 854) | = 854 | – |
| 63 | COM92C451 (ACE part) | classic-b | 1 + PIA | I (IOR/IOW) | /CE1 ACE, /CE0 PIA. Both low + IOW = undocumented test mode | 0 | – | – | – |

### A2: extended registers and flow control

| # | device | extended register access | extended registers | auto RTS/CTS | Xon/Xoff + special character |
|---|---|---|---|---|---|
| 1–4, 7, 8 | TL16C450, 451, 452, 550B, PC16550D, 82C50A | – | – (MCR[7:5]=0) | no | no |
| 5 | TL16C550C | AFE | – | MCR[5] AFE. With MCR[1]=1: RTS+CTS; with MCR[1]=0: CTS only. RTS follows the RX trigger: at 1/4/8, off at trigger and on when empty; at 14, off at bit 0 of the 16th character and on when ≥1 free. CTS checked before each byte. No ΔCTS interrupt while on | no |
| 6 | TL16C550D | AFE (no EFR, stated) | – | = 550C | no |
| 9, 10 | TL16C552 / 552A | – | – (LPT registers on CS2) | no | no |
| 11 | TL16C554 | – | – | no | no |
| 12 | TL16C554A | AFE | – | = 550C | no |
| 13 | TL16C750 | **750-style**: FCR[5] writable only while LCR[7]=1; IIR[5]=1 in 64-byte mode | IIR[7:5]: 000 = 450, 110 = 16-byte, 111 = 64-byte | MCR[5] AFE. RTS high at trigger, low when emptied | no |
| 14 | TL16C750E | 752 + **DLF**. DLF@7 when LCR[7]=1, ≠0xBF, EFR[4]=1, and MCR not 0bx1x0x1xx | EFR, Xon1/2, Xoff1/2 (not reset); AFR (0x10); TCR (×8); TLR (×8); FIFO Ready (b0 TX, b4 RX); DLF[5:0] fraction /64, DLF[7] 8×/16× | EFR[6]/[7]; TCR halt/resume ×8; overrides MCR[1] | EFR[3:0] TI table. Xoff at halt, Xon at resume. MCR[5] Xon-any. EFR[5] special vs Xoff2: stored, IIR[4], TX not halted. IER[6]/[7] RTS/CTS interrupts |
| 15 | TL16C754B | 752 without AFR or DLF | EFR, Xon/Xoff, TCR ×4 (0–60), TLR ×4; FIFORdy shared ([3:0] TX A–D, [7:4] RX A–D) | EFR[6]/[7], TCR ×4 | = 750E |
| 16 | TL16C754C | 752 (with AFR) | 754B + AFR. CONC writes all 4 channels | = 754B | = 750E |
| 17 | TL16C2550 | AFE | – | = 554A | no |
| 18 | TL16C2552 | AFE + TI AFR@2 when DLAB=1 | AFR b0 concurrent; b2:1 MF: 00 OP, 01 BAUDOUT, 10 RXRDY; reset 0 | = 554A | no |
| 19 | TL16C2752 | **854-style** + AFR@2 (DLAB) + ID (DVID 0x0A) | FC/TRG, FCTR, EFR, Xon/Xoff, FLVL/EMSR (FCTR[6]), AFR (b0 concurrent, b1 BAUDOUT, b2 RXRDY: **one bit per function**, unlike 2552) | EFR[6]/[7]; hysteresis FCTR[1:0]+EMSR[5:4], encoding not in PDF | EFR[3:0]/[5], MCR[5]; semantics not in PDF |
| 20 | TL16C752C/D | 752 (with AFR) | = 754C, but FIFORdy b0/b1 TX A/B, b4/b5 RX A/B | TCR ×4 | = 754B |
| 21 | NS16C2552 | 650 (at 0xBF, 100–111 hide MCR/LSR/MSR/SCR) + AFR@2 + DREV only | EFR, XON1/2, XOFF1/2; AFR b2:1 MF: 00 OUT2, 01 BAUDOUT, 10 RXRDY, 11 forced 1; DREV | EFR[6] (needs MCR[1]) / EFR[7]. Fixed table 1→2/0, 4→8/1, 8→14/4, 14→14/8. IER[6]/[7] interrupt on 0→1 | Xoff/Xon at 1/0, 4/1, 8/4, 14/8. Not stored. MCR[5] Xon-any. EFR[5] special on XOFF2 |
| 22 | NS16C2752 | = NS16C2552 | = | 8→16/0, 16→56/8, 56→60/16, 60→60/56 | Xoff/Xon at the same levels |
| 23, 25, 26, 27 | ST16C450, ST16C550, ST16C2550, XR16C2550 | – | – | no | no |
| 24 | ST16C1450 | **IER[5] "special mode" gate** | IER[5] unlocks ISR[5:4] (RXRDY/TXRDY, active high), MCR[2] soft reset → RST pin, MCR[7] power-down | no (removed in Rev 4.2.0) | no |
| 28 | ST16C1550/1551 | IER[5] gate | = 1450, plus FCR[5:4] TX trigger | no | no |
| 29 | ST16C650A | 650 + ID (DVID 0x04, DREV 0x01) | Also XFR@5 and IRPW@6 (write-only; LCR[7]=0 and EFR[4]=1). XFR: [0] IrDA half-duplex, [1] IR RX invert, [2] RS-485, [3] LSR-int timing, [4] Xon-any, [5] RS-485 polarity | EFR[6] with MCR[1]: 8→16/0, 16→24/8, 24→28/16, 28→28/24. EFR[7] CTS stops after current stop. ISR 0x20. Not in loopback | EFR[3:0] Table 12 (Exar "or" for 1011/0111). Xoff 2 char times after trigger; Xon one level below. Not stored. XFR[4] Xon-any. EFR[5] special vs XOFF2. Word-length bits only |
| 30 | ST16C554/554D | – | – | no (stray text) | no |
| 31 | XR16L580 | 650 + ID (DVID 0x01). EFR[4] also gates MCR[2] | Xon/Xoff write-only | Next level: 8→14/4; trigger-14 off level undefined (assume 16). Works in loopback | = 650A table; Xon one level below (1→0 …). MCR[5] Xon-any |
| 32 | XR16L2551 | 650 + ID (DVID 0x02) | – | Next upper/lower (8→14/4); ends undefined → 16/0. Not in loopback | 1/0, 4/1, 8/4, 14/8. MCR[5] Xon-any. EFR[5] special |
| 33 | XR16L2552 | 650 + AFR(Exar) + ID | AFR | = L2551 | = L2551 |
| 34, 36 | XR16M2550 / M2551 | 650 + DLD + ID (DVID 0x02) | DLD | 1→4/0, 4→8/1, 8→14/4, 14→14/8 | 1/0, 4/1, 8/4, 14/8 |
| 35, 37 | XR16M2650 / M2651 | = M2550 (DVID 0x06) | DLD | 8→16/0, 16→24/8, 24→28/16, 28→28/24 | 8/0, 16/8, 24/16, 28/24 |
| 38 | XR16M752 / XR68M752 | 650 + DLD + **TCR@6 / TLR@7** (EFR[4] and MCR[6]) + **FIFO Rdy@7** (MCR[2]=1, MCR[4]=0; priority over SPR/TLR). **No ID** | DLD[6] = auto RS-485, DLD[7] = IrDA enable. TCR reset **0x0F**. FIFO Rdy reset 0x03 | TCR halt/resume ×4; reset → 60/0 | Xoff/Xon at TCR levels. MCR[5] Xon-any. EFR[5] special |
| 39–42 | XR16V2550/2650/2551/2651 | 650 + DLD + ID (DVID 0x02 / 0x06) | DLD | 16-byte: 1→4/0, 4→8/1, 8→14/4, 14→14/8. 32-byte: 8→16/0, 16→24/8, 24→28/16, 28→28/24 | Same levels as RTS |
| 43, 44 | XR16V2552 / 2652 | 650 + DLD + ID + AFR(Exar). **AFR@2 when EFR[4]=0, DLD@2 when EFR[4]=1** | AFR, DLD | = 2550 / 2650 | = |
| 45 | XR16V554 | – ("LCR≠0xBF" is boilerplate) | – | no | no |
| 46 | XR16V564 | 650 + DLD (no ID) | DLD | 8→16/0, 16→24/8, 24→30/16, 30→30/24 | Xon at 0/8/16/24 |
| 47 | XR16V654 | = V564 | DLD; FSTAT (QFP100, FSRS pin, any address) | 8→16/0, 16→56/8, 56→60/16, 60→60/56 | Xon at 0/8/16/56 |
| 48 | XR16C854 | **854** + ID (DVID 0x14) | FC/TRG (FCTR[7] selects RX or TX); FCTR (b1:0 hysteresis, b2 IR RX invert, b3 "RS-485" = TX interrupt on TSR empty only, b5:4 table, b6 SPR swap, b7 TRG/FC select); EMSR (b1:0 FLVL mode RX/TX/RX/alternating, b5:4 hysteresis); FLVL; FSTAT | Tables A–C: next level. Table D: trigger ± hysteresis from EMSR[5:4]:FCTR[1:0] = 0, 4, 6, 8, 8, 16, 24, 32, 40, 44, 48, 52, 12, 20, 28, 36 | Exar EFR. Xon one level below, or trigger − hysteresis. MCR[5]. EFR[5] |
| 49 | XR16C864 | = 854 + EMSR[3:2] DMA-request enables. DVID 0x14 (same as 854) | = 854 | = 854 | = 854 |
| 50 | XR16V798 | **5G flat**. Fixed offsets 8–F. DLD@2 when LCR[7]=1 (no EFR or 0xBF gate). EFR[4] gates IER[7:5], ISR[5:4], FCR[5:4], MCR[7:5], MCR[3:2], MSR-write[7:2] | Offsets: 8 FCTR, 9 EFR, A TXCNT/TXTRG, B RXCNT/RXTRG, C XCHAR(R)/XOFF1(W), D XOFF2, E XON1, F XON2 (Xon/Xoff write-only). **A write to 6 is MSR-write**: [7:4] RS-485 delay, [3] TX disable, [2] RX disable. Global block 0x80–0x8E (Table B) | RTS/CTS or **DTR/DSR (MCR[2] selects)**. Tables A–C next level; D ± FCTR[3:0] hysteresis (same 16 values) | XCHAR status bits (b3 sent Xon, b2 sent Xoff, b1 got Xon, b0 got Xoff). **Send-immediate** via MCR[3] |
| 51 | XR16L784 | 5G, **no DLD** | = V798. MSR-write [7:4] delay only | = V798 | XCHAR b1:0 only; no send-immediate |
| 52 | SC16C650B | 650 (NXP); no ID | – | EFR[6]/[7]; threshold table disputed (§4) | **NXP "and" (sequence) for 1011/0111**. Special requires EFR[3:0]=0000. **No Xon-any** (MCR[5] = INT type) |
| 53 | SC16C752B | 752 without AFR | TCR, TLR, FIFO Rdy (b0/1 TX, b4/5 RX); Xon/Xoff not reset | TCR ×4 | NXP "and". MCR[5] Xon-any (Xon1 may act as Xon-any). Special on Xoff2 |
| 54–58 | SC16IS740/750/760/752/762 | SC16IS | TCR/TLR; TXLVL/RXLVL; IODir/IOState/IOIntEna/IOControl (shared across channels on 752/762); EFCR: [0] 9-bit, [1] RX disable, [2] TX disable, [4] RTSCON, [5] RTS invert, [7] fast IrDA (760/762) | TCR halt/resume ×4; **TCR=0 → FCR trigger levels**. CTS before mid-stop | EFR[3:0] (654-style; "or" for 10/01, sequence for 11/00). Sent at TCR/FCR levels. MCR[5]. EFR[5] special: stored, TX not halted |
| 59, 60 | PI7C9X760/762 | SC16IS + **SFREN(0x0D)=0x5A unlocks SFR@7 (LCR=0xBF)**; SFR[2] maps the special set over 0x02, 0x04–0x09, 0x0F | SFR bits; ASR, CPR (M/N), RFD/RLS, TFD, TIDLE, TRCTL (sample clock, RX timeout enable …), ISCR | = SC16IS + SFR[1] auto DSR/DTR | = SC16IS |
| 61 | XR20M1170 | SC16IS + DLD@2 (LCR[7]=1, EFR[4]=1). EFR[4] also gates MCR[2], TCR, TLR, DLD | = SC16IS + DLD | TCR (reset **0x0F**) | = SC16IS |
| 62 | XR20M1280 | **1280 style (A2:A0)**. LCR[7]=0: 5 W=SHR, 6 W=SFR (EFR4), 7 = SPR / GPIOLVL (SFR[0]) / EMSR-FC (FCTR[6]). DLAB: DLL/DREV, DLM, DLD (DLD[7:6] independent TX/RX baud generators). 0xBF: FC/TRG, FCTR, EFR, Xon/Xoff, or GPIOINT/GPIO3T/GPIOINV/GPIOSEL when SFR[0]=1 | SHR (RS-485 setup/turnaround delays or hysteresis), SFR, EMSR, FC, TRG (reset 0x01), FCTR (b0 SLEEP/PWRDN# pin select), DREV, GPIO set | Tables A–C next level; D SHR hysteresis (854 values) | Exar. Xoff 2 char times after trigger. EMSR[7] Xoff interrupt mode |
| 63 | COM92C451 | – | – | no (CTS/DCD don't affect TX/RX) | no |

### A3: power, IrDA, RS-485, baud generator, pins

| # | device | sleep / power-save | IrDA | RS-485 auto direction | prescaler / sampling / fractional divisor | clock pins | GPIO / output pins | printer port |
|---|---|---|---|---|---|---|---|---|
| 1 | TL16C450 | – | – | – | – / 16× / – | XTAL1, XTAL2, RCLK, BAUDOUT (9 MHz) | OUT1, OUT2 | – |
| 2 | TL16C451 | – | – | – | – / 16× | **single CLK** (RCLK = BAUDOUT internally) | none (MCR[2] loopback only; MCR[3] = INT enable) | **yes**: data/status/control, LPTOE, INT2 |
| 3 | TL16C452 | – | – | – | – | CLK | none | yes |
| 4 | TL16C550B | – | – | – | – / 16× | XIN, XOUT, RCLK, BAUDOUT (9 MHz) | OUT1, OUT2 | – |
| 5 | TL16C550C | – | – | – | – / 16× | = 550B (16 MHz at 5 V) | OUT1, OUT2 | – |
| 6 | TL16C550D | – | – | – | – / 16× | PT/PFB = 550C. RHB/ZQS: XIN/XOUT only (RCLK = BAUDOUT internally). 24 MHz at 5 V | OUT1/OUT2 on PT/PFB only | – |
| 7 | PC16550D | – | – | – | – / 16× (24 MHz, divisor ≥1) | XIN, XOUT, RCLK, BAUDOUT | OUT1, OUT2 | – |
| 8 | 82C50A | – | – | – | – / 16× | XTAL1, XTAL2, RCLK, BAUDOUT (10 MHz) | OUT1, OUT2 | – |
| 9 | TL16C552 | – | – | – | – / 16× | **CLK only** (≤8 MHz), shared by both channels | none (OUT2 = INT enable) | **yes**: PEMD, ENIRQ, TRI, BDO, DIR bit |
| 10 | TL16C552A | – | – | – | – | CLK ≤16 MHz | none | yes (PS/2 INT2 latch, readback differs) |
| 11, 12 | TL16C554 / 554A | – | – | – | – / 16× | XTAL1, XTAL2 (≤16 MHz); no BAUDOUT/RCLK | none | – |
| 13 | TL16C750 | **IER[4] sleep; IER[5] low-power** (internal clock stopped, oscillator runs). Awake while TX holds data, SIN active, loopback, or an MSR delta is set | – | – | – / 16× | XIN, XOUT, BAUDOUT, RCLK | OUT1, OUT2 | – |
| 14 | TL16C750E | EFR[4] + IER[4]. Entry: RX idle, TX FIFO and TSR empty, RX FIFO empty, no interrupt except THR/timeout. Wake: RX change, modem change, TX write. Don't write DLL/DLH asleep | AFR[1] IREN, SIR ≤115.2k, RCVEN gate | AFR[2] on **DTR**, high while TX; DLY AFR[7:5] bit times after stop; 485LG; RCVEN | MCR[7] ÷1/÷4; DLF[7] 8×/16×; DLF[5:0] /64 | XTAL1, XTAL2 (external ≤48 MHz) | OP = NOT MCR[3] | – |
| 15 | TL16C754B | = 750E (per channel) | – | – | MCR[7] ÷4; **CLKSEL pin loads MCR[7] = NOT CLKSEL at reset**; 16× | XTAL1, XTAL2, CLKSEL | none | – |
| 16 | TL16C754C | = 750E | AFR[1] (TX idles low) | AFR[2] on DTRx, DLY 0–7 | MCR[7] (no CLKSEL pin) | XTAL1, XTAL2 | none | – |
| 17 | TL16C2550 | – | – | – | – / 16× | XTAL1, XTAL2 only (feature list wrongly claims RCLK) | OPA/OPB = NOT MCR[3] (PFB) | – |
| 18 | TL16C2552 | – | – | – | – | XTAL1, XTAL2; BAUDOUT via MF | MFA/MFB: OP / BAUDOUT / RXRDY | – |
| 19 | TL16C2752 | IER[4] (conditions not given) | MCR[6]; FCTR[2] RX invert | FCTR[3]; EMSR[3] invert (pin/timing not given) | MCR[7]; EMSR[7] 16×/8× | XTAL1, XTAL2; BAUDOUT via MF | MF | – |
| 20 | TL16C752C/D | = 754B | = 754C | = 754C | MCR[7] (text cites a nonexistent CLKSEL) | XTAL1, XTAL2 (D: 48 MHz at 3.3 V) | OPA/OPB (PFB) | – |
| 21 | NS16C2552 | IER[4]. Entry: no interrupt, divisor ≠ 0, MSR[3:0]=0, SIN idle. Wake: SIN↓, THR write, modem change | MCR[6] (3/16, idle 0) | – | MCR[7] ÷4 per channel / 16× | XIN, XOUT (external ≤80 MHz); BAUDOUT via MF (also RX clock) | MF: OUT2 / BAUDOUT / RXRDY | – |
| 22 | NS16C2752 | = 2552 | = | – | = | = | = | – |
| 23 | ST16C450 | – | – | – | – / 16× | XTAL1, XTAL2, RCLK (RX only), BAUDOUT# | OP1#, OP2# | – |
| 24 | ST16C1450 | MCR[7] power-down (IER[5]) | – | – | – | XTAL1, XTAL2 only | **RST output** = RESET OR (IER[5]·MCR[2]) | – |
| 25 | ST16C550 | – | – | – | – / 16× | XTAL1, XTAL2, RCLK, BAUDOUT# | OP1#, OP2# | – |
| 26, 27 | ST16C2550 / XR16C2550 | – | – | – | – (Table 5 "MCR[7]" is a leftover) | XTAL1, XTAL2 (external ≤64 MHz at 5 V) | OP2A#/OP2B# (no OP1) | – |
| 28 | ST16C1550/1551 | MCR[7] power-down (IER[5]); oscillator/BRG stopped, bus alive | – | – | none (MCR[7] conflict, §4) | XTAL1, XTAL2; 1551 PLCC28 CLK only | RST output | – |
| 29 | ST16C650A | IER[4]. Entry: nothing pending, MSR[3:0]=0, RX idle. Oscillator stops. Wake: RX↓, THR write, modem change → wake interrupt reads ISR 0x01. Re-sleeps | MCR[6]; pulse 3/16 or IRPW×XTAL1; XFR[1] RX invert; XFR[0] half-duplex | XFR[2] on **RTS#**; XFR[5] polarity; no delay | MCR[7] ÷4 / 16× | XTAL1, XTAL2, RCLK, BAUDOUT# (Intel) | OP1#, OP2# (Intel) | PC mode decode outputs **LPT1#/LPT2#** only |
| 30 | ST16C554/554D | – | – | – | – | XTAL1, XTAL2 | none | – |
| 31 | XR16L580 | IER[4] (+ divisor ≠ 0); **PwrSave pin** isolates bus; then wakes on RX/modem only | MCR[6], fixed 3/16; MCR[2] RX invert | – | MCR[7] / 16× | XTAL1, XTAL2 | none | – |
| 32 | XR16L2551 | IER[4] on **both channels** + divisor ≠ 0; PWRSAVE pin | MCR[6] (no polarity bit) | – | MCR[7] / 16× / – | XTAL1, XTAL2 | OP2A#/OP2B# (TQFP) | – |
| 33 | XR16L2552 | = L2551, no PWRSAVE | MCR[6] | – | MCR[7] | XTAL1, XTAL2; BAUDOUT via MF | MF | – |
| 34, 35 | XR16M2550 / 2650 | IER[4] both channels (no divisor condition) | MCR[6]; **MCR[2] RX polarity** | – | MCR[7] / DLD 16/8/4× / n/16 | XTAL1, XTAL2 | OP2A/B# (TQFP) | – |
| 36, 37 | XR16M2551 / 2651 | = M2550 + **PWRSAVE** + **wake-up indicator** (ISR 0x01) | = M2550 | – | = M2550 + **CLKSEL pin (TQFP only)** | + CLKSEL | OP2A/B# | – |
| 38 | XR16M752 | IER[4] both channels (PwrSave pin only on STBGA, not in census) | **DLD[7]** | **DLD[6] on RTS#, HIGH = TX**; released 1 bit after stop; TX interrupt waits for TSR empty | MCR[7] / DLD | XTAL1, XTAL2 | OP2A/B# | – |
| 39, 40 | XR16V2550 / 2650 | IER[4] both channels | MCR[6] + MCR[2] polarity | – | MCR[7] / DLD | XTAL1, XTAL2 | OP2# (TQFP) | – |
| 41 | XR16V2551 | + PWRSAVE + wake-up indicator | = | – | = (**no CLKSEL**) | XTAL1, XTAL2 | OP2# | – |
| 42 | XR16V2651 | = 2551 | = | – | = + **CLKSEL (TQFP pin 25)** | + CLKSEL | OP2# | – |
| 43, 44 | XR16V2552 / 2652 | = 2550 | = | – | = | XTAL1, XTAL2; BAUDOUT# via MF | MF (OP2# default) | – |
| 45 | XR16V554 | – | – | – | – (text claims DLD/MCR[7]; not in map) | XTAL1, XTAL2 | none | – |
| 46 | XR16V564 | IER[4] on **all 4** channels; oscillator stops | MCR[6] | – | MCR[7] ÷4 preset by **CLKSEL**; DLD 16/8/4× | XTAL1, XTAL2, CLKSEL (not QFN48) | none | – |
| 47 | XR16V654 | = 564 | = (QFP100 IRTX pins always encoded) | – | = 564 | + CHCCLK (QFP100) | none | – |
| 48 | XR16C854 | = 564 | MCR[6]; FCTR[2] RX invert | – (FCTR[3] only changes the TX interrupt) | MCR[7]/CLKSEL; **16× only, integer** | XTAL1, XTAL2, CLKSEL, CHCCLK | none | – |
| 49 | XR16C864 | = 854 | = 854 | FCTR[3] on **OP1x#**, LOW = TX, no delay | = 854 | + **BCLKA–D** outputs (16×) | OP1A–D#, OP2A–D# | – |
| 50 | XR16V798 | **Global SLEEP register** (per-channel bits); oscillator stops when all 8 sleep; wake interrupt | MCR[6] preset by **ENIR pin**; FCTR[4] RX invert; RX disabled during TX | FCTR[5], RTS# or DTR#; delay 0–15 bits from MSR-write[7:4] | MCR[7] ÷4; **global 8XMODE** 16×/8×; DLD n/16 | XTAL1, XTAL2, TMRCK | none (ENIR strap) | – |
| 51 | XR16L784 | SLEEP register bits 3:0; wake cleared by an INT0 read | = 798 | = 798 | MCR[7]; 8XMODE; **integer only** | XTAL1, XTAL2, TMRCK | none | – |
| 52 | SC16C650B | EFR[4] + IER[4]; sleeps when nothing pending; XTAL2 floats | MCR[6] | – | MCR[7] / 16× | XTAL1, XTAL2, **BAUDOUT, RCLK** (both on HVQFN32) | OUT (MCR[2]); OUT2 not on HVQFN32 | – |
| 53 | SC16C752B | EFR[4] + IER[4], **per channel** (750E rules) | – | – | MCR[7] / 16× | XTAL1, XTAL2 | OPA/OPB = NOT MCR[3] | – |
| 54 | SC16IS740 | EFR[4] + IER[4] (750E rules); no GPIO wake | MCR[6], 3/16 | EFCR[4] RTSCON (RTS low on TX), EFCR[5] invert; 9-bit multidrop EFCR[0] | MCR[7] / 16× | XTAL1, XTAL2 | **none** (no GPIO, no modem pins) | – |
| 55, 57 | SC16IS750 / 752 | = 740 | = 740 | = 740 | = | = | GPIO0–7; IOControl turns 7:4 (and 3:0 for channel B on 752) into modem pins | – |
| 56, 58 | SC16IS760 / 762 | = | + EFCR[7] ¼-bit fast IrDA (1.152M) | = | = | = | = 750/752 | – |
| 59, 60 | PI7C9X760/762 | IER[4]; crystal stops | MCR[6]; EFCR[7] 1.0/1.1 | = SC16IS | **CPR M/N + TRCTL SCR**: baud = XIN / (DL × 2^(M+2·MCR7−1) × (16 − SCR + N)) | XTAL1, XTAL2 | GPIO0–7 | – |
| 61 | XR20M1170 | IER[4]; oscillator stops | MCR[6]; EFCR[7] | EFCR[4]/[5]; TX interrupt = TSR empty | MCR[7] / DLD 16/8/4× / n/16 | XTAL1, XTAL2 | GPIO0–7 (24-pin) | – |
| 62 | XR20M1280 | IER[4] + **SLEEP pin / PWRDN# output** (FCTR[0]); FCR[3] wake interrupt | MCR[6] or **ENIR#** strap; SFR[3] fast; FCTR[2] invert | FCTR[3] or **EN485#** strap; SHR setup + turnaround 0–15; EMSR[3] invert; 9-bit SFR[6:7] | MCR[7] / DLD / **independent TX/RX baud generators** | XTAL1, XTAL2 | 4 or 8 GPIO; **MCR[2]=1 = GPIO** (inverse of SC16IS) | – |
| 63 | COM92C451 | – | – | – | – (divisor 0 → ÷3) | **single CLK** ≤10 MHz; no XTAL/RCLK/BAUDOUT | OUT2 pin only (no OUT1) | **yes** (PIA: data/status/control, IRQ0) |

### A4: INT, reset, ID, counters, timeout, oddities

| # | device | INT pin | reset pin and notable values | ID / rev | FIFO counters | RX timeout | oddities |
|---|---|---|---|---|---|---|---|
| 1 | TL16C450 | INTRPT high, **drv** (no OUT2 gating) | MR high ≥1 µs; LCR not in table → 0 | – | – | – | DDIS, CSOUT; BAUDOUT N=1..3 waveform rules |
| 2, 3 | TL16C451 / 452 | INT0 (and INT1) high, **3S/MCR3**; INT2 = LPT | **RESET active LOW** | – | – | – | BDO pin (polarity conflict); td11 = 8 baudout |
| 4 | TL16C550B | INTRPT high, drv | MR high ≥1 µs | – | – | 4ch, td13 8 RCLK | 425 ns RX-read → status spacing; THRE delay rule |
| 5, 6 | TL16C550C / D | drv (explicitly not OUT2-gated) | MR high | – | – | 4ch, 9 RCLK | D: package pin subsets (RHB/ZQS lack modem/DMA pins) |
| 7 | PC16550D | INTR high, drv | MR high ≥5 µs; SCR reset unstated | – | – | 4ch, +8 RCLK; FIFO status +3 RCLK | IIR freeze during read; LSR write rules |
| 8 | 82C50A | drv | MR high ≥500 ns | – | – | – | **Checks 2 stop bits**; status bits set at end of a read (or cleared if they recur); sticky errors; break sets DR |
| 9, 10 | TL16C552 / 552A | INT0/1 high, 3S/MCR3 (552A: OUT2 = "external interrupt enable") | **RESET active LOW**; LCR reset conflict | – | – | 4ch | Status set deferred to end of LSR/MSR read |
| 11, 12 | TL16C554 / 554A | INTA–D high. **INTN pin**: low/open → 3S/MCR3, high → drv | RESET high; LCR conflict | – | – | 4ch | – |
| 13 | TL16C750 | drv | MR high | – | – | 4ch | FE resync samples twice; break resume after 2 RCLK |
| 14 | TL16C750E | 3S/MCR3 | RESET high ≥200 ns; **LCR=0x1D**; AFR 0x10; DLL/DLH/SPR/Xon not reset | – | – (FIFO Ready bits) | **4W+12** | 3-sample majority; LSR[4:2] from top of FIFO (0 when empty); 6-level IIR (Xoff 0x10, CTS/RTS 0x20); RCVEN=0 also kills the timeout |
| 15 | TL16C754B | INTA–D: OE = **INTSEL pin** OR MCR[3] | RESET high; LCR=0x1D; MCR[7] = NOT CLKSEL | – | – | 4W+12 | Pin-compatible with 554 (CLKSEL/INTSEL replace NC/INTN) |
| 16 | TL16C754C | 3S/MCR3 (no INTSEL; inferred) | = 754B | – | – | 4W+12 | IIR modem code printed 001000 vs 000000 |
| 17 | TL16C2550 | 3S/MCR3 | RESET high; LCR 0 | – | – | 4ch | RHB lacks modem/OP/DMA pins |
| 18 | TL16C2552 | 3S/MCR3 (reset conflict: MCR=0x08?) | RESET high | – | – | 4ch | – |
| 19 | TL16C2752 | 3S/MCR3 (same conflict) | RESET high; LCR 0; SPR FF | **DVID 0x0A, DREV 0x00** | FC, FLVL | – (not described) | **Undecodable**: bit semantics absent (an XR16L2752 clone) |
| 20 | TL16C752C/D | 3S/MCR3 | RESET high; **LCR=0x1D**; AFR 0x10 | – | – | 4W+12 | RHB pinout differs from 2550 RHB |
| 21 | NS16C2552 | INTR1/2 high, **drv** | MR high ≥70 ns + internal POR (2^23 XIN); **SCR=0xFF** | **DREV = 0x31** (ID 3, rev 1) | – | 4W+12 (from BAUDOUT) | **7-level IIR**: CTS/RTS cleared by an IIR **or MSR** read; LSR errors reset when the next good character loads; 5 V-tolerant |
| 22 | NS16C2752 | = | = | DREV 0x21 | – | 4W+12 | TX 2-character hysteresis; TX FIFO reset raises THRE |
| 23 | ST16C450 | drv | RESET high ≥40 ns; SPR FF in Table 4 only | – | – | – | CSOUT, DDIS#; start validated at 7½ |
| 24 | ST16C1450 | **3S/MCR3** | RESET high | – | – | – | **THR interrupt clears only on (ISR read AND IER[1]=0) or THR write** (not Windows-compatible); start at 8 |
| 25 | ST16C550 | drv | RESET high | – | – | **4W+12** | – |
| 26, 27 | ST16C2550 / XR16C2550 | 3S/MCR3 | RESET high | – | – | 4W+12 | **IIR priority: timeout above RX data**; IER[1] fires if already empty; LSR[4] holds until RX idle |
| 28 | ST16C1550/1551 | 3S/MCR3 | RESET high | – | – | 4W+12 | = 1450 THR-interrupt rule |
| 29 | ST16C650A | Intel: MCR[5]=0 push-pull, MCR[5]=1 **open-source**. PC mode: IRQA/B/C per S3, MCR[3] | RESET high | **DVID 0x04, DREV 0x01** | – | 4W+12 | 6-bit ISR codes; XFR[3] LSR-interrupt timing; wake interrupt = ISR 0x01 |
| 30 | ST16C554/554D | **INTSEL** (CQ64 strap 0 → 3S/MCR3; D parts strap 1 → drv) | RESET high; DLL/DLM not reset | – | – | 4W+12 | LSR/MSR writable |
| 31 | XR16L580 | Intel 3S/MCR3; Motorola **IRQ# OD-L** | Intel high / Motorola low; DLL/DLM power-up 0x0001 only | **DVID 0x01, DREV 0x01** | – | 4W+12 | – |
| 32 | XR16L2551 | Intel 3S/MCR3; Motorola single IRQ# OD-L (OR of channels); INTB drives 0 | Intel high / Motorola low; DLL/DLM XX | DVID 0x02, DREV 0x01 | – | 4W+12 | – |
| 33 | XR16L2552 | **drv** (no MCR[3] gating) | high; AFR 0 | 0x02 / 0x01 | – | 4W+12 | – |
| 34, 35 | XR16M2550 / 2650 | 3S/MCR3 | high; **DLL/DLM 01/00 at power-up only** | 0x02 / 0x06 | – | 4W+12 | QFN32 lacks DSR/CD/RI |
| 36, 37 | XR16M2551 / 2651 | Intel 3S/MCR3; Motorola IRQ# OD-L | Intel high / Motorola low | 0x02 / 0x06 | – | 4W+12 | – |
| 38 | XR16M752 | Intel 3S/MCR3; Motorola IRQ# OD-L (combining unstated) | Intel high / Motorola low; **LCR=0x1D**; TCR 0x0F | **none** | – | 4W+12 | LSR interrupt holds while LSR[7]=1 |
| 39, 40 | XR16V2550 / 2650 | 3S/MCR3 | high; DLL/DLM power-up only | 0x02 / 0x06 | – | 4W+12 | 5 V-tolerant except XTAL1 |
| 41, 42 | XR16V2551 / 2651 | Intel 3S/MCR3; Motorola IRQ# OD-L, MCR[3] must be 0 | Intel high / Motorola low | 0x02 / 0x06 | – | 4W+12 | – |
| 43, 44 | XR16V2552 / 2652 | **drv** | high | 0x02 / 0x06 | – | 4W+12 | – |
| 45–47 | XR16V554 / 564 / 654 | INTSEL/MCR[3] table (LQFP: IV = 0, DIV = 1); Motorola single IRQ OD-L on INTA | Intel high / Motorola low; SPR FF; DLL/DLM power-up 01/00 | – | – | 4W+12 | Timeout above RX data; V554/564 LSR/MSR writable; V654 read-only |
| 48 | XR16C854 | = V554 | DLL/DLM **XX**; MCR[7] from CLKSEL | **DVID 0x14**, DREV 0x01 | **FC, FLVL** | 4W+12 | IOR+IOW+CS low = factory test mode |
| 49 | XR16C864 | INTSEL is a real pin | = 854 | 0x14 (indistinguishable) | FC, FLVL | 4W+12 | – |
| 50 | XR16V798 | **One INT# for the device, OD-L**; INT0–3 summary registers | **RST# LOW, no power-on reset**; per-channel soft reset | **DVID 0x48, DREV 0x01** (global 0x8C/0x8D) | **TXCNT, RXCNT** | 4ch + 12 bits | **IIR priority: RX data above timeout**; IER[1] interrupts twice; 16-bit timer |
| 51 | XR16L784 | = 798 | RST# LOW; DLL/DLM XX | **DVID 0x24** | TXCNT, RXCNT | timeout interrupt clears when the FIFO is empty | Bit-mapped TIMERCNTL (798 uses command codes) |
| 52 | SC16C650B | high **drv**; MCR[5] open-source option | RESET high; DLL/DLM XX | – | – | **4ch** | **LSR[7] clears on LSR read**; RX data and timeout share priority; start at 7½ |
| 53 | SC16C752B | 3S/MCR3 | RESET high; **LCR=0x1D**; DLL/DLM/SPR/Xon not reset | – | – | 4ch | Bus synchronous to XTAL1 |
| 54–58 | SC16IS7xx | **IRQ OD-L, always** (IIR[0]=0); one per device | **RESET LOW** ≥3 µs = POR = IOControl SRESET; **LCR=0x1D**; TXLVL 0x40; DLL/DLH/SPR/Xon not reset | – | **TXLVL, RXLVL** | 4ch | IIR codes add 0x30 GPIO; 740 lacks modem/GPIO interrupts |
| 59, 60 | PI7C9X760/762 | IRQ OD-L | RESET LOW; DLL 0x01, SPR FF, CPR 0x10, TRCTL 0x06 (DLL/SPR/Xon POR only) | – | TXLVL, RXLVL, RFD, RLS, TFD | 4ch (TRCTL[1] enable) | **I2C NACK on THR write when the TX FIFO is full** |
| 61 | XR20M1170 | IRQ# OD-L | RESET# LOW; **LCR 0x1D, TCR 0x0F**, DLL 0x01, SPR FF (power-up only) | – | TXLVL, RXLVL | 4W+12 | NACK when full; IER[1] re-enable does interrupt |
| 62 | XR20M1280 | IRQ# OD-L | RESET# LOW; **LCR 0x00**; TRG 0x01 | DREV (value not printed) | FC (+FLVL via SPR swap) | 4W+12 | ISR[6] = GPIO flag; NACK when full |
| 63 | COM92C451 | IRQ1 high, not OUT2-gated (3-state unstated) | RESET high | – | – | – | FE resync samples twice; loopback forces outputs high |

---

## 2. Table B: other devices (non-16550 register model), plus bridge host protocols

| device | spec | register / interface model | shares with (parameters) |
|---|---|---|---|
| **AY-5-1013A family** (AY-5-1013A, AY-6-1013, AY-3-1014A, AY-3-1015D) | classic-a | **Pin-parallel, no registers.**<br>• Control word NP, TSB, NB2:NB1 (5–8 bits), EPS (1 = even): transparent while CS=1, latched on CS↓, one word for TX and RX.<br>• TX: DB1–8; DS̅ loads the holding register. TBMT and EOC flags.<br>• RX: RD1–8, 3-state under RDE̅. PE, FE, OR, DAV flags, 3-state under SWE̅; RDAV̅ clears DAV.<br>• XR active high.<br>• External 16× clocks TCP/RCP; no BRG; no INT. | `ay1013`. Parameters:<br>• pin-2 role (VGG / NC / −12 V / HIACC / MODE)<br>• 1.5-stop rule for 5-bit + TSB (1013: 2; 1014A/1015D: 1.5)<br>• reset clears RX holding register (1014A/1015D yes; 1013 no)<br>• start-sample offset (8½/7½ vs 8/7)<br>• DAV delay (3 µs vs 1 RCP)<br>• max clock (360k–640k)<br>• input pull-ups |
| **HD-6402** | classic-a | = AY-1013 with renamed pins (CRL, TBRL, DRR, RRD, SFD, MR, …). MR does **not** clear the RBR. 1.5 stop bits; unused RBR MSBs read 0; transfer at 7½ into stop 1. Up to 8 MHz (B grade). | `ay1013` (pin 2 NC) |
| **COM2017 / COM2502** | classic-a | = AY-1013. Data and control latched on the trailing edge of the strobe. COM2017 gives 1.5 stop; COM2502 gives 2. P-channel; pin 2 = −12 V; 400 kHz. | `ay1013` |
| **COM8017 / COM8502** | classic-a | = COM2017; +5 V NMOS; pin 2 NC; pull-ups; 640 kHz. | `ay1013` |
| **COM1863 / COM8018** | classic-a | = AY-1013, plus:<br>• MR clears RD1–8.<br>• ROR is qualified by an RDAR edge.<br>• COM8018 HIACC (pin 2 = 1): 32× clocks and an RDA notch.<br>• 1 MHz; pull-ups on COM8018 only. | `ay1013` (`hiacc`, `rorRdarEdge`, `resetClearsRbr`) |
| **CDP1854A / AC, mode 0** (MODE = VSS) | classic-a | TR1602/AY-1013-compatible pin-parallel. MR also clears the control register and RX holding register. CMOS; 1.5 stop; 7½-clock sampling. | `ay1013` |
| **CDP1854A / AC, mode 1** (MODE = VDD) | classic-a | **CDP1800 bus register model.** R and T buses tied together; TPB strobe, RD/WR̅, RSEL, CS1·/CS2·CS3.<br>• Registers: RSEL=0 THR(W)/RHR(R); RSEL=1 Control(W)/Status(R).<br>• Control: b7 TR, b6 BREAK, b5 IE, b4:3 WLS, b2 SBS, b1 EPE, b0 PI.<br>• Status: b7 THRE, b6 TSRE, b5 PSI, b4 ES, b3 FE, b2 PE, b1 OE, b0 DA.<br>• INT̅ active low with IE; Table I sources and clears.<br>• Modem pins RTS̅, CTS̅ (gates TX), ES̅, PSI̅. CLEAR̅ active low. | `cdp1854` = `ay1013` core + mode-1 front end (one device with a MODE pin) |
| **M66230** | classic-a | 8-bit bus, CS̅, RD̅/WR̅, C/D̅.<br>• Data port: 4-byte TX and RX FIFOs. TX FIFO written only when empty, in groups; RX read all at once.<br>• Commands (D7:6): 1 format/CRC/baud-follow, 3 block length, 4 IR/ER/RTS/WUMODE/RXEN/TXEN, 6 FIFO-disable/INT polarity/P1/P0. Commands 2 and 5 are "next write" follow-ons.<br>• Two status registers read **alternately** at one address.<br>• CRC-CCITT block check; 9-bit wakeup mode.<br>• Baud = fX1 / (2 or 32) / (n+1) / 16. 8 data bits only. | unique (`m66230`) |
| **8251A** | classic-b | USART. C/D̅ selects data vs control/status.<br>• Write sequence: Mode → (SYNC1 → SYNC2) → Command…<br>• Mode: baud factor 1/16/64×, length, parity, stop bits; or sync ESD/SCS.<br>• Command: TxEN, DTR, RxE, SBRK, ER, RTS, IR, EH.<br>• Status: TxRDY, RxRDY, TxEMPTY, PE, OE, FE, SYNDET/BRKDET, DSR.<br>• Pins TxRDY (gated by CTS·TxEN), RxRDY, TxEMPTY, SYNDET. External TxC/RxC clocks; CLK >30× bit rate. | unique (`i8251`) |
| **82510** | classic-b | 4 banks × 8 addresses; the bank is set by writing address 2 (GIR/BANK).<br>• Bank 0 is 8250-like: RXD/TXD, GER (not IER), GIR (different codes), LCR (6 parity modes, SBL), MCR (OUT0), LSR (RFIR, TFST, TxST), MSR, ACR0.<br>• Banks 1–3: FIFO levels, timers, BRGA/BRGB, µLAN address match, control-character recognition, auto RTS/CTS, DPLL, modes and pin-function config.<br>• 4-deep FIFOs (RX defaults to 1-deep). INT active high; auto/manual ack.<br>• Not 16550-compatible: no FCR, IIR coding differs. | unique (`i82510`) |
| **CDP65C51 / 65C51A** | classic-b | 6551 ACIA. RS1:0 select: 00 TDR/RDR, 01 programmed-reset(W)/Status(R), 10 Command, 11 Control.<br>• Internal BRG: 15 rates, or 16× external clock.<br>• RXC pin bidirectional. IRQ̅ open-drain; Command b0 (DTR) is the master enable. | `acia6551` (`ctsCompletesChar`: A = true) |
| **82C52** | classic-b | A1:0 select: 00 TBR/RBR, 01 UCR(W)/USR(R), 10 MCR, 11 BRSR(W)/MSR(R).<br>• BRG: prescaler × fractional codes.<br>• INTR = INTEN · (USR edge events + MIEN · modem edges).<br>• DR and TBRE pins; echo/loop/break modes; split TX/RX parity. | `hs82c52` |
| **HD-6406** | classic-b | = 82C52, plus ALE latch, CS1, TXDACK/RXDACK (chip-select + address override), RI/RLSD in MSR, TC/OE/FE/PE/DR/TBRE status pins, and SIE/SFD interrupt-mask pins. | `hs82c52` (`extendedPins`) |
| **COM78C804** | classic-b | 4-line "scanner" UART. CS̅/DS̅/WR̅ bus, ADD4:3 line, ADD2:0 register.<br>• 000 RX/THR; 001 Status; 010 MR1/MR2 (alternating pointer); 011 Command; 100 Interrupt Summary; 101 Data Set Change Summary (write-1-clear).<br>• Fixed 4.9152 MHz clock; MR2 has 16 rates per direction.<br>• On-chip interrupt scanner with IRQ̅ (OD), IRQLN, IRQTXRX pins. DSR/DCD inputs only. | `com78c80x` |
| **COM78C802** | classic-b | = 78C804 with 2 lines (ADD3); IRQLN0 only. | `com78c80x` (`lines` = 2) |
| **COM78804** | classic-b | = 78C804 (NMOS, tDSU = 0). | `com78c80x` |
| **COM92C451** | classic-b | **The UART (ACE) is 16450-compatible** (Table A row 63; no FIFO/FCR, single CLK, IRQ1 not OUT2-gated, no OUT1).<br>Also on chip: an IBM-PC Centronics **PIA** on /CE0 (Data, Status, Control; IRQ0 3-state; open-drain controls). | `ns16550` (`fifo` = 0) + `pcLpt` block |
| **MAX3100** | maxim | **16-bit full-duplex SPI word per CS frame** (mode 0). DIN[15:14] selects the operation: 11 Write Config, 01 Read Config, 10 Write Data, 00 Read Data.<br>• Config: FEN SHDNi TM RM PM RAM IR ST PE L B3:0.<br>• Data word returns R, T, RA/FE, CTS, Pr, D7:0.<br>• 8-word RX FIFO; no TX FIFO. 9th bit = user data (no parity generator). Baud = fOSC / (16 × BRD).<br>• IRQ OD-L = R·RM + T·TM + Pr·PM + RA/FE·RAM. No reset pin. | `max3100` |
| **MAX3140** | maxim | = MAX3100 UART, plus an **independent RS-485/422 transceiver** (DE, RE, H/F, TXP, RXP, SRL), not connected internally. | `max3100` (`transceiver` = true) |
| **MAX3107** | maxim | **Own flat register map 0x00–0x1F** over SPI (cmd = W/R + A6..A0) or I2C (0x2C–0x2F). Burst auto-increments except at 0x00.<br>• Registers: IRQEn/ISR, LSRIntEn/LSR, SpclChr, STS, MODE1/2, LCR (**reset 0x05**), RxTimeOut, HDplxDelay, IrDA, FlowLvl, FIFOTrgLvl (×8), TxFIFOLvl/RxFIFOLvl, FlowCtrl, XON/XOFF, GPIOConfg/Data, PLLConfig, BRGConfig, DIV, CLKSource, RevID (0xA1).<br>• 128-word FIFOs, PLL, fractional /16 BRG, 16/8/4×, AutoSleep/ForcedSleep, RS-485 TrnscvCtrl.<br>• IRQ OD-L with IRQSel. | `max310x` |
| **MAX3108** | maxim | = MAX3107; I2C 16 addresses at 1 MHz; no RevID; TxEmptyInt; CLKSource reset 0x18. | `max310x` |
| **MAX3109** | maxim | 2 × MAX3108. SPI U bit; MISO returns IRQ bits; fast read. GlobalIRQ / GloblComnd at 0x1F; extended map 0x20–0x25 (TxSynch, SynchDelay, TIMER, RevID 0xC2). No IRQSel; 8 GPIO. | `max310x` |
| **MAX14830** | maxim | 4 × MAX3107 core; U1:U0 bits; no sleep modes; RTSInvert; CLKDisabl; RevID 0xB4; 16 GPIO. | `max310x` |
| **SC18IS604** | bridges | **SPI-to-I2C controller, not a UART.** SPI mode 3, ≤1.2 MHz; one CS frame = one command.<br>• Commands: 0x00 write, 0x01 read, 0x02 write-then-read, 0x03 write-write, 0x06 read buffer, 0x18 bit order, 0x20/0x21 register write/read, 0x30 0x5A 0xA5 power-down, 0xFE version string.<br>• Registers IOConfig, IOState, I2CClock, I2CTO, I2CStat (0xF0–0xF9), I2CAdr. 255-byte buffers. INT OD-L. | unique (`sc18is604`) |
| **XR16V798 / XR16L784 global block** | exar-d | Flat 256-byte map. Each channel's 16 registers sit at ch<<4 (core in Table A).<br>• Global block at 0x80–0x8E: INT0 (channel pending bits), INT1–3 (3-bit source codes per channel), TIMERCNTL (798 command codes; 784 bit-mapped), TIMER LSB/MSB, 8XMODE, REGA, RESET (soft reset per channel), SLEEP, DREV, DVID, REGB (broadcast write).<br>• One OD INT#. | `xr78x` wrapper over `ns16550` cores (`channels` 8/4, `dld`, `sendImmediate`, `timerStyle`) |
| **SC16IS740/750/760/752/762 host protocol** | bridges | **I2C** 100/400 kHz.<br>• Address: A1/A0 strapped to VDD/VSS/SCL/SDA, 16 addresses 0x48–0x57.<br>• Subaddress byte: [6:3] = A3:0, [2:1] = CH.<br>• Write `S a+W sub data… P`; read `S a+W sub Sr a+R data… NA P`.<br>• **No auto-increment**: repeated data bytes hit the same register (FIFO burst). IIR burst read prohibited. NACK on the SRESET byte.<br><br>**SPI** mode 0. First byte R/W(1 = read) \| A3:0 \| CH1:0 \| X, then data; holding CS repeats the register. 4 MHz (750/740/752) or 15 MHz (760/762). | UART core = `ns16550` (Table A); front end = `uartbridge` (`addrTable` 0x48×16, `spiMaxHz`) |
| **PI7C9X760/762 host protocol** | bridges | = SC16IS. I2C 400 kHz with **NACK on THR write when the TX FIFO is full**. SPI 33 MHz. SFR[4] "burst R/W" (probably auto-increment; undescribed). | `uartbridge` (`nackOnFull`) |
| **XR20M1170 host protocol** | bridges | = SC16IS, but **8 addresses 0x30–0x37** (A1 = SCL acts like VCC, A1 = SDA like GND). NACK when full. SPI 18 MHz. | `uartbridge` (`addrTable` 0x30×8) |
| **XR20M1280 host protocol** | bridges | Subaddress [5:3] = **A2:A0** (8-register 16550 map). SPI: bit7 R/W, bit6 0, [5:3] A2:0, [2:1] 00; 26 MHz. NACK when full. 128-byte burst. | `uartbridge` (`regBits` = 3) over the 854-style core |

---

## 3. Proposed models

| model | devices (count) | kind |
|---|---|---|
| `ns16550` (parametric core + parallel-bus wrapper) | Table A rows 1–49, 52, 53, 63 → **52** | 16550-family |
| `ns16550` core + `uartbridge` front end | SC16IS740, 750, 760, 752, 762, PI7C9X760, 762, XR20M1170, 1280 → **9** | I2C/SPI target |
| `ns16550` core + `xr78x` wrapper | XR16V798, XR16L784 → **2** | 5G flat map + global block |
| `pcLpt` (add-on block) | TL16C451, TL16C452, TL16C552, TL16C552A, COM92C451 → 5 (attached, not counted separately) | printer port |
| `ay1013` (pin-parallel) | AY-5-1013A, AY-6-1013, AY-3-1014A, AY-3-1015D, HD-6402, COM2017, COM2502, COM8017, COM8502, COM1863, COM8018 → **11**, plus CDP1854A and CDP1854AC → **13** | |
| `cdp1854` (= `ay1013` + mode-1 CDP1800 bus) | CDP1854A, CDP1854AC (counted above). Alternative: a separate mode-1 model | |
| `i8251` | 8251A → 1 | |
| `i82510` | 82510 → 1 | |
| `acia6551` | CDP65C51, CDP65C51A → 2 | |
| `hs82c52` | 82C52, HD-6406 → 2 | |
| `com78c80x` | COM78C804, COM78C802, COM78804 → 3 | |
| `m66230` | M66230 → 1 | |
| `max3100` | MAX3100, MAX3140 → 2 | |
| `max310x` | MAX3107, MAX3108, MAX3109, MAX14830 → 4 | |
| `sc18is604` (I2C controller) | SC18IS604 → 1 | |

Total: 63 16550-core devices and 31 other devices.

The 63 Table-A rows expand to more part numbers: D-variants, 1551, 740/760/762 and so on.

### `ns16550` parameters (values seen)

**Topology and bus**
- `channels`: 1, 2, 4, 8.
- `bus`: intel, intel-dp (+ADS/AS latch), intel/motorola via pin, intelIowOnly (750E MODE), pcIsaDecode (650A SEL), i2c/spi.
- `csScheme`: 3cs, single, perChannel, cs+chsel (polarity: 1 = A, or 1 = ch1), cs+A3, cs+A4:A3, flat A7..A0.
- `broadcastWrite`: none, multi-CS, AFR[0] (one shared bit or per channel?), CONC (TI AFR), REGB.
- `busSyncToClock`: 754B, SC16C752B.
- `pinSubset` per package: 550D RHB/ZQS, 2550 RHB, V2550 QFN, M2550 QFN, 650B HVQFN, 740.

**FIFO and triggers**
- `fifo`: 0, 16, 32, 64, 128, or 750-style 16/64 switched by FCR[5].
- `rxTrig` (FCR[7:6] tables): {1,4,8,14}, {8,16,24,28}, {8,16,24,30} (V564), {8,16,56,60}, {1,4,56,60} (754C, 752C), {1,4,120,124} (750E), {1,16,32,56} (750 64-byte).
- `txTrig`: none, {1,4,8,14}, {16,8,24,30}, {8,16,32,56}, {16,32,64,120}.
- `trigTables` A–D via FCTR, with TRG (854/1280) or RXTRG/TXTRG (5G). The last selection applies to both directions.
- `tlr`: none, ×4 (64-byte parts), ×8 (750E).

**DMA**
- `dmaPins`: none, single, perChannel, sharedAnd (554, V554), sharedWireOr (754B), via MF, drq (864).
- `dmaClass`: P, N, T, X, B (550B unstated).

**Register access**
- `extStyle`: none, ier5 (1450/1550), afe, fcr5_750, 650, 650+dld, 650+afrExar, 752 (±tiAfr, ±dlf), 854, nsEfr (NS16C), 5g, sc16is, sc16is+sfr (PI7C), sc16is+dld (XR20M1170), 1280.
- `afrLayout`: tiAfr752, tiAfr2552 (2-bit MF field), tl2752 (one bit per MF function), exarAfr, nsAfr (11 = forced 1).
- `idRegs`: none, or DVID/DREV via DLL=DLM=0. Values: L580 0x01; L25xx/M2550/M2551/V2550/2551/2552 0x02; 650A 0x04; M/V 26xx 0x06; TL16C2752 0x0A; C854/864 0x14. NS DREV 0x31/0x21. Global 0x8D: 0x48 (798) / 0x24 (784). Revision mostly 0x01.
- `fifoCounters`: none, TXLVL/RXLVL, FC/FLVL, TXCNT/RXCNT, RFD/TFD/RLS.

**Flow control**
- `autoFlow`: none, afe (trigger-based, TI), efrTable (fixed per-trigger table), tcr (×4/×8), tableD hysteresis (16-entry list).
- `rtsTable` values: see A2.
- `autoFlowDtrDsr`: 5G via MCR[2]; PI7C via SFR[1].
- `swFlow`: none, exarOr (1011/0111 = either), nxpAnd (sequence).
- `xonAny`: MCR[5], XFR[4], or none (650B).
- `specialChar`: EFR[5]; special requires EFR[3:0]=0 on 650B.
- `xoffSendPoint`: trigger + 2 characters, TCR halt, or table.
- `autoFlowInLoopback`: yes (L580) or no.

**Power, IrDA, RS-485**
- `sleep`: none, ier4-perChip (all channels must set it), ier4-perChannel, 750 IER[4]/[5], mcr7PowerDown (1450/1550), globalSleepReg (5G).
- Sleep extras: `wakeIndicator` (ISR 0x01); `pwrsavePin`; `sleepPin`/`pwrdn` (1280); entry conditions such as divisor ≠ 0 or FIFOs empty.
- `irda`: none, mcr6, mcr6 + MCR[2] polarity, xfr (650A, with IRPW pulse width), afr1, dld7 (M752), fast ¼-bit (EFCR[7]/SFR[3]), enirStrap.
- `rs485`: none, xfr2 on RTS + polarity (650A), afr2 on DTR + DLY 0–7 (TI), dld6 on RTS (M752), fctr on RTS/DTR + 0–15 delay (5G), fctr3 on OP1 (864), efcr4/5 (SC16IS), fctr3/EN485 + SHR + EMSR[3] (1280).

**Baud generation and clocks**
- `prescaler`: none, MCR[7] ÷4, or MCR[7] preset from the CLKSEL pin (754B, V564/654, C854/864, M2551/2651, V2651 TQFP).
- `sampling`: 16 only, DLD 16/8/4, DLF 16/8 (750E), EMSR[7] (2752), 8XMODE (5G), PI7C SCR.
- `fracDiv`: none, DLD n/16, DLF n/64, CPR M/N.
- `independentTxRxBrg`: 1280 only.
- `clockPins`: xtal (+RCLK, BAUDOUT), single CLK (451/452/552/COM92C451), CLKSEL, CHCCLK, TMRCK, BCLK outputs, BAUDOUT via MF.

**Pins and interrupt**
- `outPins`: OUT1+OUT2, OP2 only, OP1+OP2 (864), MF mux, OUT (650B), GPIO register bank, none.
- `intMode`: drv, 3S/MCR3, INTN pin override (554), INTSEL pin or strap (554D/V554/754B), mcr5 open-source (650A/650B), Motorola OD IRQ, single device OD (5G/bridges).
- `intPolarity`: high, or low/OD.
- `resetPolarity`: high, low (451/452/552/5G/bridges), or by bus mode (Intel high / Motorola low).
- `lcrReset`: 0x00 or **0x1D** (750E, 754B/C, 752C, M752, SC16C752B, SC16IS/PI7C/XR20M1170).
- `dllDlmReset`: unchanged, XX, 01/00 at power-up only, 0x0002 (82510 – n/a).
- `sprReset`: unchanged, or 0xFF.
- `tcrReset`: 0x00 or 0x0F.

**Timing and status behaviour**
- `timeout`: 4ch or 4W+12; timeout delay 8 or 9 RCLK.
- `iirPriority`: timeout above RX (Exar), RX above timeout (5G), or equal (NXP); 6- or 7-level with Xoff/CTS.
- `thrIntClearRule`: standard, or ISR-read-and-IER1=0 (1450/1550).
- `lsr7Clear`: when no errors remain, or on LSR read (650B).
- `startSample`: 7½ or 8.
- `stopCheck`: first only, or both (82C50A).
- `statusReadDefer`: 82C50A, TL16C552.
- `ier1Reenable`: re-fires or doesn't.
- `rxrdyOnLsr7` (class T).
- `majority3`: TI EFR parts.

**Bridge front end (`uartbridge`)**
- `regBits`: 4 (SC16IS) or 3 (1280).
- `addrTable`: 0x48 × 16 or 0x30 × 8.
- `spiMaxHz`, `i2cMaxHz`.
- `nackOnFull`.
- `sharedGpio` across channels.

---

## 4. Datasheet contradictions and ambiguities flagged by the spec authors

Each line reads: device: issue → recommended reading. "?" means no recommendation was given.

### ti-a
- TL16C450: p17 IIR mentions a character timeout (a 16550 leftover) → ignore.
- TL16C450: p6 th1 "address low after ADS↑" → means address valid.
- TL16C450: LCR has no reset entry → 0.
- TL16C450: break and false-start handling unstated → use the 16450 convention.
- TL16C450: divisor 0 undescribed → ?
- TL16C451/452: BDO polarity: text says active high, Fig 3 shows low during a read → low during a read.
- TL16C451/452: INT2 text says ACK↑ edge, Fig 7 says it follows ACK → INT2 = ACK level when IRQ ENB.
- TL16C451/452: LPT status/control bit polarity vs pins unstated → IBM convention (decision needed).
- TL16C451/452: SOUT0 typed "I" → output.
- TL16C451/452: ICC condition says "XTAL1" → CLK.
- TL16C451/452: typos td15 "DTS" → DTR; tw5 "IOW↑" → low pulse; th3 "before" → after.
- TL16C451/452: read data valid "before IOR↑" vs the figure → measure from IOR↓.
- TL16C451/452: selecting several CS at once unspecified → ?
- TL16C451/452: whether BDO responds to unselected reads unstated → ?
- TL16C550B: CS1 overbar → active high.
- TL16C550B: tsu3 edge → trailing edge.
- TL16C550B: MSR[5] in loop "MCR bit 1" → bit 0.
- TL16C550B: LSR[7] "always cleared" → 0 in 450 mode, error flag in FIFO mode.
- TL16C550B: Fig 8 caption "FCR=1 or FCR3=1" → FCR0=1 and FCR3=1.
- TL16C550B: XIN max 8 MHz (p29) / 9 MHz (p8) / 562 kbit/s (p1) → 9 MHz.
- TL16C550B: census says 3.3/5 V → 5 V only.
- TL16C550B: TXRDY mode-1 re-assert threshold unstated → ?
- TL16C550C: tw8 "1 ns" → 1 µs.
- TL16C550C: FCR3 "level 0 to 1" → DMA mode 0 to 1.
- TL16C550C: "LSR bit 12" → bit 1.
- TL16C550C: LSR[7] "always cleared" → as 550B.
- TL16C550C: ΔCTS with AFE on → not set, no interrupt.
- TL16C550C: addendum CPFBRG4 temperature → TI addendum error.
- TL16C550C: no 3.3 V timing → ?
- TL16C550D: RHB pin table copies PT numbers → use the drawing (DSR 25, DTR 22, INTRPT 20, MR 23).
- TL16C550D: RHB CS2 row describes CS0/CS1 → typo.
- TL16C550D: "dc to 16 MHz" vs 24 MHz at 5 V → 24 MHz.
- TL16C550D: 2.5/5 V columns quote 3.6 V → copy error.
- TL16C550D: ties for missing pins inferred; spec text says "WR2 = 1" against its own tie-inactive rule → tie inactive (WR2 = 0).
- TL16C550D: MSR values for the missing ZQS modem inputs unspecified → ?
- PC16550D: minimum divisor 1 (p1, p4) vs 2 (p15) → 1.
- PC16550D: Table I MCR[7:4] fixed → only [7:5].
- PC16550D: 18.432 MHz / 1200 baud table prints 920 → 960.
- PC16550D: tXH/tXL at 8 MHz vs 24 MHz max → 24 MHz.
- PC16550D: both strobes of a pair active undefined → ?
- PC16550D: sample point unstated → count 7½.
- PC16550D: SCR reset unstated → ?
- 82C50A: CS1 overbar → active high.
- 82C50A: MSR(3) "Pin-36" → pin 38.
- 82C50A: MSR(7) text "MSR(4)" → MSR(7).
- 82C50A: OUT2 text "MCR(3) (OUT1)" → OUT2.
- 82C50A: IER "write register" → R/W.
- 82C50A: write to 010 undefined → ?
- 82C50A: does a 0 second stop bit set FE? → ?
- 82C50A: THRE timing ambiguous → ?
- 82C50A: TAW symbol used for two parameters → ?
- 82C50A: "echo mode" never described → ?

### ti-b
- TL16C552/552A/554/554A: LCR reset: table says cleared, text says unaffected → planner decides (TI convention is cleared).
- TL16C552: Fig 18 "OUT2 output low" with no OUT2 pin → MCR3 = INT enable.
- TL16C552: RI TERI edge (pin text high→low, MSR text low→high) → low→high.
- TL16C552: Table 16 titled 8.192 MHz but values for 8.000 MHz; 50 baud "1000" → 10000.
- TL16C552: LPC b3 "SLCT" → SLIN.
- TL16C552: BDO edge text vs figure → active high.
- TL16C552 vs 552A: tpd6 edge and td6 max (8 vs 9) differ → per device.
- TL16C552A: "dc to 8 MHz" leftover → 16 MHz.
- TL16C554: footnote "CSA or CSD" → each CS selects its own channel.
- TL16C554: INTN text "modem status register bit 3" → MCR3.
- TL16C554: RXRDY/TXRDY pin text (p5) vs pp29–30 → pp29–30.
- TL16C554: LSR4 text "LSR2" → LSR4.
- TL16C554: MSR text "IIR3" → IER3.
- TL16C554: "dc to 8 MHz" → 16 MHz.
- TL16C554: PN package TQFP vs LQFP → LQFP.
- TL16C554: multiple CS low unspecified → ?
- TL16C554A: MCR5–7 "permanently cleared" vs AFE → MCR5 = AFE.
- TL16C554A: Table 7 "MSR" header → MCR.
- TL16C554A: missing FN pin numbers for RI/RTS → use the drawing.
- TL16C554A: PM package mis-described; figure references wrong → ?
- TL16C750: CS1 overbar → active high.
- TL16C750: THR write "clears the FIFO" → clears the THRE interrupt.
- TL16C750: "06 or 0110h" → binary 0110.
- TL16C750: LSR7 "in 750 mode always cleared" → 450 mode.
- TL16C750: RXRDY/TXRDY reset states unspecified → ?
- TL16C750: which clocks sleep stops unspecified → ?
- TL16C750E: multi-channel copy text ("two devices", "CS A-B") → single channel.
- TL16C750E: DLF ×128 (p30) vs ×64 → 64.
- TL16C750E: TCR range 0–60 vs 0–120 (×8) → ×8.
- TL16C750E: RS-485 DLY 0–15 vs a 3-bit field → 0–7.
- TL16C750E: RX triggers 1/4/120/124 odd → as printed.
- TL16C750E: IOW-only mode figures blank → ?
- TL16C750E: OE clear → LSR read.
- TL16C750E: unspecified: auto CTS/RTS interrupt set point; address 010 for other LCR[7]=1 values; DLF with EFR4=0; DLF vs TLR priority at 111; outputs in loopback → ?
- TL16C754B: "754A" leftovers → ignore.
- TL16C754B: RXRDY pin text vs DMA section → per-channel rules, then combine.
- TL16C754B: EFR7 reset "NOT CLKSEL" → EFR = 0x00.
- TL16C754B: LCR 0x1D → as printed.
- TL16C754B: read decode priority at 110/111 unstated → LCR=0xBF Xon/Xoff, else TCR/TLR (EFR4&MCR6), else FIFORdy (MCR2&!MCR4), else MSR/SPR.
- TL16C754B: FIFORdy decodes with the selected channel's MCR → as stated.
- TL16C754B: T20d/T23d/T28d units → 16× clocks.
- TL16C754B: p18 notes refer to nonexistent pins; programmer's guide garbled → ignore.
- TL16C754B: loopback pin states unstated → ?
- TL16C754B: census TL16C754FN (original 754) has no datasheet → assume B pinout.
- TL16C754C: AFR DLY 0–15 vs 3 bits → 0–7.
- TL16C754C: IIR modem code 001000 vs 000000 → 000000.
- TL16C754C: "48SEN" → 485EN.
- TL16C754C: IrDA text copied from 550C → ?
- TL16C754C: INTSEL/RXRDY/TXRDY listed without pins → absent; INT enable is MCR3 only (inferred).
- TL16C754C: Xon2 default prints 0xEF → ?

### ti-c
- TL16C2550: pin text cites "EFR bit 7/6" → no EFR; autoflow is MCR5.
- TL16C2550: "independent receiver clock" claim → false.
- TL16C2550: 3.3 V band misprinted → 3.0–3.6 V.
- TL16C2550: RHB pins 21/22 INTA/INTB drawing vs table → ?
- TL16C2550: RHB called TQFP in the thermal table → ?
- TL16C2550: TXRDY pin text mentions a trigger → no TX trigger.
- TL16C2552: MCR3/INT reset: reset table says MCR=0x08 (INT driven, OP low), INT pin text says high-Z → reset table (MCR=0x08).
- TL16C2552: pin text cites EFR; IOW text cites CSA/CSB → ignore.
- TL16C2752: clock limit 10/16/20/24 MHz (ROC) vs 16/24/32/48 MHz (features) → ?
- TL16C2752: same MCR3/INT reset conflict → ?
- TL16C2752: enhanced semantics missing → get the XR16L2752 datasheet or model 550 + EFR only; **do not substitute NS16C2752**.
- TL16C2752: short-stop errata SLLZ058 missing → ?
- TL16C752C: modem IIR 001000 → 000000.
- TL16C752C: CLKSEL mentioned but absent → MCR7 resets 0.
- TL16C752C: RS-485 "1 bit before first stop" vs the figure; DLY 0–15 vs 3 bits → 0–7.
- TL16C752C: D/D-Q1/CI-Q1 Table 7 labels FIFORdy C/D bits → read 0.
- TL16C752C: priority at 111 → FIFORdy (MCR2=1, loop off), else TLR, else SPR. **Differs from the ti-b 754B order; pick one.**
- TL16C752C: AFR b2 "485RN" → 485EN.
- TL16C752C: SLLZ058 missing → ?
- TL16C752C: **spec-author error**: ti-c decodes LCR 0x1D as "8 bits, odd parity" → it is 6 bits, 2 stop, parity enabled, even.
- NS16C2552: LCR4 text "1 = odd" vs table → table (1 = even).
- NS16C2552: DREV ID "0x03" vs table → 0x31.
- NS16C2552: Table 2 IIR[5:4] for the 2552 → both devices have the full set.

### exar-a
- ST16C450: loopback prose garbled → RTS→CTS, DTR→DSR, OP1→RI, OP2→CD.
- ST16C450: AS# "logic 0 transition" vs figure → transparent low, latch ↑.
- ST16C450: IER/LCR/MCR marked write-only → R/W.
- ST16C450: LSR clear-on-read and interrupt clears unstated → 16450 standard.
- ST16C450: loopback pin states → TX=1, outputs high.
- ST16C450: LCR[7] text mentions EFR → none exists.
- ST16C450: PLCC e "0.20" → 0.020.
- ST16C450: OBS watermark vs census Active → flag.
- ST16C1450: RST "only on 1451" vs Fig (pin 22) → RST present.
- ST16C1450: "EFR bit-4" header → the gate is IER[5].
- ST16C1450: ISR[7:4] "reserved" vs IER5-enabled ISR[5:4] → enabled, active high.
- ST16C1450: start sample 8 vs 7½ → per sheet.
- ST16C1450: power-down internals → clocks stopped, bus alive.
- ST16C1450: MCR2→RST in loopback unstated → ?
- ST16C1450: Table 2 grouping of 101–111 → unaffected by LCR[7].
- ST16C1450: abs-max 7 V vs VIH max VCC → no 5 V tolerance.
- ST16C1450: AC table omits figure parameters → ?
- ST16C550: loopback prose garbled → MSR mapping.
- ST16C550: LCR[7] EFR text → none.
- ST16C550: LSR/ISR clear rules → 16550 standard.
- ST16C550: timeout is 4W+12, not 4 char → as printed.
- ST16C550: PLCC pitch misprint → ?
- ST16C550: FIFO clear at reset unstated → ?
- ST16C2550: Table 5 "MCR bit-7" → no prescaler.
- ST16C2550 / XR16C2550: SPR text mentions sleep → no sleep.
- ST16C2550: crystal unit "ns" → MHz.
- XR16C2550: abs-max VCC+0.3 vs 5 V-tolerant → inputMax 5.5 V.
- ST16C1550: MCR[7] prescaler (p9, Table 2) vs power-down (map) → power-down, no prescaler.
- ST16C1550: "EFR bit-4" title → IER[5].
- ST16C1550: RST "only 1551" → present.
- ST16C1550: abs-max vs VIH → inputMax = VCC.
- ST16C1550: SPR "sleep" → power-down.
- ST16C650A: RS-485 enable "XFR bit 3" (p18) → XFR[2].
- ST16C650A: RS-485 "re-asserts RTS# logic 1" → Table 6 polarity.
- ST16C650A: default RX trigger 1 (FCR text) vs 8 (table) → 8.
- ST16C650A: reset MSR[7:4] "logic levels" → complements.
- ST16C650A: Table 8 lists MSR[7:4] as EFR4-gated → ignore.
- ST16C650A: AS# leading vs rising edge → either.
- ST16C650A: non-A ST16C650 parts undocumented → treat as 650A or drop.
- ST16C554: copy artifacts (IR encoder, IRTX, CTS stop, LCR=0xBF comment, MCR[7] column, "Channels A and B") → ignore.
- ST16C554: reset table "INT LOW" → Z on CQ64, 0 on D parts.
- ST16C554: LSR/MSR R/W vs RO → store or ignore the write.
- ST16C554: DLL/DLM not reset → as stated.
- XR16L580: Xon/Xoff R/W (Table 5) vs write-only (Table 6) → write-only.
- XR16L580: IrDA pulse "returns 1" vs Fig 12 → pulse = 0.
- XR16L580: MCR[2] polarity naming → default is standard IrDA.
- XR16L580: trigger-14 RTS-off level undefined → 16.
- XR16L580: pull-up/pull-down on TQFP only → as stated.
- XR16L580: "channel A and B" copy text → single channel.
- XR16L580: census ILTR-F "24-QFN" → 32-QFN.
- XR16L580: 000/001 at LCR=0xBF → probably DLL/DLM.

### exar-b
- XR16L2551: 000/001 at LCR=0xBF unspecified → (usual Exar) DLL/DLM?
- XR16L2551: DREV R/W vs RO → RO.
- XR16L2551: DLL rewrite after ID read → reads are a mux; writes reach DLL.
- XR16L2551: FCR[5:4] EFR-protected but reserved → reserved.
- XR16L2551: OE clear on LSR read unstated → 16550.
- XR16L2551: RTS levels at trigger 1/14 ends → 0/16.
- XR16L2551: IrDA prose vs Fig 12 → pulse = 0.
- XR16L2551: TX pulse position → planner (IrDA centres it).
- XR16L2551: IER access and read data while bus-isolated unstated → ?
- XR16L2551: MCR[3] INT gating in loopback unstated → ?
- XR16L2551: "RESET active high" without the Motorola qualifier → Motorola low.
- XR16L2551: Fig 3 titled XR16L2751 → copy error.
- XR16L2552: Fig 3 "AFR[1:0]=01" → AFR[2:1].
- XR16L2552: MF# text "MCR[3] defaults 1" → Table 13 MCR=0x00, MF#=1.
- XR16L2552: DREV/DVID R/W vs RD → RO.
- XR16L2552: 000/001 at 0xBF undefined → ?
- XR16L2552: RTS trigger 1/14 ends → 0/16.
- XR16L2552: IrDA → pulse = 0.
- XR16L2552: sleep text cites CSA#/CSB# → copy.
- XR16L2552: 3.125 Mbps / 50 MHz vs "4 Mbps" / 64 MHz → AC table 50 MHz.
- XR16L2552: ISR[5:4] → read 0 unless EFR[4]=1.
- XR16L2552: AFR[0] shared or per channel → ?
- XR16L2552: DREV read vs DLL write path → ?
- XR16L2552: OE clear → 16550.
- XR16M2550: pin-table VCC 2.25–3.6 → 1.62–3.63 V.
- XR16M2550: duplicate TX INT row → IER[1]/LSR[5] semantics.
- XR16M2550: "DTSA#" → CTSA#.
- XR16M2550: 000/001 at 0xBF undefined → ?
- XR16M2550: divisor 0 undefined → ?
- XR16M2550: INT gating in loopback → ?
- XR16M2550: IrDA pulse position → centred.
- XR16M2550: OE clear → standard.
- XR16M2550: QFN32 missing DSR/CD/RI → deasserted.
- XR16M2650: FCR default trigger 1 → table (RX 8, TX 16).
- XR16M2650: AC table corrupted from TWDO down → use M2550/2651 values.
- XR16M2650/2551/2651: OBS watermark vs DigiKey Active → planner.
- XR16M2551/2651: "whichever selection made last applies to both RX and TX" → stray; keep independent.
- XR16M2551/2651: CLKSEL vs reset MCR=0x00 → MCR[7] = NOT CLKSEL after reset.
- XR16M2551: INTB in Motorola mode unspecified → logic 0.
- XR16M752: RS-485 polarity (p9 high = RX vs pp18/37 high = TX) → HIGH = transmit.
- XR16M752: IrDA "returns 1" → pulse = 0.
- XR16M752: MCR[1] text "FCTR bit-3" → DLD[6].
- XR16M752: TLR text "TCR[3:0]" → TLR.
- XR16M752: pin table IR enable "MCR[6]" → DLD[7].
- XR16M752: INTB description cites OP2A#/INTA → OP2B#/INTB.
- XR16M752: "M2751" and 2.25–3.6 V in the Motorola figure → copy errors.
- XR16M752: p30 TX trigger direction → TLR text (≥ spaces).
- XR16M752: RX ">" vs "≥" → ≥.
- XR16M752: 0/1 at 0xBF undefined → ?
- XR16M752: FIFO Rdy polarity → complement of the pins (inferred).
- XR16M752: Motorola IRQ# combining and MCR[3] effect → ?
- XR16M752: census "34B" FIFO → 64.

### exar-c
- All six: Table 8 brackets LCR under "LCR≠0xBF" → LCR always accessible.
- All six: 000/001 at 0xBF undefined → ?
- 2550/2551/2650/2651: 010 with LCR[7]=1, ≠0xBF, EFR4=0 → presumably ISR/FCR.
- INT tables: second "RS-485" TX row → copy-over, ignore.
- 2650/2651/2652: FCR default-trigger text → tables.
- 2551/2651: "applies to both RX and TX" → independent.
- Abs-max errors (2551 3.3 V; others 7 V) → inputMax 5.5 V, XTAL1 ≤ VCC.
- Crystal max 32 MHz at 2.5 V (AC) vs 24 MHz (2551/2651 text) → ?
- 2552: Fig 3 AFR[1:0] → AFR[2:1].
- 2552: INT table "AUTO RS485" and MCR[1] "FCTR bit-3" → copy-over (no RS-485).
- 2552: IrDA pin text → MCR[2] text authoritative.
- All six: MSR/LSR clear-on-read unstated → 16550 standard.
- 2551/2651: does the wake-up indicator assert INT? → ?
- 2551/2651: Motorola INTB NC, IRQ# OR of channels unstated, write edge inferred (CS↑) → as inferred.
- 2651: CLKSEL vs reset MCR=0x00 → inconsistent when CLKSEL=GND.
- 2551: block diagram still shows CLKSEL (pin 25 is NC per rev 1.0.3) → no CLKSEL.
- 2551/2651: OBS watermark vs ordering "Active" → ignore the watermark.
- 2652: DC header 0–70 °C → industrial only.
- 2652: IJ DigiKey Obsolete but in stock → `flags: ["obsolete"]`.
- DLL/DLM power-up only; sleep needs IER[4] on both channels → as stated.

### exar-d
- XR16V554: fractional divisor and MCR[7] in text but not in map → integer, no prescaler.
- XR16V554: CTS auto-stop text → boilerplate, no auto-CTS.
- XR16V554: INT tables (p12) inconsistent → INT = OR of enabled pending.
- XR16V554: QFN48 pin 34 missing → GND.
- XR16V554: divisor 0 undefined → ?
- XR16V554: LSR/MSR with LCR[7]=1 unspecified → ?
- XR16V564/654: MCR resets 00 but CLKSEL=GND gives ÷4 → ÷4 when CLKSEL low; MCR[7] readback unspecified.
- XR16V564/654: abs-max 4 V vs 5 V-tolerant → electrical only.
- XR16V564: decode of 100–111 at LCR[7]=1 (≠0xBF) and 000/001 at 0xBF unspecified → ?
- XR16V564: force-parity text misprinted → Table 13.
- XR16V564: 68-mode IRQ reset 3-state (V564) vs HIGH (V554) → per sheet.
- XR16V654: interleaved trigger table → first block TX, second RX.
- XR16V798: TIMERCNTL Table 8 bit labels vs Table 10 command codes → command codes.
- XR16V798: "1011 reset" vs "1011–1111 reserved" → 1011 = reset.
- XR16V798/784: FCTR[3:0]=0000 hysteresis 0 vs "next trigger level" → ? (table D only).
- XR16V798/784: reset table lists IRTX pins that don't exist → ignore.
- XR16V798: RS-485 pin "RTS# or DTR#" → MCR[2] selects (assumed).
- XR16V798/784: B/C trigger columns inferred → as inferred.
- XR16V798/784: Table 16 "9 1 0 1" → 0101.
- XR16V798: LSR R/W vs RO → RO.
- XR16V798: 5 V tolerance only on serial pins → bus inputs VCC-limited.
- XR16L784: "8 UARTs" text → 4.
- XR16L784: "EMSR bit-7" → nonexistent.
- XR16L784: timer N range unstated → ?
- XR16L784: RS-485 pin unclear → ?
- XR16L784: IrDA 1.1 (p1) vs 1.0 (body) → ?
- XR16C854: crystal max 8 MHz at 3.3 V (AC) vs 24 MHz → typo, 24 MHz.
- XR16C854: DLL/DLM XX vs V-series 01/00 → per device.
- XR16C854: FCTR[3] "RS-485" only changes the TX interrupt → as stated.
- XR16C854: OP2A# in block diagram only → no pin.
- XR16C854: interleaved trigger table → RX B 8/16/24/28.
- XR16C854: TRG range unstated → ?
- XR16C854: non-F CQ/IQ may be old non-5 V-tolerant stock → flag.
- XR16C864: DMA DRQ/TC protocol under-specified → ?
- XR16C864: OP1 polarity and OP2 pin control inferred → OP = NOT MCR bit.
- XR16C864: DVID equals 854 → as printed.
- XR16C864: reset states of OP1/OP2/BCLK/DRQ → OP high, DRQ disabled.
- SC16C650B: auto-RTS text "next trigger level" vs Table 4 (8→0, 16→7, 24→15, 28→23) → author notes 650A follows Table 4. **Planner decides.**
- SC16C650B: DMA "not supported" yet pins exist → mode 0 only.
- SC16C650B: MSR[7:6] on HVQFN32 → inactive.
- SC16C650B: SPR/DLL/DLM missing from reset table → FF / XX.
- SC16C650B: ISR[5] clear method → MSR read.
- SC16C650B: TX pin in loopback → HIGH.
- SC16C752B: special-character interrupt cleared by IIR read (p9) vs LSR read (p12) → ?
- SC16C752B: title "2.2 V" → typo.
- SC16C752B: "all four ports" FIFO Rdy → 2 channels.
- SC16C752B: LCR 0x1D → as printed.
- SC16C752B: TCR/TLR need LCR[7]=0? → unstated.
- SC16C752B: MSR[7:5] on HVQFN32 → ?
- SC16C752B: exposed pad → ?
- SC16C752B: FIFO Rdy polarity → ?
- File-wide: EFR 1011/0111 is "or" (Exar) vs "and" (NXP) → vendor flag.

### bridges
- SC16IS750: break text "LSR[6]" → LCR[6].
- SC16IS750: MSR RI "IOState bit 6" → bit 7.
- SC16IS750: "THR passes above trigger" → spaces ≥ TX trigger.
- SC16IS750: addresses 0x02–0x0F with LCR[7]=1 → general registers reachable.
- SC16IS750: LSR interrupt clear → LSR read; CTS/RTS interrupt → IIR read; THR interrupt by IIR read → unstated.
- SC16IS750: SCL/SDA address-strap decode → sample at START.
- SC16IS750: clock stretching → none.
- SC16IS750: I2C td delays in the "Min" column → maxima.
- SC16IS750: XTAL note [2] empty → ?
- SC16IS740: model as a no-GPIO variant or a separate kind → planner.
- SC16IS752: only a 15 MHz SPI table → use the 750 4 MHz table for the 752.
- SC16IS752: which channel's IIR reports the GPIO interrupt → ?
- SC16IS752: Rev D errata (IrDA wake, overflow clear, IIR edge race, IIR 0xCC with LSR[0]=0) → model Rev E?
- PI7C9X760: "eight addresses" → 16-row table.
- PI7C9X760: SFR[4] burst R/W → probably auto-increment, flag.
- PI7C9X760: TIDLE units → ?
- PI7C9X760: external clock "250 MHz @2.5 V" → 50 MHz.
- PI7C9X760: IrDA "returns 1" → pulse = 0.
- PI7C9X760: ENIR# pin text → no such pin.
- PI7C9X760: IRQ pull-up "1 Ω / 1.5 Ω" → kΩ.
- PI7C9X760: GPIO5–7 called "DSR" → DTR, CD, RI.
- PI7C9X760: DLL "WO" → R/W.
- PI7C9X760/762: census C/Q2 revisions not in PDF → flag.
- XR20M1170: only 8 unique I2C addresses → as printed.
- XR20M1170: tSP 0.5 µs → 50 ns.
- XR20M1170: tVD;DAT unit → ?
- XR20M1170: GPIO text "IOControl[0]" → IOControl[1].
- XR20M1170: IrDA → pulse = 0.
- XR20M1170: IOControl[3] reset scope → ?
- XR20M1170: LCR 0x1D → as printed.
- XR20M1280: abs-max 4 V vs 5 V-tolerant → 5.5 V except XTAL1.
- XR20M1280: TDO ≤100 ns at 26 MHz impossible → ?
- XR20M1280: DREV value not printed → ?
- XR20M1280: SLEEP pin active level → HIGH (by name).
- XR20M1280: tSP 0.5 µs → 50 ns.
- XR20M1280: reset table truncated → IRQ released.
- SC18IS604: I2CTO header 0x04 → 0x03.
- SC18IS604: I2CStat reset 0x00 vs 0xF0 → 0xF0.
- SC18IS604: 0xF2 title copied → data NACK.
- SC18IS604: 0xF9 truncated → ?
- SC18IS604: repeated START vs STOP+START in 0x02/0x03 → ?
- SC18IS604: SCL typed "O" → open-drain inout.
- SC18IS604: weak pull-ups unstated; I2CClock < 5 unstated → ?

### maxim
- MAX3100/3140: test-mode TX (loopback vs held low) → unresolved.
- MAX3100/3140: R-bit clear → 16th SCLK fall (MAX3140 rule).
- MAX3100/3140: T-bit clear at SCLK 16 vs CS rise → ?
- MAX3100/3140: RA/FE "overrun or framing" → framing only?
- MAX3100/3140: config write "clears T" → T = 1 after config.
- MAX3100/3140: EP ground or unconnected → ?
- MAX3107: LCR reset 0x05 (not 8N1) → as printed.
- MAX3107: FlowLvl Resume "transmit FIFO" → receive FIFO.
- MAX3107: Halt/Resume comparisons → MAX3108 wording.
- MAX3107: W/R bit polarity not printed → bit7 = 1 write (Linux driver).
- MAX3107: I2C "stays at 0x00" rule only in 3108/3109 → apply to 3107.
- MAX3107: "internal oscillator" in title → removed in Rev 3.
- MAX3107: 2× mode single vs 3-sample → per device.
- MAX3108: MODE1[7] "—" in map vs IRQSel in text → IRQSel.
- MAX3109: RevID 0xC2 (bits) vs 0xC1 (footnote) vs "from 0xC0" → 0xC2, check the upper nibble.
- MAX14830: auto-disable at TxSynch[4] (text) vs bit 6 (register page) → bit 6.
- MAX14830: "internal oscillator" → none.
- MAX14830: census VCC 1.71–3.6 V → VA 2.35–3.6 V.

### classic-a
- AY-5-1013A: init DAV=1 (p4) vs flowchart DAV=0 → 0.
- AY-5-1013A: "AY-6-1016" → AY-6-1013.
- AY-5-1013A: TCP edge for EOC/TBMT/SO differs between drawings → TCP falling (1013A); timing approximate.
- AY-5-1013A: start sample 8½/7½ → centre at clock 8.
- AY-5-1013A: unused RD bits → 0.
- AY-5-1013A: AY-5-1013 (no A) census part → accept as 1013A?
- HD-6402: pin 26 "TRB1" → TBR1.
- HD-6402: census C grade → R (2 MHz).
- HD-6402: TBRE after MR → 1.
- COM2017: pin 2 "NC / −12 V" → −12 V supply.
- COM1863: HIACC on both (p7) vs COM8018 only (p1) → COM8018 only.
- COM1863: 1.5-stop rule unstated → ?
- CDP1854A: THRE clear at TPB leading vs PSI at trailing → as printed.
- CDP1854A: mode-1 FE/PE-OE pins 3-state? → always driven.
- M66230 (all unresolved): bit order (→ LSB first?), CRC preset/byte order, command reset values, which status register is read first, INT for TX while CTS high, FIFO-disable behaviour, output reset states, "P1=P0" wording, Command 6 D5/D4 → ?

### classic-b
- 8251A: reset values not tabulated → derived from text.
- 8251A: N8251A PLCC vs DigiKey CDIP → ?
- 8251A: MR82510/B misfiled under 8251a → it is an 82510.
- 82510: MSR7 naming → same meaning.
- 82510: RMD/TMD bit labels → as printed.
- 82510: default RX FIFO 1-deep → yes.
- 82510: Table 7 divisors single-BRG → as stated.
- CDP65C51: Command b1 p16 vs Fig 5 → Fig 5 (0 = RX interrupt enabled).
- CDP65C51: Fig 9 recurring TDRE interrupt while idle → planner.
- CDP65C51: DCD high → disables RX interrupts.
- CDP65C51: SSOP pinout assumed → DIP numbering.
- 82C52: USR clear-on-read vs DR/TBRE live → DR/TBRE mirror pins, others latched (planner).
- 82C52: BRSR 10001–11110 → undefined.
- 82C52: "CSO" → CS0.
- 82C52: fractional-divisor edge distribution → 50 % except ÷3/÷5 with external clock.
- HD-6406: 00111 ÷22 vs 1800-baud ÷21 → 22.
- HD-6406: 01101 ÷298 → 288.
- HD-6406: 110-baud error → follow 82C52.
- HD-6406: DR "in MSR" → USR.
- HD-6406: SFD masks RBRK? → yes (figure).
- HD-6406: BRSR reset → D7←0.
- HD-6406: DACK when DR/TBRE false → ?
- COM78C804: PER "RERR bit 2" → bit 4.
- COM78C804: Interrupt Summary bit 3 → 0.
- COM78C804: TxD during RESET → mark.
- COM78C804: MR2 divisors computed, not printed → ?
- COM78804: RxD typed as output → input.
- COM78804: abs-max "+0.7 V" → VCC+0.7.
- COM92C451: "independent receiver clock" → shared BRG.
- COM92C451: LSR/IIR "cleared" at reset → 60h / 01h.
- COM92C451: IRQ1 3-state, IRQ0 clear condition, Baud Rate Test Mode → ?
