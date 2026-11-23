---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g86-q1",
  "title": "SN74AHCT1G86Q-Q1 automotive single 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74ahct1g86-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS721B",
    "date": "2024-01",
    "pages": 25
  },
  "devices": ["gate:1G86"],
  "parts": ["CAHCT1G86QDBVRQ1", "CAHCT1G86QDCKRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G86Q-Q1 automotive single 2-input exclusive-OR gate

## Description

The SN74AHCT1G86Q-Q1 is a single 2-input exclusive-OR gate. The device
performs the Boolean function Y = A ⊕ B or Y = !{A}B + A!{B} in positive
logic. [[1::1]]

AEC-Q100 qualified for automotive applications: Device temperature
grade 1: -40°C to +125°C. Device HBM ESD classification level 2. Device
CDM ESD classification level C4B. Operating range of 4.5V to 5.5V. Max
t~pd~ of 8ns at 5V. Low power consumption, 10μA max I~CC~. ±8mA output
drive at 5V. Inputs are TTL-voltage compatible. [[1::2]] [[1::3]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: A input
B: B input
GND: Ground pin
Y: Output
VCC: Power pin
```
[[3::table #1]]

## Functional description

Source error: the Overview (p.7) says the SN74AHCT1G86Q-Q1 "contains four independent 2-input AND Gates with Schmitt-trigger inputs", gives Y = A × B and CMOS output levels from 1.8-V to 5-V; the function table (p.8) is that of one exclusive-OR gate and the supply range is 4.5 V to 5.5 V. Kept out.

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. Conversion note: the
functional block diagram (Figure 7-1, a picture) shows five equivalent
exclusive-OR symbols and the logic-identity, even-parity and odd-parity
elements; it is not drawn here. [[7::2]]

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold. [[7::3]]

TTL-compatible CMOS inputs are high impedance and are typically modeled
as a resistor in parallel with the input capacitance given in the
[Electrical Characteristics](#electrical-characteristics). The worst
case resistance is calculated with the maximum input voltage, given in
the [Absolute Maximum Ratings](#absolute-maximum-ratings), and the
maximum input leakage current, given in the Electrical Characteristics,
using Ohm's law (R = V ÷ I). [[7::4]]

TTL-compatible CMOS inputs require that input signals transition
between valid logic states quickly, as defined by the input transition
time or rate in the
[Recommended Operating Conditions](#recommended-operating-conditions)
table. Failing to meet this specification will result in excessive
power consumption and could cause oscillations. More details can be
found in the Implications of Slow or Floating CMOS Inputs application
report. [[7::5]]

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a
system will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors;
however, a 10-kΩ resistor is recommended and will typically meet all
requirements. [[7::6]]

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due
to overcurrent. The electrical and thermal limits defined in the
Absolute Maximum Ratings must be followed at all times. [[7::7]]

Unused push-pull CMOS outputs should be left disconnected. [[8::1]]

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes
only as shown in Figure 7-2. [[8::2]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[8::3]]

Conversion note: Figure 7-2 shows one clamping diode from GND to the input (-I~IK~), one from GND to the output (-I~OK~) and one from the output to V~CC~ (+I~OK~).

## Function tables

```table id=function caption="Functional table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[8::table #1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ [1] | Input voltage | | -0.5 | 7 | V |
| V~O~ [1] | Output voltage range applied in the high- or low-state | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 V | | -20 | V [2] |
| I~OK~ | Output clamp current | V~O~ < 0 V or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 V to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[2] Source error: the source prints the unit V for this current; the I~OK~ and I~O~ rows of the same table are in mA. Kept as printed.
```
[[4::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| Δt/ΔV | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |
```
[[4::table #2]]

## Thermal information

```table id=thermal caption="Thermal information: thermal metric [1]"
| Symbol | Parameter | DBV (SOT-23) 5 pins | DCK (SC-70) 5 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278.0 | 289.2 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[4::table #3]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 μA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | ^ | 3.94 | | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 μA | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | μA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 A | 5.5 V | | | 1 | | 10 | μA |
| ΔI~CC~ [1] | | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~I~ | | V~I~ = V~CC~ or GND | 5 V | | | 10 | | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[4::table #4]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V ±0.5 V, T~A~ = –40°C to 125°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```
[[5::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ±0.5 V, T~A~ = –40°C to 125°C, see Load Circuit and Voltage Waveforms"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9 | ns |
```
[[5::table #2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤3 ns. The outputs are measured one at a
time with one transition per measurement. All parameters and waveforms
are not applicable to all devices. [[6::1]]

Conversion note: C~L~ is 50 pF, as given in the [switching characteristics](#sw); in the figure the inputs swing 0 V to 3 V and are measured at 1.5 V, and the outputs are measured at 50% V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 50 pF
```
[[6::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[6::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

In this application, a 2-input XOR gate is used as a phase difference
detector as shown in Figure 8-1. [[9::2]]

The SN74AHCT1G86Q-Q1 is used to identify phase difference between a
reference clock and another input clock. Whenever the clock states are
different, the XOR output will pulse HIGH until the clocks return to
the same state. The output is fed into a low-pass filter to obtain a DC
representation of the phase difference. [[9::3]]

Typically, clock signals have fast transition rates, but additional
filtering can be added to the clock signals which can lead to slower
transitions rates. This makes the SN74AHCT1G86Q-Q1 an excellent choice
for the application because it has Schmitt-trigger inputs that do not
have input transition rate requirements. [[9::4]]

Source conflict: § 8.1 (p.9) says the device "has Schmitt-trigger inputs that do not have input transition rate requirements"; § 7.3.1 (p.7) describes TTL-compatible CMOS inputs that require quick transitions, and the recommended operating conditions (p.4) limit Δt/ΔV to 20 ns/V. Both are kept as printed.

Conversion note: the typical application block diagram (Figure 8-1) shows the gate with Input Clock and Reference Clock on its inputs and its output passing through a series R, with C to ground, to Output.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
[Recommended Operating Conditions](#recommended-operating-conditions).
The supply voltage sets the electrical characteristics of the device as
described in the Electrical Characteristics section. [[9::6]]

The positive voltage supply must be capable of sourcing current equal
to the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. [[9::7]]

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74AHCT1G86Q-Q1 plus the
maximum supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only sink as much current that can be sunk into its
ground connection. Be sure to not exceed the maximum total current
through GND listed in the Absolute Maximum Ratings. [[9::8]]

The SN74AHCT1G86Q-Q1 can drive a load with a total capacitance less
than or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF. [[9::9]]

The SN74AHCT1G86Q-Q1 can drive a load with total resistance described
by R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in
the Electrical Characteristics table with V~OL~. When outputting in the
HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin. [[9::10]]

Total power consumption can be calculated using the information
provided in the CMOS Power Consumption and Cpd Calculation application
note. [[9::11]]

Thermal increase can be calculated using the information provided in
the Thermal Characteristics of Standard Linear and Logic (SLL) Packages
and Devices application note. [[9::12]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[10::1]]

Source conflict: this caution (p.10) says T~J(max)~ is listed in the Absolute Maximum Ratings; that table (p.4) lists no junction temperature.

### Input considerations

Input signals must cross to be considered a logic LOW, and to be
considered a logic HIGH. Do not exceed the maximum input voltage range
found in the Absolute Maximum Ratings. [[10::2]]

Source error: the sentence "Input signals must cross to be considered a logic LOW, and to be considered a logic HIGH" (p.10) names no levels; kept as printed.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHCT1G86Q-Q1 (as specified in the Electrical Characteristics),
and the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[10::3]]

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics. [[10::5]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[10::6]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[11::1]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. This can
   be accomplished by providing short, appropriately sized traces from
   the SN74AHCT1G86Q-Q1 to one or more of the receiving devices. [[11::1]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω, so that the maximum output current from the
   Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in MΩ; much larger than the minimum
   calculated previously. [[11::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[11::1]]

Conversion note: the application timing diagram (Figure 8-2) shows Out high whenever Reference Clock and Input Clock differ.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Section 5.2. [[11::2]]

Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. For devices with a single supply, TI recommends a
0.1-μF capacitor and if there are multiple V~CC~ terminals then TI
recommends a 0.01-μF or 0.022-μF capacitor for each power terminal.
Multiple bypass capacitors can be paralleled to reject different
frequencies of noise. Frequencies of 0.1 μF and 1 μF are commonly used
in parallel. The bypass capacitor should be installed as close as
possible to the power terminal for best results. [[11::3]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4
buffer gates are used). Such unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a logic high or logic low voltage,
as defined by the input voltage specifications, to prevent them from
floating. The logic level that must be applied to any particular unused
input depends on the function of the device. Generally, the inputs are
tied to GND or V~CC~, whichever makes more sense for the logic function
or is more convenient. [[11::4]]

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
CAHCT1G86QDBVRQ1 | Reel of 3000 | -40 to 125 | 39FH
CAHCT1G86QDCKRQ1 | Reel of 3000 | -40 to 125 | B5S
```
[[15::table #1]]

## Conversion notes

- Source pages 1–12 converted. The ordering lines are from the package
  option addendum (pp.15–16). The support and revision history pages
  (13–14), tape and reel and mechanical pages (17–25) are left out; the
  generated Package section replaces them.
- Dropped: features and applications lists (features kept as sentences
  in the Description), package information table, the exclusive-OR
  logic picture (p.1 and Figure 7-1, described in one sentence), the
  two "Refer to the Feature Description section" sentences of § 8.2.1;
  the clamping-diode figure (Figure 7-2), the application block diagram
  (Figure 8-1), the application timing diagram (Figure 8-2) and the
  example layout (Figure 8-3) are described in text.
- The pin table's note "Signal Types: I = Input, O = Output, I/O =
  Input or Output" is on its TYPE column, which the generated pin table
  draws from the catalog.
- The source has no ESD ratings table and no typical characteristics.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty. Its last Min and Max columns, and
  those of the switching table, have no group header in the source; the
  switching table's condition line gives T~A~ = –40°C to 125°C.
- From images: the overbars of Y = !{A}B + A!{B} (p.1); 0 V, 3 V, 1.5 V
  input levels and 50% V~CC~ (Figure 6-1, p.6); 0.1 µF (Figure 8-3,
  p.12; the text layer prints "0.1 F").
- Source issues:
  - Source error: the Overview describes four AND gates with
    Schmitt-trigger inputs (Functional description note).
  - Source error: I~IK~ printed with the unit V, not mA; kept as
    printed (absolute maximum ratings note 2).
  - Source conflict: "Schmitt-trigger inputs that do not have input
    transition rate requirements" (§ 8.1) vs TTL-compatible CMOS inputs
    and Δt/ΔV of 20 ns/V (Application information note).
  - Source conflict: T~J(max)~ said to be listed in the Absolute
    Maximum Ratings, which list none (Application information note).
  - Source error: "Input signals must cross to be considered a logic
    LOW, and to be considered a logic HIGH" names no levels; kept as
    printed.
  - Source error: "Frequencies of 0.1 μF and 1 μF are commonly used in
    parallel" (§ 8.3) calls capacitor values frequencies; kept as
    printed.
