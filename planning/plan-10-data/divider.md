# Dividers and rate multipliers — datasheet extraction (plan-10)

## PDF → devices

| PDF (datasheets/…) | Devices |
|---|---|
| divider/cd4017b.pdf | 4017B, 4022B |
| divider/cd74hc4017.pdf | 74_4017 (HC) |
| divider/cd4018b.pdf | 4018B |
| divider/cd4059a.pdf | 4059A |
| divider/cd74hc4059.pdf | 74_4059 (HC) |
| divider/cd4089b.pdf | 4089B |
| divider/cd4527b.pdf | 4527B |
| divider/cd4522b.pdf | 4522B |
| divider/sn5497.pdf | 74_97 (TTL) |

Census rows not mapped from the PDF itself: JM38510/05651BEA and M38510/05651BEA (cd4017b.pdf, which covers both 4017B and 4022B). They are assigned to 4017B by inference: cd4018b's own rows are /05652, so /05651 is most likely the 4017B. This is unverified.

All CD4000-series PDFs here, and sn5497.pdf, are Harris/TI scans. Logic was read from rendered page images: gate equations come from the logic diagrams, and are cross-checked against truth tables and timing diagrams wherever possible. Each section says how confident each reading is.

## 4017B

- **Title:** "CD4017B, CD4022B Types, CMOS Counter/Dividers. CD4017B, Decade Counter with 10 Decoded Outputs" (cd4017b.pdf p.1).
- **Datasheets:** datasheets/divider/cd4017b.pdf, pp.1 (text, pinout, rec. op. cond.), 2 (Fig.1 logic diagram, Fig.2 timing), 3 (static), 4 (dynamic, Fig.9), 5 (applications, Fig.18/19), 7–9 (addendum).
- **Families & prefixes:** C (CD4000): CD4017B* (CD4017BE, BEE4, BF, BF3A, BM, BM96, BM96G4, BNSR, BPW, BPWR, BPWRE4).
- **Military:** JM38510/05651BEA and M38510/05651BEA, probably (unverified, see top).
- **Pinouts:**
  - p16: N, J, D, NS, PW (one pin arrangement; p.1 "CD4017B Terminal Diagram", which matches the pin circles in Fig.1 on p.2):
    ```
    1  5      output
    2  1      output
    3  0      output
    4  2      output
    5  6      output
    6  7      output
    7  3      output
    8  VSS    ground
    9  8      output
    10 4      output
    11 9      output
    12 CARRY OUT  output
    13 CLOCK INHIBIT  input
    14 CLOCK  input
    15 RESET  input
    16 VDD    power
    ```
    The outputs are printed as bare digits 0–9 (decoded decimal outputs). No pin has an overbar.
- **Behavior** (from the Fig.1 logic diagram, p.2; text p.1; Fig.2/Fig.9):
  - **Internal clock.** CLOCK (pin 14) goes into a Schmitt-trigger inverter. That inverter's output and the buffered (non-inverted) CLOCK INHIBIT (pin 13: inverter then inverter) feed a 2-input NOR. The NOR output drives the C inputs of all five flip-flops. So **CLK_int = CLOCK · ¬INHIBIT**.
  - **Clock edge.** The flip-flops are D type and advance on the **rising edge of CLK_int**. Text p.1: "advanced one count at the positive clock signal transition if the CLOCK INHIBIT signal is low … Counter advancement via the clock line is inhibited when the CLOCK INHIBIT signal is high."
  - **Inhibit edge.** CLK_int also rises when **INHIBIT falls while CLOCK is high**, so that also advances the counter. This is confirmed by the p.1 footnote: "If Pin 13 is used as the clock input and Pin 14 is tied high (for advancing count on negative transition of the clock), rise and fall time should be ≤ 15 µs." With pin 14 = CLOCK used alone, the rise/fall time is "UNLIMITED" (Schmitt trigger).
  - **Inhibit setup.** Inhibit must be set up before the clock edge: t_s ≥ 230 ns at 5 V (rec. op. cond. p.1).
  - **RESET** (pin 15):
    - Active **HIGH** and **asynchronous**. It is buffered (two inverters) straight to the R input of all 5 flip-flops.
    - It clears all stages to 0, giving count 0: output "0" = H, outputs 1–9 = L, CARRY OUT = H.
    - Text: "A high RESET signal clears the counter to its zero count."
    - Priority: RESET over everything.
    - Reset removal time is 400 ns min at 5 V (rec. op. cond.).
  - **Johnson stages.** Five D flip-flops Q1..Q5, labelled Q1..Q5 with Q̄ outputs in Fig.1. Feedback read from the diagram:
    - D1 = Q̄5
    - D2 = Q1
    - **D3 = NOR( AND(Q̄1, Q̄3), Q̄2 ) = Q2·(Q1 + Q3)**. This is the anti-lock gate: an AND feeding a NOR, whose output goes to D of FF3.
    - D4 = Q3
    - D5 = Q4

    The scan is messy around FF1/FF2: dots appear on both the Q1 and Q̄1 lines near D2. D2 = Q1 is the only reading that matches the printed count sequence. The anti-lock gate inputs read cleanly as Q̄1, Q̄3 (AND) and Q̄2 (NOR).
  - **Valid sequence** (Q1..Q5), count n = 0…9: 00000, 10000, 11000, 11100, 11110, 11111, 01111, 00111, 00011, 00001, then back to 00000.
  - **Decoded outputs.** Each output is a 2-input NAND followed by an inverter, so it is an AND (Fig.1):
    - "0" = Q̄1·Q̄5
    - "1" = Q1·Q̄2
    - "2" = Q2·Q̄3
    - "3" = Q3·Q̄4
    - "4" = Q4·Q̄5
    - "5" = Q1·Q5
    - "6" = Q̄1·Q2
    - "7" = Q̄2·Q3
    - "8" = Q̄3·Q4
    - "9" = Q̄4·Q5

    The individual gate wiring in the scan is hard to trace one line at a time. These equations are the ones consistent with the visible wiring (gate 0 uses Q̄1 and the D1 = Q̄5 line) and with Fig.2. In a valid state exactly one output is high, for one full clock period.
  - **CARRY OUT** (pin 12):
    - Driven from Q5 through a 3-stage buffer chain. **CARRY OUT = ¬Q5**: high for counts 0–4, low for counts 5–9 (Fig.2).
    - It is a 50 %-duty square wave at f_CLK/10, and it rises on the clock edge that goes from count 9 to count 0.
    - Text: "A CARRY-OUT signal completes one cycle every 10 clock input cycles … used to ripple-clock the succeeding device."
  - **Illegal states.** The datasheet only says "Anti-lock gating is provided, thus assuring proper counting sequence." It prints no state diagram.
    - I simulated the D-equations above over all 32 states. Every illegal state reaches the valid cycle within at most 10 clocks. Examples:
      - 01000→10000
      - 01001→00000
      - 01011→00001
      - 00100→10010→11001→01100→10110→11011→01101→00110→10011→01001→00000 (10 clocks)
    - While the state is illegal, the decoded outputs are just the AND equations above, so 0, 2 or more outputs can be high.
  - **Power-up:** not stated.
  - **Priority:** RESET (async, H) > CLOCK INHIBIT (H blocks) > CLOCK ↑.
- **Timing** (C, p.4, VDD = 5 V, CL = 50 pF, typ / max):
  - CLOCK → decode out tPLH/tPHL: **325** / 650 ns
  - CLOCK → CARRY OUT: **300** / 600 ns
  - RESET → carry out or decode out lines: **265** / 530 ns
  - Transition time: 100 ns typ
  - fmax: 5 MHz typ (2.5 MHz min)
  - Min clock pulse width: 100 typ / 200 ns
  - Clock-inhibit-to-clock setup: 115 typ / 230 ns
  - Reset pulse width: 130 typ / 260 ns
  - Reset removal: 200 typ / 400 ns

  No separate inhibit→Q delay is printed. Use clock→Q.
