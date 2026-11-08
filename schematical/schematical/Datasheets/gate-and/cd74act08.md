---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74act08",
  "title": "CDx4ACT08 quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/cd74act08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS312C",
    "date": "2024-08",
    "pages": 21
  },
  "devices": ["gate:74_08"],
  "parts": ["CD54ACT08F3A", "CD74ACT08E", "CD74ACT08M", "CD74ACT08M96", "CD74ACT08M96G4"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-08"
}
---

# CDx4ACT08 quadruple 2-input positive-AND gates

## Description

The 'ACT08 devices are quadruple 2-input positive-AND gates. These
devices perform the Boolean function Y = A · B in positive logic. Inputs
are TTL-voltage compatible and buffered; propagation delays are
balanced, and the output drive current is ±24 mA.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel 1–4, Input A
1B, 2B, 3B, 4B: Channel 1–4, Input B
1Y, 2Y, 3Y, 4Y: Channel 1–4, Output Y
GND: Ground
VCC: Positive Supply
```

## Functional description

Logic diagram, each gate (positive logic): one 2-input AND gate, inputs
A and B, output Y.

## Function tables

```table id=function caption="Function table (each gate)"
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
| V~CC~ | Supply voltage range | | -0.5 | 6 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 or V~O~ > V~CC~ | | ±50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1]; HBM ESD Classification Level 2 | ±2000 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | T~A~ = 25 °C | < | –40 °C to 85 °C | < | –55 °C to 125 °C | < | Unit |
| ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 10 | | 10 | | 10 | ns/V |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information, CD74ACT08 [1]"
| Symbol | Parameter | N (PDIP) | D (SOIC) | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 80 | 119.9 | °C/W |

[1] Both packages 14 pins. For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | T~A~ = 25 °C | < | –40 °C to 85 °C | < | –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | < | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = -50 µA | 4.5 V | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.94 | | 3.8 | | 3.7 | | V |
| ^ | ^ | ^ | I~OH~ = -50 mA [1] | 5.5 V | | | | | 3.85 | | V |
| ^ | ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | 3.85 | | | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 50 µA | 4.5 V | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 24 mA | 4.5 V | | 0.36 | | 0.44 | | 0.5 | V |
| ^ | ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | | | 1.65 | V |
| ^ | ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | 1.65 | | | V |
| I~I~ | Input current | V~I~ = V~CC~ or GND | < | 5.5 V | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | < | 5.5 V | | 4 | | 40 | | 80 | µA |
| ΔI~CC~ | Supply-current change | V~I~ = V~CC~ − 2.1 V | < | 4.5 V to 5.5 V | | 2.4 | | 2.8 | | 3 | mA |
| C~i~ | Input capacitance | | < | | | 10 | | 10 | | 10 | pF |

[1] Test one output at a time, not exceeding 1-second duration. Measurement is made by forcing indicated current and measuring voltage to minimize power dissipation. Test verifies a minimum 50-Ω transmission-line drive capability at 85°C and 75-Ω transmission-line drive capability at 125°C.
```

```table id=input-load caption="ACT input load table"
| Input | Value | Unit |
|---|--:|---|
| A or B | 0.3 | unit load |
```

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Typ | Unit |
|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | 50 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF (unless otherwise noted)"
| Symbol | From (input) | To (output) | –40 °C to 85 °C | < | –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 3.3 | 11.7 | 3.2 | 12.9 | ns |
| t~PHL~ | A or B | Y | 3.3 | 11.7 | 3.2 | 12.9 | ns |
```

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance. All input pulses are
supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 3 ns, t~f~ = 3 ns. The outputs are
measured one at a time with one input transition per measurement.
t~PLH~ and t~PHL~ are the same as t~pd~.

The load circuit also has R1 = 500 Ω from the output to switch S1
(2 × V~CC~, open or GND); S1 is open for t~PLH~ and t~PHL~, which leaves
R2 and C~L~ below. The input swings 0 V to 3 V and is
measured at 1.5 V; the outputs are measured at 50 % V~CC~ (transition
times at 10 % and 90 %).

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω (R2)
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delay and output transition times"
VM: 1.5 V (input), 50 % V~CC~ (outputs)
```

## Application information

Information in this section is not part of the TI component
specification.

Power supply: the power supply can be any voltage between the minimum
and maximum supply voltage rating located in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ terminal should have a bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise; the 0.1-µF and 1-µF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results.

Layout: when using multiple-input and multiple-channel logic devices
inputs must not ever be left floating. Unused input pins must not be
left unconnected because the undefined voltages at the outside
connections result in undefined operational states. All unused inputs
of digital logic devices must be connected to a logic high or logic low
voltage, as defined by the input voltage specifications, to prevent
them from floating. The logic level that must be applied to any
particular unused input depends on the function of the device.
Generally, the inputs are tied to GND or V~CC~, whichever makes more
sense for the logic function or is more convenient.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
```

## Conversion notes

- Source pages 1–9 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages (10–21)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: features list (its electrical points are kept in the
  Description), device information table, the layout example figure;
  the logic diagram is described in text.
- Figure 5-1 is TI's generic load circuit and waveform set. Only the
  propagation-delay parts apply to this gate; the pulse-duration,
  recovery-time, setup/hold and output-enable/disable waveforms, their
  S1 settings and their notes (3-state output waveforms, f~max~, t~en~,
  t~dis~) are dropped.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; R1 and
  S1 are described in text.
- From images: 500 Ω (R1, R2), 2 × V~CC~ (S1), 50 pF (C~L~), 3 V, 0 V,
  1.5 V, 50 % V~CC~, 10 % and 90 % (waveform levels) (Figure 5-1, p.6).
- T~stg~ min is printed with its unit in the cell; written -65 with unit °C.
- Source issues: none found.
