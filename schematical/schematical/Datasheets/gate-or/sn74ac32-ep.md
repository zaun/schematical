---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ac32-ep",
  "title": "SN74AC32-EP quadruple 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ac32-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS720",
    "date": "2003-10",
    "pages": 9
  },
  "devices": ["gate:74_32"],
  "parts": ["SN74AC32MDREP", "V62/04616-01XE"],
  "packages": {"p14": "D"},
  "converted": "2026-11-21"
}
---

# SN74AC32-EP quadruple 2-input positive-OR gate

## Description

The SN74AC32 is a quadruple 2-input positive-OR gate. The device
performs the Boolean function Y = A + B in positive logic. [[1::3]]

Controlled baseline: one assembly/test site, one fabrication site.
2-V to 6-V V~CC~ operation. Inputs accept voltages to 6 V. Max t~pd~ of
7.5 ns at 5 V. [[1::1]]

Extended temperature performance of –55 °C to 125 °C. Enhanced
diminishing manufacturing sources (DMS) support. Enhanced
product-change notification. [[1::1]]

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
[[1::table #1]]

## Functional description

Conversion note: the logic diagram, each gate (positive logic), shows one
2-input OR gate, inputs A and B, output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```
[[1::table #2]]

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

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

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
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | [1] | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 3 V | 2.9 | | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | | | 5.4 | | V |
| ^ | ^ | I~OH~ = -12 mA | 3 V | 2.56 | | | 2.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.86 | | | 3.7 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.7 | | V |
| V~OL~ | | I~OL~ = 50 µA | 3 V | | 0.002 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 12 mA | 3 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.5 | V |
| I~I~ | A or B ports | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 40 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2.6 | | | | pF |

[1] Conversion note: the source prints no temperature heading over this Min and Max column group.
```
[[3::table #1]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 40 | pF |
```
[[3::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted)"
| Symbol | From (input) | To (output) | T~A~ = 25 °C | < | < | [1] | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1.5 | 7 | 9 | 1 | 12 | ns |
| t~PHL~ | A or B | Y | 1.5 | 7 | 8.5 | 1 | 11.5 | ns |

[1] Conversion note: the source prints no temperature heading over this Min and Max column group.
```
[[3::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted)"
| Symbol | From (input) | To (output) | T~A~ = 25 °C | < | < | [1] | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1.5 | 5.5 | 7.5 | 1 | 9 | ns |
| t~PHL~ | A or B | Y | 1.5 | 5 | 7 | 1 | 8.5 | ns |

[1] Conversion note: the source prints no temperature heading over this Min and Max column group.
```
[[3::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 2.5 ns, t~f~ ≤ 2.5 ns. The outputs are measured one
at a time with one input transition per measurement. [[4::1]]

Conversion note: the load circuit also has a 500 Ω resistor from the output
to switch S1 (2 × V~CC~ or open), and S1 is open for t~PLH~/t~PHL~, which
leaves the load below.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```
[[4::Figure 1]]

```figure ref=waveforms-tpd caption="Voltage waveforms"
VM: 50 % V~CC~
```
[[4::Figure 1]]

## Package

```package footprint=SOIC-14
```

## Ordering information

```ordering
SN74AC32MDREP | Reel of 2500 | -55 to 125 | SAC32MEP
V62/04616-01XE | Reel of 2500 | -55 to 125 | SAC32MEP
```
[[5::table #1]]

## Conversion notes

- Source pages 1–4 converted; the package option addendum, tape and
  reel and important notice pages (5–9) dropped, replaced by the
  generated Package and Ordering sections. The ordering lines are the
  parts' rows of the package option addendum (p.5).
- The electrical characteristics table of the source gives only symbols
  in its Parameter column (and "A or B ports" for I~I~); no parameter
  names are added.
- Dropped: features list (the enhanced-product points and electrical
  points are kept in the Description), the qualification-pedigree
  footnote, the page-1 ordering table and its footnote; the logic
  diagram is described in text. The description's second Boolean form
  (Y = A • B with bars over A and B) is left out.
- The source has no pin function table; the pin descriptions are
  generic. It has no thermal table and no application section.
- The text layer prints `"` for ± ("V~CC~ = 3.3 V " 0.3 V"), an old TI
  symbol-font glyph; written ±.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- Source issues:
  - Source error: the second Boolean form on p.1 is drawn with two
    short bars over A and two over B, where the OR function needs one
    bar over each and one over the whole product; kept out.
