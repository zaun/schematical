---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act08",
  "title": "SNx4ACT08 quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74act08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS535E",
    "date": "2025-02",
    "pages": 37
  },
  "devices": ["gate:74_08"],
  "parts": ["5962-8954702CA", "5962-8954702DA", "SN74ACT08BQAR", "SN74ACT08D", "SN74ACT08DBR", "SN74ACT08DR", "SN74ACT08DRG3", "SN74ACT08DRG4", "SN74ACT08N", "SN74ACT08NSR", "SN74ACT08PW", "SN74ACT08PWR", "SN74ACT08PWRG4", "SNJ54ACT08J", "SNJ54ACT08W", "5962-89547022A", "SNJ54ACT08FK"],
  "packages": {"p14": "D, DB, N, NS, PW, J, W, BQA", "lccc20": "FK"},
  "converted": "2026-11-08"
}
---

# SNx4ACT08 quadruple 2-input positive-AND gates

## Description

The SNx4ACT08 devices are quadruple 2-input positive-AND gates. These
devices perform the Boolean function Y = A · B in positive logic.

4.5 V to 5.5 V V~CC~ operation. Inputs accept voltages to 5.5 V. Inputs
are TTL-voltage compatible.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel 1–4, Input A
1B, 2B, 3B, 4B: Channel 1–4, Input B
1Y, 2Y, 3Y, 4Y: Channel 1–4, Output Y
GND: Ground
VCC: Positive Supply
```

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. BQA package only.

## Functional description

Conversion note: the logic diagram, each gate (positive logic), shows one
2-input AND gate, inputs A and B, output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
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
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | SN54ACT08 | < | SN74ACT08 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise and fall rate | | 8 | | 8 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 85 | °C |
```

## Thermal information

```table id=thermal caption="Thermal information, SN74ACT08 [1]"
| Symbol | Parameter | BQA (WQFN) 14 pins | DB (SSOP) 14 pins | D (SOIC) 14 pins | N (PDIP) 14 pins | NS (PDIP) 14 pins [2] | PW (TSSOP) 14 pins | Unit |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 91.3 | 96 | 86 | 80 | 76 | 145.7 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
[2] Source conflict: the thermal table labels NS "(PDIP)"; the device information table (p.1) lists NS as SOP, 14 pins.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | SNx4ACT08 [3] | < | SNx4ACT08 [3] | < | Unit |
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
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 80 | | 20 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, other inputs at GND or V~CC~ | 5.5 V | | 0.6 | | | 1.6 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 4.5 | | | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 2 ms.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
[3] Unconfirmed: the source labels both column groups SNx4ACT08 and gives no temperature range for either, and their order matches the SN54ACT08 and SN74ACT08 columns of the recommended operating conditions.
```

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 20 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted)"
| Symbol | From (input) | To (output) | T~A~ = 25 °C | < | < | SNx4ACT08 [1] | < | SNx4ACT08 [1] | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1 | 6.5 | 9 | 1 | 10 | 1 | 10 | ns |
| t~PHL~ | A or B | Y | 1 | 6.5 | 9 | 1 | 10 | 1 | 10 | ns |

[1] Unconfirmed: the source labels both column groups SNx4ACT08 and gives no temperature range for either.
```

## Typical characteristics

T~A~ = 25°C (unless otherwise noted)

```curve id=icc-vcc caption="Supply current across supply voltage"
{"x": {"label": "V_CC", "unit": "V", "range": [0.5, 5.5]},
 "y": {"label": "I_CC", "unit": "nA", "range": [0, 100]},
 "series": [
   {"label": "25 °C", "points": [[0.5, 11], [1, 14.5], [1.5, 18.5], [2, 21.5], [2.5, 24], [3, 26.5], [3.5, 28.5], [4, 30], [4.25, 31], [4.5, 29.5], [5, 28], [5.25, 29], [5.5, 32]]},
   {"label": "125 °C", "points": [[0.5, 28], [0.7, 31.5], [1, 32], [1.5, 39], [2, 37], [2.1, 41], [2.5, 38.5], [3, 42], [3.3, 36], [3.5, 42.5], [4, 39], [4.25, 36], [4.5, 41], [5, 38], [5.1, 42.5], [5.2, 55.5], [5.3, 54], [5.5, 73]]},
   {"label": "-40 °C", "points": [[0.5, 18], [1, 26.5], [1.5, 35], [2, 42.5], [2.5, 49.5], [3, 56], [3.5, 62.5], [4, 69], [4.5, 72.5], [5, 77], [5.1, 74], [5.5, 86]]}
 ]}
