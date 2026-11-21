---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g14-q1",
  "title": "SN74AHCT1G14-Q1 automotive single Schmitt-trigger inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahct1g14-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS953A",
    "date": "2024-01",
    "pages": 25
  },
  "devices": ["gate:1G14"],
  "parts": ["CAHCT1G14QDBVRQ1", "CAHCT1G14QDCKRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G14-Q1 automotive single Schmitt-trigger inverter gate

## Description

The SN74AHCT1G14-Q1 is a single inverter gate. The device performs the
Boolean function Y = !{A}.

The device functions as an independent inverter gate, but because of the
Schmitt action, gates may have different input threshold levels for
positive-going (V~T+~) and negative-going (V~T−~) signals.

AEC-Q100 qualified for automotive applications. Operating range of 4.5 V
to 5.5 V. Low power consumption, 10-µA maximum I~CC~. ±8-mA output drive
at 5 V. Inputs are TTL-voltage compatible. Latch-up performance exceeds
250 mA per JESD 17.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
NC: No Connect
A: Data Input
GND: Ground
Y: Data Output
VCC: Power
```

## Functional description

The SN74AHCT1G14-Q1 is a single inverter gate. The device performs the
Boolean function Y = !{A}.

The device functions as an independent inverter gate, but because of the
Schmitt action, gates may have different input threshold levels for
positive-going (V~T+~) and negative-going (V~T−~) signals.

Conversion note: the functional block diagram shows one Schmitt-trigger inverter with input A and output Y.

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

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture. These
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I).

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs with slow
transitioning signals will increase dynamic current consumption of the
device. For additional information regarding Schmitt-trigger inputs,
please see Understanding Schmitt Triggers.

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes only
as shown in Figure 7-1.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

Conversion note: Figure 7-1 draws the output clamps to V~CC~ (+I~OK~) and GND (-I~OK~) and the input clamp to GND (-I~IK~).

### Device functional modes

Table 7-1 lists the device function modes.

## Function tables

```table id=function caption="Function table"
| Input [1] | Output [1] |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = High Voltage Level, L = Low Voltage Level
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage range | | -0.5 | 7 | V |
| V~O~ | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state | | -0.5 | 4.6 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 | -20 | | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 or V~O~ > V~CC~ | -20 | 20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | -25 | 25 | mA |
| I~O~ | Continuous output current through V~CC~ or GND | | -50 | 50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
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

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply Voltage [1] | | 3 | 5.5 | V |
| V~IH~ | high-level input voltage | V~CC~ = 3V | 1.4 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 2 | | V |
| V~IL~ | low-level input voltage | V~CC~ = 3 V | | 0.53 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.8 | V |
| V~I~ | input voltage | | 0 | 5.5 | V |
| V~O~ | output voltage | | 0 | V~CC~ | V |
| I~OH~ | high-level output current | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | low-level output current | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | input transition rise or fall rate | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] Source conflict: this table (p.4) prints a minimum supply voltage of 3 V and gives V~IH~ and V~IL~ at V~CC~ = 3 V; the features (p.1) give an operating range of 4.5 V to 5.5 V, and the electrical and switching characteristics are specified from 4.5 V.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHCT1G14-Q1"
| Symbol | Thermal metric [1] | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | Unit |
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
| V~T+~ | Positive-going input threshold voltage | 4.5 V | 0.9 | | 1.9 | 0.9 | | 1.9 | V |
| ^ | ^ | 5.5 V | 1.1 | | 2 | 1.1 | | 2 | V |
| V~T-~ | Negative-going input threshold voltage | 4.5 V | 0.5 | | 1.5 | 0.5 | | 1.5 | V |
| ^ | ^ | 5.5 V | 0.6 | | 1.5 | 0.6 | | 1.5 | V |
| ΔV~T~ | Hysteresis (V~T+~ − V~T−~) | 4.5 V | 0.4 | | 1.4 | 0.4 | | 1.4 | V |
| ^ | ^ | 5.5 V | 0.5 | | 1.5 | 0.5 | | 1.5 | V |
| V~OH~ | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | 4.5 | | V |
| ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | | V |
| V~OL~ | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | | 0.44 | V |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | -0.1 | | 0.1 | -1 | | 1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND | 5.5 V | | | 1 | | | 10 | µA |
| ΔI~CC~ | One input at 3.4 V, other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | | pF |
| C~O~ | V~O~ = V~CC~ or GND | 5 V | | 15 | | | | | pF |
| C~PD~ | No load, f 10 MHz | 5 V | | 14 | | | | | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | V~CC~ | T~A~ = 25°C | < | < | -40°C to 85°C | < | < | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| T~PHL~ | A | Y | C~L~ = 15 pF | 5 V ± 0.5 V | | 4 | 7 | 1 | | 8 | 1 | | 9 | ns |
| T~PLH~ | A | Y | C~L~ = 15 pF | 5 V ± 0.5 V | | 4 | 7 | 1 | | 8 | 1 | | 9 | ns |
| T~PHL~ | A | Y | C~L~ = 50 pF | 5 V ± 0.5 V | | 5.5 | 8 | 1 | | 9 | 1 | | 10 | ns |
| T~PLH~ | A | Y | C~L~ = 50 pF | 5 V ± 0.5 V | | 5.5 | 8 | 1 | | 9 | 1 | | 10 | ns |
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
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.42]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.44]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.48]]}
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

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

The SN74AHCT1G14-Q1can be used to add an additional stage to a counter
with an external flip-flop. Because counters use a negative edge
trigger, the flip-flop's clock input must be inverted to provide this
function. Having Schmitt-trigger inputs is important in this application
to eliminate any noise issues that could impact the counting function
which could lead to incorrect frequency division.

