---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup1g02",
  "title": "SN74AUP1G02 low-power single 2-input positive-NOR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74aup1g02.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES568I",
    "date": "2016-09",
    "pages": 48
  },
  "devices": ["gate:1G02", "gate:1G02_DPW", "gate:1G02_SON"],
  "parts": ["SN74AUP1G02DBVR", "SN74AUP1G02DBVRG4", "SN74AUP1G02DBVT", "SN74AUP1G02DCKR", "SN74AUP1G02DCKRG4", "SN74AUP1G02DCKT", "SN74AUP1G02DRLR", "SN74AUP1G02DPWR", "SN74AUP1G02DRY2", "SN74AUP1G02DRYR", "SN74AUP1G02DRYRG4", "SN74AUP1G02DSFR", "SN74AUP1G02DSFRG4", "SN74AUP1G02YFPR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP1G02 low-power single 2-input positive-NOR gate

## Description

This single 2-input positive-NOR gate performs the Boolean function
Y = !{A + B} or Y = !{A} × !{B} in positive logic.

## Pinout

Conversion note: DBV, DCK and DRL have A on pin 1 and B on pin 2, and
the DPW (X2SON) package has B on pin 1 and A on pin 2. Conversion note:
the source draws the YFP package in bottom view, and the drawing here is
generated from the catalog.

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Input A
B: Input B
DNU: Do not use
GND: Ground
NC: No internal connection
Y: Output Y
VCC: Power Pin
```

## Functional description

The SN74LVC1G02 device contains one 2-input positive-NOR gate and
performs the Boolean function Y = !{A + B} or Y = !{A} × !{B}. Source
error: the source names the SN74LVC1G02 in this overview of the
SN74AUP1G02, kept as printed.

The AUP family of devices has quiescent power consumption less than 1 µA
and comes in the ultra small DPW package. The DPW package technology is
a major breakthrough in IC packaging. Its tiny 0.64 mm square footprint
saves significant board space over other package options while still
retaining the traditional manufacturing friendly lead pitch of 0.5 mm.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered. The I~off~
feature also allows for live insertion.

Conversion note: the functional block diagram shows one 2-input NOR gate with inputs A and B and output Y.

- Wide operating V~CC~ range of 0.8 V to 3.6 V
- 3.6-V I/O tolerant to support down translation
- Input hysteresis allows slow input transition and better switching noise immunity at the input
- I~off~ feature allows voltages on the inputs and outputs when V~CC~ is 0 V
- Low noise due to slower edge rates

Table 1 shows the functional modes of SN74AUP1G02.

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | H |
| L | H | L |
| H | L | L |
| H | H | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 4.6 | V |
| V~I~ | Input voltage [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.6 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.9 | ^ |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | -1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | -1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | -3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -4 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 4 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 3.6 V | | 200 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AUP1G02 | < | < | < | < | < | Unit |
| ^ | ^ | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DPW (X2SON) 5 pins | DRL (SOT) 5 pins | DRY (SON) 6 pins | DSF (SON) 6 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 298.6 | 314.4 | 291.8 | 349.7 | 554.9 | 407.1 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 240.2 | 128.7 | 224.2 | 120.5 | 385.4 | 232 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 134.6 | 100.6 | 245.8 | 171.4 | 388.2 | 306.9 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 114.5 | 7.1 | 31.4 | 10.8 | 159 | 40.3 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 133.9 | 99.8 | 245.6 | 169.4 | 384.1 | 306 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | 195.4 | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to +85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -20 µA | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = -1.1 mA | 1.1 V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | ^ |
| ^ | ^ | I~OH~ = -1.7 mA | 1.4 V | 1.11 | | | 1.03 | | ^ |
| ^ | ^ | I~OH~ = -1.9 mA | 1.65 V | 1.32 | | | 1.3 | | ^ |
| ^ | ^ | I~OH~ = -2.3 mA | 2.3 V | 2.05 | | | 1.97 | | ^ |
| ^ | ^ | I~OH~ = -3.1 mA | ^ | 1.9 | | | 1.85 | | ^ |
| ^ | ^ | I~OH~ = -2.7 mA | 3 V | 2.72 | | | 2.67 | | ^ |
| ^ | ^ | I~OH~ = -4 mA | ^ | 2.6 | | | 2.55 | | ^ |
| V~OL~ | | I~OL~ = 20 µA | 0.8 V to 3.6 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | 1.1 V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | ^ |
| ^ | ^ | I~OL~ = 1.7 mA | 1.4 V | | | 0.31 | | 0.37 | ^ |
| ^ | ^ | I~OL~ = 1.9 mA | 1.65 V | | | 0.31 | | 0.35 | ^ |
| ^ | ^ | I~OL~ = 2.3 mA | 2.3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 3.1 mA | ^ | | | 0.44 | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 2.7 mA | 3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 4 mA | ^ | | | 0.44 | | 0.45 | ^ |
| I~I~ | A or B inputs | V~I~ = GND to 3.6 V | 0 V to 3.6 V | | | 0.1 | | 0.5 | µA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 0.6 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 0.6 | µA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 0.9 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 0.6 V [1], I~O~ = 0 | 3.3 V | | | 40 | | 50 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 0 V | | 1.5 | | | | pF |
| ^ | ^ | ^ | 3.6 V | | 1.5 | | | | ^ |
| C~o~ | | V~O~ = GND | 0 V | | 3 | | | | pF |

[1] One input at V~CC~ – 0.6 V, other input at V~CC~ or GND.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 0.8 V | 4.1 | pF |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.1 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 4.1 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 4.1 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 4.2 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 4.3 | ^ |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3 and Figure 4)"
| Parameter | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to +85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 19.3 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 2.6 | 7.3 | 13 | 2.1 | 16.3 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1.4 | 5.2 | 8.9 | 0.9 | 10.8 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 4.2 | 6.8 | 0.5 | 8.7 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3 | 4.6 | 0.5 | 5.9 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.4 | 3.7 | 0.5 | 4.6 | ^ |
```

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3 and Figure 4)"
| Parameter | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to +85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 22.3 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 1.5 | 8.5 | 14.9 | 1 | 17.9 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1 | 6.2 | 10.2 | 0.5 | 11.8 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 5 | 7.9 | 0.5 | 9.5 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3.6 | 5.4 | 0.5 | 6.5 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.9 | 4.4 | 0.5 | 5 | ^ |
```

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3 and Figure 4)"
| Parameter | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to +85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 25 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.6 | 9.9 | 16.5 | 3.1 | 20.6 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.3 | 7.2 | 11.3 | 1.8 | 13.7 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1.6 | 5.8 | 8.9 | 1.1 | 11.1 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4.3 | 6.1 | 0.5 | 7.7 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.4 | 5 | 0.5 | 6.2 | ^ |
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3 and Figure 4)"
| Parameter | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to +85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 34.6 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 13.1 | 21.1 | 4.4 | 26.2 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9.5 | 14.4 | 2.9 | 17.4 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.7 | 11.2 | 2 | 14 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.7 | 7.8 | 1.3 | 9.8 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.7 | 6.4 | 1 | 7.8 | ^ |
```

