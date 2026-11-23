---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv02a",
  "title": "SNx4LV02A quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74lv02a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS390K",
    "date": "2015-02",
    "pages": 30
  },
  "devices": ["gate:74_02"],
  "parts": ["SN74LV02AD", "SN74LV02ADBR", "SN74LV02ADBRE4", "SN74LV02ADGVR", "SN74LV02ADR", "SN74LV02ANSR", "SN74LV02APW", "SN74LV02APWR", "SN74LV02APWT", "SN74LV02ARGYR", "SN74LV02ARGYRG4"],
  "packages": {"p14": "D, DB, DGV, NS, PW, RGY"},
  "converted": "2026-11-21"
}
---

# SNx4LV02A quadruple 2-input positive-NOR gates

## Description

The SNx4LV02A devices are quadruple 2-input positive-NOR gates designed
for 2-V to 5.5-V V~CC~ operation. [[1::3]]

The SNx4LV02A devices perform the Boolean function Y = !{A + B} or
Y = !{A} ● !{B} in positive logic. [[1::4]]

Support Mixed-Mode Voltage Operation on All Ports. I~off~ Supports Live
insertion, Partial Power Down Mode, and Back Drive Protection. Latch-Up
Performance Exceeds 250 mA Per JESD 17. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1Y: 1Y output
1A: 1A input
1B: 1B input
2Y: 2Y output
2A: 2A input
2B: 2B input
GND: GND
3A: 3A input
3B: 3B input
3Y: 3Y output
4A: 4A input
4B: 4B input
4Y: 4Y output
VCC: V~CC~
```
[[3::table #1]]

## Functional description

The SN74LV02A devices are quadruple 2-input positive-NOR gates designed
for 2-V to 5.5-V V~CC~ operation. [[9::1]]

The SN74LV02A devices perform the Boolean function Y = !{A + B} or
Y = !{A} ● !{B} in positive logic. [[9::2]]

These devices are fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the devices when they are powered
down. [[9::3]]

Conversion note: the logic diagram (Figure 4) shows one 2-input NOR gate
with inputs A and B and output Y.

- Wide operating voltage range [[9::4]]
  - Operates from 2 V to 5.5 V [[9::4]]
- Allows down voltage translation [[9::4]]
  - Inputs accept voltages to 5.5 V [[9::4]]
- I~off~ feature allows voltages on the inputs and outputs when V~CC~ is
  0 V [[9::4]]

## Function tables

```table id=function caption="Function table (each gate)"
| Input | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
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
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±1500 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±2000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | SN54LV02A [2] | < | SN74LV02A | < | Unit |
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
| V~O~ | Output voltage | | 0 | 5.5 | 0 | 5.5 | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | | -12 | mA |
| I~IH~ [3] | Low-level output current | V~CC~ = 2 V [4] | | 50 | | 50 | mA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | | 12 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
[2] Product Preview.
[3] Source error: the source prints the symbol I~IH~ for the low-level output current; the electrical characteristics test V~OL~ at I~OL~. Kept as printed.
[4] Unconfirmed: the source prints one unit, mA, for all four rows of this parameter, so this row reads 50 mA.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LV02A | < | < | < | < | < | Unit |
| ^ | ^ | D, 14 pins | DB, 14 pins | DGV, 14 pins | NS, 14 pins | PW, 14 pins | RGY, 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 97.5 | 109.5 | 133.3 | 92.2 | 125.1 | 59.0 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 58.7 | 62.1 | 55.6 | 49.8 | 53.7 | 72.5 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 51.8 | 56.9 | 66.3 | 51.0 | 66.9 | 35.0 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 22.6 | 22.6 | 7.8 | 15.7 | 7.6 | 3.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 51.6 | 56.3 | 56.6 | 50.6 | 66.3 | 35.1 | °C/W |
| R~θJC(bot)~ | | | | | | | 15.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN54LV02A [1] | < | < | –40°C to 85°C SN74LV02A | < | –40°C to 125°C SN74LV02A | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OL~ = –50 µA [2] | 2 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V~CC~ - 0.1 | | V |
| ^ | I~OL~ = –2 mA [2] | 2.3 V | 2 | | | 2 | | 2 | | V |
| ^ | I~OL~ = –6 mA [2] | 3 V | 2.48 | | | 2.48 | | 2.48 | | V |
| ^ | I~OL~ = –12 mA [2] | 4.5 V | 3.8 | | | 3.8 | | 3.8 | | V |
| V~OL~ | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | 0.4 | | 0.4 | V |
| ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | | 0.44 | | 0.44 | V |
| ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | | 0.55 | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 | | | 20 | | 20 | | 20 | µA |
| I~off~ | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | | 5 | | 5 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 1.6 | | 1.6 [3] | | 1.6 [3] | | pF |

