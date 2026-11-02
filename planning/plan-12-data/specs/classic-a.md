# classic-a — pin-parallel / early UARTs (plan-12 extraction)

Source PDFs: `datasheets/controller-uart/` — all scanned except hd-6402.pdf (text layer). Page numbers are
PDF page indices (1-based). None of these PDFs has an ordering-code table with package suffixes (except
HD-6402 p2) or any package outline drawing.

| PDF | devices (sections below) |
|---|---|
| ay-5-1013.pdf | AY-5-1013A, AY-6-1013, AY-3-1014A, AY-3-1015D → one section `AY-5-1013A` |
| com2017.pdf | COM2017, COM2502 → `COM2017` |
| com8017.pdf | COM8017, COM8502 → `COM8017` |
| com1863.pdf | COM1863, COM8018 → `COM1863` |
| hd-6402.pdf | HD-6402 → `HD-6402` |
| cdp1854a.pdf | CDP1854A, CDP1854AC → `CDP1854A` |
| m66230.pdf | M66230P/FP → `M66230` (bus UART with FIFOs, not pin-parallel) |

All AY/COM/HD/CDP(mode 0) parts share the same 40-pin "TR1602/AY-5-1013" pinout and behaviour; the
differences are pin 2 (VGG −12 V / NC / HIACC / MODE), enable-pin polarity names, 1.5-stop-bit rule,
whether reset clears the receive holding register, max clock, and start-bit sampling details.

Census parts that cannot be fully decoded from the PDFs (suffix not explained; package assumed 40-pin DIP):
- AY-5-1013 (no "A"; not in PDF), AY-5-1013A-HR, AY-5-1013A-HRS, AY-6-1013 (plastic vs ceramic grade unknown)
- COM2017BIP, COM2017HCD, COM2017P, COM2502HR, COM8017LJP, COM8018P (SMC suffixes not in PDFs)
- HD3-6402C-9 ("C" speed grade not in the ordering table; HD3 = PDIP E40.6, -9 = −40..+85 °C)
- CDP1854ADS2082 ("S2082" suffix), CP1854ACD, CP1854AE ("CP" prefix not in PDF; read as CDP1854AC-D / CDP1854A-E)
- M66230FP#TB0Q ("#TB0Q" packing suffix; FP = 24P2W-A SOP-24 decoded)
## AY-5-1013A  (family: AY-5-1013A, AY-6-1013, AY-3-1014A, AY-3-1015D)

- Title: "UAR/T: Universal Asynchronous Receiver/Transmitter" (General Instrument). Datasheet:
  `datasheets/controller-uart/ay-5-1013.pdf` (13 pp, scanned GI databook pages 6-80..6-92; PDF p1 = 6-80).
  p1 features/pinout/block diagram; p2 pin functions; p3 transmitter flowchart; p4 receiver flowchart;
  p5 TX/RX block diagrams; p6 AY-5-1013A/AY-6-1013 specs; p7–p9 their timing; p10 AY-3-1014A/1015D
  specs; p11–p13 their timing. **No ordering information and no package drawing in this PDF.**
- Devices: the four are pin-compatible, one logical design. Differences (all p1, p2, p6, p10, p8, p12):
  | | AY-5-1013A | AY-6-1013 | AY-3-1014A | AY-3-1015D |
  |---|---|---|---|---|
  | process | P-ch nitride | P-ch nitride | N-ch (GIANT II) | N-ch (GIANT II) |
  | supplies | VCC +5V ±5%, VGG −12V ±5% (pin 2) | same | single VCC 4.75–14 V, pin 2 NC | single VCC 4.75–5.25 V, pin 2 NC |
  | temp | 0..70 °C | −40..+85 (plastic), −55..+125 (ceramic) | 0..70 | 0..70 |
  | max clock / baud | 640 kHz / 40 kbaud | 360 kHz / 22.5 kbaud | 480/400 kHz, 30/25 kbaud (at 4.75 V / 14 V) | same as 1014A |
  | input pull-ups | yes | yes | no (CMOS compatible) | yes |
  | TSB=1 with 5-bit char | 2 stop bits | 2 stop bits | 1.5 stop bits | 1.5 stop bits |
  | XR clears receive holding reg | no | no | yes ("resets every register except control register", p12 Fig.35; p1 "External reset of all registers except control bits register") | yes |
  | start-bit center sample after the 1→0 edge (Fig.11 p8 / Fig.33 p12) | 8½ or 7½ RCP clocks (depends on RCP phase at the edge) | same as 1013A | 8 or 7 clocks | same as 1014A |
  | RDAV | must be tied to 1 when unused (except AY-3-1014A, p2) | | | |
  | DAV set after RX data transfer | 3 µs after data transfer (Fig.10/12 p8) | | 1 RCP clock (Fig.32 p12) | |
- Parts (census; no ordering table in PDF, all decodes below are from the census + p1 "40 LEAD DUAL IN LINE"):
  - AY-3-1015D → dip40 (40-PDIP per census), AY-3-1015D grade: 4.75–5.25 V, 0–70 °C.
  - AY-5-1013A → dip40 (40-PDIP), 5 V ±5% + −12 V ±5%, 0–70 °C.
  - AY-5-1013A-HR, AY-5-1013A-HRS → dip40 (census 40-PDIP for -HR; -HRS package blank). "-HR"/"-HRS"
    suffixes are **not explained in this PDF** (flag: undecodable; electrically treat as AY-5-1013A).
  - AY-5-1013 (no "A") → **not in this PDF** (flag). Census package blank. Presumably the earlier
    AY-5-1013 (same pinout); treat as AY-5-1013A only if the planner accepts that.
  - AY-6-1013 → dip40; grade −40..+85 °C plastic or −55..+125 °C ceramic — package/grade not
    determinable from part number (flag); 360 kHz / 22.5 kbaud.
  - **Important: AY-5-1013A/AY-6-1013 need −12 V on pin 2 (VGG).** The simulator should treat pin 2
    as a power pin for those and NC for AY-3-1014A/1015D.
- Pinout dip40 (p1, p2):
  ```
  1 VCC power            40 TCP input   (transmitter clock 16x)
  2 VGG power (-12V; NC on AY-3-1014A/1015D)   39 EPS input
  3 GND ground           38 NB1 input
  4 RDE input  (active low)   37 NB2 input
  5 RD8 output (3-state) 36 TSB input
  6 RD7 output           35 NP input
  7 RD6 output           34 CS input  (control strobe, active HIGH)
  8 RD5 output           33 DB8 input
  9 RD4 output           32 DB7 input
  10 RD3 output          31 DB6 input
  11 RD2 output          30 DB5 input
  12 RD1 output          29 DB4 input
  13 PE output (3-state) 28 DB3 input
  14 FE output (3-state) 27 DB2 input
  15 OR output (3-state) 26 DB1 input
  16 SWE input (active low)   25 SO output
  17 RCP input (receiver clock 16x)  24 EOC output
  18 RDAV input (active low)  23 DS input (active low)
  19 DAV output (3-state)     22 TBMT output (3-state)
  20 SI input                 21 XR input (active HIGH)
  ```
  RD1 = LSB (pin 12). DB1 = LSB (pin 26). Names as printed; overbars on RDE, SWE, RDAV, DS.
