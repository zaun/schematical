---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc04a",
  "title": "SNx4LVC04A hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc04a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS281T",
    "date": "2016-11",
    "pages": 39
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74LVC04AD", "SN74LVC04ADBR", "SN74LVC04ADGVR", "SN74LVC04ADR", "SN74LVC04ADRG3", "SN74LVC04ADT", "SN74LVC04ANSR", "SN74LVC04APW", "SN74LVC04APWE4", "SN74LVC04APWRG3", "SN74LVC04APWT", "SN74LVC04ARGYR"],
  "packages": {"p14": "D, DB, DGV, NS, PW, RGY, J, W"},
  "converted": "2026-11-21"
}
---

# SNx4LVC04A hex inverters

## Description

The SNx4LVC04A hex inverters contains six independent inverters designed
for 2.7-V to 3.6-V V~CC~ operation, and the SN74LVC04A hex inverter
contains six independent inverters designed for 1.65-V to 3.6-V V~CC~
operation. The SNx4LVC04A devices perform the Boolean function Y = !{A}.

Inputs can be driven from 1.8-V or 3.3-V devices. This feature allows
the use of these devices as translators in a mixed 1.8-V or 3.3-V system
environment.

Operate from 1.65 V to 3.6 V. Specified from –40°C to 85°C, –40°C to
125°C, and –55°C to 125°C. Inputs accept voltages to 5.5 V. Maximum
t~pd~ of 4.5 ns at 3.3 V. Latch-up performance exceeds 250 mA per
JESD 17. On products compliant to MIL-PRF-38535, all parameters are
tested unless otherwise noted. On all other products, production
processing does not necessarily include testing of all parameters.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1 input
1Y: Channel 1 output
2A: Channel 2 input
2Y: Channel 2 output
3A: Channel 3 input
3Y: Channel 3 output
4A: Channel 4 input
4Y: Channel 4 output
5A: Channel 5 input
5Y: Channel 5 output
6A: Channel 6 input
6Y: Channel 6 output
GND: Ground
VCC: Power supply
```

## Functional description

These hex inverters are designed for 1.65-V to 3.6-V V~CC~ operation.
The SN74LVC04A devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}. These devices are fully specified
for partial-power-down applications using Ioff. The Ioff circuitry
disables the outputs, preventing damaging current backflow through the
devices when they are powered down. The inputs are high impedance when
V~CC~ = 0V.

Conversion note: the functional block diagram shows one inverter with
input A and output Y.

Wide operating voltage range from 1.65 V to 3.6 V. Allows down-voltage
translation with inputs accept voltages to 3.6 V. I~OFF~ feature supports
live insertion, partial power down mode, and back drive protection.

Table 1 lists the functional modes of the SNx4LVC04A.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
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
| P~tot~ | Power dissipation [4] [5] | T~A~ = –40°C to 125°C | | 500 | mW |
| T~J(MAX)~ | Maximum virtual junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in Recommended Operating Conditions.
[4] For the D package: above 70°C, the value of P~tot~ derates linearly with 8 mW/K.
[5] For the DB, DGV, NS, and PW packages: above 60°C, the value of P~tot~ derates linearly with 5.5 mW/K.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |
| ^ | ^ | Machine Model (MM) | ±200 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating, SN54LVC04A | 2 | 3.6 | V |
| ^ | ^ | Operating, SN74LVC04A | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V, SN74LVC04A only | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V, SN74LVC04A only | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V, SN74LVC04A only | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V, SN74LVC04A only | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V, SN74LVC04A only | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V, SN74LVC04A only | | -8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V, SN74LVC04A only | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V, SN74LVC04A only | | 8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```

## Thermal information

