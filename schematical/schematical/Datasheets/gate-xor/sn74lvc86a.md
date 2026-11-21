---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc86a",
  "title": "SNx4LVC86A quadruple 2-input exclusive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74lvc86a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS288R",
    "date": "2024-08",
    "pages": 38
  },
  "devices": ["gate:74_86"],
  "parts": ["5962-9761901QDA", "SN74LVC86ABQAR", "SN74LVC86AD", "SN74LVC86ADBR", "SN74LVC86ADBRG4", "SN74LVC86ADG4", "SN74LVC86ADR", "SN74LVC86ADRG4", "SN74LVC86ADT", "SN74LVC86ANSR", "SN74LVC86APW", "SN74LVC86APWE4", "SN74LVC86APWG4", "SN74LVC86APWR", "SN74LVC86APWRG4", "SN74LVC86ARGYR", "SN74LVC86ARGYRG4", "SNJ54LVC86AW", "5962-9761901Q2A", "SNJ54LVC86AFK"],
  "packages": {"p14": "D, DB, NS, PW, W, BQA, RGY", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4LVC86A quadruple 2-input exclusive-OR gates

## Description

The SN54LVC86A quadruple 2-input exclusive-OR gate is designed for 2.7V
to 3.6V V~CC~ operation, and the SN74LVC86A quadruple 2-input
exclusive-OR gate is designed for 1.65V to 3.6V V~CC~ operation.
Source conflict: the SN54LVC86A recommended operating conditions give
an operating supply voltage of 2 V minimum.

Specified from –40°C to 85°C, –40°C to 125°C, and –55°C to 125°C.
Inputs accept voltages to 5.5V. Latch-up performance exceeds 250 mA per
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
NC: Do not connect
VCC: Power Pin
```

## Functional description

The 'LVC86A devices perform the Boolean function Y = A ⊕ B or
Y = !{A}B + A!{B} in positive logic.

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the output.
If one of the inputs is high, the signal on the other input is
reproduced inverted at the output.

Inputs can be driven from either 3.3V or 5V devices. This feature allows
the use of these devices as down-translators in a mixed 3.3V/5V system
environment.

### Exclusive-OR logic

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols.

These five equivalent exclusive-OR symbols are valid for an SN74LVC86A
gate in positive logic; negation may be shown at any two ports.
Conversion note: the five symbols (exclusive OR, two logic-identity
element, even-parity element and odd-parity element symbols) are not
drawn here.

### Feature description

- Wide operating voltage range
  - Operates from 1.65 V to 3.6 V
- Allows up or down voltage translation
  - Inputs and outputs accept voltages to 5.5 V

Source conflict: the absolute maximum ratings give an output voltage
range up to V~CC~ + 0.5 V, and the recommended operating conditions an
output voltage up to V~CC~.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [1] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [1] [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| P~tot~ | Power dissipation | T~A~ = –40°C to 125°C [3] [4] | | 500 | mW |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[2] The value of V~CC~ is provided in the recommended operating conditions table.
[3] For the D package: above 70°C, the value of P~tot~ derates linearly with 8 mW/K.
[4] For the DB, DGV, NS, and PW packages: above 60°C, the value of P~tot~ derates linearly with 5.5 mW/K.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc-54 caption="Recommended operating conditions, SN54LVC86A" parts=SNJ54*,5962-* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | SN54LVC86A, –55 to 125°C | < | Unit |
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
| Δt/Δv | Input transition rise or fall rate | | | 9 | ns/V |
```

```table id=roc-74 caption="Recommended operating conditions, SN74LVC86A" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | SN74LVC86A | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | –40 to 85°C | < | –40 to 125°C | < | ^ |
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
| Δt/Δv | Input transition rise or fall rate | | | 9 | | 9 | | 9 | ns/V |
```

## Thermal information

```table id=thermal caption="Thermal information, SN74LVC86A: thermal metric [1]"
| Symbol | Parameter | BQA, 14 pins | D, 14 pins | DB, 14 pins | NS, 14 pins | PW, 14 pins | RGY, 14 pins | Unit |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 96 | 123.8 | 150.8 | 92.1 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec-54 caption="Electrical characteristics, SN54LVC86A" parts=SNJ54*,5962-* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN54LVC86A, –55 to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | 2.7 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = -12 mA | 2.7 V | 2.2 | | | V |
| ^ | ^ | 3 V | 2.4 | | | V |
| ^ | I~OH~ = -24 mA | 3 V | 2.2 | | | V |
| V~OL~ | I~OL~ = 100 µA | 2.7 V to 3.6 V | | | 0.2 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | | 0.4 | V |
| ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 [1] | | pF |

[1] T~A~ = 25°C
```

```table id=ec-74 caption="Electrical characteristics, SN74LVC86A" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN74LVC86A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | < | –40 to 85°C | < | –40 to 125°C | < | ^ |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100µA | 1.65V to 3.6V | V~CC~ - 0.2 | | | V~CC~ - 0.2 | | V~CC~ - 0.3 | | V |
| ^ | I~OH~ = -4mA | 1.65V | 1.29 | | | 1.2 | | 1.05 | | V |
| ^ | I~OH~ = -8mA | 2.3V | 1.9 | | | 1.7 | | 1.55 | | V |
| ^ | I~OH~ = -12mA | 2.7V | 2.2 | | | 2.2 | | 2.05 | | V |
| ^ | ^ | 3V | 2.4 | | | 2.4 | | 2.25 | | V |
| ^ | I~OH~ = -24mA | 3V | 2.3 | | | 2.2 | | 2 | | V |
| V~OL~ | I~OL~ = 100µA | 1.65V to 3.6V | | | 0.1 | | 0.2 | | 0.3 | V |
| ^ | I~OL~ = 4mA | 1.65V | | | 0.24 | | 0.45 | | 0.6 | V |
| ^ | I~OL~ = 8mA | 2.3V | | | 0.3 | | 0.7 | | 0.75 | V |
| ^ | I~OL~ = 12mA | 2.7V | | | 0.4 | | 0.4 | | 0.6 | V |
| ^ | I~OL~ = 24mA | 3V | | | 0.55 | | 0.55 | | 0.8 | V |
| I~I~ | V~I~ = 5.5V or GND | 3.6V | | | ±1 | | ±5 | | ±20 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6V | | | 1 | | 10 | | 40 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6V, Other inputs at V~CC~ or GND | 2.7V to 3.6V | | | 500 | | 500 | | 5000 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3V | | 5 | | | | | | pF |
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 1.8V | 6.5 | pF |
| ^ | ^ | ^ | 2.5V | 7.5 | pF |
| ^ | ^ | ^ | 3.3V | 8.5 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-54 caption="Switching characteristics, SN54LVC86A" parts=SNJ54*,5962-* conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | SN54LVC86A, –55 to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|---|--:|--:|---|
| t~pd~ | A | Y | 2.7 V | | 5.6 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 4.6 | ns |
```

```table id=sw-74 caption="Switching characteristics, SN74LVC86A" parts=SN74* conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | SN74LVC86A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | ^ | T~A~ = 25°C | < | < | –40 to 85°C | < | –40 to 125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8V ± 0.15V | 1 | 4.1 | 9.4 | 1 | 9.9 | 1 | 11.4 | ns |
| ^ | ^ | ^ | 2.5V ± 0.2V | 1 | 2.9 | 7.1 | 1 | 7.6 | 1 | 9.7 | ns |
| ^ | ^ | ^ | 2.7V | 1 | 2.8 | 5.4 | 1 | 5.6 | 1 | 7.1 | ns |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1 | 2.5 | 4.4 | 1 | 4.6 | 1 | 5.8 | ns |
| t~sk(o)~ | | | 3.3V ± 0.3V | | | | | 1 | | 1.5 | ns |
```

## Typical characteristics

```curve id=tpd-vcc caption="TPD vs V~CC~ (T~A~ = 25°C)"
{"x": {"label": "VCC Input", "unit": "V", "range": [1.5, 4.0]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 4.1], [2.5, 2.9], [3.3, 2.5]]}
 ]}
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily for the
examples listed in the following table. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1MHz,
Z~O~ = 50Ω, t~t~ ≤ 2.5ns.

The outputs are measured individually with one input transition per
measurement.

```table id=test-conditions caption="Test conditions"
| V~CC~ | V~t~ | R~L~ | C~L~ | ΔV |
|---|---|---|---|---|
| 1.8V ± 0.15V | V~CC~/2 | 1kΩ | 30pF | 0.15V |
| 2.5V ± 0.2V | V~CC~/2 | 500Ω | 30pF | 0.15V |
| 2.7V | 1.5V | 500Ω | 50pF | 0.3V |
| 3.3V ± 0.3V | 1.5V | 500Ω | 50pF | 0.3V |
```

```figure ref=load-rl-cl caption="Load circuit for push-pull outputs"
RL: see table
CL: see table
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays, input and output transition times"
VM: see table
```

Conversion note: the source draws the propagation delays (Figure 6-2)
and the transition times (Figure 6-3) as two figures, and marks the
input at V~t~ and the outputs at 50%.

C~L~ includes probe and test-fixture capacitance. The greater between t~PLH~ and t~PHL~ is the same as t~pd~.
The greater between t~r~ and t~f~ is the same as t~t~.

## Application information

The SN74LVC86A device is a high-drive, open-drain CMOS device that can
be used for a multitude of buffer-type functions. It can produce 24 mA
of drive current at 3 V. Therefore, this device is ideal for driving
multiple inputs and for high-speed applications up to 100 MHz. The
inputs and outputs are 5.5-V tolerant allowing the device to translate
up to 5.5 V or down to V~CC~.

Source conflict: Figure 6-1 is the load circuit for push-pull outputs,
and the design requirements below give balanced output drive, not
open-drain.

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
     Section 5.3 table.
   - Specified high and low levels: See (V~IH~ and V~IL~) in the
     Section 5.3 table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~.
2. Recommend Output Conditions
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part.
   - Outputs should not be pulled above 5.5 V.

Conversion note: Section 5.3 of the source is the
[Recommended operating conditions, SN54LVC86A](#roc-54) table.

Source conflict: the recommended operating conditions give an output
current of 24 mA maximum and an output voltage of V~CC~ maximum.

### Application curves

```curve id=icc-freq caption="Supply current vs. input frequency"
{"x": {"label": "Input Frequency", "unit": "MHz", "range": [0, 75]},
 "y": {"label": "Supply Current", "unit": "mA", "range": [0, 16]},
 "series": [
  {"label": "Vcc 1.8V", "points": [[0, 0], [10, 0.8], [15, 1.2], [20, 1.7], [25, 1.9], [30, 2.4], [35, 2.8], [40, 3.1], [50, 3.8], [60, 4.5], [75, 5.6]]},
  {"label": "Vcc 2.5V", "points": [[0, 0], [10, 1.2], [20, 2.3], [25, 2.9], [30, 3.9], [35, 4.7], [40, 5.1], [45, 5.5], [50, 6.1], [60, 7.5], [70, 9.0], [75, 9.5]]},
  {"label": "Vcc 3.3V", "points": [[0, 0], [10, 1.6], [20, 3.2], [25, 4.0], [30, 4.8], [35, 5.9], [40, 7.3], [45, 8.6], [50, 9.6], [60, 11.0], [70, 12.7], [75, 13.5]]}
 ]}
```

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Section 5.3 table.

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
outside connections result in undefined operational states. Layout
Example specifies the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or
V~CC~, whichever makes more sense or is more convenient. It is generally
acceptable to float outputs, unless the part is a transceiver.

Conversion note: the example layout for the SN74LVC86A (Figure 8-3) is given here by its labels.

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

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9761901Q2A | Tube of 55 | -55 to 125 | 5962-9761901Q2A SNJ54LVC86AFK
5962-9761901QDA | Tube of 25 | -55 to 125 | 5962-9761901QDA SNJ54LVC86AW
SN74LVC86ABQAR | Reel of 3000 | -40 to 125 | LVC86A
SN74LVC86AD | Tube of 50 | -40 to 125 | LVC86A
SN74LVC86ADBR | Reel of 2000 | -40 to 125 | LC86A
SN74LVC86ADBRG4 | Reel of 2000 | -40 to 125 | LC86A
SN74LVC86ADG4 | Tube of 50 | -40 to 125 | LVC86A
SN74LVC86ADR | Reel of 2500 | -40 to 125 | LVC86A
SN74LVC86ADRG4 | Reel of 2500 | -40 to 125 | LVC86A
SN74LVC86ADT | Reel of 250 | -40 to 125 | LVC86A
SN74LVC86ANSR | Reel of 2000 | -40 to 125 | LVC86A
SN74LVC86APW | Tube of 90 | -40 to 125 | LC86A
SN74LVC86APWE4 | Tube of 90 | -40 to 125 | LC86A
SN74LVC86APWG4 | Tube of 90 | -40 to 125 | LC86A
SN74LVC86APWR | Reel of 2000 | -40 to 125 | LC86A
SN74LVC86APWRG4 | Reel of 2000 | -40 to 125 | LC86A
SN74LVC86ARGYR | Reel of 3000 | -40 to 125 | LC86A
SN74LVC86ARGYRG4 | Reel of 3000 | -40 to 125 | LC86A
SNJ54LVC86AFK | Tube of 55 | -55 to 125 | 5962-9761901Q2A SNJ54LVC86AFK
SNJ54LVC86AW | Tube of 25 | -55 to 125 | 5962-9761901QDA SNJ54LVC86AW
```

## Conversion notes

- Source pages 1–12 converted. The ordering lines are from the package
  option addendum (pp.15–17). The support and revision history pages
  (13–14), tape and reel and mechanical pages (18–38) are left out; the
  generated Package section replaces them.
- Dropped: features and applications lists (the temperature,
  input-voltage and latch-up lines are kept in text), device information
  table, the five exclusive-OR logic symbols (pp.1 and 9); the typical
  application (Figure 8-1) is described in text and the layout example
  (Figure 8-3) is given as a list of its labels.
- The electrical characteristics tables of the source give symbols only,
  so they have no Parameter column. The SN74LVC86A switching table runs
  over two pages (pp.6–7) and is one table here.
- The thermal table's package columns are headed by the package letters
  and "14 PINS" only.
- Both load and waveform figures are drawn with shared figures; the
  shared waveform figure joins Figure 6-2 and Figure 6-3.
- Curves digitized from Figure 5-1 and Figure 8-2 to about 2 % of full
  scale.
- The application chapter's "Section 5.3" is kept as printed; it is the
  recommended operating conditions table of the SN54LVC86A.
- The pin configuration names a J (CDIP) package for the SN54LVC86A;
  the package option addendum lists no J part.
- From images: overbars of Y = !{A}B + A!{B} (p.9).
- Source issues:
  - Source conflict: the description gives the SN54LVC86A as designed
    for 2.7V to 3.6V V~CC~ operation (p.1); its recommended operating
    conditions give an operating supply voltage of 2 V minimum (p.4).
  - Source conflict: § 7.3 and § 8.1 say the outputs accept voltages to
    5.5 V, and § 8.2.2 that outputs should not be pulled above 5.5 V
    (pp.9–10); the absolute maximum ratings give V~CC~ + 0.5 V and the
    recommended operating conditions V~CC~ (pp.4–5).
  - Source conflict: § 8.1 calls the device open-drain (p.10); Figure
    6-1 is the load circuit for push-pull outputs (p.8) and § 8.2.1
    gives balanced output drive.
  - Source conflict: § 8.2.2 gives 25 mA per output (p.10); the
    recommended operating conditions give 24 mA (pp.4–5).
  - Source conflict: Figure 4-3 is titled "FK Package, 14-Pin LCCC";
    the pin functions table gives the FK package 20 pins (p.3).
  - Unconfirmed: note 4 of the absolute maximum ratings names a DGV
    package, which the source lists nowhere else (p.4).
  - Unconfirmed: Figure 8-1 is titled "Typical OR Gate Application" in
    this exclusive-OR datasheet (p.10).
