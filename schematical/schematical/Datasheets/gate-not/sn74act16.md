---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act16",
  "title": "SN74ACT16 hex Schmitt-trigger inverters with open-drain outputs and TTL-compatible inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74act16.pdf",
    "vendor": "Texas Instruments",
    "document": "SCASE29",
    "date": "2024-11",
    "pages": 28
  },
  "devices": ["gate:74_16"],
  "parts": ["SN74ACT16BQAR", "SN74ACT16PWR"],
  "packages": {"p14": "PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74ACT16 hex Schmitt-trigger inverters with open-drain outputs and TTL-compatible inputs

## Description

The SN74ACT16 device contains six independent CMOS logic inverters with
TTL-compatible Schmitt-trigger inputs and open-drain outputs. [[1::2]]

Operating voltage range of 4.5V to 5.5V. TTL-compatible Schmitt-trigger
inputs support slow and noisy input signals. Continuous 24mA output
drive at 5V. Supports up to 75mA output drive at 5V in short bursts.
Drives 50Ω transmission lines. Fast operation with delay of 7.2ns max. [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2Y: Channel 2, Output Y
3A: Channel 3, Input A
3Y: Channel 3, Output Y
GND: Ground
4Y: Channel 4, Output Y
4A: Channel 4, Input A
5Y: Channel 5, Output Y
5A: Channel 5, Input A
6Y: Channel 6, Output Y
6A: Channel 6, Input A
VCC: Positive Supply
```
[[3::Table 4-1]]

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. Conversion note: the source has
this for the thermal pad, BQA package only. [[3::1]]

## Functional description

The SN74ACT16 hex inverters provide the logic function Y = !{A}. [[8::1]]

Conversion note: the logic diagram (positive logic) shows one of six
inverters, with a Schmitt-trigger input xA and an open-drain output xY.

### Open-drain CMOS outputs

This device includes open-drain CMOS outputs. Open-drain outputs can
only drive the output low. When in the high logical state, open-drain
outputs will be in a high-impedance state. The drive capability of this
device may create fast edges into light loads, so routing and load
conditions should be considered to prevent ringing. Additionally, the
outputs of this device are capable of driving larger currents than the
device can sustain without being damaged. It is important for the
output power of the device to be limited to avoid damage due to
overcurrent. The electrical and thermal limits defined in the Absolute
Maximum Ratings must be followed at all times. [[8::2]]

When placed into the high-impedance state, the output will neither
source nor sink current, with the exception of minor leakage current as
defined in the Electrical Characteristics table. In the high-impedance
state, the output voltage is not controlled by the device and is
dependent on external factors. If no other drivers are connected to the
node, then this is known as a floating node and the voltage is unknown.
A pull-up resistor can be connected to the output to provide a known
voltage at the output while it is in the high-impedance state. The
value of the resistor will depend on multiple factors, including
parasitic capacitance and power consumption limitations. Typically, a
10kΩ resistor can be used to meet these requirements. [[8::3]] [[8::8]]

Unused open-drain CMOS outputs should be left disconnected. [[8::4]]

### TTL-compatible Schmitt-trigger CMOS inputs

This device includes TTL-compatible CMOS inputs with Schmitt-trigger
architecture. These inputs are specifically designed to interface with
TTL logic devices by having reduced input voltage thresholds. [[8::5]]

TTL-compatible Schmitt-trigger CMOS inputs are high impedance and are
typically modeled as a resistor in parallel with the input capacitance
given in the Electrical Characteristics. The worst case resistance is
calculated with the maximum input voltage, given in the Absolute
Maximum Ratings, and the maximum input leakage current, given in the
Electrical Characteristics, using Ohm's law (R = V ÷ I). [[8::6]]

The Schmitt-trigger input architecture provides hysteresis as defined
by ΔV~T~ in the Electrical Characteristics table, which makes this
device extremely tolerant to slow or noisy inputs. While the inputs can
be driven much slower than standard CMOS inputs, it is still
recommended to properly terminate unused inputs. Driving the inputs
with slow transitioning signals will increase dynamic current
consumption of the device. For additional information regarding
Schmitt-trigger inputs, please see Understanding Schmitt Triggers. [[8::7]]

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a
system will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors,
however a 10kΩ resistor is recommended and will typically meet all
requirements. [[8::8]]

### Clamp diode structure

The inputs to this device have both positive and negative clamping
diodes, and the outputs to this device have negative clamping diodes
only as depicted in Figure 7-1. [[9::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[9::2]]

Conversion note: Figure 7-1, electrical placement of clamping diodes
for each input and output, shows a diode from the input to V~CC~
(+I~IK~), a diode from GND to the input (-I~IK~) and a diode from GND
to the output (-I~OK~).

## Function tables

Table 7-1 lists the function modes of the SN74ACT16. [[9::4]]

```table id=function caption="Function table"
| Inputs [1] | Outputs [2] |
| A | Y |
|:-:|:-:|
| L | Z |
| H | L |

[1] H = High voltage level, L = Low voltage level
[2] L = Driving low, Z = High-impedance state
```
[[3::Table 4-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | 7 | V |
| I~IK~ | Input clamp current | V~I~ < -0.5V or V~I~ > V~CC~ + 0.5V | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < -0.5V | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to 5.5V | | 50 | mA |
| | Continuous output current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |
| T~J~ | Junction temperature | | | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | 5.5 | V |
| V~I~ | Input Voltage | | 0 | V~CC~ | V |
| V~O~ | Output Voltage | | 0 | 5.5 | V |
| I~OL~ | Low-level output current | | | 24 | mA |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Package | Pins | Thermal metric [1] | < | < | < | < | < | Unit |
| ^ | ^ | R~θJA~ | R~θJC(top)~ | R~θJB~ | Ψ~JT~ | Ψ~JB~ | R~θJC(bot)~ | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| PW (TSSOP) | 14 | 132.2 | 64.8 | 88.4 | 11.9 | 87.4 | N/A | °C/W |
| BQA (WQFN) | 14 | 85.4 | 89.0 | 54.8 | 9.1 | 54.7 | 31.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 4.5V | 1.24 | 1.56 | 1.9 | V |
| ^ | ^ | | 5V | 1.32 | 1.65 | 2 | V |
| ^ | ^ | | 5.5V | 1.4 | 1.74 | 2.1 | V |
| V~T-~ | Negative-going input threshold voltage | | 4.5V | 0.67 | 0.91 | 1.18 | V |
| ^ | ^ | | 5V | 0.7 | 0.95 | 1.23 | V |
| ^ | ^ | | 5.5V | 0.73 | 0.99 | 1.28 | V |
| ΔV~T~ | Hysteresis (V~T+~ − V~T−~) | | 4.5V | 0.51 | 0.65 | 0.8 | V |
| ^ | ^ | | 5V | 0.55 | 0.7 | 0.86 | V |
| ^ | ^ | | 5.5V | 0.6 | 0.75 | 0.92 | V |
| V~OL~ | | I~OL~ = 50µA | 4.5V | | 0.001 | 0.1 | V |
| ^ | ^ | ^ | 5.5V | | 0.001 | 0.1 | V |
| ^ | ^ | I~OL~ = 24mA | 4.5V | | | 0.5 | V |
| ^ | ^ | I~OL~ = 24mA | 5.5V | | | 0.5 | V |
| ^ | ^ | I~OL~ = 75mA [1] | 5.5V | | | 1.65 | V |
| I~I~ | | V~I~ = 5.5V or GND | 0 to 5.5V | | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5V | | | 2 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 2.1; Any Input | 4.5 to 5.5V | | | 1.5 | mA |
| C~I~ | | V~I~ = V~CC~ or GND | 5V | | 2 | | pF |
| C~O~ | | V~O~ = V~CC~ or GND | 5V | | 3 | | pF |
| C~PD~ | | C~L~ = 50pF, F = 1MHz | 5V | | 12 | | pF |

[1] Duration not to exceed 2ms
```
[[5::table #1]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF; over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). See Parameter Measurement Information"
| Symbol | From (input) | To (output) | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|---|
| t~PLZ~ | A | Y | 5V | | 5.3 | 7.2 | ns |
| t~PZL~ | A | Y | 5V | | 4 | 5.9 | ns |
```
[[5::table #2]]

## Typical characteristics

T~A~ = 25°C (unless otherwise noted) [[6::1]]

```curve id=icc-vcc caption="Supply current across supply voltage"
{"x": {"label": "V_CC", "unit": "V", "range": [0.5, 5.5]},
 "y": {"label": "I_CC", "unit": "nA", "range": [0, 100]},
 "series": [
   {"label": "25 °C", "points": [[0.5, 11], [1, 15], [1.5, 19], [2, 22], [2.5, 24], [3, 27], [3.5, 29], [4, 30.5], [4.3, 31], [4.6, 29.5], [5, 28], [5.25, 29], [5.5, 32.5]]},
   {"label": "125 °C", "points": [[0.5, 28], [0.7, 32], [1, 33], [1.4, 39.5], [1.7, 38], [2, 37], [2.1, 41], [2.4, 38], [2.5, 40.5], [2.8, 39], [2.9, 42], [3.1, 42], [3.3, 36], [3.5, 42], [3.6, 38.5], [4, 39], [4.2, 36.5], [4.4, 41.5], [4.6, 42], [4.9, 37.5], [5.1, 43], [5.2, 55.5], [5.3, 54], [5.5, 73]]},
   {"label": "-40 °C", "points": [[0.5, 18], [1, 27], [1.5, 36], [2, 43], [2.5, 50], [3, 56], [3.5, 61.5], [4, 69], [4.3, 73.5], [4.5, 72.5], [4.8, 77], [5, 77], [5.1, 74], [5.3, 80], [5.5, 86]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=vol-iol caption="Output voltage vs current in LOW state; 5V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 50]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.4]},
 "series": [
   {"label": "25°C", "points": [[0, 0], [25, 0.145], [50, 0.29]]},
   {"label": "125°C", "points": [[0, 0], [25, 0.178], [50, 0.356]]},
   {"label": "-40°C", "points": [[0, 0], [25, 0.12], [50, 0.24]]}
 ]}
```
[[6::Figure 5-2]]

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily for the
examples listed in the following table. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1MHz,
Z~O~ = 50Ω, t~t~ < 2.5ns, V~t~ = 1.5V. [[7::1]]

The outputs are measured individually with one input transition per
measurement. [[7::2]]

```table id=load caption="Load conditions"
| Test | S1 | R~L~ | C~L~ | ΔV | V~LOAD~ |
|---|---|---|---|---|---|
| t~PLZ~, t~PZL~ | CLOSED | 500Ω | 50pF | 0.3V | 2×V~CC~ |
```
[[7::table #1]]

C~L~ includes probe and test-fixture capacitance. t~PLZ~ is the same as
t~dis~. t~PZL~ is the same as t~en~. [[7::3]]

Conversion note: Figure 6-1, load circuit for open-drain outputs, shows
the output under test with a test point, C~L~ and R~L~ to ground, and a
second R~L~ through switch S1 to V~LOAD~.

Conversion note: Figure 6-2, voltage waveforms propagation delays,
shows t~PLZ~ and t~PZL~ measured from the input at V~t~ to output
waveform 1 at 10% V~CC~ above V~OL~ and at 50%, and to output waveform
2 at 50% and at 10% above V~OL~.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

Open-drain outputs like those available in the SN74ACT16 provide the
ability to discharge a voltage node to ground without otherwise loading
the node significantly. It is recommended to add a series resistor
between the output and any capacitance larger than 50pF as shown in the
Typical Application Block Diagram to prevent damage to the device. [[10::2]]

The required resistor value can be determined using the maximum
capacitor voltage and the maximum continuous current for the output
from the equation: R ≥ V~C~/I~O(max)~. [[10::3]]

For any given RC combination, the discharge time can be determined
using the discharge plot provided in the Application Timing Diagram and
the equation τ = R × C. For example, to discharge a capacitor to 10% of
the starting value, it takes approximately 2.303 × τ = 2.303 × R × C
seconds. [[10::4]]

Conversion note: in the typical application block diagram (Figure 8-1)
the open-drain output drives a series resistor R to the node V~C~,
which has a capacitor C to ground.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics of the device as described in the Electrical
Characteristics section. [[11::1]]

The positive voltage supply must be capable of sourcing current equal
to the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. [[11::2]]

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74ACT16 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into its ground connection.
Ensure the maximum total current through GND listed in the Absolute
Maximum Ratings is not exceeded. [[11::3]]

The SN74ACT16 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50pF. [[11::4]]

The SN74ACT16 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the
HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin. [[11::5]]

Total power consumption can be calculated using the information
provided in CMOS Power Consumption and Cpd Calculation. [[11::6]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices. [[11::7]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::8]]

### Input considerations

Input signals must cross to be considered a logic LOW, and to be
considered a logic HIGH. Source error: the source prints this sentence
without the two levels the signals must cross (p.12); kept as printed.
Do not exceed the maximum input voltage range found in the Absolute
Maximum Ratings. [[12::1]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74ACT16 (as specified in the Electrical Characteristics), and the
desired input transition rate limits the resistor size. A 10kΩ resistor
value is often used due to these factors. [[12::2]]

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics. [[12::4]]

Open-drain outputs can be connected together directly to produce a
wired-AND configuration or for additional output drive strength. [[12::5]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[12::6]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[13::1]]
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74ACT16 to one or more of the receiving devices. [[13::1]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[13::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[13::1]]

Conversion note: the application timing diagram (Figure 8-2) plots the
capacitor voltage in percent against time in τ, a discharge curve
marked at (0.693 , 50%), (1 , 36.8%), (2.303 , 10%), (5 , 0.7%) and
(6 , 0.2%).

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1μF capacitor is recommended for this device. It
is acceptable to parallel multiple bypass capacitors to reject
different frequencies of noise. The 0.1μF and 1μF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results. [[13::2]]

### Layout guidelines

- Bypass capacitor placement [[15::1]]
  - Place near the positive supply terminal of the device [[14::1]]
  - Provide an electrically short ground return path [[14::1]]
  - Use wide traces to minimize impedance [[14::1]]
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible [[14::1]]
- Signal trace geometry [[14::1]]
  - 8mil to 12mil trace width [[14::1]]
  - Lengths less than 12cm to minimize transmission line effects [[14::1]]
  - Avoid 90° corners for signal traces [[14::1]]
  - Use an unbroken ground plane below signal traces [[14::1]]
  - Flood fill areas around signal traces [[14::1]]
  - For traces longer than 12cm [[14::1]]
    - Use impedance controlled traces [[14::1]]
    - Source-terminate using a series damping resistor near the output [[14::1]]
    - Avoid branches; buffer signals that must branch separately [[14::1]]

Conversion note: the layout example drawings (Figures 10-1 to 10-5)
show trace corners for improved signal integrity, bypass capacitor
placement for TSSOP, WQFN and SOT packages, and damping resistor
placement, and are not drawn here.

## Package

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74ACT16BQAR | Reel of 3000 | -40 to 125 | AD16
SN74ACT16PWR | Reel of 3000 | -40 to 125 | ACT16
```
[[17::table #1]]

## Conversion notes

- Source pages 1–15 converted; the support and revision history page
  (16), the package option addendum, tape and reel and mechanical pages
  (17–28) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are the parts' rows of the package option
  addendum (p.17).
- Dropped: applications list, device information table, the layout
  example drawings (Figures 10-1 to 10-5), the references to the Feature
  Description section in the input and output considerations.
- The electrical characteristics table of the source gives only symbols
  in its Parameter column for V~OL~ to C~PD~; no parameter names are
  added. The names of V~T+~, V~T-~ and ΔV~T~ are printed in its test
  conditions column and are under Parameter here.
- The source's chapters 9 (Power Supply Recommendations) and 10
  (Layout) are under Application information here.
- Typical characteristics: Figures 5-1 and 5-2 (p.6) digitized to about
  2 % of full scale.
- The logic diagram, the clamp diode figure (Figure 7-1; the shared
  figure has output diodes to both rails and does not match), the load
  circuit and voltage waveforms (Figures 6-1 and 6-2), the typical
  application block diagram (Figure 8-1) and the application timing
  diagram (Figure 8-2) are described in text.
- From images: overbar of Y = !{A} (p.8); the curve points of Figures
  5-1 and 5-2 (p.6); V~t~, 10% V~CC~, 50% and 10% (measurement points
  of Figure 6-2, p.7); +I~IK~, -I~IK~ and -I~OK~ (Figure 7-1, p.9); τ
  as the time unit of Figure 8-2 (p.13; the text layer has "Time (").
- Source issues:
  - Source conflict: the page headers and footers print SN74ACT16-Q1;
    the title, the text and the package option addendum (p.17, which
    lists SN74ACT16-Q1 as the automotive version) name the SN74ACT16.
  - Source error: the first sentence of the input considerations (p.12)
    is printed without the two levels the input signals must cross;
    kept as printed.
