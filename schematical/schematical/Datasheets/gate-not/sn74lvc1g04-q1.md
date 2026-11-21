---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g04-q1",
  "title": "SN74LVC1G04-Q1 single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc1g04-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES482F",
    "date": "2025-10",
    "pages": 25
  },
  "devices": ["gate:1G04"],
  "parts": ["SN74LVC1G04QDBVRQ1", "SN74LVC1G04QDCKRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G04-Q1 single inverter gate

## Description

Qualified for automotive applications. Supports 5V V~CC~ operation.
Inputs accept voltages to 5.5V. Latch-up performance exceeds 100mA per
JESD 78, Class II.

This single inverter gate is designed for 1.65V to 5.5V V~CC~ operation.

The SN74LVC1G04-Q1 performs the Boolean function Y = !{A}.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when the device is powered down.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Logic input
GND: Ground
NC: No internal connection
VCC: Supply voltage
Y: Inverted logic output
```

## Functional description

The SN74LVC1G04-Q1 device contains inverter gate and performs the
Boolean function Y = !{A}. This device is fully specified for
partial-power-down applications using I~off~. The I~off~ circuitry
disables the outputs, preventing damaging current backflow through the
device when it is powered down.

Conversion note: the functional block diagram shows one inverter, A (pin 2) to Y (pin 4).

Wide operating voltage range. Operates from 1.65 V to 5.5 V. Allows down
voltage translation. Inputs accept voltages to 5.5 V. I~off~ feature
allows voltages on the inputs and outputs, when V~CC~ is 0 V.

## Function tables

```table id=function caption="Function table"
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
| θ~JA~ | Package thermal impedance [4] | DBV package | | 206 | °C/W |
| ^ | ^ | DCK package | | 252 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| SN74LVC1G04-Q1 in DBV package | < | < | < | < |
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±750 | ^ |
| SN74LVC1G04-Q1 in DCK package | < | < | < | < |
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±750 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | ^ |
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
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ^ |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ^ |
| T~A~ | Operating free-air temperature | Q-suffix device | -40 | 125 | °C |
| ^ | ^ | I-suffix device | -40 | 85 | ^ |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LVC1G04-Q1 | < | Unit |
| ^ | ^ | DBV (SOT-23) 5 pins | DCK (SC-70) 5 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 357.1 | 371.0 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 263.7 | 297.5 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 264.4 | 258.6 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 195.6 | 195.6 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 262.2 | 256.2 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application note.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –100 μA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –4 mA | 1.65 V | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = –8 mA | 2.3 V | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = –16 mA | 3 V | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = –24 mA | ^ | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = –32 mA | 4.5 V | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | ^ |
| I~I~ | A input | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 3.5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 16 | 18 | 18 | 20 | pF |
```

## Switching characteristics

Conversion note: Figure 6-1 and Figure 7-1 are in [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 6-1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 2 | 6.4 | 1 | 4.2 | 0.7 | 3.3 | 0.7 | 3.1 | ns |
```

```table id=sw-50pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF (unless otherwise noted) (see Figure 7-1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3 | 7.5 | 1.4 | 5.2 | 1 | 4.2 | 1 | 3.7 | ns |
```

## Typical characteristics

```curve id=tpd-temp caption="TPD across temperature at 3.3-V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.7], [-40, 2.9], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```

```curve id=tpd-vcc caption="TPD across V~CC~ at 25°C"
{"x": {"label": "Vcc", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 7.0], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
 ]}
```

```curve id=icc-freq caption="I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 80]},
 "y": {"label": "Icc", "unit": "mA", "range": [0, 7]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [20, 0.5], [40, 0.95], [60, 1.35], [75, 1.7]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [20, 0.75], [40, 1.4], [60, 2.05], [70, 2.25], [75, 2.45]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [20, 1.0], [40, 2.0], [55, 2.6], [60, 2.9], [65, 3.55], [75, 3.75]]},
  {"label": "Icc 5V", "points": [[0, 0], [20, 1.7], [35, 2.95], [40, 3.6], [45, 3.95], [50, 4.6], [55, 4.8], [60, 5.2], [65, 6.15], [70, 6.5], [75, 6.5]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND, and the figure below
draws the load with S1 open.
Conversion note: the source figures also show pulse-duration,
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

```table id=test-15pf caption="Figure 6-1. Load circuit and voltage waveforms, C~L~ = 15 pF"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```

```table id=test-50pf caption="Figure 7-1. Load circuit and voltage waveforms, C~L~ = 30 pF"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
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

The SN74LVC1G04-Q1 is a high drive CMOS device that can be used for
implementing inversion logic with a high output drive, such as an LED
application. It can produce 24 mA of drive current at 3.3 V making it
Ideal for driving multiple outputs and good for high speed applications
up to 100 Mhz. The inputs are 5.5 V tolerant allowing it to translate
down to V~CC~.

Conversion note: the typical application figure shows the LVC1G04-Q1 as
an inverter logic function between µC or logic signals, and as a basic
LED driver with the LED and a series resistor from the output to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing.

### Detailed design procedure

1. Recommended Input Conditions
   - Rise time and fall time specs: See (Δt/ΔV) in the Recommended
     Operating Conditions table.
   - Specified high and low levels: See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ maximum) in the Recommended Operating Conditions table at any
     valid V~CC~.