```

```curve id=vol-iol caption="Output voltage vs current in LOW state; 5V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 50]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.4]},
 "series": [
   {"label": "25 °C", "points": [[0, 0], [25, 0.145], [50, 0.29]]},
   {"label": "125 °C", "points": [[0, 0], [25, 0.18], [50, 0.36]]},
   {"label": "-40 °C", "points": [[0, 0], [25, 0.12], [50, 0.24]]}
 ]}
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
50 % V~CC~.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: 1.5 V (input), 50 % V~CC~ (outputs)
```

## Application information

Information in the following applications sections is not part of the
TI component specification.

### Power supply recommendations

The power supply can be any voltage between the MIN and
MAX supply voltage rating located in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 µF is recommended.
It is acceptable to parallel multiple bypass caps to reject different
frequencies of noise. A 0.1 µF and 1 µF are commonly used in parallel.
The bypass capacitor should be installed as close to the power pin as
possible for best results.

### Layout guidelines

When using multiple bit logic devices, inputs should not float.
Such input pins should not be left unconnected because the undefined
voltages at the outside connections result in undefined operational
states. Specified in the Layout Example are rules that must be observed
under all circumstances. All unused inputs of digital logic devices must be connected to
a high or low bias to prevent them from floating. The logic level that
should be applied to any particular unused input depends on the
function of the device. Generally they will be tied to GND or V~CC~,
whichever makes more sense or is more convenient. It is acceptable to
float outputs unless the part is a transceiver.

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
5962-89547022A | Tube of 55 | -55 to 125 | 5962-89547022A SNJ54ACT08FK
5962-8954702CA | Tube of 25 | -55 to 125 | 5962-8954702CA SNJ54ACT08J
5962-8954702DA | Tube of 25 | -55 to 125 | 5962-8954702DA SNJ54ACT08W
SN74ACT08BQAR | Reel of 3000 | -40 to 85 | AD08
SN74ACT08D | | -40 to 85 | ACT08 | Obsolete
SN74ACT08DBR | Reel of 2000 | -40 to 85 | AD08
SN74ACT08DR | Reel of 2500 | -40 to 85 | ACT08
SN74ACT08DRG3 | Reel of 2500 | -40 to 85 | ACT08
SN74ACT08DRG4 | Reel of 2500 | -40 to 85 | ACT08 | NRND
SN74ACT08N | Tube of 25 | -40 to 85 | SN74ACT08N
SN74ACT08NSR | Reel of 2000 | -40 to 85 | ACT08
SN74ACT08PW | | -40 to 85 | AD08 | Obsolete
SN74ACT08PWR | Reel of 2000 | -40 to 85 | AD08
SN74ACT08PWRG4 | Reel of 2000 | -40 to 85 | AD08
SNJ54ACT08FK | Tube of 55 | -55 to 125 | 5962-89547022A SNJ54ACT08FK
SNJ54ACT08J | Tube of 25 | -55 to 125 | 5962-8954702CA SNJ54ACT08J
SNJ54ACT08W | Tube of 25 | -55 to 125 | 5962-8954702DA SNJ54ACT08W
```

## Conversion notes

- Source pages 1–9 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages (10–37)
  dropped, replaced by the generated Package and Ordering sections. The
  ordering lines are the parts' rows of the package option addendum
  (p.11 and p.12).
- The electrical characteristics table of the source gives only symbols
  in its Parameter column; no parameter names are added.
- The source gives no description for the FK package's NC pins (no NC
  row in the pin table, no legend on Figure 3-3), so the pin functions
  have none.
- Dropped: features list, device information table, the layout example
  figure, the multi-V~CC~-pin and transceiver sentences of the
  application notes (they don't apply to this part); the logic diagram
  is described in text.
- The typical characteristics curves are digitized from Figures 4-1 and
  4-2 (p.6) to about 2 % of full scale.
- The description's second Boolean form (Y = A + B with inverted inputs
  and output, set as overbars) is left out.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- Source issues:
  - Source conflict: thermal table labels NS "(PDIP)" against NS (SOP)
    in the device information table (see the thermal table note).
  - Unconfirmed: both column groups of the electrical and switching
    tables are labelled SNx4ACT08 (see the table notes).
