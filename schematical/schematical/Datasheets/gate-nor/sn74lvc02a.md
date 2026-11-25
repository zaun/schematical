---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc02a",
  "title": "SNx4LVC02A quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74lvc02a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS280T",
    "date": "2024-12",
    "pages": 39
  },
  "devices": ["gate:74_02"],
  "parts": ["5962-9760401QCA", "5962-9760401QDA", "SN74LVC02ABQAR", "SN74LVC02AD", "SN74LVC02ADBR", "SN74LVC02ADR", "SN74LVC02ADRG4", "SN74LVC02ANSR", "SN74LVC02APW", "SN74LVC02APWE4", "SN74LVC02APWR", "SN74LVC02APWR1G4", "SN74LVC02APWT", "SN74LVC02ARGYR", "SN74LVC02ARGYRG4", "SNJ54LVC02AJ", "SNJ54LVC02AW", "5962-9760401Q2A", "SNJ54LVC02AFK"],
  "packages": {"p14": "D, DB, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4LVC02A quadruple 2-input positive-NOR gates

## Description

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of these devices as translators in a mixed 3.3V/5V system
environment. [[1::1]]

The device performs the Boolean function Y = !{A + B} or
Y = !{A} ⋅ !{B} in positive logic. [[1::2]]

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of this device as a translator in a mixed 3.3V/5V system
environment. [[1::1]] [[1::3]]

Operate from 1.65V to 3.6V. Specified from –40°C to 85°C, –40°C to
125°C, and –55°C to 125°C. Inputs accept voltages to 5.5V. Latch-up
performance exceeds 250mA per JESD 17. [[1::1]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1Y: 1Y Output
1A: 1A Input
1B: 1B Input
2Y: 2Y Output
2A: 2A Input
2B: 2B Input
GND: Ground Pin
3A: 3A Input
3B: 3B Input
3Y: 3Y Output
4A: 4A Input
4B: 4B Input
4Y: 4Y Output
VCC: Power Pin
NC: No Connection
```
[[3::Table 3-1]]

## Functional description

Conversion note: the logic diagram, each gate (Figure 6-1), shows one
2-input NOR gate with inputs A and B and output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
```
[[3::Table 3-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [1] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [1] [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| P~tot~ | Power dissipation | T~A~ = –40°C to 125°C [3] [4] | | 500 | mW |

[1] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[2] The value of V~CC~ is provided in the recommended operating conditions table.
[3] For the D package: above 70°C, the value of P~tot~ derates linearly with 8 mW/K.
[4] For the DB, NS, and PW packages: above 60°C, the value of P~tot~ derates linearly with 5.5 mW/K.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc-54 caption="Recommended operating conditions, SN54LVC02A" parts=SNJ54*,5962-*
| Symbol | Parameter | Conditions | SN54LVC02A | < | Unit |
| ^ | ^ | ^ | –55°C to 125°C | < | ^ |
| ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 2 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 2.7V to 3.6V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2.7V to 3.6V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2.7V | | -12 | mA |
| ^ | ^ | V~CC~ = 3V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2.7V | | 12 | mA |
| ^ | ^ | V~CC~ = 3V | | 24 | mA |
```
[[4::table #3]]

```table id=roc-74 caption="Recommended operating conditions, SN74LVC02A" parts=SN74*
| Symbol | Parameter | Conditions | SN74LVC02A | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | –40°C to 85°C | < | –40°C to 125°C | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 3.6 | 1.65 | 3.6 | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | 1.5 | | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65V to 1.95V | 0.65 × V~CC~ | | 0.65 × V~CC~ | | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | 1.7 | | 1.7 | | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7V to 3.6V | 2 | | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65V to 1.95V | | 0.35 × V~CC~ | | 0.35 × V~CC~ | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7 V | | 0.7 | | 0.7 | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7V to 3.6 V | | 0.8 | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65V | | -4 | | -4 | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | -8 | | -8 | | -8 | mA |
| ^ | ^ | V~CC~ = 2.7V | | -12 | | -12 | | -12 | mA |
| ^ | ^ | V~CC~ = 3V | | -24 | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65V | | 4 | | 4 | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | 8 | | 8 | | 8 | mA |
| ^ | ^ | V~CC~ = 2.7V | | 12 | | 12 | | 12 | mA |
| ^ | ^ | V~CC~ = 3V | | 24 | | 24 | | 24 | mA |
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC02A | < | < | < | < | < | Unit |
| Symbol | Parameter | BQA (WQFN) 14 pins | D (SOIC) 14 pins | DB (SSOP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | RGY (VQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 140.4 | 123.8 | 150.8 | 92.1 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec-54 caption="Electrical characteristics, SN54LVC02A" parts=SNJ54*,5962-* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN54LVC02A | < | < | Unit |
| ^ | ^ | ^ | –55°C to 125°C | < | < | ^ |
| ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = –100μA | 2.7V to 3.6V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = –12mA | 2.7V | 2.2 | | | V |
| ^ | ^ | 3V | 2.4 | | | V |
| ^ | I~OH~ = –24mA | 3V | 2.2 | | | V |
| V~OL~ | I~OL~ = 100μA | 2.7V to 3.6V | | | 0.2 | V |
| ^ | I~OL~ = 12mA | 2.7V | | | 0.4 | V |
| ^ | I~OL~ = 24mA | 3V | | | 0.55 | V |
| I~I~ | V~I~ = 5.5V or GND | 3.6V | | | ±5 | μA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6V | | | 10 | μA |
| ΔI~CC~ | One input at V~CC~ – 0.6V, Other inputs at V~CC~ or GND | 2.7V to 3.6V | | | 500 | μA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3V | | 5 [1] | | pF |

[1] T~A~ = 25°C
```
[[5::table #3]]

```table id=ec-74 caption="Electrical characteristics, SN74LVC02A" parts=SN74* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN74LVC02A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | ^ |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –100μA | 1.65V to 3.6V | V~CC~ - 0.2 | | | V~CC~ - 0.2 | | V~CC~ - 0.3 | | V |
| ^ | I~OH~ = –4mA | 1.65V | 1.29 | | | 1.2 | | 1.05 | | V |
| ^ | I~OH~ = –8mA | 2.3V | 1.9 | | | 1.7 | | 1.55 | | V |
| ^ | I~OH~ = –12mA | 2.7V | 2.2 | | | 2.2 | | 2.05 | | V |
| ^ | ^ | 3V | 2.4 | | | 2.4 | | 2.25 | | V |
| ^ | I~OH~ = –24mA | 3V | 2.3 | | | 2.2 | | 2 | | V |
| V~OL~ | I~OL~ = 100μA | 1.65V to 3.6V | | | 0.1 | | 0.2 | | 0.3 | V |
| ^ | I~OL~ = 4mA | 1.65V | | | 0.24 | | 0.45 | | 0.6 | V |
| ^ | I~OL~ = 8mA | 2.3V | | | 0.3 | | 0.7 | | 0.75 | V |
| ^ | I~OL~ = 12mA | 2.7V | | | 0.4 | | 0.4 | | 0.6 | V |
| ^ | I~OL~ = 24mA | 3V | | | 0.55 | | 0.55 | | 0.8 | V |
| I~I~ | V~I~ = 5.5V or GND | 3.6V | | | ±1 | | ±5 | | ±20 | μA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6V | | | 1 | | 10 | | 40 | μA |
| ΔI~CC~ | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7V to 3.6V | | | 500 | | 500 | | 5000 | μA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | | | | pF |
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10MHz | 1.8V | 7.5 | pF |
| ^ | ^ | ^ | 2.5V | 8.5 | pF |
| ^ | ^ | ^ | 3.3V | 9.5 | pF |
```
[[7::table #1]]

## Switching characteristics

```table id=sw-54 caption="Switching characteristics, SN54LVC02A" parts=SNJ54*,5962-* conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | SN54LVC02A | < | Unit |
| ^ | ^ | ^ | ^ | –55°C to 125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|---|--:|--:|---|
| t~pd~ | A or B | Y | 2.7V | | 5.4 | ns |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1 | 4.4 | ns |
```
[[6::table #2]]

```table id=sw-74 caption="Switching characteristics, SN74LVC02A" parts=SN74* conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | SN74LVC02A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | ^ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.8V ± 0.15V | 1 | 3.8 | 8.4 | 1 | 8.9 | 1 | 10.4 | ns |
| ^ | ^ | ^ | 2.5V ± 0.2V | 1 | 2.9 | 6.9 | 1 | 7.4 | 1 | 9.5 | ns |
| ^ | ^ | ^ | 2.7V | 1 | 3 | 5.2 | 1 | 5.4 | 1 | 7 | ns |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1 | 3.6 | 4.2 | 1 | 4.4 | 1 | 5.5 | ns |
| t~sk(o)~ | | | 3.3V ± 0.3V | | | | | 1 | | 1.5 | ns |
```
[[6::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[8::2]]

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: Figure 5-1 also shows pulse-duration, setup-and-hold
and enable/disable waveforms, which are not drawn here.

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
[[8::Figure 5-1]]

```table id=test-conditions caption="Test conditions (Figure 5-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6V | 50 pF | 500 Ω | 0.3 V |
```
[[8::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[8::Figure 5-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in given example
layout image. [[10::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs must
not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used. Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient. [[10::3]]

Conversion note: the example layout for the SNx4LVC02A (Figure 7-1) is
given here by its labels.

- Recommend GND flood fill for improved signal isolation, noise reduction, and thermal dissipation [[10::4]]
- Bypass capacitor placed close to the device [[10::6]]
- 0.1 μF [[10::2]]
- Unused output left floating [[10::3]]
- Unused inputs tied to V~CC~ [[10::3]]
- Avoid 90° corners for signal lines [[10::5]]

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9760401Q2A | Tube of 55 | -55 to 125 | 5962-9760401Q2A SNJ54LVC02AFK
5962-9760401QCA | Tube of 25 | -55 to 125 | 5962-9760401QCA SNJ54LVC02AJ
5962-9760401QDA | Tube of 25 | -55 to 125 | 5962-9760401QDA SNJ54LVC02AW
SN74LVC02ABQAR | Reel of 3000 | -40 to 125 | LVC02A
SN74LVC02AD | Tube of 50 | -40 to 125 | LVC02A
SN74LVC02ADBR | Reel of 2000 | -40 to 125 | LC02A
SN74LVC02ADR | Reel of 2500 | -40 to 125 | LVC02A
SN74LVC02ADRG4 | Reel of 2500 | -40 to 125 | LVC02A
SN74LVC02ANSR | Reel of 2000 | -40 to 125 | LVC02A
SN74LVC02APW | Tube of 90 | -40 to 125 | LC02A
SN74LVC02APWE4 | Tube of 90 | -40 to 125 | LC02A
SN74LVC02APWR | Reel of 2000 | -40 to 125 | LC02A
SN74LVC02APWR1G4 | Reel of 2000 | -40 to 125 | LC02A
SN74LVC02APWT | Reel of 250 | -40 to 125 | LC02A
SN74LVC02ARGYR | Reel of 3000 | -40 to 125 | LC02A
SN74LVC02ARGYRG4 | Reel of 3000 | -40 to 125 | LC02A
SNJ54LVC02AFK | Tube of 55 | -55 to 125 | 5962-9760401Q2A SNJ54LVC02AFK
SNJ54LVC02AJ | Tube of 25 | -55 to 125 | 5962-9760401QCA SNJ54LVC02AJ
SNJ54LVC02AW | Tube of 25 | -55 to 125 | 5962-9760401QDA SNJ54LVC02AW
```
[[13::table #1]] [[14::table #1]]

## Conversion notes

- Source pages 1 and 3–10 converted. The ordering lines are from the
  package option addendum (pp.13–15). The table of contents (p.2), the
  support and revision history pages (11–12), tape and reel and
  mechanical pages (16–39) are left out; the generated Package section
  replaces them.
- Dropped: features list (the supply, temperature, input-voltage and
  latch-up lines are kept in Description), device information table,
  the vendor pinout drawings, the logic diagram (pp.1 and 9, described
  in one sentence); the layout example (Figure 7-1) is given as a list
  of its labels.
- The electrical characteristics tables of the source give symbols only,
  so they have no Parameter column. The SN74LVC02A recommended operating
  conditions run over two pages (pp.4–5) and are one table here.
- The source's Detailed Description chapter holds only the logic diagram
  and the function table.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 5-1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- From images: overbars of Y = !{A + B} and Y = !{A} ⋅ !{B} (p.1); µF
  (0.1 μF in Figure 7-1, p.10; the text layer has "0.1 F").
- Source issues:
  - Source error: the pin functions table (p.3) gives 3Y of the J and W
    packages as pin 20; Figure 3-1 (p.3) shows 3Y on pin 10 of the
    14-pin package. The generated pin table has 10.
  - Unconfirmed: the Description (p.1) prints the paragraph "Inputs can
    be driven from either 3.3V or 5V devices…" twice, once with "these
    devices as translators" and once with "this device as a
    translator". Both kept.
