---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hct08",
  "title": "SN74HCT08 quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74hct08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS063G",
    "date": "2024-08",
    "pages": 27
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74HCT08D", "SN74HCT08DBR", "SN74HCT08DR", "SN74HCT08DRE4", "SN74HCT08DRG4", "SN74HCT08N", "SN74HCT08NE4", "SN74HCT08NSR", "SN74HCT08PW", "SN74HCT08PWR", "SN74HCT08PWRG4", "SN74HCT08PWT"],
  "packages": {"p14": "D, DB, N, NS, PW"},
  "converted": "2026-11-08"
}
---

# SN74HCT08 quadruple 2-input positive-AND gates

## Description

These devices contain four independent 2-input AND gates. They perform
the Boolean function Y = A · B in positive logic.

Operating voltage range of 4.5V to 5.5V. Outputs can drive up to 10
LSTTL loads. Inputs are TTL-voltage compatible.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel input A
1B, 2B, 3B, 4B: Channel input B
1Y, 2Y, 3Y, 4Y: Channel output Y
GND: Ground
VCC: Positive supply
```

## Function tables

```table id=function caption="Function table (each gate) [1]"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don't Care
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions (SN74HCT08) [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 4.5 V to 5.5 V | 2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 4.5 V to 5.5 V | | | 0.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise/fall time | | | | 500 | ns |
| T~A~ | Operating free-air temperature | | -40 | | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report Implications of Slow or Floating SMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Parameter | D (SOIC), 14 pins | DB (SSOP), 14 pins | N (PDIP), 14 pins | NS (SO), 14 pins | PW (TSSOP), 14 pins | Unit |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance [1] | 138.7 | 114.8 | 103.8 | 129.3 | 157.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 93.8 | 60 | 91.6 | 85.7 | 84.1 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 63.8 | 83.5 | 89.9 | 100.8 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 49.1 | 19.7 | 71.1 | 48.2 | 27.5 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.3 | 63.1 | 83.4 | 89.4 | 100.2 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics"
| Symbol | Parameter | Test conditions [1] | V~CC~ (V) | T~A~ = 25 °C | < | < | SN74HCT08 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -20 µA | 4.5 | 4.4 | 4.499 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 4.5 | 3.98 | 4.3 | | 3.84 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 20 µA | 4.5 | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 4.5 | | 0.17 | 0.26 | | 0.33 | V |
| I~I~ | Input hold current | V~I~ = V~CC~ or 0 | 5.5 | | ±0.1 | ±100 | | ±1000 | nA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0, I~O~ = 0 | 5.5 | | | 2 | | 20 | µA |
| ΔI~CC~ [2] | Supply-current change | One input at 0.5 V or 2.4 V, other inputs at 0 or V~CC~ | 5.5 | | 1.4 | 2.4 | | 2.9 | mA |
| C~i~ | Input capacitance | | 4.5 to 5.5 | | 3 | 10 | | 10 | pF |

[1] V~I~ = V~IH~ or V~IL~, unless otherwise noted.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 20 | pF |
```

## Switching characteristics

C~L~ = 50 pF. See [Parameter Measurement Information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF"
| Symbol | Parameter | From (input) | To (output) | V~CC~ (V) | T~A~ = 25 °C | < | < | SN74HCT08 | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | 4.5 | | 15 | 24 | | 30 | ns |
| ^ | ^ | ^ | ^ | 5.5 | | 13 | 22 | | 27 | ns |
| t~t~ | Transition time | | Y | 4.5 | | 9 | 15 | | 19 | ns |
| ^ | ^ | ^ | ^ | 5.5 | | 8 | 14 | | 17 | ns |
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns. The outputs are measured one at a
time with one input transition per measurement. C~L~ includes probe and
test-fixture capacitance. The greater between t~PLH~ and t~PHL~ is the
same as t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: 50 pF
```

Conversion note: in the source's voltage waveforms the input swings
0 V to 3 V and is measured at 1.3 V; the outputs are measured at 50 %.

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delays for TTL-compatible inputs"
VM: 1.3 V (input), 50 % (output)
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

### Power supply recommendations

The power supply can be any voltage between the minimum and
maximum supply voltage rating located in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-µF and 1-µF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power terminal as possible
for best results.

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs
must not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4
buffer gates are used. Such unused input pins must not be left unconnected because the undefined voltages at the outside
connections result in undefined operational states. All unused inputs of
digital logic devices must be connected to a logic high or logic low
voltage, as defined by the input voltage specifications, to prevent them
from floating. The logic level that must be applied to any particular
unused input depends on the function of the device. Generally, the inputs are tied to GND or V~CC~, whichever
makes more sense for the logic function or is more convenient.

Conversion note: the source's layout example uses a GND flood fill,
places the 0.1 µF bypass capacitor close to the device, ties unused
inputs to V~CC~, leaves the unused output floating and avoids 90°
corners for signal lines.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCT08D | | -40 to 85 | HCT08 | Obsolete
SN74HCT08DBR | Reel of 2000 | -40 to 85 | HT08 | NRND
SN74HCT08DR | Reel of 2500 | -40 to 85 | HCT08
SN74HCT08DRE4 | Reel of 2500 | -40 to 85 | HCT08
SN74HCT08DRG4 | Reel of 2500 | -40 to 85 | HCT08
SN74HCT08N | Tube of 25 | -40 to 85 | SN74HCT08N
SN74HCT08NE4 | Tube of 25 | -40 to 85 | SN74HCT08N
SN74HCT08NSR | Reel of 2000 | -40 to 85 | HCT08 | Last Time Buy
SN74HCT08PW | | -40 to 85 | HT08 | Obsolete
SN74HCT08PWR | Reel of 2000 | -40 to 85 | HT08
SN74HCT08PWRG4 | Reel of 2000 | -40 to 85 | HT08
SN74HCT08PWT | | -40 to 85 | HT08 | Obsolete
```

## Conversion notes

- Source pages 1–10 converted; the package option addendum, tape and
  reel and mechanical pages (11–27) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: features list, package information table, functional block
  diagram (one 2-input AND gate, inputs xA and xB, output xY), revision history,
  support pages, layout example figure (described in one sentence), the
  "for clock inputs, f~max~" sentence of the measurement conditions (the
  part has no clock input).
- Source issues:
  - Source error: the package information table (p.1) gives NS as
    "SO, 4"; the pin configuration (p.3) shows the 14-pin SO and the
    thermal table gives NS 14 pins. Kept out (that table is dropped).
