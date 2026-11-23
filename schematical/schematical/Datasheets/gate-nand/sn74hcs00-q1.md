---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs00-q1",
  "title": "SN74HCS00-Q1 automotive quadruple 2-input NAND gates with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74hcs00-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS758F",
    "date": "2026-10",
    "pages": 33
  },
  "devices": ["gate:74_00"],
  "parts": ["SN74HCS00QBQARQ1", "SN74HCS00QDRQ1", "SN74HCS00QPWRQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74HCS00-Q1 automotive quadruple 2-input NAND gates with Schmitt-trigger inputs

## Description

This device contains four independent 2-input NAND Gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A ● B} in positive logic. [[1::2]]

Wide operating voltage range: 2V to 6V. Schmitt-trigger inputs allow
for slow or noisy input signals. [[1::5]]

## Pinout

```pinout pinout=p14
```

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. BQA Package only. [[3::1]]

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
[[3::Table 4-1]]

## Functional description

This device contains four independent 2-input NAND Gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A ● B} in positive logic. [[1::2]]

Conversion note: the functional block diagram (p.8), One of Four Channels, shows one NAND gate with a Schmitt-trigger symbol, inputs xA and xB and output xY.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device can create fast edges into light loads,
so routing and load conditions must be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. Limit the
output power of the device to avoid damage due to overcurrent. The
electrical and thermal limits defined in the Absolute Maximum Ratings
must be followed at all times. [[8::2]]

Unused push-pull CMOS outputs must be left disconnected. [[8::3]]

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture.
These inputs are high impedance and are typically modeled as a resistor
in parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I). [[8::4]]

The Schmitt-trigger input architecture provides hysteresis as defined
by ΔV~T~ in the Electrical Characteristics table, which makes this
device extremely tolerant to slow or noisy inputs. While the inputs can
be driven much slower than standard CMOS inputs, properly terminating
unused inputs is still recommended. Driving the inputs with slow
transitioning signals increases dynamic current consumption of the
device. For additional information regarding Schmitt-trigger inputs,
please see Understanding Schmitt Triggers. [[8::5]]

### Clamp diode structure

As shown in Figure 7-1, the inputs and outputs to this device have both
positive and negative clamping diodes. [[8::6]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings can be exceeded if the input and output clamp-current ratings
are observed. [[8::7]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 7-1]]

### Wettable flanks

This device includes wettable flanks for at least one package. See the
Features section on the front page of the datasheet where packages
include this feature. [[9::2]]

Conversion note: Figure 7-2 (p.9) is a simplified cutaway view of a wettable-flank QFN package and a standard QFN package after soldering.

Wettable flanks help improve side wetting after soldering, which makes
QFN packages easier to inspect with automatic optical inspection (AOI).
As shown in Figure 7-2, a wettable flank can be dimpled or step-cut to
provide additional surface area for solder adhesion which assists in
reliably creating a side fillet. See the mechanical drawing for
additional details. [[9::4]]

## Function tables