The SN74AHCT1G14-Q1 is a high drive CMOS device that can be used for
implementing inversion logic with a high output drive.

Conversion note: the typical application block diagram (Figure 8-1) shows a counter whose last output drives the inverter, the inverter driving the clock of a D-type flip-flop whose !{Q} output returns to its D input, and a Clear signal on the CLR inputs of both.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74AHCT1G14-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHCT1G14-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings.

The SN74AHCT1G14-Q1 can drive a load with a total capacitance less than
or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF.

The SN74AHCT1G14-Q1 can drive a load with total resistance described by
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

### Input considerations

Input signals must cross V~IL(max)~ V~t-(min)~ to be considered a logic
LOW, and V~IH(min)~ V~t+(max)~ to be considered a logic HIGH. Do not
exceed the maximum input voltage range found in the Absolute Maximum
Ratings.

Unconfirmed: the source prints two level names side by side in each place (p.11), kept as printed.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHCT1G14-Q1 (as specified in the Electrical Characteristics),
and the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors.

The SN74AHCT1G14-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability.

The SN74AHCT1G14-Q1 has no input signal transition rate requirements
because it has Schmitt-trigger inputs.

Source conflict: § 8.2.1.2 (p.11) says in one paragraph that the device requires fast input transitions and in the next that it has no input signal transition rate requirements; the Recommended Operating Conditions (p.4) give a maximum input transition rise or fall rate. Both are kept as printed.

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit.

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics.

Source conflict: § 8.2.1.2 (p.11) says the additional current is plotted in the Typical Characteristics; that section (p.6) holds only the two output voltage figures.

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

Source conflict: § 8.2.1.3 (p.11) speaks of two channels within the same device; the function table (p.9) and the pin table (p.3) show a single gate. Kept as printed.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74AHCT1G14-Q1 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. Doing this will not violate the maximum output
   current from the Absolute Maximum Ratings. Most CMOS inputs have a
   resistive load measured in MΩ; much larger than the minimum
   calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

Conversion note: the application timing diagram (Figure 8-2) shows the counter's last output, its inverse and the flip-flop output, which changes on each falling edge of the counter output; the legend reads Input – 32 kHz and 1 kHz for the flip-flop output.

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

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
CAHCT1G14QDBVRQ1 | Reel of 3000 | -40 to 125 | 36EH
CAHCT1G14QDCKRQ1 | Reel of 3000 | -40 to 125 | 1O5
```

## Conversion notes

- Source pages 1–13 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (p.15; it ends on p.16). The support, revision history and
  mechanical information page (14), tape and reel and mechanical pages
  (17–25) are left out; the generated Package section replaces them.
- Dropped: applications list (features kept as sentences in the
  Description; the three sub-items of the AEC-Q100 feature, temperature
  grade 1 and the HBM and CDM classification levels, are left out there
  and are in the tables), package information table; the simplified
  logic diagram (p.1), the functional block diagram (p.8), the
  clamp-diode figure (Figure 7-1; the shared clamp-diodes figure also
  draws an input clamp to V~CC~, which this part does not have), the
  application block diagram (Figure 8-1), the application timing diagram
  (Figure 8-2) and the example layout (Figure 8-3) are described in
  text.
- The pin table's note "I = input, O = output, I/O = input or output,
  G = ground, P = power." is on its TYPE column, which the generated pin
  table draws from the catalog. The source gives pin 1 (NC) the type I.
- The Electrical characteristics table of the source prints the symbols
  under PARAMETER and the names of V~T+~, V~T-~ and ΔV~T~ under TEST
  CONDITIONS; they are kept in that column. The C~PD~ condition is
  printed "No load, f 10 MHz", without a sign between f and the value.
- The input transition rate unit is printed "nS/V" in the source (p.4);
  written ns/V. The switching symbols are printed T~PLH~ and T~PHL~
  (p.5) and kept.
- The notes of Figures 6-1 to 6-3 (C~L~, t~pd~, t~t~) are given as text
  beside the figures. The load circuit (Figure 6-1) prints no value for
  C~L~, which is 15 pF or 50 pF in the switching characteristics.
- Curves digitized from Figures 5-1 and 5-2 (nearly straight lines in
  the source, p.6) to about 2 % of full scale.
- From images: the overbar of Y = !{A} (p.1 and p.8); the overbar of
  !{Q} (Figure 8-1, p.10); the curve points -0.5, 4.99, 4.455, 4.385,
  0.01, 0.42, 0.44, 0.48 (Figures 5-1 and 5-2, p.6); 0.1 µF
  (Figure 8-3, p.13; the text layer prints "0.1 F"); ±2000 and ±1000
  (ESD ratings, p.4; the text layer breaks the values over two lines).
- Source issues:
  - Source conflict: minimum supply voltage of 3 V and V~IH~/V~IL~ rows
    at 3 V against the 4.5 V to 5.5 V operating range (Recommended
    operating conditions note).
  - Unconfirmed: two level names side by side in "Input signals must
    cross" (Input considerations note).
  - Source conflict: fast input transitions required against no input
    transition rate requirements (Input considerations note).
  - Source conflict: additional current said to be plotted in the
    Typical Characteristics, which has no such figure (Input
    considerations note).
  - Source conflict: two channels named for a single gate (Output
    considerations note).
  - Source error: "SN74AHCT1G14-Q1can" is printed without a space
    (p.10); kept as printed.
