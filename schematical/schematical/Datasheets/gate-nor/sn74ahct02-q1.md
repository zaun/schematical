---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct02-q1",
  "title": "SN74AHCT02-Q1 automotive quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74ahct02-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS921",
    "date": "2023-05",
    "pages": 23
  },
  "devices": ["gate:74_02"],
  "parts": ["SN74AHCT02QPWRQ1", "SN74AHCT02QWBQARQ1"],
  "packages": {"p14": "PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74AHCT02-Q1 automotive quadruple 2-input positive-NOR gates

## Description

These devices contain four independent 2-input NOR gates that perform
the Boolean function Y = !{A} · !{B} or Y = !{A + B} in positive logic. [[1::1]]

AEC-Q100 qualified for automotive applications: Device temperature
grade 1: -40°C to +125°C. Available in wettable flank QFN (WBQA)
package. Operating range of 4.5 V to 5.5 V. Low power consumption, 10-µA
maximum I~CC~. ±8-mA output drive at 5 V. Inputs are TTL-voltage
compatible. Latch-up performance exceeds 250 mA per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
1Y: 1Y Output
2A: 2A Input
2B: 2B Input
2Y: 2Y Output
3A: 3A Input
3B: 3B Input
3Y: 3Y Output
4A: 4A Input
4B: 4B Input
4Y: 4Y Output
GND: Ground Pin
VCC: Power Pin
```
[[3::Table 5-1]]

The thermal pad can be connected to GND or left floating. Do not connect
to any other signal or supply. BQA package only [[3::1]]

## Functional description

This device contains four independent 2-input NOR Gates. Each gate
performs the Boolean function Y = !{A + B} in positive logic. [[8::1]]

Conversion note: the functional block diagram shows one gate, with inputs xA and xB each buffered into a NOR gate whose output is buffered to xY.

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold. [[8::2]]

TTL-compatible CMOS inputs are high impedance and are typically modeled
as a resistor in parallel with the input capacitance given in the
Electrical Characteristics. The worst case resistance is calculated with
the maximum input voltage, given in the Absolute Maximum Ratings, and
the maximum input leakage current, given in the Electrical
Characteristics, using Ohm's law (R = V ÷ I). [[8::3]]

TTL-compatible CMOS inputs require that input signals transition between
valid logic states quickly, as defined by the input transition time or
rate in the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations. More details can be found in the Implications of Slow or
Floating CMOS Inputs application report. [[8::4]]

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a system
will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors;
however, a 10-kΩ resistor is recommended and will typically meet all
requirements. [[8::5]]

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times. [[8::6]]

Unused push-pull CMOS outputs should be left disconnected. [[8::7]]

### Clamp diode structure

As Figure 8-1 shows, the outputs to this device have both positive and
negative clamping diodes, and the inputs to this device have negative
clamping diodes only. [[8::8]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[8::9]]

Conversion note: Figure 8-1 (electrical placement of clamping diodes for each input and output) draws the output clamps to V~CC~ (+I~OK~) and GND (-I~OK~) and the input clamp to GND (-I~IK~).

### Wettable flanks

This device includes wettable flanks for at least one package. See the
Features section on the front page of the data sheet for which packages
include this feature. [[9::2]]

Wettable flanks help improve side wetting after soldering, which makes
QFN packages easier to inspect with automatic optical inspection (AOI).
As shown in Figure 8-2, a wettable flank can be dimpled or step-cut to
provide additional surface area for solder adhesion which assists in
reliably creating a side fillet. See the mechanical drawing for
additional details. [[9::4]]

Conversion note: Figure 8-2 (simplified cutaway view of wettable-flank QFN package and standard QFN package after soldering) shows the solder rising up the side of the wettable flank lead and staying under the standard lead.

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don't Care
```
[[9::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | (V~I~ < 0) | | -20 | mA |
| I~O~ [3] | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Source error: this symbol is printed I~O~, like the next row's; the row is the output clamp current, which Figure 8-1 labels I~OK~. Kept as printed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 HBM ESD Classification Level 2 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C4B | ±1000 | ^ |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND for proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHCT02-Q1 | < | Unit |
| ^ | ^ | BQA (WQFN) 14 pins | PW (TSSOP) 14 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 88.3 | 151.0 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 90.9 | 80.0 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 56.8 | 94.2 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 9.9 | 28.0 | °C/W |
| Y~JB~ [2] | Junction-to-board characterization parameter | 56.7 | 93.6 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | 33.4 | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
[2] Source error: this symbol is printed with a Y where the row above, Ψ~JT~, has Ψ; the row is the junction-to-board characterization parameter. Kept as printed.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = -40°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = −50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | I~OH~ = −8 mA | ^ | 3.94 | | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| ΔI~CC~ [1] | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #2]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C (unless otherwise noted) [1]"
| Symbol | Parameter | SN74AHCT02-Q1 | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | -0.8 | | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.7 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance [1] [2] | No load, f = 1 MHz | 17 | pF |

[1] C~PD~ is used to determine the dynamic power consumption, per channel.
[2] P~D~ = V~CC~^2^ × F~I~ × (C~PD~ + C~L~) where F~I~ = input frequency, C~L~ = output load capacitance, V~CC~ = supply voltage.
```
[[6::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted). See Parameter Measurement Information"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | T~A~ = -40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 2.4 | 5.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.5 | 5.5 | 1 | 6.5 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 3.4 | 7.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.5 | 7.5 | 1 | 8.5 | ^ |
```
[[5::table #3]]

## Typical characteristics

T~A~ = 25°C (unless otherwise noted) [[6::1]]

```curve id=voh-5v caption="Output voltage vs current in HIGH state; 5-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.3, 5]},
 "series": [
  {"label": "-40°C", "points": [[-25, 4.5], [-0.5, 4.99]]},
  {"label": "25°C", "points": [[-25, 4.455], [-0.5, 4.99]]},
  {"label": "125°C", "points": [[-25, 4.385], [-0.5, 4.99]]}
 ]}
```
[[6::Figure 6-1]]

```curve id=vol-5v caption="Output voltage vs current in LOW state; 5-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.5]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.418]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.44]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.482]]}
 ]}
```
[[6::Figure 6-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::1]]

Conversion note: in Figure 7-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[7::Figure 7-1]]

Conversion note: the source figure (Figure 7-1) also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

In this application, two 2-input NOR gates are used to create an SR
latch as shown in Typical application block diagram. The two additional
gates can be used for a second SR latch, or the inputs can be grounded
and both channels left unused. [[10::2]]

The SN74AHCT02-Q1 is used to drive the tamper indicator LED and provide
one bit of data to the system controller. When the tamper switch outputs
HIGH, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a HIGH signal to the R
input which returns the Q output back to LOW. [[10::3]]

The user can add a small RC to the feedback path of the NOR gates to
default the output to a certain state, which can create slow transition
rates. This fact makes the SN74AHCT02-Q1 ideal for the application
because it has Schmitt-trigger inputs that do not have input transition
rate requirements. [[10::4]]

Source conflict: this paragraph (p.10) says the SN74AHCT02-Q1 has Schmitt-trigger inputs without input transition rate requirements, while the Recommended Operating Conditions (p.4) limit the input transition rate and the Feature Description (p.8) says TTL-compatible CMOS inputs require quick transitions.

Conversion note: the typical application block diagram (Figure 9-1) shows two cross-coupled NOR gates, with the system controller on the R input and the Q output, the tamper switch on the S input, a resistor R1 from R to ground, and Q driving the tamper indicator LED through a resistor R2.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics section. [[10::6]]

The positive voltage supply must be capable of sourcing current equal to
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. [[10::7]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHCT02-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings. [[10::8]]

The SN74AHCT02-Q1 can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50 pF. [[10::9]]

The SN74AHCT02-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the HIGH
state, the output voltage in the equation is defined as the difference
between the measured output voltage and the supply voltage at the V~CC~
pin. [[11::1]]

Total power consumption can be calculated using the information provided
in CMOS Power Consumption and Cpd Calculation. [[11::2]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices. [[11::3]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::4]]

Source conflict: this caution (p.11) says T~J(max)~ is listed in the Absolute Maximum Ratings, while that table (p.4) has no junction temperature row.

### Input considerations

Input signals must cross to be considered a logic LOW, and to be
considered a logic HIGH. Do not exceed the maximum input voltage range
found in the Absolute Maximum Ratings. [[11::5]]

Source error: the source leaves out the two levels the signals must cross in the first sentence; kept as printed.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHCT02-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[11::6]]

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[11::8]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[11::9]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[11::11]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74AHCT02-Q1 to one or more of the receiving devices. [[11::11]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[11::11]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[11::11]]

Conversion note: the application timing diagram (Figure 9-2) shows Q, undefined at first, going HIGH when S goes HIGH and LOW when R goes HIGH.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in given example
layout image. [[12::1]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs must
not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used. Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient. [[12::2]]

Conversion note: the example layout (Figure 9-3) recommends a GND flood fill, a bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and avoiding 90° corners for signal lines.

## Package

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74AHCT02QPWRQ1 | Reel of 3000 | -40 to 125 | LVT02Q
SN74AHCT02QWBQARQ1 | Reel of 3000 | -40 to 125 | AHT02Q
```
[[14::table #1]]

## Conversion notes

- Source pages 1–12 converted. The ordering lines are from the package
  option addendum (pp.14–15). The support page (13) and the tape and
  reel and mechanical pages (16–23) are left out; the generated Package
  section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description; the ESD classification lines are left
  out), table of contents and revision history (p.2), package
  information table, the logic diagram (p.1); the functional block
  diagram, the clamp diode figure (Figure 8-1), the wettable flank
  figure (Figure 8-2), the typical application block diagram
  (Figure 9-1), the application timing diagram (Figure 9-2) and the
  example layout (Figure 9-3) are described in text.
- Note B of Figure 7-1 belongs to the enable and disable waveforms,
  which are not drawn, and is left out.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column.
- Curves digitized from Figures 6-1 and 6-2 (p.6) to about 2 % of full
  scale.
- From images: overbars of Y = !{A} · !{B} and Y = !{A + B} (p.1 and
  p.8); the curve points 4.455, 4.385, 4.99 (Figure 6-1, p.6) and 0.01,
  0.418, 0.44, 0.482 (Figure 6-2, p.6); the drawings of the functional
  block diagram (p.8), Figures 8-1 and 8-2 (p.9), Figure 9-1 (p.10),
  Figure 9-2 and Figure 9-3 (p.12).
- Source issues:
  - Source error: the Absolute Maximum Ratings (p.4) print the symbol of
    the output clamp current as I~O~; kept as printed (table note).
  - Source error: the Thermal Information (p.5) prints Y~JB~ with a Y
    for Ψ; kept as printed (table note).
  - Source error: Input Considerations (p.11) leaves the two levels out
    of "Input signals must cross to be considered a logic LOW, and to be
    considered a logic HIGH"; kept as printed.
  - Source conflict: the Application Information (p.10) speaks of
    Schmitt-trigger inputs without input transition rate requirements;
    the Recommended Operating Conditions (p.4) and the Feature
    Description (p.8) require quick input transitions.
  - Source conflict: the caution on p.11 refers to T~J(max)~ listed in
    the Absolute Maximum Ratings, which have no such row (p.4).
  - Source conflict: the Package Information table (p.1) calls the PW
    package SOP and the part SN74AHCT02, the Thermal Information (p.5)
    and the package option addendum (p.14) TSSOP.
  - Source conflict: the Features (p.1) give a 10-µA maximum I~CC~, the
    electrical characteristics table (p.5) 2 µA and 20 µA.
