---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc00-q1",
  "title": "SN74AHC00-Q1 automotive quadruple 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74ahc00-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SGDS013C",
    "date": "2023-06",
    "pages": 25
  },
  "devices": ["gate:74_00"],
  "parts": ["SN74AHC00QDRG4Q1", "SN74AHC00QDRQ1", "SN74AHC00QPWRG4Q1", "SN74AHC00QPWRQ1", "SN74AHC00QWBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74AHC00-Q1 automotive quadruple 2-input positive-NAND gate

## Description

The SN74AHC00 device performs the Boolean function Y = !{A • B} or
Y = !{A} + !{B} in positive logic. [[1::1]]

Qualified for Automotive Applications. Operating Range 2-V to 5.5-V
V~CC~. Latch-Up Performance Exceeds 250 mA Per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
1Y: 1Y Output
2A: 2A Input
2B: 2B Input
2Y: 2Y Output
3A: 3A Input
3B: 3B Input
3Y: 3Y Output
4A: 4A Input
4B: 4B Input
4Y: 4Y Output
GND: GND
VCC: Power Pin
```
[[3::table #1]]

## Functional description

Conversion note: the functional block diagram (Figure 7-1) is the logic diagram of one 2-input NAND gate, inputs A and B, output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output Y |
| A | B | ^ |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[8::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] [3] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [2] [3] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Source error: the source prints the marker 1 on V~I~ and V~O~ (p.4); note (2), on the input and output voltage ratings, has no other marker, so the marker is given as note 2.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 | ±1000 | ^ |

[1] Source error: the source prints the marker 1 on this row (p.4) and no note under the ESD ratings table.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over recommended operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ^ |
| T~A~ | Operating free-air temperature | Q-suffix devices | -40 | 125 | °C |
| ^ | ^ | I-suffix devices | -40 | 85 | ^ |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC00-Q1"
| Symbol | Thermal metric [1] | D (SOIC) 14 pins | PW (TSSOP) 14 pins | BQA (WQFN) 14 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 147.7 | 88.3 | °C/W |

[1] Source error: the source prints the marker 1 on THERMAL METRIC (p.5) and no note under the thermal information table.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = −40 °C to 125°C | < | T~A~ = −40 °C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = −50 μA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | ^ |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | ^ |
| ^ | ^ | I~OH~ = −4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | ^ |
| ^ | ^ | I~OH~ = −8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | | I~OL~ = 50 μA | 2 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | ^ |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | | 0.44 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | ^ |
| I~I~ | | V~l~ = 5.5 V or GND [1] | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | μA |
| I~CC~ | | V~l~ = V~CC~ or GND, I~O~ = 0 [1] | 5.5 V | | | 2 | | 20 | | 20 | μA |
| C~i~ | | V~l~ = V~CC~ or GND [1] | 5 V | | 2 | 10 | | | | 10 | pF |

[1] Source error: the text layer of the source has V~l~, with a lowercase L, for V~I~ in these test conditions (p.5); it is kept as printed.
```
[[5::table #2]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.3 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.3 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.61 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 9.5 | pF |
```
[[6::table #2]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25 °C | < | < | T~A~ = −40 °C to 125°C | < | T~A~ = −40 °C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5.5 | 7.9 | 1 | 9.5 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9.5 | 1 | 9.5 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 8 | 11.4 | 1 | 13 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8 | 11.4 | 1 | 13 | 1 | 13 | ^ |
```
[[5::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25 °C | < | < | T~A~ = −40 °C to 125°C | < | T~A~ = −40 °C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 3.7 | 5.5 | 1 | 6.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.7 | 5.5 | 1 | 6.5 | 1 | 6.5 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.2 | 7.5 | 1 | 8.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.2 | 7.5 | 1 | 8.5 | 1 | 8.5 | ^ |
```
[[5::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::Figure 6-1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[7::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74AHC00QDRG4Q1 | Reel of 2500 | -40 to 125 | AHC00Q
SN74AHC00QDRQ1 | Reel of 2500 | -40 to 125 | AHC00Q
SN74AHC00QPWRG4Q1 | Reel of 2000 | -40 to 125 | HA00Q
SN74AHC00QPWRQ1 | Reel of 2000 | -40 to 125 | HA00Q
SN74AHC00QWBQARQ1 | Reel of 3000 | -40 to 125 | AHC00Q
```
[[11::table #1]]

## Conversion notes

- Source pages 1–8 converted; p.2 holds only the table of contents and
  the revision history, which are dropped. The ordering lines are from
  the package option addendum (pp.11–12). The support pages (9–10), tape
  and reel and mechanical pages (13–25) are left out; the generated
  Package section replaces them.
- Dropped: features list (kept as sentences in the Description),
  package information table, the logic diagram (one NAND gate).
- The pin drawing and pin table (p.3) are for the D or PW package; the
  source has no drawing of the BQA package.
- The absolute maximum ratings of the source print the conditions in
  parentheses; they are put in the Conditions cells.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- Figure 6-1 (p.7) is a picture; only its notes and S1 table are in the
  text layer.
- From images: 50% V~CC~ (V~M~, Figure 6-1, p.7); overbars of
  Y = !{A • B} and Y = !{A} + !{B} (Description, p.1).
- Source issues:
  - Source error: marker 1 printed on V~I~ and V~O~ for note (2) of the
    absolute maximum ratings (table note 3).
  - Source error: V~l~ (lowercase L) for V~I~ in the test conditions of
    I~I~, I~CC~ and C~i~ (electrical characteristics note).
  - Source error: marker 1 on the HBM row of the ESD ratings and on
    THERMAL METRIC, with no note under either table.
