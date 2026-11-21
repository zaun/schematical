---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74act14",
  "title": "CD74ACT14 hex Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/cd74act14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS319C",
    "date": "2024-08",
    "pages": 20
  },
  "devices": ["gate:74_14"],
  "parts": ["CD74ACT14E", "CD74ACT14M96"],
  "packages": {"p14": "D, N"},
  "converted": "2026-11-21"
}
---

# CD74ACT14 hex Schmitt-trigger inverter

## Description

The CD74ACT14 contains six independent inverters. This device performs
the Boolean function Y = !{A}.

Inputs are TTL-voltage compatible. Speed of bipolar F, AS, and S, with
significantly reduced power consumption. Greater noise immunity than
standard inverters. Operates with much slower than standard input rise
and fall slew rates. ±24mA output drive current. Fanout to 15 F
devices. SCR-latchup-resistant CMOS process and circuit design.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2Y: Channel 2, Output Y
3A: Channel 3, Input A
3Y: Channel 3, Output Y
4A: Channel 4, Input A
4Y: Channel 4, Output Y
5A: Channel 5, Input A
5Y: Channel 5, Output Y
6A: Channel 6, Input A
6Y: Channel 6, Output Y
GND: Ground
VCC: Positive Supply
```

## Functional description

The CD74ACT14 contains six independent inverters. This device performs
the Boolean function Y = !{A}.

Each circuit functions as an independent inverter, but because of the
Schmitt action, the inverters have different input threshold levels for
positive-going (V~T+~) and negative-going (V~T−~) signals.

Conversion note: the logic diagram, each inverter (positive logic),
shows one inverter with the hysteresis symbol, input A and output Y.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6 | V |
| I~IK~ [2] | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) | | ±20 | mA |
| I~OK~ [2] | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±50 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | T~A~ = 25°C | < | -55°C to 125°C | < | -40°C to 85°C | < | Unit |
| ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | | 24 | mA |
| TΔt/Δv | Input transition rise or fall rate | | 20 | | 20 | | 20 | °C |
```

Unconfirmed: the input transition rise or fall rate row is printed with the symbol TΔt/Δv and the unit °C (p.4), kept as printed.

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | CD74ACT14 | < | Unit |
| ^ | ^ | N (PDIP) 14 pins | D (SOIC) 14 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 80 | 89.9 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Parameter | Test conditions | < | V~CC~ | T~A~ = 25°C | < | -55°C to 125°C | < | -40°C to 85°C | < | Unit |
| ^ | ^ | < | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~T+~ Positive-going threshold | | < | 5 V | 1.4 | 2 | 1.4 | 2 | 1.4 | 2 | V |
| V~T−~ Negative-going threshold | | < | 5 V | 0.8 | 1.3 | 0.8 | 1.3 | 0.8 | 1.3 | V |
| ΔV~T~ Hysteresis (V~T+~ − V~T−~) | | < | 5 V | 0.4 | | 0.4 | | 0.4 | | V |
| V~OH~ | V~I~ = V~T+~ | I~OH~ = -50 µA | 4.5 V | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.94 | | 3.7 | | 3.8 | | ^ |
| ^ | ^ | I~OH~ = -50 mA [1] | 5.5 V | | | 3.85 | | | | ^ |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | | | 3.85 | | ^ |
| V~OL~ | V~I~ = V~T−~ | I~OL~ = 50 µA [1] | 4.5 V | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA [1] | 4.5 V | | 0.36 | | 0.5 | | 0.44 | ^ |
| ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | 1.65 | | | ^ |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | | 1.65 | ^ |
| I~I~ | V~I~ = V~CC~ or GND | < | 5.5 V | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, | I~O~ = 0 | 5.5 V | | 4 | | 80 | | 40 | µA |
| ΔI~CC~ [2] | V~I~ = V~CC~ − 2.1 V | < | 4.5 V to 5.5 V | | 2.4 | | 3 | | 2.8 | mA |
| C~i~ | | < | | | 10 | | 10 | | 10 | pF |

