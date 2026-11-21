---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc00a",
  "title": "SNx4LVC00A quadruple 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lvc00a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS279U",
    "date": "2024-07",
    "pages": 42
  },
  "devices": ["gate:74_00"],
  "parts": ["5962-9753301QCA", "5962-9753301QDA", "5962-9753301VDA", "SN74LVC00ABQAR", "SN74LVC00AD", "SN74LVC00ADBR", "SN74LVC00ADBRG4", "SN74LVC00ADE4", "SN74LVC00ADR", "SN74LVC00ADRG4", "SN74LVC00ADT", "SN74LVC00ANSR", "SN74LVC00ANSRG4", "SN74LVC00APW", "SN74LVC00APWE4", "SN74LVC00APWG4", "SN74LVC00APWR", "SN74LVC00APWRE4", "SN74LVC00APWRG4", "SN74LVC00APWT", "SN74LVC00ARGYR", "SN74LVC00ARGYRG4", "SNJ54LVC00AJ", "SNJ54LVC00AW", "5962-9753301Q2A", "SNJ54LVC00AFK"],
  "packages": {"p14": "D, DB, NS, PW, BQA, RGY, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4LVC00A quadruple 2-input positive-NAND gates

## Description

The SN54LVC00A quadruple 2-input positive-NAND gate is designed for 2.7V
to 3.6V V~CC~ operation, and the SN74LVC00A quadruple 2-input
positive-NAND gate is designed for 1.65V to 3.6V V~CC~ operation.

The SNx4LVC00A devices perform the Boolean function Y = !{A • B} or
Y = !{A} + !{B} in positive logic.

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of these devices as translators in a mixed 3.3V/5V system
environment.

SN74LVC00A operates from 1.65V to 3.6V. SN54LVC00A operates from 2V to
3.6V. SNx4LVC00A specified from –40°C to +85°C and –40°C to +125°C.
SN54LVC00A specified from –55°C to +125°C. Inputs accept voltages to
5.5V. Latch-up performance exceeds 250 mA per JESD 17. On products
compliant to MIL-PRF-38535, all parameters are tested unless otherwise
noted. On all other products, production processing does not necessarily
include testing of all parameters.

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
GND: Ground Pin
3Y: Gate 3 output
3A, 3B: Gate 3 input
4Y: Gate 4 output
4A, 4B: Gate 4 input
VCC: Positive supply
NC: No Connection
```

## Functional description

The maximum sink and source current is 24 mA.

Inputs can be driven from 1.8-V, 2.5-V, 3.3-V (LVTTL), or 5-V (CMOS)
devices. This feature allows the use of this device as translators in a
mixed-system environment.

Conversion note: the logic diagram, each gate (Figure 7-1), shows one
2-input NAND gate with inputs A and B and output Y.

### Balanced high-drive CMOS push-pull outputs

A balanced output allows the device to sink and source similar currents.
The high drive capability of this device creates fast edges into light
loads so routing and load conditions should be considered to prevent
ringing. Additionally, the outputs of this device are capable of driving
larger currents than the device can sustain without being damaged. It is
important for the power output of the device to be limited to avoid
thermal runaway and damage due to over-current. The electrical and
thermal limits defined in the Absolute Maximum Ratings must be followed
at all times.

### Standard CMOS inputs

Standard CMOS inputs are high impedance and are typically modelled as a
resistor in parallel with the input capacitance given in the Electrical
Characteristics. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using ohm's law (R = V ÷ I).

Signals applied to the inputs need to have fast edge rates, as defined
by Δt/Δv in Recommended Operating Conditions to avoid excessive currents
and oscillations. If a slow or noisy input signal is required, a device
with a Schmitt-trigger input should be utilized to condition the input
signal prior to the standard CMOS input.

### Clamp diodes

The inputs and outputs to this device have negative clamping diodes.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

Conversion note: Figure 7-2, electrical placement of clamping diodes for
each input and output, shows one diode from GND to the input (-I~IK~)
and one from GND to the output (-I~OK~).

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value specified in
the Absolute Maximum Ratings.

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

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| V~CC~ | Continuous current through GND | | | ±100 | mA |
| P~tot~ | Power dissipation [4] [5] | T~A~ = –40°C to +125°C | | 500 | mW |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
[4] For the D package: above 70°C, the value of P~tot~ derates linearly with 8 mW/K.
[5] For the DB, NS, and PW packages: above 60°C, the value of P~tot~ derates linearly with 5.5 mW/K.
```

Unconfirmed: the row "Continuous current through GND" is printed with
the symbol V~CC~.

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc-54 caption="Recommended operating conditions, SN54LVC00A" parts=SNJ54*,5962-* conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | SN54LVC00A | < | Unit |
| ^ | ^ | ^ | –55°C to +125°C | < | ^ |
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

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```

```table id=roc-74 caption="Recommended operating conditions, SN74LVC00A" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | SN74LVC00A | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | –40°C to 85°C | < | –40°C to 125°C | < | ^ |
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

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC00A | < | < | < | < | < | Unit |
| Symbol | Parameter | BQA (WQFN) 14 pins | D (SOIC) 14 pins | DB (SSOP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | RGY (VQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 140.4 | 123.8 | 150.8 | 92.1 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec-54 caption="Electrical characteristics, SN54LVC00A" parts=SNJ54*,5962-* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN54LVC00A | < | Unit |
| ^ | ^ | ^ | –55°C to +125°C | < | ^ |
| ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|--:|--:|---|
| V~OH~ | I~OH~ = –100 µA | 2.7 V to 3.6 V | V~CC~ - 0.2 | | V |
| ^ | I~OH~ = –12 mA | 2.7 V | 2.2 | | V |
| ^ | ^ | 3 V | 2.4 | | V |
| ^ | I~OH~ = –24 mA | 3 V | 2.2 | | V |
| V~OL~ | I~OL~ = 100 µA | 2.7 V to 3.6 V | | 0.2 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | 0.4 | V |
| ^ | I~OL~ = 24 mA | 3 V | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | 500 | µA |
```

```table id=ec-74 caption="Electrical characteristics, SN74LVC00A" parts=SN74* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN74LVC00A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | < | –40°C to +85°C | < | –40°C to +125°C | < | ^ |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –100 µA | 1.65 V to 3.6 V | V~CC~ - 0.2 | | | V~CC~ - 0.2 | | V~CC~ - 0.3 | | V |
| ^ | I~OH~ = –4 mA | 1.65 V | 1.29 | | | 1.2 | | 1.05 | | V |
| ^ | I~OH~ = –8 mA | 2.3 V | 1.9 | | | 1.7 | | 1.55 | | V |
| ^ | I~OH~ = –12 mA | 2.7 V | 2.2 | | | 2.2 | | 2.05 | | V |
| ^ | ^ | 3 V | 2.4 | | | 2.4 | | 2.25 | | V |
| ^ | I~OH~ = –24 mA | 3 V | 2.3 | | | 2.2 | | 2 | | V |
| V~OL~ | I~OL~ = 100 µA | 1.65 V to 3.6 V | | | 0.1 | | 0.2 | | 0.3 | V |
| ^ | I~OL~ = 4 mA | 1.65 V | | | 0.24 | | 0.45 | | 0.6 | V |
| ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | 0.7 | | 0.85 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | | 0.4 | | 0.4 | | 0.6 | V |
| ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | | 0.55 | | 0.8 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±1 | | ±5 | | ±20 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 1 | | 10 | | 40 | µA |
| ΔI~CC~ | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | | 500 | | 5000 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | | | | pF |
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 1.8 V | 18 | pF |
| ^ | ^ | ^ | 2.5 V | 18 | pF |
| ^ | ^ | ^ | 3.3 V | 19 | pF |
```

## Switching characteristics

```table id=sw-54 caption="Switching characteristics, SN54LVC00A" parts=SNJ54*,5962-* conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | SN54LVC00A | < | Unit |
| ^ | ^ | ^ | ^ | –55°C to +125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|---|--:|--:|---|
| t~pd~ | A or B | Y | 2.7 V | | 5.1 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 4.3 | ns |
```

```table id=sw-74 caption="Switching characteristics, SN74LVC00A" parts=SN74* conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | SN74LVC00A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | ^ | T~A~ = 25°C | < | < | –40°C to +85°C | < | –40°C to +125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.8 V ± 0.15 V | 1 | 6 | 12 | 1 | 12.5 | 1 | 14 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4.6 | 5.9 | 1 | 6.4 | 1 | 7.9 | ns |
| ^ | ^ | ^ | 2.7 V | 1 | 4.3 | 4.9 | 1 | 5.1 | 1 | 6.5 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.5 | 4.1 | 1 | 4.3 | 1 | 5.5 | ns |
| t~sk(o)~ | | | 3.3 V ± 0.3 V | | | | | 1 | | 1.5 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: Figure 6-1 also shows pulse-duration, setup-and-hold
and enable/disable waveforms, which are not drawn here.

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
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

SN74LVC00A is a high-drive CMOS device that can be used for a multitude
of buffer-type functions. It can produce 24 mA of drive current at
3.3 V. Therefore, this device is ideal for driving multiple inputs and
for high-speed applications up to 100 MHz. The inputs and outputs are
5.5-V tolerant allowing the device to perform mixed-voltage input down
translation. For example the A input can be 3.3 V and the B input can be
5 V, while V~CC~ = 2.5 V and the device will operate properly to output
a 2.5 V signal.

Conversion note: the typical NAND gate application and supply voltage
(Figure 8-1) shows gates 1A, 1B, 1Y to 4A, 4B, 4Y supplied from 1.65-V
to 3.6-V V~CC~ with a capacitor from V~CC~ to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive will also create fast edges into
light loads; therefore, routing and load conditions should be considered
to prevent ringing.

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

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Section 5.4 table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 µF is recommended; if
there are multiple V~CC~ pins, then 0.01 µF or 0.022 µF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 µF and a 1 µF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices inputs should never float.

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Section
8.3.2 specifies the rules that must be observed under all circumstances.
All unused inputs of digital logic devices must be connected to a high
or low bias to prevent them from floating. The logic level that should
be applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~, whichever makes
more sense or is more convenient. It is generally acceptable to float
outputs, unless the part is a transceiver.

Conversion note: the layout diagram for the SNx4LVC00A (Figure 8-3) is
given here by its labels.

- Recommend GND flood fill for improved signal isolation, noise reduction, and thermal dissipation
- Bypass capacitor placed close to the device
- 0.1 µF
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
5962-9753301Q2A | Tube of 55 | -55 to 125 | 5962-9753301Q2A SNJ54LVC00AFK
5962-9753301QCA | Tube of 25 | -55 to 125 | 5962-9753301QCA SNJ54LVC00AJ
5962-9753301QDA | Tube of 25 | -55 to 125 | 5962-9753301QDA SNJ54LVC00AW
5962-9753301VDA | Tube of 25 | -55 to 125 | 5962-9753301VDA SNV54LVC00AW
SN74LVC00ABQAR | Reel of 3000 | -40 to 125 | LVC00A
SN74LVC00AD | Tube of 50 | -40 to 125 | LVC00A
SN74LVC00ADBR | Reel of 2000 | -40 to 125 | LC00A
SN74LVC00ADBRG4 | Reel of 2000 | -40 to 125 | LC00A
SN74LVC00ADE4 | Tube of 50 | -40 to 125 | LVC00A
SN74LVC00ADR | Reel of 2500 | -40 to 125 | LVC00A
SN74LVC00ADRG4 | Reel of 2500 | -40 to 125 | LVC00A
SN74LVC00ADT | Reel of 250 | -40 to 125 | LVC00A
SN74LVC00ANSR | Reel of 2000 | -40 to 125 | LVC00A
SN74LVC00ANSRG4 | Reel of 2000 | -40 to 125 | LVC00A
SN74LVC00APW | Tube of 90 | -40 to 125 | LC00A
SN74LVC00APWE4 | Tube of 90 | -40 to 125 | LC00A
SN74LVC00APWG4 | Tube of 90 | -40 to 125 | LC00A
SN74LVC00APWR | Reel of 2000 | -40 to 125 | LC00A
SN74LVC00APWRE4 | Reel of 2000 | -40 to 125 | LC00A
SN74LVC00APWRG4 | Reel of 2000 | -40 to 125 | LC00A
SN74LVC00APWT | Reel of 250 | -40 to 125 | LC00A
SN74LVC00ARGYR | Reel of 3000 | -40 to 125 | LC00A
SN74LVC00ARGYRG4 | Reel of 3000 | -40 to 125 | LC00A
SNJ54LVC00AFK | Tube of 55 | -55 to 125 | 5962-9753301Q2A SNJ54LVC00AFK
SNJ54LVC00AJ | Tube of 25 | -55 to 125 | 5962-9753301QCA SNJ54LVC00AJ
SNJ54LVC00AW | Tube of 25 | -55 to 125 | 5962-9753301QDA SNJ54LVC00AW
```

## Conversion notes

- Source pages 1 and 3–12 converted. The ordering lines are from the
  package option addendum (pp.15–18). The table of contents (p.2), the
  support and revision history pages (13–14), tape and reel and
  mechanical pages (19–42) are left out; the generated Package section
  replaces them.
- Dropped: features list (the supply, temperature, input-voltage,
  latch-up and MIL-PRF-38535 lines are kept in Description),
  applications list, device information table, the vendor pinout
  drawings, the simplified schematic and the logic diagram (pp.1 and 9,
  described in one sentence), the typical application drawing
  (Figure 8-1, p.11, described in one sentence); the layout example
  (Figure 8-3, p.12) is given as a list of its labels.
- Not converted: the typical characteristic (Figure 5-1, TPD vs V~CC~,
  p.7) and the application curve (Figure 8-2, I~CC~ vs Frequency, p.12).
- The electrical characteristics tables of the source give symbols only,
  so they have no Parameter column.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 6-1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- The clamp diode figure (Figure 7-2, p.9) has diodes to GND only, so
  the shared clamp-diodes figure is not used.
- From images: overbars of Y = !{A • B} and Y = !{A} + !{B} (p.1); µF
  (0.1 µF in Figure 8-3, p.12; the text layer has "0.1 F").
- Source issues:
  - Source conflict: the description says SN54LVC00A is designed for
    2.7V to 3.6V V~CC~ operation (p.1), while the feature list (p.1,
    "operates from 2V to 3.6V") and the SN54LVC00A recommended operating
    conditions (p.4, operating V~CC~ min 2) give 2 V.
  - Unconfirmed: the absolute maximum ratings (p.4) print the symbol
    V~CC~ on the row "Continuous current through GND".
  - Unconfirmed: the detailed design procedure (p.11) and the layout
    guidelines (p.12) refer to Δt/ΔV in the Section 5.4 table and to
    Section 8.3.2 for rules; Sections 5.3 and 5.4 have no Δt/Δv row and
    Section 8.3.2 is the layout example drawing.
