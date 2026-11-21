---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc32a-q1",
  "title": "SN74LVC32A-Q1 automotive quadruple 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lvc32a-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS706E",
    "date": "2024-12",
    "pages": 25
  },
  "devices": ["gate:74_32"],
  "parts": ["SN74LVC32AQDRQ1", "SN74LVC32AQPWRG4Q1", "SN74LVC32AQPWRQ1", "SN74LVC32AWBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74LVC32A-Q1 automotive quadruple 2-input positive-OR gate

## Description

The SN74LVC32A-Q1 quadruple 2-input positive-OR gate is designed for
2.7V to 3.6V V~CC~ operation.

The device performs the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic.

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of this device as a translator in a mixed 3.3V/5V system
environment.

Qualified for automotive applications. Inputs accept voltages to 5.5V.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2B: Channel 2, Input B
2Y: Channel 2, Output Y
GND: Ground
3Y: Channel 3, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
4Y: Channel 4, Output Y
4A: Channel 4, Input A
4B: Channel 4, Input B
VCC: Positive Supply
```

The thermal pad can be connected to GND or left floating. Do not connect
to any other signal or supply. Conversion note: the source has this in
the pin functions table, for the BQA package only.

## Functional description

Inputs can be driven from either 3.3-V or 5-V devices. This feature
allows the use of this device as a translator in a mixed 3.3-V/5-V
system environment.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```

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

[1] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[2] The value of V~CC~ is provided in the recommended operating conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Conditions | Value | Unit |
|---|---|--:|---|
| V~(ESD)~ | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | Charged device model (CDM), per AEC Q100-011 | ±1000 | V |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | Min | Max | Unit |
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
| Δt/Δv | Input transition rise or fall rate | | | 7 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC32A-Q1 | < | < | Unit |
| Symbol | Parameter | BQA (WQFN) 14 pins | D (SOIC) 14 pins | PW (TSSOP) 14 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 150.8 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100μA | 2.7V to 3.6V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = -12mA | 2.7V | 2.2 | | | V |
| ^ | ^ | 3V | 2.4 | | | V |
| ^ | I~OH~ = -24mA | 3V | 2.2 | | | V |
| V~OL~ | I~OL~ = 100μA | 2.7V to 3.6V | | | 0.2 | V |
| ^ | I~OL~ = 12mA | 2.7V | | | 0.4 | V |
| ^ | I~OL~ = 24mA | 3V | | | 0.55 | V |
| I~I~ | V~I~ = 5.5V or GND | 3.6V | | | ±5 | μA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6V | | | 10 | μA |
| ΔI~CC~ | One input at V~CC~ - 0.6V, Other inputs at V~CC~ or GND | 2.7V to 3.6V | | | 500 | μA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3V, T~A~ = 25°C.
```

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 2.5V | V~CC~ = 3.3V | Unit |
| ^ | ^ | ^ | Typ | Typ | ^ |
|---|---|---|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10MHz | 10.6 | 12.5 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ = 2.7V | < | V~CC~ = 3.3V ± 0.3V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | | 4.4 | 1 | 3.8 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

All parameters and waveforms are not applicable to all devices.

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
RL: 500 Ω
CL: 50 pF
```

```table id=test-conditions caption="Test conditions (Figure 5-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in given example
layout image.

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs must
never be left floating. In many cases, functions or parts of functions
of digital logic devices are unused; for example, when only two inputs
of a triple-input AND gate are used or only 3 of the 4 buffer gates are
used. Such unused input pins must not be left unconnected because the
undefined voltages at the outside connections result in undefined
operational states. All unused inputs of digital logic devices must be
connected to a logic high or logic low voltage, as defined by the input
voltage specifications, to prevent them from floating. The logic level
that must be applied to any particular unused input depends on the
function of the device. Generally, the inputs are tied to GND or V~CC~,
whichever makes more sense for the logic function or is more convenient.

Conversion note: Figure 7-1, the example layout for the SN74LVC32A-Q1
(p.8), recommends a GND flood fill, a bypass capacitor placed close to
the device, unused inputs tied to V~CC~, an unused output left floating,
and no 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74LVC32AQDRQ1 | Reel of 2500 | -40 to 125 | LVC32AQ
SN74LVC32AQPWRG4Q1 | Reel of 2000 | -40 to 125 | LVC32AQ
SN74LVC32AQPWRQ1 | Reel of 2000 | -40 to 125 | LVC32AQ
SN74LVC32AWBQARQ1 | Reel of 3000 | -40 to 125 | LVC32Q
```

## Conversion notes

- Source pages 1 and 3–8 converted. The ordering lines are from the
  package option addendum (p.11). The table of contents (p.2), the
  support and revision history pages (9–10), the rest of the addendum
  (p.12), tape and reel and mechanical pages (13–25) are left out; the
  generated Package section replaces them.
- Dropped: features list (except the automotive qualification and the
  input voltage), package information table, the logic diagram (one
  2-input OR gate, pp.1 and 7); the layout example (Figure 7-1) is
  summarized in one sentence.
- The overview (p.7) repeats the description's Boolean-function
  sentence; it is given once, in Description.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 5-1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- The text layer prints the Boolean function as "Y + A ) B or
  Y + A • B"; the page image shows Y = A + B or Y = !{!{A} • !{B}}
  (p.1).
- From images: overbars of Y = !{!{A} • !{B}} (p.1).
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column; the ESD ratings table gives only the
  symbol V~(ESD)~.
- Source issues:
  - Source conflict: the description gives the device as designed for
    2.7V to 3.6V V~CC~ operation (p.1); the features list says
    "Operates from 2V to 3.6V" (p.1) and the recommended operating
    conditions give an operating supply voltage of 2 V minimum (p.4).
  - Source conflict: the pin functions table (p.3) lists the thermal
    pad under the name "Thermal Information".
