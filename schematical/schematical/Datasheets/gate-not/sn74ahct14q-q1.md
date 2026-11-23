---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct14q-q1",
  "title": "SN74AHCT14Q-Q1 automotive hex Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahct14q-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SGDS023D",
    "date": "2023-10",
    "pages": 24
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74AHCT14QDRG4Q1", "SN74AHCT14QDRQ1", "SN74AHCT14QPWRG4Q1", "SN74AHCT14QPWRQ1"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74AHCT14Q-Q1 automotive hex Schmitt-trigger inverter

## Description

The SN74AHCT14Q-Q1 contains six independent inverters. This device
performs the Boolean function Y = !{A}. [[1::1]]

Qualified for automotive applications. EPIC™ (Enhanced-Performance
Implanted CMOS) process. Inputs are TTL-Voltage compatible. Latch-Up
performance exceeds 250 mA per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Input 1A
1Y: Output 1Y
2A: Input 2A
2Y: Output 2Y
3A: Input 3A
3Y: Output 3Y
4Y: Output 4Y
4A: Input 4A
5Y: Output 5Y
5A: Input 5A
6Y: Output 6Y
6A: Input 6A
GND: Ground Pin
VCC: Power Pin
```
[[3::table #1]]

## Functional description

Each circuit functions as an independent inverter, but because of the
Schmitt action, the inverters have different input threshold levels for
positive-going (V~T+~) and for negative-going (V~T−~) signals. [[7::1]]

Conversion note: the logic diagram (Figure 8-1) shows one Schmitt-trigger inverter, input A, output Y, and the logic symbol of the six inverters with their pin numbers.

The ✝ symbol is in accordance with ANSI/IEEE Std 91-1984 and IEC
Publication 617-12. [[7::2]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[7::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | (V~I~ < 0) | | -20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

Source error: the source prints the marker 1 on the V~I~ and V~O~ rows (p.4); note 2, on the input and output voltage ratings, is the one that applies to them and has no other marker, so the marker is written 2.

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 | ±200 | ^ |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over recommended operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHCT14Q-Q1"
| Symbol | Thermal metric [1] | D 14 pins | PW 14 pins | BQA 14 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 147.7 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 4.5 V | 0.9 | | 1.9 | 0.9 | 1.9 | V |
| ^ | ^ | | 5.5 V | 1 | | 2.1 | 1 | 2.1 | ^ |
| V~T-~ | Negative-going input threshold voltage | | 4.5 V | 0.5 | | 1.5 | 0.5 | 1.5 | V |
| ^ | ^ | | 5.5 V | 0.6 | | 1.7 | 0.6 | 1.7 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ - V~T-~) | | 4.5 V | 0.4 | | 1.4 | 0.4 | 1.4 | V |
| ^ | ^ | | 5.5 V | 0.4 | | 1.5 | 0.4 | 1.5 | ^ |
| V~OH~ | | I~OH~ = -50 μA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | ^ |
| V~OL~ [2] | | I~OL~ = 50 μA | 4.5 V | | | 0.1 | | 0.1 | v |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | μA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | μA |
| ΔI~CC~ [1] | | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels rather than 0 V or V~CC~.
[2] Source error: the source prints the unit of V~OL~ as a lower-case v (p.5), kept as printed; the V~OH~ rows print V.
```
[[5::table #1]]

Conversion note: the source prints no temperature heading over the second Min and Max columns of this table and of the switching characteristics table.

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.9 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.7 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.1 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.5 | V |

[1] Characteristics are for surface-mount packages only.
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```
[[5::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 4 | 7 | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4 | 7 | 1 | 8 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.5 | 8 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 8 | 1 | 9 | ^ |
```
[[5::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. [[6::1]]

Conversion note: in Figure 7-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[6::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[6::Figure 7-1]]

Conversion note: the source figure (Figure 7-1) also shows the load
circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1
table, and the pulse-duration, setup-and-hold and enable/disable
waveforms with their note B, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[8::1]]

Conversion note: in the typical application block diagram (Figure 9-1) one AND gate combines !{OC} (over current detection) with PG (power supply), a second combines ON/!{OFF} (on/off switch) with !{OT} (over temp detection), and a third combines the two results into the !{RESET} input of a motor controller.

Unconfirmed: Figure 9-1 (p.8) draws three 2-input AND gates and no inverter, while the SN74AHCT14Q-Q1 holds six inverters; the source has no text on this application.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in the following
layout example. [[8::3]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used). Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient. [[8::4]]

Conversion note: the example layout (Figure 9-2) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 μF bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and avoiding 90° corners for signal lines.

Source conflict: Figure 9-2 (p.9), the example layout for the SN74AHCT14Q-Q1, letters the pins 1A, 1B, 1Y to 4B, 4A, 4Y, as on a quadruple 2-input gate; the pin table (p.3) has six inverters, 1A, 1Y to 6A, 6Y.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AHCT14QDRG4Q1 | Reel of 2500 | -40 to 125 | AHCT14Q
SN74AHCT14QDRQ1 | Reel of 2500 | -40 to 125 | AHCT14Q
SN74AHCT14QPWRG4Q1 | Reel of 2000 | -40 to 125 | HB14Q
SN74AHCT14QPWRQ1 | Reel of 2000 | -40 to 125 | HB14Q
```
[[10::table #1]]

## Conversion notes

- Source pages 1–9 converted; p.2 holds only the table of contents and
  the revision history, which are dropped. The ordering lines are from
  the package option addendum (pp.10–11). The support sections and the
  mechanical information section on p.9, tape and reel and mechanical
  pages (12–24) are left out; the generated Package section replaces
  them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the vendor
  pin drawings (p.3), the logic diagram on p.1 (one Schmitt-trigger
  inverter); the logic diagram (Figure 8-1), the typical application
  block diagram (Figure 9-1) and the example layout (Figure 9-2) are
  described in text.
- The source also covers the BQA (WQFN, 14) package, with a thermal pad
  (PAD in Figure 5-2), and the addendum lists SN74AHCT14QWBQARQ1; the
  catalog doesn't carry that part. The pin table's NC row (no pin
  number) is not in the D or PW pinout, and its Thermal Pad row, with
  the note "For BQA package only.", is left out with that package.
- The electrical characteristics table of the source prints a parameter
  name only for V~T+~, V~T-~ and ΔV~T~; the other Parameter cells are
  empty. The absolute maximum ratings print their conditions in
  parentheses.
- Figure 7-1 (p.6) is a picture with no text layer; the load circuit and
  the propagation delay waveforms were compared with the page image. Its
  S1 table is printed as text under the figure and is left out with the
  3-state load circuit.
- From images: the overbar of Y = !{A} (p.1); the overbars of !{OC},
  ON/!{OFF}, !{OT} and !{RESET} and the gate arrangement (Figure 9-1,
  p.8); 0 V, 3 V, 1.5 V, 50% V~CC~ and R~L~ = 1 kΩ (Figure 7-1, p.6);
  0.1 μF (Figure 9-2, p.9; the text layer prints "0.1 F"); the pin
  numbers of the logic symbol (Figure 8-1, p.7).
- Source issues:
  - Source error: the V~I~ and V~O~ rows of the absolute maximum ratings
    carry the marker 1 where note 2 applies (Absolute maximum ratings
    note).
  - Source error: the unit of V~OL~ is printed as a lower-case v; kept
    as printed (electrical characteristics note 2).
  - Source error: the recommended operating conditions line begins with
    an opening parenthesis that is never closed, "(over recommended
    operating free-air temperature range (unless otherwise noted)".
  - Unconfirmed: the typical application block diagram (Figure 9-1)
    draws AND gates and no inverter (Application information note).
  - Source conflict: Figure 9-2 letters the pins as on a quadruple
    2-input gate (Layout guidelines note).
