---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g14",
  "title": "SN74LVC2G14 dual Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc2g14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES200O",
    "date": "2015-08",
    "pages": 28
  },
  "devices": ["gate:2G14"],
  "parts": ["SN74LVC2G14DBVRG4", "SN74LVC2G14DBVT", "SN74LVC2G14YZPR"],
  "packages": {"p6": "DBV, DCK", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G14 dual Schmitt-trigger inverter

## Description

This dual Schmitt-trigger inverter is designed for 1.65-V to 5.5-V
V~CC~ operation.

NanoFree package technology is a major breakthrough in IC packaging
concepts, using the die as the package.

The SN74LVC2G14 device contains two inverters and performs the Boolean
function Y = !{A}. The device functions as two independent inverters,
but because of Schmitt action, it may have different input threshold
levels for positive-going (V~T+~) and negative-going (V~T–~) signals.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V. Max
t~pd~ of 5.4 ns at 3.3 V. Low-Power Consumption, 10-μA Maximum I~CC~.
±24-mA Output Drive at 3.3 V. Typical V~OLP~ (Output Ground Bounce)
<0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C. Typical V~OHV~ (Output V~OH~
Undershoot) >2 V at V~CC~ = 3.3 V, T~A~ = 25°C. I~off~ Supports Live
Insertion, Partial-Power-Down Mode, and Back-Drive Protection. Latch-Up
Performance Exceeds 100 mA Per JESD 78, Class II.

## Pinout

Conversion note: DBV and DCK share one pin order. Conversion note: the source draws the YZP package in bottom view, and the drawing here is generated from the catalog.

```pinout pinout=p6
```

```pinout pinout=dsbga
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

## Functional description

The SN74LVC2G14 device contains two Schmitt Trigger Inverter and
performs the Boolean function Y = !{A}. The device functions as an
independent inverter, but because of Schmitt Trigger action, it will
have different input threshold levels for a positive-going (V~t+~) and
negative-going (V~t-~) signals.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuit disables the output, preventing damaging
current back-flow through the device when it is powered down.

Conversion note: the functional block diagram shows two Schmitt-trigger inverters, 1A on pin 1 to 1Y on pin 6 and 2A on pin 3 to 2Y on pin 4.

### Support translation down (5 V to 3.3 V; 3.3 V to 1.8 V)

As the inputs are 5.5-V tolerant, the device can be used as a down
translator. When the input voltage exceeds V~T+~ (Max), the output will
follow V~CC~, performing down-translation if the input voltage exceeds
V~CC~.

### Device functional modes

Table 1 lists the functional modes of the SN74LVC2G14.

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
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature [4] | | -65 | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output clamp-current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
[4] Unconfirmed: the source prints a minimum junction temperature, the same as the storage temperature's.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 500-V HBM is possible with the necessary precautions. Pins listed as ±XXX V may actually have higher performance.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 250-V CDM is possible with the necessary precautions. Pins listed as ±YYY V may actually have higher performance.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="See [1]"
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

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC2G14 | < | < | Unit |
| ^ | ^ | DBV (SOT23), 6 pins | DCK (SC70), 6 pins | YZP (DSBGA), 6 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 215 | 259 | 139 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 55 | 87 | 18 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 57 | 89 | N/A | °C/W |

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
| V~OH~ | | I~OH~ = –100 μA | 1.65 V to 4.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –4 mA | 1.65 V | 1.2 | | | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = –8 mA | 2.3 V | 1.9 | | | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = –16 mA | 3 V | 2.4 | | | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = –24 mA | 3 V | 2.3 | | | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = –32 mA | 4.5 V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.55 | ^ |
| I~I~ | A input | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | μA |
| C~I~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | | | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=cpd caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 16 | 17 | 18 | 21 | pF |
```

## Switching characteristics

Conversion note: Figure 3 is in [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics, –40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3.9 | 9.5 | 1.9 | 5.7 | 2 | 5.4 | 1.5 | 4.3 | ns |
```

```table id=sw-125 caption="Switching characteristics, –40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3.9 | 10.5 | 1.9 | 6.5 | 2 | 6 | 1.5 | 4.7 | ns |
```

## Typical characteristics

```curve id=tpd-temp caption="TPD across temperature at 3.3 V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [3.1, 3.8]},
 "series": [
  {"label": "TPD", "points": [[-35, 3.285], [-20, 3.42], [0, 3.56], [20, 3.68], [35, 3.71], [50, 3.69], [70, 3.62], [85, 3.6], [100, 3.63], [125, 3.75]]}
 ]}
```

```curve id=tpd-vcc caption="TPD across V~CC~ at 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 6.7], [2.5, 4.2], [3.3, 3.7], [5.0, 2.75]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output to switch S1, which goes to V~LOAD~, open or GND; the figure below draws the load with S1 open.
Conversion note: the source figure also shows pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

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

```table id=test-conditions caption="Test conditions (Figure 3)"
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
completeness. TI's customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

The SN74LVC2G14 device is a high-drive CMOS device that can be used for
a multitude of buffer type functions where the input is slow or noisy.
The device can produce 24 mA of drive current at 3.3 V, making it Ideal
for driving multiple outputs and good for high-speed applications up to
100 MHz. The inputs are 5.5-V tolerant allowing it to translate down to
V~CC~.

Conversion note: the typical application schematic (Figure 4) shows an inverter with a ~2.2MΩ feedback resistor and a 16 pF capacitor from its output to its input, ~32 pF from each side to ground, driving one channel of the SN74LVC2G14, which drives 50 pF.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing.

### Detailed design procedure

1. Recommended Input Conditions
   - Rise time and fall time specs. See (Δt/ΔV) in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table.
   - Specified high and low levels. See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the Recommended Operating Conditions table at any
     valid V~CC~.
2. Recommend Output Conditions
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed (continuous current through V~CC~ or GND) total current
     for the part. These limits are located in the Absolute Maximum
     Ratings table.
   - Outputs should not be pulled above V~CC~.

Source error: the Recommended Operating Conditions table (p.4) has no Δt/ΔV, V~IH~ or V~IL~ row; the input thresholds are V~T+~ and V~T–~ in the Electrical Characteristics (p.5).

### Application curve

```curve id=icc-freq caption="I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 80]},
 "y": {"label": "Icc", "unit": "mA", "range": [0, 10]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [20, 0.7], [40, 1.4], [60, 2.1], [75, 2.65]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [20, 1.0], [40, 2.05], [60, 3.1], [75, 3.95]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [20, 1.4], [40, 2.85], [50, 3.6], [60, 4.25], [70, 5.0], [75, 5.3]]},
  {"label": "Icc 5V", "points": [[0, 0], [20, 2.4], [35, 4.3], [45, 5.4], [55, 6.7], [60, 7.1], [70, 8.8], [75, 9.1]]}
 ]}
