---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs30-q1",
  "title": "SN74HCS30-Q1 automotive single 8-input NAND gate with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74hcs30-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS765A",
    "date": "2026-10",
    "pages": 25
  },
  "devices": ["gate:74_30"],
  "parts": ["SN74HCS30QDRQ1", "SN74HCS30QPWRQ1"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74HCS30-Q1 automotive single 8-input NAND gate with Schmitt-trigger inputs

## Description

This device contains one independent 8-input NAND gate with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A ● B ● C ● D ● E ● F ● G ● H} in positive logic.

Wide operating voltage range: 2V to 6V. Schmitt-trigger inputs allow
for slow or noisy input signals.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
A: Input A
B: Input B
C: Input C
D: Input D
E: Input E
F: Input F
G: Input G
GND: Ground
H: Input H
NC: Not internally connected
VCC: Positive Supply
Y: Output Y
```

## Functional description

Conversion note: the functional block diagram shows one NAND gate with
inputs A, B, C, D, E, F, G and H and output xY.

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The drive capability of this device can create fast edges
into light loads so routing and load conditions must be considered to
prevent ringing. Additionally, the outputs of this device are capable
of driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to over-current. The electrical and thermal
limits defined in the Absolute Maximum Ratings must be followed at all
times.

### CMOS Schmitt-trigger inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance given in the Electrical
Characteristics. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using Ohm's law (R = V ÷ I).

The Schmitt-trigger input architecture provides hysteresis as defined
by ΔV~T~ in the Electrical Characteristics, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended
to properly terminate unused inputs. Driving the inputs slowly will
also increase dynamic current consumption of the device.

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes as depicted in Figure 7-1.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

### Device functional modes

Table 7-1 lists the functional modes of SN74HCS30-Q1.

## Function tables

```table id=function caption="Function table"
| Inputs | < | < | < | < | < | < | < | Output |
| A | B | C | D | E | F | G | H | Y |
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| H | H | H | H | H | H | H | H | L |
| L | X | X | X | X | X | X | X | H |
| X | L | X | X | X | X | X | X | H |
| X | X | L | X | X | X | X | X | H |
| X | X | X | L | X | X | X | X | H |
| X | X | X | X | L | X | X | X | H |
| X | X | X | X | X | L | X | X | H |
| X | X | X | X | X | X | L | X | H |
| X | X | X | X | X | X | X | L | H |
```

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

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] HBM ESD Classification Level 2 | ±4000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C6 | ±1500 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

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

## Thermal information

```table id=thermal caption="Thermal information (SN74HCS30-Q1)"
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

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25°C (unless otherwise noted). See the Parameter Measurement Information"
| Symbol | Parameter | From (input) | To (output) | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | 2V | | 22 | 46 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 9 | 28 | ns |
| ^ | ^ | ^ | ^ | 6V | | 8 | 27 | ns |
| t~t~ | Transition-time | | Y | 2V | | 9 | 17 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 5 | 8 | ns |
| ^ | ^ | ^ | ^ | 6V | | 4 | 7 | ns |
```

## Typical characteristics

```curve id=rol caption="Output driver resistance in Low state, T~A~ = 25°C"
{"x": {"label": "Output sink current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [26, 46]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 41.4], [2.5, 42.0], [5, 43.0], [7.5, 44.3], [10, 46.0]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 31.4], [12.5, 32.0], [25, 33.5]]},
  {"label": "VCC = 4.5 V", "points": [[0.5, 28.4], [12.5, 28.7], [25, 29.1]]},
  {"label": "VCC = 6 V", "points": [[0.5, 27.0], [25, 27.3]]}
 ]}
```

```curve id=roh caption="Output driver resistance in High state, T~A~ = 25°C"
{"x": {"label": "Output source current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [30, 70]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 54.2], [2.5, 55.5], [5, 57.8], [7.5, 61.0], [10, 66.0]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 38.9], [7.5, 39.9], [12.5, 40.7], [20, 42.4], [25, 43.8]]},
  {"label": "VCC = 4.5 V", "points": [[0.5, 34.1], [12.5, 34.8], [25, 35.7]]},
  {"label": "VCC = 6 V", "points": [[0.5, 31.1], [12.5, 31.5], [25, 31.9]]}
 ]}
