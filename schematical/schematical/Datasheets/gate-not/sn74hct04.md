---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hct04",
  "title": "SNx4HCT04 hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74hct04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS042F",
    "date": "2022-10",
    "pages": 29
  },
  "devices": ["gate:74_04"],
  "parts": ["5962-89747012A", "SN74HCT04D", "SN74HCT04DR", "SN74HCT04DT", "SN74HCT04N", "SN74HCT04NSR", "SN74HCT04PW", "SN74HCT04PWR", "SN74HCT04PWT"],
  "packages": {"p14": "J, D, N, NS, PW", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4HCT04 hex inverters

## Description

These devices contain six independent inverters. They perform the
Boolean function Y = !{A} in positive logic. [[7::1]]

Operating voltage range of 4.5V to 5.5V. Outputs can drive up to 10
LSTTL loads. Inputs are TTL-Voltage compatible. [[1::2]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y
GND: Ground
VCC: Positive supply
```
[[4::table #1]]

## Functional description

These devices contain six independent inverters. They perform the
Boolean function Y = !{A} in positive logic. [[7::1]]

Conversion note: the functional block diagram (p.1, p.7) shows a buffer, an inverter and a buffer in series between input xA and output xY.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[7::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | (V~I~ < 0 or V~I~ > V~CC~) | | ±20 | mA |
| I~OK~ | Output clamp current [2] | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| V~CC~ or GND | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | SN54HCT04 | < | < | SN74HCT04 | < | < | Unit |
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
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric | D (SOIC) | N (PDIP) | NS (SO) | PW (TSSOP) | Unit |
| ^ | ^ | 14 pins | 14 pins | 14 pins | 14 pins | ^ |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance [1] | 138.7 | 62.7 | 90.9 | 117.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 93.8 | 50.5 | 48.5 | 46.9 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 42.5 | 51.5 | 60.6 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 49.1 | 30.1 | 15.9 | 5.6 | °C/W |
| ψ~JB~ | Junction-to-case (bottom) thermal resistance [2] | 94.3 | 42.2 | 51 | 60 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
[2] Unconfirmed: the source names the ψ~JB~ row "Junction-to-case (bottom) thermal resistance", the same name as the R~θJC(bot)~ row (p.4); kept as printed.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics"
| Symbol | Parameter | Test conditions [1] | V~CC~ (V) | T~A~ = 25°C | < | < | SN54HCT04 | < | SN74HCT04 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = – 20 μA | 4.5 | 4.4 | 4.499 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = – 4 mA | ^ | 3.98 | 4.3 | | 3.7 | | 3.84 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 20 μA | 5.5 | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | ^ | | 0.17 | 0.26 | | 0.4 | | 0.33 | V |
| I~I~ | Input hold current | V~I~ = V~CC~ or 0 | 5.5 | | ±0.1 | ±100 | | ±1000 | | ±1000 | nA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0. I~O~ = 0 | 5.5 | | | 2 | | 40 | | 20 | μA |
| ΔI~CC~ [2] | Supply-current change | One input at 0.5V or 2.4 V, Other inputs at 0 or V~CC~ | 5.5 | | 1.4 | 2.4 | | 3 | | 2.9 | mA |
| C~i~ | Input capacitance | | 4.5 to 5.5 | | 3 | 10 | | 10 | | 10 | pF |

[1] V~I~ = V~IH~ or V~IL~, unless otherwise noted.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #1]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load | 20 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF. See Parameter Measurement Information"
| Symbol | Parameter | From (input) | To (output) | V~CC~ (V) | T~A~ = 25°C | < | < | SN54HCT04 | < | SN74HCT04 | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B [1] | Y | 4.5 | | 14 | 20 | | 30 | | 25 | ns |
| ^ | ^ | ^ | ^ | 5.5 | | 13 | 18 | | 27 | | 23 | ns |
| t~t~ | Transition time | | Y | 4.5 | | 9 | 15 | | 22 | | 19 | ns |
| ^ | ^ | ^ | ^ | 5.5 | | 8 | 14 | | 20 | | 17 | ns |

[1] Source conflict: the switching characteristics (p.5) print the input as "A or B"; the pin drawings (p.3) and the function table (p.7) give each inverter the one input A.
```
[[5::table #3]]

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns. [[6::1]]

For clock inputs, f~max~ is measured when the input duty cycle is 50%. [[6::2]]

The outputs are measured one at a time with one input transition per
measurement. [[6::3]]

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. [[6::4]] [[6::6]]

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```
[[6::Figure 6-1]]

Conversion note: in the source's voltage waveforms (Figure 6-2, p.6) the input swings 0 V to 3V and is measured at 1.3V, and the outputs are measured at 50%.

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delays for TTL-compatible inputs"
VM: 1.3V (input), 50% (output)
```
[[6::Figure 6-2]]

## Application information

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1-μF capacitor is recommended for this device.
It is acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results. [[8::1]]

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
or is more convenient. [[8::2]]

## Package

```package footprint=LCCC-20
```

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
5962-89747012A | Tube of 55 | -55 to 125 | 5962-89747012A SNJ54HCT04FK
SN74HCT04D | | -40 to 85 | HCT04 | Obsolete
SN74HCT04DR | Reel of 2500 | -40 to 85 | HCT04
SN74HCT04DT | | -40 to 85 | HCT04 | Obsolete
SN74HCT04N | Tube of 25 | -40 to 85 | SN74HCT04N | NRND
SN74HCT04NSR | Reel of 2000 | -40 to 85 | HCT04
SN74HCT04PW | | -40 to 85 | HT04 | Obsolete
SN74HCT04PWR | Reel of 2000 | -40 to 85 | HT04
SN74HCT04PWT | | -40 to 85 | HT04 | Obsolete
```
[[10::table #1]] [[11::table #1]]

## Conversion notes

- Source pages 1–8 converted; the device and documentation support
  page (9), the package option addendum (p.10–12), tape and reel, tube
  and mechanical pages (13–29) dropped, replaced by the generated
  Package and Ordering sections. The ordering lines are from the
  package option addendum.
- Dropped: features list, the device information table, table of
  contents and revision history (p.2), the vendor's pin drawings (p.3).
  The functional block diagram is described in text.
- The source has no pin functions table; the pin descriptions are the
  converter's.
- The source's Power Supply Recommendations and Layout chapters (8 and
  9) are filed under Application information; the source has no
  Application and Implementation chapter.
- The source also covers the J (CDIP) package, and its addendum the W
  (CFP) package, of the SN54HCT04, and further orderables
  (SN74HCT04DRE4, SN74HCT04DRG4, SN74HCT04NE4, SNJ54HCT04FK,
  SNJ54HCT04J, SN54HCT04J and the other 5962- and JM38510 numbers),
  which the catalog doesn't carry.
- The source's load circuit (Figure 6-1, p.6) prints no value for
  C~L~; the switching characteristics give C~L~ = 50 pF.
- From images: overbars of Y = !{A} (p.1, p.7).
- Source issues:
  - Source conflict: the switching characteristics (p.5) print the
    input as "A or B"; the pin drawings and the function table give
    each inverter the one input A (switching table note 1).
  - Unconfirmed: the thermal information (p.4) names the ψ~JB~ row
    "Junction-to-case (bottom) thermal resistance" (thermal table
    note 2).
  - Unconfirmed: the electrical characteristics (p.5) give the V~OL~
    rows at V~CC~ = 5.5 V and the V~OH~ rows at 4.5 V; kept as printed.
  - Unconfirmed: the recommended operating conditions (p.4) give the
    symbol Δt/Δv for "Input transition rise/fall time" in ns; kept as
    printed.
  - Unconfirmed: note 1 of the recommended operating conditions (p.4)
    names the application report "Implications of Slow or Floating SMOS
    Inputs"; kept as printed.