- Host interface: pin-parallel, no register addressing (p2):
  - Control word: NP, TSB, NB2, NB1, EPS are latched into the "control bits holding register" by CS
    (active high). Fig.15a/15b/16 (p8): control bits must be stable for the last 300 ns of CS (1013A;
    200 ns for 1014A/1015D Fig.37a p12) and hold >0 ns (1013A) / 20 ns (1014A/15D) after CS falls →
    the register is transparent while CS=1 and latches on CS **falling** edge. CS may be hard-wired to 1
    (then control inputs act directly, and must be stable during the "illegal CS" time, Fig.8 p7).
    Fig.8: CS must not change the control word while the transmitter is sending (from DS/start bit until
    DA... "ILLEGAL TO USE CS" region spans the whole character being sent); DS and CS may occur together
    when transmitter inactive. One control word serves both TX and RX.
  - Character length NB2 NB1: 00=5, 01=6, 10=7, 11=8 bits. NP=1: no parity bit (TX and RX), PE forced 0
    (p4 text: PE unconditionally 0). EPS=0 odd parity, 1 even. TSB=0 one stop bit, 1 two stop bits
    (1.5 if 5-bit on AY-3-1014A/1015D).
  - Transmit: DB1..DB8 loaded into the transmitter holding register by DS low ("A strobe on this line
    will enter the data bits into the data bits holding register. Initial data transmission is initiated
    by the rising edge of DS. Data must be stable during entire strobe", p2). Fig.14 (p8): data setup
    ≥190 ns before DS rising edge = DS pulse width; hold >0 ns (1013A); 1014A/15D Fig.36 p12: 20 ns
    setup before falling, 200 ns pulse, 20 ns hold after rising. Unused high DB bits ignored (right-justified,
    DB1 = LSB sent first).
  - Receive: RD1..RD8 3-state, enabled when RDE=0. Status word PE, FE, OR, DAV, TBMT 3-state, enabled
    when SWE=0 (EOC and SO are NOT 3-state). RDAV=0 resets the DAV flip-flop ("The DAV F/F is the only
    thing that is reset", p2); level-sensitive (Fig.22 p9: DAV falls ≤500 ns after RDAV low, RDAV width
    ≥250 ns).
  - Unused data bits in RX holding register = right-justified, LSB on RD1 (p8 note 5). Datasheet does not
    say what the unused high RD bits read; HD-6402 (pin-compatible clone) says 0.
- Behavior:
  - Clocks: TCP and RCP are 16× baud clocks (p2). Bit time = 16 clock cycles. Which TCP edge: Fig.6/7 p7
    shows SO and EOC changing after a TCP **falling** edge (Fig.17 "EOC turn-on" referenced to TCP falling
    edge, 1 µs typ p9; Fig.19 EOC turn-off referenced to TCP rising? — Fig.19 shows TCP rising, 1 µs to EOC
    low; Fig.20 TBMT turn-on referenced to TCP rising 1 µs). For 1014A/1015D: Fig.39 EOC turn-on referenced
    to TCP rising (1.5 µs typ), Fig.41 EOC turn-off to TCP falling (2 µs typ), Fig.42 TBMT turn-on to TCP
    falling (1.5 µs). Edge polarity therefore differs between drawings; model on one edge (pick TCP falling
    for 1013A) and treat as approximate.
  - Transmitter (p3 Fig.1 flowchart, Fig.5 p7): after XR: TBMT=1, EOC=1, SO=1. DS pulse → TBMT=0
    (Fig.18: TBMT falls ≤500 ns typ after DS rising... drawn 500 ns typ from DS going high). If transmitter
    shift register empty (EOC=1): load shift register, SO=0 (start bit), EOC=0, then TBMT=1. Fig.6: when
    transmitter inactive the start bit appears within 1 clock cycle of DS (detail drawing p7; "1 to 2
    clock cycles" for 1014A/15D Fig.27 p11) and TBMT returns high 2–6 clock cycles after (Fig.6). Shift out
    1 bit per 16 TCP: start, data LSB first, parity (unless NP), stop bit(s). After the last stop bit has
    been on the line one bit time: EOC=1, SO=1. If TBMT=0 at that point (holding register loaded), the next
    character starts immediately (Fig.7: EOC pulses high for ~1 clock period between characters, start bit
    follows). Note 3 Fig.5: "Since transmitter is double buffered another data strobe can occur anywhere
    during transmission of character 1 after TBMT goes high."
  - Receiver (p4 Fig.2, p8 Fig.10–12): idle SI=1. A 1→0 on SI begins start-bit verification. 1013A
    (Fig.11): sample point is 8 RCP clocks after the edge; "If 1→0 transition of start bit occurs here
    [while RCP is high?], then sample point will be here": 8½ / 7½ clocks variants are drawn (true center
    sampling = clock 8). AY-3-1014A/1015D (Fig.33): 8 clocks or 7 clocks. If SI is 1 at the center sample,
    verification resets (false start). p4: "If the Serial Input line transitions from 1 to 0 when the 16×
    clock is in a logic 1 state, the bit time for center sampling will begin when the clock line transitions
    from a logic 1 to a logic 0 state." Then each data bit sampled every 16 clocks; parity compared (PE set
    1 on mismatch else 0 — evaluated per character), stop bit checked (FE=1 if first stop bit is 0, else 0;
    only one stop bit checked: Fig.10 FE timing at Stop 1). OR=1 if DAV still 1 at transfer, else OR=0.
    Data transferred to holding register, DAV=1. All status (PE, FE, OR) updated at same time (Fig.10 note
    1: at the stop-bit sample). DAV rises 3 µs after (1013A Fig.12: data/PE/FE/OR valid 1 µs after the
    internal sample in first stop bit; DAV ON 3 µs after that); 1014A/15D: DAV 1 clock after (Fig.32/34).
    Receiver is then ready for the next start bit (receiver looks for next start bit after center of
    stop 1, per receiver timing; "one full character time to remove the received character").
    Note 3 p8: "All information is good in holding register until data available tries to set for next
    character."
  - Reset XR (active high, min 500 ns; 1 µs AY-6-1013 p6; hold at GND when unused): resets every register
    except control register and (AY-5-1013A/AY-6-1013) received data register; SO, TBMT, EOC → 1, all
    other outputs (DAV, PE, FE, OR) → 0 (Fig.13 p8, Fig.35 p12). p2: "Clears input data buffer" (TX holding).
  - Nothing reserved/undefined beyond CS illegal window.
- Timing (p6 AY-5-1013A / AY-6-1013, TA=25 °C, 50 pF; p10 AY-3-1014A/1015D):
  | param | 5-1013A | 6-1013 | 3-1014A/1015D |
  |---|---|---|---|
  | clock freq max | 640 kHz | 360 kHz | 480 kHz @4.75 V / 400 kHz @14 V |
  | clock pulse width min | 750 ns | 1.5 µs | 1.0 µs |
  | CS pulse min | 300 ns | 600 ns | 500 ns |
  | DS pulse min | 190 ns | 250 ns | 200 ns |
  | XR pulse min | 500 ns | 1.0 µs | 500 ns |
  | SWE pulse min | 500 ns | 600 ns | 500 ns |
  | RDAV pulse min | 250 ns | 350 ns | 200 ns |
  | RDE pulse min | 500 ns | 600 ns | 500 ns |
  | data / control setup & hold | 0 / 0 ns | 0 / 0 | 20 / 20 ns |
  | TPD0, TPD1 (output prop delay) max | 500 ns | 650 ns | 500 ns |
  Output enable/disable (Fig.21 p9, Fig.43 p13): RDE/SWE low → outputs valid ≤500 ns max; float ≤500 ns
  after RDE/SWE high. RDAV→DAV low ≤500 ns (Fig.22/44). EOC/TBMT from TCP ≈1–2 µs typ (Fig.17–20, 39–42).
  TBMT off after DS ≈500 ns typ (1013A) / 1 µs typ (1014A).
- VCC: AY-5-1013A/6-1013: VCC 4.75–5.25 V and VGG −11.4..−12.6 V; inputs VIH VCC−1.5 to VCC+0.3
  (p6). AY-3-1014A 4.75–14 V; AY-3-1015D 4.75–5.25 V (p10). Inputs not tolerant above VCC+0.3.
- Package drawings: none in the PDF (p1 says "40 LEAD DUAL IN LINE" only). Use a standard 600-mil
  40-pin DIP (2.54 mm pitch, 15.24 mm row spacing).
- Obsolete: none (all census rows Active).
- Notes: p4 text says initialization "will set data available (DAV) to a logic 1" but the flowchart (p4)
  and Fig.13 (all other outputs reset to 0V) say DAV=0 — take DAV=0. p6 lists "AY-6-1016" for XR pulse —
  typo for AY-6-1013.

## HD-6402  (Harris/Intersil CMOS UART, AY-5-1013-pin-compatible except pin 2 NC)

- Title: "CMOS Universal Asynchronous Receiver Transmitter (UART)", FN2956 Rev 4.00 (Aug 2015).
  Datasheet `datasheets/controller-uart/hd-6402.pdf` (9 pp, text layer): p1 features/pinout, p2 ordering,
  p3 functional diagram + control table, p4 pin description, p5 TX/RX operation, p6 start-bit/RX timing,
  p7 DC/AC specs, p8 switching waveforms. **No package drawing pages in this PDF.**
