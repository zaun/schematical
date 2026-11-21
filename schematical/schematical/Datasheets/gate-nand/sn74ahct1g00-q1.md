---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g00-q1",
  "title": "SN74AHCT1G00-Q1 automotive 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74ahct1g00-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS940A",
    "date": "2024-01",
    "pages": 25
  },
  "devices": ["gate:1G00"],
  "parts": ["CAHCT1G00QDBVRQ1", "CAHCT1G00QDCKRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G00-Q1 automotive 2-input positive-NAND gates

## Description

The SN74AHCT1G00-Q1 is a 2-input NAND Gate. Each gate performs the
Boolean function Y = !{A × B} in positive logic.

AEC-Q100 qualified for automotive applications: Device temperature
grade 1: -40°C to +125°C. Device HBM ESD classification level 2. Device
CDM ESD classification level C4B. Operating range of 4.5V to 5.5V. Low
power consumption, 10µA maximum I~CC~. ±8mA output drive at 5V. Inputs
are TTL-voltage compatible. Latch-up performance exceeds 250mA per JESD
17.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground
Y: Output Y
VCC: Positive Supply
```

## Functional description

The SN74AHCT1G00-Q1 is a 2-input NAND Gate. Each gate performs the
Boolean function Y = !{A × B} in positive logic.

Conversion note: the functional block diagram shows one NAND gate with inputs A (pin 1) and B (pin 2) and output Y (pin 4).

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times.

Unused push-pull CMOS outputs should be left disconnected.

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold.

TTL-compatible CMOS inputs are high impedance and are typically modeled
as a resistor in parallel with the input capacitance given in the
Electrical Characteristics. The worst case resistance is calculated with
the maximum input voltage, given in the Absolute Maximum Ratings, and
the maximum input leakage current, given in the Electrical
Characteristics, using Ohm's law (R = V ÷ I).

TTL-compatible CMOS inputs require that input signals transition between
valid logic states quickly, as defined by the input transition time or
rate in the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations. More details can be found in the Implications of Slow or
Floating CMOS Inputs application report.

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a system
will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors;
however, a 10-kΩ resistor is recommended and will typically meet all
requirements.

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes only
as shown in Figure 7-1.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

Conversion note: Figure 7-1 (electrical placement of clamping diodes for each input and output) draws the output clamps to V~CC~ (+I~OK~) and GND (-I~OK~) and the input clamp to GND (-I~IK~).

## Function tables

Table 7-1 lists the functional modes of the SN74AHCT1G00-Q1.

```table id=function caption="Function table"
| Inputs [1] | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |

[1] H = high voltage level, L = low voltage level, X = do not care, Z = high impedance
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage range | | -0.5 | 7 | V |
| V~O~ | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 V | | -20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 V or V~O~ > V~CC~ +0.5 V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| I~O~ | Continuous output current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] HBM ESD Classification Level 2 | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C4B | ±1000 | ^ |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| V~IH~ | High-level input voltage | 2.00 | | V |
| V~IL~ | Low-Level input voltage | | 0.8 | V |
| I~O~ | Output current | | ±8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at VCC or GND to ensure proper device operation. Refer to the TI application report: Implications of Slow or Floating CMOS Inputs.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278.0 | 293.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 208.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 180.6 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 120.6 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 179.5 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –50 µA | 4.5 V | 4.4 | 4.5 | | V~CC~ - 0.1 | | | V |
| ^ | I~OH~ = –8 mA | ^ | 3.94 | | | 3.8 | | | ^ |
| V~OL~ | I~OH~ = 50 µA [3] | 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OH~ = 8 mA [3] | ^ | | | 0.36 | | | 0.44 | ^ |
| I~I~ | V~I~ = 0 V to V~CC~ | 0 V to 5.5 V | | | ±0.1 | | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | | 10 | µA |
| ΔI~CC~ | One input at 0.3 V or 3.4 V, other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 2 | 10 | pF |
| C~PD~ [1] [2] | F = 1 MHz | 5 V | | 11 | | | | | pF |

[1] C~PD~ is used to determine the dynamic power consumption, per channel.
[2] P~D~ = V~CC~ ^2^ × F~I~ × (C~PD~ + C~L~) where F~I~ = input frequency, C~L~ = output load capacitance, V~CC~ = supply voltage
[3] Source error: the source prints the test current of the V~OL~ rows as I~OH~; a low-level output voltage is measured with the output sinking current. Kept as printed.
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|---|
| t~PD~ | A or B | Y | C~L~ = 15 pF | 5 | 6.2 | 1 | 8 | ns |
| t~PD~ | A or B | Y | C~L~ = 50 pF | 5.5 | 7.9 | 1 | 10 | ns |
```

## Typical characteristics

T~A~ = 25°C (unless otherwise noted)

```curve id=voh-5v caption="Output voltage vs current in HIGH state; 5-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.3, 5]},
 "series": [
  {"label": "-40°C", "points": [[-25, 4.5], [-0.5, 4.99]]},
  {"label": "25°C", "points": [[-25, 4.455], [-0.5, 4.99]]},
  {"label": "125°C", "points": [[-25, 4.385], [-0.5, 4.99]]}
 ]}
