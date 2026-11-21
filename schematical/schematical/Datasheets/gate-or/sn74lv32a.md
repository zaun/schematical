---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv32a",
  "title": "SN74LV32A quadruple 2-input positive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lv32a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS385K",
    "date": "2014-12",
    "pages": 29
  },
  "devices": ["gate:74_32"],
  "parts": ["SN74LV32AD", "SN74LV32ADBR", "SN74LV32ADGVR", "SN74LV32ADR", "SN74LV32ANSR", "SN74LV32APW", "SN74LV32APWR", "SN74LV32APWRG4", "SN74LV32APWT", "SN74LV32ARGYR"],
  "packages": {"p14": "D, DB, DGV, NS, PW, RGY"},
  "converted": "2026-11-21"
}
---

# SN74LV32A quadruple 2-input positive-OR gates

## Description

This quadruple 2-input positive-OR gates is designed for 2-V to 5.5-V
V~CC~ operation.

The SN74LV32A device performs the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic.

Support Mixed-Mode Voltage Operation on All Ports. I~off~ Supports
Partial-Power-Down Mode Operation. Latch-Up Performance Exceeds 250 mA
Per JESD 17.

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
3Y: 3Y Output
3A: 3A Input
3B: 3B Input
4Y: 4Y Output
4A: 4A Input
4B: 4B Input
GND: Ground Pin
VCC: Power Pin
```

## Functional description

This quadruple 2-input positive-OR gate is designed for 2-V to 5.5-V
V~CC~ operation.

The SN74LV32A device performs the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic.

This part has low drive which produces slower rise and fall times that
will reduce ringing on the output signal. The inputs and outputs are of
high impedance when V~CC~ = 0 V.

Conversion note: the logic diagram (Figure 4) shows one 2-input OR gate
with inputs A and B and output Y.

- Wide operating voltage range
  - Operates from 2 V to 5.5 V
- Allows down-voltage translation
  - Inputs accept voltages to 5.5 V
- I~off~ feature
  - Supports Live Insertion, Partial Power DownMode, and Back Drive
    Protection

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous channel current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed. 2. This value is limited to 5.5 V maximum.
[3] This value is limited to 5.5-V maximum.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | 1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 to 2.7 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 to 2.7 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 2.3 to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LV32A, 14 pins | < | < | < | < | < | Unit |
| ^ | ^ | D | DBV [2] | DVG [2] | NS | PW | RGY | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 90.6 | 107.1 | 129.0 | 90.7 | 122.6 | 57.5 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 50.9 | 59.6 | 52.1 | 48.3 | 51.4 | 70.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 44.8 | 54.4 | 62.0 | 49.4 | 64.4 | 33.6 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 14.7 | 20.5 | 6.5 | 14.6 | 6.7 | 3.4 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 44.5 | 53.8 | 61.3 | 49.1 | 63.8 | 33.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | 13.9 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
[2] Source conflict: this table heads these columns DBV and DVG; the pin configuration and the package option addendum give the 14-pin packages DB (SSOP) and DGV (TVSOP).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
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
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 3.3 | | | | | | pF |
| ^ | ^ | 5 V | | 3.3 | | | | | | pF |
```

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | SN74LV32A | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.0 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 9.5 | pF |
| ^ | ^ | ^ | 5 V | 11.5 | pF |
```

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 7.1 [1] | 12.8 [1] | 1 [1] | 15 [1] | 1 | 16 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 9.6 | 16.2 | 1 | 19 | 1 | 20 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 5 [1] | 7.9 [1] | 1 [1] | 9.5 [1] | 1 | 10.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 6.9 | 11.4 | 1 | 13 | 1 | 14 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | C~L~ = 15 pF | | 3.6 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 7.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 4.9 | 7.5 | 1 | 8.5 | 1 | 9.5 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature at 5V"
{"x": {"label": "Temperature", "unit": "°C", "range": [-150, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 4.5]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.93], [-40, 2.97], [25, 3.6], [85, 4.0], [125, 4.25]]}
 ]}
```

