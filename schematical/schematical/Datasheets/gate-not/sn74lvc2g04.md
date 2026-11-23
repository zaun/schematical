---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g04",
  "title": "SN74LVC2G04 dual inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc2g04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES195N",
    "date": "2015-08",
    "pages": 31
  },
  "devices": ["gate:2G04"],
  "parts": ["SN74LVC2G04DBVR", "SN74LVC2G04DBVT", "SN74LVC2G04DCKR", "SN74LVC2G04DCKT", "SN74LVC2G04DRLR", "SN74LVC2G04YZPR"],
  "packages": {"p6": "DBV, DCK, DRL", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G04 dual inverter gate

## Description

This dual inverter is designed for 1.65-V to 5.5-V V~CC~ operation. The
SN74LVC2G04 device performs the Boolean function Y = !{A}. [[1::4]]

NanoFree package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[8::2]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::6]]

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V. Max
t~pd~ of 4.1 ns at 3.3 V. Low Power Consumption, 10-μA Max I~CC~.
±24-mA Output Drive at 3.3 V. Typical V~OLP~ (Output Ground Bounce)
<0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C. Typical V~OHV~ (Output V~OH~
Undershoot) >2 V at V~CC~ = 3.3 V, T~A~ = 25°C. I~off~ Supports
Partial-Power-Down Mode Operation. Latch-Up Performance Exceeds 100 mA
Per JESD 78, Class II. [[1::1]]

## Pinout

