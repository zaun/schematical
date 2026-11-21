---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act00",
  "title": "SNx4ACT00 quadruple 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74act00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS523F",
    "date": "2025-04",
    "pages": 37
  },
  "devices": ["gate:74_00"],
  "parts": ["5962-8769901MCA", "5962-8769901MDA", "SN74ACT00BQAR", "SN74ACT00D", "SN74ACT00DBR", "SN74ACT00DR", "SN74ACT00DR1G4", "SN74ACT00N", "SN74ACT00NE4", "SN74ACT00NSR", "SN74ACT00PW", "SN74ACT00PWR", "SN74ACT00PWRG4", "SNJ54ACT00J", "SNJ54ACT00W", "5962-8769901M2A", "SNJ54ACT00FK"],
  "packages": {"p14": "D, DB, N, NS, PW, J, W, BQA", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4ACT00 quadruple 2-input positive-NAND gates

## Description

The ‘ACT00 devices contain four independent 2-input NAND gates. Each
gate performs the Boolean function of Y = !{A•B} or Y = !{A}+!{B} in
positive logic.

4.5V to 5.5V V~CC~ operation. Inputs accept voltages to 5.5V. Maximum
t~pd~ of 8ns at 5V. Inputs are TTL-voltage compatible.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2B: Channel 2, Input B
2Y: Channel 2, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
3Y: Channel 3, Output Y
4A: Channel 4, Input A
4B: Channel 4, Input B
4Y: Channel 4, Output Y
GND: Ground
VCC: Positive Supply
NC: Not internally connected
```

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. Conversion note: the source has
this for the thermal pad, BQA package only.

Source error: the pin table gives the rows 3A, 3B, 3Y the types O, I, I
and the descriptions "Channel 3, Output Y", "Channel 3, Input A",
"Channel 3, Input B", and the rows 4A, 4B, 4Y likewise; the rows of
channels 1 and 2 and the pin drawings (3Y on pin 8, 4Y on pin 11) show
that 3A and 4A are Input A, 3B and 4B are Input B, and 3Y and 4Y are
Output Y, as written above.

## Functional description

Conversion note: the logic diagram, each gate (positive logic), shows
one 2-input NAND gate, inputs A and B, output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) | | ±20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | SN54ACT00 | < | SN74ACT00 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 8 | | 8 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SNx4ACT00 | < | < | < | < | < | Unit |
| ^ | ^ | D (SOIC) 14 pins | DB (SSOP) 14 pins | N (PDIP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | BQA (WQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 119.9 | 96 | 80 | 76 | 145.7 | 91.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | | | | | | 99.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | | | | | | 61.0 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | | | | | | 14.5 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | | | | | | 60.8 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | 37.0 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application note.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54ACT00 | < | SN74ACT00 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.49 | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.49 | | 5.4 | | 5.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.86 | | | 3.7 | | 3.76 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.7 | | 4.76 | | V |
| ^ | ^ | I~OH~ = -50 mA [1] | 5.5 V | | | | 3.85 | | | | V |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | | | | 3.85 | | V |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA [3] | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 50 mA [1] [3] | 5.5 V | | | | | 1.65 | | | V |
| ^ | ^ | I~OL~ = 75 mA [1] [3] | 5.5 V | | | | | | | 1.65 | V |
| I~I~ | | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 40 | | 20 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | 0.6 | | | 1.6 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5V | | 2.6 | | | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 2 ms.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
[3] Source error: the source prints I~OH~ = 24 mA, I~OH~ = 50 mA and I~OH~ = 75 mA in these V~OL~ rows; the currents are positive and the recommended operating conditions give I~OL~ = 24 mA, so they are I~OL~.
```

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 40 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | T~A~ = 25°C | < | < | SNx4ACT00 [1] | < | SNx4ACT00 [1] | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1.5 | 5.5 | 9 | 1 | 9.5 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | 1.5 | 4 | 7 | 1 | 7 | 1 | 8 | ns |

[1] Unconfirmed: the source labels both column groups SNx4ACT00 and gives no temperature range for either.
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤2.5 ns, t~f~ ≤ 2.5 ns. The outputs are measured one
at a time with one input transition per measurement.

Conversion note: the load circuit also has a 500 Ω resistor from the output
to switch S1 (2 × V~CC~ or open), and S1 is open for t~PLH~/t~PHL~, which
leaves the load below. Conversion note: in the voltage waveforms the input
swings 0 V to 3 V and is measured at 1.5 V, and the outputs are measured at
50% V~CC~.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in the following
layout example.

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used. Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient.

Conversion note: Figure 7-1, the example layout for the SNx4ACT00
(p.8), recommends a GND flood fill, a bypass capacitor placed close to
the device, unused inputs tied to V~CC~, an unused output left floating,
and no 90° corners for signal lines.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-8769901M2A | Tube of 55 | -55 to 125 | 5962-8769901M2A SNJ54ACT00FK | Last Time Buy
5962-8769901MCA | Tube of 25 | -55 to 125 | 5962-8769901MCA SNJ54ACT00J | NRND
5962-8769901MDA | Tube of 25 | -55 to 125 | 5962-8769901MDA SNJ54ACT00W
SN74ACT00BQAR | Reel of 3000 | -40 to 85 | AD00
SN74ACT00D | | -40 to 85 | ACT00 | Obsolete
SN74ACT00DBR | Reel of 2000 | -40 to 85 | AD00 | NRND
SN74ACT00DR | Reel of 2500 | -40 to 85 | ACT00
SN74ACT00DR1G4 | Reel of 2500 | -40 to 85 | ACT00
SN74ACT00N | Tube of 25 | -40 to 85 | SN74ACT00N
SN74ACT00NE4 | Tube of 25 | -40 to 85 | SN74ACT00N
SN74ACT00NSR | Reel of 2000 | -40 to 85 | ACT00
SN74ACT00PW | | -40 to 85 | AD00 | Obsolete
SN74ACT00PWR | Reel of 2000 | -40 to 85 | AD00
SN74ACT00PWRG4 | Reel of 2000 | -40 to 85 | AD00
SNJ54ACT00FK | Tube of 55 | -55 to 125 | 5962-8769901M2A SNJ54ACT00FK | Last Time Buy
SNJ54ACT00J | Tube of 25 | -55 to 125 | 5962-8769901MCA SNJ54ACT00J | NRND
SNJ54ACT00W | Tube of 25 | -55 to 125 | 5962-8769901MDA SNJ54ACT00W
```

## Conversion notes

- Source pages 1–8 converted; the support and revision history pages
  (9–10), the package option addendum, tape and reel and mechanical
  pages (11–37) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are the parts' rows of the package option
  addendum (p.11 and p.12).
- Dropped: device information table, the layout example drawing
  (Figure 7-1, summarized in one sentence); the logic diagram (p.1 and
  Figure 6-1) is described in text.
- The electrical characteristics table of the source gives only symbols
  in its Parameter column; no parameter names are added. The recommended
  operating conditions print no conditions column.
- The thermal table (pp.5–6) prints "—" for every package but BQA in the
  rows after R~θJA~; those cells are empty here. Its N column is headed
  "N (PDIP", without the closing parenthesis.
- The pin functions table continues on the next page (pp.3–4).
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- From images: overbars of Y = !{A•B} and Y = !{A}+!{B} (p.1); 500 Ω
  (both load resistors), 2 × V~CC~ (S1), 50 pF (C~L~), 3 V, 0 V and
  1.5 V (input), 50% V~CC~ (outputs) (Figure 5-1, p.7).
- Source issues:
  - Source error: pin functions table, types and descriptions of the
    3A, 3B, 3Y, 4A, 4B and 4Y rows (p.3); see Pin functions.
  - Source error: electrical characteristics, I~OH~ printed for I~OL~ in
    the V~OL~ rows at 24 mA, 50 mA and 75 mA (p.6); see the table note.
  - Unconfirmed: both column groups of the switching characteristics
    table are labelled SNx4ACT00 (p.6); see the table note.
