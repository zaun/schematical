---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g00",
  "title": "SN74LVC2G00 dual 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lvc2g00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES193N",
    "date": "2015-01",
    "pages": 26
  },
  "devices": ["gate:2G00"],
  "parts": ["SN74LVC2G00DCT3", "SN74LVC2G00DCTR", "SN74LVC2G00DCTRE4", "SN74LVC2G00DCUR", "SN74LVC2G00DCUT", "SN74LVC2G00DCUTG4", "SN74LVC2G00YZPR"],
  "packages": {"p8": "DCT, DCU", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G00 dual 2-input positive-NAND gate

## Description

This dual 2-input positive-NAND gate is designed for 1.65-V to 5.5-V
V~CC~ operation. [[1::1]]

The SN74LVC2G00 device performs the Boolean function Y = !{A × B} or
Y = !{A} + !{B} in positive logic. [[1::4]]

NanoFree package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[1::5]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[9::1]]

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V. Max
t~pd~ of 4.3 ns at 3.3 V. Low Power Consumption, 10-μA Max I~CC~.
±24-mA Output Drive at 3.3 V. Typical V~OLP~ (Output Ground Bounce)
< 0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C. Typical V~OHV~ (Output V~OH~
Undershoot) > 2 V at V~CC~ = 3.3 V, T~A~ = 25°C. I~off~ Supports Live
Insertion, Partial Power Down Mode, and Back Drive Protection. Latch-Up
Performance Exceeds 100 mA Per JESD 78, Class II. [[1::2]]

## Pinout

```pinout pinout=p8
```

```pinout pinout=dsbga
```

## Pin functions

