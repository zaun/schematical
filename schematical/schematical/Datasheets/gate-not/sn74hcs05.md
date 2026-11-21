---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs05",
  "title": "SN74HCS05 hex inverter with open-drain outputs and Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74hcs05.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS797B",
    "date": "2026-10",
    "pages": 28
  },
  "devices": ["gate:74_05"],
  "parts": ["SN74HCS05DR", "SN74HCS05DYYR", "SN74HCS05PWR"],
  "packages": {"p14": "PW, D, DYY"},
  "converted": "2026-11-21"
}
---

# SN74HCS05 hex inverter with open-drain outputs and Schmitt-trigger inputs

## Description

This device contains six independent inverter with open-drain outputs
and Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A} in positive logic.

Wide operating voltage range: 2V to 6V. Schmitt-trigger inputs allow
for slow or noisy input signals. Extended ambient temperature range:
–40°C to +125°C, T~A~.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2Y: Channel 2, Output Y
3A: Channel 3, Input A
3Y: Channel 3, Output Y
4A: Channel 4, Input A
4Y: Channel 4, Output Y
5A: Channel 5, Input A
5Y: Channel 5, Output Y
6A: Channel 6, Input A
6Y: Channel 6, Output Y
GND: Ground
VCC: Positive Supply
```

## Functional description

This device contains six independent inverter with open-drain outputs
and Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A} in positive logic.

Conversion note: the functional block diagram (p.8), One of Six Channels, shows one inverter with a Schmitt-trigger symbol and an open-drain symbol, input xA and output xY.

### Open-drain CMOS outputs

Open-drain outputs are included in SN74HCS05. Open-drain outputs can
only drive the output low. When in the high logical state, outputs are
in a high-impedance state, meaning outputs neither source nor sink
current (with the exception of minor leakage current as defined in the
Electrical Characteristics table). In the high-impedance state, the
output voltage is not controlled by SN74HCS05 and is dependent on
external factors.

Because of the high-impedance state, an external pull-up resistor is
required to define a logic high. Without a pull-up resistor, the output
floats and has an undefined logic level. The resistor value depends on
factors like parasitic capacitance and power consumption; typically, a
10kΩ resistor is suitable.

The drive capability of SN74HCS05 can create fast edges into light
loads, so consider routing and load conditions to prevent ringing.
Additionally, outputs can drive higher currents than SN74HCS05 is
designed to handle continuously. Limit the device output power to avoid
damage due to overcurrent. Follow the electrical and thermal limits
defined in the Absolute Maximum Ratings at all times. See the
Recommended Operating Conditions table for the maximum output voltage
that can be externally connected to SN74HCS05.

Leave unused open-drain CMOS outputs disconnected.

### CMOS Schmitt-trigger inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor from the input to ground in parallel with the input
capacitance given in the Electrical Characteristics. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings, and the maximum input leakage current, given
in the Electrical Characteristics, using Ohm's law (R = V ÷ I).

The Schmitt-trigger input architecture provides hysteresis as defined
by ΔV~T~ in the Electrical Characteristics, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended
to properly terminate unused inputs. Driving the inputs slowly also
increases dynamic current consumption of the device. For additional
information regarding Schmitt-trigger inputs, please see Understanding
Schmitt Triggers.

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes as depicted in Figure 7-1.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The recommended input and output
voltage ratings may be exceeded if the input and output clamp-current
ratings are observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

## Function tables

Table 7-1 lists the functional modes of the SN74HCS05.

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| L | Z |
| H | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 or V~I~ > V~CC~ + 0.5 | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 or V~O~ > V~CC~ + 0.5 | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±70 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings can be exceeded if the input and output current ratings are observed.
[3] Verified by design.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±4000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1500 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | 2 | 5 | 6 | V |
| V~I~ | Input voltage | 0 | | V~CC~ | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| T~A~ | Ambient temperature [1] | -55 | | 125 | °C |

[1] Source conflict: this table gives –55 °C as the minimum ambient temperature (p.4); the features list (p.1) gives an extended ambient temperature range of –40°C to +125°C, and the package option addendum (p.15) lists every part at -40 to 125.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74HCS05 | < | Unit |
| ^ | ^ | DYY (SOT-23) 14 pins | PW (TSSOP) 14 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 219.6 | 151.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 128.2 | 79.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 130.9 | 94.7 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 21.7 | 25.2 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 130.0 | 94.1 | °C/W |
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
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20µA | 2V to 6V | | 0.002 | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4mA | 4.5V | | 0.18 | 0.30 | V |
| ^ | ^ | ^ | I~OL~ = 7.8mA | 6V | | 0.22 | 0.33 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ or 0 | < | 6V | | ±100 | ±1000 | nA |
| I~OZ~ | Output leakage current (3-state only) | V~O~ = V~CC~ or 0 | < | 6V | | 0.01 | 2 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0, I~O~ = 0 | < | 6V | | 0.1 | 2 | µA |
| C~i~ | Input capacitance | | < | 2V to 6V | | | 5 | pF |
```