```

```curve id=vol-5v caption="Output voltage vs current in LOW state; 5-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.5]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.418]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.44]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.482]]}
 ]}
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 3 ns.

For clock inputs, f~max~ is measured when the input duty cycle is 50%.

The outputs are measured one at a time with one input transition per
measurement.

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. The greater between t~r~ and
t~f~ is the same as t~t~.

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and input and output transition times"
VM: 50%
```

Conversion note: the source draws the propagation delays (Figure 6-2) and the transition times (Figure 6-3) as two figures, with the input swinging between 0 V and V~CC~.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

In this application, two 2-input NAND gates are used to create an
active-low SR latch as shown in Figure 8-1. The SN74AHCT1G00-Q1 is used
to drive the tamper indicator LED and provide one bit of data to the
system controller. When the tamper switch outputs LOW, the output Q
becomes HIGH. This output remains HIGH until the system controller
addresses the event and sends a LOW signal to the !{R} input which
returns the Q output back to LOW.

Conversion note: the typical application block diagram (Figure 8-1) shows two cross-coupled NAND gates, with the system controller on the !{R} input and the Q output, the tamper switch on the !{S} input, a pull-up resistor R1 on !{R}, and Q driving the tamper indicator LED through a resistor R2 to ground.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74AHCT1G00-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHCT1G00-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings.

The SN74AHCT1G00-Q1 can drive a load with a total capacitance less than
or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF.

The SN74AHCT1G00-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin.

Total power consumption can be calculated using the information provided
in the CMOS Power Consumption and Cpd Calculation application note.

Thermal increase can be calculated using the information provided in the
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

Source conflict: this caution (p.11) says T~J(max)~ is listed in the Absolute Maximum Ratings, while that table (p.4) has no junction temperature row.

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHCT1G00-Q1 (as specified in the Electrical Characteristics),
and the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors.

The SN74AHCT1G00-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability.

Refer to the Feature Description section for additional information
regarding the inputs for this device.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics.

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device.

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

Refer to the Feature Description section for additional information
regarding the outputs for this device.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74AHCT1G00-Q1 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω, so that the maximum output current from the
   Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in MΩ; much larger than the minimum
   calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

Conversion note: the application timing diagram (Figure 8-2) shows Q, undefined at first, going LOW when !{R} goes LOW and HIGH when !{S} goes LOW.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in the following
layout example.

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used). Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient.

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill, a 0.1 µF bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
CAHCT1G00QDBVRQ1 | Reel of 3000 | -40 to 125 | 37SH
CAHCT1G00QDCKRQ1 | Reel of 3000 | -40 to 125 | 1PB
```

## Conversion notes

- Source pages 1–13 converted. The ordering lines are from the package
  option addendum (pp.15–16). The support, revision history and
  mechanical information page (14) and the tape and reel and mechanical
  pages (17–25) are left out; the generated Package section replaces
  them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), table of contents (p.2), package
  information table, the vendor pin drawing (p.3); the logic diagram
  (p.1), the functional block diagram, the clamp diode figure
  (Figure 7-1), the typical application block diagram (Figure 8-1), the
  application timing diagram (Figure 8-2) and the example layout
  (Figure 8-3) are described in text.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column. The switching
  characteristics table has no MIN column at T~A~ = 25°C; the source
  prints its "over operating free-air temperature range" line above the
  section heading.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale; each series is drawn as a straight line between its end points.
- From images: overbars of Y = !{A × B} (p.1 and p.8) and of !{R} and
  !{S} (p.10 and Figure 8-2, p.12); the curve points 4.455, 4.385, 4.99
  (Figure 5-1, p.6) and 0.01, 0.418, 0.44, 0.482 (Figure 5-2, p.6); the
  drawings of the functional block diagram (p.8), Figure 7-1 (p.9),
  Figure 8-1 (p.10), Figure 8-2 (p.12) and Figure 8-3 (p.13), with its
  0.1 µF.
- Source issues:
  - Source error: the Electrical Characteristics (p.5) print the test
    current of the V~OL~ rows as I~OH~; kept as printed (table note).
  - Source conflict: the caution on p.11 refers to T~J(max)~ listed in
    the Absolute Maximum Ratings, which have no such row (p.4).
  - Unconfirmed: the title, the Description and the Overview speak of
    "gates" and "each gate", the Application Information (p.10) of two
    NAND gates, and the Output Considerations (p.11) of two channels
    within the same device; the logic diagram, the pin table and the
    function table show one gate.
  - Unconfirmed: the note of the function table (p.9) defines
    Z = high impedance, which the table does not use.