- **VCC:** VDD 3–18 V recommended (p.1). Absolute max is −0.5 to +20 V (p.3).
- **Obsolete:** CD4017BM, CD4017BPW.
- **Notes:**
  - Fig.18 (p.5) shows a divide-by-N trick using the decoded outputs plus an external CD4001B S-R latch.
  - Fig.19 cascades with an external AND gate driving CE (the inhibit input).
  - Fig.9 shows reset taking decode outputs 1–9 low (tPHL) and decode "0"/carry high (tPRLH).

## 4022B

- **Title:** "CD4022B, Octal Counter with 8 Decoded Outputs" (cd4017b.pdf p.1).
- **Datasheets:** datasheets/divider/cd4017b.pdf, pp.1, 2 (Fig.3 logic diagram, Fig.4 timing), 3, 4, 5, 7–9.
- **Families & prefixes:** C: CD4022B* (CD4022BE, BEE4, BF, BF3A, BNSR, BPW, BPWR). It is not made in SOIC-D: the text says M/M96 are CD4017B only.
- **Military:** none mapped (see top).
- **Pinouts:**
  - p16: N, J, NS, PW (p.1 "CD4022B Terminal Diagram", matching Fig.3):
    ```
    1  1      output
    2  0      output
    3  2      output
    4  5      output
    5  6      output
    6  NC     nc
    7  3      output
    8  VSS    ground
    9  NC     nc
    10 7      output
    11 4      output
    12 CARRY OUT  output
    13 CLOCK INHIBIT  input
    14 CLOCK  input
    15 RESET  input
    16 VDD    power
    ```
- **Behavior:** clock, inhibit and reset work exactly as in 4017B (Fig.3 has the same input circuit: Schmitt inverter plus NOR with buffered inhibit, so **CLK_int = CLOCK·¬INHIBIT**, advancing on its rising edge; RESET is async active-H to all R inputs).
  - **Johnson stages.** Four D flip-flops Q1..Q4:
    - D1 = Q̄4
    - D2 = Q1
    - **D3 = NOR(AND(Q̄1, Q̄3), Q̄2) = Q2·(Q1+Q3)** (the same anti-lock gate)
    - D4 = Q3
  - **Valid sequence** (Q1..Q4): 0000, 1000, 1100, 1110, 1111, 0111, 0011, 0001, then back to 0000.
  - **Decoded outputs:**
    - "0" = Q̄1·Q̄4
    - "1" = Q1·Q̄2
    - "2" = Q2·Q̄3
    - "3" = Q3·Q̄4
    - "4" = Q1·Q4
    - "5" = Q̄1·Q2
    - "6" = Q̄2·Q3
    - "7" = Q̄3·Q4
  - **CARRY OUT** = ¬Q4: high for counts 0–3, low for counts 4–7. It cycles once every 8 clocks (Fig.4, text p.1).
  - **Illegal states** (simulated from the equations): all 8 illegal states reach the cycle within 5 clocks, for example 1010→1101→0110→1011→0101→0000 and 0100→1000.
  - **Priority:** RESET > INHIBIT > CLOCK ↑.
- **Timing:** the same table as 4017B covers both devices (p.4): clock→decode 325 ns typ, clock→carry 300 ns typ, reset→out 265 ns typ (VDD 5 V, 50 pF).
- **VCC:** 3–18 V.
- **Obsolete:** CD4022BPW.
- **Notes:** none beyond the shared notes under 4017B.

## 74_4017

- **Title:** "CD54HC4017, CD74HC4017 High-Speed CMOS Logic Decade Counter/Divider with 10 Decoded Outputs" (cd74hc4017.pdf p.1).
- **Datasheets:** datasheets/divider/cd74hc4017.pdf, pp.1 (description, pinout), 2 (truth table), 3 (operating conditions, DC), 4–5 (switching), 6 (Fig.4 flip-flop detail, Fig.5 timing), addendum pp.7–8.
- **Families & prefixes:** HC: CD74HC4017*, CD54HC4017* (CD54HC4017F3A is in the ordering table but not in the census).
  - The addendum also lists CD74HC4017-Q1 and -EP.
  - The operating conditions print the generic "HCT Types 4.5V to 5.5V" line, but **there is no HCT part** in this PDF.
- **Military:** no census rows (the CD54 is not in the census).
- **Pinouts:**
  - p16: N, D, NS, PW, J (CERDIP) (p.1):
    ```
    1  5   output
    2  1   output
    3  0   output
    4  2   output
    5  6   output
    6  7   output
    7  3   output
    8  GND ground
    9  8   output
    10 4   output
    11 9   output
    12 TC  output
    13 CE  input
    14 CP  input
    15 MR  input
    16 VCC power
    ```
    This is the same pin arrangement as CD4017B with different names: CP = CLOCK, CE = CLOCK INHIBIT (active-low enable; Fig.5 prints "CLOCK ENABLE" with an overbar), MR = RESET, TC = CARRY OUT.
- **Behavior:** the PDF prints no gate-level logic diagram. Fig.4 is only the master-slave flip-flop detail with an async R, and Fig.5 is the timing diagram. Everything below comes from the text, the truth table and the timing diagram, all of which match the CD4017B.
  - **Truth table (p.2), quoted:**
    - CP L, CE X, MR L: No Change
    - CP X, CE H, MR L: No Change
    - CP X, CE X, MR H: "0" = H, "1"–"9" = L
    - CP ↑, CE L, MR L: Increments Counter
    - CP ↓, CE X, MR L: No Change
    - CP X, CE ↑, MR L: No Change
    - CP H, CE ↓, MR L: Increments Counter
    - Footnote †: "If n < 5 TC = H, Otherwise = L"

    **Datasheet error:** the legend swaps the arrows ("↑ = High to Low Transition, ↓ = Low to High Transition"). The features list says "Positive Edge Clocking", and the description says outputs go high "on the low to high transition clock". So read the table with ↑ = rising: count on **CP rising while CE = L**, and on **CE falling while CP = H**.
  - **Internal clock:** CLK_int = CP·¬CE, counting on its rising edge.
  - **CE:** "The CLOCK ENABLE input disables counting when in the high state." CE setup to CP is 15 ns and hold is 0 ns at 4.5 V.
  - **MR:** active HIGH and async. "sets all the decoded outputs, except '0', low", and TC goes high. MR removal time is 5 ns.
  - **Johnson counter:** 5 stages, with the same 10-state sequence and decode as 4017B. TC = ¬(stage 5): high for counts 0–4, low for 5–9. "The CARRY (TC) output transitions low to high after OUTPUT 10 [i.e. '9'] goes from high to low."
  - **Illegal-state behavior:** not stated in this PDF. Use the 4017B anti-lock model.
  - **Priority:** MR > CE (H blocks) > CP ↑.
- **Timing** (HC, p.4–5): no typical is printed at 4.5 V / 50 pF, so these are 25 °C MAX values at 4.5 V, CL = 50 pF, with the 5 V / 15 pF typical in parentheses:
  - CP → any decoded out: **46 ns max** (typ 19)
  - CP → TC: **46** (19)
  - CE → decoded out: **50** (21)
  - CE → TC: **50** (21)
  - MR → decoded out: **46** (19)
  - MR → TC: **46** (19)
  - Transition time: 15 ns
  - fmax: 30 MHz min at 4.5 V; 60 MHz typ at 5 V / 15 pF
  - tW (CP, MR): 16 ns
- **VCC:** HC 2–6 V (p.3). Inputs are limited to 0 V…VCC.
- **Obsolete:** CD74HC4017M, CD74HC4017MT, CD74HC4017PW, CD74HC4017PWT. The NRND rows are CD74HC4017PWR and PWRE4.
- **Notes:**
  - The arrow legend error described above.
  - p.1 lists "Bus Driver Outputs 15 LSTTL" as generic boilerplate.
  - The CD54HC4017F3A is in the ordering table but missing from the packaging addendum.

## 4018B

