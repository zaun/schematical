---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs10-q1",
  "title": "SN74HCS10-Q1 automotive triple 3-input NAND gates with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74hcs10-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS764B",
    "date": "2026-10",
    "pages": 26
  },
  "devices": ["gate:74_10"],
  "parts": ["SN74HCS10QDRQ1", "SN74HCS10QPWRQ1"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74HCS10-Q1 automotive triple 3-input NAND gates with Schmitt-trigger inputs

## Description

This device contains three independent 3-input NAND Gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A ● B ● C} in positive logic. [[1::2]]

Wide operating voltage range: 2V to 6V. Schmitt-trigger inputs allow
for slow or noisy input signals. [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A: Channel 1–3, Input A
1B, 2B, 3B: Channel 1–3, Input B
1C, 2C, 3C: Channel 1–3, Input C
1Y, 2Y, 3Y: Channel 1–3, Output Y
GND: Ground
VCC: Positive Supply
```
[[3::Table 4-1]]

## Functional description

Conversion note: the functional block diagram shows one of three
channels, a NAND gate with Schmitt-trigger inputs xA, xB and xC and
output xY.

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
device. [[8::5]]

### Clamp diode structure

As shown in Figure 7-1, the inputs and outputs to this device have both
positive and negative clamping diodes. [[8::6]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings can be exceeded if the input and output clamp-current ratings
are observed. [[8::7]]

```figure ref=clamp-diodes caption="Figure 7-1. Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 7-1]]

### Positive clamp diodes

The SN74HCS10-Q1 includes inputs with positive clamp diodes. The
positive clamp diodes prevent the input voltage from reaching levels
dangerous to the device, holding the input voltage (V~A~) to
approximately V~CC~ + 0.5V as long as the input clamp current (I~IK~)
does not exceed the maximum limit provided in Absolute Maximum Ratings. [[9::2]]

The input can draw significant current when the input voltage (V~A~)
exceeds the supply voltage (V~S~) because the diode is forward biased
and must be externally limited to avoid damage to the device.
Additionally, the input clamp current, I~IK~, can be several orders of
magnitude larger than I~CC~, resulting in current returning to the
supply (the reverse of normal operation) which must be accounted for by
the system designer, as shown in Figure 7-2. [[9::3]]

A resistor (R) can be used to prevent excessive current on the input as
shown in Figure 7-2. The resistor value can be determined with
R ≥ (V~IN~ - V~A~)/I~IK(max)~, with V~A~ ⋍ V~S~ + 0.5V. The
system-level input voltage (V~IN~) can exceed the supply voltage (V~S~)
because the device input voltage (V~A~) is held at approximately
V~S~ + 0.5V by the clamp diode. [[9::4]]

Conversion note: Figure 7-2 shows the resistor R between V~IN~ and the
device input, the clamp current I~IK~ flowing through the positive
clamp diode to V~CC~ and back to the supply V~S~, and about 0mA into
the logic.

## Function tables

```table id=function caption="Function table"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | L |
| L | X | X | H |
| X | L | X | H |
| X | X | L | H |
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5V or V~I~ > V~CC~ + 0.5V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~I~ < -0.5V or V~I~ > V~CC~ + 0.5V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±70 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings can be exceeded if the input and output current ratings are observed.
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
| T~A~ | Ambient temperature | -40 | | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information (SN74HCS10-Q1)"
| Symbol | Thermal metric [1] | PW (TSSOP), 14 pins | D (SOIC), 14 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 151.7 | 133.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 79.4 | 89.0 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 89.5 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 25.2 | 45.5 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.1 | 89.1 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[4::table #4]]

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
```
[[5::table #1]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50pF; over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). See Parameter Measurement Information."
| Symbol | Parameter | From (input) | To (output) | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B or C | Y | 2V | | 14 | 40 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 6 | 17 | ns |
| ^ | ^ | ^ | ^ | 6V | | 5 | 16 | ns |
| t~t~ | Transition-time | | Y | 2V | | 9 | 16 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 5 | 9 | ns |
| ^ | ^ | ^ | ^ | 6V | | 4 | 8 | ns |
```
[[5::table #2]]

## Typical characteristics

T~A~ = 25°C [[5::1]]

```curve id=rol caption="Figure 5-1. Output driver resistance in Low state"
{"x": {"label": "Output sink current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [26, 46]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 41.5], [2.5, 42.0], [5, 43.0], [7.5, 44.2], [10, 46.0]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 31.4], [12.5, 32.0], [25, 33.5]]},
  {"label": "VCC = 4.5 V", "points": [[0.5, 28.5], [12.5, 28.7], [25, 29.1]]},
  {"label": "VCC = 6 V", "points": [[0.5, 27.0], [25, 27.3]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=roh caption="Figure 5-2. Output driver resistance in High state"
{"x": {"label": "Output source current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [30, 70]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 54.2], [2.5, 55.4], [5, 57.7], [7.5, 60.8], [10, 66.0]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 38.9], [7.5, 39.9], [12.5, 40.7], [20, 42.4], [25, 43.8]]},
  {"label": "VCC = 4.5 V", "points": [[0.5, 34.1], [12.5, 34.8], [25, 35.7]]},
  {"label": "VCC = 6 V", "points": [[0.5, 31.1], [12.5, 31.5], [25, 31.9]]}
 ]}
```
[[6::Figure 5-2]]

Conversion note: Figures 5-3 and 5-4 plot typical supply current versus
input voltage across common supply values (2V to 3.3V) and (4.5V to
6V).

## Parameter measurement information

Phase relationships between waveforms are chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1MHz, Z~O~ = 50Ω, t~t~ < 2.5ns. [[7::1]]

The outputs are measured one at a time, with one input transition per
measurement. [[7::1]]

C~L~= 50pF and includes probe and jig capacitance. The maximum between
t~PLH~ and T~PHL~ is used for t~pd~. [[7::2]] [[7::4]]

```figure ref=load-cmos-50pf caption="Figure 6-1. Load circuit"
CL: 50pF
```
[[7::Figure 6-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Figures 6-2 and 6-3. Voltage waveforms, transition times and propagation delays"
VM: 50%
```
[[7::Figure 6-2]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality. [[10::1]]

In this application, two 3-input NAND gates are used to create an
active-low SR latch as shown in Figure 8-1. The additional gate can be
used elsewhere in the system, or the inputs can be grounded and left
unused. [[10::2]]

The SN74HCS10-Q1 is used to drive the tamper indicator LED and provide
one bit of data to the system controller. When the tamper switch
outputs LOW, the output Q becomes HIGH. This output remains HIGH until
the system controller addresses the event and sends a LOW signal to the
R input which returns the Q output back to LOW. [[10::3]]

Conversion note: the typical application block diagram (Figure 8-1)
shows two cross-coupled NAND gates with inputs !{R~A~}, !{R~B~} and
!{S~A~}, !{S~B~}, Tamper Switch 1 and Tamper Switch 2 on the !{S}
inputs, the System Controller and the pullup resistor R~1~ on the !{R}
inputs, the output Q going to the System Controller, and Q driving the
Tamper Indicator through R~2~.

### Power considerations

Verify that the desired supply voltage is within the range specified in
the Electrical Characteristics. The supply voltage sets the device
electrical characteristics, as described in the Electrical
Characteristics section. [[10::5]]

The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the SN74HCS10-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Verify that the maximum total current through
V~CC~ listed in the Absolute Maximum Ratings is not exceeded. [[10::6]]

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74HCS10-Q1 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into the ground connection.
Verify that the maximum total current through GND listed in the
Absolute Maximum Ratings is not exceeded. [[10::7]]

The SN74HCS10-Q1 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the datasheet specifications.
Larger capacitive loads can be applied; however, TI recommends adding a
series resistor between the output and capacitor to prevent excessive
current. [[11::1]]

The SN74HCS10-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin. [[11::2]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::5]]

### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[11::6]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
the inputs can be connected with a pullup or pulldown resistor if the
input is used sometimes, but not always. A pullup resistor is used for
a default state of HIGH, and a pulldown resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74HCS10-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors. [[11::7]]

The SN74HCS10-Q1 has no input signal transition rate requirements
because the device has Schmitt-Trigger inputs. [[11::8]]

Another benefit to having Schmitt-Trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, see the ΔV~T(min)~ in the
Electrical Characteristics. This hysteresis value provides the
peak-to-peak limit. [[11::9]]

Unlike what occurs with standard CMOS inputs, Schmitt-Trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[11::10]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output decreases the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output increases the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[11::12]]

Push-pull outputs that can be in opposite states, even for a very short
time period, must never be connected directly together. This can cause
excessive current and damage to the device. [[11::13]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[11::14]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[11::15]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the
   Layout section. [[12::1]]
2. Verify that the capacitive load at the output is ≤ 50pF. Low load
   capacitance can be accomplished by providing short, appropriately
   sized traces from the SN74HCS10-Q1 to the receiving device. [[12::1]]
3. Verify that the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Never violate the maximum output current from
   the Absolute Maximum Ratings. Most CMOS inputs have a resistive load
   measured in MΩ; much larger than the minimum calculated previously. [[12::1]]
4. Thermal issues are rarely a concern for logic gates; however, the
   power consumption and thermal increase can be calculated using the
   steps provided in the CMOS Power Consumption and Cpd Calculation
   application note. [[12::1]]

Conversion note: the application timing diagram (Figure 8-2) shows Q
against the !{R} and !{S} signals.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal must have a good bypass capacitor to prevent power
disturbance. [[12::2]]

A 0.1μF capacitor is recommended for this device. A design can use
multiple bypass capacitors in parallel to reject different frequencies
of noise. The 0.1μF and 1μF capacitors are commonly used in parallel.
The bypass capacitor must be installed as close to the power terminal
as possible for best results. [[12::3]]

### Layout guidelines

- Bypass capacitor placement [[13::4]]
  - Place near the positive supply terminal of the device [[12::4]]
  - Provide an electrically short ground return path [[12::4]]
  - Use wide traces to minimize impedance [[12::4]]
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible [[12::4]]
- Signal trace geometry [[12::4]]
  - 8mil to 12mil trace width [[12::4]]
  - Lengths less than 12cm to minimize transmission line effects [[12::4]]
  - Avoid 90° corners for signal traces [[12::4]]
  - Use an unbroken ground plane below signal traces [[12::4]]
  - Flood fill areas around signal traces with ground [[12::4]]
  - Parallel traces must be separated by at least 3x dielectric
    thickness [[12::4]]
  - For traces longer than 12cm [[12::4]]
    - Use impedance controlled traces [[12::4]]
    - Source-terminate using a series damping resistor near the output [[12::4]]
    - Avoid branches; buffer each signal that must branch separately [[12::4]]

Conversion note: the layout example drawings (Figures 8-3 to 8-7) show
trace corners for improved signal integrity, bypass capacitor placement
for TSSOP, WQFN and SOT packages, and damping resistor placement, and
are not drawn here.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCS10QDRQ1 | Reel of 2500 | -40 to 125 | HCS10Q1
SN74HCS10QPWRQ1 | Reel of 2000 | -40 to 125 | HCS10Q1
```
[[16::table #1]]

## Conversion notes

- Source pages 1–13 converted; the device and documentation support,
  revision history, package option addendum, tape and reel and
  mechanical pages (14–26) dropped, replaced by the generated Package
  and Ordering sections (packing, temperature range and marking taken
  from the package option addendum, p.16).
- Dropped: features and applications lists, the package information
  table, the "Benefits of Schmitt-trigger Inputs" figure, the overview
  (it repeats the description), references to other documents and to
  the Feature Description section. The functional block diagram, the
  example with a resistor to limit input current (Figure 7-2), the
  typical application block diagram (Figure 8-1), the application
  timing diagram (Figure 8-2) and the layout examples (Figures 8-3 to
  8-7) are described in text.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
  The supply current curves (Figures 5-3 and 5-4) are not digitized.
- The source's Figures 6-2 and 6-3 (transition times; propagation
  delays) are shown as one shared figure.
- From images: overbars of Y = !{A ● B ● C} (p.1).
- From images: overbars of !{R~A~}, !{R~B~}, !{S~A~}, !{S~B~}
  (Figure 8-1, p.10) and of !{R} and !{S} (Figure 8-2, p.12).
- From images: the curve points in Figures 5-1 and 5-2 (p.6).
- Source issues:
  - Source conflict: the features list (p.1) gives "±7.8mA output drive
    at 5V"; the electrical characteristics (p.5) specify
    I~OH~ = -7.8mA and I~OL~ = 7.8mA at 6V.
  - Unconfirmed: the absolute maximum ratings (p.4) print the condition
    of the output clamp current I~OK~ with V~I~, the same as for the
    input clamp current.
  - Source error: the note of Figure 6-3 (p.7) prints "TPHL" for
    t~PHL~; kept as printed.
  - Source error: the device functional modes sentence (p.9) names the
    device without its automotive suffix; kept out.
