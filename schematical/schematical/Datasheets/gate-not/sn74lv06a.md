---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv06a",
  "title": "SN74LV06A hex inverter buffers/drivers with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lv06a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES336K",
    "date": "2023-03",
    "pages": 28
  },
  "devices": ["gate:74_06"],
  "parts": ["SN74LV06AD", "SN74LV06ADBR", "SN74LV06ADGVR", "SN74LV06ADR", "SN74LV06ANSR", "SN74LV06APW", "SN74LV06APWR", "SN74LV06APWT"],
  "packages": {"p14": "D, DB, DGV, NS, PW"},
  "converted": "2026-11-21"
}
---

# SN74LV06A hex inverter buffers/drivers with open-drain outputs

## Description

These hex inverter buffers/drivers are designed for 2 V to 5.5 V V~CC~
operation. [[8::3]]

The SN74LV06A device performs the Boolean function Y = !{A} in positive
logic. [[1::5]]

The open-drain output require pull-up resistors to perform correctly and
can be connected to other open-drain outputs to implement active-low
wired-OR or active-high wired-AND functions. [[1::6]]

Outputs are disabled during power up and power down with inputs tied to
V~CC~. Support mixed-mode voltage operation on all ports. I~off~
supports live insertion, partial power down mode, and back drive
protection. Latch-up performance exceeds 100 mA per JESD 78, Class II. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Input 1
1Y: Output 1
2A: Input 2
2Y: Output 2
3A: Input 3
3Y: Output 3
4Y: Output 4
4A: Input 4
5Y: Output 5
5A: Input 5
6Y: Output 6
6A: Input 6
GND: Ground Pin
VCC: Power Pin
```
[[3::Table 5-1]]

## Functional description

These hex inverter buffers/drivers are designed for 2-V to 5.5-V V~CC~
operation. [[8::3]]

The SN74LV06A device performs the Boolean function Y = !{A} in positive
logic. [[1::5]]

The open-drain output require pull-up resistors to perform correctly and
can be connected to other open-drain outputs to implement active-low
wired-OR or active-high wired-AND functions. [[1::6]]

These devices are fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current back-flow through the devices when they are powered
down. [[8::6]]

Conversion note: the functional block diagram (Figure 8-1) shows one
inverter with an open-drain output, input A and output Y.

- Wide operating voltage range [[9::1]]
  - Operates from 2 V to 5.5 V [[9::1]]
- Allows up or down voltage translation [[9::1]]
  - Inputs and outputs accept voltages to 5.5 V [[9::1]]
- I~off~ feature [[9::1]]
  - Allows voltages on the inputs and outputs when V~CC~ is 0 V [[9::1]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input [1] | Output [2] |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = High Voltage Level, L = Low Voltage Level
[2] H = Driving High, L = Driving Low
```
[[9::Table 8-1]]

Source conflict: the function table's note gives the output state H as
"Driving High", while the overview says the open-drain output require
pull-up resistors to perform correctly. Both as printed.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | -35 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| T~J~ | Junction Temperature | | | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2500 | V |
| ^ | ^ | Machine Model (MM), per JEDEC specification | ±200 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±2000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 500-V HBM is possible with the necessary precautions.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 250-V CDM is possible with the necessary precautions.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | SN74LV06A | < | Unit |
| ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V |
| V~IL~ | Low level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | 5.5 | V |
| I~OL~ | Low level output current | V~CC~ = 2 V | | 20 [2] | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 8 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 16 | mA |
| Δt/Δv | Input transition rise and fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
[2] Unconfirmed: the electrical characteristics specify V~OL~ at I~OL~ = 50 µA for V~CC~ = 2 V to 5.5 V; kept as printed.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LV06A | < | < | < | < | Unit |
| ^ | ^ | D, 14 pins | DB, 14 pins | DGV, 14 pins | NS, 14 pins | PW, 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 100.6 | 112.5 | 135.2 | 95.4 | 128.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 51.8 | 65.0 | 57.9 | 52.9 | 57.2 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 54.9 | 59.9 | 68.3 | 51.2 | 70.7 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 25.0 | 25.0 | 9.2 | 17.9 | 9.3 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 54.7 | 59.3 | 67.6 | 53.8 | 70.0 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN74LV06A | < | < | –40°C to 85°C SN74LV06A | < | < | –40°C to 125°C SN74LV06A [1] | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OL~ | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | | 0.4 | | | 0.4 | V |
| ^ | I~OL~ = 8 mA | 3 V | | | 0.44 | | | 0.44 | | | 0.44 | V |
| ^ | I~OL~ = 16 mA | 4.5 V | | | 0.55 | | | 0.55 | | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | | | ±1 | | | ±1 | µA |
| I~OH~ | V~I~ = V~IL~, V~OH~ = V~CC~ | 5.5 V | | | ±2.5 | | | ±2.5 | | | ±2.5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | | | 20 | | | 20 | µA |
| I~off~ | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | | | 5 | | | 5 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 1.6 | | | 1.6 | | | 1.6 | | pF |