```

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the table. Each V~CC~ pin should have a
good bypass capacitor to prevent power disturbance. For devices with a
single supply, TI recommends a 0.1-μF capacitor. If there are multiple
V~CC~ pins, then TI recommends a 0.01-μF or 0.022-μF capacitor for each
power pin. It is ok to parallel multiple bypass capacitors to reject
different frequencies of noise. 0.1-μF and 1-μF capacitors are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices inputs should not ever float. In
many cases, functions or parts of functions of digital logic devices are
unused, for example, when only two inputs of a triple-input AND gate are
used or only 3 of the 4 buffer gates are used. Such input terminals
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. The
following rules must be observed under all circumstances:

- All unused inputs of digital logic devices must be connected to a high
  or low bias to prevent them from floating.
- The logic level that should be applied to any particular unused input
  depends on the function of the device. Generally they will be tied to
  GND or V~CC~ whichever make more sense or is more convenient.

Conversion note: the layout schematic (Figure 6) shows the 6-pin SN74LVC2G14 with a 0.1µF capacitor from V~CC~ (pin 5) to ground beside the V~DD~ supply, and the labels "Keep signal traces as short as possible" at 1A and "Unused inputs connected to GND or V~CC~ to prevent floating" at 2A.

## Package

```package footprint=SOT23-6
```

```package footprint=DSBGA-6-YZP
```

## Ordering information

```ordering
SN74LVC2G14DBVRG4 | Reel of 3000 | -40 to 125 | C14F, C14R
SN74LVC2G14DBVT | Reel of 250 | -40 to 125 | C145, C14F, C14K, C14R
SN74LVC2G14YZPR | Reel of 3000 | -40 to 125 | CF7, CFN
```

## Conversion notes

- Source pages 1 and 3–11 converted. The ordering lines are from the
  package option addendum (pp.13–14). The table of contents and revision
  history page (2), the support page (12), tape and reel (15–18),
  mechanical pages (19–27) and the notice page (28) are left out; the
  generated Package section replaces them.
- Dropped: applications list, device information table, the vendor
  pinout drawings, the NanoFree package and translation feature lines;
  the block diagram, the typical application schematic (Figure 4) and
  the layout schematic (Figure 6) are described in text. The other
  features are sentences in Description.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 3, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- The source's electrical characteristics table prints no parameter
  names from V~OH~ on, so those Parameter cells are empty except for
  I~I~.
- Curves digitized from Figures 1, 2 and 5 to about 2 % of full scale.
- From images: overbars of Y = !{A} (p.1 and p.8; the text layer prints
  "Y = A"); ≤2 ns, ≤2.5 ns, ≤ 10 MHz, 1 kΩ, 500 Ω, 50 Ω, V~Δ~ (p.7; the
  text layer prints "£2 ns", "1 kW", "500 W", "VD"); the axis numbers of
  Figure 1 (p.6) and the arrangement of Figures 4 and 6 (pp.9, 11).
- Source issues:
  - Unconfirmed: the Absolute Maximum Ratings (p.4) print a minimum
    junction temperature of -65 °C.
  - Source error: the detailed design procedure (p.9) points to Δt/ΔV,
    V~IH~ and V~IL~ in the Recommended Operating Conditions table, which
    has no such rows (p.4).
