---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hct02",
  "title": "SNx4HCT02 quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74hct02.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS065G",
    "date": "2022-10",
    "pages": 23
  },
  "devices": ["gate:74_02"],
  "parts": ["SN74HCT02D", "SN74HCT02DR", "SN74HCT02DRG4", "SN74HCT02DT", "SN74HCT02N", "SN74HCT02NE4", "SN74HCT02NSR", "SN74HCT02PW", "SN74HCT02PWR", "SN74HCT02PWRG4", "SN74HCT02PWT"],
  "packages": {"p14": "D, N, NS, PW"},
  "converted": "2026-11-21"
}
---

# SNx4HCT02 quadruple 2-input positive-NOR gates

## Description

These devices contain four independent 2-input NOR gates. They perform
the Boolean function Y = !{A} • !{B} or Y = !{A + B} in positive logic.

Operating voltage range of 4.5V to 5.5V. Outputs can drive up to 10
LSTTL loads. Inputs are TTL-Voltage compatible.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Gate input A
1B, 2B, 3B, 4B: Gate input B
1Y, 2Y, 3Y, 4Y: Gate output Y
GND: Ground
VCC: Positive supply
```

## Functional description

These devices contain four independent 2-input NOR gates. They perform
the Boolean function Y = !{A} • !{B} or Y = !{A + B} in positive logic.

Conversion note: the functional block diagram shows one NOR gate with
inputs A and B and output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| I~IK~ | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) [2] | | ±20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) [2] | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or ground current | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | SN54HCT02 [2] | < | < | SN74HCT02 | < | < | Unit |
| ^ | ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 4.5 V to 5.5 V | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 4.5 V to 5.5 V | | | 0.8 | | | 0.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise/fall time | | | | 500 | | | 500 | ns |
| T~A~ | Operating free-air temperature | | -55 | | 125 | -40 | | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report Implications of Slow or Floating SMOS Inputs, literature number SCBA004.
[2] SN54HCT02 is in product preview.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric | D (SOIC), 14 pins | N (PDIP), 14 pins | NS (SO), 14 pins | PW (TSSOP), 14 pins | Unit |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance [1] | 138.7 | 67 | 93.3 | 120.1 | °C/W |
| R~θJC (top)~ | Junction-to-case (top) thermal resistance | 93.8 | 55 | 50.9 | 49.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 46.7 | 53.8 | 63.1 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 49.1 | 35.1 | 17.8 | 6.7 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.3 | 46.5 | 53.3 | 62.5 | °C/W |
| R~θJC (bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics"
| Symbol | Parameter | Test conditions [1] | V~CC~ (V) | T~A~ = 25°C | < | < | SN54HCT00 [3] [4] | < | SN74HCT00 [4] | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -20 μA | 4.5 | 4.4 | 4.499 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | ^ | 3.98 | 4.3 | | 3.7 | | 3.84 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 20 μA | 5.5 | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | ^ | | 0.17 | 0.26 | | 0.4 | | 0.33 | V |
| I~I~ | Input hold current | V~I~ = V~CC~ or 0 | 5.5 | | ±0.1 | ±100 | | ±1000 | | ±1000 | nA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0. I~O~ = 0 | 5.5 | | | 2 | | 40 | | 20 | μA |
| ΔI~CC~ [2] | Supply-current change | One input at 0.5V or 2.4 V, Other inputs at 0 or V~CC~ | 5.5 | | 1.4 | 2.4 | | 3 | | 2.9 | mA |
| C~i~ | Input capacitance | | 4.5 to 5.5 | | 3 | 10 | | 10 | | 10 | pF |

[1] V~I~ = V~IH~ or V~IL~, unless otherwise noted.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
[3] SN54HCT02 is in product preview.
[4] Source error: the table heads these column groups "SN54HCT00" and "SN74HCT00"; this is the SNx4HCT02 data sheet (title, every page header and note 3 of this table), and the columns are those of the SN54HCT02 and SN74HCT02.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load | 20 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF. See Parameter Measurement Information"
| Symbol | Parameter | From (input) | To (output) | V~CC~ (V) | T~A~ = 25°C | < | < | SN54HCT02 [1] | < | SN74HCT00 [2] | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | 4.5 | | 11 | 20 | | 30 | | 25 | ns |
| ^ | ^ | ^ | ^ | 5.5 | | 10 | 18 | | 27 | | 22 | ns |
| t~t~ | Transition time | | Y | 4.5 | | 9 | 15 | | 22 | | 19 | ns |
| ^ | ^ | ^ | ^ | 5.5 | | 8 | 14 | | 20 | | 17 | ns |

[1] SN54HCT02 is in product preview.
[2] Source error: the table heads this column group "SN74HCT00"; this is the SNx4HCT02 data sheet (title and every page header), and the columns are those of the SN74HCT02.
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns.

For clock inputs, f~max~ is measured when the input duty cycle is 50%.

The outputs are measured one at a time with one input transition per
measurement.

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```

