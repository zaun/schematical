---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc86a-q1",
  "title": "SN74LVC86A-Q1 automotive quadruple 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74lvc86a-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS707E",
    "date": "2024-12",
    "pages": 24
  },
  "devices": ["gate:74_86"],
  "parts": ["SN74LVC86AQDRG4Q1", "SN74LVC86AQPWRG4Q1", "SN74LVC86AQPWRQ1", "SN74LVC86AWBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74LVC86A-Q1 automotive quadruple 2-input exclusive-OR gate

## Description

The SN74LVC86A-Q1 quadruple 2-input exclusive-OR gate is designed for
2.7V to 3.6V V~CC~ operation. [[1::1]]

Qualified for automotive applications. Inputs accept voltages to 5.5V. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Gate 1 input
1B: Gate 1 input
1Y: Gate 1 output
2A: Gate 2 input
2B: Gate 2 input
2Y: Gate 2 output
3Y: Gate 3 output
3A: Gate 3 input
3B: Gate 3 input
4Y: Gate 4 output
4A: Gate 4 input
4B: Gate 4 input
GND: Ground Pin
VCC: Power Pin
```
[[3::Table 3-1]]

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. Conversion note: the source has
this for the thermal pad of the BQA package drawing. [[3::1]]

## Functional description

The device performs the Boolean function Y = A ⊕ B or
Y = !{A}B + A!{B} in positive logic. [[7::1]]

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the output.
If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[7::2]]

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of this device as a translator in a mixed 3.3V/5V system
environment. [[7::3]]

### Exclusive-OR logic

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These five equivalent
exclusive-OR symbols are valid for an SN74LVC86A gate in positive logic;
negation may be shown at any two ports. Conversion note: the five
symbols are not drawn here. [[7::4]] [[7::5]]

The output is active (low) if all inputs stand at the same logic level
(i.e., A = B). Conversion note: this is the logic-identity element
(symbol =). [[7::6]]

The output is active (low) if an even number of inputs (i.e., 0 or 2)
are active. Conversion note: this is the even-parity element (symbol
2k). [[7::7]]

The output is active (high) if an odd number of inputs (i.e., only 1 of
the 2) are active. Conversion note: this is the odd-parity element
(symbol 2k + 1). [[7::8]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[3::Table 3-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 2 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 2.7 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2.7 V to 3.6 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 9 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC86A-Q1 | < | < | Unit |
| Symbol | Parameter | BQA 14 pins | D 14 pins | PW 14 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 150.8 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
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
| ΔI~CC~ | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 2.7V to 3.6V | | | 500 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[5::table #1]]

```table id=operating caption="Operating characteristics, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | ^ |
|---|---|---|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 7.5 | 8.5 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics (see Load Circuit and Voltage Waveforms)" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A | Y | | 5.6 | 1 | 4.6 | ns |
```
[[5::table #3]]

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily for the
examples listed in the following table. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1MHz,
Z~O~ = 50Ω, t~t~ ≤ 2.5ns. The outputs are measured individually with one
input transition per measurement. [[6::1]] [[6::2]]

```table id=pmi caption="Load circuit and input conditions"
| V~CC~ | V~t~ | R~L~ | C~L~ | ΔV |
|---|---|---|---|---|
| 1.8V ± 0.15V | V~CC~/2 | 1kΩ | 30pF | 0.15V |
| 2.5V ± 0.2V | V~CC~/2 | 500Ω | 30pF | 0.15V |
| 2.7V | 1.5V | 500Ω | 50pF | 0.3V |
| 3.3V ± 0.3V | 1.5V | 500Ω | 50pF | 0.3V |
```
[[6::table #1]]

```figure ref=load-rl-cl caption="Load circuit for push-pull outputs"
RL: see table
CL: see table
```
[[6::Figure 5-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delays"
VM: V~t~ at the input, 50% at the outputs
```
[[6::Figure 5-2]]

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. [[6::3]] [[6::5]]

Conversion note: Figure 5-3, voltage waveforms, input and output
transition times, shows t~r~ and t~f~ between the 10% and 90% points of
the input and of the output. The greater between t~r~ and t~f~ is the
same as t~t~.

## Application information

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Section 4.3 table. [[8::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 µF is recommended; if
there are multiple V~CC~ pins, then 0.01 µF or 0.022 µF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 µF and a 1 µF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results. [[8::2]]

### Layout guidelines

When using multiple bit logic devices inputs should never float. [[8::3]]

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states.
Section 7.2.2 specifies the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or
V~CC~, whichever makes more sense or is more convenient. It is generally
acceptable to float outputs, unless the part is a transceiver. [[8::4]]

Conversion note: Figure 7-1, the example layout for the SN74LVC86A-Q1
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
SN74LVC86AQDRG4Q1 | Reel of 2500 | -40 to 125 | LVC86AQ
SN74LVC86AQPWRG4Q1 | Reel of 2000 | -40 to 125 | LVC86AQ
SN74LVC86AQPWRQ1 | Reel of 2000 | -40 to 125 | LVC86AQ
SN74LVC86AWBQARQ1 | Reel of 3000 | -40 to 125 | LVC86Q
```
[[10::table #1]]

## Conversion notes

- Source pages 1–8 converted; the support page (9), the package option
  addendum, tape and reel and mechanical pages (10–24) dropped, replaced
  by the generated Package and Ordering sections. The ordering block
  follows the package option addendum (p.10).
- Dropped: features list (except the automotive qualification and the
  input voltage), package information table, table of contents, revision
  history, the exclusive-OR symbol drawings (pp.1 and 7), the transition
  time waveforms (Figure 5-3) and the layout example drawing
  (Figure 7-1), each summarized in one sentence.
- The lettered notes of Figures 5-1, 5-2 and 5-3 are written as
  sentences next to the figures.
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column.
- From images: overbars of Y = !{A}B + A!{B} (p.7; the text layer has
  "Y = AB + A B").
- Source issues:
  - Source conflict: the pin functions table (p.3) marks TYPE with (1)
    and Thermal pad with (2) and prints no notes.
  - Source conflict: the pin functions table (p.3) lists a pin NC, "Do
    not connect", with no pin number; neither package drawing has an NC
    pin.
  - Source conflict: the measurement table (p.6) has rows for V~CC~ of
    1.8V ± 0.15V and 2.5V ± 0.2V; the recommended operating conditions
    give an operating V~CC~ of 2 V to 3.6 V (p.4) and the switching
    characteristics give no values below 2.7 V (p.5).