```table id=thermal-54 caption="Thermal information – SN54LVC04A"
| Thermal metric [1] | < | SN54LVC04A | < | < | Unit |
| Symbol | Parameter | J (CDIP) 14 pins | W (CFP) 14 pins | FK (LCCC) 20 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 92 | 158.2 | 85 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 55.1 | 88.7 | 62.5 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 80.5 | 156.5 | 61.2 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 40.2 | 58.5 | 55.8 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 74.2 | 135.5 | 61.2 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | 25.3 | 15.3 | 10.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

```table id=thermal-74 caption="Thermal information – SN74LVC04A"
| Thermal metric [1] | < | SN74LVC04A | < | < | < | < | < | Unit |
| Symbol | Parameter | D (SOIC) 14 pins | DB (SSOP) 14 pins | DGV (TVSOP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | RGY (VQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 105.7 | 113.1 | 142.7 | 95.4 | 129.5 | 63.2 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 50.8 | 65.1 | 61.9 | 53.2 | 57.9 | 61 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 46.1 | 60.5 | 72.1 | 54.2 | 71.3 | 39.1 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 8.2 | 29.1 | 10.1 | 21.9 | 9.9 | 5.2 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 45.6 | 60 | 71.4 | 53.8 | 70.7 | 39.1 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | 20.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec-54 caption="Electrical characteristics – SN54LVC04A" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = –100 µA, V~CC~ = 2.7 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | ^ | I~OH~ = –12 mA, V~CC~ = 2.7 V | 2.2 | | | V |
| ^ | ^ | I~OH~ = –12 mA, V~CC~ = 3 V | 2.4 | | | V |
| ^ | ^ | I~OH~ = –24 mA, V~CC~ = 3 V | 2.2 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA, V~CC~ = 2.7 V to 3.6 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 12 mA, V~CC~ = 2.7 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA, V~CC~ = 3 V | | | 0.55 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND, V~CC~ = 3.6 V | | | ±5 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0, V~CC~ = 3.6 V | | | 10 | µA |
| ΔI~CC~ | Change in supply current | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND, V~CC~ = 2.7 V to 3.6 V | | | 500 | µA |
```

```table id=ec-74 caption="Electrical characteristics – SN74LVC04A" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = –100 µA, V~CC~ = 1.65 V to 3.6 V | T~A~ = 25°C | V~CC~ - 0.2 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | V~CC~ - 0.3 | | | V |
| ^ | ^ | I~OH~ = –4 mA, V~CC~ = 1.65 V | T~A~ = 25°C | 1.29 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | 1.2 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | 1.05 | | | V |
| ^ | ^ | I~OH~ = –8 mA, V~CC~ = 2.3 V | T~A~ = 25°C | 1.9 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | 1.7 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | 1.55 | | | V |
| ^ | ^ | I~OH~ = –12 mA, V~CC~ = 2.7 V | T~A~ = 25°C | 2.2 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | 2.05 | | | V |
| ^ | ^ | I~OH~ = –12 mA, V~CC~ = 3 V | T~A~ = 25°C | 2.4 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | 2.25 | | | V |
| ^ | ^ | I~OH~ = –24 mA, V~CC~ = 3 V | T~A~ = 25°C | 2.3 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | 2.2 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | 2 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA, V~CC~ = 1.65 V to 3.6 V | T~A~ = 25°C | | | 0.1 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | | | 0.2 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 0.3 | V |
| ^ | ^ | I~OL~ = 4 mA, V~CC~ = 1.65 V | T~A~ = 25°C | | | 0.24 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | | | 0.45 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 0.6 | V |
| ^ | ^ | I~OL~ = 8 mA, V~CC~ = 2.3 V | T~A~ = 25°C | | | 0.3 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | | | 0.7 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 0.85 | V |
| ^ | ^ | I~OL~ = 12 mA, V~CC~ = 2.7 V | T~A~ = 25°C | | | 0.4 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 0.6 | V |
| ^ | ^ | I~OL~ = 24 mA, V~CC~ = 3 V | T~A~ = 25°C | | | 0.55 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 0.8 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND, V~CC~ = 3.6 V | T~A~ = 25°C | | | ±1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | | | ±5 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | ±20 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0, V~CC~ = 3.6 V | T~A~ = 25°C | | | 1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | | | 10 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 40 | µA |
| ΔI~CC~ | Change in supply current | One input at V~CC~ – 0.6 V, other inputs at V~CC~ or GND, V~CC~ = 2.7 V to 3.6 V | T~A~ = 25°C | | | 500 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 5000 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND, V~CC~ = 3.3 V | | | 5 | | pF |
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz, T~A~ = 25°C | V~CC~ = 1.8 V | | 6 | | pF |
| ^ | ^ | ^ | V~CC~ = 2.5 V | | 7 | | pF |
| ^ | ^ | ^ | V~CC~ = 3.3 V | | 8 | | pF |
```

## Switching characteristics

```table id=switching caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted; see Figure 2)"
| Symbol | Parameter | Test conditions | < | < | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation (delay) time | From A (input) to Y (output) | V~CC~ = 1.8 V ±0.15 V, SN74LVC04A only | T~A~ = 25°C | 1 | 4.1 | 7.5 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 85°C | 1 | | 8 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C | 1 | | 9.5 | ns |
| ^ | ^ | ^ | V~CC~ = 2.5 V ±0.2 V, SN74LVC04A only | T~A~ = 25°C | 1 | 3.6 | 7 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 85°C | 1 | | 7.5 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C | 1 | | 9 | ns |
| ^ | ^ | ^ | V~CC~ = 2.7 V | T~A~ = –55°C to 125°C, SN54LVC04A | | | 5.5 | ns |
| ^ | ^ | ^ | ^ | T~A~ = 25°C, SN74LVC04A | 1 | 3 | 5.3 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 85°C, SN74LVC04A | 1 | | 5.5 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C, SN74LVC04A | 1 | | 7 | ns |
| ^ | ^ | ^ | V~CC~ = 3.3 V ±0.3 V | T~A~ = –55°C to 125°C, SN54LVC04A | 0.5 | | 4.5 | ns |
| ^ | ^ | ^ | ^ | T~A~ = 25°C, SN74LVC04A | 1 | 2.5 | 4.3 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 85°C, SN74LVC04A | 1 | | 4.5 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C, SN74LVC04A | 1 | | 6 | ns |
| t~sk(o)~ | Skew (time), output | V~CC~ = 3.3 V ±0.3 V, SN74LVC04A only | T~A~ = –40°C to 85°C | | | | 1 | ns |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | | 1.5 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: Figure 2 also shows pulse-duration, setup-and-hold
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

```table id=test-conditions caption="Test conditions (Figure 2)"
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

SN74LVC04A is a low-drive CMOS device that can be used for a multitude
of bus interface type applications where output ringing is a concern.
The low drive and slow edge rates minimize overshoot and undershoot on
the outputs. The inputs can accept voltages to 3.6 V at any valid V~CC~
making it Ideal for down translation.

Conversion note: the typical application schematic (Figure 3) shows a
3.3V regulated supply with a 0.1 µF capacitor and an inverter driving a
1.8V or 3.3V accessory.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention, because it can drive currents that would
exceed maximum limits. The high drive also creates fast edges into light
loads, so routing and load conditions must be considered to prevent
ringing.

### Detailed design procedure

Recommended Input Conditions:

- For rise time and fall time specifications, see Δt/ΔV in Recommended Operating Conditions.
- For specified high and low levels, see V~IH~ and V~IL~ in Recommended Operating Conditions.
- Inputs are overvoltage tolerant allowing them to go as high as 3.6 V at any valid V~CC~.

Recommend Output Conditions:

- Load currents must not exceed 25 mA per output and 50 mA total for the part.
- Outputs must not be pulled above V~CC~.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.

Each V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 µF is recommended. If
there are multiple V~CC~ pins, 0.01 µF or 0.022 µF is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 µF and 1 µF are commonly
used in parallel. The bypass capacitor must be installed as close to the
power pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices, inputs must not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such input
pins must not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states.

Specified in Figure 5 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that must be applied to any particular unused input depends on the
function of the device. Generally they are tied to GND or V~CC~,
whichever makes more sense or is more convenient. It is acceptable to
float outputs unless the part is a transceiver. If the transceiver has
an output enable pin, it disables the outputs section of the part when
asserted. This does not disable the input section of the I/Os so they
also cannot float when disabled.

Conversion note: the layout diagram (Figure 5) shows two gates, one with
its unused input tied to Vcc and one with its unused input tied to
ground.

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74LVC04AD | Tube of 50 | -40 to 125 | LVC04A
SN74LVC04ADBR | Reel of 2000 | -40 to 125 | LC04A
SN74LVC04ADGVR | Reel of 2000 | -40 to 125 | LC04A
SN74LVC04ADR | Reel of 2500 | -40 to 125 | LVC04A
SN74LVC04ADRG3 | Reel of 2500 | -40 to 125 | LVC04A
SN74LVC04ADT | Reel of 250 | -40 to 125 | LVC04A
SN74LVC04ANSR | Reel of 2000 | -40 to 125 | LVC04A
SN74LVC04APW | Tube of 90 | -40 to 125 | LC04A
SN74LVC04APWE4 | Tube of 90 | -40 to 125 | LC04A
SN74LVC04APWRG3 | Reel of 2000 | -40 to 125 | LC04A
SN74LVC04APWT | Reel of 250 | -40 to 125 | LC04A
SN74LVC04ARGYR | Reel of 3000 | -40 to 125 | LC04A
```

## Conversion notes

- Source pages 1 and 3–13 converted. The ordering lines are from the
  package option addendum (pp.15–18). The table of contents and revision
  history (p.2), the support page (14), tape and reel and mechanical
  pages (19–39) are left out; the generated Package section replaces
  them.
- Dropped: features list (the supply, temperature, input-voltage,
  t~pd~, latch-up and MIL-PRF-38535 lines are kept in Description),
  applications list, device information table, the vendor pinout
  drawings, the logic diagram and functional block diagram (pp.1 and 11,
  described in one sentence), the typical application schematic
  (Figure 3, p.12) and the layout diagram (Figure 5, p.13), each
  described in one sentence.
- Not converted: the typical characteristic (Figure 1, V~IH~ minimum and
  V~IL~ maximum vs supply voltage, p.9) and the application curve
  (Figure 4, propagation delay vs supply voltage, p.13).
- The catalog carries no SN54LVC04A part, so the FK (LCCC) pinout and
  the NC row of the pin table are left out; the SN54LVC04A tables and
  rows are kept as the source prints them.
- The absolute maximum ratings are printed as "name, symbol"; they are
  split into Symbol and Parameter columns.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 2, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- From images: overbars of Y = !{A} (pp.1 and 11).
- Source issues:
  - Source conflict: the description says the SNx4LVC04A is designed for
    2.7-V to 3.6-V V~CC~ operation (p.1), while the recommended
    operating conditions (p.5) give the SN54LVC04A an operating V~CC~
    of 2 V to 3.6 V.
  - Source conflict: the features (p.1) say inputs accept voltages to
    5.5 V and the recommended operating conditions (p.5) give V~I~ max
    5.5 V, while the feature description (p.11) and the application
    information (p.12) say inputs accept voltages to 3.6 V.
  - Unconfirmed: the overview and feature description (p.11) describe an
    Ioff feature; the electrical characteristics (pp.7–8) have no
    I~off~ row.
  - Unconfirmed: the detailed design procedure (p.12) refers to Δt/ΔV in
    Recommended Operating Conditions, which has no Δt/ΔV row.
  - Unconfirmed: the design requirements (p.12) speak of "the high
    drive", while the application information on the same page calls
    the SN74LVC04A a low-drive device.