[1] Test one output at a time, not exceeding 1-second duration. Measurement is made by forcing indicated current and measuring voltage to minimize power dissipation. Test verifies a minimum 50-Ω transmission-line drive capability at 85°C and 75-Ω transmission-line drive capability at 125°C.
[2] Additional quiescent supply current per input pin, TTL inputs high, 1 unit load.
```

```table id=input-load caption="Act input load table" values=Value
| Input | Unit load | Unit |
|---|--:|---|
| A | 0.21 | |
```

Unit load is ΔI~CC~ limit specified in electrical characteristics table
(e.g., 2.4 mA at 25°C).

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Typ | Unit |
|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | 45 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Parameter | From (input) | To (output) | -55°C to 125°C | < | -40°C to 85°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | 3.6 | 14.5 | 3.7 | 13.2 | ns |
| t~PHL~ | ^ | ^ | 2.4 | 9.5 | 2.4 | 8.6 | ^ |
```

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance. All input pulses are
supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 3 ns, t~f~ = 3 ns. Phase relationships
between waveforms are arbitrary. The outputs are measured one at a time,
with one input transition per measurement. t~PLH~ and t~PHL~ are the
same as t~pd~.

Conversion note: the load circuit of Figure 5-1 also has R1 = 500 Ω
from the output to switch S1 (2 × V~CC~, Open or GND), and S1 is Open
for t~PLH~/t~PHL~, which leaves R2 and C~L~ below. Conversion note: in
the voltage waveforms the input swings 0 V to 3 V and is measured at
1.5 V, and the outputs are measured at 50% V~CC~ (transition times at
10% and 90%).

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω (R2)
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delay and output transition times"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the
[recommended operating conditions](#recommended-operating-conditions)
table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps
to reject different frequencies of noise. A 0.1 μF and 1 μF are
commonly used in parallel. The bypass capacitor should be installed as
close to the power pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such
input pins should not be left unconnected because the undefined
voltages at the outside connections result in undefined operational
states.

Specified in Example Layout are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The
logic level that should be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver.

Conversion note: the layout diagram (Figure 7-1) shows a 0.1 μF bypass capacitor placed close to the device between V~CC~ and GND, a GND flood fill recommended for improved signal isolation, noise reduction, and thermal dissipation, unused inputs tied to V~CC~, an unused output left floating, and signal lines that avoid 90° corners.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

## Ordering information

```ordering
CD74ACT14E | Tube of 25 | -55 to 125 | CD74ACT14E
CD74ACT14M96 | Reel of 2500 | -55 to 125 | ACT14M
```

## Conversion notes

- Source pages 1–8 converted; the table of contents (2), the support
  and revision history page (9), the package option addendum, tape,
  tube and mechanical pages (10–19) and the notice page (20) dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: package information table, the vendor's pinout drawing, the
  layout diagram (Figure 7-1, described in text) and the two
  transceiver sentences that end the layout guidelines; the logic
  diagram is described in text.
- The electrical characteristics table, printed in two parts (p.4 and
  p.5) under the same header, is one table.
- Figure 5-1 is TI's generic load circuit and waveform set. Only the
  propagation-delay parts apply to this inverter; the pulse-duration,
  recovery-time, setup/hold and output-enable/disable waveforms, the
  other rows of the S1 table (p.6) and their notes (B, D, G, H) are
  dropped.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; R1 and
  S1 are described in text.
- CD74ACT14M96G4, which the package option addendum (p.11) lists, is
  not in the catalog and is not claimed.
- From images: 500 Ω (R1, R2), 2 × V~CC~ (S1), 50 pF (C~L~), 0 V, 3 V
  and 1.5 V (input), 50% V~CC~, 10% and 90% (outputs) (Figure 5-1,
  p.6); 0.1 μF and 90° (Figure 7-1, p.8).
- From images: notes A, C, E and F of Figure 5-1 (p.6): "C~L~ includes
  probe and test-fixture capacitance." "All input pulses are supplied
  by generators having the following characteristics: PRR ≤ 1 MHz,
  Z~O~ = 50 Ω, t~r~ = 3 ns, t~f~ = 3 ns. Phase relationships between
  waveforms are arbitrary." "The outputs are measured one at a time,
  with one input transition per measurement." "t~PLH~ and t~PHL~ are
  the same as t~pd~."
- From images: overbars of Y = !{A} (p.1, p.7).
- Ordering vendor columns are from the package option addendum (p.11).
- Source issues:
  - Unconfirmed: the input transition rise or fall rate row of the
    recommended operating conditions is printed with the symbol TΔt/Δv
    and the unit °C (p.4), kept as printed.