Conversion note: in the source's voltage waveforms the input swings
0V to 3V and is measured at 1.3V; the outputs are measured at 50%.

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delays for TTL-compatible inputs"
VM: 1.3V (input), 50% (output)
```

## Application information

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1-μF capacitor is recommended for this device.
It is acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results.

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
or is more convenient.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCT02D | | -40 to 85 | HCT02 | Obsolete
SN74HCT02DR | Reel of 2500 | -40 to 85 | HCT02
SN74HCT02DRG4 | Reel of 2500 | -40 to 85 | HCT02
SN74HCT02DT | | -40 to 85 | HCT02 | Obsolete
SN74HCT02N | Tube of 25 | -40 to 85 | SN74HCT02N | NRND
SN74HCT02NE4 | Tube of 25 | -40 to 85 | SN74HCT02N | NRND
SN74HCT02NSR | Reel of 2000 | -40 to 85 | HCT02
SN74HCT02PW | | -40 to 85 | HT02 | Obsolete
SN74HCT02PWR | Reel of 2000 | -40 to 85 | HT02
SN74HCT02PWRG4 | Reel of 2000 | -40 to 85 | HT02
SN74HCT02PWT | | -40 to 85 | HT02 | Obsolete
```

## Conversion notes

- Source pages 1–8 converted; the device and documentation support,
  package option addendum, tape and reel and mechanical pages (9–23)
  dropped, replaced by the generated Package and Ordering sections
  (packing, temperature range and marking taken from the package option
  addendum, p.10).
- Dropped: features list, the device information table, table of
  contents and revision history (p.2), the vendor's pin drawings (p.3).
  The functional block diagram is described in text.
- The source has no pin functions table; the pin descriptions are the
  converter's.
- The source's Power Supply Recommendations and Layout chapters (8 and
  9) are filed under Application information; the source has no
  Application and Implementation chapter.
- The source also draws the J and W packages and the FK package (20-pin
  LCCC) of the SN54HCT02, which is in product preview; the catalog
  carries no SN54HCT02 part.
- The source's load circuit (Figure 6-1) prints no value for C~L~; the
  switching characteristics give C~L~ = 50 pF.
- From images: overbars of Y = !{A} • !{B} and Y = !{A + B} (p.1).
- Source issues:
  - Source error: the electrical characteristics (p.5) head their
    device column groups "SN54HCT00" and "SN74HCT00", and the
    switching characteristics (p.5) head one "SN74HCT00"; the headers
    are kept as printed, and a table note names the SN54HCT02 and
    SN74HCT02, as the title and the page headers name the devices.
  - Unconfirmed: the electrical characteristics (p.5) give the V~OL~
    rows at V~CC~ = 5.5 V and the V~OH~ rows at 4.5 V; kept as printed.
  - Unconfirmed: the recommended operating conditions (p.4) give the
    symbol Δt/Δv for "Input transition rise/fall time" in ns; kept as
    printed.
  - Unconfirmed: note 1 of the recommended operating conditions (p.4)
    names the application report "Implications of Slow or Floating SMOS
    Inputs"; kept as printed.