- Parts: census `HD3-6402C-9` → **"C" grade is not in the ordering table** (p2 lists HD3-6402R-9
  (2 MHz/125 kbaud, −40..+85 °C, 40 Ld PDIP E40.6, "no longer available"), HD3-6402R-9Z, and SMD
  5962-9052502MQA (8 MHz, CERDIP F40.6)). Decode by Harris convention: HD3 = plastic DIP (E40.6),
  "-9" = −40..+85 °C; "C" speed letter undocumented here (flag). Pinout dip40, VCC 4.5–5.5 V. AC table
  gives two columns "HD-6402R" (2 MHz) and "HD-6402B" (8 MHz); which applies to "C" is unknown — use R
  (2 MHz) conservatively.
- Pinout dip40 (p1, p4):
  ```
  1 VCC power       40 TRC input  (TX clock 16x)
  2 NC nc           39 EPE input
  3 GND ground      38 CLS1 input
  4 RRD input       37 CLS2 input
  5 RBR8 output(3s) 36 SBS input
  6 RBR7 output     35 PI input
  7 RBR6 output     34 CRL input
  8 RBR5 output     33 TBR8 input
  9 RBR4 output     32 TBR7 input
  10 RBR3 output    31 TBR6 input
  11 RBR2 output    30 TBR5 input
  12 RBR1 output    29 TBR4 input
  13 PE output(3s)  28 TBR3 input
  14 FE output(3s)  27 TBR2 input
  15 OE output(3s)  26 TBR1 input   (printed "TRB1" in p4 table — typo)
  16 SFD input      25 TRO output
  17 RRC input      24 TRE output
  18 DRR input      23 TBRL input
  19 DR output(3s)  22 TBRE output(3s)
  20 RRI input      21 MR input
  ```
  Maps 1:1 to AY-5-1013 pins (RRD=RDE, SFD=SWE, DRR=RDAV, DR=DAV, RRI=SI, MR=XR, TBRE=TBMT, TBRL=DS,
  TRE=EOC, TRO=SO, CRL=CS, PI=NP, SBS=TSB, CLS2=NB2, CLS1=NB1, EPE=EPS, TRC=TCP, RRC=RCP). Same
  electrical polarities as the AY part, only named as disables: **RRD high = RBR outputs high-Z; SFD high =
  PE, FE, OE, DR, TBRE high-Z** (low = enabled, like AY RDE/SWE). DRR active low (same as RDAV). TBRL active low,
  transfer on rising edge (same as DS). CRL active high, latched on falling edge, may be tied high. MR
  active high.
