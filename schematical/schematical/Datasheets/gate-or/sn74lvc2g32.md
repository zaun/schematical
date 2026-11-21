---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g32",
  "title": "SN74LVC2G32 dual 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lvc2g32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES201N",
    "date": "2015-09",
    "pages": 26
  },
  "devices": ["gate:2G32"],
  "parts": ["SN74LVC2G32DCTR", "SN74LVC2G32DCTRE4", "SN74LVC2G32DCTRG4", "SN74LVC2G32DCUR", "SN74LVC2G32DCURE4", "SN74LVC2G32DCURG4", "SN74LVC2G32DCUT", "SN74LVC2G32YZPR"],
  "packages": {"p8": "DCT, DCU", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G32 dual 2-input positive-OR gate

## Description

This dual 2-input positive-OR gate is designed for 1.65-V to 5.5-V
V~CC~ operation.

The SN74LVC2G32 device performs the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic.

NanoFree package technology is a major breakthrough in IC packaging
concepts, using the die as the package.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V. Maximum
t~pd~ of 3.8 ns at 3.3 V. Low Power Consumption, 10-µA Maximum I~CC~.
±24-mA Output Drive at 3.3 V. Typical V~OLP~ (Output Ground Bounce)
<0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C. Typical V~OHV~ (Output V~OH~
Undershoot) >2 V at V~CC~ = 3.3 V, T~A~ = 25°C. I~off~ Supports Live
Insertion, Partial-Power-Down Mode, and Back-Drive Protection. Can Be
Used as a Down Translator to Translate Inputs From a Maximum of 5.5 V
Down to the V~CC~ Level. Latch-Up Performance Exceeds 100 mA Per
JESD 78, Class II.

## Pinout

```pinout pinout=p8
```

```pinout pinout=dsbga
```

## Pin functions

```pins
1A: Input for first OR gate
1B: Input for first OR gate
1Y: Output for first OR gate
2A: Input for second OR gate
2B: Input for second OR gate
2Y: Output for second OR gate
GND: Ground
VCC: Power
```

## Functional description

The SN74LVC2G32 provides two logical OR gates per device and each gate
has two inputs. Both input paths use identical circuitry for matching
propagation delays. Supply voltage from 1.65 V to 5.5 V is supported.

Conversion note: the logic diagram (positive logic, Figure 3) shows two
2-input OR gates, 1A (pin 1) and 1B (pin 2) to 1Y (pin 7), and 2A
(pin 5) and 2B (pin 6) to 2Y (pin 3).

### Feature description

The SN74LVC2G32 inputs per gate can accept up to 5.5 V regardless of
V~CC~.

## Function tables

```table id=function caption="Function table (each gate) [1]"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |

[1] Y = A + B in positive logic.
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
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
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output clamp-current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Definition | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 3 V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 3 V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | DCT and DCU packages | -40 | 125 | °C |
| ^ | ^ | YZP package | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC2G32 | < | < | Unit |
| ^ | ^ | DCT (SSOP), 8 pins | DCU (VSSOP), 8 pins | YZP (DSBGA), 8 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 220 | 227 | 102 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.6 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.6 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 17 | 17 | 17 | 19 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 2)"
| Symbol | From (input) | To (output) | Temperature | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | –40°C to 85°C | 2.4 | 8 | 1 | 4.4 | 1 | 3.8 | 1 | 3.2 | ns |
| ^ | ^ | ^ | –40°C to 125°C | 2.4 | 10 | 1 | 5.6 | 1 | 4.8 | 1 | 3.9 | ns |
```

## Typical characteristics

```curve id=tpd-vcc caption="Propagation delay vs. V~CC~, T~A~ = 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [1.5, 5]},
 "y": {"label": "Propagation Delay", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "tpHL", "points": [[1.8, 5.2], [2.0, 4.6], [2.25, 3.9], [2.5, 3.3], [3.0, 2.65], [3.5, 2.35], [4.0, 2.15], [4.5, 2.0], [5.0, 1.85]]},
  {"label": "tpLH", "points": [[1.8, 5.6], [2.0, 4.8], [2.25, 3.85], [2.5, 3.05], [3.0, 2.35], [3.5, 2.05], [4.0, 1.9], [4.5, 1.75], [5.0, 1.6]]}
 ]}
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

```table id=test-conditions caption="Test conditions (Figure 2)"
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
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

The SN74LVC2G32 device is dual 2-input OR gate. High-output current
capability is ideal for driving multiple outputs.

3-input OR configuration, Y = A + B + C

Conversion note: Figure 4 (3-input OR gate) shows the output of the first gate, with inputs A and B, connected to one input of the second gate, whose other input is C and whose output is Y.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. Outputs can be combined to produce higher drive
but the high drive will also create faster edges into light loads so
routing and load conditions should be considered to prevent ringing.

### Detailed design procedure

1. Recommended Input Conditions:
   - For rise time and fall time specifications, see (Δt/ΔV) in
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table.
   - For specified high and low levels, see (V~IH~ and V~IL~) in
     Recommended Operating Conditions table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~.
2. Recommend Output Conditions:
   - Load currents should not exceed 50 mA per output and 100 mA total
     for the part.
   - Series resistors on the output may be used if the user desires to
     slow the output edge signal or limit the output current.

### Application curve

```curve id=icc-freq caption="I~CC~ vs frequency, no load"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 40]},
 "y": {"label": "ICC", "unit": "mA", "range": [0, 5]},
 "series": [
  {"label": "VCC=5V", "points": [[0, 0], [10, 0.95], [20, 1.9], [30, 2.85], [40, 3.8]]},
  {"label": "VCC=3.3V", "points": [[0, 0], [10, 0.56], [20, 1.13], [30, 1.7], [40, 2.25]]},
  {"label": "VCC=2.5V", "points": [[0, 0], [10, 0.43], [20, 0.85], [30, 1.28], [40, 1.7]]},
  {"label": "VCC=1.8V", "points": [[0, 0], [10, 0.31], [20, 0.62], [30, 0.93], [40, 1.25]]}
 ]}
```

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in Recommended Operating Conditions table.

Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1-μF capacitor is
recommended. If there are multiple V~CC~ terminals then 0.01-μF or
0.022-μF capacitors are recommended for each power terminal. It is ok to
parallel multiple bypass capacitors to reject different frequencies of
noise. Multiple bypass capacitors may be paralleled to reject different
frequencies of noise. The bypass capacitor should be installed as close
to the power terminal as possible for the best results.

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states.

Specified in Figure 6 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or V~CC~,
whichever makes more sense or is more convenient.

Conversion note: the layout diagram (Figure 6) shows a gate with its unused input tied to V~CC~ and a gate with its unused input tied to ground.

## Package

```package footprint=SSOP-8
```

```package footprint=VSSOP-8
```

```package footprint=DSBGA-8-YZP
```

## Ordering information

```ordering
SN74LVC2G32DCTR | Reel of 3000 | -40 to 125 | 2WQ5, C32, R, Z
SN74LVC2G32DCTRE4 | 3000 | -40 to 125 | C32, R, Z
SN74LVC2G32DCTRG4 | Reel of 3000 | -40 to 125 | C32, R, Z
SN74LVC2G32DCUR | Reel of 3000 | -40 to 125 | C32J, C32Q, C32R, CR
SN74LVC2G32DCURE4 | Reel of 3000 | -40 to 125 | C32R
SN74LVC2G32DCURG4 | Reel of 3000 | -40 to 125 | C32R
SN74LVC2G32DCUT | Reel of 250 | -40 to 125 | C32J, C32Q, C32R, CR
SN74LVC2G32YZPR | Reel of 3000 | -40 to 85 | CGN
```

## Conversion notes

- Source pages 1 and 3–11 converted. The ordering lines are from the
  package option addendum (pp.13–14). The table of contents and revision
  history page (2), the support page (12), tape and reel (15–16),
  mechanical pages (17–25) and the notice page (26) are left out; the generated Package section
  replaces them.
- Dropped: applications list, device information table, the vendor
  pinout drawings, the NanoFree package and ESD protection feature
  lines; the logic diagram (Figure 3), the 3-input OR gate (Figure 4)
  and the layout diagram (Figure 6) are described in text. The other
  features are sentences in Description.
- The package option addendum lists SN74LVC2G32DCTRE4 with a quantity
  of 3000 and "null" as the carrier; its packing is written 3000.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- Curves digitized from Figure 1 and Figure 5 to about 2 % of full
  scale.
- From images: Y = A + B and the overbars of Y = !{!{A} • !{B}} (p.1;
  the text layer prints "Y + A ) B or Y + A • B").
- From images: ≤2 ns, ≤2.5 ns, 1 kΩ, 500 Ω, 50 Ω and V~Δ~ in the
  Figure 2 table and notes (p.8; the text layer has other glyphs).
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- Source issues:
  - Source error: § 9.1 (p.10) prints "device is dual 2-input OR gate";
    kept as printed.
  - Source error: § 10 (p.11) says twice, in two sentences, that
    multiple bypass capacitors may be paralleled to reject different
    frequencies of noise; both kept as printed.
