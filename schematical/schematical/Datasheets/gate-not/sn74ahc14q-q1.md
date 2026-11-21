---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc14q-q1",
  "title": "SN74AHC14Q-Q1 automotive hex Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc14q-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SGDS016E",
    "date": "2024-02",
    "pages": 29
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74AHC14QDRQ1", "SN74AHC14QPWRG4Q1", "SN74AHC14QPWRQ1"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74AHC14Q-Q1 automotive hex Schmitt-trigger inverter

## Description

The SN74AHC14Q contains six independent inverters. This device performs
the Boolean function Y = !{A}.

Qualified for automotive applications. EPIC™ (Enhanced-Performance
Implanted CMOS) process. Operating range of 2V to 5.5V V~CC~. Latch-up
performance exceeds 250mA per JESD 17. ESD protection exceeds 2000V per
MIL-STD-883, Method 3015.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Input 1A
1Y: Output 1Y
2A: Input 2A
2Y: Output 2Y
3A: Input 3A
3Y: Output 3Y
4Y: Output 4Y
4A: Input 4A
5Y: Output 5Y
5A: Input 5A
6Y: Output 6Y
6A: Input 6A
GND: Ground Pin
VCC: Power Pin
```

## Functional description

Each circuit functions as an independent inverter, but because of the
Schmitt action, the inverters have different input threshold levels for
positive-going (V~T+~) and negative-going (V~T−~) signals.

Conversion note: the logic diagram (Figure 7-1) shows one Schmitt-trigger inverter, input A, output Y.

This symbol is in accordance with ANSI/IEEE Std 91-1984 and IEC
Publication 617-12.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

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
| | Continuous current V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over recommended operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V [2] | | -50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V [2] | | 50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
[2] Unconfirmed: the source prints the unit mA for the V~CC~ = 2 V rows; the absolute maximum continuous output current is ±25 mA and the electrical characteristics test at I~OH~ = -50 μA and I~OL~ = 50 μA.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC14Q-Q1"
| Symbol | Thermal metric [1] | D 14 pins | PW 14 pins | BQA 14 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.6 | 147.7 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 3 V | 1.2 | | 2.2 | 1.2 | 2.2 | V |
| ^ | ^ | | 4.5 V | 1.75 | | 3.15 | 1.75 | 3.15 | V |
| ^ | ^ | | 5.5 V | 2.15 | | 3.85 | 2.15 | 3.85 | V |
| V~T−~ | Negative-going input threshold voltage | | 3 V | 0.9 | | 1.9 | 0.9 | 1.9 | V |
| ^ | ^ | | 4.5 V | 1.35 | | 2.75 | 1.35 | 2.75 | V |
| ^ | ^ | | 5.5 V | 1.65 | | 3.35 | 1.65 | 3.35 | V |
| ΔV~T~ | Hysteresis (V~T+~ − V~T−~) | | 3 V | 0.3 | | 1.2 | 0.3 | 1.2 | V |
| ^ | ^ | | 4.5 V | 0.4 | | 1.4 | 0.4 | 1.4 | V |
| ^ | ^ | | 5.5 V | 0.5 | | 1.6 | 0.5 | 1.6 | V |
| V~OH~ | | I~OH~ = -50 μA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.58 | | | 2.48 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 μA | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | μA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | μA |
| C~i~ [1] | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | PF |

[1] Source error: the source prints the unit of C~i~ as PF (p.5), kept as printed; the operating characteristics table prints the capacitance unit as pF.
```

