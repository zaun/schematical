---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc32q-q1",
  "title": "SN74AHC32Q-Q1 automotive quadruple 2-input positive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ahc32q-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SGDS019D",
    "date": "2024-02",
    "pages": 31
  },
  "devices": ["gate:74_32"],
  "parts": ["SN74AHC32QDRG4Q1", "SN74AHC32QDRQ1", "SN74AHC32QPWRG4Q1", "SN74AHC32QPWRQ1", "SN74AHC32QWBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74AHC32Q-Q1 automotive quadruple 2-input positive-OR gates

## Description

The SN74AHC32Q-Q1 devices are quadruple 2-input positive-OR gates.
These devices perform the Boolean function Y = A + B in positive logic. [[1::1]]

Source error: the source prints "Y = !{A × B} or Y = A + B" (one bar
over A × B); the function table (Table 7-1) gives Y = L only when A and
B are both L, so !{A × B} (a NAND) is kept out.

Qualified for automotive applications. Operating range 2V to 5.5V
V~CC~. Low power consumption, 10µA maximum I~CC~. ±8mA output drive at
5V. Latch-up performance exceeds 250mA per JESD 17. [[1::2]]

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
[[3::Table 4-1]]

Conversion note: the pin table also lists a Thermal Pad, BQA Package Only.

## Functional description

The SN74AHC32Q-Q1 contains four independent 2-input OR Gates. Each gate
performs the Boolean function Y = A + B in positive logic. [[9::1]]

Conversion note: the functional block diagram shows one 2-input OR gate with inputs xA and xB and output xY.

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are
high impedance and are typically modeled as a resistor in parallel with
the input capacitance given in the Electrical Characteristics. The
worst case resistance is calculated with the maximum input voltage,
given in the Absolute Maximum Ratings, and the maximum input leakage
current, given in the Electrical Characteristics, using Ohm's law
(R = V ÷ I). [[9::2]]

Standard CMOS inputs require that input signals transition between
valid logic states quickly, as defined by the input transition time or
rate in the Recommended Operating Conditions table. Failing to meet
this specification will result in excessive power consumption and could
cause oscillations. More details can be found in Implications of Slow
or Floating CMOS Inputs. [[9::3]]

Do not leave standard CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a
system will not be actively driving an input at all times, then a
pull-up or pull-down resistor can be added to provide a valid input
voltage during these times. The resistor value will depend on multiple
factors; a 10-kΩ resistor, however, is recommended and will typically
meet all requirements. [[9::4]]

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times. [[9::5]]

Unused push-pull CMOS outputs should be left disconnected. [[9::6]]

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes
only as shown in Figure 7-1. [[9::7]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[9::8]]

Conversion note: Figure 7-1 shows one clamping diode from GND to the input (-I~IK~), one from GND to the output (-I~OK~) and one from the output to V~CC~ (+I~OK~).

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | H | H |
| H | L | H |
| L | L | L |

[1] H = high voltage level, L = low voltage level, X = do not care, Z = high impedance
```
[[10::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 HBM ESD Classification Level 2 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C4B | ±1000 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage [1] | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | Low-level Input voltage [1] | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input Transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC32Q-Q1, 14 pins"
| Symbol | Thermal metric [1] | D | PW | BQA | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.6 | 147.7 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.58 | | | 2.48 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA [1] | 3 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | I~OL~ = 8 mA [1] | 4.5 V | | | 0.36 | | 0.5 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | pF |

[1] Source error: the source prints I~OH~ = 4 mA and I~OH~ = 8 mA on these V~OL~ rows; the recommended operating conditions give 4 mA and 8 mA as I~OL~.
```
[[6::table #1]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.3 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.3 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.7 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```
[[7::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 14 | pF |
```
[[7::table #2]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 5.5 | 7.9 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.5 | 7.9 | 1 | 9.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 8 | 11.4 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | 8 | 11.4 | 1 | 13 | ns |
```
[[6::table #2]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 3.8 | 5.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 3.8 | 5.5 | 1 | 6.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 5.3 | 7.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.3 | 7.5 | 1 | 8.5 | ns |
```
[[6::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[8::1]]

Conversion note: C~L~ is 15 pF or 50 pF, as given in the [switching characteristics](#switching-characteristics); in the figure the inputs swing 0 V to V~CC~ and inputs and outputs are measured at 50% V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[8::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[8::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load circuit for 3-state and open-drain outputs with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[11::1]]

In this application, three 2-input OR gates are combined to produce a
4-input OR gate function as shown in Figure 8-1. The fourth gate can be
used for another application in the system, or the inputs can be
grounded and the channel left unused. [[11::2]]

The SN74AHC32Q-Q1 is used to directly control the Enable pin of a fan
driver. The fan driver requires only one input signal to be HIGH before
being enabled, and should be disabled in the event that all signals go
LOW. The 4-input OR gate function combines the four individual overheat
signals into a single active-high enable signal. [[11::3]]

Temperature sensors can often be spread throughout a system rather than
being in a centralized location. This would mean longer length traces
or wires to pass signals through leading to slower edge transitions.
This makes the SN74AHC32Q-Q1 useful for combining the incoming signals. [[11::4]]

Conversion note: the typical application block diagram (Figure 8-1) shows the Device 1 and Device 2 Overheat signals on one OR gate, the Device 3 and Device 4 Overheat signals on a second, and a third OR gate combining the two outputs to drive the EN pin of the Fan Driver.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the
electrical characteristics of the device as described in the Electrical
Characteristics section. [[11::6]]

The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the SN74AHC32Q-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings. [[11::7]]

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74AHC32Q-Q1 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into its ground connection. Be
sure to not exceed the maximum total current through GND listed in the
Absolute Maximum Ratings. [[12::1]]

The SN74AHC32Q-Q1 can drive a load with a total capacitance less than
or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF. [[12::2]]

The SN74AHC32Q-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin. [[12::3]]

Total power consumption can be calculated using the information
provided in CMOS Power Consumption and Cpd Calculation application
note. [[12::4]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note. [[12::5]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[12::6]]

Source conflict: this caution (p.12) says T~J(max)~ is listed in the Absolute Maximum Ratings; that table (p.4) lists no junction temperature.

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[12::7]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHC32Q-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[12::8]]

The SN74AHC32Q-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability. [[12::9]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics. [[12::11]]

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This
can cause excessive current and damage to the device. [[12::12]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[12::13]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[12::14]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[13::1]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74AHC32Q-Q1 to one or more of the receiving devices. [[13::1]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[13::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[13::1]]

Conversion note: the application timing diagram (Figure 8-2) shows EN high whenever any of Device 1 to Device 4 is high.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1-μF capacitor is recommended for this device.
It is acceptable to parallel multiple bypass capacitors to reject
different frequencies of noise. The 0.1-μF and 1-μF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results, as shown in
Layout Example. [[13::2]]

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
or is more convenient. [[13::3]]

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74AHC32QDRG4Q1 | Reel of 2500 | -40 to 125 | AHC32Q
SN74AHC32QDRQ1 | Reel of 2500 | -40 to 125 | AHC32Q
SN74AHC32QPWRG4Q1 | Reel of 2000 | -40 to 125 | HA32Q
SN74AHC32QPWRQ1 | Reel of 2000 | -40 to 125 | HA32Q
SN74AHC32QWBQARQ1 | Reel of 3000 | -40 to 125 | AHC32Q
```
[[17::table #1]]

## Conversion notes

- Source pages 1–14 converted. The ordering lines are from the package
  option addendum (p.17). The support and revision history page (15),
  page 16, tape and reel and mechanical pages (18–31) are left out; the
  generated Package section replaces them.
- Dropped: applications list (features kept as sentences in the
  Description), package information table, the logic diagram (p.1, one
  OR gate), the two "Refer to the Feature Description section" sentences
  of § 8.2.1; the functional block diagram, the clamping-diode figure
  (Figure 7-1), the application block diagram (Figure 8-1), the
  application timing diagram (Figure 8-2) and the example layout
  (Figure 8-3) are described in text.
- The pin table's note "Signal Types: I = Input, O = Output, I/O =
  Input or Output" is on its TYPE column, which the generated pin table
  draws from the catalog.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty. Its last Min and Max columns have no
  group header in the source (the cell is blank, and the unit header is
  printed "NIT"); the switching tables head the same columns
  T~A~ = –40°C to 125°C.
- From images: the overbar of !{A × B} (Description, p.1); 50% V~CC~ and
  the 0 V to V~CC~ input levels (Figure 6-1, p.8); 0.1 µF (Figure 8-3,
  p.14; the text layer prints "0.1 F").
- Source issues:
  - Source error: Description formula "Y = !{A × B} or Y = A + B"
    (Description note).
  - Source error: I~OH~ printed for I~OL~ on the 4 mA and 8 mA V~OL~
    rows (electrical characteristics note 1).
  - Source conflict: T~J(max)~ said to be listed in the Absolute
    Maximum Ratings, which list none (Application information note).
