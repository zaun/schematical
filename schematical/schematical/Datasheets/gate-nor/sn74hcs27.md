---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs27",
  "title": "SN74HCS27 triple 3-input NOR gates with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74hcs27.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS794B",
    "date": "2026-10",
    "pages": 25
  },
  "devices": ["gate:74_27"],
  "parts": ["SN74HCS27DR", "SN74HCS27PWR"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74HCS27 triple 3-input NOR gates with Schmitt-trigger inputs

## Description

This device contains three independent 3-input NOR Gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A + B + C} in positive logic. [[1::2]]

Wide operating voltage range: 2V to 6V. Schmitt-trigger inputs allow
for slow or noisy input signals. [[1::5]]

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
channels, a NOR gate with inputs xA, xB and xC and output xY.

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The drive capability of this device may create fast edges
into light loads so routing and load conditions must be considered to
prevent ringing. Additionally, the outputs of this device are capable
of driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to over-current. The electrical and thermal
limits defined in the Absolute Maximum Ratings must be followed at all
times. [[8::2]]

### CMOS Schmitt-trigger inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance given in the Electrical
Characteristics. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using Ohm's law (R = V ÷ I). [[8::3]]

The Schmitt-trigger input architecture provides hysteresis as defined
by ΔV~T~ in the Electrical Characteristics, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended
to properly terminate unused inputs. Driving the inputs slowly
increases dynamic current consumption of the device. [[8::4]]

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes. [[8::5]]

