---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv00a",
  "title": "SNx4LV00A quadruple 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lv00a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS389K",
    "date": "2015-02",
    "pages": 29
  },
  "devices": ["gate:74_00"],
  "parts": ["SN74LV00AD", "SN74LV00ADBR", "SN74LV00ADGVR", "SN74LV00ADR", "SN74LV00ANSR", "SN74LV00APW", "SN74LV00APWR", "SN74LV00APWRG4", "SN74LV00APWT", "SN74LV00ARGYR"],
  "packages": {"p14": "D, DB, DGV, NS, PW, RGY"},
  "converted": "2026-11-21"
}
---

# SNx4LV00A quadruple 2-input positive-NAND gates

## Description

These quadruple 2-input positive-NAND gates are designed for 2-V to
5.5-V V~CC~ operation. [[1::3]]

The SNx4LV00A devices perform the boolean function Y = !{A ● B} or
Y = !{A} + !{B} in positive logic. [[1::4]]

Support Mixed-Mode Voltage Operation on All Ports. I~off~ Supports Live
Insertion, Partial Power Down Mode, and Back Drive Protection. Latch-Up
Performance Exceeds 250 mA Per JESD 17. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
1Y: 1Y Output
2A: 2A Input
2B: 2B Input
2Y: 2Y Output
GND: GND
3Y: 3Y Output
3A: 3A Input
3B: 3B Input
4Y: 4Y Output
4A: 4A Input
4B: 4B Input
VCC: Power Pin
```
[[3::table #1]]

## Functional description

These quadruple 2-input positive-NAND gates are designed for 2-V to
5.5-V V~CC~ operation. [[1::3]]

The SNx4LV00A devices perform the boolean function Y = !{A ● B} or
Y = !{A} + !{B} in positive logic [[1::4]]

These devices are fully specified for partial-power-down application
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the devices when they are powered
down. [[9::3]]

Conversion note: the functional block diagram shows one 2-input NAND
gate with inputs A and B and output Y.

- Wide operating voltage range [[9::4]]
  - Operates from 2 V to 5.5 V [[9::4]]
- Allows down voltage translation [[9::4]]
  - Inputs accept voltages to 5.5 V [[10::2]]
- I~off~ feature allows voltages on the input or output when V~CC~ is
  0 V. [[9::4]]

## Function tables

```table id=function caption="Function table (each gate)"
| Input | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[9::Table 1]]

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
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5-V maximum.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | +2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | +1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | SN54LV00A [2] | < | SN74LV00A | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V~CC~ × 0.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | | -12 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | | 12 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
[2] Product Preview.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SNx4LV00A | < | < | < | < | < | Unit |
| ^ | ^ | D, 14 pins | DB, 14 pins | DGV, 14 pins | NS, 14 pins | PW, 14 pins | RGY, 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 90.6 | 107.1 | 129.0 | 90.7 | 122.6 | 57.5 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 50.9 | 59.6 | 521 [2] | 48.3 | 51.4 | 70.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 44.8 | 54.4 | 62.0 | 49.4 | 64.4 | 33.6 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 14.7 | 20.5 | 6.5 | 14.6 | 6.7 | 34 [3] | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 44.5 | 53.8 | 61.3 | 49.1 | 63.8 | 33.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | 13.9 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
[2] Unconfirmed: 521 for the DGV package is about ten times the values of the other packages; kept as printed.
[3] Unconfirmed: 34 for the RGY package is far above the values of the other packages; kept as printed.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN54LV00A [1] | < | < | –40°C to 85°C SN74LV00A | < | –40°C to 125°C SN74LV00A | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V~CC~ - 0.1 | | V |
| ^ | I~OH~ = –2 mA | 2.3 V | 2 | | | 2 | | 2 | | V |
| ^ | I~OH~ = –6 mA | 3 V | 2.48 | | | 2.48 | | 2.48 | | V |
| ^ | I~OH~ = –12 mA | 4.5 V | 3.8 | | | 3.8 | | 3.8 | | V |
| V~OL~ | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | 0.4 | | 0.4 | V |
| ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | | 0.44 | | 0.44 | V |
| ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | | 0.55 | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | | 20 | | 20 | µA |
| I~off~ | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | | 5 | | 5 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 3.3 | | | | 3.3 [2] | | pF |
| ^ | ^ | 5 V | | 3.3 | | | | 3.3 [2] | | pF |

