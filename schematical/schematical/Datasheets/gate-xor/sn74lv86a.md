---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv86a",
  "title": "SNx4LV86A quadruple 2-input exclusive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74lv86a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS392G",
    "date": "2015-02",
    "pages": 28
  },
  "devices": ["gate:74_86"],
  "parts": ["SN74LV86AD", "SN74LV86ADBR", "SN74LV86ADGVR", "SN74LV86ADR", "SN74LV86ANSR", "SN74LV86APW", "SN74LV86APWR", "SN74LV86APWRG4", "SN74LV86APWT"],
  "packages": {"p14": "D, DB, DGV, NS, PW"},
  "converted": "2026-11-21"
}
---

# SNx4LV86A quadruple 2-input exclusive-OR gates

## Description

The ’LV86A devices are quadruple 2-input exclusive-OR gates designed for
2-V to 5.5-V V~CC~ operation.

These devices contain four independent 2-input exclusive-OR gates. They
perform the Boolean function Y = A ⊕ B or Y = !{A}B + A!{B} in positive
logic.

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the output.
If one of the inputs is high, the signal on the other input is
reproduced inverted at the output.

Support Mixed-Mode Voltage Operation on All Ports. Latch-Up Performance
Exceeds 250 mA per JESD 17.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: A input 1
1B: B input 1
1Y: Output 1
2A: A input 2
2B: B input 2
2Y: Output 2
GND: ground
3Y: Output 3
3A: A input 3
3B: B input 3
4Y: Output 4
4A: A input 4
4B: B input 4
VCC: Power pin
NC: No internal connection
```

## Functional description

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These are five
equivalent exclusive-OR symbols valid for an ’LV86A gate in positive
logic; negation can be shown at any two ports. Conversion note: the five
symbols of Figure 4 are not drawn here.

The output is active (low) if all inputs stand at the same logic level
(that is, A = B). Conversion note: this is Figure 5, the logic-identity
element (symbol =).

The output is active (low) if an even number of inputs (that is, 0 or 2)
are active. Conversion note: this is Figure 6, the even-parity element
(symbol 2k).

The output is active (high) if an odd number of inputs (that is, only 1
of the 2) are active. Conversion note: this is Figure 7, the odd-parity
element (symbol 2k + 1).

Wide operating voltage range, operates from 2 to 5.5 V. Allows down
voltage translation, inputs accept voltages to 5.5 V.

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

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Voltage applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | -25 | 25 | mA |
| | Continuous current through V~CC~ or GND | | -50 | 50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5-V maximum.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | D, 14 pins | DB, 14 pins | DGV, 14 pins | NS, 14 pins | PW, 14 pins | Unit |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance [2] | 90.6 | 107.1 | 129.0 | 90.7 | 122.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 50.9 | 59.6 | 52.1 | 48.3 | 51.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 44.8 | 54.4 | 62.0 | 49.4 | 64.4 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 14.7 | 20.5 | 6.5 | 14.6 | 6.7 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 44.5 | 53.8 | 61.3 | 49.1 | 63.8 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
[2] The package thermal impedance is calculated in accordance with JESD 51-7.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | SN54LV86A | < | < | SN74LV86A, –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = -50 µA | 2 to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | I~OH~ = -2 mA | 2.3 V | 2 | | | 2 | | | V |
| ^ | I~OH~ = -6 mA | 3 V | 2.48 | | | 2.48 | | | V |
| ^ | I~OH~ = -12 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | I~OL~ = 50 µA | 2 to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | | 0.4 | V |
| ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | | | 0.44 | V |
| ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | | | 20 | µA |
| I~off~ | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | | | 5 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 1.4 | | | 1.4 | | pF |
```

```table id=noise caption="Noise characteristics for SN74LV86A" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.1 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 8.4 | pF |
| ^ | ^ | ^ | 5 V | 8.8 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 7.9 [1] | 17.6 [1] | 1 [2] | 21 [2] | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 10.5 | 22.6 | 1 | 26.5 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
[2] This note applies to SN54LV86A only: On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 5.5 [1] | 11 [1] | 1 [2] | 13 [2] | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 7.4 | 14.5 | 1 | 16.5 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
[2] This note applies to SN54LV86A only: On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 3.7 [1] | 6.8 [1] | 1 [2] | 8 [2] | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 5.3 | 8.8 | 1 | 10 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
[2] This note applies to SN54LV86A only: On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Typical characteristics

```curve id=tpd-temp caption="t~pd~ vs temperature at 3.3 V"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "tpd", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "tpd", "points": [[-55, 4.9], [-40, 4.95], [25, 5.6], [85, 6.0], [125, 6.25]]}
 ]}
