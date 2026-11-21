---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv27a",
  "title": "SN74LV27A triple 3-input positive-NOR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74lv27a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES341F",
    "date": "2023-07",
    "pages": 25
  },
  "devices": ["gate:74_27"],
  "parts": ["SN74LV27AD", "SN74LV27ADBR", "SN74LV27ADGVR", "SN74LV27ADR", "SN74LV27ANSR", "SN74LV27APWR", "SN74LV27APWRG4"],
  "packages": {"p14": "D, DB, DGV, NS, PW"},
  "converted": "2026-11-21"
}
---

# SN74LV27A triple 3-input positive-NOR gate

## Description

These triple 3-input positive-NOR gates are designed for 2-V to 5.5-V
V~CC~ operation.

The SN74LV27A devices perform the Boolean function Y = !{A + B + C} in
positive logic.

These devices are fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the devices when they are powered
down.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
2A: 2A Input
2B: 2B Input
2C: 2C Input
2Y: 2Y Output
3Y: 3Y Output
3A: 3A Input
3B: 3B Input
3C: 3C Input
1Y: 1Y Output
1C: 1C Input
GND: Ground Pin
VCC: Power Pin
```

## Functional description

These triple 3-input positive-NOR gates are designed for 2-V to 5.5-V
V~CC~ operation. The SN74LV27A devices perform the Boolean function
Y = !{A + B + C} in positive logic. These devices are fully specified
for partial-power-down applications using I~off~. The I~off~ circuitry
disables the outputs, preventing damaging current backflow through the
devices when they are powered down.

Conversion note: the logic diagram, each gate (Figure 7-1), shows one
3-input NOR gate with buffered inputs xA, xB and xC and a buffered
output xY.

## Function tables

```table id=function caption="Function table (each gate)"
| Input [1] | < | < | Output [2] |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | X | X | L |
| X | H | X | L |
| X | X | H | L |
| L | L | L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don't Care
[2] H = Driving High, L = Driving Low
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range applied in high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range applied in power-off state [2] | | -0.5 | 7 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V |
| V~IL~ | Low level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | mA |
| I~OL~ | Low level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise and fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LV27A | < | < | < | < | Unit |
| ^ | ^ | D (14 pins) | NS (14 pins) | PW (14 pins) | DB (14 pins) | DGV (14 pins) | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 76 | 113 | 96 | 127 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = –50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –2 mA | 2.3 V | 2 | | | V |
| ^ | ^ | I~OH~ = –6 mA | 3 V | 2.48 | | | V |
| ^ | ^ | I~OH~ = –12 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | V |
| ^ | ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | µA |
| I~off~ | Off-state leakage current | V~I~ or V~O~ = 0 to 5.5 V | 0 V | | | 5 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 1.9 | | pF |
```

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | 0 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.2 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 13.7 | pF |
| ^ | ^ | ^ | 5 V | 15 | pF |
```

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV27A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 6.7 | 13.8 | 1 | 16 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 9.5 | 17.5 | 1 | 21 | ns |
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV27A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 5 | 8.8 | 1 | 10.5 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 6.9 | 12.3 | 1 | 14 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV27A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 3.9 | 5.9 | 1 | 7 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 5.2 | 7.9 | 1 | 9 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices.

Conversion note: C~L~ is 15 pF or 50 pF as given in the
[switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

Conversion note: Figure 6-1 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse
duration, setup and hold times, and enable and disable times.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

## Ordering information

```ordering
SN74LV27AD | | -40 to 85 | LV27A | Obsolete
SN74LV27ADBR | Reel of 2000 | -40 to 85 | LV27A
SN74LV27ADGVR | Reel of 2000 | -40 to 85 | LV27A
SN74LV27ADR | Reel of 2500 | -40 to 85 | LV27A
SN74LV27ANSR | Reel of 2000 | -40 to 85 | 74LV27A
SN74LV27APWR | Reel of 2000 | -40 to 85 | LV27A
SN74LV27APWRG4 | Reel of 2000 | -40 to 85 | LV27A
```

## Conversion notes

- Source pages 1–9 converted; the package option addendum (pp.10–11),
  tape and reel and mechanical pages (12–25) dropped, replaced by the
  generated Package and Ordering sections.
- Dropped: features list (the latch-up line is under the ESD ratings),
  package information table, simplified schematic (p.1), table of
  contents, revision history, the vendor pinout drawing, the logic
  diagram (Figure 7-1, described in one sentence), the device and
  documentation support section.
- The source has no Application and Implementation chapter.
- Of Figure 6-1 (p.7) the load circuit for totem-pole outputs and the
  propagation delay waveforms are kept, with notes A, C, D, G and H.
- From images: overbars of Y = !{A + B + C} (p.1 and p.8); R~L~ = 1 kΩ
  and the 50% V~CC~ measurement levels (Figure 6-1, p.7, an image).
