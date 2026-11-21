---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hct00",
  "title": "SNx4HCT00 quadruple 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74hct00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS062F",
    "date": "2022-10",
    "pages": 26
  },
  "devices": ["gate:74_00"],
  "parts": ["SN74HCT00D", "SN74HCT00DBR", "SN74HCT00DR", "SN74HCT00DRE4", "SN74HCT00DT", "SN74HCT00N", "SN74HCT00NSR", "SN74HCT00PW", "SN74HCT00PWR", "SN74HCT00PWRG4"],
  "packages": {"p14": "D, DB, N, NS, PW"},
  "converted": "2026-11-21"
}
---

# SNx4HCT00 quadruple 2-input positive-NAND gates

## Description

These devices contain four independent 2-input NAND gates. They perform
the Boolean function Y = !{A • B} in positive logic.

Operating voltage range of 4.5V to 5.5V. Outputs can drive up to 10
LSTTL loads. Inputs are TTL-voltage compatible.

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

These devices contain four independent 2-input NAND gates. They perform
the Boolean function Y = !{A • B} in positive logic.

Conversion note: the functional block diagram shows one NAND gate with
buffered inputs xA and xB and buffered output xY.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| I~IK~ | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) [2] | | ±20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) [2] | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | SN54HCT00 [2] | < | < | SN74HCT00 | < | < | Unit |
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
[2] SN54HCT00 is in product preview.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric | D (SOIC), 14 pins | DB (SSOP), 14 pins | N (PDIP), 14 pins | NS (SO), 14 pins | PW (TSSOP), 14 pins | Unit |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance [1] | 138.7 | 117.9 | 69.5 | 95.3 | 122.8 | °C/W |
| R~θJC (top)~ | Junction-to-case (top) thermal resistance | 93.8 | 63.1 | 57.6 | 52.9 | 52.1 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 66.9 | 49.3 | 55.9 | 65.8 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 49.1 | 22.2 | 37.6 | 19.5 | 7.8 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.3 | 66.2 | 49.1 | 55.4 | 65.3 | °C/W |
| R~θJC (bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics"
| Symbol | Parameter | Test conditions [1] | V~CC~ (V) | T~A~ = 25°C | < | < | SN54HCT00 [3] | < | SN74HCT00 | < | Unit |
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
[3] SN54HCT00 is in product preview.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load | 20 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF. See Parameter Measurement Information"
| Symbol | Parameter | From (input) | To (output) | V~CC~ (V) | T~A~ = 25°C | < | < | SN54HCT00 [1] | < | SN74HCT00 | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | 4.5 | | 11 | 20 | | 30 | | 25 | ns |
| ^ | ^ | ^ | ^ | 5.5 | | 10 | 18 | | 27 | | 22 | ns |
| t~t~ | Transition time | | Y | 4.5 | | 9 | 15 | | 22 | | 19 | ns |
| ^ | ^ | ^ | ^ | 5.5 | | 8 | 14 | | 20 | | 17 | ns |

[1] SN54HCT00 is in product preview.
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

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCT00D | | -40 to 85 | HCT00 | Obsolete
SN74HCT00DBR | Reel of 2000 | -40 to 85 | HT00 | NRND
SN74HCT00DR | Reel of 2500 | -40 to 85 | HCT00
SN74HCT00DRE4 | Reel of 2500 | -40 to 85 | HCT00
SN74HCT00DT | | -40 to 85 | HCT00 | Obsolete
SN74HCT00N | Tube of 25 | -40 to 85 | SN74HCT00N | NRND
SN74HCT00NSR | Reel of 2000 | -40 to 85 | HCT00 | Last Time Buy
SN74HCT00PW | | -40 to 85 | HT00 | Obsolete
SN74HCT00PWR | Reel of 2000 | -40 to 85 | HT00
SN74HCT00PWRG4 | 2000 | -40 to 85 | HT00
```

## Conversion notes

- Source pages 1–8 converted; the device and documentation support,
  package option addendum, tape and reel and mechanical pages (9–26)
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
  LCCC) of the SN54HCT00, which is in product preview; the catalog
  carries no SN54HCT00 part.
- The source's load circuit (Figure 6-1) prints no value for C~L~; the
  switching characteristics give C~L~ = 50 pF.
- From images: overbars of Y = !{A • B} (p.1).
- Source issues:
  - Unconfirmed: the electrical characteristics (p.5) give the V~OL~
    rows at V~CC~ = 5.5 V and the V~OH~ rows at 4.5 V; kept as printed.
  - Unconfirmed: the recommended operating conditions (p.4) give the
    symbol Δt/Δv for "Input transition rise/fall time" in ns; kept as
    printed.
  - Unconfirmed: note 1 of the recommended operating conditions (p.4)
    names the application report "Implications of Slow or Floating SMOS
    Inputs"; kept as printed.
