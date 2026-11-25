---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g32-q1",
  "title": "SN74LVC1G32-Q1 single 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lvc1g32-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES648B",
    "date": "2026-08",
    "pages": 29
  },
  "devices": ["gate:1G32", "gate:1G32_SON"],
  "parts": ["74LVC1G32QDBVRQ1G4", "SN74LVC1G32QDBVRQ1", "SN74LVC1G32QDCKRQ1", "SN74LVC1G32QDRYRQ1"],
  "packages": {"p5": "DBV, DCK", "p6": "DRY"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G32-Q1 single 2-input positive-OR gate

## Description

This single 2-input positive-OR gate is designed for 1.65V to 5.5V
V~CC~ operation. [[1::4]]

The SN74LVC1G32-Q1 device performs the Boolean function Y = A + B or
Y = !{!{A} · !{B}} in positive logic. Source error: the source draws a
bar over A and one bar over the whole product A · B, but none over B
(pp.1 and 10), which is not an OR; Y = A + B and the function table
(p.10) show the OR. [[1::5]]

The CMOS device has high output drive while maintaining low static
power dissipation over a broad V~CC~ operating range. [[1::6]]

Supports V~CC~ operation: 5V. Inputs accept voltages to: 5.5V. Supports
down translation to V~CC~. Maximum t~pd~ of 3.6ns at 3.3V. Low power
consumption: 10µA maximum I~CC~. ±24mA output drive at 3.3V. I~off~
supports live insertion, partial-power-down mode, and back-drive
protection. Latch-up performance exceeds 100mA per JESD 78, Class II. [[1::1]]

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

## Pin functions

```pins
A: Input
B: Input
GND: Ground
NC: Not connected
VCC: Power pin
Y: Output
```
[[3::Table 4-1]]

## Functional description

The SN74LVC1G32-Q1 device contains one 2-input positive OR gate device
and performs the Boolean function Y = A + B or Y = !{!{A} · !{B}}.
SN74LVC1G32-Q1 is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when powered down. [[10::1]]

Conversion note: the functional block diagram shows one 2-input OR gate
with inputs A and B and output Y.

### Feature description

- Wide operating voltage range. [[10::2]]
  - Operates from 1.65V to 5.5V. [[10::2]]
- Allows down voltage translation. [[10::2]]
- Inputs accept voltages to 5.5V. [[1::1]]
- I~off~ feature allows voltages on the inputs and outputs, when V~CC~
  is 0V. [[10::2]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```
[[10::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Definition | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
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
| T~A~ | Operating free-air temperature | DSBGA package | -40 | 85 | °C |
| ^ | ^ | All other packages | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application note, Implications of Slow or Floating CMOS Inputs.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC1G32-Q1 | < | < | Unit |
| ^ | ^ | DBV, 5 pins | DCK, 5 pins | DRY, 6 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 357.1 | 371 | 439 | °C/W |
| R~θJCtop~ | Junction-to-case (top) thermal resistance | 263.7 | 297.5 | 277 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 264.4 | 258.6 | 271 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 195.6 | 195.6 | 84 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 262.2 | 256.2 | 271 | °C/W |
| R~θJCbot~ | Junction-to-case (bottom) thermal resistance | | | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C recommended | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | 1.65V to 5.5V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4mA | 1.65V | 1.2 | | | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8mA | 2.3V | 1.9 | | | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16mA | 3V | 2.4 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32mA | 4.5V | 3.8 | | | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | 1.65V to 5.5V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4mA | 1.65V | | | 0.45 | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8mA | 2.3V | | | 0.3 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 16mA | 3V | | | 0.4 | | | 0.5 | V |
| ^ | ^ | I~OL~ = 24mA | ^ | | | 0.55 | | | 0.65 | V |
| ^ | ^ | I~OL~ = 32mA | 4.5V | | | 0.55 | | | 0.65 | V |
| I~I~ | A or B inputs | V~I~ = 5.5V or GND | 0 to 5.5V | | | ±5 | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5V | 0 | | | ±10 | | | ±25 | µA |
| I~CC~ | | V~I~ = 5.5V or GND, I~O~ = 0 | 1.65V to 5.5V | | | 10 | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6V, Other inputs at V~CC~ or GND | 3V to 5.5V | | | 500 | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3V | | 4 | | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3V, T~A~ = 25°C.
```
[[6::table #1]]

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8V | V~CC~ = 2.5V | V~CC~ = 3.3V | V~CC~ = 5V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10MHz | 20 | 20 | 21 | 22 | pF |
```
[[7::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15pF (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.9 | 7.2 | 0.8 | 4.4 | 0.9 | 3.6 | 0.8 | 3.4 | ns |
```
[[6::table #2]]

```table id=sw-1v8-2v5 caption="Switching characteristics, 1.8V and 2.5V [1]" conditions="Over recommended operating free-air temperature range, C~L~ = 30pF or 50pF (unless otherwise noted) (see Figure 6-2)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | –40°C to 125°C recommended | < | –40°C to 85°C | < | –40°C to 125°C recommended | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 2.5V ± 0.2V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.8 | 8 | 2.8 | 9 | 1.2 | 5.5 | 1.2 | 6 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

```table id=sw-3v3-5v caption="Switching characteristics, 3.3V and 5V [1]" conditions="Over recommended operating free-air temperature range, C~L~ = 30pF or 50pF (unless otherwise noted) (see Figure 6-2)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | –40°C to 125°C recommended | < | –40°C to 85°C | < | –40°C to 125°C recommended | < | Unit |
| ^ | ^ | ^ | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.1 | 4.5 | 1 | 4 | 1 | 4 | 1 | 4.5 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested
```
[[7::table #2]]

## Typical characteristics

```curve id=tpd-temp caption="TPD across temperature at 3.3V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.7], [-40, 2.9], [0, 3.0], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```
[[7::Figure 5-1]]

```curve id=tpd-vcc caption="TPD across V~CC~ at 25°C"
{"x": {"label": "Vcc", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 6.9], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
 ]}
```
[[7::Figure 5-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[9::1]]

Conversion note: in both load circuits one R~L~ goes from the output to
ground and a second R~L~ from the output to switch S1, which goes to
V~LOAD~, open or GND; the figures below draw the load with S1 open.
Conversion note: both source figures also show pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

### Figure 6-1 (C~L~ = 15 pF)

```table id=s1-fig1 caption="Figure 6-1, load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[8::table #1]]

```figure ref=load-rl-cl caption="Figure 6-1, load circuit (S1 open)"
RL: see table
CL: see table
```
[[8::Figure 6-1]]

```table id=test-conditions-fig1 caption="Figure 6-1, test conditions"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```
[[8::table #2]]

```figure ref=waveforms-tpd caption="Figure 6-1, voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[8::Figure 6-1]]

### Figure 6-2 (C~L~ = 30 pF or 50 pF)

```table id=s1-fig2 caption="Figure 6-2, load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[8::table #3]]

```figure ref=load-rl-cl caption="Figure 6-2, load circuit (S1 open)"
RL: see table
CL: see table
```
[[8::Figure 6-1]]

```table id=test-conditions-fig2 caption="Figure 6-2, test conditions"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[9::table #1]]

```figure ref=waveforms-tpd caption="Figure 6-2, voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[8::Figure 6-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[11::1]]

The SN74LVC1G32-Q1 device is a high drive CMOS device that can be used
for implementing OR logic with a high output drive, such as an LED
application. SN74LVC1G32-Q1 produces 24mA of drive current at 3.3V
making the device and excellent choice for driving multiple outputs and
good for high speed applications up to 100MHz. The inputs are 5.5V
tolerant allowing translation down to V~CC~. [[11::2]]

Conversion note: the typical application schematic (Figure 8-1) shows the gate as an OR logic function between µC or logic signals, and as a basic LED driver with the LED and a series resistor from Y to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive will also create fast edges into
light loads, so routing and load conditions should be considered to
prevent ringing. [[11::3]]

### Detailed design procedure

1. Recommended Input Conditions: [[11::4]]
   - Rise time and fall time specs. See (Δt/ΔV) in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[11::4]]
   - Specified high and low levels. See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table. [[11::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the Recommended Operating Conditions table at any
     valid V~CC~. [[11::4]]
2. Recommend Output Conditions: [[11::4]]
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed total current (continuous current through V~CC~ or GND)
     for the part. These limits are located in the
     [Absolute Maximum Ratings](#absolute-maximum-ratings) table. [[12::1]]
   - Outputs should not be pulled above V~CC~. [[12::1]]

### Application curves

```curve id=icc-freq caption="I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [-20, 80]},
 "y": {"label": "Icc", "unit": "mA", "range": [-2, 10]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [20, 0.6], [40, 1.1], [60, 1.6], [75, 2.0]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [20, 1.0], [40, 1.9], [60, 2.5], [70, 2.9], [75, 3.5]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [30, 1.9], [49, 3.0], [55, 3.7], [60, 4.0], [75, 5.2]]},
  {"label": "Icc 5V", "points": [[0, 0], [25, 2.9], [30, 3.9], [49, 6.0], [55, 6.9], [67, 8.1], [75, 9.3]]}
 ]}
```
[[12::Figure 8-2]]

### Power supply recommendations

The power supply can be any voltage between the min and max supply
voltage rating located in the Recommended Operating Conditions table. [[12::2]]

Each VCC pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1μF capacitor is
recommended. If there are multiple VCC pins, then a 0.01μF or 0.022μF
capacitor is recommended for each power pin. It is ok to parallel
multiple bypass capacitors to reject different frequencies of noise. A
0.1μF and 1μF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power pin as possible for
best results. [[12::3]]

### Layout guidelines

When using multiple bit logic devices inputs should not ever float. [[12::4]]

In many cases, functions or parts of functions of digital logic devices
are unused; for example, when only two inputs of a triple-input AND
gate are used, or only 3 of the 4 buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states. [[12::5]]

Specified in the layout example are the rules that must be observed
under all circumstances. All unused inputs of digital logic devices
must be connected to a high or low bias to prevent them from floating.
The logic level that should be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or VCC, whichever make more sense or is more convenient. [[12::6]]

Conversion note: the layout example (Figure 8-3) shows a gate with its unused input tied to V~CC~ and a gate with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SON-6-DRY
```

## Ordering information

```ordering
74LVC1G32QDBVRQ1G4 | Reel of 3000 | -40 to 125 | 34P5, C32O
SN74LVC1G32QDBVRQ1 | Reel of 3000 | -40 to 125 | 34P5, C32O
SN74LVC1G32QDCKRQ1 | Reel of 3000 | -40 to 125 | 24V, CGJ, CGO
SN74LVC1G32QDRYRQ1 | Reel of 5000 | -40 to 125 | FW
```
[[15::table #1]]

## Conversion notes

- Source pages 1–12 converted. The ordering lines are from the package
  option addendum (pp.15–16). The table of contents (p.2), the support
  and revision history page (13), page 14, tape and reel (17–18),
  mechanical pages (19–28) and the notice page (29) are left out; the generated Package section
  replaces them.
- Dropped: applications list, package information table, the vendor
  pinout drawings; the functional block diagram (one OR gate), the
  typical application schematic (Figure 8-1) and the layout example
  (Figure 8-3) are described in text. The features list is given as
  sentences in Description.
- The load circuits are drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- Curves digitized from Figures 5-1, 5-2 and 8-2 to about 2 % of full
  scale.
- From images: ≤2 ns, ≤2.5 ns, 1 MΩ, 1 kΩ, 500 Ω, 50 Ω and V~Δ~ in the
  Figure 6-1 and Figure 6-2 tables and notes (pp.8–9; the text layer
  has other glyphs).
- From images: overbars of Y = !{!{A} · !{B}} (pp.1 and 10; the bar over
  B is the correction below).
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- Source issues:
  - Source error: the Description (p.1) and § 7.1 (p.10) print the
    second form of the Boolean function with a bar over A and one bar
    over the whole product A · B, but none over B, which is not an OR;
    Y = A + B and the function table (p.10) show the OR. Written
    Y = !{!{A} · !{B}}.
  - Source error: the ESD ratings table (p.4) heads its value column
    "VAUE"; written Value.
  - Source error: § 8.1 (p.11) prints "making the device and excellent
    choice"; kept as printed.
  - Unconfirmed: the recommended operating conditions (p.5) give a
    T~A~ row for a DSBGA package, which this datasheet does not
    otherwise list.
  - Unconfirmed: ψ~JT~ is 195.6 °C/W for both DBV and DCK (p.5).
