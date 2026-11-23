---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs08-q1",
  "title": "SN74HCS08-Q1 automotive quadruple 2-input AND gates with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74hcs08-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS756F",
    "date": "2026-10",
    "pages": 33
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74HCS08QBQARQ1", "SN74HCS08QDRQ1", "SN74HCS08QPWRQ1"],
  "packages": {"p14": "D, PW, DYY, BQA"},
  "converted": "2026-11-08"
}
---

# SN74HCS08-Q1 automotive quadruple 2-input AND gates with Schmitt-trigger inputs

## Description

This device contains four independent 2-input AND gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = A · B in positive logic. [[1::3]]

Schmitt-trigger inputs allow for slow or noisy input signals. This
device includes balanced CMOS push-pull outputs. [[1::5]] [[8::2]]

## Pinout

```pinout pinout=p14
```

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. Thermal Pad, BQA Package only. [[3::1]]

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel 1–4, input A
1B, 2B, 3B, 4B: Channel 1–4, input B
1Y, 2Y, 3Y, 4Y: Channel 1–4, output Y
GND: Ground
VCC: Positive supply
```
[[3::Table 4-1]]

## Functional description

Balanced CMOS push-pull outputs: the device can sink and source similar
currents. The drive capability of this device can create fast edges into
light loads, so routing and load conditions must be considered to
prevent ringing. Additionally, the outputs of this device are capable of
driving larger currents than the device can sustain without being
damaged. Limit the output power of the device to avoid damage due to
overcurrent. The electrical and thermal limits defined in the
[absolute maximum ratings](#absolute-maximum-ratings) must be followed
at all times. Unused push-pull CMOS outputs must be left disconnected. [[8::2]] [[8::3]]

CMOS Schmitt-trigger inputs: these inputs are high impedance and are
typically modeled as a resistor in parallel with the input capacitance
given in the [Electrical Characteristics](#electrical-characteristics)
table from the input to ground. The worst case resistance is calculated
with the maximum input voltage, given in the
[Absolute Maximum Ratings](#absolute-maximum-ratings) table, and the
maximum input leakage current, given in the Electrical Characteristics
table, using Ohm's law (R = V ÷ I). The Schmitt-trigger input
architecture provides hysteresis as defined by ΔV~T~ in the Electrical
Characteristics table, which makes this device extremely tolerant to
slow or noisy inputs. While the inputs can be driven much slower than
standard CMOS inputs, properly terminating unused inputs is still
recommended. Driving the inputs with slow transitioning signals
increases dynamic current consumption of the device. [[8::4]] [[8::5]]

Clamp diodes: the inputs and outputs have both positive and negative
clamping diodes. Voltages beyond the values specified in the absolute
maximum ratings can cause damage to the device. The input and output
voltage ratings can be exceeded if the input and output clamp-current
ratings are observed. [[8::6]] [[8::7]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 7-1]]

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don't Care
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 or V~I~ > V~CC~ + 0.5 | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 or V~O~ > V~CC~ + 0.5 | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±70 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings can be exceeded if the input and output current ratings are observed. Do not exceed the absolute maximum voltage supply rating.
[3] Verified by design.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1]; HBM ESD Classification Level 2 | ±4000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011; CDM ESD Classification Level C6 | ±1500 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | 2 | | 6 | V |
| V~I~ | Input voltage | 0 | | V~CC~ | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise and fall rate [1] | | | | ns/V |
| T~A~ | Ambient temperature | -40 | | 125 | °C |

[1] Max: Unlimited.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information (SN74HCS08-Q1) [1]"
| Symbol | Parameter | PW (TSSOP), 14 pins | D (SOIC), 14 pins | BQA (WQFN), 14 pins | DYY (SOT), 14 pins | Unit |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 151.7 | 133.6 | 109.7 | 236.5 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 79.4 | 89.0 | 111.0 | 143.2 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 89.5 | 77.9 | 146.0 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 25.2 | 45.5 | 20.2 | 29.5 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.1 | 89.1 | 77.8 | 145.6 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | 56.6 | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive switching threshold | | < | 2 V | 1.13 | 1.26 | 1.37 | V |
| ^ | ^ | ^ | ^ | 4.5 V | 2.31 | 2.47 | 2.66 | V |
| ^ | ^ | ^ | ^ | 6 V | 2.99 | 3.18 | 3.44 | V |
| V~T-~ | Negative switching threshold | | < | 2 V | 0.55 | 0.62 | 0.71 | V |
| ^ | ^ | ^ | ^ | 4.5 V | 1.21 | 1.36 | 1.5 | V |
| ^ | ^ | ^ | ^ | 6 V | 1.56 | 1.79 | 1.99 | V |
| ΔV~T~ | V~T+~ - V~T-~ | | < | 2 V | 0.54 | 0.64 | 0.71 | V |
| ^ | ^ | ^ | ^ | 4.5 V | 0.94 | 1.11 | 1.27 | V |
| ^ | ^ | ^ | ^ | 6 V | 1.20 | 1.38 | 1.57 | V |
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = -20 µA | 2 V to 6 V | V~CC~ - 0.1 | V~CC~ - 0.002 | | V |
| ^ | ^ | ^ | I~OH~ = -6 mA | 4.5 V | 4.0 | 4.3 | | V |
| ^ | ^ | ^ | I~OH~ = -7.8 mA | 6 V | 5.4 | 5.75 | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 2 V to 6 V | | 0.002 | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4 mA | 4.5 V | | 0.18 | 0.30 | V |
| ^ | ^ | ^ | I~OL~ = 7.8 mA | 6 V | | 0.22 | 0.33 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ or 0 | < | 6 V | | ±100 | ±1000 | nA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0, I~O~ = 0 | < | 6 V | | 0.1 | 2 | µA |
| C~i~ | Input capacitance | | < | 2 V to 6 V | | | 5 | pF |
```
[[5::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | From | To | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | 2 V | | 14 | 36 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 7 | 13 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 6 | 12 | ns |
| t~t~ | Transition-time | | Y | 2 V | | 9 | 17 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 5 | 8 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 4 | 7 | ns |
```
[[6::table #1]]

## Typical characteristics

```curve id=rol caption="Output driver resistance in LOW state, T~A~ = 25 °C"
{"x": {"label": "Output sink current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [26, 46]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 41.5], [1.25, 41.6], [2.5, 42.0], [3.75, 42.4], [6.25, 43.6], [8.75, 45.0], [9.75, 45.8]]},
  {"label": "VCC = 3.3 V", "points": [[0, 31.35], [5, 31.6], [10, 31.9], [15, 32.25], [20, 32.8], [25, 33.5]]},
  {"label": "VCC = 4.5 V", "points": [[0, 28.5], [5, 28.5], [10, 28.65], [15, 28.75], [20, 28.95], [25, 29.15]]},
  {"label": "VCC = 6 V", "points": [[0, 27.0], [5, 27.0], [10, 27.05], [15, 27.15], [20, 27.25], [25, 27.3]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=roh caption="Output driver resistance in HIGH state, T~A~ = 25 °C"
{"x": {"label": "Output source current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [30, 70]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 54.1], [1.25, 54.4], [3.75, 56.6], [6.25, 59.4], [8.75, 63.3], [10, 66.0]]},
  {"label": "VCC = 3.3 V", "points": [[0, 39.0], [5, 39.6], [10, 40.4], [15, 41.2], [20, 42.4], [25, 43.9]]},
  {"label": "VCC = 4.5 V", "points": [[0, 34.2], [5, 34.4], [10, 34.7], [15, 35.0], [20, 35.4], [25, 35.8]]},
  {"label": "VCC = 6 V", "points": [[0, 31.2], [5, 31.35], [10, 31.5], [15, 31.6], [20, 31.8], [25, 31.95]]}
 ]}
```
[[6::Figure 5-2]]

Conversion note: the source also plots supply current against input
voltage (V~CC~ = 2 V to 6 V): the typical additional current caused by
holding an input at a value other than V~CC~ or ground.

## Parameter measurement information

Phase relationships between waveforms are selected arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 2.5 ns. The outputs
are measured one at a time with one input transition per measurement.
C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. The greater between t~r~ and
t~f~ is the same as t~t~. [[7::1]] [[7::3]] [[7::4]] [[7::6]] [[7::8]]

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: not given
```
[[7::Figure 6-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Propagation delays and input and output transition times"
VM: 50 %
```
[[7::Figure 6-3]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. [[10::1]]

In this application, three 2-input AND gates are combined to produce a
4-input AND gate function. The fourth gate can be used for another
application in the system, or the inputs can be grounded and the
channel left unused. The SN74HCS08-Q1 is used to directly control the
!{RESET} pin of a motor controller. The controller requires four input
signals to all be HIGH before being enabled, and should be disabled in
the event that any one signal goes LOW. The 4-input AND gate function
combines the four individual reset signals into a single active-low
reset signal. Many power good signals use open-drain outputs which can
produce slow input transition rates when they transition from LOW to
Hi-Z mode. This makes the SN74HCS08-Q1 an excellent choice for the
application because it has Schmitt-trigger inputs that do not have input
transition rate requirements.
Conversion note: Figure 8-1 (p.10) names the four signals !{OC}, PG, ON/!{OFF} and !{OT}, from blocks labelled Over Current Detection, Power Supply, On/Off Switch and Over Temp Detection, and the output !{RESET}. [[10::2]] [[10::3]] [[10::4]]

```wavedrom id=app-timing caption="Application timing diagram"
{"signal": [
  {"name": "OC#", "wave": "10101010"},
  {"name": "PG", "wave": "1.0.1.0."},
  {"name": "ON/OFF#", "wave": "1..0.1.."},
  {"name": "OT#", "wave": "1...0..."},
  {"name": "RESET#", "wave": "10......"}
]}
```
[[12::Figure 8-2]]

Power: verify that the desired supply voltage is within the range
specified in the [Electrical Characteristics](#electrical-characteristics).
The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74HCS08-Q1 plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching.
Verify that the maximum total current through V~CC~ listed in the
Absolute Maximum Ratings is not exceeded. The ground must be capable of
sinking current equal to the total current to be sunk by all outputs of
the SN74HCS08-Q1 plus the maximum supply current, I~CC~, listed in the
Electrical Characteristics, and any transient current required for
switching. Verify that the maximum total current through GND listed in
the Absolute Maximum Ratings is not exceeded. The maximum junction
temperature, T~J~(max) listed in the Absolute Maximum Ratings, is an
additional limitation to prevent damage to the device. Do not violate
any values listed in the Absolute Maximum Ratings. Each V~CC~
terminal must have a good bypass capacitor to prevent power disturbance.
A 0.1 µF capacitor is recommended for this device. The
0.1 µF and 1 µF capacitors are commonly used in parallel. The bypass
capacitor must be installed as close to the power terminal as
possible. [[10::6]] [[10::7]] [[10::8]] [[11::5]] [[12::2]]

Loads: the SN74HCS08-Q1 can drive a load with a total capacitance less
than or equal to 50 pF while still meeting all of the datasheet
specifications. Larger capacitive loads can be applied; however, TI
recommends adding a series resistor between the output and capacitor to
prevent excessive current. The SN74HCS08-Q1 can drive a load with total
resistance described by R~L~ ≥ V~O~ / I~O~, with the output voltage and
current defined in the Electrical Characteristics table with V~OH~ and
V~OL~. When outputting in the HIGH state, the output voltage in the
equation is defined as the difference between the measured output
voltage and the supply voltage at the V~CC~ pin. Verify that the
resistive load at the output is larger than (V~CC~ / I~O(max)~) Ω.
Never violate the maximum output
current from the absolute maximum ratings. [[11::1]] [[11::2]] [[12::1]]

Inputs: input signals must cross V~T-~(min) to be considered a logic
LOW, and V~T+~(max) to be considered a logic HIGH. Do not exceed the
maximum input voltage range found in the absolute maximum ratings.
Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
the inputs can be connected with a pullup or pulldown resistor if the
input is used sometimes, but not always. A 10 kΩ resistor value is
often used. Noise with a large enough amplitude can still cause issues.
To know how much noise is too much, see the ΔV~T~(min) in the Electrical
Characteristics. This hysteresis value provides the peak-to-peak limit.
Schmitt-trigger inputs can
be held at any valid value without causing huge increases in power
consumption. [[11::6]] [[11::7]] [[11::9]] [[11::10]]

Outputs: push-pull outputs that can be in opposite states, even for a
very short time period, must never be connected directly together. Two
channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. Unused
outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[11::13]] [[11::14]] [[11::15]]

Layout: inputs must never be left floating. Such unused input pins must
not be left unconnected because the undefined voltages at the outside
connections result in undefined operational states. All unused inputs
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or V~CC~,
whichever makes more sense for the logic function or is more
convenient. Conversion note: the source's layout example uses a GND flood fill, places
the bypass capacitor close to the device, ties unused inputs to V~CC~,
leaves the unused output floating, and avoids 90° corners for signal
lines. [[12::3]]

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74HCS08QBQARQ1 | Reel of 3000 | -40 to 125 | HCS08Q
SN74HCS08QDRQ1 | Reel of 2500 | -40 to 125 | HCS08Q1
SN74HCS08QPWRQ1 | Reel of 2000 | -40 to 125 | HCS08Q
```
[[16::table #1]]

## Conversion notes

- Source pages 1–15 converted; the package option addendum, tape and
  reel and mechanical pages (16–33) dropped, replaced by the generated
  Package and Ordering sections (packing, temperature range and marking
  taken from the addendum).
- Dropped: features and applications lists, the "Benefits of
  Schmitt-trigger inputs" figure, revision history, support pages. The
  typical application block diagram (Figure 8-1) and the layout example
  (Figure 8-3) are described in text; the application timing diagram
  (Figure 8-2) is redrawn as WaveDrom.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
  The supply current curves (Figures 5-3 and 5-4) are not digitized.
- The source's load circuit (Figure 6-1) gives no value for C~L~, so
  the load figure says "CL: not given".
- From images: the curve points in Figures 5-1 and 5-2 (p.6).
- The source's package option addendum also lists SN74HCS08QDYYRQ1
  (SOT-23-THIN, DYY), which the catalog doesn't carry.
- Source issues: none found.
