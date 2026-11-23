---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act00-q1",
  "title": "SN74ACT00-Q1 quadruple 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74act00-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS517B",
    "date": "2025-04",
    "pages": 23
  },
  "devices": ["gate:74_00"],
  "parts": ["SN74ACT00PWRQ1", "SN74ACT00TDRQ1", "SN74ACT00WBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74ACT00-Q1 quadruple 2-input positive-NAND gate

## Description

The SN74ACT00 contains four independent 2-input NAND gates. Each gate
performs the Boolean function of Y = !{A B} or Y = !{A} + !{B} in
positive logic. [[1::1]]

Qualified for automotive applications. ESD protection exceeds 2000V per
MIL-STD-883, Method 3015; exceeds 200V using machine model (C = 20 pF,
R = 0). 4.5V to 5.5V V~CC~ operation. Inputs accept voltages to 5.5V.
Maximum t~pd~ of 8ns at 5V. Inputs are TTL-voltage compatible. [[1::2]]

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

Conversion note: the source has no pin functions table, and these
descriptions are the converter's.

## Functional description

Conversion note: the logic diagram, each gate (positive logic), shows
one 2-input NAND gate, inputs A and B, output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output Y |
| A | B | ^ |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[1::table #2]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage [2] | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current (V~I~ < 0 or V~I~ > V~CC~) | | ±20 | mA |
| I~OK~ | Output clamp current (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current (V~O~ = 0 to V~CC~) | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | ±200 | mA |
| T~stg~ | Storage temperature | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[3::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | mA |
| ∆t/∆v | Input transition rise or fall rate | | 8 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 105 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI Implications of Slow or Floating CMOS Inputs application note.
```
[[3::table #2]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | D (SOIC) 14 pins | PW (TSSOP) 14 pins | BQA (WQFN) 14 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 [2] | 145.7 | 91.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | | 76.5 | 99.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | | 102.0 | 61.0 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | | 18.8 | 14.5 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | | 100.7 | 60.8 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | 37.0 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application note.
[2] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[3::table #3]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.49 | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.49 | | 5.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.86 | | | 3.7 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.7 | | V |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.5 | V |
| I~I~ | | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 40 | µA |
| ΔI~CC~ [1] | | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | 0.6 | | | 1.6 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2.6 | | | | pF |

[1] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[4::table #1]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | | 40 | | pF |
```
[[4::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 4-1)"
| Symbol | From (input) | To (output) | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1.5 | 5.5 | 9 | 1 | 9.5 | ns |
| t~PHL~ | A or B | Y | 1.5 | 4 | 7 | 1 | 8 | ns |
```
[[4::table #3]]

## Parameter measurement information

Conversion note: Figure 4-1 is drawn with the two figures below; the
load circuit also has a 500 Ω resistor from the output to switch S1
(2 × V~CC~ or open), and S1 is open for t~PLH~/t~PHL~, which leaves the
load below. Conversion note: in the voltage waveforms the input swings
0V to 3 V and is measured at 1.5 V, and the outputs are measured at
50% V~CC~.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```
[[5::Figure 4-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```
[[5::Figure 4-1]]

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74ACT00PWRQ1 | Reel of 3000 | -40 to 105 | ACT00Q
SN74ACT00TDRQ1 | Reel of 2500 | -40 to 105 | ACT00TQ1
SN74ACT00WBQARQ1 | Reel of 3000 | -40 to 105 | AD00Q
```
[[9::table #1]]

## Conversion notes

- Source pages 1–5 converted; the support and revision history page
  (6), the tape and reel pages (7–8), the package option addendum and
  mechanical pages (9–23) dropped, replaced by the generated Package and
  Ordering sections. The ordering lines are the parts' rows of the
  package option addendum (p.9).
- Dropped: package information table; the logic diagram (p.5) is
  described in text.
- The source has no pin functions table and no application chapter.
- The electrical characteristics table of the source gives only symbols
  in its Parameter column; no parameter names are added. Its last
  Min/Max columns, and those of the switching characteristics table,
  have no group header in the source. The absolute maximum ratings and
  recommended operating conditions print no conditions column.
- The thermal table prints "—" where a cell is empty here.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text. The load circuit says
  "see Note A", and the source prints no notes under Figure 4-1.
- From images: overbars of Y = !{A B} and Y = !{A} + !{B} (p.1); 3 V, 0 V
  and 1.5 V (input), 50% V~CC~ (outputs) (Figure 4-1, p.5; the text
  layer runs the two 1.5 V labels together).
- Source issues:
  - Source error: the Description names the device SN74ACT00, without
    the -Q1 of the title and of every page header (p.1); kept as
    printed.
