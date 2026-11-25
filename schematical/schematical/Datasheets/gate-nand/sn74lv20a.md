---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv20a",
  "title": "SN74LV20A dual 4-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lv20a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES339F",
    "date": "2024-05",
    "pages": 26
  },
  "devices": ["gate:74_20"],
  "parts": ["SN74LV20AD", "SN74LV20ADBR", "SN74LV20ADGVR", "SN74LV20ADR", "SN74LV20ANSR", "SN74LV20APW", "SN74LV20APWR", "SN74LV20APWRG4"],
  "packages": {"p14": "D, DB, DGV, NS, PW"},
  "converted": "2026-11-21"
}
---

# SN74LV20A dual 4-input positive-NAND gate

## Description

These dual 4-input positive-NAND gates are designed for 2V to 5.5V
V~CC~ operation. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
NC: Not internally connected
1C: 1C Input
1D: 1D Input
1Y: 1Y Output
2Y: 2Y Output
2A: 2A Input
2B: 2B Input
2C: 2C Input
2D: 2D Input
GND: Ground Pin
VCC: Power Pin
```
[[3::Table 3-1]]

## Functional description

The SN74LV20A devices perform the Boolean function
Y = !{A • B • C • D} or Y = !{A} + !{B} + !{C} + !{D} in positive logic. [[8::1]]

These devices are fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the devices when they are powered
down. [[8::1]]

Conversion note: the logic diagram (Figure 6-1) shows one gate with
buffered inputs xA, xB, xC and xD and a buffered output xY.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | < | < | Output |
| A | B | C | D | Y |
|:-:|:-:|:-:|:-:|:-:|
| H | H | H | H | L |
| L | X | X | X | H |
| X | L | X | X | H |
| X | X | L | X | H |
| X | X | X | L | H |
```
[[3::Table 3-1]]

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
| T~J~ | Junction Temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

Latch-up performance exceeds 100mA per JESD 78, Class II. [[1::1]]

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
| Symbol | Thermal metric [1] | SN74LV20A | < | < | < | < | Unit |
| ^ | ^ | D (14 pins) | DB (14 pins) | DGV (14 pins) | NS (14 pins) | PW (14 pins) | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 96 | 127 | 76 | 113 | °C/W |

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
| I~off~ | Off-state leakage current | V~I~ or V~O~ = 0 to 5.5 V | 0 V | | | 5 | µA |
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
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 20.5 | pF |
| ^ | ^ | ^ | 5 V | 23.9 | pF |
```
[[6::table #2]]

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV20A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, C or D | Y | C~L~ = 15 pF | | 6.8 | 11.6 | 1 | 13.5 | ns |
| t~pd~ | A, B, C or D | Y | C~L~ = 50 pF | | 9.2 | 15.3 | 1 | 18.5 | ns |
```
[[5::table #4]]

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV20A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, C, or D | Y | C~L~ = 15 pF | | 4.9 | 6.6 | 1 | 8 | ns |
| t~pd~ | A, B, C, or D | Y | C~L~ = 50 pF | | 6.5 | 10.1 | 1 | 11.5 | ns |
```
[[6::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74LV20A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, C, or D | Y | C~L~ = 15 pF | | 3.7 | 5 | 1 | 6 | ns |
| t~pd~ | A, B, C, or D | Y | C~L~ = 50 pF | | 4.8 | 7 | 1 | 8 | ns |
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
CL: see table
```
[[7::Figure 5-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```
[[7::Figure 5-1]]

Conversion note: Figure 5-1 also shows the load circuit for 3-state and
open-drain outputs and the voltage waveforms for pulse duration, setup
and hold times, and enable and disable times.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results. [[9::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs must
not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used. Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient. [[9::3]]

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

## Ordering information

```ordering
SN74LV20AD | | -40 to 85 | LV20A | Obsolete
SN74LV20ADBR | Reel of 2000 | -40 to 85 | LV20A
SN74LV20ADGVR | Reel of 2000 | -40 to 85 | LV20A
SN74LV20ADR | Reel of 2500 | -40 to 85 | LV20A
SN74LV20ANSR | Reel of 2000 | -40 to 85 | 74LV20A
SN74LV20APW | | -40 to 85 | LV20A | Obsolete
SN74LV20APWR | Reel of 2000 | -40 to 85 | LV20A
SN74LV20APWRG4 | Reel of 2000 | -40 to 85 | LV20A
```
[[11::table #1]]

## Conversion notes

- Source pages 1–9 converted; the device and documentation support and
  revision history page (10), the package option addendum (pp.11–12),
  tape and reel and mechanical pages (13–26) dropped, replaced by the
  generated Package and Ordering sections.
- Dropped: features list (the latch-up line is under the ESD ratings),
  package information table, simplified schematic (p.1), table of
  contents (p.2), the vendor pinout drawing, the logic diagram
  (Figure 6-1, described in one sentence).
- Of Figure 5-1 (p.7) the load circuit for totem-pole outputs and the
  propagation delay waveforms are kept, with notes A, C, D, G and H.
- From images: overbars of Y = !{A • B • C • D} and
  Y = !{A} + !{B} + !{C} + !{D} (p.8); the 50% V~CC~ measurement levels
  (Figure 5-1, p.7, an image).
