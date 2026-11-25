---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g02",
  "title": "SN74AHCT1G02 single 2-input positive-NOR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74ahct1g02.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS341M",
    "date": "2024-02",
    "pages": 24
  },
  "devices": ["gate:1G02"],
  "parts": ["74AHCT1G02DBVRG4", "74AHCT1G02DBVTG4", "74AHCT1G02DCKRE4", "74AHCT1G02DCKRG4", "SN74AHCT1G02DBVR", "SN74AHCT1G02DBVT", "SN74AHCT1G02DCKR"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G02 single 2-input positive-NOR gate

## Description

This device contains a single 2-input NOR gate that performs the Boolean
function Y = !{A + B} positive logic. [[1::2]]

Source error: the Description (p.1) prints "Y = !{A × B} or Y = !{A + B}", with one bar over the whole of A × B; the function table (Table 7-1) gives Y = L when only one of A and B is H, so !{A × B} (a NAND) is kept out.

Operating range of 4.5 V to 5.5 V. eMax t~pd~ of 6.5 ns at 5 V. Low
power consumption, 10-µA max I~CC~. ± 8-mA output drive at 5 V. Inputs
are TTL-voltage compatible. Latch-up performance exceeds 250 mA per JESD 17. [[1::3]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground Pin
Y: Output Y
VCC: Power Pin
```
[[3::Table 4-1]]

## Functional description

Source error: the Overview (p.7) says "The SN74AHCT1G02 contains four independent 2-input AND Gates with Schmitt-trigger inputs", each performing Y = A × B, with output levels for supplies from 1.8 V; the title, the Description (p.1), the logic diagram and the function table (p.7) show a single NOR gate, and the recommended operating conditions (p.4) give V~CC~ from 4.5 V. Kept out.

Conversion note: the logic diagram (Figure 7-1) shows one 2-input NOR gate with inputs A (pin 1) and B (pin 2) and output Y (pin 4).

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes only
as shown in Figure 7-2. [[7::2]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[7::3]]

Conversion note: Figure 7-2 (electrical placement of clamping diodes for each input and output) draws the output clamps to V~CC~ (+I~OK~) and GND (-I~OK~) and the input clamp to GND (-I~IK~).

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output Y |
| A | B | ^ |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
```
[[7::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | (V~I~ < 0) | | -20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHCT1G02 | < | Unit |
| ^ | ^ | DBV 5 pins | DCK 5 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = -50 μA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | I~OH~ = -8 mA | ^ | 3.94 | | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 μA | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | μA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | μA |
| ΔI~CC~ [1] | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | 10 | PF [2] |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels rather than 0 V or V~CC~.
[2] Source error: the unit is printed PF; a capacitance is in pF, as in the Operating Characteristics table.
```
[[5::table #1]]

Conversion note: the source prints no heading over the second Min and Max columns of this table and of the switching characteristics table.

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 17 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 2.4 | 5.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.5 | 5.5 | 1 | 6.5 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 3.4 | 7.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.5 | 7.5 | 1 | 8.5 | ^ |
```
[[5::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[6::Figure 6-1]]

Conversion note: in Figure 6-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[6::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[6::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[8::1]]

Source error: the Application Information and Typical Application (§ 8.1 and § 8.2, pp.8 and 10) describe three 2-input AND gates combined into a 4-input AND gate that drives the !{RESET} pin of a motor controller, with a block diagram (Figure 8-1) and a timing diagram (Figure 8-2) of AND gates; the SN74AHCT1G02 is a single NOR gate (function table, p.7). Kept out.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section. [[8::5]]

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74AHCT1G02 plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings. [[8::6]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHCT1G02 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings. [[8::7]]

The SN74AHCT1G02 can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50 pF. [[9::1]]

The SN74AHCT1G02 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin. [[9::2]]

Total power consumption can be calculated using the information provided
in the CMOS Power Consumption and Cpd Calculation application note.
Thermal increase can be calculated using the information provided in the
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note. [[9::3]] [[9::4]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[9::5]]

Source conflict: this caution (p.9) says T~J(max)~ is listed in the Absolute Maximum Ratings, while that table (p.4) has no junction temperature row.

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[9::6]]

Source error: the source prints "V~IL(max)~ V~t-(min)~" and "V~IH(min)~ V~t+(max)~" in the first sentence; the SN74AHCT1G02 has no V~t-~ or V~t+~ in its tables (pp.4–5), so these two symbols are kept out.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHCT1G02 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[9::7]]

The SN74AHCT1G02 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability. [[9::8]]

Source error: the next two paragraphs of the source (p.9) say the SN74AHCT1G02 "has no input signal transition rate requirements because it has Schmitt-trigger inputs" and refer to a ΔV~T(min)~ in the Electrical Characteristics and to Typical Characteristics; the electrical characteristics table (p.5) has no ΔV~T~, the document has no Typical Characteristics, and the recommended operating conditions (p.4) limit the input transition rate. Kept out.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[9::13]]

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device. [[9::14]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[10::2]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[10::4]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74AHCT1G02 to one or more of the receiving devices. [[10::4]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω, so that the maximum output current from the
   Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in MΩ; much larger than the minimum
   calculated previously. [[10::4]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[10::4]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in the following
layout example. [[10::5]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used). Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient. [[10::6]]

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill, a bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
74AHCT1G02DBVRG4 | Reel of 3000 | -40 to 85 | B02G
74AHCT1G02DBVTG4 | | -40 to 85 | B02G | Obsolete
74AHCT1G02DCKRE4 | Reel of 3000 | -40 to 85 | BB3
74AHCT1G02DCKRG4 | Reel of 3000 | -40 to 85 | BB3
SN74AHCT1G02DBVR | Reel of 3000 | -40 to 85 | 38UH, 3C6F, B023, B02G, B02J, B02S
SN74AHCT1G02DBVT | | -40 to 85 | B023, B02G, B02J, B02S | Obsolete
SN74AHCT1G02DCKR | Reel of 3000 | -40 to 85 | 1QR, BB3, BBG, BBJ, BBS
```
[[14::table #1]]

## Conversion notes

- Source pages 1–11 converted. The ordering lines are from the package
  option addendum (pp.14–15). The support and revision history page
  (12), the mechanical information page (13) and the tape and reel and
  mechanical pages (16–24) are left out; the generated Package section
  replaces them.
- Dropped: applications list (the features are kept as sentences in the
  Description), table of contents (p.2), package information table; the
  logic diagrams (p.1 and Figure 7-1), the clamp diode figure
  (Figure 7-2) and the example layout (Figure 8-3) are described in
  text.
- Figure 6-1 (p.6) is a picture; note B of the figure belongs to the
  enable and disable waveforms, which are not drawn, and is left out.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column.
- From images: the overbars of Y = !{A + B} and Y = !{A × B} (p.1) and
  of !{RESET} (p.8); 0 V, 3 V, 1.5 V, 50% V~CC~, R~L~ = 1 kΩ and the
  drawing of Figure 6-1 (p.6); the pin numbers of Figure 7-1 and the
  diode placement of Figure 7-2 (p.7); the gates of Figures 8-1 (p.8)
  and 8-2 (p.10); the notes of Figure 8-3 (p.11).
- Source issues:
  - Source error: the Description (p.1) prints Y = !{A × B} (a NAND) as
    the first form of the function; kept out.
  - Source error: the Features (p.1) print "eMax t~pd~"; kept as
    printed.
  - Source error: the Overview (p.7) describes four AND gates with
    Schmitt-trigger inputs; kept out (Functional description).
  - Source error: the unit of C~i~ is printed PF (p.5); kept as printed
    (table note).
  - Source error: § 8.1, Figure 8-1 and Figure 8-2 describe an AND-gate
    application; kept out (Application information).
  - Source error: Input Considerations (p.9) prints V~t-(min)~ and
    V~t+(max)~ and two paragraphs on Schmitt-trigger inputs; kept out.
  - Source conflict: the caution on p.9 refers to T~J(max)~ listed in
    the Absolute Maximum Ratings, which have no such row (p.4).
  - Unconfirmed: Output Considerations (pp.9–10) speaks of two channels
    within the same device connected in parallel and of open-drain
    outputs; the SN74AHCT1G02 has one push-pull gate, and these two
    sentences are left out.
  - Unconfirmed: the Features (p.1) give a max t~pd~ of 6.5 ns at 5 V,
    which the switching characteristics (p.5) give for C~L~ = 15 pF;
    for C~L~ = 50 pF the table gives 8.5 ns.
  - Unconfirmed: the title on p.1 is printed twice, run together.
