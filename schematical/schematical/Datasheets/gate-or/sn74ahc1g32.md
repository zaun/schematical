---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g32",
  "title": "SN74AHC1G32 single 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ahc1g32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS317Q",
    "date": "2024-04",
    "pages": 26
  },
  "devices": ["gate:1G32"],
  "parts": ["SN74AHC1G32DBVR", "SN74AHC1G32DBVRE4", "SN74AHC1G32DBVRG4", "SN74AHC1G32DBVT", "SN74AHC1G32DBVTG4", "SN74AHC1G32DCK3", "SN74AHC1G32DCKR", "SN74AHC1G32DCKRE4", "SN74AHC1G32DCKRG4", "SN74AHC1G32DCKTE4", "SN74AHC1G32DCKTG4", "SN74AHC1G32DRLR", "SN74AHC1G32DRLRG4"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G32 single 2-input positive-OR gate

## Description

The SN74AHC1G32 device is a single 2-input positive-OR gate. The device
performs the Boolean function Y = A + B or Y = !{!{A} • !{B}} in
positive logic. [[1::1]]

Operating range of 2V to 5.5V. Max t~pd~ of 6.5ns at 5V. Low power
consumption, 10µA max I~CC~. ±8mA output drive at 5V. Schmitt-trigger
action at all inputs makes the circuit tolerant for slower input rise
and fall time. Latch-up performance exceeds 250mA per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground Pin
Y: Output Y
VCC: Power Pin
```
[[3::Table 4-1]]

## Functional description

The SN74AHC1G32 device is a single 2-input positive OR gate with low
drive that produces slow rise and fall times. This reduces ringing on
the output signal. The device also has Schmitt-trigger action that will
allow for slower or noisier inputs. The input signals are high impedance
when V~CC~ = 0 V. [[8::1]]

Conversion note: the functional block diagram shows one 2-input OR gate.

### Feature description

- Wide operating voltage [[8::2]]
  - Operates from 2 V to 5.5 V [[8::2]]
- Allows down voltage translation [[8::2]]
  - Accepts input voltages to 5.5 V [[8::2]]

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output [2] |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```
[[8::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Maximum junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -60 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±1500 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input transition rise and fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G32: thermal metric [1]"
| Symbol | Parameter | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT) 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 293.4 | 328.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 208.8 | 105.1 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 180.6 | 150.3 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 120.6 | 6.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 179.5 | 148.4 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40 to +80°C | < | –40 to +125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | V |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.44 | | 0.44 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | | 0.44 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 10 | µA |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | | 10 | pF |
```
[[5::table #2]]

Source conflict: this table heads its second temperature group
–40 to +80°C, while the switching characteristics tables head it
–40°C to +85°C; both are kept as printed.

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 14 | pF |
```
[[6::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to +85°C | < | < | –40°C to +125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5.5 | 7.9 | 1 | | 9.5 | 1 | | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | | 9.5 | 1 | | 10 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 8 | 11.4 | 1 | | 13 | 1 | | 14 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8 | 11.4 | 1 | | 13 | 1 | | 14 | ns |
```
[[5::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to +85°C | < | < | –40°C to +125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 3.8 | 5.5 | 1 | | 6.5 | 1 | | 7 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.8 | 5.5 | 1 | | 6.5 | 1 | | 7 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.3 | 7.5 | 1 | | 8.5 | 1 | | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.3 | 7.5 | 1 | | 8.5 | 1 | | 9.5 | ns |
```
[[6::table #2]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.8], [-40, 4.9], [0, 5.2], [25, 5.4], [85, 5.8], [125, 6.1]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=tpd-vcc caption="TPD vs V~CC~"
{"x": {"label": "V_CC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD in ns", "points": [[2, 7], [3.3, 5.4], [5, 3.9]]}
 ]}
```
[[6::Figure 5-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. Conversion note: C~L~ is
15 pF or 50 pF, as given in the
[switching characteristics](#switching-characteristics). [[7::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[7::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

The SN74AHC1G32 is a low-drive CMOS device that can be used for a
multitude of bus-interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The inputs can except voltages to 5.5 V at
any valid V~CC~ making it ideal for down translation. [[9::2]]

Conversion note: the specific application schematic (Figure 8-1) shows the gate, supplied from a 3.3 V - 5 V regulated rail with a 0.1 µF capacitor to ground, between a 5-V driver and a µC.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive will also create fast edges into
light loads so routing and load conditions should be considered to
prevent ringing. [[9::3]]

Source conflict: Overview (§ 7.1) and Application Information (§ 8.1) say low drive and slow edges, Design Requirements (§ 8.2.1) says "the high drive will also create fast edges"; both are kept as printed.

### Detailed design procedure

- Recommended input conditions [[9::4]]
  - Specified high and low levels. See (V~IH~ and V~IL~) in the
    [Recommended Operating Conditions](#recommended-operating-conditions)
    table. [[9::4]]
  - Inputs are overvoltage tolerant allowing them to go as high as
    5.5 V at any valid V~CC~ [[9::4]]
- Recommended output conditions [[9::4]]
  - Load currents should not exceed 25 mA per output and 50 mA total
    for the part [[9::4]]
  - Outputs should not be pulled above V~CC~ [[9::4]]

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[10::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μf is recommended; if
there are multiple V~CC~ pins, then 0.01 μf or 0.022 μf is recommended
for each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μf and a 1 μf are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results. [[10::2]]

### Layout guidelines

When using multiple-bit logic devices, inputs should never float. [[10::3]]

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Figure 8-3
specifies the rules that must be observed under all circumstances. All
unused inputs of digital logic devices must be connected to a high or
low bias to prevent them from floating. The logic level that should be
applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~, whichever makes
more sense or is most convenient. It is generally acceptable to float
outputs, unless the part is a transceiver. If the transceiver has an
output enable pin, it will disable the output section of the part when
asserted. This will not disable the input section of the I/Os, so they
cannot float when disabled. [[10::4]]

Conversion note: the layout diagram (Figure 8-3) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
SN74AHC1G32DBVR | Reel of 3000 | -40 to 125 | 39CH, 3C3F, A323, A32G, A32J, A32L, A32S
SN74AHC1G32DBVRE4 | Reel of 3000 | -40 to 125 | A32G
SN74AHC1G32DBVRG4 | Reel of 3000 | -40 to 125 | A32G
SN74AHC1G32DBVT | | -40 to 125 | A323, A32G, A32J, A32S | Obsolete
SN74AHC1G32DBVTG4 | | -40 to 125 | A32G | Obsolete
SN74AHC1G32DCK3 | | -40 to 85 | AGY | Obsolete
SN74AHC1G32DCKR | Reel of 3000 | -40 to 125 | 1RA, AG3, AGG, AGJ, AGL, AGS
SN74AHC1G32DCKRE4 | Reel of 3000 | -40 to 125 | AG3
SN74AHC1G32DCKRG4 | Reel of 3000 | -40 to 125 | AG3
SN74AHC1G32DCKTE4 | Reel of 250 | -40 to 125 | AG3
SN74AHC1G32DCKTG4 | Reel of 250 | -40 to 125 | AG3
SN74AHC1G32DRLR | Reel of 4000 | -40 to 125 | AGB, AGS
SN74AHC1G32DRLRG4 | Reel of 4000 | -40 to 125 | AGB, AGS
```
[[13::table #1]]

## Conversion notes

- Source pages 1–10 converted. The ordering lines are from the package
  option addendum (pp.13–14). The support and revision history page
  (11), the mechanical information page (12), tape and reel and
  mechanical pages (15–26) are left out; the generated Package section
  replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the
  simplified schematic and functional block diagram (one OR gate); the
  specific application schematic (Figure 8-1) and the layout diagram
  (Figure 8-3) are described in text.
- Dropped: Figure 8-2 (switching characteristics comparison, an
  oscilloscope-style plot of V~I~ and the AC, HC and AHC outputs); it is
  not a characteristic of this part alone.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for Figure 5-1 and no load
  or temperature for Figure 5-2.
- From images: the overbars of Y = !{!{A} • !{B}} (p.1: a bar over A, a
  bar over B and one bar over the whole product); the curve points -55,
  4.8, 4.9, 5.2, 5.4, 85, 5.8, 6.1 (Figure 5-1, p.6) and 3.3, 5.4, 3.9
  (Figure 5-2, p.6).
- Source issues:
  - Source conflict: the electrical characteristics table heads its
    second temperature group –40 to +80°C, the switching tables –40°C
    to +85°C (Electrical characteristics note).
  - Source conflict: low drive and slow edges (§ 7.1, § 8.1) vs "high
    drive ... fast edges" (§ 8.2.1) (Application information note).
  - Source error: "The inputs can except voltages" (§ 8.1) is printed
    for "accept"; kept as printed.
  - Source error: the unit of V~OL~ in the electrical characteristics
    table is printed as a lowercase v; written V.
  - Source error: the bypass capacitor values (§ 8.3) are printed with
    the unit μf; kept as printed.
  - Unconfirmed: the absolute maximum ratings give a storage
    temperature minimum of –60 °C.
