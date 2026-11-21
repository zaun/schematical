---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv14a",
  "title": "SN74LV14A hex Schmitt-trigger inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lv14a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS386L",
    "date": "2022-12",
    "pages": 30
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74LV14ADBR", "SN74LV14ADGVR", "SN74LV14ADR", "SN74LV14ADRG4", "SN74LV14ANSR", "SN74LV14APW", "SN74LV14APWR", "SN74LV14APWRG3", "SN74LV14ARGYR"],
  "packages": {"p14": "D, DB, DGV, NS, PW, RGY"},
  "converted": "2026-11-21"
}
---

# SN74LV14A hex Schmitt-trigger inverters

## Description

These hex Schmitt-trigger inverters are designed for 2 V to 5.5 V V~CC~
operation.

The SN74LV14A devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}.

Max t~pd~ of 10 ns at 5 V. Support Mixed-Mode Voltage Operation on All
Ports. I~off~ Supports Partial-Power-Down Mode Operation. Latch-Up
Performance Exceeds 250 mA Per JESD 17.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Input 1A
1Y: Output 1Y
2A: Input 2A
2Y: Output 2Y
3A: Input 3A
3Y: Output 3Y
4Y: Output 4Y
4A: Input 4A
5Y: Output 5Y
5A: Input 5A
6Y: Output 6Y
6A: Input 6A
GND: Ground Pin
VCC: Power Pin
```

## Functional description

These hex Schmitt-trigger inverters are designed for 2 V to 5.5 V V~CC~
operation.

The SN74LV14A devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}.

These devices are fully specified for partial-power-down application
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the devices when they are powered
down.

Conversion note: the logic diagram, each inverter (positive logic)
(Figure 8-1), shows one inverter with a Schmitt-trigger symbol, input A
and output Y.

- Wide operating voltage range
  - Operates From 2 V to 5.5 V
- Allows up or down voltage translation
  - Inputs and outputs accept voltages to 5.5 V
- I~off~ feature
  - Allows voltages on the inputs and outputs when V~CC~ is 0 V
- Schmitt-trigger inputs allow for slow or noisy inputs

## Function tables

```table id=function caption="Function table (each inverter)"
| Input [1] | Output [2] |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Section 6.1 may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Section 6.3 is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5-V maximum.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | ^ |
| ^ | ^ | Machine Model | ±200 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | SN74LV14A | < | Unit |
| ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LV14A | < | < | < | < | < | Unit |
| ^ | ^ | D, 14 pins | DB, 14 pins | DGV, 14 pins | NS, 14 pins | PW, 14 pins | RGY, 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 94.9 | 107.4 | 130.4 | 91.4 | 122.6 | 57.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 56.3 | 59.9 | 53.4 | 49.0 | 51.3 | 70.4 | ^ |
| R~θJB~ | Junction-to-board thermal resistance | 49.2 | 54.7 | 63.5 | 50.2 | 64.4 | 33.6 | ^ |
| ψ~JT~ | Junction-to-top characterization parameter | 20.7 | 21.0 | 7.3 | 15.3 | 6.8 | 3.5 | ^ |
| ψ~JB~ | Junction-to-board characterization parameter | 48.9 | 51.2 | 62.8 | 49.8 | 63.8 | 33.7 | ^ |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | 14.1 | ^ |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | SN74LV14A –40°C to 85°C | < | < | SN74LV14A –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going threshold | | 2.5 V | | | 1.75 | | | 1.75 | V |
| ^ | ^ | | 3.3 V | | | 2.31 | | | 2.31 | ^ |
| ^ | ^ | | 5 V | | | 3.5 | | | 3.5 | ^ |
| V~T–~ | Negative-going threshold | | 2.5 V | 0.75 | | | 0.75 | | | V |
| ^ | ^ | | 3.3 V | 0.99 | | | 0.99 | | | ^ |
| ^ | ^ | | 5 V | 1.5 | | | 1.5 | | | ^ |
| ΔV~T~ (V~T+~ – V~T–~) | Hysteresis | | 2.5 V | 0.25 | | | 0.25 | | | V |
| ^ | ^ | | 3.3 V | 0.33 | | | 0.33 | | | ^ |
| ^ | ^ | | 5 V | 0.5 | | | 0.5 | | | ^ |
| V~OH~ | High-level output voltage | I~OH~ = –50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –2 mA | 2.3 V | 2 | | | 2 | | | ^ |
| ^ | ^ | I~OH~ = –6 mA | 3 V | 2.48 | | | 2.48 | | | ^ |
| ^ | ^ | I~OH~ = –12 mA | 4.5 V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | | | 0.44 | ^ |
| ^ | ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | | | 0.55 | ^ |
| I~I~ | Input leakage current | V~I~ = V~CC~ or GND | 0 to 5.5 V | | | ±1 | | | ±1 | µA |
| I~CC~ | Static supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | | | 20 | µA |
| I~off~ | Input/Output Power-Off Leakage Current | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | | | 5 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 2.3 | | | 2.3 | | pF |
| ^ | ^ | ^ | 5 V | | 2.3 | | | 2.3 | | ^ |
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | SN74LV14A | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic | | 3.1 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 8.8 | pF |
| ^ | ^ | ^ | 5 V | 9.6 | ^ |
```

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV14A –40°C to 85°C | < | SN74LV14A –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 10.2 [1] | 19.7 [1] | 1 | 22 | 1 | 23 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 13.3 | 24 | 1 | 27 | 1 | 28 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV14A –40°C to 85°C | < | SN74LV14A –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 7.3 [1] | 12.8 [1] | 1 | 15 | 1 | 16 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 9.6 | 16.3 | 1 | 18.5 | 1 | 19.5 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV14A –40°C to 85°C | < | SN74LV14A –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 5.1 [1] | 8.6 [1] | 1 | 10 | 1 | 11 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 6.7 | 10.6 | 1 | 12 | 1 | 13 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Typical characteristics

```curve id=tpd-vcc caption="Figure 6-1. TPD vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 12]},
 "series": [
  {"label": "TPD in ns", "points": [[2.5, 10.2], [3.3, 7.3], [5, 5.1]]}
 ]}
