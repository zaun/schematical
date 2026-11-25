---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs10",
  "title": "SN74HCS10 triple 3-input NAND gates with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74hcs10.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS789B",
    "date": "2026-10",
    "pages": 26
  },
  "devices": ["gate:74_10"],
  "parts": ["SN74HCS10DR", "SN74HCS10PWR"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74HCS10 triple 3-input NAND gates with Schmitt-trigger inputs

## Description

This device contains three independent 3-input NAND Gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A ● B ● C} in positive logic. [[8::1]]

Wide operating voltage range: 2V to 6V. Schmitt-trigger inputs allow
for slow or noisy input signals. Extended ambient temperature range:
–40°C to +125°C, T~A~. [[1::5]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
2A: Channel 2, Input A
2B: Channel 2, Input B
2C: Channel 2, Input C
2Y: Channel 2, Output Y
GND: Ground
3Y: Channel 3, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
3C: Channel 3, Input C
1Y: Channel 1, Output Y
1C: Channel 1, Input C
VCC: Positive Supply
```
[[3::Table 4-1]]

## Functional description

This device contains three independent 3-input NAND gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A•B•C} in positive logic. [[8::1]]

Conversion note: the functional block diagram (p.8), One of Three Channels, shows one NAND gate with Schmitt-trigger inputs xA, xB and xC and output xY.

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

### Positive clamp diodes

The SN74HCS10 includes inputs with positive clamp diodes. The positive
clamp diodes prevent the input voltage from reaching levels dangerous
to the device, holding the input voltage (V~A~) to approximately
V~CC~ + 0.5V as long as the input clamp current (I~IK~) does not exceed
the maximum limit provided in Absolute Maximum Ratings. [[9::2]]

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

Conversion note: Figure 7-2 (p.9), the example with resistor to limit input current, shows V~IN~ applied through the resistor R to the device input at V~A~, and the current I~IK~ flowing through the positive clamp diode to V~CC~ and back into the supply V~S~.

## Function tables

Table 7-1 lists the functional modes of the SN74HCS10. [[9::6]]

```table id=function caption="Function table"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | L |
| L | X | X | H |
| X | L | X | H |
| X | X | L | H |
```
[[3::Table 4-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < –0.5V or V~I~ > V~CC~ + 0.5V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~I~ < –0.5V or V~I~ > V~CC~ + 0.5V [4] | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±70 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings can be exceeded if the input and output current ratings are observed.
[3] Verified by design.
[4] Unconfirmed: the source prints the output clamp current's condition with V~I~, the same as the input clamp current's (p.4).
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±4000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1500 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | 2 | 5 | 6 | V |
| V~I~ | Input voltage | 0 | | V~CC~ | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| T~A~ | Ambient temperature [1] | -55 | | 125 | °C |

[1] Source conflict: this table gives –55 °C as the minimum ambient temperature (p.4); the features list (p.1) gives an extended ambient temperature range of –40°C to +125°C, and the package option addendum (p.16) lists every part at -40 to 125.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74HCS10 | < | Unit |
| ^ | ^ | PW (TSSOP) 14 pins | D (SOIC) 14 pins | ^ |
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

```curve id=rol caption="Output driver resistance in Low state, T~A~ = 25°C"
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

```curve id=roh caption="Output driver resistance in High state, T~A~ = 25°C"
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

Conversion note: Figures 5-3 and 5-4 (p.6) plot the typical supply current versus input voltage for supply voltages of 2 V, 2.5 V and 3.3 V and of 4.5 V, 5 V and 6 V, as hysteresis loops that are not digitized here.

## Parameter measurement information

Phase relationships between waveforms are chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1MHz, Z~O~ = 50Ω, t~t~ < 2.5ns. The outputs are measured one at a
time, with one input transition per measurement. C~L~= 50pF and
includes probe and jig capacitance. The maximum between t~PLH~ and
T~PHL~ is used for t~pd~. [[7::1]] [[7::2]] [[7::4]]

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50pF
```
[[7::Figure 6-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, transition times and propagation delays"
VM: 50%
```
[[7::Figure 6-2]]

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality. [[10::1]]

In this application, two 3-input NAND gates are used to create an
active-low SR latch as shown in Figure 8-1. The additional gate can be
used elsewhere in the system, or the inputs can be grounded and left
unused. [[10::2]]

The SN74HCS10 is used to drive the tamper indicator LED and provide one
bit of data to the system controller. When the tamper switch outputs
LOW, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a LOW signal to the R
input which returns the Q output back to LOW. [[10::3]]

Conversion note: Figure 8-1 (p.10), the typical application block diagram, shows two cross-coupled 3-input NAND gates with Schmitt-trigger inputs, the inputs !{R~A~} and !{R~B~} driven by the System Controller with resistor R~1~ to the supply, the input !{S~A~} driven by Tamper Switch 1 and !{S~B~} by Tamper Switch 2, and the output Q going to the System Controller and through resistor R~2~ to the Tamper Indicator LED.

### Power considerations

Verify that the desired supply voltage is within the range specified in
the Electrical Characteristics. The supply voltage sets the device
electrical characteristics, as described in the Electrical
Characteristics section. [[10::5]]

The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the SN74HCS10 plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Verify that the maximum total current through
V~CC~ listed in the Absolute Maximum Ratings is not exceeded. [[10::6]]

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74HCS10 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into the ground connection.
Verify that the maximum total current through GND listed in the
Absolute Maximum Ratings is not exceeded. [[10::7]]

The SN74HCS10 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the datasheet specifications.
Larger capacitive loads can be applied; however, TI recommends adding a
series resistor between the output and capacitor to prevent excessive
current. [[10::8]]

The SN74HCS10 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin. [[11::1]]

Total power consumption can be calculated using the information
provided in the CMOS Power Consumption and Cpd Calculation application
note. [[11::2]]

Thermal increase can be calculated using the information provided in
the Thermal Characteristics of Standard Linear and Logic (SLL) Packages
and Devices application note. [[11::3]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::4]]

### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[11::5]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
the inputs can be connected with a pullup or pulldown resistor if the
input is used sometimes, but not always. A pullup resistor is used for
a default state of HIGH, and a pulldown resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74HCS10 (as specified in the Electrical Characteristics), and the
desired input transition rate limits the resistor size. A 10kΩ resistor
value is often used due to these factors. [[11::6]]

The SN74HCS10 has no input signal transition rate requirements because
the device has Schmitt-Trigger inputs. [[11::7]]

Another benefit to having Schmitt-Trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, see the ΔV~T(min)~ in the
Electrical Characteristics. This hysteresis value provides the
peak-to-peak limit. [[11::8]]

Unlike what occurs with standard CMOS inputs, Schmitt-Trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[11::9]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output decreases the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output increases the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[11::11]]

Push-pull outputs that can be in opposite states, even for a very short
time period, must never be connected directly together. This can cause
excessive current and damage to the device. [[11::12]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[11::13]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[11::14]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the
   Layout section. [[12::1]]
2. Verify that the capacitive load at the output is ≤ 50pF. Low load
   capacitance can be accomplished by providing short, appropriately
   sized traces from the SN74HCS10 to the receiving device. [[12::1]]
3. Verify that the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Never violate the maximum output current from
   the Absolute Maximum Ratings. Most CMOS inputs have a resistive load
   measured in MΩ; much larger than the minimum calculated previously. [[12::1]]
4. Thermal issues are rarely a concern for logic gates; however, the
   power consumption and thermal increase can be calculated using the
   steps provided in the CMOS Power Consumption and Cpd Calculation
   application note. [[12::1]]

Conversion note: Figure 8-2 (p.12), the application timing diagram, shows the waveforms of R, S and Q.

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

- Bypass capacitor placement [[13::3]]
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

Conversion note: the layout examples (p.13) are Figure 8-3, example trace corners for improved signal integrity, Figures 8-4 to 8-6, example bypass capacitor placement for TSSOP, WQFN, SOT, SC70 and similar packages, and Figure 8-7, example damping resistor placement for improved signal integrity.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCS10DR | Reel of 2500 | -40 to 125 | HCS10
SN74HCS10PWR | Reel of 2000 | -40 to 125 | HCS10
```
[[1::table #1]]

## Conversion notes

- Source pages 1–13 converted; the support and revision history pages
  (14–15), the package option addendum (p.16–17), tape and reel and
  mechanical pages (18–26) dropped, replaced by the generated Package
  and Ordering sections. The ordering lines are from the package option
  addendum.
- Dropped: features and applications lists, the package information
  table, the "Benefits of Schmitt-trigger Inputs" figure (p.1), table
  of contents, the pin table's note on the TYPE letters, the functional
  block diagram, the input current example (Figure 7-2), the
  application block diagram and timing diagram (Figures 8-1 and 8-2)
  and the layout examples (Figures 8-3 to 8-7), all described in text,
  and the two sentences that refer to the Feature Description for
  additional information regarding the inputs and outputs.
- The thermal information table runs over pages 4 and 5 in the source
  and is one table here.
- Figures 6-2 and 6-3 are shown by one shared waveform figure.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale. The supply current curves (Figures 5-3 and 5-4) are not
  digitized.
- From images: overbars of Y = !{A ● B ● C} (p.1) and Y = !{A•B•C}
  (p.8).
- From images: overbars of !{R~A~}, !{R~B~}, !{S~A~} and !{S~B~}
  (Figure 8-1, p.10).
- From images: the curve points of Figures 5-1 and 5-2 (p.6): 41.5,
  42.0, 43.0, 44.4, 31.4, 31.6, 31.9, 32.3, 32.8, 33.5, 28.5, 28.6,
  28.9, 29.1, 27.0, 27.05, 27.2, 27.3, 54.2, 55.5, 58.0, 61.3, 66.0,
  39.0, 39.6, 40.4, 41.3, 42.5, 43.9, 34.2, 34.7, 35.4, 35.8, 31.2,
  31.5, 31.8, 32.0.
- Source issues:
  - Source conflict: minimum ambient temperature –55 °C in the
    recommended operating conditions against –40°C in the features list
    and the package option addendum (ROC note 1).
  - Unconfirmed: the output clamp current's condition is printed with
    V~I~ (abs-max note 4).
  - Source error: the source prints "T~PHL~" for t~PHL~ in note A of
    Figure 6-3 (p.7); kept as printed.