Conversion note: the source prints no heading over the second Min and Max columns of this table and of the switching characteristics tables.

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.8 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.6 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 9 | pF |
```

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 8.3 | 12.8 | 1 | 15 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8.3 | 12.8 | 1 | 15 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 10.8 | 16.3 | 1 | 18.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 10.8 | 16.3 | 1 | 18.5 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 5.5 | 8.6 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 8.6 | 1 | 10 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 7 | 10.6 | 1 | 12 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7 | 10.6 | 1 | 12 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

The SN74AHC14Q-Q1 can be used to add an additional stage to a counter
with an external flip-flop. Because counters use a negative edge
trigger, the flip-flop's clock input must be inverted to provide this
function. Having Schmitt-trigger inputs is important in this application
to eliminate any noise issues that could impact the counting function
which could lead to incorrect frequency division. This function only
requires one of the six available inverters in the SN74AHC14Q-Q1 device,
so the remaining channels can be used for other applications needing an
inverted signal or improved signal integrity. Unused inputs must be
terminated at V~CC~ or GND. Unused outputs can be left floating.

Conversion note: in the typical application block diagram (Figure 8-1) a Schmitt-trigger inverter takes the last output of a counter to the clock input of a D-Type Flip-Flop, whose !{Q} output returns to its D input and whose Q output is the added stage; Clear goes to the CLR inputs of both.

### Design requirements

#### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74AHC14Q-Q1 plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHC14Q-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings.

The SN74AHC14Q-Q1 can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50 pF.

The SN74AHC14Q-Q1 can drive a load with total resistance described by
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

Source conflict: this caution (p.10) says T~J(max)~ is listed in the Absolute Maximum Ratings; that table (p.4) lists no junction temperature.

#### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHC14Q-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors.

The SN74AHC14Q-Q1 has no input signal transition rate requirements
because it has Schmitt-trigger inputs.

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit.

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics.

Source conflict: the source has no Typical Characteristics section and no Feature Description section, which this section and the next refer to.

Refer to the Feature Description section for additional information
regarding the inputs for this device.

#### Output considerations

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

Refer to the Feature Description section for additional information
regarding the outputs for this device.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74AHC14Q-Q1 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. This will ensure that the maximum output
   current from the Absolute Maximum Ratings is not violated. Most CMOS
   inputs have a resistive load measured in MΩ; much larger than the
   minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

Conversion note: the application timing diagram (Figure 8-2) shows the counter's last output, its inverse and the flip-flop output against time, with the legend Input – 32 kHz and flip-flop output – 1 kHz.

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

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill, a 0.1 μF bypass capacitor placed close to the device, unused inputs tied to GND or V~CC~, an unused output left floating, and no 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AHC14QDRQ1 | Reel of 2500 | -40 to 125 | AHC14Q
SN74AHC14QPWRG4Q1 | Reel of 2000 | -40 to 125 | HA14Q
SN74AHC14QPWRQ1 | Reel of 2000 | -40 to 125 | HA14Q | Last Time Buy
```

## Conversion notes

- Source pages 1–12 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.15–16). The support and revision history page (13), the
  mechanical information page (14), tape and reel and mechanical pages
  (17–29) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the logic
  diagram (one Schmitt-trigger inverter); the typical application block
  diagram (Figure 8-1), the application timing diagram (Figure 8-2) and
  the example layout (Figure 8-3) are described in text.
- The source also covers the BQA (WQFN, 14) package, with a thermal pad
  (PAD in Figure 4-2), and the addendum lists SN74AHC14QWBQARQ1; the
  catalog doesn't carry that part. The pin table's NC row (no pin
  number) is not in the D or PW pinout.
- The pin table's note "Signal Types: I = Input, O = Output, I/O = Input
  or Output." is on its TYPE column, which the generated pin table draws
  from the catalog.
- The electrical characteristics table of the source prints a parameter
  name only for V~T+~, V~T−~ and ΔV~T~; the other Parameter cells are
  empty. The absolute maximum ratings print their conditions in
  parentheses.
- The CAUTION box of the source (p.10) is kept as a paragraph at the end
  of Power considerations.
- Figure 6-1 (p.7) is a picture with no text layer; the load circuit and
  the propagation delay waveforms were compared with the page image. Its
  S1 table is printed as text under the figure and is left out with the
  3-state load circuit.
- From images: the overbar of Y = !{A} (p.1); the overbar of !{Q}
  (Figure 8-1, p.9); 50% V~CC~ (V~M~, Figure 6-1, p.7); 0.1 μF
  (Figure 8-3, p.12; the text layer prints "0.1 F"); the I~CC~ test
  condition I~O~ = 0 and V~CC~ = 5.5 V (p.5; the text layer runs them
  together as "IO = 05.5 V").
- Source issues:
  - Unconfirmed: I~OH~/I~OL~ at V~CC~ = 2 V printed as ±50 mA (ROC table
    note 2).
  - Source error: the unit of C~i~ is printed PF; kept as printed
    (electrical characteristics note 1).
  - Source error: the recommended operating conditions line ends
    "(unless otherwise noted)(1))" with a stray closing parenthesis.
  - Source conflict: T~J(max)~ said to be listed in the Absolute Maximum
    Ratings, which list no junction temperature (Power considerations
    note).
  - Source conflict: Typical Characteristics and Feature Description
    sections referenced but not in the source (Input considerations
    note).
  - Unconfirmed: the Description (p.1) names the device SN74AHC14Q,
    without the -Q1 suffix; kept as printed.
