---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv11a",
  "title": "SN74LV11A triple 3-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lv11a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES345E",
    "date": "2024-05",
    "pages": 26
  },
  "devices": ["gate:74_11"],
  "parts": ["SN74LV11AD", "SN74LV11ADBR", "SN74LV11ADGVR", "SN74LV11ADR", "SN74LV11ANSR", "SN74LV11APW", "SN74LV11APWR"],
  "packages": {"p14": "D, NS, DGV, DB, PW"},
  "converted": "2026-11-08"
}
---

# SN74LV11A triple 3-input positive-AND gates

## Description

These triple 3-input positive-AND gates are designed for 2-V to 5.5-V
V~CC~ operation. The SN74LV11A devices perform the Boolean function
Y = A • B • C in positive logic. [[8::1]] [[8::2]]

These devices are fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the devices when they are powered
down. [[8::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A: Input A
1B, 2B, 3B: Input B
1C, 2C, 3C: Input C
1Y, 2Y, 3Y: Output Y
GND: Ground pin
VCC: Power pin
```
[[3::table #1]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs [1] | < | < | Output [2] |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | H |
| L | X | X | L |
| X | L | X | L |
| X | X | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don't Care
[2] H = Driving High, L = Driving Low
```
[[8::Table 6-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range applied in high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range applied in power-off state [2] | | -0.5 | 7 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
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

Latch-up performance exceeds 100 mA per JESD 78, Class II. [[1::2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
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

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | D (14 pins) | NS (14 pins) | PW (14 pins) | DB (14 pins) | DGV (14 pins) | Unit |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 76 | 113 | 96 | 127 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #2]]

## Electrical characteristics

Conversion note: the source has no electrical characteristics table for output
voltages, leakage and supply current; only the noise and operating
characteristics below.

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25 °C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | 0 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.2 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```
[[5::table #3]] [[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 13.9 | pF |
| ^ | ^ | ^ | 5 V | 15.4 | pF |
```
[[6::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).
Conversion note: the SN74LV11A columns are over the recommended operating free-air
temperature range.

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | Load capacitance | T~A~ = 25 °C | < | < | SN74LV11A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 6.9 | 13.8 | 1 | 16 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 9.9 | 17.5 | 1 | 21 | ns |
```
[[5::table #4]]

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | Load capacitance | T~A~ = 25 °C | < | < | SN74LV11A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 5.2 | 8.8 | 1 | 10.5 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 7.2 | 12.3 | 1 | 14 | ns |
```
[[5::table #5]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | Load capacitance | T~A~ = 25 °C | < | < | SN74LV11A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | C~L~ = 15 pF | | 3.9 | 5.9 | 1 | 7 | ns |
| t~pd~ | A, B, or C | Y | C~L~ = 50 pF | | 5.4 | 7.9 | 1 | 9 | ns |
```
[[5::table #6]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The
outputs are measured one at a time, with one input transition per
measurement. t~PHL~ and t~PLH~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[7::2]]

Conversion note: C~L~ is 15 pF or 50 pF as given in the
[switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[7::Figure 5-1]]

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: 50 % V~CC~
```
[[7::Figure 5-1]]

Conversion note: Figure 5-1 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse
duration, setup and hold times, and enable and disable times.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum
and maximum supply voltage rating located in the Recommended Operating
Conditions. Each V~CC~ terminal should have a good bypass capacitor to
prevent power disturbance. A 0.1-µF capacitor is recommended for this
device. It is acceptable to parallel multiple bypass caps to reject
different frequencies of noise. The 0.1-µF and 1-µF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results. [[9::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices
inputs must not ever be left floating. Such unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a logic high or logic low voltage, as
defined by the input voltage specifications, to prevent them from
floating. The logic level that must be applied to any particular unused
input depends on the function of the device. Generally, the inputs are
tied to GND or V~CC~, whichever makes more sense for the logic function
or is more convenient. [[9::3]]

Conversion note: the example layout for the SN74LV11A (Figure 7-1) is given here by its labels.

- Recommend GND flood fill for improved signal isolation, noise reduction, and thermal dissipation [[9::4]]
- Unused inputs tied to GND [[9::3]]
- Bypass capacitor placed close to the device [[9::6]]
- 0.1 µF [[9::2]]
- Unused input tied to V~CC~ [[9::3]]
- Unused output left floating [[9::3]]
- Avoid 90° corners for signal lines [[9::5]]

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
SN74LV11AD | | -40 to 85 | LV11A | Obsolete
SN74LV11ADBR | Reel of 2000 | -40 to 85 | LV11A
SN74LV11ADGVR | Reel of 2000 | -40 to 85 | LV11A
SN74LV11ADR | Reel of 2500 | -40 to 85 | LV11A
SN74LV11ANSR | Reel of 2000 | -40 to 85 | 74LV11A
SN74LV11APW | | -40 to 85 | LV11A | Obsolete
SN74LV11APWR | Reel of 2000 | -40 to 85 | LV11A
```
[[11::table #1]]

## Conversion notes

- Source pages 1–10 converted; the package option addendum, tape and reel
  and mechanical pages (11–26) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: features list (except the latch-up rating), package
  information table, simplified schematic and logic diagram (three
  3-input AND gates), related links table, support pages, revision
  history.
- The layout example (Figure 7-1) is given as a list of its labels.
- From images: R~L~ = 1 kΩ and the 50 % V~CC~ measurement levels
  (Figure 5-1, p.7, an image); µF (0.1 µF in Figure 7-1, p.9; the text
  layer has "0.1 F").
- Source issues:
  - Source error: the Overview (p.8) gives the Boolean function as
    "Y = A • B • C or Y = !{A + B + C}" with a single overline (a NOR);
    the function table (Table 6-1, p.8) shows an AND. The second form is
    kept out.
  - Unconfirmed: the source has no electrical characteristics table for
    output voltages, leakage and supply current; the table of contents
    (p.2) lists none either.
