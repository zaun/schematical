---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ac08-ep",
  "title": "SN74AC08-EP quadruple 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ac08-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS718A",
    "date": "2023-06",
    "pages": 12
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74AC08MDREP"],
  "packages": {"p14": "D"},
  "converted": "2026-11-08"
}
---

# SN74AC08-EP quadruple 2-input positive-AND gate

## Description

The SN74AC08 is a quadruple 2-input positive-AND gate. This device
performs the Boolean function Y = A · B in positive logic. It operates
from a V~CC~ of 2 V to 6 V, inputs accept voltages to 6 V, and it is
specified from –55 °C to 125 °C. Enhanced product: controlled baseline
(one assembly/test site, one fabrication site), enhanced diminishing
manufacturing sources (DMS) support and enhanced product-change
notification.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel 1–4, Input A
1B, 2B, 3B, 4B: Channel 1–4, Input B
1Y, 2Y, 3Y, 4Y: Channel 1–4, Output Y
GND: Ground
VCC: Positive Supply
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
| V~I~ | Input voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Source error: the source marks V~I~ and V~O~ with note 1 (stress ratings); note 2, on the input and output voltage ratings, has no marker. Marked here with note 2.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 3 V | 2.1 | | V |
| ^ | ^ | V~CC~ = 4.5 V | 3.15 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 4.5 V | | 1.35 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~I~ | Input voltage | | 0 | V~CC~ | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 3 V | | -12 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -24 | mA |
| ^ | ^ | V~CC~ = 5.5 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 3 V | | 12 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 24 | mA |
| ^ | ^ | V~CC~ = 5.5 V | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 8 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND for proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AC08-EP [1]"
| Symbol | Parameter | D (SOIC) | Unit |
|---|---|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | °C/W |

[1] 14 pins. For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | Full range [1] | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 3 V | 2.9 | | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | | | 5.4 | | V |
| ^ | ^ | I~OH~ = -12 mA | 3 V | 2.56 | | | 2.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.86 | | | 3.7 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.7 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 3 V | | 0.002 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 12 mA | 3 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.5 | V |
| I~I~ | Input current, A or B ports | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 40 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 4.5 | | | | pF |

[1] The source leaves this column group without a temperature heading; the table applies over the recommended operating free-air temperature range (T~A~ –55 °C to 125 °C).
```

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 20 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted)"
| Symbol | From (input) | To (output) | T~A~ = 25 °C | < | < | Full range [1] | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1.5 | 7.5 | 9.5 | 1 | 12.5 | ns |
| t~PHL~ | A or B | Y | 1.5 | 7 | 8.5 | 1 | 11.5 | ns |

[1] No temperature heading in the source (see the electrical characteristics).
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted)"
| Symbol | From (input) | To (output) | T~A~ = 25 °C | < | < | Full range [1] | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1.5 | 5.5 | 7.5 | 1 | 9 | ns |
| t~PHL~ | A or B | Y | 1.5 | 5.5 | 7 | 1 | 8.5 | ns |

[1] No temperature heading in the source (see the electrical characteristics).
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 2.5 ns, t~f~ ≤ 2.5 ns. The outputs are measured one
at a time with one input transition per measurement.

The load circuit also has a 500 Ω resistor from the output to switch S1
(2 × V~CC~ or open); S1 is open for t~PLH~ and t~PHL~, which leaves the
load below.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: 50 % V~CC~
```

## Package

```package footprint=SOIC-14
```

## Ordering information

```ordering
```

## Conversion notes

- Source pages 1–7 converted; revision history, support pages, the
  package option addendum, tape and reel and mechanical pages (8–12)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: features list (the enhanced-product points and electrical
  points are kept in the Description), the qualification-pedigree
  footnote, package information table; the logic diagram is described in
  text. The description's second Boolean form (Y = A + B with inverted
  inputs and output, set as overbars) is left out.
- The source has no application section.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- From images: 500 Ω (both load resistors), 2 × V~CC~ (S1), 50 pF (C~L~),
  50 % V~CC~ (V~M~) (Figure 6-1, p.6).
- Note B prints "t~r~ v 2.5 ns, t~f~ v 2.5 ns", the old TI symbol-font
  glyph for ≤; written ≤.
- Source issues:
  - Source error: absolute maximum ratings note markers on V~I~ and V~O~
    (see the table note).
