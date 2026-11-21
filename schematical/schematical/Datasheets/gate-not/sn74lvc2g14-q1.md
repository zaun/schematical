---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g14-q1",
  "title": "SN74LVC2G14-Q1 automotive dual Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc2g14-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES558E",
    "date": "2025-01",
    "pages": 23
  },
  "devices": ["gate:2G14"],
  "parts": ["SN74LVC2G14IDCKRQ1"],
  "packages": {"p6": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G14-Q1 automotive dual Schmitt-trigger inverter

## Description

This dual Schmitt-trigger inverter is designed for 1.65V to 5.5V V~CC~
operation.

Qualified for automotive applications. Supports 5V V~CC~ operation.
Inputs accept voltages to 5.5V. Max t~pd~ of 5.4ns at 3.3V. Low power
consumption, 10μA max I~CC~. ±24mA output drive at 3.3V. Typical V~OLP~
(output ground bounce) <0.8V at V~CC~ = 3.3V, T~A~ = 25°C. Typical
V~OHV~ (output V~OH~ undershoot) >2V at V~CC~ = 3.3V, T~A~ = 25°C.
I~off~ feature supports partial-power-down mode operation. Latch-up
performance exceeds 100mA per JESD 78, class II.

## Pinout

Conversion note: DBV and DCK share one pin order.

```pinout pinout=p6
```

## Pin functions

```pins
1A: Gate 1 logic signal
1Y: Gate 1 inverted signal
2A: Gate 2 logic signal
2Y: Gate 2 inverted signal
GND: Ground
VCC: Supply/Power Pin
```

I = input, O = output, P = power, FB = feedback, GND = ground, N/A = not applicable

## Functional description

The contains two inverters and performs the Boolean function Y = !{A}.
The device functions as two independent inverters, but because of
Schmitt action, it may have different input threshold levels for
positive-going (V~T+~) and negative-going (V~T–~) signals.

Source error: the first sentence of the source's Overview (p.9) prints "The contains", without the device name. Kept as printed.

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down.

Conversion note: the logic diagram (positive logic, Figure 6-1) shows two Schmitt-trigger inverters, 1A (pin 1) to 1Y (pin 6) and 2A (pin 3) to 2Y (pin 4).

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture. These
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I).

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs with slow
transitioning signals will increase dynamic current consumption of the
device. For additional information regarding Schmitt-trigger inputs,
please see Understanding Schmitt Triggers.

## Function tables

```table id=function caption="Functional table (each inverter)"
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
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 | ±1000 | ^ |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC2G14-Q1 | < | Unit |
| ^ | ^ | DBV (SOT23), 6 pins | DCK (SC70), 6 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 165 | 259 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 1.65 V | 0.7 | | 1.4 | 0.7 | | 1.4 | V |
| ^ | ^ | | 2.3 V | 1 | | 1.7 | 1 | | 1.7 | ^ |
| ^ | ^ | | 3 V | 1.3 | | 2.2 | 1.3 | | 2.2 | ^ |
| ^ | ^ | | 4.5 V | 1.9 | | 3.1 | 1.9 | | 3.1 | ^ |
| ^ | ^ | | 5.5 V | 2.2 | | 3.7 | 2.2 | | 3.7 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 1.65 V | 0.3 | | 0.7 | 0.3 | | 0.7 | V |
| ^ | ^ | | 2.3 V | 0.4 | | 1 | 0.4 | | 1 | ^ |
| ^ | ^ | | 3 V | 0.6 | | 1.3 | 0.6 | | 1.3 | ^ |
| ^ | ^ | | 4.5 V | 1.1 | | 2 | 1.1 | | 2 | ^ |
| ^ | ^ | | 5.5 V | 1.4 | | 2.5 | 1.4 | | 2.5 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 1.65 V | 0.3 | | 0.8 | 0.3 | | 0.8 | V |
| ^ | ^ | | 2.3 V | 0.4 | | 0.9 | 0.4 | | 0.9 | ^ |
| ^ | ^ | | 3 V | 0.4 | | 1.1 | 0.4 | | 1.1 | ^ |
| ^ | ^ | | 4.5 V | 0.6 | | 1.3 | 0.6 | | 1.3 | ^ |
| ^ | ^ | | 5.5 V | 0.7 | | 1.4 | 0.7 | | 1.4 | ^ |
| V~OH~ | | I~OH~ = -100 μA | 1.65 V to 4.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.60 | ^ |
| I~I~ | A inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 16 | 17 | 18 | 21 | pF |
```

## Switching characteristics

```table id=sw-85 caption="Switching characteristics, –40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3.9 | 9.5 | 1.9 | 5.7 | 2 | 5.4 | 1.5 | 4.3 | ns |
```