```table id=operating caption="Operating characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | | 10 | | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). See Parameter Measurement Information."
| Symbol | Parameter | From (input) | To (output) | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A | Y | 2V | | 18 | 39 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 13 | 15 | ns |
| ^ | ^ | ^ | ^ | 6V | | 12 | 15 | ns |
| t~t~ | Transition-time | | Y | 2V | | 9 | 16 | ns |
| ^ | ^ | ^ | ^ | 4.5V | | 5 | 9 | ns |
| ^ | ^ | ^ | ^ | 6V | | 4 | 8 | ns |
```

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

Conversion note: Figures 5-2 and 5-3 (p.6) plot the typical supply current versus input voltage for supply voltages of 2 V, 2.5 V and 3.3 V and of 4.5 V, 5 V and 6 V, as hysteresis loops that are not digitized here.

## Parameter measurement information

- Phase relationships between waveforms are chosen arbitrarily. All
  input pulses are supplied by generators having the following
  characteristics: PRR ≤ 1MHz, Z~O~ = 50Ω, t~t~ < 6ns.
- The outputs are measured one at a time, with one input transition per
  measurement.

```figure ref=load-ttl caption="Load circuit"
RL: not given
CL: 50pF
```

C~L~= 50pF and includes probe and jig capacitance.

Conversion note: in the source's load circuit (Figure 6-1, p.7), R~L~ goes to V~CC~ through a switch, S~1~, whose position is not given; the figure above draws R~L~ connected to V~CC~.

Conversion note: Figure 6-2 (p.7), voltage waveforms transition times, shows t~r~ and t~f~ between the 10% and 90% points of the input, from 0V to V~CC~, and of the output, from V~OL~ to V~OH~; it is not drawn here.

Conversion note: Figure 6-3 (p.7), voltage waveforms propagation delays, shows t~PLZ~ from 50% of an input transition to 10% V~CC~ of the rising output and t~PZL~ from 50% of an input transition to 50% of the falling output, for two outputs of opposite phase; it is not drawn here.

The maximum between t~PLH~ and t~PHL~ is used for t~pd~.

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality.

In this application, the device is used to drive an indicator LED
directly. Unused channels must have the inputs terminated at either
V~CC~ or GND, whichever is more convenient, and the outputs must be
left disconnected.

Conversion note: Figure 8-1 (p.10), the typical application block diagram, shows one inverter with Schmitt-trigger and open-drain symbols, supplied from V~CC~ and driven by Input, whose output sinks the current of an LED fed from V~CC~ through resistor R~1~.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics.

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74HCS05 plus the maximum
supply current, I~CC~, listed in Electrical Characteristics. The logic
device can only sink as much current as is provided by the external
pull-up resistor or other supply source. Be sure not to exceed the
maximum total current through GND listed in the Absolute Maximum
Ratings.

Total power consumption can be calculated using the information
provided in CMOS Power Consumption and Cpd Calculation.

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices.

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
controller, leakage current into the SN74HCS05, as specified in the
Electrical Characteristics, and the desired input transition rate. A
10kΩ resistor value is often used due to these factors.

The SN74HCS05 has no input signal transition rate requirements because
it has Schmitt-trigger inputs.

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T~(min) in the Electrical Characteristics. This hysteresis value
will provide the peak-to-peak limit.

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics.

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output increases the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. The plot in
Typical Characteristics provides a typical relationship between output
voltage and current for this device.

