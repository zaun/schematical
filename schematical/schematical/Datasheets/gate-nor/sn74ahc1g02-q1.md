---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g02-q1",
  "title": "SN74AHC1G02-Q1 automotive single 2-input positive-NOR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74ahc1g02-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS957A",
    "date": "2023-10",
    "pages": 26
  },
  "devices": ["gate:1G02"],
  "parts": ["SN74AHC1G02DBVRQ1", "SN74AHC1G02QDCKRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G02-Q1 automotive single 2-input positive-NOR gate

## Description

The SN74AHC1G02-Q1 is a single 2-input NOR gate that performs the
Boolean function Y = !{A} × !{B} or Y = !{A + B} in positive logic.

AEC-Q100 qualified for automotive applications: Device temperature
grade 1: -40°C to +125°C. Operating range of 2-V to 5.5-V. Low power
consumption, 10-µA maximum I~CC~. ±8-mA output drive at 5 V. Latch-up
performance exceeds 250 mA per JESD 17.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground
Y: Output Y
VCC: VCC
```

## Functional description

The SN74AHC1G02-Q1 is a single 2-input NOR gate that performs the
Boolean function Y = !{A} × !{B} or Y = !{A + B} in positive logic.

Conversion note: the functional block diagram shows one 2-input NOR gate with inputs A and B and output Y.

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the Electrical Characteristics. The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I).

Standard CMOS inputs require that input signals transition between valid
logic states quickly, as defined by the input transition time or rate in
the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations. More details can be found in Implications of Slow or
Floating CMOS Inputs.

Do not leave standard CMOS inputs floating at any time during operation.
Unused inputs must be terminated at V~CC~ or GND. If a system will not
be actively driving an input at all times, then a pull-up or pull-down
resistor can be added to provide a valid input voltage during these
times. The resistor value will depend on multiple factors; a 10-kΩ
resistor, however, is recommended and will typically meet all
requirements.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times.

Unused push-pull CMOS outputs should be left disconnected.

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes only
as shown in Figure 8-1.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

Conversion note: Figure 8-1 (electrical placement of clamping diodes for each input and output) draws the output clamps to V~CC~ (+I~OK~) and GND (-I~OK~) and the input clamp to GND (-I~IK~).

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output [1] |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Do Not Care
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage range | | -0.5 | 7 | V |
| V~O~ | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state | | -0.5 | 4.6 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 | -20 | | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 or V~O~ > V~CC~ | -20 | 20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | -25 | 25 | mA |
| I~O~ | Continuous output current through V~CC~ or GND | | -50 | 50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] HBM ESD Classification Level 2 | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C4B | ±1000 | ^ |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply Voltage | | 2 | 5.5 | V |
| V~IH~ | high-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~I~ | input voltage | | 0 | 5.5 | V |
| V~O~ | output voltage | | 0 | V~CC~ | V |
| I~OH~ | high-level output current | V~CC~ = 2 V | | -50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | low-level output current | V~CC~ = 2 V | | 50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 | | 100 | nS/V [1] |
| Δt/Δv | input transition rise or fall rate | V~CC~ = 5 V ± 0.5 V | | 20 | nS/V [1] |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] Source error: the unit is printed nS/V; a transition rate is in ns/V.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHC1G02-Q1 | < | Unit |
| ^ | ^ | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278.0 | 293.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 208.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 180.6 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 120.6 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 179.5 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –50 μA | V~CC~ = 2 V | 1.9 | | | 1.9 | | | V |
| ^ | I~OH~ = –50 μA | V~CC~ = 3 V | 2.9 | | | 2.9 | | | V |
| ^ | I~OH~ = –50 μA | V~CC~ = 4.5 V | 4.4 | | | 4.4 | | | V |
| ^ | I~OH~ = –4 mA | V~CC~ = 3 V | 2.58 | | | 2.48 | | | V |
| ^ | I~OH~ = –8 mA | V~CC~ = 4.5 V | 3.94 | | | 3.8 | | | V |
| V~OL~ | I~OL~ = 50 μA [1] | V~CC~ = 2 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 50 μA [1] | V~CC~ = 3 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 50 μA [1] | V~CC~ = 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 4 mA [1] | V~CC~ = 3 V | | | 0.36 | | | 0.44 | V |
| ^ | I~OL~ = 8 mA [1] | V~CC~ = 4.5 V | | | 0.36 | | | 0.44 | V |
| I~I~ | V~I~ = 5.5V or GND | 0 V to 5.5 V | -0.1 | | 0.1 | -1 | | 1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | | 4 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 5V | | 1.7 | 10 | | | | pF |
| C~O~ | V~O~ = V~CC~ or GND | 5V | | 3 | | | | | pF |
| C~PD~ | Power dissipation capacitance | 5V | | 14 | | | | | pF |

[1] Source error: printed as I~OH~ = 50 μA, I~OH~ = 4 mA and I~OH~ = 8 mA in the V~OL~ rows; the rows give V~OL~, and the Recommended Operating Conditions give these currents as I~OL~.
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | V~CC~ | T~A~ = 25°C | < | < | -40°C to 85°C | < | < | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| T~PHL~ | A or B | Y | C~L~ = 15 pF | 3.3 V ± 0.3 V | | 5 | 7.1 | 1 | | 8.5 | 1 | | 9.5 | ns |
| T~PLH~ | A or B | Y | C~L~ = 15 pF | 3.3 V ± 0.3 V | | 5 | 7.1 | 1 | | 8.5 | 1 | | 9.5 | ns |
| T~PHL~ | A or B | Y | C~L~ = 50 pF | 3.3 V ± 0.3 V | | 7.5 | 10.6 | 1 | | 12 | 1 | | 13 | ns |
| T~PLH~ | A or B | Y | C~L~ = 50 pF | 3.3 V ± 0.3 V | | 7.5 | 10.6 | 1 | | 12 | 1 | | 13 | ns |
| T~PHL~ | A or B | Y | C~L~ = 15 pF | 5 V ± 0.5 V | | 3.6 | 5.5 | 1 | | 6 | 1 | | 6.5 | ns |
| T~PLH~ | A or B | Y | C~L~ = 15 pF | 5 V ± 0.5 V | | 3.6 | 5.5 | 1 | | 6 | 1 | | 6.5 | ns |
| T~PHL~ | A or B | Y | C~L~ = 50 pF | 5 V ± 0.5 V | | 5.1 | 6 | 1 | | 6.5 | 1 | | 7 | ns |
| T~PLH~ | A or B | Y | C~L~ = 50 pF | 5 V ± 0.5 V | | 5.1 | 6 | 1 | | 6.5 | 1 | | 7 | ns |
```

## Typical characteristics

T~A~ = 25°C (unless otherwise noted)

```curve id=icc-vin caption="Supply current across input voltage 3.3-V and 5.0-V supply"
{"x": {"label": "V_IN - Input Voltage", "unit": "V", "range": [0, 5]},
 "y": {"label": "I_CC - Supply Current", "unit": "µA", "range": [0, 800]},
 "series": [
  {"label": "3.3 V", "points": [[0.6, 0], [0.9, 30], [1.05, 70], [1.12, 185], [1.15, 90], [1.5, 62], [2, 30], [2.3, 10], [2.6, 0]]},
  {"label": "5.0 V", "points": [[0.6, 0], [0.8, 15], [1, 50], [1.15, 90], [1.3, 170], [1.4, 330], [1.46, 640], [1.5, 230], [2, 180], [2.5, 125], [3, 80], [3.5, 40], [4, 12], [4.3, 0]]}
 ]}
```

```curve id=voh-5v caption="Output voltage vs current in HIGH state; 5-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.3, 5]},
 "series": [
  {"label": "-40°C", "points": [[-25, 4.5], [-0.5, 4.99]]},
  {"label": "25°C", "points": [[-25, 4.455], [-0.5, 4.99]]},
  {"label": "125°C", "points": [[-25, 4.385], [-0.5, 4.99]]}
 ]}
```

```curve id=vol-5v caption="Output voltage vs current in LOW state; 5-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.5]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.418]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.44]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.482]]}
 ]}
```

```curve id=voh-3v3 caption="Output voltage vs current in HIGH state; 3.3-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [2.5, 3.3]},
 "series": [
  {"label": "-40°C", "points": [[-25, 2.685], [-20, 2.82], [-15, 2.95], [-10, 3.065], [-5, 3.18], [-0.5, 3.29]]},
  {"label": "25°C", "points": [[-25, 2.615], [-20, 2.765], [-15, 2.905], [-10, 3.045], [-5, 3.175], [-0.5, 3.29]]},
  {"label": "125°C", "points": [[-25, 2.51], [-20, 2.685], [-15, 2.855], [-10, 3.01], [-5, 3.155], [-0.5, 3.29]]}
 ]}
```

```curve id=vol-3v3 caption="Output voltage vs current in LOW state; 3.3-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.6]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.475]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.508]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.568]]}
 ]}
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω. For clock inputs, f~max~ is measured when the
input duty cycle is 50%. The outputs are measured one at a time with one
input transition per measurement.

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. The greater between t~r~ and
t~f~ is the same as t~t~.

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays, input and output transition times"
VM: 50%
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

This low-drive CMOS device that can be used for a multitude of bus
interface type applications where output ringing is a concern. The low
drive and slow edge rates will minimize overshoot and undershoot on the
outputs. The inputs can accept voltages to 5.5 V at any valid V~CC~
making it an excellent choice for down translation.

Conversion note: the typical application block diagram (Figure 9-1) shows the gate, supplied from a 3.3-V or 5-V regulated rail with a 0.1 µF capacitor to ground, between 5-V system logic on a 5-V accessory and a µC or system logic.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74AHC1G02-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHC1G02-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings.

The SN74AHC1G02-Q1 can drive a load with a total capacitance less than
or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF.

The SN74AHC1G02-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin.

Total power consumption can be calculated using the information provided
in the CMOS Power Consumption and Cpd Calculation application note.
Thermal increase can be calculated using the information provided in the
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHC1G02-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors.

The SN74AHC1G02-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics.

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device.

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74AHC1G02-Q1 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

Conversion note: the application curve (Figure 9-2) is a picture of output edges against time in ns, and is not drawn here.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in the following
layout example.

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
convenient.

Conversion note: the example layout (Figure 9-3) recommends a GND flood fill, a bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74AHC1G02DBVRQ1 | Reel of 3000 | -40 to 125 | 38BH
SN74AHC1G02QDCKRQ1 | Reel of 3000 | -40 to 125 | 1QJ
```

## Conversion notes

- Source pages 1–14 converted. The ordering lines are from the package
  option addendum (pp.16–17). The support page (15) and the tape and
  reel and mechanical pages (18–26) are left out; the generated Package
  section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description; the ESD classification lines are left
  out), table of contents and revision history (p.2), package
  information table, the simplified logic diagram and the functional
  block diagram (one NOR gate); the clamp diode figure (Figure 8-1), the
  typical application block diagram (Figure 9-1), the application curve
  (Figure 9-2) and the example layout (Figure 9-3) are described in
  text.