- **Title:** "CD4018B Types — CMOS Presettable Divide-By-'N' Counter, High-Voltage Types (20-Volt Rating)" (cd4018b.pdf p.1).
- **Datasheets:** datasheets/divider/cd4018b.pdf. All of pp.1–5 are scanned images (no text layer). Sources used:
  - p.1: text, terminal diagram, functional diagram
  - p.2: recommended operating conditions, Fig.1 logic diagram, Fig.2 stage detail
  - p.4: dynamic characteristics
  - p.5: Fig.15 timing, Fig.16 divide-by tables, Fig.17 ÷7 example
  - pp.6–7: addendum
- **Families & prefixes:** C: CD4018B* (CD4018BE, BF, BF3A, BM, BM96, BMT, BNSR, BPW).
- **Military:** JM38510/05652BEA and M38510/05652BEA: all rows (single-function PDF).
- **Pinouts:**
  - p16: N, J, D, NS, PW (terminal diagram, p.1):
    ```
    1  DATA            input
    2  JAM1            input
    3  JAM2            input
    4  Q2   (printed Q̄2)  output
    5  Q1   (printed Q̄1)  output
    6  Q3   (printed Q̄3)  output
    7  JAM3            input
    8  VSS             ground
    9  JAM4            input
    10 PRESET ENABLE   input
    11 Q4   (printed Q̄4)  output
    12 JAM5            input
    13 Q5   (printed Q̄5)  output
    14 CLOCK           input
    15 RESET           input
    16 VDD             power
    ```
    Every output pin is printed with an overbar (Q̄1…Q̄5, "BUFFERED OUT"). They are the complements of the internal stage states (see below). No true Q pins exist, so there is no name collision.
- **Behavior:**
  - **Text p.1:** "5 Johnson-Counter stages, buffered Q̄ outputs from each stage, and counter preset control gating. CLOCK, RESET, DATA, PRESET ENABLE, and 5 individual JAM inputs … The counter is advanced one count at the positive clock-signal transition. Schmitt Trigger action on the clock line permits unlimited clock rise and fall times. A high RESET signal clears the counter to an all-zero condition. A high PRESET-ENABLE signal allows information on the JAM inputs to preset the counter. Anti-lock gating is provided to assure the proper counting sequence."
  - **Fig.1 logic diagram (p.2), read gate by gate:**
    - Internal R̄ = NOT RESET (inverter on pin 15).
    - Internal PE = NAND(R̄, NOT PRESET_ENABLE) = **RESET + PRESET_ENABLE**. Its buffered complement is PĒ. So a reset also asserts the internal preset.
    - **Internal clock CL** = NAND(Schmitt-inverted CLOCK, R̄, NOT PRESET_ENABLE) = **CLOCK + RESET + PRESET_ENABLE**. The D flip-flops are master-slave (Fig.2): the master is transparent while CL is low and the slave while CL is high, so they are **rising-edge triggered on CL**.
    - So the counter advances on the **CLOCK rising edge while RESET = PRESET ENABLE = L**. While RESET or PE is high, CL is held high.
    - Releasing RESET/PE while CLOCK is low makes CL fall, which is not an active edge. Asserting RESET/PE while CLOCK is low makes CL rise, but the preset overrides it.
    - Stage chain: **D1 = DATA (pin 1); D2 = Q1; D3 = NOR(AND(Q̄1, Q̄3), Q̄2) = Q2·(Q1 + Q3)** (the "anti-lock" gate, labelled "N=1" in the figure); **D4 = Q3; D5 = Q4**.
    - Outputs: pin Q̄n = NOT Qn, through an inverting buffer for each stage.
  - **Stage detail (Fig.2, p.2).**
    - Preset path: when PE is high, the master node is forced to NAND(R̄, Jn) through a transmission gate, and the D path is cut. CL is high at the same time (forced), so the slave is transparent.
    - Result: **Qn = R̄·Jn**, so pin Q̄n = NOT Jn when presetting, and Qn = 0 on reset.
    - So **PRESET ENABLE and RESET are asynchronous and level-sensitive**. RESET has priority over PRESET ENABLE (RESET forces all Q = 0, so all Q̄ pins = H).
    - JAMn loads stage n **non-inverted** (Qn = JAMn). The output pin therefore shows Q̄n = NOT JAMn.
    - The Fig.15 timing shows Q̄ outputs being preset by PRESET with "JAM … DON'T CARE UNTIL PRESET GOES HIGH".
  - **Priority:** RESET (async, all stages 0) > PRESET ENABLE (async, Qn = JAMn) > CLOCK rising edge (Qn ← Dn).
  - **Count sequence (decade, DATA = Q̄5)**, Q1..Q5: 00000, 10000, 11000, 11100, 11110, 11111, 01111, 00111, 00011, 00001, then repeat. The anti-lock term D3 = Q2(Q1+Q3) equals Q2 on every legal state.
  - **External feedback table** (Fig.16, p.5), copied as printed: "EXTERNAL CONNECTIONS FOR DIVIDE BY 10, 9, 8, 7, 6, 5, 4, 3 OPERATION".

    | Divide by | Connected back to "DATA" | Notes |
    |---|---|---|
    | 10 | Q̄5 | no external components required |
    | 8 | Q̄4 | no external components required |
    | 6 | Q̄3 | no external components required |
    | 4 | Q̄2 | no external components required |
    | 2 | Q̄1 | no external components required |
    | 9 | Q̄4 and Q̄5 into a gate made from "1/2 CD4011B" (a NAND followed by an inverter = AND) → DATA | "(SKIPS 'ALL-1's' STATE)" |
    | 7 | Q̄3 and Q̄4 into the same AND → DATA | skips all-1s state |
    | 5 | Q̄2 and Q̄3 into the same AND → DATA | skips all-1s state |
    | 3 | Q̄1 and Q̄2 into the same AND → DATA | skips all-1s state |

    So DATA = Q̄k·Q̄(k+1) for odd N = 2k+1.
    - Fig.17 ("Example of divide by 7") shows the same wiring: AND(Q̄3, Q̄4) → buffer → D, with J1–J5 and R tied low, and the output "CL ÷ 7" taken from the gate.
    - Text p.1: "Divide by 10, 8, 6, 4, or 2 counter configurations can be implemented by feeding the Q̄5, Q̄4, Q̄3, Q̄2, Q̄1 signals, respectively, back to the DATA input. Divide-by-9, 7, 5, or 3 counter configurations can be implemented by the use of a CD4011B to gate the feedback connection to the DATA input. Divide-by functions greater than 10 can be achieved by use of multiple CD4018B units."
    - Example, my derivation of ÷9 from the equations: 00000→10000→11000→11100→11110→01111→00111→00011→00001→00000, which is 9 states and skips 11111.
  - **Illegal states:** not stated in the datasheet. Use the equations; the D3 anti-lock term breaks lock-up loops.
  - **Power-up:** not stated.
- **Timing** (p.4; VDD = 5 V, CL = 50 pF, typ/max):
  - CLOCK→Q̄ (tPLH, tPHL): **200 / 400 ns**
  - Preset or Reset→Q̄: **275 / 550 ns** (footnote: "*At PRESET ENABLE or JAM inputs")
  - Transition time: 100 / 200 ns
  - fmax: 6 MHz typ (3 MHz min)
  - Clock pulse width: 80 typ / 160 ns
  - Data setup: 20 typ / 40 ns. Data hold: 70 typ / 140 ns.
  - Preset/reset pulse width: 80 / 160 ns. Removal time: 80 / 160 ns.
- **VDD:** 3–18 V (p.2 recommended).
- **Obsolete:** CD4018BM, CD4018BMT.
- **Notes:**
  - The functional diagram on p.1 labels the outputs "BUFFERED OUT" Q̄1–Q̄5.
  - The Fig.15 timing diagram is drawn for the "DATA input tied to Q̄5 for decade counter configuration".

## 4059A