[1] Product Preview.
[2] Source error: the source prints I~OL~ in the V~OH~ test conditions; the negative currents are the high-level output currents I~OH~ of the recommended operating conditions. Kept as printed.
[3] Conversion note: the source prints this value between the MIN and MAX columns of a group that has no TYP column.
```
[[6::table #1]]

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | SN74LV02A | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.2 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```
[[7::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 8.9 | pF |
| ^ | ^ | ^ | 5 V | 3 [1] | pF |

[1] Unconfirmed: 3 pF at 5 V is below the 8.9 pF at 3.3 V; kept as printed.
```
[[7::table #2]]

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LV02A | < | –40°C to 85°C SN74LV02A | < | –40°C to 125°C SN74LV02A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 8.3 [1] | 12.4 [1] | 1 [1] | 15 [1] | 1 | 15 | 1 | 17.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 11 [1] | 16.1 [1] | 1 | 19 | 1 | 19 | 1 | 21.5 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #2]]

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LV02A | < | –40°C to 85°C SN74LV02A | < | –40°C to 125°C SN74LV02A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 5.6 [1] | 7.9 [1] | 1 [1] | 9.5 [1] | 1 | 9.5 | 1 | 11.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 7.6 [1] | 11.4 [1] | 1 | 13 | 1 | 13 | 1 | 15 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LV02A | < | –40°C to 85°C SN74LV02A | < | –40°C to 125°C SN74LV02A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 3.9 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | 1 | 8 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 5.3 [1] | 7.5 [1] | 1 | 8.5 | 1 | 8.5 | 1 | 10 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #4]]

## Typical characteristics

```curve id=tpd-vcc caption="Figure 1. TPD vs Temperature"
{"x": {"label": "VCC", "unit": "V", "range": [1, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 9]},
 "series": [
  {"label": "TPD in ns", "points": [[2.5, 8.3], [3.3, 5.6], [5, 3.9]]}
 ]}
```
[[7::Figure 1]]

```curve id=tpd-temp caption="Figure 2. TPD vs V~CC~ at 25°C"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 5]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 3.5], [-40, 3.7], [25, 3.9], [85, 4.35], [125, 4.7]]}
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
all devices. [[8::Figure 3]]

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

The SN74LV02A is a Low drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The inputs can accept voltages to 5.5 V at
any valid V~CC~ making it Ideal for down translation. [[10::2]]

Conversion note: in the typical application schematic (Figure 5) a 5-V
system logic block drives the two inputs of one NOR gate, whose output
drives a µC or system logic block; the gate is supplied from 3.3 V or
5 V regulated, with a 0.1 µF capacitor to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing. [[10::3]]

### Detailed design procedure

