---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct02",
  "title": "SNx4AHCT02 quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74ahct02.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS262N",
    "date": "2024-02",
    "pages": 42
  },
  "devices": ["gate:74_02"],
  "parts": ["5962-9757101QCA", "5962-9757101QDA", "SN74AHCT02BQAR", "SN74AHCT02D", "SN74AHCT02DBR", "SN74AHCT02DGVR", "SN74AHCT02DR", "SN74AHCT02N", "SN74AHCT02NSR", "SN74AHCT02PW", "SN74AHCT02PWR", "SN74AHCT02RGYR", "SNJ54AHCT02J", "SNJ54AHCT02W", "5962-9757101Q2A", "SNJ54AHCT02FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4AHCT02 quadruple 2-input positive-NOR gates

## Description

These devices contain four independent 2-input NOR gates that perform
the Boolean function Y = !{A} × !{B} or Y = !{A + B} in positive logic. [[1::1]]

Operating range of 4.5V to 5.5V. Low power consumption, 10µA maximum
I~CC~. ±8mA output drive at 5V. Inputs are TTL-voltage compatible.
Latch-up performance exceeds 250mA per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
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
NC: No Connection
VCC: Power Pin
```
[[3::Table 4-1]]

Conversion note: the RGY and BQA package drawing (Figure 4-2) also has a thermal pad (PAD), which the pin table does not list.

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
however, a 10kΩ resistor is recommended and will typically meet all
requirements. [[8::5]]

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due to
overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times. [[8::6]]

Unused push-pull CMOS outputs should be left disconnected. [[8::7]]

### Clamp diode structure

As Figure 7-1 shows, the outputs to this device have both positive and
negative clamping diodes, and the inputs to this device have negative
clamping diodes only. [[8::8]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[8::9]]

Conversion note: Figure 7-1 (electrical placement of clamping diodes for each input and output) draws the output clamps to V~CC~ (+I~OK~) and GND (-I~OK~) and the input clamp to GND (-I~IK~).

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output Y |
| A | B | ^ |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Do Not Care
```
[[9::Table 7-1]]

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
[3] Source error: this symbol is printed I~O~, like the next row's; the row is the output clamp current, which Figure 7-1 labels I~OK~. Kept as printed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | SN54AHCT02 | < | SN74AHCT02 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SNx4AHCT02 | < | < | < | < | Unit |
| ^ | ^ | D 14 pins | DB 14 pins | NS 14 pins | PW 14 pins | RGY 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 96 | 76 | 113 | 87.1 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54AHCT02 | < | SN74AHCT02 | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = −50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | I~OH~ = −8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| ΔI~CC~ [2] | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | | 10 | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #1]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C (unless otherwise noted) [1]"
| Symbol | Parameter | SN74AHCT02 | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.7 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 17 | pF |
```
[[5::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted). See Figure 6-1"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54AHCT02 | < | SN74AHCT02 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 2.4 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.5 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 3.4 | 7.5 | 1 | 8.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.5 | 7.5 | 1 | 8.5 | 1 | 8.5 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[5::table #4]]

## Typical characteristics

T~A~ = 25°C (unless otherwise noted) [[5::4]]

```curve id=voh-5v caption="Output voltage vs current in HIGH state; 5-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.3, 5]},
 "series": [
  {"label": "-40°C", "points": [[-25, 4.5], [-0.5, 4.99]]},
  {"label": "25°C", "points": [[-25, 4.455], [-0.5, 4.99]]},
  {"label": "125°C", "points": [[-25, 4.385], [-0.5, 4.99]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=vol-5v caption="Output voltage vs current in LOW state; 5-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.5]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.418]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.44]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.482]]}
 ]}
```
[[6::Figure 5-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::1]]

Conversion note: in Figure 6-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[7::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

In this application, two 2-input NOR gates are used to create an SR
latch as shown in Figure 8-1. The two additional
gates can be used for a second SR latch, or the inputs can be grounded
and both channels left unused. [[10::2]]

The SNx4AHCT02 is used to drive the tamper indicator LED and provide
one bit of data to the system controller. When the tamper switch outputs
HIGH, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a HIGH signal to the R
input which returns the Q output back to LOW. [[10::3]]

The user can add a small RC to the feedback path of the NOR gates to
default the output to a certain state, which can create slow transition
rates. This fact makes the SNx4AHCT02 suitable for the application
because it has Schmitt-trigger inputs that do not have input transition
rate requirements. [[10::4]]

Source conflict: this paragraph (p.10) says the SNx4AHCT02 has Schmitt-trigger inputs without input transition rate requirements, while the Recommended Operating Conditions (p.4) limit the input transition rate and the Feature Description (p.8) says TTL-compatible CMOS inputs require quick transitions.

Conversion note: the typical application block diagram (Figure 8-1) shows two cross-coupled NOR gates, with the system controller on the R input and the Q output, the tamper switch on the S input, a resistor R1 from R to ground, and Q driving the tamper indicator LED through a resistor R2.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics section. [[10::6]]

The positive voltage supply must be capable of sourcing current equal to
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. [[10::7]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SNx4AHCT02 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Ensure the
maximum total current through GND listed in the Absolute Maximum Ratings
is not exceeded. [[10::8]]

The SNx4AHCT02 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50pF. [[10::9]]

The SNx4AHCT02 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the HIGH
state, the output voltage in the equation is defined as the difference
between the measured output voltage and the supply voltage at the V~CC~
pin. [[10::10]] [[11::1]]

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
the SNx4AHCT02 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
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
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SNx4AHCT02 to one or more of the receiving devices. [[11::11]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[11::11]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[11::11]]

Conversion note: the application timing diagram (Figure 8-2) shows Q, undefined at first, going HIGH when S goes HIGH and LOW when R goes HIGH.

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

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill, a bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=PDIP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9757101Q2A | Tube of 55 | -55 to 125 | 5962-9757101Q2A SNJ54AHCT02FK
5962-9757101QCA | Tube of 25 | -55 to 125 | 5962-9757101QCA SNJ54AHCT02J
5962-9757101QDA | Tube of 25 | -55 to 125 | 5962-9757101QDA SNJ54AHCT02W
SN74AHCT02BQAR | Reel of 3000 | -40 to 125 | AHCT02
SN74AHCT02D | | -40 to 85 | AHCT02 | Obsolete
SN74AHCT02DBR | Reel of 2000 | -40 to 85 | HB02
SN74AHCT02DGVR | Reel of 2000 | -40 to 85 | HB02
SN74AHCT02DR | Reel of 2500 | -40 to 85 | AHCT02
SN74AHCT02N | Tube of 25 | -40 to 85 | SN74AHCT02N
SN74AHCT02NSR | Reel of 2000 | -40 to 85 | AHCT02
SN74AHCT02PW | | -40 to 85 | HB02 | Obsolete
SN74AHCT02PWR | Reel of 2000 | -40 to 85 | HB02
SN74AHCT02RGYR | Reel of 3000 | -40 to 85 | HB02
SNJ54AHCT02FK | Tube of 55 | -55 to 125 | 5962-9757101Q2A SNJ54AHCT02FK
SNJ54AHCT02J | Tube of 25 | -55 to 125 | 5962-9757101QCA SNJ54AHCT02J
SNJ54AHCT02W | Tube of 25 | -55 to 125 | 5962-9757101QDA SNJ54AHCT02W
```
[[15::table #1]]

## Conversion notes

- Source pages 1–12 converted. The ordering lines are from the package
  option addendum (pp.15–17). The support and revision history page
  (13), the mechanical information page (14) and the tape and reel and
  mechanical pages (18–42) are left out; the generated Package section
  replaces them.
- Dropped: applications list (the features are kept as sentences in the
  Description), table of contents (p.2), device information table, the
  logic diagram (p.1); the functional block diagram, the clamp diode
  figure (Figure 7-1), the typical application block diagram
  (Figure 8-1), the application timing diagram (Figure 8-2) and the
  example layout (Figure 8-3) are described in text.
- Note B of Figure 6-1 belongs to the enable and disable waveforms,
  which are not drawn, and is left out.
- The absolute maximum ratings of the source are a list ("Supply voltage
  range, V~CC~"); the symbols are put in the Symbol column.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale.
- From images: overbars of Y = !{A} × !{B} and Y = !{A + B} (p.1 and
  p.8); the curve points 4.455, 4.385, 4.99 (Figure 5-1, p.6) and 0.01,
  0.418, 0.44, 0.482 (Figure 5-2, p.6); the drawings of the functional
  block diagram (p.8), Figure 7-1 (p.9), Figure 8-1 (p.10), Figure 8-2
  (p.11) and Figure 8-3 (p.12); the thermal pad of Figure 4-2 (p.3).
- Source issues:
  - Source error: the Absolute Maximum Ratings (p.4) print the symbol of
    the output clamp current as I~O~; kept as printed (table note).
  - Source error: the Pin Functions table (p.3) prints pin 20 for 3Y in
    the J, W column; Figure 4-1 on the same page draws 3Y on pin 10 of
    the J and W packages, as the generated pinout does.
  - Source error: Input Considerations (p.11) leaves the two levels out
    of "Input signals must cross to be considered a logic LOW, and to be
    considered a logic HIGH"; kept as printed.
  - Source conflict: the Application Information (p.10) speaks of
    Schmitt-trigger inputs without input transition rate requirements;
    the Recommended Operating Conditions (p.4) and the Feature
    Description (p.8) require quick input transitions.
  - Source conflict: the caution on p.11 refers to T~J(max)~ listed in
    the Absolute Maximum Ratings, which have no such row (p.4).
  - Source conflict: the Recommended Operating Conditions (p.4) give the
    SN74AHCT02 an operating temperature of −40 °C to 125 °C; the package
    option addendum (p.15) gives -40 to 85 for every SN74AHCT02 part but
    SN74AHCT02BQAR.
  - Source conflict: the Device Information table (p.1) calls the PW
    package SOP, the package option addendum (p.15) TSSOP.
  - Source conflict: the Features (p.1) give a 10µA maximum I~CC~, the
    electrical characteristics table (p.5) 2 µA and 20 µA.
  - Unconfirmed: the Thermal Information (p.4) has no column for the
    DGV, N, BQA, J, W and FK packages.
