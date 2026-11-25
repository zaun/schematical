---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74hct00",
  "title": "CDx4HCT00 quadruple 2-input NAND gates with TTL-compatible inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/cd74hct00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS398",
    "date": "2021-01",
    "pages": 25
  },
  "devices": ["gate:74_00"],
  "parts": ["5962-8683101CA", "CD54HCT00F", "CD54HCT00F3A", "CD74HCT00E", "CD74HCT00M", "CD74HCT00M96", "CD74HCT00M96G4", "CD74HCT00MT"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-21"
}
---

# CDx4HCT00 quadruple 2-input NAND gates with TTL-compatible inputs

## Description

This device contains four independent 2-input NAND gates. Each gate
performs the Boolean function Y = !{A ● B} in positive logic.
LSTTL input logic compatible: V~IL(max)~ = 0.8 V, V~IH(min)~ = 2 V.
CMOS input logic compatible: I~I~ ≤ 1 µA at V~OL~, V~OH~. Buffered
inputs. 4.5 V to 5.5 V operation. Wide operating temperature range:
–55°C to +125°C. Supports fanout up to 10 LSTTL loads. Significant
power reduction compared to LSTTL logic ICs. [[1::2]] [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2B: Channel 2, Input B
2Y: Channel 2, Output Y
GND: Ground
3Y: Channel 3, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
4Y: Channel 4, Output Y
4A: Channel 4, Input A
4B: Channel 4, Input B
VCC: Positive Supply
```
[[1::table #1]]

## Functional description

This device contains four independent 2-input NAND gates. Each gate
performs the Boolean function Y = !{A ● B} in positive logic. [[8::1]]

Conversion note: the logic diagram (Figure 8-1, p.8) shows one 2-input NAND gate with a buffer on each of the inputs xA and xB and a buffer on the output xY.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term
"balanced" indicates that the device can sink and source similar
currents. The drive capability of this device may create fast edges into
light loads so routing and load conditions should be considered to
prevent ringing. Additionally, the outputs of this device are capable of
driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to overcurrent. The electrical and thermal
limits defined in the Absolute Maximum Ratings must be followed at all
times. [[8::3]]

Unused push-pull CMOS outputs should be left disconnected. [[8::4]]

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold. [[8::5]]

TTL-compatible CMOS inputs are high impedance and are typically modeled
as a resistor in parallel with the input capacitance given in the
Electrical Characteristics. The worst case resistance is calculated with
the maximum input voltage, given in the Absolute Maximum Ratings, and
the maximum input leakage current, given in the Electrical
Characteristics, using Ohm's law (R = V ÷ I). [[8::6]]

TTL-compatible CMOS inputs require that input signals transition between
valid logic states quickly, as defined by the input transition time or
rate in the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations. More details can be found in Implications of Slow or
Floating CMOS Inputs. [[8::7]]

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a system
will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors, however
a 10-kΩ resistor is recommended and will typically meet all
requirements. [[8::8]]

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes as depicted in Electrical Placement of Clamping Diodes
for Each Input and Output. [[8::9]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[8::10]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 8-2]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[9::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 V or V~I~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ > -0.5 V or V~O~ < V~CC~ + 0.5 V [4] | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature [3] | Plastic package | | 150 | °C |
| ^ | ^ | Hermetic package or die | | 175 | °C |
| | Maximum lead temperature (soldering 10s) | SOIC - lead tips only | | 300 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Rating may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Condition. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Guaranteed by design.
[4] Unconfirmed: the source gives this condition as printed, "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V", which every output voltage meets.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| CD74HCT00 in N (PDIP) package | < | < | < | < |
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±500 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±750 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 4.5 V to 5.5 V | 2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 4.5 V to 5.5 V | | | 0.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | V |
| t~t~ | Input transition time | V~CC~ = 4.5 V | | | 500 | ns |
| ^ | ^ | V~CC~ = 5.5 V | | | 400 | ns |
| T~A~ | Operating free-air temperature | | -55 | | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information (CD74HCT00)"
| Symbol | Thermal metric [1] | N (PDIP) 14 pins | D (SOIC) 14 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 62.8 | 91.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 50.6 | 50.3 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 42.6 | 47.2 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 30.2 | 15.2 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 42.3 | 46.9 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). Operating free-air temperature (T~A~)"
| Symbol | Parameter | Test conditions | < | V~CC~ | 25°C | < | < | –40°C to 85°C | < | < | –55°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | < | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = -20 µA | 4.5 V | 4.4 | | | 4.4 | | | 4.4 | | | V |
| ^ | ^ | ^ | I~OH~ = -4 mA | 4.5 V | 3.98 | | | 3.84 | | | 3.7 | | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 4.5 V | | | 0.1 | | | 0.1 | | | 0.1 | V |
| V~OL~ | Low-level output voltage | ^ | I~OL~ = 4 mA | 4.5 V | | | 0.26 | | | 0.33 | | | 0.4 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ and GND | I~O~ = 0 | 5.5 V | | | ±0.1 | | | ±1 | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND | I~O~ = 0 | 5.5 V | | | 2 | | | 20 | | | 40 | µA |
| ΔI~CC~ | Additional Quiescent Device Current | One A input at V~CC~ – 2.1 | Other inputs at 0V or V~CC~ | 4.5 V to 5.5 V | | 100 | 648 | | | 810 | | | 882 | µA |
| ^ | ^ | One B input at V~CC~ – 2.1 | Other inputs at 0V or V~CC~ | 4.5 V to 5.5 V | | 100 | 396 | | | 495 | | | 539 | µA |
| C~i~ | Input capacitance | | < | | | | 10 | | | 10 | | | 10 | pF |
```
[[5::table #2]]

```table id=op caption="Operating characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 4.5 V to 5.5 V | | 25 | | pF |
```
[[6::table #1]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). Operating free-air temperature (T~A~)"
| Symbol | Parameter | From | To | Test conditions | V~CC~ | 25°C | < | < | –40°C to 85°C | < | < | –55°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | C~L~ = 50 pF | 4.5 V | | | 20 | | | 25 | | | 30 | ns |
| ^ | ^ | A or B | Y | C~L~ = 15 pF | 5 V | | 8 | | | | | | | | ns |
| t~t~ | Transition-time | | Y | C~L~ = 50 pF | 4.5 V | | | 15 | | | 19 | | | 22 | ns |
```
[[5::table #3]]

## Typical characteristics

```curve id=voh caption="Typical output voltage in the high state (V~OH~), T~A~ = 25°C"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 7]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0, 1.98], [1, 1.85], [1.5, 1.69]]},
  {"label": "VCC = 4.5 V", "points": [[0, 4.47], [4, 4.3]]},
  {"label": "VCC = 6 V", "points": [[0, 5.98], [5.5, 5.8]]}
 ]}