```table id=sw-125 caption="Switching characteristics, –40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3.9 | 10.5 | 1.9 | 6.5 | 2 | 6 | 1.5 | 4.7 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figure also shows pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions caption="Test conditions (Figure 5-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

The device is a high-drive CMOS device that can be used for a multitude
of buffer type functions where the input is slow or noisy. The device
can produce 24mA of drive current at 3.3V, making it Ideal for driving
multiple outputs and good for high-speed applications up to 100MHz. The
inputs are 5.5V tolerant allowing it to translate down to V~CC~.

Conversion note: the typical application schematic (Figure 7-1) shows an
inverter with a ~2.2MΩ resistor and a 16 pF capacitor from its output
back to its input and a ~32 pF capacitor from each of the two nodes to
ground, driving one channel of the device, whose output has a 50 pF load
to ground.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74LVC2G14-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Ensure the
maximum total current through GND listed in the Absolute Maximum Ratings
is not exceeded.

The SN74LVC2G14-Q1 can drive a load with a total capacitance less than
or equal to 50pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50pF.

The SN74LVC2G14-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the HIGH
state, the output voltage in the equation is defined as the difference
between the measured output voltage and the supply voltage at the V~CC~
pin.

Total power consumption can be calculated using the information provided
in CMOS Power Consumption and Cpd Calculation.

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices.

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
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LVC2G14-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors.

The SN74LVC2G14-Q1 has no input signal transition rate requirements
because it has Schmitt-Trigger inputs.

Another benefit to having Schmitt-Trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit.

Unlike what happens with standard CMOS inputs, Schmitt-Trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics.

Refer to the Feature Description section for additional information
regarding the inputs for this device.

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

Refer to the Feature Description section for additional information
regarding the outputs for this device.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74LVC2G14-Q1 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

### Application curve

```curve id=icc-freq caption="ICC vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 80]},
 "y": {"label": "Icc", "unit": "mA", "range": [0, 10]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [20, 0.8], [40, 1.4], [60, 2.1], [75, 2.6]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [20, 1.0], [40, 2.1], [60, 3.2], [75, 3.9]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [20, 1.4], [40, 2.9], [60, 4.3], [75, 5.3]]},
  {"label": "Icc 5V", "points": [[0, 0], [20, 2.5], [40, 5.0], [60, 7.2], [72, 9.0], [75, 9.2]]}
 ]}
```

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1μF and 1μF capacitors are commonly used in
parallel. The bypass capacitor should be installed as close to the power
terminal as possible for best results.

### Layout guidelines

- Bypass capacitor placement
  - Place near the positive supply terminal of the device
  - Provide an electrically short ground return path
  - Use wide traces to minimize impedance
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible
- Signal trace geometry
  - 8mil to 12mil trace width
  - Lengths less than 12cm to minimize transmission line effects
  - Avoid 90° corners for signal traces
  - Use an unbroken ground plane below signal traces
  - Flood fill areas around signal traces with ground
  - For traces longer than 12cm
    - Use impedance controlled traces
    - Source-terminate using a series damping resistor near the output
    - Avoid branches; buffer signals that must branch separately

Conversion note: the layout examples (Figures 7-3 to 7-7) show trace
corners for improved signal integrity, bypass capacitor placement for
TSSOP, WQFN, SOT, SC70 and similar packages, and damping resistor
placement at the transmitting port of a long controlled-impedance trace.

## Package

```package footprint=SC70-6
```

## Ordering information

```ordering
SN74LVC2G14IDCKRQ1 | Reel of 3000 | -40 to 125 | CFJ, CFO
```

## Conversion notes

- Source pages 1 and 3–14 converted. The ordering line is from the
  package option addendum (p.16). The table of contents (p.2), the
  support and revision history page (15), the rest of the addendum
  (p.17), tape and reel (pp.18–19), mechanical pages (20–22) and the
  notice page (23) are left out; the generated Package section replaces
  them.
- Dropped: package information table and the vendor pinout drawing; the
  sentence "Table 6-1 lists the functional modes of the ." (p.9), which
  names no device; the logic diagram, the typical application schematic
  (Figure 7-1) and the layout examples (Figures 7-3 to 7-7) are
  described in text. The features are sentences in Description.
- The pin table's note on its I/O column is kept as a line under the pin
  descriptions; the generated pin table draws the type from the catalog.
- The CAUTION box of the Power Considerations is kept as a paragraph.
- The source also covers the DBV package, of which the package option
  addendum lists no part.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 5-1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- Curve digitized from Figure 7-2 (p.13) to about 2 % of full scale.
- From images: overbar of Y = !{A} (p.9; the text layer prints
  "Y = A"); ≤2 ns, ≤2.5 ns, ≤ 10 MHz, 1 kΩ, 500 Ω, 50 Ω, V~Δ~ (p.8; the
  text layer prints "£2 ns", "1 kW", "500 W", "VD"); the Ω of ~2.2MΩ
  and the arrangement of Figure 7-1 (p.10); the curve points of
  Figure 7-2 (p.13); the arrangement of Figures 7-3 to 7-7 (p.14).
- Source issues:
  - Source error: the Overview (p.9) prints "The contains two
    inverters", without the device name. Kept as printed.
  - Source conflict: the Power Considerations (p.11) refer to T~J(max)~
    listed in the Absolute Maximum Ratings; that table (p.4) has no
    junction temperature row.
  - Source conflict: the Input Considerations (p.12) refer to a plot in
    the Typical Characteristics; the data sheet has no Typical
    Characteristics section.
  - Unconfirmed: the Electrical Characteristics (p.6) give V~OH~ and
    V~OL~ at ±100 μA for V~CC~ = 1.65 V to 4.5 V, not to 5.5 V.