Open-drain outputs can be directly connected together to produce a
wired-AND. This is possible because the outputs cannot source current,
and thus can never be in bus-contention.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the
   Layout.
2. Ensure the capacitive load at the output is ≤ 70pF. This is not a
   hard limit, however it will ensure specified performance. This can
   be accomplished by providing short, appropriately sized traces from
   the SN74HCS05 to the receiving device.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O~(max)) Ω. This will ensure that the maximum output
   current from the Absolute Maximum Ratings is not violated. Most CMOS
   inputs have a resistive load measured in megaohms; much larger than
   the minimum calculated above.
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application note, CMOS Power Consumption and
   Cpd Calculation

Conversion note: Figure 8-2 (p.11), the application timing diagram, shows the waveforms of Input and Output and the LED Status, OFF while Input is low and ON while Input is high.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Condtions.
Each V~CC~ terminal must have a bypass capacitor to prevent power
disturbance. A 0.1μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1μF and 1μF capacitors are commonly used in
parallel. The bypass capacitor must be installed as close to the power
terminal as possible for best results, as shown in Figure 8-3.

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs
must not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used. Such unused input pins
must not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. All unused
inputs of digital logic devices must be connected to a logic high or
logic low voltage, as defined by the input voltage specifications, to
prevent them from floating. The logic level that must be applied to any
particular unused input depends on the function of the device.
Generally, the inputs are tied to GND or V~CC~, whichever makes more
sense for the logic function or is more convenient.

Conversion note: Figure 8-3 (p.12), the example layout for the SN74HCS05, shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, an unused input tied to GND, an unused input tied to V~CC~, an unused output left floating, and a GND flood fill.

## Package

```package footprint=SOIC-14
```

```package footprint=SOT23-THIN-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCS05DR | Reel of 2500 | -40 to 125 | HCS05
SN74HCS05DYYR | Reel of 3000 | -40 to 125 | HCS05
SN74HCS05PWR | Reel of 2000 | -40 to 125 | HCS05
```

## Conversion notes

- Source pages 1–12 converted; the support and revision history pages
  (13–14), the package option addendum (p.15–16), tape and reel and
  mechanical pages (17–28) dropped, replaced by the generated Package
  and Ordering sections. The ordering lines are from the package option
  addendum.
- Dropped: features and applications lists, the package information
  table, the "Benefits of Schmitt-trigger Inputs" figure (p.1), table
  of contents, the pin table's note on the TYPE letters, the functional
  block diagram, the transition-time and propagation-delay waveforms
  (Figures 6-2 and 6-3), the application block diagram, timing diagram
  and layout example (Figures 8-1, 8-2 and 8-3), all described in text,
  and the two sentences that refer to the Feature Description for
  additional information regarding the inputs and outputs.
- The thermal information table has no column for the D (SOIC) package
  of SN74HCS05DR.
- The source's Operating Characteristics table (C~pd~) is under
  Electrical characteristics.
- Curve digitized from Figure 5-1 (p.6) to about 2 % of full scale. The
  supply current curves (Figures 5-2 and 5-3) are not digitized.
- From images: overbars of Y = !{A} (p.1, p.8).
- From images: the curve points of Figure 5-1 (p.6): 41.5, 42.0, 43.0,
  44.4, 31.4, 31.6, 31.9, 32.3, 32.8, 33.5, 28.5, 28.6, 28.9, 29.1,
  27.0, 27.05, 27.2, 27.3.
- From images: the subscript of S~1~ and the 10% V~CC~ level
  (Figures 6-1 and 6-3, p.7).
- Source issues:
  - Source conflict: minimum ambient temperature –55 °C in the
    recommended operating conditions against –40°C in the features list
    and the package option addendum (ROC note 1).
  - Source conflict: note A of Figure 6-3 (p.7) names t~PLH~ and
    t~PHL~; the figure itself marks t~PLZ~ and t~PZL~.
  - Source error: the source prints "six independent inverter" (p.1,
    p.8) and "Recommended Operating Condtions" (p.11); kept as printed.
  - Unconfirmed: the electrical characteristics print an I~OZ~ row
    named "Output leakage current (3-state only)" for this open-drain
    device (p.5); kept as printed.