```
[[6::Figure 6-1]]

```curve id=vol caption="Typical output voltage in the low state (V~OL~), T~A~ = 25°C"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.3]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0, 0], [1.5, 0.096]]},
  {"label": "VCC = 4.5 V", "points": [[0, 0], [4, 0.114]]},
  {"label": "VCC = 6 V", "points": [[0, 0], [5.5, 0.127]]}
 ]}
```
[[6::Figure 6-2]]

Unconfirmed: Figures 6-1 and 6-2 plot 2-V and 6-V curves, outside the 4.5 V to 5.5 V supply voltage of the recommended operating conditions.

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns. For clock inputs, f~max~ is
measured when the input duty cycle is 50%. The outputs are measured one
at a time with one input transition per measurement. C~L~ includes probe
and test-fixture capacitance. The greater between t~PLH~ and t~PHL~ is
the same as t~pd~. The greater between t~r~ and t~f~ is the same as
t~t~. [[7::1]] [[7::2]] [[7::3]] [[7::4]] [[7::6]] [[7::8]]

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```
[[7::Figure 7-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and input and output transition times"
VM: 1.3 V (input), 50% (output)
```
[[7::Figure 7-3]]

Conversion note: Figures 7-2 and 7-3 (p.7) draw the input between 0V and 3V, with the delays measured from its 1.3V points and its transition times between 0.3V and 2.7V.

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

In this application, the CD74HCT00 is used to create an active-low SR
latch. The two additional gates can be used for a second active-low SR
latch, individually used for their logic function, or the inputs can be
grounded and both channels left unused. This device is used to drive the
tamper indicator LED and provide one bit of data to the system
controller. When the tamper switch outputs LOW, the output Q becomes
HIGH. This output remains HIGH until the system controller addresses the
event and sends a LOW signal to the R input which returns the Q output
back to LOW. [[10::2]]

Conversion note: the typical application diagram (Figure 9-1, p.10) shows two cross-coupled NAND gates with the System Controller on input !{R}, which a resistor R1 pulls up, the Tamper Switch on input !{S}, and output Q going to the System Controller and through R2 to the Tamper Indicator LED.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics. [[10::3]]

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the CD74HCT00 plus the
maximum static supply current, I~CC~, listed in Electrical
Characteristics and any transient current required for switching. The
logic device can only source as much current as is provided by the
positive supply source. Be sure not to exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings. [[10::4]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the CD74HCT00 plus the maximum supply
current, I~CC~, listed in Electrical Characteristics, and any transient
current required for switching. The logic device can only sink as much
current as can be sunk into its ground connection. Be sure not to exceed
the maximum total current through GND listed in the Absolute Maximum
Ratings. [[10::5]]

The CD74HCT00 can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the datasheet specifications.
Larger capacitive loads can be applied, however it is not recommended to
exceed 50 pF. [[10::6]]

The CD74HCT00 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the high state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin. [[10::7]] [[11::1]]

Total power consumption can be calculated using the information provided
in CMOS Power Consumption and Cpd Calculation. [[11::2]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices. [[11::3]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::4]]

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[11::5]]

Unused inputs must be terminated to either V~CC~ or ground. These can be
directly terminated if the input is completely unused, or they can be
connected with a pull-up or pull-down resistor if the input is to be
used sometimes, but not always. A pull-up resistor is used for a default
state of HIGH, and a pull-down resistor is used for a default state of
LOW. The resistor size is limited by drive current of the controller,
leakage current into the CD74HCT00, as specified in the Electrical
Characteristics, and the desired input transition rate. A 10-kΩ resistor
value is often used due to these factors. [[11::6]]

The CD74HCT00 has CMOS inputs and thus requires fast input transitions to
operate correctly, as defined in the Recommended Operating Conditions
table. Slow input transitions can cause oscillations, additional power
consumption, and reduction in device reliability. [[11::7]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[11::9]]

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device. [[11::10]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[11::11]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[11::12]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[11::14]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit, however it will ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   CD74HCT00 to the receiving device(s). [[11::14]]
3. Ensure the resistive load at the output is larger than (V~CC~ /
   I~O(max)~) Ω. This will ensure that the maximum output current from
   the Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in megaohms; much larger than the minimum
   calculated above. [[11::14]]
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application report, CMOS Power Consumption and
   Cpd Calculation. [[12::1]]

Conversion note: the application timing diagram (Figure 9-2, p.12) shows the !{R}, !{S} and Q waveforms of the latch.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in given example
layout image. [[13::1]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs must
not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used. Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient. [[13::2]]

Conversion note: the example layout for the CD74HCT00 (Figure 11-1, p.13) shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, unused inputs tied to V~CC~, an unused output left floating, and a GND flood fill.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
5962-8683101CA | Tube of 25 | -55 to 125 | 5962-8683101CA CD54HCT00F3A
CD54HCT00F | Tube of 25 | -55 to 125 | CD54HCT00F
CD54HCT00F3A | Tube of 25 | -55 to 125 | 5962-8683101CA CD54HCT00F3A
CD74HCT00E | Tube of 25 | -55 to 125 | CD74HCT00E
CD74HCT00M | | -55 to 125 | HCT00M | Obsolete
CD74HCT00M96 | Reel of 2500 | -55 to 125 | HCT00M
CD74HCT00M96G4 | Reel of 2500 | -55 to 125 | HCT00M
CD74HCT00MT | | -55 to 125 | HCT00M | Obsolete
```
[[16::table #1]]

## Conversion notes

- Source pages 1–13 converted; the support pages (14–15), the package
  option addendum (p.16–17), tape and reel, tube and mechanical pages
  (18–25) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are from the package option addendum.
- Dropped: features and applications lists (the feature lines are in
  the Description), the device information table, table of contents,
  revision history, the logic diagram, the typical application diagram,
  the application timing diagram and the layout example (Figures 8-1,
  9-1, 9-2 and 11-1, described in text), and the "Refer to the Feature
  Description section" sentences of 9.2.1.2 and 9.2.1.3.
- Curves digitized from Figures 6-1 and 6-2 (p.6) to about 2 % of full
  scale (straight lines in the source, the 2-V V~OH~ line slightly
  bent).
- Figures 7-2 and 7-3 (propagation delays, transition times) are shown
  by one shared waveform figure. The load circuit (Figure 7-1, p.7)
  prints no value for C~L~; the switching characteristics give C~L~.
- The device information table (p.1) names CD54HCT00J; the package
  option addendum lists the CDIP parts as CD54HCT00F and CD54HCT00F3A.
- From images: overbars of Y = !{A ● B} (p.1 and p.8), and of !{R} and
  !{S} (Figures 9-1 and 9-2, p.10 and p.12).
- From images: the curve points of Figures 6-1 and 6-2 (p.6): 1.98,
  1.85, 1.69, 4.47, 4.3, 5.98, 5.8, 0.096, 0.114, 0.127.
- Source issues:
  - Unconfirmed: I~O~ condition "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V" (absolute maximum ratings note 4).
  - Unconfirmed: the typical characteristics (p.6) plot 2-V and 6-V
    curves for a 4.5 V to 5.5 V device.
