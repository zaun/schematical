---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct32-q1",
  "title": "SN74AHCT32-Q1 automotive quadruple 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ahct32-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS528D",
    "date": "2024-02",
    "pages": 28
  },
  "devices": ["gate:74_32"],
  "parts": ["SN74AHCT32QDRG4Q1", "SN74AHCT32QDRQ1", "SN74AHCT32QPWRG4Q1", "SN74AHCT32QPWRQ1", "SN74AHCT32QWBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-21"
}
---

# SN74AHCT32-Q1 automotive quadruple 2-input positive-OR gate

## Description

The SN74AHCT32 is a quadruple 2-input positive-OR gate. This device
performs the Boolean function Y = !{A × B} or Y = A + B in positive
logic. Source error: the source draws one bar over the whole of A × B,
which is not the OR function that Y = A + B and the function table
give; kept as printed. [[1::2]]

AEC-Q100 qualified for automotive applications. Device temperature
grade 1: -40°C to +125°C. Device HBM ESD classification level 2. Device
CDM ESD classification level C4B. Available in wettable flank QFN (WBQA)
package. Operating range of 4.5V to 5.5V. Inputs are TTL-voltage
compatible. [[1::1]]

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
GND: Ground Pin
3Y: 3Y Output
3A: 3A Input
3B: 3B Input
4Y: 4Y Output
4A: 4A Input
4B: 4B Input
VCC: Power Pin
```
[[3::Table 4-1]]

The thermal pad can be connected to GND or left floating. Do not connect
to any other signal or supply. BQA Package Only. [[3::1]]

## Functional description

The SN74AHCT32-Q1 is a quadruple 2-input positive-OR gate with low drive
that will produce slow rise and fall times. This slow transition reduces
ringing on the output signal. The device has TTL inputs that allow up
translation from 3.3 V to 5 V. The inputs are high impedance when
V~CC~ = 0 V. [[8::1]]

Conversion note: the functional block diagram shows one 2-input OR gate with inputs A and B and output Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```
[[8::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1], All pins | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011, All pins [2] | ±100 | ^ |

[1] AEC Q100-002 indicates HBM stressing is done in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
[2] Unconfirmed: the CDM value is printed ±100 V, while the feature list (p.1) gives CDM ESD classification level C4B.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ [2] | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs.
[2] Source error: the source prints this symbol as "IH", without the V; the row is the high-level input voltage, next to V~IL~.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHCT32-Q1 | < | < | Unit |
| ^ | ^ | D | PW | BQA | ^ |
| ^ | ^ | 14 pins | 14 pins | 14 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.6 | 147.7 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | V |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| ΔI~CC~ [1] | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #1]]

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | SN74AHCT32-Q1 | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.5 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 11.5 | pF |
```
[[5::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5 | 6.9 | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 6.9 | 1 | 8 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9 | ns |
```
[[5::table #4]]

## Typical characteristics

T~A~ = 25°C (unless otherwise noted) [[6::1]]

```curve id=voh-ioh caption="Figure 5-1. Output voltage vs current in HIGH state; 5-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.3, 5]},
 "series": [
   {"label": "-40°C", "points": [[-25, 4.5], [-12.5, 4.755], [-0.5, 5]]},
   {"label": "25°C", "points": [[-25, 4.455], [-12.5, 4.73], [-0.5, 5]]},
   {"label": "125°C", "points": [[-25, 4.385], [-12.5, 4.7], [-0.5, 5]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=vol-iol caption="Figure 5-2. Output voltage vs current in LOW state; 5-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.5]},
 "series": [
   {"label": "-40°C", "points": [[0.5, 0.01], [12.5, 0.21], [25, 0.418]]},
   {"label": "25°C", "points": [[0.5, 0.01], [12.5, 0.222], [25, 0.442]]},
   {"label": "125°C", "points": [[0.5, 0.01], [12.5, 0.242], [25, 0.483]]}
 ]}
```
[[6::Figure 5-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::Figure 6-1]]

Conversion note: C~L~ is 15 pF or 50 pF, as given in the [switching characteristics](#sw); in the figure the inputs swing 0 V to 3 V and are measured at 1.5 V, and the outputs are measured at 50 % V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50 % VCC (output)
```
[[7::Figure 6-1]]

Conversion note: the source figure also shows the 3-state and open-drain load circuit and the pulse-duration, setup-and-hold and enable/disable waveforms.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality. [[9::1]]

In this application, three 2-input OR gates are combined to produce a
4-input OR gate function as shown in Typical Application Block Diagram.
The fourth gate can be used for another application in the system, or
the inputs can be grounded and the channel left unused. [[9::2]]

The SN74AHCT32-Q1 is used to directly control the Enable pin of a fan
driver. The fan driver requires only one input signal to be HIGH before
being enabled, and should be disabled in the event that all signals go
LOW. The 4-input OR gate function combines the four individual overheat
signals into a single active-high enable signal. [[9::3]]

Temperature sensors can often be spread throughout a system rather than
being in a centralized location. This would mean longer length traces or
wires to pass signals through leading to slower edge transitions. This
makes the SN74AHCT32-Q1 useful for combining the incoming signals. [[9::4]]

Conversion note: in the typical application block diagram (Figure 8-1) the overheat signals of Device 1 to Device 4 are combined by three OR gates whose output drives the EN input of the fan driver.

```wavedrom id=app-timing caption="Figure 8-2. Application timing diagram"
{"signal": [
  {"name": "Device 1", "wave": "0...1.......0........10."},
  {"name": "Device 2", "wave": "0.......1.....0........."},
  {"name": "Device 3", "wave": "0...1...0..............."},
  {"name": "Device 4", "wave": "0.....1.....0..........."},
  {"name": "EN", "wave": "0...1.........0......10."}
]}
```
[[11::Figure 8-2]]

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section. [[9::6]]

The positive voltage supply must be capable of sourcing current equal to
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. [[9::7]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHCT32-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings. [[10::1]]

The SN74AHCT32-Q1 can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50 pF. [[10::2]]

The SN74AHCT32-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the HIGH
state, the output voltage in the equation is defined as the difference
between the measured output voltage and the supply voltage at the V~CC~
pin. [[10::3]]

Total power consumption can be calculated using the information provided
in CMOS Power Consumption and Cpd Calculation application note. [[10::4]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note. [[10::5]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.
Source conflict: this caution (p.10) says T~J(max)~ is listed in the Absolute Maximum Ratings, while that table (p.4) has no junction temperature row. [[10::6]]

### Input considerations

Input signals must cross to be considered a logic LOW, and to be
considered a logic HIGH. Source error: the source leaves out the two
levels the signals must cross in this sentence; kept as printed. Do not
exceed the maximum input voltage range found in the Absolute Maximum
Ratings. [[10::7]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHCT32-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[10::8]]

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[10::10]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[10::11]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[11::1]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74AHCT32-Q1 to one or more of the receiving devices. [[11::1]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[11::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[11::1]]

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX
supply-voltage rating located in Section 5.3. [[11::2]]

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in Layout Example. [[11::3]]

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
convenient. [[11::4]]

Conversion note: the layout example (Figure 8-3) recommends a GND flood fill, a 0.1 µF bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74AHCT32QDRG4Q1 | Reel of 2500 | -40 to 125 | AHCT32Q
SN74AHCT32QDRQ1 | Reel of 2500 | -40 to 125 | AHCT32Q
SN74AHCT32QPWRG4Q1 | Reel of 2000 | -40 to 125 | AHCT32Q | NRND
SN74AHCT32QPWRQ1 | Reel of 2000 | -40 to 125 | AHCT32Q, HB32Q
SN74AHCT32QWBQARQ1 | Reel of 3000 | -40 to 125 | AHT32Q
```
[[14::table #1]]

## Conversion notes

- Source pages 1–12 converted; the support and revision history page
  (13), the package option addendum (p.14–15), tape and reel and
  mechanical pages (16–28) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: features and applications lists (features kept as sentences
  in Description, except the 10µA maximum I~CC~ and ±8mA output drive
  lines), package information table, logic symbol and logic diagram, the
  layout example drawing (summarized in one sentence), the references to
  the Feature Description section in the input and output
  considerations (the source has no section of that name).
- The typical characteristics curves are digitized from Figures 5-1 and
  5-2 (p.6) to about 2 % of full scale.
- The functional block diagram and the typical application block diagram
  (Figure 8-1) are described in text; the application timing diagram is
  redrawn from Figure 8-2 (p.11).
- The Electrical characteristics table of the source names its
  parameters by symbol only, and heads its last two value columns MIN
  and MAX with no temperature; neither is added.
- From images: the overbars of Y = !{A × B} (p.1); the curve points of
  Figures 5-1 and 5-2 (p.6); 0.1 µF (Figure 8-3, p.12; the text layer
  drops the µ).
- From images: page 7 (Figure 6-1 and its notes A to E) is a picture
  with no text layer: the Parameter measurement information sentences,
  "C~L~ includes probe and jig capacitance.", "All input pulses are
  supplied by generators having the following characteristics:
  PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns.", "The outputs
  are measured one at a time with one input transition per
  measurement." and "All parameters and waveforms are not applicable to
  all devices.", the 0 V, 3 V and 1.5 V input levels and 50 % V~CC~ are
  read from it.
- Source issues:
  - Source error: the Boolean function on p.1 is drawn with one bar over
    A × B (Description note).
  - Source error: recommended operating conditions print the V~IH~
    symbol as "IH" (table note).
  - Source error: Input considerations (p.10) leaves the two levels out
    of "Input signals must cross to be considered a logic LOW, and to be
    considered a logic HIGH"; kept as printed.
  - Source conflict: the caution on p.10 refers to T~J(max)~ listed in
    the Absolute Maximum Ratings; the table on p.4 has no such row.
  - Unconfirmed: CDM ESD value ±100 V (p.4) beside classification level
    C4B (p.1).
  - Source conflict: the pinout figure caption (p.3) calls the BQA
    package "Preview", while the package option addendum (p.14) lists
    SN74AHCT32QWBQARQ1 as Active.