2. Recommend Output Conditions
   - Load currents should not exceed (I~O~ maximum) per output and
     should not exceed total current (continuous current through V~CC~
     or GND) for the part. These limits are located in the Absolute
     Maximum Ratings table.
   - Outputs should not be pulled above V~CC~.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions
table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1 μF capacitor is
recommended. if there are multiple V~CC~ pins, then a 0.01 μF or
0.022 μF capacitor is recommended for each power pin. It is ok to
parallel multiple bypass capacitors to reject different frequencies of
noise. 0.1 μF and 1 μF capacitors are commonly used in parallel. The
bypass capacitor should be installed as close to the power pin as
possible for best results.

### Layout guidelines

When using multiple bit logic devices, inputs should never float.

In many cases, functions or parts of functions of digital logic devices
are unused; for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. The rules
that must be observed under all circumstances are specified in the next
paragraph.

All unused inputs of digital logic devices must be connected to a high
or low bias to prevent them from floating. The logic level that should
be applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~; whichever makes
more sense or is more convenient.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74LVC1G04QDBVRQ1 | Reel of 3000 | -40 to 125 | 34Z5, 3NRH, 3VGF, C04O
SN74LVC1G04QDCKRQ1 | Reel of 3000 | -40 to 125 | CCJ, CCO
```

## Conversion notes

- Source pages 1–12 converted; p.2 holds only the table of contents,
  which is dropped. The documentation support and revision history pages
  (13–14), the package option addendum (p.15–16), tape and reel and
  mechanical pages (17–25) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: the features list except the lines quoted, the applications
  list, the device information table, the logic diagram on p.1 (one
  inverter), the enable/disable-time, pulse-duration and setup/hold
  parts of Figures 6-1 and 7-1 with their notes B, E and F (this device
  has no output enable), and the layout example (an unused input tied to
  V~CC~ or GND). The typical application figure is described in text.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- Curves digitized from Figures 5-1, 5-2 and 8-1 to about 2 % of full
  scale.
- The source's electrical characteristics table gives symbols only, so
  its Parameter cells are empty except for I~I~.
- From images: overbar of Y = !{A} (p.1, p.10); ≤ 2 ns, ≤ 2.5 ns, 1 MΩ,
  1 kΩ, 500 Ω, 50 Ω, V~Δ~ (Figures 6-1 and 7-1, p.8–9; the text layer
  reads "£2 ns", "1 MW", "1 kW", "500 W", "50 W" and "VD").
- Source issues:
  - Source error: the features list (p.1) prints "CDM ESG classification
    level"; ESD is meant, and the line is kept out.
  - Source error: Overview (p.10) describes a DPW package with a 0.64 mm
    square footprint, which this datasheet does not offer (the device
    information table and the addendum list DBV and DCK only); the
    paragraph is kept out.
  - Source conflict: the second switching characteristics table (p.6)
    and the load figure's caption (p.9) call the figure "Figure 7-1",
    while it is in section 6 after Figure 6-1.
  - Source conflict: the absolute maximum ratings (p.4) give θ~JA~ as
    206 °C/W (DBV) and 252 °C/W (DCK), and the thermal information table
    (p.5) gives R~θJA~ as 357.1 °C/W and 371.0 °C/W.
  - Unconfirmed: ψ~JT~ is 195.6 °C/W for both DBV and DCK (p.5).
  - Unconfirmed: "100 Mhz" in Application Information (p.11) is kept as
    printed.
