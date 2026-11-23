---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv10a",
  "title": "SN74LV10A triple 3-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lv10a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES338G",
    "date": "2023-03",
    "pages": 22
  },
  "devices": ["gate:74_10"],
  "parts": ["SN74LV10AD", "SN74LV10ADR", "SN74LV10ADRG4", "SN74LV10ANSR", "SN74LV10APWR"],
  "packages": {"p14": "D, NS, PW"},
  "converted": "2026-11-21"
}
---

# SN74LV10A triple 3-input positive-NAND gate

## Description

These triple 3-input positive-NAND gates are designed for 2 V to 5.5 V
V~CC~ operation. The SN74LV10A devices perform the Boolean function
Y = !{A • B • C} in positive logic. These devices are fully specified
for partial-power-down applications using I~off~. The I~off~ circuitry
disables the outputs, preventing damaging current backflow through the
devices when they are powered down. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
2A: 2A Input
2B: 2B Input
2C: 2C Input
2Y: 2Y Output
GND: Ground Pin
3Y: 3Y Output
3A: 3A Input
3B: 3B Input
3C: 3C Input
1Y: 1Y Output
1C: 1C Input
VCC: Power Pin
```
[[3::Table 5-1]]

Source error: the source's pin functions table (Table 5-1) is not that
of this device: it lists NC at pins 3 and 11, 1C at pin 4, 1D at pin 5,
1Y at pin 6, 2Y at pin 8 and 2D at pin 13, and no gate 3. Conversion
note: the pin numbers here follow the package drawing (Figure 5-1).

## Functional description

These triple 3-input positive-NAND gates are designed for 2-V to 5.5-V
V~CC~ operation. The SN74LV10A devices perform the Boolean function
Y = !{A • B • C} in positive logic. These devices are fully specified
for partial-power-down applications using I~off~. The I~off~ circuitry
disables the outputs, preventing damaging current backflow through the
devices when they are powered down. [[1::1]]

Conversion note: the simplified schematic (Figure 8-1) shows one
3-input NAND gate with buffered inputs xA, xB and xC and a buffered
output xY.

## Function tables

```table id=function caption="Function table (each gate)"
| Input [1] | < | < | Output [2] |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | L |
| L | X | X | H |
| X | L | X | H |
| X | X | L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don't Care
[2] H = Driving High, L = Driving Low
```
[[8::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range applied in high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range applied in power-off state [2] | | -0.5 | 7 | V |
| I~IK~ | Input clamp current | (V~I~ < 0) | | -20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0) | | -50 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance [4] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
[4] Unconfirmed: the source gives a package thermal impedance θ~JA~ of 150 in °C; the thermal information gives R~θJA~ in °C/W. Kept as printed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Machine Model, per JEDEC specification | ±200 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. [[1::1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V |
| V~IL~ | Low level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | mA |
| I~OL~ | Low level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise and fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LV10A | < | < | Unit |
| ^ | ^ | D, 14 pins | NS, 14 pins | PW, 14 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 76 | 113 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = –50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –2 mA | 2.3 V | 2 | | | V |
| ^ | ^ | I~OH~ = –6 mA | 3 V | 2.48 | | | V |
| ^ | ^ | I~OH~ = –12 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | V |
| ^ | ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 1.9 | | pF |
```
[[5::table #3]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter [1] | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | 0 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.2 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 14 | pF |
| ^ | ^ | ^ | 5 V | 16.7 | pF |
```
[[6::table #2]]

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV10A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 7.1 | 13 | 1 | 15.5 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 10.3 | 17.1 | 1 | 20.5 | ns |
```
[[5::table #4]]

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV10A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 5.2 | 8.4 | 1 | 10 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 7.4 | 11.9 | 1 | 13.5 | ns |
```
[[6::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV10A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 3.9 | 5.9 | 1 | 7 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 5.4 | 7.9 | 1 | 9 | ns |
```
[[6::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices. [[7::2]]

Conversion note: C~L~ is 15 pF or 50 pF as given in the
[switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[7::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```
[[7::Figure 7-1]]

Conversion note: Figure 7-1 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse
duration, setup and hold times, and enable and disable times.

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality. [[9::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1-μF capacitor is recommended for this device.
It is acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results. [[9::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs
must not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4
buffer gates are used. Such unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a logic high or logic low voltage,
as defined by the input voltage specifications, to prevent them from
floating. The logic level that must be applied to any particular unused
input depends on the function of the device. Generally, the inputs are
tied to GND or V~CC~, whichever makes more sense for the logic function
or is more convenient. [[9::3]]

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LV10AD | | -40 to 85 | LV10A | Obsolete
SN74LV10ADR | Reel of 2500 | -40 to 85 | LV10A
SN74LV10ADRG4 | Reel of 2500 | -40 to 85 | LV10A
SN74LV10ANSR | Reel of 2000 | -40 to 85 | 74LV10A
SN74LV10APWR | Reel of 2000 | -40 to 85 | LV10A
```
[[11::table #1]]

## Conversion notes

- Source pages 1–9 converted; the device and documentation support
  page (10), the package option addendum (pp.11–12), tape and reel and
  mechanical pages (13–22) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: features and applications lists (the latch-up line is under
  the ESD ratings), package information table, simplified schematic
  (p.1), table of contents, revision history, the vendor pinout
  drawing and pin functions table, the simplified schematic of the
  functional block diagram (Figure 8-1, described in one sentence).
- The recommended operating conditions table, which the source splits
  over pages 4 and 5, is one table here.
- Of Figure 7-1 (p.7) the load circuit for totem-pole outputs and the
  propagation delay waveforms are kept, with notes A, C, D, G and H.
- The package option addendum also lists SN74LV10APWRG4.A, which the
  catalog does not carry.
- From images: overbar of Y = !{A • B • C} (p.1 and p.8); R~L~ = 1 kΩ
  and the 50% V~CC~ measurement levels (Figure 7-1, p.7, an image).
- Source issues:
  - Source error: the pin functions table (Table 5-1, p.3) lists NC,
    1D and 2D pins and no gate 3; it is not the table of this device.
    Kept out; the pin descriptions follow its wording and the pin
    numbers of Figure 5-1.
  - Unconfirmed: the absolute maximum ratings (p.4) give a package
    thermal impedance θ~JA~ of 150 °C.