Voltages beyond the values specified in the Absolute MAximum Ratings
table can cause damage to the device. The input negative-voltage and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed. [[8::6]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 7-1]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | X | X | L |
| X | H | X | L |
| X | X | H | L |
| L | L | L | H |
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 or V~I~ > V~CC~ + 0.5 | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 or V~O~ > V~CC~ + 0.5 | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings can be exceeded if the input and output current ratings are observed. Do not exceed the absolute maximum voltage supply rating.
[3] Verified by design.
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
| V~CC~ | Supply voltage | 2 | | 6 | V |
| V~I~ | Input voltage | 0 | | V~CC~ | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise and fall rate | | | Unlimited | ns/V |
| T~A~ | Ambient temperature | -40 | | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information (SN74HCS27)"
| Symbol | Thermal metric [1] | PW (TSSOP), 14 pins | Unit |
|---|---|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 151.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 79.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 25.2 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.1 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | °C/W |

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

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25°C (unless otherwise noted). See the Parameter Measurement Information"
| Symbol | Parameter | From (input) | To (output) | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B or C | Y | 2V | | 15 | 32 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 7 | 15 | ns |
| ^ | ^ | ^ | ^ | 6V | | 6 | 12 | ns |
| t~t~ | Transition-time | | Y | 2V | | 9 | 17 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 5 | 8 | ns |
| ^ | ^ | ^ | ^ | 6V | | 4 | 7 | ns |
```
[[5::table #2]]

## Typical characteristics

```curve id=rol caption="Output driver resistance in Low state, T~A~ = 25°C"
{"x": {"label": "Output sink current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [26, 46]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 41.4], [2.5, 42.0], [5, 43.0], [7.5, 44.2], [10, 46.0]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 31.4], [12.5, 32.0], [25, 33.5]]},
  {"label": "VCC = 4.5 V", "points": [[0.5, 28.4], [12.5, 28.6], [25, 29.1]]},
  {"label": "VCC = 6 V", "points": [[0.5, 27.0], [25, 27.3]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=roh caption="Output driver resistance in High state, T~A~ = 25°C"
{"x": {"label": "Output source current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [30, 70]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 54.1], [2.5, 55.4], [5, 57.7], [7.5, 60.8], [10, 66.0]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 38.9], [7.5, 39.9], [12.5, 40.6], [20, 42.4], [25, 43.8]]},
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

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50pF
```
[[7::Figure 6-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, transition times and propagation delays"
VM: 50%
```
[[7::Figure 6-2]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality. [[10::1]]

In this application, two 3-input NOR gates are used to create an SR
latch as shown in Figure 8-1. The additional gate can be used for other
applications, or the inputs can be grounded and the channel left
unused. [[10::2]]

The SN74HCS27 is used to drive the tamper indicator LED and provide one
bit of data to the system controller. When either tamper switch outputs
HIGH, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a HIGH signal to any of
the R inputs which returns the Q output back to LOW. [[10::3]]

The user can add a small RC to the feedback path of the NOR gates to
default the output to a certain state, which can create slow transition
rates. This fact makes the SN74HCS27 an excellent choice for the
application because it has Schmitt-trigger inputs that do not have
input transition rate requirements. [[10::4]]

Conversion note: the typical application block diagram (Figure 8-1)
shows two cross-coupled NOR gates with inputs RA, RB and SA, SB,
Tamper Switch 1 and Tamper Switch 2 on the S inputs, the System
Controller on an R input and on the output Q, and Q driving the Tamper
Indicator through R2.

### Design requirements

- All signals in the system operate at 5V [[10::6]]
- Avoid unstable state by not having HIGH signals on both inputs [[10::6]]
- Q output is HIGH when any S input is HIGH [[10::6]]
- Q output remains High until any R input is HIGH [[10::3]]

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics. [[10::7]]

The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the SN74HCS27 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics. The
logic device can only source or sink as much current as it is provided
at the supply and ground pins, respectively. Be sure not to exceed the
maximum total current through GND or V~CC~ listed in the Absolute
Maximum Ratings. [[10::8]]

The SN74HCS27 can drive a load with a total capacitance less than or
equal to 50pF connected to a high-impedance CMOS input while still
meeting all of the datasheet specifications. Larger capacitive loads
can be applied, however it is not recommended to exceed 70pF. [[11::1]]

The maximum junction temperature, T~J~(max) listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::4]]

### Input considerations

Input signals must cross V~t-~(min) to be considered a logic LOW, and
V~t+~(max) to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Electrical Characteristics. [[11::5]]

Unused inputs must be terminated to either V~CC~ or ground. These can
be directly terminated if the input is completely unused, or they can
be connected with a pull-up or pull-down resistor if the input is to be
used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The resistor size is limited by drive current of the
controller, leakage current into the SN74HCS27, as specified in the
Electrical Characteristics, and the desired input transition rate. A
10kΩ resistor value is often used due to these factors. [[11::6]]

The SN74HCS27 has no input signal transition rate requirements because
it has Schmitt-trigger inputs. [[11::7]]

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T~(min) in the Electrical Characteristics. This hysteresis value
provides the peak-to-peak limit. [[11::8]]

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[11::9]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output decreases the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
Similarly, the ground voltage is used to produce the output LOW
voltage. Sinking current into the output increases the output voltage
as specified by the V~OL~ specification in the Electrical
Characteristics. [[11::11]]

Unused outputs can be left floating. [[11::12]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the
   LAyout. [[11::14]]
2. Ensure the capacitive load at the output is ≤ 70pF. This is not a
   hard limit, however it establishes peak performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74HCS27 to the receiving device. [[11::14]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / 25mA) Ω. This verifies that the maximum output current from
   the Absolute Maximum Ratings is not violated. Most CMOS inputs have
   a resistive load measured in megaohms; much larger than the minimum
   calculated above. [[11::14]]

Conversion note: the application timing diagram (Figure 8-2) shows Q
against the R and S signals.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal must have a good bypass capacitor to prevent power
disturbance. A 0.1μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1μF and 1μF capacitors are commonly used in
parallel. The bypass capacitor must be installed as close to the power
terminal as possible for best results, as shown in Figure 8-3. [[12::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs
must not ever be left floating. In many cases, functions or parts of
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
or is more convenient. [[12::3]]

Conversion note: the layout example (Figure 8-3) recommends a GND flood
fill, places the 0.1 µF bypass capacitor close to the device, ties
unused inputs to V~CC~, leaves the unused output floating, and avoids
90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCS27DR | Reel of 2500 | -40 to 125 | HCS27
SN74HCS27PWR | Reel of 2000 | -40 to 125 | HCS27
```
[[15::table #1]]

## Conversion notes

- Source pages 1–12 converted; the device and documentation support,
  revision history, package option addendum, tape and reel and
  mechanical pages (13–25) dropped, replaced by the generated Package
  and Ordering sections (packing, temperature range and marking taken
  from the package option addendum, p.15).
- Dropped: features and applications lists, the package information
  table, the "Benefits of Schmitt-trigger Inputs" figure, references to
  other documents. The functional block diagram, the typical
  application block diagram (Figure 8-1), the application timing
  diagram (Figure 8-2) and the layout example (Figure 8-3) are
  described in text.
- The pin drawing, the package information table and the thermal table
  cover the PW package only; the D package of SN74HCS27DR is in the
  package option addendum (p.15) and the mechanical pages.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
  The supply current curves (Figures 5-3 and 5-4) are not digitized.
- The source's Figures 6-2 and 6-3 (transition times; propagation
  delays) are shown as one shared figure.
- From images: overbars of Y = !{A + B + C} (p.1).
- From images: the curve points in Figures 5-1 and 5-2 (p.6).
- Source issues:
  - Source conflict: the features list (p.1) gives "±7.8mA output drive
    at 5V"; the electrical characteristics (p.5) specify
    I~OH~ = -7.8mA and I~OL~ = 7.8mA at 6V.
  - Source error: the note of Figure 6-3 (p.7) prints "TPHL" for
    t~PHL~, the clamp diode caution (p.8) prints "Absolute MAximum
    Ratings" and the detailed design procedure (p.11) prints "LAyout";
    all kept as printed.
  - Unconfirmed: the input considerations (p.11) send the reader to
    the Electrical Characteristics for the maximum input voltage
    range.
