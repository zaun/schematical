---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act11-q1",
  "title": "SN74ACT11-Q1 automotive triple 3-input AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74act11-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCASE76",
    "date": "2025-02",
    "pages": 25
  },
  "devices": ["gate:74_11"],
  "parts": ["SN74ACT11PWRQ1", "SN74ACT11WBQARQ1"],
  "packages": {"p14": "PW, BQA"},
  "converted": "2026-11-08"
}
---

# SN74ACT11-Q1 automotive triple 3-input AND gates

## Description

The SN74ACT11-Q1 device contains three independent 3-input AND gates.
These devices perform the Boolean function Y = A · B · C in positive
logic. It is AEC-Q100 qualified for automotive applications (device
temperature grade 1: –40 °C to +125 °C), operates from 4.5 V to 5.5 V,
has TTL-compatible inputs, and drives ±24 mA continuous, or up to
±75 mA in short bursts, at 5 V. It is available in a wettable flank QFN
package.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A: Channel 1–3, Input A
1B, 2B, 3B: Channel 1–3, Input B
1C, 2C, 3C: Channel 1–3, Input C
1Y, 2Y, 3Y: Channel 1–3, Output Y
GND: Ground
VCC: Positive Supply
```

Thermal pad (BQA package only): the thermal pad can be connected to GND
or left floating. Do not connect to any other signal or supply.

Source conflict: the source's pin table names pin 9 3C and pin 11 3A,
but describes pin 9 as "Channel 3, Input A" and pin 11 as "Channel 3,
Input C". The catalog pinout used here has 3A on pin 9 and 3C on pin
11. The three inputs of each gate perform the same function.

## Functional description

Functional block diagram: one 3-input AND gate per channel, inputs xA,
xB and xC, output xY.

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
at all times. Unused push-pull CMOS outputs must be left disconnected.

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold. TTL-compatible CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the
[electrical characteristics](#electrical-characteristics). The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I).

TTL-compatible CMOS inputs require that input signals transition
between valid logic states quickly, as defined by the input transition
time or rate in the
[recommended operating conditions](#recommended-operating-conditions).
Failing to meet this specification will result in excessive power
consumption and could cause oscillations. Do not leave TTL-compatible
CMOS inputs floating at any time during operation. Unused inputs must
be terminated at V~CC~ or GND. If a system will not be actively driving
an input at all times, a pull-up or pull-down resistor can be added to
provide a valid input voltage during these times; a 10kΩ resistor is
recommended and typically will meet all requirements.

### Wettable flanks

This device includes wettable flanks for at least one package.
Wettable flanks help improve side wetting after soldering, which makes
QFN packages easier to inspect with automatic optical inspection (AOI).
A wettable flank can be dimpled or step-cut to provide additional
surface area for solder adhesion which assists in reliably creating a
side fillet. See the mechanical drawing for additional details.

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes. Voltages beyond the values specified in the Absolute
Maximum Ratings table can cause damage to the device. The input and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

## Function tables

```table id=function caption="Function table (each gate)"
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
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < -0.5 V or V~I~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous output current through V~CC~ or GND | | | ±200 | mA |
| T~J~ | Operating junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002, HBM ESD Classification Level 2 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011, CDM ESD Classification Level C4B | ±1000 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordrance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -50 | mA |
| I~OL~ | Low-level output current | | 50 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Package | Pins | R~θJA~ | R~θJC(top)~ | R~θJB~ | Ψ~JT~ | Ψ~JB~ | R~θJC(bot)~ | Unit |
|---|--:|--:|--:|--:|--:|--:|--:|---|
| PW (TSSOP) | 14 | 145.7 | 76.5 | 102.0 | 18.8 | 100.7 | | °C/W |
| BQA (WQFN) | 14 | 91.3 | 99.4 | 61.0 | 14.5 | 60.8 | 37.0 | °C/W |

[1] R~θJC(bot)~ is printed "-" for PW. For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted); typical at T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ | –40 °C to 125 °C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.499 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.499 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.7 | 4.16 | | V |
| ^ | ^ | I~OH~ = -24 mA | 5.5 V | 4.7 | 5.18 | | V |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | 3.85 | | | V |
| V~OL~ | Low-level output voltage [2] | I~OL~ = 50 µA | 4.5 V | | 0.001 | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | 0.16 | 0.5 | V |
| ^ | ^ | I~OL~ = 24 mA | 5.5 V | | 0.14 | 0.5 | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | 1.65 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | ±0.03 | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | 0.08 | 1 | µA |
| ΔI~CC~ | Supply-current change | V~I~ = V~CC~ – 2.1 V; any input | 4.5 V to 5.5 V | | | 100 | µA |
| C~I~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | | 2 | pF |
| C~PD~ | Power dissipation capacitance | C~L~ = 50 pF, F = 1 MHz | 5 V | | | 20 | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 2 ms.
[2] Source error: the source labels the V~OL~ test currents I~OH~ (I~OH~ = 50 µA, 24 mA, 75 mA). The V~OH~ rows of the same table give I~OH~ as negative (sourcing) currents; written I~OL~.
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF; over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | V~CC~ | –40 °C to 125 °C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | A, B, or C | Y | C~L~ = 50 pF | 5 V | | 5 | 8 | ns |
| t~PHL~ | A, B, or C | Y | C~L~ = 50 pF | 5 V | | 5.2 | 7.8 | ns |
| t~r~ | | Y | C~L~ = 50 pF | 5 V | | 2.8 | 4.7 | ns |
| t~f~ | | Y | C~L~ = 50 pF | 5 V | | 2.6 | 4.1 | ns |
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1MHz, Z~O~ = 50Ω, t~t~ < 2.5ns, V~t~ = 1.5V. For
push-pull outputs, R~L~ = 500Ω. The outputs are measured individually
with one input transition per measurement. C~L~ includes probe and
test-fixture capacitance. The greater between t~PLH~ and t~PHL~ is the
same as t~pd~; the greater between t~r~ and t~f~ is the same as t~t~.