[1] Product Preview.
[2] Conversion note: the source prints this value under MIN in the –40°C to 125°C group, which has no TYP column.
```
[[6::table #1]]

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | SN74LV04A [2] | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.1 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
[2] Source error: the source heads these columns SN74LV04A in the data sheet of the SN74LV00A. Kept as printed.
```
[[7::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 9.5 | pF |
| ^ | ^ | ^ | 5 V | 11 | pF |
```
[[7::table #2]]

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LV00A [1] | < | –40°C to 85°C SN74LV00A | < | –40°C to 125°C SN74LV00A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 7.1 [2] | 12.9 [2] | 1 [2] | 16 [2] | 1 | 15 | 1 | 16 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 9.6 | 16.6 | 1 | 21 | 1 | 20 | 1 | 21 | ns |

[1] Product Preview.
[2] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #2]]

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LV00A [1] | < | –40°C to 85°C SN74LV00A | < | –40°C to 125°C SN74LV00A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 5 [2] | 7.9 [2] | 1 [2] | 10.5 [2] | 1 | 9.5 | 1 | 10.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 6.9 | 11.4 | 1 | 14 | 1 | 13 | 1 | 14 | ns |

[1] Product Preview.
[2] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LV00A [1] | < | –40°C to 85°C SN74LV00A | < | –40°C to 125°C SN74LV00A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 3.6 [2] | 5.5 [2] | 1 [3] | 7.5 [3] | 1 | 6.5 | 1 | 7 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 4.9 | 7.5 | 1 | 9.5 | 1 | 8.5 | 1 | 9 | ns |

[1] Product Preview.
[2] On products compliant to MIL-PRF-38535, this parameter is not production tested.
[3] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #4]]

## Typical characteristics

```curve id=tpd-vcc caption="Figure 1. TPD vs Temperature at 5 V"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD in ns", "points": [[2.5, 7.1], [3.3, 5], [5, 3.6]]}
 ]}
```
[[7::Figure 1]]

```curve id=tpd-temp caption="Figure 2. TPD vs V~CC~ at 25°C"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.6], [-40, 4.8], [25, 5.0], [85, 5.4], [125, 5.8]]}
 ]}
```
[[7::Figure 2]]

Source error: the source's captions of Figures 1 and 2 are exchanged;
the axis of Figure 1 is V~CC~ and the axis of Figure 2 is temperature.
Kept as printed.

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices. [[8::1]]

Conversion note: C~L~ is 15 pF or 50 pF as given in the
[switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[8::Figure 3]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```
[[8::Figure 3]]

Conversion note: Figure 3 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse
duration, setup and hold times, and enable and disable times.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality. [[10::1]]

The SN74LV00A is a Low drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The inputs can accept voltages to 5.5 V at
any valid V~CC~ making it Ideal for down translation. [[10::2]]

Conversion note: in the typical application schematic (Figure 4) a 5-V
system logic block, supplied from a 5-V accessory, drives the two
inputs of one NAND gate, whose output drives a µC or system logic
block; the gate is supplied from 3.3 V or 5 V regulated, with a 0.1 µF
capacitor to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing. [[10::3]]

### Detailed design procedure

1. Recommended Input Condition [[10::4]]
   - Specified high and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~. [[10::4]]
2. Recommend Output Conditions [[10::4]]
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part. [[10::4]]
   - Outputs should not be pulled above V~CC~. [[10::4]]

