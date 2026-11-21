---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs86",
  "title": "SN74HCS86 quadruple 2-input XOR gates with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74hcs86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS783C",
    "date": "2026-10",
    "pages": 33
  },
  "devices": ["gate:74_86"],
  "parts": ["SN74HCS86BQAR", "SN74HCS86DR", "SN74HCS86PWR"],
  "packages": {"p14": "D, DYY, PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74HCS86 quadruple 2-input XOR gates with Schmitt-trigger inputs

## Description

This device contains four independent 2-input XOR Gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = A ⊕ B in positive logic.

Wide operating voltage range: 2V to 6V. Schmitt-trigger inputs allow
for slow or noisy input signals. Extended ambient temperature range:
–40°C to +125°C, T~A~.

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

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. BQA Package only. Source error:
the pin table marks Thermal Pad (1), the note on the TYPE letters; its
note (2), BQA Package only, has no marker and is the one that fits.

## Functional description

This device contains four independent 2-input XOR Gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = A ⊕ B in positive logic.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due
to overcurrent. The electrical and thermal limits defined in the
Absolute Maximum Ratings must be followed at all times.

Unused push-pull CMOS outputs must be left disconnected.

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture.
These inputs are high impedance and are typically modeled as a resistor
in parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I).

The Schmitt-trigger input architecture provides hysteresis as defined
by ΔV~T~ in the Electrical Characteristics table, which makes this
device extremely tolerant to slow or noisy inputs. While the inputs can
be driven much slower than standard CMOS inputs, it is still
recommended to properly terminate unused inputs. Driving the inputs
with slow transitioning signals will increase dynamic current
consumption of the device.

### Clamp diode structure

As shown in Figure 7-1, the inputs and outputs to this device have both
positive and negative clamping diodes.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

## Function tables

Function Table lists the functional modes of the SN74HCS86.

```table id=function caption="Function table"
| Inputs [1] | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |

[1] H = High Voltage Level, L = Low Voltage Level
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < –0.5 V or V~I~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~I~ < –0.5 V or V~I~ > V~CC~ + 0.5 V [4] | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±70 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Rating may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Condition. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Guaranteed by design.
[4] Unconfirmed: the source prints the output clamp current's condition with V~I~, the same text as the input clamp current's.
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
| V~CC~ | Supply voltage | 2 | 5 | 6 | V |
| V~I~ | Input voltage | 0 | | V~CC~ | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise and fall rate [1] | | | | ns/V |
| T~A~ | Ambient temperature | -40 | | 125 | °C |

[1] Conversion note: the source prints Unlimited in the Max column of this row.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | SN74HCS86 | < | < | < | Unit |
| ^ | ^ | PW (TSSOP) 14 pins | D (SOIC) 14 pins | DYY (SOT-23) 14 pins | BQA (WQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 151.7 | 133.6 | 219.6 | 109.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 79.4 | 89.0 | 128.2 | 111.0 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 89.5 | 130.9 | 77.9 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 25.2 | 45.5 | 21.7 | 20.2 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.1 | 89.1 | 130.0 | 77.8 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | 56.6 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application.
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

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF; over operating free-air temperature range; typical ratings measured at T~A~ = 25°C (unless otherwise noted). See Parameter Measurement Information."
| Symbol | Parameter | From (input) | To (output) | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | 2V | | 15 | 36 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 7 | 13 | ns |
| ^ | ^ | ^ | ^ | 6V | | 6 | 12 | ns |
| t~t~ | Transition-time | | Y | 2V | | 9 | 16 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 5 | 9 | ns |
| ^ | ^ | ^ | ^ | 6V | | 4 | 8 | ns |
```

## Typical characteristics

```curve id=rol caption="Output driver resistance in LOW state, T~A~ = 25°C"
{"x": {"label": "Output sink current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [26, 46]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 41.5], [2.5, 42.0], [5, 43.0], [7.5, 44.4], [9.8, 46]]},
  {"label": "VCC = 3.3 V", "points": [[0.5, 31.4], [5, 31.6], [10, 31.9], [15, 32.3], [20, 32.8], [25, 33.5]]},
  {"label": "VCC = 4.5 V", "points": [[0.5, 28.5], [10, 28.6], [20, 28.9], [25, 29.1]]},
  {"label": "VCC = 6 V", "points": [[0.5, 27.0], [10, 27.05], [20, 27.2], [25, 27.3]]}
 ]}
```

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