- Host interface (p4, p5):
  - Control word CLS2 CLS1 PI EPE SBS latched on falling edge of CRL (transparent while high).
    Table p3: CLS2:CLS1 00=5,01=6,10=7,11=8 data bits; PI=1 no parity (EPE don't care); EPE=1 even, 0 odd;
    SBS=1 → 1.5 stop bits for 5-bit, 2 stop for 6–8; SBS=0 → 1 stop. (p4 text "CLS1 low CLS2 high 7 bits"
    matches table.)
  - Transmit: TBRL low loads TBR1–8 into transmitter buffer register; data setup tSET before / hold tHOLD
    after TBRL rising edge (p5). TBR8..6 ignored for shorter words; LSB = TBR1 sent first.
  - Receive: RBR1–8 3-state (RRD); unused MSBs read 0 (p5 "unused most significant bits will be a logic
    low"); DRR low clears DR.
- Behavior:
  - Transmitter (p5 Fig.1): rising edge of TBRL clears TBRE (TBRE→0). 0 to 1 clock cycles later, if the
    transmitter register is idle, data transfers to the transmitter register, TRE→0, TBRE→1, and the start
    bit begins. Bits clocked by TRC (16×). Second TBRL while busy: data held in buffer (TBRE=0) and
    transferred automatically at end of current character; transmission of it "begins one clock cycle
    later" — characters sent end to end. TRE→1 at end of last stop bit (Fig.1 shows ½ clock after) when no
    further data. Frame: start, 5–8 data LSB first, optional parity, 1/1.5/2 stop.
  - Receiver (p5–p6 Fig.2, Fig.4): RRI idle high; RRC 16×. Start bit center defined as clock count 7½
    after the detected falling edge (edge may have occurred up to 1 clock earlier; Fig.4 shows 7½ / 8½
    clock cycles); receiver margin 46.875%. Data shifted; "During the first stop bit data is transferred
    from the Receiver Register to the RBR" (at 7½ clocks into stop bit per Fig.2 "beginning of first stop
    bit … 7 1/2 clock cycles": RBR1-8, OE, PE update); one clock cycle later DR → 1 and FE is evaluated.
    OE=1 if DR had not been cleared before the transfer. PE=1 on parity mismatch; PI=1 forces PE=0. FE=1
    if first stop bit invalid. Receiver starts searching for the next start bit at the center of the first
    stop bit.
  - MR (active high, ≥ tMR): clears PE, FE, OE, DR to 0; sets TRE=1 "18 clock cycles after MR falling
    edge"; does NOT clear the RBR. Must be pulsed after power-up; wait 18 clock cycles after MR falls
    before operating (p4). (TBRE after MR not stated; AY-5-1013 equivalent: TBMT=1, TRO=1.)
- Timing (p7 AC table, VCC 5 V ±10%, CL 50 pF; columns HD-6402R / HD-6402B):
  fCLOCK max 2.0 / 8.0 MHz; tPW (CRL, DRR, TBRL) min 150 / 75 ns; tMR min 150 / 150 ns; tSET min
  50 / 20 ns; tHOLD min 60 / 20 ns; tEN (output enable, RRD/SFD → data/status) max 160 / 35 ns.
  Switching waveforms p8 Fig.6–8.
- VCC: 4.5–5.5 V (p7). Inputs: abs max VCC+0.5 V (not 5 V tolerant above VCC). DC: VIH 2.0 V,
  VIL 0.8 V.
- Package drawings: ordering table names E40.6 (40 Ld PDIP) and F40.6 (CERDIP), but the drawings are
  not in the PDF. Standard 600-mil DIP-40, 2.54 mm pitch.
- Obsolete: none in census (HD3-6402C-9 Active).

## COM2017  (SMC; family COM2017 / COM2502 — and COM8017 / COM8502, see next section)

- Title: "Universal Asynchronous Receiver/Transmitter UART" — SMC COM2502/COM2017. Datasheet
  `datasheets/controller-uart/com2017.pdf` (8 pp scanned, databook pp 91–98): p1 features/pinout/block,
  p2 TX operation, p3 RX operation + pin table start, p4–p5 pin functions, p5 TX/RX timing, p6 ratings,
  DC/AC table, p7 data/control/output timing, p8 flowcharts. **No ordering info, no package drawing.**
- Process: P-channel low-voltage oxide-nitride (p1). Supplies VCC +5 V ±5% and **VDD −12 V ±5%** (p6
  header; pin 2 labelled VDD on pinout p1; pin table p3 prints "2 NC No Connection −12 volt Supply" —
  scan contradiction; the electrical table lists IDD 28 mA, so pin 2 is the −12 V supply). Note 4 p6
  mentions COM2502 only for steady-state input current.
- COM2017 vs COM2502: identical except "Selection of 2 stop bits when programming a 5 data bit word
  generates 1.5 stop bits from the COM2017" (p5); COM2502 then gives 2 stop bits. (p1 description:
  "1.5 stop bits when utilizing a 5-bit code from the COM2017".)
- Parts (census): COM2017BIP, COM2017HCD, COM2017P, COM2502HR → all dip40 (only package: "40-Pin D.I.P.",
  p1; "Ceramic or Plastic Dip Package" feature). Suffixes P / BIP / HCD / HR are **not decoded in this
  PDF** (flag all four; likely P = plastic, C/HCD = ceramic). Grade: 0..70 °C, 400 kHz max clock (25 kbaud).
- Pinout dip40 (p1, p3–p5); same pin positions as AY-5-1013:
  ```
  1 VCC power(+5)    40 TCP input
  2 VDD power(-12V)  39 POE input
  3 GND ground       38 NDB1 input
  4 RDE input        37 NDB2 input
  5 RD8 output(3s)   36 NSB input
  6 RD7 output       35 NPB input
  7 RD6 output       34 CS input
  8 RD5 output       33 TD8 input
  9 RD4 output       32 TD7 input
  10 RD3 output      31 TD6 input
  11 RD2 output      30 TD5 input
  12 RD1 output      29 TD4 input
  13 RPE output(3s)  28 TD3 input
  14 RFE output(3s)  27 TD2 input
  15 ROR output(3s)  26 TD1 input
  16 SWE input       25 TSO output
  17 RCP input       24 TEOC output
  18 RDAR input      23 TDS input
  19 RDA output(3s)  22 TBMT output(3s)
  20 RSI input       21 MR input
  ```
- Host interface (p3–p5, p7): pin-parallel, same as AY-5-1013:
  RDE low → RD8..RD1 driven (3-state otherwise). SWE low → RPE, RFE, ROR, RDA, TBMT driven. TEOC and TSO
  always driven. RDAR low resets RDA (level). TDS low strobes TD1..TD8 into transmitter buffer register.
  CS high enters NDB1, NDB2, NSB, POE, NPB into the control holding register; may be hard-wired high.
  Data/control inputs need only be valid during the last tPW(min) of the strobe (p7 note) → latch on the
  trailing edge (TDS rising, CS falling); setup ≥0, hold ≥0.
  NDB2 NDB1: LL=5, LH=6, HL=7, HH=8. NPB=H no parity (RX expects stop right after data; RPE forced low);
  NPB=L: POE=L odd, POE=H even. NSB=L 1 stop, H 2 stop (1.5 for 5-bit on COM2017).
  Unused RD outputs are low (p3); unused TD inputs don't care; LSB on TD1/RD1, sent first.
- Behavior:
  - Clocks: TCP, RCP 16× baud.
  - TX (p2, p5, p8): after MR: TBMT=1, TEOC=1, TSO=1. TDS pulse → TBMT=0. If shift register empty
    (TEOC=1): load, TSO=0 start bit, TEOC=0, TBMT=1. Note 1 p6: if transmitter inactive, start bit appears
    on TSO within one TCP clock period after the trailing edge of TDS; p5 "Transmitter start-up": start
    bit placed at the high-to-low transition of the TCP clock following the trailing edge of TDS. TEOC
    goes high when the last stop bit has been on the line one bit time; it stays high until start of next
    character, or for one-half of a TCP period in continuous transmission (p4). If TBMT low at that
    point, next character starts immediately.
  - RX (p2–p3, p5, p8): MR → RDA low. Mark→space on RSI starts verification; if RSI stays space for ½ bit
    time (8 RCP; "8–16×CLK" in flowchart) start bit verified, else verification restarts. Note 2 p6: start
    edge detected within one RCP period (max slippage 1/16 bit). Bits center-sampled each 16 RCP. Parity:
    RPE set high on mismatch (held low in no-parity). RFE set high if stop bit not received (first stop).
    At end of character: if RDA still high, ROR set high (else ROR cleared); data → buffer; RDA high. Receiver
    timing p5: RDA rises ~1/16 bit time after the stop-1 center sample (RDA* drawing).
  - MR (active high, pulse after power-on, ≥500 ns): sets TSO, TEOC, TBMT high; resets RDA, RPE, RFE, ROR
    low (p4). Receiver buffer not stated as cleared.
- Timing (p6, TA 25 °C): clock DC–400 kHz (COM2502, COM2017); clock pulse width min 1 µs; MR 500 ns;
  CS 200 ns; TDS 200 ns; RDAR 200 ns; setup/hold data and control ≥0; strobe-to-output delay (RDE → data,
  SWE → status, tPD1/tPD0) max 350 ns; output disable max 350 ns (20 pF + 1 TTL). p7: RDAR→RDA low 300 ns;
  TDS rising → TBMT low 400 ns.
- VCC: +5 V ±5%, VDD −12 V ±5%, 0–70 °C (p6). VIH min VCC−1.5, max VCC; VIL min VDD, max 0.8 V.
- Package drawings: none in PDF; 40-pin DIP 600 mil assumed.
- Obsolete: none (census all Active).

## COM8017  (SMC; COM8017 / COM8502)

- Title: "Universal Asynchronous Receiver/Transmitter UART" — SMC COM8017/COM8502. Datasheet
  `datasheets/controller-uart/com8017.pdf` (8 pp scanned, databook pp 153–160). Same page layout as
  com2017.pdf. **No ordering info, no package drawing.**
- Single +5 V version (COPLAMOS N-channel depletion load) of COM2017/COM2502; "Compatible with COM2017,
  COM2502" (p1). **Pin 2 = NC** (p3 pin table, p1). Pinout identical to COM2017 otherwise (p1, p3–p5).
- COM8017 vs COM8502: "Selection of 2 stop bits when programming a 5 data bit word generates 1.5 stop bits
  from the COM8017 or COM8017/H" (p5); COM8502 gives 2.
- Parts: COM8017LJP → **"LJP" suffix not decodable** from PDF (flag; only package is 40-pin DIP, p1);
  COM8502 → dip40. Grade 0–70 °C, VCC 5 V ±5%.
- Host interface / behavior: identical text to COM2017 (p2–p5, p7–p8 checked page by page: transmitter,
  receiver, pin functions, timing diagrams, flowcharts are the same). MR sets TSO, TEOC, TBMT high, resets
  RDA, RPE, RFE, ROR low (p4).
- Timing (p6, TA 25 °C): clock DC–640 kHz (40 kbaud); clock pulse width min 0.7 µs; MR 500 ns; CS, TDS,
  RDAR 200 ns; setup/hold ≥0; RDE/SWE to output 350 ns max; output disable 350 ns. p7: RDAR→RDA low 300 ns,
  TDS→TBMT low 400 ns. Note 1: start bit within one TCP period after trailing edge of TDS when idle.
- DC (p6): VIL 0..0.8 V, VIH 2.0..VCC; IIL 300 µA max at VIN=GND (internal pull-ups implied); abs max
  +8.0 V / −0.3 V on any pin.
- Package: none in PDF; dip40.
- Obsolete: none.

## COM1863  (SMC; COM1863 / COM8018, "µPC family")

- Title: "Universal Asynchronous Receiver/Transmitter UART" — SMC COM 1863 / COM 8018. Datasheet
  `datasheets/controller-uart/com1863.pdf` (8 pp scanned, databook pp 83–90). p1 features/pinout, p2 TX
  operation, p3 RX operation + pin table, p4–p5 pins, p5 timing, p6 electrical, p7 timing + HIACC notes,
  p8 flowcharts. **No ordering info, no package drawing.**
- Single +5 V COPLAMOS; "compatible with TR1863 timing". Same pinout as COM8017 except **pin 1 is named VDD
  (+5 V)** and **pin 2 = HIACC (COM8018 only; NC on COM1863** — p1 footnote).
- COM1863 vs COM8018 (p1): COM8018 has low-current input pull-ups and the HIACC (32× clock, improved
  RDA/ROR) mode; COM1863 has no pull-ups and pin 2 not connected. (p7 note heading says "NOTES ON COM 8018
  AND COM 1863 HIGH-ACCURACY…" — conflicts with p1 footnote and feature "(COM 8018 only)"; take COM8018
  only.)
- Parts: COM1863 → dip40; COM8018, COM8018P → dip40 ("P" suffix undecoded — flag; probably plastic).
  0–70 °C, VDD 5 V ±5%, up to 1.0 MHz clock (62.5 kbaud).
- Pinout dip40 (p1, p3–p5): as COM2017 table above, but pin 1 `VDD power`, pin 2 `HIACC input`
  (COM8018) / `NC nc` (COM1863).
- HIACC (p1 footnote, p7): logic 1 on pin 2 → high-accuracy mode. Pin 2 to −12 V, GND, logic 0, or
  unconnected → disabled, 16× mode (internal pull-down resistor). Max current HIACC sources if tied to
  −13.2 V is 3.5 mA. In HIACC mode TX and RX run on **32× clocks** (TCP, RCP = 32× baud) and RDA is
  "notched" (forced low) during the one-half receiver clock cycle preceding the stop-bit center sample,
  when RD1–RD8 and ROR are changing. Receiver distortion immunity 48.4375% (HIACC) vs 46.875%.
- Host interface: identical to COM2017/COM8017 (pins p3–p5). Differences:
  - RDAR (p4): "RDAR must have gone low and come high again before ROR is sampled to avoid overrun
    indication."
  - TEOC (p4): high "during the last half clock cycle of the last stop bit", stays until start of the next
    character, or one-half TCP period in continuous transmission.
  - MR (p4): sets TSO, TEOC, TBMT high; resets RDA, RPE, RFE, ROR **and RD1–RD8** low.
- Behavior (p2, p3, p5, p8):
  - Start bit: RSI must remain spacing 15/32 to 17/32 bit time (16× mode) or 31/64 to 33/64 (32× mode)
    to verify. Start edge detected within one RCP period (note 2 p6: slippage ±1/32 bit (16×) or ±1/64
    (32×)). p5 "Start bit detect and verify": center sample at RCP count 8 (16×) / 16 (32×) after edge;
    minimum continuous low required. Return to mark before ½ bit → restart.
  - Receiver timing detail (p5) & flowchart (p8), exact sequence per character after the last data/parity
    bit: on the **negative RCP edge ½ RCP before the stop-bit center sample**: if RDA=0 and RDAR=1 → reset
    ROR, else set ROR; if HIACC=1 also reset RDA (notch); transfer data to RD1–RD8 (RD and ROR change here).
    At the stop-bit center sample: set/reset RFE and RPE. Then wait until RDAR=1 and set RDA (RDA rises at
    the stop-bit center sample in HIACC=0, i.e. ½ clock later than data). p7 "improved RDA/ROR" diagrams:
    if RDAR is held low past the stop-bit center sample, RDA goes high after RDAR returns high.
  - TX: TDS → TBMT low; if idle, start bit within 1½ TCP periods after trailing edge of TDS (note 1 p6);
    p5 "Transmitter start-up" shows TEOC falling and TSO going low on a TCP edge after TDS.
  - Parity: RPE high on mismatch, held low in no-parity mode. RFE high if stop bit not received.
- Timing (p6, TA 25 °C): clock DC–1.0 MHz; clock pulse width min 0.45 µs; MR 500 ns; CS, TDS, RDAR 200 ns;
  setup/hold 0; RDE/SWE → output 250 ns max; output disable 250 ns. p7: RDAR→RDA low 300 ns; TDS→TBMT low
  400 ns; TCP falling → TEOC low ≈500 ns; TCP rising → TSO low 500 ns; TCP falling → TBMT high 500 ns.
- DC (p6): VIL ≤0.8, VIH ≥2.0; IIL 300 µA (COM8018 only, pull-up); input leakage ±10 µA (COM1863);
  abs max +8.0 V / −0.3 V (pin 2: −13.2 V).
- Package: none in PDF; dip40.
- Obsolete: none.

## CDP1854A  (RCA/Harris CDP1854A, CDP1854AC)

- Title: "Programmable Universal Asynchronous Receiver/Transmitter (UART)" — CDP1854A, CDP1854AC, RCA
  File Number 1193. Datasheet `datasheets/controller-uart/cdp1854a.pdf` (17 pp scanned, databook
  pp 387–403). p1 features, mode-1/mode-0 DIP pinouts, PLCC-44 pinout, description; p2 abs max + mode-1
  block diagram; p3 static characteristics + recommended operating conditions; p4 mode-1 pin functions;
  p5 Table I (interrupts), Table II (status register); p6–p7 mode-1 operation, Table III (register
  select), Table IV (control register); p8 mode-1 TX timing; p9 mode-1 RX timing; p10 mode-1 CPU
  write/read timing; p11 waveforms + mode-0 block diagram; p12 mode-0 pin functions; p13 mode-0
  operation; p14 mode-0 interface timing; p15 mode-0 TX timing; p16–p17 mode-0 RX timing, serial format.
  **No ordering-code table and no package outline drawings in this PDF.**
- CDP1854A vs CDP1854AC: "functionally identical"; CDP1854A recommended VDD 4–10.5 V, CDP1854AC 4–6.5 V
  (p1, p3). Same design → one device.
- Packages (p1): 40-lead hermetic ceramic DIP (D suffix), 40-lead plastic DIP (E suffix), 44-lead plastic
  chip carrier (Q suffix); CDP1854AC also chip form (H). Temperature: D −55..+125 °C; E and Q −40..+85 °C
  (p2).
- Parts (census, all Active, maker "Harris Corporation"):
  - CDP1854ADS2082 → CDP1854A, **D = 40-lead ceramic DIP** → dip40, VDD 4–10.5 V, −55..+125 °C.
    "S2082" suffix not explained (flag; probably a customer/screening code).
  - CP1854ACD → read as CDP1854AC + D (ceramic DIP) → dip40, VDD 4–6.5 V. The "CP" prefix is **not in
    the datasheet** (flag; best reading only).
  - CP1854AE → read as CDP1854A + E (plastic DIP) → dip40, VDD 4–10.5 V, −40..+85 °C. "CP" prefix flag;
    could also be "CP1854A" = CDP1854A. No census part uses the PLCC-44.
- MODE pin (pin 2 DIP / pin 3 PLCC): high (VDD) = Mode 1, CDP1800-series microprocessor bus; low (VSS)
  = Mode 0, industry-standard TR1602A/AY-5-1013-compatible pin-parallel UART (pin-compatible except pin 2
  is MODE instead of VGG −12 V; p1 note: pin 2 NC on CDP6402).
- Pinout dip40 (p1 mode-0 and mode-1 drawings; mode-0 name / mode-1 name, role):
  ```
  1  VDD power
  2  MODE input  (VSS = mode 0, VDD = mode 1)
  3  VSS ground
  4  RRD input (active high = R BUS high-Z)        / CS2 input (active low)
  5  RBUS7 3-state output (both modes)
  6  RBUS6   7 RBUS5   8 RBUS4   9 RBUS3   10 RBUS2   11 RBUS1   12 RBUS0   (3-state outputs, both modes)
  13 PE output (3s via SFD)                         / INT output (active low)
  14 FE output (3s via SFD)                         / FE output (always driven? see note)
  15 OE output (3s via SFD)                         / PE_OE output ("PE/OE": PE OR OE)
  16 SFD input (active high = status 3-state)       / RSEL input
  17 RCLOCK input (16x)                             / RCLOCK input
  18 DAR input (active low)                         / TPB input (active high pulse)
  19 DA output (active high, 3s via SFD)            / DA output (active LOW)
  20 SDI input                                      / SDI input
  21 MR input (active high)                         / CLEAR input (active low)
  22 THRE output (active high, 3s via SFD)          / THRE output (active LOW)
  23 THRL input (active low)                        / CS1 input (active high)
  24 TSRE output (active high)                      / RTS output (active low)
  25 SDO output                                     / SDO output
  26 TBUS0 input  27 TBUS1  28 TBUS2  29 TBUS3  30 TBUS4  31 TBUS5  32 TBUS6  33 TBUS7  (inputs, both modes)
  34 CRL input (active high)                        / RD_WR input (high = read, low = write)
  35 PI input                                       / CS3 input (active high)
  36 SBS input                                      / NC
  37 WLS2 input                                     / PSI input (active low, falling edge)
  38 WLS1 input                                     / ES input (active low)
  39 EPE input                                      / CTS input (active low)
  40 TCLOCK input (16x)                             / TCLOCK input
  ```
  Notes: R BUS and T BUS are separate unidirectional buses in both modes; in mode 1 they are tied
  together externally to make the CPU's bidirectional bus (p4 "may be externally connected to
  corresponding transmitter bus terminals"; p2 block diagram "user interconnect"). Mode-1 FE (pin 14)
  and PE/OE (pin 15) — the datasheet does not say they are 3-state in mode 1 (SFD pin doesn't exist in
  mode 1); treat as always-driven totem-pole. Mode-1 DA/THRE pins are inverted copies of status bits
  (Table II footnote "polarity reversed at output terminal"). Pins 24/25 (RTS/TSRE, SDO) have higher sink
  current (p3).
- Pinout plcc44 (Q suffix, p1 "44-Lead Plastic Chip-Carrier Package"; mode-0 name (mode-1 name)):
  1 NC, 2 VDD, 3 MODE, 4 VSS, 5 RRD (CS2), 6 RBUS7, 7 RBUS6, 8 RBUS5, 9 RBUS4, 10 RBUS3, 11 RBUS2, 12 NC,
  13 RBUS1, 14 RBUS0, 15 PE (INT), 16 FE, 17 OE (PE_OE), 18 SFD (RSEL), 19 RCLOCK, 20 DAR (TPB), 21 DA (DA),
  22 SDI, 23 NC, 24 MR (CLEAR), 25 THRE (THRE), 26 THRL (CS1), 27 TSRE (RTS), 28 SDO, 29 TBUS0, 30 TBUS1,
  31 TBUS2, 32 TBUS3, 33 TBUS4, 34 NC, 35 TBUS5, 36 TBUS6, 37 TBUS7, 38 CRL (RD_WR), 39 PI (CS3),
  40 SBS (NC), 41 WLS2 (PSI), 42 WLS1 (ES), 43 EPE (CTS), 44 TCLOCK. (Not used by any census part.)

### Mode 0 (MODE = VSS) — pin-parallel, AY-5-1013/TR1602A style (p12–p17)
- Control: CRL high loads PI, EPE, SBS, WLS1, WLS2 into the control register; may be hardwired high.
  Timing p14 Fig.9: control word setup tCWC before CRL **falling** edge and hold tCCW after it → latch on
  CRL falling edge (transparent while high).
  WLS2 WLS1: LL 5, LH 6, HL 7, HH 8 bits (p13). PI high: no parity generated/checked, PE output clamped low,
  stop bits follow last data bit. EPE high even, low odd. SBS high: 2 stop bits (1.5 when 5 data bits);
  low: 1 stop.
- Transmit (p13, Fig.10 p15): THRL low pulse loads T BUS into Transmitter Holding Register; "data is
  latched on the trailing edge of this signal" (rising). THRE goes low. If the Transmitter Shift Register is
  empty (TSRE high) and the clock is low, on the next high-to-low transition of TCLOCK the character is
  loaded into the shift register (Fig.10 note: the first TCLOCK high-to-low transition occurring at least
  ½ clock period + tTHC after the THRL trailing edge); transmission of the start bit begins ½ clock period
  later (i.e. on the following TCLOCK rising edge, + tCD). THRE goes high ½ clock period later on the
  high-to-low transition of the clock (i.e. one TCLOCK falling edge after the load... Fig.10: THRE rises
  tCT after the TCLOCK edge that follows load). TSRE goes low at load (tTTS after clock). When all sent,
  THRE and TSRE both high. Bit time = 16 TCLOCK periods. Unused MSBs of short words not sent.
- Receive (p13, Fig.11 p17): after a high-to-low on SDI, a divide-by-16 counter is enabled and the start bit
  verified by checking SDI low 7½ RCLOCK periods later. Data, parity, stop shifted in at clock count 7½ of
  each bit. On count 7½ of the first stop bit the data is loaded into the Receiver Holding Register
  (unused MSBs = 0). If DA had not been reset by then, OE is raised. One-half clock period later PE and FE
  become valid, and DA is raised (Fig.11: R BUS valid tCDV after the clock-7½ edge, DA tCDA, OE tCOE,
  PE tCPE, FE tCFE after the falling clock edge "8"). DAR low pulse resets DA (tDDA). Start bit occurring
  less than tDC before a high-to-low clock transition may not be recognized until the next one.
  FE: stop bit (bit after parity, or after data) not high. Only the first stop bit checked.
- Outputs: RRD high → R BUS 3-state (low = driven). SFD high → PE, FE, OE, DA, THRE 3-state (TSRE, SDO not
  3-state). Status outputs are active-high in mode 0.
- MR (active high, p12): resets Receiver Holding Register, Control Register, and Status Register; sets SDO
  high. (So after MR: DA, OE, PE, FE = 0; THRE = 1 and TSRE = 1 implied by p13 "at the beginning ... THRE is
  HIGH". Control register cleared → 5 bits, odd parity, 1 stop unless CRL tied high.)

### Mode 1 (MODE = VDD) — CDP1800 bus (p4–p11)
- Selection: chip selected when CS1·CS2bar·CS3 = 1 (CS1 high, CS2 low, CS3 high). Strobes: TPB (positive
  pulse) is the data load/reset strobe. WRITE = overlap of TPB, CS1, CS3 = 1 and CS2 = 0, RD/WR = 0
  (Fig.3, Fig.5). READ = overlap of CS1, CS3, RD/WR = 1 and CS2 = 0 (Fig.4, Fig.6) — R BUS drivers enabled
  for the whole read selection (not only during TPB); TPB during a read performs the side effects (reset
  DA, clear interrupts).
- Table III (p6), register select:
  | RSEL | RD/WR | function |
  |---|---|---|
  | 0 | 0 | load Transmitter Holding Register from T BUS |
  | 0 | 1 | read Receiver Holding Register onto R BUS |
  | 1 | 0 | load Control Register from T BUS |
  | 1 | 1 | read Status Register onto R BUS |
  Data latched on the trailing (falling) edge of TPB (Fig.3 note "The holding register is loaded on the
  trailing edge of TPB"; write data setup tDW, hold tWD vs TPB end, Fig.5).
- Control register (Table IV p7), write-only:
  | bit | name | meaning |
  |---|---|---|
  | 7 | TR | Transmit Request: when set, RTS is set low and data transfer through the transmitter is initiated by the initial THRE interrupt. "When loading the Control Register from the bus, this (TR) bit inhibits changing of other control flip-flops." |
  | 6 | BREAK | holds SDO low while set; once set SDO stays low until BREAK is reset low AND one of: CLEAR goes low; CTS goes high; or a word is transmitted (that word is not valid — no start bit if SDO already low). SDO can be set high without intermediate transitions by transmitting a word of all zeros. |
  | 5 | IE | interrupt enable for THRE, DA, THRE·TSRE, CTS and PSI interrupts |
  | 4 | WLS2 | word length (with WLS1) |
  | 3 | WLS1 | |
  | 2 | SBS | stop bits |
  | 1 | EPE | 1 even, 0 odd parity |
  | 0 | PI | 1 = parity inhibited (PE status held low; stop follows data; EPE ignored) |
  WLS2 WLS1 SBS: 000 5 bits 1 stop; 001 5 bits 1.5 stop; 010 6/1; 011 6/2; 100 7/1; 101 7/2; 110 8/1;
  111 8/2. Two loads are required to start transmitting: one to format the UART, the second to set TR
  (p6: "Loading the Control Register with TR=1 inhibits changing the other control bits"). TR is reset by
  reloading the original control byte with TR = 0 (p6), which ends RTS. TR is not needed for TX operation
  but enables THRE interrupts and RTS.
- Status register (Table II p5), read at RSEL=1, RD/WR=1:
  | bit | name | also at pin | meaning |
  |---|---|---|---|
  | 7 | THRE | 22 (inverted) | holding register transferred to shift register, may be reloaded; setting it sets THRE pin low and interrupts (INT low) if TR high |
  | 6 | TSRE | — | shift register completed a full character incl. stop bits; stays set until next character starts |
  | 5 | PSI | — | set by high-to-low transition on PSI pin (also asserts INT) |
  | 4 | ES | — | set high by a low level on ES pin (level, not latched — "external status") |
  | 3 | FE | 14 | received char has no valid stop bit; updated each char transfer |
  | 2 | PE | 15 (ORed with OE) | parity mismatch; updated each char |
  | 1 | OE | 15 (ORed with PE) | DA not reset before next char transferred |
  | 0 | DA | 19 (inverted) | char received and transferred to holding register |
- Interrupts (Table I p5): INT pin low = interrupt; only when IE = 1. Sources and clears:
  | cause (INT low) | reset condition | reset time |
  |---|---|---|
  | DA (receipt of data) | read of data (receiver holding reg) | TPB leading edge |
  | THRE (ability to reload) — only after TR set | read of status or write of character | TPB leading edge |
  | THRE·TSRE (transmitter done) | read of status or write of character | TPB leading edge |
  | PSI negative edge | read of status | TPB trailing edge |
  | CTS positive edge when THRE·TSRE | read of status | TPB leading edge |
- Transmit (p6, Fig.3 p8): CPU writes THR (RSEL=0, RD/WR=0) with TPB. When CTS is low, the Transmitter Shift
  Register is loaded from the holding register and transmission begins; if CTS is always low, the shift
  register is loaded on the first high-to-low edge of TCLOCK at least ½ clock period + tTC after the trailing
  edge of TPB, and the start bit begins ½ clock period later (+ tCD). One TCLOCK period after the shift
  register is loaded, THRE goes low (THRE pin) and an interrupt occurs. Next character starts immediately
  after the last stop bit of the previous one. After the last character a final THRE·TSRE interrupt.
  CTS high inhibits transfer of a character to the shift register and shifting of serial data out (p4).
  RTS: set low when data is latched in the THR or TR is set high; reset high when both THR and TSR are
  empty and TR is low (p4).
- Receive (p6–p7, Fig.4 p9): same as mode 0 (start verify at 7½ RCLOCK, sample at 7½ each bit, load holding
  register at count 7½ of stop bit 1, OE if DA not reset, PE/FE valid ½ clock later, DA set and DA pin + INT
  go low). CPU reads data (RSEL=0, RD/WR=1); "When reading data, TPB latches data in the microprocessor and
  resets DATA AVAILABLE (DA) in the UART" (p7). Fig.4 note: OE comes true if a pending DA has not been
  cleared by a read of the receiver holding register by the time a new word is loaded.
- Peripheral interface (p7): RTS, CTS (as above), ES (status level), PSI (edge → PSI bit + interrupt; e.g.
  modem DCD).
- CLEAR (active low, p4): resets the interrupt flip-flop, Receiver Holding Register, Control Register and
  Status Register, sets SDO high.
- Timing (5 V column, typ / max-of-min; CDP1854AC same at 5 V; 10 V values exist for CDP1854A only):
  Mode 1 TX (p8): tCC min clock period 250/310 ns (10 V: 125/155); tCL, tCH 100/125; TPB width tTT 100/150;
  TPB-to-clock setup tTC 175/225; clock→start bit tCD 300/450; TPB→THRE tTTH 200/300; clock→THRE tCTH
  200/300.
  Mode 1 RX (p9): tCC 250/310; tCL/tCH 100/125; tTT 100/150; data-start-bit-to-clock setup tDC 100/150;
  TPB→DA tTDA 220/325; clock→DA tCDA 220/325; clock→OE tCOE 210/300; clock→PE tCPE 240/375; clock→FE tCFE
  200/300.
  Mode 1 CPU write (p10): tTT 100/150; RSEL setup tRSW 50/75; data setup tDW −30/0; RSEL hold tWRS 50/75;
  data hold tWD 75/125.
  Mode 1 CPU read (p10): tTT 100/150; RSEL setup to TPB tRST 50/75; RSEL hold tTRS 50/75; read→data access
  tRDDA 200/300; read→data valid tRDV 200/300; RSEL→data valid tRSDV 150/225; data hold after read tRDH min
  50, typ 150.
  Mode 0 interface (p14): CRL width tCRL 100/150; MR width tMR 200/400; control setup tCWC 40/80; hold tCCW
  100/150; SFD high→status high-Z tSFDH 200/300; SFD low→status valid tSFDL 75/120; RRD high→R BUS high-Z
  tRRDH 200/300; RRD low→active tRRDL 100/150.
  Mode 0 TX (p15): tCC 250/310; tCL, tCH 100/125; THRL width tTHTH 100/150; THRL-to-clock setup tTHC
  175/275; data setup to THRL tDT 20/50; data hold after THRL tTD 80/120; clock→start bit tCD 300/450;
  clock→THRE tCT 200/300; THRL→THRE tTTHR 200/300; clock→TSRE tTTS 200/300.
  Mode 0 RX (p16): tCC 250/310; tCL/tCH 100/125; DAR width tDD 50/75; tDC 100/150; DAR→DA tDDA 150/225;
  clock→data valid tCDV 225/325; tCDA 225/325; tCOE 210/300; tCPE 240/375; tCFE 200/300.
  (All at TA −40..+85 °C, VDD ±5%, CL 100 pF. Max clock: 200 kbaud @ 5 V → 3.2 MHz 16× clock; 400 kbaud @ 10 V.)
- VCC (p3): CDP1854A VDD 4–10.5 V, CDP1854AC 4–6.5 V; input range VSS..VDD (abs max VDD+0.5 V — not 5 V
  tolerant above VDD). VIL 1.5 V / VIH 3.5 V at VDD 5 V (CMOS levels, VIH 0.7 VDD).
- Package drawings: none in PDF. dip40 = standard 600-mil 40-pin DIP (ceramic D or plastic E).
- Obsolete: none (all census rows Active).
- Notes: Table I says THRE interrupt reset by "read of status or write of character" at TPB leading edge
  but PSI at TPB trailing edge — as printed. The DIP mode-1 drawing labels pin 2 "MODE (VDD)", mode-0
  drawing "MODE (VSS)".

## M66230  (Mitsubishi / Renesas M66230P/FP "A²RT")

- Title: "A²RT (Advanced Asynchronous Receiver & Transmitter)", Mitsubishi Digital ASSP M66230P/FP.
  Datasheet `datasheets/controller-uart/m66230.pdf` (20 pp scanned; p20 is a Findchips stub). p1 features,
  pinout, block diagram; p2 bus interface + Table 1 access; p3 pin descriptions; p4 baud generator, block
  length counter, TX FIFO; p5 RX FIFO; p6 wakeup (multidrop); p7 error detection, CRC; p8 programming flow;
  p9–p10 command and status formats; p11–p12 frame formats; p13 ratings/DC; p14 timing; p15–p17 timing
  waveforms; p18 TX flag timing; p19 RX flag timing. **No package outline drawing, no ordering table.**
- NOT pin-parallel: an 8-bit microprocessor-bus UART (Intel-style RD/WR/CS, one address line C/D) with
  4-byte TX and RX FIFOs, block (packet) length counter, CRC-CCITT, wakeup (9th-bit address) mode.
  8 data bits only.
- Parts: census `M66230FP#TB0Q` (Renesas, Active) → **M66230FP = 24-pin plastic SOP, outline 24P2W-A**
  (p1 "Outline 24P4D (P), 24P2W-A (FP)"; M66230P is the 24-pin DIP 24P4D). "#TB0Q" = Renesas
  packing/taping suffix, **not in the datasheet** (flag; doesn't change the die). Pinout name: sop24.
  Grade: VCC 4.5–5.5 V, −40..+85 °C (p13).
- Pinout sop24 (same as dip24) (p1, p3):
  ```
  1 D0 inout     24 VCC power
  2 D1 inout     23 TXD output
  3 D2 inout     22 RXD input (Schmitt)
  4 D3 inout     21 CTS input (active low, Schmitt)
  5 D4 inout     20 RTS output (active low)
  6 D5 inout     19 P0 output
  7 D6 inout     18 P1 output
  8 D7 inout     17 INT output (polarity programmable, default active low)
  9 RD input     16 CS input (active low)
  10 WR input    15 RESET input (active low, Schmitt)
  11 C_D input   14 X1 input (crystal / external clock)
  12 GND ground  13 X2 output (crystal; leave open with external clock)
  ```
  (C/D printed "C/D̄": high = command/status, low = data; name it `CD` or `C_D`.)
- Host interface (p2 Table 1, p3, p16–p17): CS low enables. Intel style, RD and WR active low.
  | C/D | RD | WR | CS | operation |
  |---|---|---|---|---|
  | L | L | H | L | data bus ← receive data buffer (FIFO) — read received data |
  | L | H | L | L | data bus → transmit data buffer (FIFO) — write TX data |
  | H | L | H | L | data bus ← status register — read status |
  | H | H | L | L | data bus → command register — write command |
  | X | H | H | L | data bus high-Z |
  | X | X | X | H | data bus high-Z |
  Write: CS, C/D set up before WR falling (tsu 0), held after WR rising (th 0); data setup 50 ns before and
  hold 5 ns after **WR rising** → latch on WR rising edge (p14, p16). Read: data driven from RD falling
  (tPZL/tPZH ≤100 ns) until RD rising (+tPLZ/tPHZ ≤85 ns). INT/RTS/P0/P1 change after a command write is
  timed from WR rising (p16); INT after data write is drawn from WR falling and after data read from RD
  falling (p17).
- Command registers (written with C/D = 1; decoded by D7:D6) (p9–p10):
  - **Command 1** (D7:D6 = 00): D5 ST stop bits (1 = 2, 0 = 1); D4 EP (1 even, 0 odd parity); D3 PEN parity
    enable; D2 CRCEN CRC enable; D1 BAUD (1 = the next command-byte write is Command 2, the baud divisor);
    D0 PS prescaler (1 = ÷32, 0 = ÷2). Note 1: if PEN and CRCEN are both 1, parity has priority (CRC off).
    "Attention: TxD output wave is Stop bit (D5) setup value +1 (always)" — the transmitter sends ST+1 stop
    bits, i.e. 2 (ST=0) or 3 (ST=1); p11 frame "Stop bit (1~2 bits)+1"; the receiver expects 1–2 (p12).
  - **Command 2** (the byte written immediately after a Command 1 with D1 = 1; all 8 bits): n = 0..255.
  - **Command 3** (D7:D6 = 01): D5..D0 = m, block length = m+1 bytes (1..64).
  - **Command 4** (D7:D6 = 10): D5 IR internal reset (1 = reset); D4 ER error reset (1 = clear error flags
    CRCE/PE/OE/FE); D3 RTS (1 → RTS pin low, 0 → high); D2 WUMODE wakeup mode enable (if 1, the next
    command-byte write is Command 5); D1 RXEN receive enable; D0 TXEN transmit enable.
  - **Command 5** (byte after Command 4 with D2 = 1; all 8 bits): this station's wakeup address.
  - **Command 6** (D7:D6 = 11): D3 (1 = FIFO disable, 0 = FIFO enable); D2 INT polarity (1 = INT active
    high, 0 = active low "INT̄"); D1 P1 function (1 = P1 indicates "packet transmission is complete",
    0 = P1 follows P0 value... printed "P1=P0"); D0 P0 level (1 = H, 0 = L). D5, D4 not labelled (unused).
  Initial-setting flow (p8): after reset load Command 1 (→ Command 2 optionally), Command 3, Command 4
  (→ Command 5 optionally), Command 6, in any order; then data transfer. RESET is a master reset; commands
  must be loaded after it (p3). Reset values of commands are **not given**.
- Status (read with C/D = 1) (p10): "Status 1 and 2 cannot be addressed from an external pin. Status is
  discriminated by D7. Status 1 and 2 are read alternately (the same status is not read twice in a row)."
  So successive status reads alternate Status 1 / Status 2; which comes first after reset is not stated.
  - **Status 1** (D7 = 0): D6 CRCE CRC error in received data; D5 PE parity error; D4 OE overrun; D3 FE
    framing error; D2 TxBEMP transmit data FIFO empty; D1 RxBFULL receive FIFO full; D0 RxBPE receive FIFO
    packet end.
  - **Status 2** (D7 = 1): D6–D4 = 0; D3 TxEMP: no transmit characters in the transmitter (FIFO + transmit
    buffer); D2 TxPE: packet transmission complete from the transmitter; D1 WUEN: wakeup is maintained
    (station currently woken/accepting); D0 WUMODE: wakeup mode.
- Behavior:
  - Clock: crystal on X1/X2 or external clock into X1. Baud (p4): baud = f(X1) / (prescaler(2 or 32) ×
    (n+1) × 16). Example 9600 = 9.8304 MHz /(2·32·16). Max 500 kbps. X1 max 16 MHz (62.5 ns period) except
    in wakeup or CRC mode 12.5 MHz (80 ns) (p14). 16× sampling implied by the formula; sampling point not
    described.
  - Frame (p11–p12): start, 8 data bits, wakeup bit (only in wakeup mode), parity (if PEN), stop bits.
    Bit order not stated (assume LSB first). Idle = mark (high).
  - Block length counter (p4): 6-bit, block = m+1 bytes. With CRC enabled each block of m+1 data bytes is
    followed by 2 block-check characters (CRC-CCITT x^16+x^12+x^5+1 over data characters only — start,
    stop, wakeup bits excluded; p7). Byte order / initial value / bit order of the CRC are **not given**.
    Receiver strips the 2 BCC bytes (only data goes to the FIFO); a CRC mismatch sets CRCE (status 1 D6)
    after the BCC is received.
  - TX FIFO (4 bytes, p4, p18): MPU may write only when the FIFO is empty. Block length 1–3: write exactly the
    whole block; Block length ≥4: write 4 bytes at a time, and the last (block mod 4) bytes at the end.
    Data moves FIFO → transmit buffer (shift register) when the block/4-byte group has been written, CTS is
    low and TXEN = 1. If CTS goes high during a transfer, all data already queued (incl. FIFO) is still
    sent. After the FIFO empties, the next FIFO data is not sent until the MPU writes a new full group.
    TxBEMP (status 1 D2) = FIFO empty, and when INT is enabled, INT asserts on TX FIFO empty.
    P1 (command 6 D1 = 1) = "packet transmission complete"; status 2 TxPE / TxEMP likewise.
  - RX FIFO (4 bytes, p5, p19): BL 1–3: when a full block has been received, RxBPE (packet end) sets and INT
    asserts; the MPU must read all the bytes; until then no further character is moved from the receive
    buffer into the FIFO, and the MPU cannot read before packet end. BL ≥4: when 4 bytes are in the FIFO
    RxBFULL sets and INT asserts; read all 4; at the last part of the block, packet end as above (if the
    block is a multiple of 4, RxBFULL and RxBPE set together on the last byte). Flags clear / INT
    deasserts when the FIFO has been read empty (p19: INT returns inactive and RxBPE/RxBFULL clear on the
    RD of the last byte). "Read/write by the MPU should be made for all data in the FIFO at once."
  - Errors (p7): PE (D5), FE (D3) set and the character still goes to the FIFO. OE (D4) set when a character
    completes before the MPU has read all data in the RX FIFO; that new character is lost (p19 examples:
    "DATA 3 LOST" with BL=1 when DATA2 still unread; "DATA 7 LOST", "DATA 10 LOST"). CRCE (D6) set after
    the BCC. All error flags held until Command 4 with ER (D4) = 1. Reception continues after errors.
  - Interrupt INT (p3): asserted when the RX FIFO is full, the TX FIFO is empty, or block reception is
    complete (packet end). Polarity set by Command 6 D2 (low-active default per the timing drawings).
    Interrupt enables per source are not provided; FIFO disable (command 6 D3) behaviour is **not
    described** further.
  - Wakeup / multidrop (p6): Command 4 D2 = 1 enables wakeup mode, and Command 5 holds the address. In
    wakeup mode the transmitter adds a 9th bit: 1 on the first byte of a block (the address), 0 on the
    rest. A receiving M66230 compares the first byte (9th bit = 1) with Command 5: on match it accepts the
    following data (status 2 WUEN = 1); on mismatch it only accepts a byte whose 9th bit is 1 (waits for
    the next address). "The wakeup function is automatically canceled when the transfer block data has been
    read by the MPU (the wakeup mode continues)" — i.e. after a block is read the station goes back to
    waiting for an address. With CRC, the 2 BCC frames also carry a wakeup bit (0).
  - RTS: direct from Command 4 D3 (1 → low). CTS: gates transmission (low = send) when TXEN = 1.
  - P0, P1: general outputs (P0 = Command 6 D0; P1 = P0 or packet-complete flag).
  - Reset: RESET low ≥100 ns (tw(RESET)); "recovery time between write" trec 100 ns. Command 4 D5 IR =
    internal (software) reset. States of outputs after reset are **not documented**.
- Timing (p14, VCC 5 V ±10%, −40..+85 °C, CL 150 pF): X1 period ≥62.5 ns, high/low ≥30 ns (non-wakeup/CRC);
  ≥80 ns, high/low ≥38 ns (wakeup/CRC); X1 tr/tf ≤20 ns. Address (CS, C/D) setup/hold to RD and WR: 0/0 ns.
  RD pulse ≥100 ns, WR pulse ≥100 ns. Data setup before WR rising 50 ns, hold 5 ns. RESET pulse ≥100 ns,
  recovery 100 ns. Switching (typ/max): RD→data enable tPZH 43/100, tPZL 52/100; RD→float tPHZ 33/85,
  tPLZ 32/85; INT after read data 62–63/170; INT after write data 54/150; INT after write command 4
  33–35/100; INT after command 6 28–30/100; WR→P0 25–28/70; WR→P1 26–28/70; WR→RTS 25–27/70.
  p1: access time ta(RD-D) 100 ns max.
- VCC (p13): 4.5–5.5 V; VI abs max −0.5..VCC+0.5 V (not tolerant above VCC). VIH 2.0/VIL 0.8 V for RD, WR,
  C/D, CS, D0–D7 (TTL levels); X1 CMOS 0.8/0.2 VCC; RXD, CTS, RESET Schmitt (VT+ ≤2.4, VT− ≥0.6 V).
  Drive: TXD, RTS, P0, P1 ±24 mA; INT, D0–D7 ±8 mA.
- Package drawings: none in the PDF. 24P2W-A is Mitsubishi's 24-pin 300-mil SOP (1.27 mm pitch) —
  dimensions must come from a Renesas package drawing (not available here).
- Obsolete: none (Active).
- Notes: unresolved from the datasheet — bit order on the line (LSB first assumed), CRC preset/byte order,
  command-register reset values, which status (1 or 2) is read first after reset, how INT behaves for TX
  while CTS is high (p18 arrows ambiguous), FIFO-disable mode behaviour. 
