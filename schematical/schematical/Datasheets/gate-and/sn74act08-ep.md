---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act08-ep",
  "title": "SN74ACT08-EP quadruple 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74act08-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS779",
    "date": "2004-09",
    "pages": 12
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74ACT08IDREP", "V62/04759-01XE"],
  "packages": {"p14": "D"},
  "converted": "2026-11-08"
}
---

# SN74ACT08-EP quadruple 2-input positive-AND gate

## Description

The SN74ACT08 is a quadruple 2-input positive-AND gate. This device
performs the Boolean function Y = A · B in positive logic. It operates
from a V~CC~ of 4.5 V to 5.5 V; inputs are TTL-voltage compatible and
accept voltages to 5.5 V. It is specified from –40 °C to 85 °C.
Enhanced product: controlled baseline (one assembly/test site, one
fabrication site), enhanced diminishing manufacturing sources (DMS)
support and enhanced product-change notification.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Gate input A
1B, 2B, 3B, 4B: Gate input B
1Y, 2Y, 3Y, 4Y: Gate output Y
GND: Ground
VCC: Supply voltage
```

## Functional description

Logic diagram, each gate (positive logic): one 2-input AND gate, inputs
A and B, output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| θ~JA~ | Package thermal impedance [3] | | | 86 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 8 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | Full range [3] | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.49 | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.49 | | 5.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.86 | | | 3.76 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.76 | | V |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | | 3.85 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 4.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.44 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.44 | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | 1.65 | V |
| I~I~ | Input current | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| ΔI~CC~ | Supply-current change [2] | One input at 3.4 V, other inputs at GND or V~CC~ | 5.5 V | | 0.6 | | | 1.5 | mA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 4.5 | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 2 ms.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
[3] The source leaves this column group without a temperature heading; the table applies over the recommended operating free-air temperature range (T~A~ –40 °C to 85 °C).
```

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 20 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted)"
| Symbol | From (input) | To (output) | T~A~ = 25 °C | < | < | Full range [1] | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1 | 6.5 | 9 | 1 | 10 | ns |
| t~PHL~ | A or B | Y | 1 | 6.5 | 9 | 1 | 10 | ns |

[1] No temperature heading in the source (see the electrical characteristics).
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 2.5 ns, t~f~ ≤ 2.5 ns. The outputs are measured one
at a time with one input transition per measurement.

The load circuit also has a 500 Ω resistor from the output to switch S1
(2 × V~CC~ or open); S1 is open for t~PLH~ and t~PHL~, which leaves the
load below. The input swings 0 V to 3 V and is measured at 1.5 V; the
outputs are measured at 50 % V~CC~.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: 1.5 V (input), 50 % V~CC~ (outputs)
```

## Package

```package footprint=SOIC-14
```

## Ordering information

```ordering
SN74ACT08IDREP | Tape and reel | -40 to 85 | SACT08IEP
```

## Conversion notes

- Source pages 1–4 converted; the package option addendum, tape and
  reel and mechanical pages (5–12) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: features list (the enhanced-product points and electrical
  points are kept in the Description), the qualification-pedigree
  footnote; the logic diagram is described in text. The description's
  second Boolean form (Y = A + B with inverted inputs and output, set
  as overbars) is left out.
- The source has no pin function table; the pin descriptions are
  generic. It has no application section.
- The text layer prints `"` for ± (I~I~ row, "V~CC~ = 5 V " 0.5 V") and
  "t~r~ v 2.5 ns" for ≤, old TI symbol-font glyphs; written ± and ≤.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- Source issues: none found.
