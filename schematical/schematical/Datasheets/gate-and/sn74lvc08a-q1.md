---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc08a-q1",
  "title": "SN74LVC08A-Q1 automotive quadruple 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc08a-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES480E",
    "date": "2024-05",
    "pages": 22
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74LVC08AQPWRG4Q1", "SN74LVC08AQPWRQ1", "SN74LVC08AWBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-08"
}
---

# SN74LVC08A-Q1 automotive quadruple 2-input positive-AND gate

## Description

Qualified for automotive applications. [[1::1]]

The SN74LVC08A-Q1 quadruple 2-input positive-AND gate is designed for
2.7V to 3.6V V~CC~ operation. The device performs the Boolean function
Y = A • B or Y = !{!{A} + !{B}} in positive logic. [[1::1]] [[1::2]]

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of this device as a translator in a mixed 3.3V/5V system
environment. [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel input A
1B, 2B, 3B, 4B: Channel input B
1Y, 2Y, 3Y, 4Y: Channel output Y
GND: Ground
VCC: Positive supply
```
[[3::Table 3-1]]

Conversion note: the BQA package drawing shows an exposed pad (PAD); the
pin functions table does not describe it.

## Functional description

Conversion note: the logic diagram, each gate (positive logic), is one 2-input AND gate.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
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

[1] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[2] The value of V~CC~ is provided in the recommended operating conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Value | Unit |
|---|---|--:|---|
| V~(ESD)~ | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

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
| Δt/Δv | Input transition rise or fall rate | | 0 | 8 | ns/V |
| T~A~ | Operating free-air temperature | Q suffix | -40 | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | BQA (WQFN), 14 pins | D (SOIC), 14 pins | PW (TSSOP), 14 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 150.8 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100µA | 2.7V to 3.6V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = -12mA | 2.7V | 2.2 | | | V |
| ^ | ^ | 3V | 2.4 | | | V |
| ^ | I~OH~ = -24mA | 3V | 2.2 | | | V |
| V~OL~ | I~OL~ = 100µA | 2.7V to 3.6V | | | 0.2 | V |
| ^ | I~OL~ = 12mA | 2.7V | | | 0.4 | V |
| ^ | I~OL~ = 24mA | 3V | | | 0.55 | V |
| I~I~ | V~I~ = 5.5V or GND | 3.6V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6V, other inputs at V~CC~ or GND | 2.7V to 3.6V | | | 500 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3V, T~A~ = 25°C.
```
[[5::table #1]]

```table id=operating caption="Operating characteristics, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ = 2.5V | V~CC~ = 3.3V | Unit |
| ^ | ^ | ^ | Typ | Typ | ^ |
|---|---|---|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10MHz | 9.8 | 10 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics (see Load Circuit and Voltage Waveforms)" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ = 2.7V | < | V~CC~ = 3.3V ± 0.3V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | | 4.8 | 1 | 4.1 | ns |
```
[[5::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 10 MHz, Z~O~ = 50 Ω. The outputs are measured one at a time, with
one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. All parameters and waveforms are not applicable to all devices. [[6::2]]

Conversion note: for t~PLH~/t~PHL~ the switch S1 of the load circuit
(Figure 5-1) is open, which leaves R~L~ and C~L~ from the output to
ground.

```table id=pmi caption="Load circuit and input conditions"
| V~CC~ | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|---|---|---|---|
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6V | 50 pF | 500 Ω | 0.3 V |
```
[[6::table #1]]

```figure ref=load-rl-cl caption="Load circuit for t~pd~ (S1 open)"
RL: 500 Ω
CL: 50 pF
```
[[6::Figure 5-1]]

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: 1.5 V
```
[[6::figure #1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. [[8::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum
and maximum supply voltage rating located in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-µF and 1-µF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results. [[8::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices
inputs must never be left floating. Such unused input pins must not be
left unconnected because the undefined voltages at the outside
connections result in undefined operational states. All unused inputs of
digital logic devices must be connected to a logic high or logic low
voltage, as defined by the input voltage specifications, to prevent them
from floating. The logic level that must be applied to any particular
unused input depends on the function of the device. Generally, the inputs are tied to GND or V~CC~, whichever
makes more sense for the logic function or is more convenient. [[8::3]]

Conversion note: the example layout for the SN74LVC08A-Q1 (Figure 7-1) is given here by its labels.

- Recommend GND flood fill for improved signal isolation, noise reduction, and thermal dissipation [[8::4]]
- Bypass capacitor placed close to the device [[8::6]]
- 0.1 µF [[8::2]]
- Unused inputs tied to V~CC~ [[8::3]]
- Unused output left floating [[8::3]]
- Avoid 90° corners for signal lines [[8::5]]

## Package

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74LVC08AQPWRQ1 | Reel of 2000 | -40 to 125 | LVC08AQ
SN74LVC08AQPWRG4Q1 | Reel of 2000 | -40 to 125 | LVC08AQ
SN74LVC08AWBQARQ1 | Reel of 3000 | -40 to 125 | LVC08Q
```
[[11::table #1]]

## Conversion notes

- Source pages 1–9 converted; the package option addendum, tape and
  reel and mechanical pages (10–22) dropped, replaced by the generated Package
  and Ordering sections. The D (SOIC) package is in the source but no
  claimed catalog part uses it.
- Dropped: features list, package information table, revision history,
  support pages, the enable/disable and setup/hold waveforms of the
  generic measurement figure (they do not apply to this part); the
  example layout (Figure 7-1) is given as a list of its labels.
- From images: µF (0.1 µF in Figure 7-1, p.8; the text layer has
  "0.1 F").
- From images: overbars of Y = !{!{A} + !{B}} (p.1; the second form of
  the Boolean function is set as an image).
- Source issues:
  - Source conflict: the feature list says "Operates from 1.65V to
    3.6V" (p.1), the description says the device is designed for 2.7V
    to 3.6V V~CC~ operation (p.1), and the recommended operating
    conditions give an operating V~CC~ of 2 V to 3.6 V (p.4).
