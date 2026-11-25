---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ls86a",
  "title": "SN5486, SN54LS86A, SN54S86, SN7486, SN74LS86A, SN74S86 quadruple 2-input exclusive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74ls86a.pdf",
    "vendor": "Texas Instruments",
    "document": "SDLS124",
    "date": "1988-03",
    "pages": 24
  },
  "devices": ["gate:74_86"],
  "parts": ["SN54LS86AJ", "SN74LS86AD", "SN74LS86ADR", "SN74LS86ADRE4", "SN74LS86AN", "SN74LS86ANE4", "SN74LS86ANSR", "SNJ54LS86AJ", "SNJ54LS86AW", "SN54S86J", "SNJ54S86J", "SNJ54LS86AFK"],
  "packages": {"p14": "D, J, N, NS, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SN5486, SN54LS86A, SN54S86, SN7486, SN74LS86A, SN74S86 quadruple 2-input exclusive-OR gates

## Description

These devices contain four independent 2-input Exclusive-OR gates. They
perform the Boolean functions Y = A ⊕ B = !{A}B + A!{B} in positive
logic. [[1::2]]

A common application is as a true/complement element. If one of the
inputs is low, the other input will be reproduced in true form at the
output. If one of the inputs is high, the signal on the other input
will be reproduced inverted at the output. [[1::3]]

The SN5486, 54LS86A, and the SN54S86 are characterized for operation
over the full military temperature range of –55°C to 125°C. The SN7486,
SN74LS86A, and the SN74S86 are characterized for operation from 0°C to
70°C. [[1::4]]

Package Options Include Plastic "Small Outline" Packages, Ceramic Chip
Carriers and Flat Packages, and Standard Plastic and Ceramic 300-mil
DIPs. [[1::1]]

```table id=types caption="Typical average propagation delay time and typical total power dissipation"
| Type | Typical average propagation delay time | Typical total power dissipation |
|---|---|---|
| '86 | 14 ns | 150 mW |
| 'LS86A | 10 ns | 30.5 mW |
| 'S86 | 7 ns | 250 mW |
```
[[1::table #1]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Gate input A
1B, 2B, 3B, 4B: Gate input B
1Y, 2Y, 3Y, 4Y: Gate output Y
GND: Ground
VCC: Positive supply
NC: No internal connection
```
[[1::figure #1]] [[1::figure #2]]

## Functional description

### Exclusive-OR logic

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. [[1::5]]

These are five equivalent Exclusive-OR symbols valid for an '86 or
'LS86A gate in positive logic; negation may be shown at any two ports. [[1::6]]

```table id=elements caption="Alternative logic symbols"
| Element | Output |
|---|---|
| Logic identity element | The output is active (low) if all inputs stand at the same logic level (i.e., A=B). |
| Even-parity | The output is active (low) if an even number of inputs (i.e., 0 or 2) are active. |
| Odd-parity element | The output is active (high) if an odd number of inputs (i.e., only 1 of the 2) are active. |
```
[[1::table #2]]

Conversion note: the source also shows the schematics of inputs and
outputs (equivalent of each input, typical of all outputs) of the '86,
'LS86A and 'S86, with nominal resistor values, and the logic symbol.

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[2::table #1]]

H = high level, L = low level [[2::table #1]]

## Absolute maximum ratings

```table id=abs-max-86 caption="Absolute maximum ratings, SN5486 and SN7486" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage [1] | | | 7 | V |
| | Input voltage | | | 5.5 | V |
| | Operating free-air temperature range | SN5486 | -55 | 125 | °C |
| ^ | ^ | SN7486 | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Voltage values are with respect to network ground terminal.
```
[[3::table #1]]

```table id=abs-max-ls86a caption="Absolute maximum ratings, SN54LS86A and SN74LS86A" parts=*LS86A* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage [1] | | | 7 | V |
| | Input voltage | | | 7 | V |
| | Operating free-air temperature range | SN54LS86A | -55 | 125 | °C |
| ^ | ^ | SN74LS86A | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Voltage values are with respect to network ground terminal.
```
[[4::table #1]]

```table id=abs-max-s86 caption="Absolute maximum ratings, SN54S86 and SN74S86" parts=SN?4S86*,SNJ54S86* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage [1] | | | 7 | V |
| | Input voltage | | | 5.5 | V |
| | Operating free-air temperature range | SN54S86 | -55 | 125 | °C |
| ^ | ^ | SN74S86 | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Voltage values are with respect to network ground terminal.
```
[[5::table #1]]

## Recommended operating conditions

```table id=roc-86 caption="Recommended operating conditions, SN5486 and SN7486"
| Symbol | Parameter | SN5486 | < | < | SN7486 | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.75 | 5 | 5.25 | V |
| I~OH~ | High-level output current | | | -800 | | | -800 | µA |
| I~OL~ | Low-level output current | | | 16 | | | 16 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |
```
[[3::table #2]]

```table id=roc-ls86a caption="Recommended operating conditions, SN54LS86A and SN74LS86A" parts=*LS86A*
| Symbol | Parameter | SN54LS86A | < | < | SN74LS86A | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.75 | 5 | 5.25 | V |
| I~OH~ | High-level output current | | | -400 | | | -400 | µA |
| I~OL~ | Low-level output current | | | 4 | | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |
```
[[4::table #2]]

```table id=roc-s86 caption="Recommended operating conditions, SN54S86 and SN74S86" parts=SN?4S86*,SNJ54S86*
| Symbol | Parameter | SN54S86 | < | < | SN74S86 | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.75 | 5 | 5.25 | V |
| I~OH~ | High-level output current | | | -1 | | | -1 | mA |
| I~OL~ | Low-level output current | | | 20 | | | 20 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |
```
[[5::table #2]]

## Electrical characteristics

```table id=ec-86 caption="Electrical characteristics, SN5486 and SN7486" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions [1] | < | SN5486 | < | < | SN7486 | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [2] | Max | Min | Typ [2] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IH~ | High-level input voltage | | < | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | < | | | 0.8 | | | 0.8 | V |
| V~IK~ | Input clamp voltage | V~CC~ = MIN, I~I~ = -8 mA | < | | | -1.5 | | | -1.5 | V |
| V~OH~ | High-level output voltage | V~CC~ = MIN, V~IH~ = 2 V, V~IL~ = 0.8 V, I~OH~ = -800 µA | < | 2.4 | 3.4 | | 2.4 | 3.4 | | V |
| V~OL~ | Low-level output voltage | V~CC~ = MIN, V~IH~ = 2 V, V~IL~ = 0.8 V, I~OL~ = 16 mA | < | | 0.2 | 0.4 | | 0.2 | 0.4 | V |
| I~I~ | Input current at maximum input voltage | V~CC~ = MAX, V~I~ = 5.5 V | < | | | 1 | | | 1 | mA |
| I~IH~ | High-level input current | V~CC~ = MAX, V~I~ = 2.4 V | < | | | 40 | | | 40 | µA |
| I~IL~ | Low-level input current | V~CC~ = MAX, V~I~ = 0.4 V | < | | | -1.6 | | | -1.6 | mA |
| I~OS~ | Short-circuit output current [3] | V~CC~ = MAX | < | -20 | | -55 | -18 | | -55 | mA |
| I~CC~ | Supply current | V~CC~ = MAX [4] | < | | 30 | 43 | | 30 | 50 | mA |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions for the applicable type.
[2] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[3] Not more than one output should be shorted at a time.
[4] I~CC~ is measured with the inputs grounded and the outputs open.
```
[[3::table #3]]

```table id=ec-ls86a caption="Electrical characteristics, SN54LS86A and SN74LS86A" parts=*LS86A* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions [1] | < | SN54LS86A | < | < | SN74LS86A | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [2] | Max | Min | Typ [2] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IH~ | High-level input voltage | | < | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | < | | | 0.7 | | | 0.8 | V |
| V~IK~ | Input clamp voltage | V~CC~ = MIN, I~I~ = -18 mA | < | | | -1.5 | | | -1.5 | V |
| V~OH~ | High-level output voltage | V~CC~ = MIN, V~IH~ = 2 V, V~IL~ = V~IL~ max, I~OH~ = -400 µA | < | 2.5 | 3.4 | | 2.7 | 3.4 | | V |
| V~OL~ | Low-level output voltage | V~CC~ = MIN, V~IH~ = 2 V, V~IL~ = V~IL~ max | I~OL~ = 4 mA | | 0.25 | 0.4 | | 0.25 | 0.4 | V |
| ^ | ^ | ^ | I~OL~ = 8 mA | | | | | 0.35 | 0.5 | V |
| I~I~ | Input current at maximum input voltage | V~CC~ = MAX, V~I~ = 7 V | < | | | 0.2 | | | 0.2 | mA |
| I~IH~ | High-level input current | V~CC~ = MAX, V~I~ = 2.7 V | < | | | 40 | | | 40 | µA |
| I~IL~ | Low-level input current | V~CC~ = MAX, V~I~ = 0.4 V | < | | | -0.8 | | | -0.8 | mA |
| I~OS~ | Short-circuit output current [3] | V~CC~ = MAX | < | -20 | | -100 | -20 | | -100 | mA |
| I~CC~ | Supply current | V~CC~ = MAX [4] | < | | 6.1 | 10 | | 6.1 | 10 | mA |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions for the applicable type.
[2] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[3] Not more than one output should be shorted at a time.
[4] I~CC~ is measured with the inputs grounded and the outputs open.
```
[[4::table #3]]

```table id=ec-s86 caption="Electrical characteristics, SN54S86 and SN74S86" parts=SN?4S86*,SNJ54S86* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions [1] | < | SN54S86 | < | < | SN74S86 | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [2] | Max | Min | Typ [2] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IH~ | High-level input voltage | | < | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | < | | | 0.8 | | | 0.8 | V |
| V~IK~ | Input clamp voltage | V~CC~ = MIN, I~I~ = -18 mA | < | | | -1.2 | | | -1.2 | V |
| V~OH~ | High-level output voltage | V~CC~ = MIN, V~IH~ = 2 V, V~IL~ = 0.8 V, I~OH~ = -1 mA | < | 2.5 | 3.4 | | 2.7 | 3.4 | | V |
| V~OL~ | Low-level output voltage | V~CC~ = MIN, V~IH~ = 2 V, V~IL~ = 0.8 V, I~OL~ = 20 mA | < | | | 0.5 | | | 0.5 | V |
| I~I~ | Input current at maximum input voltage | V~CC~ = MAX, V~I~ = 5.5 V | < | | | 1 | | | 1 | mA |
| I~IH~ | High-level input current | V~CC~ = MAX, V~I~ = 2.7 V | < | | | 50 | | | 50 | µA |
| I~IL~ | Low-level input current | V~CC~ = MAX, V~I~ = 0.5 V | < | | | -2 | | | -2 | mA |
| I~OS~ | Short-circuit output current [3] | V~CC~ = MAX | < | -40 | | -100 | -40 | | -100 | mA |
| I~CC~ | Supply current | V~CC~ = MAX [4] | < | | 50 | 75 | | 50 | 75 | mA |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions for the applicable type.
[2] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[3] Not more than one output should be shorted at a time, and duration of the short-circuit should not exceed one second.
[4] I~CC~ is measured with the inputs grounded and the outputs open.
```
[[5::table #3]]

## Switching characteristics

Conversion note: Section 1 belongs to the source data book and is not
part of this PDF.

```table id=sw-86 caption="Switching characteristics, SN5486 and SN7486, V~CC~ = 5 V, T~A~ = 25°C"
| Symbol [1] | From (input) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | A or B | Other input low | C~L~ = 15 pF, R~L~ = 400 Ω [2] | | 15 | 23 | ns |
| t~PHL~ | ^ | ^ | ^ | | 11 | 17 | ns |
| t~PLH~ | A or B | Other input high | ^ | | 18 | 30 | ns |
| t~PHL~ | ^ | ^ | ^ | | 13 | 22 | ns |

[1] t~PLH~ = propagation delay time, low-to-high-level output; t~PHL~ = propagation delay time, high-to-low-level output
[2] Load circuits and voltage waveforms are shown in Section 1.
```
[[3::table #4]]

```table id=sw-ls86a caption="Switching characteristics, SN54LS86A and SN74LS86A, V~CC~ = 5 V, T~A~ = 25°C" parts=*LS86A*
| Symbol [1] | From (input) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | A or B | Other input low | C~L~ = 15 pF, R~L~ = 2 kΩ [2] | | 12 | 23 | ns |
| t~PHL~ | ^ | ^ | ^ | | 10 | 17 | ns |
| t~PLH~ | A or B | Other input high | ^ | | 20 | 30 | ns |
| t~PHL~ | ^ | ^ | ^ | | 13 | 22 | ns |

[1] t~PLH~ = propagation delay time, low-to-high-level output; t~PHL~ = propagation delay time, high-to-low-level output
[2] Load circuits and voltage waveforms are shown in Section 1.
```
[[4::table #4]]

```table id=sw-s86 caption="Switching characteristics, SN54S86 and SN74S86, V~CC~ = 5 V, T~A~ = 25°C" parts=SN?4S86*,SNJ54S86*
| Symbol [1] | From (input) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | A or B | Other input low | C~L~ = 15 pF, R~L~ = 280 Ω [2] | | 7 | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 6.5 | 10 | ns |
| t~PLH~ | A or B | Other input high | ^ | | 7 | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 6.5 | 10 | ns |

[1] t~PLH~ = propagation delay time, low-to-high-level output; t~PHL~ = propagation delay time, high-to-low-level output
[2] Load circuits and voltage waveforms are shown in Section 1.
```
[[5::table #4]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
SN54LS86AJ | Tube of 25 | -55 to 125 | SN54LS86AJ
SN74LS86AD | | 0 to 70 | LS86A | Obsolete
SN74LS86ADR | Reel of 2500 | 0 to 70 | LS86A
SN74LS86ADRE4 | Reel of 2500 | 0 to 70 | LS86A
SN74LS86AN | Tube of 25 | 0 to 70 | SN74LS86AN
SN74LS86ANE4 | Tube of 25 | 0 to 70 | SN74LS86AN
SN74LS86ANSR | Reel of 2000 | 0 to 70 | 74LS86A
SNJ54LS86AJ | Tube of 25 | -55 to 125 | SNJ54LS86AJ
SNJ54LS86AW | Tube of 25 | -55 to 125 | SNJ54LS86AW
SN54S86J | Tube of 25 | -55 to 125 | SN54S86J
SNJ54S86J | Tube of 25 | -55 to 125 | SNJ54S86J
SNJ54LS86AFK | Tube of 55 | -55 to 125 | SNJ54LS86AFK
```
[[7::table #1]] [[8::table #1]] [[9::table #1]]

## Conversion notes

- Source pages 1–5 converted; the package option addendum, tape and
  reel and mechanical pages (6–24) dropped, replaced by the generated
  Package and Ordering sections. Ordering packing, temperature and
  marking columns are from the package option addendum (p.6–9).
- Pages 1–5 are scans with no text layer for their content; every value
  on them was read from the page images (110 to 200 dpi) and checked
  there.
- Dropped: the logic symbol, the five equivalent exclusive-OR symbols
  and the three element symbols (their texts are kept), the schematics
  of inputs and outputs (only named in Functional description, without
  resistor values).
- The legend of the function table (H = high level, L = low level) is
  printed under the table without a marker, so it follows the table as
  a line of text.
- In the absolute maximum ratings the source prints a symbol only for
  the supply voltage; the other Symbol cells are empty.
- In the switching characteristics, the cell "See Note 3" is the marker
  of the note on Section 1, and "See Note 2" in the I~CC~ rows is the
  marker of the note on how I~CC~ is measured.
- The SN5486 and SN7486 tables are kept because the PDF covers them,
  though no SN5486 / SN7486 part is claimed here; the package option
  addendum lists no SN5486 / SN7486 and no SN74S86 orderable.
- The package option addendum also lists JM38510/… and M38510/…
  military parts, which this file doesn't claim.
- From images: overbars of Y = A ⊕ B = !{A}B + A!{B} (description,
  p.1); 300, 14, 10, 7, 150, 30.5, 250, -55, 125, 0, 70 (p.1); the function
  table (p.2); 7, 5.5, -65, 150, 4.5, 5, 4.75, 5.25, -800, 16, 2, 0.8,
  -8, -1.5, 2.4, 3.4, 0.2, 0.4, 1, 40, -1.6, -20, -18, -55, 30, 43, 50,
  15, 400, 23, 11, 17, 18, 13, 22 (p.3); -400, 4, 8, 0.7, 2.5, 2.7,
  0.25, 0.35, 0.5, -0.8, -100, 6.1, 10, 12, 20 (p.4); -1, -1.2, -2,
  -40, 75, 280, 10.5, 6.5 (p.5).
- Source issues:
  - Source error: the description prints "54LS86A" for the SN54LS86A
    (p.1); kept as printed.
  - Source conflict: pages 1–5 are stamped "OBSOLETE - No Longer
    Available"; the package option addendum (p.6–9) lists SN74LS86A,
    SN54LS86A and SN54S86 orderables as Active.
