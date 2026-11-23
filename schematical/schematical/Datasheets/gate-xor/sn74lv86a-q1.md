---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv86a-q1",
  "title": "SN74LV86A-Q1 automotive quadruple 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74lv86a-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS534D",
    "date": "2023-08",
    "pages": 17
  },
  "devices": ["gate:74_86"],
  "parts": ["SN74LV86ATPWRG4Q1"],
  "packages": {"p14": "PW"},
  "converted": "2026-11-21"
}
---

# SN74LV86A-Q1 automotive quadruple 2-input exclusive-OR gate

## Description

The SN74LV86A-Q1 is a quadruple 2-input exclusive-OR gate designed for
2-V to 5.5-V V~CC~ operation. [[8::1]]

Qualified for automotive applications. Support mixed-mode voltage
operation on all ports. [[1::1]]

Conversion note: the logic symbol (p.1) shows the four exclusive-OR
gates (=1) with their pin numbers. This symbol is in accordance with
ANSI/IEEE Std 91-1984 and IEC Publication 617-12.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Input A
1B, 2B, 3B, 4B: Input B
1Y, 2Y, 3Y, 4Y: Output Y
GND: Ground
VCC: Supply voltage
```
[[3::table #1]]

## Functional description

This device contains four independent 2-input exclusive-OR gates. It
performs the Boolean function Y = A ⊕ B or Y = !{A}B + A!{B} in positive
logic. Source error: the source draws each bar over a whole product,
!{AB} + !{AB} (p.8); the function table (Table 7-1, p.8) and Y = A ⊕ B
show an exclusive-OR, so one bar is over A and the other over B. [[8::2]]

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the output.
If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[8::3]]

Conversion note: the functional block diagram (Figure 7-1, a picture)
shows five equivalent exclusive-OR symbols and the logic-identity,
even-parity and odd-parity elements.

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output [2] |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```
[[8::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Voltage applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | -25 | 25 | mA |
| | Continuous current through V~CC~ or GND | | -50 | 50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value is limited to 5.5-V maximum.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ x 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ x 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ x 0.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ x 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ x 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ x 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 105 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | PW, 14 pins | Unit |
|---|---|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 113 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = -50 μA | 2 to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -2 mA | 2.3 V | 2 | | | V |
| ^ | ^ | I~OH~ = -6 mA | 3 V | 2.48 | | | V |
| ^ | ^ | I~OH~ = -12 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 μA | 2 to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | V |
| ^ | ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | μA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | μA |
| I~off~ | Input/Output Power-Off Leakage Current | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | μA |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 1.4 | | pF |
```
[[5::table #3]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.1 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 8.4 | pF |
| ^ | ^ | ^ | 5 V | 8.8 | pF |
```
[[6::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 2.5 V ± 0.2 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 50 pF | | 10.5 | 22.6 | 1 | 26.5 | ns |
```
[[5::table #4]]

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 50 pF | | 7.4 | 14.5 | 1 | 16.5 | ns |
```
[[5::table #5]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 50 pF | | 5.3 | 8.8 | 1 | 10 | ns |
```
[[6::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices. [[7::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 50 pF
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```
[[7::Figure 6-1]]

Conversion note: Figure 6-1 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse
duration, setup and hold times, and enable and disable times.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LV86ATPWRG4Q1 | Reel of 2000 | -40 to 105 | LV86AT
```
[[10::table #1]]

## Conversion notes

- Source pages 1–8 converted; the support page (9), the package option
  addendum (pp.10–11), tape and reel and mechanical pages (12–17)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: features list (two feature lines are in Description), package
  information table, the logic symbol (p.1) and the functional block
  diagram (Figure 7-1), each described in one sentence, the vendor
  pinout, revision history.
- The Overview's first sentence (p.8) repeats the Description (p.1) and
  is given once, under Description. The Overview names the device
  "SN74LV86A"; the data sheet's title is SN74LV86A-Q1.
- The pin functions table gives only the pin name as the description
  (except "1A input"); descriptions are written from the pin names.
- The recommended operating conditions table runs over two pages
  (pp.4–5) and is one table here; its note is printed once, on p.5.
- The switching tables' second Min/Max group has no heading in the
  source.
- The C~i~ typical value sits across the Typ and Max columns in the
  source; it is placed under Typ.
- The load figure prints no C~L~ value; 50 pF is the load capacitance of
  the switching tables (pp.5–6).
- From images: the load circuit and waveform values (R~L~ = 1 kΩ, 50%
  V~CC~) in Figure 6-1, p.7, which is an image; the overbars of
  Y = !{A}B + A!{B} and of !{AB} + !{AB} (p.8).
- Source issues:
  - Source error: the Overview (p.8) draws the second form of the
    Boolean function with a bar over each whole product, !{AB} + !{AB};
    the function table (Table 7-1, p.8) shows an exclusive-OR. Corrected
    to !{A}B + A!{B}.