```pins
1A: A input for gate 1
1B: B input for gate 1
2Y: Output for gate 2
GND: Ground
2A: A input for gate 2
2B: B input for gate 2
1Y: Output for gate 1
VCC: Power input.
```
[[3::table #1]]

## Functional description

The SN74LVC2G00 device contains two 2-input positive-NAND gates and
performs the Boolean function Y = !{A × B} or Y = !{A} + !{B} on each
gate. This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down. [[9::1]]

Conversion note: the functional block diagram shows two 2-input NAND
gates, 1A (pin 1) and 1B (pin 2) to 1Y (pin 7), and 2A (pin 5) and 2B
(pin 6) to 2Y (pin 3).

### Feature description

- Wide operating voltage range. [[9::2]]
  - Operates from 1.65 V to 5.5 V [[9::2]]
- Allows down voltage translation [[9::2]]
  - Inputs accept voltages to 5.5 V [[1::2]]
- I~off~ feature [[9::2]]
  - Allows voltages on the inputs and outputs, when V~CC~ is 0 V [[9::2]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[9::Table 1]]

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
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output clamp-current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | 1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

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
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC1G00 [2] | < | < | Unit |
| ^ | ^ | DCT, 5 pins [2] | DCU, 5 pins [2] | YZP, 5 pins [2] | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 220 | 227 | 102 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
[2] Source error: the table's header names the SN74LVC1G00 and 5 pins; this data sheet is the SN74LVC2G00's, whose DCT, DCU and YZP packages have 8 pins. Kept as printed.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 μA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.55 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | μA |
| C~I~ | | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[6::table #1]]

```table id=cpd caption="Electrical characteristics (continued)" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 19 | 19 | 20 | 22 | pF |
```
[[6::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics, -40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 3.7 | 8.6 | 1.6 | 4.8 | 1.1 | 4.3 | 1 | 3.3 | ns |
```
[[6::table #3]]

```table id=sw-125 caption="Switching characteristics, -40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 3.7 | 9.4 | 1.6 | 5.5 | 1.1 | 4.9 | 1 | 3.8 | ns |
```
[[6::table #4]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature at 3.3-V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.7], [-40, 2.9], [0, 3.0], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```
[[7::Figure 1]]

```curve id=tpd-vcc caption="TPD vs V~CC~ at 25°C"
{"x": {"label": "Vcc", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 7.0], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
 ]}
```
[[7::Figure 2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[8::1]]

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
[[8::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[8::Figure 3]]

```table id=test-conditions caption="Test conditions (Figure 3)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[8::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[8::Figure 3]]

## Application information

SN74LVC2G00 is a high-drive CMOS device that can be used for
implementing NAND logic with a high output drive, such as an LED
application. It can produce 24 mA of drive current at 3.3 V, making it
Ideal for driving multiple outputs and good for high speed applications
up to 100 MHz. The inputs are 5.5-V tolerant, allowing it to translate
down to V~CC~. [[10::1]]

Conversion note: the typical application drawing shows the 8-pin device
on a 1.65 V to 5.5 V supply, with the inputs of each gate driven from a
microcontroller or logic.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing. [[10::2]]

### Detailed design procedure

1. Recommended Input Conditions [[10::3]]
   - For rise time and fall time specifications, see (Δt/ΔV) in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[10::3]]
   - For specified high and low levels, see (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table. [[10::3]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the Recommended Operating Conditions table at any
     valid V~CC~. [[10::3]]
2. Recommend Output Conditions [[10::3]]
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed total current (continuous current through V~CC~ or GND)
     for the part. These limits are located in the Absolute Maximum
     Ratings table. [[10::3]]
   - Outputs should not be pulled above V~CC~. [[10::3]]

### Application curves

```curve id=icc-freq caption="I~CC~ vs frequency (each gate)"
{"x": {"label": "Frequency", "unit": "MHz", "range": [-20, 80]},
 "y": {"label": "ICC", "unit": "mA", "range": [-2, 10]},
 "series": [
  {"label": "VCC 1.8 V", "points": [[0, 0], [10, 0.3], [40, 1.1], [60, 1.6], [75, 2.0]]},
  {"label": "VCC 2.5 V", "points": [[0, 0], [40, 1.8], [60, 2.4], [70, 2.8], [75, 3.5]]},
  {"label": "VCC 3.3 V", "points": [[0, 0], [30, 1.9], [50, 3.0], [55, 3.7], [60, 3.9], [75, 5.1]]},
  {"label": "VCC 5 V", "points": [[0, 0], [25, 2.9], [30, 3.8], [50, 6.0], [55, 6.9], [70, 8.3], [75, 9.2]]}
 ]}
```
[[11::Figure 4]]

### Power supply recommendations

The power supply can be any voltage between the min and max supply
voltage rating located in the Recommended Operating Conditions table. [[11::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply a 0.1-μF capacitor is
recommended and if there are multiple V~CC~ pins then a 0.01-μF or
0.022-μF capacitor is recommended for each power pin. It is ok to
parallel multiple bypass caps to reject different frequencies of noise.
0.1-μF and 1-μF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power pin as possible for
best results. [[11::2]]

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4 buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states. [[12::1]]

Specified in Figure 5 are the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or V~CC~,
whichever makes more sense or is more convenient. [[12::2]]

Conversion note: the layout diagram (Figure 5) shows a gate with its
unused input tied to V~CC~ and a gate with its unused input tied to
ground.

## Package

```package footprint=SSOP-8
```

```package footprint=VSSOP-8
```

```package footprint=DSBGA-8-YZP
```

## Ordering information

```ordering
SN74LVC2G00DCT3 | | -40 to 125 | C00, Z | Obsolete
SN74LVC2G00DCTR | Reel of 3000 | -40 to 125 | 2WH5, C00, R, Z
SN74LVC2G00DCTRE4 | 3000 | -40 to 125 | C00, R, Z
SN74LVC2G00DCUR | Reel of 3000 | -40 to 125 | C00J, C00Q, C00R
SN74LVC2G00DCUT | Reel of 250 | -40 to 125 | C00J, C00Q, C00R
SN74LVC2G00DCUTG4 | Reel of 250 | -40 to 125 | C00R
SN74LVC2G00YZPR | Reel of 3000 | -40 to 125 | CAN
```
[[13::table #1]]

## Conversion notes

- Source pages 1 and 3–12 converted. The ordering lines are from the
  package option addendum (pp.13–14). The table of contents and revision
  history page (2), tape and reel (15–16), mechanical pages (17–25) and
  the notice page (26) are left out; the generated Package section
  replaces them.
- Dropped: applications list, device information table, the vendor
  pinout drawings, the NanoFree package and ESD protection feature
  lines, the trademarks, ESD caution and glossary sections (p.12); the
  simplified schematic and functional block diagram, the typical
  application drawing and the layout diagram (Figure 5) are described in
  text. The other features are sentences in Description.
- The package option addendum lists SN74LVC2G00DCTRE4 with a quantity
  of 3000 and "null" as the carrier; its packing is written 3000.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- Curves digitized from Figures 1, 2 and 4 to about 2 % of full scale.
- From images: overbars of Y = !{A × B} and Y = !{A} + !{B} (p.1 and
  p.9; the text layer prints "Y = A × B or Y = A + B"), and ≤2 ns,
  ≤2.5 ns, ≤ 10 MHz, 1 kΩ, 500 Ω, 50 Ω, V~Δ~ (p.8; the text layer
  prints "£2 ns", "1 kW", "500 W", "VD").
- Source issues:
  - Source error: the Thermal Information table (p.5) is headed
    "SN74LVC1G00" with "5 PINS" under each package; the data sheet is
    for the 8-pin SN74LVC2G00. Kept as printed.
  - Source error: the note under Recommended Operating Conditions (p.5)
    has no marker in the table; the marker is put on the caption.