```

```curve id=tpd-vcc caption="t~pd~ vs V~CC~ at 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "tpd", "unit": "ns", "range": [0, 9]},
 "series": [
  {"label": "tpd", "points": [[2.5, 7.9], [3.3, 5.6], [5, 3.9]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices.

Conversion note: C~L~ is 15 pF or 50 pF as given in the
[switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

Conversion note: Figure 3 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse
duration, setup and hold times, and enable and disable times.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

The SN74LV86A is a low drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The inputs can accept voltages to 5.5 V at
any valid V~CC~ making it Ideal for down translation.

Conversion note: in the typical application schematic (Figure 8) a 5-V
bus driver drives the two inputs of one exclusive-OR gate, whose output
drives a 5-V accessory; the gate is supplied from 5-V regulated, with a
0.1 µF capacitor to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing.

### Detailed design procedure

1. Recommended Input conditions
   - Rise time and fall time specs see (Δt/ΔV) in
     [Recommended Operating Conditions](#recommended-operating-conditions).
   - Specified High and low levels. See (V~IH~ and V~IL~) in
     [Recommended Operating Conditions](#recommended-operating-conditions).
2. Recommend output conditions
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part
   - Outputs should not be pulled above V~CC~

Conversion note: the application curve (Figure 9, Switching
Characteristics Comparison) is a picture of the output falling edges of
the LS86, the HC86 and the LV86A over 20 ns.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in
[Recommended Operating Conditions](#recommended-operating-conditions).

Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, TI recommends 0.1 µF and
if there are multiple V~CC~ terminals then .01 or .022 µF is recommended
for each power terminal. It is okay to parallel multiple bypass
capacitors to reject different frequencies of noise. A 0.1 µF and 1 µF
are commonly used in parallel. The bypass capacitor should be installed
as close to the power terminal as possible for best results.

### Layout guidelines

When using multiple bit logic devices inputs should not ever float.

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Specified
below are the rules that must be observed under all circumstances. All
unused inputs of digital logic devices must be connected to a high or
low bias to prevent them from floating. The logic level that should be
applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~ whichever make more
sense or is more convenient. It is generally okay to float outputs
unless the part is a transceiver. If the transceiver has an output
enable pin it will disable the outputs section of the part when
asserted. This will not disable the input section of the IOs so they
also cannot float when disabled.

Conversion note: the layout recommendation (Figure 10) shows one gate
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

## Ordering information

```ordering
SN74LV86AD | | -40 to 125 | LV86A | Obsolete
SN74LV86ADBR | Reel of 2000 | -40 to 125 | LV86A
SN74LV86ADGVR | Reel of 2000 | -40 to 125 | LV86A
SN74LV86ADR | Reel of 2500 | -40 to 125 | LV86A
SN74LV86ANSR | Reel of 2000 | -40 to 125 | 74LV86A | Last Time Buy
SN74LV86APW | | -40 to 125 | LV86A | Obsolete
SN74LV86APWR | Reel of 2000 | -40 to 125 | LV86A
SN74LV86APWRG4 | Reel of 2000 | -40 to 125 | LV86A
SN74LV86APWT | | -40 to 125 | LV86A | Obsolete
```

## Conversion notes

- Source pages 1–11 converted; the support page (12), the package option
  addendum (pp.13–14), tape and reel and mechanical pages (15–28)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists (two feature lines are in
  Description), device information table, revision history, the
  exclusive-OR symbol drawings (p.1 and Figures 4–7), the vendor pinout
  drawings, the typical application schematic (Figure 8), the
  application curve (Figure 9) and the layout recommendation
  (Figure 10), each described in one sentence.
- The Overview (p.9) repeats the Description (p.1) and is given once,
  under Description.
- The data sheet also covers the SN54LV86A (J, W and FK packages), which
  the catalog does not carry; its columns and notes are kept in the
  tables.
- The Pin Functions table (p.3) prints the pin number under PIN and the
  pin name under I/O. The NC line is note B of the FK package drawing.
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column. Its SN54LV86A column group has no
  temperature range in the source.
- The switching tables' second Min/Max group has no heading in the
  source.
- C~i~ typical values sit across the Min and Typ columns in the source;
  they are placed under Typ.
- Curves digitized from Figures 1 and 2 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{A}B + A!{B} (p.1 and p.9).
- Source issues:
  - Source conflict: the recommended operating conditions (p.5) give
    T~A~ from –55 °C; the electrical characteristics (p.6) head the
    SN74LV86A columns –40°C to 125°C and the package option addendum
    (p.13) lists the SN74LV86A parts from –40 to 125. Both kept.
  - Source conflict: Application Information (p.10) calls the part a
    "low drive CMOS device" with "slow edge rates", while Design
    Requirements (p.10) says "The high drive will also create fast
    edges". Both kept.
