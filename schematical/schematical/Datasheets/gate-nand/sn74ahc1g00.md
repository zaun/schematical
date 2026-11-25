---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g00",
  "title": "SN74AHC1G00 single 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74ahc1g00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS313Q",
    "date": "2024-01",
    "pages": 28
  },
  "devices": ["gate:1G00"],
  "parts": ["SN74AHC1G00DBVR", "SN74AHC1G00DBVRG4", "SN74AHC1G00DBVT", "SN74AHC1G00DCK3", "SN74AHC1G00DCKR", "SN74AHC1G00DCKRE4", "SN74AHC1G00DCKRG4", "SN74AHC1G00DCKT", "SN74AHC1G00DCKTG4", "SN74AHC1G00DRLR"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G00 single 2-input positive-NAND gate

## Description

The SN74AHC1G00 performs the Boolean function Y = !{A • B} or
Y = !{A} + !{B} in positive logic. [[1::1]]

Operating range: 2 V to 5.5 V. Maximum t~pd~ of 6.5 ns at 5 V. Low power
consumption: maximum I~CC~ of 10 μA. ±8-mA output drive at 5 V. Schmitt
trigger action at all inputs makes the circuit tolerant for slower input
rise and fall time. Latch-up performance exceeds 250 mA per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: A input
B: B input
GND: Ground
Y: Output
VCC: Power
```
[[3::Table 4-1]]

## Functional description

The SN74AHC1G00 device performs the NAND Boolean function Y = !{A × B}
or Y = !{A} + !{B} in positive logic. The device has a wide operating
range of V~CC~ from 2 V to 5 V. [[9::1]]

Conversion note: the functional block diagram (Figure 7-1) is the logic diagram of one 2-input NAND gate, inputs A (pin 1) and B (pin 2), output Y (pin 4).

### Feature description

The SN74AHC1G00 device has wide operating voltage range for logic system
from 2 V to 5 V. The low propagation delay allows fast switching and
higher speeds of operation. In addition, the low power consumption of
10-uA (maximum) makes this device a good choice for portable and battery
power-sensitive applications. The Schmitt trigger action on all inputs
have noise rejection capabilities. [[9::2]]

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output [2] |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Maximum junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G00"
| Symbol | Thermal metric [1] | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT) 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 256 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | 130 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | 152 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | 9.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | 152 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bot) thermal resistance | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter [1] | Test conditions | < | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = –50 µA | T~A~ = 25°C | 2 V | 1.9 | 2 | | V |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 1.9 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | 1.9 | | | ^ |
| ^ | ^ | ^ | T~A~ = 25°C | 3 V | 2.9 | 3 | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 2.9 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | 2.9 | | | ^ |
| ^ | ^ | ^ | T~A~ = 25°C | 4.5 V | 4.4 | 4.5 | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 4.4 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | 4.4 | | | ^ |
| ^ | ^ | I~OH~ = –4 mA | T~A~ = 25°C | 3 V | 2.58 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 2.48 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | 2.48 | | | ^ |
| ^ | ^ | I~OH~ = –8 mA | T~A~ = 25°C | 4.5 V | 3.94 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 3.8 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | 3.8 | | | ^ |
| V~OL~ | Low level output voltage | I~OL~ = 50 µA | T~A~ = 25°C | 2 V | | | 0.1 | V |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.1 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | | | 0.1 | ^ |
| ^ | ^ | ^ | T~A~ = 25°C | 3 V | | | 0.1 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.1 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | | | 0.1 | ^ |
| ^ | ^ | ^ | T~A~ = 25°C | 4.5 V | | | 0.1 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.1 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 4 mA | T~A~ = 25°C | 3 V | | | 0.36 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.44 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | | | 0.44 | ^ |
| ^ | ^ | I~OL~ = 8 mA | T~A~ = 25°C | 4.5 V | | | 0.36 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.44 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | | | 0.44 | ^ |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | T~A~ = 25°C | 0 V to 5.5 V | | | ±0.1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | ±1 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | | | ±1 | ^ |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | T~A~ = 25°C | 5.5 V | | | 1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 10 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | | | 10 | ^ |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | T~A~ = 25°C | 5 V | | 2 | 10 | pF |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 10 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +125°C | ^ | | | 10 | ^ |

[1] Recommended T~A~ = –40°C to +125°C
```
[[5::table #3]] [[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | | 9.5 | | pF |
```
[[7::table #1]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics: V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ [1] | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 25°C | | 5.5 | 7.9 | ns |
| ^ | ^ | ^ | ^ | –40°C to +85°C | 1 | | 9.5 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +125°C | 1 | | 10.5 | ^ |
| t~PHL~ | ^ | ^ | ^ | 25°C | | 5.5 | 7.9 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +85°C | 1 | | 9.5 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +125°C | 1 | | 10.5 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 25°C | | 8 | 11.4 | ns |
| ^ | ^ | ^ | ^ | –40°C to +85°C | 1 | | 13 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +125°C | 1 | | 14 | ^ |
| t~PHL~ | ^ | ^ | ^ | 25°C | | 8 | 11.4 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +85°C | 1 | | 13 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +125°C | 1 | | 14 | ^ |

[1] Recommended T~A~ = –40°C to +125°C
```
[[6::table #2]]

```table id=sw-5v caption="Switching characteristics: V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ [1] | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 25°C | | 3.7 | 5.5 | ns |
| ^ | ^ | ^ | ^ | –40°C to +85°C | 1 | | 6.5 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +125°C | 1 | | 7 | ^ |
| t~PHL~ | ^ | ^ | ^ | 25°C | | 3.7 | 5.5 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +85°C | 1 | | 6.5 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +125°C | 1 | | 7 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 25°C | | 5.2 | 7.5 | ns |
| ^ | ^ | ^ | ^ | –40°C to +85°C [2] | 1 | | 6.5 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +125°C | 1 | | 9 | ^ |
| t~PHL~ | ^ | ^ | ^ | 25°C | | 5.2 | 7.5 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +85°C [2] | 1 | | 6.5 | ^ |
| ^ | ^ | ^ | ^ | –40°C to +125°C | 1 | | 9 | ^ |

[1] Recommended T~A~ = –40°C to +125°C
[2] Unconfirmed: the source prints a maximum of 6.5 ns for –40°C to +85°C at C~L~ = 50 pF, below the 7.5 ns maximum it prints for 25°C on the row above.
```
[[7::table #2]]

## Typical characteristics

```curve id=tpd-temp-50pf caption="Propagation Delay vs Temperature, C~L~ = 50 pF"
{"x": {"label": "Temperature", "unit": "°C", "range": [0, 125]},
 "y": {"label": "tPLH/HL(max)", "unit": "ns", "range": [4, 14]},
 "series": [
  {"label": "Vcc=3.3 +/-0.3 V, A/B to Y, CL = 50 pF", "points": [[25, 11.4], [50, 12.1], [85, 13], [125, 13.8]]},
  {"label": "Vcc=5 +/-0.5 V, A/B to Y, CL = 50 pF", "points": [[25, 7.5], [50, 7.6], [85, 8.2], [125, 9.2]]}
 ]}
```
[[7::Figure 5-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[8::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[8::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[7::Figure 5-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

Conversion note: the typical application (Figure 8-1) shows two cross-coupled NAND gates, with inputs S and R and outputs Q and !{Q}.

### Design requirements

This SN74AHC1G00 device uses CMOS technology and has balanced output
drive. Take care to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive also creates fast edges
into light loads. Routing and load conditions must be considered to
prevent ringing. [[10::2]]

### Detailed design procedure

- Recommended input conditions: [[10::3]]
  - Specified high and low levels. See V~IH~ and V~IL~ in
    [Recommended Operating Conditions](#recommended-operating-conditions). [[10::3]]
  - Inputs are overvoltage tolerant allowing them to go as high as 5.5 V
    at any valid V~CC~. [[10::3]]
- Recommended output conditions: [[10::3]]
  - Load currents must not exceed 25 mA per output and 50 mA total for
    the part. [[10::3]]
  - Outputs should not be pulled above V~CC~. [[10::3]]

### Application curve

```curve id=tpd-temp-15pf caption="Propagation Delay vs Temperature, C~L~ = 15 pF"
{"x": {"label": "Temperature", "unit": "°C", "range": [0, 125]},
 "y": {"label": "tPLH/HL(max)", "unit": "ns", "range": [2, 12]},
 "series": [
  {"label": "Vcc=3.3 +/-0.3 V, A/B to Y, CL = 15 pF", "points": [[25, 8], [50, 8.6], [85, 9.5], [125, 10.5]]},
  {"label": "Vcc=5 +/-0.5 V, A/B to Y, CL = 15 pF", "points": [[25, 5.4], [50, 5.8], [85, 6.5], [125, 7.2]]}
 ]}
```
[[7::Figure 5-1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the
[Recommended Operating Conditions](#recommended-operating-conditions). [[11::1]]

Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, TI recommends a 0.1-μF
capacitor; if there are multiple V~CC~ terminals, then TI recommends a
0.01-μF or 0.022-μF capacitor for each power terminal. Multiple bypass
capacitors can be paralleled to reject different frequencies of noise.
Frequencies of 0.1 μF and 1 μF are commonly used in parallel. The bypass
capacitor must be installed as close as possible to the power terminal
for best results. [[11::2]]

### Layout guidelines

When using multiple bit logic devices inputs must not ever float. [[12::1]]

In many cases, functions or parts of functions of digital logic devices
are unused. For example, when only two inputs of a triple-input AND gate
are used or only three of the four buffer gates are used. Such input
pins must not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. The
following are the rules must be observed under all circumstances. [[12::2]]

All unused inputs of digital logic devices must be connected to a high
or low bias to prevent them from floating. The logic level that should
be applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~ whichever make more
sense or is more convenient. Floating outputs is generally acceptable,
unless the part is a transceiver. If the transceiver has an output
enable pin, it disables the outputs section of the part when asserted.
This does not disable the input section of the input and output, so they
also cannot float when disabled. [[12::3]]

Conversion note: the layout recommendation (Figure 8-3) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
SN74AHC1G00DBVR | Reel of 3000 | -40 to 125 | 35XH, 3BSF, A003, A00G, A00J, A00L, A00S
SN74AHC1G00DBVRG4 | Reel of 3000 | -40 to 125 | A00G
SN74AHC1G00DBVT | | -40 to 125 | A003, A00G, A00J, A00S | Obsolete
SN74AHC1G00DCK3 | | -40 to 125 | AAY | Obsolete
SN74AHC1G00DCKR | Reel of 3000 | -40 to 125 | 1QP, AA3, AAG, AAJ, AAL, AAS
SN74AHC1G00DCKRE4 | Reel of 3000 | -40 to 125 | AA3
SN74AHC1G00DCKRG4 | Reel of 3000 | -40 to 125 | AA3
SN74AHC1G00DCKT | | -40 to 125 | AA3, AAG, AAJ, AAS | Obsolete
SN74AHC1G00DCKTG4 | Reel of 250 | -40 to 125 | AA3
SN74AHC1G00DRLR | Reel of 4000 | -40 to 125 | AAB, AAS
```
[[15::table #1]]

## Conversion notes

- Source pages 1–12 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.15–16). The support and revision history page (13), the
  mechanical information page (14), tape and reel and mechanical pages
  (17–28) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the logic
  diagram (one NAND gate); the typical application (Figure 8-1) and the
  layout recommendation (Figure 8-3) are described in text.
- Section 5.3, named in the detailed design procedure and the power
  supply recommendations, is the source's Recommended Operating
  Conditions.
- The recommended operating conditions table of the source runs over
  pp.4–5 and the electrical characteristics table over pp.5–6; each
  repeats its heading on the second page and is one table here.
- The absolute maximum ratings of the source print the conditions in
  parentheses; they are put in the Conditions cells.
- The electrical and switching characteristics tables of the source
  have one row per temperature range, the range printed in the test
  conditions (T~A~) column; the rows are kept.
- The addendum also lists SN74AHC1G00DBVTG4.A, which the catalog doesn't
  carry.
- Curves digitized from Figure 5-1 (p.7) and Figure 8-2 (p.11) to about
  2 % of full scale; the source's temperature axis is marked 0, 25, 50,
  85 and 125 at equal spacing, and the points are read at those marks.
- Figure 6-1 (p.8) is drawn as text; the load circuits and waveforms
  were compared with the page image.
- From images: overbars of Y = !{A • B} and Y = !{A} + !{B} (p.1), of
  Y = !{A × B} and Y = !{A} + !{B} (Overview, p.9) and of !{Q}
  (Figure 8-1, p.10); the curve points 12.1, 13.8, 7.6, 8.2, 9.2
  (Figure 5-1, p.7) and 8.6, 5.4, 5.8, 7.2 (Figure 8-2, p.11).
- Source issues:
  - Unconfirmed: t~PLH~ and t~PHL~ at V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF,
    –40°C to +85°C printed with a maximum of 6.5 ns, below the 7.5 ns
    maximum at 25°C (switching characteristics note 2).