[1] Conversion note: the source leaves the Min/Typ/Max cells under this group blank; its values are printed in the same positions as those of the other two groups.
```
[[5::table #3]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter [1] | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.5 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 2.6 | pF |
| ^ | ^ | ^ | 5 V | 4.7 | pF |
```
[[6::table #2]]

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C SN74LV06A | < | –40°C to 125°C SN74LV06A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 5.4 [1] | 10.4 [1] | 1 [1] | 13 [1] | 1 | 14 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7.2 [1] | 10.4 [1] | 1 [1] | 13 [1] | 1 | 14 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 9.7 | 15.2 | 1 | 18 | 1 | 19 | ns |
| t~PHL~ | A | Y | ^ | | 9.3 | 15.2 | 1 | 18 | 1 | 19 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[5::table #4]]

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C SN74LV06A | < | –40°C to 125°C SN74LV06A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 4.1 [1] | 7.1 [1] | 1 [1] | 8.5 [1] | 1 | 9.5 | ns |
| t~PHL~ | A | Y | ^ | | 4.9 [1] | 7.1 [1] | 1 [1] | 8.5 [1] | 1 | 9.5 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 7.1 | 10.6 | 1 | 12 | 1 | 13 | ns |
| t~PHL~ | A | Y | ^ | | 6.4 | 10.6 | 1 | 12 | 1 | 13 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[2::table #1]] [[5::table #5]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C SN74LV06A | < | –40°C to 125°C SN74LV06A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 3 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 7 | ns |
| t~PHL~ | A | Y | ^ | | 3.3 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 7 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 4.8 | 7.5 | 1 | 8.5 | 1 | 9 | ns |
| t~PHL~ | A | Y | ^ | | 4.4 | 7.5 | 1 | 8.5 | 1 | 9 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

## Typical characteristics

```curve id=tpd-temp caption="Figure 6-1. TPD vs Temperature at 5 V"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 4]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 2.3], [0, 2.7], [25, 2.9], [50, 3.0], [85, 3.2], [125, 3.6]]}
 ]}
```
[[7::Figure 6-1]]

```curve id=tpd-vcc caption="Figure 6-2. TPD vs V~CC~ at 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[2.5, 5.8], [3.3, 4.1], [5, 2.9]]}
 ]}
```
[[7::Figure 6-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. [[8::1]]

Conversion note: C~L~ is 15 pF or 50 pF as given in the
[switching characteristics](#switching-characteristics).

```figure ref=load-ttl caption="Load circuit for open-drain outputs"
RL: 1 kΩ
CL: 15 pF or 50 pF
```
[[8::Figure 7-1]]

Conversion note: the voltage waveforms of Figure 7-1 (propagation delay
times) show an input pulse between 0 V and V~CC~ and the inverted
output; t~PHL~ is measured from 50% V~CC~ of the rising input to
50% V~CC~ of the falling output, and t~PLH~ from 50% V~CC~ of the
falling input to V~OL~ + 0.3 V of the output, which rises from V~OL~ to
≈V~CC~.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

The SN74LV06A is a low drive Open drain CMOS device that can be used for
a multitude of buffer type functions. The inputs are 5.5 V tolerant and
the outputs open drain and 5.5 V tolerant allowing it to translate up to
5.5 V or down to any other voltage between GND and 5.5 V. [[10::2]]

Conversion note: the typical application schematic (Figure 9-1) shows a
buffer function, in which two LV06A inverters, each driven by a µC or
logic, have their outputs tied together and to a pull-up resistor to
VPU as a wired OR to a µC or logic, and a basic LED driver, in which one
LV06A inverter drives an LED in series with a resistor to VPU.

### Design requirements

This device uses CMOS technology and is open drain so it has low output
drive only. Care should be taken to avoid bus contention because it can
drive currents that would exceed maximum limits. The parallel output
drive can create fast edges into light loads so routing and load
conditions should be considered to prevent ringing. [[10::3]]

### Detailed design procedure

1. Recommended Input Conditions: [[10::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     Recommended Operating Conditions table. [[10::4]]
   - For specified high and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~. [[10::4]]
2. Recommended Output Conditions: [[10::4]]
   - Load currents should not exceed 35 mA per output and 50 mA total
     for the part. [[10::4]]

Conversion note: the application curve (Figure 9-2, Output During Power
Up with 4 k Pull-up at 3.3 V) plots VOUT against V~CC~ from 0 to
3.3 V; the trace "VOUT input low" follows V~CC~ to 3.3 V and the trace
"VOUT input high" follows it to about 0.6 V and then falls to 0.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions. Each
V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1 μF capacitor is
recommended. If there are multiple V~CC~ terminals then 0.01 μF or
0.022 μF capacitor is recommended for each power terminal. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. 0.1 μF and 1 μF capacitors are commonly used in
parallel. The bypass capacitor should be installed as close to the power
terminal as possible for best results. [[11::2]]

### Layout guidelines

When using multiple bit logic devices, inputs should not float. [[11::3]]

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Specified
below are the rules that must be observed under all circumstances. All
unused inputs of digital logic devices must be connected to a high or
low bias to prevent them from floating. The logic level that should be
applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~ whichever make more
sense or is more convenient. It is generally OK to float outputs unless
the part is a transceiver. [[11::4]]

Conversion note: the layout diagram (Figure 9-3) shows one gate with its
unused input tied to V~CC~ and one with its unused input tied to
ground.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

## Ordering information

```ordering
SN74LV06AD | | -40 to 125 | LV06A | Obsolete
SN74LV06ADBR | Reel of 2000 | -40 to 125 | LV06A
SN74LV06ADGVR | Reel of 2000 | -40 to 125 | LV06A | NRND
SN74LV06ADR | Reel of 2500 | -40 to 125 | LV06A
SN74LV06ANSR | Reel of 2000 | -40 to 125 | 74LV06A
SN74LV06APW | | -40 to 125 | LV06A | Obsolete
SN74LV06APWR | Reel of 2000 | -40 to 125 | LV06A
SN74LV06APWT | | -40 to 125 | LV06A | Obsolete
```
[[13::table #1]]

## Conversion notes

- Source pages 1–12 converted (of page 12 nothing is kept); the package
  option addendum (pp.13–14), tape and reel and mechanical pages
  (15–28) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists (four feature lines are in
  Description), package information table, table of contents, revision
  history, the device and documentation support section, the
  simplified schematic (p.1), the vendor pinout drawing, the
  functional block diagram, the typical application schematic
  (Figure 9-1), the application curve (Figure 9-2) and the layout
  diagram (Figure 9-3), the last four each described in one sentence.
- The package option addendum also lists SN74LV06ADRE4, which the
  catalog does not carry.
- "Section 6.3" of the source is written as that section's name,
  Recommended Operating Conditions.
- The electrical characteristics table of the source gives symbols
  only, so it has no Parameter column. Its third value group has blank
  Min/Typ/Max cells in the source, and its C~i~ values are printed
  between the Min and Typ positions; they are under Typ here.
- The recommended operating conditions and the 3.3-V switching
  characteristics run over a page break in the source; each is one
  table here.
- The open-drain load circuit of Figure 7-1 (p.8) is drawn with the
  shared figure; its voltage waveforms are described in one sentence.
  The figure prints no C~L~ value.
- Curves digitized from Figures 6-1 and 6-2 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{A} (p.1 and p.8); the curve points of
  Figures 6-1 and 6-2 (p.7); 1 kΩ (R~L~), 50% V~CC~, V~OL~ + 0.3 V,
  ≈V~CC~ and 0 V (Figure 7-1, p.8); the labels LV06A and VPU of
  Figure 9-1 (p.10); the trace labels and the 0.6 V of Figure 9-2
  (p.11).
- Source issues:
  - Source error: "The open-drain output require pull-up resistors"
    (p.1 and p.8) lacks the plural; kept as printed.
  - Source conflict: the function table's note (p.9) gives the output
    state H as "Driving High", while the overview (p.8) says the
    open-drain output require pull-up resistors. Both kept.
  - Unconfirmed: the recommended operating conditions (p.4) give a
    maximum I~OL~ of 20 µA at V~CC~ = 2 V, while the electrical
    characteristics (p.5) specify V~OL~ at I~OL~ = 50 µA for V~CC~ =
    2 V to 5.5 V.
