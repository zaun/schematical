---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74act05",
  "title": "CDx4ACT05 hex inverters with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/cd74act05.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS311E",
    "date": "2024-11",
    "pages": 30
  },
  "devices": ["gate:74_05"],
  "parts": ["5962-9068601QCA", "CD74ACT05BQAR", "CD74ACT05E", "CD74ACT05M", "CD74ACT05M96", "CD74ACT05PWR"],
  "packages": {"p14": "BQA, D, N, PW, J"},
  "converted": "2026-11-21"
}
---

# CDx4ACT05 hex inverters with open-drain outputs

## Description

The 'ACT05 devices contain six independent inverters.

Inputs are TTL-voltage compatible. Speed of bipolar F, AS, and S, with
significantly reduced power consumption. Fanout to 15 F devices.
SCR-latchup-resistant CMOS process and circuit design. Exceeds 2kV ESD
protection per MIL-STD-883, method 3015.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1Y: 1Y Output
2A: 2A Input
2Y: 2Y Output
3A: 3A Input
3Y: 3Y Output
GND: Ground Pin
4Y: 4Y Output
4A: 4A Input
5Y: 5Y Output
5A: 5A Input
6Y: 6Y Output
6A: 6A Input
VCC: Power Pin
```

Connect the GND pin to the exposed thermal pad for correct operation.
Connect the thermal pad to any internal PCB ground plane using multiple
vias for good thermal performance. Conversion note: the source has this
for the thermal pad of the BQA package.

## Functional description

These devices perform the Boolean function Y = !{A}. The open-drain
outputs require pullup resistors to perform correctly, and can be
connected to other open-drain outputs to implement active-low wired-OR
or active-high wired-AND functions.

Conversion note: the functional block diagram shows one inverter with
input A and output Y.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | Z |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous current | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | T~A~ = 25°C | < | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | 5.5 | 0 | 5.5 | 0 | 5.5 | V |
| I~OL~ | Low-level output current | | 24 | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 10 | | 10 | | 10 | ns/V |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | CD74ACT05 | < | < | < | Unit |
| ^ | ^ | BQA (WQFN) | D (SOIC) | N (PDIP) | PW (TSSOP) | ^ |
| ^ | ^ | 14 pins | < | < | < | ^ |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 85.4 | 89.9 | 80 | 132.2 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Parameter | Test conditions | < | V~CC~ | T~A~ = 25°C | < | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | < | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OL~ | V~I~ = V~IH~ or V~IL~ | I~OL~ = 50 mA | 4.5 V | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | 0.36 | | 0.44 | | 0.5 | V |
| ^ | ^ | I~OL~ = 50 mA | 5.5 V | | | | | | 1.65 | V |
| ^ | ^ | I~OL~ = 75 mA | 5.5 V | | | | 1.65 | | | V |
| I~I~ | V~I~ = V~CC~ or GND | < | 5.5 V | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, | I~O~ = 0 | 5.5 V | | 4 | | 40 | | 80 | µA |
| ΔI~CC~ | V~I~ = V~CC~ – 2.1 V | < | 4.5 V to 5.5 V | | 2.4 | | 2.8 | | 3 | mA |
| C~i~ | | < | | | 10 | | 10 | | 10 | pF |
```

Unconfirmed: the first V~OL~ row is printed with I~OL~ = 50 mA and a
0.1 V limit (p.5).

Source error: the two V~OL~ rows at 5.5 V carry a note marker (1) (p.5), and the source prints no note 1 under the electrical characteristics table; the markers are kept out.

