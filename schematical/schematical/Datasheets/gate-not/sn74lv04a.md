---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv04a",
  "title": "SN74LV04A hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lv04a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS388K",
    "date": "2014-12",
    "pages": 29
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74LV04AD", "SN74LV04ADBR", "SN74LV04ADGVR", "SN74LV04ADGVRE4", "SN74LV04ADR", "SN74LV04ANSR", "SN74LV04APWR", "SN74LV04ARGYR"],
  "packages": {"p14": "D, DB, DGV, NS, PW, RGY"},
  "converted": "2026-11-21"
}
---

# SN74LV04A hex inverters

## Description

This hex inverter is designed for 2-V to 5.5-V V~CC~ operation. The
SN74LV04A device contains six independent inverters. This device perform
the Boolean function Y = !{A}.

Latch-Up Performance Exceeds 250 mA Per JESD 17. Support Mixed-Mode
Voltage Operation on All Ports. I~off~ Supports Partial-Power-Down Mode
Operation.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1Y: 1Y Output
2A: 2A Input
2Y: 2Y Output
3A: 3A Input
3Y: 3Y Output
4Y: 4Y Output
4A: 4A Input
5Y: 5Y Output
5A: 5A Input
6Y: 6Y Output
6A: 6A Input
GND: Ground Pin
VCC: Power Pin
```

## Functional description

These hex inverters are designed for 2-V to 5.5-V V~CC~ operation. The
SN74LV04A devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}.

These devices are fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the devices when they are powered
down.

The inputs are high impedance when V~CC~ = 0V.

Conversion note: the functional block diagram (Figure 4, Logic Diagram Each Inverter (Positive Logic)) shows one inverter with input A and output Y.

- Wide operating voltage range
  - Operates from 2 V to 5.5 V
- Allows down-voltage translation
  - Inputs accept voltages to 5.5 V
- I~off~ feature
  - Supports Live Insertion, Partial Power DownMode, and Back Drive
    Protection

## Function tables

```table id=function caption="Table 1. Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
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
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5-V maximum.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | 1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | ^ |
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
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LV04A | < | < | < | < | < | Unit |
| ^ | ^ | D | DB | DGV | NS | PW | RGY | ^ |
| ^ | ^ | 14 pins | < | < | < | < | < | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 94.9 | 107.4 | 130.4 | 91.4 | 122.6 | 57.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 56.3 | 59.9 | 53.4 | 49.0 | 51.3 | 70.4 | ^ |
| R~θJB~ | Junction-to-board thermal resistance | 49.2 | 54.7 | 63.5 | 50.2 | 64.4 | 33.6 | ^ |
| ψ~JT~ | Junction-to-top characterization parameter | 20.7 | 21.0 | 7.3 | 15.3 | 6.8 | 3.5 [2] | ^ |
| ψ~JB~ | Junction-to-board characterization parameter | 48.9 | 51.2 | 62.8 | 49.8 | 63.8 | 33.7 | ^ |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | 14.1 | ^ |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
[2] Unconfirmed: the source prints this value as ".3.5", with a point before and after the 3; written here as 3.5.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V~CC~ - 0.1 | | V |
| ^ | I~OH~ = –2 mA | 2.3 V | 2 | | | 2 | | 2 | | ^ |
| ^ | I~OH~ = –6 mA | 3 V | 2.48 | | | 2.48 | | 2.48 | | ^ |
| ^ | I~OH~ = –12 mA | 4.5 V | 3.8 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | 0.4 | | 0.4 | ^ |
| ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | | 0.44 | | 0.44 | ^ |
| ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | | 0.55 | | 0.55 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | | 20 | | 20 | µA |
| I~off~ | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | | 5 | | 5 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 2.3 | | | | | | pF |
| ^ | ^ | 5 V | | 2.3 | | | | | | ^ |
```

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | SN74LV04A | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.3 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.1 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 9.6 | pF |
| ^ | ^ | ^ | 5 V | 11.4 | ^ |
```

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 7.1 [1] | 11.7 [1] | 1 | 14 | 1 | 15 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 10 | 15.5 | 1 | 18 | 1 | 19 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 5.1 [1] | 7.1 [1] | 1 | 8.5 | 1 | 9.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 7.3 | 10.6 | 1 | 12 | 1 | 13 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 3.6 [1] | 5.5 [1] | 1 | 6.5 | 1 | 7.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 5.1 | 7.5 | 1 | 8.5 | 1 | 9.5 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Typical characteristics