```figure ref=load-rl-cl caption="Load circuit for push-pull outputs"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and transition times"
VM: V~t~ = 1.5 V (input), 50 % (outputs)
```

## Application information

Information in this section is not part of the TI component
specification.

### Typical application

In this application, this device is used to directly control the RESET
pin of a motor controller. The controller requires three input signals
to all be HIGH before being enabled, and should be disabled in the event
that any one signal goes LOW. The 3-input AND gate function combines
the three individual reset signals into a single active-low reset
signal. Typical application schematic: the power supply's PG output,
the over-current detection's !{OC} output and an on/off switch drive
the three inputs of one gate; its output drives the motor controller's
!{RESET}.

```wavedrom id=app-timing caption="Typical application timing diagram"
{"signal": [
  {"name": "PG", "wave": "01...0..1."},
  {"name": "OC#", "wave": "0.1...0.1."},
  {"name": "ON/OFF#", "wave": "0..1......"},
  {"name": "RESET#", "wave": "0..1.0..1."}
]}
```

Power considerations: ensure the desired supply voltage is within the
range specified in the
[recommended operating conditions](#recommended-operating-conditions).
The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the device (Source error: the
source names the SN74HC11 here) plus the maximum supply current, I~CC~, listed in
the [electrical characteristics](#electrical-characteristics). The logic
device can only source or sink as much current as it is provided at the
supply and ground pins, respectively. Be sure not to exceed the maximum
total current through GND or V~CC~ listed in the
[absolute maximum ratings](#absolute-maximum-ratings). The maximum
junction temperature, T~J(max)~ listed in the Absolute Maximum Ratings,
is an additional limitation to prevent damage to the device. Do
not violate any values listed in the Absolute Maximum Ratings.

Input considerations: unused inputs must be terminated to either V~CC~
or ground. These can be directly terminated if the input is completely
unused, or they can be connected with a pull-up or pull-down resistor
if the input is to be used sometimes, but not always. The resistor size
is limited by drive current of the controller, leakage current into the
device, and the desired input transition rate. A 10-kΩ resistor value
is often used due to these factors. The source states that the device
has standard CMOS inputs, so input signal edge rates cannot be slow
(Source conflict: the description and the Feature Description give
TTL-compatible CMOS inputs). Slow input edge rates can cause
oscillations and damaging shoot-through current. The recommended rates
are defined in the
[recommended operating conditions](#recommended-operating-conditions).

Output considerations: drawing current from the output will decrease
the output voltage as specified by the V~OH~ specification; sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification. Unused outputs can be left floating. Do not
connect outputs directly to V~CC~ or ground.

Detailed design procedure:

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins.
2. Ensure the capacitive load at the output is ≤ 70 pF. This is not a
   hard limit, however it will ensure optimal performance.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. This will ensure that the maximum output
   current from the Absolute Maximum Ratings is not violated.
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application report, CMOS Power Consumption and
   C~pd~ Calculation.

Power supply: the power supply can be any voltage between the minimum
and maximum supply voltage rating located in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ terminal should have a bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise; the 0.1-µF and 1-µF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results.

Layout guidelines:

- Bypass capacitor placement: place near the positive supply terminal
  of the device; provide an electrically short ground return path; use
  wide traces to minimize impedance; keep the device, capacitors, and
  traces on the same side of the board whenever possible.
- Signal trace geometry: 8mil to 12mil trace width; lengths less than
  12cm to minimize transmission line effects; avoid 90° corners for
  signal traces; use an unbroken ground plane below signal traces;
  flood fill areas around signal traces with ground.
- For traces longer than 12cm: use impedance controlled traces;
  source-terminate using a series damping resistor near the output
  (22 Ω in the source's example); avoid branches; buffer signals that
  must branch separately.

## Package

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
```

## Conversion notes

- Source pages 1–12 converted; support pages, revision history, tape
  and reel and mechanical pages, and the package option addendum
  (13–25) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists, device information table,
  the wettable-flank cutaway figure, the layout figures (trace corners,
  bypass capacitor placement, damping resistor; summarized in the layout
  guidelines), cross-references to other sections.
- The functional block diagram and the typical application schematic
  (Figure 8-1) are described in text; the application timing diagram is
  redrawn from Figure 8-2.
- C~L~ = 50 pF for the load figure is from the switching
  characteristics conditions; the load figure itself gives no value.
- From images: 22 Ω (Figure 8-7, p.12).
- Source issues:
  - Source conflict: pin 9 / pin 11 names (3C / 3A) in the pin table
    against their descriptions (Input A / Input C); see Pin functions.
  - Source conflict: the pinout figure (Figure 4-1) is titled "BQB
    Package"; the pin table, device information and thermal table name
    the package BQA.
  - Source error: V~OL~ test currents printed as I~OH~ (see the
    electrical characteristics table note).
  - Source error: the application section (8.2.1) names the SN74HC11;
    the document is the SN74ACT11-Q1 data sheet (title, p.1). Written
    "the device".
  - Source conflict: the input considerations call the inputs standard
    CMOS; the description and Feature Description give TTL-compatible
    CMOS inputs.