- **Title:** "CD4059A Types — CMOS Programmable Divide-by-'N' Counter, Standard 'A'-Series Types (3-to-15-Volt Rating)" (cd4059a.pdf p.1).
- **Datasheets:** datasheets/divider/cd4059a.pdf. All of pp.1–8 are scanned. Sources used:
  - p.1: text, Fig.1 (total count of 3), terminal diagram
  - p.2: operating conditions
  - p.3: dynamic characteristics, Fig.5 block diagram
  - p.4: Table I, N formula, examples
  - p.5: examples
  - pp.6–7: applications
  - p.8: Fig.21 power-on circuit
  - p.9: addendum
- **Families & prefixes:** C: CD4059A* (CD4059AM, CD4059AMG4). Note the **A suffix** (A-series, 3–15 V).
- **Military:** none.
- **Pinouts:**
  - p24: DW (census); the text also mentions E (PDIP) and M/M96 (SOIC). Terminal diagram, p.1:
    ```
    1  CL   input   (clock)
    2  L    input   (latch enable; "LATCH ENABLE" in Fig.5)
    3  J1   input
    4  J2   input
    5  J3   input
    6  J4   input
    7  J16  input
    8  J15  input
    9  J14  input
    10 J13  input
    11 Kc   input
    12 VSS  ground
    13 Kb   input
    14 Ka   input
    15 J12  input
    16 J11  input
    17 J10  input
    18 J9   input
    19 J8   input
    20 J7   input
    21 J6   input
    22 J5   input
    23 OUT  output  ("DIVIDE-BY-N OUTPUT")
    24 VDD  power
    ```
- **Behavior.** No gate-level logic diagram is printed. Only the Fig.5 block diagram exists: "first counting section ÷10,8,5,4,2" → three "intermediate counting section ÷10" decades → "last counting section ÷1,2,2,4,8", plus "recognition gating" → "preset enable" (which re-jams all sections) → "output stage" (with LATCH ENABLE). Behavior quoted from the text (p.1):
  - "divide-by-N down-counters that can be programmed to divide an input frequency by any number 'N' from 3 to 15,999. The output signal is a pulse one clock-cycle wide occurring at a rate equal to the input frequency divided by N. … The down-counter is preset by means of 16 jam inputs."
  - "Every time the first (fastest) counting section goes through one cycle, it reduces by 1 the number that has been preset (jammed) into the three decades of the intermediate counting section and the last counting section, which consists of flip-flops that are not needed for operating the first counting section."
  - **Table I (p.4)**, copied completely:

    | Ka | Kb | Kc | First section: divides by | First: can be preset to max of | First: jam inputs used | Last section: divides by | Last: can be preset to max of | Last: jam inputs used | Counter range: design max | Counter range: extended max |
    |---|---|---|---|---|---|---|---|---|---|---|
    | 1 | 1 | 1 | 2 | 1 | J1 | 8 | 7 | J2,J3,J4 | 15,999 | 17,331 |
    | 0 | 1 | 1 | 4 | 3 | J1,J2 | 4 | 3 | J3,J4 | 15,999 | 18,663 |
    | 1 | 0 | 1 | 5# | 4 | J1,J2,J3 | 2 | 1 | J4 | 9,999 | 13,329 |
    | 0 | 0 | 1 | 8 | 7 | J1,J2,J3 | 2 | 1 | J4 | 15,999 | 21,327 |
    | 1 | 1 | 0 | 10 | 9 | J1,J2,J3,J4 | 1 | 0 | – | 9,999 | 16,659 |
    | X | 0 | 0 | MASTER PRESET | | | MASTER PRESET | | | – | – |

    Table notes:
    - X = don't care.
    - J1 = least significant bit, J4 = most significant bit.
    - "#Operation in the ÷5 mode (1st counting section) requires going through the Master Preset mode prior to going into the ÷5 mode. At power turn-on, Kc must be a logic '0' for a period of 3 input clock pulses after VDD reaches a minimum of 3 volts. See Fig. 21 for a suggested external preset circuit."
  - **Intermediate decades.**
    - J5–J8 = "Decade 2" (units), J9–J12 = "Decade 3" (tens), J13–J16 = "Decade 4" (hundreds). Each is BCD, with J5/J9/J13 the LSBs.
    - The last counting section (J2–J4, J3–J4 or J4, depending on mode) is "Decade 5" (thousands).
    - The first section (J1…) is "Decade 1".
    - Text: "The three decades of the intermediate counter can be preset to a binary 15 instead of a binary 9, while their place values are still 1, 10, and 100, multiplied by the number of the ÷N mode." That is what the "Extended" column relies on.
  - **Formula (p.4, Eq.1):** "N = [MODE*] [1000 × Decade 5 Preset + 100 × Decade 4 Preset + 10 × Decade 3 Preset + 1 × Decade 2 Preset] + Decade 1 Preset". Here *MODE = first counting section divider (10, 8, 5, 4 or 2).
    - Eq.2: "Preset Value = N / Mode". "To calculate preset values for any N count, divide the N count by the Mode. The resultant is the corresponding preset values of the 5th through 2nd decade with the remainder being equal to the 1st decade value."
  - **Worked examples (p.4–5), copied:**
    - A) N = 8479, mode 5 (Ka Kb Kc = 1 0 1): 8479/5 = 1695 r 4.
      - J1..J4 = 0 0 1 1 (J1–J3 = 4 = first section; J4 = 1 = thousands)
      - J5..J8 = 1 0 1 0 (5)
      - J9..J12 = 1 0 0 1 (9)
      - J13..J16 = 0 1 1 0 (6)
      - Check: N = 5(1000×1 + 100×6 + 10×9 + 1×5) + 4 = 8479.
    - B) N = 12382, mode 8 (0 0 1): 12382/8 = 1547 r 6.
      - J1..J4 = 0 1 1 1
      - J5..J8 = 1 1 1 0 (7)
      - J9..J12 = 0 0 1 0 (4)
      - J13..J16 = 1 0 1 0 (5)
      - Check: N = 8(1000×1 + 100×5 + 10×4 + 7) + 6.
    - C) N = 8479, mode 10 (1 1 0): 0847 r 9.
      - J1..J4 = 1 0 0 1
      - J5..J8 = 1 1 1 0 (7)
      - J9..J12 = 0 0 1 0 (4)
      - J13..J16 = 0 0 0 1 (8)
      - Check: N = 10(1000×0 + 100×8 + 10×4 + 1×7) + 9.
  - **Master preset:** "Control inputs Kb and Kc can be used to initiate and lock the counter in the 'master preset' state. In this condition the flip-flops in the counter are preset in accordance with the jam inputs and the counter remains in that state as long as Kb and Kc both remain low. The counter begins to count down from the preset state when a counting mode other than the master preset mode is selected." "Whenever the master preset mode is used, control signals Kb = 0 and Kc = 0 must be applied for at least 3 full clock pulses."
  - **Leaving master preset:** "After the Master Preset Mode inputs have been changed to one of the ÷ modes, the next positive-going clock transition changes an internal flip-flop so that the countdown can begin at the second positive-going clock transition. Thus, after an MP mode, there is always one extra count before the output goes high. … If the Master Preset Mode is started two clock cycles or less before an output pulse, the output pulse will appear at the time due. If the Master Preset Mode is not used, the counter jumps back to the 'JAM' count when the output pulse appears."
  - **Clock edge:** positive-going (all counting is on CL rising edges).
  - **Fig.1, "Total count of 3"** (÷8 mode, Ka = Kb = 0, Kc stepping 0→1; p.1), read from the scan:
    - The internal state reads 3, 3, 3 while Kc = 0.
    - After Kc goes high it reads: 3 (the extra count), 2, "1→3", then [3, 2, 1→3] repeating.
    - OUTPUT is high for exactly one clock period: the period after each "1→3" cell.
    - So the first output pulse begins at the (N+1)th rising edge after master preset is released, and pulses then repeat every N rising edges. Each pulse is high from one rising CL edge to the next. (This edge alignment is my reading of the small scan.)
  - **Latch enable:** "A '1' on the Latch Enable input will cause the counter output to remain high once an output pulse occurs, and to remain in the high state until the latch input returns to '0'. If the Latch Enable is '0', the output pulse will remain high for only 1 cycle of the clock-input signal." The datasheet says nothing that suggests counting stops while latched.
  - **Range:** N = 3 … 15,999 (design), and up to the "extended" maxima using BCD digits > 9. Text p.5: "the CD4059A can never be preset to a count less than 3". Behavior for N < 3 is not specified.
  - **Priority:** master preset (Kb = Kc = 0, synchronous to the clock: it needs 3 clocks) > counting. LATCH ENABLE only affects the output stage.