Conversion note: Figures 5-3 and 5-4 (p.6) plot the supply current across the input voltage for six supply voltages from 2 V to 6 V, as hysteresis loops that are not digitized here.

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50Ω, t~t~ < 2.5 ns. For clock
inputs, f~max~ is measured when the input duty cycle is 50%. The
outputs are measured one at a time with one input transition per
measurement. C~L~ includes probe and test-fixture capacitance. The
greater between t~PLH~ and t~PHL~ is the same as t~pd~. The greater
between t~r~ and t~f~ is the same as t~t~.

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and input and output transition times"
VM: 50%
```

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality.

In this application, a 2-input XOR gate is used as a phase difference
detector as shown in Typical application block diagram. The remaining
three gates can be used for other applications in the system, or the
inputs can be grounded and the channels left unused.

The SN74HCS86 is used to identify phase difference between a reference
clock and another input clock. Whenever the clock states are different,
the XOR output will pulse HIGH until the clocks return to the same
state. The output is fed into a low-pass filter to obtain a DC
representation of the phase difference.

Typically, clock signals have fast transition rates, but additional
filtering can be added to the clock signals which can lead to slower
transitions rates. This makes the SN74HCS86 an excellent choice for the
application because it has Schmitt-trigger inputs that do not have
input transition rate requirements.

Conversion note: Figure 8-1 (p.11) shows one XOR gate with the inputs Input Clock and Reference Clock, and its output through a series resistor R and a capacitor C to ground to Output.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the SN74HCS86 plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Ensure the maximum total current through V~CC~
listed in the Absolute Maximum Ratings is not exceeded.

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74HCS86 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into its ground connection.
Ensure the maximum total current through GND listed in the Absolute
Maximum Ratings is not exceeded.

The SN74HCS86 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50pF.

The SN74HCS86 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74HCS86 (as specified in the Electrical Characteristics), and the
desired input transition rate limits the resistor size. A 10kΩ resistor
value is often used due to these factors.

The SN74HCS86 has no input signal transition rate requirements because
it has Schmitt-Trigger inputs.

Another benefit to having Schmitt-Trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value
will provide the peak-to-peak limit.

Unlike what happens with standard CMOS inputs, Schmitt-Trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics.

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This
can cause excessive current and damage to the device.

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins.
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74HCS86 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

Conversion note: Figure 8-2 (p.14), the application timing diagram, shows Out high while the reference clock and the input clock differ.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1μF capacitor is recommended for this device. It
is acceptable to parallel multiple bypass capacitors to reject
different frequencies of noise. The 0.1μF and 1μF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results, as shown in
the following layout example.

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
or is more convenient.

Conversion note: Figure 8-3 (p.15), the example layout for the SN74HCS86, shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, unused inputs tied to V~CC~, an unused output left floating, and a GND flood fill.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74HCS86BQAR | Reel of 3000 | -40 to 125 | HCS86
SN74HCS86DR | Reel of 2500 | -40 to 125 | HCS86
SN74HCS86PWR | Reel of 2000 | -40 to 125 | HCS86
```

## Conversion notes

- Source pages 1–15 converted; the support and revision history pages
  (16–18), the package option addendum (p.19–20), tape and reel and
  mechanical pages (21–33) dropped, replaced by the generated Package
  and Ordering sections. The ordering lines are from the package option
  addendum.
- Dropped: features and applications lists, the package information
  table, the "Benefits of Schmitt-trigger inputs" figure (p.1), table
  of contents, the functional block diagram (one of four channels, xA
  and xB to xY), the pin table's note on the TYPE letters, the
  application block diagram, timing diagram and layout example (Figures
  8-1, 8-2 and 8-3, described in text).
- The source also covers the DYY (SOT-23) package; the catalog carries
  no DYY part.
- Figures 6-2 and 6-3 are shown by one shared waveform figure. The
  load circuit (Figure 6-1, p.7) prints no value for C~L~; the
  switching characteristics give C~L~ = 50 pF.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale. The supply current curves (Figures 5-3 and 5-4) are not
  digitized.
- From images: the curve points of Figures 5-1 and 5-2 (p.6): 41.5,
  42.0, 43.0, 44.4, 9.8, 31.4, 31.6, 31.9, 32.3, 32.8, 33.5, 28.5,
  28.6, 28.9, 29.1, 27.0, 27.05, 27.2, 27.3, 54.2, 55.5, 58.0, 61.3,
  66.0, 39.0, 39.6, 40.4, 41.3, 42.5, 43.9, 34.2, 34.7, 35.4, 35.8,
  31.2, 31.5, 31.8, 32.0.
- Source issues:
  - Source error: the pin table marks Thermal Pad with note (1), the
    TYPE legend; note (2), BQA Package only, has no marker (see Pin
    functions).
  - Unconfirmed: the output clamp current's condition is printed with
    V~I~ (absolute maximum ratings note 4).
