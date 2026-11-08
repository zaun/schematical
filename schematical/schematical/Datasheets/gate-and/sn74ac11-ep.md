---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ac11-ep",
  "title": "SN74AC11-EP triple 3-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ac11-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS557",
    "date": "2004-01",
    "pages": 12
  },
  "devices": ["gate:74_11"],
  "parts": ["SN74AC11IPWREP", "V62/04701-01XE"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-08"
}
---

# SN74AC11-EP triple 3-input positive-AND gate

## Description

The SN74AC11 device contains three independent 3-input AND gates. This
device performs the Boolean function Y = A · B · C in positive logic. It
operates from a V~CC~ of 2 V to 6 V, inputs accept voltages to 6 V, and
it is specified from –40 °C to 85 °C. Enhanced product: controlled
baseline (one assembly/test site, one fabrication site), enhanced
diminishing manufacturing sources (DMS) support and enhanced
product-change notification.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A: Gate input A
1B, 2B, 3B: Gate input B
1C, 2C, 3C: Gate input C
1Y, 2Y, 3Y: Gate output Y
GND: Ground
VCC: Supply voltage
```

## Functional description

Logic diagram (positive logic): three 3-input AND gates; 1A, 1B, 1C
(pins 1, 2, 13) drive 1Y (pin 12), 2A, 2B, 2C (pins 3, 4, 5) drive 2Y
(pin 6), and 3A, 3B, 3C (pins 11, 10, 9) drive 3Y (pin 8).

## Function tables

```table id=function caption="Function table (each gate)"
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | H |
| L | X | X | L |
| X | L | X | L |
| X | X | L | L |
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
| θ~JA~ | Package thermal impedance [3] | D package | | 86 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
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
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | Full range [2] | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 3 V | 2.9 | 2.99 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.49 | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.49 | | 5.4 | | V |
| ^ | ^ | I~OH~ = -12 mA | 3 V | 2.56 | | | 2.46 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.86 | | | 3.76 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.76 | | V |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | | 3.85 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 3 V | | 0.002 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 12 mA | 3 V | | | 0.36 | | 0.44 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.44 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.44 | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | 1.65 | V |
| I~I~ | Input current | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 2.6 | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 10 ms.
[2] The source leaves this column group without a temperature heading; the table applies over the recommended operating free-air temperature range (T~A~ –40 °C to 85 °C).
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
| t~PLH~ | A, B, or C | Y | 1.5 | 5.5 | 9.5 | 1 | 10 | ns |
| t~PHL~ | A, B, or C | Y | 1.5 | 5.5 | 8.5 | 1 | 9.5 | ns |

[1] No temperature heading in the source (see the electrical characteristics).
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted)"
| Symbol | From (input) | To (output) | T~A~ = 25 °C | < | < | Full range [1] | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A, B, or C | Y | 1.5 | 4 | 8 | 1 | 8.5 | ns |
| t~PHL~ | A, B, or C | Y | 1.5 | 4 | 7 | 1 | 7.5 | ns |

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

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AC11IPWREP | Tape and reel | -40 to 85 | SAC11EP
```

## Conversion notes

- Source pages 1–4 converted; the package option addendum, tape and
  reel and mechanical pages (5–12) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: features list (the enhanced-product points and electrical
  points are kept in the Description), the qualification-pedigree
  footnote; the logic diagram is described in text. The description's
  second Boolean form (Y = A + B + C with inverted inputs and output,
  set as overbars) is left out.
- The source's ordering table also lists SN74AC11IDREP (SOIC, marked
  Product Preview), which the catalog doesn't carry.
- The source has no pin function table; the pin descriptions are
  generic. It has no application section.
- Catalog pin names: the source's pinout and logic diagram put 3A on
  pin 11 and 3C on pin 9; the catalog pinout drawn here has 3A on pin 9
  and 3C on pin 11. The three inputs of a gate perform the same
  function.
- The text layer prints `"` for ± ("V~CC~ = 3.3 V " 0.3 V") and
  "t~r~ v 2.5 ns" for ≤, old TI symbol-font glyphs; written ± and ≤.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- Source issues: none found.