```

```curve id=tpd-temp caption="Figure 6-2. TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.7], [-40, 4.9], [25, 5.1], [85, 5.5], [125, 5.9]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

Conversion note: Figure 7-1 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the voltage
waveforms for pulse duration, setup and hold times, and enable and
disable times, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

Schmitt triggers should be used anytime you need to translate a sign
wave into a square wave as shown in Figure 9-1. They may also be used
where a slow or noisy input needs to be sped up or cleaned up as shown
in Figure 9-2.

Source error: "sign wave" is the source's spelling of "sine wave"
(Application Information, p.10); kept as printed.

Conversion note: the oscillator application schematic (Figure 9-1)
shows an inverter with a feedback resistor, a series resistor R~S~ and
a crystal with two capacitors C~2~ to ground, followed by one SN74LV14A
inverter that drives a capacitor C.

Conversion note: the switch de-bouncer schematic (Figure 9-2) shows a
resistor to 5 V and a 10 nF capacitor and a push button to ground on the
input of one Schmitt-trigger inverter.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing.

### Detailed design procedure

1. Recommended Input Conditions:
   - For rise time and fall time specifcations, see Δt/ΔV in Recommended
     Operating Conditions table.
   - For specified high and low levels, see V~IH~ and V~IL~ in
     Recommended Operating Conditions table.
   - Inputs and outputs are overvoltage tolerant allowing them to go as
     high as 5.5 V at any valid V~CC~.
2. Recommend Output Conditions:
   - Load currents should not exceed 35 mA per output and 50 mA total
     for the part.

Source error: "specifcations" is the source's spelling of
"specifications" (Detailed Design Procedure, p.11); kept as printed.

