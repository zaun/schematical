---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act04",
  "title": "SNx4ACT04 hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74act04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS518F",
    "date": "2024-11",
    "pages": 38
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74ACT04BQAR", "SN74ACT04D", "SN74ACT04DBR", "SN74ACT04DR", "SN74ACT04N", "SN74ACT04NSR", "SN74ACT04PW", "SN74ACT04PWR"],
  "packages": {"p14": "BQA, D, DB, N, NS, PW"},
  "converted": "2026-11-21"
}
---

# SNx4ACT04 hex inverters

## Description

The ‘ACT04 devices contain six independent inverters. The devices
perform the Boolean function Y = !{A}.

V~CC~ operation of 4.5V to 5.5V. Inputs accept voltages to 5.5V. Max
t~pd~ of 8.5ns at 5V. Inputs are TTL-voltage compatible.

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
GND: Ground
4Y: Channel 4, Output Y
4A: Channel 4, Input A
5Y: Channel 5, Output Y
5A: Channel 5, Input A
6Y: Channel 6, Output Y
6A: Channel 6, Input A
VCC: Positive Supply
```

Connect the GND pin to the exposed thermal pad for correct operation.
Connect the thermal pad to any internal PCB ground plane using multiple
vias for good thermal performance. Conversion note: the source has this
in the thermal pad row of the pin table.

## Functional description

Conversion note: the logic diagram (positive logic) shows one inverter,
input A, output Y.

The SNx4ACT04 devices have an operating V~CC~ range from 4.5 V to 5.5 V.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [1] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [1] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature range | | -60 | 150 | °C |

[1] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±4000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±300 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | SN54ACT04 | < | SN74ACT04 | < | Unit |
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
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SNx4ACT04 | < | < | < | < | < | Unit |
| ^ | ^ | BQA (WQFN) 14 pins | D (SOIC) 14 pins | DB (SSOP) 14 pins | N (PDIP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 85.4 | 89.9 | 96 | 80 | 76 | 148 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54ACT04 | < | SN74ACT04 | < | Unit |
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
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | | 1.65 | | | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | | | 1.65 | V |
| I~I~ | | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 40 | | 20 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | 0.6 | | | 1.6 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5V | | 4.5 | | | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 2 ms.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 45 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | T~A~ = 25°C | < | < | SN54ACT04 | < | SN74ACT04 | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | 1 | 6 | 8.5 | 1 | 9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | 1 | 5.5 | 8 | 1 | 8.5 | 1 | 8.5 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 2.5 ns, t~f~ ≤ 2.5 ns. The outputs are measured one
at a time with one input transition per measurement.

Conversion note: the load circuit also has a 500 Ω resistor from the output
to switch S1 (2 × V~CC~ or open), and S1 is open for t~PLH~/t~PHL~, which
leaves the load below. Conversion note: in the voltage waveforms the input
swings 0 V to 3 V and is measured at 1.5 V, and the outputs are measured at
50% V~CC~.

```figure ref=load-rl-cl caption="Figure 6-1. Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Figure 6-1. Voltage waveforms"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended; if
there are multiple V~CC~ pins, then 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and a 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

When using multiple-bit logic devices, inputs should never float.

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND
gate are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Layout
Diagram specifies the rules that must be observed under all
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

Conversion note: the layout diagram (Figure 8-1) shows a GND flood
fill, a bypass capacitor placed close to the device between V~CC~ and
GND, an unused input tied to GND, an unused input tied to V~CC~, an
unused output left floating, and signal lines without 90° corners.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74ACT04BQAR | Reel of 3000 | -40 to 125 | AD04
SN74ACT04D | | -40 to 85 | ACT04 | Obsolete
SN74ACT04DBR | Reel of 2000 | -40 to 85 | AD04
SN74ACT04DR | Reel of 2500 | -40 to 85 | ACT04
SN74ACT04N | Tube of 25 | -40 to 85 | SN74ACT04N
SN74ACT04NSR | Reel of 2000 | -40 to 85 | ACT04
SN74ACT04PW | | -40 to 85 | AD04 | Obsolete
SN74ACT04PWR | Reel of 2000 | -40 to 85 | AD04
```

## Conversion notes

- Source pages 1–9 converted; the support and revision history page
  (10), the mechanical information page (11), the package option
  addendum, tape and reel and mechanical pages (12–38) dropped, replaced
  by the generated Package and Ordering sections. The ordering lines are
  the parts' rows of the package option addendum (p.12 and p.13).
- Dropped: applications list, device information table, table of
  contents, the vendor's pin drawings; the logic diagram (p.1 and p.8) and the layout diagram
  (Figure 8-1, p.9) are described in text.
- The catalog doesn't carry the SN54ACT04 parts (J, W and FK packages),
  the 5962- parts, SN74ACT04NSRG4 or SN74ACT04PWR1G4 that the source
  lists; the SN54ACT04 columns of the tables are kept, the FK column
  and the NC row of the pin table are not.
- The electrical characteristics table of the source gives only symbols
  in its Parameter column; no parameter names are added. The recommended
  operating conditions print no conditions column.
- The pin functions table continues on the next page (pp.3–4).
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- From images: overbars of Y = !{A} (p.1); 500 Ω (both load resistors),
  2 × V~CC~ (S1), 50 pF (C~L~), 3 V, 0 V and 1.5 V (input), 50% V~CC~
  (outputs) (Figure 6-1, p.7).
- Source issues:
  - Unconfirmed: the absolute maximum ratings (p.5) print a storage
    temperature range from –60 °C.
  - Source conflict: the package option addendum (p.12) gives
    SN74ACT04BQAR an operating temperature of -40 to 125; the
    recommended operating conditions (p.5) give the SN74ACT04 -40 to 85.
