---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g11",
  "title": "SN74LVC1G11 single 3-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc1g11.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES487I",
    "date": "2024-11",
    "pages": 37
  },
  "devices": ["gate:1G11"],
  "parts": [
    "SN74LVC1G11DBVR", "SN74LVC1G11DBVRE4", "SN74LVC1G11DBVRG4", "SN74LVC1G11DCKR",
    "SN74LVC1G11DCKRE4", "SN74LVC1G11DCKRG4", "SN74LVC1G11DRYR", "SN74LVC1G11DRYRG4",
    "SN74LVC1G11DSFR", "SN74LVC1G11DSFRG4", "SN74LVC1G11YZPR"
  ],
  "packages": {"p6": "DBV, DCK, DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-08"
}
---

# SN74LVC1G11 single 3-input positive-AND gate

## Description

The SN74LVC1G11 performs the Boolean function Y = A • B • C or
Y = !{!{A} + !{B} + !{C}} in positive logic. This 3-input AND gate is
designed for 1.65 V to 5.5 V V~CC~ operation. The 5.5 V I/Os allow down
translation and also allow voltages at the inputs when V~CC~ = 0 V.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. The YZP
package is a NanoFree (DSBGA) package, using the die as the package.

## Pinout

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: A Input
B: B Input
C: C Input
GND: Ground
VCC: Power Supply
Y: Y Output
```

## Function tables

```table id=function caption="Function table"
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | H |
| L | X | X | L |
| X | L | X | L |
| X | X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