- **Timing** (p.3; VDD = 5 V, CL = 50 pF):
  - Propagation delay tPHL/tPLH (clock→OUT): **180 typ / 360 max ns**
  - Transition tTHL: 35 / 70 ns. tTLH: 100 / 200 ns.
  - fmax: 3 MHz typ (1.5 MHz min)
  - Clock pulse width: 200 ns min (operating conditions, p.2)
  - No LE→OUT or Kb/Kc figures are printed.
- **VDD:** 3–12 V recommended (p.2; absolute max 15 V). "T²L drive capability" output.
- **Obsolete:** none.
- **Notes:**
  - Fig.21 (p.8) is a power-on master-preset circuit for mode 5.
  - Figs.14–16 show cascading with CD4029/CD4013/CD4018.
  - The HC datasheet repeats the text with a typo, "2665 × 8 = 12,320". The CD4059A correctly says 21,320.

## 74_4059

- **Title:** "CD54HC4059, CD74HC4059 High-Speed CMOS Logic CMOS Programmable Divide-by-N Counter" (cd74hc4059.pdf p.1).
- **Datasheets:** datasheets/divider/cd74hc4059.pdf, pp.1–3 (text, truth table, pinout, preset formula, Fig.1 block diagram), 4–5 (DC, switching), 7 (addendum). It has a text layer.
- **Families & prefixes:** HC: CD74HC4059*, CD54HC4059*.
  - The ordering table lists CD54HC4059F3A, CD74HC4059E and CD74HC4059M96. The census only has CD74HC4059M96.
  - The section heading says "How to Preset the HC/HCT4059", but **no HCT part** exists in this PDF.
- **Military:** none in the census.
- **Pinouts:**
  - p24: DW (census; the text also lists E PDIP and F3A CERDIP), p.2:
    ```
    1 CP in; 2 LE in; 3 J1; 4 J2; 5 J3; 6 J4; 7 J16; 8 J15; 9 J14; 10 J13; 11 Kc in;
    12 GND; 13 Kb in; 14 Ka in; 15 J12; 16 J11; 17 J10; 18 J9; 19 J8; 20 J7; 21 J6;
    22 J5; 23 Q out; 24 VCC
    ```
    All J pins are inputs. The pin arrangement is the same as CD4059A ("pin-compatible with the CD4059A"): CP = CL, LE = L, Q = OUT.
- **Behavior:** identical to 4059A. The text (pp.1–3) repeats the CD4059A description word for word, including master preset, the extra count after MP, latch enable and the formula.
  - **Truth table (p.2):** identical to CD4059A Table I. Same rows, same jam inputs used, same design/extended maxima: 15,999/17,331; 15,999/18,663; 9,999/13,329; 15,999/21,327; 9,999/16,659; X L L = Master Preset.
    - Note 2: "Operation in the ÷5 mode … requires going through the Master Preset mode … At power turn-on, Kc must be 'low' for a period of 3 input clock pulses after VCC reaches a minimum of 3V."
  - **Formula (Eq.1/Eq.2):** identical, with the same 8479/mode-5 example.
  - **Master preset setup:** prerequisite "Setup Time Kb, Kc to CP" of 15 ns at 4.5 V confirms that Kb/Kc are sampled on CP edges (synchronous).
  - **Fig.1 mismatch:** the text says "Figure 1 illustrates a total count of 3", but in the HC PDF Figure 1 is the functional block diagram. Use the CD4059A Fig.1 instead.
- **Timing** (p.5; 25 °C, CL = 50 pF; no 4.5 V typical, so MAX values are given):
  - CP→Q: **40 ns max** at 4.5 V (typ 17 ns at 5 V, CL = 15 pF)
  - LE→Q: **35 ns max** at 4.5 V (typ 14 ns at 5 V / 15 pF)
  - Transition time: 15 ns
  - fmax: 27 MHz min at 4.5 V; 54 MHz typ at 5 V / 15 pF
  - CP pulse width: 18 ns min
- **VCC:** 2–6 V.
- **Obsolete:** none.
- **Notes:** the "12,320" typo is described under 4059A.

## 4089B

- **Title:** "CD4089B Types — CMOS Binary Rate Multiplier, High-Voltage Types (20-Volt Rating)" (cd4089b.pdf p.1).
- **Datasheets:** datasheets/divider/cd4089b.pdf. All of pp.1–5 are scanned. Sources used:
  - p.1: text, functional diagram
  - p.2: recommended conditions, Fig.4 logic diagram
  - p.3: dynamic characteristics, terminal assignment
  - p.5: truth table, Fig.13/14 cascading, Fig.15 timing
- **Families & prefixes:** C: CD4089B* (CD4089BE, BEE4, BNSR, BPWR).
- **Military:** none.
- **Pinouts:**
  - p16: N, NS, PW (terminal assignment, p.3):
    ```
    1  "15" OUT                 output   (suggested id: 15OUT)
    2  C                        input
    3  D                        input
    4  SET TO "15"              input    (SET15)
    5  OUT  (printed OUT̄)       output   → name OUTBAR (pin 6 is OUT)
    6  OUT                      output
    7  INHIBIT OUT (CARRY)      output   (INHOUT)
    8  VSS                      ground
    9  CLOCK                    input
    10 STROBE                   input
    11 INHIBIT IN (CARRY)       input    (INHIN)
    12 CASCADE                  input
    13 CLEAR                    input
    14 A                        input
    15 B                        input
    16 VDD                      power
    ```
    Rate inputs: A = LSB (weight 1), D = MSB (weight 8). Fig.15 labels "(LSB) INPUT A" and "(MSB) INPUT D".