```

Conversion note: Figures 5-3 and 5-4 plot typical supply current versus
input voltage across common supply values (2V to 3.3V) and (4.5V to
6V).

## Parameter measurement information

Phase relationships between waveforms are chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1MHz, Z~O~ = 50Ω, t~t~ < 2.5ns.

The outputs are measured one at a time, with one input transition per
measurement.

C~L~= 50pF and includes probe and jig capacitance. The maximum between
t~PLH~ and T~PHL~ is used for t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, transition times and propagation delays"
VM: 50%
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality.

In this application, two 8-input NAND Gates are used to create an
active-low SR latch as shown in Figure 8-1. The SN74HCS30-Q1 is used to
drive the tamper indicator LED and provide one bit of data to the
system controller. When the tamper switch outputs LOW, the output Q
becomes HIGH. This output remains HIGH until the system controller
addresses the event and sends a LOW signal to the R input which returns
the Q output back to LOW.

Conversion note: the typical application block diagram (Figure 8-1)
shows two cross-coupled NAND gates, Tamper Switch 1 to Tamper Switch 7
on the inputs SA to SG, the System Controller and the pull-up R1 on the
input R, and the output Q going to the System Controller and driving
the Tamper Indicator through R2.

### Design requirements

- All signals in the system operate at 5V
- Avoid unstable state by not having LOW signals on both R and S inputs
- Conditions for output:
  - Q output is HIGH when any S input is LOW
  - Q output remains High until any R input is LOW

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics.

The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the SN74HCS30-Q1 plus the
maximum supply current, I~CC~, listed in the Electrical
Characteristics. The logic device can only source or sink as much
current as it is provided at the supply and ground pins, respectively.
Be sure not to exceed the maximum total current through GND or V~CC~
listed in the Absolute Maximum Ratings.

The SN74HCS30-Q1 can drive a load with a total capacitance less than or
equal to 50pF connected to a high-impedance CMOS input while still
meeting all of the datasheet specifications. Larger capacitive loads
can be applied, however it is not recommended to exceed 70pF.

The maximum junction temperature, T~J~(max) listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

### Input considerations

Input signals must cross V~t-~(min) to be considered a logic LOW, and
V~t+~(max) to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. These can
be directly terminated if the input is completely unused, or they can
be connected with a pull-up or pull-down resistor if the input is to be
used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The resistor size is limited by drive current of the
controller, leakage current into the SN74HCS30-Q1, as specified in the
Electrical Characteristics, and the desired input transition rate. A
10kΩ resistor value is often used due to these factors.

The SN74HCS30-Q1 has no input signal transition rate requirements
because it has Schmitt-trigger inputs.

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T~(min) in the Electrical Characteristics. This hysteresis value
provides the peak-to-peak limit.

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output decreases the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
Similarly, the ground voltage is used to produce the output LOW
voltage. Sinking current into the output will increase the output
voltage as specified by the V~OL~ specification in the Electrical
Characteristics.

Unused outputs can be left floating.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the
   Layout.
2. Ensure the capacitive load at the output is ≤ 70pF. This is not a
   hard limit, however it will ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74HCS30-Q1 to the receiving device.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / 25mA) Ω. This verifies that the maximum output current from
   the Absolute Maximum Ratings is not violated. Most CMOS inputs have
   a resistive load measured in megaohms; much larger than the minimum
   calculated above.

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
terminal as possible for best results, as shown in Figure 8-3.

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
or is more convenient.

Conversion note: the layout example (Figure 8-3) recommends a GND flood
fill, places the 0.1 µF bypass capacitor close to the device, ties
unused inputs to V~CC~, leaves the NC pins floating, and avoids 90°
corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCS30QDRQ1 | Reel of 2500 | -40 to 125 | HCS30Q1
SN74HCS30QPWRQ1 | Reel of 2000 | -40 to 125 | HCS30Q1
```

## Conversion notes

- Source pages 1–12 converted; the device and documentation support,
  revision history, package option addendum, tape and reel and
  mechanical pages (13–25) dropped, replaced by the generated Package
  and Ordering sections (packing, temperature range and marking taken
  from the package option addendum, p.15).
- Dropped: features and applications lists, the package information
  table, the "Benefits of Schmitt-Trigger Inputs" figure, the table of
  contents (p.2), the vendor's pin drawing (p.3), references to other
  documents, step 4 of the detailed design procedure (a reference to an
  application note). The functional block diagram, the typical
  application block diagram (Figure 8-1), the application timing
  diagram (Figure 8-2) and the layout example (Figure 8-3) are
  described in text.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
  The supply current curves (Figures 5-3 and 5-4) are not digitized.
- The source's Figures 6-2 and 6-3 (transition times; propagation
  delays) are shown as one shared figure.
- From images: overbars of Y = !{A ● B ● C ● D ● E ● F ● G ● H} (p.1).
- From images: the curve points in Figures 5-1 and 5-2 (p.6).
- Source issues:
  - Source conflict: the features list (p.1) gives "±7.8mA output drive
    at 5V"; the electrical characteristics (p.5) specify
    I~OH~ = -7.8mA and I~OL~ = 7.8mA at 6V.
  - Source error: the note of Figure 6-3 (p.7) prints "TPHL" for
    t~PHL~; kept as printed.
  - Unconfirmed: the switching characteristics (p.5) name only "A or B"
    as the input of t~pd~ for this 8-input gate; kept as printed.