Conversion note: the typical application curve (Figure 5) is a picture
of the output falling edges of the AC00, the HC00 and the LV00A over
20 ns, at V~CC~ = 5 V with a load of 50 Ω/50 pF.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[11::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1 μF capacitor is
recommended. If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF
capacitors are recommended for each power pin. It is acceptable to
parallel multiple bypass capacitors to reject different frequencies of
noise. 0.1 μF and 1 μF capacitors are commonly used in parallel. The
bypass capacitor should be installed as close to the power pin as
possible for best results. [[11::2]]

### Layout guidelines

When using multiple bit logic devices inputs should not ever float. In
many cases, functions or parts of functions of digital logic devices are
unused, for example, when only two inputs of a triple-input AND gate are
used or only 3 of the 4 buffer gates are used. Such input pins should
not be left unconnected because the undefined voltages at the outside
connections result in undefined operational states. Specified below are
the rules that must be observed under all circumstances. All unused
inputs of digital logic devices must be connected to a high or low bias
to prevent them from floating. The logic level that should be applied to
any particular unused input depends on the function of the device.
Generally they will be tied to GND or V~CC~ whichever make more sense or
is more convenient. It is generally acceptable to float outputs unless
the part is a transceiver. If the transceiver has an output enable pin
it will disable the outputs section of the part when asserted. This will
not disable the input section of the I.O’s so they also cannot float
when disabled. [[11::3]]

Conversion note: the layout diagram (Figure 6) shows one gate with its
unused input tied to V~CC~ and one with its unused input tied to
ground.

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
SN74LV00AD | | -40 to 125 | LV00A | Obsolete
SN74LV00ADBR | Reel of 2000 | -40 to 125 | LV00A | NRND
SN74LV00ADGVR | Reel of 2000 | -40 to 125 | LV00A
SN74LV00ADR | Reel of 2500 | -40 to 125 | LV00A
SN74LV00ANSR | Reel of 2000 | -40 to 125 | 74LV00A | Last Time Buy
SN74LV00APW | | -40 to 125 | LV00A | Obsolete
SN74LV00APWR | Reel of 2000 | -40 to 125 | LV00A
SN74LV00APWRG4 | 2000 | -40 to 125 | LV00A
SN74LV00APWT | | -40 to 125 | LV00A | Obsolete
SN74LV00ARGYR | Reel of 3000 | -40 to 125 | LV00A | Last Time Buy
```
[[13::table #1]]

## Conversion notes

- Source pages 1–12 converted (of page 12 nothing is kept); the package
  option addendum (pp.13–14), tape and reel and mechanical pages
  (15–29) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists (three feature lines are in
  Description), device information table, table of contents, revision
  history, the device and documentation support section, the
  simplified schematic (p.1), the vendor pinout drawings, the
  functional block diagram, the typical application schematic
  (Figure 4), the application curve (Figure 5) and the layout diagram
  (Figure 6), the last four each described in one sentence.
- The data sheet also covers the SN54LV00A (J, W and FK packages,
  product preview), which the catalog does not carry; its columns and
  notes are kept in the tables.
- The electrical characteristics table of the source gives symbols
  only, so it has no Parameter column.
- Dashes in the thermal table (no value) are empty cells.
- Of Figure 3 (p.8) the load circuit for totem-pole outputs and the
  propagation delay waveforms are kept, with notes A, C, D, G and H.
- Curves digitized from Figures 1 and 2 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{A ● B} and Y = !{A} + !{B} (p.1 and
  p.9); the curve points of Figures 1 and 2 (p.7), of which -55, 85,
  4.6, 4.8, 5.0, 5.4 and 5.8 are not in the text layer; the labels
  AC00, HC00 and LV00A of Figure 5 (p.11).
- Source issues:
  - Unconfirmed: R~θJC(top)~ of the DGV package is printed 521 and
    ψ~JT~ of the RGY package 34 (p.5).
  - Source error: the noise characteristics (p.7) head the value
    columns SN74LV04A; kept as printed.
  - Source error: the captions of Figures 1 and 2 (p.7) are exchanged
    against their axes; kept as printed.
  - Source conflict: Application Information (p.10) calls the part a
    "Low drive CMOS device" with "slow edge rates", while Design
    Requirements (p.10) says "The high drive will also create fast
    edges". Both kept.
