---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act08-q1",
  "title": "SN74ACT08-Q1 automotive quadruple 2-input AND gates with TTL-compatible inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74act08-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS993B",
    "date": "2025-02",
    "pages": 30
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74ACT08PWRQ1", "SN74ACT08WBQARQ1"],
  "packages": {"p14": "PW, BQA"},
  "converted": "2026-11-08"
}
---

# SN74ACT08-Q1 automotive quadruple 2-input AND gates with TTL-compatible inputs

## Description

The SN74ACT08-Q1 contains four independent 2-input AND gates. Each gate
performs the Boolean function Y = A · B in positive logic. [[1::2]] [[8::1]]

AEC-Q100 qualified for automotive applications. Device temperature
grade 1: –40 °C to +125 °C. Operating voltage range of 4.5 V to 5.5 V.
TTL-compatible inputs. Continuous ±24 mA output drive at 5 V. Supports
up to ±75 mA output drive at 5 V in short bursts. [[1::1]] [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel 1–4, Input A
1B, 2B, 3B, 4B: Channel 1–4, Input B
1Y, 2Y, 3Y, 4Y: Channel 1–4, Output Y
GND: Ground
VCC: Positive Supply
```
[[1::table #1]]

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. BQA package only. [[3::1]]

## Functional description

Conversion note: the functional block diagram shows one 2-input AND gate
per channel, inputs xA and xB, output xY.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due
to overcurrent. The electrical and thermal limits defined in the
[absolute maximum ratings](#absolute-maximum-ratings) must be followed
at all times. Unused push-pull CMOS outputs must be left disconnected. [[8::2]] [[8::3]]

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold. TTL-compatible CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the
[electrical characteristics](#electrical-characteristics). The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I). [[8::4]] [[8::5]]

TTL-compatible CMOS inputs require that input signals transition
between valid logic states quickly, as defined by the input transition
time or rate in the
[recommended operating conditions](#recommended-operating-conditions).
Failing to meet this specification will result in excessive power
consumption and could cause oscillations. Do not leave TTL-compatible
CMOS inputs floating at any time during operation. Unused inputs must
be terminated at V~CC~ or GND. If a system will not be actively driving
an input at all times, a pull-up or pull-down resistor can be added to
provide a valid input voltage during these times. The resistor value
will depend on multiple factors; however, a 10kΩ resistor is
recommended and typically will meet all requirements. [[8::6]] [[8::7]]

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes. Voltages beyond the values specified in the Absolute
Maximum Ratings table can cause damage to the device. The input and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed. [[8::8]] [[8::9]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 7-1]]

## Function tables

```table id=function caption="Function table [1]"
| Inputs | < | Output |
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
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < -0.5 V or V~I~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous output current through V~CC~ or GND | | | ±200 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002, HBM ESD Classification Level 2 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011, CDM ESD Classification Level C4B | ±1000 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 4.5 V ≤ V~CC~ ≤ 5.5 V | 2 | | V |
| V~IL~ | Low-level input voltage | 4.5 V ≤ V~CC~ ≤ 5.5 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | V~CC~ | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | 4.5 V ≤ V~CC~ ≤ 5.5 V | | -24 | mA |
| I~OL~ | Low-level output current | 4.5 V ≤ V~CC~ ≤ 5.5 V | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | 4.5 V ≤ V~CC~ ≤ 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74ACT08-Q1 [1]"
| Symbol | Parameter | BQA (WQFN) 14 pins | PW (TSSOP) 14 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 91.3 | 145.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 99.4 | 76.5 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 61.0 | 102.0 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 14.5 | 18.8 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 60.8 | 100.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | 37.0 | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[4::table #4]] [[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | –40 °C to 125 °C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.49 | | 4.4 | | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.49 | | 5.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.94 | | | 3.7 | | | V |
| ^ | ^ | I~OH~ = -24 mA | 5.5 V | 4.86 | | | 4.7 | | | V |
| ^ | ^ | I~OH~ = -50 mA | 5.5 V | | | | 3.85 | | | V |
| ^ | ^ | I~OH~ = -75 mA | 5.5 V | | | | 3.85 | | | V |
| V~OL~ | | I~OL~ = 50 µA [3] | 4.5 V | | 0.001 | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA [3] | 4.5 V | | | 0.36 | | | 0.5 | V |
| ^ | ^ | I~OL~ = 24 mA [3] | 5.5 V | | | 0.36 | | | 0.5 | V |
| ^ | ^ | I~OL~ = 50 mA [3] | 5.5 V | | | | | | 1.65 | V |
| ^ | ^ | I~OL~ = 75 mA [3] | 5.5 V | | | | | | 1.65 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | | 80 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 2.1 V; any input | 4.5 V to 5.5 V | | 0.6 | 1.2 | | | 1.5 | mA |
| C~I~ | | V~I~ = V~CC~ or GND | 5 V | | 8 | | | | | pF |
| C~PD~ [1] [2] | | C~L~ = 50 pF, F = 1 MHz | 5 V | | 14 | | | | | pF |

[1] C~PD~ is used to determine the dynamic power consumption, per channel.
[2] P~D~ = V~CC~^2^ × F~I~ × (C~PD~ + C~L~) where F~I~ = input frequency, C~L~ = output load capacitance, V~CC~ = supply voltage.
[3] Source error: the source labels the V~OL~ test currents I~OH~ (I~OH~ = 50 µA, 24 mA, 50 mA, 75 mA), while the V~OH~ rows of the same table give I~OH~ as negative (sourcing) currents and the recommended operating conditions give the 24 mA sink current as I~OL~, so they are written I~OL~ here.
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF; over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25 °C | < | < | –40 °C to 125 °C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 5 V | 1 | 6.5 | 9 | 1 | | 10 | ns |
| t~PHL~ | A or B | Y | 5 V | 1 | 6.5 | 9 | 1 | | 10 | ns |
| t~r~ | | Y | 5 V | 2.4 | | 3.9 | 3 | | 4.6 | ns |
| t~f~ | | Y | 5 V | 2.3 | | 3.4 | 2.6 | | 4.1 | ns |
```
[[5::table #3]]

## Typical characteristics

T~A~ = 25°C (unless otherwise noted) [[6::1]]

```curve id=icc-vcc caption="Supply current across supply voltage"
{"x": {"label": "V_CC", "unit": "V", "range": [0.5, 5.5]},
 "y": {"label": "I_CC", "unit": "nA", "range": [0, 100]},
 "series": [
   {"label": "25 °C", "points": [[0.5, 11], [1, 14.5], [1.5, 18.5], [2, 21.5], [2.5, 24], [3, 26.5], [3.5, 28.5], [4, 30], [4.25, 31], [4.5, 29.5], [5, 28], [5.25, 29], [5.5, 32]]},
   {"label": "125 °C", "points": [[0.5, 28], [0.7, 31.5], [1, 32], [1.5, 39], [2, 37], [2.1, 41], [2.5, 38.5], [3, 42], [3.3, 36], [3.5, 42.5], [4, 39], [4.25, 36], [4.5, 41], [5, 38], [5.1, 42.5], [5.2, 55.5], [5.3, 54], [5.5, 73]]},
   {"label": "-40 °C", "points": [[0.5, 18], [1, 26.5], [1.5, 35], [2, 42.5], [2.5, 49.5], [3, 56], [3.5, 62.5], [4, 69], [4.5, 72.5], [5, 77], [5.1, 74], [5.5, 86]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=voh-ioh caption="Output voltage vs current in HIGH state; 5V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-50, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.24, 5.04]},
 "series": [
   {"label": "25 °C", "points": [[-50, 4.43], [-25, 4.71], [0, 4.99]]},
   {"label": "125 °C", "points": [[-50, 4.33], [-25, 4.66], [0, 4.99]]},
   {"label": "-40 °C", "points": [[-50, 4.52], [-25, 4.76], [0, 4.99]]}
 ]}
```
[[6::Figure 5-2]]

```curve id=vol-iol caption="Output voltage vs current in LOW state; 5V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 50]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.4]},
 "series": [
   {"label": "25 °C", "points": [[0, 0], [25, 0.145], [50, 0.29]]},
   {"label": "125 °C", "points": [[0, 0], [25, 0.18], [50, 0.36]]},
   {"label": "-40 °C", "points": [[0, 0], [25, 0.12], [50, 0.24]]}
 ]}
```
[[6::Figure 5-3]]

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1MHz, Z~O~ = 50Ω, t~t~ < 2.5ns, V~t~ = 1.5V. For
push-pull outputs, R~L~ = 500Ω. The outputs are measured individually
with one input transition per measurement. C~L~ includes probe and
test-fixture capacitance. The greater between t~PLH~ and t~PHL~ is the
same as t~pd~. The greater between t~r~ and t~f~ is the same as t~t~. [[7::1]] [[7::2]] [[7::3]] [[7::5]] [[7::7]]

```figure ref=load-rl-cl caption="Load circuit for push-pull outputs"
RL: 500 Ω
CL: 50 pF
```
[[7::Figure 6-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and transition times"
VM: V~t~ = 1.5 V (input), 50 % (outputs)
```
[[7::Figure 6-2]]

## Application information

Information in the following applications sections is not part of the
TI component specification. [[10::1]]

### Typical application

Three 2-input AND gates are combined to produce a 4-input AND gate
function. The fourth gate can be used for another application in the
system, or the inputs can be grounded and the channel left unused. This
device is used to directly control the RESET pin of a motor controller.
The controller requires four input signals to all be HIGH before being
enabled, and should be disabled in the event that any one signal goes
LOW. The 4-input AND gate function combines the four individual reset
signals into a single active-low reset signal. [[10::2]] [[10::3]]

Conversion note: in the typical application schematic the power supply's
PG, the over-current detection's !{OC}, the on/off switch and the
over-temperature detection's !{OT} drive the gates, and the result drives
the motor controller's !{RESET}.

```wavedrom id=app-timing caption="Typical application timing diagram"
{"signal": [
  {"name": "OC#", "wave": "10101010"},
  {"name": "PG", "wave": "1.0.1.0."},
  {"name": "ON/OFF#", "wave": "1..0.1.."},
  {"name": "OT#", "wave": "1...0..."},
  {"name": "RESET#", "wave": "10......"}
]}
```
[[13::Figure 8-2]]

### Power considerations

Ensure the desired supply voltage is within the
range specified in the
[recommended operating conditions](#recommended-operating-conditions).
The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the SN74ACT08-Q1
plus the maximum static supply current, I~CC~, listed in the
[electrical characteristics](#electrical-characteristics), and any
transient current required for switching. Ensure the maximum total
current through V~CC~ listed in the
[absolute maximum ratings](#absolute-maximum-ratings) is not exceeded.
The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74ACT08-Q1 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. Ensure the maximum total
current through GND listed in the Absolute Maximum Ratings is not
exceeded.
The SN74ACT08-Q1 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50pF. The SN74ACT08-Q1 can drive a load with total resistance
described by R~L~ ≥ V~O~ / I~O~, with the output voltage and current
defined in the Electrical Characteristics table with V~OH~ and V~OL~.
When outputting in the HIGH state, the output voltage in the equation
is defined as the difference between the measured output voltage and
the supply voltage at the V~CC~ pin. The maximum junction temperature,
T~J(max)~ listed in the Absolute Maximum Ratings, is an additional
limitation to prevent damage to the device. Do not violate any values
listed in the Absolute Maximum Ratings. [[11::1]] [[11::2]] [[11::3]] [[11::4]] [[11::5]] [[11::8]]

### Input considerations

Input signals must cross V~IL(max)~ to be
considered a logic LOW, and V~IH(min)~ to be considered a logic HIGH.
Do not exceed the maximum input voltage range found in the Absolute
Maximum Ratings. Unused inputs must be terminated to either V~CC~ or
ground. The unused inputs can be directly terminated if the input is
completely unused, or they can be connected with a pull-up or pull-down
resistor if the input will be used sometimes, but not always. The drive
current of the controller, leakage current into the SN74ACT08-Q1, and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors. The SN74ACT08-Q1 has
CMOS inputs and thus requires fast input transitions to operate
correctly, as defined in the Recommended Operating Conditions table.
Slow input transitions can cause oscillations, additional power
consumption, and reduction in device reliability. [[12::1]] [[12::2]] [[12::3]]

### Output considerations

Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
Sinking current into the output will increase the output voltage as
specified by the V~OL~ specification in the Electrical Characteristics. Push-pull outputs that could be in opposite
states, even for a very short time period, should never be connected
directly together. This can cause excessive current and damage to the
device. Two channels within the same device with the same input signals
can be connected in parallel for additional output drive strength.
Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[12::5]] [[12::6]] [[12::7]] [[12::8]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. [[13::1]]
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. [[13::1]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. [[13::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and C~pd~ Calculation. [[13::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum
and maximum supply voltage rating located in the Recommended Operating
Conditions. Each V~CC~ terminal should have a good bypass capacitor to
prevent power disturbance. A 0.1µF capacitor is recommended for this
device. It is acceptable to parallel multiple bypass capacitors to
reject different frequencies of noise. The 0.1µF and 1µF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results. [[13::3]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices,
inputs must never be left floating. Such unused input pins must not be
left unconnected because the undefined voltages at the outside
connections result in undefined operational states. All unused inputs
of digital logic devices must be connected to a logic high or logic low
voltage, as defined by the input voltage specifications, to prevent
them from floating. The logic level that must be applied to any
particular unused input depends on the function of the device.
Generally, the inputs are tied to GND or V~CC~, whichever makes more
sense for the logic function or is more convenient. [[13::4]]

## Package

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74ACT08PWRQ1 | Reel of 3000 | -40 to 125 | ACT08Q
SN74ACT08WBQARQ1 | Reel of 3000 | -40 to 125 | AD08Q
```
[[21::table #1]]

## Conversion notes

- Source pages 1–14 converted; support pages, revision history, tape
  and reel and mechanical pages, and the package option addendum
  (15–30) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are the parts' rows of the package option
  addendum (p.21).
- The electrical characteristics table of the source gives only symbols
  in its Parameter column; no parameter names are added.
- Dropped: features and applications lists, package information table,
  the layout example figure, and cross-references to other sections.
- The typical characteristics curves are digitized from Figures 5-1,
  5-2 and 5-3 (p.6) to about 2 % of full scale.
- In the application timing diagram the source overlines only OFF in
  ON/OFF; the WaveDrom name `ON/OFF#` overlines the whole name.
- The functional block diagram and the typical application schematic
  (Figure 8-1) are described in text; the application timing diagram is
  redrawn from Figure 8-2.
- C~L~ = 50 pF for the load figure is from the switching
  characteristics conditions; the load figure itself gives no value.
- Source issues:
  - Source error: V~OL~ test currents printed as I~OH~ (see the
    electrical characteristics table note).
