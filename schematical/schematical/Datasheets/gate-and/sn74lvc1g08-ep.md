---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g08-ep",
  "title": "SN74LVC1G08-EP single 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc1g08-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES454C",
    "date": "2006-08",
    "pages": 18
  },
  "devices": ["gate:1G08"],
  "parts": [
    "1P1G08MDBVREPG4", "SN74LVC1G08IDCKREP", "SN74LVC1G08MDBVREP", "SN74LVC1G08MDCKREP",
    "V62/04733-01XE", "V62/04733-02XE", "V62/04733-02YE"
  ],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-08"
}
---

# SN74LVC1G08-EP single 2-input positive-AND gate

## Description

The SN74LVC1G08 performs the Boolean function Y = A · B or
Y = !{!{A} + !{B}} in positive logic. It operates from 1.65 V to 5.5 V
V~CC~ and its inputs accept voltages to 5.5 V.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

This enhanced-product (EP) version is ordered in two temperature grades:
I temperature (–40 °C to 85 °C) and M temperature (–55 °C to 125 °C).

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input
B: Input
GND: Ground
Y: Output
VCC: Supply voltage
```

## Function tables

```table id=function caption="Function table"
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
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | DBV package | | 256 | °C/W |
| ^ | ^ | DCK package | | 252 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```

ESD protection exceeds JESD 22: 2000-V Human-Body Model (A114-A),
200-V Machine Model (A115-A), 1000-V Charged-Device Model (C101).
Latch-up performance exceeds 100 mA per JESD 78, Class II.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 3 V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 3 V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | I temperature | -40 | 85 | °C |
| ^ | ^ | M temperature | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range for both I temp = –40 °C to 85 °C and M temp = –55 °C to 125 °C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | µA |
| I~off~ | Partial-power-down leakage | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | µA |
| I~CC~ | Supply current | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | µA |
| ΔI~CC~ | Supply-current change | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25 °C.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-i-15pf caption="Switching characteristics, I temp, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range of I temp = –40 °C to 85 °C, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.5 | 7.2 | 0.7 | 4.4 | 0.8 | 3.6 | 0.8 | 3.4 | ns |
```

```table id=sw-i-30-50pf caption="Switching characteristics, I temp, C~L~ = 30 pF or 50 pF" conditions="Over recommended operating free-air temperature range of I temp = –40 °C to 85 °C, C~L~ = 30 pF or 50 pF (unless otherwise noted) (see Figure 2)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.4 | 8 | 1.1 | 5.5 | 1 | 4.5 | 1 | 4 | ns |
```

```table id=sw-m-50pf caption="Switching characteristics, M temp, C~L~ = 50 pF [1]" conditions="Over recommended operating free-air temperature range of M temp = –55 °C to 125 °C, C~L~ = 50 pF (unless otherwise noted) (see Figure 2)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.4 | 11 | 1.1 | 7.5 | 1 | 6.5 | 1 | 5 | ns |

[1] Source conflict: this table's heading says C~L~ = 50 pF, but Figure 2, which it refers to, uses C~L~ = 30 pF at V~CC~ = 1.8 V and 2.5 V.
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 21 | 24 | 26 | 31 | pF |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

The load circuit also has an R~L~ from the output to switch S1 (V~LOAD~,
open or GND). S1 is open for t~PLH~/t~PHL~, which leaves the load below.
The source figures also show pulse-duration, setup/hold and
enable/disable waveforms; all parameters and waveforms are not
applicable to all devices.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions caption="Test conditions (Figure 1 and Figure 2)"
| V~CC~ | Figure | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | 1 | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | 1 | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 1 | 3 V | ≤2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | 1 | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
| 1.8 V ± 0.15 V | 2 | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 2 | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 2 | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | 2 | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: see table
```

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74LVC1G08IDCKREP | Reel of 3000 | -40 to 85 | CEO
SN74LVC1G08MDBVREP | Reel of 3000 | -55 to 125 | C08O
SN74LVC1G08MDCKREP | Reel of 3000 | -55 to 125 | BUB
```

## Conversion notes

- Source pages 1–7 converted; the package option addendum, tape and reel
  and mechanical pages (8–18) dropped, replaced by the generated Package
  and Ordering sections. Ordering rows are from the ordering information
  table on p.1.
- Dropped: features list (the ESD, latch-up and temperature facts are
  kept in text), the qualification-pedigree footnote, the logic diagram
  (one AND gate).
- The pin descriptions are added; the source has no pin function table.
  Parameter names in the electrical characteristics table are added
  (the source gives symbols only).
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Figures 1 and 2 are the same
  circuit with different values, joined in one table.
- The text layer prints the Boolean function as "Y + A • B or Y + A ) B";
  the page image shows Y = A • B or Y = !{!{A} + !{B}} (p.1).
- Source issues:
  - Source conflict: the M-temperature switching table says
    C~L~ = 50 pF and refers to Figure 2, which uses C~L~ = 30 pF at
    V~CC~ = 1.8 V and 2.5 V (pp.4 and 7).