Latch-up performance exceeds 100 mA per JESD 78, class II.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 3 V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 3 V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 10 | ns/V |
| T~A~ | Operating free-air temperature | BGA package | -40 | 85 | °C |
| ^ | ^ | All other packages | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DBV (SOT-23), 6 pins | DCK (SC70), 6 pins | DRY (SON), 6 pins | YZP (DSBGA), 6 pins | DSF (SON), 6 pins | Unit |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 195.9 | 260.1 | 424.6 | 105.8 | 413.7 | °C/W |
| R~θJCtop~ | Junction-to-case (top) thermal resistance | 177.4 | 98.1 | 309 | 1.6 | 226.6 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 51.7 | 63.1 | 292 | 10.8 | 317 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 61.3 | 2.2 | 135.4 | 3.1 | 37.4 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 51.3 | 62.4 | 292 | 10.8 | 317 | °C/W |
| R~θJCbot~ | Junction-to-case (bottom) thermal resistance | | | | | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | V |
| I~I~ | All inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | µA |
| I~off~ | Partial-power-down leakage | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | µA |
| I~CC~ | Supply current | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | µA |
| ΔI~CC~ | Supply-current change | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 3.5 | | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF, T~A~ = –40 °C to +85 °C (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | V~CC~ | Min | Max | Unit |
|---|---|---|---|--:|--:|---|
| t~pd~ | A, B, or C | Y | V~CC~ = 1.8 V ± 0.15 V | 2.6 | 15.2 | ns |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | 1.6 | 5.6 | ns |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | 1.2 | 4.1 | ns |
| ^ | ^ | ^ | V~CC~ = 5 V ± 0.5 V | 1 | 3.1 | ns |
```

```table id=sw-85 caption="Switching characteristics, C~L~ = 30 pF or 50 pF, –40 °C to 85 °C [1]" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF, T~A~ = –40 °C to +85 °C (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | Min | Max | Unit |
|---|---|---|---|--:|--:|---|
| t~pd~ | A, B, or C | Y | V~CC~ = 1.8 V ± 0.15 V | 2.9 | 17.2 | ns |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | 1.4 | 6.2 | ns |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | 1.3 | 4.9 | ns |
| ^ | ^ | ^ | V~CC~ = 5 V ± 0.5 V | 1 | 3.5 | ns |

[1] Source conflict: the only Load Circuit and Voltage Waveforms figure (Figure 6-1) gives C~L~ = 15 pF and R~L~ = 1 MΩ; the source gives no 30 pF or 50 pF load values.
```

```table id=sw-125 caption="Switching characteristics, C~L~ = 30 pF or 50 pF, –40 °C to 125 °C" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF, T~A~ = –40 °C to +125 °C (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | V~CC~ | Min | Max | Unit |
|---|---|---|---|--:|--:|---|
| t~pd~ | A, B, or C | Y | V~CC~ = 1.8 V ± 0.15 V | 2.9 | 20 | ns |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | 1.4 | 7.8 | ns |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | 1.3 | 6.2 | ns |
| ^ | ^ | ^ | V~CC~ = 5 V ± 0.5 V | 1 | 4.6 | ns |
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | V~CC~ = 1.8 V | 18 | pF |
| ^ | ^ | ^ | V~CC~ = 2.5 V | 19 | pF |
| ^ | ^ | ^ | V~CC~ = 3.3 V | 20 | pF |
| ^ | ^ | ^ | V~CC~ = 5 V | 23 | pF |
```

## Typical characteristics

```curve id=ioh-voh caption="Output current drive vs high-level output voltage; T~A~ = 25 °C, V~CC~ = 3 V, V~IH~ = 3 V, V~IL~ = 0 V, all outputs switching"
{"x": {"label": "V_OH", "unit": "V", "range": [-1, 4]},
 "y": {"label": "I_OH", "unit": "mA", "range": [-100, 60]},
 "series": [
  {"label": "VCC = 3 V", "points": [[-1.0, -90], [-0.9, -68], [-0.8, -58], [-0.6, -55], [0, -53], [0.5, -52], [1.0, -49], [1.5, -45], [2.0, -36], [2.5, -22], [3.0, 0], [3.5, 24], [4.0, 47]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

The load circuit also has an R~L~ from the output to switch S1 (V~LOAD~,
open or GND). S1 is open for t~PLH~/t~PHL~, which leaves the load below.
The source figure also shows pulse-duration, setup/hold and
enable/disable waveforms; all parameters and waveforms are not
applicable to all devices.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 1 MΩ
CL: 15 pF
```

```table id=test-conditions caption="Test conditions (Figure 6-1)"
| V~CC~ | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: see table
```

## Application information

The SN74LVC1G11 device offers logical AND configuration for many design
applications. This example describes basic power sequencing: power
sequencing is often used in applications that require a processor or
other delicate device with specific voltage timing requirements in order
to protect the device from malfunctioning. In the application below, the
power-good signals from the supplies tell the MCU to continue an
operation: Supply 1, Supply 2 and Supply 3 drive the three inputs, Y
drives the MCU, and V~CC~ = 3.3 V is bypassed with 0.1 µF.

Design requirements:

- Inputs: for rise time and fall time specifications, see Δt/Δv; for
  specified high and low levels, see V~IH~ and V~IL~ in the
  [recommended operating conditions](#recommended-operating-conditions).
  Inputs and outputs are overvoltage tolerant and can therefore go as
  high as 5.5 V at any valid V~CC~.
- Outputs: load currents must not exceed ±50 mA.
- Frequency: added trace resistance and capacitance can reduce maximum
  frequency capability. Follow the layout practices below.

The SN74LVC1G11 device uses CMOS technology and has balanced output
drive. Avoid bus contentions that can drive currents that can exceed
maximum limits. Maintain input signals as close as possible to either
0 V or V~CC~ for optimal operation. Figure 8-2 of the source shows a
simulated input-to-output voltage response at V~CC~ = 5 V showing the
propagation delay.

Power: the power supply can be any voltage between the minimum and
maximum supply voltage rating listed in the recommended operating
conditions. Ensure good bypass capacitance for each V~CC~ terminal; for
devices with a single-supply, a 0.1-µF bypass capacitor is recommended.
Capacitors with values of 0.1 µF and 1 µF are commonly used in parallel.
Place the bypass capacitor as close to the power terminal as possible for
best results.

Layout: inputs must never float. Unused input pins (for example, when
only two inputs of a triple-input AND gate are used) must not be left
unconnected, because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a high or low bias to prevent them
from floating; generally they are tied to GND or V~CC~, whichever makes
more sense or is more convenient. It is generally acceptable to float
outputs.

## Package

```package footprint=SOT23-6
```

```package footprint=SC70-6
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-6-YZP
```

## Ordering information

```ordering
```

## Conversion notes

- Source pages 1–12 converted; support pages, revision history (13),
  and the mechanical, package option addendum and tape and reel pages
  (14–37) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists, device information table,
  logic diagram (one 3-input AND gate), the layout diagrams (Figure 8-3:
  an unused input tied to V~CC~ or GND), the transceiver and
  multi-supply bypass advice (not applicable to this device). The
  application circuit (Figure 8-1) and the simulated response
  (Figure 8-2) are described in text.
- Curve digitized from Figure 5-1 to about 2 % of full scale.
- The text layer prints "£2 ns" and "1 MW"; the page image shows ≤ 2 ns
  and 1 MΩ. The text layer drops the overbars of the second Boolean form;
  the page image shows Y = !{!{A} + !{B} + !{C}} (p.1).
- Parameter names in the electrical characteristics table are added
  (the source gives symbols only).
- Source issues:
  - Source conflict: the 30 pF/50 pF switching tables refer to the Load
    Circuit and Voltage Waveforms, but the only such figure (Figure 6-1,
    p.8) gives C~L~ = 15 pF and R~L~ = 1 MΩ (p.6, p.8).
  - Source conflict: § 8.2.1 says inputs and outputs "can therefore go
    as high as 5.5 V at any valid VCC"; the absolute maximum ratings
    limit V~O~ in the high or low state to V~CC~ + 0.5 V (p.4).
  - Source error: § 8.2.1 says "Figure 8-2 illustrates the effects of
    frequency on output current"; Figure 8-2 is a simulated
    input-to-output voltage response against time (p.11). Kept out.