1. Recommended Input Conditions [[10::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~. [[10::4]]
2. Recommend Output Conditions [[10::4]]
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part. [[10::4]]
   - Outputs should not be pulled above V~CC~. [[10::4]]

Conversion note: the typical application curve (Figure 6) is a picture
of the output falling edges of the AC02, the HC02 and the LV02A over
20 ns, at V~CC~ = 5 V with a load of 50 Ω/50 pF.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[11::1]]

Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1 μf capacitor is
recommended and if there are multiple V~CC~ terminals then .01 μf or
.022 μf capactior is recommended for each power terminal. It is ok to
parallel multiple bypass capacitors to reject different frequencies of
noise. 0.1 μf and 1 μf capacitors are commonly used in parallel. The
bypass capacitor should be installed as close to the power terminal as
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
is more convenient. It is generally OK to float outputs unless the part
is a transceiver. If the transceiver has an output enable pin it will
disable the outputs section of the part when asserted. This will not
disable the input section of the I.O’s so they also cannot float when
disabled. [[12::1]]

Conversion note: the layout diagram (Figure 7) shows one gate with its
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
SN74LV02AD | | -40 to 125 | LV02A | Obsolete
SN74LV02ADBR | Reel of 2000 | -40 to 125 | LV02A | NRND
SN74LV02ADBRE4 | Reel of 2000 | -40 to 125 | LV02A | NRND
SN74LV02ADGVR | Reel of 2000 | -40 to 125 | LV02A
SN74LV02ADR | Reel of 2500 | -40 to 125 | LV02A
SN74LV02ANSR | Reel of 2000 | -40 to 125 | 74LV02A | Last Time Buy
SN74LV02APW | | -40 to 125 | LV02A | Obsolete
SN74LV02APWR | Reel of 2000 | -40 to 125 | LV02A
SN74LV02APWT | | -40 to 125 | LV02A | Obsolete
SN74LV02ARGYR | Reel of 3000 | -40 to 125 | LV02A
SN74LV02ARGYRG4 | Reel of 3000 | -40 to 125 | LV02A
```
[[14::table #1]]

## Conversion notes

- Source pages 1–12 converted (of page 12 the layout section only); the
  mechanical, packaging and orderable information page (13), the package
  option addendum (pp.14–15), tape and reel and mechanical pages (16–30)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists (three feature lines are in
  Description), device information table, revision history, the device
  and documentation support section, the simplified schematic (p.1),
  the vendor pinout drawings, the logic diagram (Figure 4), the typical
  application schematic (Figure 5), the application curve (Figure 6)
  and the layout diagram (Figure 7), the last four each described in
  one sentence.
- The data sheet also covers the SN54LV02A (J, W and FK packages,
  product preview), which the catalog does not carry; its columns and
  notes are kept in the tables.
- The electrical characteristics table of the source gives symbols
  only, so it has no Parameter column. The R~θJC(bot)~ row of the
  thermal table has no metric name in the source.
- Dashes in the thermal table (no value) are empty cells.
- Curves digitized from Figures 1 and 2 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{A + B} and Y = !{A} ● !{B} (p.1 and
  p.9); the note markers on the 50-pF rows of the switching tables
  (p.6); the curve points of Figures 1 and 2 (p.7); the labels of
  Figure 6 (p.11).
- From images: page 8 (Figure 3 and its notes A to H) is a picture
  with no text layer: the Parameter measurement information sentences,
  "C~L~ includes probe and jig capacitance.", "All input pulses are
  supplied by generators having the following characteristics:
  PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns.", "The outputs
  are measured one at a time, with one input transition per
  measurement.", "t~PHL~ and t~PLH~ are the same as t~pd~." and "All
  parameters and waveforms are not applicable to all devices.", 50%
  V~CC~ and R~L~ = 1 kΩ are read from it.
- Source issues:
  - Source error: the recommended operating conditions (p.5) print the
    symbol I~IH~ for the low-level output current; kept as printed.
  - Unconfirmed: the same parameter has one unit, mA, for its four rows,
    so the V~CC~ = 2 V row reads 50 mA (p.5).
  - Source error: the V~OH~ test conditions of the electrical
    characteristics (p.6) print I~OL~ with negative currents; kept as
    printed.
  - Unconfirmed: C~pd~ at 5 V is printed 3 pF (p.7), below the 8.9 pF at
    3.3 V.
  - Source error: the captions of Figures 1 and 2 (p.7) are exchanged
    against their axes; kept as printed.
  - Source conflict: the ESD ratings (p.4) give ±1500 V HBM and ±2000 V
    CDM; the Features (p.1) give "2000-V Human-Body Model" and "1000-V
    Charged-Device Model". The table is kept as printed.
  - Source error: Power Supply Recommendations (p.11) prints "capactior"
    and the unit "μf"; kept as printed.
  - Source conflict: Application Information (p.10) calls the part a
    "Low drive CMOS device" with "slow edge rates", while Design
    Requirements (p.10) says "The high drive will also create fast
    edges". Both kept.
