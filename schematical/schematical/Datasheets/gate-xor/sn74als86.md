---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74als86",
  "title": "SN54ALS86, SN54AS86A, SN74ALS86, SN74AS86A quadruple 2-input exclusive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74als86.pdf",
    "vendor": "Texas Instruments",
    "document": "SDAS006B",
    "date": "1994-12",
    "pages": 20
  },
  "devices": ["gate:74_86"],
  "parts": ["5962-8862101CA", "5962-8862101DA", "SN54ALS86J", "SN74ALS86D", "SN74ALS86DR", "SN74ALS86N", "SN74ALS86NSR", "SNJ54ALS86J", "SNJ54ALS86W", "SN74AS86AD", "SN74AS86AN"],
  "packages": {"p14": "D, J, N, NS, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SN54ALS86, SN54AS86A, SN74ALS86, SN74AS86A quadruple 2-input exclusive-OR gates

## Description

These devices contain four independent 2-input exclusive-OR gates. They
perform the Boolean functions Y = A ⊕ B or Y = !{A}B + A!{B} in positive
logic. [[1::2]]

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the output.
If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[1::3]]

The SN54ALS86 and SN54AS86A are characterized for operation over the
full military temperature range of –55°C to 125°C. The SN74ALS86 and
SN74AS86A are characterized for operation from 0°C to 70°C. [[1::4]]

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
[[1::table #1]]

## Functional description

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These are five
equivalent exclusive-OR symbols valid for an ′ALS86 or ′AS86A gate in
positive logic. Negation may be shown at any two ports. Conversion note:
the five symbols are not drawn here. [[2::1]] [[2::2]]

The output is active (low) if all inputs are at the same logic level
(i.e., A = B). Conversion note: this is the logic-identity element
(symbol =). [[2::3]]

The output is active (low) if an even number of inputs (i.e., 0 or 2)
are active. Conversion note: this is the even-parity element (symbol
2k). [[2::4]]

The output is active (high) if an odd number of inputs (i.e., only 1 of
the 2) are active. Conversion note: this is the odd-parity element
(symbol 2k + 1). [[2::5]]

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
[[1::table #2]]

## Absolute maximum ratings

```table id=abs-max-als caption="Absolute maximum ratings, SN54ALS86 and SN74ALS86 [1]" parts="*ALS86*, 5962-88621*" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54ALS86 | -55 | 125 | °C |
| ^ | ^ | SN74ALS86 | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```
[[3::table #1]]

```table id=abs-max-as caption="Absolute maximum ratings, SN54AS86A and SN74AS86A [1]" parts=*AS86A* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54AS86A | -55 | 125 | °C |
| ^ | ^ | SN74AS86A | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```
[[4::table #1]]

## Recommended operating conditions

```table id=roc-als caption="Recommended operating conditions, SN54ALS86 and SN74ALS86" parts="*ALS86*, 5962-88621*"
| Symbol | Parameter | SN54ALS86 | < | < | SN74ALS86 | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.7 | | | 0.8 | V |
| I~OH~ | High-level output current | | | -0.4 | | | -0.4 | mA |
| I~OL~ | Low-level output current | | | 4 | | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |
```
[[3::table #2]]

```table id=roc-as caption="Recommended operating conditions, SN54AS86A and SN74AS86A" parts=*AS86A*
| Symbol | Parameter | SN54AS86A | < | < | SN74AS86A | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.8 | | | 0.8 | V |
| I~OH~ | High-level output current | | | -2 | | | -2 | mA |
| I~OL~ | Low-level output current | | | 20 | | | 20 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |
```
[[3::table #3]]

## Electrical characteristics

```table id=ec-als caption="Electrical characteristics, SN54ALS86 and SN74ALS86" parts="*ALS86*, 5962-88621*" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54ALS86 | < | < | SN74ALS86 | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.5 | | | -1.5 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -0.4 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 4 mA | | 0.25 | 0.4 | | 0.25 | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | | | | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.1 | | | -0.1 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -20 | | -112 | -30 | | -112 | mA |
| I~CC~ | V~CC~ = 5.5 V | All inputs at 4.5 V | | 3.9 | 5.9 | | 3.9 | 5.9 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```
[[3::table #4]]

```table id=ec-as caption="Electrical characteristics, SN54AS86A and SN74AS86A" parts=*AS86A* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54AS86A | < | < | SN74AS86A | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.2 | | | -1.2 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -2 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 20 mA | | 0.35 | 0.5 | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.5 | | | -0.5 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -30 | | -112 | -30 | | -112 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I(A)~ = 4.5 V, V~I(B)~ = 0 | | 11 | 18 | | 11 | 18 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 20 | 38 | | 20 | 38 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```
[[4::table #2]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-als caption="Switching characteristics, SN54ALS86 and SN74ALS86 (see Figure 1)" parts="*ALS86*, 5962-88621*" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54ALS86 | < | SN74ALS86 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B (other input low) | Y | 3 | 22 | 3 | 17 | ns |
| t~PHL~ | ^ | ^ | 2 | 14 | 2 | 12 | ns |
| t~PLH~ | A or B (other input high) | Y | 3 | 22 | 3 | 17 | ns |
| t~PHL~ | ^ | ^ | 2 | 12 | 2 | 10 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```
[[3::table #5]]

```table id=sw-as caption="Switching characteristics, SN54AS86A and SN74AS86A (see Figure 1)" parts=*AS86A* conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54AS86A | < | SN74AS86A | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B (other input low) | Y | 2 | 8.5 | 2 | 7.5 | ns |
| t~PHL~ | ^ | ^ | 2 | 8 | 2 | 6.5 | ns |
| t~PLH~ | A or B (other input high) | Y | 1 | 8 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | 1 | 9 | 1 | 7 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```
[[4::table #3]]

## Parameter measurement information

Series 54ALS/74ALS and 54AS/74AS devices. C~L~ includes probe and jig
capacitance. All input pulses have the following characteristics:
PRR ≤ 1 MHz, t~r~ = t~f~ = 2 ns, duty cycle = 50%. The outputs are
measured one at a time with one transition per measurement. [[5::1]]

Conversion note: in the figure the input pulses swing from 0.3 V to 3.5 V and all waveforms are measured at 1.3 V.

```figure ref=load-rl-cl caption="Load circuit for bi-state totem-pole outputs"
RL: 500 Ω
CL: 50 pF
```
[[5::Figure 1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: 1.3 V
```
[[5::Figure 1]]

Conversion note: Figure 1 also shows the load circuits for
open-collector and 3-state outputs and the setup-and-hold,
pulse-duration and enable/disable waveforms, which are not drawn here.

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

## Ordering information

```ordering
5962-8862101CA | Tube of 25 | -55 to 125 | 5962-8862101CA SNJ54ALS86J
5962-8862101DA | Tube of 25 | -55 to 125 | 5962-8862101DA SNJ54ALS86W
SN54ALS86J | Tube of 25 | -55 to 125 | SN54ALS86J
SN74ALS86D | | 0 to 70 | ALS86 | Obsolete
SN74ALS86DR | Reel of 2500 | 0 to 70 | ALS86
SN74ALS86N | Tube of 25 | 0 to 70 | SN74ALS86N
SN74ALS86NSR | Reel of 2000 | 0 to 70 | ALS86
SNJ54ALS86J | Tube of 25 | -55 to 125 | 5962-8862101CA SNJ54ALS86J
SNJ54ALS86W | Tube of 25 | -55 to 125 | 5962-8862101DA SNJ54ALS86W
SN74AS86AD | Tube of 50 | 0 to 70 | AS86A
SN74AS86AN | Tube of 25 | 0 to 70 | SN74AS86AN
```
[[6::table #1]]

## Conversion notes

- Source pages 1–5 converted; the package option addendum, tape and
  reel and mechanical pages (6–20) dropped, replaced by the generated
  Package and Ordering sections.
- The ordering block follows the package option addendum (pp.6–7). The
  catalog carries no FK part of this datasheet; the FK pinout is drawn
  because the source gives it.
- Dropped: the feature list, the logic symbol (four exclusive-OR
  elements, pin numbers for the D, J, and N packages), the five
  exclusive-OR symbol drawings (p.2), and the parts of Figure 1 that
  don't apply to these totem-pole parts: the open-collector and 3-state
  load circuits, setup/hold, pulse-duration and enable/disable
  waveforms, and Notes B and C (3-state outputs).
- The absolute maximum ratings are a list in the source ("Supply
  voltage, V~CC~ . . . 7 V"); its lines are rows with Min and Max here.
- The electrical characteristics tables of the source name their
  parameters by symbol only; no parameter names are added.
- From images: overbars of Y = !{A}B + A!{B} (p.1).