Table 7-1 lists the functional modes of the SN74HCS00-Q1. [[9::5]]

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[3::Table 4-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < –0.5V or V~I~ > V~CC~ + 0.5V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < –0.5V or V~O~ > V~CC~ + 0.5V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±70 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Verified by design.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] HBM ESD Classification Level 2 | ±4000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C6 | ±1500 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | 2 | 5 | 6 | V |
| V~I~ | Input voltage | 0 | | V~CC~ | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise and fall rate | | | Unlimited | ns/V |
| T~A~ | Ambient temperature | -40 | | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74HCS00-Q1 | < | < | < | Unit |
| ^ | ^ | DYY (SOT) 14 pins | PW (TSSOP) 14 pins | D (SOIC) 14 pins | BQA (WQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 236.5 | 151.7 | 133.6 | 109.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 143.2 | 79.4 | 89.0 | 111.0 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 146 | 94.7 | 89.5 | 77.9 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 29.5 | 25.2 | 45.5 | 20.2 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 145.6 | 94.1 | 89.1 | 77.8 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | 56.6 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25°C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive switching threshold | | < | 2V | 1.13 | 1.26 | 1.37 | V |
| ^ | ^ | ^ | ^ | 4.5V | 2.31 | 2.47 | 2.66 | V |
| ^ | ^ | ^ | ^ | 6V | 2.99 | 3.18 | 3.44 | V |
| V~T-~ | Negative switching threshold | | < | 2V | 0.55 | 0.62 | 0.71 | V |
| ^ | ^ | ^ | ^ | 4.5V | 1.21 | 1.36 | 1.5 | V |
| ^ | ^ | ^ | ^ | 6V | 1.56 | 1.79 | 1.99 | V |
| ΔV~T~ | V~T+~ - V~T-~ | | < | 2V | 0.54 | 0.64 | 0.71 | V |
| ^ | ^ | ^ | ^ | 4.5V | 0.94 | 1.11 | 1.27 | V |
| ^ | ^ | ^ | ^ | 6V | 1.20 | 1.38 | 1.57 | V |
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = -20µA | 2V to 6V | V~CC~ - 0.1 | V~CC~ - 0.002 | | V |
| ^ | ^ | ^ | I~OH~ = -6mA | 4.5V | 4.0 | 4.3 | | V |
| ^ | ^ | ^ | I~OH~ = -7.8mA | 6V | 5.4 | 5.75 | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20µA | 2V to 6V | | 0.002 | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4mA | 4.5V | | 0.18 | 0.30 | V |
| ^ | ^ | ^ | I~OL~ = 7.8mA | 6V | | 0.22 | 0.33 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ or 0 | < | 6V | | ±100 | ±1000 | nA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0, I~O~ = 0 | < | 6V | | 0.1 | 2 | µA |
| C~i~ | Input capacitance | | < | 2V to 6V | | | 5 | pF |
| C~pd~ | Power dissipation capacitance per gate | No load | < | 2V to 6V | | 10 | | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50pF; over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). See Parameter Measurement Information"
| Symbol | Parameter | From (input) | To (output) | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | 2V | | 15 | 36 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 7 | 13 | ns |
| ^ | ^ | ^ | ^ | 6V | | 5 | 12 | ns |
| t~t~ | Transition-time | | Y | 2V | | 9 | 16 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 5 | 9 | ns |
| ^ | ^ | ^ | ^ | 6V | | 4 | 8 | ns |
```
[[6::table #1]]

## Typical characteristics

```curve id=rol caption="Output driver resistance in LOW state, T~A~ = 25°C"
{"x": {"label": "Output sink current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [26, 46]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 41.5], [2.5, 42.0], [5, 43.0], [7.5, 44.4], [10, 46]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 31.4], [5, 31.6], [10, 31.9], [15, 32.3], [20, 32.8], [25, 33.5]]},
  {"label": "VCC = 4.5 V", "points": [[0.5, 28.5], [10, 28.6], [20, 28.9], [25, 29.1]]},
  {"label": "VCC = 6 V", "points": [[0.5, 27.0], [10, 27.05], [20, 27.2], [25, 27.3]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=roh caption="Output driver resistance in HIGH state, T~A~ = 25°C"
{"x": {"label": "Output source current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [30, 70]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 54.2], [2.5, 55.5], [5, 58.0], [7.5, 61.3], [10, 66.0]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 39.0], [5, 39.6], [10, 40.4], [15, 41.3], [20, 42.5], [25, 43.9]]},
  {"label": "VCC = 4.5 V", "points": [[0.5, 34.2], [10, 34.7], [20, 35.4], [25, 35.8]]},
  {"label": "VCC = 6 V", "points": [[0.5, 31.2], [10, 31.5], [20, 31.8], [25, 32.0]]}
 ]}
```
[[6::Figure 5-2]]

Conversion note: Figures 5-3 and 5-4 (p.6) plot the supply current across the input voltage for supply voltages of 2 V, 2.5 V and 3.3 V and of 4.5 V, 5 V and 6 V, as hysteresis loops that are not digitized here.

## Parameter measurement information

Phase relationships between waveforms are selected arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1MHz, Z~O~ = 50Ω, t~t~ < 2.5ns. For clock
inputs, f~max~ is measured when the input duty cycle is 50%. The
outputs are measured one at a time with one input transition per
measurement. C~L~ includes probe and test-fixture capacitance. The
greater between t~PLH~ and t~PHL~ is the same as t~pd~. The greater
between t~r~ and t~f~ is the same as t~t~. [[7::1]] [[7::2]] [[7::3]] [[7::4]] [[7::6]] [[7::8]]

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and input and output transition times"
VM: 50%
```
[[7::Figure 6-3]]

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality. [[10::1]]

In this application, two 2-input NAND gates are used to create an
active-low SR latch as shown in Figure 8-1. The two additional gates
can be used for a second SR latch, or the inputs can be grounded and
both channels left unused. [[10::2]]

The SN74HCS00-Q1 is used to drive the tamper indicator LED and provide
one bit of data to the system controller. When the tamper switch
outputs LOW, the output Q becomes HIGH. This output remains HIGH until
the system controller addresses the event and sends a LOW signal to the
!{R} input which returns the Q output back to LOW. [[10::3]]

The inputs of this active-low SR latch can often be driven by
open-drain outputs which can produce slow input transition rates when
the inputs transition from LOW to Hi-Z. This makes the SN74HCS00-Q1
designed for the application because the device has Schmitt-trigger
inputs that do not have input transition rate requirements. [[10::4]]

Conversion note: Figure 8-1 (p.10), the typical application block diagram, shows two cross-coupled NAND gates with Schmitt-trigger inputs, the !{R} input driven by the System Controller with resistor R~1~ to the supply, the !{S} input driven by the Tamper Switch, and the output Q going to the System Controller and through resistor R~2~ to the Tamper Indicator LED.

### Power considerations

Verify that the desired supply voltage is within the range specified in
the Electrical Characteristics. The supply voltage sets the device
electrical characteristics, as described in the Electrical
Characteristics section. [[10::6]]

The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the SN74HCS00-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Verify that the maximum total current through
V~CC~ listed in the Absolute Maximum Ratings is not exceeded. [[10::7]]

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74HCS00-Q1 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into the ground connection.
Verify that the maximum total current through GND listed in the
Absolute Maximum Ratings is not exceeded. [[10::7]] [[10::8]]

The SN74HCS00-Q1 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the datasheet specifications.
Larger capacitive loads can be applied; however, TI recommends adding a
series resistor between the output and capacitor to prevent excessive
current. [[11::2]]

The SN74HCS00-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin. [[11::3]]

Total power consumption can be calculated using the information
provided in the CMOS Power Consumption and Cpd Calculation application
note. [[11::4]]

Thermal increase can be calculated using the information provided in
the Thermal Characteristics of Standard Linear and Logic (SLL) Packages
and Devices application note. [[11::5]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::6]]

### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[11::7]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
the inputs can be connected with a pullup or pulldown resistor if the
input is used sometimes, but not always. A pullup resistor is used for
a default state of HIGH, and a pulldown resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74HCS00-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors. [[11::8]]

The SN74HCS00-Q1 has no input signal transition rate requirements
because the device has Schmitt-Trigger inputs. [[11::9]]

Another benefit to having Schmitt-Trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, see the ΔV~T(min)~ in the
Electrical Characteristics. This hysteresis value provides the
peak-to-peak limit. [[11::10]]

Unlike what occurs with standard CMOS inputs, Schmitt-Trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[11::11]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output decreases the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output increases the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[11::13]]

Push-pull outputs that can be in opposite states, even for a very short
time period, must never be connected directly together. This can cause
excessive current and damage to the device. [[11::14]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[11::15]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[12::1]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the
   Layout section. [[12::3]]
2. Verify that the capacitive load at the output is ≤ 50pF. Low load
   capacitance can be accomplished by providing short, appropriately
   sized traces from the SN74HCS00-Q1 to the receiving device. [[12::3]]
3. Verify that the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Never violate the maximum output current from
   the Absolute Maximum Ratings. Most CMOS inputs have a resistive load
   measured in MΩ; much larger than the minimum calculated previously. [[12::3]]
4. Thermal issues are rarely a concern for logic gates; however, the
   power consumption and thermal increase can be calculated using the
   steps provided in the CMOS Power Consumption and Cpd Calculation
   application note. [[12::3]]

Conversion note: Figure 8-2 (p.12), the application timing diagram, shows the waveforms of R, S and Q.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal must have a good bypass capacitor to prevent power
disturbance. A 0.1μF capacitor is recommended for this device. Multiple
bypass capacitors can be placed in parallel to reject different
frequencies of noise. The 0.1μF and 1μF capacitors are commonly used in
parallel. The bypass capacitor must be installed as close to the power
terminal as possible for best results, as shown in the following layout
example. [[12::4]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4
buffer gates are used. Such unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a logic high or logic low voltage,
as defined by the input voltage specifications, to prevent them from
floating. The logic level that must be applied to any particular unused
input depends on the function of the device. Generally, the inputs are
tied to GND or V~CC~, whichever makes more sense for the logic function
or is more convenient. [[13::1]]

Conversion note: Figure 8-3 (p.13), the example layout for the SN74HCS00-Q1, shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, unused inputs tied to V~CC~, an unused output left floating, and a GND flood fill.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74HCS00QBQARQ1 | Reel of 3000 | -40 to 125 | HCS00Q
SN74HCS00QDRQ1 | Reel of 2500 | -40 to 125 | HCS00Q1
SN74HCS00QPWRQ1 | Reel of 2000 | -40 to 125 | HCS00Q
```
[[16::table #1]]

## Conversion notes

- Source pages 1–13 converted; the support and revision history pages
  (14–15), the package option addendum (p.16–17), tape and reel and
  mechanical pages (18–33) dropped, replaced by the generated Package
  and Ordering sections. The ordering lines are from the package option
  addendum.
- Dropped: features and applications lists, the package information
  table, the "Benefits of Schmitt-Trigger Inputs" figure (p.1), table
  of contents, the pin table's note on the TYPE letters, the functional
  block diagram, the wettable-flank cutaway view, the application block
  diagram, timing diagram and layout example (Figures 8-1, 8-2 and 8-3,
  described in text), and the two sentences that refer to the Feature
  Description for additional information regarding the inputs and
  outputs.
- The source also covers the DYY (SOT-23) package and its part
  SN74HCS00QDYYRQ1, which the catalog doesn't carry; its thermal column
  is kept.
- Figures 6-2 and 6-3 are shown by one shared waveform figure. The load
  circuit (Figure 6-1, p.7) prints no value for C~L~; the switching
  characteristics give C~L~ = 50pF.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale. The supply current curves (Figures 5-3 and 5-4) are not
  digitized.
- From images: overbars of Y = !{A ● B} (p.1, p.8).
- From images: overbars of !{R} and !{S} (text and Figure 8-1, p.10).
- From images: the curve points of Figures 5-1 and 5-2 (p.6): 41.5,
  42.0, 43.0, 44.4, 31.4, 31.6, 31.9, 32.3, 32.8, 33.5, 28.5, 28.6,
  28.9, 29.1, 27.0, 27.05, 27.2, 27.3, 54.2, 55.5, 58.0, 61.3, 66.0,
  39.0, 39.6, 40.4, 41.3, 42.5, 43.9, 34.2, 34.7, 35.4, 35.8, 31.2,
  31.5, 31.8, 32.0.
