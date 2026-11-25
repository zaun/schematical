---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc10a",
  "title": "SN74LVC10A triple 3-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lvc10a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS284Q",
    "date": "2024-08",
    "pages": 32
  },
  "devices": ["gate:74_10"],
  "parts": ["SN74LVC10ABQAR", "SN74LVC10AD", "SN74LVC10ADBR", "SN74LVC10ADG4", "SN74LVC10ADR", "SN74LVC10ADT", "SN74LVC10ANSR", "SN74LVC10APW", "SN74LVC10APWR", "SN74LVC10APWRG4", "SN74LVC10APWT", "SN74LVC10ARGYR"],
  "packages": {"p14": "D, DB, NS, PW, BQA, RGY"},
  "converted": "2026-11-21"
}
---

# SN74LVC10A triple 3-input positive-NAND gate

## Description

This triple 3-input positive-NAND gate is designed for 1.65V to 3.6V
V~CC~ operation. [[1::1]]

Operates from 1.65V to 3.6V. Specified from –40°C to 85°C and –40°C to
125°C. Inputs accept voltages to 5.5V. Latch-up performance exceeds
250 mA per JESD 17. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
2A: Channel 2, Input A
2B: Channel 2, Input B
2C: Channel 2, Input C
2Y: Channel 2, Output Y
GND: Ground
3Y: Channel 3, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
3C: Channel 3, Input C
1Y: Channel 1, Output Y
1C: Channel 1, Input C
VCC: Positive Supply
```
[[3::Table 3-1]]

Thermal pad: Connect the GND pin to the exposed thermal pad for correct
operation. Connect the thermal pad to any internal PCB ground plane
using multiple vias for good thermal performance. [[3::1]]

## Functional description

The SN74LVC10A performs the Boolean function Y = !{A • B • C} or
Y = !{A} + !{B} + !{C} in positive logic. [[8::1]]

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of these devices as translators in a mixed 3.3V/5V system
environment. [[8::1]]

Conversion note: the logic diagram, each gate (Figure 6-1), shows one
3-input NAND gate with inputs A, B and C and output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | L |
| L | X | X | H |
| X | L | X | H |
| X | X | L | H |
```
[[8::Table 6-1]]

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

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | T~A~ = 25°C | < | –40 to 85°C | < | –40 to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 3.6 | 1.65 | 3.6 | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | 1.5 | | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65V to 1.95V | 0.65 × V~CC~ | | 0.65 × V~CC~ | | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | 1.7 | | 1.7 | | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7V to 3.6V | 2 | | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65V to 1.95V | | 0.35 × V~CC~ | | 0.35 × V~CC~ | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | | 0.7 | | 0.7 | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7V to 3.6V | | 0.8 | | 0.8 | | 0.8 | V |
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
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC10A | < | < | < | < | < | Unit |
| Symbol | Parameter | BQA (WQFN) 14 pins | D (SOIC) 14 pins | DB (SSOP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | RGY (VQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 96 | 123.8 | 150.8 | 92.1 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40 to 85°C | < | –40 to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –100μA | 1.65V to 3.6V | V~CC~ - 0.2 | | | V~CC~ - 0.2 | | V~CC~ - 0.3 | | V |
| ^ | I~OH~ = –4mA | 1.65V | 1.29 | | | 1.2 | | 1.05 | | V |
| ^ | I~OH~ = –8mA | 2.3 V | 1.9 | | | 1.7 | | 1.55 | | V |
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
| C~i~ | V~I~ = V~CC~ or GND | 3.3V | | 5 | | | | | | pF |
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 1.8V | 9 | pF |
| ^ | ^ | ^ | 2.5V | 10 | pF |
| ^ | ^ | ^ | 3.3V | 11 | pF |
```
[[6::table #1]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | –40 to 85°C | < | –40 to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | 1.8V ± 0.15V | 1 | 4.2 | 10.1 | 1 | 10.6 | 1 | 12.1 | ns |
| ^ | ^ | ^ | 2.5V ± 0.2V | 1 | 2.9 | 7.3 | 1 | 7.8 | 1 | 9.9 | ns |
| ^ | ^ | ^ | 2.7V | 1 | 3.1 | 5.6 | 1 | 5.8 | 1 | 7.4 | ns |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1 | 2.7 | 4.7 | 1 | 4.9 | 1 | 6 | ns |
| t~sk(o)~ | | | 3.3V ± 0.3V | | | | | 1 | | 1.5 | ns |
```
[[5::table #3]]

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily for the
examples listed in the following table. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1MHz,
Z~O~ = 50Ω, t~t~ ≤ 2.5ns. The outputs are measured individually with one
input transition per measurement. [[7::1]] [[7::2]]

```table id=test-conditions caption="Test conditions"
| V~CC~ | V~t~ | R~L~ | C~L~ | ΔV |
|---|---|---|---|---|
| 1.8V ± 0.15V | V~CC~/2 | 1kΩ | 30pF | 0.15V |
| 2.5V ± 0.2V | V~CC~/2 | 500Ω | 30pF | 0.15V |
| 2.7V | 1.5V | 500Ω | 50pF | 0.3V |
| 3.3V ± 0.3V | 1.5V | 500Ω | 50pF | 0.3V |
```
[[7::table #1]]

```figure ref=load-rl-cl caption="Load circuit for push-pull outputs"
RL: see table
CL: see table
```
[[7::Figure 5-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays, input and output transition times"
VM: V~t~ (input), 50% (output)
```
[[7::Figure 5-3]]

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. The greater between t~r~ and t~f~ is the same as t~t~. [[7::3]] [[7::5]] [[7::7]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating in the Recommended Operating Conditions. [[9::2]]

Each V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 µF is recommended; if
there are multiple V~CC~ pins, then 0.01 µF or 0.022 µF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 µF and a 1 µF are commonly
used in parallel. The bypass capacitor must be installed as close to the
power pin as possible for best results. [[9::3]]

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
convenient. [[9::4]]

Conversion note: the example layout for the SN74LVC10A (Figure 7-1) is
given here by its labels.

- Recommend GND flood fill for improved signal isolation, noise reduction, and thermal dissipation [[9::5]]
- Bypass capacitor placed close to the device [[9::6]]
- 0.1 μF [[9::3]]
- Unused inputs tied to V~CC~ [[9::4]]
- Unused output left floating [[9::4]]
- Avoid 90° corners for signal lines [[9::7]]

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74LVC10ABQAR | Reel of 3000 | -40 to 125 | LVC10A
SN74LVC10AD | Tube of 50 | -40 to 125 | LVC10A
SN74LVC10ADBR | Reel of 2000 | -40 to 125 | LC10A
SN74LVC10ADG4 | Tube of 50 | -40 to 125 | LVC10A
SN74LVC10ADR | Reel of 2500 | -40 to 125 | LVC10A
SN74LVC10ADT | Reel of 250 | -40 to 125 | LVC10A
SN74LVC10ANSR | Reel of 2000 | -40 to 125 | LVC10A
SN74LVC10APW | Tube of 90 | -40 to 125 | LC10A
SN74LVC10APWR | Reel of 2000 | -40 to 125 | LC10A
SN74LVC10APWRG4 | Reel of 2000 | -40 to 125 | LC10A
SN74LVC10APWT | Reel of 250 | -40 to 125 | LC10A
SN74LVC10ARGYR | Reel of 3000 | -40 to 125 | LC10A
```
[[12::table #1]]

## Conversion notes

- Source pages 1 and 3–9 converted. The ordering lines are from the
  package option addendum (pp.12–13). The table of contents (p.2), the
  support and revision history pages (10–11), tape and reel and
  mechanical pages (14–32) are left out; the generated Package section
  replaces them.
- Dropped: features list (the supply, temperature, input-voltage and
  latch-up lines are kept in Description), package information table,
  the vendor pinout drawings, the logic diagram (pp.1 and 8, described
  in one sentence); the layout example (Figure 7-1) is given as a list
  of its labels.
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column. The propagation-delay waveforms
  (Figure 5-2) and the transition-time waveforms (Figure 5-3) are drawn
  as one shared figure; the notes under Figures 5-1 to 5-3 are the
  sentences beside the figures.
- From images: overbars of Y = !{A • B • C} and Y = !{A} + !{B} + !{C}
  (p.8).