```table id=input-load caption="ACT input load table [1]"
| Input | Value | Unit |
|---|--:|---|
| A | 0.18 | unit load |

[1] Unit load is ΔI~CC~ limit specified in electrical characteristics table (e.g., 2.4 mA at 25°C).
```

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Typ | Unit |
|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | 105 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Parameter | From (input) | To (output) | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PZL~ | A or B | Y | 2.4 | 8.5 | 2.3 | 9.3 | ns |
| t~PLZ~ | ^ | ^ | 2.8 | 9.8 | 2.7 | 10.8 | ns |
```

Source error: the switching characteristics (p.5) print the input as
"A or B", kept as printed; the pin functions table (p.3) has no B
input.

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at
a time, with one input transition per measurement.

Conversion note: the load circuit of Figure 5-1 has R1 = 500 Ω from the
output under test to switch S1 at 2 × V~CC~, and R2 = 500 Ω and
C~L~ = 50 pF from the output to ground. When V~CC~ = 1.5 V,
R1 = R2 = 1 kΩ.

Conversion note: the voltage waveforms of Figure 5-1 (propagation delay
times) show t~PZL~ from 50% V~CC~ on the rising input to 50% V~CC~ on
the falling output, and t~PLZ~ from 50% V~CC~ on the falling input to
20% V~CC~ on the output rising from V~OL~ to ≈V~CC~, with the input
between 0 V and V~CC~.

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the
[Recommended Operating Conditions](#recommended-operating-conditions).

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 µF is recommended;
if there are multiple V~CC~ pins, then 0.01 µF or 0.022 µF is
recommended for each power pin. It is acceptable to parallel multiple
bypass caps to reject different frequencies of noise. A 0.1 µF and a
1 µF are commonly used in parallel. The bypass capacitor should be
installed as close to the power pin as possible for best results.

### Layout guidelines

When using multiple-bit logic devices, inputs should never float.

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND
gate are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Layout
Example specifies the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The
logic level that should be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~, whichever makes more sense or is more convenient. It is
generally acceptable to float outputs, unless the part is a
transceiver. If the transceiver has an output enable pin, it will
disable the output section of the part when asserted. This will not
disable the input section of the IOs, so they cannot float when
disabled.

Conversion note: the example layout for the CD74ACT05 (Figure 7-1) shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, a GND flood fill recommended for improved signal isolation, noise reduction, and thermal dissipation, unused inputs tied to V~CC~, an unused output left floating, and signal lines that avoid 90° corners.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
5962-9068601QCA | Tube of 25 | -55 to 125 | 5962-9068601QCA CD54ACT05F3A
CD74ACT05BQAR | Reel of 3000 | -55 to 125 | AD05
CD74ACT05E | Tube of 25 | -55 to 125 | CD74ACT05E
CD74ACT05M | | -55 to 125 | ACT05M | Obsolete
CD74ACT05M96 | Reel of 2500 | -55 to 125 | ACT05M
CD74ACT05PWR | Reel of 3000 | -55 to 125 | AD05
```

## Conversion notes

- Source pages 1–8 converted; the table of contents (2), the support
  and revision history pages (9–10), the package option addendum, tape
  and reel, tube and mechanical pages (11–29) and the notice page (30)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: device information table, the vendor's pinout drawings, the
  layout example figure (Figure 7-1, described in text) and the NC row
  of the pin functions table (it gives no pin number; the 14-pin
  packages have no NC pin); the functional block diagram is described
  in text.
- The load circuit and the voltage waveforms of Figure 5-1 (p.6) match
  no shared figure and are described in text.
- The ACT input load table's "Unit Load" column is written as a Value
  column with the unit "unit load".
- The catalog does not carry CD54ACT05F3A, CD74ACT05EE4 and
  CD74ACT05M96G4, which the package option addendum lists.
- From images: overbar of Y = !{A} (p.7).
- From images: 0.1 µF and 90° (Figure 7-1, p.8).
- Ordering vendor columns are from the package option addendum (p.11).
- Source issues:
  - Unconfirmed: the first V~OL~ row of the electrical characteristics
    (p.5) is printed with I~OL~ = 50 mA and a 0.1 V limit.
  - Source error: the V~OL~ rows at 5.5 V (p.5) carry a note marker
    (1), and no note 1 is printed under the electrical characteristics
    table; the markers are kept out.
  - Source error: the switching characteristics (p.5) print the input
    as "A or B", kept as printed; the pin functions table (p.3) has no
    B input.
  - Source error: the pin functions table (p.3) prints I/O type I for
    the 5Y Output pin; the other outputs are O. The I/O column is not
    transcribed.
