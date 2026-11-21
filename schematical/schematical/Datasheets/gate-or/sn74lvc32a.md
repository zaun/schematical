---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc32a",
  "title": "SNx4LVC32A quadruple 2-input positive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lvc32a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS286U",
    "date": "2024-07",
    "pages": 41
  },
  "devices": ["gate:74_32"],
  "parts": ["5962-9761801QCA", "5962-9761801QDA", "SN74LVC32ABQAR", "SN74LVC32AD", "SN74LVC32ADBR", "SN74LVC32ADBRG4", "SN74LVC32ADR", "SN74LVC32ADRE4", "SN74LVC32ADT", "SN74LVC32ANSR", "SN74LVC32APW", "SN74LVC32APWE4", "SN74LVC32APWG4", "SN74LVC32APWR", "SN74LVC32APWRE4", "SN74LVC32APWRG3", "SN74LVC32APWRG4", "SN74LVC32APWT", "SN74LVC32ARGYR", "SN74LVC32ARGYRG4", "SNJ54LVC32AJ", "SNJ54LVC32AW", "5962-9761801Q2A", "SNJ54LVC32AFK"],
  "packages": {"p14": "D, DB, NS, PW, J, W, BQA, RGY", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4LVC32A quadruple 2-input positive-OR gates

## Description

The SN54LVC32A quadruple 2-input positive-OR gate is designed for 2.7V
to 3.6V V~CC~ operation, and the SN74LVC32A quadruple 2-input
positive-OR gate is designed for 1.65V to 3.6V V~CC~ operation.
Source conflict: the overview and the SN54LVC32A recommended operating
conditions give 2 V minimum.

The SNx4LVC32A devices perform the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic.

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of these devices as translators in a mixed 3.3V/5V system
environment.

Specified from –40°C to +85°C, –40°C to +125°C, and –55°C to +125°C.
Inputs accept voltages to 5.5V. Latch-up performance exceeds 250mA per
JESD 17.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 1B: Gate 1 input
1Y: Gate 1 output
2A, 2B: Gate 2 input
2Y: Gate 2 output
3A, 3B: Gate 3 input
3Y: Gate 3 output
4A, 4B: Gate 4 input
4Y: Gate 4 output
GND: Ground Pin
NC: No Connection
VCC: Power Pin
```

## Functional description

The SN54LVC32A quadruple 2-input positive-OR gate is designed for 2-V to
3.6-V V~CC~ operation, and the SN74LVC32A quadruple 2-input positive-OR
gate is designed for 1.65-V to 3.6-V V~CC~ operation.

Inputs can be driven from either 3.3-V or 5-V devices. This feature
allows the use of these devices as down-translators in a mixed
3.3-V/5-V system environment.

### Feature description

- Wide operating voltage range
  - Operates from 1.65 V to 3.6 V
- Allows up or down voltage translation
  - Inputs accept voltages to 5.5 V

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| P~tot~ | Power dissipation | T~A~ = –40°C to +125°C [4] [5] | | 500 | mW |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions tables.
[4] For the D package: above 70°C, the value of P~tot~ derates linearly with 8 mW/K.
[5] For the DB, DGV, NS, and PW packages: above 60°C, the value of P~tot~ derates linearly with 5.5 mW/K.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc-54 caption="Recommended operating conditions, SN54LVC32A" parts=SNJ54*,5962-* conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | SN54LVC32A, –55 to +125°C | < | Unit |
| ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 2 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 2.7 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2.7 V to 3.6 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| Δt/Δv | Input transition rise and fall rate | | | 7 | ns/V |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```

```table id=roc-74 caption="Recommended operating conditions, SN74LVC32A" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | SN74LVC32A | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | –40 to +85°C | < | –40 to +125°C | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 3.6 | 1.65 | 3.6 | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | 1.5 | | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | 0.65 × V~CC~ | | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | 1.7 | | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | 2 | | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | | 0.35 × V~CC~ | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | | 0.7 | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | | 0.8 | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | | -4 | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | | -8 | | -8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | -12 | | -12 | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | | 4 | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | | 8 | | 8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | | 12 | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise and fall rate | | | 7 | | 7 | | 7 | ns/V |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC32A | < | < | < | < | < | Unit |
| Symbol | Parameter | BQA (WQFN) 14 pins | D (SOIC) 14 pins | DB (SSOP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | RGY (VQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 140.4 | 123.8 | 150.8 | 92.1 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec-54 caption="Electrical characteristics, SN54LVC32A" parts=SNJ54*,5962-* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN54LVC32A, –55 to +125°C | < | Unit |
| ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | 2.7 V to 3.6 V | V~CC~ - 0.2 | | V |
| ^ | I~OH~ = -12 mA | 2.7 V | 2.2 | | V |
| ^ | ^ | 3 V | 2.4 | | V |
| ^ | I~OH~ = -24 mA | 3 V | 2.2 | | V |
| V~OL~ | I~OL~ = 100 µA | 2.7 V to 3.6 V | | 0.2 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | 0.4 | V |
| ^ | I~OL~ = 24 mA | 3 V | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | 500 | µA |
```

```table id=ec-74 caption="Electrical characteristics, SN74LVC32A" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN74LVC32A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | < | –40 to +85°C | < | –40 to +125°C | < | ^ |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | 1.65 V to 3.6 V | V~CC~ - 0.2 | | | V~CC~ - 0.2 | | V~CC~ - 0.3 | | V |
| ^ | I~OH~ = -4 mA | 1.65 V | 1.29 | | | 1.2 | | 1.05 | | V |
| ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.7 | | 1.55 | | V |
| ^ | I~OH~ = -12 mA | 2.7 V | 2.2 | | | 2.2 | | 2.05 | | V |
| ^ | ^ | 3 V | 2.4 | | | 2.4 | | 2.25 | | V |
| ^ | I~OH~ = -24 mA | 3 V | 2.3 | | | 2.2 | | 2 | | V |
| V~OL~ | I~OL~ = 100 µA | 1.65 V to 3.6 V | | | 0.1 | | 0.2 | | 0.3 | V |
| ^ | I~OL~ = 4 mA | 1.65 V | | | 0.24 | | 0.45 | | 0.6 | V |
| ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | 0.7 | | 0.85 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | | 0.4 | | 0.4 | | 0.6 | V |
| ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | | 0.55 | | 0.8 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±1 | | ±5 | | ±20 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 1 | | 10 | | 40 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | | 500 | | 5000 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | | | | pF |
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 1.8V | 7.5 | pF |
| ^ | ^ | ^ | 2.5V | 10.6 | pF |
| ^ | ^ | ^ | 3.3V | 12.5 | pF |
```

## Switching characteristics

```table id=sw-54 caption="Switching characteristics, SN54LVC32A" parts=SNJ54*,5962-* conditions="Over operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | SN54LVC32A, –55 to +125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|---|--:|--:|---|
| t~pd~ | A or B | Y | 2.7V | | 4.4 | ns |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1 | 3.8 | ns |
```

```table id=sw-74 caption="Switching characteristics, SN74LVC32A" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | SN74LVC32A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | ^ | T~A~ = 25°C | < | < | –40 to +85°C | < | –40 to +125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.8V ± 0.15V | 1 | 4.2 | 8.2 | 1 | 8.7 | 1 | 10.2 | ns |
| ^ | ^ | ^ | 2.5V ± 0.2V | 1 | 2.6 | 4.9 | 1 | 5.4 | 1 | 6.9 | ns |
| ^ | ^ | ^ | 2.7V | 1 | 3 | 4.2 | 1 | 4.4 | 1 | 5.5 | ns |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1 | 2.5 | 3.6 | 1 | 3.8 | 1 | 5 | ns |
| t~sk(o)~ | | | 3.3V ± 0.3V | | | | | 1 | | 1.5 | ns |
```

## Typical characteristics

```curve id=tpd-vcc caption="TPD vs V~CC~ (T~A~ = 25°C)"
{"x": {"label": "VCC Input", "unit": "V", "range": [1.5, 4.0]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 4.2], [2.0, 3.8], [2.3, 3.0], [2.5, 2.6], [2.55, 2.6], [2.7, 3.0], [3.0, 2.8], [3.3, 2.5]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

All parameters and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figure also shows pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions caption="Test conditions (Figure 6-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

SN74LVC32A device is a high-drive, CMOS device that can be used for a
multitude of buffer-type functions. It can produce 24 mA of drive
current at 3 V. Therefore, this device is ideal for driving multiple
inputs and for high-speed applications up to 100 MHz. The inputs and
outputs are 5.5-V tolerant allowing the device to translate down to
V~CC~.

Source conflict: the absolute maximum ratings give an output voltage up
to V~CC~ + 0.5 V, and the recommended operating conditions an output
voltage up to V~CC~.

Conversion note: the typical application (Figure 8-1, "Typical OR Gate Application and Supply Voltage") shows gates 1 to 4 between system inputs/MCU and system outputs/MCU, with a 1.65-V to 3.6-V V~CC~.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads; therefore, routing and load conditions should be
considered to prevent ringing.

### Detailed design procedure

1. Recommended Input Conditions
   - Rise time and fall time specs: See (Δt/ΔV) in the
     Section 5.4 table.
   - Specified high and low levels: See (V~IH~ and V~IL~) in the
     Section 5.4 table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~.
2. Recommended Output Conditions
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part.
   - Outputs should not be pulled above 5.5 V.

Conversion note: Section 5.4 of the source is the
[Recommended operating conditions, SN74LVC32A](#roc-74) table.

Source conflict: the recommended operating conditions give an output
current of 24 mA maximum and an output voltage of V~CC~ maximum.

### Application curve

```curve id=icc-freq caption="Supply current vs input frequency"
{"x": {"label": "Input Frequency", "unit": "MHz", "range": [0, 75]},
 "y": {"label": "Supply Current", "unit": "mA", "range": [0, 16]},
 "series": [
  {"label": "Vcc 1.8V", "points": [[0, 0], [10, 0.8], [20, 1.7], [30, 2.4], [40, 3.1], [50, 3.8], [60, 4.5], [75, 5.6]]},
  {"label": "Vcc 2.5V", "points": [[0, 0], [10, 1.2], [20, 2.3], [25, 2.9], [30, 3.9], [35, 4.7], [40, 5.1], [50, 6.1], [60, 7.5], [70, 9.0], [75, 9.5]]},
  {"label": "Vcc 3.3V", "points": [[0, 0], [10, 1.6], [20, 3.2], [30, 4.6], [35, 5.8], [40, 7.2], [50, 9.6], [60, 11.0], [70, 12.7], [75, 13.5]]}
 ]}
```

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Section 5.4 table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended; if
there are multiple V~CC~ pins, then 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and a 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices inputs should never float.

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states.
Section 8.3.2 specifies the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or
V~CC~, whichever makes more sense or is more convenient. It is generally
acceptable to float outputs, unless the part is a transceiver.

Conversion note: the layout diagram (Figure 8-3) is given here by its labels.

- Recommend GND flood fill for improved signal isolation, noise reduction, and thermal dissipation
- Bypass capacitor placed close to the device
- Unused inputs tied to V~CC~
- Unused output left floating
- Avoid 90° corners for signal lines

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=VQFN-14
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
5962-9761801Q2A | Tube of 55 | -55 to 125 | 5962-9761801Q2A SNJ54LVC32AFK
5962-9761801QCA | Tube of 25 | -55 to 125 | 5962-9761801QCA SNJ54LVC32AJ
5962-9761801QDA | Tube of 25 | -55 to 125 | 5962-9761801QDA SNJ54LVC32AW
SN74LVC32ABQAR | Reel of 3000 | -40 to 125 | LVC32A
SN74LVC32AD | Tube of 50 | -40 to 125 | LVC32A
SN74LVC32ADBR | Reel of 2000 | -40 to 125 | LC32A
SN74LVC32ADBRG4 | Reel of 2000 | -40 to 125 | LC32A
SN74LVC32ADR | Reel of 2500 | -40 to 125 | LVC32A
SN74LVC32ADRE4 | Reel of 2500 | -40 to 125 | LVC32A
SN74LVC32ADT | Reel of 250 | -40 to 125 | LVC32A
SN74LVC32ANSR | Reel of 2000 | -40 to 125 | LVC32A
SN74LVC32APW | Tube of 90 | -40 to 125 | LC32A
SN74LVC32APWE4 | Tube of 90 | -40 to 125 | LC32A
SN74LVC32APWG4 | Tube of 90 | -40 to 125 | LC32A
SN74LVC32APWR | Reel of 2000 | -40 to 125 | LC32A
SN74LVC32APWRE4 | Reel of 2000 | -40 to 125 | LC32A
SN74LVC32APWRG3 | Reel of 2000 | -40 to 125 | LC32A
SN74LVC32APWRG4 | Reel of 2000 | -40 to 125 | LC32A
SN74LVC32APWT | Reel of 250 | -40 to 125 | LC32A
SN74LVC32ARGYR | Reel of 3000 | -40 to 125 | LC32A
SN74LVC32ARGYRG4 | Reel of 3000 | -40 to 125 | LC32A
SNJ54LVC32AFK | Tube of 55 | -55 to 125 | 5962-9761801Q2A SNJ54LVC32AFK
SNJ54LVC32AJ | Tube of 25 | -55 to 125 | 5962-9761801QCA SNJ54LVC32AJ
SNJ54LVC32AW | Tube of 25 | -55 to 125 | 5962-9761801QDA SNJ54LVC32AW
```

## Conversion notes

- Source pages 1 and 3–11 converted. The ordering lines are from the
  package option addendum (pp.14–15). The table of contents (p.2), the
  support and revision history pages (12–13), the rest of the addendum
  (16–17), tape and reel and mechanical pages (18–41) are left out; the
  generated Package section replaces them.
- Dropped: features and applications lists (the temperature,
  input-voltage and latch-up lines are kept in text), device information
  table, the simplified schematic and logic diagram (four 2-input OR
  gates); the typical application (Figure 8-1) is described in text and
  the layout diagram (Figure 8-3) is given as a list of its labels.
- The electrical characteristics tables of the source give symbols only,
  so they have no Parameter column. The SN74LVC32A electrical table runs
  over two pages (pp.6–7) and is one table here.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 6-1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- Curves digitized from Figure 5-1 and Figure 8-2 to about 2 % of full
  scale.
- The application chapter's "Section 5.4" and "Section 8.3.2" are kept
  as printed; Section 5.4 is the recommended operating conditions table
  of the SN74LVC32A, and the source has no Section 8.3.2.
- The text layer prints the Boolean function as "Y + A ) B or
  Y + A • B"; the page images show Y = A + B or Y = !{!{A} • !{B}}
  (pp.1, 9).
- From images: overbars of Y = !{!{A} • !{B}} (p.1).
- Source issues:
  - Source conflict: the description gives the SN54LVC32A as designed
    for 2.7V to 3.6V V~CC~ operation (p.1); the overview (p.9) and its
    recommended operating conditions (p.5) give 2 V minimum.
  - Source conflict: the application information says the outputs are
    5.5-V tolerant and should not be pulled above 5.5 V (p.10); the
    absolute maximum ratings give V~CC~ + 0.5 V (p.4) and the
    recommended operating conditions V~CC~ (p.5).
  - Source conflict: the detailed design procedure gives 25 mA per
    output (p.10); the recommended operating conditions give 24 mA
    (p.5).
  - Unconfirmed: note 5 of the absolute maximum ratings names a DGV
    package, which the source lists nowhere else (p.4).
  - Unconfirmed: the layout guidelines refer to a Section 8.3.2 for the
    rules (p.11); Section 8.3 of the source is Support Resources.