Source conflict: the design procedure (p.11) refers to Δt/ΔV, V~IH~ and
V~IL~ in the recommended operating conditions, whose table (p.4) has no
such rows.

Source conflict: the design procedure (p.11) allows load currents of
35 mA per output, while the absolute maximum ratings (p.4) give a
continuous output current of ±25 mA.

Conversion note: the Schmitt trigger curves (Figure 9-3) are a picture
of three pairs of IN and OUT waveforms: a sine wave, a noisy signal and
slow edges, each with a square-wave output.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table.

Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1 μF capacitor is
recommended. If there are multiple V~CC~ terminals then 0.01 μF or
0.022 μF capacitors are recommended for each power terminal. It is ok to
parallel multiple bypass capacitors to reject different frequencies of
noise. 0.1 μF and 1.0 μF capacitors are commonly used in parallel. The
bypass capacitor should be installed as close to the power terminal as
possible for the best results.

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states.
Specified in the Figure 11-1 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or
V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver.

Conversion note: the layout diagram (Figure 11-1) shows two gates, one
with its unused input tied to V~CC~ and one with its unused input tied
to ground.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74LV14ADBR | Reel of 2000 | -40 to 125 | LV14A
SN74LV14ADGVR | Reel of 2000 | -40 to 125 | LV14A
SN74LV14ADR | Reel of 2500 | -40 to 125 | LV14A
SN74LV14ADRG4 | Reel of 2500 | -40 to 125 | LV14A
SN74LV14ANSR | Reel of 2000 | -40 to 125 | 74LV14A
SN74LV14APW | | -40 to 125 | LV14A | Obsolete
SN74LV14APWR | Reel of 2000 | -40 to 125 | LV14A
SN74LV14APWRG3 | Reel of 2000 | -40 to 125 | LV14A
SN74LV14ARGYR | Reel of 3000 | -40 to 125 | LV14A
```

## Conversion notes

- Source pages 1–12 converted; of p.2 (table of contents and revision
  history) nothing is kept. The ordering lines are from the package
  option addendum (pp.14–15). The device and documentation support page
  (13), tape and reel and mechanical pages (16–30) are dropped; the
  generated Package section replaces them.
- Dropped: features and applications lists (four feature lines are in
  Description), the package information table, the simplified schematic
  (p.1), the vendor pin drawings and the NC line of the pin table (no
  package has such a pin). The logic diagram, the application
  schematics, the Schmitt trigger curves and the layout diagram are each
  described in one sentence.
- Not claimed here: SN74LV14AD and SN74LV14APWRG4 of the package option
  addendum, which the catalog does not carry.
- Dashes in the thermal table (no value) are empty cells.
- A "Section 6.3" of the source's application chapter is written as
  that section's name.
- Of Figure 7-1 (p.8) the load circuit for totem-pole outputs and the
  propagation delay waveforms are kept, with notes A, C, D, G and H.
- Curves digitized from Figures 6-1 and 6-2 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{A} (p.1 and p.9); the curve points of
  Figures 6-1 and 6-2 (p.7), of which -55, 4.7, 4.9, 5.5 and 5.9 are
  not in the text layer; 5 V and 10 nF of Figure 9-2 (p.10); 50% V~CC~
  and R~L~ = 1 kΩ of Figure 7-1 (p.8, a picture); R~θJC(top)~ and
  R~θJC(bot)~ (thermal table, p.5; the text layer breaks each over two
  lines).
- Source issues:
  - Source error: "sign wave" for "sine wave" (Application Information,
    p.10), kept as printed.
  - Source error: "specifcations" for "specifications" (Detailed Design
    Procedure, p.11), kept as printed.
  - Source conflict: the design procedure (p.11) refers to Δt/ΔV, V~IH~
    and V~IL~ rows that the recommended operating conditions (p.4) do
    not have.
  - Source conflict: 35 mA per output in the design procedure (p.11)
    against a continuous output current of ±25 mA in the absolute
    maximum ratings (p.4).
