---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act04-q1",
  "title": "SN74ACT04-Q1 automotive hex inverters with TTL-compatible inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74act04-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCASE24",
    "date": "2024-11",
    "pages": 27
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74ACT04PWRQ1", "SN74ACT04WBQARQ1"],
  "packages": {"p14": "PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74ACT04-Q1 automotive hex inverters with TTL-compatible inputs

## Description

The SN74ACT04-Q1 device contains six independent inverters with
TTL-compatible inputs.

AEC-Q100 qualified for automotive applications. Device temperature
grade 1: -40°C to +125°C. Device HBM ESD classification level 2. Device
CDM ESD classification level C4B. Available in wettable flank QFN
package. Operating voltage range of 4.5V to 5.5V. TTL-compatible
inputs. Continuous ±24mA output drive at 5V. Supports up to ±75mA
output drive at 5V in short bursts. Drives 50Ω transmission lines. Fast
operation with delay of 5.9ns max. Source conflict: the switching
characteristics give maximum delays of 9.5ns and 7.1ns.

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

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. Conversion note: the source has
this for the thermal pad, BQA package only.

## Functional description

This device contains six independent inverters. Each gate performs the
Boolean function Y = !{A} in positive logic.

Conversion note: the functional block diagram shows one inverter per
channel, with a buffered input xA and a buffered output xY.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due
to overcurrent. The electrical and thermal limits defined in the
Absolute Maximum Ratings must be followed at all times.

Unused push-pull CMOS outputs must be left disconnected.

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold.

TTL-compatible CMOS inputs are high impedance and are typically modeled
as a resistor in parallel with the input capacitance given in the
Electrical Characteristics. The worst case resistance is calculated
with the maximum input voltage, given in the Absolute Maximum Ratings,
and the maximum input leakage current, given in the Electrical
Characteristics, using Ohm's law (R = V ÷ I).

TTL-compatible CMOS inputs require that input signals transition
between valid logic states quickly, as defined by the input transition
time or rate in the Recommended Operating Conditions table. Failing to
meet this specification will result in excessive power consumption and
could cause oscillations. More details can be found in the Implications
of Slow or Floating CMOS Inputs application report.

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a
system will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors;
however, a 10kΩ resistor is recommended and typically will meet all
requirements.

### Wettable flanks

This device includes wettable flanks for at least one package. See the
Features section on the front page of the data sheet where packages
include this feature. Conversion note: Figure 7-1, a simplified cutaway
view of a wettable-flank QFN package and a standard QFN package after
soldering, is not drawn here.

Wettable flanks help improve side wetting after soldering, which makes
QFN packages easier to inspect with automatic optical inspection (AOI).
As shown in Figure 7-1, a wettable flank can be dimpled or step-cut to
provide additional surface area for solder adhesion which assists in
reliably creating a side fillet. See the mechanical drawing for
additional details.

### Clamp diode structure

As shown in Figure 7-2, the inputs and outputs to this device have both
positive and negative clamping diodes.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

```figure ref=clamp-diodes caption="Figure 7-2. Electrical placement of clamping diodes for each input and output"
```

## Function tables

```table id=function caption="Function table"
| Inputs [1] | Output |
| A | Y |
|:-:|:-:|
| L | H |
| H | L |

[1] H = High Voltage Level, L = Low Voltage Level
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < -0.5V or V~I~ > V~CC~ + 0.5V | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < -0.5V or V~O~ > V~CC~ + 0.5V | | ±50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous output current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |
| T~J~ | Junction temperature | | | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 HBM ESD Classification Level 2 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C4B | ±1000 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordrance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | | 2 | | V |
| V~IL~ | Low-Level input voltage | | | 0.8 | V |
| V~I~ | Input Voltage | | 0 | V~CC~ | V |
| V~O~ | Output Voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | | -24 | mA |
| I~OL~ | Low-level output current | | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
```

## Thermal information

```table id=thermal caption="Thermal information"
| Package | Pins | Thermal metric [1] | < | < | < | < | < | Unit |
| ^ | ^ | R~θJA~ | R~θJC(top)~ | R~θJB~ | Ψ~JT~ | Ψ~JB~ | R~θJC(bot)~ | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| PW (TSSOP) | 14 | 132.2 | 64.8 | 88.4 | 11.9 | 87.4 | N/A | °C/W |
| BQA (WQFN) | 14 | 85.4 | 89.0 | 54.8 | 9.1 | 54.7 | 31.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50µA | 4.5V | 4.4 | 4.499 | | V |
| ^ | ^ | ^ | 5.5V | 5.4 | 5.499 | | ^ |
| ^ | ^ | I~OH~ = -24mA | 4.5V | 3.7 | | | ^ |
| ^ | ^ | I~OH~ = -24mA | 5.5V | 4.7 | | | ^ |
| ^ | ^ | I~OH~ = -75mA [1] | 5.5V | 3.85 | | | ^ |
| V~OL~ | | I~OL~ = 50µA | 4.5V | | 0.001 | 0.1 | V |
| ^ | ^ | ^ | 5.5V | | 0.001 | 0.1 | ^ |
| ^ | ^ | I~OL~ = 24mA | 4.5V | | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 24mA | 5.5V | | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 75mA [1] | 5.5V | | | 1.65 | ^ |
| I~I~ | | V~I~ = 5.5V or GND | 0 to 5.5V | | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5V | | | 2 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 2.1; Any Input | 4.5 to 5.5V | | | 1.5 | mA |
| C~I~ | | V~I~ = V~CC~ or GND | 5V | | 2 | | pF |
| C~O~ | | V~O~ = V~CC~ or GND | 5V | | 3 | | pF |
| C~PD~ | | C~L~ = 50pF, F = 1MHz | 5V | | 12 | | pF |

[1] Duration not to exceed 2ms
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="C~L~ = 50 pF; over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). See Parameter Measurement Information."
| Symbol | From (input) | To (output) | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | A | Y | 5V | | 6.6 | 9.5 | ns |
| t~PHL~ | A | Y | 5V | | 5 | 7.1 | ns |
```

## Typical characteristics

T~A~ = 25°C (unless otherwise noted)

```curve id=icc-vcc caption="Figure 5-1. Supply current across supply voltage"
{"x": {"label": "V_CC", "unit": "V", "range": [0.5, 5.5]},
 "y": {"label": "I_CC", "unit": "nA", "range": [0, 100]},
 "series": [
   {"label": "25 °C", "points": [[0.5, 11], [1, 14.5], [1.5, 18.5], [2, 21.5], [2.5, 24], [3, 27], [3.5, 29], [4, 30.5], [4.5, 30], [5, 27.5], [5.5, 32]]},
   {"label": "125 °C", "points": [[0.5, 28], [1, 34], [1.5, 37], [2, 37], [2.5, 38], [3, 42], [3.5, 42], [4, 40], [4.5, 41], [5, 38], [5.2, 55], [5.3, 54], [5.5, 73]]},
   {"label": "-40 °C", "points": [[0.5, 18], [1, 26], [1.5, 35], [2, 42], [2.5, 49], [3, 56], [3.5, 63], [4, 69], [4.5, 72], [5, 77], [5.5, 86]]}
 ]}
```

```curve id=voh-ioh caption="Figure 5-2. Output voltage vs current in HIGH state; 5V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-50, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.24, 5.04]},
 "series": [
   {"label": "25°C", "points": [[-50, 4.43], [-25, 4.71], [0, 4.99]]},
   {"label": "125°C", "points": [[-50, 4.33], [-25, 4.66], [0, 4.99]]},
   {"label": "-40°C", "points": [[-50, 4.52], [-25, 4.76], [0, 4.99]]}
 ]}
```

```curve id=vol-iol caption="Figure 5-3. Output voltage vs current in LOW state; 5V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 50]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.4]},
 "series": [
   {"label": "25°C", "points": [[0, 0], [25, 0.145], [50, 0.29]]},
   {"label": "125°C", "points": [[0, 0], [25, 0.178], [50, 0.356]]},
   {"label": "-40°C", "points": [[0, 0], [25, 0.12], [50, 0.24]]}
 ]}
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily for the
examples listed in the following table. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1MHz,
Z~O~ = 50Ω, t~t~ < 2.5ns, V~t~ = 1.5V. For push-pull outputs,
R~L~ = 500Ω.

The outputs are measured individually with one input transition per
measurement.

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. The greater between t~r~ and
t~f~ is the same as t~t~.

```figure ref=load-rl-cl caption="Figure 6-1. Load circuit for push-pull outputs"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Figures 6-2 and 6-3. Voltage waveforms, propagation delays and input and output transition times"
VM: V~t~ = 1.5V (input), 50% (outputs)
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

The SN74ACT04-Q1 can be used to add an additional stage to a counter
with an external flip-flop. Because counters use a negative edge
trigger, the flip-flop's clock input must be inverted to provide this
function. This function only requires one of the six available
inverters in the SN74ACT04-Q1 device, so the remaining channels can be
used for other applications needing an inverted signal. Unused inputs
must be terminated at V~CC~ or GND. Unused outputs can be left
floating.

Conversion note: in the typical application block diagram (Figure 8-1)
the last output of a counter drives an inverter whose output clocks a
D-type flip-flop, the flip-flop's inverted output is fed back to its D
input, its Q output is the added stage, and the Clear signal goes to
the CLR inputs of the counter and the flip-flop.

Conversion note: the application timing diagram (Figure 8-2) shows the
last counter output, its inverse and the flip-flop output, which
toggles at half the rate of the counter output, with the legend
"Input – 32 kHz" and "1 kHz" for the added stage.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the SN74ACT04-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Ensure the maximum total current through V~CC~
listed in the Absolute Maximum Ratings is not exceeded.

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74ACT04-Q1 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into its ground connection.
Ensure the maximum total current through GND listed in the Absolute
Maximum Ratings is not exceeded.

The SN74ACT04-Q1 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50pF.

The SN74ACT04-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin.

Total power consumption can be calculated using the information
provided in CMOS Power Consumption and Cpd Calculation.

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74ACT04-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors.

The SN74ACT04-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics.

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This
can cause excessive current and damage to the device.

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74ACT04-Q1 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1μF capacitor is recommended for this device. It
is acceptable to parallel multiple bypass capacitors to reject
different frequencies of noise. The 0.1μF and 1μF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results.

### Layout guidelines

- Bypass capacitor placement
  - Place near the positive supply terminal of the device
  - Provide an electrically short ground return path
  - Use wide traces to minimize impedance
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible
- Signal trace geometry
  - 8mil to 12mil trace width
  - Lengths less than 12cm to minimize transmission line effects
  - Avoid 90° corners for signal traces
  - Use an unbroken ground plane below signal traces
  - Flood fill areas around signal traces
  - For traces longer than 12cm
    - Use impedance controlled traces
    - Source-terminate using a series damping resistor near the output
    - Avoid branches; buffer signals that must branch separately

Conversion note: the layout example drawings (Figures 8-3 to 8-7) show
trace corners for improved signal integrity, bypass capacitor placement
for TSSOP, WQFN and SOT packages, and damping resistor placement, and
are not drawn here.

## Package

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74ACT04PWRQ1 | Reel of 3000 | -40 to 125 | ACT04Q
SN74ACT04WBQARQ1 | Reel of 3000 | -40 to 125 | AD04Q
```

## Conversion notes

- Source pages 1–14 converted; the support and revision history page
  (15), the package option addendum, tape and reel and mechanical pages
  (16–27) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are the parts' rows of the package option
  addendum (p.16).
- Dropped: applications list, device information table, table of
  contents, the wettable flank cutaway drawing (Figure 7-1), the layout
  example drawings (Figures 8-3 to 8-7), the references to the Feature
  Description section in the input and output considerations.
- The electrical characteristics table of the source gives only symbols
  in its Parameter column; no parameter names are added. The Condition
  column of the recommended operating conditions is empty in the
  source.
- Typical characteristics: Figures 5-1, 5-2 (p.5) and 5-3 (p.6)
  digitized to about 2 % of full scale.
- The functional diagram (p.1), the functional block diagram (p.8), the
  typical application block diagram (Figure 8-1, p.10) and the
  application timing diagram (Figure 8-2, p.13) are described in text.
- C~L~ = 50 pF for the load figure is from the switching
  characteristics conditions and R~L~ = 500Ω from the text above the
  figure; the load figure itself (Figure 6-1, p.7) gives no value.
- From images: overbars of Y = !{A} (p.8); the curve points of Figures
  5-1 to 5-3 (pp.5–6); 50% (measurement points of Figure 6-2, p.7).
- Source issues:
  - Source error: the ESD ratings note prints "accordrance" for
    "accordance"; kept as printed.
  - Source conflict: the features give "Fast operation with delay of
    5.9ns max" (p.1); the switching characteristics give maximum
    t~PLH~ of 9.5ns and t~PHL~ of 7.1ns (p.5).
  - Unconfirmed: the ΔI~CC~ test condition is printed "V~I~ = V~CC~ –
    2.1", without a unit (p.5).