- The notes of Figures 7-1 to 7-3 are kept as sentences in Parameter
  measurement information.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column; "Power dissipation
  capacitance" is printed in its test conditions column.
- Curves digitized from Figures 6-1 to 6-5 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{A} × !{B} and Y = !{A + B} (p.1 and
  p.9); the curve points 0.6, 0.9, 30, 1.05, 70, 1.12, 185, 1.15, 90,
  62, 2.3, 2.6, 0.8, 15, 1.3, 170, 1.4, 330, 1.46, 230, 180, 125, 40,
  12, 4.3 (Figure 6-1, p.7), 4.455, 4.385, 4.99 (Figure 6-2, p.7), 0.01,
  0.418, 0.482 (Figure 6-3, p.7), 2.685, 2.82, 3.065, 3.18, 3.29, 2.615,
  2.765, 2.905, 3.045, 3.175, 2.51, 2.855, 3.01, 3.155 (Figure 6-4,
  p.7), 0.475, 0.508, 0.568 (Figure 6-5, p.7); the arrangement of
  Figure 9-1 (p.11), Figure 9-2 (p.13) and Figure 9-3 (p.14); the diode
  placement of Figure 8-1 (p.10).
- Source issues:
  - Source error: the V~OL~ rows of the electrical characteristics table
    (p.5) are printed with I~OH~; written I~OL~ (table note).
  - Source error: the unit of Δt/Δv is printed nS/V (p.5); kept as
    printed (table note).
  - Source conflict: the Recommended Operating Conditions (p.5) give
    T~A~ from -55 °C, the features (p.1), the electrical and switching
    characteristics tables (pp.5–6) and the package option addendum
    -40 °C.
  - Source conflict: the features (p.1) give a 10-µA maximum I~CC~, the
    electrical characteristics table (p.5) 1 µA and 4 µA.
  - Unconfirmed: the first sentence of the application information
    ("This low-drive CMOS device that can be used") is kept as printed.
  - Unconfirmed: the switching characteristics symbols are printed
    T~PHL~ and T~PLH~ (p.6), the figures t~PLH~ and t~PHL~ (p.8).