- **Behavior.** Text (p.1): "low-power 4-bit digital rate multiplier that provides an output pulse rate that is the clock-input-pulse rate multiplied by 1/16 times the binary input … when the binary input number is 13, there will be 13 output pulses for every 16 input pulses." "internal synchronous 4-bit counter which, together with one of the four binary input bits, produces pulse trains … If more than one binary input bit is high, the resulting pulse train is a combination of the separate pulse trains."
  - **Counter:** synchronous 4-bit binary **up** counter. Qa is the LSB and Qd the MSB; Fig.15 shows states 0,1,…,15,0.
    - It advances on the **CLOCK rising edge** while INHIBIT IN = L. Fig.15 shows each state beginning at a rising edge, with output pulses in the low half that follows.
    - **INHIBIT IN** is a synchronous count-enable (active-low enable): its inverted level is fed to the counter, and it has setup/removal times referenced to the clock.
    - The inhibit does **not** gate the output stage. Truth table: INH IN = 1 → OUT "† depends on internal state of counter".
  - **Logic diagram Fig.4 (p.2), read from the scan:**
    - Internal clear = NOR(NOT CLEAR, SET) = **CLEAR·¬SET**, so SET has priority over CLEAR.
    - **SET TO "15"** forces the counter to 1111. CLEAR forces it to 0000. Both are **asynchronous** (pulse-width and removal-time specs exist) and active-high.
    - Rate decode ANDs (non-overlapping):
      - TA = A·Qa·Qb·Qc·Q̄d
      - TB = B·Qa·Qb·Q̄c
      - TC = C·Qa·Q̄b
      - TD = D·Q̄a
    - NOR4 = NOR(TA, TB, TC, TD).
    - **G = AND(¬STROBE, ¬CLOCK, ¬NOR4) = ¬STROBE · ¬CLOCK · (TA+TB+TC+TD)**. CLOCK here is taken directly from the pin, before the counter's clock buffer.
    - **OUT̄ (pin 5) = ¬G.**
    - **OUT (pin 6) = CASCADE + G.** The gate is a NOR followed by an inverter.
    - **"15" (pin 1) = Qa·Qb·Qc·Qd** (NAND followed by an inverter), high during count 15.
    - **INHIBIT OUT (pin 7) = OR(INHIBIT IN, Q̄a, NAND(·,·,·))**. The scan labels the 3-input NAND's inputs "Q̄b, Q̄c, Q̄d". That contradicts the truth table: SET → INH OUT = L, CLEAR → INH OUT = H, INH IN = 1 → H. The function consistent with the truth table is **INHIBIT OUT = INHIBIT IN + ¬(Qa·Qb·Qc·Qd)**, i.e. low only during count 15 with INH IN low. I treat the bars as a drawing error.
  - **Output pulses** (Fig.15, p.5). OUT is high while CLOCK is low during these counter states:
    - D: 0,2,4,6,8,10,12,14 (8 pulses)
    - C: 1,5,9,13 (4)
    - B: 3,11 (2)
    - A: 7 (1)
    - Output count = 8D + 4C + 2B + A per 16 clocks.
    - State 15 never produces a pulse. Fig.15 note: "*An output bit may be filled in this counter state by a less significant CD4089 cascaded in the Add mode."
  - **Truth table (p.5), copied.** Inputs D C B A, CLK, INH IN, STR, CAS, CLR, SET; outputs OUT, OUT̄, INH OUT, "15" OUT.
    - The 16 rows DCBA = 0000…1111, with CLK = 16 pulses and every control 0, give OUT = OUT̄ = 0…15 pulses ("0" row: OUT = L, OUT̄ = H), INH OUT = 1 and "15" = 1 (one pulse each per 16 clocks).
    - X X X X, 16 clocks, INH IN 1: OUT †, OUT̄ †, INH OUT H, "15" †
    - XXXX, STR 1: OUT L, OUT̄ H, INH OUT 1, "15" 1
    - XXXX, CAS 1: OUT H, OUT̄ * (same as the first 16 lines), INH OUT 1, "15" 1
    - 1XXX, CLR 1: OUT 16, OUT̄ 16, INH OUT H, "15" L
    - 0XXX, CLR 1: OUT L, OUT̄ H, INH OUT H, "15" L
    - XXXX, CLR X, SET 1: OUT L, OUT̄ H, INH OUT L, "15" H
    - Footnotes: "* Output same as the first 16 lines of this truth table (depending on values of A, B, C, D). † Depends on internal state of counter."
  - **Priority:** SET > CLEAR (both async) > INHIBIT IN (sync) > CLOCK rising edge.
    - Output: STROBE high forces OUT̄ = H (and OUT = CASCADE).
    - CASCADE high forces OUT = H only.
  - **Cascading:**
    - Fig.13, "Add" mode: both units share the clock. The less-significant unit's INH IN is driven by the more-significant unit's INH OUT. Output: N1/16 + N2/256 (11/16 + 13/256 = 189/256).
    - Fig.14, "Multiply" mode: N1/16 × N2/16 (11/16 × 13/16 = 143/256).
    - The exact wiring is in the figures (OUT̄ of one unit into CASCADE of the other, etc.). The figure scans are small, so the wiring is not transcribed beyond this.
- **Timing** (p.3; VDD = 5 V, CL = 50 pF, typ/max ns):

  | Path | typ | max |
  |---|---|---|
  | Clock→OUT̄ | 110 | 220 |
  | Clock or Strobe→OUT | 150 | 300 |
  | Clock→Inhibit Out H→L | 360 | 720 |
  | Clock→Inhibit Out L→H | 250 | 500 |
  | Clear→Out | 380 | 760 |
  | Clock→"9" or "15" Out | 300 | 600 |
  | Cascade→Out | 90 | 180 |
  | Inhibit In→Inhibit Out | 160 | 320 |
  | Set→Out | 330 | 660 |

  Also: fmax 2.4 MHz typ (1.2 min); clock pulse width 165 typ / 330 ns.
- **VDD:** 3–18 V.
- **Obsolete:** none.
- **Notes:**
  - The INHIBIT OUT label conflict described above.
  - The dynamic table is shared text with CD4527 (it says '"9" or "15" Out').

## 4527B

- **Title:** "CD4527B Types — CMOS BCD Rate Multiplier, High-Voltage Types (20-Volt Rating)" (cd4527b.pdf p.1).
- **Datasheets:** datasheets/divider/cd4527b.pdf. All of pp.1–5 are scanned. Sources used:
  - p.1: text, terminal assignment, recommended conditions
  - p.3: dynamic characteristics, Fig.12
  - p.4: Fig.13 logic diagram, Fig.14 timing, Fig.15
  - p.5: truth table
- **Families & prefixes:** C: CD4527B* (CD4527BE, BNSR, BPW, BPWR).
- **Military:** none.
- **Pinouts:**
  - p16: N, NS, PW (p.1):
    ```
    1  "9" OUT              output  (9OUT)
    2  C                    input
    3  D                    input
    4  SET TO "9"           input   (SET9)
    5  OUT  (printed OUT̄)   output  → OUTBAR
    6  OUT                  output
    7  INHIBIT OUT (CARRY)  output
    8  VSS                  ground
    9  CLOCK                input
    10 STROBE               input
    11 INHIBIT IN (CARRY)   input
    12 CASCADE              input
    13 CLEAR                input
    14 A                    input
    15 B                    input
    16 VDD                  power
    ```
    This is the same arrangement as CD4089B except pin 1 ("9" vs "15") and pin 4 (SET TO 9 vs SET TO 15).