## Typical characteristics

```curve id=tpd-vcc caption="TDP vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [0, 3.5]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 40]},
 "series": [
  {"label": "TPD in ns", "points": [[0.8, 34], [1.2, 13], [1.5, 9.5], [1.8, 7.7], [2.5, 5.7], [3.3, 4.7]]}
 ]}
```

```curve id=tpd-temp caption="TPD vs Temperature at 1.8 V"
{"x": {"label": "Temperature", "unit": "°C", "range": [-50, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 9]},
 "series": [
  {"label": "TPD in ns", "points": [[-40, 7.0], [25, 7.7], [85, 8.0], [125, 8.4]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, t~r~/t~f~ = 3 ns. The outputs are measured one at a
time, with one transition per measurement. t~PLH~ and t~PHL~ are the
same as t~pd~. All parameters and waveforms are not applicable to all
devices.

```table id=pmi-conditions caption="Load circuit test conditions"
| | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | V~CC~ = 1.5 V ± 0.1 V | V~CC~ = 1.8 V ± 0.15 V | V~CC~ = 2.5 V ± 0.2 V | V~CC~ = 3.3 V ± 0.3 V |
|---|---|---|---|---|---|---|
| C~L~ | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF |
| V~M~ | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 |
| V~I~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ |
```

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: VCC/2
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

The AUP family is TI's premier solution to the industry’s low-power
needs in battery-powered portable applications. This family ensures a
very low static and dynamic power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in an increased battery life. This
product also maintains excellent signal integrity. It has a small amount
of hysteresis built in allowing for slower or noisy input signals. The
lowered drive produces slower edges and prevents overshoot and
undershoot on the outputs.

Conversion note: the typical application schematic (Figure 5) shows a 3.3 V bus driver
driving the gate's inputs, the gate on a 1 V regulated V~CC~ with a 0.1 µF
capacitor, and the gate's output driving a 1 V micro processor.

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits.

1. Recommended Input conditions
   - Rise time and fall time specs. See (Δt/ΔV) in Recommended Operating Conditions
   - Specified high and low levels. See (V~IH~ and V~IL~) in Recommended Operating Conditions
   - Inputs are overvoltage tolerant allowing them to go as high as 3.6 V at any valid V~CC~
2. Recommend output conditions
   - Load currents should not exceed 20 mA on the output and 50 mA total for the part
   - Outputs should not be pulled above V~CC~

The AUP family of single gate logic makes excellent translators for the
new lower voltage microprocessors that typically are powered from 0.8 V
to 1.2 V. They can drop the voltage of peripheral drivers and
accessories that are still powered by 3.3 V to the new uC power levels.

### Power supply recommendations

The power supply can be any voltage between the Min and Max supply
voltage rating located in the Recommended Operating Conditions table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended; if
there are multiple V~CC~ pins, then 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and a 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Figure 8
specifies the rules that must be observed under all circumstances. All
unused inputs of digital logic devices must be connected to a high or
low bias to prevent them from floating. The logic level that should be
applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~, whichever makes
more sense or is more convenient. It is generally acceptable to float
outputs, unless the part is a transceiver. If the transceiver has an
output enable pin, it will disable the output section of the part when
asserted. This will not disable the input section of the I/Os, so they
cannot float when disabled.

Conversion note: the layout diagram (Figure 8, p.14) shows two gates, one with its
unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

```package footprint=X2SON-5-DPW
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-6-YFP
```

## Ordering information

```ordering
SN74AUP1G02DBVR | Reel of 3000 | -40 to 85 | H02R
SN74AUP1G02DBVRG4 | Reel of 3000 | -40 to 85 | H02R
SN74AUP1G02DBVT | Reel of 250 | -40 to 85 | H02R
SN74AUP1G02DCKR | Reel of 3000 | -40 to 85 | HB5, HBF, HBR
SN74AUP1G02DCKRG4 | Reel of 3000 | -40 to 85 | HB5, HBF, HBR
SN74AUP1G02DCKT | Reel of 250 | -40 to 85 | HB5, HBR
SN74AUP1G02DPWR | Reel of 3000 | -40 to 85 | B, B4
SN74AUP1G02DRLR | Reel of 4000 | -40 to 85 | HB7, HBR
SN74AUP1G02DRY2 | Reel of 5000 | -40 to 85 | HB
SN74AUP1G02DRYR | Reel of 5000 | -40 to 85 | HB
SN74AUP1G02DRYRG4 | Reel of 5000 | -40 to 85 | HB
SN74AUP1G02DSFR | Reel of 5000 | -40 to 85 | HB
SN74AUP1G02DSFRG4 | Reel of 5000 | -40 to 85 | HB
SN74AUP1G02YFPR | Reel of 3000 | | HBN
```

## Conversion notes

- Source pages 1–14 converted; the documentation support, package
  option addendum (p.16), tape and reel and mechanical pages (15–48)
  dropped, replaced by the generated Package and Ordering sections. The
  table of contents and revision history on page 2 are dropped too.
- Dropped: the features and applications lists, the Device Information
  table, Figure 4 (enable and disable times load circuit and waveforms,
  p.10) and the pulse-duration and setup/hold waveforms of Figure 3,
  which the source notes are not applicable to all devices (this gate
  has no output enable and no clock), Figure 6 (family power
  comparison) and Figure 7 (25 MHz switching waveform of the AUP1G08).
  The typical application schematic and the layout diagram are described
  in text.
- Curves digitized from Figures 1 and 2 (p.8) to about 2 % of full
  scale.
- The thermal table's "n/a" cells are written N/A.
- From images: overbars of Y = !{A + B} and Y = !{A} × !{B} (p.1,
  p.11); curve points 34, 13, 9.5, 7.7, 5.7, 4.7, 7.0, 7.7, 8.0, 8.4,
  125 (Figures 1 and 2, p.8).
- Source issues:
  - Source error: the overview (p.11) names the SN74LVC1G02; the
    datasheet is the SN74AUP1G02's. Kept as printed.
  - Source error: the caption of Figure 1 (p.8) reads "TDP vs V~CC~"
    where the axis and Figure 2 read TPD. Kept as printed.
  - Unconfirmed: the overview (p.11) says the I~off~ circuitry prevents
    backflow "when it is powered", without "down".