```curve id=tpd-vcc caption="TPD vs V~CC~ at 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD", "points": [[2, 6.0], [3, 4.55], [4, 4.05], [4.5, 3.8], [5, 3.6], [5.5, 3.2]]}
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

SN74LV32A is a low-drive CMOS device that can be used for a multitude
of bus interface type applications where output ringing is a concern.
Source error: the source names the SN74LV04A in this sentence; the data
sheet and its page headers are for the SN74LV32A. Corrected. The low
drive and slow edge rates will minimize overshoot and undershoot on the
outputs. The inputs can accept voltages to 5.5 V at any valid V~CC~
making it Ideal for down translation.

Conversion note: in the typical application schematic (Figure 5) one
block drives the two inputs of one OR gate, whose output drives a 3.3-V
or 5-V accessory; the gate is supplied from 5-V regulated, with a
0.1 µF capacitor to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing.

### Detailed design procedure

1. Recommended Input Conditions
   - For rise time and fall time specifications, see Δt/ΔV in the
     Recommended Operating Conditions table.
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~.
2. Recommend Output Conditions
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part.
   - Outputs should not be pulled above V~CC~.

Conversion note: the typical application curve (Figure 6) is a picture
of the input V~I~ and the output falling edges of the AC32, the HC32
and the LV32 over 20 ns, at V~CC~ = 5 V with a load of 50 Ω/50 pF.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps
to reject different frequencies of noise. A 0.1 μF and 1 μF are
commonly used in parallel. The bypass capacitor should be installed as
close to the power pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such
input pins should not be left unconnected because the undefined
voltages at the outside connections result in undefined operational
states.

Specified in Figure 7 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The
logic level that should be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver. If the
transceiver has an output enable pin, it will disable the outputs
section of the part when asserted. This will not disable the input
section of the I/Os so they also cannot float when disabled.

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
SN74LV32AD | | -40 to 125 | LV32A | Obsolete
SN74LV32ADBR | Reel of 2000 | -40 to 125 | LV32A
SN74LV32ADGVR | Reel of 2000 | -40 to 125 | LV32A
SN74LV32ADR | Reel of 2500 | -40 to 125 | LV32A
SN74LV32ANSR | Reel of 2000 | -40 to 125 | 74LV32A
SN74LV32APW | | -40 to 125 | LV32A | Obsolete
SN74LV32APWR | Reel of 2000 | -40 to 125 | LV32A
SN74LV32APWRG4 | Reel of 2000 | -40 to 125 | LV32A
SN74LV32APWT | | -40 to 125 | LV32A | Obsolete
SN74LV32ARGYR | Reel of 3000 | -40 to 125 | LV32A
```

## Conversion notes

- Source pages 1–12 converted (of page 12 the layout section only); the
  package option addendum (pp.13–14), tape and reel and mechanical
  pages (15–29) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists (three feature lines are in
  Description), device information table, revision history, the device
  and documentation support section, the simplified schematic (p.1),
  the vendor pinout drawings, the logic diagram (Figure 4), the typical
  application schematic (Figure 5), the application curve (Figure 6)
  and the layout diagram (Figure 7), the last four each described in
  one sentence.
- The electrical characteristics table of the source gives symbols
  only, so it has no Parameter column.
- Dashes in the thermal table (no value) are empty cells.
- Curves digitized from Figures 1 and 2 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{!{A} • !{B}} (p.1 and p.9); the curve
  points of Figures 1 and 2 (p.7); the labels of Figure 6 (p.11).
- Source issues:
  - Source error: Application Information (p.10) names the part
    "SN74LV04A"; corrected to SN74LV32A, the part of this data sheet.
  - Source error: note 2 of the absolute maximum ratings (p.4) ends
    with a stray "2. This value is limited to 5.5 V maximum.", which
    note 3 repeats; kept as printed.
  - Source error: the Description (p.1) prints "This quadruple 2-input
    positive-OR gates is designed"; kept as printed (the Overview, p.9,
    has "gate").
  - Source conflict: the thermal table (p.5) heads two columns DBV and
    DVG; the pin configuration (p.3) and the package option addendum
    (p.13) give DB and DGV. Kept as printed.
  - Source conflict: the Overview (p.9) and Application Information
    (p.10) give the part "low drive" and "slow edge rates", while
    Design Requirements (p.10) says "The high drive will also create
    fast edges". Both kept.