- **Behavior.** Text: "provides an output-pulse rate which is the clock-input-pulse rate multiplied by 1/10 times the BCD input. For example, when the BCD input is 8, there will be 8 output pulses for every 10 input pulses."
  - **Counter** (Fig.13): four T flip-flops a, b, c, d, clocked from CLOCK (buffered).
    - Each T input = AND(¬INHIBIT IN, carry terms), so INHIBIT IN is a synchronous count enable, active-low.
    - c and d have S inputs and a, b, c, d have R inputs.
    - **SET TO "9"** sets c and d and resets a and b. **CLEAR** resets all four. Both are async and active-high.
    - Truth-table footnote: "#Clear and Set inputs should not be high at the same time; device draws increased quiescent current in this non-valid state."
  - **Count edge:** Fig.14 shows each state starting at a CLOCK rising edge, with outputs in the clock-low half.
  - **Internal state encoding** (read from Fig.14's Qa–Qd waveforms). It is **not 8421**: it is a bi-quinary decade, value = Qa + 2Qb + 4Qc + 5Qd. The 10 states in order:

    | Count | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
    |---|---|---|---|---|---|---|---|---|---|---|
    | Qd Qc Qb Qa | 0000 | 0001 | 0010 | 0011 | 0100 | 1000 | 1001 | 1010 | 1011 | 1100 |

    Consistent checks: SET TO 9 sets c and d (1100 = 9); "9" = Qc·Qd. If a model only uses the decoded signals below, a plain decade counter 0–9 is equivalent.
  - **Rate decode** (Fig.13 gates R1–R4, waveforms in Fig.14):
    - R1 (with A) is high in count 4.
    - R2 (with B) in counts 2, 7.
    - R3 (with C) in counts 1, 3, 6, 8.
    - R4 (with D) in counts 0, 1, 2, 3, 5, 6, 7, 8.
    - Equivalent equations in the encoding above: R1 = A·Qc·Q̄d, R2 = B·Qb·Q̄a, R3 = C·Qa, R4 = D·Q̄c.
    - The B and C sets are subsets of the D set. So with D = 1 the B and C inputs add nothing, which matches the truth table: DCBA 1010 → 8, 1011 → 9, 1100 → 8, 1101 → 9, 1110 → 8, 1111 → 9.
  - **Output stage:** the same structure as 4089B.
    - G = ¬STROBE · ¬CLOCK · (R1+R2+R3+R4)
    - **OUT̄ = ¬G**
    - **OUT = CASCADE + G**
  - **"9" OUT** = count 9 (Qc·Qd), active-high. **INHIBIT OUT** = low during count 9 when INHIBIT IN is low (Fig.14 "INH. OUT" low during the 9 state). Truth table: SET → INH OUT L, "9" H; CLEAR → INH OUT H, "9" L; INH IN = 1 → INH OUT H.
  - **Truth table (p.5), copied.** Same columns as 4089 with CLK = 10.
    - DCBA 0000…1001 give OUT = OUT̄ = 0…9 pulses ("0": OUT L, OUT̄ H).
    - 1010 → 8, 1011 → 9, 1100 → 8, 1101 → 9, 1110 → 8, 1111 → 9.
    - INH OUT = 1 and "9" = 1 per 10 clocks.
    - XXXX, INH IN 1: OUT †, OUT̄ †, INH OUT H, "9" †
    - STR 1: OUT L, OUT̄ H, INH OUT 1, "9" 1
    - CAS 1: OUT H, OUT̄ *, INH OUT 1, "9" 1
    - 1XXX, CLR 1: OUT 10, OUT̄ 10, INH OUT H, "9" L
    - 0XXX, CLR 1: OUT L, OUT̄ H, INH OUT H, "9" L
    - XXXX, SET 1: OUT L, OUT̄ H, INH OUT L, "9" H
  - **Fig.14 output patterns per preset number**: 1 through 9 output pulses per 10 clocks, combining the R patterns above.
  - **Cascading** (text p.1):
    - Add mode: Output Rate = Clock Rate × [0.1 BCD1 + 0.01 BCD2 + 0.001 BCD3 + …]. Fig.12 shows 9/10 + 4/100 = 94/100.
    - Multiply mode (Fig.15): 9/10 × 4/10 = 36/100.
  - **Priority:** SET, CLEAR (async; both high is invalid) > INHIBIT IN (sync) > CLOCK rising edge.
- **Timing** (p.3; VDD = 5 V, CL = 50 pF, typ/max ns):

  | Path | typ | max |
  |---|---|---|
  | Clock→OUT̄ | 110 | 220 |
  | Clock or Strobe→OUT | 150 | 300 |
  | Clock→Inhibit Out H→L | 320 | 640 |
  | Clock→Inhibit Out L→H | 250 | 500 |
  | Clear→Out | 380 | 760 |
  | Clock→"9" Out | 300 | 600 |
  | Cascade→Out | 90 | 180 |
  | Inhibit In→Inhibit Out | 130 | 260 |
  | Set→Out | 330 | 660 |

  Also: fmax 2.4 MHz typ.
- **VDD:** 3–18 V.
- **Obsolete:** CD4527BPW.
- **Notes:** the Fig.13 gate-level wiring is dense and scanned. The R-term equations above are derived from the Fig.14 waveforms, which are clear, and they match the truth table.

## 4522B

- **Title:** "CD4522B Types — CMOS Programmable BCD Divide-by-'N' Counter, High-Voltage Types (20-Volt Rating)".
  - Stamped "NOT RECOMMENDED FOR NEW DESIGNS" and "Advance Information/Preliminary Data" (cd4522b.pdf p.1).
  - The census lists every row as Active.
- **Datasheets:** datasheets/divider/cd4522b.pdf. All of pp.1–7 are scanned. Sources used:
  - p.1: text, functional diagram
  - p.2: truth tables, Fig.1 logic diagram
  - p.3: recommended conditions
  - p.5: dynamic characteristics, terminal assignment
  - p.6: Figs.10/11 cascading tables
- **Families & prefixes:** C: CD4522B* (CD4522BE, BM, BMT, BPW).
- **Military:** none.
- **Pinouts:**
  - p16: N, D, PW (terminal assignment, p.5):
    ```
    1 Q3 out; 2 P3 in; 3 PE in; 4 CI in (CLOCK INHIBIT); 5 P0 in; 6 CL in (CLOCK);
    7 Q0 out; 8 VSS; 9 Q1 out; 10 MR in (MASTER RESET); 11 P1 in;
    12 "0" out (suggested id: ZERO); 13 CF in (CASCADE FEEDBACK); 14 P2 in; 15 Q2 out; 16 VDD
    ```
- **Behavior.**
  - **Text p.1:** "programmable BCD counter has a decoded '0' state output for divide-by-N applications. In single stage operation the '0' output is tied to the Preset Enable input. The Cascade Feedback allows multiple stage divide-by-N operation without the need for external gating. A HIGH on the Clock Inhibit disables the pulse-counting function. A HIGH on the Master Reset asynchronously resets the divide-by-N operation." Features: "Internally synchronous", "Logic edge-clocked design — increments on positive Clock transition or on negative Clock Inhibit transition".
  - **Truth table (p.2), copied:**

    | CLOCK | CLOCK INHIBIT | PRESET ENABLE | MASTER RESET | ACTION |
    |---|---|---|---|---|
    | 0 | 0 | 0 | 0 | No Count |
    | ↑ | 0 | 0 | 0 | Count Down |
    | X | 1 | 0 | 0 | No Count |
    | 1 | ↓ | 0 | 0 | Count Down |
    | X | X | 1 | 0 | Preset |
    | X | X | X | 1 | Reset |

    Count/output table: count 0–9 = BCD on Q3 Q2 Q1 Q0 (0 = 0000 … 9 = 1001, Q0 = LSB).
  - **Internal clock:** from Fig.1, CK = NOR(CI, ¬CL) = **CL·¬CI**. The counter decrements on the **rising edge of CK**, i.e. CL rising while CI = L, or CI falling while CL = H.
  - **MR:** async, active-high, highest priority; it clears to 0000. The flip-flops have an MR input.
  - **PE:** async (level), active-high; it loads Qn = Pn. The flip-flops have P and PE inputs. Setup 0 ns, hold 75 ns at 5 V.
  - **Count direction:** BCD down, …2→1→0→9 (decade). Q3 rises on the 0→9 step. Fig.10 cascading uses "Q3" of stage N → CL of stage N+1.
  - **Illegal states 10–15:** not specified.
  - **Priority:** MR > PE > CK rising edge.
  - **"0" output.** Fig.1a (scan, p.2) shows a cross-coupled NAND set/reset latch driving pin 12 through an inverter. My reading of the gates (medium confidence):
    - **set** when the count-zero decode AND **CF = H** (gate inputs: CF plus Q-decode lines);
    - **reset** when (PE = H and CK low, i.e. clock low or inhibited), or when (count ≠ 0 and PE = L).

    Functionally, with "0" tied to PE (single-stage ÷N):
    - The count reaches 0 on a CK rising edge, so "0" goes high.
    - PE goes high and asynchronously reloads N, so the count is no longer 0. The latch holds "0" high.
    - When CL falls, the latch resets, "0" goes low and PE is released.
    - The next rising edge counts N→N−1.
    - So the output is a pulse about one clock-high-time wide, once every N clocks, and the zero state is never observed for a full period.
    - Without the latch the preset would kill its own pulse, which is why the latch exists. Treat the exact reset term as a reading of the scan.
    - CF must be H for "0" to assert; tie it to VDD in single-stage use.
  - **Cascading table (Figs.10/11, p.6), copied:**

    | From stage | From pin | To stage | To pin | Range of N |
    |---|---|---|---|---|
    | LSD | "0" | All | PE | LSD < N < MSD |
    | N | "0" | N−1 | CF | LSD + 1 < N < MSD |
    | N | "Q3" | N+1 | CL | LSD < N < MSD−1 |

    The MSD's CF is tied to VDD. So each "0" means "this digit is 0 and all more-significant digits are 0", and the LSD's "0" presets every stage.
- **Timing** (p.5; VDD = 5 V, CL = 50 pF, typ/max):
  - Clock→Q: **550 / 1100 ns**
  - Clock→"0": **420 / 710**
  - Clock inhibit→Q: **270 / 540**
  - Master reset→Q: **270 / 540**
  - Transition: 100 / 200
  - fmax: 3 MHz typ (1.5 min)
  - Clock pulse width: 125 / 250. PE width: 125 / 250. MR width: 175 / 350.
  - MR removal: 130 / 260
  - No PE→Q delay is printed.
- **VDD:** 3–18 V.
- **Obsolete:** none (but see the NRND stamp).
- **Notes:**
  - The application figures label Q01–Q04 and D0–D3 inconsistently with the pinout names (Q0–Q3, P0–P3).

## 74_97

- **Title:** "SN5497, SN7497 Synchronous 6-Bit Binary Rate Multipliers" (sn5497.pdf p.1, SDLS130, Dec 1972 – rev. Mar 1988).
- **Datasheets:** datasheets/divider/sn5497.pdf. pp.1–7 are images with a small text layer. Sources used:
  - p.1: description, pinout, logic symbol
  - p.2: function table
  - p.3: logic diagram
  - p.4: recommended conditions
  - p.5: switching, application
  - pp.6–7: waveforms
- **Families & prefixes:** TTL (standard): SN7497N, SN5497J, SNJ5497J.
- **Military:** SNJ5497J: all rows (single function).
- **Pinouts:**
  - p16: J, N (p.1):
    ```
    1  B1          input
    2  B4          input
    3  B5          input
    4  B0          input
    5  Z           output
    6  Y           output
    7  ENABLE OUT  (printed "EN out", overbar on EN) output  (ENOUT)
    8  GND         ground
    9  CLK         input
    10 STRB        input   (printed overbar: active low)
    11 EN in       input   (printed overbar: active low; ENIN)
    12 UNITY/CAS   input
    13 CLR         input
    14 B2          input
    15 B3          input
    16 VCC         power
    ```
- **Behavior.** Text p.1: "The counter is enabled when the clear, strobe, and enable inputs are low. With the counter enabled, the output frequency is equal to the input frequency multiplied by the rate input M and divided by 64, i.e. fout = M·fin/64, where M = F·2^5 + E·2^4 + D·2^3 + C·2^2 + B·2^1 + A·2^0" (A…F = B0…B5).
  - **Logic diagram (p.3), read gate by gate:**
    - **Counter:** six T flip-flops QA (LSB) … QF.
      - Each is clocked by the inverted CLK on a bubbled dynamic input, so it **counts on the CLK rising edge**. Fig.1 note: "total number of counts will be determined by the total number of positive-going clock transitions enabled".
      - Each has a G (toggle-enable) input:
        - G_A = ¬EN
        - G_B = ¬EN·QA
        - G_C = ¬EN·QA·QB
        - G_D = ¬EN·QA·QB·QC
        - G_E = ¬EN·QA…QD
        - G_F = ¬EN·QA…QE
      - So it is a synchronous binary up-counter, enabled by EN in = L.
      - The enable has setup/hold windows relative to both the rising edge and the previous falling edge (p.4), typical of pulse-triggered TTL flip-flops.
    - **CLR:** active-high, **asynchronous**, into the active-low CLEAR of all FFs. Count becomes 0.
    - **S = AND(¬CLK, ¬STRB)** (the gate has two bubbled inputs).
    - **Rate gates:**
      - F = B5·S·Q̄A
      - E = B4·S·QA·Q̄B
      - D = B3·S·QA·QB·Q̄C
      - C = B2·S·QA·QB·QC·Q̄D
      - B = B1·S·QA·QB·QC·QD·Q̄E
      - A = B0·S·QA·QB·QC·QD·QE·Q̄F
    - **Z** (pin 5) = NOR(A, B, C, D, E, F): a low pulse during CLK low for each selected state.
    - **Y** (pin 6) = OR(¬Z, ¬UNITY/CAS) = **NAND(Z, UNITY/CAS)**.
    - **ENABLE out** (pin 7) = NAND(¬EN, QA, QB, QC, QD, QE, QF): low when EN in = L and the count is 63.
  - **Selected states** (count s = QF…QA, pulse in the CLK-low half of state s):
    - B5: s even (32 states)
    - B4: s ≡ 1 mod 4 (16)
    - B3: s ≡ 3 mod 8 (8)
    - B2: s ≡ 7 mod 16 (4)
    - B1: s ≡ 15 mod 32 (2)
    - B0: s = 31 (1)
    - Count 63 never pulses.
  - **Function table (p.2), copied:**

    | CLEAR | ENABLE | STROBE | B5 B4 B3 B2 B1 B0 | No. of clock pulses | UNITY/CASCADE | Y | Z | ENABLE | Note |
    |---|---|---|---|---|---|---|---|---|---|
    | H | X | H | X X X X X X | X | H | L | H | H | B |
    | L | L | L | L L L L L L | 64 | H | L | H | 1 | C |
    | L | L | L | L L L L L H | 64 | H | 1 | 1 | 1 | C |
    | L | L | L | L L L L H L | 64 | H | 2 | 2 | 1 | C |
    | L | L | L | L L L H L L | 64 | H | 4 | 4 | 1 | C |
    | L | L | L | L L H L L L | 64 | H | 8 | 8 | 1 | C |
    | L | L | L | L H L L L L | 64 | H | 16 | 16 | 1 | C |
    | L | L | L | H L L L L L | 64 | H | 32 | 32 | 1 | C |
    | L | L | L | H H H H H H | 64 | H | 63 | 63 | 1 | C |
    | L | L | L | H H H H H H | 64 | L | H | 63 | 1 | D |
    | L | L | L | H L H L L L | 64 | H | 40 | 40 | 1 | E |

    Notes:
    - A: H = high, L = low, X = irrelevant; all remaining entries are numeric counts.
    - B: "This is a simplified illustration of the clear function. The states of clock and strobe can affect the logic level of Y and Z. A low unity/cascade will cause output Y to remain high."
    - C: "Each rate illustrated assumes a constant value at rate inputs; however, these illustrations in no way prohibit variable-rate inputs."
    - D: "Unity/cascade is used to inhibit output Y."
    - E: fout = M·fin/64 = (8+32)fin/64 = 40fin/64 = 0.625 fin.
  - **Text:**
    - "When the rate input is binary 0 (all rate inputs low), Z remains high."
    - Cascading: "the enable output is connected to the enable and strobe inputs of the next stage, the Z output of each stage is connected to the unity/cascade input of the other stage, and the sub-multiple frequency is taken from the Y output."
    - "The unity/cascade input, when connected to the clock input, may be utilized to pass the clock frequency (inverted) to the Y output when the rate input/decoding gates are inhibited by the strobe."
    - The p.5 application shows 18-bit cascading (Y outputs NORed, or Z→UNITY/CAS chaining).
  - **Priority:** CLR (async) > EN (sync count enable) > CLK rising edge. STRB only gates the outputs, and UNITY/CAS only gates Y.
  - **Waveform note (p.7):** with clock or strobe rising, Z goes high (tPLH) and Y goes low. This confirms that Z pulses low and Y pulses high while CLK is low.
- **Timing** (p.5; VCC = 5 V, CL = 15 pF, RL = 400 Ω, typ/max ns):

  | From→To | tPLH | tPHL |
  |---|---|---|
  | Enable→Enable | 13/20 | 14/21 |
  | Strobe→Z | 12/18 | 15/23 |
  | Clock→Y | 26/39 | 20/30 |
  | Clock→Z | 12/18 | 17/26 |
  | Rate→Z | 6/10 | 9/14 |
  | Unity/Cascade→Y | 9/14 | 6/10 |
  | Strobe→Y | 19/30 | 22/33 |
  | Clock→Enable | 19/30 | 22/33 |
  | Clear→Y | 24/36 (tPLH) | – |
  | Clear→Z | – | 15/23 (tPHL) |
  | Any rate input→Y | 15/23 | 15/23 |

  Also: fmax 32 MHz typ (25 min).
- **VCC:** SN5497 4.5–5.5 V; SN7497 4.75–5.25 V (p.4). Input absolute max 5.5 V.
- **Obsolete:** none.
- **Notes:**
  - The logic diagram labels pin 6 "Y OURPUT" (a typo).
  - The pinout prints overbars on EN in, EN out and STRB, and the logic symbol shows them active-low.
