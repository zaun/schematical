---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2gu04-q1",
  "title": "SN74LVC2GU04-Q1 dual unbuffered inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc2gu04-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES902A",
    "date": "2025-10",
    "pages": 31
  },
  "devices": ["gate:2GU04"],
  "parts": ["1P2GU04QDRYRQ1"],
  "packages": {"p6": "DRY, DCK"},
  "converted": "2026-11-21"
}
---

# SN74LVC2GU04-Q1 dual unbuffered inverter

## Description

This dual inverter is designed for 1.65V to 5.5V V~CC~ operation. [[1::3]]

The SN74LVC2GU04-Q1 device contains two inverters with unbuffered
outputs and performs the Boolean function Y = !{A}. [[1::4]]

AEC-Q100 Qualified for automotive applications: Device temperature
grade 1: –40°C to +125°C, T~A~. Supports 5V V~CC~ operation. Inputs
accept voltages to 5.5V. Max t~pd~ of 3.7ns at 3.3V. Low power
consumption, 10µA max I~CC~. ±24mA Output drive at 3.3V. Typical
V~OLP~ (output ground bounce) <0.8V at V~CC~ = 3.3V, T~A~ = 25°C.
Typical V~OHV~ (output V~OH~ undershoot) >2V at V~CC~ = 3.3V,
T~A~ = 25°C. Can be used as a down translator to translate inputs from
a max of 5.5V down to the V~CC~ level. Unbuffered outputs. [[1::1]]

## Pinout

Conversion note: DRY and DCK share one pin order.

```pinout pinout=p6
```

## Pin functions

```pins
1A: Input
1Y: Output
2A: Input
2Y: Output
VCC: Positive Supply
GND: Ground
```
[[3::Table 4-1]]

See Package drawing at the end of the data sheet for dimensions [[3::1]]

## Functional description

The SN74LVC2GU04Q1 device contains two inverters with unbuffered
outputs with a maximum sink current of 32mA. [[8::1]]

Conversion note: the logic diagram (positive logic) shows two inverters, 1A (pin 1) to 1Y (pin 6) and 2A (pin 3) to 2Y (pin 4).

### Balanced high-drive CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The high-drive capability of this device creates fast edges
into light loads, so routing and load conditions must be considered to
prevent ringing. Additionally, the outputs of this device are capable
of driving larger currents than the device can sustain without being
damaged. It is important for the power output of the device to be
limited to avoid thermal runaway and damage due to overcurrent. The
electrical and thermal limits defined in the Absolute Maximum Ratings
must be followed at all times. [[8::2]]

### Standard CMOS inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance given in the Electrical
Characteristics. The worst-case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using Ohm's law (R = V ÷ I). [[8::3]]

Signals that are applied to the inputs need to have fast edge rates, as
shown by Δt/Δv in the Recommended Operating Conditions, to avoid
excessive current consumption and oscillations. If a slow or noisy
input signal is required, a device with a Schmitt-trigger input should
be used to condition the input signal prior to the standard CMOS input. [[8::4]]

### Negative clamping diodes

