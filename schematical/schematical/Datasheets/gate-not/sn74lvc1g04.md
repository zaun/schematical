---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g04",
  "title": "SN74LVC1G04 single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc1g04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES214AF",
    "date": "2025-10",
    "pages": 47
  },
  "devices": ["gate:1G04", "gate:1G04_SON"],
  "parts": ["SN74LVC1G04DBVR", "SN74LVC1G04DBVT", "SN74LVC1G04DBVTE4", "SN74LVC1G04DCKR", "SN74LVC1G04DCKT", "SN74LVC1G04DPWR", "SN74LVC1G04YZPR", "SN74LVC1G04DRY2", "SN74LVC1G04DRYR", "SN74LVC1G04DSF2", "SN74LVC1G04DSFR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G04 single inverter gate

## Description

This single inverter gate is designed for 1.65V to 5.5V V~CC~ operation.

The SN74LVC1G04 device performs the Boolean function Y = !{A}.

The CMOS device has high output drive while maintaining low static power
dissipation over a broad V~CC~ operating range.

Supports 5V V~CC~ operation. Inputs accept voltages up to 5.5V allowing
down translation to V~CC~. Maximum t~pd~ of 3.3ns at 3.3V. Low power
consumption, 10μA maximum I~CC~. ±24mA output drive at 3.3V. I~off~
supports live-insertion, partial-power-down mode, and back-drive
protection. Latch-up performance exceeds 100mA per JESD 78, Class II.

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
NC: No connect
A: Input
GND: Ground
Y: Output
VCC: Power terminal
DNU: Do not use
```

## Functional description

The SN74LVC1G04 device contains inverter gate and performs the Boolean
function Y = !{A}. This device is fully specified for
partial-power-down applications using I~off~. The I~off~ circuitry
disables the outputs, preventing damaging current backflow through the
device when it is powered down.

The DPW package technology is a major breakthrough in IC packaging. Its
tiny 0.64mm square footprint saves significant board space over other
package options while still retaining the traditional manufacturing
friendly lead pitch of 0.5mm.

Conversion note: the functional block diagram shows one inverter with
input A and output Y.

- Wide operating voltage range.
  - Operates from 1.65V to 5.5V.
- Allows down voltage translation.
- Inputs accept voltages to 5.5V.
- I~off~ feature allows voltages on the inputs and outputs, when V~CC~
  is 0V.

## Function tables

```table id=function caption="Function table"
| Input A | Output Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65V to 1.95V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3V to 3.6V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65V to 1.95V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3V to 3.6V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | -8 | mA |
| ^ | ^ | V~CC~ = 3V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | 8 | mA |
| ^ | ^ | V~CC~ = 3V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8V ± 0.15V, 2.5V ± 0.2V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3V ± 0.3V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5V ± 0.5V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC1G04 | < | < | < | < | < | Unit |
| ^ | ^ | DBV 5 pins | DCK 5 pins | DRL 5 pins | DRY 6 pins | YZP 5 pins | DPW 4 pins [2] | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 357.1 | 371.0 | 243 | 439 | 130 | 340 | °C/W |
| R~θJCtop~ | Junction-to-case (top) thermal resistance | 263.7 | 297.5 | 78 | 277 | 54 | 215 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 264.4 | 258.6 | 78 | 271 | 51 | 294 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 195.6 | 195.6 | 10 | 84 | 1 | 41 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 262.2 | 256.2 | 77 | 271 | 50 | 294 | °C/W |
| R~θJCbot~ | Junction-to-case (bottom) thermal resistance | | | | | | 250 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
[2] Source conflict: the thermal table (p.5) heads the DPW column "4 PINS"; the package information table (p.1), the pin functions table (p.3) and the package option addendum (p.16) give DPW five pins.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | Recommended –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –100μA | 1.65V to 5.5V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | | I~OH~ = –4mA | 1.65V | 1.2 | | | 1.2 | | | V |
| ^ | | I~OH~ = –8mA | 2.3V | 1.9 | | | 1.9 | | | V |
| ^ | | I~OH~ = –16mA | 3V | 2.4 | | | 2.4 | | | V |
| ^ | | I~OH~ = –24mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | | I~OH~ = –32mA | 4.5V | 3.8 | | | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100μA | 1.65V to 5.5V | | | 0.1 | | | 0.1 | V |
| ^ | | I~OL~ = 4mA | 1.65V | | | 0.45 | | | 0.45 | V |
| ^ | | I~OL~ = 8mA | 2.3V | | | 0.3 | | | 0.3 | V |
| ^ | | I~OL~ = 16mA | 3V | | | 0.4 | | | 0.4 | V |
| ^ | | I~OL~ = 24mA | ^ | | | 0.55 | | | 0.55 | V |
| ^ | | I~OL~ = 32mA | 4.5V | | | 0.55 | | | 0.55 | V |
| I~I~ | A input | V~I~ = 5.5V or GND | 0 to 5.5V | | | ±5 | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5V | 0 | | | ±10 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5V or GND, I~O~ = 0 | 1.65V to 5.5V | | | 10 | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6V, Other inputs at V~CC~ or GND | 3V to 5.5V | | | 500 | | | 500 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3V | | 3.5 | | | 3.50 | | pF |

[1] All typical values are at V~CC~ = 3.3V, T~A~ = 25°C.
```

```table id=operating caption="Operating characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8V | V~CC~ = 2.5V | V~CC~ = 3.3V | V~CC~ = 5.0V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10MHz | 16 | 18 | 18 | 20 | pF |
```

## Switching characteristics

```table id=sw-15pf-85 caption="Switching characteristics, C~L~ = 15pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15pF (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 2 | 6.4 | 1 | 4.2 | 0.7 | 3.3 | 0.7 | 3.1 | ns |
```

```table id=sw-50pf-85 caption="Switching characteristics, C~L~ = 30pF or 50pF, –40°C to 85°C" conditions="Over recommended operating free-air temperature range, C~L~ = 30pF or 50pF (unless otherwise noted) (see Figure 6-2)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3 | 7.5 | 1.4 | 5.2 | 1 | 4.2 | 1 | 3.7 | ns |
```

```table id=sw-15pf-125 caption="Switching characteristics, C~L~ = 15pF, –40°C to 125°C" conditions="Over recommended operating free-air temperature range, C~L~ = 15pF (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 2 | 6.4 | 1 | 4.2 | 0.7 | 3.3 | 0.7 | 3.1 | ns |
```

```table id=sw-50pf-125 caption="Switching characteristics, C~L~ = 30pF or 50pF, –40°C to 125°C" conditions="Over recommended operating free-air temperature range, C~L~ = 30pF or 50pF (unless otherwise noted) (see Figure 6-2)"
| Symbol | From (input) | To (output) | –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3 | 7.5 | 1.4 | 5.2 | 1 | 4.2 | 1 | 3.7 | ns |
```

## Typical characteristics

```curve id=tpd-temp caption="TPD across temperature at 3.3V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.7], [-40, 2.9], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```

```curve id=tpd-vcc caption="TPD across V~CC~ at 25°C"
{"x": {"label": "Vcc", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 7.0], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: Figure 6-1 and Figure 6-2 are the same circuit and
waveforms with different values, and also show pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions-1 caption="Test conditions (Figure 6-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```

```table id=test-conditions-2 caption="Test conditions (Figure 6-2)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

The SN74LVC1G04 is a high drive CMOS device that can be used for
implementing inversion logic with a high output drive, such as an LED
application. It can produce 24mA of drive current at 3.3V making it
Ideal for driving multiple outputs and good for high speed applications
up to 100Mhz. The inputs are 5.5V tolerant allowing it to translate down
to V~CC~.

Conversion note: the typical application drawings show an inverter logic
function, with an LVC1G04 between two blocks labelled uC or Logic, and a
basic LED driver, with an LVC1G04 driven by uC or Logic and its output
driving an LED with a series resistor to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing.

### Detailed design procedure

1. Recommended Input Conditions
   - Rise time and fall time specs: See (Δt/ΔV) in the Recommended
     Operating Conditions table.
   - Specified high and low levels: See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the Recommended Operating Conditions table at any
     valid V~CC~.
2. Recommend Output Conditions
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed total current (continuous current through V~CC~ or GND)
     for the part. These limits are located in the Absolute Maximum
     Ratings table.
   - Outputs should not be pulled above V~CC~.

### Power supply recommendations

The power supply can be any voltage between the min and max supply
voltage rating located in the Recommended Operating Conditions table.

Each VCC pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1μF capacitor is
recommended. if there are multiple VCC pins, then a 0.01μF or 0.022μF
capacitor is recommended for each power pin. It is ok to parallel
multiple bypass capacitors to reject different frequencies of noise.
0.1μF and 1μF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power pin as possible for
best results.

### Layout guidelines

When using multiple bit logic devices, inputs should never float.

In many cases, functions or parts of functions of digital logic devices
are unused; for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. The rules
that must be observed under all circumstances are specified in the next
paragraph.

All unused inputs of digital logic devices must be connected to a high
or low bias to prevent them from floating. The logic level that should
be applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or VCC; whichever makes more
sense or is more convenient.

Conversion note: the layout example shows two gates, one with its unused
input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=X2SON-5-DPW
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74LVC1G04DBVR | Reel of 3000 | -40 to 125 | 3VHF, C045, C04F, C04J, C04K, C04R, C04H, C04P, C04S
SN74LVC1G04DBVT | Reel of 250 | -40 to 125 | C045, C04F, C04J, C04K, C04R, C04H, C04P, C04S
SN74LVC1G04DBVTE4 | Reel of 250 | -40 to 125 | C04 C04P
SN74LVC1G04DCKR | Reel of 3000 | -40 to 125 | 1X6, CC5, CCF, CCJ, CCK, CCR, CCH, CCP, CCS
SN74LVC1G04DCKT | Reel of 250 | -40 to 125 | CC5, CCF, CCJ, CCR, CCH, CCP
SN74LVC1G04DPWR | Reel of 3000 | -40 to 125 | K4
SN74LVC1G04DRY2 | Reel of 5000 | -40 to 125 | CC
SN74LVC1G04DRYR | Reel of 5000 | -40 to 125 | CC
SN74LVC1G04DSF2 | Reel of 5000 | -40 to 125 | CC
SN74LVC1G04DSFR | Reel of 5000 | -40 to 125 | CC
SN74LVC1G04YZPR | Reel of 3000 | -40 to 85 | CC7, CCN
```

## Conversion notes

- Source pages 1 and 3–12 converted. The ordering lines are from the
  package option addendum (pp.15–17). The table of contents (p.2), the
  support and revision history page (13), p.14, the rest of the addendum
  (p.18), tape and reel and mechanical pages (19–47) are left out; the
  generated Package section replaces them.
- Dropped: features list (the supply, input-voltage, t~pd~, I~CC~,
  output-drive, I~off~ and latch-up lines are kept in Description; its
  ESD lines are in the ESD ratings table), applications list, package
  information table, the sentence on available packages (p.1), the
  vendor pinout drawings, the simplified schematic and the functional
  block diagram (pp.1 and 10, described in one sentence); the typical
  application drawings (p.11) and the layout example (p.12) are
  described in one sentence each.
- Not converted: the application curve (Figure 8-1, I~CC~ vs Frequency,
  p.12).
- Not claimed here: the other orderables of the package option addendum
  (DRL and YZV parts and further E4/G4 variants). The YZV column of the
  pin functions table is left out: no claimed part uses that pinout.
- The pin descriptions are the DESCRIPTION column of the pin functions
  table; "Do not use" for DNU is from the legend of the pin drawings
  (p.3).
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figures 6-1 and 6-2, A, C, D, G and H are kept as sentences; B, E and
  F are for the enable and disable waveforms, which are not drawn.
- Curves digitized from Figures 5-1 and 5-2 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{A} (pp.1 and 10).
- Source issues:
  - Source conflict: the thermal table (p.5) heads the DPW column
    "4 PINS"; elsewhere DPW has five pins (thermal table note).
  - Source conflict: the recommended operating conditions (p.5) give
    T~A~ –40 to 125 °C for all packages; the package option addendum
    (p.17) lists SN74LVC1G04YZPR at -40 to 85.
  - Source conflict: the YZP pin drawing (p.3) labels ball A1 DNU; the
    pin functions table (p.3) lists A1 and B2 of the YZP package under
    NC, "No connect".
  - Source error: "contains inverter gate" (Overview, p.10), "Ideal" and
    "100Mhz" (p.11), "Recommend Output Conditions" (p.11) and "if there
    are" (p.12) are the source's wording, kept as printed.
  - Unconfirmed: the C~L~ = 15pF switching tables (pp.6–7) and the
    C~L~ = 30pF or 50pF tables give the same values for –40°C to 85°C
    and –40°C to 125°C, and ψ~JT~ is 195.6 °C/W for both DBV and DCK
    (p.5).