Conversion note: DBV, DCK and DRL share one pin order. Conversion note:
the source draws the YZP package in bottom view, and the drawing here is
generated from the catalog.

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
1A: Inverter 1 input
1Y: Inverter 1 output
2A: Inverter 2 input
2Y: Inverter 2 output
GND: Ground
VCC: Power
```
[[3::table #1]]

## Functional description

The SN74LVC2G04 contains two identical inverters that operate from
1.65-V to 5.5-V V~CC~. Each inverter has a balanced output capable of
outputting 32 mA at V~CC~ = 4.5 V. The overvoltage tolerant inputs allow
for down-translation of up to 6.5 V, and the partial power-off feature
ensures that the inputs and outputs can be any value from –0.5 V to
6.5 V when V~CC~ is 0 V [[8::1]]

Conversion note: the functional block diagram shows two inverters, 1A (pin 1) to 1Y (pin 6) and 2A (pin 3) to 2Y (pin 4).

### Feature description

NanoFree package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[8::2]]

This device supports 5-V V~CC~ operation and up to 5.5-V inputs. It has
a low propagation delay of only 4.1 ns at 3.3 V. [[8::2]]

Power consumption is low with only 10-μA Max I~CC~. Balanced drive
output at 3.3 V can put out ±24-mA. [[8::3]]

Typical output ground bounce is less than 0.8 V at 3.3-V V~CC~ and
typical output undershoot is greater than 2 V at 3.3-V V~CC~. [[8::4]]

This device supports partial-power-down mode operation. [[1::1]]

### Device functional modes

Table 1 lists the functional modes of the SN74LVC2G04. [[8::6]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[8::Table 1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
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
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | +2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | +1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
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
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC2G04 | < | < | < | Unit |
| ^ | ^ | DBV (SOT-23), 6 pins | DCK (SC70), 6 pins | DRL (SOT), 6 pins | YZP (DSBGA), 6 pins | ^ |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 165 | 259 | 142 | 123 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 μA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | ^ |
| I~I~ | A inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | μA |
| C~i~ | | V~I~ = V~CC~ or GND, –40°C to 85°C | 3.3 V | | 3.5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[5::table #3]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 14 | 14 | 14 | 16 | pF |
```
[[6::table #1]]

## Switching characteristics

Conversion note: Figure 2 is in [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 2)"
| Parameter | From (input) | To (output) | Test conditions | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | –40°C to 85°C | 3.1 | 8 | 1.5 | 4.4 | 1.2 | 4.1 | 1 | 3.2 | ns |
| ^ | ^ | ^ | –40°C to 125°C | 3.1 | 8 | 1.5 | 4.9 | 1.2 | 4.6 | 1 | 3.7 | ns |
```
[[6::table #2]]

## Typical characteristics

```curve id=voh-ioh caption="I~OH~ vs V~OH~ (V~CC~: 4.5 V, V~IH~: 3.10 V, V~IL~: 1.35 V, V~OH~ vs I~OH~ 1Y)"
{"x": {"label": "I_OH", "unit": "mA", "range": [-200, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [-1, 5]},
 "series": [
  {"label": "-40 C", "points": [[-200, 1.85], [-195, 1.85], [-180, 2.3], [-160, 2.75], [-120, 3.35], [-80, 3.8], [-40, 4.15], [0, 4.5]]},
  {"label": "25 C", "points": [[-200, -0.65], [-185, -0.55], [-180, -0.3], [-178, 0.5], [-170, 1.4], [-160, 2.0], [-140, 2.65], [-120, 3.05], [-80, 3.65], [-40, 4.1], [0, 4.5]]},
  {"label": "85 C", "points": [[-200, -0.65], [-175, -0.55], [-167, -0.35], [-165, 0], [-162, 0.8], [-155, 1.6], [-140, 2.35], [-120, 2.85], [-80, 3.5], [-40, 4.05], [0, 4.5]]}
 ]}
```
[[6::figure #1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[7::2]]

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
[[7::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[7::Figure 2]]

```table id=test-conditions caption="Test conditions (Figure 2)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[7::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[7::Figure 2]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality. [[9::1]]

The SN74LVC2G04 contains two logic inverters. It can be used in a wide
variety of applications, with this being one example. Because this part
has overvoltage tolerant inputs, it can be used for down translating
logic levels. This example explains the method used for down-translating
with this logic gate. [[9::2]]

Conversion note: the application schematic (Figure 3) shows input X on 1A (pin 1) with its inverse on 1Y (pin 6), input Y on 2A (pin 3) with its inverse on 2Y (pin 4), V~CC~ on pin 5 and ground on pin 2.

### Design requirements

The inputs, X and Y in Figure 3, to this device can be any value from
–0.5 V to 6.5 V, according to Absolute Maximum Ratings. Because the
input limits are not associated with V~CC~, down-translation is simple.
The output voltage is selected with V~CC~, and so long as the input
logic voltage is larger than V~IH~, found in Recommended Operating
Conditions, the output will trigger properly. [[9::3]]

### Detailed design procedure

1. Recommended Input Conditions [[9::4]]
   - For rise time and fall time specifications, see (Δt/ΔV) in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[9::4]]
   - For specified high and low levels, see (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table. [[9::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the Recommended Operating Conditions table at any
     valid V~CC~. [[9::4]]
2. Recommend Output Conditions [[9::4]]
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed total current (continuous current through V~CC~ or GND)
     for the part. These limits are located in the Absolute Maximum
     Ratings table. [[9::4]]
   - Outputs should not be pulled above V~CC~. [[9::4]]

### Application curve

There is a slight delay from input to output in addition to the voltage
change. Figure 4 shows the expected output of the SN74LVC2G04 when an
input is switched from 0 to 5 V and V~CC~ is set at 1.8 V. With V~CC~
set to 1.8 V, the output switches at 1.17 V (0.65 × V~CC~), and
therefore the input can be anything from 1.18 V up to 6.5 V and the
SN74LVC2G04 will work perfectly. [[10::1]]

```curve id=down-translation caption="Simulated voltage down-translation from 5-V input to 1.8-V output with t~pd~ = 3.4 ns"
{"x": {"label": "Time", "unit": "ns", "range": [0, 10]},
 "y": {"label": "Voltage", "unit": "V", "range": [-1, 6]},
 "series": [
  {"label": "VIN", "points": [[0, 0], [1.6, 0], [1.7, 2.5], [1.8, 5], [10, 5]]},
  {"label": "VOUT", "points": [[0, 1.8], [5.0, 1.8], [5.2, 0.9], [5.3, 0.1], [5.6, 0], [10, 0]]}
 ]}
```
[[10::Figure 4]]

### Power supply recommendations

The power supply can be any voltage between the min and max supply
voltage rating located in the Recommended Operating Conditions table. [[10::3]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply a 0.1-μF capacitor is
recommended and if there are multiple V~CC~ pins then a 0.01-μF or
0.022-μF capacitor is recommended for each power pin. It is ok to
parallel multiple bypass caps to reject different frequencies of noise.
0.1-μF and 1-μF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power pin as possible for
best results. [[10::4]]

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4 buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states. [[11::1]]

Specified in Figure 5 are the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or V~CC~,
whichever makes more sense or is more convenient. [[11::2]]

Conversion note: the layout diagram (Figure 5) shows a gate with its
unused input tied to V~CC~ and a gate with its unused input tied to
ground.

## Package

```package footprint=SOT23-6
```

```package footprint=SC70-6
```

```package footprint=SOT5X3-6
```

```package footprint=DSBGA-6-YZP
```

## Ordering information

```ordering
SN74LVC2G04DBVR | Reel of 3000 | -40 to 125 | C045, C04F, C04K, C04R
SN74LVC2G04DBVT | Reel of 250 | -40 to 125 | C045, C04F, C04K, C04R
SN74LVC2G04DCKR | Reel of 3000 | -40 to 125 | CC5, CCF, CCJ, CCK, CCR
SN74LVC2G04DCKT | Reel of 250 | -40 to 125 | CC5, CCF, CCJ, CCK, CCR
SN74LVC2G04DRLR | Reel of 4000 | -40 to 125 | 1K7, CC7, CCR
SN74LVC2G04YZPR | Reel of 3000 | -40 to 125 | CC7, CCN
```
[[13::table #1]]

## Conversion notes

- Source pages 1 and 3–12 converted. The ordering lines are from the
  package option addendum (pp.13–14). The table of contents and revision
  history page (2), tape and reel (15–18), mechanical pages (19–30) and
  the notice page (31) are left out; the generated Package section
  replaces them.
- Dropped: applications list, device information table, the vendor
  pinout drawings, the NanoFree package and ESD protection feature
  lines, the device and documentation support sections (p.12); the logic
  diagram and functional block diagram, the application schematic
  (Figure 3) and the layout diagram (Figure 5) are described in text.
  The other features are sentences in Description.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. The pulse-duration,
  setup-and-hold and enable/disable waveforms of Figure 2 and their
  notes B, E and F are left out (this inverter has no output enable).
- Curves digitized from Figures 1 and 4 to about 2 % of full scale.
- The addendum also lists the E4, G4 and .B orderables, which the
  catalog doesn't carry.
- From images: overbar of Y = !{A} (p.1), and V~Δ~ (p.7; the text layer
  prints "VD").
- Source issues: none found.