The inputs and outputs to this device have negative clamping diodes as
shown in Figure 7-1. [[9::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed. [[9::2]]

Conversion note: Figure 7-1 (electrical placement of clamping diodes for
each input and output) shows a diode from GND to the input, carrying
-I~IK~, and a diode from GND to the output, carrying -I~OK~.

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value specified in
the Recommended Operating Conditions. [[9::4]]

### Unbuffered logic

A standard CMOS logic function typically consists of at least three
stages: the input inverter, the logic function, and the output
inverter. Some devices have multiple stages at the input or output for
various reasons. An unbuffered CMOS logic function eliminates the extra
input and output stages; the device only contains the required logic
function which is directly driven from the inputs and directly drives
the outputs. [[9::5]]

The unbuffered inverter is commonly used in oscillator circuits because
it is less sensitive to parameter changes in the oscillator circuit due
to having lower total gain than a buffered equivalent. To learn more
about how to use an unbuffered inverter in an oscillator circuit, see
Use of the CMOS Unbuffered Inverter in Oscillator Circuits. [[10::2]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
| T~J~ | Operating junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Max | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per AEC Q100-002 [1] HBM ESD Classification Level | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per AEC Q100-011 CDM ESD Classification Level | ±1000 | ^ |

[1] AEC Q100-002 indicates that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 1.65 | 5.5 | V |
| V~IH~ | High-level input voltage | I~O~ = –100µA | 0.75 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | I~O~ = 100µA | | 0.25 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5V | | 32 | ^ |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | DCK (6 pins) | DRY (6 pins) | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 187.7 | 233.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 115.2 | 144.6 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 48.4 | 119.9 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 30.9 | 15.8 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 48.2 | 119.3 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application note.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | V~IL~ = 0V | I~OH~ = -100µA | 1.65V to 5.5V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | ^ | I~OH~ = -4mA | 1.65 V | 1.2 | | | 1.2 | | | ^ |
| ^ | ^ | ^ | I~OH~ = -8mA | 2.3 V | 1.9 | | | 1.9 | | | ^ |
| ^ | ^ | ^ | I~OH~ = -16mA | 3V | 2.4 | | | 2.4 | | | ^ |
| ^ | ^ | ^ | I~OH~ = -24mA | ^ | 2.3 | | | 2.3 | | | ^ |
| ^ | ^ | ^ | I~OH~ = -32mA | 4.5 V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | | V~IH~ = V~CC~ | I~OL~ = 100µA | 1.65V to 5.5V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4mA | 1.65 V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | ^ | I~OL~ = 8mA | 2.3 V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | ^ | I~OL~ = 16mA | 3V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | ^ | I~OL~ = 24mA | ^ | | | 0.55 | | | 0.55 | ^ |
| ^ | ^ | ^ | I~OL~ = 32mA | 4.5 V | | | 0.55 | | | 0.55 | ^ |
| I~I~ | A inputs | V~I~ = 5.5V or GND | < | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~CC~ | | V~I~ = 5.5V or GND, I~O~ = 0 | < | 1.65V to 5.5V | | | 10 | | | 10 | µA |
| C~I~ | | V~I~ = V~CC~ or GND | < | 3.3 V | | 7 | | | | | pF |

[1] All typical values are at V~CC~ = 3.3V, T~A~ = 25°C.
```
[[5::table #2]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8V | V~CC~ = 2.5V | V~CC~ = 3.3V | V~CC~ = 5V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10MHz | 7 | 7 | 8 | 23 | pF |
```
[[6::table #1]]

## Switching characteristics

```table id=sw-85 caption="Switching characteristics – -40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Parameter Measurement Information)"
| Parameter | From (input) | To (output) | –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.2 | 5.5 | 1 | 4 | 1.1 | 3.7 | 1 | 3 | ns |
```
[[6::table #2]]

```table id=sw-125 caption="Switching characteristics – -40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Parameter Measurement Information)"
| Parameter | From (input) | To (output) | –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.2 | 6.3 | 1 | 4.5 | 1.1 | 4.2 | 1 | 3.5 | ns |
```
[[6::table #3]]

## Typical characteristics

```curve id=tpd-vcc caption="t~pd~ vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [1, 6]},
 "y": {"label": "tpd", "unit": "ns", "range": [0, 5]},
 "series": [
  {"label": "tpd(MAX)", "points": [[2, 5], [2.5, 4.45], [3.3, 4.15], [5, 3.45]]},
  {"label": "tpd(MIN)", "points": [[1.8, 1.3], [2.5, 1.0], [3.3, 1.15], [5, 1.0]]}
 ]}
```
[[6::figure #1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[7::1]]

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figure also shows pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[7::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[7::Figure 6-1]]

```table id=test-conditions caption="Test conditions (Figure 6-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[7::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[7::Figure 6-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

The unbuffered inverter is commonly used in oscillator circuits because
it is less sensitive to parameter changes in the oscillator circuit due
to having lower total gain than a buffered equivalent. An example
application circuit is shown in Figure 8-1. To learn more about how to
use an unbuffered inverter in an oscillator circuit, refer to the Use
of the CMOS Unbuffered Inverter in Oscillator Circuits application
report. [[10::2]]

Conversion note: the typical application diagram (Figure 8-1) shows a
crystal oscillator, with the inverter U and a feedback resistor R~F~
(~2.2 MΩ) in parallel, a series resistor R~S~ (~1 kΩ) from the inverter
output to the crystal, capacitors C~1~ and C~2~ (~32 pF each) from the
two sides of the crystal to ground, and a load C~L~ on the output.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive also creates fast edges into light
loads, so routing and load conditions should be considered to prevent
ringing. [[10::3]]

### Detailed design procedure

To learn more about how to use an unbuffered inverter in an oscillator
circuit, refer to the Use of the CMOS Unbuffered Inverter in Oscillator
Circuits application report. [[10::2]]

1. Recommended Input Conditions [[10::4]]
   - Specified high and low levels. See (V~IH~ and V~IL~) in
     Recommended Operating Conditions. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in Recommended Operating Conditions at any valid V~CC~. [[10::4]]
2. Absolute Maximum Output Conditions [[10::4]]
   - Load currents must not exceed (I~O~ max) per output and must not
     exceed (Continuous current through V~CC~ or GND) total current for
     the part. These limits are located in Absolute Maximum Ratings. [[10::4]]
   - Outputs must not be pulled above the voltage rated in the Absolute
     Maximum Ratings. [[10::4]]

### Application curve

```curve id=icc-freq caption="I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 80]},
 "y": {"label": "Icc", "unit": "µA", "range": [0, 1600]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [20, 70], [40, 130], [60, 180], [75, 220]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [20, 130], [40, 200], [60, 290], [75, 350]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [20, 180], [40, 340], [60, 470], [75, 600]]},
  {"label": "Icc 5V", "points": [[0, 0], [20, 400], [35, 700], [45, 930], [60, 1200], [75, 1500]]}
 ]}
```
[[11::Figure 8-2]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions
table. [[11::1]]

The V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. A 0.1µF capacitor is recommended, and it is ok to parallel
multiple bypass caps to reject different frequencies of noise. 0.1µF
and 1µF capacitors are commonly used in parallel. The bypass capacitor
must be installed as close to the power pin as possible for best
results. [[11::2]]

### Layout guidelines

Even low data rate digital signals can contain high-frequency signal
components due to fast edge rates. When a printed-circuit board (PCB)
trace turns a corner at a 90° angle, a reflection can occur. A
reflection occurs primarily because of the change of width of the
trace. At the apex of the turn, the trace width increases to 1.414
times the width. This increase upsets the transmission-line
characteristics, especially the distributed capacitance and
self–inductance of the trace which results in the reflection. Not all
PCB traces can be straight and therefore some traces must turn corners. [[11::3]]

An example layout is given in Figure 8-3 for the DRY (SON) package.
This example layout includes a 0402 (metric) capacitor and uses the
measurements found in the example board layout appended to this end of
this datasheet. A via of diameter 0.1mm (3.973 mil) is placed directly
in the center of the device. This via can be used to trace out the
center pin connection through another board layer, or it can be left
out of the layout [[11::4]]

Conversion note: the layout example for the DRY package (Figure 8-3)
shows a 0402 bypass capacitor placed close to the device between V~CC~
and GND, the six pads with a via in the center, signal lines without
90° corners, and a GND pour around them.

## Package

```package footprint=SON-6-DRY
```

## Ordering information

```ordering
1P2GU04QDRYRQ1 | Reel of 5000 | -40 to 125 | FZ
```
[[14::table #1]]

## Conversion notes

- Source pages 1 and 3–12 converted. The ordering line is from the
  package option addendum (p.22). The table of contents (p.2), the
  support and revision history page (13), tape and reel (pp.14–15),
  mechanical pages (16–21 and 24–30), the rest of the addendum (p.23)
  and the notice page (31) are left out; the generated Package section
  replaces them.
- Dropped: applications list, device information table and the vendor
  pinout drawings; the logic diagram, the clamp diode figure
  (Figure 7-1), the typical application diagram (Figure 8-1) and the
  layout example (Figure 8-3) are described in text. The features are
  sentences in Description.
- The pin table's note on its DRY (SON) column is kept as a line under
  the pin descriptions.
- The CAUTION box of the Negative Clamping Diodes section is kept as a
  paragraph.
- The source also covers the DCK package (CLVC2GU04QDCKRQ1 in the
  addendum), which the catalog doesn't carry.
- The Thermal Information table prints "—" for R~θJC(bot)~ of both
  packages; the cells are left empty.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 6-1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- Curves digitized from Figure 5-1 (p.6) and Figure 8-2 (p.11) to about
  2 % of full scale.
- From images: overbar of Y = !{A} (p.1; the text layer prints
  "Y = A"); ≤2 ns, ≤2.5 ns, ≤ 10 MHz, 1 kΩ, 500 Ω, 50 Ω, V~Δ~ (p.7; the
  text layer prints "£2 ns", "1 kW", "500 W", "VD"); the curve points
  of Figures 5-1 and 8-2; the Ω of ~2.2 MΩ and ~1 kΩ and the
  arrangement of Figure 8-1 (p.10); the arrangement of Figure 7-1 (p.9)
  and Figure 8-3 (p.12).
- Source issues:
  - Source conflict: the Standard CMOS Inputs section (p.8) refers to
    Δt/Δv in the Recommended Operating Conditions; that table (p.4) has
    no Δt/Δv row.
  - Source error: the last sentence of the Layout Guidelines (p.11),
    "or it can be left out of the layout", ends there without a full
    stop, and the next page does not continue it. Kept as printed.
  - Unconfirmed: the ESD Ratings table (p.4) prints "HBM ESD
    Classification Level" and "CDM ESD Classification Level" with no
    level after them.