Conversion note: Figure 1 (TPD vs Temperature at 5 V) plots TPD in ns against temperature, rising from about 3 to about 4.5.

Conversion note: Figure 2 (TPD vs V~CC~ at 25°C) plots TPD in ns against V~CC~, falling from about 6 at 2 V to about 3 at 5.5 V.

## Parameter measurement information

```figure ref=load-cmos-50pf caption="Figure 3. Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Figure 3. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices.

Conversion note: Figure 3 also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse duration, setup and hold times, and enable and disable times, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

SN74LV04A is a low-drive CMOS device that can be used for a multitude of
bus interface type applications where output ringing is a concern. The
low drive and slow edge rates will minimize overshoot and undershoot on
the outputs. The inputs can accept voltages to 5.5 V at any valid V~CC~
making it Ideal for down translation.

Conversion note: in the typical application schematic (Figure 5) a block supplied from 5-V regulated, with a 0.1 µF capacitor to ground, drives one inverter, whose output drives a block labelled 3.3-V or 5-V accessory.

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

Conversion note: the typical application curve (Figure 6) is a picture of the output falling edges of the AC04, the HC04 and the LV04A over 20 ns, at V~CC~ = 5 V with a load of 50 Ω/50 pF.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended. If
there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states.

Specified in Figure 7 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or
V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver. If the
transceiver has an output enable pin, it will disable the outputs
section of the part when asserted. This will not disable the input
section of the I/Os so they also cannot float when disabled.

Conversion note: the layout diagram (Figure 7) shows one gate with its unused input tied to V~CC~ and one with its unused input tied to ground.

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
SN74LV04AD | | -40 to 125 | LV04A | Obsolete
SN74LV04ADBR | Reel of 2000 | -40 to 125 | LV04A | Last Time Buy
SN74LV04ADGVR | Reel of 2000 | -40 to 125 | LV04A
SN74LV04ADGVRE4 | Reel of 2000 | -40 to 125 | LV04A
SN74LV04ADR | Reel of 2500 | -40 to 125 | LV04A
SN74LV04ANSR | Reel of 2000 | -40 to 125 | 74LV04A
SN74LV04APWR | Reel of 2000 | -40 to 125 | LV04A
SN74LV04ARGYR | Reel of 3000 | -40 to 125 | LV04A
```

## Conversion notes

- Source pages 1–12 converted (of page 12 only the layout text and
  Figure 7 are kept); the package option addendum (pp.13–14), tape and
  reel and mechanical pages and the notice (15–29) dropped, replaced by
  the generated Package and Ordering sections. The ordering lines are
  the parts' rows of the package option addendum (p.13).
- Dropped: features and applications lists (three feature lines are in
  Description), device information table, table of contents, revision
  history, the device and documentation support section, the
  simplified schematic (p.1) and the vendor's pin drawings (p.3). The
  functional block diagram (Figure 4), the typical characteristics
  (Figures 1 and 2, p.7; not digitized), the typical application
  schematic (Figure 5), the application curve (Figure 6) and the layout
  diagram (Figure 7) are each described in one sentence.
- The electrical characteristics table of the source gives symbols
  only, so it has no Parameter column. Dashes in the thermal table (no
  value) are empty cells.
- Of Figure 3 (p.8) the load circuit for totem-pole outputs and the
  propagation delay waveforms are kept, with notes A, C, D, G and H.
- The source's Power Supply Recommendations and Layout chapters (11 and
  12) are filed under Application information.
- The package option addendum also lists SN74LV04ADRG4, SN74LV04APWRG4
  and SN74LV04APWT, which this file doesn't claim.
- From images: overbars of Y = !{A} (p.1 and p.9); the ends of the
  curves of Figures 1 and 2, about 3, 4.5, 6 and 3 (p.7); the labels
  AC04, HC04 and LV04A of Figure 6 (p.11).
- Source issues:
  - Unconfirmed: ψ~JT~ of the RGY package is printed ".3.5" (p.5); the
    table has 3.5.
  - Source error: the description (p.1) prints "This device perform";
    kept as printed.
  - Source error: the feature description (p.9) prints "DownMode" as
    one word; kept as printed.
  - Source conflict: Application Information (p.10) calls the part a
    "low-drive CMOS device" with "slow edge rates", while Design
    Requirements (p.10) says "The high drive will also create fast
    edges". Both kept.
