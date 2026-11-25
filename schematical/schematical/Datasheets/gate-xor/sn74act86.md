---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act86",
  "title": "SNx4ACT86 quadruple 2-input exclusive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74act86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS534E",
    "date": "2025-04",
    "pages": 38
  },
  "devices": ["gate:74_86"],
  "parts": ["5962-9068701QCA", "5962-9068701QDA", "SN74ACT86BQAR", "SN74ACT86D", "SN74ACT86DBR", "SN74ACT86DR", "SN74ACT86DRG4", "SN74ACT86N", "SN74ACT86NSR", "SN74ACT86PW", "SN74ACT86PWR", "SN74ACT86PWRG4", "SNJ54ACT86J", "SNJ54ACT86W", "5962-9068701Q2A", "SNJ54ACT86FK"],
  "packages": {"p14": "D, DB, N, NS, PW, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4ACT86 quadruple 2-input exclusive-OR gates

## Description

The ’ACT86 devices are quadruple 2-input exclusive-OR gates. The devices
perform the Boolean functions Y = A ⊕ B or Y = !{A}B + A!{B} in positive
logic. [[1::1]]

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the
output. If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[1::3]]

4.5V to 5.5V V~CC~ operation. Inputs accept voltages to 5.5V. Max
t~pd~ of 10ns at 5V. Inputs are TTL-voltage compatible. [[1::2]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
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
NC: Not internally connected
```
[[3::Table 3-1]]

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. Conversion note: the source has
this for the thermal pad, BQA package only. [[4::1]]

## Functional description

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These five equivalent
exclusive-OR symbols are valid for an ’ACT86 gate in positive logic;
negation may be shown at any two ports. Conversion note: the five
symbols of Figure 6-1 are not drawn here. [[8::1]] [[8::2]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output Y |
| A | B | ^ |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[8::Table 6-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[5::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | SN54ACT86 | < | SN74ACT86 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 8 | | 8 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to verify proper device operation. Refer to Implications of Slow or Floating CMOS Inputs, application note.
```
[[5::table #2]]

## Thermal information

```table id=thermal caption="Thermal information, SN74ACT86"
| Symbol | Parameter [1] | BQA (WQFN) 14 pins | D (SOIC) 14 pins | DB (SSOP) 14 pins | N (PDIP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | Unit |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 91.3 | 119.9 | 96 | 80 | 76 | 145.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 99.4 | | | | | | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 60.0 | | | | | | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 14.5 | | | | | | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 60.8 | | | | | | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | 37.0 | | | | | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[5::table #3]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | SN54ACT86 | < | SN74ACT86 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50µA | 4.5V | 4.4 | 4.49 | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | 5.5V | 5.4 | 5.49 | | 5.4 | | 5.4 | | V |
| ^ | ^ | I~OH~ = -24mA | 4.5V | 3.86 | | | 3.7 | | 3.76 | | V |
| ^ | ^ | ^ | 5.5V | 4.86 | | | 4.7 | | 4.76 | | V |
| ^ | ^ | I~OH~ = -50mA [1] | 5.5V | | | | 3.85 | | | | V |
| ^ | ^ | I~OH~ = -75mA [1] | 5.5V | | | | | | 3.85 | | V |
| V~OL~ | | I~OL~ = 50µA | 4.5V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24mA | 4.5V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | ^ | 5.5V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 50mA [1] | 5.5V | | | | | 1.65 | | | V |
| ^ | ^ | I~OL~ = 75mA [1] | 5.5V | | | | | | | 1.65 | V |
| I~I~ | | V~I~ = V~CC~ or GND | 5.5V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5V | | | 4 | | 80 | | 40 | µA |
| ΔI~CC~ [2] | | One input at 3.4V, Other inputs at GND or V~cc~ | 5.5V | | 0.6 | | | 1.6 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5V | | 2.6 | | | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 10ms.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0V or V~CC~.
```
[[6::table #1]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50pF, f = 1MHz | 25 | pF |
```
[[6::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5V ± 0.5V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | T~A~ = 25°C | < | < | SN54ACT86 | < | SN74ACT86 | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1.5 | 8.5 | 9.5 | 1 | 10 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | 1.5 | 7 | 9.5 | 1 | 10.5 | 1 | 10.5 | ns |
```
[[6::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1MHz,
Z~O~ = 50Ω, t~r~ ≤ 2.5ns, t~f~ ≤ 2.5ns. The outputs are measured one
at a time with one input transition per measurement. [[7::1]]

Conversion note: the load circuit also has a 500 Ω resistor from the output
to switch S1 (2 × V~CC~ or open), and S1 is open for t~PLH~/t~PHL~, which
leaves the load below. Conversion note: in the voltage waveforms the input
swings 0 V to 3 V and is measured at 1.5 V, and the outputs are measured at
50 % V~CC~.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```
[[7::Figure 5-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: 1.5 V (input), 50 % V~CC~ (outputs)
```
[[7::Figure 5-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used in
parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in Figure 7-1. [[9::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs
must not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used. Such unused input pins
must not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. All unused
inputs of digital logic devices must be connected to a logic high or
logic low voltage, as defined by the input voltage specifications, to
prevent them from floating. The logic level that must be applied to any
particular unused input depends on the function of the device.
Generally, the inputs are tied to GND or V~CC~, whichever makes more
sense for the logic function or is more convenient. Conversion note:
Figure 7-1, the example layout (p.9), recommends a GND flood fill, a
bypass capacitor placed close to the device, unused inputs tied to
V~CC~, an unused output left floating, and no 90° corners for signal
lines. [[9::3]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
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
5962-9068701Q2A | Tube of 55 | -55 to 125 | 5962-9068701Q2A SNJ54ACT86FK
5962-9068701QCA | Tube of 25 | -55 to 125 | 5962-9068701QCA SNJ54ACT86J
5962-9068701QDA | Tube of 25 | -55 to 125 | 5962-9068701QDA SNJ54ACT86W
SN74ACT86BQAR | Reel of 3000 | -40 to 85 | AD86
SN74ACT86D | | -40 to 85 | ACT86 | Obsolete
SN74ACT86DBR | Reel of 2000 | -40 to 85 | AD86
SN74ACT86DR | Reel of 2500 | -40 to 85 | ACT86
SN74ACT86DRG4 | Reel of 2500 | -40 to 85 | ACT86
SN74ACT86N | Tube of 25 | -40 to 85 | SN74ACT86N
SN74ACT86NSR | Reel of 2000 | -40 to 85 | ACT86 | NRND
SN74ACT86PW | | -40 to 85 | AD86 | Obsolete
SN74ACT86PWR | Reel of 2000 | -40 to 85 | AD86
SN74ACT86PWRG4 | Reel of 2000 | -40 to 85 | AD86
SNJ54ACT86FK | Tube of 55 | -55 to 125 | 5962-9068701Q2A SNJ54ACT86FK
SNJ54ACT86J | Tube of 25 | -55 to 125 | 5962-9068701QCA SNJ54ACT86J
SNJ54ACT86W | Tube of 25 | -55 to 125 | 5962-9068701QDA SNJ54ACT86W
```
[[12::table #1]]

## Conversion notes

- Source pages 1–9 converted; the support and revision history pages
  (10–11), the package option addendum, tape and reel and mechanical
  pages (12–38) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are the parts' rows of the package option
  addendum (p.12 and p.13).
- Dropped: device information table, the exclusive-OR symbol drawings
  (p.1 and Figure 6-1), the layout example drawing (Figure 7-1,
  summarized in one sentence).
- The electrical characteristics table of the source gives only symbols
  in its Parameter column; no parameter names are added. The recommended
  operating conditions print no conditions column.
- The thermal table prints "—" for every package but BQA in the rows
  after R~θJA~; those cells are empty here.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- From images: 500 Ω (both load resistors), 2 × V~CC~ (S1), 50 pF (C~L~),
  3 V, 0 V and 1.5 V (input), 50 % V~CC~ (outputs) (Figure 5-1, p.7); the
  overbars of Y = !{A}B + A!{B} (Description, p.1).
- The pin functions table and the thermal table each continue on the
  next page (pp.3–4 and pp.5–6); each is one table here.
- Note B prints "t~r~ v 2.5ns, t~f~ v 2.5ns", the old TI symbol-font
  glyph for ≤; written ≤.
- Source issues: none found.
